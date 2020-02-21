#include "../Shared/Config.hpp"
#include "../Shared/KnowledgeLibrary.hpp"
#include "../Shared/Tester.hpp"
#include <ctime>
#include <sstream>

BUS_MODULE_NAME("NCEEHelper.Biology");

class BiologyLibrary final : public KnowledgeLibrary {
private:
    std::unordered_map<GUID, std::shared_ptr<Tester>, GUIDHasher> mKPS;
    uint32_t mFillCnt, mJudgeCnt;
    fs::path mDataBase;

public:
    explicit BiologyLibrary(Bus::ModuleInstance& instance)
        : KnowledgeLibrary(instance) {}

    void load(const fs::path& dataBase) override {
        BUS_TRACE_BEG() {
            mFillCnt = mJudgeCnt = 0;
            mDataBase = dataBase / "Biology";
            Bus::FunctionId id{
                str2GUID("{D375A669-D1E1-431A-AE14-999A896AF1B2}"), ""
            };
            // Fill.json
            {
                id.name = "JsonConfig";
                auto cfg = system().instantiate<Config>(id);
                if(!cfg->load(mDataBase / "Fill.json"))
                    BUS_TRACE_THROW(
                        std::runtime_error("Failed to load Fill.json"));
                id.name = "FillTester";
                for(auto ele : cfg->expand()) {
                    ++mFillCnt;
                    auto tester = system().instantiate<Tester>(id);
                    GUID id = tester->init(ele);
                    auto& val = mKPS[id];
                    if(val)
                        BUS_TRACE_THROW(std::logic_error("GUID redefined"));
                    val.swap(tester);
                }
            }
            // Judge.json
            {
                id.name = "JsonConfig";
                auto cfg = system().instantiate<Config>(id);
                if(!cfg->load(mDataBase / "Judge.json"))
                    BUS_TRACE_THROW(
                        std::runtime_error("Failed to load Judge.json"));
                id.name = "JudgeTester";
                for(auto ele : cfg->expand()) {
                    ++mJudgeCnt;
                    auto tester = system().instantiate<Tester>(id);
                    GUID id = tester->init(ele);
                    auto& val = mKPS[id];
                    if(val)
                        BUS_TRACE_THROW(std::logic_error("GUID redefined"));
                    val.swap(tester);
                }
            }
        }
        BUS_TRACE_END();
    }
    GUIDTable getTable() override {
        GUIDTable res;
        for(auto&& x : mKPS)
            res.emplace_back(x.first);
        return res;
    }
    TestResult test(GUID kpID) override {
        BUS_TRACE_BEG() {
            TestResult res;
            res.kpID = { kpID };
            auto iter = mKPS.find(kpID);
            if(iter == mKPS.end())
                BUS_TRACE_THROW(std::logic_error("Invaild GUID"));
            res.result = iter->second->test();
            return res;
        }
        BUS_TRACE_END();
    }
    std::string summary() override {
        std::stringstream ss;
        using Clock = fs::file_time_type::clock;
        // TODO:filetime
        ss << "Fill.json count: " << mFillCnt << std::endl;
        ss << "Judge.json count: " << mJudgeCnt << std::endl;
        return ss.str();
    }
    void outputProblem(GUID id, std::ostream& out) override {
        mKPS[id]->outputProblem(out);
    }
    void outputAnswer(GUID id, std::ostream& out) override {
        mKPS[id]->outputAnswer(out);
    }
};

class Instance final : public Bus::ModuleInstance {
public:
    Instance(const fs::path& path, Bus::ModuleSystem& sys)
        : Bus::ModuleInstance(path, sys) {}
    Bus::ModuleInfo info() const override {
        Bus::ModuleInfo res;
        res.name = BUS_DEFAULT_MODULE_NAME;
        res.guid = Bus::str2GUID("{574F04A9-7F1D-4ADD-B077-7E8FE136DF4A}");
        res.busVersion = BUS_VERSION;
        res.version = "0.0.1";
        res.description = "Biology";
        res.copyright = "Copyright (c) 2019 Zheng Yingwei";
        res.modulePath = getModulePath();
        return res;
    }
    std::vector<Bus::Name> list(Bus::Name api) const override {
        if(api == KnowledgeLibrary::getInterface())
            return { "Biology" };
        return {};
    }
    std::shared_ptr<Bus::ModuleFunctionBase> instantiate(Name name) override {
        if(name == "Biology")
            return std::make_shared<BiologyLibrary>(*this);
        return nullptr;
    }
};

BUS_API void busInitModule(const Bus::fs::path& path, Bus::ModuleSystem& system,
                           std::shared_ptr<Bus::ModuleInstance>& instance) {
    instance = std::make_shared<Instance>(path, system);
}
