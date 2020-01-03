#include "../Shared/Tester.hpp"
#include "Common.hpp"
#include <map>
#pragma warning(push, 0)
#define GUID_DEFINED
#include <rang.hpp>
#include <unicode/regex.h>
#include <unicode/ustream.h>
#pragma warning(pop)

BUS_MODULE_NAME("NCEEHelper.Builtin.FillTester");

enum class PieceType { Fill, Statement };
struct Piece final {
    std::vector<String> ans;
    String state;
    PieceType type;
};

class FillTester final : public Tester {
private:
    std::vector<Piece> mPieces;

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
            UErrorCode status = UErrorCode::U_ZERO_ERROR;
            icu::RegexMatcher matcher("\\[([^\\[\\]]+)\\]", 0, status);
            CHECKSTATUS();
            matcher.reset(str);
            int32_t lastPos = 0;
            while(matcher.find(status)) {
                CHECKSTATUS();
                String ans = matcher.group(1, status);
                CHECKSTATUS();
                int32_t cpos = matcher.start(status);
                CHECKSTATUS();
                {
                    Piece p;
                    p.state = str.tempSubStringBetween(lastPos, cpos);
                    p.type = PieceType::Statement;
                    mPieces.emplace_back(p);
                }
                lastPos = matcher.end(status);
                CHECKSTATUS();
                Piece res;
                res.ans = splitAnswer(ans);
                res.type = PieceType::Fill;
                mPieces.emplace_back(res);
            }
            {
                Piece p;
                p.state = str.tempSubStringBetween(lastPos);
                p.type = PieceType::Statement;
                mPieces.emplace_back(p);
            }
            std::cout << std::endl;
            for(auto p : mPieces) {
                if(p.type == PieceType::Fill) {
                    std::cout << "@";
                    for(auto ans : p.ans)
                        std::cout << ans << "&";
                    std::cout << "@";
                } else
                    std::cout << p.state;
            }
            std::cout << std::endl << std::endl;
            return id;
        }
        BUS_TRACE_END();
    }
    int test() override {
        BUS_TRACE_BEG() {
            bool res = true;
            std::map<size_t, bool> history;
            for(size_t i = 0; i < mPieces.size(); ++i) {
                Piece& p = mPieces[i];
                if(p.type == PieceType::Statement)
                    continue;
                // Problem
                for(size_t j = 0; j < mPieces.size(); ++j) {
                    Piece& cp = mPieces[j];
                    if(cp.type == PieceType::Statement) {
                        std::cout << cp.state;
                    } else {
                        if(i < j) {
                            rang::fg col =
                                (history[i] ? rang::fg::green : rang::fg::red);
                            std::cout << col << "[";
                            for(auto ans : cp.ans)
                                std::cout << ans << " ";
                            std::cout << "]" << rang::fg::reset;
                        } else {
                            std::cout
                                << (i > j ? rang::fg::reset : rang::fg::yellow)
                                << "[          ]" << rang::fg::reset;
                        }
                    }
                }
                std::cout << std::endl;

                bool quit = false;
                while(true) {
                    std::vector<String> ans = getAnswer(quit);
                    if(quit)
                        return -1;
                    bool eq = compareAnswer(p.ans, ans, quit);
                    res = res & eq;
                    history[i] = eq;
                    if(quit)
                        return -1;
                }
            }
            return res;
        }
        BUS_TRACE_END();
    }
};

std::shared_ptr<Bus::ModuleFunctionBase>
makeFillTester(Bus::ModuleInstance& instance) {
    return std::make_shared<FillTester>(instance);
}
