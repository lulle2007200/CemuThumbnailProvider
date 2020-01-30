#pragma once
#include <Windows.h>
#include <thumbcache.h>  


class RpxThumbnailProvider : 
    public IInitializeWithFile, 
    public IThumbnailProvider
{
public:
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    IFACEMETHODIMP Initialize(LPCWSTR FilePath, DWORD Mode);

    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);

    RpxThumbnailProvider();

protected:
    ~RpxThumbnailProvider();

private:
    long RefCount;

    char *FullTgaPath;
    BOOL isInitialized;
};