// 7zUpdate.h

#ifndef __7Z_UPDATE_H
#define __7Z_UPDATE_H

#include "7zCompressionMode.h"
#include "7zIn.h"
#include "7zOut.h"

#include "../IArchive.h"

namespace NArchive {
namespace N7z {

struct CUpdateItem
{
  int IndexInArchive;
  int IndexInClient;
  
  UInt64 CTime;
  UInt64 ATime;
  UInt64 MTime;

  UInt64 Size;
  UString Name;

  UInt32 Attrib;
  
  bool NewData;
  bool NewProps;

  bool IsAnti;
  bool IsDir;

  bool AttribDefined;
  bool CTimeDefined;
  bool ATimeDefined;
  bool MTimeDefined;

  bool HasStream() const { return !IsDir && !IsAnti && Size != 0; }

  CUpdateItem():
      IsAnti(false),
      IsDir(false),
      AttribDefined(false),
      CTimeDefined(false),
      ATimeDefined(false),
      MTimeDefined(false)
      {}
  void SetDirStatusFromAttrib() { IsDir = ((Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0); };

  int GetExtensionPos() const;
  UString GetExtension() const;
};

struct CUpdateOptions
{
  const CCompressionMethodMode *Method;
  const CCompressionMethodMode *HeaderMethod;
  bool UseFilters;
  bool MaxFilter;

  CHeaderOptions HeaderOptions;

  UInt64 NumSolidFiles;
  UInt64 NumSolidBytes;
  bool SolidExtension;
  bool RemoveSfxBlock;
  bool VolumeMode;
};

HRESULT Update(
    DECL_EXTERNAL_CODECS_LOC_VARS
    IInStream *inStream,
    const CArchiveDatabaseEx *db,
    const CObjectVector<CUpdateItem> &updateItems,
    COutArchive &archive,
    CArchiveDatabase &newDatabase,
    ISequentialOutStream *seqOutStream,
    IArchiveUpdateCallback *updateCallback,
    const CUpdateOptions &options
    #ifndef _NO_CRYPTO
    , ICryptoGetTextPassword *getDecoderPassword
    #endif
    );
}}

#endif
