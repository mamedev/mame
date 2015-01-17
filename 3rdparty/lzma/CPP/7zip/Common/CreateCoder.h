// CreateCoder.h

#ifndef __CREATE_CODER_H
#define __CREATE_CODER_H

#include "../../Common/MyCom.h"
#include "../../Common/MyString.h"
#include "../ICoder.h"

#include "MethodId.h"

#ifdef EXTERNAL_CODECS

struct CCodecInfoEx
{
  UString Name;
  CMethodId Id;
  UInt32 NumInStreams;
  UInt32 NumOutStreams;
  bool EncoderIsAssigned;
  bool DecoderIsAssigned;
  bool IsSimpleCodec() const { return NumOutStreams == 1 && NumInStreams == 1; }
  CCodecInfoEx(): EncoderIsAssigned(false), DecoderIsAssigned(false) {}
};

HRESULT LoadExternalCodecs(ICompressCodecsInfo *codecsInfo, CObjectVector<CCodecInfoEx> &externalCodecs);

#define PUBLIC_ISetCompressCodecsInfo public ISetCompressCodecsInfo,
#define QUERY_ENTRY_ISetCompressCodecsInfo MY_QUERYINTERFACE_ENTRY(ISetCompressCodecsInfo)
#define DECL_ISetCompressCodecsInfo STDMETHOD(SetCompressCodecsInfo)(ICompressCodecsInfo *compressCodecsInfo);
#define IMPL_ISetCompressCodecsInfo2(x) \
STDMETHODIMP x::SetCompressCodecsInfo(ICompressCodecsInfo *compressCodecsInfo) { \
  COM_TRY_BEGIN _codecsInfo = compressCodecsInfo;  return LoadExternalCodecs(_codecsInfo, _externalCodecs); COM_TRY_END }
#define IMPL_ISetCompressCodecsInfo IMPL_ISetCompressCodecsInfo2(CHandler)

#define EXTERNAL_CODECS_VARS2 _codecsInfo, &_externalCodecs

#define DECL_EXTERNAL_CODECS_VARS CMyComPtr<ICompressCodecsInfo> _codecsInfo; CObjectVector<CCodecInfoEx> _externalCodecs;
#define EXTERNAL_CODECS_VARS EXTERNAL_CODECS_VARS2,

#define DECL_EXTERNAL_CODECS_LOC_VARS2 ICompressCodecsInfo *codecsInfo, const CObjectVector<CCodecInfoEx> *externalCodecs
#define EXTERNAL_CODECS_LOC_VARS2 codecsInfo, externalCodecs

#define DECL_EXTERNAL_CODECS_LOC_VARS DECL_EXTERNAL_CODECS_LOC_VARS2,
#define EXTERNAL_CODECS_LOC_VARS EXTERNAL_CODECS_LOC_VARS2,

#else

#define PUBLIC_ISetCompressCodecsInfo
#define QUERY_ENTRY_ISetCompressCodecsInfo
#define DECL_ISetCompressCodecsInfo
#define IMPL_ISetCompressCodecsInfo
#define EXTERNAL_CODECS_VARS2
#define DECL_EXTERNAL_CODECS_VARS
#define EXTERNAL_CODECS_VARS EXTERNAL_CODECS_VARS2
#define DECL_EXTERNAL_CODECS_LOC_VARS2
#define EXTERNAL_CODECS_LOC_VARS2
#define DECL_EXTERNAL_CODECS_LOC_VARS
#define EXTERNAL_CODECS_LOC_VARS

#endif

bool FindMethod(
  DECL_EXTERNAL_CODECS_LOC_VARS
  const UString &name, CMethodId &methodId, UInt32 &numInStreams, UInt32 &numOutStreams);

bool FindMethod(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId, UString &name);


HRESULT CreateCoder(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId,
  CMyComPtr<ICompressFilter> &filter,
  CMyComPtr<ICompressCoder> &coder,
  CMyComPtr<ICompressCoder2> &coder2,
  bool encode, bool onlyCoder);

HRESULT CreateCoder(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId,
  CMyComPtr<ICompressCoder> &coder,
  CMyComPtr<ICompressCoder2> &coder2,
  bool encode);

HRESULT CreateCoder(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId,
  CMyComPtr<ICompressCoder> &coder, bool encode);

HRESULT CreateFilter(
  DECL_EXTERNAL_CODECS_LOC_VARS
  CMethodId methodId,
  CMyComPtr<ICompressFilter> &filter,
  bool encode);

#endif
