#pragma once
#include "Shared.hpp"

using GUIDTable = std::vector<GUID>;

struct TestResult final {
    std::vector<GUID> kpID;
    int result;
};

class KnowledgeLibrary : public Bus::ModuleFunctionBase {
protected:
    explicit KnowledgeLibrary(Bus::ModuleInstance& instance)
        : ModuleFunctionBase(instance) {}

public:
    static Name getInterface() {
        return "NCEEHelper.KnowledgeLibrary";
    }

    virtual void load(const fs::path& dataBase) = 0;
    virtual GUIDTable getTable() = 0;
    virtual TestResult test(GUID kpID) = 0;
    virtual std::string summary() = 0;
    virtual void outputProblem(GUID id, std::ostream& out) {
        BUS_MODULE_NAME("NCEEHelper.KnowledgeLibrary");
        BUS_TRACE_BEG() {
            BUS_TRACE_THROW(std::logic_error("Not implemented."));
        }
        BUS_TRACE_END();
    }
    virtual void outputAnswer(GUID id, std::ostream& out) {
        BUS_MODULE_NAME("NCEEHelper.KnowledgeLibrary");
        BUS_TRACE_BEG() {
            BUS_TRACE_THROW(std::logic_error("Not implemented."));
        }
        BUS_TRACE_END();
    }
    virtual void outputBoth(GUID id, std::ostream& out) {
        BUS_MODULE_NAME("NCEEHelper.KnowledgeLibrary");
        BUS_TRACE_BEG() {
            BUS_TRACE_THROW(std::logic_error("Not implemented."));
        }
        BUS_TRACE_END();
    }
};
