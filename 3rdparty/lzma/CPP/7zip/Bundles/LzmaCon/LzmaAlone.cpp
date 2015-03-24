// LzmaAlone.cpp

#include "StdAfx.h"

#include <stdio.h>

#if (defined(_WIN32) || defined(OS2) || defined(MSDOS)) && !defined(UNDER_CE)
#include <fcntl.h>
#include <io.h>
#define MY_SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define MY_SET_BINARY_MODE(file)
#endif

// #include "../../../Common/MyWindows.h"
#include "../../../Common/MyInitGuid.h"

#include "../../../../C/7zVersion.h"
#include "../../../../C/Alloc.h"
#include "../../../../C/Lzma86.h"

#include "../../../Windows/NtCheck.h"

#ifndef _7ZIP_ST
#include "../../../Windows/System.h"
#endif

#include "../../../Common/CommandLineParser.h"
#include "../../../Common/StringConvert.h"
#include "../../../Common/StringToInt.h"

#include "../../Common/FileStreams.h"
#include "../../Common/StreamUtils.h"

#include "../../Compress/LzmaDecoder.h"
#include "../../Compress/LzmaEncoder.h"

#include "../../UI/Console/BenchCon.h"


using namespace NCommandLineParser;

static const char *kCantAllocate = "Can not allocate memory";
static const char *kReadError = "Read error";
static const char *kWriteError = "Write error";

namespace NKey {
enum Enum
{
  kHelp1 = 0,
  kHelp2,
  kMethod,
  kLevel,
  kAlgo,
  kDict,
  kFb,
  kMc,
  kLc,
  kLp,
  kPb,
  kMatchFinder,
  kMultiThread,
  kEOS,
  kStdIn,
  kStdOut,
  kFilter86
};
}

static const CSwitchForm kSwitchForms[] =
{
  { L"?",  NSwitchType::kSimple, false },
  { L"H",  NSwitchType::kSimple, false },
  { L"MM", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"X", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"A", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"D", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"FB", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"MC", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"LC", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"LP", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"PB", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"MF", NSwitchType::kUnLimitedPostString, false, 1 },
  { L"MT", NSwitchType::kUnLimitedPostString, false, 0 },
  { L"EOS", NSwitchType::kSimple, false },
  { L"SI",  NSwitchType::kSimple, false },
  { L"SO",  NSwitchType::kSimple, false },
  { L"F86",  NSwitchType::kPostChar, false, 0, 0, L"+" }
};

static const int kNumSwitches = sizeof(kSwitchForms) / sizeof(kSwitchForms[0]);

static void PrintMessage(const char *s)
{
  fputs(s, stderr);
}

static void PrintHelp()
{
  PrintMessage("\nUsage:  LZMA <e|d> inputFile outputFile [<switches>...]\n"
             "  e: encode file\n"
             "  d: decode file\n"
             "  b: Benchmark\n"
    "<Switches>\n"
    "  -a{N}:  set compression mode - [0, 1], default: 1 (max)\n"
    "  -d{N}:  set dictionary size - [12, 30], default: 23 (8MB)\n"
    "  -fb{N}: set number of fast bytes - [5, 273], default: 128\n"
    "  -mc{N}: set number of cycles for match finder\n"
    "  -lc{N}: set number of literal context bits - [0, 8], default: 3\n"
    "  -lp{N}: set number of literal pos bits - [0, 4], default: 0\n"
    "  -pb{N}: set number of pos bits - [0, 4], default: 2\n"
    "  -mf{MF_ID}: set Match Finder: [bt2, bt3, bt4, hc4], default: bt4\n"
    "  -mt{N}: set number of CPU threads\n"
    "  -eos:   write End Of Stream marker\n"
    "  -si:    read data from stdin\n"
    "  -so:    write data to stdout\n"
    );
}

static void PrintHelpAndExit(const char *s)
{
  fprintf(stderr, "\nError: %s\n\n", s);
  PrintHelp();
  throw -1;
}

static void IncorrectCommand()
{
  PrintHelpAndExit("Incorrect command");
}

static void WriteArgumentsToStringList(int numArgs, const char *args[], UStringVector &strings)
{
  for (int i = 1; i < numArgs; i++)
    strings.Add(MultiByteToUnicodeString(args[i]));
}

static bool GetNumber(const wchar_t *s, UInt32 &value)
{
  value = 0;
  if (MyStringLen(s) == 0)
    return false;
  const wchar_t *end;
  UInt64 res = ConvertStringToUInt64(s, &end);
  if (*end != L'\0')
    return false;
  if (res > 0xFFFFFFFF)
    return false;
  value = UInt32(res);
  return true;
}

static void ParseUInt32(const CParser &parser, int index, UInt32 &res)
{
  if (parser[index].ThereIs)
    if (!GetNumber(parser[index].PostStrings[0], res))
      IncorrectCommand();
}

#define NT_CHECK_FAIL_ACTION PrintMessage("Unsupported Windows version"); return 1;

int main2(int numArgs, const char *args[])
{
  NT_CHECK

  PrintMessage("\nLZMA " MY_VERSION_COPYRIGHT_DATE "\n");

  if (numArgs == 1)
  {
    PrintHelp();
    return 0;
  }

  bool unsupportedTypes = (sizeof(Byte) != 1 || sizeof(UInt32) < 4 || sizeof(UInt64) < 4);
  if (unsupportedTypes)
  {
    PrintMessage("Unsupported base types. Edit Common/Types.h and recompile");
    return 1;
  }

  UStringVector commandStrings;
  WriteArgumentsToStringList(numArgs, args, commandStrings);
  CParser parser(kNumSwitches);
  try
  {
    parser.ParseStrings(kSwitchForms, commandStrings);
  }
  catch(...)
  {
    IncorrectCommand();
  }

  if (parser[NKey::kHelp1].ThereIs || parser[NKey::kHelp2].ThereIs)
  {
    PrintHelp();
    return 0;
  }
  const UStringVector &nonSwitchStrings = parser.NonSwitchStrings;

  int paramIndex = 0;
  if (paramIndex >= nonSwitchStrings.Size())
    IncorrectCommand();
  const UString &command = nonSwitchStrings[paramIndex++];

  CObjectVector<CProperty> props;
  bool dictDefined = false;
  UInt32 dict = (UInt32)(Int32)-1;
  if (parser[NKey::kDict].ThereIs)
  {
    UInt32 dicLog;
    const UString &s = parser[NKey::kDict].PostStrings[0];
    if (!GetNumber(s, dicLog))
      IncorrectCommand();
    dict = 1 << dicLog;
    dictDefined = true;
    CProperty prop;
    prop.Name = L"d";
    prop.Value = s;
    props.Add(prop);
  }
  if (parser[NKey::kLevel].ThereIs)
  {
    UInt32 level = 5;
    const UString &s = parser[NKey::kLevel].PostStrings[0];
    if (!GetNumber(s, level))
      IncorrectCommand();
    CProperty prop;
    prop.Name = L"x";
    prop.Value = s;
    props.Add(prop);
  }
  UString mf = L"BT4";
  if (parser[NKey::kMatchFinder].ThereIs)
    mf = parser[NKey::kMatchFinder].PostStrings[0];

  UInt32 numThreads = (UInt32)(Int32)-1;

  #ifndef _7ZIP_ST
  if (parser[NKey::kMultiThread].ThereIs)
  {
    UInt32 numCPUs = NWindows::NSystem::GetNumberOfProcessors();
    const UString &s = parser[NKey::kMultiThread].PostStrings[0];
    if (s.IsEmpty())
      numThreads = numCPUs;
    else
      if (!GetNumber(s, numThreads))
        IncorrectCommand();
    CProperty prop;
    prop.Name = L"mt";
    prop.Value = s;
    props.Add(prop);
  }
  #endif

  if (parser[NKey::kMethod].ThereIs)
  {
    UString s = parser[NKey::kMethod].PostStrings[0];
    if (s.IsEmpty() || s[0] != '=')
      IncorrectCommand();
    CProperty prop;
    prop.Name = L"m";
    prop.Value = s.Mid(1);
    props.Add(prop);
  }

  if (command.CompareNoCase(L"b") == 0)
  {
    const UInt32 kNumDefaultItereations = 1;
    UInt32 numIterations = kNumDefaultItereations;
    {
      if (paramIndex < nonSwitchStrings.Size())
        if (!GetNumber(nonSwitchStrings[paramIndex++], numIterations))
          numIterations = kNumDefaultItereations;
    }
    HRESULT res = BenchCon(props, numIterations, stderr);
    if (res != S_OK)
    {
      if (res != E_ABORT)
      {
        PrintMessage("Benchmark Error");
        return 1;
      }
    }
    return 0;
  }

  if (numThreads == (UInt32)(Int32)-1)
    numThreads = 1;

  bool encodeMode = false;
  if (command.CompareNoCase(L"e") == 0)
    encodeMode = true;
  else if (command.CompareNoCase(L"d") == 0)
    encodeMode = false;
  else
    IncorrectCommand();

  bool stdInMode = parser[NKey::kStdIn].ThereIs;
  bool stdOutMode = parser[NKey::kStdOut].ThereIs;

  CMyComPtr<ISequentialInStream> inStream;
  CInFileStream *inStreamSpec = 0;
  if (stdInMode)
  {
    inStream = new CStdInFileStream;
    MY_SET_BINARY_MODE(stdin);
  }
  else
  {
    if (paramIndex >= nonSwitchStrings.Size())
      IncorrectCommand();
    const UString &inputName = nonSwitchStrings[paramIndex++];
    inStreamSpec = new CInFileStream;
    inStream = inStreamSpec;
    if (!inStreamSpec->Open(us2fs(inputName)))
    {
      fprintf(stderr, "\nError: can not open input file %s\n",
          (const char *)GetOemString(inputName));
      return 1;
    }
  }

  CMyComPtr<ISequentialOutStream> outStream;
  COutFileStream *outStreamSpec = NULL;
  if (stdOutMode)
  {
    outStream = new CStdOutFileStream;
    MY_SET_BINARY_MODE(stdout);
  }
  else
  {
    if (paramIndex >= nonSwitchStrings.Size())
      IncorrectCommand();
    const UString &outputName = nonSwitchStrings[paramIndex++];
    outStreamSpec = new COutFileStream;
    outStream = outStreamSpec;
    if (!outStreamSpec->Create(us2fs(outputName), true))
    {
      fprintf(stderr, "\nError: can not open output file %s\n",
        (const char *)GetOemString(outputName));
      return 1;
    }
  }

  if (parser[NKey::kFilter86].ThereIs)
  {
    // -f86 switch is for x86 filtered mode: BCJ + LZMA.
    if (parser[NKey::kEOS].ThereIs || stdInMode)
      throw "Can not use stdin in this mode";
    UInt64 fileSize;
    inStreamSpec->File.GetLength(fileSize);
    if (fileSize > 0xF0000000)
      throw "File is too big";
    size_t inSize = (size_t)fileSize;
    Byte *inBuffer = 0;
    if (inSize != 0)
    {
      inBuffer = (Byte *)MyAlloc((size_t)inSize);
      if (inBuffer == 0)
        throw kCantAllocate;
    }
    
    if (ReadStream_FAIL(inStream, inBuffer, inSize) != S_OK)
      throw "Can not read";

    Byte *outBuffer = 0;
    size_t outSize;
    if (encodeMode)
    {
      // we allocate 105% of original size for output buffer
      outSize = (size_t)fileSize / 20 * 21 + (1 << 16);
      if (outSize != 0)
      {
        outBuffer = (Byte *)MyAlloc((size_t)outSize);
        if (outBuffer == 0)
          throw kCantAllocate;
      }
      if (!dictDefined)
        dict = 1 << 23;
      int res = Lzma86_Encode(outBuffer, &outSize, inBuffer, inSize,
          5, dict, parser[NKey::kFilter86].PostCharIndex == 0 ? SZ_FILTER_YES : SZ_FILTER_AUTO);
      if (res != 0)
      {
        fprintf(stderr, "\nEncoder error = %d\n", (int)res);
        return 1;
      }
    }
    else
    {
      UInt64 outSize64;
      if (Lzma86_GetUnpackSize(inBuffer, inSize, &outSize64) != 0)
        throw "data error";
      outSize = (size_t)outSize64;
      if (outSize != outSize64)
        throw "too big";
      if (outSize != 0)
      {
        outBuffer = (Byte *)MyAlloc(outSize);
        if (outBuffer == 0)
          throw kCantAllocate;
      }
      int res = Lzma86_Decode(outBuffer, &outSize, inBuffer, &inSize);
      if (inSize != (size_t)fileSize)
        throw "incorrect processed size";
      if (res != 0)
        throw "LzmaDecoder error";
    }
    if (WriteStream(outStream, outBuffer, outSize) != S_OK)
      throw kWriteError;
    MyFree(outBuffer);
    MyFree(inBuffer);
    return 0;
  }


  UInt64 fileSize;
  if (encodeMode)
  {
    NCompress::NLzma::CEncoder *encoderSpec = new NCompress::NLzma::CEncoder;
    CMyComPtr<ICompressCoder> encoder = encoderSpec;

    if (!dictDefined)
      dict = 1 << 23;

    UInt32 pb = 2;
    UInt32 lc = 3; // = 0; for 32-bit data
    UInt32 lp = 0; // = 2; for 32-bit data
    UInt32 algo = 1;
    UInt32 fb = 128;
    UInt32 mc = 16 + fb / 2;
    bool mcDefined = false;

    bool eos = parser[NKey::kEOS].ThereIs || stdInMode;
 
    ParseUInt32(parser, NKey::kAlgo, algo);
    ParseUInt32(parser, NKey::kFb, fb);
    ParseUInt32(parser, NKey::kLc, lc);
    ParseUInt32(parser, NKey::kLp, lp);
    ParseUInt32(parser, NKey::kPb, pb);

    mcDefined = parser[NKey::kMc].ThereIs;
    if (mcDefined)
      if (!GetNumber(parser[NKey::kMc].PostStrings[0], mc))
        IncorrectCommand();
    
    const PROPID propIDs[] =
    {
      NCoderPropID::kDictionarySize,
      NCoderPropID::kPosStateBits,
      NCoderPropID::kLitContextBits,
      NCoderPropID::kLitPosBits,
      NCoderPropID::kAlgorithm,
      NCoderPropID::kNumFastBytes,
      NCoderPropID::kMatchFinder,
      NCoderPropID::kEndMarker,
      NCoderPropID::kNumThreads,
      NCoderPropID::kMatchFinderCycles,
    };
    const int kNumPropsMax = sizeof(propIDs) / sizeof(propIDs[0]);

    PROPVARIANT props[kNumPropsMax];
    for (int p = 0; p < 6; p++)
      props[p].vt = VT_UI4;

    props[0].ulVal = (UInt32)dict;
    props[1].ulVal = (UInt32)pb;
    props[2].ulVal = (UInt32)lc;
    props[3].ulVal = (UInt32)lp;
    props[4].ulVal = (UInt32)algo;
    props[5].ulVal = (UInt32)fb;

    props[6].vt = VT_BSTR;
    props[6].bstrVal = const_cast<BSTR>((const wchar_t *)mf);

    props[7].vt = VT_BOOL;
    props[7].boolVal = eos ? VARIANT_TRUE : VARIANT_FALSE;

    props[8].vt = VT_UI4;
    props[8].ulVal = (UInt32)numThreads;

    // it must be last in property list
    props[9].vt = VT_UI4;
    props[9].ulVal = (UInt32)mc;

    int numProps = kNumPropsMax;
    if (!mcDefined)
      numProps--;

    if (encoderSpec->SetCoderProperties(propIDs, props, numProps) != S_OK)
      IncorrectCommand();
    encoderSpec->WriteCoderProperties(outStream);

    if (eos || stdInMode)
      fileSize = (UInt64)(Int64)-1;
    else
      inStreamSpec->File.GetLength(fileSize);

    for (int i = 0; i < 8; i++)
    {
      Byte b = Byte(fileSize >> (8 * i));
      if (outStream->Write(&b, 1, 0) != S_OK)
      {
        PrintMessage(kWriteError);
        return 1;
      }
    }
    HRESULT result = encoder->Code(inStream, outStream, 0, 0, 0);
    if (result == E_OUTOFMEMORY)
    {
      PrintMessage("\nError: Can not allocate memory\n");
      return 1;
    }
    else if (result != S_OK)
    {
      fprintf(stderr, "\nEncoder error = %X\n", (unsigned int)result);
      return 1;
    }
  }
  else
  {
    NCompress::NLzma::CDecoder *decoderSpec = new NCompress::NLzma::CDecoder;
    CMyComPtr<ICompressCoder> decoder = decoderSpec;
    decoderSpec->FinishStream = true;
    const UInt32 kPropertiesSize = 5;
    Byte header[kPropertiesSize + 8];
    if (ReadStream_FALSE(inStream, header, kPropertiesSize + 8) != S_OK)
    {
      PrintMessage(kReadError);
      return 1;
    }
    if (decoderSpec->SetDecoderProperties2(header, kPropertiesSize) != S_OK)
    {
      PrintMessage("SetDecoderProperties error");
      return 1;
    }
    fileSize = 0;
    for (int i = 0; i < 8; i++)
      fileSize |= ((UInt64)header[kPropertiesSize + i]) << (8 * i);

    if (decoder->Code(inStream, outStream, 0, (fileSize == (UInt64)(Int64)-1) ? 0 : &fileSize, 0) != S_OK)
    {
      PrintMessage("Decoder error");
      return 1;
    }
  }
  if (outStreamSpec != NULL)
  {
    if (outStreamSpec->Close() != S_OK)
    {
      PrintMessage("File closing error");
      return 1;
    }
  }
  return 0;
}

int MY_CDECL main(int numArgs, const char *args[])
{
  try { return main2(numArgs, args); }
  catch (const char *s)
  {
    fprintf(stderr, "\nError: %s\n", s);
    return 1;
  }
  catch(...)
  {
    PrintMessage("\nError\n");
    return 1;
  }
}
