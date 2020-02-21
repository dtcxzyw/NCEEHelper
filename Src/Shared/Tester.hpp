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
    virtual int test() = 0;
    virtual void outputProblem(std::ostream& out) = 0;
    virtual void outputAnswer(std::ostream& out) = 0;
};
