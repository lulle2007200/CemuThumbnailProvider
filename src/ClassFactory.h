#pragma once

#include <unknwn.h>     
#include <windows.h>


class ClassFactory : public IClassFactory
{
public:
    IFACEMETHODIMP QueryInterface(REFIID RefIID, void **Ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    IFACEMETHODIMP CreateInstance(IUnknown *POuterIUnknown, REFIID RefIID, void **Ppv);
    IFACEMETHODIMP LockServer(BOOL Lock);

    ClassFactory();

protected:
    ~ClassFactory();

private:
    long RefCount;
};