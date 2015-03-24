// BcjCoder.h

#ifndef __COMPRESS_BCJ_CODER_H
#define __COMPRESS_BCJ_CODER_H

#include "../../../C/Bra.h"

#include "BranchCoder.h"

struct CBranch86
{
  UInt32 _prevMask;
  void x86Init() { x86_Convert_Init(_prevMask); }
};

MyClassB(BCJ_x86, 0x01, 3, CBranch86 ,
    virtual void SubInit() { x86Init(); })

#endif
