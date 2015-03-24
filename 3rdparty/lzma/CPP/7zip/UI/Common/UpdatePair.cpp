// UpdatePair.cpp

#include "StdAfx.h"

#include <time.h>

#include "Common/Defs.h"
#include "Common/Wildcard.h"

#include "Windows/Time.h"

#include "SortUtils.h"
#include "UpdatePair.h"

using namespace NWindows;
using namespace NTime;

static int MyCompareTime(NFileTimeType::EEnum fileTimeType, const FILETIME &time1, const FILETIME &time2)
{
  switch(fileTimeType)
  {
    case NFileTimeType::kWindows:
      return ::CompareFileTime(&time1, &time2);
    case NFileTimeType::kUnix:
      {
        UInt32 unixTime1, unixTime2;
        FileTimeToUnixTime(time1, unixTime1);
        FileTimeToUnixTime(time2, unixTime2);
        return MyCompare(unixTime1, unixTime2);
      }
    case NFileTimeType::kDOS:
      {
        UInt32 dosTime1, dosTime2;
        FileTimeToDosTime(time1, dosTime1);
        FileTimeToDosTime(time2, dosTime2);
        return MyCompare(dosTime1, dosTime2);
      }
  }
  throw 4191618;
}

static const wchar_t *kDuplicateFileNameMessage = L"Duplicate filename:";
static const wchar_t *kNotCensoredCollisionMessaged = L"Internal file name collision (file on disk, file in archive):";

static void ThrowError(const UString &message, const UString &s1, const UString &s2)
{
  UString m = message;
  m += L'\n';
  m += s1;
  m += L'\n';
  m += s2;
  throw m;
}

static void TestDuplicateString(const UStringVector &strings, const CIntVector &indices)
{
  for(int i = 0; i + 1 < indices.Size(); i++)
    if (CompareFileNames(strings[indices[i]], strings[indices[i + 1]]) == 0)
      ThrowError(kDuplicateFileNameMessage, strings[indices[i]], strings[indices[i + 1]]);
}

void GetUpdatePairInfoList(
    const CDirItems &dirItems,
    const CObjectVector<CArcItem> &arcItems,
    NFileTimeType::EEnum fileTimeType,
    CRecordVector<CUpdatePair> &updatePairs)
{
  CIntVector dirIndices, arcIndices;
  
  int numDirItems = dirItems.Items.Size();
  int numArcItems = arcItems.Size();
  
  
  {
    UStringVector arcNames;
    arcNames.Reserve(numArcItems);
    for (int i = 0; i < numArcItems; i++)
      arcNames.Add(arcItems[i].Name);
    SortFileNames(arcNames, arcIndices);
    TestDuplicateString(arcNames, arcIndices);
  }

  UStringVector dirNames;
  {
    dirNames.Reserve(numDirItems);
    for (int i = 0; i < numDirItems; i++)
      dirNames.Add(dirItems.GetLogPath(i));
    SortFileNames(dirNames, dirIndices);
    TestDuplicateString(dirNames, dirIndices);
  }
  
  int dirIndex = 0, arcIndex = 0;
  while (dirIndex < numDirItems && arcIndex < numArcItems)
  {
    CUpdatePair pair;
    int dirIndex2 = dirIndices[dirIndex];
    int arcIndex2 = arcIndices[arcIndex];
    const CDirItem &di = dirItems.Items[dirIndex2];
    const CArcItem &ai = arcItems[arcIndex2];
    int compareResult = CompareFileNames(dirNames[dirIndex2], ai.Name);
    if (compareResult < 0)
    {
      pair.State = NUpdateArchive::NPairState::kOnlyOnDisk;
      pair.DirIndex = dirIndex2;
      dirIndex++;
    }
    else if (compareResult > 0)
    {
      pair.State = ai.Censored ?
          NUpdateArchive::NPairState::kOnlyInArchive:
          NUpdateArchive::NPairState::kNotMasked;
      pair.ArcIndex = arcIndex2;
      arcIndex++;
    }
    else
    {
      if (!ai.Censored)
        ThrowError(kNotCensoredCollisionMessaged, dirNames[dirIndex2], ai.Name);
      pair.DirIndex = dirIndex2;
      pair.ArcIndex = arcIndex2;
      switch (ai.MTimeDefined ? MyCompareTime(
          ai.TimeType != - 1 ? (NFileTimeType::EEnum)ai.TimeType : fileTimeType,
          di.MTime, ai.MTime): 0)
      {
        case -1: pair.State = NUpdateArchive::NPairState::kNewInArchive; break;
        case 1:  pair.State = NUpdateArchive::NPairState::kOldInArchive; break;
        default:
          pair.State = (ai.SizeDefined && di.Size == ai.Size) ?
              NUpdateArchive::NPairState::kSameFiles :
              NUpdateArchive::NPairState::kUnknowNewerFiles;
      }
      dirIndex++;
      arcIndex++;
    }
    updatePairs.Add(pair);
  }

  for (; dirIndex < numDirItems; dirIndex++)
  {
    CUpdatePair pair;
    pair.State = NUpdateArchive::NPairState::kOnlyOnDisk;
    pair.DirIndex = dirIndices[dirIndex];
    updatePairs.Add(pair);
  }
  
  for (; arcIndex < numArcItems; arcIndex++)
  {
    CUpdatePair pair;
    int arcIndex2 = arcIndices[arcIndex];
    pair.State = arcItems[arcIndex2].Censored ?
        NUpdateArchive::NPairState::kOnlyInArchive:
        NUpdateArchive::NPairState::kNotMasked;
    pair.ArcIndex = arcIndex2;
    updatePairs.Add(pair);
  }

  updatePairs.ReserveDown();
}
