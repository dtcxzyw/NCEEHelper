#include "../Shared/Command.hpp"
#include "../Shared/Config.hpp"
#include "../Shared/KnowledgeLibrary.hpp"
#include "../Shared/Tester.hpp"
#include <ctime>
#include <fstream>
#include <sstream>
#pragma warning(push, 0)
#include <crossguid/guid.hpp>
#include <pybind11/embed.h>
namespace py = pybind11;
using namespace py::literals;
#pragma warning(pop)

BUS_MODULE_NAME("NCEEHelper.English");

// TODO:from pattern.en import suggest 拼写检查

class Lemmatizer final {
private:
    py::object mLemmaDB;
    py::object mWordNetLemmatizer;

public:
    explicit Lemmatizer(const fs::path& dataPath) {
        BUS_TRACE_BEG() {
            mWordNetLemmatizer = py::module::import("nltk.stem.wordnet")
                                     .attr("WordNetLemmatizer")();
            mLemmaDB = py::module::import("stardict").attr("LemmaDB")();
            mLemmaDB.attr("load")(
                py::str((dataPath / "lemma.en.txt").string()));
        }
        BUS_TRACE_END();
    }
    std::string lemmatize(const std::string& word) const {
        BUS_TRACE_BEG() {
            auto base = py::str(word).attr("lower")();
            py::object res = mLemmaDB.attr("word_stem")(base);
            if(res.is_none())
                return mWordNetLemmatizer.attr("lemmatize")(base)
                    .cast<std::string>();
            else
                return (*res.begin()).cast<std::string>();
        }
        BUS_TRACE_END();
    }
};

class Tagger final {
private:
    py::object mTagger;

public:
    Tagger() {
        mTagger = py::module::import("nltk.tag").attr("pos_tag");
    }

    py::str tagNLTK(py::str word) {
        BUS_TRACE_BEG() {
            py::list list;
            list.append(word);
            auto res = mTagger(list, "universal");
            auto tuple = *res.begin();
            auto iter = tuple.begin();
            ++iter;
            ++iter;
            return py::str(*iter);
        }
        BUS_TRACE_END();
    }

    bool isImpl(py::str tag, py::object pos, const char* t1, const char* t2) {
        BUS_TRACE_BEG() {
            if(pos.is_none())
                return tag.equal(py::str(t2));
            return pos.contains(t1) &&
                (std::stoi(pos[t1].cast<std::string>()) > 10);
        }
        BUS_TRACE_END();
    }

    bool isVerb(py::str tag, py::object pos) {
        return isImpl(tag, pos, "v", "VERB");
    }

    bool isNoun(py::str tag, py::object pos) {
        return isImpl(tag, pos, "n", "NOUN");
    }

    bool isAdj(py::str tag, py::object pos) {
        return isImpl(tag, pos, "j", "ADJ");
    }

    bool isAdv(py::str tag, py::object pos) {
        return isImpl(tag, pos, "r", "ADV");
    }
};

class WordLUT final {
private:
    std::unordered_set<size_t> mWords;
    py::object mDataBase;

    size_t hash(const std::string& str) const {
        std::hash<std::string> hasher;
        return hasher(str);
    }

public:
    WordLUT(const fs::path& dataPath) {
        py::module dict = py::module::import("stardict");
        mDataBase = dict.attr("StarDict")(
            py::str{ (dataPath / "readword.db").string() });
        for(auto ref : mDataBase) {
            auto iter = ref.begin();
            ++iter;
            ++iter;
            mWords.insert(hash(iter->cast<std::string>()));
        }
        py::print("LUT size {}"_s.format(mWords.size()));
    }
    bool count(const std::string& str) const {
        return mWords.count(hash(str));
    }
    std::string translation(const std::string& word) const {
        BUS_TRACE_BEG() {
            auto res = mDataBase[py::str(word)];
            auto str = res["translation"];
            str = str.attr("replace")("[", "【");
            str = str.attr("replace")("]", "】");
            str = str.attr("replace")("\n", "");
            str = str.attr("replace")("\r", "");
            str = str.attr("replace")("\"", "");
            return str.cast<std::string>();
        }
        BUS_TRACE_END();
    }
};

class WordCastGenerator final : public Command {
private:
    py::object mConjugate;
    py::str conjImpl(py::str word, py::dict ex, const char* t1,
                     const char* t2) {
        if(ex.contains(t1))
            return ex[t1];
        return mConjugate(word, t2);
    }

    void fixTail(std::string& str) {
        if(str.back() == 'y')
            str.back() = 'i';
        else if(str.back() == 'e')
            str.pop_back();
    }

    py::str get3sg(py::str word, py::dict ex) {
        return conjImpl(word, ex, "3", "3sg");
    }
    py::str getPass(py::str word, py::dict ex) {
        return conjImpl(word, ex, "p", "p");
    }
    py::str getPPart(py::str word, py::dict ex) {
        return conjImpl(word, ex, "d", "ppart");
    }
    py::str getPart(py::str word, py::dict ex) {
        return conjImpl(word, ex, "i", "part");
    }

    bool is3sgNormal(py::str word, py::str tsg) {
        std::string sw = word.cast<std::string>();
        std::string st = tsg.cast<std::string>();
        if(sw + "s" == st)
            return true;
        if(sw + "es" == st)
            return true;
        if(sw.back() == 'y')
            return st == sw.substr(0, sw.size() - 1) + "ies";
        return false;
    }

    bool isPartNormal(py::str word, py::str part) {
        std::string sw = word.cast<std::string>();
        std::string sp = part.cast<std::string>();
        if(sw.back() == 'e')
            sw.pop_back();
        return sp == sw + "ing";
    }

    bool isPassNormal(py::str word, py::str pass) {
        std::string sw = word.cast<std::string>();
        std::string sp = pass.cast<std::string>();
        fixTail(sw);
        return sp == sw + "ed";
    }

    bool isPPartNormal(py::str word, py::str ppart) {
        return isPassNormal(word, ppart);
    }

    py::object mPlur;

    py::str getPlur(py::str word, py::dict ex) {
        if(ex.contains("s"))
            return ex["s"];
        return mPlur(word);
    }

    bool isPlurNormal(py::str word, py::str plur) {
        return is3sgNormal(word, plur);
    }

    py::object mComp, mSuper;

    py::str getComp(py::str word, py::dict ex) {
        if(ex.contains("r"))
            return ex["r"];
        return mComp(word);
    }

    py::str getSuper(py::str word, py::dict ex) {
        if(ex.contains("t"))
            return ex["t"];
        return mSuper(word);
    }

    bool isCompNormal(py::str word, py::str comp) {
        std::string sw = word.cast<std::string>();
        std::string sc = comp.cast<std::string>();
        if("more " + sw == sc)
            return true;
        fixTail(sw);
        return sw + "er" == sc;
    }

    bool isSuperNormal(py::str word, py::str super) {
        std::string sw = word.cast<std::string>();
        std::string ss = super.cast<std::string>();
        if("most " + sw == ss)
            return true;
        fixTail(sw);
        return sw + "est" == ss;
    }

    py::object mSuggest;

    std::string suggest(const std::string& str) {
        auto res = mSuggest(py::str(str));
        return res.begin()->begin()->cast<std::string>();
    }

    void gen(std::function<void(const std::string&, const std::string&,
                                const std::string&)>
                 out,
             const fs::path& dbp) {
        BUS_TRACE_BEG() {
            fs::path dataPath = dbp / "English" / "ECDICTData";
            py::module::import("sys").attr("path").attr("append")(
                py::str{ dataPath.string() });
            py::module dict = py::module::import("stardict");
            auto dataBase = dict.attr("StarDict")(
                py::str{ (dataPath / "wordcast.db").string() });
            Tagger tagger;

            py::module patternEn = py::module::import("pattern.en");
            mConjugate = patternEn.attr("conjugate");
            mPlur = patternEn.attr("pluralize");
            auto refer = patternEn.attr("referenced");
            mComp = patternEn.attr("comparative");
            mSuper = patternEn.attr("superlative");
            mSuggest = patternEn.attr("suggest");

            py::object infoLoader = dict.attr("DictHelper")();
            py::object loadExg = infoLoader.attr("exchange_loads");
            py::object loadPos = infoLoader.attr("pos_loads");

            WordLUT lut{ dataPath };
            static const char* prefix[] = {
                "a",     "ab",   "ap",    "ante",  "anti",  "auto",  "be",
                "bi",    "co",   "de",    "demi",  "dis",   "en",    "ex",
                "extra", "fore", "mid",   "post",  "hyper", "hypo",  "in",
                "im",    "ir",   "il",    "infra", "inter", "intra", "mis",
                "mono",  "non",  "off",   "oft",   "out",   "over",  "para",
                "post",  "pre",  "pro",   "poly",  "re",    "retro", "semi",
                "sub",   "sur",  "trans", "ultra", "un",    "under", "up"
            };
            static const char* suffix[] = {
                "able", "age",   "al",   "ance",   "ant",  "ate",  "arian",
                "ary",  "ation", "cy",   "cation", "dom",  "ee",   "er",
                "ese",  "esque", "ess",  "en",     "ence", "ety",  "fold",
                "ful",  "hood",  "ics",  "ial",    "ical", "ify",  "ion",
                "ish",  "ism",   "ist",  "ista",   "ite",  "itis", "ize",
                "ive",  "ity",   "less", "let",    "ling", "logy", "ment",
                "most", "ness",  "oid",  "or",     "ory",  "ous",  "ship",
                "t",    "ty",    "th",   "ward",   "wise", "y",    "ure",
                "ze"
            };

            BUS_TRACE_POINT();

            for(auto ref : dataBase) {
                BUS_TRACE_POINT();
                auto id = *ref.begin();
                auto info = dataBase.attr("query")(id);
                auto word = info["word"];
                if(!islower(word.cast<std::string>().front()))
                    continue;
                auto pos = loadPos(info["pos"]);
                auto exg = loadExg(info["exchange"]);
                if(exg.is_none())
                    exg = py::dict();
                auto tag = tagger.tagNLTK(word);

                bool drv = false;
                BUS_TRACE_POINT();
                //动词
                if(tagger.isVerb(tag, pos)) {
                    drv = true;
                    py::print("{} is verb"_s.format(word));
                    //第3人称单数
                    auto tsg = get3sg(word, exg);
                    if(!is3sgNormal(word, tsg))
                        out("第三人称单数", word.cast<std::string>(),
                            tsg.cast<std::string>());
                    //过去式
                    auto pass = getPass(word, exg);
                    if(!isPassNormal(word, pass)) {
                        out("过去式", word.cast<std::string>(),
                            pass.cast<std::string>());
                    }
                    //过去分词
                    auto ppart = getPPart(word, exg);
                    if(!isPPartNormal(word, ppart)) {
                        out("过去分词", word.cast<std::string>(),
                            ppart.cast<std::string>());
                    }
                    //现在分词
                    auto part = getPart(word, exg);
                    if(!isPartNormal(word, part)) {
                        out("现在分词", word.cast<std::string>(),
                            part.cast<std::string>());
                    }
                }
                BUS_TRACE_POINT();
                //名词
                if(tagger.isNoun(tag, pos)) {
                    drv = true;
                    py::print("{} is noun"_s.format(word));
                    //复数
                    auto plur = getPlur(word, exg);
                    if(!isPlurNormal(word, plur)) {
                        out("复数", word.cast<std::string>(),
                            plur.cast<std::string>());
                    }
                    //定冠词
                    auto ref = refer(word);
                    if(ref.cast<std::string>().substr(0, 2) == "an") {
                        std::string sw = word.cast<std::string>();
                        if(!(sw[0] == 'a' || sw[0] == 'e' || sw[0] == 'i' ||
                             sw[0] == 'o' || sw[0] == 'u'))
                            out("定冠词", word.cast<std::string>(), "an");
                    }
                }
                BUS_TRACE_POINT();
                //形容词
                if(tagger.isAdj(tag, pos)) {
                    drv = true;
                    py::print("{} is adj"_s.format(word));
                    //比较级
                    auto comp = getComp(word, exg);
                    if(!isCompNormal(word, comp)) {
                        out("比较级", word.cast<std::string>(),
                            comp.cast<std::string>());
                    }
                    //最高级
                    auto super = getSuper(word, exg);
                    if(!isSuperNormal(word, super)) {
                        out("最高级", word.cast<std::string>(),
                            super.cast<std::string>());
                    }
                }
                BUS_TRACE_POINT();
                //副词
                if(tagger.isAdv(tag, pos)) {
                    py::print("{} is adv"_s.format(word));
                }
                //前后缀派生词
                if(!drv)
                    continue;
                std::string sw = word.cast<std::string>();
                for(auto pre : prefix) {
                    std::string drv = pre + sw;
                    if(lut.count(drv)) {
                        out("派生：" + lut.translation(drv),
                            word.cast<std::string>(), drv);
                    }
                }
                for(auto suf : suffix) {
                    std::string drv = sw + suf;
                    if(lut.count(drv))
                        out("派生：" + lut.translation(drv),
                            word.cast<std::string>(), drv);
                }
                std::string old = sw;
                fixTail(sw);
                if(sw != old) {
                    for(auto suf : suffix) {
                        std::string drv = sw + suf;
                        if(lut.count(drv))
                            out("派生：" + lut.translation(drv),
                                word.cast<std::string>(), drv);
                    }
                }
                //末尾修改派生词
                sw = old;
                // ial,ious->iety
                auto checkTail = [&](const std::string& ot,
                                     const std::string& nt) {
                    if(sw.size() > ot.size() &&
                       sw.substr(sw.size() - ot.size()) == ot) {
                        std ::string drv =
                            sw.substr(0, sw.size() - ot.size()) + nt;
                        if(lut.count(drv))
                            out("派生：" + lut.translation(drv),
                                word.cast<std::string>(), drv);
                    }
                };
                checkTail("al", "ety");
                checkTail("ous", "ety");
                checkTail("ble", "bility");
                checkTail("t", "cy");
                checkTail("te", "cy");
                // checkTail("e", "");
                checkTail("t", "ssion");
                checkTail("ve", "f");
                checkTail("d", "se");
                checkTail("d", "sion");
                checkTail("de", "sion");
                checkTail("fy", "fication");
                checkTail("e", "ption");
                checkTail("ve", "ption");
                checkTail("be", "ption");
            }
        }
        BUS_TRACE_END();
    }

public:
    explicit WordCastGenerator(Bus::ModuleInstance& instance)
        : Command(instance) {}
    int doCommand(int argc, char** argv) override {
        BUS_TRACE_BEG() {
            Bus::FunctionId id{
                str2GUID("{D375A669-D1E1-431A-AE14-999A896AF1B2}"), "JsonConfig"
            };
            auto config = system().instantiate<Config>(id);
            config->load("config.json");
            fs::path dataBaseRoot = config->attribute("DataBase")->asString();
            std::ofstream out(dataBaseRoot / "English" / "ECDICTData" /
                              "CastGen.json");
            out << "[" << std::endl;
            bool first = true;
            auto write = [&](const std::string& note, const std::string& src,
                             const std::string& dst) {
                if(first)
                    first = false;
                else
                    out << ",\n";
                auto guid = xg::newGuid().str();
                for(char& c : guid)
                    if(islower(c))
                        c = toupper(c);
                out << "\"{" << guid << "}" << src << "->(" << note << ")["
                    << dst << "]\"";
            };
            gen(write, dataBaseRoot);
            out << "\n]\n" << std::endl;
            reporter().apply(ReportLevel::Info, "Done", BUS_DEFSRCLOC());
            return EXIT_SUCCESS;
        }
        BUS_TRACE_END();
    }
};

class WordCast final : public KnowledgeLibrary {
private:
    std::unordered_map<GUID, std::shared_ptr<Tester>, GUIDHasher> mKPS;
    uint32_t mFillCnt, mJudgeCnt;
    fs::path mDataBase;

public:
    explicit WordCast(Bus::ModuleInstance& instance)
        : KnowledgeLibrary(instance) {}

    void load(const fs::path& dataBase) {
        BUS_TRACE_BEG() {
            mFillCnt = mJudgeCnt = 0;
            mDataBase = dataBase / "English";
            Bus::FunctionId id{
                str2GUID("{D375A669-D1E1-431A-AE14-999A896AF1B2}"), ""
            };
            // Fill.json
            {
                id.name = "JsonConfig";
                auto cfg = system().instantiate<Config>(id);
                if(!cfg->load(mDataBase / "Cast.json"))
                    BUS_TRACE_THROW(
                        std::runtime_error("Failed to load Fill.json"));
                id.name = "FillTester";
                for(auto ele : cfg->expand()) {
                    ++mFillCnt;
                    auto tester = system().instantiate<Tester>(id);
                    GUID id = tester->init(ele);
                    auto& val = mKPS[id];
                    if(val)
                        BUS_TRACE_THROW(std::logic_error("GUID redefined"));
                    val.swap(tester);
                }
            }
        }
        BUS_TRACE_END();
    }
    GUIDTable getTable() {
        GUIDTable res;
        for(auto&& x : mKPS)
            res.emplace_back(x.first);
        return res;
    }
    TestResult test(GUID kpID) {
        BUS_TRACE_BEG() {
            TestResult res;
            res.kpID = { kpID };
            auto iter = mKPS.find(kpID);
            if(iter == mKPS.end())
                BUS_TRACE_THROW(std::logic_error("Invaild GUID"));
            res.result = iter->second->test();
            return res;
        }
        BUS_TRACE_END();
    }
    std::string summary() {
        std::stringstream ss;
        using Clock = fs::file_time_type::clock;
        ss << "Fill.json count: " << mFillCnt << std::endl;
        return ss.str();
    }
};

class Instance final : public Bus::ModuleInstance {
private:
    py::scoped_interpreter mInterpreter;

public:
    Instance(const fs::path& path, Bus::ModuleSystem& sys)
        : Bus::ModuleInstance(path, sys), mInterpreter(true) {
        py::print("Hello Pybind11");
        py::print(py::module::import("sys").attr("version"));
    }
    Bus::ModuleInfo info() const override {
        Bus::ModuleInfo res;
        res.name = BUS_DEFAULT_MODULE_NAME;
        res.guid = Bus::str2GUID("{3D20059B-9A17-4142-85E9-C08CE5A56130}");
        res.busVersion = BUS_VERSION;
        res.version = "0.0.1";
        res.description = "English";
        res.copyright = "Copyright (c) 2020 Zheng Yingwei";
        res.modulePath = getModulePath();
        return res;
    }
    std::vector<Bus::Name> list(Bus::Name api) const override {
        if(api == KnowledgeLibrary::getInterface())
            return { "WordCast" };
        if(api == Command::getInterface())
            return { "WordCastGen" };
        return {};
    }
    std::shared_ptr<Bus::ModuleFunctionBase> instantiate(Name name) override {
        if(name == "WordCast")
            return std::make_shared<WordCast>(*this);
        if(name == "WordCastGen")
            return std::make_shared<WordCastGenerator>(*this);
        return nullptr;
    }
};

BUS_API void busInitModule(const Bus::fs::path& path, Bus::ModuleSystem& system,
                           std::shared_ptr<Bus::ModuleInstance>& instance) {
    instance = std::make_shared<Instance>(path, system);
}
