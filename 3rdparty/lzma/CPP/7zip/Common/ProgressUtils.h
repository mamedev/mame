// ProgressUtils.h

#ifndef __PROGRESSUTILS_H
#define __PROGRESSUTILS_H

#include "../../Common/MyCom.h"

#include "../ICoder.h"
#include "../IProgress.h"

class CLocalProgress:
  public ICompressProgressInfo,
  public CMyUnknownImp
{
  CMyComPtr<IProgress> _progress;
  CMyComPtr<ICompressProgressInfo> _ratioProgress;
  bool _inSizeIsMain;
public:
  UInt64 ProgressOffset;
  UInt64 InSize;
  UInt64 OutSize;
  bool SendRatio;
  bool SendProgress;

  CLocalProgress();
  void Init(IProgress *progress, bool inSizeIsMain);
  HRESULT SetCur();

  MY_UNKNOWN_IMP

  STDMETHOD(SetRatioInfo)(const UInt64 *inSize, const UInt64 *outSize);
};

#endif
