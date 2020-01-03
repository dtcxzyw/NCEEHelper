#include "Common.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#pragma warning(push, 0)
#define GUID_DEFINED
#include <rang.hpp>
#include <unicode/regex.h>
#include <unicode/ustream.h>
#pragma warning(pop)

BUS_MODULE_NAME("NCEEHelper.Common");

std::vector<String> getAnswer(bool& quit) {
    BUS_TRACE_BEG() {
        while(true) {
            std::string line;
            std::getline(std::cin, line);
            if(line == "#quit") {
                quit = true;
                return {};
            }
            std::stringstream ss;
            ss.str(line);
            std::vector<String> res;
            String one;
            while(ss >> one)
                res.emplace_back(one);
            std::sort(res.begin(), res.end());
            if(res.size())
                return res;
            else
                std::cout << String("请键入内容", "utf8") << std::endl;
        }
    }
    BUS_TRACE_END();
}

std::vector<String> splitAnswer(const String& str) {
    BUS_TRACE_BEG() {
        UErrorCode status = U_ZERO_ERROR;
        icu::RegexMatcher matcher("([^,]+)", 0, status);
        CHECKSTATUS();
        matcher.reset(str);
        std::vector<String> res;
        while(matcher.find(status)) {
            CHECKSTATUS();
            res.emplace_back(matcher.group(status));
            CHECKSTATUS();
        }
        std::sort(res.begin(), res.end());
        return res;
    }
    BUS_TRACE_END();
}

void checkStatus(UErrorCode status) {
    if(status > UErrorCode::U_ZERO_ERROR) {
        throw std::runtime_error("ICU Error[" +
                                 std::to_string(static_cast<int>(status)) +
                                 "]:" + std::string(u_errorName(status)));
    }
    // TODO:warning
}

bool getJudge(bool& quit) {
    while(true) {
        auto ans = getAnswer(quit);
        if(quit)
            return false;
        if(ans.size() == 1 &&
           (ans.front() == "T" || ans.front() == "F" || ans.front() == "t" ||
            ans.front() == "f"))
            return (ans.front() == "T" || ans.front() == "t");
        else
            std::cout << String("请输入T/F判断", "utf8") << std::endl;
    }
}

bool compareAnswer(const std::vector<String>& stdAns,
                   const std::vector<String>& usrAns, bool& quit) {
    if(usrAns != stdAns) {
        std::cout << rang::fg::yellow << String("标准原因/例子有：", "utf8");
        for(auto rea : stdAns)
            std::cout << rea << " ";
        std::cout << rang::fg::reset << std::endl;
        std::cout << String("请手动判断T/F", "utf8") << std::endl;
        bool match = getJudge(quit);
        return match;
    } else {
        std::cout << rang::fg::green << String("完全正确", "utf8")
                  << rang::fg::reset << std::endl;
        return true;
    }
}
