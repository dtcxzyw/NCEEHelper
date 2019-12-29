#include "../Shared/Tester.hpp"

BUS_MODULE_NAME("NCEEHelper.Builtin.FillTester");

class FillTester final : public Tester {
private:
public:
    explicit FillTester(Bus::ModuleInstance& instance) : Tester(instance) {}
    GUID init(std::shared_ptr<Config> cfg) override {
        BUS_TRACE_BEG() {
            return {};
        }
        BUS_TRACE_END();
    }
    bool test() override {
        return false;
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeFillTester(Bus::ModuleInstance& instance) {
    return std::make_shared<FillTester>(instance);
}
