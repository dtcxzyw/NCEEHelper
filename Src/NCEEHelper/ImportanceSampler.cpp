#include "../Shared/TestEngine.hpp"
#include <chrono>
#include <deque>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
#include <unordered_map>

using Clock = std::chrono::high_resolution_clock;

BUS_MODULE_NAME("NCEEHelper.Builtin.ImportanceSampler");

struct TestHistory final {
    uint32_t passCnt, testCnt, lastHistory, lastTime;
    double weight;
    TestHistory() : passCnt(0), testCnt(0), lastHistory(0), lastTime(0) {}
};

class ImportanceSampler final : public TestEngine {
private:
    static constexpr size_t queueSize = 1500ULL;
    std::unordered_map<GUID, TestHistory, GUIDHasher> mHistory;
    std::deque<bool> mQueue;
    std::vector<GUID> mMap;
    std::discrete_distribution<size_t> mDistrib;
    std::mt19937_64 mRNG;
    std::unique_ptr<std::ofstream> mOutput;
    fs::path mOutputPath;
    uint64_t mCurrent, mInvalid, mValid;
    std::vector<Ratio> mAResults;
    std::vector<GUID> mNew;
    Ratio analyseHistory() {
        BUS_TRACE_BEG() {
            Ratio res{ 0U, 0.0, 0.0, 0.0, 0.0 };
            if(mHistory.empty())
                return res;
            uint32_t pass = 0, test = static_cast<uint32_t>(mQueue.size()),
                     coverage = 0, master = 0, m1 = 0, m2 = 0;
            for(auto&& x : mQueue)
                pass += x;
            for(auto&& x : mHistory) {
                TestHistory& his = x.second;
                coverage += (his.passCnt != 0);
                master += ((his.lastHistory & 7U) == 7U);
                m1 += ((his.lastHistory & 1U) == 1U);
                m2 += ((his.lastHistory & 3U) == 3U);
            }
            res.count = mValid;
            double tot = static_cast<double>(mHistory.size());
            if(test)
                res.accuracy = pass / static_cast<double>(test);
            res.coverage = coverage / tot;
            res.master = master / tot;
            res.emaster = (master + m1 + m2) / (3 * tot);
            return res;
        }
        BUS_TRACE_END();
    }
    void loadRecord(const fs::path& historyFile) {
        BUS_TRACE_BEG() {
            std::ifstream in(historyFile);
            std::stringstream ss;
            ss << historyFile.stem().string();
            uint64_t t;
            ss >> std::uppercase >> std::hex >> t;
            constexpr uint64_t perDay =
                86400 * std::chrono::system_clock::period::den;
            uint32_t day = static_cast<uint32_t>((mCurrent - t) / perDay) + 1U;
            reporter().apply(ReportLevel::Info,
                             "Loading history " + ss.str() + " (" +
                                 std::to_string(day) + " days ago)",
                             BUS_DEFSRCLOC());
            std::string line;
            size_t lineCnt = 0;
            while(std::getline(in, line)) {
                ++lineCnt;
#define FAIL()                                                                \
    {                                                                         \
        reporter().apply(ReportLevel::Error,                                  \
                         "Bad record in " + historyFile.string() + " line " + \
                             std::to_string(lineCnt),                         \
                         BUS_DEFSRCLOC());                                    \
        continue;                                                             \
    }
                if(line.size() != 39)
                    FAIL();
                if(line.front() != 'T' && line.front() != 'F')
                    FAIL();
                bool res = (line.front() == 'T');
                GUID guid{ 0, 0 };
                try {
                    guid = str2GUID(line.substr(1));
                } catch(...) {
                }
                if(guid.first == 0 && guid.second == 0)
                    FAIL();
                auto iter = mHistory.find(guid);
                if(iter != mHistory.end()) {
                    TestHistory& his = iter->second;
                    his.passCnt += res;
                    ++his.testCnt;
                    his.lastHistory =
                        ((his.lastHistory << 1) | static_cast<uint32_t>(res));
                    his.lastTime = day;
                    ++mValid;
                    mQueue.push_back(res);
                    while(mQueue.size() > queueSize)
                        mQueue.pop_front();
                } else
                    ++mInvalid;
            }
        }
        BUS_TRACE_END();
    }
    void buildAccBuffer(bool masterMode) {
        BUS_TRACE_BEG() {
            if(masterMode)
                reporter().apply(ReportLevel::Debug, "masterMode ON",
                                 BUS_DEFSRCLOC());
            std::vector<double> weights;
            mMap.clear();
            for(auto& key : mHistory) {
                TestHistory& his = key.second;
                // accuracy 60%
                double weight = 12.0 *
                    std::min(5.0,
                             (his.testCnt + 0.001) / (his.passCnt + 0.001));
                // new knowledge 10%
                if(his.testCnt <= 3)
                    weight += 10.0 - his.testCnt * 3;
                // time 10%
                weight += std::min(his.lastTime, 20U) * 0.5;
                // forget 20%
                if((his.lastHistory & 1) == 0)
                    weight += 20.0;
                // master
                if((his.lastHistory & 7U) == 7U)
                    weight *= std::max(
                        0.005, 1.0 - his.passCnt / (his.testCnt + 0.001));
                // coverage
                if(his.passCnt == 0) {
                    weight = 100.0;
                    mNew.emplace_back(key.first);
                }
                his.weight = weight;
                if((!masterMode) ||
                   ((his.lastHistory & 7U) != 7U && his.passCnt)) {
                    mMap.emplace_back(key.first);
                    weights.emplace_back(weight);
                }
            }
            mDistrib = { weights.begin(), weights.end() };
            if(masterMode)
                reporter().apply(ReportLevel::Debug,
                                 "sublibrary size:" +
                                     std::to_string(mMap.size()),
                                 BUS_DEFSRCLOC());
        }
        BUS_TRACE_END();
    }

public:
    explicit ImportanceSampler(Bus::ModuleInstance& instance)
        : TestEngine(instance) {}
    void init(const fs::path& history, const GUIDTable& table,
              bool masterMode) override {
        BUS_TRACE_BEG() {
            if(table.empty())
                BUS_TRACE_THROW(std::logic_error("No knowledge point!!!"));
            for(auto id : table)
                mHistory[id] = {};
            std::set<fs::path> logs;
            for(auto p : fs::directory_iterator(history)) {
                if(p.is_regular_file()) {
                    auto path = p.path();
                    if(path.extension() == ".log")
                        logs.emplace(path);
                }
            }
            mCurrent =
                std::chrono::system_clock::now().time_since_epoch().count();
            mInvalid = mValid = 0;
            {
                std::stringstream ss;
                ss << std::hex << std::uppercase << mCurrent;
                reporter().apply(ReportLevel::Info, "Timestamp " + ss.str(),
                                 BUS_DEFSRCLOC());
                ss << ".log";
                mOutputPath = history / ss.str();
            }
            for(auto log : logs) {
                loadRecord(log);
                mAResults.emplace_back(analyseHistory());
            }
            buildAccBuffer(masterMode);
            /*
            reporter().apply(ReportLevel::Debug,
                             "random_device entropy=" +
                                 std::to_string(mRNG.entropy()),
                             BUS_DEFSRCLOC());
                             */
            reporter().apply(ReportLevel::Debug,
                             "new size: " + std::to_string(mNew.size()),
                             BUS_DEFSRCLOC());
            // std::shuffle(mNew.begin(), mNew.end(), mRNG);
            std::random_device rdv;
            mRNG.seed(std::seed_seq({ rdv() }));
            if(mNew.size() > static_cast<size_t>(20))
                mNew.resize(static_cast<size_t>(20));
        }
        BUS_TRACE_END();
    }
    GUID sampleTest() override {
        BUS_TRACE_BEG() {
            if(mMap.empty() && mNew.empty())
                BUS_TRACE_THROW(std::runtime_error("Empty library."));
            GUID res{};
            if(mNew.empty()) {
                res = mMap[mDistrib(mRNG)];
            } else {
                res = mNew.back();
                mNew.pop_back();
            }
            std::cout << GUID2Str(res) << std::endl;
            TestHistory& his = mHistory[res];
            std::cout << "Weight:" << his.weight << " History:";
            for(uint32_t i = 0; i < std::min(32U, his.testCnt); ++i)
                std::cout << ((his.lastHistory >> i) & 1 ? 'T' : 'F');
            std::cout << " Time:" << his.lastTime << " days ago" << std::endl;
            return res;
        }
        BUS_TRACE_END();
    }
    std::string summary() override {
        BUS_TRACE_BEG() {
            uint32_t pass = 0, test = static_cast<uint32_t>(mValid),
                     coverage = 0, master = 0, m1 = 0, m2 = 0;
            std::vector<std::pair<double, GUID>> top;
            for(auto&& x : mHistory) {
                TestHistory& his = x.second;
                pass += his.passCnt;
                coverage += (his.passCnt != 0);
                master += ((his.lastHistory & 7U) == 7U);
                m1 += ((his.lastHistory & 1U) == 1U);
                m2 += ((his.lastHistory & 3U) == 3U);
                if(his.passCnt)
                    top.push_back(std::make_pair(his.weight, x.first));
            }
            std::stringstream ss;
            ss.precision(2);
            ss << std::fixed << "TestCount: " << test << " PassCount: " << pass
               << " Invalid: " << mInvalid << " Accuracy: ";
            if(test)
                ss << (static_cast<double>(pass) / test) * 100.0 << "%";
            else
                ss << "N/A";
            ss << std::endl;
            ss << "Coverage: " << (coverage * 100.0 / mHistory.size()) << "% ("
               << coverage << "/" << mHistory.size() << ")" << std::endl;
            ss << "Master: " << (master * 100.0 / mHistory.size()) << "% ("
               << master << "/" << mHistory.size() << ")" << std::endl;
            ss << "E[Master]: "
               << ((master + m1 + m2) * 100.0 / (3 * mHistory.size())) << "%"
               << std::endl;
            std::sort(top.rbegin(), top.rend());
            ss << "Top 10" << std::endl;
            size_t msiz = std::min(10ULL, top.size());
            for(size_t i = 0; i < msiz; ++i) {
                GUID guid = top[i].second;
                TestHistory his = mHistory[guid];
                ss << GUID2Str(guid) << " " << top[i].first << " "
                   << his.passCnt << "/" << his.testCnt << std::endl;
            }
            return ss.str();
        }
        BUS_TRACE_END();
    }
    void recordTestResult(TestResult res) override {
        BUS_TRACE_BEG() {
            if(!mOutput) {
                mOutput = std::make_unique<std::ofstream>(mOutputPath);
                if(!mOutput->is_open())
                    BUS_TRACE_THROW(
                        std::runtime_error("Failed to save history. file=" +
                                           mOutputPath.string()));
            }
            for(auto x : res.kpID) {
                (*mOutput) << (res.result ? "T" : "F") << GUID2Str(x) << '\n';
            }
            mOutput->flush();
        }
        BUS_TRACE_END();
    }
    std::vector<Ratio> analyse() override {
        return mAResults;
    }
    double getAcc(GUID guid) override {
        const auto& his = mHistory[guid];
        if(his.testCnt)
            return static_cast<double>(his.passCnt) / his.testCnt;
        else
            return -1.0;
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeISTE(Bus::ModuleInstance& instance) {
    return std::make_shared<ImportanceSampler>(instance);
}
