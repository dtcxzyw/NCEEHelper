#pragma once
#include "KnowledgeLibrary.hpp"

struct Ratio final {
    double accuracy, coverage, master;
};

class TestEngine : public Bus::ModuleFunctionBase {
protected:
    explicit TestEngine(Bus::ModuleInstance& instance)
        : ModuleFunctionBase(instance) {}

public:
    static Name getInterface() {
        return "NCEEHelper.TestEngine";
    }

    virtual void init(const fs::path& history, const GUIDTable& table) = 0;
    virtual GUID sampleTest() = 0;
    virtual std::string summary() = 0;
    virtual void recordTestResult(TestResult res) = 0;
    virtual std::vector<Ratio> analyse() = 0;
};
