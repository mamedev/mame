// CreateCoder.cpp

#include "StdAfx.h"

#include "../../Windows/Defs.h"
#include "../../Windows/PropVariant.h"

#include "CreateCoder.h"

#include "FilterCoder.h"
#include "RegisterCodec.h"

static const unsigned int kNumCodecsMax = 64;
unsigned int g_NumCodecs = 0;
const CCodecInfo *g_Codecs[kNumCodecsMax];
void RegisterCodec(const CCodecInfo *codecInfo)
{
  if (g_NumCodecs < kNumCodecsMax)
    g_Codecs[g_NumCodecs++] = codecInfo;
}

#ifdef EXTERNAL_CODECS
static HRESULT ReadNumberOfStreams(ICompressCodecsInfo *codecsInfo, UInt32 index, PROPID propID, UInt32 &res)
{
  NWindows::NCOM::CPropVariant prop;
  RINOK(codecsInfo->GetProperty(index, propID, &prop));
  if (prop.vt == VT_EMPTY)
    res = 1;
  else if (prop.vt == VT_UI4)
    res = prop.ulVal;
  else
    return E_INVALIDARG;
  return S_OK;
}

static HRESULT ReadIsAssignedProp(ICompressCodecsInfo *codecsInfo, UInt32 index, PROPID propID, bool &res)
{
  NWindows::NCOM::CPropVariant prop;
  RINOK(codecsInfo->GetProperty(index, propID, &prop));
  if (prop.vt == VT_EMPTY)
    res = true;
  else if (prop.vt == VT_BOOL)
    res = VARIANT_BOOLToBool(prop.boolVal);
  else
    return E_INVALIDARG;
  return S_OK;
}

HRESULT LoadExternalCodecs(ICompressCodecsInfo *codecsInfo, CObjectVector<CCodecInfoEx> &externalCodecs)
{
  UInt32 num;
  RINOK(codecsInfo->GetNumberOfMethods(&num));
  for (UInt32 i = 0; i < num; i++)
  {
    CCodecInfoEx info;
    NWindows::NCOM::CPropVariant prop;
    RINOK(codecsInfo->GetProperty(i, NMethodPropID::kID, &prop));
    // if (prop.vt != VT_BSTR)
    // info.Id.IDSize = (Byte)SysStringByteLen(prop.bstrVal);
    // memmove(info.Id.ID, prop.bstrVal, info.Id.IDSize);
    if (prop.vt != VT_UI8)
    {
      continue; // old Interface
      // return E_INVALIDARG;
    }
    info.Id = prop.uhVal.QuadPart;
    prop.Clear();
    
    RINOK(codecsInfo->GetProperty(i, NMethodPropID::kName, &prop));
    if (prop.vt == VT_BSTR)
      info.Name = prop.bstrVal;
    else if (prop.vt != VT_EMPTY)
      return E_INVALIDARG;;
    
    RINOK(ReadNumberOfStreams(codecsInfo, i, NMethodPropID::kInStreams, info.NumInStreams));
    RINOK(ReadNumberOfStreams(codecsInfo, i, NMethodPropID::kOutStreams, info.NumOutStreams));
    RINOK(ReadIsAssignedProp(codecsInfo, i, NMethodPropID::kEncoderIsAssigned, info.EncoderIsAssigned));
    RINOK(ReadIsAssignedProp(codecsInfo, i, NMethodPropID::kDecoderIsAssigned, info.DecoderIsAssigned));
    
    externalCodecs.Add(info);
  }
  return S_OK;
}

#endif

bool FindMethod(
  #ifdef EXTERNAL_CODECS
  ICompressCodecsInfo * /* codecsInfo */, const CObjectVector<CCodecInfoEx> *externalCodecs,
  #endif
  const UString &name,
  CMethodId &methodId, UInt32 &numInStreams, UInt32 &numOutStreams)
{
  UInt32 i;
  for (i = 0; i < g_NumCodecs; i++)
  {
    const CCodecInfo &codec = *g_Codecs[i];
    if (name.CompareNoCase(codec.Name) == 0)
    {
      methodId = codec.Id;
      numInStreams = codec.NumInStreams;
      numOutStreams = 1;
      return true;
    }
  }
  #ifdef EXTERNAL_CODECS
  if (externalCodecs)
    for (i = 0; i < (UInt32)externalCodecs->Size(); i++)
    {
      const CCodecInfoEx &codec = (*externalCodecs)[i];
      if (codec.Name.CompareNoCase(name) == 0)
      {
        methodId = codec.Id;
        numInStreams = codec.NumInStreams;
        numOutStreams = codec.NumOutStreams;
        return true;
      }
    }
  #endif
  return false;
}

bool FindMethod(
  #ifdef EXTERNAL_CODECS
  ICompressCodecsInfo * /* codecsInfo */, const CObjectVector<CCodecInfoEx> *externalCodecs,
  #endif
  CMethodId methodId, UString &name)
{
  UInt32 i;
  for (i = 0; i < g_NumCodecs; i++)
  {
    const CCodecInfo &codec = *g_Codecs[i];
    if (methodId == codec.Id)
    {
      name = codec.Name;
      return true;
    }
  }
  #ifdef EXTERNAL_CODECS
  if (externalCodecs)
    for (i = 0; i < (UInt32)externalCodecs->Size(); i++)
    {
      const CCodecInfoEx &codec = (*externalCodecs)[i];
      if (methodId == codec.Id)
      {
        name = codec.Name;
        return true;
      }
    }
  #endif
  return false;
}

HRESULT CreateCoder(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId,
  CMyComPtr<ICompressFilter> &filter,
  CMyComPtr<ICompressCoder> &coder,
  CMyComPtr<ICompressCoder2> &coder2,
  bool encode, bool onlyCoder)
{
  bool created = false;
  UInt32 i;
  for (i = 0; i < g_NumCodecs; i++)
  {
    const CCodecInfo &codec = *g_Codecs[i];
    if (codec.Id == methodId)
    {
      if (encode)
      {
        if (codec.CreateEncoder)
        {
          void *p = codec.CreateEncoder();
          if (codec.IsFilter) filter = (ICompressFilter *)p;
          else if (codec.NumInStreams == 1) coder = (ICompressCoder *)p;
          else coder2 = (ICompressCoder2 *)p;
          created = (p != 0);
          break;
        }
      }
      else
        if (codec.CreateDecoder)
        {
          void *p = codec.CreateDecoder();
          if (codec.IsFilter) filter = (ICompressFilter *)p;
          else if (codec.NumInStreams == 1) coder = (ICompressCoder *)p;
          else coder2 = (ICompressCoder2 *)p;
          created = (p != 0);
          break;
        }
    }
  }

  #ifdef EXTERNAL_CODECS
  if (!created && externalCodecs)
    for (i = 0; i < (UInt32)externalCodecs->Size(); i++)
    {
      const CCodecInfoEx &codec = (*externalCodecs)[i];
      if (codec.Id == methodId)
      {
        if (encode)
        {
          if (codec.EncoderIsAssigned)
          {
            if (codec.IsSimpleCodec())
            {
              HRESULT result = codecsInfo->CreateEncoder(i, &IID_ICompressCoder, (void **)&coder);
              if (result != S_OK && result != E_NOINTERFACE && result != CLASS_E_CLASSNOTAVAILABLE)
                return result;
              if (!coder)
              {
                RINOK(codecsInfo->CreateEncoder(i, &IID_ICompressFilter, (void **)&filter));
              }
            }
            else
            {
              RINOK(codecsInfo->CreateEncoder(i, &IID_ICompressCoder2, (void **)&coder2));
            }
            break;
          }
        }
        else
          if (codec.DecoderIsAssigned)
          {
            if (codec.IsSimpleCodec())
            {
              HRESULT result = codecsInfo->CreateDecoder(i, &IID_ICompressCoder, (void **)&coder);
              if (result != S_OK && result != E_NOINTERFACE && result != CLASS_E_CLASSNOTAVAILABLE)
                return result;
              if (!coder)
              {
                RINOK(codecsInfo->CreateDecoder(i, &IID_ICompressFilter, (void **)&filter));
              }
            }
            else
            {
              RINOK(codecsInfo->CreateDecoder(i, &IID_ICompressCoder2, (void **)&coder2));
            }
            break;
          }
      }
    }
  #endif

  if (onlyCoder && filter)
  {
    CFilterCoder *coderSpec = new CFilterCoder;
    coder = coderSpec;
    coderSpec->Filter = filter;
  }
  return S_OK;
}

HRESULT CreateCoder(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId,
  CMyComPtr<ICompressCoder> &coder,
  CMyComPtr<ICompressCoder2> &coder2,
  bool encode)
{
  CMyComPtr<ICompressFilter> filter;
  return CreateCoder(
    EXTERNAL_CODECS_LOC_VARS
    methodId,
    filter, coder, coder2, encode, true);
}

HRESULT CreateCoder(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId,
  CMyComPtr<ICompressCoder> &coder, bool encode)
{
  CMyComPtr<ICompressFilter> filter;
  CMyComPtr<ICompressCoder2> coder2;
  return CreateCoder(
    EXTERNAL_CODECS_LOC_VARS
    methodId,
    coder, coder2, encode);
}

HRESULT CreateFilter(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId,
  CMyComPtr<ICompressFilter> &filter,
  bool encode)
{
  CMyComPtr<ICompressCoder> coder;
  CMyComPtr<ICompressCoder2> coder2;
  return CreateCoder(
    EXTERNAL_CODECS_LOC_VARS
    methodId,
    filter, coder, coder2, encode, false);
}
