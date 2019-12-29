#pragma once
#include "Config.hpp"

class Tester : public Bus::ModuleFunctionBase {
protected:
    explicit Tester(Bus::ModuleInstance& instance)
        : ModuleFunctionBase(instance) {}

public:
    static Name getInterface() {
        return "NCEEHelper.Tester";
    }

    virtual GUID init(std::shared_ptr<Config> cfg) = 0;
    virtual bool test() = 0;
};
