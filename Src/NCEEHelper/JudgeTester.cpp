#include "../Shared/Tester.hpp"
#include "Common.hpp"
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
            return id;
        }
        BUS_TRACE_END();
    }
    int test() override {
        BUS_TRACE_BEG() {
            std::cout << String("判断：", "utf8") << mProblem << std::endl;
            bool quit = false;
            bool res = getJudge(quit);
            if(quit)
                return -1;
            if(res == mAnswer) {
                std::cout << rang::fg::green << String("判断正确", "utf8")
                          << rang::fg::reset << std::endl;
                if(mReason.size()) {
                    std::cout << String("原因/例子为：", "utf8") << std::endl;
                    auto ans = getAnswer(quit);
                    if(quit)
                        return -1;
                    res = compareAnswer(mReason, ans, quit);
                    return quit ? -1 : res;
                }
                return 1;
            } else {
                std::cout << rang::fg::red << String("正确答案为", "utf8")
                          << (mAnswer ? "T" : "F") << std::endl;
                if(mReason.size()) {
                    std::cout << rang::fg::yellow
                              << String("原因/例子有：", "utf8");
                    for(auto rea : mReason)
                        std::cout << rea << " ";
                    std::cout << rang::fg::reset << std::endl;
                }
                return 0;
            }
        }
        BUS_TRACE_END();
    }
    void outputProblem(std::ostream& out) override {
        out << mProblem << " (&emsp;)";
        if(mReason.size()) {
            out << " 原因/例子：";
            outputBlank(mReason, out);
        }
    }
    void outputAnswer(std::ostream& out) override {
        out << (mAnswer ? "√" : "X");
        if(mReason.size()) {
            out << " ";
            for(auto rea : mReason)
                out << rea << " ";
        }
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeJudgeTester(Bus::ModuleInstance& instance) {
    return std::make_shared<JudgeTester>(instance);
}
