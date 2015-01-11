// BcjRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "BcjCoder.h"

static void *CreateCodec() { return (void *)(ICompressFilter *)(new CBCJ_x86_Decoder()); }
#ifndef EXTRACT_ONLY
static void *CreateCodecOut() { return (void *)(ICompressFilter *)(new CBCJ_x86_Encoder());  }
#else
#define CreateCodecOut 0
#endif

static CCodecInfo g_CodecInfo =
  { CreateCodec, CreateCodecOut, 0x03030103, L"BCJ", 1, true };

REGISTER_CODEC(BCJ)
