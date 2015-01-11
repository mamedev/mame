// 7zUpdate.cpp

#include "StdAfx.h"

#include "../../../../C/CpuArch.h"

#include "../../Common/LimitedStreams.h"
#include "../../Common/ProgressUtils.h"

#include "../../Common/CreateCoder.h"

#include "../../Compress/CopyCoder.h"

#include "../Common/ItemNameUtils.h"
#include "../Common/OutStreamWithCRC.h"

#include "7zDecode.h"
#include "7zEncode.h"
#include "7zFolderInStream.h"
#include "7zHandler.h"
#include "7zOut.h"
#include "7zUpdate.h"

namespace NArchive {
namespace N7z {

#ifdef MY_CPU_X86_OR_AMD64
#define USE_86_FILTER
#endif

static HRESULT WriteRange(IInStream *inStream, ISequentialOutStream *outStream,
    UInt64 position, UInt64 size, ICompressProgressInfo *progress)
{
  RINOK(inStream->Seek(position, STREAM_SEEK_SET, 0));
  CLimitedSequentialInStream *streamSpec = new CLimitedSequentialInStream;
  CMyComPtr<CLimitedSequentialInStream> inStreamLimited(streamSpec);
  streamSpec->SetStream(inStream);
  streamSpec->Init(size);

  NCompress::CCopyCoder *copyCoderSpec = new NCompress::CCopyCoder;
  CMyComPtr<ICompressCoder> copyCoder = copyCoderSpec;
  RINOK(copyCoder->Code(inStreamLimited, outStream, NULL, NULL, progress));
  return (copyCoderSpec->TotalSize == size ? S_OK : E_FAIL);
}

static int GetReverseSlashPos(const UString &name)
{
  int slashPos = name.ReverseFind(L'/');
  #ifdef _WIN32
  int slash1Pos = name.ReverseFind(L'\\');
  slashPos = MyMax(slashPos, slash1Pos);
  #endif
  return slashPos;
}

int CUpdateItem::GetExtensionPos() const
{
  int slashPos = GetReverseSlashPos(Name);
  int dotPos = Name.ReverseFind(L'.');
  if (dotPos < 0 || (dotPos < slashPos && slashPos >= 0))
    return Name.Length();
  return dotPos + 1;
}

UString CUpdateItem::GetExtension() const
{
  return Name.Mid(GetExtensionPos());
}

#define RINOZ(x) { int __tt = (x); if (__tt != 0) return __tt; }

#define RINOZ_COMP(a, b) RINOZ(MyCompare(a, b))

static int CompareBuffers(const CByteBuffer &a1, const CByteBuffer &a2)
{
  size_t c1 = a1.GetCapacity();
  size_t c2 = a2.GetCapacity();
  RINOZ_COMP(c1, c2);
  for (size_t i = 0; i < c1; i++)
    RINOZ_COMP(a1[i], a2[i]);
  return 0;
}

static int CompareCoders(const CCoderInfo &c1, const CCoderInfo &c2)
{
  RINOZ_COMP(c1.NumInStreams, c2.NumInStreams);
  RINOZ_COMP(c1.NumOutStreams, c2.NumOutStreams);
  RINOZ_COMP(c1.MethodID, c2.MethodID);
  return CompareBuffers(c1.Props, c2.Props);
}

static int CompareBindPairs(const CBindPair &b1, const CBindPair &b2)
{
  RINOZ_COMP(b1.InIndex, b2.InIndex);
  return MyCompare(b1.OutIndex, b2.OutIndex);
}

static int CompareFolders(const CFolder &f1, const CFolder &f2)
{
  int s1 = f1.Coders.Size();
  int s2 = f2.Coders.Size();
  RINOZ_COMP(s1, s2);
  int i;
  for (i = 0; i < s1; i++)
    RINOZ(CompareCoders(f1.Coders[i], f2.Coders[i]));
  s1 = f1.BindPairs.Size();
  s2 = f2.BindPairs.Size();
  RINOZ_COMP(s1, s2);
  for (i = 0; i < s1; i++)
    RINOZ(CompareBindPairs(f1.BindPairs[i], f2.BindPairs[i]));
  return 0;
}

/*
static int CompareFiles(const CFileItem &f1, const CFileItem &f2)
{
  return MyStringCompareNoCase(f1.Name, f2.Name);
}
*/

struct CFolderRepack
{
  int FolderIndex;
  int Group;
  CNum NumCopyFiles;
};

static int CompareFolderRepacks(const CFolderRepack *p1, const CFolderRepack *p2, void *param)
{
  RINOZ_COMP(p1->Group, p2->Group);
  int i1 = p1->FolderIndex;
  int i2 = p2->FolderIndex;
  const CArchiveDatabaseEx &db = *(const CArchiveDatabaseEx *)param;
  RINOZ(CompareFolders(
      db.Folders[i1],
      db.Folders[i2]));
  return MyCompare(i1, i2);
  /*
  RINOZ_COMP(
      db.NumUnpackStreamsVector[i1],
      db.NumUnpackStreamsVector[i2]);
  if (db.NumUnpackStreamsVector[i1] == 0)
    return 0;
  return CompareFiles(
      db.Files[db.FolderStartFileIndex[i1]],
      db.Files[db.FolderStartFileIndex[i2]]);
  */
}

////////////////////////////////////////////////////////////

static int CompareEmptyItems(const int *p1, const int *p2, void *param)
{
  const CObjectVector<CUpdateItem> &updateItems = *(const CObjectVector<CUpdateItem> *)param;
  const CUpdateItem &u1 = updateItems[*p1];
  const CUpdateItem &u2 = updateItems[*p2];
  if (u1.IsDir != u2.IsDir)
    return (u1.IsDir) ? 1 : -1;
  if (u1.IsDir)
  {
    if (u1.IsAnti != u2.IsAnti)
      return (u1.IsAnti ? 1 : -1);
    int n = MyStringCompareNoCase(u1.Name, u2.Name);
    return -n;
  }
  if (u1.IsAnti != u2.IsAnti)
    return (u1.IsAnti ? 1 : -1);
  return MyStringCompareNoCase(u1.Name, u2.Name);
}

static const char *g_Exts =
  " lzma 7z ace arc arj bz bz2 deb lzo lzx gz pak rpm sit tgz tbz tbz2 tgz cab ha lha lzh rar zoo"
  " zip jar ear war msi"
  " 3gp avi mov mpeg mpg mpe wmv"
  " aac ape fla flac la mp3 m4a mp4 ofr ogg pac ra rm rka shn swa tta wv wma wav"
  " swf "
  " chm hxi hxs"
  " gif jpeg jpg jp2 png tiff  bmp ico psd psp"
  " awg ps eps cgm dxf svg vrml wmf emf ai md"
  " cad dwg pps key sxi"
  " max 3ds"
  " iso bin nrg mdf img pdi tar cpio xpi"
  " vfd vhd vud vmc vsv"
  " vmdk dsk nvram vmem vmsd vmsn vmss vmtm"
  " inl inc idl acf asa h hpp hxx c cpp cxx rc java cs pas bas vb cls ctl frm dlg def"
  " f77 f f90 f95"
  " asm sql manifest dep "
  " mak clw csproj vcproj sln dsp dsw "
  " class "
  " bat cmd"
  " xml xsd xsl xslt hxk hxc htm html xhtml xht mht mhtml htw asp aspx css cgi jsp shtml"
  " awk sed hta js php php3 php4 php5 phptml pl pm py pyo rb sh tcl vbs"
  " text txt tex ans asc srt reg ini doc docx mcw dot rtf hlp xls xlr xlt xlw ppt pdf"
  " sxc sxd sxi sxg sxw stc sti stw stm odt ott odg otg odp otp ods ots odf"
  " abw afp cwk lwp wpd wps wpt wrf wri"
  " abf afm bdf fon mgf otf pcf pfa snf ttf"
  " dbf mdb nsf ntf wdb db fdb gdb"
  " exe dll ocx vbx sfx sys tlb awx com obj lib out o so "
  " pdb pch idb ncb opt";

int GetExtIndex(const char *ext)
{
  int extIndex = 1;
  const char *p = g_Exts;
  for (;;)
  {
    char c = *p++;
    if (c == 0)
      return extIndex;
    if (c == ' ')
      continue;
    int pos = 0;
    for (;;)
    {
      char c2 = ext[pos++];
      if (c2 == 0 && (c == 0 || c == ' '))
        return extIndex;
      if (c != c2)
        break;
      c = *p++;
    }
    extIndex++;
    for (;;)
    {
      if (c == 0)
        return extIndex;
      if (c == ' ')
        break;
      c = *p++;
    }
  }
}

struct CRefItem
{
  const CUpdateItem *UpdateItem;
  UInt32 Index;
  UInt32 ExtensionPos;
  UInt32 NamePos;
  int ExtensionIndex;
  CRefItem(UInt32 index, const CUpdateItem &ui, bool sortByType):
    UpdateItem(&ui),
    Index(index),
    ExtensionPos(0),
    NamePos(0),
    ExtensionIndex(0)
  {
    if (sortByType)
    {
      int slashPos = GetReverseSlashPos(ui.Name);
      NamePos = ((slashPos >= 0) ? (slashPos + 1) : 0);
      int dotPos = ui.Name.ReverseFind(L'.');
      if (dotPos < 0 || (dotPos < slashPos && slashPos >= 0))
        ExtensionPos = ui.Name.Length();
      else
      {
        ExtensionPos = dotPos + 1;
        UString us = ui.Name.Mid(ExtensionPos);
        if (!us.IsEmpty())
        {
          us.MakeLower();
          int i;
          AString s;
          for (i = 0; i < us.Length(); i++)
          {
            wchar_t c = us[i];
            if (c >= 0x80)
              break;
            s += (char)c;
          }
          if (i == us.Length())
            ExtensionIndex = GetExtIndex(s);
          else
            ExtensionIndex = 0;
        }
      }
    }
  }
};

static int CompareUpdateItems(const CRefItem *p1, const CRefItem *p2, void *param)
{
  const CRefItem &a1 = *p1;
  const CRefItem &a2 = *p2;
  const CUpdateItem &u1 = *a1.UpdateItem;
  const CUpdateItem &u2 = *a2.UpdateItem;
  int n;
  if (u1.IsDir != u2.IsDir)
    return (u1.IsDir) ? 1 : -1;
  if (u1.IsDir)
  {
    if (u1.IsAnti != u2.IsAnti)
      return (u1.IsAnti ? 1 : -1);
    n = MyStringCompareNoCase(u1.Name, u2.Name);
    return -n;
  }
  bool sortByType = *(bool *)param;
  if (sortByType)
  {
    RINOZ_COMP(a1.ExtensionIndex, a2.ExtensionIndex);
    RINOZ(MyStringCompareNoCase(u1.Name + a1.ExtensionPos, u2.Name + a2.ExtensionPos));
    RINOZ(MyStringCompareNoCase(u1.Name + a1.NamePos, u2.Name + a2.NamePos));
    if (!u1.MTimeDefined && u2.MTimeDefined) return 1;
    if (u1.MTimeDefined && !u2.MTimeDefined) return -1;
    if (u1.MTimeDefined && u2.MTimeDefined) RINOZ_COMP(u1.MTime, u2.MTime);
    RINOZ_COMP(u1.Size, u2.Size);
  }
  return MyStringCompareNoCase(u1.Name, u2.Name);
}

struct CSolidGroup
{
  CRecordVector<UInt32> Indices;
};

static wchar_t *g_ExeExts[] =
{
  L"dll",
  L"exe",
  L"ocx",
  L"sfx",
  L"sys"
};

static bool IsExeExt(const UString &ext)
{
  for (int i = 0; i < sizeof(g_ExeExts) / sizeof(g_ExeExts[0]); i++)
    if (ext.CompareNoCase(g_ExeExts[i]) == 0)
      return true;
  return false;
}


static inline void GetMethodFull(UInt64 methodID, UInt32 numInStreams, CMethodFull &m)
{
  m.Id = methodID;
  m.NumInStreams = numInStreams;
  m.NumOutStreams = 1;
}

static void AddBcj2Methods(CCompressionMethodMode &mode)
{
  CMethodFull m;
  GetMethodFull(k_LZMA, 1, m);
  
  m.AddProp32(NCoderPropID::kDictionarySize, 1 << 20);
  m.AddProp32(NCoderPropID::kNumFastBytes, 128);
  m.AddProp32(NCoderPropID::kNumThreads, 1);
  m.AddProp32(NCoderPropID::kLitPosBits, 2);
  m.AddProp32(NCoderPropID::kLitContextBits, 0);
  // m.AddPropString(NCoderPropID::kMatchFinder, L"BT2");

  mode.Methods.Add(m);
  mode.Methods.Add(m);
  
  CBind bind;
  bind.OutCoder = 0;
  bind.InStream = 0;
  bind.InCoder = 1;  bind.OutStream = 0;  mode.Binds.Add(bind);
  bind.InCoder = 2;  bind.OutStream = 1;  mode.Binds.Add(bind);
  bind.InCoder = 3;  bind.OutStream = 2;  mode.Binds.Add(bind);
}

static void MakeExeMethod(CCompressionMethodMode &mode,
    bool useFilters, bool addFilter, bool bcj2Filter)
{
  if (!mode.Binds.IsEmpty() || !useFilters || mode.Methods.Size() > 2)
    return;
  if (mode.Methods.Size() == 2)
  {
    if (mode.Methods[0].Id == k_BCJ2)
      AddBcj2Methods(mode);
    return;
  }
  if (!addFilter)
    return;
  bcj2Filter = bcj2Filter;
  #ifdef USE_86_FILTER
  if (bcj2Filter)
  {
    CMethodFull m;
    GetMethodFull(k_BCJ2, 4, m);
    mode.Methods.Insert(0, m);
    AddBcj2Methods(mode);
  }
  else
  {
    CMethodFull m;
    GetMethodFull(k_BCJ, 1, m);
    mode.Methods.Insert(0, m);
    CBind bind;
    bind.OutCoder = 0;
    bind.InStream = 0;
    bind.InCoder = 1;
    bind.OutStream = 0;
    mode.Binds.Add(bind);
  }
  #endif
}


static void FromUpdateItemToFileItem(const CUpdateItem &ui,
    CFileItem &file, CFileItem2 &file2)
{
  file.Name = NItemName::MakeLegalName(ui.Name);
  if (ui.AttribDefined)
    file.SetAttrib(ui.Attrib);
  
  file2.CTime = ui.CTime;  file2.CTimeDefined = ui.CTimeDefined;
  file2.ATime = ui.ATime;  file2.ATimeDefined = ui.ATimeDefined;
  file2.MTime = ui.MTime;  file2.MTimeDefined = ui.MTimeDefined;
  file2.IsAnti = ui.IsAnti;
  file2.StartPosDefined = false;

  file.Size = ui.Size;
  file.IsDir = ui.IsDir;
  file.HasStream = ui.HasStream();
}

class CFolderOutStream2:
  public ISequentialOutStream,
  public CMyUnknownImp
{
  COutStreamWithCRC *_crcStreamSpec;
  CMyComPtr<ISequentialOutStream> _crcStream;
  const CArchiveDatabaseEx *_db;
  const CBoolVector *_extractStatuses;
  CMyComPtr<ISequentialOutStream> _outStream;
  UInt32 _startIndex;
  int _currentIndex;
  bool _fileIsOpen;
  UInt64 _rem;

  void OpenFile();
  void CloseFile();
  HRESULT CloseFileAndSetResult();
  HRESULT ProcessEmptyFiles();
public:
  MY_UNKNOWN_IMP
  
  CFolderOutStream2()
  {
    _crcStreamSpec = new COutStreamWithCRC;
    _crcStream = _crcStreamSpec;
  }

  HRESULT Init(const CArchiveDatabaseEx *db, UInt32 startIndex,
      const CBoolVector *extractStatuses, ISequentialOutStream *outStream);
  void ReleaseOutStream();
  HRESULT CheckFinishedState() const { return (_currentIndex == _extractStatuses->Size()) ? S_OK: E_FAIL; }

  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

HRESULT CFolderOutStream2::Init(const CArchiveDatabaseEx *db, UInt32 startIndex,
    const CBoolVector *extractStatuses, ISequentialOutStream *outStream)
{
  _db = db;
  _startIndex = startIndex;
  _extractStatuses = extractStatuses;
  _outStream = outStream;

  _currentIndex = 0;
  _fileIsOpen = false;
  return ProcessEmptyFiles();
}

void CFolderOutStream2::ReleaseOutStream()
{
  _outStream.Release();
  _crcStreamSpec->ReleaseStream();
}

void CFolderOutStream2::OpenFile()
{
  _crcStreamSpec->SetStream((*_extractStatuses)[_currentIndex] ? _outStream : NULL);
  _crcStreamSpec->Init(true);
  _fileIsOpen = true;
  _rem = _db->Files[_startIndex + _currentIndex].Size;
}

void CFolderOutStream2::CloseFile()
{
  _crcStreamSpec->ReleaseStream();
  _fileIsOpen = false;
  _currentIndex++;
}

HRESULT CFolderOutStream2::CloseFileAndSetResult()
{
  const CFileItem &file = _db->Files[_startIndex + _currentIndex];
  CloseFile();
  return (file.IsDir || !file.CrcDefined || file.Crc == _crcStreamSpec->GetCRC()) ? S_OK: S_FALSE;
}

HRESULT CFolderOutStream2::ProcessEmptyFiles()
{
  while (_currentIndex < _extractStatuses->Size() && _db->Files[_startIndex + _currentIndex].Size == 0)
  {
    OpenFile();
    RINOK(CloseFileAndSetResult());
  }
  return S_OK;
}

STDMETHODIMP CFolderOutStream2::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize != NULL)
    *processedSize = 0;
  while (size != 0)
  {
    if (_fileIsOpen)
    {
      UInt32 cur = size < _rem ? size : (UInt32)_rem;
      RINOK(_crcStream->Write(data, cur, &cur));
      if (cur == 0)
        break;
      data = (const Byte *)data + cur;
      size -= cur;
      _rem -= cur;
      if (processedSize != NULL)
        *processedSize += cur;
      if (_rem == 0)
      {
        RINOK(CloseFileAndSetResult());
        RINOK(ProcessEmptyFiles());
        continue;
      }
    }
    else
    {
      RINOK(ProcessEmptyFiles());
      if (_currentIndex == _extractStatuses->Size())
      {
        // we don't support partial extracting
        return E_FAIL;
      }
      OpenFile();
    }
  }
  return S_OK;
}

class CThreadDecoder: public CVirtThread
{
public:
  HRESULT Result;
  CMyComPtr<IInStream> InStream;

  CFolderOutStream2 *FosSpec;
  CMyComPtr<ISequentialOutStream> Fos;

  UInt64 StartPos;
  const UInt64 *PackSizes;
  const CFolder *Folder;
  #ifndef _NO_CRYPTO
  CMyComPtr<ICryptoGetTextPassword> GetTextPassword;
  #endif

  DECL_EXTERNAL_CODECS_VARS
  CDecoder Decoder;

  #ifndef _7ZIP_ST
  bool MtMode;
  UInt32 NumThreads;
  #endif

  CThreadDecoder():
    Decoder(true)
  {
    #ifndef _7ZIP_ST
    MtMode = false;
    NumThreads = 1;
    #endif
    FosSpec = new CFolderOutStream2;
    Fos = FosSpec;
    Result = E_FAIL;
  }
  ~CThreadDecoder() { CVirtThread::WaitThreadFinish(); }
  virtual void Execute();
};

void CThreadDecoder::Execute()
{
  try
  {
    #ifndef _NO_CRYPTO
    bool passwordIsDefined;
    #endif
    Result = Decoder.Decode(
      EXTERNAL_CODECS_VARS
      InStream,
      StartPos,
      PackSizes,
      *Folder,
      Fos,
      NULL
      #ifndef _NO_CRYPTO
      , GetTextPassword, passwordIsDefined
      #endif
      #ifndef _7ZIP_ST
      , MtMode, NumThreads
      #endif
      );
  }
  catch(...)
  {
    Result = E_FAIL;
  }
  if (Result == S_OK)
    Result = FosSpec->CheckFinishedState();
  FosSpec->ReleaseOutStream();
}

bool static Is86FilteredFolder(const CFolder &f)
{
  for (int i = 0; i < f.Coders.Size(); i++)
  {
    CMethodId m = f.Coders[i].MethodID;
    if (m == k_BCJ || m == k_BCJ2)
      return true;
  }
  return false;
}

#ifndef _NO_CRYPTO

class CCryptoGetTextPassword:
  public ICryptoGetTextPassword,
  public CMyUnknownImp
{
public:
  UString Password;

  MY_UNKNOWN_IMP
  STDMETHOD(CryptoGetTextPassword)(BSTR *password);
};

STDMETHODIMP CCryptoGetTextPassword::CryptoGetTextPassword(BSTR *password)
{
  return StringToBstr(Password, password);
}

#endif

static const int kNumGroupsMax = 4;

static bool Is86Group(int group) { return (group & 1) != 0; }
static bool IsEncryptedGroup(int group) { return (group & 2) != 0; }
static int GetGroupIndex(bool encrypted, int bcjFiltered)
  { return (encrypted ? 2 : 0) + (bcjFiltered ? 1 : 0); }

HRESULT Update(
    DECL_EXTERNAL_CODECS_LOC_VARS
    IInStream *inStream,
    const CArchiveDatabaseEx *db,
    const CObjectVector<CUpdateItem> &updateItems,
    COutArchive &archive,
    CArchiveDatabase &newDatabase,
    ISequentialOutStream *seqOutStream,
    IArchiveUpdateCallback *updateCallback,
    const CUpdateOptions &options
    #ifndef _NO_CRYPTO
    , ICryptoGetTextPassword *getDecoderPassword
    #endif
    )
{
  UInt64 numSolidFiles = options.NumSolidFiles;
  if (numSolidFiles == 0)
    numSolidFiles = 1;
  /*
  CMyComPtr<IOutStream> outStream;
  RINOK(seqOutStream->QueryInterface(IID_IOutStream, (void **)&outStream));
  if (!outStream)
    return E_NOTIMPL;
  */

  UInt64 startBlockSize = db != 0 ? db->ArchiveInfo.StartPosition: 0;
  if (startBlockSize > 0 && !options.RemoveSfxBlock)
  {
    RINOK(WriteRange(inStream, seqOutStream, 0, startBlockSize, NULL));
  }

  CRecordVector<int> fileIndexToUpdateIndexMap;
  CRecordVector<CFolderRepack> folderRefs;
  UInt64 complexity = 0;
  UInt64 inSizeForReduce2 = 0;
  bool needEncryptedRepack = false;
  if (db != 0)
  {
    fileIndexToUpdateIndexMap.Reserve(db->Files.Size());
    int i;
    for (i = 0; i < db->Files.Size(); i++)
      fileIndexToUpdateIndexMap.Add(-1);

    for (i = 0; i < updateItems.Size(); i++)
    {
      int index = updateItems[i].IndexInArchive;
      if (index != -1)
        fileIndexToUpdateIndexMap[index] = i;
    }

    for (i = 0; i < db->Folders.Size(); i++)
    {
      CNum indexInFolder = 0;
      CNum numCopyItems = 0;
      CNum numUnpackStreams = db->NumUnpackStreamsVector[i];
      UInt64 repackSize = 0;
      for (CNum fi = db->FolderStartFileIndex[i]; indexInFolder < numUnpackStreams; fi++)
      {
        const CFileItem &file = db->Files[fi];
        if (file.HasStream)
        {
          indexInFolder++;
          int updateIndex = fileIndexToUpdateIndexMap[fi];
          if (updateIndex >= 0 && !updateItems[updateIndex].NewData)
          {
            numCopyItems++;
            repackSize += file.Size;
          }
        }
      }

      if (numCopyItems == 0)
        continue;

      CFolderRepack rep;
      rep.FolderIndex = i;
      rep.NumCopyFiles = numCopyItems;
      const CFolder &f = db->Folders[i];
      bool isEncrypted = f.IsEncrypted();
      rep.Group = GetGroupIndex(isEncrypted, Is86FilteredFolder(f));
      folderRefs.Add(rep);
      if (numCopyItems == numUnpackStreams)
        complexity += db->GetFolderFullPackSize(i);
      else
      {
        complexity += repackSize;
        if (repackSize > inSizeForReduce2)
          inSizeForReduce2 = repackSize;
        if (isEncrypted)
          needEncryptedRepack = true;
      }
    }
    folderRefs.Sort(CompareFolderRepacks, (void *)db);
  }

  UInt64 inSizeForReduce = 0;
  int i;
  for (i = 0; i < updateItems.Size(); i++)
  {
    const CUpdateItem &ui = updateItems[i];
    if (ui.NewData)
    {
      complexity += ui.Size;
      if (numSolidFiles != 1)
        inSizeForReduce += ui.Size;
      else if (ui.Size > inSizeForReduce)
        inSizeForReduce = ui.Size;
    }
  }

  if (inSizeForReduce2 > inSizeForReduce)
    inSizeForReduce = inSizeForReduce2;

  RINOK(updateCallback->SetTotal(complexity));

  CLocalProgress *lps = new CLocalProgress;
  CMyComPtr<ICompressProgressInfo> progress = lps;
  lps->Init(updateCallback, true);

  CStreamBinder sb;
  RINOK(sb.CreateEvents());

  CThreadDecoder threadDecoder;
  if (!folderRefs.IsEmpty())
  {
    #ifdef EXTERNAL_CODECS
    threadDecoder._codecsInfo = codecsInfo;
    threadDecoder._externalCodecs = *externalCodecs;
    #endif
    RINOK(threadDecoder.Create());
  }

  CObjectVector<CSolidGroup> groups;
  for (i = 0; i < kNumGroupsMax; i++)
    groups.Add(CSolidGroup());

  {
    // ---------- Split files to 2 groups ----------

    bool useFilters = options.UseFilters;
    const CCompressionMethodMode &method = *options.Method;
    if (method.Methods.Size() != 1 || method.Binds.Size() != 0)
      useFilters = false;
    for (i = 0; i < updateItems.Size(); i++)
    {
      const CUpdateItem &ui = updateItems[i];
      if (!ui.NewData || !ui.HasStream())
        continue;
      bool filteredGroup = false;
      if (useFilters)
      {
        int dotPos = ui.Name.ReverseFind(L'.');
        if (dotPos >= 0)
          filteredGroup = IsExeExt(ui.Name.Mid(dotPos + 1));
      }
      groups[GetGroupIndex(method.PasswordIsDefined, filteredGroup)].Indices.Add(i);
    }
  }

  #ifndef _NO_CRYPTO

  CCryptoGetTextPassword *getPasswordSpec = NULL;
  if (needEncryptedRepack)
  {
    getPasswordSpec = new CCryptoGetTextPassword;
    threadDecoder.GetTextPassword = getPasswordSpec;

    if (options.Method->PasswordIsDefined)
      getPasswordSpec->Password = options.Method->Password;
    else
    {
      if (!getDecoderPassword)
        return E_NOTIMPL;
      CMyComBSTR password;
      RINOK(getDecoderPassword->CryptoGetTextPassword(&password));
      getPasswordSpec->Password = password;
    }
  }

  #endif

  // ---------- Compress ----------

  RINOK(archive.Create(seqOutStream, false));
  RINOK(archive.SkipPrefixArchiveHeader());

  int folderRefIndex = 0;
  lps->ProgressOffset = 0;

  for (int groupIndex = 0; groupIndex < kNumGroupsMax; groupIndex++)
  {
    const CSolidGroup &group = groups[groupIndex];

    CCompressionMethodMode method = *options.Method;
    MakeExeMethod(method, options.UseFilters, Is86Group(groupIndex), options.MaxFilter);

    if (IsEncryptedGroup(groupIndex))
    {
      if (!method.PasswordIsDefined)
      {
        #ifndef _NO_CRYPTO
        if (getPasswordSpec)
          method.Password = getPasswordSpec->Password;
        #endif
        method.PasswordIsDefined = true;
      }
    }
    else
    {
      method.PasswordIsDefined = false;
      method.Password.Empty();
    }

    CEncoder encoder(method);

    for (; folderRefIndex < folderRefs.Size(); folderRefIndex++)
    {
      const CFolderRepack &rep = folderRefs[folderRefIndex];
      if (rep.Group != groupIndex)
        break;
      int folderIndex = rep.FolderIndex;
      
      if (rep.NumCopyFiles == db->NumUnpackStreamsVector[folderIndex])
      {
        UInt64 packSize = db->GetFolderFullPackSize(folderIndex);
        RINOK(WriteRange(inStream, archive.SeqStream,
          db->GetFolderStreamPos(folderIndex, 0), packSize, progress));
        lps->ProgressOffset += packSize;
        
        const CFolder &folder = db->Folders[folderIndex];
        CNum startIndex = db->FolderStartPackStreamIndex[folderIndex];
        for (int j = 0; j < folder.PackStreams.Size(); j++)
        {
          newDatabase.PackSizes.Add(db->PackSizes[startIndex + j]);
          // newDatabase.PackCRCsDefined.Add(db.PackCRCsDefined[startIndex + j]);
          // newDatabase.PackCRCs.Add(db.PackCRCs[startIndex + j]);
        }
        newDatabase.Folders.Add(folder);
      }
      else
      {
        CBoolVector extractStatuses;
        
        CNum numUnpackStreams = db->NumUnpackStreamsVector[folderIndex];
        CNum indexInFolder = 0;
        
        for (CNum fi = db->FolderStartFileIndex[folderIndex]; indexInFolder < numUnpackStreams; fi++)
        {
          bool needExtract = false;
          if (db->Files[fi].HasStream)
          {
            indexInFolder++;
            int updateIndex = fileIndexToUpdateIndexMap[fi];
            if (updateIndex >= 0 && !updateItems[updateIndex].NewData)
              needExtract = true;
          }
          extractStatuses.Add(needExtract);
        }

        int startPackIndex = newDatabase.PackSizes.Size();
        CFolder newFolder;
        {
          CMyComPtr<ISequentialInStream> sbInStream;
          {
            CMyComPtr<ISequentialOutStream> sbOutStream;
            sb.CreateStreams(&sbInStream, &sbOutStream);
            sb.ReInit();
            RINOK(threadDecoder.FosSpec->Init(db, db->FolderStartFileIndex[folderIndex], &extractStatuses, sbOutStream));
          }
          
          threadDecoder.InStream = inStream;
          threadDecoder.Folder = &db->Folders[folderIndex];
          threadDecoder.StartPos = db->GetFolderStreamPos(folderIndex, 0);
          threadDecoder.PackSizes = &db->PackSizes[db->FolderStartPackStreamIndex[folderIndex]];
          
          threadDecoder.Start();
          
          RINOK(encoder.Encode(
            EXTERNAL_CODECS_LOC_VARS
            sbInStream, NULL, &inSizeForReduce, newFolder,
            archive.SeqStream, newDatabase.PackSizes, progress));
          
          threadDecoder.WaitExecuteFinish();
        }

        RINOK(threadDecoder.Result);

        for (; startPackIndex < newDatabase.PackSizes.Size(); startPackIndex++)
          lps->OutSize += newDatabase.PackSizes[startPackIndex];
        lps->InSize += newFolder.GetUnpackSize();
        
        newDatabase.Folders.Add(newFolder);
      }
      
      newDatabase.NumUnpackStreamsVector.Add(rep.NumCopyFiles);
      
      CNum numUnpackStreams = db->NumUnpackStreamsVector[folderIndex];
      
      CNum indexInFolder = 0;
      for (CNum fi = db->FolderStartFileIndex[folderIndex]; indexInFolder < numUnpackStreams; fi++)
      {
        CFileItem file;
        CFileItem2 file2;
        db->GetFile(fi, file, file2);
        if (file.HasStream)
        {
          indexInFolder++;
          int updateIndex = fileIndexToUpdateIndexMap[fi];
          if (updateIndex >= 0)
          {
            const CUpdateItem &ui = updateItems[updateIndex];
            if (ui.NewData)
              continue;
            if (ui.NewProps)
            {
              CFileItem uf;
              FromUpdateItemToFileItem(ui, uf, file2);
              uf.Size = file.Size;
              uf.Crc = file.Crc;
              uf.CrcDefined = file.CrcDefined;
              uf.HasStream = file.HasStream;
              file = uf;
            }
            newDatabase.AddFile(file, file2);
          }
        }
      }
    }

    int numFiles = group.Indices.Size();
    if (numFiles == 0)
      continue;
    CRecordVector<CRefItem> refItems;
    refItems.Reserve(numFiles);
    bool sortByType = (numSolidFiles > 1);
    for (i = 0; i < numFiles; i++)
      refItems.Add(CRefItem(group.Indices[i], updateItems[group.Indices[i]], sortByType));
    refItems.Sort(CompareUpdateItems, (void *)&sortByType);
    
    CRecordVector<UInt32> indices;
    indices.Reserve(numFiles);

    for (i = 0; i < numFiles; i++)
    {
      UInt32 index = refItems[i].Index;
      indices.Add(index);
      /*
      const CUpdateItem &ui = updateItems[index];
      CFileItem file;
      if (ui.NewProps)
        FromUpdateItemToFileItem(ui, file);
      else
        file = db.Files[ui.IndexInArchive];
      if (file.IsAnti || file.IsDir)
        return E_FAIL;
      newDatabase.Files.Add(file);
      */
    }
    
    for (i = 0; i < numFiles;)
    {
      UInt64 totalSize = 0;
      int numSubFiles;
      UString prevExtension;
      for (numSubFiles = 0; i + numSubFiles < numFiles &&
          numSubFiles < numSolidFiles; numSubFiles++)
      {
        const CUpdateItem &ui = updateItems[indices[i + numSubFiles]];
        totalSize += ui.Size;
        if (totalSize > options.NumSolidBytes)
          break;
        if (options.SolidExtension)
        {
          UString ext = ui.GetExtension();
          if (numSubFiles == 0)
            prevExtension = ext;
          else
            if (ext.CompareNoCase(prevExtension) != 0)
              break;
        }
      }
      if (numSubFiles < 1)
        numSubFiles = 1;

      CFolderInStream *inStreamSpec = new CFolderInStream;
      CMyComPtr<ISequentialInStream> solidInStream(inStreamSpec);
      inStreamSpec->Init(updateCallback, &indices[i], numSubFiles);
      
      CFolder folderItem;

      int startPackIndex = newDatabase.PackSizes.Size();
      RINOK(encoder.Encode(
          EXTERNAL_CODECS_LOC_VARS
          solidInStream, NULL, &inSizeForReduce, folderItem,
          archive.SeqStream, newDatabase.PackSizes, progress));

      for (; startPackIndex < newDatabase.PackSizes.Size(); startPackIndex++)
        lps->OutSize += newDatabase.PackSizes[startPackIndex];

      lps->InSize += folderItem.GetUnpackSize();
      // for ()
      // newDatabase.PackCRCsDefined.Add(false);
      // newDatabase.PackCRCs.Add(0);
      
      newDatabase.Folders.Add(folderItem);
      
      CNum numUnpackStreams = 0;
      for (int subIndex = 0; subIndex < numSubFiles; subIndex++)
      {
        const CUpdateItem &ui = updateItems[indices[i + subIndex]];
        CFileItem file;
        CFileItem2 file2;
        if (ui.NewProps)
          FromUpdateItemToFileItem(ui, file, file2);
        else
          db->GetFile(ui.IndexInArchive, file, file2);
        if (file2.IsAnti || file.IsDir)
          return E_FAIL;
        
        /*
        CFileItem &file = newDatabase.Files[
              startFileIndexInDatabase + i + subIndex];
        */
        if (!inStreamSpec->Processed[subIndex])
        {
          continue;
          // file.Name += L".locked";
        }

        file.Crc = inStreamSpec->CRCs[subIndex];
        file.Size = inStreamSpec->Sizes[subIndex];
        if (file.Size != 0)
        {
          file.CrcDefined = true;
          file.HasStream = true;
          numUnpackStreams++;
        }
        else
        {
          file.CrcDefined = false;
          file.HasStream = false;
        }
        newDatabase.AddFile(file, file2);
      }
      // numUnpackStreams = 0 is very bad case for locked files
      // v3.13 doesn't understand it.
      newDatabase.NumUnpackStreamsVector.Add(numUnpackStreams);
      i += numSubFiles;
    }
  }

  if (folderRefIndex != folderRefs.Size())
    return E_FAIL;

  RINOK(lps->SetCur());

  /*
  folderRefs.ClearAndFree();
  fileIndexToUpdateIndexMap.ClearAndFree();
  groups.ClearAndFree();
  */

  {
    // ---------- Write Folders & Empty Files ----------
    
    CRecordVector<int> emptyRefs;
    for (i = 0; i < updateItems.Size(); i++)
    {
      const CUpdateItem &ui = updateItems[i];
      if (ui.NewData)
      {
        if (ui.HasStream())
          continue;
      }
      else if (ui.IndexInArchive != -1 && db->Files[ui.IndexInArchive].HasStream)
        continue;
      emptyRefs.Add(i);
    }
    emptyRefs.Sort(CompareEmptyItems, (void *)&updateItems);
    for (i = 0; i < emptyRefs.Size(); i++)
    {
      const CUpdateItem &ui = updateItems[emptyRefs[i]];
      CFileItem file;
      CFileItem2 file2;
      if (ui.NewProps)
        FromUpdateItemToFileItem(ui, file, file2);
      else
        db->GetFile(ui.IndexInArchive, file, file2);
      newDatabase.AddFile(file, file2);
    }
  }

  newDatabase.ReserveDown();
  return S_OK;
}

}}
