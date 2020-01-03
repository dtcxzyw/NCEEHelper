#include "../Shared/Command.hpp"
#include "../Shared/Config.hpp"
#include "../Shared/KnowledgeLibrary.hpp"
#include "../Shared/TestEngine.hpp"
#include <iostream>

BUS_MODULE_NAME("NCEEHelper.Builtin.TestDriver");

static void testImpl(std::shared_ptr<KnowledgeLibrary> klib,
                     std::shared_ptr<TestEngine> eng) {
    BUS_TRACE_BEG() {
        while(true) {
            GUID id = eng->sampleTest();
            TestResult res = klib->test(id);
            if(res.result == -1)
                return;
            eng->recordTestResult(res);
        }
    }
    BUS_TRACE_END();
}

class TestDriver final : public Command {
public:
    explicit TestDriver(Bus::ModuleInstance& instance) : Command(instance) {}
    int doCommand(int argc, char** argv) override {
        BUS_TRACE_BEG() {
            if(argc != 3)
                BUS_TRACE_THROW(std::logic_error("Bad arguments"));
            auto klib = system().instantiateByName<KnowledgeLibrary>(argv[1]);
            auto eng = system().instantiateByName<TestEngine>(argv[2]);
            Bus::FunctionId id{
                str2GUID("{D375A669-D1E1-431A-AE14-999A896AF1B2}"), "JsonConfig"
            };
            auto config = system().instantiate<Config>(id);
            config->load("TestConfig.json");
            fs::path historyRoot = config->attribute("History")->asString();
            fs::path dataBaseRoot = config->attribute("DataBase")->asString();
            klib->load(dataBaseRoot / (argv[1]));
            std::cout << klib->summary() << std::endl;
            eng->init(historyRoot / (argv[2]) / (argv[1]), klib->getTable());
            std::cout << eng->summary() << std::endl;
            testImpl(klib, eng);
            return EXIT_SUCCESS;
        }
        BUS_TRACE_END();
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeTestDriver(Bus::ModuleInstance& instance) {
    return std::make_shared<TestDriver>(instance);
}
