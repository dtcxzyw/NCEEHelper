#include "../Shared/Command.hpp"
#include "../Shared/Config.hpp"
#include "../Shared/TestEngine.hpp"
#include "../Shared/Tester.hpp"

std::shared_ptr<Bus::ModuleFunctionBase>
makeJsonConfig(Bus::ModuleInstance& instance);
std::shared_ptr<Bus::ModuleFunctionBase>
makeISTE(Bus::ModuleInstance& instance);
std::shared_ptr<Bus::ModuleFunctionBase>
makeTestDriver(Bus::ModuleInstance& instance);
std::shared_ptr<Bus::ModuleFunctionBase>
makeAnalyser(Bus::ModuleInstance& instance);
std::shared_ptr<Bus::ModuleFunctionBase>
makePrinter(Bus::ModuleInstance& instance);
std::shared_ptr<Bus::ModuleFunctionBase>
makeFillTester(Bus::ModuleInstance& instance);
std::shared_ptr<Bus::ModuleFunctionBase>
makeJudgeTester(Bus::ModuleInstance& instance);

class BuiltinFunction final : public Bus::ModuleInstance {
public:
    explicit BuiltinFunction(Bus::ModuleSystem& sys)
        : Bus::ModuleInstance("builtin", sys) {}
    Bus::ModuleInfo info() const override {
        Bus::ModuleInfo res;
        res.name = "NCEEHelper.Builtin";
        res.guid = Bus::str2GUID("{D375A669-D1E1-431A-AE14-999A896AF1B2}");
        res.busVersion = BUS_VERSION;
        res.version = "0.0.1";
        res.description = "NCEEHelper utilities";
        res.copyright = "Copyright (c) 2019 Zheng Yingwei";
        return res;
    }
    std::vector<Bus::Name> list(Bus::Name api) const override {
        if(api == Config::getInterface())
            return { "JsonConfig" };
        if(api == TestEngine::getInterface())
            return { "ImportanceSampler" };
        if(api == Command::getInterface())
            return { "TestDriver", "Analyser", "Printer" };
        if(api == Tester::getInterface())
            return { "FillTester", "JudgeTester" };
        return {};
    }
    std::shared_ptr<Bus::ModuleFunctionBase> instantiate(Name name) override {
        if(name == "FillTester")
            return makeFillTester(*this);
        if(name == "JudgeTester")
            return makeJudgeTester(*this);
        if(name == "JsonConfig")
            return makeJsonConfig(*this);
        if(name == "ImportanceSampler")
            return makeISTE(*this);
        if(name == "TestDriver")
            return makeTestDriver(*this);
        if(name == "Analyser")
            return makeAnalyser(*this);
        if(name == "Printer")
            return makePrinter(*this);
        return nullptr;
    }
};

std::shared_ptr<Bus::ModuleInstance> getBuiltin(Bus::ModuleSystem& sys) {
    return std::make_shared<BuiltinFunction>(sys);
}
