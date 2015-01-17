// 7zEncode.cpp

#include "StdAfx.h"

#include "../../Common/CreateCoder.h"
#include "../../Common/FilterCoder.h"
#include "../../Common/LimitedStreams.h"
#include "../../Common/InOutTempBuffer.h"
#include "../../Common/ProgressUtils.h"
#include "../../Common/StreamObjects.h"

#include "7zEncode.h"
#include "7zSpecStream.h"

static const UInt64 k_Delta = 0x03;
static const UInt64 k_BCJ = 0x03030103;
static const UInt64 k_BCJ2 = 0x0303011B;

namespace NArchive {
namespace N7z {

static void ConvertBindInfoToFolderItemInfo(const NCoderMixer::CBindInfo &bindInfo,
    const CRecordVector<CMethodId> decompressionMethods,
    CFolder &folder)
{
  folder.Coders.Clear();
  // bindInfo.CoderMethodIDs.Clear();
  // folder.OutStreams.Clear();
  folder.PackStreams.Clear();
  folder.BindPairs.Clear();
  int i;
  for (i = 0; i < bindInfo.BindPairs.Size(); i++)
  {
    CBindPair bindPair;
    bindPair.InIndex = bindInfo.BindPairs[i].InIndex;
    bindPair.OutIndex = bindInfo.BindPairs[i].OutIndex;
    folder.BindPairs.Add(bindPair);
  }
  for (i = 0; i < bindInfo.Coders.Size(); i++)
  {
    CCoderInfo coderInfo;
    const NCoderMixer::CCoderStreamsInfo &coderStreamsInfo = bindInfo.Coders[i];
    coderInfo.NumInStreams = coderStreamsInfo.NumInStreams;
    coderInfo.NumOutStreams = coderStreamsInfo.NumOutStreams;
    coderInfo.MethodID = decompressionMethods[i];
    folder.Coders.Add(coderInfo);
  }
  for (i = 0; i < bindInfo.InStreams.Size(); i++)
    folder.PackStreams.Add(bindInfo.InStreams[i]);
}

static HRESULT SetCoderProps2(const CProps &props, const UInt64 *dataSizeReduce, IUnknown *coder)
{
  CMyComPtr<ICompressSetCoderProperties> setCoderProperties;
  coder->QueryInterface(IID_ICompressSetCoderProperties, (void **)&setCoderProperties);
  if (setCoderProperties)
    return props.SetCoderProps(setCoderProperties, dataSizeReduce);
  return props.AreThereNonOptionalProps() ? E_INVALIDARG : S_OK;
}

HRESULT CEncoder::CreateMixerCoder(
    DECL_EXTERNAL_CODECS_LOC_VARS
    const UInt64 *inSizeForReduce)
{
  _mixerCoderSpec = new NCoderMixer::CCoderMixer2MT;
  _mixerCoder = _mixerCoderSpec;
  RINOK(_mixerCoderSpec->SetBindInfo(_bindInfo));
  for (int i = 0; i < _options.Methods.Size(); i++)
  {
    const CMethodFull &methodFull = _options.Methods[i];
    _codersInfo.Add(CCoderInfo());
    CCoderInfo &encodingInfo = _codersInfo.Back();
    encodingInfo.MethodID = methodFull.Id;
    CMyComPtr<ICompressCoder> encoder;
    CMyComPtr<ICompressCoder2> encoder2;
    

    RINOK(CreateCoder(
        EXTERNAL_CODECS_LOC_VARS
        methodFull.Id, encoder, encoder2, true));

    if (!encoder && !encoder2)
      return E_FAIL;

    CMyComPtr<IUnknown> encoderCommon = encoder ? (IUnknown *)encoder : (IUnknown *)encoder2;
   
    #ifndef _7ZIP_ST
    {
      CMyComPtr<ICompressSetCoderMt> setCoderMt;
      encoderCommon.QueryInterface(IID_ICompressSetCoderMt, &setCoderMt);
      if (setCoderMt)
      {
        RINOK(setCoderMt->SetNumberOfThreads(_options.NumThreads));
      }
    }
    #endif
        
    RINOK(SetCoderProps2(methodFull, inSizeForReduce, encoderCommon));

    /*
    CMyComPtr<ICryptoResetSalt> resetSalt;
    encoderCommon.QueryInterface(IID_ICryptoResetSalt, (void **)&resetSalt);
    if (resetSalt != NULL)
    {
      resetSalt->ResetSalt();
    }
    */

    #ifdef EXTERNAL_CODECS
    CMyComPtr<ISetCompressCodecsInfo> setCompressCodecsInfo;
    encoderCommon.QueryInterface(IID_ISetCompressCodecsInfo, (void **)&setCompressCodecsInfo);
    if (setCompressCodecsInfo)
    {
      RINOK(setCompressCodecsInfo->SetCompressCodecsInfo(codecsInfo));
    }
    #endif
    
    CMyComPtr<ICryptoSetPassword> cryptoSetPassword;
    encoderCommon.QueryInterface(IID_ICryptoSetPassword, &cryptoSetPassword);

    if (cryptoSetPassword)
    {
      CByteBuffer buffer;
      const UInt32 sizeInBytes = _options.Password.Length() * 2;
      buffer.SetCapacity(sizeInBytes);
      for (int i = 0; i < _options.Password.Length(); i++)
      {
        wchar_t c = _options.Password[i];
        ((Byte *)buffer)[i * 2] = (Byte)c;
        ((Byte *)buffer)[i * 2 + 1] = (Byte)(c >> 8);
      }
      RINOK(cryptoSetPassword->CryptoSetPassword((const Byte *)buffer, sizeInBytes));
    }

    if (encoder)
      _mixerCoderSpec->AddCoder(encoder);
    else
      _mixerCoderSpec->AddCoder2(encoder2);
  }
  return S_OK;
}

HRESULT CEncoder::Encode(
    DECL_EXTERNAL_CODECS_LOC_VARS
    ISequentialInStream *inStream,
    const UInt64 *inStreamSize, const UInt64 *inSizeForReduce,
    CFolder &folderItem,
    ISequentialOutStream *outStream,
    CRecordVector<UInt64> &packSizes,
    ICompressProgressInfo *compressProgress)
{
  RINOK(EncoderConstr());

  if (_mixerCoderSpec == NULL)
  {
    RINOK(CreateMixerCoder(EXTERNAL_CODECS_LOC_VARS inSizeForReduce));
  }
  _mixerCoderSpec->ReInit();
  // _mixerCoderSpec->SetCoderInfo(0, NULL, NULL, progress);

  CObjectVector<CInOutTempBuffer> inOutTempBuffers;
  CObjectVector<CSequentialOutTempBufferImp *> tempBufferSpecs;
  CObjectVector<CMyComPtr<ISequentialOutStream> > tempBuffers;
  int numMethods = _bindInfo.Coders.Size();
  int i;
  for (i = 1; i < _bindInfo.OutStreams.Size(); i++)
  {
    inOutTempBuffers.Add(CInOutTempBuffer());
    inOutTempBuffers.Back().Create();
    inOutTempBuffers.Back().InitWriting();
  }
  for (i = 1; i < _bindInfo.OutStreams.Size(); i++)
  {
    CSequentialOutTempBufferImp *tempBufferSpec = new CSequentialOutTempBufferImp;
    CMyComPtr<ISequentialOutStream> tempBuffer = tempBufferSpec;
    tempBufferSpec->Init(&inOutTempBuffers[i - 1]);
    tempBuffers.Add(tempBuffer);
    tempBufferSpecs.Add(tempBufferSpec);
  }

  for (i = 0; i < numMethods; i++)
    _mixerCoderSpec->SetCoderInfo(i, NULL, NULL);

  if (_bindInfo.InStreams.IsEmpty())
    return E_FAIL;
  UInt32 mainCoderIndex, mainStreamIndex;
  _bindInfo.FindInStream(_bindInfo.InStreams[0], mainCoderIndex, mainStreamIndex);
  
  if (inStreamSize != NULL)
  {
    CRecordVector<const UInt64 *> sizePointers;
    for (UInt32 i = 0; i < _bindInfo.Coders[mainCoderIndex].NumInStreams; i++)
      if (i == mainStreamIndex)
        sizePointers.Add(inStreamSize);
      else
        sizePointers.Add(NULL);
    _mixerCoderSpec->SetCoderInfo(mainCoderIndex, &sizePointers.Front(), NULL);
  }

  
  // UInt64 outStreamStartPos;
  // RINOK(stream->Seek(0, STREAM_SEEK_CUR, &outStreamStartPos));
  
  CSequentialInStreamSizeCount2 *inStreamSizeCountSpec = new CSequentialInStreamSizeCount2;
  CMyComPtr<ISequentialInStream> inStreamSizeCount = inStreamSizeCountSpec;
  CSequentialOutStreamSizeCount *outStreamSizeCountSpec = new CSequentialOutStreamSizeCount;
  CMyComPtr<ISequentialOutStream> outStreamSizeCount = outStreamSizeCountSpec;

  inStreamSizeCountSpec->Init(inStream);
  outStreamSizeCountSpec->SetStream(outStream);
  outStreamSizeCountSpec->Init();

  CRecordVector<ISequentialInStream *> inStreamPointers;
  CRecordVector<ISequentialOutStream *> outStreamPointers;
  inStreamPointers.Add(inStreamSizeCount);
  outStreamPointers.Add(outStreamSizeCount);
  for (i = 1; i < _bindInfo.OutStreams.Size(); i++)
    outStreamPointers.Add(tempBuffers[i - 1]);

  for (i = 0; i < _codersInfo.Size(); i++)
  {
    CCoderInfo &encodingInfo = _codersInfo[i];
    
    CMyComPtr<ICryptoResetInitVector> resetInitVector;
    _mixerCoderSpec->_coders[i].QueryInterface(IID_ICryptoResetInitVector, (void **)&resetInitVector);
    if (resetInitVector != NULL)
    {
      resetInitVector->ResetInitVector();
    }

    CMyComPtr<ICompressWriteCoderProperties> writeCoderProperties;
    _mixerCoderSpec->_coders[i].QueryInterface(IID_ICompressWriteCoderProperties, (void **)&writeCoderProperties);
    if (writeCoderProperties != NULL)
    {
      CDynBufSeqOutStream *outStreamSpec = new CDynBufSeqOutStream;
      CMyComPtr<ISequentialOutStream> outStream(outStreamSpec);
      outStreamSpec->Init();
      writeCoderProperties->WriteCoderProperties(outStream);
      outStreamSpec->CopyToBuffer(encodingInfo.Props);
    }
  }

  UInt32 progressIndex = mainCoderIndex;

  for (i = 0; i + 1 < _codersInfo.Size(); i++)
  {
    UInt64 m = _codersInfo[i].MethodID;
    if (m == k_Delta || m == k_BCJ || m == k_BCJ2)
      progressIndex = i + 1;
  }

  _mixerCoderSpec->SetProgressCoderIndex(progressIndex);
  
  RINOK(_mixerCoder->Code(&inStreamPointers.Front(), NULL, 1,
    &outStreamPointers.Front(), NULL, outStreamPointers.Size(), compressProgress));
  
  ConvertBindInfoToFolderItemInfo(_decompressBindInfo, _decompressionMethods, folderItem);
  
  packSizes.Add(outStreamSizeCountSpec->GetSize());
  
  for (i = 1; i < _bindInfo.OutStreams.Size(); i++)
  {
    CInOutTempBuffer &inOutTempBuffer = inOutTempBuffers[i - 1];
    RINOK(inOutTempBuffer.WriteToStream(outStream));
    packSizes.Add(inOutTempBuffer.GetDataSize());
  }
  
  for (i = 0; i < (int)_bindReverseConverter->NumSrcInStreams; i++)
  {
    int binder = _bindInfo.FindBinderForInStream(
        _bindReverseConverter->DestOutToSrcInMap[i]);
    UInt64 streamSize;
    if (binder < 0)
      streamSize = inStreamSizeCountSpec->GetSize();
    else
      streamSize = _mixerCoderSpec->GetWriteProcessedSize(binder);
    folderItem.UnpackSizes.Add(streamSize);
  }
  for (i = numMethods - 1; i >= 0; i--)
    folderItem.Coders[numMethods - 1 - i].Props = _codersInfo[i].Props;
  return S_OK;
}


CEncoder::CEncoder(const CCompressionMethodMode &options):
  _bindReverseConverter(0),
  _constructed(false)
{
  if (options.IsEmpty())
    throw 1;

  _options = options;
  _mixerCoderSpec = NULL;
}

HRESULT CEncoder::EncoderConstr()
{
  if (_constructed)
    return S_OK;
  if (_options.Methods.IsEmpty())
  {
    // it has only password method;
    if (!_options.PasswordIsDefined)
      throw 1;
    if (!_options.Binds.IsEmpty())
      throw 1;
    NCoderMixer::CCoderStreamsInfo coderStreamsInfo;
    CMethodFull method;
    
    method.NumInStreams = 1;
    method.NumOutStreams = 1;
    coderStreamsInfo.NumInStreams = 1;
    coderStreamsInfo.NumOutStreams = 1;
    method.Id = k_AES;
    
    _options.Methods.Add(method);
    _bindInfo.Coders.Add(coderStreamsInfo);
  
    _bindInfo.InStreams.Add(0);
    _bindInfo.OutStreams.Add(0);
  }
  else
  {

  UInt32 numInStreams = 0, numOutStreams = 0;
  int i;
  for (i = 0; i < _options.Methods.Size(); i++)
  {
    const CMethodFull &methodFull = _options.Methods[i];
    NCoderMixer::CCoderStreamsInfo coderStreamsInfo;
    coderStreamsInfo.NumInStreams = methodFull.NumOutStreams;
    coderStreamsInfo.NumOutStreams = methodFull.NumInStreams;
    if (_options.Binds.IsEmpty())
    {
      if (i < _options.Methods.Size() - 1)
      {
        NCoderMixer::CBindPair bindPair;
        bindPair.InIndex = numInStreams + coderStreamsInfo.NumInStreams;
        bindPair.OutIndex = numOutStreams;
        _bindInfo.BindPairs.Add(bindPair);
      }
      else
        _bindInfo.OutStreams.Insert(0, numOutStreams);
      for (UInt32 j = 1; j < coderStreamsInfo.NumOutStreams; j++)
        _bindInfo.OutStreams.Add(numOutStreams + j);
    }
    
    numInStreams += coderStreamsInfo.NumInStreams;
    numOutStreams += coderStreamsInfo.NumOutStreams;

    _bindInfo.Coders.Add(coderStreamsInfo);
  }

  if (!_options.Binds.IsEmpty())
  {
    for (i = 0; i < _options.Binds.Size(); i++)
    {
      NCoderMixer::CBindPair bindPair;
      const CBind &bind = _options.Binds[i];
      bindPair.InIndex = _bindInfo.GetCoderInStreamIndex(bind.InCoder) + bind.InStream;
      bindPair.OutIndex = _bindInfo.GetCoderOutStreamIndex(bind.OutCoder) + bind.OutStream;
      _bindInfo.BindPairs.Add(bindPair);
    }
    for (i = 0; i < (int)numOutStreams; i++)
      if (_bindInfo.FindBinderForOutStream(i) == -1)
        _bindInfo.OutStreams.Add(i);
  }

  for (i = 0; i < (int)numInStreams; i++)
    if (_bindInfo.FindBinderForInStream(i) == -1)
      _bindInfo.InStreams.Add(i);

  if (_bindInfo.InStreams.IsEmpty())
    throw 1; // this is error

  // Make main stream first in list
  int inIndex = _bindInfo.InStreams[0];
  for (;;)
  {
    UInt32 coderIndex, coderStreamIndex;
    _bindInfo.FindInStream(inIndex, coderIndex, coderStreamIndex);
    UInt32 outIndex = _bindInfo.GetCoderOutStreamIndex(coderIndex);
    int binder = _bindInfo.FindBinderForOutStream(outIndex);
    if (binder >= 0)
    {
      inIndex = _bindInfo.BindPairs[binder].InIndex;
      continue;
    }
    for (i = 0; i < _bindInfo.OutStreams.Size(); i++)
      if (_bindInfo.OutStreams[i] == outIndex)
      {
        _bindInfo.OutStreams.Delete(i);
        _bindInfo.OutStreams.Insert(0, outIndex);
        break;
      }
    break;
  }

  if (_options.PasswordIsDefined)
  {
    int numCryptoStreams = _bindInfo.OutStreams.Size();

    for (i = 0; i < numCryptoStreams; i++)
    {
      NCoderMixer::CBindPair bindPair;
      bindPair.InIndex = numInStreams + i;
      bindPair.OutIndex = _bindInfo.OutStreams[i];
      _bindInfo.BindPairs.Add(bindPair);
    }
    _bindInfo.OutStreams.Clear();

    /*
    if (numCryptoStreams == 0)
      numCryptoStreams = 1;
    */

    for (i = 0; i < numCryptoStreams; i++)
    {
      NCoderMixer::CCoderStreamsInfo coderStreamsInfo;
      CMethodFull method;
      method.NumInStreams = 1;
      method.NumOutStreams = 1;
      coderStreamsInfo.NumInStreams = method.NumOutStreams;
      coderStreamsInfo.NumOutStreams = method.NumInStreams;
      method.Id = k_AES;

      _options.Methods.Add(method);
      _bindInfo.Coders.Add(coderStreamsInfo);
      _bindInfo.OutStreams.Add(numOutStreams + i);
    }
  }

  }

  for (int i = _options.Methods.Size() - 1; i >= 0; i--)
  {
    const CMethodFull &methodFull = _options.Methods[i];
    _decompressionMethods.Add(methodFull.Id);
  }

  _bindReverseConverter = new NCoderMixer::CBindReverseConverter(_bindInfo);
  _bindReverseConverter->CreateReverseBindInfo(_decompressBindInfo);
  _constructed = true;
  return S_OK;
}

CEncoder::~CEncoder()
{
  delete _bindReverseConverter;
}

}}
