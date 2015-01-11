// RegisterArc.h

#ifndef __REGISTER_ARC_H
#define __REGISTER_ARC_H

#include "../Archive/IArchive.h"

typedef IInArchive * (*CreateInArchiveP)();
typedef IOutArchive * (*CreateOutArchiveP)();

struct CArcInfo
{
  const wchar_t *Name;
  const wchar_t *Ext;
  const wchar_t *AddExt;
  Byte ClassId;
  Byte Signature[16];
  int SignatureSize;
  bool KeepName;
  CreateInArchiveP CreateInArchive;
  CreateOutArchiveP CreateOutArchive;
};

void RegisterArc(const CArcInfo *arcInfo);

#define REGISTER_ARC_NAME(x) CRegister ## x

#define REGISTER_ARC(x) struct REGISTER_ARC_NAME(x) { \
    REGISTER_ARC_NAME(x)() { RegisterArc(&g_ArcInfo); }}; \
    static REGISTER_ARC_NAME(x) g_RegisterArc;

#endif
