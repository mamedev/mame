// BranchMisc.cpp

#include "StdAfx.h"

#include "BranchMisc.h"

STDMETHODIMP CBranchCoder::Init()
{
  _bufferPos = 0;
  return S_OK;
}

STDMETHODIMP_(UInt32) CBranchCoder::Filter(Byte *data, UInt32 size)
{
  UInt32 processed = (UInt32)BraFunc(data, size, _bufferPos, _encode);
  _bufferPos += processed;
  return processed;
}
