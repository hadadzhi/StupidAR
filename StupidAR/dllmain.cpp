#include <comdef.h>

#include "streams.h"
#include "common.h"
#include "MyRenderer.h"

namespace {
    const AMOVIESETUP_MEDIATYPE kSetupMediaTypes[] = {
        { &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM },
        { &MEDIATYPE_Audio, &MEDIASUBTYPE_IEEE_FLOAT },
    };

    const AMOVIESETUP_PIN kSetupPins[] = {
        { L"", TRUE, FALSE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(kSetupMediaTypes), kSetupMediaTypes },
    };

    const AMOVIESETUP_FILTER kSetupFilter = { &StupidAR::kMyFilterGuid, StupidAR::kMyFilterName, MERIT_DO_NOT_USE, _countof(kSetupPins), kSetupPins };

    CUnknown* WINAPI CreateFilter(IUnknown* pUnknown, HRESULT* pResult) {
        if (FAILED(*pResult)) {
            return nullptr;
        }

        auto pInstance = new(std::nothrow) StupidAR::MyRenderer(pUnknown, pResult);

        if (!pInstance) {
            *pResult = E_OUTOFMEMORY;
        }

        return pInstance;
    }

    HRESULT DllRegUnregServer(bool reg) {
        RETURN_FAILED(AMovieDllRegisterServer2(reg));

        _COM_SMARTPTR_TYPEDEF(IFilterMapper2, __uuidof(IFilterMapper2));

        IFilterMapper2Ptr fm;
        RETURN_FAILED(CoCreateInstance(CLSID_FilterMapper2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fm)));

        HRESULT hr = fm->UnregisterFilter(nullptr, nullptr, *kSetupFilter.clsID);
        if (FAILED(hr) && 0x80070002 != hr) { // 0x80070002 is returned if the filter is not registered
            return hr;
        }
        hr = fm->UnregisterFilter(&CLSID_AudioRendererCategory, nullptr, *kSetupFilter.clsID);
        if (FAILED(hr) && 0x80070002 != hr) {
            return hr;
        }

        if (reg) { // Register the filter under the correct category
            const REGFILTER2 rf = { 1, kSetupFilter.dwMerit, kSetupFilter.nPins, kSetupFilter.lpPin };
            RETURN_FAILED(fm->RegisterFilter(*kSetupFilter.clsID, kSetupFilter.strName, nullptr, &CLSID_AudioRendererCategory, nullptr, &rf));
        }

        return S_OK;
    }
}

CFactoryTemplate g_Templates[] = {
	{ StupidAR::kMyFilterName, &StupidAR::kMyFilterGuid, CreateFilter, nullptr, &kSetupFilter }
};

int g_cTemplates = _countof(g_Templates);

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD dwReason, LPVOID pReserved) {
	return DllEntryPoint(hDllHandle, dwReason, pReserved);
}

STDAPI DllRegisterServer() {
	return DllRegUnregServer(true);
}

STDAPI DllUnregisterServer() {
	return DllRegUnregServer(false);
}
