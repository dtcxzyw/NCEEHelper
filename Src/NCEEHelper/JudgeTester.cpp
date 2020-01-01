#include "../Shared/Tester.hpp"
#pragma warning(push, 0)
#include <utf8.h>
#pragma warning(pop)

BUS_MODULE_NAME("NCEEHelper.Builtin.JudgeTester");

class JudgeTester final : public Tester {
private:
    std::string mProblem;
    bool mAnswer;
    std::vector<std::string> mReason;

public:
    explicit JudgeTester(Bus::ModuleInstance& instance) : Tester(instance) {}
    GUID init(std::shared_ptr<Config> cfg) override {
        BUS_TRACE_BEG() {
            std::string desc = cfg->asString();
            if(desc.size() <= 38)
                BUS_TRACE_THROW(std::logic_error("Bad description"));
            GUID id = str2GUID(desc.substr(0, 38));
            desc = desc.substr(38);
            if(!utf8::is_valid(desc.cbegin(), desc.cend()))
                BUS_TRACE_THROW("Invalid utf8 string");

            return id;
        }
    }
    bool test() override {
        return false;
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeJudgeTester(Bus::ModuleInstance& instance) {
    return std::make_shared<JudgeTester>(instance);
}
