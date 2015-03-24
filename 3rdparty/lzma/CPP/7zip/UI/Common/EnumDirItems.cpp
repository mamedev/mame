// EnumDirItems.cpp

#include "StdAfx.h"

#include "Windows/FileName.h"

#include "EnumDirItems.h"

using namespace NWindows;
using namespace NFile;
using namespace NName;

void AddDirFileInfo(int phyParent, int logParent,
    const NFind::CFileInfo &fi, CObjectVector<CDirItem> &dirItems)
{
  CDirItem di;
  di.Size = fi.Size;
  di.CTime = fi.CTime;
  di.ATime = fi.ATime;
  di.MTime = fi.MTime;
  di.Attrib = fi.Attrib;
  di.PhyParent = phyParent;
  di.LogParent = logParent;
  di.Name = fs2us(fi.Name);
  dirItems.Add(di);
}

UString CDirItems::GetPrefixesPath(const CIntVector &parents, int index, const UString &name) const
{
  UString path;
  int len = name.Length();
  int i;
  for (i = index; i >= 0; i = parents[i])
    len += Prefixes[i].Length();
  int totalLen = len;
  wchar_t *p = path.GetBuffer(len);
  p[len] = 0;
  len -= name.Length();
  memcpy(p + len, (const wchar_t *)name, name.Length() * sizeof(wchar_t));
  for (i = index; i >= 0; i = parents[i])
  {
    const UString &s = Prefixes[i];
    len -= s.Length();
    memcpy(p + len, (const wchar_t *)s, s.Length() * sizeof(wchar_t));
  }
  path.ReleaseBuffer(totalLen);
  return path;
}

UString CDirItems::GetPhyPath(int index) const
{
  const CDirItem &di = Items[index];
  return GetPrefixesPath(PhyParents, di.PhyParent, di.Name);
}

UString CDirItems::GetLogPath(int index) const
{
  const CDirItem &di = Items[index];
  return GetPrefixesPath(LogParents, di.LogParent, di.Name);
}

void CDirItems::ReserveDown()
{
  Prefixes.ReserveDown();
  PhyParents.ReserveDown();
  LogParents.ReserveDown();
  Items.ReserveDown();
}

int CDirItems::AddPrefix(int phyParent, int logParent, const UString &prefix)
{
  PhyParents.Add(phyParent);
  LogParents.Add(logParent);
  return Prefixes.Add(prefix);
}

void CDirItems::DeleteLastPrefix()
{
  PhyParents.DeleteBack();
  LogParents.DeleteBack();
  Prefixes.DeleteBack();
}

void CDirItems::EnumerateDirectory(int phyParent, int logParent, const FString &phyPrefix,
    FStringVector &errorPaths, CRecordVector<DWORD> &errorCodes)
{
  NFind::CEnumerator enumerator(phyPrefix + FCHAR_ANY_MASK);
  for (;;)
  {
    NFind::CFileInfo fi;
    bool found;
    if (!enumerator.Next(fi, found))
    {
      errorCodes.Add(::GetLastError());
      errorPaths.Add(phyPrefix);
      return;
    }
    if (!found)
      break;
    AddDirFileInfo(phyParent, logParent, fi, Items);
    if (fi.IsDir())
    {
      const FString name2 = fi.Name + FCHAR_PATH_SEPARATOR;
      int parent = AddPrefix(phyParent, logParent, fs2us(name2));
      EnumerateDirectory(parent, parent, phyPrefix + name2, errorPaths, errorCodes);
    }
  }
}

void CDirItems::EnumerateDirItems2(const FString &phyPrefix, const UString &logPrefix,
    const FStringVector &filePaths, FStringVector &errorPaths, CRecordVector<DWORD> &errorCodes)
{
  int phyParent = phyPrefix.IsEmpty() ? -1 : AddPrefix(-1, -1, fs2us(phyPrefix));
  int logParent = logPrefix.IsEmpty() ? -1 : AddPrefix(-1, -1, logPrefix);

  for (int i = 0; i < filePaths.Size(); i++)
  {
    const FString &filePath = filePaths[i];
    NFind::CFileInfo fi;
    const FString phyPath = phyPrefix + filePath;
    if (!fi.Find(phyPath))
    {
      errorCodes.Add(::GetLastError());
      errorPaths.Add(phyPath);
      continue;
    }
    int delimiter = filePath.ReverseFind(FCHAR_PATH_SEPARATOR);
    FString phyPrefixCur;
    int phyParentCur = phyParent;
    if (delimiter >= 0)
    {
      phyPrefixCur = filePath.Left(delimiter + 1);
      phyParentCur = AddPrefix(phyParent, logParent, fs2us(phyPrefixCur));
    }
    AddDirFileInfo(phyParentCur, logParent, fi, Items);
    if (fi.IsDir())
    {
      const FString name2 = fi.Name + FCHAR_PATH_SEPARATOR;
      int parent = AddPrefix(phyParentCur, logParent, fs2us(name2));
      EnumerateDirectory(parent, parent, phyPrefix + phyPrefixCur + name2, errorPaths, errorCodes);
    }
  }
  ReserveDown();
}

static HRESULT EnumerateDirItems(const NWildcard::CCensorNode &curNode,
    int phyParent, int logParent, const FString &phyPrefix,
    const UStringVector &addArchivePrefix,
    CDirItems &dirItems,
    bool enterToSubFolders,
    IEnumDirItemCallback *callback,
    FStringVector &errorPaths,
    CRecordVector<DWORD> &errorCodes);

static HRESULT EnumerateDirItems_Spec(const NWildcard::CCensorNode &curNode,
    int phyParent, int logParent, const FString &curFolderName,
    const FString &phyPrefix,
    const UStringVector &addArchivePrefix,
    CDirItems &dirItems,
    bool enterToSubFolders,
    IEnumDirItemCallback *callback,
    FStringVector &errorPaths,
    CRecordVector<DWORD> &errorCodes)
  
{
  const FString name2 = curFolderName + FCHAR_PATH_SEPARATOR;
  int parent = dirItems.AddPrefix(phyParent, logParent, fs2us(name2));
  int numItems = dirItems.Items.Size();
  HRESULT res = EnumerateDirItems(curNode, parent, parent, phyPrefix + name2,
    addArchivePrefix, dirItems, enterToSubFolders, callback, errorPaths, errorCodes);
  if (numItems == dirItems.Items.Size())
    dirItems.DeleteLastPrefix();
  return res;
}


static HRESULT EnumerateDirItems(const NWildcard::CCensorNode &curNode,
    int phyParent, int logParent, const FString &phyPrefix,
    const UStringVector &addArchivePrefix,  // prefix from curNode
    CDirItems &dirItems,
    bool enterToSubFolders,
    IEnumDirItemCallback *callback,
    FStringVector &errorPaths,
    CRecordVector<DWORD> &errorCodes)
{
  if (!enterToSubFolders)
    if (curNode.NeedCheckSubDirs())
      enterToSubFolders = true;
  if (callback)
    RINOK(callback->ScanProgress(dirItems.GetNumFolders(), dirItems.Items.Size(), fs2us(phyPrefix)));

  // try direct_names case at first
  if (addArchivePrefix.IsEmpty() && !enterToSubFolders)
  {
    // check that all names are direct
    int i;
    for (i = 0; i < curNode.IncludeItems.Size(); i++)
    {
      const NWildcard::CItem &item = curNode.IncludeItems[i];
      if (item.Recursive || item.PathParts.Size() != 1)
        break;
      const UString &name = item.PathParts.Front();
      if (name.IsEmpty() || DoesNameContainWildCard(name))
        break;
    }
    if (i == curNode.IncludeItems.Size())
    {
      // all names are direct (no wildcards)
      // so we don't need file_system's dir enumerator
      CRecordVector<bool> needEnterVector;
      for (i = 0; i < curNode.IncludeItems.Size(); i++)
      {
        const NWildcard::CItem &item = curNode.IncludeItems[i];
        const UString &name = item.PathParts.Front();
        const FString fullPath = phyPrefix + us2fs(name);
        NFind::CFileInfo fi;
        if (!fi.Find(fullPath))
        {
          errorCodes.Add(::GetLastError());
          errorPaths.Add(fullPath);
          continue;
        }
        bool isDir = fi.IsDir();
        if (isDir && !item.ForDir || !isDir && !item.ForFile)
        {
          errorCodes.Add((DWORD)E_FAIL);
          errorPaths.Add(fullPath);
          continue;
        }
        {
          UStringVector pathParts;
          pathParts.Add(fs2us(fi.Name));
          if (curNode.CheckPathToRoot(false, pathParts, !isDir))
            continue;
        }
        AddDirFileInfo(phyParent, logParent, fi, dirItems.Items);
        if (!isDir)
          continue;
        
        UStringVector addArchivePrefixNew;
        const NWildcard::CCensorNode *nextNode = 0;
        int index = curNode.FindSubNode(name);
        if (index >= 0)
        {
          for (int t = needEnterVector.Size(); t <= index; t++)
            needEnterVector.Add(true);
          needEnterVector[index] = false;
          nextNode = &curNode.SubNodes[index];
        }
        else
        {
          nextNode = &curNode;
          addArchivePrefixNew.Add(name); // don't change it to fi.Name. It's for shortnames support
        }

        RINOK(EnumerateDirItems_Spec(*nextNode, phyParent, logParent, fi.Name, phyPrefix,
            addArchivePrefixNew, dirItems, true, callback, errorPaths, errorCodes));
      }
      for (i = 0; i < curNode.SubNodes.Size(); i++)
      {
        if (i < needEnterVector.Size())
          if (!needEnterVector[i])
            continue;
        const NWildcard::CCensorNode &nextNode = curNode.SubNodes[i];
        const FString fullPath = phyPrefix + us2fs(nextNode.Name);
        NFind::CFileInfo fi;
        if (!fi.Find(fullPath))
        {
          if (!nextNode.AreThereIncludeItems())
            continue;
          errorCodes.Add(::GetLastError());
          errorPaths.Add(fullPath);
          continue;
        }
        if (!fi.IsDir())
        {
          errorCodes.Add((DWORD)E_FAIL);
          errorPaths.Add(fullPath);
          continue;
        }

        RINOK(EnumerateDirItems_Spec(nextNode, phyParent, logParent, fi.Name, phyPrefix,
            UStringVector(), dirItems, false, callback, errorPaths, errorCodes));
      }
      return S_OK;
    }
  }


  NFind::CEnumerator enumerator(phyPrefix + FCHAR_ANY_MASK);
  for (int ttt = 0; ; ttt++)
  {
    NFind::CFileInfo fi;
    bool found;
    if (!enumerator.Next(fi, found))
    {
      errorCodes.Add(::GetLastError());
      errorPaths.Add(phyPrefix);
      break;
    }
    if (!found)
      break;

    if (callback && (ttt & 0xFF) == 0xFF)
      RINOK(callback->ScanProgress(dirItems.GetNumFolders(), dirItems.Items.Size(), fs2us(phyPrefix)));
    const UString &name = fs2us(fi.Name);
    bool enterToSubFolders2 = enterToSubFolders;
    UStringVector addArchivePrefixNew = addArchivePrefix;
    addArchivePrefixNew.Add(name);
    {
      UStringVector addArchivePrefixNewTemp(addArchivePrefixNew);
      if (curNode.CheckPathToRoot(false, addArchivePrefixNewTemp, !fi.IsDir()))
        continue;
    }
    if (curNode.CheckPathToRoot(true, addArchivePrefixNew, !fi.IsDir()))
    {
      AddDirFileInfo(phyParent, logParent, fi, dirItems.Items);
      if (fi.IsDir())
        enterToSubFolders2 = true;
    }
    if (!fi.IsDir())
      continue;

    const NWildcard::CCensorNode *nextNode = 0;
    if (addArchivePrefix.IsEmpty())
    {
      int index = curNode.FindSubNode(name);
      if (index >= 0)
        nextNode = &curNode.SubNodes[index];
    }
    if (!enterToSubFolders2 && nextNode == 0)
      continue;

    addArchivePrefixNew = addArchivePrefix;
    if (nextNode == 0)
    {
      nextNode = &curNode;
      addArchivePrefixNew.Add(name);
    }

    RINOK(EnumerateDirItems_Spec(*nextNode, phyParent, logParent, fi.Name, phyPrefix,
        addArchivePrefixNew, dirItems, enterToSubFolders2, callback, errorPaths, errorCodes));
  }
  return S_OK;
}

HRESULT EnumerateItems(
    const NWildcard::CCensor &censor,
    CDirItems &dirItems,
    IEnumDirItemCallback *callback,
    FStringVector &errorPaths,
    CRecordVector<DWORD> &errorCodes)
{
  for (int i = 0; i < censor.Pairs.Size(); i++)
  {
    const NWildcard::CPair &pair = censor.Pairs[i];
    int phyParent = pair.Prefix.IsEmpty() ? -1 : dirItems.AddPrefix(-1, -1, pair.Prefix);
    RINOK(EnumerateDirItems(pair.Head, phyParent, -1, us2fs(pair.Prefix), UStringVector(), dirItems, false,
        callback, errorPaths, errorCodes));
  }
  dirItems.ReserveDown();
  return S_OK;
}
