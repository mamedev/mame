// MyAesReg.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "MyAes.h"

REGISTER_FILTER_E(AES256CBC,
    NCrypto::CAesCbcDecoder(32),
    NCrypto::CAesCbcEncoder(32),
    0x6F00181, "AES256CBC")
