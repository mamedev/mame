// LoadCodecs.cpp

#include "StdAfx.h"

#include "LoadCodecs.h"

#include "../../../Common/MyCom.h"
#ifdef NEW_FOLDER_INTERFACE
#include "../../../Common/StringToInt.h"
#endif
#include "../../../Windows/PropVariant.h"

#include "../../ICoder.h"
#include "../../Common/RegisterArc.h"

#ifdef EXTERNAL_CODECS
#include "../../../Windows/FileFind.h"
#include "../../../Windows/DLL.h"
#ifdef NEW_FOLDER_INTERFACE
#include "../../../Windows/ResourceString.h"
static const UINT kIconTypesResId = 100;
#endif

#ifdef _WIN32
#include "Windows/FileName.h"
#include "Windows/Registry.h"
#endif

using namespace NWindows;
using namespace NFile;

#ifdef _WIN32
extern HINSTANCE g_hInstance;
#endif

#define kCodecsFolderName FTEXT("Codecs")
#define kFormatsFolderName FTEXT("Formats")
static CFSTR kMainDll = FTEXT("7z.dll");

#ifdef _WIN32

static LPCTSTR kRegistryPath = TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("7-zip");
static LPCWSTR kProgramPathValue = L"Path";
static LPCWSTR kProgramPath2Value = L"Path"
  #ifdef _WIN64
  L"64";
  #else
  L"32";
  #endif

static bool ReadPathFromRegistry(HKEY baseKey, LPCWSTR value, FString &path)
{
  NRegistry::CKey key;
  if (key.Open(baseKey, kRegistryPath, KEY_READ) == ERROR_SUCCESS)
  {
    UString pathU;
    if (key.QueryValue(value, pathU) == ERROR_SUCCESS)
    {
      path = us2fs(pathU);
      NName::NormalizeDirPathPrefix(path);
      return NFind::DoesFileExist(path + kMainDll);
    }
  }
  return false;
}

#endif

static FString GetBaseFolderPrefixFromRegistry()
{
  FString moduleFolderPrefix = NDLL::GetModuleDirPrefix();
  #ifdef _WIN32
  if (!NFind::DoesFileExist(moduleFolderPrefix + kMainDll) &&
      !NFind::DoesDirExist(moduleFolderPrefix + kCodecsFolderName) &&
      !NFind::DoesDirExist(moduleFolderPrefix + kFormatsFolderName))
  {
    FString path;
    if (ReadPathFromRegistry(HKEY_CURRENT_USER,  kProgramPath2Value, path)) return path;
    if (ReadPathFromRegistry(HKEY_LOCAL_MACHINE, kProgramPath2Value, path)) return path;
    if (ReadPathFromRegistry(HKEY_CURRENT_USER,  kProgramPathValue,  path)) return path;
    if (ReadPathFromRegistry(HKEY_LOCAL_MACHINE, kProgramPathValue,  path)) return path;
  }
  #endif
  return moduleFolderPrefix;
}

typedef UInt32 (WINAPI *GetNumberOfMethodsFunc)(UInt32 *numMethods);
typedef UInt32 (WINAPI *GetNumberOfFormatsFunc)(UInt32 *numFormats);
typedef UInt32 (WINAPI *GetHandlerPropertyFunc)(PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *GetHandlerPropertyFunc2)(UInt32 index, PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *CreateObjectFunc)(const GUID *clsID, const GUID *iid, void **outObject);
typedef UInt32 (WINAPI *SetLargePageModeFunc)();


static HRESULT GetCoderClass(GetMethodPropertyFunc getMethodProperty, UInt32 index,
    PROPID propId, CLSID &clsId, bool &isAssigned)
{
  NWindows::NCOM::CPropVariant prop;
  isAssigned = false;
  RINOK(getMethodProperty(index, propId, &prop));
  if (prop.vt == VT_BSTR)
  {
    isAssigned = true;
    clsId = *(const GUID *)prop.bstrVal;
  }
  else if (prop.vt != VT_EMPTY)
    return E_FAIL;
  return S_OK;
}

HRESULT CCodecs::LoadCodecs()
{
  CCodecLib &lib = Libs.Back();
  lib.GetMethodProperty = (GetMethodPropertyFunc)lib.Lib.GetProc("GetMethodProperty");
  if (lib.GetMethodProperty == NULL)
    return S_OK;

  UInt32 numMethods = 1;
  GetNumberOfMethodsFunc getNumberOfMethodsFunc = (GetNumberOfMethodsFunc)lib.Lib.GetProc("GetNumberOfMethods");
  if (getNumberOfMethodsFunc != NULL)
  {
    RINOK(getNumberOfMethodsFunc(&numMethods));
  }

  for(UInt32 i = 0; i < numMethods; i++)
  {
    CDllCodecInfo info;
    info.LibIndex = Libs.Size() - 1;
    info.CodecIndex = i;

    RINOK(GetCoderClass(lib.GetMethodProperty, i, NMethodPropID::kEncoder, info.Encoder, info.EncoderIsAssigned));
    RINOK(GetCoderClass(lib.GetMethodProperty, i, NMethodPropID::kDecoder, info.Decoder, info.DecoderIsAssigned));

    Codecs.Add(info);
  }
  return S_OK;
}

static HRESULT ReadProp(
    GetHandlerPropertyFunc getProp,
    GetHandlerPropertyFunc2 getProp2,
    UInt32 index, PROPID propID, NCOM::CPropVariant &prop)
{
  if (getProp2)
    return getProp2(index, propID, &prop);;
  return getProp(propID, &prop);
}

static HRESULT ReadBoolProp(
    GetHandlerPropertyFunc getProp,
    GetHandlerPropertyFunc2 getProp2,
    UInt32 index, PROPID propID, bool &res)
{
  NCOM::CPropVariant prop;
  RINOK(ReadProp(getProp, getProp2, index, propID, prop));
  if (prop.vt == VT_BOOL)
    res = VARIANT_BOOLToBool(prop.boolVal);
  else if (prop.vt != VT_EMPTY)
    return E_FAIL;
  return S_OK;
}

static HRESULT ReadStringProp(
    GetHandlerPropertyFunc getProp,
    GetHandlerPropertyFunc2 getProp2,
    UInt32 index, PROPID propID, UString &res)
{
  NCOM::CPropVariant prop;
  RINOK(ReadProp(getProp, getProp2, index, propID, prop));
  if (prop.vt == VT_BSTR)
    res = prop.bstrVal;
  else if (prop.vt != VT_EMPTY)
    return E_FAIL;
  return S_OK;
}

#endif

static const unsigned int kNumArcsMax = 48;
static unsigned int g_NumArcs = 0;
static const CArcInfo *g_Arcs[kNumArcsMax];
void RegisterArc(const CArcInfo *arcInfo)
{
  if (g_NumArcs < kNumArcsMax)
    g_Arcs[g_NumArcs++] = arcInfo;
}

static void SplitString(const UString &srcString, UStringVector &destStrings)
{
  destStrings.Clear();
  UString s;
  int len = srcString.Length();
  if (len == 0)
    return;
  for (int i = 0; i < len; i++)
  {
    wchar_t c = srcString[i];
    if (c == L' ')
    {
      if (!s.IsEmpty())
      {
        destStrings.Add(s);
        s.Empty();
      }
    }
    else
      s += c;
  }
  if (!s.IsEmpty())
    destStrings.Add(s);
}

int CArcInfoEx::FindExtension(const UString &ext) const
{
  for (int i = 0; i < Exts.Size(); i++)
    if (ext.CompareNoCase(Exts[i].Ext) == 0)
      return i;
  return -1;
}

void CArcInfoEx::AddExts(const wchar_t *ext, const wchar_t *addExt)
{
  UStringVector exts, addExts;
  if (ext != 0)
    SplitString(ext, exts);
  if (addExt != 0)
    SplitString(addExt, addExts);
  for (int i = 0; i < exts.Size(); i++)
  {
    CArcExtInfo extInfo;
    extInfo.Ext = exts[i];
    if (i < addExts.Size())
    {
      extInfo.AddExt = addExts[i];
      if (extInfo.AddExt == L"*")
        extInfo.AddExt.Empty();
    }
    Exts.Add(extInfo);
  }
}

#ifdef EXTERNAL_CODECS

HRESULT CCodecs::LoadFormats()
{
  const NDLL::CLibrary &lib = Libs.Back().Lib;
  GetHandlerPropertyFunc getProp = 0;
  GetHandlerPropertyFunc2 getProp2 = (GetHandlerPropertyFunc2)lib.GetProc("GetHandlerProperty2");
  if (getProp2 == NULL)
  {
    getProp = (GetHandlerPropertyFunc)lib.GetProc("GetHandlerProperty");
    if (getProp == NULL)
      return S_OK;
  }

  UInt32 numFormats = 1;
  GetNumberOfFormatsFunc getNumberOfFormats = (GetNumberOfFormatsFunc)lib.GetProc("GetNumberOfFormats");
  if (getNumberOfFormats != NULL)
  {
    RINOK(getNumberOfFormats(&numFormats));
  }
  if (getProp2 == NULL)
    numFormats = 1;

  for(UInt32 i = 0; i < numFormats; i++)
  {
    CArcInfoEx item;
    item.LibIndex = Libs.Size() - 1;
    item.FormatIndex = i;

    RINOK(ReadStringProp(getProp, getProp2, i, NArchive::kName, item.Name));

    NCOM::CPropVariant prop;
    if (ReadProp(getProp, getProp2, i, NArchive::kClassID, prop) != S_OK)
      continue;
    if (prop.vt != VT_BSTR)
      continue;
    item.ClassID = *(const GUID *)prop.bstrVal;
    prop.Clear();

    UString ext, addExt;
    RINOK(ReadStringProp(getProp, getProp2, i, NArchive::kExtension, ext));
    RINOK(ReadStringProp(getProp, getProp2, i, NArchive::kAddExtension, addExt));
    item.AddExts(ext, addExt);

    ReadBoolProp(getProp, getProp2, i, NArchive::kUpdate, item.UpdateEnabled);
    if (item.UpdateEnabled)
      ReadBoolProp(getProp, getProp2, i, NArchive::kKeepName, item.KeepName);
    
    if (ReadProp(getProp, getProp2, i, NArchive::kStartSignature, prop) == S_OK)
      if (prop.vt == VT_BSTR)
      {
        UINT len = ::SysStringByteLen(prop.bstrVal);
        item.StartSignature.SetCapacity(len);
        memmove(item.StartSignature, prop.bstrVal, len);
      }
    Formats.Add(item);
  }
  return S_OK;
}

#ifdef NEW_FOLDER_INTERFACE
void CCodecIcons::LoadIcons(HMODULE m)
{
  UString iconTypes = MyLoadStringW(m, kIconTypesResId);
  UStringVector pairs;
  SplitString(iconTypes, pairs);
  for (int i = 0; i < pairs.Size(); i++)
  {
    const UString &s = pairs[i];
    int pos = s.Find(L':');
    CIconPair iconPair;
    iconPair.IconIndex = -1;
    if (pos < 0)
      pos = s.Length();
    else
    {
      UString num = s.Mid(pos + 1);
      if (!num.IsEmpty())
      {
        const wchar_t *end;
        iconPair.IconIndex = (UInt32)ConvertStringToUInt64(num, &end);
        if (*end != L'\0')
          continue;
      }
    }
    iconPair.Ext = s.Left(pos);
    IconPairs.Add(iconPair);
  }
}

bool CCodecIcons::FindIconIndex(const UString &ext, int &iconIndex) const
{
  iconIndex = -1;
  for (int i = 0; i < IconPairs.Size(); i++)
  {
    const CIconPair &pair = IconPairs[i];
    if (ext.CompareNoCase(pair.Ext) == 0)
    {
      iconIndex = pair.IconIndex;
      return true;
    }
  }
  return false;
}
#endif

#ifdef _7ZIP_LARGE_PAGES
extern "C"
{
  extern SIZE_T g_LargePageSize;
}
#endif

HRESULT CCodecs::LoadDll(const FString &dllPath, bool needCheckDll)
{
  if (needCheckDll)
  {
    NDLL::CLibrary library;
    if (!library.LoadEx(dllPath, LOAD_LIBRARY_AS_DATAFILE))
      return S_OK;
  }
  Libs.Add(CCodecLib());
  CCodecLib &lib = Libs.Back();
  #ifdef NEW_FOLDER_INTERFACE
  lib.Path = dllPath;
  #endif
  bool used = false;
  HRESULT res = S_OK;
  if (lib.Lib.Load(dllPath))
  {
    #ifdef NEW_FOLDER_INTERFACE
    lib.LoadIcons();
    #endif

    #ifdef _7ZIP_LARGE_PAGES
    if (g_LargePageSize != 0)
    {
      SetLargePageModeFunc setLargePageMode = (SetLargePageModeFunc)lib.Lib.GetProc("SetLargePageMode");
      if (setLargePageMode != 0)
        setLargePageMode();
    }
    #endif

    lib.CreateObject = (CreateObjectFunc)lib.Lib.GetProc("CreateObject");
    if (lib.CreateObject != 0)
    {
      int startSize = Codecs.Size();
      res = LoadCodecs();
      used = (Codecs.Size() != startSize);
      if (res == S_OK)
      {
        startSize = Formats.Size();
        res = LoadFormats();
        used = used || (Formats.Size() != startSize);
      }
    }
  }
  if (!used)
    Libs.DeleteBack();
  return res;
}

HRESULT CCodecs::LoadDllsFromFolder(const FString &folderPrefix)
{
  NFile::NFind::CEnumerator enumerator(folderPrefix + FCHAR_ANY_MASK);
  NFile::NFind::CFileInfo fi;
  while (enumerator.Next(fi))
  {
    if (fi.IsDir())
      continue;
    RINOK(LoadDll(folderPrefix + fi.Name, true));
  }
  return S_OK;
}

#endif

#ifndef _SFX
static inline void SetBuffer(CByteBuffer &bb, const Byte *data, int size)
{
  bb.SetCapacity(size);
  memmove((Byte *)bb, data, size);
}
#endif

HRESULT CCodecs::Load()
{
  #ifdef NEW_FOLDER_INTERFACE
  InternalIcons.LoadIcons(g_hInstance);
  #endif

  Formats.Clear();
  #ifdef EXTERNAL_CODECS
  Codecs.Clear();
  #endif
  for (UInt32 i = 0; i < g_NumArcs; i++)
  {
    const CArcInfo &arc = *g_Arcs[i];
    CArcInfoEx item;
    item.Name = arc.Name;
    item.CreateInArchive = arc.CreateInArchive;
    item.CreateOutArchive = arc.CreateOutArchive;
    item.AddExts(arc.Ext, arc.AddExt);
    item.UpdateEnabled = (arc.CreateOutArchive != 0);
    item.KeepName = arc.KeepName;

    #ifndef _SFX
    SetBuffer(item.StartSignature, arc.Signature, arc.SignatureSize);
    #endif
    Formats.Add(item);
  }
  #ifdef EXTERNAL_CODECS
  const FString baseFolder = GetBaseFolderPrefixFromRegistry();
  RINOK(LoadDll(baseFolder + kMainDll, false));
  RINOK(LoadDllsFromFolder(baseFolder + kCodecsFolderName FSTRING_PATH_SEPARATOR));
  RINOK(LoadDllsFromFolder(baseFolder + kFormatsFolderName FSTRING_PATH_SEPARATOR));
  #endif
  return S_OK;
}

#ifndef _SFX

int CCodecs::FindFormatForArchiveName(const UString &arcPath) const
{
  int slashPos = arcPath.ReverseFind(WCHAR_PATH_SEPARATOR);
  int dotPos = arcPath.ReverseFind(L'.');
  if (dotPos < 0 || dotPos < slashPos)
    return -1;
  const UString ext = arcPath.Mid(dotPos + 1);
  for (int i = 0; i < Formats.Size(); i++)
  {
    const CArcInfoEx &arc = Formats[i];
    if (!arc.UpdateEnabled)
      continue;
    if (arc.FindExtension(ext) >= 0)
      return i;
  }
  return -1;
}

int CCodecs::FindFormatForExtension(const UString &ext) const
{
  if (ext.IsEmpty())
    return -1;
  for (int i = 0; i < Formats.Size(); i++)
    if (Formats[i].FindExtension(ext) >= 0)
      return i;
  return -1;
}

int CCodecs::FindFormatForArchiveType(const UString &arcType) const
{
  for (int i = 0; i < Formats.Size(); i++)
    if (Formats[i].Name.CompareNoCase(arcType) == 0)
      return i;
  return -1;
}

bool CCodecs::FindFormatForArchiveType(const UString &arcType, CIntVector &formatIndices) const
{
  formatIndices.Clear();
  for (int pos = 0; pos < arcType.Length();)
  {
    int pos2 = arcType.Find('.', pos);
    if (pos2 < 0)
      pos2 = arcType.Length();
    const UString name = arcType.Mid(pos, pos2 - pos);
    int index = FindFormatForArchiveType(name);
    if (index < 0 && name != L"*")
    {
      formatIndices.Clear();
      return false;
    }
    formatIndices.Add(index);
    pos = pos2 + 1;
  }
  return true;
}

#endif

#ifdef EXTERNAL_CODECS

#ifdef EXPORT_CODECS
extern unsigned int g_NumCodecs;
STDAPI CreateCoder2(bool encode, UInt32 index, const GUID *iid, void **outObject);
STDAPI GetMethodProperty(UInt32 codecIndex, PROPID propID, PROPVARIANT *value);
// STDAPI GetNumberOfMethods(UInt32 *numCodecs);
#endif

STDMETHODIMP CCodecs::GetNumberOfMethods(UInt32 *numMethods)
{
  *numMethods =
      #ifdef EXPORT_CODECS
      g_NumCodecs +
      #endif
      Codecs.Size();
  return S_OK;
}

STDMETHODIMP CCodecs::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
  #ifdef EXPORT_CODECS
  if (index < g_NumCodecs)
    return GetMethodProperty(index, propID, value);
  #endif

  const CDllCodecInfo &ci = Codecs[index
      #ifdef EXPORT_CODECS
      - g_NumCodecs
      #endif
      ];

  if (propID == NMethodPropID::kDecoderIsAssigned)
  {
    NWindows::NCOM::CPropVariant propVariant;
    propVariant = ci.DecoderIsAssigned;
    propVariant.Detach(value);
    return S_OK;
  }
  if (propID == NMethodPropID::kEncoderIsAssigned)
  {
    NWindows::NCOM::CPropVariant propVariant;
    propVariant = ci.EncoderIsAssigned;
    propVariant.Detach(value);
    return S_OK;
  }
  return Libs[ci.LibIndex].GetMethodProperty(ci.CodecIndex, propID, value);
}

STDMETHODIMP CCodecs::CreateDecoder(UInt32 index, const GUID *iid, void **coder)
{
  #ifdef EXPORT_CODECS
  if (index < g_NumCodecs)
    return CreateCoder2(false, index, iid, coder);
  #endif
  const CDllCodecInfo &ci = Codecs[index
      #ifdef EXPORT_CODECS
      - g_NumCodecs
      #endif
      ];
  if (ci.DecoderIsAssigned)
    return Libs[ci.LibIndex].CreateObject(&ci.Decoder, iid, (void **)coder);
  return S_OK;
}

STDMETHODIMP CCodecs::CreateEncoder(UInt32 index, const GUID *iid, void **coder)
{
  #ifdef EXPORT_CODECS
  if (index < g_NumCodecs)
    return CreateCoder2(true, index, iid, coder);
  #endif
  const CDllCodecInfo &ci = Codecs[index
      #ifdef EXPORT_CODECS
      - g_NumCodecs
      #endif
      ];
  if (ci.EncoderIsAssigned)
    return Libs[ci.LibIndex].CreateObject(&ci.Encoder, iid, (void **)coder);
  return S_OK;
}

HRESULT CCodecs::CreateCoder(const UString &name, bool encode, CMyComPtr<ICompressCoder> &coder) const
{
  for (int i = 0; i < Codecs.Size(); i++)
  {
    const CDllCodecInfo &codec = Codecs[i];
    if (encode && !codec.EncoderIsAssigned || !encode && !codec.DecoderIsAssigned)
      continue;
    const CCodecLib &lib = Libs[codec.LibIndex];
    UString res;
    NWindows::NCOM::CPropVariant prop;
    RINOK(lib.GetMethodProperty(codec.CodecIndex, NMethodPropID::kName, &prop));
    if (prop.vt == VT_BSTR)
      res = prop.bstrVal;
    else if (prop.vt != VT_EMPTY)
      continue;
    if (name.CompareNoCase(res) == 0)
      return lib.CreateObject(encode ? &codec.Encoder : &codec.Decoder, &IID_ICompressCoder, (void **)&coder);
  }
  return CLASS_E_CLASSNOTAVAILABLE;
}

int CCodecs::GetCodecLibIndex(UInt32 index)
{
  #ifdef EXPORT_CODECS
  if (index < g_NumCodecs)
    return -1;
  #endif
  #ifdef EXTERNAL_CODECS
  const CDllCodecInfo &ci = Codecs[index
      #ifdef EXPORT_CODECS
      - g_NumCodecs
      #endif
      ];
  return ci.LibIndex;
  #else
  return -1;
  #endif
}

bool CCodecs::GetCodecEncoderIsAssigned(UInt32 index)
{
  #ifdef EXPORT_CODECS
  if (index < g_NumCodecs)
  {
    NWindows::NCOM::CPropVariant prop;
    if (GetProperty(index, NMethodPropID::kEncoder, &prop) == S_OK)
      if (prop.vt != VT_EMPTY)
        return true;
    return false;
  }
  #endif
  #ifdef EXTERNAL_CODECS
  const CDllCodecInfo &ci = Codecs[index
      #ifdef EXPORT_CODECS
      - g_NumCodecs
      #endif
      ];
  return ci.EncoderIsAssigned;
  #else
  return false;
  #endif
}

HRESULT CCodecs::GetCodecId(UInt32 index, UInt64 &id)
{
  UString s;
  NWindows::NCOM::CPropVariant prop;
  RINOK(GetProperty(index, NMethodPropID::kID, &prop));
  if (prop.vt != VT_UI8)
    return E_INVALIDARG;
  id = prop.uhVal.QuadPart;
  return S_OK;
}

UString CCodecs::GetCodecName(UInt32 index)
{
  UString s;
  NWindows::NCOM::CPropVariant prop;
  if (GetProperty(index, NMethodPropID::kName, &prop) == S_OK)
    if (prop.vt == VT_BSTR)
      s = prop.bstrVal;
  return s;
}

#endif
