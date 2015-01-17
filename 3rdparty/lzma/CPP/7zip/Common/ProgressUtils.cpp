// ProgressUtils.h

#include "StdAfx.h"

#include "ProgressUtils.h"

CLocalProgress::CLocalProgress()
{
  ProgressOffset = InSize = OutSize = 0;
  SendRatio = SendProgress = true;
}

void CLocalProgress::Init(IProgress *progress, bool inSizeIsMain)
{
  _ratioProgress.Release();
  _progress = progress;
  _progress.QueryInterface(IID_ICompressProgressInfo, &_ratioProgress);
  _inSizeIsMain = inSizeIsMain;
}

STDMETHODIMP CLocalProgress::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize)
{
  UInt64 inSizeNew = InSize, outSizeNew = OutSize;
  if (inSize)
    inSizeNew += (*inSize);
  if (outSize)
    outSizeNew += (*outSize);
  if (SendRatio && _ratioProgress)
  {
    RINOK(_ratioProgress->SetRatioInfo(&inSizeNew, &outSizeNew));
  }
  inSizeNew += ProgressOffset;
  outSizeNew += ProgressOffset;
  if (SendProgress)
    return _progress->SetCompleted(_inSizeIsMain ? &inSizeNew : &outSizeNew);
  return S_OK;
}

HRESULT CLocalProgress::SetCur()
{
  return SetRatioInfo(NULL, NULL);
}
