// Bench.cpp

#include "StdAfx.h"

#ifndef _WIN32
#define USE_POSIX_TIME
#define USE_POSIX_TIME2
#endif

#ifdef USE_POSIX_TIME
#include <time.h>
#ifdef USE_POSIX_TIME2
#include <sys/time.h>
#endif
#endif

#ifdef _WIN32
#define USE_ALLOCA
#endif

#ifdef USE_ALLOCA
#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#endif

#include "../../../../C/7zCrc.h"
#include "../../../../C/Alloc.h"

#if !defined(_7ZIP_ST) || defined(_WIN32)
#include "../../../Windows/System.h"
#endif

#ifndef _7ZIP_ST
#include "../../../Windows/Synchronization.h"
#include "../../../Windows/Thread.h"
#endif

#include "../../../Common/IntToString.h"
#include "../../../Common/StringConvert.h"
#include "../../../Common/StringToInt.h"

#include "../../Common/MethodProps.h"

#include "Bench.h"

using namespace NWindows;

static const UInt64 kUncompressMinBlockSize =
#ifdef UNDER_CE
(UInt64)1 << 30;
#else
(UInt64)1 << 33;
#endif

static const UInt32 kCrcBlockSize =
#ifdef UNDER_CE
1 << 25;
#else
1 << 30;
#endif

static const unsigned kOldLzmaDictBits = 30;

static const UInt32 kAdditionalSize = (1 << 16);
static const UInt32 kCompressedAdditionalSize = (1 << 10);
static const UInt32 kMaxLzmaPropSize = 5;

class CBaseRandomGenerator
{
  UInt32 A1;
  UInt32 A2;
public:
  CBaseRandomGenerator() { Init(); }
  void Init() { A1 = 362436069; A2 = 521288629;}
  UInt32 GetRnd()
  {
    return
      ((A1 = 36969 * (A1 & 0xffff) + (A1 >> 16)) << 16) +
      ((A2 = 18000 * (A2 & 0xffff) + (A2 >> 16)) );
  }
};

class CBenchBuffer
{
public:
  size_t BufferSize;
  Byte *Buffer;
  CBenchBuffer(): Buffer(0) {}
  virtual ~CBenchBuffer() { Free(); }
  void Free()
  {
    ::MidFree(Buffer);
    Buffer = 0;
  }
  bool Alloc(size_t bufferSize)
  {
    if (Buffer != 0 && BufferSize == bufferSize)
      return true;
    Free();
    Buffer = (Byte *)::MidAlloc(bufferSize);
    BufferSize = bufferSize;
    return (Buffer != 0);
  }
};

class CBenchRandomGenerator: public CBenchBuffer
{
  CBaseRandomGenerator *RG;
public:
  void Set(CBaseRandomGenerator *rg) { RG = rg; }
  UInt32 GetVal(UInt32 &res, unsigned numBits)
  {
    UInt32 val = res & (((UInt32)1 << numBits) - 1);
    res >>= numBits;
    return val;
  }
  UInt32 GetLen(UInt32 &res)
  {
    UInt32 len = GetVal(res, 2);
    return GetVal(res, 1 + len);
  }
  void Generate(unsigned dictBits)
  {
    UInt32 pos = 0;
    UInt32 rep0 = 1;
    while (pos < BufferSize)
    {
      UInt32 res = RG->GetRnd();
      res >>= 1;
      if (GetVal(res, 1) == 0 || pos < 1024)
        Buffer[pos++] = (Byte)(res & 0xFF);
      else
      {
        UInt32 len;
        len = 1 + GetLen(res);
        if (GetVal(res, 3) != 0)
        {
          len += GetLen(res);
          do
          {
            UInt32 ppp = GetVal(res, 5) + 6;
            res = RG->GetRnd();
            if (ppp > dictBits)
              continue;
            rep0 = /* (1 << ppp) +*/  GetVal(res, ppp);
            res = RG->GetRnd();
          }
          while (rep0 >= pos);
          rep0++;
        }

        for (UInt32 i = 0; i < len && pos < BufferSize; i++, pos++)
          Buffer[pos] = Buffer[pos - rep0];
      }
    }
  }
};


class CBenchmarkInStream:
  public ISequentialInStream,
  public CMyUnknownImp
{
  const Byte *Data;
  size_t Pos;
  size_t Size;
public:
  MY_UNKNOWN_IMP
  void Init(const Byte *data, size_t size)
  {
    Data = data;
    Size = size;
    Pos = 0;
  }
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
};

STDMETHODIMP CBenchmarkInStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  size_t remain = Size - Pos;
  UInt32 kMaxBlockSize = (1 << 20);
  if (size > kMaxBlockSize)
    size = kMaxBlockSize;
  if (size > remain)
    size = (UInt32)remain;
  for (UInt32 i = 0; i < size; i++)
    ((Byte *)data)[i] = Data[Pos + i];
  Pos += size;
  if(processedSize != NULL)
    *processedSize = size;
  return S_OK;
}
  
class CBenchmarkOutStream:
  public ISequentialOutStream,
  public CBenchBuffer,
  public CMyUnknownImp
{
  // bool _overflow;
public:
  UInt32 Pos;
  // CBenchmarkOutStream(): _overflow(false) {}
  void Init()
  {
    // _overflow = false;
    Pos = 0;
  }
  MY_UNKNOWN_IMP
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

STDMETHODIMP CBenchmarkOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  size_t curSize = BufferSize - Pos;
  if (curSize > size)
    curSize = size;
  memcpy(Buffer + Pos, data, curSize);
  Pos += (UInt32)curSize;
  if(processedSize != NULL)
    *processedSize = (UInt32)curSize;
  if (curSize != size)
  {
    // _overflow = true;
    return E_FAIL;
  }
  return S_OK;
}
  
class CCrcOutStream:
  public ISequentialOutStream,
  public CMyUnknownImp
{
public:
  UInt32 Crc;
  MY_UNKNOWN_IMP
  void Init() { Crc = CRC_INIT_VAL; }
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

STDMETHODIMP CCrcOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  Crc = CrcUpdate(Crc, data, size);
  if (processedSize != NULL)
    *processedSize = size;
  return S_OK;
}
  
static UInt64 GetTimeCount()
{
  #ifdef USE_POSIX_TIME
  #ifdef USE_POSIX_TIME2
  timeval v;
  if (gettimeofday(&v, 0) == 0)
    return (UInt64)(v.tv_sec) * 1000000 + v.tv_usec;
  return (UInt64)time(NULL) * 1000000;
  #else
  return time(NULL);
  #endif
  #else
  /*
  LARGE_INTEGER value;
  if (::QueryPerformanceCounter(&value))
    return value.QuadPart;
  */
  return GetTickCount();
  #endif
}

static UInt64 GetFreq()
{
  #ifdef USE_POSIX_TIME
  #ifdef USE_POSIX_TIME2
  return 1000000;
  #else
  return 1;
  #endif
  #else
  /*
  LARGE_INTEGER value;
  if (::QueryPerformanceFrequency(&value))
    return value.QuadPart;
  */
  return 1000;
  #endif
}

#ifdef USE_POSIX_TIME

struct CUserTime
{
  UInt64 Sum;
  clock_t Prev;
  
  void Init()
  {
    Prev = clock();
    Sum = 0;
  }

  UInt64 GetUserTime()
  {
    clock_t v = clock();
    Sum += v - Prev;
    Prev = v;
    return Sum;
  }
};

#else

static inline UInt64 GetTime64(const FILETIME &t) { return ((UInt64)t.dwHighDateTime << 32) | t.dwLowDateTime; }
UInt64 GetWinUserTime()
{
  FILETIME creationTime, exitTime, kernelTime, userTime;
  if (
  #ifdef UNDER_CE
    ::GetThreadTimes(::GetCurrentThread()
  #else
    ::GetProcessTimes(::GetCurrentProcess()
  #endif
    , &creationTime, &exitTime, &kernelTime, &userTime) != 0)
    return GetTime64(userTime) + GetTime64(kernelTime);
  return (UInt64)GetTickCount() * 10000;
}

struct CUserTime
{
  UInt64 StartTime;

  void Init() { StartTime = GetWinUserTime(); }
  UInt64 GetUserTime() { return GetWinUserTime() - StartTime; }
};

#endif

static UInt64 GetUserFreq()
{
  #ifdef USE_POSIX_TIME
  return CLOCKS_PER_SEC;
  #else
  return 10000000;
  #endif
}

class CBenchProgressStatus
{
  #ifndef _7ZIP_ST
  NSynchronization::CCriticalSection CS;
  #endif
public:
  HRESULT Res;
  bool EncodeMode;
  void SetResult(HRESULT res)
  {
    #ifndef _7ZIP_ST
    NSynchronization::CCriticalSectionLock lock(CS);
    #endif
    Res = res;
  }
  HRESULT GetResult()
  {
    #ifndef _7ZIP_ST
    NSynchronization::CCriticalSectionLock lock(CS);
    #endif
    return Res;
  }
};

class CBenchProgressInfo:
  public ICompressProgressInfo,
  public CMyUnknownImp
{
public:
  CBenchProgressStatus *Status;
  CBenchInfo BenchInfo;
  CUserTime UserTime;
  HRESULT Res;
  IBenchCallback *Callback;

  CBenchProgressInfo(): Callback(0) {}
  void SetStartTime();
  void SetFinishTime(CBenchInfo &dest);
  MY_UNKNOWN_IMP
  STDMETHOD(SetRatioInfo)(const UInt64 *inSize, const UInt64 *outSize);
};

void CBenchProgressInfo::SetStartTime()
{
  BenchInfo.GlobalFreq = GetFreq();
  BenchInfo.UserFreq = GetUserFreq();
  BenchInfo.GlobalTime = ::GetTimeCount();
  BenchInfo.UserTime = 0;
  UserTime.Init();
}

void CBenchProgressInfo::SetFinishTime(CBenchInfo &dest)
{
  dest = BenchInfo;
  dest.GlobalTime = ::GetTimeCount() - BenchInfo.GlobalTime;
  dest.UserTime = UserTime.GetUserTime();
}

STDMETHODIMP CBenchProgressInfo::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize)
{
  HRESULT res = Status->GetResult();
  if (res != S_OK)
    return res;
  if (!Callback)
    return res;
  CBenchInfo info;
  SetFinishTime(info);
  if (Status->EncodeMode)
  {
    info.UnpackSize = *inSize;
    info.PackSize = *outSize;
    res = Callback->SetEncodeResult(info, false);
  }
  else
  {
    info.PackSize = BenchInfo.PackSize + *inSize;
    info.UnpackSize = BenchInfo.UnpackSize + *outSize;
    res = Callback->SetDecodeResult(info, false);
  }
  if (res != S_OK)
    Status->SetResult(res);
  return res;
}

static const int kSubBits = 8;

static UInt32 GetLogSize(UInt32 size)
{
  for (int i = kSubBits; i < 32; i++)
    for (UInt32 j = 0; j < (1 << kSubBits); j++)
      if (size <= (((UInt32)1) << i) + (j << (i - kSubBits)))
        return (i << kSubBits) + j;
  return (32 << kSubBits);
}

static void NormalizeVals(UInt64 &v1, UInt64 &v2)
{
  while (v1 > 1000000)
  {
    v1 >>= 1;
    v2 >>= 1;
  }
}

UInt64 CBenchInfo::GetUsage() const
{
  UInt64 userTime = UserTime;
  UInt64 userFreq = UserFreq;
  UInt64 globalTime = GlobalTime;
  UInt64 globalFreq = GlobalFreq;
  NormalizeVals(userTime, userFreq);
  NormalizeVals(globalFreq, globalTime);
  if (userFreq == 0)
    userFreq = 1;
  if (globalTime == 0)
    globalTime = 1;
  return userTime * globalFreq * 1000000 / userFreq / globalTime;
}

UInt64 CBenchInfo::GetRatingPerUsage(UInt64 rating) const
{
  UInt64 userTime = UserTime;
  UInt64 userFreq = UserFreq;
  UInt64 globalTime = GlobalTime;
  UInt64 globalFreq = GlobalFreq;
  NormalizeVals(userFreq, userTime);
  NormalizeVals(globalTime, globalFreq);
  if (globalFreq == 0)
    globalFreq = 1;
  if (userTime == 0)
    userTime = 1;
  return userFreq * globalTime / globalFreq * rating / userTime;
}

static UInt64 MyMultDiv64(UInt64 value, UInt64 elapsedTime, UInt64 freq)
{
  UInt64 elTime = elapsedTime;
  NormalizeVals(freq, elTime);
  if (elTime == 0)
    elTime = 1;
  return value * freq / elTime;
}

struct CBenchProps
{
  bool LzmaRatingMode;
  
  UInt32 EncComplex;
  UInt32 DecComplexCompr;
  UInt32 DecComplexUnc;

  CBenchProps(): LzmaRatingMode(false) {}
  void SetLzmaCompexity();

  UInt64 GeDecomprCommands(UInt64 packSize, UInt64 unpackSize)
  {
    return (packSize * DecComplexCompr + unpackSize * DecComplexUnc);
  }

  UInt64 GetCompressRating(UInt32 dictSize, UInt64 elapsedTime, UInt64 freq, UInt64 size);
  UInt64 GetDecompressRating(UInt64 elapsedTime, UInt64 freq, UInt64 outSize, UInt64 inSize, UInt32 numIterations);
};

void CBenchProps::SetLzmaCompexity()
{
  DecComplexUnc = 4;
  DecComplexCompr = 200;
  LzmaRatingMode = true;
}

UInt64 CBenchProps::GetCompressRating(UInt32 dictSize, UInt64 elapsedTime, UInt64 freq, UInt64 size)
{
  if (dictSize < (1 << kBenchMinDicLogSize))
    dictSize = (1 << kBenchMinDicLogSize);
  UInt64 encComplex = EncComplex;
  if (LzmaRatingMode)
  {
    UInt64 t = GetLogSize(dictSize) - (kBenchMinDicLogSize << kSubBits);
    encComplex = 870 + ((t * t * 5) >> (2 * kSubBits));
  }
  UInt64 numCommands = (UInt64)size * encComplex;
  return MyMultDiv64(numCommands, elapsedTime, freq);
}

UInt64 CBenchProps::GetDecompressRating(UInt64 elapsedTime, UInt64 freq, UInt64 outSize, UInt64 inSize, UInt32 numIterations)
{
  UInt64 numCommands = (inSize * DecComplexCompr + outSize * DecComplexUnc) * numIterations;
  return MyMultDiv64(numCommands, elapsedTime, freq);
}

UInt64 GetCompressRating(UInt32 dictSize, UInt64 elapsedTime, UInt64 freq, UInt64 size)
{
  CBenchProps props;
  props.SetLzmaCompexity();
  return props.GetCompressRating(dictSize, elapsedTime, freq, size);
}

UInt64 GetDecompressRating(UInt64 elapsedTime, UInt64 freq, UInt64 outSize, UInt64 inSize, UInt32 numIterations)
{
  CBenchProps props;
  props.SetLzmaCompexity();
  return props.GetDecompressRating(elapsedTime, freq, outSize, inSize, numIterations);
}

struct CEncoderInfo;

struct CEncoderInfo
{
  #ifndef _7ZIP_ST
  NWindows::CThread thread[2];
  UInt32 NumDecoderSubThreads;
  #endif
  CMyComPtr<ICompressCoder> encoder;
  CBenchProgressInfo *progressInfoSpec[2];
  CMyComPtr<ICompressProgressInfo> progressInfo[2];
  UInt32 NumIterations;
  #ifdef USE_ALLOCA
  size_t AllocaSize;
  #endif

  struct CDecoderInfo
  {
    CEncoderInfo *Encoder;
    UInt32 DecoderIndex;
    #ifdef USE_ALLOCA
    size_t AllocaSize;
    #endif
    bool CallbackMode;
  };
  CDecoderInfo decodersInfo[2];

  CMyComPtr<ICompressCoder> decoders[2];
  HRESULT Results[2];
  CBenchmarkOutStream *outStreamSpec;
  CMyComPtr<ISequentialOutStream> outStream;
  IBenchCallback *callback;
  IBenchPrintCallback *printCallback;
  UInt32 crc;
  UInt32 kBufferSize;
  UInt32 compressedSize;
  CBenchRandomGenerator rg;
  CBenchmarkOutStream *propStreamSpec;
  CMyComPtr<ISequentialOutStream> propStream;
  HRESULT Init(
      const COneMethodInfo &method,
      UInt32 uncompressedDataSize,
      unsigned generateDictBits,
      CBaseRandomGenerator *rg);
  HRESULT Encode();
  HRESULT Decode(UInt32 decoderIndex);

  CEncoderInfo(): outStreamSpec(0), callback(0), printCallback(0), propStreamSpec(0) {}

  #ifndef _7ZIP_ST
  static THREAD_FUNC_DECL EncodeThreadFunction(void *param)
  {
    CEncoderInfo *encoder = (CEncoderInfo *)param;
    #ifdef USE_ALLOCA
    alloca(encoder->AllocaSize);
    #endif
    HRESULT res = encoder->Encode();
    encoder->Results[0] = res;
    if (res != S_OK)
      encoder->progressInfoSpec[0]->Status->SetResult(res);

    return 0;
  }
  static THREAD_FUNC_DECL DecodeThreadFunction(void *param)
  {
    CDecoderInfo *decoder = (CDecoderInfo *)param;
    #ifdef USE_ALLOCA
    alloca(decoder->AllocaSize);
    #endif
    CEncoderInfo *encoder = decoder->Encoder;
    encoder->Results[decoder->DecoderIndex] = encoder->Decode(decoder->DecoderIndex);
    return 0;
  }

  HRESULT CreateEncoderThread()
  {
    return thread[0].Create(EncodeThreadFunction, this);
  }

  HRESULT CreateDecoderThread(int index, bool callbackMode
      #ifdef USE_ALLOCA
      , size_t allocaSize
      #endif
      )
  {
    CDecoderInfo &decoder = decodersInfo[index];
    decoder.DecoderIndex = index;
    decoder.Encoder = this;
    #ifdef USE_ALLOCA
    decoder.AllocaSize = allocaSize;
    #endif
    decoder.CallbackMode = callbackMode;
    return thread[index].Create(DecodeThreadFunction, &decoder);
  }
  #endif
};

static const UInt32 k_LZMA  = 0x030101;

HRESULT CEncoderInfo::Init(
    const COneMethodInfo &method,
    UInt32 uncompressedDataSize,
    unsigned generateDictBits,
    CBaseRandomGenerator *rgLoc)
{
  rg.Set(rgLoc);
  kBufferSize = uncompressedDataSize;
  UInt32 kCompressedBufferSize = (kBufferSize - kBufferSize / 4) + kCompressedAdditionalSize;
  if (!rg.Alloc(kBufferSize))
    return E_OUTOFMEMORY;
  rg.Generate(generateDictBits);
  crc = CrcCalc(rg.Buffer, rg.BufferSize);

  outStreamSpec = new CBenchmarkOutStream;
  if (!outStreamSpec->Alloc(kCompressedBufferSize))
    return E_OUTOFMEMORY;

  outStream = outStreamSpec;

  propStreamSpec = 0;
  if (!propStream)
  {
    propStreamSpec = new CBenchmarkOutStream;
    propStream = propStreamSpec;
  }
  if (!propStreamSpec->Alloc(kMaxLzmaPropSize))
    return E_OUTOFMEMORY;
  propStreamSpec->Init();
  
 
  {
    CMyComPtr<ICompressSetCoderProperties> scp;
    encoder.QueryInterface(IID_ICompressSetCoderProperties, &scp);
    if (scp)
    {
      UInt64 reduceSize = uncompressedDataSize;
      RINOK(method.SetCoderProps(scp, &reduceSize));
    }
    else
    {
      if (method.AreThereNonOptionalProps())
        return E_FAIL;
    }

    CMyComPtr<ICompressWriteCoderProperties> writeCoderProps;
    encoder.QueryInterface(IID_ICompressWriteCoderProperties, &writeCoderProps);
    if (writeCoderProps)
    {
      RINOK(writeCoderProps->WriteCoderProperties(propStream));
    }
  }
  return S_OK;
}

HRESULT CEncoderInfo::Encode()
{
  CBenchmarkInStream *inStreamSpec = new CBenchmarkInStream;
  CMyComPtr<ISequentialInStream> inStream = inStreamSpec;
  inStreamSpec->Init(rg.Buffer, rg.BufferSize);
  outStreamSpec->Init();

  RINOK(encoder->Code(inStream, outStream, 0, 0, progressInfo[0]));
  compressedSize = outStreamSpec->Pos;
  encoder.Release();
  return S_OK;
}

HRESULT CEncoderInfo::Decode(UInt32 decoderIndex)
{
  CBenchmarkInStream *inStreamSpec = new CBenchmarkInStream;
  CMyComPtr<ISequentialInStream> inStream = inStreamSpec;
  CMyComPtr<ICompressCoder> &decoder = decoders[decoderIndex];

  CMyComPtr<ICompressSetDecoderProperties2> setDecProps;
  decoder.QueryInterface(IID_ICompressSetDecoderProperties2, &setDecProps);
  if (!setDecProps && propStreamSpec->Pos != 0)
    return E_FAIL;

  CCrcOutStream *crcOutStreamSpec = new CCrcOutStream;
  CMyComPtr<ISequentialOutStream> crcOutStream = crcOutStreamSpec;
    
  CBenchProgressInfo *pi = progressInfoSpec[decoderIndex];
  pi->BenchInfo.UnpackSize = 0;
  pi->BenchInfo.PackSize = 0;

  #ifndef _7ZIP_ST
  {
    CMyComPtr<ICompressSetCoderMt> setCoderMt;
    decoder.QueryInterface(IID_ICompressSetCoderMt, &setCoderMt);
    if (setCoderMt)
    {
      RINOK(setCoderMt->SetNumberOfThreads(NumDecoderSubThreads));
    }
  }
  #endif

  for (UInt32 j = 0; j < NumIterations; j++)
  {
    if (printCallback)
    {
      RINOK(printCallback->CheckBreak());
    }
    inStreamSpec->Init(outStreamSpec->Buffer, compressedSize);
    crcOutStreamSpec->Init();
    
    if (setDecProps)
    {
      RINOK(setDecProps->SetDecoderProperties2(propStreamSpec->Buffer, propStreamSpec->Pos));
    }
    UInt64 outSize = kBufferSize;
    RINOK(decoder->Code(inStream, crcOutStream, 0, &outSize, progressInfo[decoderIndex]));
    if (CRC_GET_DIGEST(crcOutStreamSpec->Crc) != crc)
      return S_FALSE;
    pi->BenchInfo.UnpackSize += kBufferSize;
    pi->BenchInfo.PackSize += compressedSize;
  }
  decoder.Release();
  return S_OK;
}

static const UInt32 kNumThreadsMax = (1 << 12);

struct CBenchEncoders
{
  CEncoderInfo *encoders;
  CBenchEncoders(UInt32 num): encoders(0) { encoders = new CEncoderInfo[num]; }
  ~CBenchEncoders() { delete []encoders; }
};

static HRESULT MethodBench(
    DECL_EXTERNAL_CODECS_LOC_VARS
    bool oldLzmaBenchMode,
    UInt32 numThreads,
    const COneMethodInfo &method2,
    UInt32 uncompressedDataSize,
    unsigned generateDictBits,
    IBenchPrintCallback *printCallback,
    IBenchCallback *callback,
    CBenchProps *benchProps)
{
  COneMethodInfo method = method2;
  UInt64 methodId;
  UInt32 numInStreams, numOutStreams;
  if (!FindMethod(
      EXTERNAL_CODECS_LOC_VARS
      method.MethodName, methodId, numInStreams, numOutStreams))
    return E_NOTIMPL;
  if (numInStreams != 1 || numOutStreams != 1)
    return E_INVALIDARG;

  UInt32 numEncoderThreads = 1;
  UInt32 numSubDecoderThreads = 1;
  
  #ifndef _7ZIP_ST
    numEncoderThreads = numThreads;

    if (oldLzmaBenchMode && methodId == k_LZMA)
    {
      bool fixedNumber;
      UInt32 numLzmaThreads = method.Get_Lzma_NumThreads(fixedNumber);
      if (!fixedNumber && numThreads == 1)
        method.AddNumThreadsProp(1);
      if (numThreads > 1 && numLzmaThreads > 1)
      {
        numEncoderThreads = numThreads / 2;
        numSubDecoderThreads = 2;
      }
    }
  #endif

  if (numThreads < 1 || numEncoderThreads > kNumThreadsMax)
    return E_INVALIDARG;

  CBenchEncoders encodersSpec(numEncoderThreads);
  CEncoderInfo *encoders = encodersSpec.encoders;

  UInt32 i;
  for (i = 0; i < numEncoderThreads; i++)
  {
    CEncoderInfo &encoder = encoders[i];
    encoder.callback = (i == 0) ? callback : 0;
    encoder.printCallback = printCallback;

    RINOK(CreateCoder(EXTERNAL_CODECS_LOC_VARS methodId, encoder.encoder, true));
    if (!encoder.encoder)
      return E_NOTIMPL;
    for (UInt32 j = 0; j < numSubDecoderThreads; j++)
    {
      RINOK(CreateCoder(EXTERNAL_CODECS_LOC_VARS methodId, encoder.decoders[j], false));
      if (!encoder.decoders[j])
        return E_NOTIMPL;
    }
  }

  CBaseRandomGenerator rg;
  rg.Init();
  for (i = 0; i < numEncoderThreads; i++)
  {
    RINOK(encoders[i].Init(method, uncompressedDataSize, generateDictBits, &rg));
  }

  CBenchProgressStatus status;
  status.Res = S_OK;
  status.EncodeMode = true;

  for (i = 0; i < numEncoderThreads; i++)
  {
    CEncoderInfo &encoder = encoders[i];
    for (int j = 0; j < 2; j++)
    {
      encoder.progressInfo[j] = encoder.progressInfoSpec[j] = new CBenchProgressInfo;
      encoder.progressInfoSpec[j]->Status = &status;
    }
    if (i == 0)
    {
      CBenchProgressInfo *bpi = encoder.progressInfoSpec[0];
      bpi->Callback = callback;
      bpi->BenchInfo.NumIterations = numEncoderThreads;
      bpi->SetStartTime();
    }

    #ifndef _7ZIP_ST
    if (numEncoderThreads > 1)
    {
      #ifdef USE_ALLOCA
      encoder.AllocaSize = (i * 16 * 21) & 0x7FF;
      #endif
      RINOK(encoder.CreateEncoderThread())
    }
    else
    #endif
    {
      RINOK(encoder.Encode());
    }
  }
  #ifndef _7ZIP_ST
  if (numEncoderThreads > 1)
    for (i = 0; i < numEncoderThreads; i++)
      encoders[i].thread[0].Wait();
  #endif

  RINOK(status.Res);

  CBenchInfo info;

  encoders[0].progressInfoSpec[0]->SetFinishTime(info);
  info.UnpackSize = 0;
  info.PackSize = 0;
  info.NumIterations = 1; // progressInfoSpec->NumIterations;
  for (i = 0; i < numEncoderThreads; i++)
  {
    CEncoderInfo &encoder = encoders[i];
    info.UnpackSize += encoder.kBufferSize;
    info.PackSize += encoder.compressedSize;
  }
  RINOK(callback->SetEncodeResult(info, true));


  status.Res = S_OK;
  status.EncodeMode = false;

  UInt32 numDecoderThreads = numEncoderThreads * numSubDecoderThreads;
  for (i = 0; i < numEncoderThreads; i++)
  {
    CEncoderInfo &encoder = encoders[i];

    if (i == 0)
    {
      encoder.NumIterations = (UInt32)(1 + kUncompressMinBlockSize /
          benchProps->GeDecomprCommands(encoder.compressedSize, encoder.kBufferSize));
      CBenchProgressInfo *bpi = encoder.progressInfoSpec[0];
      bpi->Callback = callback;
      bpi->BenchInfo.NumIterations = numDecoderThreads;
      bpi->SetStartTime();
    }
    else
      encoder.NumIterations = encoders[0].NumIterations;

    #ifndef _7ZIP_ST
    {
      int numSubThreads = method.Get_NumThreads();
      encoder.NumDecoderSubThreads = (numSubThreads <= 0) ? 1 : numSubThreads;
    }
    if (numDecoderThreads > 1)
    {
      for (UInt32 j = 0; j < numSubDecoderThreads; j++)
      {
        HRESULT res = encoder.CreateDecoderThread(j, (i == 0 && j == 0)
            #ifdef USE_ALLOCA
            , ((i * numSubDecoderThreads + j) * 16 * 21) & 0x7FF
            #endif
            );
        RINOK(res);
      }
    }
    else
    #endif
    {
      RINOK(encoder.Decode(0));
    }
  }
  #ifndef _7ZIP_ST
  HRESULT res = S_OK;
  if (numDecoderThreads > 1)
    for (i = 0; i < numEncoderThreads; i++)
      for (UInt32 j = 0; j < numSubDecoderThreads; j++)
      {
        CEncoderInfo &encoder = encoders[i];
        encoder.thread[j].Wait();
        if (encoder.Results[j] != S_OK)
          res = encoder.Results[j];
      }
  RINOK(res);
  #endif
  RINOK(status.Res);
  encoders[0].progressInfoSpec[0]->SetFinishTime(info);
  #ifndef _7ZIP_ST
  #ifdef UNDER_CE
  if (numDecoderThreads > 1)
    for (i = 0; i < numEncoderThreads; i++)
      for (UInt32 j = 0; j < numSubDecoderThreads; j++)
      {
        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (::GetThreadTimes(encoders[i].thread[j], &creationTime, &exitTime, &kernelTime, &userTime) != 0)
          info.UserTime += GetTime64(userTime) + GetTime64(kernelTime);
      }
  #endif
  #endif
  info.UnpackSize = 0;
  info.PackSize = 0;
  info.NumIterations = numSubDecoderThreads * encoders[0].NumIterations;
  for (i = 0; i < numEncoderThreads; i++)
  {
    CEncoderInfo &encoder = encoders[i];
    info.UnpackSize += encoder.kBufferSize;
    info.PackSize += encoder.compressedSize;
  }
  RINOK(callback->SetDecodeResult(info, false));
  RINOK(callback->SetDecodeResult(info, true));
  return S_OK;
}


inline UInt64 GetLZMAUsage(bool multiThread, UInt32 dictionary)
{
  UInt32 hs = dictionary - 1;
  hs |= (hs >> 1);
  hs |= (hs >> 2);
  hs |= (hs >> 4);
  hs |= (hs >> 8);
  hs >>= 1;
  hs |= 0xFFFF;
  if (hs > (1 << 24))
    hs >>= 1;
  hs++;
  return ((hs + (1 << 16)) + (UInt64)dictionary * 2) * 4 + (UInt64)dictionary * 3 / 2 +
      (1 << 20) + (multiThread ? (6 << 20) : 0);
}

UInt64 GetBenchMemoryUsage(UInt32 numThreads, UInt32 dictionary)
{
  const UInt32 kBufferSize = dictionary;
  const UInt32 kCompressedBufferSize = (kBufferSize / 2);
  UInt32 numSubThreads = (numThreads > 1) ? 2 : 1;
  UInt32 numBigThreads = numThreads / numSubThreads;
  return (kBufferSize + kCompressedBufferSize +
    GetLZMAUsage((numThreads > 1), dictionary) + (2 << 20)) * numBigThreads;
}

static bool CrcBig(const void *data, UInt32 size, UInt32 numCycles, UInt32 crcBase)
{
  for (UInt32 i = 0; i < numCycles; i++)
    if (CrcCalc(data, size) != crcBase)
      return false;
  return true;
}

#ifndef _7ZIP_ST
struct CCrcInfo
{
  NWindows::CThread Thread;
  const Byte *Data;
  UInt32 Size;
  UInt32 NumCycles;
  UInt32 Crc;
  bool Res;
  void Wait()
  {
    Thread.Wait();
    Thread.Close();
  }
};

static THREAD_FUNC_DECL CrcThreadFunction(void *param)
{
  CCrcInfo *p = (CCrcInfo *)param;
  p->Res = CrcBig(p->Data, p->Size, p->NumCycles, p->Crc);
  return 0;
}

struct CCrcThreads
{
  UInt32 NumThreads;
  CCrcInfo *Items;
  CCrcThreads(): Items(0), NumThreads(0) {}
  void WaitAll()
  {
    for (UInt32 i = 0; i < NumThreads; i++)
      Items[i].Wait();
    NumThreads = 0;
  }
  ~CCrcThreads()
  {
    WaitAll();
    delete []Items;
  }
};
#endif

static UInt32 CrcCalc1(const Byte *buf, UInt32 size)
{
  UInt32 crc = CRC_INIT_VAL;;
  for (UInt32 i = 0; i < size; i++)
    crc = CRC_UPDATE_BYTE(crc, buf[i]);
  return CRC_GET_DIGEST(crc);
}

static void RandGen(Byte *buf, UInt32 size, CBaseRandomGenerator &RG)
{
  for (UInt32 i = 0; i < size; i++)
    buf[i] = (Byte)RG.GetRnd();
}

static UInt32 RandGenCrc(Byte *buf, UInt32 size, CBaseRandomGenerator &RG)
{
  RandGen(buf, size, RG);
  return CrcCalc1(buf, size);
}

bool CrcInternalTest()
{
  CBenchBuffer buffer;
  const UInt32 kBufferSize0 = (1 << 8);
  const UInt32 kBufferSize1 = (1 << 10);
  const UInt32 kCheckSize = (1 << 5);
  if (!buffer.Alloc(kBufferSize0 + kBufferSize1))
    return false;
  Byte *buf = buffer.Buffer;
  UInt32 i;
  for (i = 0; i < kBufferSize0; i++)
    buf[i] = (Byte)i;
  UInt32 crc1 = CrcCalc1(buf, kBufferSize0);
  if (crc1 != 0x29058C73)
    return false;
  CBaseRandomGenerator RG;
  RandGen(buf + kBufferSize0, kBufferSize1, RG);
  for (i = 0; i < kBufferSize0 + kBufferSize1 - kCheckSize; i++)
    for (UInt32 j = 0; j < kCheckSize; j++)
      if (CrcCalc1(buf + i, j) != CrcCalc(buf + i, j))
        return false;
  return true;
}

static HRESULT CrcBench(UInt32 numThreads, UInt32 bufferSize, UInt64 &speed)
{
  if (numThreads == 0)
    numThreads = 1;

  CBenchBuffer buffer;
  size_t totalSize = (size_t)bufferSize * numThreads;
  if (totalSize / numThreads != bufferSize)
    return E_OUTOFMEMORY;
  if (!buffer.Alloc(totalSize))
    return E_OUTOFMEMORY;

  Byte *buf = buffer.Buffer;
  CBaseRandomGenerator RG;
  UInt32 numCycles = (kCrcBlockSize) / ((bufferSize >> 2) + 1) + 1;

  UInt64 timeVal;
  #ifndef _7ZIP_ST
  CCrcThreads threads;
  if (numThreads > 1)
  {
    threads.Items = new CCrcInfo[numThreads];
    UInt32 i;
    for (i = 0; i < numThreads; i++)
    {
      CCrcInfo &info = threads.Items[i];
      Byte *data = buf + (size_t)bufferSize * i;
      info.Data = data;
      info.NumCycles = numCycles;
      info.Size = bufferSize;
      info.Crc = RandGenCrc(data, bufferSize, RG);
    }
    timeVal = GetTimeCount();
    for (i = 0; i < numThreads; i++)
    {
      CCrcInfo &info = threads.Items[i];
      RINOK(info.Thread.Create(CrcThreadFunction, &info));
      threads.NumThreads++;
    }
    threads.WaitAll();
    for (i = 0; i < numThreads; i++)
      if (!threads.Items[i].Res)
        return S_FALSE;
  }
  else
  #endif
  {
    UInt32 crc = RandGenCrc(buf, bufferSize, RG);
    timeVal = GetTimeCount();
    if (!CrcBig(buf, bufferSize, numCycles, crc))
      return S_FALSE;
  }
  timeVal = GetTimeCount() - timeVal;
  if (timeVal == 0)
    timeVal = 1;

  UInt64 size = (UInt64)numCycles * totalSize;
  speed = MyMultDiv64(size, timeVal, GetFreq());
  return S_OK;
}

struct CBenchMethod
{
  unsigned dictBits;
  UInt32 EncComplex;
  UInt32 DecComplexCompr;
  UInt32 DecComplexUnc;
  const char *Name;
};

static const CBenchMethod g_Bench[] =
{
  { 17,  340,  155,   20, "LZMA:x1" },
  { 24, 1182,  155,   20, "LZMA:x5:mt1" },
  { 24, 1182,  155,   20, "LZMA:x5:mt2" },
  { 16,  124,   47,   14, "Deflate:x1" },
  { 16,  376,   47,   14, "Deflate:x5" },
  { 16, 1084,   47,   14, "Deflate:x7" },
  { 17,  420,   47,   14, "Deflate64:x5" },
  { 15,  590,   69,   70, "BZip2:x1" },
  { 19,  792,  119,  119, "BZip2:x5"  },
  #ifndef UNDER_CE
  { 19,  792,  119,  119, "BZip2:x5:mt2" },
  #endif
  { 19, 2500,  118,  118, "BZip2:x7" },
  { 18, 1010,    0, 1155, "PPMD:x1" },
  { 22, 1650,    0, 1830, "PPMD:x5" }
};

struct CTotalBenchRes
{
  UInt64 NumIterations;
  UInt64 Rating;
  UInt64 Usage;
  UInt64 RPU;
  void Init() { NumIterations = 0; Rating = 0; Usage = 0; RPU = 0; }
  void Normalize()
  {
    if (NumIterations == 0)
      return;
    Rating /= NumIterations;
    Usage /= NumIterations;
    RPU /= NumIterations;
    NumIterations = 1;
  }
  void SetMid(const CTotalBenchRes &r1, const CTotalBenchRes &r2)
  {
    Rating = (r1.Rating + r2.Rating) / 2;
    Usage = (r1.Usage + r2.Usage) / 2;
    RPU = (r1.RPU + r2.RPU) / 2;
    NumIterations = (r1.NumIterations + r2.NumIterations) / 2;
  }
};

static void PrintNumber(IBenchPrintCallback &f, UInt64 value, int size, bool withSpace = true)
{
  char s[128];
  int startPos = (int)sizeof(s) - 32;
  memset(s, ' ', startPos);
  ConvertUInt64ToString(value, s + startPos);
  if (withSpace)
  {
    startPos--;
    size++;
  }
  int len = (int)strlen(s + startPos);
  if (size > len)
  {
    startPos -= (size - len);
    if (startPos < 0)
      startPos = 0;
  }
  f.Print(s + startPos);
}

static void PrintRating(IBenchPrintCallback &f, UInt64 rating)
{
  PrintNumber(f, rating / 1000000, 6);
}

static void PrintResults(IBenchPrintCallback &f, UInt64 usage, UInt64 rpu, UInt64 rating)
{
  PrintNumber(f, (usage + 5000) / 10000, 5);
  PrintRating(f, rpu);
  PrintRating(f, rating);
}

static void PrintResults(IBenchPrintCallback &f, const CBenchInfo &info, UInt64 rating, CTotalBenchRes &res)
{
  UInt64 speed = MyMultDiv64(info.UnpackSize, info.GlobalTime, info.GlobalFreq);
  PrintNumber(f, speed / 1024, 7);
  UInt64 usage = info.GetUsage();
  UInt64 rpu = info.GetRatingPerUsage(rating);
  PrintResults(f, usage, rpu, rating);
  res.NumIterations++;
  res.RPU += rpu;
  res.Rating += rating;
  res.Usage += usage;
}

static void PrintTotals(IBenchPrintCallback &f, const CTotalBenchRes &res)
{
  f.Print("       ");
  PrintResults(f, res.Usage, res.RPU, res.Rating);
}

static void PrintRequirements(IBenchPrintCallback &f, const char *sizeString, UInt64 size, const char *threadsString, UInt32 numThreads)
{
  f.Print("RAM ");
  f.Print(sizeString);
  PrintNumber(f, (size >> 20), 5, true);
  f.Print(" MB,  # ");
  f.Print(threadsString);
  PrintNumber(f, numThreads, 3, true);
  f.NewLine();
}

struct CBenchCallbackToPrint: public IBenchCallback
{
  CBenchProps BenchProps;
  CTotalBenchRes EncodeRes;
  CTotalBenchRes DecodeRes;
  IBenchPrintCallback *_file;
  UInt32 DictSize;

  void Init() { EncodeRes.Init(); DecodeRes.Init(); }
  void Normalize() { EncodeRes.Normalize(); DecodeRes.Normalize(); }
  HRESULT SetEncodeResult(const CBenchInfo &info, bool final);
  HRESULT SetDecodeResult(const CBenchInfo &info, bool final);
  void Print(const char *string);
  void NewLine();
  void PrintLeftAligned(const char *string, unsigned size);
};

HRESULT CBenchCallbackToPrint::SetEncodeResult(const CBenchInfo &info, bool final)
{
  RINOK(_file->CheckBreak());
  if (final)
  {
    UInt64 rating = BenchProps.GetCompressRating(DictSize, info.GlobalTime, info.GlobalFreq, info.UnpackSize);
    PrintResults(*_file, info, rating, EncodeRes);
  }
  return S_OK;
}

static const char *kSep = "  | ";


HRESULT CBenchCallbackToPrint::SetDecodeResult(const CBenchInfo &info, bool final)
{
  RINOK(_file->CheckBreak());
  if (final)
  {
    UInt64 rating = BenchProps.GetDecompressRating(info.GlobalTime, info.GlobalFreq, info.UnpackSize, info.PackSize, info.NumIterations);
    _file->Print(kSep);
    CBenchInfo info2 = info;
    info2.UnpackSize *= info2.NumIterations;
    info2.PackSize *= info2.NumIterations;
    info2.NumIterations = 1;
    PrintResults(*_file, info2, rating, DecodeRes);
  }
  return S_OK;
}

void CBenchCallbackToPrint::Print(const char *s)
{
  _file->Print(s);
}

void CBenchCallbackToPrint::NewLine()
{
  _file->NewLine();
}

void CBenchCallbackToPrint::PrintLeftAligned(const char *s, unsigned size)
{
  AString s2 = s;
  for (unsigned len = (unsigned)strlen(s); len < size; len++)
    s2 += ' ';
  Print(s2);
}

static HRESULT TotalBench(
  DECL_EXTERNAL_CODECS_LOC_VARS
  UInt32 numThreads, UInt32 unpackSize, IBenchPrintCallback *printCallback, CBenchCallbackToPrint *callback)
{
  for (unsigned i = 0; i < sizeof(g_Bench) / sizeof(g_Bench[0]); i++)
  {
    CBenchMethod bench = g_Bench[i];
    callback->PrintLeftAligned(bench.Name, 12);
    callback->BenchProps.DecComplexUnc = bench.DecComplexUnc;
    callback->BenchProps.DecComplexCompr = bench.DecComplexCompr;
    callback->BenchProps.EncComplex = bench.EncComplex;
    COneMethodInfo method;
    NCOM::CPropVariant propVariant;
    propVariant = bench.Name;
    RINOK(method.ParseMethodFromPROPVARIANT(L"", propVariant));

    HRESULT res = MethodBench(
        EXTERNAL_CODECS_LOC_VARS
        false, numThreads, method, unpackSize, bench.dictBits,
        printCallback, callback, &callback->BenchProps);
    if (res == E_NOTIMPL)
      callback->Print(" ---");
    else
    {
      RINOK(res);
    }
    callback->NewLine();
  }
  return  S_OK;
}

struct CTempValues
{
  UInt64 *Values;
  CTempValues(UInt32 num) { Values = new UInt64[num]; }
  ~CTempValues() { delete []Values; }
};

static void String_to_PropVariant(const UString &s, NCOM::CPropVariant &prop)
{
  const wchar_t *endPtr;
  UInt64 result = ConvertStringToUInt64(s, &endPtr);
  if (endPtr - (const wchar_t *)s != s.Length())
    prop = s;
  else if (result <= 0xFFFFFFFF)
    prop = (UInt32)result;
  else
    prop = result;
}

HRESULT Bench(
    DECL_EXTERNAL_CODECS_LOC_VARS
    IBenchPrintCallback *printCallback,
    IBenchCallback *benchCallback,
    const CObjectVector<CProperty> props,
    UInt32 numIterations,
    bool multiDict)
{
  if (!CrcInternalTest())
    return S_FALSE;

  UInt32 numCPUs = 1;
  UInt64 ramSize = (UInt64)512 << 20;
  #ifndef _7ZIP_ST
  numCPUs = NSystem::GetNumberOfProcessors();
  #endif
  #if !defined(_7ZIP_ST) || defined(_WIN32)
  ramSize = NSystem::GetRamSize();
  #endif
  UInt32 numThreads = numCPUs;

  if (printCallback)
    PrintRequirements(*printCallback, "size: ", ramSize, "CPU hardware threads:", numCPUs);

  COneMethodInfo method;
  int i;
  for (i = 0; i < props.Size(); i++)
  {
    const CProperty &property = props[i];
    NCOM::CPropVariant propVariant;
    UString name = property.Name;
    name.MakeUpper();
    if (!property.Value.IsEmpty())
      String_to_PropVariant(property.Value, propVariant);
    if (name.Left(2).CompareNoCase(L"MT") == 0)
    {
      #ifndef _7ZIP_ST
      RINOK(ParseMtProp(name.Mid(2), propVariant, numCPUs, numThreads));
      #endif
      continue;
    }
    RINOK(method.ParseMethodFromPROPVARIANT(name, propVariant));
  }

  UInt32 dict;
  bool dictIsDefined = method.Get_DicSize(dict);

  if (method.MethodName.IsEmpty())
    method.MethodName = L"LZMA";

  if (benchCallback)
  {
    CBenchProps benchProps;
    benchProps.SetLzmaCompexity();
    UInt32 dictSize = method.Get_Lzma_DicSize();
    UInt32 uncompressedDataSize = kAdditionalSize + dictSize;
    return MethodBench(
        EXTERNAL_CODECS_LOC_VARS
        true, numThreads,
        method, uncompressedDataSize,
        kOldLzmaDictBits, printCallback, benchCallback, &benchProps);
  }

  if (method.MethodName.CompareNoCase(L"CRC") == 0)
  {
    if (!printCallback)
      return S_FALSE;
    IBenchPrintCallback &f = *printCallback;
    if (!dictIsDefined)
      dict = (1 << 24);

    CTempValues speedTotals(numThreads);
    f.NewLine();
    f.Print("Size");
    for (UInt32 ti = 0; ti < numThreads; ti++)
    {
      PrintNumber(f, ti + 1, 5);
      speedTotals.Values[ti] = 0;
    }
    f.NewLine();
    f.NewLine();
    
    UInt64 numSteps = 0;
    for (UInt32 i = 0; i < numIterations; i++)
    {
      for (int pow = 10; pow < 32; pow++)
      {
        UInt32 bufSize = (UInt32)1 << pow;
        if (bufSize > dict)
          break;
        PrintNumber(f, pow, 2, false);
        f.Print(": ");
        for (UInt32 ti = 0; ti < numThreads; ti++)
        {
          RINOK(f.CheckBreak());
          UInt64 speed;
          RINOK(CrcBench(ti + 1, bufSize, speed));
          PrintNumber(f, (speed >> 20), 5);
          speedTotals.Values[ti] += speed;
        }
        f.NewLine();
        numSteps++;
      }
    }
    if (numSteps != 0)
    {
      f.NewLine();
      f.Print("Avg:");
      for (UInt32 ti = 0; ti < numThreads; ti++)
        PrintNumber(f, ((speedTotals.Values[ti] / numSteps) >> 20), 5);
      f.NewLine();
    }
    return S_OK;
  }

  CBenchCallbackToPrint callback;
  callback.Init();
  callback._file = printCallback;

  if (!dictIsDefined)
  {
    int dicSizeLog;
    for (dicSizeLog = 25; dicSizeLog > kBenchMinDicLogSize; dicSizeLog--)
      if (GetBenchMemoryUsage(numThreads, ((UInt32)1 << dicSizeLog)) + (8 << 20) <= ramSize)
        break;
    dict = (1 << dicSizeLog);
  }

  IBenchPrintCallback &f = *printCallback;
  PrintRequirements(f, "usage:", GetBenchMemoryUsage(numThreads, dict), "Benchmark threads:   ", numThreads);

  bool totalBenchMode = (method.MethodName == L"*");
  f.NewLine();
  f.Print(totalBenchMode ? "Method       " : "Dict");
  f.Print("        Compressing          |        Decompressing");
  f.NewLine();
  const char *kSpaces = totalBenchMode ? "            " : "   ";
  f.Print(kSpaces);
  int j;
     
  for (j = 0; j < 2; j++)
  {
    f.Print("   Speed Usage    R/U Rating");
    if (j == 0)
      f.Print(kSep);
  }
  f.NewLine();
  f.Print(kSpaces);
  for (j = 0; j < 2; j++)
  {
    f.Print("    KB/s     %   MIPS   MIPS");
    if (j == 0)
      f.Print(kSep);
  }
  f.NewLine();
  f.NewLine();

  if (totalBenchMode)
  {
    if (!dictIsDefined)
      dict =
        #ifdef UNDER_CE
          (UInt64)1 << 20;
        #else
          (UInt64)1 << 24;
        #endif
    for (UInt32 i = 0; i < numIterations; i++)
    {
      if (i != 0)
        printCallback->NewLine();
      HRESULT res = TotalBench(
          EXTERNAL_CODECS_LOC_VARS
          numThreads, dict, printCallback, &callback);
      RINOK(res);
    }
  }
  else
  {

  callback.BenchProps.SetLzmaCompexity();

  for (i = 0; i < (int)numIterations; i++)
  {
    const int kStartDicLog = 22;
    int pow = (dict < ((UInt32)1 << kStartDicLog)) ? kBenchMinDicLogSize : kStartDicLog;
    if (!multiDict)
      pow = 31;
    while (((UInt32)1 << pow) > dict)
      pow--;
    for (; ((UInt32)1 << pow) <= dict; pow++)
    {
      PrintNumber(f, pow, 2, false);
      f.Print(":");
      callback.DictSize = (UInt32)1 << pow;

      UInt32 uncompressedDataSize = kAdditionalSize + callback.DictSize;

      HRESULT res = MethodBench(
        EXTERNAL_CODECS_LOC_VARS
        true, numThreads,
        method, uncompressedDataSize,
        kOldLzmaDictBits, printCallback, &callback, &callback.BenchProps);
      f.NewLine();
      RINOK(res);
      if (!multiDict)
        break;
    }
  }
  }
  callback.Normalize();
  f.Print("----------------------------------------------------------------");
  f.NewLine();
  f.Print("Avr:");
  const char *kSpaces2 = totalBenchMode ? "         " : "";
  f.Print(kSpaces2);
  PrintTotals(f, callback.EncodeRes);
  f.Print("     ");
  PrintTotals(f, callback.DecodeRes);
  f.NewLine();
  f.Print("Tot:");
  f.Print(kSpaces2);
  CTotalBenchRes midRes;
  midRes.SetMid(callback.EncodeRes, callback.DecodeRes);
  PrintTotals(f, midRes);
  f.NewLine();
  return S_OK;
}
