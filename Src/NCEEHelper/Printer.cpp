#include "../Shared/Command.hpp"
#include "../Shared/Config.hpp"
#include "../Shared/KnowledgeLibrary.hpp"
#include "../Shared/TestEngine.hpp"
#include <chrono>
#include <ctime>
#include <iostream>
#pragma warning(push, 0)
#include <unicode/ucnv.h>
#pragma warning(pop)
BUS_MODULE_NAME("NCEEHelper.Builtin.Printer");

static void outputTime(std::ostream& out) {
    using Clock = std::chrono::system_clock;
    auto tp = Clock::now();
    auto tt = Clock::to_time_t(tp);
    auto tm = std::localtime(&tt);
    out << std::put_time(tm, "%Y-%m-%d");
}

class Printer final : public Command {
public:
    explicit Printer(Bus::ModuleInstance& instance) : Command(instance) {}
    int doCommand(int argc, char** argv) override {
        BUS_TRACE_BEG() {
            if(argc != 4)
                BUS_TRACE_THROW(std::logic_error("Bad arguments"));
            std::string mode = argv[3];
            bool sep = true;
            if(mode == "sep") {
                sep = true;
            } else if(mode == "link") {
                sep = false;
            } else {
                BUS_TRACE_THROW(std::logic_error("Bad mode " + mode +
                                                 " (accept sep/link)"));
            }
            auto klib = system().instantiateByName<KnowledgeLibrary>(argv[1]);
            if(!klib)
                BUS_TRACE_THROW(std::runtime_error(
                    std::string("Failed to instantiate KnowledgeLibrary ") +
                    argv[1]));
            auto eng = system().instantiateByName<TestEngine>(argv[2]);
            if(!eng)
                BUS_TRACE_THROW(std::runtime_error(
                    std::string("Failed to inistantiate TestEngine ") +
                    argv[2]));
            Bus::FunctionId id{
                str2GUID("{D375A669-D1E1-431A-AE14-999A896AF1B2}"), "JsonConfig"
            };
            auto config = system().instantiate<Config>(id);
            config->load("config.json");
            fs::path historyRoot = config->attribute("History")->asString();
            fs::path dataBaseRoot = config->attribute("DataBase")->asString();
            klib->load(dataBaseRoot);
            std::cout << klib->summary() << std::endl;
            fs::path hisPath = historyRoot / argv[2] / argv[1];
            eng->init(hisPath, klib->getTable(), TestMode::Weight);
            fs::path outputPath = fs::path("Output") /
                (std::string(argv[1]) + "-" + mode + ".md");
            std::ofstream out(outputPath);
            out.precision(2);
            out << std::uppercase << std::fixed;
            ucnv_setDefaultName("utf8");
            if(sep) {
                out << "# Problem ";
                outputTime(out);
                out << "\n";
                uint32_t idx = 0;
                for(auto id : klib->getTable()) {
                    double acc = eng->getAcc(id);
                    out << (++idx) << ".[";
                    if(acc < 0.0)
                        out << "N/A";
                    else
                        out << acc * 100.0 << "%";
                    out << "]";
                    klib->outputProblem(id, out);
                    out << "\n\n";
                }
                out << "# Answer\n";
                idx = 0;
                for(auto id : klib->getTable()) {
                    out << (++idx) << ".";
                    klib->outputAnswer(id, out);
                    out << "&emsp;";
                }
            } else {
                out << "# Problem & Answer ";
                outputTime(out);
                out << "\n";
                uint32_t idx = 0;
                for(auto id : klib->getTable()) {
                    double acc = eng->getAcc(id);
                    out << (++idx) << ".[";
                    if(acc < 0.0)
                        out << "N/A";
                    else
                        out << acc * 100.0 << "%";
                    out << "]";
                    klib->outputBoth(id, out);
                    out << "\n\n";
                }
            }
            std::cout << "->" << outputPath << std::endl;
            return EXIT_SUCCESS;
        }
        BUS_TRACE_END();
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makePrinter(Bus::ModuleInstance& instance) {
    return std::make_shared<Printer>(instance);
}
