// 7zHandlerOut.cpp

#include "StdAfx.h"

#include "../../../Common/ComTry.h"
#include "../../../Common/StringToInt.h"

#include "../Common/ItemNameUtils.h"
#include "../Common/ParseProperties.h"

#include "7zHandler.h"
#include "7zOut.h"
#include "7zUpdate.h"

using namespace NWindows;

namespace NArchive {
namespace N7z {

static const wchar_t *k_LZMA_Name = L"LZMA";
static const wchar_t *kDefaultMethodName = k_LZMA_Name;
static const wchar_t *k_Copy_Name = L"Copy";

static const wchar_t *k_MatchFinder_ForHeaders = L"BT2";
static const UInt32 k_NumFastBytes_ForHeaders = 273;
static const UInt32 k_Level_ForHeaders = 5;
static const UInt32 k_Dictionary_ForHeaders =
  #ifdef UNDER_CE
  1 << 18;
  #else
  1 << 20;
  #endif

STDMETHODIMP CHandler::GetFileTimeType(UInt32 *type)
{
  *type = NFileTimeType::kWindows;
  return S_OK;
}

HRESULT CHandler::PropsMethod_To_FullMethod(CMethodFull &dest, const COneMethodInfo &m)
{
  if (!FindMethod(
      EXTERNAL_CODECS_VARS
      m.MethodName, dest.Id, dest.NumInStreams, dest.NumOutStreams))
    return E_INVALIDARG;
  (CProps &)dest = (CProps &)m;
  return S_OK;
}

HRESULT CHandler::SetHeaderMethod(CCompressionMethodMode &headerMethod)
{
  if (!_compressHeaders)
    return S_OK;
  COneMethodInfo m;
  m.MethodName = k_LZMA_Name;
  m.AddPropString(NCoderPropID::kMatchFinder, k_MatchFinder_ForHeaders);
  m.AddProp32(NCoderPropID::kLevel, k_Level_ForHeaders);
  m.AddProp32(NCoderPropID::kNumFastBytes, k_NumFastBytes_ForHeaders);
  m.AddProp32(NCoderPropID::kDictionarySize, k_Dictionary_ForHeaders);
  m.AddNumThreadsProp(1);

  CMethodFull methodFull;
  RINOK(PropsMethod_To_FullMethod(methodFull, m));
  headerMethod.Methods.Add(methodFull);
  return S_OK;
}

void CHandler::AddDefaultMethod()
{
  for (int i = 0; i < _methods.Size(); i++)
  {
    UString &methodName = _methods[0].MethodName;
    if (methodName.IsEmpty())
      methodName = kDefaultMethodName;
  }
  if (_methods.IsEmpty())
  {
    COneMethodInfo m;
    m.MethodName = (GetLevel() == 0 ? k_Copy_Name : kDefaultMethodName);
    _methods.Add(m);
  }
}

HRESULT CHandler::SetMainMethod(
    CCompressionMethodMode &methodMode,
    CObjectVector<COneMethodInfo> &methods
    #ifndef _7ZIP_ST
    , UInt32 numThreads
    #endif
    )
{
  AddDefaultMethod();

  const UInt64 kSolidBytes_Min = (1 << 24);
  const UInt64 kSolidBytes_Max = ((UInt64)1 << 32) - 1;

  bool needSolid = false;
  for (int i = 0; i < methods.Size(); i++)
  {
    COneMethodInfo &oneMethodInfo = methods[i];
    SetGlobalLevelAndThreads(oneMethodInfo
      #ifndef _7ZIP_ST
      , numThreads
      #endif
      );

    CMethodFull methodFull;
    RINOK(PropsMethod_To_FullMethod(methodFull, oneMethodInfo));
    methodMode.Methods.Add(methodFull);

    if (methodFull.Id != k_Copy)
      needSolid = true;

    if (_numSolidBytesDefined)
      continue;

    UInt32 dicSize;
    switch (methodFull.Id)
    {
      case k_LZMA:
      case k_LZMA2: dicSize = oneMethodInfo.Get_Lzma_DicSize(); break;
      case k_PPMD: dicSize = oneMethodInfo.Get_Ppmd_MemSize(); break;
      case k_Deflate: dicSize = (UInt32)1 << 15; break;
      case k_BZip2: dicSize = oneMethodInfo.Get_BZip2_BlockSize(); break;
      default: continue;
    }
    _numSolidBytes = (UInt64)dicSize << 7;
    if (_numSolidBytes < kSolidBytes_Min) _numSolidBytes = kSolidBytes_Min;
    if (_numSolidBytes > kSolidBytes_Max) _numSolidBytes = kSolidBytes_Max;
    _numSolidBytesDefined = true;
  }

  if (!_numSolidBytesDefined)
    if (needSolid)
      _numSolidBytes = kSolidBytes_Max;
    else
      _numSolidBytes = 0;
  _numSolidBytesDefined = true;
  return S_OK;
}

static HRESULT GetTime(IArchiveUpdateCallback *updateCallback, int index, bool writeTime, PROPID propID, UInt64 &ft, bool &ftDefined)
{
  ft = 0;
  ftDefined = false;
  if (!writeTime)
    return S_OK;
  NCOM::CPropVariant prop;
  RINOK(updateCallback->GetProperty(index, propID, &prop));
  if (prop.vt == VT_FILETIME)
  {
    ft = prop.filetime.dwLowDateTime | ((UInt64)prop.filetime.dwHighDateTime << 32);
    ftDefined = true;
  }
  else if (prop.vt != VT_EMPTY)
    return E_INVALIDARG;
  return S_OK;
}

STDMETHODIMP CHandler::UpdateItems(ISequentialOutStream *outStream, UInt32 numItems,
    IArchiveUpdateCallback *updateCallback)
{
  COM_TRY_BEGIN

  const CArchiveDatabaseEx *db = 0;
  #ifdef _7Z_VOL
  if (_volumes.Size() > 1)
    return E_FAIL;
  const CVolume *volume = 0;
  if (_volumes.Size() == 1)
  {
    volume = &_volumes.Front();
    db = &volume->Database;
  }
  #else
  if (_inStream != 0)
    db = &_db;
  #endif

  CObjectVector<CUpdateItem> updateItems;
  
  for (UInt32 i = 0; i < numItems; i++)
  {
    Int32 newData, newProps;
    UInt32 indexInArchive;
    if (!updateCallback)
      return E_FAIL;
    RINOK(updateCallback->GetUpdateItemInfo(i, &newData, &newProps, &indexInArchive));
    CUpdateItem ui;
    ui.NewProps = IntToBool(newProps);
    ui.NewData = IntToBool(newData);
    ui.IndexInArchive = indexInArchive;
    ui.IndexInClient = i;
    ui.IsAnti = false;
    ui.Size = 0;

    if (ui.IndexInArchive != -1)
    {
      if (db == 0 || ui.IndexInArchive >= db->Files.Size())
        return E_INVALIDARG;
      const CFileItem &fi = db->Files[ui.IndexInArchive];
      ui.Name = fi.Name;
      ui.IsDir = fi.IsDir;
      ui.Size = fi.Size;
      ui.IsAnti = db->IsItemAnti(ui.IndexInArchive);
      
      ui.CTimeDefined = db->CTime.GetItem(ui.IndexInArchive, ui.CTime);
      ui.ATimeDefined = db->ATime.GetItem(ui.IndexInArchive, ui.ATime);
      ui.MTimeDefined = db->MTime.GetItem(ui.IndexInArchive, ui.MTime);
    }

    if (ui.NewProps)
    {
      bool nameIsDefined;
      bool folderStatusIsDefined;
      {
        NCOM::CPropVariant prop;
        RINOK(updateCallback->GetProperty(i, kpidAttrib, &prop));
        if (prop.vt == VT_EMPTY)
          ui.AttribDefined = false;
        else if (prop.vt != VT_UI4)
          return E_INVALIDARG;
        else
        {
          ui.Attrib = prop.ulVal;
          ui.AttribDefined = true;
        }
      }
      
      // we need MTime to sort files.
      RINOK(GetTime(updateCallback, i, WriteCTime, kpidCTime, ui.CTime, ui.CTimeDefined));
      RINOK(GetTime(updateCallback, i, WriteATime, kpidATime, ui.ATime, ui.ATimeDefined));
      RINOK(GetTime(updateCallback, i, true,       kpidMTime, ui.MTime, ui.MTimeDefined));

      {
        NCOM::CPropVariant prop;
        RINOK(updateCallback->GetProperty(i, kpidPath, &prop));
        if (prop.vt == VT_EMPTY)
          nameIsDefined = false;
        else if (prop.vt != VT_BSTR)
          return E_INVALIDARG;
        else
        {
          ui.Name = NItemName::MakeLegalName(prop.bstrVal);
          nameIsDefined = true;
        }
      }
      {
        NCOM::CPropVariant prop;
        RINOK(updateCallback->GetProperty(i, kpidIsDir, &prop));
        if (prop.vt == VT_EMPTY)
          folderStatusIsDefined = false;
        else if (prop.vt != VT_BOOL)
          return E_INVALIDARG;
        else
        {
          ui.IsDir = (prop.boolVal != VARIANT_FALSE);
          folderStatusIsDefined = true;
        }
      }

      {
        NCOM::CPropVariant prop;
        RINOK(updateCallback->GetProperty(i, kpidIsAnti, &prop));
        if (prop.vt == VT_EMPTY)
          ui.IsAnti = false;
        else if (prop.vt != VT_BOOL)
          return E_INVALIDARG;
        else
          ui.IsAnti = (prop.boolVal != VARIANT_FALSE);
      }

      if (ui.IsAnti)
      {
        ui.AttribDefined = false;

        ui.CTimeDefined = false;
        ui.ATimeDefined = false;
        ui.MTimeDefined = false;
        
        ui.Size = 0;
      }

      if (!folderStatusIsDefined && ui.AttribDefined)
        ui.SetDirStatusFromAttrib();
    }

    if (ui.NewData)
    {
      NCOM::CPropVariant prop;
      RINOK(updateCallback->GetProperty(i, kpidSize, &prop));
      if (prop.vt != VT_UI8)
        return E_INVALIDARG;
      ui.Size = (UInt64)prop.uhVal.QuadPart;
      if (ui.Size != 0 && ui.IsAnti)
        return E_INVALIDARG;
    }
    updateItems.Add(ui);
  }

  CCompressionMethodMode methodMode, headerMethod;

  HRESULT res = SetMainMethod(methodMode, _methods
    #ifndef _7ZIP_ST
    , _numThreads
    #endif
    );
  RINOK(res);
  methodMode.Binds = _binds;

  RINOK(SetHeaderMethod(headerMethod));
  #ifndef _7ZIP_ST
  methodMode.NumThreads = _numThreads;
  headerMethod.NumThreads = 1;
  #endif

  CMyComPtr<ICryptoGetTextPassword2> getPassword2;
  updateCallback->QueryInterface(IID_ICryptoGetTextPassword2, (void **)&getPassword2);

  if (getPassword2)
  {
    CMyComBSTR password;
    Int32 passwordIsDefined;
    RINOK(getPassword2->CryptoGetTextPassword2(&passwordIsDefined, &password));
    methodMode.PasswordIsDefined = IntToBool(passwordIsDefined);
    if (methodMode.PasswordIsDefined)
      methodMode.Password = password;
  }
  else
    methodMode.PasswordIsDefined = false;

  bool compressMainHeader = _compressHeaders;  // check it

  bool encryptHeaders = false;

  if (methodMode.PasswordIsDefined)
  {
    if (_encryptHeadersSpecified)
      encryptHeaders = _encryptHeaders;
    #ifndef _NO_CRYPTO
    else
      encryptHeaders = _passwordIsDefined;
    #endif
    compressMainHeader = true;
    if (encryptHeaders)
    {
      headerMethod.PasswordIsDefined = methodMode.PasswordIsDefined;
      headerMethod.Password = methodMode.Password;
    }
  }

  if (numItems < 2)
    compressMainHeader = false;

  CUpdateOptions options;
  options.Method = &methodMode;
  options.HeaderMethod = (_compressHeaders || encryptHeaders) ? &headerMethod : 0;
  int level = GetLevel();
  options.UseFilters = level != 0 && _autoFilter;
  options.MaxFilter = level >= 8;

  options.HeaderOptions.CompressMainHeader = compressMainHeader;
  options.HeaderOptions.WriteCTime = WriteCTime;
  options.HeaderOptions.WriteATime = WriteATime;
  options.HeaderOptions.WriteMTime = WriteMTime;
  
  options.NumSolidFiles = _numSolidFiles;
  options.NumSolidBytes = _numSolidBytes;
  options.SolidExtension = _solidExtension;
  options.RemoveSfxBlock = _removeSfxBlock;
  options.VolumeMode = _volumeMode;

  COutArchive archive;
  CArchiveDatabase newDatabase;

  CMyComPtr<ICryptoGetTextPassword> getPassword;
  updateCallback->QueryInterface(IID_ICryptoGetTextPassword, (void **)&getPassword);
  
  res = Update(
      EXTERNAL_CODECS_VARS
      #ifdef _7Z_VOL
      volume ? volume->Stream: 0,
      volume ? db : 0,
      #else
      _inStream,
      db,
      #endif
      updateItems,
      archive, newDatabase, outStream, updateCallback, options
      #ifndef _NO_CRYPTO
      , getPassword
      #endif
      );

  RINOK(res);

  updateItems.ClearAndFree();

  return archive.WriteDatabase(EXTERNAL_CODECS_VARS
      newDatabase, options.HeaderMethod, options.HeaderOptions);

  COM_TRY_END
}

static HRESULT GetBindInfoPart(UString &srcString, UInt32 &coder, UInt32 &stream)
{
  stream = 0;
  int index = ParseStringToUInt32(srcString, coder);
  if (index == 0)
    return E_INVALIDARG;
  srcString.Delete(0, index);
  if (srcString[0] == 'S')
  {
    srcString.Delete(0);
    int index = ParseStringToUInt32(srcString, stream);
    if (index == 0)
      return E_INVALIDARG;
    srcString.Delete(0, index);
  }
  return S_OK;
}

void COutHandler::InitProps()
{
  CMultiMethodProps::Init();

  _removeSfxBlock = false;
  _compressHeaders = true;
  _encryptHeadersSpecified = false;
  _encryptHeaders = false;
  
  WriteCTime = false;
  WriteATime = false;
  WriteMTime = true;
  
  _volumeMode = false;
  InitSolid();
}

HRESULT COutHandler::SetSolidFromString(const UString &s)
{
  UString s2 = s;
  s2.MakeUpper();
  for (int i = 0; i < s2.Length();)
  {
    const wchar_t *start = ((const wchar_t *)s2) + i;
    const wchar_t *end;
    UInt64 v = ConvertStringToUInt64(start, &end);
    if (start == end)
    {
      if (s2[i++] != 'E')
        return E_INVALIDARG;
      _solidExtension = true;
      continue;
    }
    i += (int)(end - start);
    if (i == s2.Length())
      return E_INVALIDARG;
    wchar_t c = s2[i++];
    if (c == 'F')
    {
      if (v < 1)
        v = 1;
      _numSolidFiles = v;
    }
    else
    {
      unsigned numBits;
      switch (c)
      {
        case 'B': numBits =  0; break;
        case 'K': numBits = 10; break;
        case 'M': numBits = 20; break;
        case 'G': numBits = 30; break;
        default: return E_INVALIDARG;
      }
      _numSolidBytes = (v << numBits);
      _numSolidBytesDefined = true;
    }
  }
  return S_OK;
}

HRESULT COutHandler::SetSolidFromPROPVARIANT(const PROPVARIANT &value)
{
  bool isSolid;
  switch (value.vt)
  {
    case VT_EMPTY: isSolid = true; break;
    case VT_BOOL: isSolid = (value.boolVal != VARIANT_FALSE); break;
    case VT_BSTR:
      if (StringToBool(value.bstrVal, isSolid))
        break;
      return SetSolidFromString(value.bstrVal);
    default: return E_INVALIDARG;
  }
  if (isSolid)
    InitSolid();
  else
    _numSolidFiles = 1;
  return S_OK;
}

HRESULT COutHandler::SetProperty(const wchar_t *nameSpec, const PROPVARIANT &value)
{
  UString name = nameSpec;
  name.MakeUpper();
  if (name.IsEmpty())
    return E_INVALIDARG;
  
  if (name[0] == L'S')
  {
    name.Delete(0);
    if (name.IsEmpty())
      return SetSolidFromPROPVARIANT(value);
    if (value.vt != VT_EMPTY)
      return E_INVALIDARG;
    return SetSolidFromString(name);
  }
  
  UInt32 number;
  int index = ParseStringToUInt32(name, number);
  UString realName = name.Mid(index);
  if (index == 0)
  {
    if (name.CompareNoCase(L"RSFX") == 0) return PROPVARIANT_to_bool(value, _removeSfxBlock);
    if (name.CompareNoCase(L"HC") == 0) return PROPVARIANT_to_bool(value, _compressHeaders);
    if (name.CompareNoCase(L"HCF") == 0)
    {
      bool compressHeadersFull = true;
      RINOK(PROPVARIANT_to_bool(value, compressHeadersFull));
      return compressHeadersFull ? S_OK: E_INVALIDARG;
    }
    if (name.CompareNoCase(L"HE") == 0)
    {
      RINOK(PROPVARIANT_to_bool(value, _encryptHeaders));
      _encryptHeadersSpecified = true;
      return S_OK;
    }
    if (name.CompareNoCase(L"TC") == 0) return PROPVARIANT_to_bool(value, WriteCTime);
    if (name.CompareNoCase(L"TA") == 0) return PROPVARIANT_to_bool(value, WriteATime);
    if (name.CompareNoCase(L"TM") == 0) return PROPVARIANT_to_bool(value, WriteMTime);
    if (name.CompareNoCase(L"V") == 0) return PROPVARIANT_to_bool(value, _volumeMode);
  }
  return CMultiMethodProps::SetProperty(name, value);
}

STDMETHODIMP CHandler::SetProperties(const wchar_t **names, const PROPVARIANT *values, Int32 numProps)
{
  COM_TRY_BEGIN
  _binds.Clear();
  InitProps();

  for (int i = 0; i < numProps; i++)
  {
    UString name = names[i];
    name.MakeUpper();
    if (name.IsEmpty())
      return E_INVALIDARG;

    const PROPVARIANT &value = values[i];

    if (name[0] == 'B')
    {
      if (value.vt != VT_EMPTY)
        return E_INVALIDARG;
      name.Delete(0);
      CBind bind;
      RINOK(GetBindInfoPart(name, bind.OutCoder, bind.OutStream));
      if (name[0] != ':')
        return E_INVALIDARG;
      name.Delete(0);
      RINOK(GetBindInfoPart(name, bind.InCoder, bind.InStream));
      if (!name.IsEmpty())
        return E_INVALIDARG;
      _binds.Add(bind);
      continue;
    }

    RINOK(SetProperty(name, value));
  }

  int numEmptyMethods = GetNumEmptyMethods();
  if (numEmptyMethods > 0)
  {
    int k;
    for (k = 0; k < _binds.Size(); k++)
    {
      const CBind &bind = _binds[k];
      if (bind.InCoder < (UInt32)numEmptyMethods ||
          bind.OutCoder < (UInt32)numEmptyMethods)
        return E_INVALIDARG;
    }
    for (k = 0; k < _binds.Size(); k++)
    {
      CBind &bind = _binds[k];
      bind.InCoder -= (UInt32)numEmptyMethods;
      bind.OutCoder -= (UInt32)numEmptyMethods;
    }
    _methods.Delete(0, numEmptyMethods);
  }
  
  AddDefaultMethod();

  if (!_filterMethod.MethodName.IsEmpty())
  {
    for (int k = 0; k < _binds.Size(); k++)
    {
      CBind &bind = _binds[k];
      bind.InCoder++;
      bind.OutCoder++;
    }
    _methods.Insert(0, _filterMethod);
  }

  for (int k = 0; k < _binds.Size(); k++)
  {
    const CBind &bind = _binds[k];
    if (bind.InCoder >= (UInt32)_methods.Size() ||
        bind.OutCoder >= (UInt32)_methods.Size())
      return E_INVALIDARG;
  }

  return S_OK;
  COM_TRY_END
}

}}
