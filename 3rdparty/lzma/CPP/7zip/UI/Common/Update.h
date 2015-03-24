// Update.h

#ifndef __COMMON_UPDATE_H
#define __COMMON_UPDATE_H

#include "Common/Wildcard.h"

#include "ArchiveOpenCallback.h"
#include "LoadCodecs.h"
#include "Property.h"
#include "UpdateAction.h"
#include "UpdateCallback.h"

struct CArchivePath
{
  UString OriginalPath;

  UString Prefix;   // path(folder) prefix including slash
  UString Name; // base name
  UString BaseExtension; // archive type extension or "exe" extension
  UString VolExtension;  // archive type extension for volumes

  bool Temp;
  FString TempPrefix;  // path(folder) for temp location
  FString TempPostfix;

  CArchivePath(): Temp(false) {};
  
  void ParseFromPath(const UString &path)
  {
    OriginalPath = path;

    SplitPathToParts(path, Prefix, Name);
    int dotPos = Name.ReverseFind(L'.');
    if (dotPos < 0)
      return;
    if (dotPos == Name.Length() - 1)
    {
      Name = Name.Left(dotPos);
      BaseExtension.Empty();
      return;
    }
    if (BaseExtension.CompareNoCase(Name.Mid(dotPos + 1)) == 0)
    {
      BaseExtension = Name.Mid(dotPos + 1);
      Name = Name.Left(dotPos);
    }
    else
      BaseExtension.Empty();
  }

  UString GetPathWithoutExt() const
  {
    return Prefix + Name;
  }

  UString GetFinalPath() const
  {
    UString path = GetPathWithoutExt();
    if (!BaseExtension.IsEmpty())
      path += UString(L'.') + BaseExtension;
    return path;
  }

  
  FString GetTempPath() const
  {
    FString path = TempPrefix + us2fs(Name);
    if (!BaseExtension.IsEmpty())
      path += FString(FTEXT('.')) + us2fs(BaseExtension);
    path += FTEXT(".tmp");
    path += TempPostfix;
    return path;
  }
};

struct CUpdateArchiveCommand
{
  UString UserArchivePath;
  CArchivePath ArchivePath;
  NUpdateArchive::CActionSet ActionSet;
};

struct CCompressionMethodMode
{
  int FormatIndex;
  CObjectVector<CProperty> Properties;
  CCompressionMethodMode(): FormatIndex(-1) {}
};

struct CUpdateOptions
{
  CCompressionMethodMode MethodMode;

  CObjectVector<CUpdateArchiveCommand> Commands;
  bool UpdateArchiveItself;
  CArchivePath ArchivePath;
  
  bool SfxMode;
  FString SfxModule;
  
  bool OpenShareForWrite;

  bool StdInMode;
  UString StdInFileName;
  bool StdOutMode;
  
  bool EMailMode;
  bool EMailRemoveAfter;
  UString EMailAddress;

  FString WorkingDir;

  bool Init(const CCodecs *codecs, const CIntVector &formatIndices, const UString &arcPath);

  CUpdateOptions():
    UpdateArchiveItself(true),
    SfxMode(false),
    StdInMode(false),
    StdOutMode(false),
    EMailMode(false),
    EMailRemoveAfter(false),
    OpenShareForWrite(false)
      {};

  void SetAddActionCommand()
  {
    Commands.Clear();
    CUpdateArchiveCommand c;
    c.ActionSet = NUpdateArchive::kAddActionSet;
    Commands.Add(c);
  }

  CRecordVector<UInt64> VolumesSizes;
};

struct CErrorInfo
{
  DWORD SystemError;
  FString FileName;
  FString FileName2;
  UString Message;
  // UStringVector ErrorPaths;
  // CRecordVector<DWORD> ErrorCodes;
  CErrorInfo(): SystemError(0) {};
};

struct CUpdateErrorInfo: public CErrorInfo
{
};

#define INTERFACE_IUpdateCallbackUI2(x) \
  INTERFACE_IUpdateCallbackUI(x) \
  virtual HRESULT OpenResult(const wchar_t *name, HRESULT result) x; \
  virtual HRESULT StartScanning() x; \
  virtual HRESULT ScanProgress(UInt64 numFolders, UInt64 numFiles, const wchar_t *path) x; \
  virtual HRESULT CanNotFindError(const wchar_t *name, DWORD systemError) x; \
  virtual HRESULT FinishScanning() x; \
  virtual HRESULT StartArchive(const wchar_t *name, bool updating) x; \
  virtual HRESULT FinishArchive() x; \

struct IUpdateCallbackUI2: public IUpdateCallbackUI
{
  INTERFACE_IUpdateCallbackUI2(=0)
};

HRESULT UpdateArchive(
    CCodecs *codecs,
    const NWildcard::CCensor &censor,
    CUpdateOptions &options,
    CUpdateErrorInfo &errorInfo,
    IOpenCallbackUI *openCallback,
    IUpdateCallbackUI2 *callback);

#endif
