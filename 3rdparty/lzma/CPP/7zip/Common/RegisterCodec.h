// RegisterCodec.h

#ifndef __REGISTERCODEC_H
#define __REGISTERCODEC_H

#include "../Common/MethodId.h"

typedef void * (*CreateCodecP)();
struct CCodecInfo
{
  CreateCodecP CreateDecoder;
  CreateCodecP CreateEncoder;
  CMethodId Id;
  const wchar_t *Name;
  UInt32 NumInStreams;
  bool IsFilter;
};

void RegisterCodec(const CCodecInfo *codecInfo);

#define REGISTER_CODEC_NAME(x) CRegisterCodec ## x

#define REGISTER_CODEC(x) struct REGISTER_CODEC_NAME(x) { \
    REGISTER_CODEC_NAME(x)() { RegisterCodec(&g_CodecInfo); }}; \
    static REGISTER_CODEC_NAME(x) g_RegisterCodec;

#define REGISTER_CODECS_NAME(x) CRegisterCodecs ## x
#define REGISTER_CODECS(x) struct REGISTER_CODECS_NAME(x) { \
    REGISTER_CODECS_NAME(x)() { for (int i = 0; i < sizeof(g_CodecsInfo) / sizeof(g_CodecsInfo[0]); i++) \
    RegisterCodec(&g_CodecsInfo[i]); }}; \
    static REGISTER_CODECS_NAME(x) g_RegisterCodecs;

#endif
