// UpdateProduce.cpp

#include "StdAfx.h"

#include "UpdateProduce.h"

using namespace NUpdateArchive;

static const char *kUpdateActionSetCollision = "Internal collision in update action set";

void UpdateProduce(
    const CRecordVector<CUpdatePair> &updatePairs,
    const CActionSet &actionSet,
    CRecordVector<CUpdatePair2> &operationChain,
    IUpdateProduceCallback *callback)
{
  for (int i = 0; i < updatePairs.Size(); i++)
  {
    const CUpdatePair &pair = updatePairs[i];

    CUpdatePair2 up2;
    up2.IsAnti = false;
    up2.DirIndex = pair.DirIndex;
    up2.ArcIndex = pair.ArcIndex;
    up2.NewData = up2.NewProps = true;
    
    switch(actionSet.StateActions[pair.State])
    {
      case NPairAction::kIgnore:
        /*
        if (pair.State != NPairState::kOnlyOnDisk)
          IgnoreArchiveItem(m_ArchiveItems[pair.ArcIndex]);
        // cout << "deleting";
        */
        if (callback)
          callback->ShowDeleteFile(pair.ArcIndex);
        continue;

      case NPairAction::kCopy:
        if (pair.State == NPairState::kOnlyOnDisk)
          throw kUpdateActionSetCollision;
        up2.NewData = up2.NewProps = false;
        break;
      
      case NPairAction::kCompress:
        if (pair.State == NPairState::kOnlyInArchive ||
            pair.State == NPairState::kNotMasked)
          throw kUpdateActionSetCollision;
        break;
      
      case NPairAction::kCompressAsAnti:
        up2.IsAnti = true;
        break;
    }
    operationChain.Add(up2);
  }
  operationChain.ReserveDown();
}
