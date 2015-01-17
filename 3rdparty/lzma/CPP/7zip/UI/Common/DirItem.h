// DirItem.h

#ifndef __DIR_ITEM_H
#define __DIR_ITEM_H

#include "Common/MyString.h"
#include "Common/Types.h"
#include "../../Archive/IArchive.h"

struct CDirItem
{
  UInt64 Size;
  FILETIME CTime;
  FILETIME ATime;
  FILETIME MTime;
  UString Name;
  UInt32 Attrib;
  int PhyParent;
  int LogParent;
  
  CDirItem(): PhyParent(-1), LogParent(-1) {}
  bool IsDir() const { return (Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0 ; }
};

class CDirItems
{
  UStringVector Prefixes;
  CIntVector PhyParents;
  CIntVector LogParents;

  UString GetPrefixesPath(const CIntVector &parents, int index, const UString &name) const;
public:
  CObjectVector<CDirItem> Items;

  int GetNumFolders() const { return Prefixes.Size(); }
  UString GetPhyPath(int index) const;
  UString GetLogPath(int index) const;

  int AddPrefix(int phyParent, int logParent, const UString &prefix);
  void DeleteLastPrefix();

  void EnumerateDirectory(int phyParent, int logParent, const FString &phyPrefix,
    FStringVector &errorPaths, CRecordVector<DWORD> &errorCodes);

  void EnumerateDirItems2(
    const FString &phyPrefix,
    const UString &logPrefix,
    const FStringVector &filePaths,
    FStringVector &errorPaths, CRecordVector<DWORD> &errorCodes);

  void ReserveDown();
};

struct CArcItem
{
  UInt64 Size;
  FILETIME MTime;
  UString Name;
  bool IsDir;
  bool SizeDefined;
  bool MTimeDefined;
  bool Censored;
  UInt32 IndexInServer;
  int TimeType;
  
  CArcItem(): IsDir(false), SizeDefined(false), MTimeDefined(false), Censored(false), TimeType(-1) {}
};

#endif
