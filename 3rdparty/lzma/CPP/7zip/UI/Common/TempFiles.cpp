// TempFiles.cpp

#include "StdAfx.h"

#include "TempFiles.h"

#include "Windows/FileDir.h"

using namespace NWindows;
using namespace NFile;

void CTempFiles::Clear()
{
  while (!Paths.IsEmpty())
  {
    NDirectory::DeleteFileAlways(Paths.Back());
    Paths.DeleteBack();
  }
}


