// Lzma2Register.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "Lzma2Decoder.h"

static void *CreateCodec() { return (void *)(ICompressCoder *)(new NCompress::NLzma2::CDecoder); }
#ifndef EXTRACT_ONLY
#include "Lzma2Encoder.h"
static void *CreateCodecOut() { return (void *)(ICompressCoder *)(new NCompress::NLzma2::CEncoder);  }
#else
#define CreateCodecOut 0
#endif

static CCodecInfo g_CodecInfo =
  { CreateCodec, CreateCodecOut, 0x21, L"LZMA2", 1, false };

REGISTER_CODEC(LZMA2)
