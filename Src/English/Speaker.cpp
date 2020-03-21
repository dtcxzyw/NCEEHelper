#pragma once
#include <string>
#ifdef WIN32
#include <sapi.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "sapi.lib")
namespace {
    class SAPI final {
    private:
        ISpVoice* mVoice;

    public:
        SAPI() {
            ::CoInitialize(NULL);
            CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice,
                             (void**)&mVoice);
        }
        ~SAPI() {
            mVoice->Release();
            ::CoUninitialize();
        }
        void speak(LPCWSTR str) {
            mVoice->Speak(str, SPF_ASYNC, NULL);
        }
    };
}  // namespace

static std::wstring string2wstring(const std::string& str) {
    std::wstring result;
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(),
                                  static_cast<int>(str.size()), NULL, 0);
    result.resize(len);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()),
                        result.data(), len);
    return result;
}
void speak(const std::string& str) {
    static SAPI speaker;
    auto wstr = string2wstring(str);
    speaker.speak(wstr.c_str());
}
#else
void speak(const std::string& str) {}
#endif  //  WIN32
