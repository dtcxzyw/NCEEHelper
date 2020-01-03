#include "../Shared/Tester.hpp"
#include "Common.hpp"
#include <algorithm>
#pragma warning(push, 0)
#define GUID_DEFINED
#include <rang.hpp>
#include <unicode/regex.h>
#include <unicode/ustream.h>
#pragma warning(pop)

BUS_MODULE_NAME("NCEEHelper.Builtin.JudgeTester");

class JudgeTester final : public Tester {
private:
    String mProblem;
    bool mAnswer;
    std::vector<String> mReason;

public:
    explicit JudgeTester(Bus::ModuleInstance& instance) : Tester(instance) {}
    GUID init(std::shared_ptr<Config> cfg) override {
        BUS_TRACE_BEG() {
            std::string desc = cfg->asString();
            if(desc.size() <= 38)
                BUS_TRACE_THROW(std::logic_error("Bad description"));
            GUID id = str2GUID(desc.substr(0, 38));
            desc = desc.substr(38);
            String str(desc.c_str(), "utf8");
            UErrorCode status = UErrorCode::U_ZERO_ERROR;
            icu::RegexMatcher matcher("(\\S+)\\[([TF])\\](\\[(\\S+)\\])?",
                                      UREGEX_CASE_INSENSITIVE, status);
            CHECKSTATUS();
            matcher.reset(str);
            if(!matcher.matches(status))
                BUS_TRACE_THROW(std::logic_error("Bad description"));
            CHECKSTATUS();
            mProblem = matcher.group(1, status);
            CHECKSTATUS();
            mAnswer = (matcher.group(2, status) == "T");
            CHECKSTATUS();
            mReason = splitAnswer(matcher.group(4, status));
            CHECKSTATUS();
            std::sort(mReason.begin(), mReason.end());
            return id;
        }
        BUS_TRACE_END();
    }
    int test() override {
        BUS_TRACE_BEG() {
            std::cout << "判断：" << mProblem << std::endl;
            bool quit = false;
            bool res = getJudge(quit);
            if(quit)
                return -1;
            if(res == mAnswer) {
                std::cout << rang::fg::green << "判断正确" << rang::fg::reset
                          << std::endl;
                if(mReason.size()) {
                    std::cout << "原因/例子为：" << std::endl;
                    auto ans = getAnswer(quit);
                    if(quit)
                        return -1;
                    std::sort(ans.begin(), ans.end());
                    if(ans != mReason) {
                        std::cout << rang::fg::yellow << "标准原因/例子有：";
                        for(auto rea : mReason)
                            std::cout << rea << " ";
                        std::cout << rang::fg::reset << std::endl;
                        std::cout << "请手动判断T/F" << std::endl;
                        bool match = getJudge(quit);
                        return quit ? -1 : match;
                    } else {
                        std::cout << rang::fg::green << "完全正确"
                                  << rang::fg::reset << std::endl;
                        return 1;
                    }
                }
                return 1;
            } else {
                std::cout << rang::fg::red << "正确答案为"
                          << (mAnswer ? "T" : "F") << std::endl;
                std::cout << rang::fg::yellow << "原因/例子有：";
                for(auto rea : mReason)
                    std::cout << rea << " ";
                std::cout << rang::fg::reset << std::endl;
                return 0;
            }
        }
        BUS_TRACE_END();
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeJudgeTester(Bus::ModuleInstance& instance) {
    return std::make_shared<JudgeTester>(instance);
}
