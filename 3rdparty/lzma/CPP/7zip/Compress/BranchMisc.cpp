// BranchMisc.cpp

#include "StdAfx.h"

#include "../../../C/Bra.h"

#include "BranchMisc.h"

UInt32 CBC_ARM_Encoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::ARM_Convert(data, size, _bufferPos, 1); }

UInt32 CBC_ARM_Decoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::ARM_Convert(data, size, _bufferPos, 0); }

UInt32 CBC_ARMT_Encoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::ARMT_Convert(data, size, _bufferPos, 1); }

UInt32 CBC_ARMT_Decoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::ARMT_Convert(data, size, _bufferPos, 0); }

UInt32 CBC_PPC_Encoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::PPC_Convert(data, size, _bufferPos, 1); }

UInt32 CBC_PPC_Decoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::PPC_Convert(data, size, _bufferPos, 0); }

UInt32 CBC_SPARC_Encoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::SPARC_Convert(data, size, _bufferPos, 1); }

UInt32 CBC_SPARC_Decoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::SPARC_Convert(data, size, _bufferPos, 0); }

UInt32 CBC_IA64_Encoder::SubFilter(Byte *data, UInt32 size)
  { return (UInt32)::IA64_Convert(data, size, _bufferPos, 1); }

UInt32 CBC_IA64_Decoder::SubFilter(Byte *data, UInt32 size)
  {  return (UInt32)::IA64_Convert(data, size, _bufferPos, 0); }
