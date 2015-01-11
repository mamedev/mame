// 7zFolderInStream.h

#ifndef __7Z_FOLDER_IN_STREAM_H
#define __7Z_FOLDER_IN_STREAM_H

#include "../../ICoder.h"
#include "../IArchive.h"
#include "../Common/InStreamWithCRC.h"

#include "7zItem.h"

namespace NArchive {
namespace N7z {

class CFolderInStream:
  public ISequentialInStream,
  public ICompressGetSubStreamSize,
  public CMyUnknownImp
{
  CSequentialInStreamWithCRC *_inStreamWithHashSpec;
  CMyComPtr<ISequentialInStream> _inStreamWithHash;
  CMyComPtr<IArchiveUpdateCallback> _updateCallback;

  bool _currentSizeIsDefined;
  bool _fileIsOpen;
  UInt64 _currentSize;
  UInt64 _filePos;
  const UInt32 *_fileIndices;
  UInt32 _numFiles;
  UInt32 _fileIndex;

  HRESULT OpenStream();
  HRESULT CloseStream();
  void AddDigest();

public:
  CRecordVector<bool> Processed;
  CRecordVector<UInt32> CRCs;
  CRecordVector<UInt64> Sizes;

  MY_UNKNOWN_IMP1(ICompressGetSubStreamSize)
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
  STDMETHOD(GetSubStreamSize)(UInt64 subStream, UInt64 *value);

  CFolderInStream();
  void Init(IArchiveUpdateCallback *updateCallback, const UInt32 *fileIndices, UInt32 numFiles);
  UInt64 GetFullSize() const
  {
    UInt64 size = 0;
    for (int i = 0; i < Sizes.Size(); i++)
      size += Sizes[i];
    return size;
  }
};

}}

#endif
