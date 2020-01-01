#include "Common.hpp"
#include <iostream>
#include <utf8.h>

std::vector<std::string> getAnswer(bool& quit) {
    std::string line;
    std::getline(std::cin, line);
    if(line == "#quit") {
        quit = true;
        return {};
    }
    std::vector<std::string> res;
    size_t pos = 0;
    return res;
}
