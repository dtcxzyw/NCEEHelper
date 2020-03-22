#pragma once
#include <string>
#ifdef WIN32
#include <atlbase.h>
#include <sapi.h>
#include <sphelper.h>
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
            CComPtr<ISpObjectToken> cpVoiceToken;
            CComPtr<IEnumSpObjectTokens> cpEnum;
            ULONG ulCount = 0;

            // Create the SAPI voice.

            SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);

            cpEnum->GetCount(&ulCount);

            // Obtain a list of available voice tokens, set
            // the voice to the token, and call Speak.
            while(ulCount--) {
                cpVoiceToken.Release();
                cpEnum->Next(1, &cpVoiceToken, NULL);
                LPWSTR str = nullptr;
                cpVoiceToken->GetId(&str);
                wprintf(L"%s\n", str);
                mVoice->SetVoice(cpVoiceToken);
            }
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
