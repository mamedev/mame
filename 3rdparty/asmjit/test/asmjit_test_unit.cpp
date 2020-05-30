// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <asmjit/asmjit.h>
#include "./broken.h"

using namespace asmjit;

// ============================================================================
// [DumpCpu]
// ============================================================================

struct DumpCpuFeature {
  uint32_t feature;
  const char* name;
};

static const char* hostArch() noexcept {
  switch (ArchInfo::kIdHost) {
    case ArchInfo::kIdX86: return "X86";
    case ArchInfo::kIdX64: return "X64";
    case ArchInfo::kIdA32: return "ARM32";
    case ArchInfo::kIdA64: return "ARM64";
    default: return "Unknown";
  }
}

static void dumpFeatures(const CpuInfo& cpu, const DumpCpuFeature* data, size_t count) noexcept {
  for (size_t i = 0; i < count; i++)
    if (cpu.hasFeature(data[i].feature))
      INFO("  %s", data[i].name);
}

static void dumpCpu(void) noexcept {
  const CpuInfo& cpu = CpuInfo::host();

  INFO("Host CPU:");
  INFO("  Vendor                  : %s", cpu.vendor());
  INFO("  Brand                   : %s", cpu.brand());
  INFO("  Model ID                : %u", cpu.modelId());
  INFO("  Brand ID                : %u", cpu.brandId());
  INFO("  Family ID               : %u", cpu.familyId());
  INFO("  Stepping                : %u", cpu.stepping());
  INFO("  Processor Type          : %u", cpu.processorType());
  INFO("  Max logical Processors  : %u", cpu.maxLogicalProcessors());
  INFO("  Cache-Line Size         : %u", cpu.cacheLineSize());
  INFO("  HW-Thread Count         : %u", cpu.hwThreadCount());
  INFO("");

  // --------------------------------------------------------------------------
  // [X86]
  // --------------------------------------------------------------------------

#if ASMJIT_ARCH_X86
  static const DumpCpuFeature x86FeaturesList[] = {
    { x86::Features::kNX              , "NX"               },
    { x86::Features::kMT              , "MT"               },
    { x86::Features::k3DNOW           , "3DNOW"            },
    { x86::Features::k3DNOW2          , "3DNOW2"           },
    { x86::Features::kADX             , "ADX"              },
    { x86::Features::kAESNI           , "AESNI"            },
    { x86::Features::kALTMOVCR8       , "ALTMOVCR8"        },
    { x86::Features::kAVX             , "AVX"              },
    { x86::Features::kAVX2            , "AVX2"             },
    { x86::Features::kAVX512_4FMAPS   , "AVX512_4FMAPS"    },
    { x86::Features::kAVX512_4VNNIW   , "AVX512_4VNNIW"    },
    { x86::Features::kAVX512_BITALG   , "AVX512_BITALG"    },
    { x86::Features::kAVX512_BW       , "AVX512_BW"        },
    { x86::Features::kAVX512_CDI      , "AVX512_CDI"       },
    { x86::Features::kAVX512_DQ       , "AVX512_DQ"        },
    { x86::Features::kAVX512_ERI      , "AVX512_ERI"       },
    { x86::Features::kAVX512_F        , "AVX512_F"         },
    { x86::Features::kAVX512_IFMA     , "AVX512_IFMA"      },
    { x86::Features::kAVX512_PFI      , "AVX512_PFI"       },
    { x86::Features::kAVX512_VBMI     , "AVX512_VBMI"      },
    { x86::Features::kAVX512_VBMI2    , "AVX512_VBMI2"     },
    { x86::Features::kAVX512_VL       , "AVX512_VL"        },
    { x86::Features::kAVX512_VNNI     , "AVX512_VNNI"      },
    { x86::Features::kAVX512_VPOPCNTDQ, "AVX512_VPOPCNTDQ" },
    { x86::Features::kBMI             , "BMI"              },
    { x86::Features::kBMI2            , "BMI2"             },
    { x86::Features::kCLFLUSH         , "CLFLUSH"          },
    { x86::Features::kCLFLUSHOPT      , "CLFLUSHOPT"       },
    { x86::Features::kCLWB            , "CLWB"             },
    { x86::Features::kCLZERO          , "CLZERO"           },
    { x86::Features::kCMOV            , "CMOV"             },
    { x86::Features::kCMPXCHG16B      , "CMPXCHG16B"       },
    { x86::Features::kCMPXCHG8B       , "CMPXCHG8B"        },
    { x86::Features::kERMS            , "ERMS"             },
    { x86::Features::kF16C            , "F16C"             },
    { x86::Features::kFMA             , "FMA"              },
    { x86::Features::kFMA4            , "FMA4"             },
    { x86::Features::kFPU             , "FPU"              },
    { x86::Features::kFSGSBASE        , "FSGSBASE"         },
    { x86::Features::kFXSR            , "FXSR"             },
    { x86::Features::kFXSROPT         , "FXSROPT"          },
    { x86::Features::kGEODE           , "GEODE"            },
    { x86::Features::kGFNI            , "GFNI"             },
    { x86::Features::kHLE             , "HLE"              },
    { x86::Features::kI486            , "I486"             },
    { x86::Features::kLAHFSAHF        , "LAHFSAHF"         },
    { x86::Features::kLWP             , "LWP"              },
    { x86::Features::kLZCNT           , "LZCNT"            },
    { x86::Features::kMMX             , "MMX"              },
    { x86::Features::kMMX2            , "MMX2"             },
    { x86::Features::kMONITOR         , "MONITOR"          },
    { x86::Features::kMONITORX        , "MONITORX"         },
    { x86::Features::kMOVBE           , "MOVBE"            },
    { x86::Features::kMPX             , "MPX"              },
    { x86::Features::kMSR             , "MSR"              },
    { x86::Features::kMSSE            , "MSSE"             },
    { x86::Features::kOSXSAVE         , "OSXSAVE"          },
    { x86::Features::kPCLMULQDQ       , "PCLMULQDQ"        },
    { x86::Features::kPCOMMIT         , "PCOMMIT"          },
    { x86::Features::kPOPCNT          , "POPCNT"           },
    { x86::Features::kPREFETCHW       , "PREFETCHW"        },
    { x86::Features::kPREFETCHWT1     , "PREFETCHWT1"      },
    { x86::Features::kRDRAND          , "RDRAND"           },
    { x86::Features::kRDSEED          , "RDSEED"           },
    { x86::Features::kRDTSC           , "RDTSC"            },
    { x86::Features::kRDTSCP          , "RDTSCP"           },
    { x86::Features::kRTM             , "RTM"              },
    { x86::Features::kSHA             , "SHA"              },
    { x86::Features::kSKINIT          , "SKINIT"           },
    { x86::Features::kSMAP            , "SMAP"             },
    { x86::Features::kSMEP            , "SMEP"             },
    { x86::Features::kSMX             , "SMX"              },
    { x86::Features::kSSE             , "SSE"              },
    { x86::Features::kSSE2            , "SSE2"             },
    { x86::Features::kSSE3            , "SSE3"             },
    { x86::Features::kSSE4_1          , "SSE4.1"           },
    { x86::Features::kSSE4_2          , "SSE4.2"           },
    { x86::Features::kSSE4A           , "SSE4A"            },
    { x86::Features::kSSSE3           , "SSSE3"            },
    { x86::Features::kSVM             , "SVM"              },
    { x86::Features::kTBM             , "TBM"              },
    { x86::Features::kTSX             , "TSX"              },
    { x86::Features::kVAES            , "VAES"             },
    { x86::Features::kVMX             , "VMX"              },
    { x86::Features::kVPCLMULQDQ      , "VPCLMULQDQ"       },
    { x86::Features::kXOP             , "XOP"              },
    { x86::Features::kXSAVE           , "XSAVE"            },
    { x86::Features::kXSAVEC          , "XSAVEC"           },
    { x86::Features::kXSAVEOPT        , "XSAVEOPT"         },
    { x86::Features::kXSAVES          , "XSAVES"           }
  };

  INFO("X86 Features:");
  dumpFeatures(cpu, x86FeaturesList, ASMJIT_ARRAY_SIZE(x86FeaturesList));
  INFO("");
#endif

  // --------------------------------------------------------------------------
  // [ARM]
  // --------------------------------------------------------------------------

#if ASMJIT_ARCH_ARM
  static const DumpCpuFeature armFeaturesList[] = {
    { arm::Features::kARMv6           , "ARMv6"            },
    { arm::Features::kARMv7           , "ARMv7"            },
    { arm::Features::kARMv8           , "ARMv8"            },
    { arm::Features::kTHUMB           , "THUMB"            },
    { arm::Features::kTHUMBv2         , "THUMBv2"          },
    { arm::Features::kVFP2            , "VFPv2"            },
    { arm::Features::kVFP3            , "VFPv3"            },
    { arm::Features::kVFP4            , "VFPv4"            },
    { arm::Features::kVFP_D32         , "VFP D32"          },
    { arm::Features::kNEON            , "NEON"             },
    { arm::Features::kDSP             , "DSP"              },
    { arm::Features::kIDIV            , "IDIV"             },
    { arm::Features::kAES             , "AES"              },
    { arm::Features::kCRC32           , "CRC32"            },
    { arm::Features::kSHA1            , "SHA1"             },
    { arm::Features::kSHA256          , "SHA256"           },
    { arm::Features::kATOMIC64        , "ATOMIC64"         }
  };

  INFO("ARM Features:");
  dumpFeatures(cpu, armFeaturesList, ASMJIT_ARRAY_SIZE(armFeaturesList));
  INFO("");
#endif
}

// ============================================================================
// [DumpSizeOf]
// ============================================================================

#define DUMP_TYPE(...) \
  INFO("  %-26s: %u", #__VA_ARGS__, uint32_t(sizeof(__VA_ARGS__)))

static void dumpSizeOf(void) noexcept {
  INFO("Size of C++ types:");
    DUMP_TYPE(int8_t);
    DUMP_TYPE(int16_t);
    DUMP_TYPE(int32_t);
    DUMP_TYPE(int64_t);
    DUMP_TYPE(int);
    DUMP_TYPE(long);
    DUMP_TYPE(size_t);
    DUMP_TYPE(intptr_t);
    DUMP_TYPE(float);
    DUMP_TYPE(double);
    DUMP_TYPE(void*);
  INFO("");

  INFO("Size of base classes:");
    DUMP_TYPE(BaseAssembler);
    DUMP_TYPE(BaseEmitter);
    DUMP_TYPE(CodeBuffer);
    DUMP_TYPE(CodeHolder);
    DUMP_TYPE(ConstPool);
    DUMP_TYPE(LabelEntry);
    DUMP_TYPE(RelocEntry);
    DUMP_TYPE(Section);
    DUMP_TYPE(String);
    DUMP_TYPE(Target);
    DUMP_TYPE(Zone);
    DUMP_TYPE(ZoneAllocator);
    DUMP_TYPE(ZoneBitVector);
    DUMP_TYPE(ZoneHashNode);
    DUMP_TYPE(ZoneHash<ZoneHashNode>);
    DUMP_TYPE(ZoneList<int>);
    DUMP_TYPE(ZoneVector<int>);
  INFO("");

  INFO("Size of operand classes:");
    DUMP_TYPE(Operand);
    DUMP_TYPE(BaseReg);
    DUMP_TYPE(BaseMem);
    DUMP_TYPE(Imm);
    DUMP_TYPE(Label);
  INFO("");

  INFO("Size of function classes:");
    DUMP_TYPE(CallConv);
    DUMP_TYPE(FuncFrame);
    DUMP_TYPE(FuncValue);
    DUMP_TYPE(FuncDetail);
    DUMP_TYPE(FuncSignature);
    DUMP_TYPE(FuncArgsAssignment);
  INFO("");

#ifndef ASMJIT_NO_BUILDER
  INFO("Size of builder classes:");
    DUMP_TYPE(BaseBuilder);
    DUMP_TYPE(BaseNode);
    DUMP_TYPE(InstNode);
    DUMP_TYPE(InstExNode);
    DUMP_TYPE(AlignNode);
    DUMP_TYPE(LabelNode);
    DUMP_TYPE(EmbedDataNode);
    DUMP_TYPE(EmbedLabelNode);
    DUMP_TYPE(ConstPoolNode);
    DUMP_TYPE(CommentNode);
    DUMP_TYPE(SentinelNode);
  INFO("");
#endif

#ifndef ASMJIT_NO_COMPILER
  INFO("Size of compiler classes:");
    DUMP_TYPE(BaseCompiler);
    DUMP_TYPE(FuncNode);
    DUMP_TYPE(FuncRetNode);
    DUMP_TYPE(FuncCallNode);
  INFO("");
#endif

#ifdef ASMJIT_BUILD_X86
  INFO("Size of x86-specific classes:");
    DUMP_TYPE(x86::Assembler);
    #ifndef ASMJIT_NO_BUILDER
    DUMP_TYPE(x86::Builder);
    #endif
    #ifndef ASMJIT_NO_COMPILER
    DUMP_TYPE(x86::Compiler);
    #endif
    DUMP_TYPE(x86::InstDB::InstInfo);
    DUMP_TYPE(x86::InstDB::CommonInfo);
    DUMP_TYPE(x86::InstDB::OpSignature);
    DUMP_TYPE(x86::InstDB::InstSignature);
  INFO("");
#endif
}

#undef DUMP_TYPE

// ============================================================================
// [Main]
// ============================================================================

static void onBeforeRun(void) noexcept {
  dumpCpu();
  dumpSizeOf();
}

int main(int argc, const char* argv[]) {
#if defined(ASMJIT_BUILD_DEBUG)
  const char buildType[] = "Debug";
#else
  const char buildType[] = "Release";
#endif

  INFO("AsmJit Unit-Test v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    hostArch(),
    buildType
  );

  return BrokenAPI::run(argc, argv, onBeforeRun);
}
