// Lzma2Register.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "Lzma2Decoder.h"

#ifndef EXTRACT_ONLY
#include "Lzma2Encoder.h"
#endif

REGISTER_CODEC_E(LZMA2,
    NCompress::NLzma2::CDecoder(),
    NCompress::NLzma2::CEncoder(),
    0x21,
    "LZMA2")
