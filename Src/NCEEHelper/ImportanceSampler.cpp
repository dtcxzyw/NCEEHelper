#include "../Shared/TestEngine.hpp"
#include <chrono>
#include <fstream>
#include <random>
#include <sstream>
#include <unordered_map>

using Clock = std::chrono::high_resolution_clock;

BUS_MODULE_NAME("NCEEHelper.Builtin.ImportanceSampler");

struct TestHistory final {
    uint32_t passCnt, testCnt;
    TestHistory() : passCnt(0), testCnt(0) {}
};

class ImportanceSampler final : public TestEngine {
private:
    std::unordered_map<GUID, TestHistory, GUIDHasher> mHistory;
    std::vector<TestResult> mRecord;
    std::vector<std::pair<GUID, double>> mAccBuffer;
    std::mt19937_64 mRNG;
    fs::path mHistoryRoot;
    void loadRecord(const fs::path& historyFile) {
        BUS_TRACE_BEG() {
            std::ifstream in(historyFile);
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
                    iter->second.passCnt += res;
                    ++iter->second.testCnt;
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
            uint32_t pass = key.second.passCnt;
            uint32_t test = key.second.testCnt;
            // accuracy
            sum += std::min(10.0, static_cast<double>(test + 1) / (pass + 1));
            // new knowledge
            if(test == 0)
                sum += 5.0;
            // TODO:time,forget
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
            mHistoryRoot = history;
            if(table.empty())
                BUS_TRACE_THROW(std::logic_error("No knowledge point!!!"));
            for(auto id : table)
                mHistory[id] = {};
            for(auto p : fs::directory_iterator(history)) {
                if(p.is_regular_file()) {
                    auto path = p.path();
                    if(path.extension() == ".log")
                        loadRecord(path);
                }
            }
            buildAccBuffer();
            mRNG.seed(Clock::now().time_since_epoch().count());
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
        uint32_t pass = 0, test = 0;
        for(auto&& x : mHistory)
            pass += x.second.passCnt, test += x.second.testCnt;
        std::stringstream ss;
        ss << "TestCount: " << test << " PassCount: " << pass << " Accuracy:";
        if(test)
            ss << (static_cast<double>(pass) / test);
        else
            ss << "N/A";
        return ss.str();
    }
    void recordTestResult(TestResult res) override {
        mRecord.emplace_back(res);
    }
    void flushHistory() override {
        BUS_TRACE_BEG() {
            if(mRecord.empty())
                return;
            std::stringstream ss;
            ss << std::hex << std::uppercase
               << Clock::now().time_since_epoch().count() << ".log";
            fs::path path = mHistoryRoot / ss.str();
            std::ofstream out(path);
            for(auto x : mRecord)
                out << (x.result ? "T" : "F") << GUID2Str(x.kpID) << '\n';
            out.close();
            mRecord.clear();
            reporter().apply(ReportLevel::Info,
                             "Test history has been flushed to file " +
                                 path.string() + ".",
                             BUS_DEFSRCLOC());
        }
        BUS_TRACE_END();
    }
    ~ImportanceSampler() {
        flushHistory();
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeISTE(Bus::ModuleInstance& instance) {
    return std::make_shared<ImportanceSampler>(instance);
}
