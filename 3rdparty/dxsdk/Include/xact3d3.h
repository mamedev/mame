/*-========================================================================-_
 |                               - XACT3D3 -                                |
 |        Copyright (c) Microsoft Corporation.  All rights reserved.        |
 |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 |VERSION:  0.1                         MODEL:   Unmanaged User-mode        |
 |CONTRACT: N / A                       EXCEPT:  No Exceptions              |
 |PARENT:   N / A                       MINREQ:  Win2000, Xbox360           |
 |PROJECT:  XACT3D                      DIALECT: MS Visual C++ 7.0          |
 |>------------------------------------------------------------------------<|
 | DUTY: XACT 3D support                                                    |
 ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^
  NOTES:
    1.  See X3DAudio.h for information regarding X3DAudio types.            */


#ifndef __XACT3D3_H__
#define __XACT3D3_H__

//--------------<D-E-F-I-N-I-T-I-O-N-S>-------------------------------------//
    #include <x3daudio.h>
    #include <xact3.h>

    #pragma warning(push)
    #pragma warning(disable: 4701) // disable "local variable may be used without having been initialized" compile warning

    // Supported speaker positions, represented as azimuth angles.
    //
    // Here's a picture of the azimuth angles for the 8 cardinal points,
    // seen from above.  The emitter's base position is at the origin 0.
    //
    //           FRONT
    //             | 0  <-- azimuth
    //             |
    //    7pi/4 \  |  / pi/4
    //           \ | /
    // LEFT       \|/      RIGHT
    // 3pi/2-------0-------pi/2
    //            /|\
    //           / | \
    //    5pi/4 /  |  \ 3pi/4
    //             |
    //             | pi
    //           BACK
    //
    #define LEFT_AZIMUTH                    (3*X3DAUDIO_PI/2)
    #define RIGHT_AZIMUTH                   (X3DAUDIO_PI/2)
    #define FRONT_LEFT_AZIMUTH              (7*X3DAUDIO_PI/4)
    #define FRONT_RIGHT_AZIMUTH             (X3DAUDIO_PI/4)
    #define FRONT_CENTER_AZIMUTH            0.0f
    #define LOW_FREQUENCY_AZIMUTH           X3DAUDIO_2PI
    #define BACK_LEFT_AZIMUTH               (5*X3DAUDIO_PI/4)
    #define BACK_RIGHT_AZIMUTH              (3*X3DAUDIO_PI/4)
    #define BACK_CENTER_AZIMUTH             X3DAUDIO_PI
    #define FRONT_LEFT_OF_CENTER_AZIMUTH    (15*X3DAUDIO_PI/8)
    #define FRONT_RIGHT_OF_CENTER_AZIMUTH   (X3DAUDIO_PI/8)


//--------------<D-A-T-A---T-Y-P-E-S>---------------------------------------//
    // Supported emitter channel layouts:
    static const float aStereoLayout[] =
    {
        LEFT_AZIMUTH,
        RIGHT_AZIMUTH
    };
    static const float a2Point1Layout[] =
    {
        LEFT_AZIMUTH,
        RIGHT_AZIMUTH,
        LOW_FREQUENCY_AZIMUTH
    };
    static const float aQuadLayout[] =
    {
        FRONT_LEFT_AZIMUTH,
        FRONT_RIGHT_AZIMUTH,
        BACK_LEFT_AZIMUTH,
        BACK_RIGHT_AZIMUTH
    };
    static const float a4Point1Layout[] =
    {
        FRONT_LEFT_AZIMUTH,
        FRONT_RIGHT_AZIMUTH,
        LOW_FREQUENCY_AZIMUTH,
        BACK_LEFT_AZIMUTH,
        BACK_RIGHT_AZIMUTH
    };
    static const float a5Point1Layout[] =
    {
        FRONT_LEFT_AZIMUTH,
        FRONT_RIGHT_AZIMUTH,
        FRONT_CENTER_AZIMUTH,
        LOW_FREQUENCY_AZIMUTH,
        BACK_LEFT_AZIMUTH,
        BACK_RIGHT_AZIMUTH
    };
    static const float a7Point1Layout[] =
    {
        FRONT_LEFT_AZIMUTH,
        FRONT_RIGHT_AZIMUTH,
        FRONT_CENTER_AZIMUTH,
        LOW_FREQUENCY_AZIMUTH,
        BACK_LEFT_AZIMUTH,
        BACK_RIGHT_AZIMUTH,
        LEFT_AZIMUTH,
        RIGHT_AZIMUTH
    };


//--------------<F-U-N-C-T-I-O-N-S>-----------------------------------------//
    ////
    // DESCRIPTION:
    //  Initializes the 3D API's:
    //
    // REMARKS:
    //  This method only needs to be called once.
    //  X3DAudio will be initialized such that its speaker channel mask
    //  matches the format of the given XACT engine's final mix.
    //
    // PARAMETERS:
    //  pEngine     - [in]  XACT engine
    //  X3DInstance - [out] X3DAudio instance handle
    //
    // RETURN VALUE:
    //  HResult error code
    ////
    EXTERN_C HRESULT inline XACT3DInitialize (__in IXACT3Engine* pEngine, __in X3DAUDIO_HANDLE X3DInstance)
    {
        HRESULT hr = S_OK;
        if (pEngine == NULL) {
            hr = E_POINTER;
        }

        XACTVARIABLEVALUE nSpeedOfSound;
        if (SUCCEEDED(hr)) {
            XACTVARIABLEINDEX xactSpeedOfSoundID = pEngine->GetGlobalVariableIndex("SpeedOfSound");
            hr = pEngine->GetGlobalVariable(xactSpeedOfSoundID, &nSpeedOfSound);
        }

        if (SUCCEEDED(hr)) {
            WAVEFORMATEXTENSIBLE wfxFinalMixFormat;
            hr = pEngine->GetFinalMixFormat(&wfxFinalMixFormat);
            if (SUCCEEDED(hr)) {
                X3DAudioInitialize(wfxFinalMixFormat.dwChannelMask, nSpeedOfSound, X3DInstance);
            }
        }
        return hr;
    }


    ////
    // DESCRIPTION:
    //  Calculates DSP settings with respect to 3D parameters:
    //
    // REMARKS:
    //  Note the following flags are always specified for XACT3D calculation:
    //  X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_EMITTER_ANGLE
    //
    //  This means the caller must set at least the following fields:
    //    X3DAUDIO_LISTENER.OrientFront
    //    X3DAUDIO_LISTENER.OrientTop
    //    X3DAUDIO_LISTENER.Position
    //    X3DAUDIO_LISTENER.Velocity
    //
    //    X3DAUDIO_EMITTER.OrientFront
    //    X3DAUDIO_EMITTER.OrientTop, if emitter is multi-channel
    //    X3DAUDIO_EMITTER.Position
    //    X3DAUDIO_EMITTER.Velocity
    //    X3DAUDIO_EMITTER.InnerRadius
    //    X3DAUDIO_EMITTER.InnerRadiusAngle
    //    X3DAUDIO_EMITTER.ChannelCount
    //    X3DAUDIO_EMITTER.CurveDistanceScaler
    //    X3DAUDIO_EMITTER.DopplerScaler
    //
    //    X3DAUDIO_DSP_SETTINGS.pMatrixCoefficients, the caller need only allocate space for SrcChannelCount*DstChannelCount elements
    //    X3DAUDIO_DSP_SETTINGS.SrcChannelCount
    //    X3DAUDIO_DSP_SETTINGS.DstChannelCount
    //
    //  If X3DAUDIO_EMITTER.pChannelAzimuths is left NULL for multi-channel emitters,
    //  a default channel radius and channel azimuth array will be applied below.
    //  Distance curves such as X3DAUDIO_EMITTER.pVolumeCurve should be
    //  left NULL as XACT's native RPCs will be used to define DSP behaviour
    //  with respect to normalized distance.
    //
    //  See X3DAudio.h for information regarding X3DAudio types.
    //
    // PARAMETERS:
    //  X3DInstance  - [in]  X3DAudio instance handle, returned from XACT3DInitialize()
    //  pListener    - [in]  point of 3D audio reception
    //  pEmitter     - [in]  3D audio source
    //  pDSPSettings - [out] receives calculation results, applied to an XACT cue via XACT3DApply()
    //
    // RETURN VALUE:
    //  HResult error code
    ////
    EXTERN_C HRESULT inline XACT3DCalculate (__in X3DAUDIO_HANDLE X3DInstance, __in const X3DAUDIO_LISTENER* pListener, __inout X3DAUDIO_EMITTER* pEmitter, __inout X3DAUDIO_DSP_SETTINGS* pDSPSettings)
    {
        HRESULT hr = S_OK;
        if (pListener == NULL || pEmitter == NULL || pDSPSettings == NULL) {
            hr = E_POINTER;
        }

        if (SUCCEEDED(hr)) {
            if (pEmitter->ChannelCount > 1 && pEmitter->pChannelAzimuths == NULL) {
                pEmitter->ChannelRadius = 1.0f;

                switch (pEmitter->ChannelCount) {
                    case 2: pEmitter->pChannelAzimuths = (float*)&aStereoLayout[0]; break;
                    case 3: pEmitter->pChannelAzimuths = (float*)&a2Point1Layout[0]; break;
                    case 4: pEmitter->pChannelAzimuths = (float*)&aQuadLayout[0]; break;
                    case 5: pEmitter->pChannelAzimuths = (float*)&a4Point1Layout[0]; break;
                    case 6: pEmitter->pChannelAzimuths = (float*)&a5Point1Layout[0]; break;
                    case 8: pEmitter->pChannelAzimuths = (float*)&a7Point1Layout[0]; break;
                    default: hr = E_FAIL; break;
                }
            }
        }

        if (SUCCEEDED(hr)) {
            static X3DAUDIO_DISTANCE_CURVE_POINT DefaultCurvePoints[2] = { 0.0f, 1.0f, 1.0f, 1.0f };
            static X3DAUDIO_DISTANCE_CURVE       DefaultCurve          = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&DefaultCurvePoints[0], 2 };
            if (pEmitter->pVolumeCurve == NULL) {
                pEmitter->pVolumeCurve = &DefaultCurve;
            }
            if (pEmitter->pLFECurve == NULL) {
                pEmitter->pLFECurve = &DefaultCurve;
            }

            X3DAudioCalculate(X3DInstance, pListener, pEmitter, (X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_EMITTER_ANGLE), pDSPSettings);
        }

        return hr;
    }


    ////
    // DESCRIPTION:
    //  Applies results from a call to XACT3DCalculate() to a cue.
    //
    // PARAMETERS:
    //  pDSPSettings - [in] calculation results generated by XACT3DCalculate()
    //  pCue         - [in] cue to which to apply pDSPSettings
    //
    // RETURN VALUE:
    //  HResult error code
    ////
    EXTERN_C HRESULT inline XACT3DApply (__in const X3DAUDIO_DSP_SETTINGS* pDSPSettings, __in IXACT3Cue* pCue)
    {
        HRESULT hr = S_OK;
        if (pDSPSettings == NULL || pCue == NULL) {
            hr = E_POINTER;
        }

        if (SUCCEEDED(hr)) {
            hr = pCue->SetMatrixCoefficients(pDSPSettings->SrcChannelCount, pDSPSettings->DstChannelCount, pDSPSettings->pMatrixCoefficients);
        }
        if (SUCCEEDED(hr)) {
            XACTVARIABLEINDEX xactDistanceID = pCue->GetVariableIndex("Distance");
            hr = pCue->SetVariable(xactDistanceID, pDSPSettings->EmitterToListenerDistance);
        }
        if (SUCCEEDED(hr)) {
            XACTVARIABLEINDEX xactDopplerID = pCue->GetVariableIndex("DopplerPitchScalar");
            hr = pCue->SetVariable(xactDopplerID, pDSPSettings->DopplerFactor);
        }
        if (SUCCEEDED(hr)) {
            XACTVARIABLEINDEX xactOrientationID = pCue->GetVariableIndex("OrientationAngle");
            hr = pCue->SetVariable(xactOrientationID, pDSPSettings->EmitterToListenerAngle * (180.0f / X3DAUDIO_PI));
        }

        return hr;
    }


    #pragma warning(pop)

#endif // __XACT3D3_H__
//---------------------------------<-EOF->----------------------------------//
