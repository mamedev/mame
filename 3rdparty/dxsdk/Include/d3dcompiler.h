//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  File:       D3DCompiler.h
//  Content:    D3D Compilation Types and APIs
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __D3DCOMPILER_H__
#define __D3DCOMPILER_H__

// Current name of the DLL shipped in the same SDK as this header.


#define D3DCOMPILER_DLL_W L"d3dcompiler_43.dll"
#define D3DCOMPILER_DLL_A "d3dcompiler_43.dll"

#ifdef UNICODE
    #define D3DCOMPILER_DLL D3DCOMPILER_DLL_W 
#else
    #define D3DCOMPILER_DLL D3DCOMPILER_DLL_A
#endif

#include "d3d11shader.h"

//////////////////////////////////////////////////////////////////////////////
// APIs //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

//----------------------------------------------------------------------------
// D3DCOMPILE flags:
// -----------------
// D3DCOMPILE_DEBUG
//   Insert debug file/line/type/symbol information.
//
// D3DCOMPILE_SKIP_VALIDATION
//   Do not validate the generated code against known capabilities and
//   constraints.  This option is only recommended when compiling shaders
//   you KNOW will work.  (ie. have compiled before without this option.)
//   Shaders are always validated by D3D before they are set to the device.
//
// D3DCOMPILE_SKIP_OPTIMIZATION 
//   Instructs the compiler to skip optimization steps during code generation.
//   Unless you are trying to isolate a problem in your code using this option 
//   is not recommended.
//
// D3DCOMPILE_PACK_MATRIX_ROW_MAJOR
//   Unless explicitly specified, matrices will be packed in row-major order
//   on input and output from the shader.
//
// D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR
//   Unless explicitly specified, matrices will be packed in column-major 
//   order on input and output from the shader.  This is generally more 
//   efficient, since it allows vector-matrix multiplication to be performed
//   using a series of dot-products.
//
// D3DCOMPILE_PARTIAL_PRECISION
//   Force all computations in resulting shader to occur at partial precision.
//   This may result in faster evaluation of shaders on some hardware.
//
// D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT
//   Force compiler to compile against the next highest available software
//   target for vertex shaders.  This flag also turns optimizations off, 
//   and debugging on.  
//
// D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT
//   Force compiler to compile against the next highest available software
//   target for pixel shaders.  This flag also turns optimizations off, 
//   and debugging on.
//
// D3DCOMPILE_NO_PRESHADER
//   Disables Preshaders. Using this flag will cause the compiler to not 
//   pull out static expression for evaluation on the host cpu
//
// D3DCOMPILE_AVOID_FLOW_CONTROL
//   Hint compiler to avoid flow-control constructs where possible.
//
// D3DCOMPILE_PREFER_FLOW_CONTROL
//   Hint compiler to prefer flow-control constructs where possible.
//
// D3DCOMPILE_ENABLE_STRICTNESS
//   By default, the HLSL/Effect compilers are not strict on deprecated syntax.
//   Specifying this flag enables the strict mode. Deprecated syntax may be
//   removed in a future release, and enabling syntax is a good way to make
//   sure your shaders comply to the latest spec.
//
// D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY
//   This enables older shaders to compile to 4_0 targets.
//
//----------------------------------------------------------------------------

#define D3DCOMPILE_DEBUG                          (1 << 0)
#define D3DCOMPILE_SKIP_VALIDATION                (1 << 1)
#define D3DCOMPILE_SKIP_OPTIMIZATION              (1 << 2)
#define D3DCOMPILE_PACK_MATRIX_ROW_MAJOR          (1 << 3)
#define D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR       (1 << 4)
#define D3DCOMPILE_PARTIAL_PRECISION              (1 << 5)
#define D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT       (1 << 6)
#define D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT       (1 << 7)
#define D3DCOMPILE_NO_PRESHADER                   (1 << 8)
#define D3DCOMPILE_AVOID_FLOW_CONTROL             (1 << 9)
#define D3DCOMPILE_PREFER_FLOW_CONTROL            (1 << 10)
#define D3DCOMPILE_ENABLE_STRICTNESS              (1 << 11)
#define D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY (1 << 12)
#define D3DCOMPILE_IEEE_STRICTNESS                (1 << 13)
#define D3DCOMPILE_OPTIMIZATION_LEVEL0            (1 << 14)
#define D3DCOMPILE_OPTIMIZATION_LEVEL1            0
#define D3DCOMPILE_OPTIMIZATION_LEVEL2            ((1 << 14) | (1 << 15))
#define D3DCOMPILE_OPTIMIZATION_LEVEL3            (1 << 15)
#define D3DCOMPILE_RESERVED16                     (1 << 16)
#define D3DCOMPILE_RESERVED17                     (1 << 17)
#define D3DCOMPILE_WARNINGS_ARE_ERRORS            (1 << 18)

//----------------------------------------------------------------------------
// D3DCOMPILE_EFFECT flags:
// -------------------------------------
// These flags are passed in when creating an effect, and affect
// either compilation behavior or runtime effect behavior
//
// D3DCOMPILE_EFFECT_CHILD_EFFECT
//   Compile this .fx file to a child effect. Child effects have no
//   initializers for any shared values as these are initialied in the
//   master effect (pool).
//
// D3DCOMPILE_EFFECT_ALLOW_SLOW_OPS
//   By default, performance mode is enabled.  Performance mode
//   disallows mutable state objects by preventing non-literal
//   expressions from appearing in state object definitions.
//   Specifying this flag will disable the mode and allow for mutable
//   state objects.
//
//----------------------------------------------------------------------------

#define D3DCOMPILE_EFFECT_CHILD_EFFECT              (1 << 0)
#define D3DCOMPILE_EFFECT_ALLOW_SLOW_OPS            (1 << 1)

//----------------------------------------------------------------------------
// D3DCompile:
// ----------
// Compile source text into bytecode appropriate for the given target.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DCompile(__in_bcount(SrcDataSize) LPCVOID pSrcData,
           __in SIZE_T SrcDataSize,
           __in_opt LPCSTR pSourceName,
           __in_xcount_opt(pDefines->Name != NULL) CONST D3D_SHADER_MACRO* pDefines,
           __in_opt ID3DInclude* pInclude,
           __in LPCSTR pEntrypoint,
           __in LPCSTR pTarget,
           __in UINT Flags1,
           __in UINT Flags2,
           __out ID3DBlob** ppCode,
           __out_opt ID3DBlob** ppErrorMsgs);

typedef HRESULT (WINAPI *pD3DCompile)
    (LPCVOID                         pSrcData,
     SIZE_T                          SrcDataSize,
     LPCSTR                          pFileName,
     CONST D3D_SHADER_MACRO*         pDefines,
     ID3DInclude*                    pInclude,
     LPCSTR                          pEntrypoint,
     LPCSTR                          pTarget,
     UINT                            Flags1,
     UINT                            Flags2,
     ID3DBlob**                      ppCode,
     ID3DBlob**                      ppErrorMsgs);
     
//----------------------------------------------------------------------------
// D3DPreprocess:
// ----------
// Process source text with the compiler's preprocessor and return
// the resulting text.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DPreprocess(__in_bcount(SrcDataSize) LPCVOID pSrcData,
              __in SIZE_T SrcDataSize,
              __in_opt LPCSTR pSourceName,
              __in_opt CONST D3D_SHADER_MACRO* pDefines,
              __in_opt ID3DInclude* pInclude,
              __out ID3DBlob** ppCodeText,
              __out_opt ID3DBlob** ppErrorMsgs);

typedef HRESULT (WINAPI *pD3DPreprocess)
    (LPCVOID                      pSrcData,
     SIZE_T                       SrcDataSize,
     LPCSTR                       pFileName,
     CONST D3D_SHADER_MACRO*      pDefines,
     ID3DInclude*                 pInclude,
     ID3DBlob**                   ppCodeText,
     ID3DBlob**                   ppErrorMsgs);

//----------------------------------------------------------------------------
// D3DGetDebugInfo:
// -----------------------
// Gets shader debug info.  Debug info is generated by D3DCompile and is
// embedded in the body of the shader.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DGetDebugInfo(__in_bcount(SrcDataSize) LPCVOID pSrcData,
                __in SIZE_T SrcDataSize,
                __out ID3DBlob** ppDebugInfo);

//----------------------------------------------------------------------------
// D3DReflect:
// ----------
// Shader code contains metadata that can be inspected via the
// reflection APIs.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DReflect(__in_bcount(SrcDataSize) LPCVOID pSrcData,
           __in SIZE_T SrcDataSize,
	   __in REFIID pInterface,
           __out void** ppReflector);

//----------------------------------------------------------------------------
// D3DDisassemble:
// ----------------------
// Takes a binary shader and returns a buffer containing text assembly.
//----------------------------------------------------------------------------

#define D3D_DISASM_ENABLE_COLOR_CODE            0x00000001
#define D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS  0x00000002
#define D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING 0x00000004
#define D3D_DISASM_ENABLE_INSTRUCTION_CYCLE     0x00000008
#define D3D_DISASM_DISABLE_DEBUG_INFO           0x00000010

HRESULT WINAPI 
D3DDisassemble(__in_bcount(SrcDataSize) LPCVOID pSrcData,
               __in SIZE_T SrcDataSize,
               __in UINT Flags,
               __in_opt LPCSTR szComments,
               __out ID3DBlob** ppDisassembly);

typedef HRESULT (WINAPI *pD3DDisassemble)
    (__in_bcount(SrcDataSize) LPCVOID pSrcData,
     __in SIZE_T SrcDataSize,
     __in UINT Flags,
     __in_opt LPCSTR szComments,
     __out ID3DBlob** ppDisassembly);

//----------------------------------------------------------------------------
// D3DDisassemble10Effect:
// -----------------------
// Takes a D3D10 effect interface and returns a
// buffer containing text assembly.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DDisassemble10Effect(__in interface ID3D10Effect *pEffect, 
                       __in UINT Flags,
                       __out ID3DBlob** ppDisassembly);

//----------------------------------------------------------------------------
// D3DGetInputSignatureBlob:
// -----------------------
// Retrieve the input signature from a compilation result.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DGetInputSignatureBlob(__in_bcount(SrcDataSize) LPCVOID pSrcData,
                         __in SIZE_T SrcDataSize,
                         __out ID3DBlob** ppSignatureBlob);

//----------------------------------------------------------------------------
// D3DGetOutputSignatureBlob:
// -----------------------
// Retrieve the output signature from a compilation result.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DGetOutputSignatureBlob(__in_bcount(SrcDataSize) LPCVOID pSrcData,
                          __in SIZE_T SrcDataSize,
                          __out ID3DBlob** ppSignatureBlob);

//----------------------------------------------------------------------------
// D3DGetInputAndOutputSignatureBlob:
// -----------------------
// Retrieve the input and output signatures from a compilation result.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DGetInputAndOutputSignatureBlob(__in_bcount(SrcDataSize) LPCVOID pSrcData,
                                  __in SIZE_T SrcDataSize,
                                  __out ID3DBlob** ppSignatureBlob);

//----------------------------------------------------------------------------
// D3DStripShader:
// -----------------------
// Removes unwanted blobs from a compilation result
//----------------------------------------------------------------------------

typedef enum D3DCOMPILER_STRIP_FLAGS
{
    D3DCOMPILER_STRIP_REFLECTION_DATA = 1,
    D3DCOMPILER_STRIP_DEBUG_INFO      = 2,
    D3DCOMPILER_STRIP_TEST_BLOBS      = 4,
    D3DCOMPILER_STRIP_FORCE_DWORD     = 0x7fffffff,
} D3DCOMPILER_STRIP_FLAGS;

HRESULT WINAPI
D3DStripShader(__in_bcount(BytecodeLength) LPCVOID pShaderBytecode,
               __in SIZE_T BytecodeLength,
               __in UINT uStripFlags,
               __out ID3DBlob** ppStrippedBlob);

//----------------------------------------------------------------------------
// D3DGetBlobPart:
// -----------------------
// Extracts information from a compilation result.
//----------------------------------------------------------------------------

typedef enum D3D_BLOB_PART
{
    D3D_BLOB_INPUT_SIGNATURE_BLOB,
    D3D_BLOB_OUTPUT_SIGNATURE_BLOB,
    D3D_BLOB_INPUT_AND_OUTPUT_SIGNATURE_BLOB,
    D3D_BLOB_PATCH_CONSTANT_SIGNATURE_BLOB,
    D3D_BLOB_ALL_SIGNATURE_BLOB,
    D3D_BLOB_DEBUG_INFO,
    D3D_BLOB_LEGACY_SHADER,
    D3D_BLOB_XNA_PREPASS_SHADER,
    D3D_BLOB_XNA_SHADER,

    // Test parts are only produced by special compiler versions and so
    // are usually not present in shaders.
    D3D_BLOB_TEST_ALTERNATE_SHADER = 0x8000,
    D3D_BLOB_TEST_COMPILE_DETAILS,
    D3D_BLOB_TEST_COMPILE_PERF,
} D3D_BLOB_PART;

HRESULT WINAPI 
D3DGetBlobPart(__in_bcount(SrcDataSize) LPCVOID pSrcData,
               __in SIZE_T SrcDataSize,
               __in D3D_BLOB_PART Part,
               __in UINT Flags,
               __out ID3DBlob** ppPart);

//----------------------------------------------------------------------------
// D3DCompressShaders:
// -----------------------
// Compresses a set of shaders into a more compact form.
//----------------------------------------------------------------------------

typedef struct _D3D_SHADER_DATA
{
    LPCVOID pBytecode;
    SIZE_T BytecodeLength;
} D3D_SHADER_DATA;

#define D3D_COMPRESS_SHADER_KEEP_ALL_PARTS 0x00000001

HRESULT WINAPI
D3DCompressShaders(__in UINT uNumShaders,
                   __in_ecount(uNumShaders) D3D_SHADER_DATA* pShaderData,
                   __in UINT uFlags,
                   __out ID3DBlob** ppCompressedData);

//----------------------------------------------------------------------------
// D3DDecompressShaders:
// -----------------------
// Decompresses one or more shaders from a compressed set.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DDecompressShaders(__in_bcount(SrcDataSize) LPCVOID pSrcData,
                     __in SIZE_T SrcDataSize,
                     __in UINT uNumShaders,	      
                     __in UINT uStartIndex,
                     __in_ecount_opt(uNumShaders) UINT* pIndices,
                     __in UINT uFlags,
                     __out_ecount(uNumShaders) ID3DBlob** ppShaders,
		     __out_opt UINT* pTotalShaders);

//----------------------------------------------------------------------------
// D3DCreateBlob:
// -----------------------
// Create an ID3DBlob instance.
//----------------------------------------------------------------------------

HRESULT WINAPI
D3DCreateBlob(__in SIZE_T Size,
              __out ID3DBlob** ppBlob);

#ifdef __cplusplus
}
#endif //__cplusplus
    
#endif // #ifndef __D3DCOMPILER_H__
