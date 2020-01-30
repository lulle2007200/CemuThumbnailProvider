#pragma once

#include <windows.h>


HRESULT RegisterInprocServer(PCWSTR Module, const CLSID& RefCLSID, PCWSTR FriendlyName, PCWSTR ThreadModel);

HRESULT UnregisterInprocServer(const CLSID& RefCLSID);

HRESULT RegisterThumbnailHandler(PCWSTR FileType, const CLSID& RefCLSID);

HRESULT UnregisterThumbnailHandler(PCWSTR FileType);