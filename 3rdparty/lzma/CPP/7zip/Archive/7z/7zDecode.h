// 7zDecode.h

#ifndef __7Z_DECODE_H
#define __7Z_DECODE_H

#include "../../IStream.h"
#include "../../IPassword.h"

#include "../Common/CoderMixer2.h"
#include "../Common/CoderMixer2MT.h"
#ifdef _ST_MODE
#include "../Common/CoderMixer2ST.h"
#endif

#include "../../Common/CreateCoder.h"

#include "7zItem.h"

namespace NArchive {
namespace N7z {

struct CBindInfoEx: public NCoderMixer::CBindInfo
{
  CRecordVector<CMethodId> CoderMethodIDs;
  void Clear()
  {
    CBindInfo::Clear();
    CoderMethodIDs.Clear();
  }
};

class CDecoder
{
  bool _bindInfoExPrevIsDefined;
  CBindInfoEx _bindInfoExPrev;
  
  bool _multiThread;
  #ifdef _ST_MODE
  NCoderMixer::CCoderMixer2ST *_mixerCoderSTSpec;
  #endif
  NCoderMixer::CCoderMixer2MT *_mixerCoderMTSpec;
  NCoderMixer::CCoderMixer2 *_mixerCoderCommon;
  
  CMyComPtr<ICompressCoder2> _mixerCoder;
  CObjectVector<CMyComPtr<IUnknown> > _decoders;
  // CObjectVector<CMyComPtr<ICompressCoder2> > _decoders2;
public:
  CDecoder(bool multiThread);
  HRESULT Decode(
      DECL_EXTERNAL_CODECS_LOC_VARS
      IInStream *inStream,
      UInt64 startPos,
      const UInt64 *packSizes,
      const CFolder &folder,
      ISequentialOutStream *outStream,
      ICompressProgressInfo *compressProgress
      #ifndef _NO_CRYPTO
      , ICryptoGetTextPassword *getTextPasswordSpec, bool &passwordIsDefined
      #endif
      #if !defined(_7ZIP_ST) && !defined(_SFX)
      , bool mtMode, UInt32 numThreads
      #endif
      );
};

}}

#endif
