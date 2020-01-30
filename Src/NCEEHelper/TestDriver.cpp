#include "../Shared/Command.hpp"
#include "../Shared/Config.hpp"
#include "../Shared/KnowledgeLibrary.hpp"
#include "../Shared/TestEngine.hpp"
#include <chrono>
#include <iostream>

BUS_MODULE_NAME("NCEEHelper.Builtin.TestDriver");

static void testImpl(std::shared_ptr<KnowledgeLibrary> klib,
                     std::shared_ptr<TestEngine> eng) {
    BUS_TRACE_BEG() {
        using Clock = std::chrono::system_clock;
        auto beg = Clock::now();
        uint32_t cnt = 0;
        while(true) {
            ++cnt;
            std::cout << "Round " << cnt << std::endl;
            {
                auto sec =
                    std::chrono::ceil<std::chrono::seconds>(Clock::now() - beg)
                        .count();
                auto hour = sec / 3600;
                sec %= 3600;
                auto min = sec / 60;
                sec %= 60;
                std::cout << hour << " h " << min << " min " << sec << " s"
                          << std::endl;
            }
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
            config->load("config.json");
            fs::path historyRoot = config->attribute("History")->asString();
            fs::path dataBaseRoot = config->attribute("DataBase")->asString();
            klib->load(dataBaseRoot);
            std::cout << klib->summary() << std::endl;
            fs::path hisPath = historyRoot;
            fs::create_directory(hisPath);
            hisPath /= argv[2];
            fs::create_directory(hisPath);
            hisPath /= argv[1];
            fs::create_directory(hisPath);
            eng->init(hisPath, klib->getTable());
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
