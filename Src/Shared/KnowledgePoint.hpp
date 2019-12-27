#pragma once
#include "Config.hpp"

class KnowledgePoint :public Bus::ModuleFunctionBase {
protected:
    explicit KnowledgePoint(Bus::ModuleInstance &instance)
        : ModuleFunctionBase(instance) {}

public:
    static Name getInterface() {
        return "NCEEHelper.Knowledge";
    }

    virtual GUID load(std::shared_ptr<Config> cfg) = 0;
    virtual bool test() = 0;
};
