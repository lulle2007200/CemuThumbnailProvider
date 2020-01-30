/******************************** Module Header ********************************\
Module Name:  RpxThumbnailProvider.cpp
Project:      CppShellExtThumbnailHandler
Copyright (c) Microsoft Corporation.

The code sample demonstrates the C++ implementation of a thumbnail handler 
for a new file type registered with the .recipe extension. 

A thumbnail image handler provides an image to represent the item. It lets you 
customize the thumbnail of files with a specific file extension. Windows Vista 
and newer operating systems make greater use of file-specific thumbnail images 
than earlier versions of Windows. Thumbnails of 32-bit resolution and as large 
as 256x256 pixels are often used. File format owners should be prepared to 
display their thumbnails at that size. 

The example thumbnail handler implements the IInitializeWithStream and 
IThumbnailProvider interfaces, and provides thumbnails for .recipe files. 
The .recipe file type is simply an XML file registered as a unique file name 
extension. It includes an element called "Picture", embedding an image file. 
The thumbnail handler extracts the embedded image and asks the Shell to 
display it as a thumbnail.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\*******************************************************************************/

#include "RpxThumbnailProvider.h"
#include <Shlwapi.h>
#include <wincrypt.h>   // For CryptStringToBinary.
#include <MsXml6.h>
#include <iostream>
#include <fstream>
#include "libtga-1.0.1\src\tga.h"

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "msxml6.lib")



extern HINSTANCE g_hInst;
extern long DLLRefCount;

const char * ImgType[] = {"no image data included", "color mapped", "grayscale", "true-color (RGB/RGBA)", "RLE color mapped", "RLE grayscale", "RLE true-color"};


RpxThumbnailProvider::RpxThumbnailProvider() : RefCount(1), FullTgaPath(NULL), isInitialized(FALSE)
{
    InterlockedIncrement(&DLLRefCount);
}


RpxThumbnailProvider::~RpxThumbnailProvider()
{
    InterlockedDecrement(&DLLRefCount);
}


IFACEMETHODIMP RpxThumbnailProvider::QueryInterface(REFIID RefRIID, void **Ppv)
{
    static const QITAB Qit[] = {QITABENT(RpxThumbnailProvider, IThumbnailProvider), QITABENT(RpxThumbnailProvider, IInitializeWithFile), { 0 }};
    return QISearch(this, Qit, RefRIID, Ppv);
}


IFACEMETHODIMP_(ULONG) RpxThumbnailProvider::AddRef()
{
    return InterlockedIncrement(&RefCount);
}

IFACEMETHODIMP_(ULONG) RpxThumbnailProvider::Release()
{
    ULONG TempRef = InterlockedDecrement(&RefCount);
    if (TempRef == 0)
    {
        delete this;
    }
    return TempRef;
}

IFACEMETHODIMP RpxThumbnailProvider::Initialize(LPCWSTR FilePath, DWORD Mode){
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);

    if(!isInitialized){
        isInitialized = TRUE;

        wchar_t *FullPath = new wchar_t [MAX_PATH];

        wcscpy_s(FullPath, MAX_PATH, FilePath);
        wcscat_s(FullPath, MAX_PATH, L"\\..\\..\\meta\\boottvtex.tga");
        if(!PathFileExistsW(FullPath)){
            hr = ERROR_PATH_NOT_FOUND;
        }else{
            FullTgaPath = new char [MAX_PATH];
            WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_SEPCHARS, FullPath, -1, FullTgaPath, MAX_PATH, NULL, NULL);

            hr =  HRESULT_FROM_WIN32(S_OK);
        }
        delete [] FullPath;
    }
    return hr;
}

IFACEMETHODIMP RpxThumbnailProvider::GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *Alpha)
{
    *phbmp = NULL;
    *Alpha = WTSAT_UNKNOWN;

    if(!isInitialized || (FullTgaPath==NULL)){
        return(HRESULT_FROM_WIN32(E_FAIL));
    }

    TGA *TGAThumbnail = TGAOpen(FullTgaPath, "rb");

    if(TGAThumbnail == NULL){
        return( HRESULT_FROM_WIN32(E_FAIL));
    }

    TGAData TGAThumbnailData;
    TGAThumbnailData.flags = TGA_IMAGE_DATA | TGA_IMAGE_ID | TGA_BGR;

    if(TGAReadImage(TGAThumbnail, &TGAThumbnailData)!=TGA_OK){
        TGAClose(TGAThumbnail);
        return( HRESULT_FROM_WIN32(E_FAIL));
    }

    char *buffer = (char *) malloc(TGAThumbnail->hdr.width*TGAThumbnail->hdr.height*4);

    if(buffer==NULL){
        TGAClose(TGAThumbnail);
        return( HRESULT_FROM_WIN32(E_OUTOFMEMORY));
    }

    char *CurrentSubPixelPNG = buffer;
    char *CurrentSubPixelTGA = (char *)TGAThumbnailData.img_data;
    
    for(int i=TGAThumbnail->hdr.height-1; i>=0;i--){

        CurrentSubPixelPNG = buffer + i*4*TGAThumbnail->hdr.width;

        for(int j=0; j< TGAThumbnail->hdr.width;j++){
            *CurrentSubPixelPNG  = *CurrentSubPixelTGA;
            CurrentSubPixelTGA++;
            CurrentSubPixelPNG++;
            *CurrentSubPixelPNG  = *CurrentSubPixelTGA;
            CurrentSubPixelTGA++;
            CurrentSubPixelPNG++;
            *CurrentSubPixelPNG  = *CurrentSubPixelTGA;
            CurrentSubPixelTGA++;
            CurrentSubPixelPNG++;
            if(TGAThumbnail->hdr.alpha!=0){
                *CurrentSubPixelPNG  = *CurrentSubPixelTGA;
                CurrentSubPixelTGA++;

            }else{
                *CurrentSubPixelPNG  = 0;
            }
            CurrentSubPixelPNG++;
        }
    }

    TGAClose(TGAThumbnail);
   
    *phbmp = CreateBitmap(TGAThumbnail->hdr.width, TGAThumbnail->hdr.height, 1, 32, (void *)buffer);

    if(*phbmp == NULL){
        return( HRESULT_FROM_WIN32(GetLastError()));
    }

    if(TGAThumbnail->hdr.alpha !=0){
        *Alpha=WTSAT_ARGB;
    }else{
        *Alpha=WTSAT_RGB;
    }

    return  HRESULT_FROM_WIN32(S_OK);
}




