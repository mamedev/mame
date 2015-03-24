// LoadCodecs.h

#ifndef __LOAD_CODECS_H
#define __LOAD_CODECS_H

#include "../../../Common/MyCom.h"
#include "../../../Common/MyString.h"
#include "../../../Common/Buffer.h"
#include "../../ICoder.h"

#ifdef EXTERNAL_CODECS
#include "../../../Windows/DLL.h"
#endif

struct CDllCodecInfo
{
  CLSID Encoder;
  CLSID Decoder;
  bool EncoderIsAssigned;
  bool DecoderIsAssigned;
  int LibIndex;
  UInt32 CodecIndex;
};

#include "../../Archive/IArchive.h"

typedef IInArchive * (*CreateInArchiveP)();
typedef IOutArchive * (*CreateOutArchiveP)();

struct CArcExtInfo
{
  UString Ext;
  UString AddExt;
  CArcExtInfo() {}
  CArcExtInfo(const UString &ext): Ext(ext) {}
  CArcExtInfo(const UString &ext, const UString &addExt): Ext(ext), AddExt(addExt) {}
};


struct CArcInfoEx
{
  #ifdef EXTERNAL_CODECS
  int LibIndex;
  UInt32 FormatIndex;
  CLSID ClassID;
  #endif
  bool UpdateEnabled;
  CreateInArchiveP CreateInArchive;
  CreateOutArchiveP CreateOutArchive;
  UString Name;
  CObjectVector<CArcExtInfo> Exts;
  #ifndef _SFX
  CByteBuffer StartSignature;
  // CByteBuffer FinishSignature;
  #ifdef NEW_FOLDER_INTERFACE
  UStringVector AssociateExts;
  #endif
  #endif
  bool KeepName;
  
  UString GetMainExt() const
  {
    if (Exts.IsEmpty())
      return UString();
    return Exts[0].Ext;
  }
  int FindExtension(const UString &ext) const;
  
  /*
  UString GetAllExtensions() const
  {
    UString s;
    for (int i = 0; i < Exts.Size(); i++)
    {
      if (i > 0)
        s += ' ';
      s += Exts[i].Ext;
    }
    return s;
  }
  */

  void AddExts(const wchar_t* ext, const wchar_t* addExt);

  CArcInfoEx():
    #ifdef EXTERNAL_CODECS
    LibIndex(-1),
    #endif
    UpdateEnabled(false),
    CreateInArchive(0), CreateOutArchive(0),
    KeepName(false)
    #ifndef _SFX
    #endif
  {}
};

#ifdef EXTERNAL_CODECS
typedef UInt32 (WINAPI *GetMethodPropertyFunc)(UInt32 index, PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *CreateObjectFunc)(const GUID *clsID, const GUID *interfaceID, void **outObject);


#ifdef NEW_FOLDER_INTERFACE
struct CCodecIcons
{
  struct CIconPair
  {
    UString Ext;
    int IconIndex;
  };
  CObjectVector<CIconPair> IconPairs;
  void LoadIcons(HMODULE m);
  bool FindIconIndex(const UString &ext, int &iconIndex) const;
};
#endif

struct CCodecLib
#ifdef NEW_FOLDER_INTERFACE
: public CCodecIcons
#endif
{
  NWindows::NDLL::CLibrary Lib;
  GetMethodPropertyFunc GetMethodProperty;
  CreateObjectFunc CreateObject;
  #ifdef NEW_FOLDER_INTERFACE
  FString Path;
  void LoadIcons() { CCodecIcons::LoadIcons((HMODULE)Lib); }
  #endif
  CCodecLib(): GetMethodProperty(0) {}
};
#endif

class CCodecs:
  #ifdef EXTERNAL_CODECS
  public ICompressCodecsInfo,
  #else
  public IUnknown,
  #endif
  public CMyUnknownImp
{
public:
  #ifdef EXTERNAL_CODECS
  CObjectVector<CCodecLib> Libs;
  CObjectVector<CDllCodecInfo> Codecs;

  #ifdef NEW_FOLDER_INTERFACE
  CCodecIcons InternalIcons;
  #endif

  HRESULT LoadCodecs();
  HRESULT LoadFormats();
  HRESULT LoadDll(const FString &path, bool needCheckDll);
  HRESULT LoadDllsFromFolder(const FString &folderPrefix);

  HRESULT CreateArchiveHandler(const CArcInfoEx &ai, void **archive, bool outHandler) const
  {
    return Libs[ai.LibIndex].CreateObject(&ai.ClassID, outHandler ? &IID_IOutArchive : &IID_IInArchive, (void **)archive);
  }
  #endif

public:
  CObjectVector<CArcInfoEx> Formats;
  HRESULT Load();
  
  #ifndef _SFX
  int FindFormatForArchiveName(const UString &arcPath) const;
  int FindFormatForExtension(const UString &ext) const;
  int FindFormatForArchiveType(const UString &arcType) const;
  bool FindFormatForArchiveType(const UString &arcType, CIntVector &formatIndices) const;
  #endif

  MY_UNKNOWN_IMP

  #ifdef EXTERNAL_CODECS
  STDMETHOD(GetNumberOfMethods)(UInt32 *numMethods);
  STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
  STDMETHOD(CreateDecoder)(UInt32 index, const GUID *interfaceID, void **coder);
  STDMETHOD(CreateEncoder)(UInt32 index, const GUID *interfaceID, void **coder);
  #endif

  int GetCodecLibIndex(UInt32 index);
  bool GetCodecEncoderIsAssigned(UInt32 index);
  HRESULT GetCodecId(UInt32 index, UInt64 &id);
  UString GetCodecName(UInt32 index);

  HRESULT CreateInArchive(int formatIndex, CMyComPtr<IInArchive> &archive) const
  {
    const CArcInfoEx &ai = Formats[formatIndex];
    #ifdef EXTERNAL_CODECS
    if (ai.LibIndex < 0)
    #endif
    {
      archive = ai.CreateInArchive();
      return S_OK;
    }
    #ifdef EXTERNAL_CODECS
    return CreateArchiveHandler(ai, (void **)&archive, false);
    #endif
  }
  HRESULT CreateOutArchive(int formatIndex, CMyComPtr<IOutArchive> &archive) const
  {
    const CArcInfoEx &ai = Formats[formatIndex];
    #ifdef EXTERNAL_CODECS
    if (ai.LibIndex < 0)
    #endif
    {
      archive = ai.CreateOutArchive();
      return S_OK;
    }
    #ifdef EXTERNAL_CODECS
    return CreateArchiveHandler(ai, (void **)&archive, true);
    #endif
  }
  int FindOutFormatFromName(const UString &name) const
  {
    for (int i = 0; i < Formats.Size(); i++)
    {
      const CArcInfoEx &arc = Formats[i];
      if (!arc.UpdateEnabled)
        continue;
      if (arc.Name.CompareNoCase(name) == 0)
        return i;
    }
    return -1;
  }

  #ifdef EXTERNAL_CODECS
  HRESULT CreateCoder(const UString &name, bool encode, CMyComPtr<ICompressCoder> &coder) const;
  #endif

};

#endif
