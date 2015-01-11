// 7zFolderInStream.cpp

#include "StdAfx.h"

#include "7zFolderInStream.h"

namespace NArchive {
namespace N7z {

CFolderInStream::CFolderInStream()
{
  _inStreamWithHashSpec = new CSequentialInStreamWithCRC;
  _inStreamWithHash = _inStreamWithHashSpec;
}

void CFolderInStream::Init(IArchiveUpdateCallback *updateCallback,
    const UInt32 *fileIndices, UInt32 numFiles)
{
  _updateCallback = updateCallback;
  _numFiles = numFiles;
  _fileIndex = 0;
  _fileIndices = fileIndices;
  Processed.Clear();
  CRCs.Clear();
  Sizes.Clear();
  _fileIsOpen = false;
  _currentSizeIsDefined = false;
}

HRESULT CFolderInStream::OpenStream()
{
  _filePos = 0;
  while (_fileIndex < _numFiles)
  {
    CMyComPtr<ISequentialInStream> stream;
    HRESULT result = _updateCallback->GetStream(_fileIndices[_fileIndex], &stream);
    if (result != S_OK && result != S_FALSE)
      return result;
    _fileIndex++;
    _inStreamWithHashSpec->SetStream(stream);
    _inStreamWithHashSpec->Init();
    if (stream)
    {
      _fileIsOpen = true;
      CMyComPtr<IStreamGetSize> streamGetSize;
      stream.QueryInterface(IID_IStreamGetSize, &streamGetSize);
      if (streamGetSize)
      {
        RINOK(streamGetSize->GetSize(&_currentSize));
        _currentSizeIsDefined = true;
      }
      return S_OK;
    }
    RINOK(_updateCallback->SetOperationResult(NArchive::NUpdate::NOperationResult::kOK));
    Sizes.Add(0);
    Processed.Add(result == S_OK);
    AddDigest();
  }
  return S_OK;
}

void CFolderInStream::AddDigest()
{
  CRCs.Add(_inStreamWithHashSpec->GetCRC());
}

HRESULT CFolderInStream::CloseStream()
{
  RINOK(_updateCallback->SetOperationResult(NArchive::NUpdate::NOperationResult::kOK));
  _inStreamWithHashSpec->ReleaseStream();
  _fileIsOpen = false;
  _currentSizeIsDefined = false;
  Processed.Add(true);
  Sizes.Add(_filePos);
  AddDigest();
  return S_OK;
}

STDMETHODIMP CFolderInStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != 0)
    *processedSize = 0;
  while (size > 0)
  {
    if (_fileIsOpen)
    {
      UInt32 processed2;
      RINOK(_inStreamWithHash->Read(data, size, &processed2));
      if (processed2 == 0)
      {
        RINOK(CloseStream());
        continue;
      }
      if (processedSize != 0)
        *processedSize = processed2;
      _filePos += processed2;
      break;
    }
    if (_fileIndex >= _numFiles)
      break;
    RINOK(OpenStream());
  }
  return S_OK;
}

STDMETHODIMP CFolderInStream::GetSubStreamSize(UInt64 subStream, UInt64 *value)
{
  *value = 0;
  int index2 = (int)subStream;
  if (index2 < 0 || subStream > Sizes.Size())
    return E_FAIL;
  if (index2 < Sizes.Size())
  {
    *value = Sizes[index2];
    return S_OK;
  }
  if (!_currentSizeIsDefined)
    return S_FALSE;
  *value = _currentSize;
  return S_OK;
}

}}
