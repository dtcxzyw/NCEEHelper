#include "../Shared/Command.hpp"
#include "../Shared/Config.hpp"
#include "../Shared/KnowledgeLibrary.hpp"
#include "../Shared/TestEngine.hpp"
#include <chrono>
#include <iostream>

BUS_MODULE_NAME("NCEEHelper.Builtin.Analyser");

class Analyser final : public Command {
public:
    explicit Analyser(Bus::ModuleInstance& instance) : Command(instance) {}
    int doCommand(int argc, char** argv) override {
        BUS_TRACE_BEG() {
            if(argc != 3)
                BUS_TRACE_THROW(std::logic_error("Bad arguments"));
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
            auto table = klib->getTable();
            eng->init(hisPath, table, TestMode::Weight);
            auto [res, lastPass] = eng->analyse();
            {
                Ratio empty{ 0U, 0.0, 0.0, 0.0, 0.0 };
                if(res.size())
                    empty.accuracy = res.front().accuracy;
                res.insert(res.begin(), empty);
                fs::path outputPath =
                    fs::path("Results") / (std::string(argv[1]) + ".log");
                std::ofstream out(outputPath);
                for(auto entry : res)
                    out << entry.count << " " << entry.accuracy << " "
                        << entry.coverage << " " << entry.master << " "
                        << entry.emaster << std::endl;
                std::cout << "->" << outputPath << std::endl;
            }
            {
                fs::path outputPath =
                    fs::path("Results") / (std::string(argv[1]) + ".res");
                std::ofstream out(outputPath);
                for(auto cnt : lastPass)
                    out << cnt.first << " " << cnt.second << std::endl;
                std::cout << "->" << outputPath << std::endl;
            }
            {
                fs::path outputPath =
                    fs::path("Results") / (std::string(argv[1]) + ".dist");
                std::map<double, uint32_t> dist;
                for(auto id : table)
                    ++dist[eng->getAcc(id)];
                std::ofstream out(outputPath);
                for(auto cnt : dist)
                    if(cnt.first != -1.0)
                        out << cnt.first << " " << cnt.second << std::endl;
                std::cout << "->" << outputPath << std::endl;
            }
            return EXIT_SUCCESS;
        }
        BUS_TRACE_END();
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeAnalyser(Bus::ModuleInstance& instance) {
    return std::make_shared<Analyser>(instance);
}
