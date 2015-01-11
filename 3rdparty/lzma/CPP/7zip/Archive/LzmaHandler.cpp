// LzmaHandler.cpp

#include "StdAfx.h"

#include "../../../C/CpuArch.h"

#include "Common/ComTry.h"
#include "Common/IntToString.h"

#include "Windows/PropVariant.h"

#include "../Common/CreateCoder.h"
#include "../Common/ProgressUtils.h"
#include "../Common/RegisterArc.h"
#include "../Common/StreamUtils.h"

#include "../Compress/LzmaDecoder.h"

#include "Common/DummyOutStream.h"

using namespace NWindows;

namespace NArchive {
namespace NLzma {

static bool CheckDicSize(const Byte *p)
{
  UInt32 dicSize = GetUi32(p);
  for (int i = 1; i <= 30; i++)
    if (dicSize == ((UInt32)2 << i) || dicSize == ((UInt32)3 << i))
      return true;
  return (dicSize == 0xFFFFFFFF);
}

STATPROPSTG kProps[] =
{
  { NULL, kpidSize, VT_UI8},
  { NULL, kpidPackSize, VT_UI8},
  { NULL, kpidMethod, VT_BSTR}
};

struct CHeader
{
  UInt64 Size;
  Byte FilterID;
  Byte LzmaProps[5];

  UInt32 GetDicSize() const { return GetUi32(LzmaProps + 1); }
  bool HasSize() const { return (Size != (UInt64)(Int64)-1); }
  bool Parse(const Byte *buf, bool isThereFilter);
};

bool CHeader::Parse(const Byte *buf, bool isThereFilter)
{
  FilterID = 0;
  if (isThereFilter)
    FilterID = buf[0];
  const Byte *sig = buf + (isThereFilter ? 1 : 0);
  for (int i = 0; i < 5; i++)
    LzmaProps[i] = sig[i];
  Size = GetUi64(sig + 5);
  return
    LzmaProps[0] < 5 * 5 * 9 &&
    FilterID < 2 &&
    (!HasSize() || Size < ((UInt64)1 << 56)) &&
    CheckDicSize(LzmaProps + 1);
}

class CDecoder
{
  NCompress::NLzma::CDecoder *_lzmaDecoderSpec;
  CMyComPtr<ICompressCoder> _lzmaDecoder;
  CMyComPtr<ISequentialOutStream> _bcjStream;
public:
  ~CDecoder();
  HRESULT Create(DECL_EXTERNAL_CODECS_LOC_VARS
      bool filtered, ISequentialInStream *inStream);

  HRESULT Code(const CHeader &header, ISequentialOutStream *outStream, ICompressProgressInfo *progress);

  UInt64 GetInputProcessedSize() const { return _lzmaDecoderSpec->GetInputProcessedSize(); }

  void ReleaseInStream() { if (_lzmaDecoder) _lzmaDecoderSpec->ReleaseInStream(); }

  HRESULT ReadInput(Byte *data, UInt32 size, UInt32 *processedSize)
    { return _lzmaDecoderSpec->ReadFromInputStream(data, size, processedSize); }
};

static const UInt64 k_BCJ = 0x03030103;
  
HRESULT CDecoder::Create(
    DECL_EXTERNAL_CODECS_LOC_VARS
    bool filteredMode, ISequentialInStream *inStream)
{
  if (!_lzmaDecoder)
  {
    _lzmaDecoderSpec = new NCompress::NLzma::CDecoder;
    _lzmaDecoder = _lzmaDecoderSpec;
  }

  if (filteredMode)
  {
    if (!_bcjStream)
    {
      CMyComPtr<ICompressCoder> coder;
      RINOK(CreateCoder(EXTERNAL_CODECS_LOC_VARS k_BCJ, coder, false));
      if (!coder)
        return E_NOTIMPL;
      coder.QueryInterface(IID_ISequentialOutStream, &_bcjStream);
      if (!_bcjStream)
        return E_NOTIMPL;
    }
  }

  return _lzmaDecoderSpec->SetInStream(inStream);
}

CDecoder::~CDecoder()
{
  ReleaseInStream();
}

HRESULT CDecoder::Code(const CHeader &header, ISequentialOutStream *outStream,
    ICompressProgressInfo *progress)
{
  if (header.FilterID > 1)
    return E_NOTIMPL;

  {
    CMyComPtr<ICompressSetDecoderProperties2> setDecoderProperties;
    _lzmaDecoder.QueryInterface(IID_ICompressSetDecoderProperties2, &setDecoderProperties);
    if (!setDecoderProperties)
      return E_NOTIMPL;
    RINOK(setDecoderProperties->SetDecoderProperties2(header.LzmaProps, 5));
  }

  CMyComPtr<ICompressSetOutStream> setOutStream;

  bool filteredMode = (header.FilterID == 1);

  if (filteredMode)
  {
    _bcjStream.QueryInterface(IID_ICompressSetOutStream, &setOutStream);
    if (!setOutStream)
      return E_NOTIMPL;
    RINOK(setOutStream->SetOutStream(outStream));
    outStream = _bcjStream;
  }

  const UInt64 *Size = header.HasSize() ? &header.Size : NULL;
  HRESULT res = _lzmaDecoderSpec->CodeResume(outStream, Size, progress);

  if (filteredMode)
  {
    CMyComPtr<IOutStreamFlush> flush;
    _bcjStream.QueryInterface(IID_IOutStreamFlush, &flush);
    if (flush)
    {
      HRESULT res2 = flush->Flush();
      if (res == S_OK)
        res = res2;
    }
    HRESULT res2 = setOutStream->ReleaseOutStream();
    if (res == S_OK)
      res = res2;
  }
  RINOK(res);

  return S_OK;
}


class CHandler:
  public IInArchive,
  public IArchiveOpenSeq,
  PUBLIC_ISetCompressCodecsInfo
  public CMyUnknownImp
{
  CHeader _header;
  bool _lzma86;
  UInt64 _startPosition;
  UInt64 _packSize;
  bool _packSizeDefined;
  CMyComPtr<IInStream> _stream;
  CMyComPtr<ISequentialInStream> _seqStream;

  DECL_EXTERNAL_CODECS_VARS
  DECL_ISetCompressCodecsInfo

public:
  MY_QUERYINTERFACE_BEGIN2(IInArchive)
  MY_QUERYINTERFACE_ENTRY(IArchiveOpenSeq)
  QUERY_ENTRY_ISetCompressCodecsInfo
  MY_QUERYINTERFACE_END
  MY_ADDREF_RELEASE

  INTERFACE_IInArchive(;)
  STDMETHOD(OpenSeq)(ISequentialInStream *stream);

  CHandler(bool lzma86) { _lzma86 = lzma86; }

  unsigned GetHeaderSize() const { return 5 + 8 + (_lzma86 ? 1 : 0); }

};

IMP_IInArchive_Props
IMP_IInArchive_ArcProps_NO_Table

STDMETHODIMP CHandler::GetArchiveProperty(PROPID propID, PROPVARIANT *value)
{
  NCOM::CPropVariant prop;
  switch(propID)
  {
    case kpidPhySize: if (_packSizeDefined) prop = _packSize; break;
  }
  prop.Detach(value);
  return S_OK;
}

STDMETHODIMP CHandler::GetNumberOfItems(UInt32 *numItems)
{
  *numItems = 1;
  return S_OK;
}

static void DictSizeToString(UInt32 value, char *s)
{
  for (int i = 0; i <= 31; i++)
    if ((UInt32(1) << i) == value)
    {
      ::ConvertUInt32ToString(i, s);
      return;
    }
  char c = 'b';
  if ((value & ((1 << 20) - 1)) == 0)
  {
    value >>= 20;
    c = 'm';
  }
  else if ((value & ((1 << 10) - 1)) == 0)
  {
    value >>= 10;
    c = 'k';
  }
  ::ConvertUInt32ToString(value, s);
  int p = MyStringLen(s);
  s[p++] = c;
  s[p++] = '\0';
}

static void MyStrCat(char *d, const char *s)
{
  MyStringCopy(d + MyStringLen(d), s);
}

STDMETHODIMP CHandler::GetProperty(UInt32 /* index */, PROPID propID,  PROPVARIANT *value)
{
  NCOM::CPropVariant prop;
  switch(propID)
  {
    case kpidSize: if (_stream && _header.HasSize()) prop = _header.Size; break;
    case kpidPackSize: if (_packSizeDefined) prop = _packSize; break;
    case kpidMethod:
      if (_stream)
      {
        char s[64];
        s[0] = '\0';
        if (_header.FilterID != 0)
          MyStrCat(s, "BCJ ");
        MyStrCat(s, "LZMA:");
        DictSizeToString(_header.GetDicSize(), s + MyStringLen(s));
        prop = s;
      }
      break;
  }
  prop.Detach(value);
  return S_OK;
}

STDMETHODIMP CHandler::Open(IInStream *inStream, const UInt64 *, IArchiveOpenCallback *)
{
  RINOK(inStream->Seek(0, STREAM_SEEK_CUR, &_startPosition));
  
  const UInt32 kBufSize = 1 + 5 + 8 + 1;
  Byte buf[kBufSize];
  
  RINOK(ReadStream_FALSE(inStream, buf, kBufSize));
  
  if (!_header.Parse(buf, _lzma86))
    return S_FALSE;
  const Byte *start = buf + GetHeaderSize();
  if (start[0] != 0)
    return S_FALSE;
  
  UInt64 endPos;
  RINOK(inStream->Seek(0, STREAM_SEEK_END, &endPos));
  _packSize = endPos - _startPosition;
  _packSizeDefined = true;
  if (_packSize >= 24 && _header.Size == 0 && _header.FilterID == 0 && _header.LzmaProps[0] == 0)
    return S_FALSE;
  _stream = inStream;
  _seqStream = inStream;
  return S_OK;
}

STDMETHODIMP CHandler::OpenSeq(ISequentialInStream *stream)
{
  Close();
  _seqStream = stream;
  return S_OK;
}

STDMETHODIMP CHandler::Close()
{
  _packSizeDefined = false;
  _stream.Release();
  _seqStream.Release();
   return S_OK;
}


STDMETHODIMP CHandler::Extract(const UInt32 *indices, UInt32 numItems,
    Int32 testMode, IArchiveExtractCallback *extractCallback)
{
  COM_TRY_BEGIN
  if (numItems == 0)
    return S_OK;
  if (numItems != (UInt32)-1 && (numItems != 1 || indices[0] != 0))
    return E_INVALIDARG;

  if (_stream)
    extractCallback->SetTotal(_packSize);
    
  
  CMyComPtr<ISequentialOutStream> realOutStream;
  Int32 askMode = testMode ?
      NExtract::NAskMode::kTest :
      NExtract::NAskMode::kExtract;
  RINOK(extractCallback->GetStream(0, &realOutStream, askMode));
  if (!testMode && !realOutStream)
    return S_OK;
  
  extractCallback->PrepareOperation(askMode);

  CDummyOutStream *outStreamSpec = new CDummyOutStream;
  CMyComPtr<ISequentialOutStream> outStream(outStreamSpec);
  outStreamSpec->SetStream(realOutStream);
  outStreamSpec->Init();
  realOutStream.Release();

  CLocalProgress *lps = new CLocalProgress;
  CMyComPtr<ICompressProgressInfo> progress = lps;
  lps->Init(extractCallback, true);

  if (_stream)
  {
    RINOK(_stream->Seek(_startPosition, STREAM_SEEK_SET, NULL));
  }

  CDecoder decoder;
  HRESULT result = decoder.Create(
      EXTERNAL_CODECS_VARS
      _lzma86, _seqStream);
  RINOK(result);
 
  Int32 opRes = NExtract::NOperationResult::kOK;
  bool firstItem = true;

  for (;;)
  {
    lps->OutSize = outStreamSpec->GetSize();
    lps->InSize = _packSize = decoder.GetInputProcessedSize();
    _packSizeDefined = true;
    RINOK(lps->SetCur());

    CHeader st;

    const UInt32 kBufSize = 1 + 5 + 8;
    Byte buf[kBufSize];
    const UInt32 headerSize = GetHeaderSize();
    UInt32 processed;
    RINOK(decoder.ReadInput(buf, headerSize, &processed));
    if (processed != headerSize)
      break;
  
    if (!st.Parse(buf, _lzma86))
      break;
    firstItem = false;

    result = decoder.Code(st, outStream, progress);
    if (result == E_NOTIMPL)
    {
      opRes = NExtract::NOperationResult::kUnSupportedMethod;
      break;
    }
    if (result == S_FALSE)
    {
      opRes = NExtract::NOperationResult::kDataError;
      break;
    }
    RINOK(result);
  }
  if (firstItem)
    return E_FAIL;
  outStream.Release();
  return extractCallback->SetOperationResult(opRes);
  COM_TRY_END
}

IMPL_ISetCompressCodecsInfo

static IInArchive *CreateArc() { return new CHandler(false); }
static IInArchive *CreateArc86() { return new CHandler(true); }

namespace NLzmaAr {
  
static CArcInfo g_ArcInfo =
  { L"lzma", L"lzma", 0, 0xA, { 0 }, 0, true, CreateArc, NULL };
REGISTER_ARC(Lzma)

}

namespace NLzma86Ar {

static CArcInfo g_ArcInfo =
  { L"lzma86", L"lzma86", 0, 0xB, { 0 }, 0, true, CreateArc86, NULL };
REGISTER_ARC(Lzma86)

}

}}
