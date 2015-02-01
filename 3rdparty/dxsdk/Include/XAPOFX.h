/*-========================================================================-_
 |                                - XAPOFX -                                |
 |        Copyright (c) Microsoft Corporation.  All rights reserved.        |
 |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 |PROJECT: XAPOFX                       MODEL:   Unmanaged User-mode        |
 |VERSION: 1.3                          EXCEPT:  No Exceptions              |
 |CLASS:   N / A                        MINREQ:  WinXP, Xbox360             |
 |BASE:    N / A                        DIALECT: MSC++ 14.00                |
 |>------------------------------------------------------------------------<|
 | DUTY: Cross-platform Audio Processing Objects                            |
 ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^
  NOTES:
    1.  USE THE DEBUG DLL TO ENABLE PARAMETER VALIDATION VIA ASSERTS!
        Here's how:
        Copy XAPOFXDX_X.dll to where your application exists.
        The debug DLL can be found under %WINDIR%\system32.
        Rename XAPOFXDX_X.dll to XAPOFXX_X.dll to use the debug version.    */

#pragma once
//--------------<D-E-F-I-N-I-T-I-O-N-S>-------------------------------------//
#include "comdecl.h" // for DEFINE_CLSID

// FX class IDs
DEFINE_CLSID(FXEQ,               A90BC001, E897, E897, 74, 39, 43, 55, 00, 00, 00, 00);
DEFINE_CLSID(FXMasteringLimiter, A90BC001, E897, E897, 74, 39, 43, 55, 00, 00, 00, 01);
DEFINE_CLSID(FXReverb,           A90BC001, E897, E897, 74, 39, 43, 55, 00, 00, 00, 02);
DEFINE_CLSID(FXEcho,             A90BC001, E897, E897, 74, 39, 43, 55, 00, 00, 00, 03);


#if !defined(GUID_DEFS_ONLY) // ignore rest if only GUID definitions requested
    #if defined(_XBOX)       // general windows and COM declarations
        #include <xtl.h>
        #include <xobjbase.h>
    #else
        #include <windows.h>
        #include <objbase.h>
    #endif
    #include <float.h>       // float bounds


    // EQ parameter bounds (inclusive), used with XEQ:
    #define FXEQ_MIN_FRAMERATE 22000
    #define FXEQ_MAX_FRAMERATE 48000

    #define FXEQ_MIN_FREQUENCY_CENTER       20.0f
    #define FXEQ_MAX_FREQUENCY_CENTER       20000.0f
    #define FXEQ_DEFAULT_FREQUENCY_CENTER_0 100.0f   // band 0
    #define FXEQ_DEFAULT_FREQUENCY_CENTER_1 800.0f   // band 1
    #define FXEQ_DEFAULT_FREQUENCY_CENTER_2 2000.0f  // band 2
    #define FXEQ_DEFAULT_FREQUENCY_CENTER_3 10000.0f // band 3

    #define FXEQ_MIN_GAIN     0.126f // -18dB
    #define FXEQ_MAX_GAIN     7.94f  // +18dB
    #define FXEQ_DEFAULT_GAIN 1.0f   // 0dB change, all bands

    #define FXEQ_MIN_BANDWIDTH     0.1f
    #define FXEQ_MAX_BANDWIDTH     2.0f
    #define FXEQ_DEFAULT_BANDWIDTH 1.0f // all bands


    // Mastering limiter parameter bounds (inclusive), used with XMasteringLimiter:
    #define FXMASTERINGLIMITER_MIN_RELEASE     1
    #define FXMASTERINGLIMITER_MAX_RELEASE     20
    #define FXMASTERINGLIMITER_DEFAULT_RELEASE 6

    #define FXMASTERINGLIMITER_MIN_LOUDNESS     1
    #define FXMASTERINGLIMITER_MAX_LOUDNESS     1800
    #define FXMASTERINGLIMITER_DEFAULT_LOUDNESS 1000


    // Reverb parameter bounds (inclusive), used with XReverb:
    #define FXREVERB_MIN_DIFFUSION     0.0f
    #define FXREVERB_MAX_DIFFUSION     1.0f
    #define FXREVERB_DEFAULT_DIFFUSION 0.9f

    #define FXREVERB_MIN_ROOMSIZE     0.0001f
    #define FXREVERB_MAX_ROOMSIZE     1.0f
    #define FXREVERB_DEFAULT_ROOMSIZE 0.6f


    // Echo parameter bounds (inclusive), used with XEcho:
    #define FXECHO_MIN_WETDRYMIX     0.0f
    #define FXECHO_MAX_WETDRYMIX     1.0f
    #define FXECHO_DEFAULT_WETDRYMIX 0.5f

    #define FXECHO_MIN_FEEDBACK     0.0f
    #define FXECHO_MAX_FEEDBACK     1.0f
    #define FXECHO_DEFAULT_FEEDBACK 0.5f

    #define FXECHO_MIN_DELAY     1.0f
    #define FXECHO_MAX_DELAY     2000.0f
    #define FXECHO_DEFAULT_DELAY 500.0f


//--------------<D-A-T-A---T-Y-P-E-S>---------------------------------------//
    #pragma pack(push, 1) // set packing alignment to ensure consistency across arbitrary build environments


    // EQ parameters (4 bands), used with IXAPOParameters::SetParameters:
    // The EQ supports only FLOAT32 audio foramts.
    // The framerate must be within [22000, 48000] Hz.
    typedef struct FXEQ_PARAMETERS {
        float FrequencyCenter0; // center frequency in Hz, band 0
        float Gain0;            // boost/cut
        float Bandwidth0;       // bandwidth, region of EQ is center frequency +/- bandwidth/2
        float FrequencyCenter1; // band 1
        float Gain1;
        float Bandwidth1;
        float FrequencyCenter2; // band 2
        float Gain2;
        float Bandwidth2;
        float FrequencyCenter3; // band 3
        float Gain3;
        float Bandwidth3;
    } FXEQ_PARAMETERS;


    // Mastering limiter parameters, used with IXAPOParameters::SetParameters:
    // The mastering limiter supports only FLOAT32 audio formats.
    typedef struct FXMASTERINGLIMITER_PARAMETERS {
        UINT32 Release;  // release time (tuning factor with no specific units)
        UINT32 Loudness; // loudness target (threshold)
    } FXMASTERINGLIMITER_PARAMETERS;


    // Reverb parameters, used with IXAPOParameters::SetParameters:
    // The reverb supports only FLOAT32 audio formats with the following
    // channel configurations:
    //     Input: Mono   Output: Mono
    //     Input: Stereo Output: Stereo
    typedef struct FXREVERB_PARAMETERS {
        float Diffusion; // diffusion
        float RoomSize;  // room size
    } FXREVERB_PARAMETERS;


    // Echo parameters, used with IXAPOParameters::SetParameters:
    // The echo supports only FLOAT32 audio formats.
    typedef struct FXECHO_PARAMETERS {
        float WetDryMix; // ratio of wet (processed) signal to dry (original) signal
        float Feedback;  // amount of output fed back into input
        float Delay;     // delay (all channels) in milliseconds
    } FXECHO_PARAMETERS;


//--------------<M-A-C-R-O-S>-----------------------------------------------//
    // function storage-class attribute and calltype
    #if defined(_XBOX) || !defined(FXDLL)
        #define FX_API_(type) EXTERN_C type STDAPIVCALLTYPE
    #else
        #if defined(FXEXPORT)
            #define FX_API_(type) EXTERN_C __declspec(dllexport) type STDAPIVCALLTYPE
        #else
            #define FX_API_(type) EXTERN_C __declspec(dllimport) type STDAPIVCALLTYPE
        #endif
    #endif
    #define FX_IMP_(type) type STDMETHODVCALLTYPE


//--------------<F-U-N-C-T-I-O-N-S>-----------------------------------------//
    // creates instance of requested XAPO, use Release to free instance
    FX_API_(HRESULT) CreateFX (REFCLSID clsid, __deref_out IUnknown** pEffect);


    #pragma pack(pop) // revert packing alignment
#endif // !defined(GUID_DEFS_ONLY)
//---------------------------------<-EOF->----------------------------------//
