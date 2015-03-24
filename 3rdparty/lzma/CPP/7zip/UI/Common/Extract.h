// Extract.h

#ifndef __EXTRACT_H
#define __EXTRACT_H

#include "Windows/FileFind.h"

#include "../../Archive/IArchive.h"

#include "ArchiveExtractCallback.h"
#include "ArchiveOpenCallback.h"
#include "ExtractMode.h"
#include "Property.h"

#include "../Common/LoadCodecs.h"

struct CExtractOptions
{
  bool StdInMode;
  bool StdOutMode;
  bool YesToAll;
  bool TestMode;
  bool CalcCrc;
  NExtract::NPathMode::EEnum PathMode;
  NExtract::NOverwriteMode::EEnum OverwriteMode;
  FString OutputDir;
  
  // bool ShowDialog;
  // bool PasswordEnabled;
  // UString Password;
  #if !defined(_7ZIP_ST) && !defined(_SFX)
  CObjectVector<CProperty> Properties;
  #endif

  #ifdef EXTERNAL_CODECS
  CCodecs *Codecs;
  #endif

  CExtractOptions():
      StdInMode(false),
      StdOutMode(false),
      YesToAll(false),
      TestMode(false),
      CalcCrc(false),
      PathMode(NExtract::NPathMode::kFullPathnames),
      OverwriteMode(NExtract::NOverwriteMode::kAskBefore)
      {}
};

struct CDecompressStat
{
  UInt64 NumArchives;
  UInt64 UnpackSize;
  UInt64 PackSize;
  UInt64 NumFolders;
  UInt64 NumFiles;
  UInt32 CrcSum;

  void Clear()
  {
    NumArchives = UnpackSize = PackSize = NumFolders = NumFiles = 0;
    CrcSum = 0;
  }
};

HRESULT DecompressArchives(
    CCodecs *codecs, const CIntVector &formatIndices,
    UStringVector &archivePaths, UStringVector &archivePathsFull,
    const NWildcard::CCensorNode &wildcardCensor,
    const CExtractOptions &options,
    IOpenCallbackUI *openCallback,
    IExtractCallbackUI *extractCallback,
    UString &errorMessage,
    CDecompressStat &stat);

#endif
