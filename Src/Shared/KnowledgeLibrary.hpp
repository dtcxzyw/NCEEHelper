#pragma once
#include "Config.hpp"

class KnowledgeLibrary :public Bus::ModuleFunctionBase {
protected:
    explicit KnowledgeLibrary(Bus::ModuleInstance &instance)
        : ModuleFunctionBase(instance) {}

public:
    static Name getInterface() {
        return "NCEEHelper.KnowledgeLibrary";
    }

    virtual GUID load(std::shared_ptr<Config> cfg) = 0;
};
