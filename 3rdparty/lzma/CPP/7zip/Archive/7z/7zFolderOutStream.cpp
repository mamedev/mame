// 7zFolderOutStream.cpp

#include "StdAfx.h"

#include "7zFolderOutStream.h"

namespace NArchive {
namespace N7z {

CFolderOutStream::CFolderOutStream()
{
  _crcStreamSpec = new COutStreamWithCRC;
  _crcStream = _crcStreamSpec;
}

HRESULT CFolderOutStream::Init(
    const CArchiveDatabaseEx *db,
    UInt32 ref2Offset, UInt32 startIndex,
    const CBoolVector *extractStatuses,
    IArchiveExtractCallback *extractCallback,
    bool testMode, bool checkCrc)
{
  _db = db;
  _ref2Offset = ref2Offset;
  _startIndex = startIndex;

  _extractStatuses = extractStatuses;
  _extractCallback = extractCallback;
  _testMode = testMode;
  _checkCrc = checkCrc;

  _currentIndex = 0;
  _fileIsOpen = false;
  return ProcessEmptyFiles();
}

HRESULT CFolderOutStream::OpenFile()
{
  Int32 askMode = ((*_extractStatuses)[_currentIndex]) ? (_testMode ?
      NExtract::NAskMode::kTest :
      NExtract::NAskMode::kExtract) :
      NExtract::NAskMode::kSkip;
  CMyComPtr<ISequentialOutStream> realOutStream;
  UInt32 index = _startIndex + _currentIndex;
  RINOK(_extractCallback->GetStream(_ref2Offset + index, &realOutStream, askMode));
  _crcStreamSpec->SetStream(realOutStream);
  _crcStreamSpec->Init(_checkCrc);
  _fileIsOpen = true;
  const CFileItem &fi = _db->Files[index];
  _rem = fi.Size;
  if (askMode == NExtract::NAskMode::kExtract && !realOutStream &&
      !_db->IsItemAnti(index) && !fi.IsDir)
    askMode = NExtract::NAskMode::kSkip;
  return _extractCallback->PrepareOperation(askMode);
}

HRESULT CFolderOutStream::CloseFileAndSetResult(Int32 res)
{
  _crcStreamSpec->ReleaseStream();
  _fileIsOpen = false;
  _currentIndex++;
  return _extractCallback->SetOperationResult(res);
}

HRESULT CFolderOutStream::CloseFileAndSetResult()
{
  const CFileItem &fi = _db->Files[_startIndex + _currentIndex];
  return CloseFileAndSetResult(
      (fi.IsDir || !fi.CrcDefined || !_checkCrc || fi.Crc == _crcStreamSpec->GetCRC()) ?
      NExtract::NOperationResult::kOK :
      NExtract::NOperationResult::kCRCError);
}

HRESULT CFolderOutStream::ProcessEmptyFiles()
{
  while (_currentIndex < _extractStatuses->Size() && _db->Files[_startIndex + _currentIndex].Size == 0)
  {
    RINOK(OpenFile());
    RINOK(CloseFileAndSetResult());
  }
  return S_OK;
}

STDMETHODIMP CFolderOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != NULL)
    *processedSize = 0;
  while (size != 0)
  {
    if (_fileIsOpen)
    {
      UInt32 cur = size < _rem ? size : (UInt32)_rem;
      RINOK(_crcStream->Write(data, cur, &cur));
      if (cur == 0)
        break;
      data = (const Byte *)data + cur;
      size -= cur;
      _rem -= cur;
      if (processedSize != NULL)
        *processedSize += cur;
      if (_rem == 0)
      {
        RINOK(CloseFileAndSetResult());
        RINOK(ProcessEmptyFiles());
        continue;
      }
    }
    else
    {
      RINOK(ProcessEmptyFiles());
      if (_currentIndex == _extractStatuses->Size())
      {
        // we support partial extracting
        if (processedSize != NULL)
          *processedSize += size;
        break;
      }
      RINOK(OpenFile());
    }
  }
  return S_OK;
}

STDMETHODIMP CFolderOutStream::GetSubStreamSize(UInt64 subStream, UInt64 *value)
{
  *value = 0;
  if ((int)subStream >= _extractStatuses->Size())
    return S_FALSE;
  *value = _db->Files[_startIndex + (int)subStream].Size;
  return S_OK;
}

HRESULT CFolderOutStream::FlushCorrupted(Int32 resultEOperationResult)
{
  while (_currentIndex < _extractStatuses->Size())
  {
    if (_fileIsOpen)
    {
      RINOK(CloseFileAndSetResult(resultEOperationResult));
    }
    else
    {
      RINOK(OpenFile());
    }
  }
  return S_OK;
}

}}
