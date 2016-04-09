// LzmaRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "LzmaDecoder.h"

#ifndef EXTRACT_ONLY
#include "LzmaEncoder.h"
#endif

REGISTER_CODEC_E(LZMA,
    NCompress::NLzma::CDecoder(),
    NCompress::NLzma::CEncoder(),
    0x30101,
    "LZMA")
