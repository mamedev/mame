// 7z/Handler.h

#ifndef __7Z_HANDLER_H
#define __7Z_HANDLER_H

#include "../../ICoder.h"
#include "../IArchive.h"

#include "../../Common/CreateCoder.h"

#ifndef EXTRACT_ONLY
#include "../Common/HandlerOut.h"
#endif

#include "7zCompressionMode.h"
#include "7zIn.h"

namespace NArchive {
namespace N7z {

const UInt32 k_Copy = 0x0;
const UInt32 k_Delta = 3;
const UInt32 k_LZMA2 = 0x21;
const UInt32 k_LZMA  = 0x030101;
const UInt32 k_PPMD  = 0x030401;
const UInt32 k_BCJ  = 0x03030103;
const UInt32 k_BCJ2 = 0x0303011B;
const UInt32 k_Deflate = 0x040108;
const UInt32 k_BZip2   = 0x040202;

#ifndef __7Z_SET_PROPERTIES

#ifdef EXTRACT_ONLY
#if !defined(_7ZIP_ST) && !defined(_SFX)
#define __7Z_SET_PROPERTIES
#endif
#else
#define __7Z_SET_PROPERTIES
#endif

#endif


#ifndef EXTRACT_ONLY

class COutHandler: public CMultiMethodProps
{
  HRESULT SetSolidFromString(const UString &s);
  HRESULT SetSolidFromPROPVARIANT(const PROPVARIANT &value);
public:
  bool _removeSfxBlock;
  
  UInt64 _numSolidFiles;
  UInt64 _numSolidBytes;
  bool _numSolidBytesDefined;
  bool _solidExtension;

  bool _compressHeaders;
  bool _encryptHeadersSpecified;
  bool _encryptHeaders;

  bool WriteCTime;
  bool WriteATime;
  bool WriteMTime;

  bool _volumeMode;

  void InitSolidFiles() { _numSolidFiles = (UInt64)(Int64)(-1); }
  void InitSolidSize()  { _numSolidBytes = (UInt64)(Int64)(-1); }
  void InitSolid()
  {
    InitSolidFiles();
    InitSolidSize();
    _solidExtension = false;
    _numSolidBytesDefined = false;
  }

  void InitProps();

  COutHandler() { InitProps(); }

  HRESULT SetProperty(const wchar_t *name, const PROPVARIANT &value);
};

#endif

class CHandler:
  #ifndef EXTRACT_ONLY
  public COutHandler,
  #endif
  public IInArchive,
  #ifdef __7Z_SET_PROPERTIES
  public ISetProperties,
  #endif
  #ifndef EXTRACT_ONLY
  public IOutArchive,
  #endif
  PUBLIC_ISetCompressCodecsInfo
  public CMyUnknownImp
{
public:
  MY_QUERYINTERFACE_BEGIN2(IInArchive)
  #ifdef __7Z_SET_PROPERTIES
  MY_QUERYINTERFACE_ENTRY(ISetProperties)
  #endif
  #ifndef EXTRACT_ONLY
  MY_QUERYINTERFACE_ENTRY(IOutArchive)
  #endif
  QUERY_ENTRY_ISetCompressCodecsInfo
  MY_QUERYINTERFACE_END
  MY_ADDREF_RELEASE

  INTERFACE_IInArchive(;)

  #ifdef __7Z_SET_PROPERTIES
  STDMETHOD(SetProperties)(const wchar_t **names, const PROPVARIANT *values, Int32 numProperties);
  #endif

  #ifndef EXTRACT_ONLY
  INTERFACE_IOutArchive(;)
  #endif

  DECL_ISetCompressCodecsInfo

  CHandler();

private:
  CMyComPtr<IInStream> _inStream;
  NArchive::N7z::CArchiveDatabaseEx _db;
  #ifndef _NO_CRYPTO
  bool _passwordIsDefined;
  #endif

  #ifdef EXTRACT_ONLY
  
  #ifdef __7Z_SET_PROPERTIES
  UInt32 _numThreads;
  #endif

  UInt32 _crcSize;

  #else
  
  CRecordVector<CBind> _binds;

  HRESULT PropsMethod_To_FullMethod(CMethodFull &dest, const COneMethodInfo &m);
  HRESULT SetHeaderMethod(CCompressionMethodMode &headerMethod);
  void AddDefaultMethod();
  HRESULT SetMainMethod(CCompressionMethodMode &method,
      CObjectVector<COneMethodInfo> &methodsInfo
      #ifndef _7ZIP_ST
      , UInt32 numThreads
      #endif
      );


  #endif

  bool IsEncrypted(UInt32 index2) const;
  #ifndef _SFX

  CRecordVector<UInt64> _fileInfoPopIDs;
  void FillPopIDs();

  #endif

  DECL_EXTERNAL_CODECS_VARS
};

}}

#endif
