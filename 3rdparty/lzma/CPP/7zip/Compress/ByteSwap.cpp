// ByteSwap.cpp

#include "StdAfx.h"

#include "../../Common/MyCom.h"

#include "../ICoder.h"

#include "../Common/RegisterCodec.h"

class CByteSwap2:
  public ICompressFilter,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP
  STDMETHOD(Init)();
  STDMETHOD_(UInt32, Filter)(Byte *data, UInt32 size);
};

class CByteSwap4:
  public ICompressFilter,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP
  STDMETHOD(Init)();
  STDMETHOD_(UInt32, Filter)(Byte *data, UInt32 size);
};

STDMETHODIMP CByteSwap2::Init() { return S_OK; }

STDMETHODIMP_(UInt32) CByteSwap2::Filter(Byte *data, UInt32 size)
{
  const UInt32 kStep = 2;
  UInt32 i;
  for (i = 0; i + kStep <= size; i += kStep)
  {
    Byte b = data[i];
    data[i] = data[i + 1];
    data[i + 1] = b;
  }
  return i;
}

STDMETHODIMP CByteSwap4::Init() { return S_OK; }

STDMETHODIMP_(UInt32) CByteSwap4::Filter(Byte *data, UInt32 size)
{
  const UInt32 kStep = 4;
  UInt32 i;
  for (i = 0; i + kStep <= size; i += kStep)
  {
    Byte b0 = data[i];
    Byte b1 = data[i + 1];
    data[i] = data[i + 3];
    data[i + 1] = data[i + 2];
    data[i + 2] = b1;
    data[i + 3] = b0;
  }
  return i;
}

static void *CreateCodec2() { return (void *)(ICompressFilter *)(new CByteSwap2); }
static void *CreateCodec4() { return (void *)(ICompressFilter *)(new CByteSwap4); }

static CCodecInfo g_CodecsInfo[] =
{
  { CreateCodec2, CreateCodec2, 0x020302, L"Swap2", 1, true },
  { CreateCodec4, CreateCodec4, 0x020304, L"Swap4", 1, true }
};

REGISTER_CODECS(ByteSwap)
