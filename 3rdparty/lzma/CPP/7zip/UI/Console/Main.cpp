// Main.cpp

#include "StdAfx.h"

#if defined( _WIN32) && defined( _7ZIP_LARGE_PAGES)
#include "../../../../C/Alloc.h"
#endif

#include "Common/MyInitGuid.h"

#include "Common/CommandLineParser.h"
#include "Common/IntToString.h"
#include "Common/MyException.h"
#include "Common/StdOutStream.h"
#include "Common/StringConvert.h"
#include "Common/StringToInt.h"

#include "Windows/Error.h"
#ifdef _WIN32
#include "Windows/MemoryLock.h"
#endif

#include "../Common/ArchiveCommandLine.h"
#include "../Common/ExitCode.h"
#include "../Common/Extract.h"
#ifdef EXTERNAL_CODECS
#include "../Common/LoadCodecs.h"
#endif

#include "BenchCon.h"
#include "ExtractCallbackConsole.h"
#include "List.h"
#include "OpenCallbackConsole.h"
#include "UpdateCallbackConsole.h"

#if !defined(EXTERNAL_CODECS) && defined(_NO_CRYPTO)
#define IT_IS_REDUCED_VERSION
#endif

#ifdef IT_IS_REDUCED_VERSION
#include "../../../../C/7zVersion.h"
#else
#include "../../MyVersion.h"
#endif

using namespace NWindows;
using namespace NFile;
using namespace NCommandLineParser;

#ifdef _WIN32
HINSTANCE g_hInstance = 0;
#endif
extern CStdOutStream *g_StdStream;

static const char *kCopyrightString = "\n7-Zip"
#ifndef EXTERNAL_CODECS
#ifdef IT_IS_REDUCED_VERSION
" (r)"
#else
" (a)"
#endif
#endif

#ifdef _WIN64
" [64]"
#endif

" " MY_VERSION_COPYRIGHT_DATE "\n";

static const char *kHelpString =
    "\nUsage: 7z"
#ifndef EXTERNAL_CODECS
#ifdef IT_IS_REDUCED_VERSION
    "r"
#else
    "a"
#endif
#endif
    " <command> [<switches>...] <archive_name> [<file_names>...]\n"
    "       [<@listfiles...>]\n"
    "\n"
    "<Commands>\n"
    "  a: Add files to archive\n"
    "  b: Benchmark\n"
    "  d: Delete files from archive\n"
    "  e: Extract files from archive (without using directory names)\n"
    "  l: List contents of archive\n"
//    "  l[a|t][f]: List contents of archive\n"
//    "    a - with Additional fields\n"
//    "    t - with all fields\n"
//    "    f - with Full pathnames\n"
    "  t: Test integrity of archive\n"
    "  u: Update files to archive\n"
    "  x: eXtract files with full paths\n"
    "<Switches>\n"
    "  -ai[r[-|0]]{@listfile|!wildcard}: Include archives\n"
    "  -ax[r[-|0]]{@listfile|!wildcard}: eXclude archives\n"
    "  -bd: Disable percentage indicator\n"
    "  -i[r[-|0]]{@listfile|!wildcard}: Include filenames\n"
    "  -m{Parameters}: set compression Method\n"
    "  -o{Directory}: set Output directory\n"
    #ifndef _NO_CRYPTO
    "  -p{Password}: set Password\n"
    #endif
    "  -r[-|0]: Recurse subdirectories\n"
    "  -scs{UTF-8 | WIN | DOS}: set charset for list files\n"
    "  -sfx[{name}]: Create SFX archive\n"
    "  -si[{name}]: read data from stdin\n"
    "  -slt: show technical information for l (List) command\n"
    "  -so: write data to stdout\n"
    "  -ssc[-]: set sensitive case mode\n"
    "  -ssw: compress shared files\n"
    "  -t{Type}: Set type of archive\n"
    "  -u[-][p#][q#][r#][x#][y#][z#][!newArchiveName]: Update options\n"
    "  -v{Size}[b|k|m|g]: Create volumes\n"
    "  -w[{path}]: assign Work directory. Empty path means a temporary directory\n"
    "  -x[r[-|0]]]{@listfile|!wildcard}: eXclude filenames\n"
    "  -y: assume Yes on all queries\n";

// ---------------------------
// exception messages

static const char *kEverythingIsOk = "Everything is Ok";
static const char *kUserErrorMessage = "Incorrect command line";
static const char *kNoFormats = "7-Zip cannot find the code that works with archives.";
static const char *kUnsupportedArcTypeMessage = "Unsupported archive type";

static CFSTR kDefaultSfxModule = FTEXT("7zCon.sfx");

static void ShowMessageAndThrowException(CStdOutStream &s, LPCSTR message, NExitCode::EEnum code)
{
  s << message << endl;
  throw code;
}

static void PrintHelpAndExit(CStdOutStream &s)
{
  s << kHelpString;
  ShowMessageAndThrowException(s, kUserErrorMessage, NExitCode::kUserError);
}

#ifndef _WIN32
static void GetArguments(int numArgs, const char *args[], UStringVector &parts)
{
  parts.Clear();
  for (int i = 0; i < numArgs; i++)
  {
    UString s = MultiByteToUnicodeString(args[i]);
    parts.Add(s);
  }
}
#endif

static void ShowCopyrightAndHelp(CStdOutStream &s, bool needHelp)
{
  s << kCopyrightString;
  // s << "# CPUs: " << (UInt64)NWindows::NSystem::GetNumberOfProcessors() << "\n";
  if (needHelp)
    s << kHelpString;
}

#ifdef EXTERNAL_CODECS
static void PrintString(CStdOutStream &stdStream, const AString &s, int size)
{
  int len = s.Length();
  stdStream << s;
  for (int i = len; i < size; i++)
    stdStream << ' ';
}
#endif

static void PrintString(CStdOutStream &stdStream, const UString &s, int size)
{
  int len = s.Length();
  stdStream << s;
  for (int i = len; i < size; i++)
    stdStream << ' ';
}

static inline char GetHex(Byte value)
{
  return (char)((value < 10) ? ('0' + value) : ('A' + (value - 10)));
}

int Main2(
  #ifndef _WIN32
  int numArgs, const char *args[]
  #endif
)
{
  #if defined(_WIN32) && !defined(UNDER_CE)
  SetFileApisToOEM();
  #endif
  
  UStringVector commandStrings;
  #ifdef _WIN32
  NCommandLineParser::SplitCommandLine(GetCommandLineW(), commandStrings);
  #else
  GetArguments(numArgs, args, commandStrings);
  #endif

  if (commandStrings.Size() == 1)
  {
    ShowCopyrightAndHelp(g_StdOut, true);
    return 0;
  }
  commandStrings.Delete(0);

  CArchiveCommandLineOptions options;

  CArchiveCommandLineParser parser;

  parser.Parse1(commandStrings, options);

  if (options.HelpMode)
  {
    ShowCopyrightAndHelp(g_StdOut, true);
    return 0;
  }

  #if defined(_WIN32) && defined(_7ZIP_LARGE_PAGES)
  if (options.LargePages)
  {
    SetLargePageSize();
    NSecurity::EnableLockMemoryPrivilege();
  }
  #endif

  CStdOutStream &stdStream = options.StdOutMode ? g_StdErr : g_StdOut;
  g_StdStream = &stdStream;

  if (options.EnableHeaders)
    ShowCopyrightAndHelp(stdStream, false);

  parser.Parse2(options);

  CCodecs *codecs = new CCodecs;
  CMyComPtr<
    #ifdef EXTERNAL_CODECS
    ICompressCodecsInfo
    #else
    IUnknown
    #endif
    > compressCodecsInfo = codecs;
  HRESULT result = codecs->Load();
  if (result != S_OK)
    throw CSystemException(result);

  bool isExtractGroupCommand = options.Command.IsFromExtractGroup();

  if (codecs->Formats.Size() == 0 &&
        (isExtractGroupCommand ||
        options.Command.CommandType == NCommandType::kList ||
        options.Command.IsFromUpdateGroup()))
    throw kNoFormats;

  CIntVector formatIndices;
  if (!codecs->FindFormatForArchiveType(options.ArcType, formatIndices))
    throw kUnsupportedArcTypeMessage;

  if (options.Command.CommandType == NCommandType::kInfo)
  {
    stdStream << endl << "Formats:" << endl;
    int i;
    for (i = 0; i < codecs->Formats.Size(); i++)
    {
      const CArcInfoEx &arc = codecs->Formats[i];
      #ifdef EXTERNAL_CODECS
      if (arc.LibIndex >= 0)
      {
        char s[16];
        ConvertUInt32ToString(arc.LibIndex, s);
        PrintString(stdStream, s, 2);
      }
      else
      #endif
        stdStream << "  ";
      stdStream << ' ';
      stdStream << (char)(arc.UpdateEnabled ? 'C' : ' ');
      stdStream << (char)(arc.KeepName ? 'K' : ' ');
      stdStream << "  ";
      PrintString(stdStream, arc.Name, 6);
      stdStream << "  ";
      UString s;
      for (int t = 0; t < arc.Exts.Size(); t++)
      {
        const CArcExtInfo &ext = arc.Exts[t];
        s += ext.Ext;
        if (!ext.AddExt.IsEmpty())
        {
          s += L" (";
          s += ext.AddExt;
          s += L')';
        }
        s += L' ';
      }
      PrintString(stdStream, s, 14);
      stdStream << "  ";
      const CByteBuffer &sig = arc.StartSignature;
      for (size_t j = 0; j < sig.GetCapacity(); j++)
      {
        Byte b = sig[j];
        if (b > 0x20 && b < 0x80)
        {
          stdStream << (char)b;
        }
        else
        {
          stdStream << GetHex((Byte)((b >> 4) & 0xF));
          stdStream << GetHex((Byte)(b & 0xF));
        }
        stdStream << ' ';
      }
      stdStream << endl;
    }
    stdStream << endl << "Codecs:" << endl;

    #ifdef EXTERNAL_CODECS
    UInt32 numMethods;
    if (codecs->GetNumberOfMethods(&numMethods) == S_OK)
    for (UInt32 j = 0; j < numMethods; j++)
    {
      int libIndex = codecs->GetCodecLibIndex(j);
      if (libIndex >= 0)
      {
        char s[16];
        ConvertUInt32ToString(libIndex, s);
        PrintString(stdStream, s, 2);
      }
      else
        stdStream << "  ";
      stdStream << ' ';
      stdStream << (char)(codecs->GetCodecEncoderIsAssigned(j) ? 'C' : ' ');
      UInt64 id;
      stdStream << "  ";
      HRESULT res = codecs->GetCodecId(j, id);
      if (res != S_OK)
        id = (UInt64)(Int64)-1;
      char s[32];
      ConvertUInt64ToString(id, s, 16);
      PrintString(stdStream, s, 8);
      stdStream << "  ";
      PrintString(stdStream, codecs->GetCodecName(j), 11);
      stdStream << endl;
      /*
      if (res != S_OK)
        throw "incorrect Codec ID";
      */
    }
    #endif
    return S_OK;
  }
  else if (options.Command.CommandType == NCommandType::kBenchmark)
  {
    {
      HRESULT res;
      #ifdef EXTERNAL_CODECS
      CObjectVector<CCodecInfoEx> externalCodecs;
      res = LoadExternalCodecs(compressCodecsInfo, externalCodecs);
      if (res != S_OK)
        throw CSystemException(res);
      #endif
      res = BenchCon(
          #ifdef EXTERNAL_CODECS
          compressCodecsInfo, &externalCodecs,
          #endif
          options.Properties, options.NumIterations, (FILE *)stdStream);
      if (res != S_OK)
      {
        if (res == S_FALSE)
        {
          stdStream << "\nDecoding Error\n";
          return NExitCode::kFatalError;
        }
        throw CSystemException(res);
      }
    }
  }
  else if (isExtractGroupCommand || options.Command.CommandType == NCommandType::kList)
  {
    if (isExtractGroupCommand)
    {
      CExtractCallbackConsole *ecs = new CExtractCallbackConsole;
      CMyComPtr<IFolderArchiveExtractCallback> extractCallback = ecs;

      ecs->OutStream = &stdStream;

      #ifndef _NO_CRYPTO
      ecs->PasswordIsDefined = options.PasswordEnabled;
      ecs->Password = options.Password;
      #endif

      ecs->Init();

      COpenCallbackConsole openCallback;
      openCallback.OutStream = &stdStream;

      #ifndef _NO_CRYPTO
      openCallback.PasswordIsDefined = options.PasswordEnabled;
      openCallback.Password = options.Password;
      #endif

      CExtractOptions eo;
      eo.StdInMode = options.StdInMode;
      eo.StdOutMode = options.StdOutMode;
      eo.PathMode = options.Command.GetPathMode();
      eo.TestMode = options.Command.IsTestMode();
      eo.OverwriteMode = options.OverwriteMode;
      eo.OutputDir = options.OutputDir;
      eo.YesToAll = options.YesToAll;
      eo.CalcCrc = options.CalcCrc;
      #if !defined(_7ZIP_ST) && !defined(_SFX)
      eo.Properties = options.Properties;
      #endif
      UString errorMessage;
      CDecompressStat stat;
      HRESULT result = DecompressArchives(
          codecs,
          formatIndices,
          options.ArchivePathsSorted,
          options.ArchivePathsFullSorted,
          options.WildcardCensor.Pairs.Front().Head,
          eo, &openCallback, ecs, errorMessage, stat);
      if (!errorMessage.IsEmpty())
      {
        stdStream << endl << "Error: " << errorMessage;
        if (result == S_OK)
          result = E_FAIL;
      }

      stdStream << endl;
      if (ecs->NumArchives > 1)
        stdStream << "Archives: " << ecs->NumArchives << endl;
      if (ecs->NumArchiveErrors != 0 || ecs->NumFileErrors != 0)
      {
        if (ecs->NumArchives > 1)
        {
          stdStream << endl;
          if (ecs->NumArchiveErrors != 0)
            stdStream << "Archive Errors: " << ecs->NumArchiveErrors << endl;
          if (ecs->NumFileErrors != 0)
            stdStream << "Sub items Errors: " << ecs->NumFileErrors << endl;
        }
        if (result != S_OK)
          throw CSystemException(result);
        return NExitCode::kFatalError;
      }
      if (result != S_OK)
        throw CSystemException(result);
      if (stat.NumFolders != 0)
        stdStream << "Folders: " << stat.NumFolders << endl;
      if (stat.NumFiles != 1 || stat.NumFolders != 0)
          stdStream << "Files: " << stat.NumFiles << endl;
      stdStream
           << "Size:       " << stat.UnpackSize << endl
           << "Compressed: " << stat.PackSize << endl;
      if (options.CalcCrc)
      {
        char s[16];
        ConvertUInt32ToHexWithZeros(stat.CrcSum, s);
        stdStream << "CRC: " << s << endl;
      }
    }
    else
    {
      UInt64 numErrors = 0;
      HRESULT result = ListArchives(
          codecs,
          formatIndices,
          options.StdInMode,
          options.ArchivePathsSorted,
          options.ArchivePathsFullSorted,
          options.WildcardCensor.Pairs.Front().Head,
          options.EnableHeaders,
          options.TechMode,
          #ifndef _NO_CRYPTO
          options.PasswordEnabled,
          options.Password,
          #endif
          numErrors);
      if (numErrors > 0)
      {
        g_StdOut << endl << "Errors: " << numErrors << endl;
        return NExitCode::kFatalError;
      }
      if (result != S_OK)
        throw CSystemException(result);
    }
  }
  else if (options.Command.IsFromUpdateGroup())
  {
    CUpdateOptions &uo = options.UpdateOptions;
    if (uo.SfxMode && uo.SfxModule.IsEmpty())
      uo.SfxModule = kDefaultSfxModule;

    COpenCallbackConsole openCallback;
    openCallback.OutStream = &stdStream;

    #ifndef _NO_CRYPTO
    bool passwordIsDefined =
        options.PasswordEnabled && !options.Password.IsEmpty();
    openCallback.PasswordIsDefined = passwordIsDefined;
    openCallback.Password = options.Password;
    #endif

    CUpdateCallbackConsole callback;
    callback.EnablePercents = options.EnablePercents;

    #ifndef _NO_CRYPTO
    callback.PasswordIsDefined = passwordIsDefined;
    callback.AskPassword = options.PasswordEnabled && options.Password.IsEmpty();
    callback.Password = options.Password;
    #endif
    callback.StdOutMode = uo.StdOutMode;
    callback.Init(&stdStream);

    CUpdateErrorInfo errorInfo;

    if (!uo.Init(codecs, formatIndices, options.ArchiveName))
      throw kUnsupportedArcTypeMessage;
    HRESULT result = UpdateArchive(codecs,
        options.WildcardCensor, uo,
        errorInfo, &openCallback, &callback);

    int exitCode = NExitCode::kSuccess;
    if (callback.CantFindFiles.Size() > 0)
    {
      stdStream << endl;
      stdStream << "WARNINGS for files:" << endl << endl;
      int numErrors = callback.CantFindFiles.Size();
      for (int i = 0; i < numErrors; i++)
      {
        stdStream << callback.CantFindFiles[i] << " : ";
        stdStream << NError::MyFormatMessageW(callback.CantFindCodes[i]) << endl;
      }
      stdStream << "----------------" << endl;
      stdStream << "WARNING: Cannot find " << numErrors << " file";
      if (numErrors > 1)
        stdStream << "s";
      stdStream << endl;
      exitCode = NExitCode::kWarning;
    }

    if (result != S_OK)
    {
      UString message;
      if (!errorInfo.Message.IsEmpty())
      {
        message += errorInfo.Message;
        message += L"\n";
      }
      if (!errorInfo.FileName.IsEmpty())
      {
        message += fs2us(errorInfo.FileName);
        message += L"\n";
      }
      if (!errorInfo.FileName2.IsEmpty())
      {
        message += fs2us(errorInfo.FileName2);
        message += L"\n";
      }
      if (errorInfo.SystemError != 0)
      {
        message += NError::MyFormatMessageW(errorInfo.SystemError);
        message += L"\n";
      }
      if (!message.IsEmpty())
        stdStream << L"\nError:\n" << message;
      throw CSystemException(result);
    }
    int numErrors = callback.FailedFiles.Size();
    if (numErrors == 0)
    {
      if (callback.CantFindFiles.Size() == 0)
        stdStream << kEverythingIsOk << endl;
    }
    else
    {
      stdStream << endl;
      stdStream << "WARNINGS for files:" << endl << endl;
      for (int i = 0; i < numErrors; i++)
      {
        stdStream << callback.FailedFiles[i] << " : ";
        stdStream << NError::MyFormatMessageW(callback.FailedCodes[i]) << endl;
      }
      stdStream << "----------------" << endl;
      stdStream << "WARNING: Cannot open " << numErrors << " file";
      if (numErrors > 1)
        stdStream << "s";
      stdStream << endl;
      exitCode = NExitCode::kWarning;
    }
    return exitCode;
  }
  else
    PrintHelpAndExit(stdStream);
  return 0;
}
