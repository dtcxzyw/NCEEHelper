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
        return res;
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
            std::cout << "请输入T/F判断" << std::endl;
    }
}

bool compareAnswer(const std::vector<String>& stdAns,
                   const std::vector<String>& usrAns, bool& quit) {
    if(usrAns != stdAns) {
        std::cout << rang::fg::yellow << "标准原因/例子有：";
        for(auto rea : stdAns)
            std::cout << rea << " ";
        std::cout << rang::fg::reset << std::endl;
        std::cout << "请手动判断T/F" << std::endl;
        bool match = getJudge(quit);
        return match;
    } else {
        std::cout << rang::fg::green << "完全正确" << rang::fg::reset
                  << std::endl;
        return true;
    }
}
