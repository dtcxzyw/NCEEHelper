#include "../Shared/Config.hpp"
#include "../Shared/KnowledgeLibrary.hpp"
#include "../Shared/Tester.hpp"
#include <ctime>
#include <sstream>
#pragma warning(push, 0)
#include <pybind11/embed.h>
namespace py = pybind11;
#pragma warning(pop)

BUS_MODULE_NAME("NCEEHelper.English");

class EnglishLibrary final : public KnowledgeLibrary {
private:
    fs::path mDataBase;

public:
    explicit EnglishLibrary(Bus::ModuleInstance& instance)
        : KnowledgeLibrary(instance) {}

    void load(const fs::path& dataBase) {
        BUS_TRACE_BEG() {}
        BUS_TRACE_END();
    }
    GUIDTable getTable() {
        GUIDTable res;
        return res;
    }
    TestResult test(GUID kpID) {
        BUS_TRACE_BEG() {
            TestResult res;
            return res;
        }
        BUS_TRACE_END();
    }
    std::string summary() {
        std::stringstream ss;
        return ss.str();
    }
};

class Instance final : public Bus::ModuleInstance {
private:
    py::scoped_interpreter mInterpreter;

public:
    Instance(const fs::path& path, Bus::ModuleSystem& sys)
        : Bus::ModuleInstance(path, sys), mInterpreter(true) {
        py::print("Hello Pybind11");
    }
    Bus::ModuleInfo info() const override {
        Bus::ModuleInfo res;
        res.name = BUS_DEFAULT_MODULE_NAME;
        res.guid = Bus::str2GUID("{3D20059B-9A17-4142-85E9-C08CE5A56130}");
        res.busVersion = BUS_VERSION;
        res.version = "0.0.1";
        res.description = "English";
        res.copyright = "Copyright (c) 2020 Zheng Yingwei";
        res.modulePath = getModulePath();
        return res;
    }
    std::vector<Bus::Name> list(Bus::Name api) const override {
        if(api == KnowledgeLibrary::getInterface())
            return { "English" };
        return {};
    }
    std::shared_ptr<Bus::ModuleFunctionBase> instantiate(Name name) override {
        if(name == "English")
            return std::make_shared<EnglishLibrary>(*this);
        return nullptr;
    }
};

BUS_API void busInitModule(const Bus::fs::path& path, Bus::ModuleSystem& system,
                           std::shared_ptr<Bus::ModuleInstance>& instance) {
    instance = std::make_shared<Instance>(path, system);
}
