#include "../Shared/Command.hpp"
#pragma warning(push, 0)
#define GUID_DEFINED
#include "../ThirdParty/Bus/BusSystem.hpp"
#include <rang.hpp>
#pragma warning(pop)
#include <sstream>

BUS_MODULE_NAME("NCEEHelper.Main");

// TODO:Command usage help

static int mainImpl(int argc, char** argv, Bus::ModuleSystem& sys) {
    BUS_TRACE_BEG() {
        if(argc < 2) {
            std::stringstream ss;
            ss << "Possible Command:" << std::endl;
            for(auto id : sys.list<Command>()) {
                ss << id.name << std::endl;
            }
            sys.getReporter().apply(ReportLevel::Info, ss.str(),
                                    BUS_DEFSRCLOC());
            BUS_TRACE_THROW(std::invalid_argument("Need Command Name"));
        }
        std::shared_ptr<Command> command =
            sys.instantiateByName<Command>(argv[1]);
        if(!command)
            BUS_TRACE_THROW(std::invalid_argument("No function called " +
                                                  std::string(argv[1])));
        std::vector<char*> nargv{ argv[0] };
        for(int i = 2; i < argc; ++i)
            nargv.emplace_back(argv[i]);
        return command->doCommand(static_cast<int>(nargv.size()), nargv.data());
    }
    BUS_TRACE_END();
}

static Bus::ReportFunction colorOutput(std::ostream& out, rang::fg col,
                                       const std::string& pre,
                                       bool inDetail = false) {
    return [=, &out](Bus::ReportLevel, const std::string& message,
                     const Bus::SourceLocation& srcLoc) {
        if(srcLoc.module == std::string("BusSystem.MSVCDelayLoader"))
            return;
        out << col;
        if(inDetail) {
            out << pre << ':' << message << std::endl;
            out << "module:" << srcLoc.module << std::endl;
            out << "function:" << srcLoc.functionName << std::endl;
            out << "location:" << srcLoc.srcFile << " line " << srcLoc.line
                << std::endl;
        } else {
            out << pre << "[" << srcLoc.module << "]:" << message << std::endl;
        }
        out << std::endl << rang::fg::reset;
    };
}

static void processException(const std::exception& ex,
                             const std::string& lastModule);
static void processException(const Bus::SourceLocation& ex,
                             const std::string& lastModule);

template <typename T>
static void nestedException(const T& exc, const std::string& lastModule) {
    try {
        std::rethrow_if_nested(exc);
    } catch(const std::exception& ex) {
        if(&ex)
            processException(ex, lastModule);
        else {
            std::cerr << "Unrecognizable Exception." << std::endl;
        }
    } catch(const Bus::SourceLocation& ex) {
        if(&ex)
            processException(ex, lastModule);
        else {
            std::cerr << "Unrecognizable SourceLocation." << std::endl;
        }
    } catch(...) {
        std::cerr << "Unknown Error" << std::endl;
    }
}

static void processException(const std::exception& ex,
                             const std::string& lastModule) {
    std::cerr << "Reason:" << ex.what() << std::endl;
    nestedException(ex, lastModule);
}

static void processException(const Bus::SourceLocation& src,
                             const std::string& lastModule) {
    if(lastModule != src.module)
        std::cerr << "in module " << src.module << std::endl;
    std::cerr << src.functionName << " at " << src.srcFile << " line "
              << src.line << std::endl;
    nestedException(src, src.module);
}

#define PROCEXC()                                                \
    catch(const Bus::SourceLocation& ex) {                       \
        std::cerr << rang::fg::red << "Exception:" << std::endl; \
        processException(ex, "");                                \
        std::cerr << rang::fg::reset;                            \
    }                                                            \
    catch(const std::exception& ex) {                            \
        std::cerr << rang::fg::red << "Exception:" << std::endl; \
        processException(ex, "");                                \
        std::cerr << rang::fg::reset;                            \
    }

static void loadPlugins(Bus::ModuleSystem& sys) {
    for(auto p : fs::directory_iterator("Plugins")) {
        if(p.status().type() == fs::file_type::directory) {
            auto dir = p.path();
            auto dll = dir / dir.filename().replace_extension(".dll");
            if(fs::exists(dll)) {
                try {
                    sys.getReporter().apply(ReportLevel::Info,
                                            "Loading module " +
                                                dir.filename().string(),
                                            BUS_DEFSRCLOC());
                    sys.loadModuleFile(dll);
                }
                PROCEXC();
            }
        }
    }
    std::stringstream ss;
    ss << "Loaded Module:" << std::endl;
    for(auto mod : sys.listModules()) {
        ss << Bus::GUID2Str(mod.guid) << " " << mod.name << " " << mod.version
           << std::endl;
    }
    sys.getReporter().apply(ReportLevel::Info, ss.str(), BUS_DEFSRCLOC());
}

std::shared_ptr<Bus::ModuleInstance> getBuiltin(Bus::ModuleSystem& sys);

int main(int argc, char** argv) {
    auto reporter = std::make_shared<Bus::Reporter>();
    reporter->addAction(
        ReportLevel::Debug,
        colorOutput(std::cerr, rang::fg::yellow, "Debug", true));
    reporter->addAction(ReportLevel::Error,
                        colorOutput(std::cerr, rang::fg::red, "Error", true));
    reporter->addAction(ReportLevel::Info,
                        colorOutput(std::cerr, rang::fg::reset, "Info"));
    reporter->addAction(ReportLevel::Warning,
                        colorOutput(std::cerr, rang::fg::yellow, "Warning"));
    try {
        Bus::ModuleSystem sys(reporter, [] {
            try {
                std::rethrow_exception(std::current_exception());
            }
            PROCEXC();
        });
        try {
            Bus::addModuleSearchPath(fs::current_path() / "Shared", *reporter);
            sys.wrapBuiltin(getBuiltin);
            loadPlugins(sys);
            return mainImpl(argc, argv, sys);
        }
        PROCEXC();
    }
    PROCEXC();
    return EXIT_FAILURE;
}
