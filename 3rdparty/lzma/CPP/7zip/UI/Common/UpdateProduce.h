// UpdateProduce.h

#ifndef __UPDATE_PRODUCE_H
#define __UPDATE_PRODUCE_H

#include "UpdatePair.h"

struct CUpdatePair2
{
  bool NewData;
  bool NewProps;
  bool IsAnti;
  
  int DirIndex;
  int ArcIndex;
  int NewNameIndex;

  bool ExistOnDisk() const { return DirIndex != -1; }
  bool ExistInArchive() const { return ArcIndex != -1; }

  CUpdatePair2(): IsAnti(false), DirIndex(-1), ArcIndex(-1), NewNameIndex(-1) {}
};

struct IUpdateProduceCallback
{
  virtual HRESULT ShowDeleteFile(int arcIndex) = 0;
};

void UpdateProduce(
    const CRecordVector<CUpdatePair> &updatePairs,
    const NUpdateArchive::CActionSet &actionSet,
    CRecordVector<CUpdatePair2> &operationChain,
    IUpdateProduceCallback *callback);

#endif
