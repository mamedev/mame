// Extract.cpp

#include "StdAfx.h"

#include <stdio.h>

#include "Windows/FileDir.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"

#include "../Common/ExtractingFilePath.h"

#include "Extract.h"
#include "SetProperties.h"

using namespace NWindows;

static HRESULT DecompressArchive(
    const CArc &arc,
    UInt64 packSize,
    const NWildcard::CCensorNode &wildcardCensor,
    const CExtractOptions &options,
    IExtractCallbackUI *callback,
    CArchiveExtractCallback *extractCallbackSpec,
    UString &errorMessage,
    UInt64 &stdInProcessed)
{
  stdInProcessed = 0;
  IInArchive *archive = arc.Archive;
  CRecordVector<UInt32> realIndices;
  if (!options.StdInMode)
  {
    UInt32 numItems;
    RINOK(archive->GetNumberOfItems(&numItems));
    
    for (UInt32 i = 0; i < numItems; i++)
    {
      UString filePath;
      RINOK(arc.GetItemPath(i, filePath));
      bool isFolder;
      RINOK(IsArchiveItemFolder(archive, i, isFolder));
      if (!wildcardCensor.CheckPath(filePath, !isFolder))
        continue;
      realIndices.Add(i);
    }
    if (realIndices.Size() == 0)
    {
      callback->ThereAreNoFiles();
      return S_OK;
    }
  }

  UStringVector removePathParts;

  FString outDir = options.OutputDir;
  outDir.Replace(FSTRING_ANY_MASK, us2fs(GetCorrectFsPath(arc.DefaultName)));
  #ifdef _WIN32
  // GetCorrectFullFsPath doesn't like "..".
  // outDir.TrimRight();
  // outDir = GetCorrectFullFsPath(outDir);
  #endif

  if (!outDir.IsEmpty())
    if (!NFile::NDirectory::CreateComplexDirectory(outDir))
    {
      HRESULT res = ::GetLastError();
      if (res == S_OK)
        res = E_FAIL;
      errorMessage = ((UString)L"Can not create output directory ") + fs2us(outDir);
      return res;
    }

  extractCallbackSpec->Init(
      options.StdInMode ? &wildcardCensor : NULL,
      &arc,
      callback,
      options.StdOutMode, options.TestMode, options.CalcCrc,
      outDir,
      removePathParts,
      packSize);

  #if !defined(_7ZIP_ST) && !defined(_SFX)
  RINOK(SetProperties(archive, options.Properties));
  #endif

  HRESULT result;
  Int32 testMode = (options.TestMode && !options.CalcCrc) ? 1: 0;
  if (options.StdInMode)
  {
    result = archive->Extract(NULL, (UInt32)(Int32)-1, testMode, extractCallbackSpec);
    NCOM::CPropVariant prop;
    if (archive->GetArchiveProperty(kpidPhySize, &prop) == S_OK)
      if (prop.vt == VT_UI8 || prop.vt == VT_UI4)
        stdInProcessed = ConvertPropVariantToUInt64(prop);
  }
  else
    result = archive->Extract(&realIndices.Front(), realIndices.Size(), testMode, extractCallbackSpec);

  return callback->ExtractResult(result);
}

HRESULT DecompressArchives(
    CCodecs *codecs, const CIntVector &formatIndices,
    UStringVector &arcPaths, UStringVector &arcPathsFull,
    const NWildcard::CCensorNode &wildcardCensor,
    const CExtractOptions &options,
    IOpenCallbackUI *openCallback,
    IExtractCallbackUI *extractCallback,
    UString &errorMessage,
    CDecompressStat &stat)
{
  stat.Clear();
  int i;
  UInt64 totalPackSize = 0;
  CRecordVector<UInt64> archiveSizes;

  int numArcs = options.StdInMode ? 1 : arcPaths.Size();

  for (i = 0; i < numArcs; i++)
  {
    NFile::NFind::CFileInfo fi;
    fi.Size = 0;
    if (!options.StdInMode)
    {
      const FString &arcPath = us2fs(arcPaths[i]);
      if (!fi.Find(arcPath))
        throw "there is no such archive";
      if (fi.IsDir())
        throw "can't decompress folder";
    }
    archiveSizes.Add(fi.Size);
    totalPackSize += fi.Size;
  }
  CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
  CMyComPtr<IArchiveExtractCallback> ec(extractCallbackSpec);
  bool multi = (numArcs > 1);
  extractCallbackSpec->InitForMulti(multi, options.PathMode, options.OverwriteMode);
  if (multi)
  {
    RINOK(extractCallback->SetTotal(totalPackSize));
  }
  for (i = 0; i < numArcs; i++)
  {
    const UString &arcPath = arcPaths[i];
    NFile::NFind::CFileInfo fi;
    if (options.StdInMode)
    {
      fi.Size = 0;
      fi.Attrib = 0;
    }
    else
    {
      if (!fi.Find(us2fs(arcPath)) || fi.IsDir())
        throw "there is no such archive";
    }

    #ifndef _NO_CRYPTO
    openCallback->Open_ClearPasswordWasAskedFlag();
    #endif

    RINOK(extractCallback->BeforeOpen(arcPath));
    CArchiveLink archiveLink;

    CIntVector formatIndices2 = formatIndices;
    #ifndef _SFX
    if (formatIndices.IsEmpty())
    {
      int pos = arcPath.ReverseFind(L'.');
      if (pos >= 0)
      {
        UString s = arcPath.Mid(pos + 1);
        int index = codecs->FindFormatForExtension(s);
        if (index >= 0 && s == L"001")
        {
          s = arcPath.Left(pos);
          pos = s.ReverseFind(L'.');
          if (pos >= 0)
          {
            int index2 = codecs->FindFormatForExtension(s.Mid(pos + 1));
            if (index2 >= 0 && s.CompareNoCase(L"rar") != 0)
            {
              formatIndices2.Add(index2);
              formatIndices2.Add(index);
            }
          }
        }
      }
    }
    #endif
    HRESULT result = archiveLink.Open2(codecs, formatIndices2, options.StdInMode, NULL, arcPath, openCallback);
    if (result == E_ABORT)
      return result;

    bool crypted = false;
    #ifndef _NO_CRYPTO
    crypted = openCallback->Open_WasPasswordAsked();
    #endif

    RINOK(extractCallback->OpenResult(arcPath, result, crypted));
    if (result != S_OK)
      continue;

    if (!options.StdInMode)
    for (int v = 0; v < archiveLink.VolumePaths.Size(); v++)
    {
      int index = arcPathsFull.FindInSorted(archiveLink.VolumePaths[v]);
      if (index >= 0 && index > i)
      {
        arcPaths.Delete(index);
        arcPathsFull.Delete(index);
        totalPackSize -= archiveSizes[index];
        archiveSizes.Delete(index);
        numArcs = arcPaths.Size();
      }
    }
    if (archiveLink.VolumePaths.Size() != 0)
    {
      totalPackSize += archiveLink.VolumesSize;
      RINOK(extractCallback->SetTotal(totalPackSize));
    }

    #ifndef _NO_CRYPTO
    UString password;
    RINOK(openCallback->Open_GetPasswordIfAny(password));
    if (!password.IsEmpty())
    {
      RINOK(extractCallback->SetPassword(password));
    }
    #endif

    for (int v = 0; v < archiveLink.Arcs.Size(); v++)
    {
      const UString &s = archiveLink.Arcs[v].ErrorMessage;
      if (!s.IsEmpty())
      {
        RINOK(extractCallback->MessageError(s));
      }
    }

    CArc &arc = archiveLink.Arcs.Back();
    arc.MTimeDefined = (!options.StdInMode && !fi.IsDevice);
    arc.MTime = fi.MTime;

    UInt64 packProcessed;
    RINOK(DecompressArchive(arc,
        fi.Size + archiveLink.VolumesSize,
        wildcardCensor, options, extractCallback, extractCallbackSpec, errorMessage, packProcessed));
    if (!options.StdInMode)
      packProcessed = fi.Size + archiveLink.VolumesSize;
    extractCallbackSpec->LocalProgressSpec->InSize += packProcessed;
    extractCallbackSpec->LocalProgressSpec->OutSize = extractCallbackSpec->UnpackSize;
    if (!errorMessage.IsEmpty())
      return E_FAIL;
  }
  stat.NumFolders = extractCallbackSpec->NumFolders;
  stat.NumFiles = extractCallbackSpec->NumFiles;
  stat.UnpackSize = extractCallbackSpec->UnpackSize;
  stat.CrcSum = extractCallbackSpec->CrcSum;

  stat.NumArchives = arcPaths.Size();
  stat.PackSize = extractCallbackSpec->LocalProgressSpec->InSize;
  return S_OK;
}
