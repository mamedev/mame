// MethodProps.cpp

#include "StdAfx.h"

#include "../../Common/StringToInt.h"

#include "MethodProps.h"

using namespace NWindows;

bool StringToBool(const UString &s, bool &res)
{
  if (s.IsEmpty() || s.CompareNoCase(L"ON") == 0 || s.Compare(L"+") == 0)
  {
    res = true;
    return true;
  }
  if (s.CompareNoCase(L"OFF") == 0 || s.Compare(L"-") == 0)
  {
    res = false;
    return true;
  }
  return false;
}

HRESULT PROPVARIANT_to_bool(const PROPVARIANT &prop, bool &dest)
{
  switch (prop.vt)
  {
    case VT_EMPTY: dest = true; return S_OK;
    case VT_BOOL: dest = (prop.boolVal != VARIANT_FALSE); return S_OK;
    case VT_BSTR: return StringToBool(prop.bstrVal, dest) ? S_OK : E_INVALIDARG;
  }
  return E_INVALIDARG;
}

int ParseStringToUInt32(const UString &srcString, UInt32 &number)
{
  const wchar_t *start = srcString;
  const wchar_t *end;
  UInt64 number64 = ConvertStringToUInt64(start, &end);
  if (number64 > (UInt32)0xFFFFFFFF)
  {
    number = 0;
    return 0;
  }
  number = (UInt32)number64;
  return (int)(end - start);
}

HRESULT ParsePropToUInt32(const UString &name, const PROPVARIANT &prop, UInt32 &resValue)
{
  // =VT_UI4
  // =VT_EMPTY
  // {stringUInt32}=VT_EMPTY

  if (prop.vt == VT_UI4)
  {
    if (!name.IsEmpty())
      return E_INVALIDARG;
    resValue = prop.ulVal;
    return S_OK;
  }
  if (prop.vt != VT_EMPTY)
    return E_INVALIDARG;
  if (name.IsEmpty())
    return S_OK;
  UInt32 v;
  if (ParseStringToUInt32(name, v) != name.Length())
    return E_INVALIDARG;
  resValue = v;
  return S_OK;
}

HRESULT ParseMtProp(const UString &name, const PROPVARIANT &prop, UInt32 defaultNumThreads, UInt32 &numThreads)
{
  if (name.IsEmpty())
  {
    switch (prop.vt)
    {
      case VT_UI4:
        numThreads = prop.ulVal;
        break;
      default:
      {
        bool val;
        RINOK(PROPVARIANT_to_bool(prop, val));
        numThreads = (val ? defaultNumThreads : 1);
        break;
      }
    }
    return S_OK;
  }
  if (prop.vt != VT_EMPTY)
    return E_INVALIDARG;
  return ParsePropToUInt32(name, prop, numThreads);
}

static HRESULT StringToDictSize(const UString &srcStringSpec, UInt32 &dicSize)
{
  UString srcString = srcStringSpec;
  srcString.MakeUpper();
  const wchar_t *start = srcString;
  const wchar_t *end;
  UInt64 number = ConvertStringToUInt64(start, &end);
  int numDigits = (int)(end - start);
  if (numDigits == 0 || srcString.Length() > numDigits + 1)
    return E_INVALIDARG;
  const unsigned kLogDictSizeLimit = 32;
  if (srcString.Length() == numDigits)
  {
    if (number >= kLogDictSizeLimit)
      return E_INVALIDARG;
    dicSize = (UInt32)1 << (int)number;
    return S_OK;
  }
  unsigned numBits;
  switch (srcString[numDigits])
  {
    case 'B': numBits =  0; break;
    case 'K': numBits = 10; break;
    case 'M': numBits = 20; break;
    case 'G': numBits = 30; break;
    default: return E_INVALIDARG;
  }
  if (number >= ((UInt64)1 << (kLogDictSizeLimit - numBits)))
    return E_INVALIDARG;
  dicSize = (UInt32)number << numBits;
  return S_OK;
}

static HRESULT PROPVARIANT_to_DictSize(const PROPVARIANT &prop, UInt32 &resValue)
{
  if (prop.vt == VT_UI4)
  {
    UInt32 v = prop.ulVal;
    if (v >= 32)
      return E_INVALIDARG;
    resValue = (UInt32)1 << v;
    return S_OK;
  }
  if (prop.vt == VT_BSTR)
    return StringToDictSize(prop.bstrVal, resValue);
  return E_INVALIDARG;
}

void CProps::AddProp32(PROPID propid, UInt32 level)
{
  CProp prop;
  prop.IsOptional = true;
  prop.Id = propid;
  prop.Value = (UInt32)level;
  Props.Add(prop);
}

class CCoderProps
{
  PROPID *_propIDs;
  NCOM::CPropVariant *_props;
  unsigned _numProps;
  unsigned _numPropsMax;
public:
  CCoderProps(unsigned numPropsMax)
  {
    _numPropsMax = numPropsMax;
    _numProps = 0;
    _propIDs = new PROPID[numPropsMax];
    _props = new NCOM::CPropVariant[numPropsMax];
  }
  ~CCoderProps()
  {
    delete []_propIDs;
    delete []_props;
  }
  void AddProp(const CProp &prop);
  HRESULT SetProps(ICompressSetCoderProperties *setCoderProperties)
  {
    return setCoderProperties->SetCoderProperties(_propIDs, _props, _numProps);
  }
};

void CCoderProps::AddProp(const CProp &prop)
{
  if (_numProps >= _numPropsMax)
    throw 1;
  _propIDs[_numProps] = prop.Id;
  _props[_numProps] = prop.Value;
  _numProps++;
}

HRESULT CProps::SetCoderProps(ICompressSetCoderProperties *scp, const UInt64 *dataSizeReduce) const
{
  CCoderProps coderProps(Props.Size() + (dataSizeReduce ? 1 : 0));
  for (int i = 0; i < Props.Size(); i++)
    coderProps.AddProp(Props[i]);
  if (dataSizeReduce)
  {
    CProp prop;
    prop.Id = NCoderPropID::kReduceSize;
    prop.Value = *dataSizeReduce;
    coderProps.AddProp(prop);
  }
  return coderProps.SetProps(scp);
}


int CMethodProps::FindProp(PROPID id) const
{
  for (int i = Props.Size() - 1; i >= 0; i--)
    if (Props[i].Id == id)
      return i;
  return -1;
}

int CMethodProps::GetLevel() const
{
  int i = FindProp(NCoderPropID::kLevel);
  if (i < 0)
    return 5;
  if (Props[i].Value.vt != VT_UI4)
    return 9;
  UInt32 level = Props[i].Value.ulVal;
  return level > 9 ? 9 : (int)level;
}

struct CNameToPropID
{
  VARTYPE VarType;
  const wchar_t *Name;
};

static const CNameToPropID g_NameToPropID[] =
{
  { VT_UI4, L"" },
  { VT_UI4, L"d" },
  { VT_UI4, L"mem" },
  { VT_UI4, L"o" },
  { VT_UI4, L"c" },
  { VT_UI4, L"pb" },
  { VT_UI4, L"lc" },
  { VT_UI4, L"lp" },
  { VT_UI4, L"fb" },
  { VT_BSTR, L"mf" },
  { VT_UI4, L"mc" },
  { VT_UI4, L"pass" },
  { VT_UI4, L"a" },
  { VT_UI4, L"mt" },
  { VT_BOOL, L"eos" },
  { VT_UI4, L"x" },
  { VT_UI4, L"reduceSize" }
};

static int FindPropIdExact(const UString &name)
{
  for (unsigned i = 0; i < sizeof(g_NameToPropID) / sizeof(g_NameToPropID[0]); i++)
    if (name.CompareNoCase(g_NameToPropID[i].Name) == 0)
      return i;
  return -1;
}

static bool ConvertProperty(const PROPVARIANT &srcProp, VARTYPE varType, NCOM::CPropVariant &destProp)
{
  if (varType == srcProp.vt)
  {
    destProp = srcProp;
    return true;
  }
  if (varType == VT_BOOL)
  {
    bool res;
    if (PROPVARIANT_to_bool(srcProp, res) != S_OK)
      return false;
    destProp = res;
    return true;
  }
  if (srcProp.vt == VT_EMPTY)
  {
    destProp = srcProp;
    return true;
  }
  return false;
}
    
static void SplitParams(const UString &srcString, UStringVector &subStrings)
{
  subStrings.Clear();
  UString s;
  int len = srcString.Length();
  if (len == 0)
    return;
  for (int i = 0; i < len; i++)
  {
    wchar_t c = srcString[i];
    if (c == L':')
    {
      subStrings.Add(s);
      s.Empty();
    }
    else
      s += c;
  }
  subStrings.Add(s);
}

static void SplitParam(const UString &param, UString &name, UString &value)
{
  int eqPos = param.Find(L'=');
  if (eqPos >= 0)
  {
    name = param.Left(eqPos);
    value = param.Mid(eqPos + 1);
    return;
  }
  int i;
  for (i = 0; i < param.Length(); i++)
  {
    wchar_t c = param[i];
    if (c >= L'0' && c <= L'9')
      break;
  }
  name = param.Left(i);
  value = param.Mid(i);
}

static bool IsLogSizeProp(PROPID propid)
{
  switch (propid)
  {
    case NCoderPropID::kDictionarySize:
    case NCoderPropID::kUsedMemorySize:
    case NCoderPropID::kBlockSize:
    case NCoderPropID::kReduceSize:
      return true;
  }
  return false;
}

HRESULT CMethodProps::SetParam(const UString &name, const UString &value)
{
  int index = FindPropIdExact(name);
  if (index < 0)
    return E_INVALIDARG;
  const CNameToPropID &nameToPropID = g_NameToPropID[index];
  CProp prop;
  prop.Id = index;

  if (IsLogSizeProp(prop.Id))
  {
    UInt32 dicSize;
    RINOK(StringToDictSize(value, dicSize));
    prop.Value = dicSize;
  }
  else
  {
    NCOM::CPropVariant propValue;
    if (nameToPropID.VarType == VT_BSTR)
      propValue = value;
    else if (nameToPropID.VarType == VT_BOOL)
    {
      bool res;
      if (!StringToBool(value, res))
        return E_INVALIDARG;
      propValue = res;
    }
    else if (!value.IsEmpty())
    {
      UInt32 number;
      if (ParseStringToUInt32(value, number) == value.Length())
        propValue = number;
      else
        propValue = value;
    }
    if (!ConvertProperty(propValue, nameToPropID.VarType, prop.Value))
      return E_INVALIDARG;
  }
  Props.Add(prop);
  return S_OK;
}

HRESULT CMethodProps::ParseParamsFromString(const UString &srcString)
{
  UStringVector params;
  SplitParams(srcString, params);
  for (int i = 0; i < params.Size(); i++)
  {
    const UString &param = params[i];
    UString name, value;
    SplitParam(param, name, value);
    RINOK(SetParam(name, value));
  }
  return S_OK;
}

HRESULT CMethodProps::ParseParamsFromPROPVARIANT(const UString &realName, const PROPVARIANT &value)
{
  if (realName.Length() == 0)
  {
    // [empty]=method
    return E_INVALIDARG;
  }
  if (value.vt == VT_EMPTY)
  {
    // {realName}=[empty]
    UString name, value;
    SplitParam(realName, name, value);
    return SetParam(name, value);
  }
  
  // {realName}=value
  int index = FindPropIdExact(realName);
  if (index < 0)
    return E_INVALIDARG;
  const CNameToPropID &nameToPropID = g_NameToPropID[index];
  CProp prop;
  prop.Id = index;
  
  if (IsLogSizeProp(prop.Id))
  {
    UInt32 dicSize;
    RINOK(PROPVARIANT_to_DictSize(value, dicSize));
    prop.Value = dicSize;
  }
  else
  {
    if (!ConvertProperty(value, nameToPropID.VarType, prop.Value))
      return E_INVALIDARG;
  }
  Props.Add(prop);
  return S_OK;
}

HRESULT COneMethodInfo::ParseMethodFromPROPVARIANT(const UString &realName, const PROPVARIANT &value)
{
  if (!realName.IsEmpty() && realName.CompareNoCase(L"m") != 0)
    return ParseParamsFromPROPVARIANT(realName, value);
  // -m{N}=method
  if (value.vt != VT_BSTR)
    return E_INVALIDARG;
  const UString s = value.bstrVal;
  int splitPos = s.Find(':');
  if (splitPos < 0)
  {
    MethodName = s;
    return S_OK;
  }
  MethodName = s.Left(splitPos);
  return ParseParamsFromString(s.Mid(splitPos + 1));
}
