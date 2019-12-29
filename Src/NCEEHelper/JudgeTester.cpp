#include "../Shared/Tester.hpp"

BUS_MODULE_NAME("NCEEHelper.Builtin.JudgeTester");

class JudgeTester final : public Tester {
private:
public:
    explicit JudgeTester(Bus::ModuleInstance& instance) : Tester(instance) {}
    GUID init(std::shared_ptr<Config> cfg) override {
        return {};
    }
    bool test() override {
        return false;
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeJudgeTester(Bus::ModuleInstance& instance) {
    return std::make_shared<JudgeTester>(instance);
}
