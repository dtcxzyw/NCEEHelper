#include "../Shared/TestEngine.hpp"
#include <chrono>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
#include <unordered_map>

using Clock = std::chrono::high_resolution_clock;

BUS_MODULE_NAME("NCEEHelper.Builtin.ImportanceSampler");

struct TestHistory final {
    uint32_t passCnt, testCnt, lastHistory, lastTime;
    TestHistory() : passCnt(0), testCnt(0), lastHistory(0), lastTime(0) {}
};

class ImportanceSampler final : public TestEngine {
private:
    std::unordered_map<GUID, TestHistory, GUIDHasher> mHistory;
    std::vector<std::pair<GUID, double>> mAccBuffer;
    std::mt19937_64 mRNG;
    std::unique_ptr<std::ofstream> mOutput;
    fs::path mOutputPath;
    uint64_t mCurrent;
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
                }
            }
        }
        BUS_TRACE_END();
    }
    void buildAccBuffer() {
        mAccBuffer.resize(mHistory.size());
        double sum = 0.0;
        size_t idx = 0;
        for(auto key : mHistory) {
            TestHistory& his = key.second;
            // accuracy 60%
            double weight = std::min(
                60.0, static_cast<double>(his.testCnt + 1) / (his.passCnt + 1));
            // new knowledge 10%
            if(his.testCnt <= 3)
                weight += 10.0 - his.testCnt * 3;
            // time 10%
            weight += std::min(his.lastTime, 20U) * 0.5;
            // forget 20%
            if((his.lastHistory & 1) == 0)
                weight += 20.0;
            sum += weight;
            mAccBuffer[idx].first = key.first;
            mAccBuffer[idx].second = sum;
            ++idx;
        }
    }

public:
    explicit ImportanceSampler(Bus::ModuleInstance& instance)
        : TestEngine(instance) {}
    void init(const fs::path& history, const GUIDTable& table) override {
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
            std::stringstream ss;
            ss << std::hex << std::uppercase << mCurrent;
            reporter().apply(ReportLevel::Info, "Timestamp " + ss.str(),
                             BUS_DEFSRCLOC());
            for(auto log : logs)
                loadRecord(log);

            buildAccBuffer();
            mRNG.seed(Clock::now().time_since_epoch().count());
            ss << ".log";
            mOutputPath = history / ss.str();
        }
        BUS_TRACE_END();
    }
    GUID sampleTest() override {
        std::uniform_real_distribution urd(0.0, mAccBuffer.back().second);
        std::pair<GUID, double> key;
        key.second = urd(mRNG);
        auto iter = std::lower_bound(mAccBuffer.begin(), mAccBuffer.end(), key,
                                     [](const auto& lhs, const auto& rhs) {
                                         return lhs.second < rhs.second;
                                     });
        if(iter == mAccBuffer.end())
            return mAccBuffer.back().first;
        return iter->first;
    }
    std::string summary() override {
        uint32_t pass = 0, test = 0, coverage = 0, master = 0;
        for(auto&& x : mHistory) {
            pass += x.second.passCnt, test += x.second.testCnt;
            coverage += (x.second.testCnt > 0);
            master += ((x.second.lastHistory & 7U) == 7U);
        }
        std::stringstream ss;
        ss << "TestCount: " << test << " PassCount: " << pass << " Accuracy: ";
        ss.precision(2);
        if(test)
            ss << std::fixed << (static_cast<double>(pass) / test) * 100.0
               << "%";
        else
            ss << "N/A";
        ss << std::endl;
        ss << "Coverage: " << (coverage * 100.0 / mHistory.size()) << "% ("
           << coverage << "/" << mHistory.size() << ")" << std::endl;
        ss << "Master: " << (master * 100.0 / mHistory.size()) << "% ("
           << master << "/" << mHistory.size() << ")" << std::endl;
        return ss.str();
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
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeISTE(Bus::ModuleInstance& instance) {
    return std::make_shared<ImportanceSampler>(instance);
}
