// 7zEncode.h

#ifndef __7Z_ENCODE_H
#define __7Z_ENCODE_H

// #include "../../Common/StreamObjects.h"

#include "7zCompressionMode.h"

#include "../Common/CoderMixer2.h"
#include "../Common/CoderMixer2MT.h"
#ifdef _ST_MODE
#include "../Common/CoderMixer2ST.h"
#endif
#include "7zItem.h"

#include "../../Common/CreateCoder.h"

namespace NArchive {
namespace N7z {

class CEncoder
{
  NCoderMixer::CCoderMixer2MT *_mixerCoderSpec;
  CMyComPtr<ICompressCoder2> _mixerCoder;

  CObjectVector<CCoderInfo> _codersInfo;

  CCompressionMethodMode _options;
  NCoderMixer::CBindInfo _bindInfo;
  NCoderMixer::CBindInfo _decompressBindInfo;
  NCoderMixer::CBindReverseConverter *_bindReverseConverter;
  CRecordVector<CMethodId> _decompressionMethods;

  HRESULT CreateMixerCoder(DECL_EXTERNAL_CODECS_LOC_VARS
      const UInt64 *inSizeForReduce);

  bool _constructed;
public:
  CEncoder(const CCompressionMethodMode &options);
  ~CEncoder();
  HRESULT EncoderConstr();
  HRESULT Encode(
      DECL_EXTERNAL_CODECS_LOC_VARS
      ISequentialInStream *inStream,
      const UInt64 *inStreamSize, const UInt64 *inSizeForReduce,
      CFolder &folderItem,
      ISequentialOutStream *outStream,
      CRecordVector<UInt64> &packSizes,
      ICompressProgressInfo *compressProgress);
};

}}

#endif
