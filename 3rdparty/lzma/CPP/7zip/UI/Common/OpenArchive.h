// OpenArchive.h

#ifndef __OPEN_ARCHIVE_H
#define __OPEN_ARCHIVE_H

#include "Common/MyString.h"

#include "Windows/FileFind.h"

#include "../../Archive/IArchive.h"

#include "ArchiveOpenCallback.h"
#include "LoadCodecs.h"

HRESULT GetArchiveItemBoolProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result);
HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result);

struct CArc
{
  CMyComPtr<IInArchive> Archive;
  UString Path;
  UString DefaultName;
  int FormatIndex;
  int SubfileIndex;
  FILETIME MTime;
  bool MTimeDefined;
  UString ErrorMessage;

  CArc(): MTimeDefined(false) {}

  HRESULT GetItemPath(UInt32 index, UString &result) const;
  HRESULT GetItemSize(UInt32 index, UInt64 &size, bool &defined) const;
  HRESULT GetItemMTime(UInt32 index, FILETIME &ft, bool &defined) const;
  HRESULT IsItemAnti(UInt32 index, bool &result) const
    { return GetArchiveItemBoolProp(Archive, index, kpidIsAnti, result); }

  HRESULT OpenStream(
    CCodecs *codecs,
    int formatIndex,
    IInStream *stream,
    ISequentialInStream *seqStream,
    IArchiveOpenCallback *callback);

  HRESULT OpenStreamOrFile(
    CCodecs *codecs,
    int formatIndex,
    bool stdInMode,
    IInStream *stream,
    IArchiveOpenCallback *callback);
};

struct CArchiveLink
{
  CObjectVector<CArc> Arcs;
  UStringVector VolumePaths;
  UInt64 VolumesSize;
  bool IsOpen;

  CArchiveLink(): VolumesSize(0), IsOpen(false) {}
  HRESULT Close();
  void Release();
  ~CArchiveLink() { Release(); }

  IInArchive *GetArchive() const { return Arcs.Back().Archive; }

  HRESULT Open(
    CCodecs *codecs,
    const CIntVector &formatIndices,
    bool stdInMode,
    IInStream *stream,
    const UString &filePath,
    IArchiveOpenCallback *callback);

  HRESULT Open2(
    CCodecs *codecs,
    const CIntVector &formatIndices,
    bool stdInMode,
    IInStream *stream,
    const UString &filePath,
    IOpenCallbackUI *callbackUI);

  HRESULT ReOpen(
    CCodecs *codecs,
    const UString &filePath,
    IArchiveOpenCallback *callback);
};

#endif
