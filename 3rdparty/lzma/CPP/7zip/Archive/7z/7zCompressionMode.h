// 7zCompressionMode.h

#ifndef __7Z_COMPRESSION_MODE_H
#define __7Z_COMPRESSION_MODE_H

#include "../../Common/MethodId.h"
#include "../../Common/MethodProps.h"

namespace NArchive {
namespace N7z {

struct CMethodFull: public CProps
{
  CMethodId Id;
  UInt32 NumInStreams;
  UInt32 NumOutStreams;

  bool IsSimpleCoder() const { return (NumInStreams == 1) && (NumOutStreams == 1); }
};

struct CBind
{
  UInt32 InCoder;
  UInt32 InStream;
  UInt32 OutCoder;
  UInt32 OutStream;
};

struct CCompressionMethodMode
{
  CObjectVector<CMethodFull> Methods;
  CRecordVector<CBind> Binds;
  #ifndef _7ZIP_ST
  UInt32 NumThreads;
  #endif
  bool PasswordIsDefined;
  UString Password;

  bool IsEmpty() const { return (Methods.IsEmpty() && !PasswordIsDefined); }
  CCompressionMethodMode(): PasswordIsDefined(false)
      #ifndef _7ZIP_ST
      , NumThreads(1)
      #endif
  {}
};

}}

#endif
