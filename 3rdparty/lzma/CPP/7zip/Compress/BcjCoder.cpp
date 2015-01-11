// BcjCoder.cpp

#include "StdAfx.h"

#include "BcjCoder.h"

UInt32 CBCJ_x86_Encoder::SubFilter(Byte *data, UInt32 size)
{
  return (UInt32)::x86_Convert(data, size, _bufferPos, &_prevMask, 1);
}

UInt32 CBCJ_x86_Decoder::SubFilter(Byte *data, UInt32 size)
{
  return (UInt32)::x86_Convert(data, size, _bufferPos, &_prevMask, 0);
}
