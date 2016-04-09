// PpmdRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "PpmdDecoder.h"

#ifndef EXTRACT_ONLY
#include "PpmdEncoder.h"
#endif

REGISTER_CODEC_E(PPMD,
    NCompress::NPpmd::CDecoder(),
    NCompress::NPpmd::CEncoder(),
    0x30401,
    "PPMD")
