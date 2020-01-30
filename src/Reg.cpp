#include "Reg.h"
#include <strsafe.h>

HRESULT SetHKCURegKeyVal(PCWSTR SubKey, PCWSTR ValName, 
    PCWSTR Data)
{
    HRESULT hr;
    HKEY Key = NULL;

    hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CURRENT_USER, SubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &Key, NULL));

    if (SUCCEEDED(hr))
    {
        if (Data != NULL)
        {
            DWORD DataSize = lstrlen(Data) * sizeof(*Data);
            hr = HRESULT_FROM_WIN32(RegSetValueEx(Key, ValName, 0, REG_SZ, reinterpret_cast<const BYTE *>(Data), DataSize));
        }

        RegCloseKey(Key);
    }

    return hr;
}

HRESULT SetHKCURegKeyValDWORD(PCWSTR SubKey, PCWSTR ValName, 
    DWORD Data)
{
    HRESULT hr;
    HKEY Key = NULL;
 
    hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CURRENT_USER, SubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &Key, NULL));

    if (SUCCEEDED(hr))
    {
        if (Data != NULL)
        {
            hr = HRESULT_FROM_WIN32(RegSetValueEx(Key, ValName, 0,  REG_DWORD, (const BYTE *)&Data, sizeof(DWORD)));
        }

        RegCloseKey(Key);
    }

    return hr;
}

HRESULT GetHKCURegKeyVal(PCWSTR SubKey, PCWSTR ValName, 
    PWSTR Data, DWORD DataSize)
{
    HRESULT hr;
    HKEY Key = NULL;

    hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_CURRENT_USER, SubKey, 0,  KEY_READ, &Key));

    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegQueryValueEx(Key, (ValName), NULL, NULL, reinterpret_cast<LPBYTE>(Data), &DataSize));

        RegCloseKey(Key);
    }

    return hr;
}

HRESULT RegisterInprocServer(PCWSTR Module, const CLSID& RefCLSID, 
    PCWSTR FriendlyName, PCWSTR ThreadModel)
{
    if (Module == NULL || ThreadModel == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(RefCLSID, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t SubKey[MAX_PATH];

    hr = StringCchPrintf(SubKey, ARRAYSIZE(SubKey), L"Software\\Classes\\CLSID\\%s", szCLSID);
    if (SUCCEEDED(hr))
    {
        hr = SetHKCURegKeyVal(SubKey, NULL, FriendlyName);
        hr = SetHKCURegKeyValDWORD(SubKey, L"DisableProcessIsolation", 1);

        if (SUCCEEDED(hr))
        {
            hr = StringCchPrintf(SubKey, ARRAYSIZE(SubKey), 
                L"Software\\Classes\\CLSID\\%s\\InprocServer32", szCLSID);
            if (SUCCEEDED(hr))
            {

                hr = SetHKCURegKeyVal(SubKey, NULL, Module);
                if (SUCCEEDED(hr))
                {
                    hr = SetHKCURegKeyVal(SubKey, 
                        L"ThreadingModel", ThreadModel);
                }
            }
        }
    }

    return hr;
}

HRESULT UnregisterInprocServer(const CLSID& RefCLSID)
{
    HRESULT hr = S_OK;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(RefCLSID, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t SubKey[MAX_PATH];

    hr = HRESULT_FROM_WIN32(StringCchPrintf(SubKey, ARRAYSIZE(SubKey), L"Software\\Classes\\CLSID\\%s", szCLSID));
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CURRENT_USER, SubKey));
    }

    return hr;
}

HRESULT RegisterThumbnailHandler(PCWSTR FileType, const CLSID& RefCLSID)
{
    if (FileType == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(RefCLSID, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t SubKey[MAX_PATH];

    if (*FileType == L'.')
    {
        wchar_t szDefaultVal[260];
        hr = GetHKCURegKeyVal(FileType, NULL, szDefaultVal,  sizeof(szDefaultVal));

        if (SUCCEEDED(hr) && szDefaultVal[0] != L'\0')
        {
            FileType = szDefaultVal;
        }
    }

    hr = StringCchPrintf(SubKey, ARRAYSIZE(SubKey), L"Software\\Classes\\%s\\shellex\\{e357fccd-a995-4576-b01f-234630154e96}", FileType);
    if (SUCCEEDED(hr))
    {
        hr = SetHKCURegKeyVal(SubKey, NULL, szCLSID);
    }

    return hr;
}

HRESULT UnregisterThumbnailHandler(PCWSTR FileType)
{
    if (FileType == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t SubKey[MAX_PATH];

    if (*FileType == L'.')
    {
        wchar_t szDefaultVal[260];
        hr = GetHKCURegKeyVal(FileType, NULL, szDefaultVal, sizeof(szDefaultVal));

        if (SUCCEEDED(hr) && szDefaultVal[0] != L'\0')
        {
            FileType = szDefaultVal;
        }
    }

    hr = StringCchPrintf(SubKey, ARRAYSIZE(SubKey), L"Software\\Classes\\%s\\shellex\\{e357fccd-a995-4576-b01f-234630154e96}", FileType);
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CURRENT_USER, SubKey));
    }

    return hr;
}