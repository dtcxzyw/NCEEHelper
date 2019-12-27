#pragma once
#include "Shared.hpp"

enum class DataType { Object, Float, Unsigned, String, Bool, Array };

class Config :public Bus::ModuleFunctionBase {
protected:
    explicit Config(Bus::ModuleInstance &instance)
        : ModuleFunctionBase(instance) {}

public:
    static Name getInterface() {
        return "NCEEHelper.Config";
    }
    virtual bool load(const fs::path &path) = 0;
    virtual std::shared_ptr<Config> attribute(Name attr) const = 0;
    virtual std::vector<std::shared_ptr<Config>> expand() const = 0;
    virtual size_t size() const = 0;
    virtual DataType getType() const = 0;
    virtual unsigned asUint() const = 0;
    virtual float asFloat() const = 0;
    virtual std::string asString() const = 0;
    virtual bool asBool() const = 0;
    virtual bool hasAttr(Name attr) const = 0;
};
