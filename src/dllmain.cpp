#include <windows.h>
#include <Guiddef.h>
#include <shlobj.h>               
#include "ClassFactory.h"           
#include "Reg.h"
#include <iostream>

const CLSID CLSID_RpxThumbnailProvider = 
{ 0xa66a266a, 0x1d43, 0x41e8, { 0x9D, 0xF1, 0x1E, 0x66, 0xDF, 0x1C, 0x43, 0xB4 } };


HINSTANCE   DLLInstance     = NULL;
long        DLLRefCount   = 0;


BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
        DLLInstance = Module;
        DisableThreadLibraryCalls(Module);
        break;
	default:
		break;
	}
	return TRUE;
}

STDAPI DllGetClassObject(REFCLSID RefCLSID, REFIID RefIID, void **Ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (IsEqualCLSID(CLSID_RpxThumbnailProvider, RefCLSID))
    {
        hr = E_OUTOFMEMORY;

        ClassFactory *pClassFactory = new ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(RefIID, Ppv);
            pClassFactory->Release();
        }
    }
    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    return DLLRefCount > 0 ? S_FALSE : S_OK;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = HRESULT_FROM_WIN32(S_OK);

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(DLLInstance, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    hr = HRESULT_FROM_WIN32(RegisterInprocServer(szModule, CLSID_RpxThumbnailProvider, L"WiiUGameThumbnailHandler.RpxThumbnailProvider Class", L"Apartment"));
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegisterThumbnailHandler(L".rpx", CLSID_RpxThumbnailProvider));
        if (SUCCEEDED(hr))
        {
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        }
    }
    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = HRESULT_FROM_WIN32(S_OK);

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(DLLInstance, szModule, ARRAYSIZE(szModule)) == 0)
    {   
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    hr = UnregisterInprocServer(CLSID_RpxThumbnailProvider);
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(UnregisterThumbnailHandler(L".rpx"));
    }
    return hr;
}


/*cl /LD /D UNICODE dllmain.cpp reg.cpp rpxthumbnailprovider.cpp classfactory.cpp libtga-1.0.1\src\tga.c libtga-1.0.1\src\tgaread.c libtga-1.0.1\src\tgawrite.c /link /DEF:globalexportfunctions.def /OUT:RpxThumbnailProvider.dll user32.lib gdi32.lib oleaut32.lib  Advapi32.lib Shell32.lib Ole32.lib*/