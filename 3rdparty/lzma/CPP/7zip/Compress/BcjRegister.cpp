// BcjRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "BcjCoder.h"

REGISTER_FILTER_E(BCJ,
    CBcjCoder(false),
    CBcjCoder(true),
    0x3030103, "BCJ")
