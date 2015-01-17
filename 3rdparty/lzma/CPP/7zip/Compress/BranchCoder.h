// BranchCoder.h

#ifndef __COMPRESS_BRANCH_CODER_H
#define __COMPRESS_BRANCH_CODER_H

#include "../../Common/MyCom.h"

#include "../ICoder.h"

class CBranchConverter:
  public ICompressFilter,
  public CMyUnknownImp
{
protected:
  UInt32 _bufferPos;
  virtual void SubInit() {}
  virtual UInt32 SubFilter(Byte *data, UInt32 size) = 0;
public:
  MY_UNKNOWN_IMP;
  STDMETHOD(Init)();
  STDMETHOD_(UInt32, Filter)(Byte *data, UInt32 size);
};

#define MyClassEncoderA(Name) class C ## Name: public CBranchConverter \
  { public: UInt32 SubFilter(Byte *data, UInt32 size); };

#define MyClassDecoderA(Name) class C ## Name: public CBranchConverter \
  { public: UInt32 SubFilter(Byte *data, UInt32 size); };

#define MyClassEncoderB(Name, ADD_ITEMS, ADD_INIT) class C ## Name: public CBranchConverter, public ADD_ITEMS \
  { public: UInt32 SubFilter(Byte *data, UInt32 size); ADD_INIT};

#define MyClassDecoderB(Name, ADD_ITEMS, ADD_INIT) class C ## Name: public CBranchConverter, public ADD_ITEMS \
  { public: UInt32 SubFilter(Byte *data, UInt32 size); ADD_INIT};

#define MyClassA(Name, id, subId)  \
MyClassEncoderA(Name ## _Encoder) \
MyClassDecoderA(Name ## _Decoder)

#define MyClassB(Name, id, subId, ADD_ITEMS, ADD_INIT)  \
MyClassEncoderB(Name ## _Encoder, ADD_ITEMS, ADD_INIT) \
MyClassDecoderB(Name ## _Decoder, ADD_ITEMS, ADD_INIT)

#endif
