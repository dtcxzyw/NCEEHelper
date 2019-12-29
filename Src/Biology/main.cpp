#include "../Shared/KnowledgeLibrary.hpp"

BUS_MODULE_NAME("NCEEHelper.Biology");

class BiologyLibrary final : public KnowledgeLibrary {
private:
public:
    explicit BiologyLibrary(Bus::ModuleInstance& instance)
        : KnowledgeLibrary(instance) {}

    void load(const fs::path& dataBase) {}
    GUIDTable getTable() {return {};}
    TestResult test(GUID kpID) {return {};}
    std::string summary() {
        return {};
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
