// UpdateAction.h

#ifndef __UPDATE_ACTION_H
#define __UPDATE_ACTION_H

namespace NUpdateArchive {

  namespace NPairState
  {
    const int kNumValues = 7;
    enum EEnum
    {
      kNotMasked = 0,
      kOnlyInArchive,
      kOnlyOnDisk,
      kNewInArchive,
      kOldInArchive,
      kSameFiles,
      kUnknowNewerFiles
    };
  }
 
  namespace NPairAction
  {
    enum EEnum
    {
      kIgnore = 0,
      kCopy,
      kCompress,
      kCompressAsAnti
    };
  }
  
  struct CActionSet
  {
    NPairAction::EEnum StateActions[NPairState::kNumValues];
    bool NeedScanning() const
    {
      int i;
      for (i = 0; i < NPairState::kNumValues; i++)
        if (StateActions[i] == NPairAction::kCompress)
          return true;
      for (i = 1; i < NPairState::kNumValues; i++)
        if (StateActions[i] != NPairAction::kIgnore)
          return true;
      return false;
    }
  };
  
  extern const CActionSet kAddActionSet;
  extern const CActionSet kUpdateActionSet;
  extern const CActionSet kFreshActionSet;
  extern const CActionSet kSynchronizeActionSet;
  extern const CActionSet kDeleteActionSet;
}

#endif
