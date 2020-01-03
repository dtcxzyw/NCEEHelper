#include "../Shared/Tester.hpp"
#include "Common.hpp"

BUS_MODULE_NAME("NCEEHelper.Builtin.FillTester");

class FillTester final : public Tester {
private:
public:
    explicit FillTester(Bus::ModuleInstance& instance) : Tester(instance) {}
    GUID init(std::shared_ptr<Config> cfg) override {
        BUS_TRACE_BEG() {
            std::string desc = cfg->asString();
            if(desc.size() <= 38)
                BUS_TRACE_THROW(std::logic_error("Bad description"));
            GUID id = str2GUID(desc.substr(0, 38));
            desc = desc.substr(38);
            String str(desc.c_str(), "utf8");
            return id;
        }
        BUS_TRACE_END();
    }
    int test() override {
        throw std::runtime_error("");
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeFillTester(Bus::ModuleInstance& instance) {
    return std::make_shared<FillTester>(instance);
}
