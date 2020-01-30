#include <Shlwapi.h>
#include "ClassFactory.h"
#include "RpxThumbnailProvider.h"
#include <new>



extern long DLLRefCount;


ClassFactory::ClassFactory() : RefCount(1)
{
    InterlockedIncrement(&DLLRefCount);
}

ClassFactory::~ClassFactory()
{
    InterlockedDecrement(&DLLRefCount);
}

IFACEMETHODIMP ClassFactory::QueryInterface(REFIID RefIID, void **Ppv)
{
    static const QITAB Qit[] = {
        QITABENT(ClassFactory, IClassFactory), { 0 }};
    return QISearch(this, Qit, RefIID, Ppv);
}

IFACEMETHODIMP_(ULONG) ClassFactory::AddRef()
{
    return InterlockedIncrement(&RefCount);
}

IFACEMETHODIMP_(ULONG) ClassFactory::Release()
{
    ULONG TempRef = InterlockedDecrement(&RefCount);
    if (0 == TempRef)
    {
        delete this;
    }
    return TempRef;
}

IFACEMETHODIMP ClassFactory::CreateInstance(IUnknown *POuterIUnknown, REFIID RefIID, void **Ppv)
{
    HRESULT hr = HRESULT_FROM_WIN32(CLASS_E_NOAGGREGATION);

    if (POuterIUnknown == NULL)
    {
        hr = HRESULT_FROM_WIN32(E_OUTOFMEMORY);
        RpxThumbnailProvider *ThumbnailProvider = new RpxThumbnailProvider();
        if (ThumbnailProvider!=NULL)
        {
            hr = HRESULT_FROM_WIN32(ThumbnailProvider->QueryInterface(RefIID, Ppv));
            ThumbnailProvider->Release();
        }
    }

    return hr;
}

IFACEMETHODIMP ClassFactory::LockServer(BOOL Lock)
{
    if (Lock)
    {
        InterlockedIncrement(&DLLRefCount);
    }
    else
    {
        InterlockedDecrement(&DLLRefCount);
    }
    return S_OK;
}