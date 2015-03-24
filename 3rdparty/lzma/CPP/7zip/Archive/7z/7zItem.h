// 7zItem.h

#ifndef __7Z_ITEM_H
#define __7Z_ITEM_H

#include "../../../Common/Buffer.h"
#include "../../../Common/MyString.h"

#include "../../Common/MethodId.h"

#include "7zHeader.h"

namespace NArchive {
namespace N7z {

const UInt64 k_AES = 0x06F10701;

typedef UInt32 CNum;
const CNum kNumMax     = 0x7FFFFFFF;
const CNum kNumNoIndex = 0xFFFFFFFF;

struct CCoderInfo
{
  CMethodId MethodID;
  CByteBuffer Props;
  CNum NumInStreams;
  CNum NumOutStreams;
  bool IsSimpleCoder() const { return (NumInStreams == 1) && (NumOutStreams == 1); }
};

struct CBindPair
{
  CNum InIndex;
  CNum OutIndex;
};

struct CFolder
{
  CObjectVector<CCoderInfo> Coders;
  CRecordVector<CBindPair> BindPairs;
  CRecordVector<CNum> PackStreams;
  CRecordVector<UInt64> UnpackSizes;
  UInt32 UnpackCRC;
  bool UnpackCRCDefined;

  CFolder(): UnpackCRCDefined(false) {}

  UInt64 GetUnpackSize() const // test it
  {
    if (UnpackSizes.IsEmpty())
      return 0;
    for (int i = UnpackSizes.Size() - 1; i >= 0; i--)
      if (FindBindPairForOutStream(i) < 0)
        return UnpackSizes[i];
    throw 1;
  }

  CNum GetNumOutStreams() const
  {
    CNum result = 0;
    for (int i = 0; i < Coders.Size(); i++)
      result += Coders[i].NumOutStreams;
    return result;
  }

  int FindBindPairForInStream(CNum inStreamIndex) const
  {
    for(int i = 0; i < BindPairs.Size(); i++)
      if (BindPairs[i].InIndex == inStreamIndex)
        return i;
    return -1;
  }
  int FindBindPairForOutStream(CNum outStreamIndex) const
  {
    for(int i = 0; i < BindPairs.Size(); i++)
      if (BindPairs[i].OutIndex == outStreamIndex)
        return i;
    return -1;
  }
  int FindPackStreamArrayIndex(CNum inStreamIndex) const
  {
    for(int i = 0; i < PackStreams.Size(); i++)
      if (PackStreams[i] == inStreamIndex)
        return i;
    return -1;
  }

  bool IsEncrypted() const
  {
    for (int i = Coders.Size() - 1; i >= 0; i--)
      if (Coders[i].MethodID == k_AES)
        return true;
    return false;
  }

  bool CheckStructure() const;
};

struct CUInt64DefVector
{
  CRecordVector<UInt64> Values;
  CRecordVector<bool> Defined;
  
  void Clear()
  {
    Values.Clear();
    Defined.Clear();
  }
  
  void ReserveDown()
  {
    Values.ReserveDown();
    Values.ReserveDown();
  }

  bool GetItem(int index, UInt64 &value) const
  {
    if (index < Defined.Size() && Defined[index])
    {
      value = Values[index];
      return true;
    }
    value = 0;
    return false;
  }
  
  void SetItem(int index, bool defined, UInt64 value)
  {
    while (index >= Defined.Size())
      Defined.Add(false);
    Defined[index] = defined;
    if (!defined)
      return;
    while (index >= Values.Size())
      Values.Add(0);
    Values[index] = value;
  }

  bool CheckSize(int size) const { return Defined.Size() == size || Defined.Size() == 0; }
};

struct CFileItem
{
  UInt64 Size;
  UInt32 Attrib;
  UInt32 Crc;
  UString Name;

  bool HasStream; // Test it !!! it means that there is
                  // stream in some folder. It can be empty stream
  bool IsDir;
  bool CrcDefined;
  bool AttribDefined;

  CFileItem():
    HasStream(true),
    IsDir(false),
    CrcDefined(false),
    AttribDefined(false)
      {}
  void SetAttrib(UInt32 attrib)
  {
    AttribDefined = true;
    Attrib = attrib;
  }
};

struct CFileItem2
{
  UInt64 CTime;
  UInt64 ATime;
  UInt64 MTime;
  UInt64 StartPos;
  bool CTimeDefined;
  bool ATimeDefined;
  bool MTimeDefined;
  bool StartPosDefined;
  bool IsAnti;
};

struct CArchiveDatabase
{
  CRecordVector<UInt64> PackSizes;
  CRecordVector<bool> PackCRCsDefined;
  CRecordVector<UInt32> PackCRCs;
  CObjectVector<CFolder> Folders;
  CRecordVector<CNum> NumUnpackStreamsVector;
  CObjectVector<CFileItem> Files;

  CUInt64DefVector CTime;
  CUInt64DefVector ATime;
  CUInt64DefVector MTime;
  CUInt64DefVector StartPos;
  CRecordVector<bool> IsAnti;

  void Clear()
  {
    PackSizes.Clear();
    PackCRCsDefined.Clear();
    PackCRCs.Clear();
    Folders.Clear();
    NumUnpackStreamsVector.Clear();
    Files.Clear();
    CTime.Clear();
    ATime.Clear();
    MTime.Clear();
    StartPos.Clear();
    IsAnti.Clear();
  }

  void ReserveDown()
  {
    PackSizes.ReserveDown();
    PackCRCsDefined.ReserveDown();
    PackCRCs.ReserveDown();
    Folders.ReserveDown();
    NumUnpackStreamsVector.ReserveDown();
    Files.ReserveDown();
    CTime.ReserveDown();
    ATime.ReserveDown();
    MTime.ReserveDown();
    StartPos.ReserveDown();
    IsAnti.ReserveDown();
  }

  bool IsEmpty() const
  {
    return (PackSizes.IsEmpty() &&
      PackCRCsDefined.IsEmpty() &&
      PackCRCs.IsEmpty() &&
      Folders.IsEmpty() &&
      NumUnpackStreamsVector.IsEmpty() &&
      Files.IsEmpty());
  }

  bool CheckNumFiles() const
  {
    int size = Files.Size();
    return (
      CTime.CheckSize(size) &&
      ATime.CheckSize(size) &&
      MTime.CheckSize(size) &&
      StartPos.CheckSize(size) &&
      (size == IsAnti.Size() || IsAnti.Size() == 0));
  }

  bool IsSolid() const
  {
    for (int i = 0; i < NumUnpackStreamsVector.Size(); i++)
      if (NumUnpackStreamsVector[i] > 1)
        return true;
    return false;
  }
  bool IsItemAnti(int index) const { return (index < IsAnti.Size() && IsAnti[index]); }
  void SetItemAnti(int index, bool isAnti)
  {
    while (index >= IsAnti.Size())
      IsAnti.Add(false);
    IsAnti[index] = isAnti;
  }

  void GetFile(int index, CFileItem &file, CFileItem2 &file2) const;
  void AddFile(const CFileItem &file, const CFileItem2 &file2);
};

}}

#endif
