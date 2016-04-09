// CopyRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "CopyCoder.h"

REGISTER_CODEC_CREATE(CreateCodec, NCompress::CCopyCoder())

REGISTER_CODEC_2(Copy, CreateCodec, CreateCodec, 0, "Copy")
