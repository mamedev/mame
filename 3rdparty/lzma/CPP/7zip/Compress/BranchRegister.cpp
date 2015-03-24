// BranchRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "BranchMisc.h"

#define CREATE_CODEC(x) \
  static void *CreateCodec ## x() { return (void *)(ICompressFilter *)(new C ## x ## _Decoder); } \
  static void *CreateCodec ## x ## Out() { return (void *)(ICompressFilter *)(new C ## x ## _Encoder); }

CREATE_CODEC(BC_PPC)
CREATE_CODEC(BC_IA64)
CREATE_CODEC(BC_ARM)
CREATE_CODEC(BC_ARMT)
CREATE_CODEC(BC_SPARC)

#define METHOD_ITEM(x, id1, id2, name) { CreateCodec ## x, CreateCodec ## x ## Out, 0x03030000 + (id1 * 256) + id2, name, 1, true  }

static CCodecInfo g_CodecsInfo[] =
{
  METHOD_ITEM(BC_PPC,   0x02, 0x05, L"PPC"),
  METHOD_ITEM(BC_IA64,  0x04, 1, L"IA64"),
  METHOD_ITEM(BC_ARM,   0x05, 1, L"ARM"),
  METHOD_ITEM(BC_ARMT,  0x07, 1, L"ARMT"),
  METHOD_ITEM(BC_SPARC, 0x08, 0x05, L"SPARC")
};

REGISTER_CODECS(Branch)
