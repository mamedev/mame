// XzHandler.cpp

#include "StdAfx.h"

#include "../../../C/Alloc.h"
#include "../../../C/XzCrc64.h"
#include "../../../C/XzEnc.h"

#include "../../Common/ComTry.h"
#include "../../Common/IntToString.h"
#include "../../Common/StringConvert.h"

#include "../ICoder.h"

#include "../Common/CWrappers.h"
#include "../Common/ProgressUtils.h"
#include "../Common/RegisterArc.h"
#include "../Common/StreamUtils.h"

#include "../Compress/CopyCoder.h"

#include "IArchive.h"

#include "Common/HandlerOut.h"

using namespace NWindows;

namespace NCompress {
namespace NLzma2 {

HRESULT SetLzma2Prop(PROPID propID, const PROPVARIANT &prop, CLzma2EncProps &lzma2Props);

}}

static void *SzAlloc(void *, size_t size) { return MyAlloc(size); }
static void SzFree(void *, void *address) { MyFree(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

namespace NArchive {
namespace NXz {

struct CCrc64Gen { CCrc64Gen() { Crc64GenerateTable(); } } g_Crc64TableInit;

static const wchar_t *k_LZMA2_Name = L"LZMA2";

class CHandler:
  public IInArchive,
  public IArchiveOpenSeq,
  #ifndef EXTRACT_ONLY
  public IOutArchive,
  public ISetProperties,
  public CMultiMethodProps,
  #endif
  public CMyUnknownImp
{
  Int64 _startPosition;
  UInt64 _packSize;
  UInt64 _unpackSize;
  UInt64 _numBlocks;
  AString _methodsString;
  bool _useSeq;
  UInt64 _unpackSizeDefined;
  UInt64 _packSizeDefined;
  
  CMyComPtr<IInStream> _stream;
  CMyComPtr<ISequentialInStream> _seqStream;

  UInt32 _filterId;

  void Init()
  {
    _filterId = 0;
    CMultiMethodProps::Init();
  }

  HRESULT Open2(IInStream *inStream, IArchiveOpenCallback *callback);

public:
  MY_QUERYINTERFACE_BEGIN2(IInArchive)
  MY_QUERYINTERFACE_ENTRY(IArchiveOpenSeq)
  #ifndef EXTRACT_ONLY
  MY_QUERYINTERFACE_ENTRY(IOutArchive)
  MY_QUERYINTERFACE_ENTRY(ISetProperties)
  #endif
  MY_QUERYINTERFACE_END
  MY_ADDREF_RELEASE

  INTERFACE_IInArchive(;)
  STDMETHOD(OpenSeq)(ISequentialInStream *stream);

  #ifndef EXTRACT_ONLY
  INTERFACE_IOutArchive(;)
  STDMETHOD(SetProperties)(const wchar_t **names, const PROPVARIANT *values, Int32 numProps);
  #endif

  CHandler();
};

CHandler::CHandler()
{
  Init();
}

static STATPROPSTG const kProps[] =
{
  { NULL, kpidSize, VT_UI8},
  { NULL, kpidPackSize, VT_UI8},
  { NULL, kpidMethod, VT_BSTR}
};

static STATPROPSTG const kArcProps[] =
{
  { NULL, kpidMethod, VT_BSTR},
  { NULL, kpidNumBlocks, VT_UI4}
};

IMP_IInArchive_Props
IMP_IInArchive_ArcProps

static char GetHex(Byte value)
{
  return (char)((value < 10) ? ('0' + value) : ('A' + (value - 10)));
}

static inline void AddHexToString(AString &res, Byte value)
{
  res += GetHex((Byte)(value >> 4));
  res += GetHex((Byte)(value & 0xF));
}

static AString ConvertUInt32ToString(UInt32 value)
{
  char temp[32];
  ::ConvertUInt32ToString(value, temp);
  return temp;
}

static AString Lzma2PropToString(int prop)
{
  if ((prop & 1) == 0)
    return ConvertUInt32ToString(prop / 2 + 12);
  AString res;
  char c;

  UInt32 size = (2 | ((prop) & 1)) << ((prop) / 2 + 1);

  if (prop > 17)
  {
    res = ConvertUInt32ToString(size >> 10);
    c = 'm';
  }
  else
  {
    res = ConvertUInt32ToString(size);
    c = 'k';
  }
  return res + c;
}

struct CMethodNamePair
{
  UInt32 Id;
  const char *Name;
};

static const CMethodNamePair g_NamePairs[] =
{
  { XZ_ID_Subblock, "SB" },
  { XZ_ID_Delta, "Delta" },
  { XZ_ID_X86, "BCJ" },
  { XZ_ID_PPC, "PPC" },
  { XZ_ID_IA64, "IA64" },
  { XZ_ID_ARM, "ARM" },
  { XZ_ID_ARMT, "ARMT" },
  { XZ_ID_SPARC, "SPARC" },
  { XZ_ID_LZMA2, "LZMA2" }
};

static AString GetMethodString(const CXzFilter &f)
{
  AString s;

  for (int i = 0; i < sizeof(g_NamePairs) / sizeof(g_NamePairs[i]); i++)
    if (g_NamePairs[i].Id == f.id)
      s = g_NamePairs[i].Name;
  if (s.IsEmpty())
  {
    char temp[32];
    ::ConvertUInt64ToString(f.id, temp);
    s = temp;
  }

  if (f.propsSize > 0)
  {
    s += ':';
    if (f.id == XZ_ID_LZMA2 && f.propsSize == 1)
      s += Lzma2PropToString(f.props[0]);
    else if (f.id == XZ_ID_Delta && f.propsSize == 1)
      s += ConvertUInt32ToString((UInt32)f.props[0] + 1);
    else
    {
      s += '[';
      for (UInt32 bi = 0; bi < f.propsSize; bi++)
        AddHexToString(s, f.props[bi]);
      s += ']';
    }
  }
  return s;
}

static void AddString(AString &dest, const AString &src)
{
  if (!dest.IsEmpty())
    dest += ' ';
  dest += src;
}

static const char *kChecks[] =
{
  "NoCheck",
  "CRC32",
  NULL,
  NULL,
  "CRC64",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  "SHA256",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static AString GetCheckString(const CXzs &xzs)
{
  size_t i;
  UInt32 mask = 0;
  for (i = 0; i < xzs.num; i++)
    mask |= ((UInt32)1 << XzFlags_GetCheckType(xzs.streams[i].flags));
  AString s;
  for (i = 0; i <= XZ_CHECK_MASK; i++)
    if (((mask >> i) & 1) != 0)
    {
      AString s2;
      if (kChecks[i])
        s2 = kChecks[i];
      else
        s2 = "Check-" + ConvertUInt32ToString((UInt32)i);
      AddString(s, s2);
    }
  return s;
}

STDMETHODIMP CHandler::GetArchiveProperty(PROPID propID, PROPVARIANT *value)
{
  COM_TRY_BEGIN
  NWindows::NCOM::CPropVariant prop;
  switch(propID)
  {
    case kpidNumBlocks: if (!_useSeq) prop = _numBlocks; break;
    case kpidPhySize: if (_packSizeDefined) prop = _packSize; break;
    case kpidMethod: if (!_methodsString.IsEmpty()) prop = _methodsString; break;
  }
  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CHandler::GetNumberOfItems(UInt32 *numItems)
{
  *numItems = 1;
  return S_OK;
}

STDMETHODIMP CHandler::GetProperty(UInt32, PROPID propID,  PROPVARIANT *value)
{
  COM_TRY_BEGIN
  NWindows::NCOM::CPropVariant prop;
  switch(propID)
  {
    case kpidSize: if (_unpackSizeDefined) prop = _unpackSize; break;
    case kpidPackSize: if (_packSizeDefined) prop = _packSize; break;
    case kpidMethod: if (!_methodsString.IsEmpty()) prop = _methodsString; break;
  }
  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}


struct COpenCallbackWrap
{
  ICompressProgress p;
  IArchiveOpenCallback *OpenCallback;
  HRESULT Res;
  COpenCallbackWrap(IArchiveOpenCallback *progress);
};

static SRes OpenCallbackProgress(void *pp, UInt64 inSize, UInt64 /* outSize */)
{
  COpenCallbackWrap *p = (COpenCallbackWrap *)pp;
  p->Res = p->OpenCallback->SetCompleted(NULL, &inSize);
  return (SRes)p->Res;
}

COpenCallbackWrap::COpenCallbackWrap(IArchiveOpenCallback *callback)
{
  p.Progress = OpenCallbackProgress;
  OpenCallback = callback;
  Res = SZ_OK;
}

struct CXzsCPP
{
  CXzs p;
  CXzsCPP() { Xzs_Construct(&p); }
  ~CXzsCPP() { Xzs_Free(&p, &g_Alloc); }
};

HRESULT CHandler::Open2(IInStream *inStream, IArchiveOpenCallback *callback)
{
  CSeekInStreamWrap inStreamImp(inStream);

  CLookToRead lookStream;
  LookToRead_CreateVTable(&lookStream, True);
  lookStream.realStream = &inStreamImp.p;
  LookToRead_Init(&lookStream);

  COpenCallbackWrap openWrap(callback);
  RINOK(inStream->Seek(0, STREAM_SEEK_END, &_packSize));
  RINOK(callback->SetTotal(NULL, &_packSize));

  CXzsCPP xzs;
  SRes res = Xzs_ReadBackward(&xzs.p, &lookStream.s, &_startPosition, &openWrap.p, &g_Alloc);
  if (res == SZ_ERROR_NO_ARCHIVE && xzs.p.num > 0)
    res = SZ_OK;
  if (res == SZ_OK)
  {
    _packSize -= _startPosition;
    _unpackSize = Xzs_GetUnpackSize(&xzs.p);
    _unpackSizeDefined = _packSizeDefined = true;
    _numBlocks = (UInt64)Xzs_GetNumBlocks(&xzs.p);

    RINOK(inStream->Seek(0, STREAM_SEEK_SET, NULL));
    CXzStreamFlags st;
    CSeqInStreamWrap inStreamWrap(inStream);
    SRes res2 = Xz_ReadHeader(&st, &inStreamWrap.p);

    if (res2 == SZ_OK)
    {
      CXzBlock block;
      Bool isIndex;
      UInt32 headerSizeRes;
      res2 = XzBlock_ReadHeader(&block, &inStreamWrap.p, &isIndex, &headerSizeRes);
      if (res2 == SZ_OK && !isIndex)
      {
        int numFilters = XzBlock_GetNumFilters(&block);
        for (int i = 0; i < numFilters; i++)
          AddString(_methodsString, GetMethodString(block.filters[i]));
      }
    }
    AddString(_methodsString, GetCheckString(xzs.p));
  }

  if (res != SZ_OK || _startPosition != 0)
  {
    RINOK(inStream->Seek(0, STREAM_SEEK_SET, NULL));
    CXzStreamFlags st;
    CSeqInStreamWrap inStreamWrap(inStream);
    SRes res2 = Xz_ReadHeader(&st, &inStreamWrap.p);
    if (res2 == SZ_OK)
    {
      res = res2;
      _startPosition = 0;
      _useSeq = True;
      _unpackSizeDefined = _packSizeDefined = false;
    }
  }
  if (res == SZ_ERROR_NO_ARCHIVE)
    return S_FALSE;
  RINOK(SResToHRESULT(res));
  _stream = inStream;
  _seqStream = inStream;
  return S_OK;
}

STDMETHODIMP CHandler::Open(IInStream *inStream, const UInt64 *, IArchiveOpenCallback *callback)
{
  COM_TRY_BEGIN
  try
  {
    Close();
    return Open2(inStream, callback);
  }
  catch(...) { return S_FALSE; }
  COM_TRY_END
}

STDMETHODIMP CHandler::OpenSeq(ISequentialInStream *stream)
{
  Close();
  _seqStream = stream;
  return S_OK;
}

STDMETHODIMP CHandler::Close()
{
  _numBlocks = 0;
  _useSeq = true;
  _unpackSizeDefined = _packSizeDefined = false;
  _methodsString.Empty();
  _stream.Release();
  _seqStream.Release();
  return S_OK;
}

class CSeekToSeqStream:
  public IInStream,
  public CMyUnknownImp
{
public:
  CMyComPtr<ISequentialInStream> Stream;
  MY_UNKNOWN_IMP1(IInStream)

  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
};

STDMETHODIMP CSeekToSeqStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  return Stream->Read(data, size, processedSize);
}

STDMETHODIMP CSeekToSeqStream::Seek(Int64, UInt32, UInt64 *) { return E_NOTIMPL; }

struct CXzUnpackerCPP
{
  Byte *InBuf;
  Byte *OutBuf;
  CXzUnpacker p;
  CXzUnpackerCPP(): InBuf(0), OutBuf(0)
  {
    XzUnpacker_Construct(&p, &g_Alloc);
  }
  ~CXzUnpackerCPP()
  {
    XzUnpacker_Free(&p);
    MyFree(InBuf);
    MyFree(OutBuf);
  }
};

STDMETHODIMP CHandler::Extract(const UInt32 *indices, UInt32 numItems,
    Int32 testMode, IArchiveExtractCallback *extractCallback)
{
  COM_TRY_BEGIN
  if (numItems == 0)
    return S_OK;
  if (numItems != (UInt32)-1 && (numItems != 1 || indices[0] != 0))
    return E_INVALIDARG;

  extractCallback->SetTotal(_packSize);
  UInt64 currentTotalPacked = 0;
  RINOK(extractCallback->SetCompleted(&currentTotalPacked));
  CMyComPtr<ISequentialOutStream> realOutStream;
  Int32 askMode = testMode ?
      NExtract::NAskMode::kTest :
      NExtract::NAskMode::kExtract;
  
  RINOK(extractCallback->GetStream(0, &realOutStream, askMode));
  
  if (!testMode && !realOutStream)
    return S_OK;

  extractCallback->PrepareOperation(askMode);

  if (_stream)
  {
    RINOK(_stream->Seek(_startPosition, STREAM_SEEK_SET, NULL));
  }

  CLocalProgress *lps = new CLocalProgress;
  CMyComPtr<ICompressProgressInfo> progress = lps;
  lps->Init(extractCallback, true);

  CCompressProgressWrap progressWrap(progress);

  SRes res = S_OK;

  const UInt32 kInBufSize = 1 << 15;
  const UInt32 kOutBufSize = 1 << 21;

  UInt32 inPos = 0;
  UInt32 inSize = 0;
  UInt32 outPos = 0;
  CXzUnpackerCPP xzu;
  XzUnpacker_Init(&xzu.p);
  {
    xzu.InBuf = (Byte *)MyAlloc(kInBufSize);
    xzu.OutBuf = (Byte *)MyAlloc(kOutBufSize);
    if (xzu.InBuf == 0 || xzu.OutBuf == 0)
      res = SZ_ERROR_MEM;
  }
  if (res == SZ_OK)
  for (;;)
  {
    if (inPos == inSize)
    {
      inPos = inSize = 0;
      RINOK(_seqStream->Read(xzu.InBuf, kInBufSize, &inSize));
    }

    SizeT inLen = inSize - inPos;
    SizeT outLen = kOutBufSize - outPos;
    ECoderStatus status;
    res = XzUnpacker_Code(&xzu.p,
        xzu.OutBuf + outPos, &outLen,
        xzu.InBuf + inPos, &inLen,
        (inSize == 0 ? CODER_FINISH_END : CODER_FINISH_ANY), &status);

    // printf("\n_inPos = %6d  inLen = %5d, outLen = %5d", inPos, inLen, outLen);

    inPos += (UInt32)inLen;
    outPos += (UInt32)outLen;
    lps->InSize += inLen;
    lps->OutSize += outLen;

    bool finished = (((inLen == 0) && (outLen == 0)) || res != SZ_OK);

    if (outPos == kOutBufSize || finished)
    {
      if (realOutStream && outPos > 0)
      {
        RINOK(WriteStream(realOutStream, xzu.OutBuf, outPos));
      }
      outPos = 0;
    }
    RINOK(lps->SetCur());
    if (finished)
    {
      _packSize = lps->InSize;
      _unpackSize = lps->OutSize;
      _packSizeDefined = _unpackSizeDefined = true;
      if (res == SZ_OK)
      {
        if (status == CODER_STATUS_NEEDS_MORE_INPUT)
        {
          if (XzUnpacker_IsStreamWasFinished(&xzu.p))
            _packSize -= xzu.p.padSize;
          else
            res = SZ_ERROR_DATA;
        }
        else
          res = SZ_ERROR_DATA;
      }
      break;
    }
  }

  Int32 opRes;
  switch(res)
  {
    case SZ_OK:
      opRes = NExtract::NOperationResult::kOK; break;
    case SZ_ERROR_UNSUPPORTED:
      opRes = NExtract::NOperationResult::kUnSupportedMethod; break;
    case SZ_ERROR_CRC:
      opRes = NExtract::NOperationResult::kCRCError; break;
    case SZ_ERROR_DATA:
    case SZ_ERROR_ARCHIVE:
    case SZ_ERROR_NO_ARCHIVE:
      opRes = NExtract::NOperationResult::kDataError; break;
    default:
      return SResToHRESULT(res);
  }
  realOutStream.Release();
  return extractCallback->SetOperationResult(opRes);
  COM_TRY_END
}

#ifndef EXTRACT_ONLY

STDMETHODIMP CHandler::GetFileTimeType(UInt32 *timeType)
{
  *timeType = NFileTimeType::kUnix;
  return S_OK;
}

STDMETHODIMP CHandler::UpdateItems(ISequentialOutStream *outStream, UInt32 numItems,
    IArchiveUpdateCallback *updateCallback)
{
  CSeqOutStreamWrap seqOutStream(outStream);
  
  if (numItems == 0)
  {
    SRes res = Xz_EncodeEmpty(&seqOutStream.p);
    return SResToHRESULT(res);
  }
  
  if (numItems != 1)
    return E_INVALIDARG;

  Int32 newData, newProps;
  UInt32 indexInArchive;
  if (!updateCallback)
    return E_FAIL;
  RINOK(updateCallback->GetUpdateItemInfo(0, &newData, &newProps, &indexInArchive));

  if (IntToBool(newProps))
  {
    {
      NCOM::CPropVariant prop;
      RINOK(updateCallback->GetProperty(0, kpidIsDir, &prop));
      if (prop.vt != VT_EMPTY)
        if (prop.vt != VT_BOOL || prop.boolVal != VARIANT_FALSE)
          return E_INVALIDARG;
    }
  }

  if (IntToBool(newData))
  {
    UInt64 size;
    {
      NCOM::CPropVariant prop;
      RINOK(updateCallback->GetProperty(0, kpidSize, &prop));
      if (prop.vt != VT_UI8)
        return E_INVALIDARG;
      size = prop.uhVal.QuadPart;
      RINOK(updateCallback->SetTotal(size));
    }

    CLzma2EncProps lzma2Props;
    Lzma2EncProps_Init(&lzma2Props);

    lzma2Props.lzmaProps.level = GetLevel();

    CMyComPtr<ISequentialInStream> fileInStream;
    RINOK(updateCallback->GetStream(0, &fileInStream));

    CSeqInStreamWrap seqInStream(fileInStream);

    {
      NCOM::CPropVariant prop = (UInt64)size;
      RINOK(NCompress::NLzma2::SetLzma2Prop(NCoderPropID::kReduceSize, prop, lzma2Props));
    }

    for (int i = 0; i < _methods.Size(); i++)
    {
      COneMethodInfo &m = _methods[i];
      SetGlobalLevelAndThreads(m
      #ifndef _7ZIP_ST
      , _numThreads
      #endif
      );
      {
        for (int j = 0; j < m.Props.Size(); j++)
        {
          const CProp &prop = m.Props[j];
          RINOK(NCompress::NLzma2::SetLzma2Prop(prop.Id, prop.Value, lzma2Props));
        }
      }
    }

    #ifndef _7ZIP_ST
    lzma2Props.numTotalThreads = _numThreads;
    #endif

    CLocalProgress *lps = new CLocalProgress;
    CMyComPtr<ICompressProgressInfo> progress = lps;
    lps->Init(updateCallback, true);

    CCompressProgressWrap progressWrap(progress);
    CXzProps xzProps;
    CXzFilterProps filter;
    XzProps_Init(&xzProps);
    XzFilterProps_Init(&filter);
    xzProps.lzma2Props = &lzma2Props;
    xzProps.filterProps = (_filterId != 0 ? &filter : NULL);
    switch (_crcSize)
    {
      case  0: xzProps.checkId = XZ_CHECK_NO; break;
      case  4: xzProps.checkId = XZ_CHECK_CRC32; break;
      case  8: xzProps.checkId = XZ_CHECK_CRC64; break;
      case 32: xzProps.checkId = XZ_CHECK_SHA256; break;
      default: return E_INVALIDARG;
    }
    filter.id = _filterId;
    if (_filterId == XZ_ID_Delta)
    {
      bool deltaDefined = false;
      for (int j = 0; j < _filterMethod.Props.Size(); j++)
      {
        const CProp &prop = _filterMethod.Props[j];
        if (prop.Id == NCoderPropID::kDefaultProp && prop.Value.vt == VT_UI4)
        {
          UInt32 delta = (UInt32)prop.Value.ulVal;
          if (delta < 1 || delta > 256)
            return E_INVALIDARG;
          filter.delta = delta;
          deltaDefined = true;
        }
      }
      if (!deltaDefined)
        return E_INVALIDARG;
    }
    SRes res = Xz_Encode(&seqOutStream.p, &seqInStream.p, &xzProps, &progressWrap.p);
    if (res == SZ_OK)
      return updateCallback->SetOperationResult(NArchive::NUpdate::NOperationResult::kOK);
    return SResToHRESULT(res);
  }
  if (indexInArchive != 0)
    return E_INVALIDARG;
  if (_stream)
    RINOK(_stream->Seek(_startPosition, STREAM_SEEK_SET, NULL));
  return NCompress::CopyStream(_stream, outStream, 0);
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

STDMETHODIMP CHandler::SetProperties(const wchar_t **names, const PROPVARIANT *values, Int32 numProps)
{
  COM_TRY_BEGIN
  Init();
  for (int i = 0; i < numProps; i++)
  {
    RINOK(SetProperty(names[i], values[i]));
  }

  if (!_filterMethod.MethodName.IsEmpty())
  {
    int k;
    for (k = 0; k < ARRAY_SIZE(g_NamePairs); k++)
    {
      const CMethodNamePair &pair = g_NamePairs[k];
      UString m = GetUnicodeString(pair.Name);
      if (_filterMethod.MethodName.CompareNoCase(m) == 0)
      {
        _filterId = pair.Id;
        break;
      }
    }
    if (k == ARRAY_SIZE(g_NamePairs))
      return E_INVALIDARG;
  }

  int numEmptyMethods = GetNumEmptyMethods();
  _methods.Delete(0, numEmptyMethods);
  if (_methods.Size() > 1)
    return E_INVALIDARG;
  if (_methods.Size() == 1)
  {
    UString &methodName = _methods[0].MethodName;
    if (methodName.IsEmpty())
      methodName = k_LZMA2_Name;
    else if (methodName.CompareNoCase(k_LZMA2_Name) != 0)
      return E_INVALIDARG;
  }
  return S_OK;
  COM_TRY_END
}

#endif

static IInArchive *CreateArc() { return new NArchive::NXz::CHandler; }
#ifndef EXTRACT_ONLY
static IOutArchive *CreateArcOut() { return new NArchive::NXz::CHandler; }
#else
#define CreateArcOut 0
#endif

static CArcInfo g_ArcInfo =
  { L"xz", L"xz txz", L"* .tar", 0xC, {0xFD, '7' , 'z', 'X', 'Z', '\0'}, 6, true, CreateArc, CreateArcOut };

REGISTER_ARC(xz)

}}
