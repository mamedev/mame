// CopyRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "CopyCoder.h"

static void *CreateCodec() { return (void *)(ICompressCoder *)(new NCompress::CCopyCoder); }

static CCodecInfo g_CodecInfo =
{ CreateCodec, CreateCodec, 0x00, L"Copy", 1, false };

REGISTER_CODEC(Copy)
