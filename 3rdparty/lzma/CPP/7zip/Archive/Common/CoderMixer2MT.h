// CoderMixer2MT.h

#ifndef __CODER_MIXER2_MT_H
#define __CODER_MIXER2_MT_H

#include "CoderMixer2.h"
#include "../../../Common/MyCom.h"
#include "../../Common/StreamBinder.h"
#include "../../Common/VirtThread.h"

namespace NCoderMixer {

struct CCoder2: public CCoderInfo2, public CVirtThread
{
  HRESULT Result;
  CObjectVector< CMyComPtr<ISequentialInStream> > InStreams;
  CObjectVector< CMyComPtr<ISequentialOutStream> > OutStreams;
  CRecordVector<ISequentialInStream*> InStreamPointers;
  CRecordVector<ISequentialOutStream*> OutStreamPointers;

  CCoder2(UInt32 numInStreams, UInt32 numOutStreams);
  ~CCoder2() { CVirtThread::WaitThreadFinish(); }
  void SetCoderInfo(const UInt64 **inSizes, const UInt64 **outSizes);
  virtual void Execute();
  void Code(ICompressProgressInfo *progress);
};


/*
  SetBindInfo()
  for each coder
    AddCoder[2]()
  SetProgressIndex(UInt32 coderIndex);
 
  for each file
  {
    ReInit()
    for each coder
      SetCoderInfo
    Code
  }
*/

class CCoderMixer2MT:
  public ICompressCoder2,
  public CCoderMixer2,
  public CMyUnknownImp
{
  CBindInfo _bindInfo;
  CObjectVector<CStreamBinder> _streamBinders;
  int _progressCoderIndex;

  void AddCoderCommon();
  HRESULT Init(ISequentialInStream **inStreams, ISequentialOutStream **outStreams);
  HRESULT ReturnIfError(HRESULT code);
public:
  CObjectVector<CCoder2> _coders;
  MY_UNKNOWN_IMP

  STDMETHOD(Code)(ISequentialInStream **inStreams,
      const UInt64 **inSizes,
      UInt32 numInStreams,
      ISequentialOutStream **outStreams,
      const UInt64 **outSizes,
      UInt32 numOutStreams,
      ICompressProgressInfo *progress);

  HRESULT SetBindInfo(const CBindInfo &bindInfo);
  void AddCoder(ICompressCoder *coder);
  void AddCoder2(ICompressCoder2 *coder);
  void SetProgressCoderIndex(int coderIndex) {  _progressCoderIndex = coderIndex; }

  void ReInit();
  void SetCoderInfo(UInt32 coderIndex, const UInt64 **inSizes, const UInt64 **outSizes)
    {  _coders[coderIndex].SetCoderInfo(inSizes, outSizes); }
  UInt64 GetWriteProcessedSize(UInt32 binderIndex) const
    {  return _streamBinders[binderIndex].ProcessedSize; }
};

}
#endif
