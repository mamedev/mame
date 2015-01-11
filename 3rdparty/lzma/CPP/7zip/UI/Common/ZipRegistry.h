// ZipRegistry.h

#ifndef __ZIP_REGISTRY_H
#define __ZIP_REGISTRY_H

#include "Common/MyString.h"
#include "Common/Types.h"

#include "ExtractMode.h"

namespace NExtract
{
  struct CInfo
  {
    NPathMode::EEnum PathMode;
    NOverwriteMode::EEnum OverwriteMode;
    bool ShowPassword;
    UStringVector Paths;

    void Save() const;
    void Load();
  };
  void Save_ShowPassword(bool showPassword);
  bool Read_ShowPassword();
}

namespace NCompression
{
  struct CFormatOptions
  {
    UInt32 Level;
    UInt32 Dictionary;
    UInt32 Order;
    UInt32 BlockLogSize;
    UInt32 NumThreads;
    
    CSysString FormatID;
    UString Method;
    UString Options;
    UString EncryptionMethod;

    void ResetForLevelChange()
    {
      BlockLogSize = NumThreads = Level = Dictionary = Order = UInt32(-1);
      Method.Empty();
      // Options.Empty();
      // EncryptionMethod.Empty();
    }
    CFormatOptions() { ResetForLevelChange(); }
  };

  struct CInfo
  {
    UInt32 Level;
    bool ShowPassword;
    bool EncryptHeaders;
    UString ArcType;
    UStringVector ArcPaths;

    CObjectVector<CFormatOptions> Formats;

    void Save() const;
    void Load();
  };
}

namespace NWorkDir
{
  namespace NMode
  {
    enum EEnum
    {
      kSystem,
      kCurrent,
      kSpecified
    };
  }
  struct CInfo
  {
    NMode::EEnum Mode;
    FString Path;
    bool ForRemovableOnly;

    void SetForRemovableOnlyDefault() { ForRemovableOnly = true; }
    void SetDefault()
    {
      Mode = NMode::kSystem;
      Path.Empty();
      SetForRemovableOnlyDefault();
    }

    void Save() const;
    void Load();
  };
}


struct CContextMenuInfo
{
  bool Cascaded;
  UInt32 Flags;

  void Save() const;
  void Load();
};

#endif
