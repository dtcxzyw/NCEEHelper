#include "../Shared/Shared.hpp"
#pragma warning(push, 0)
#include <unicode/unistr.h>
#pragma warning(pop)

using String = icu::UnicodeString;

std::vector<String> getAnswer(bool& quit);
bool getJudge(bool& quit);
std::vector<String> splitAnswer(const String& str);
void checkStatus(UErrorCode status);
#define CHECKSTATUS()  \
    BUS_TRACE_POINT(); \
    checkStatus(status)
