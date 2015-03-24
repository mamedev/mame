// ExtractMode.h

#ifndef __EXTRACT_MODE_H
#define __EXTRACT_MODE_H

namespace NExtract {
  
  namespace NPathMode
  {
    enum EEnum
    {
      kFullPathnames,
      kCurrentPathnames,
      kNoPathnames
    };
  }
  
  namespace NOverwriteMode
  {
    enum EEnum
    {
      kAskBefore,
      kWithoutPrompt,
      kSkipExisting,
      kAutoRename,
      kAutoRenameExisting
    };
  }
}

#endif
