// CodecExports.cpp

#include "StdAfx.h"

#include "../../Common/ComTry.h"

#include "../../Windows/PropVariant.h"

#include "../ICoder.h"

#include "../Common/RegisterCodec.h"

extern unsigned int g_NumCodecs;
extern const CCodecInfo *g_Codecs[];

static const UInt16 kDecodeId = 0x2790;

DEFINE_GUID(CLSID_CCodec,
0x23170F69, 0x40C1, kDecodeId, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

static inline HRESULT SetPropString(const char *s, unsigned int size, PROPVARIANT *value)
{
  if ((value->bstrVal = ::SysAllocStringByteLen(s, size)) != 0)
    value->vt = VT_BSTR;
  return S_OK;
}

static inline HRESULT SetPropGUID(const GUID &guid, PROPVARIANT *value)
{
  return SetPropString((const char *)&guid, sizeof(GUID), value);
}

static HRESULT SetClassID(CMethodId id, bool encode, PROPVARIANT *value)
{
  GUID clsId = CLSID_CCodec;
  for (int i = 0; i < sizeof(id); i++, id >>= 8)
    clsId.Data4[i] = (Byte)(id & 0xFF);
  if (encode)
    clsId.Data3++;
  return SetPropGUID(clsId, value);
}

static HRESULT FindCodecClassId(const GUID *clsID, UInt32 isCoder2, bool isFilter, bool &encode, int &index)
{
  index = -1;
  if (clsID->Data1 != CLSID_CCodec.Data1 ||
      clsID->Data2 != CLSID_CCodec.Data2 ||
      (clsID->Data3 & ~1) != kDecodeId)
    return S_OK;
  encode = (clsID->Data3 != kDecodeId);
  UInt64 id = 0;
  for (int j = 0; j < 8; j++)
    id |= ((UInt64)clsID->Data4[j]) << (8 * j);
  for (unsigned i = 0; i < g_NumCodecs; i++)
  {
    const CCodecInfo &codec = *g_Codecs[i];
    if (id != codec.Id || encode && !codec.CreateEncoder || !encode && !codec.CreateDecoder)
      continue;
    if (!isFilter && codec.IsFilter || isFilter && !codec.IsFilter ||
        codec.NumInStreams != 1 && !isCoder2 || codec.NumInStreams == 1 && isCoder2)
      return E_NOINTERFACE;
    index = i;
    return S_OK;
  }
  return S_OK;
}

STDAPI CreateCoder2(bool encode, UInt32 index, const GUID *iid, void **outObject)
{
  COM_TRY_BEGIN
  *outObject = 0;
  bool isCoder = (*iid == IID_ICompressCoder) != 0;
  bool isCoder2 = (*iid == IID_ICompressCoder2) != 0;
  bool isFilter = (*iid == IID_ICompressFilter) != 0;
  const CCodecInfo &codec = *g_Codecs[index];
  if (!isFilter && codec.IsFilter || isFilter && !codec.IsFilter ||
      codec.NumInStreams != 1 && !isCoder2 || codec.NumInStreams == 1 && isCoder2)
    return E_NOINTERFACE;
  if (encode)
  {
    if (!codec.CreateEncoder)
      return CLASS_E_CLASSNOTAVAILABLE;
    *outObject = codec.CreateEncoder();
  }
  else
  {
    if (!codec.CreateDecoder)
      return CLASS_E_CLASSNOTAVAILABLE;
    *outObject = codec.CreateDecoder();
  }
  if (isCoder)
    ((ICompressCoder *)*outObject)->AddRef();
  else if (isCoder2)
    ((ICompressCoder2 *)*outObject)->AddRef();
  else
    ((ICompressFilter *)*outObject)->AddRef();
  return S_OK;
  COM_TRY_END
}

STDAPI CreateCoder(const GUID *clsid, const GUID *iid, void **outObject)
{
  *outObject = 0;
  bool isCoder = (*iid == IID_ICompressCoder) != 0;
  bool isCoder2 = (*iid == IID_ICompressCoder2) != 0;
  bool isFilter = (*iid == IID_ICompressFilter) != 0;
  if (!isCoder && !isCoder2 && !isFilter)
    return E_NOINTERFACE;
  bool encode;
  int codecIndex;
  HRESULT res = FindCodecClassId(clsid, isCoder2, isFilter, encode, codecIndex);
  if (res != S_OK)
    return res;
  if (codecIndex < 0)
    return CLASS_E_CLASSNOTAVAILABLE;
  return CreateCoder2(encode, codecIndex, iid, outObject);
}

STDAPI GetMethodProperty(UInt32 codecIndex, PROPID propID, PROPVARIANT *value)
{
  ::VariantClear((VARIANTARG *)value);
  const CCodecInfo &codec = *g_Codecs[codecIndex];
  switch(propID)
  {
    case NMethodPropID::kID:
    {
      value->uhVal.QuadPart = (UInt64)codec.Id;
      value->vt = VT_UI8;
      break;
    }
    case NMethodPropID::kName:
      if ((value->bstrVal = ::SysAllocString(codec.Name)) != 0)
        value->vt = VT_BSTR;
      break;
    case NMethodPropID::kDecoder:
      if (codec.CreateDecoder)
        return SetClassID(codec.Id, false, value);
      break;
    case NMethodPropID::kEncoder:
      if (codec.CreateEncoder)
        return SetClassID(codec.Id, true, value);
      break;
    case NMethodPropID::kInStreams:
    {
      if (codec.NumInStreams != 1)
      {
        value->vt = VT_UI4;
        value->ulVal = (ULONG)codec.NumInStreams;
      }
      break;
    }
  }
  return S_OK;
}

STDAPI GetNumberOfMethods(UINT32 *numCodecs)
{
  *numCodecs = g_NumCodecs;
  return S_OK;
}
