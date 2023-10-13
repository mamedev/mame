/** @file paqa_devs.c
    @ingroup qa_src
    @brief Self Testing Quality Assurance app for PortAudio
    Try to open devices and run through all possible configurations.
    By default, open only the default devices. Command line options support
    opening every device, or all input devices, or all output devices.
    This test does not verify that the configuration works well.
    It just verifies that it does not crash. It requires a human to
    listen to the sine wave outputs.

    @author Phil Burk  http://www.softsynth.com

    Pieter adapted to V19 API. Test now relies heavily on
    Pa_IsFormatSupported(). Uses same 'standard' sample rates
    as in test pa_devs.c.
*/
/*
 * $Id$
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> /* for EXIT_SUCCESS and EXIT_FAILURE */
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "portaudio.h"
#include "pa_trace.h"
#include "paqa_macros.h"

/****************************************** Definitions ***********/
#define RUN_TIME_SECONDS   (1.2)
#define BYPASS_TESTS       (0) /* If 1 then skip actual tests and just iterate. */

#define MODE_INPUT         (0)
#define MODE_OUTPUT        (1)
#define MAX_TEST_CHANNELS  (4)
#define LOWEST_FREQUENCY   (300.0)
#define SINE_AMPLITUDE     (0.2)
#define MILLIS_PER_SECOND  (1000.0)
#define DEFAULT_FRAMES_PER_BUFFER  (128)

#define TEST_LEVEL_QUICK   (0)
#define TEST_LEVEL_NORMAL  (1)
#define TEST_LEVEL_EXHAUSTIVE  (2)

PAQA_INSTANTIATE_GLOBALS

typedef struct PaSineOscillator
{
    float          phase;
    float          phaseIncrement;
} PaSineOscillator;

/* Parameters that cover all options for a test.
 */
typedef struct PaQaTestParameters
{
    PaDeviceIndex    deviceID;
    PaSampleFormat   format;
    double           sampleRate;
    double           durationSeconds;
    double           suggestedLatency;
    int              framesPerBuffer;
    int              numInputChannels;
    int              numOutputChannels;
    int              mode;
    int              useCallback;
    int              useNonInterleaved; /* Test paNonInterleaved flag */
} PaQaTestParameters;

PaQaTestParameters kDefaultTestParameters = {
    0, /* deviceId */
    paFloat32,
    44100,
    RUN_TIME_SECONDS,
    0.020,
    DEFAULT_FRAMES_PER_BUFFER,
    0, /* numInputChannels */
    1, /* numOutputChannels */
    MODE_OUTPUT,
    1, /* useCallback */
    0, /* useNonInterleaved */
};

/* Runtime data used during the test. */
typedef struct PaQaData
{
    const PaQaTestParameters *parameters;
    // Dynamic state.
    int              bytesPerSample;
    volatile unsigned long    frameCounter;
    volatile unsigned long    framesLeft;
    unsigned long    framesPerBurst;
    unsigned long    minFramesPerBuffer;
    unsigned long    maxFramesPerBuffer;
    unsigned long    framesDuration;
    PaSineOscillator sineOscillators[MAX_TEST_CHANNELS];
    void            *audioBuffer;
} PaQaData;

/****************************************** Prototypes ***********/
static int TestSingleStreamParameters(PaQaTestParameters parameters);
static int QaCallback( const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData );

static void PaQaSetupData(PaQaData *myData,
                          const PaQaTestParameters *parameters)
{
    memset(myData, 0, sizeof(PaQaData));

    myData->parameters = parameters;
    myData->frameCounter = 0;
    myData->framesLeft = (unsigned long) (parameters->sampleRate * parameters->durationSeconds);

    myData->minFramesPerBuffer = UINT32_MAX;
    myData->maxFramesPerBuffer = 0;

    for (int channelIndex = 0; channelIndex < MAX_TEST_CHANNELS; channelIndex++)
    {
        myData->sineOscillators[channelIndex].phase = 0.0f;
        myData->sineOscillators[channelIndex].phaseIncrement =
                (2.0 * M_PI * LOWEST_FREQUENCY / parameters->sampleRate);
    }

    switch( parameters->format )
    {
        case paFloat32:
        case paInt32:
        case paInt24:
            myData->bytesPerSample = 4;
            break;
            /*  case paPackedInt24:
             myData->bytesPerSample = 3;
             break; */
        default:
            myData->bytesPerSample = 2;
            break;
    }
    myData->framesPerBurst = (parameters->framesPerBuffer == 0) ? 128 : parameters->framesPerBuffer;
    if (parameters->useCallback == 0) {
        /* We need our own buffer for blocking IO. */
        int numChannels = (parameters->mode == MODE_OUTPUT)
                ? parameters->numOutputChannels
                : parameters->numInputChannels;
        myData->audioBuffer = malloc(myData->bytesPerSample * numChannels * myData->framesPerBurst);
    }
}

static void PaQaTeardownData(PaQaData *myData,
                          const PaQaTestParameters *parameters)
{
    (void) parameters;
    free(myData->audioBuffer);
}

static float NextSineSample( PaSineOscillator *sineOscillator )
{
    float phase = sineOscillator->phase + sineOscillator->phaseIncrement;
    if( phase > (float)M_PI ) phase -= (float)(2.0 * M_PI);
    sineOscillator->phase = phase;
    return sinf(phase) * SINE_AMPLITUDE;
}

#define SETUP_BUFFERS(_data_type) \
    _data_type *out; \
    int stride; \
    if (parameters->useNonInterleaved) { \
        /* outputData points to an array of pointers to the buffers. */ \
        void **buffers = (void **)outputData; \
        out = (_data_type *)buffers[channelIndex]; \
        stride = 1; \
    } else { \
        out =  &((_data_type *) outputData)[channelIndex]; \
        stride = parameters->numOutputChannels; \
    }

/*******************************************************************/
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int QaCallback( const void *inputData,
                       void *outputData,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData )
{
    unsigned long frameIndex;
    int channelIndex;
    float sample;
    PaQaData *data = (PaQaData *) userData;
    const PaQaTestParameters *parameters = data->parameters;
    (void) inputData;

    data->minFramesPerBuffer = (framesPerBuffer < data->minFramesPerBuffer)
            ? framesPerBuffer : data->minFramesPerBuffer;
    data->maxFramesPerBuffer = (framesPerBuffer > data->maxFramesPerBuffer)
            ? framesPerBuffer : data->maxFramesPerBuffer;

    /* Play simple sine wave. */
    if( parameters->mode == MODE_OUTPUT )
    {
        switch( parameters->format )
        {
        case paFloat32:
            {
                for( channelIndex = 0; channelIndex < parameters->numOutputChannels; channelIndex++ )
                {
                    SETUP_BUFFERS(float);
                    for( frameIndex = 0; frameIndex < framesPerBuffer; frameIndex++ )
                    {
                        sample = NextSineSample( &data->sineOscillators[channelIndex] );
                        *out = sample;
                        out += stride;
                    }
                }
            }
            break;

        case paInt32:
            {
                for( channelIndex = 0; channelIndex < parameters->numOutputChannels; channelIndex++ )
                {
                    SETUP_BUFFERS(int32_t);
                    for( frameIndex = 0; frameIndex < framesPerBuffer; frameIndex++ )
                    {
                        sample = NextSineSample( &data->sineOscillators[channelIndex] );
                        *out = ((int32_t)(sample * 8388607)) << 8;
                        out += stride;
                    }
                }
            }
            break;

        case paInt16:
            {
                for( channelIndex = 0; channelIndex < parameters->numOutputChannels; channelIndex++ )
                {
                    SETUP_BUFFERS(int16_t);
                    for( frameIndex = 0; frameIndex < framesPerBuffer; frameIndex++ )
                    {
                        sample = NextSineSample( &data->sineOscillators[channelIndex] );
                        *out = (int16_t)(sample * 32767);
                        out += stride;
                    }
                }
            }
            break;

        default:
            {
                unsigned char *out =  (unsigned char *) outputData;
                unsigned long numBytes = framesPerBuffer * parameters->numOutputChannels * data->bytesPerSample;
                memset(out, 0, numBytes);
            }
            break;
        }
    }

    data->frameCounter += framesPerBuffer;

    /* Are we through yet? */
    if( data->framesLeft > framesPerBuffer )
    {
        PaUtil_AddTraceMessage("QaCallback: running. framesLeft", data->framesLeft );
        data->framesLeft -= framesPerBuffer;
        return paContinue;
    }
    else
    {
        PaUtil_AddTraceMessage("QaCallback: DONE! framesLeft", data->framesLeft );
        data->framesLeft = 0;
        return paComplete;
    }
}

static PaError CheckBlockingIO(PaStream *stream,
                               PaQaData *data,
                               int millis) {
    PaError result = paNoError;
    double elapsedTime = 0.0;
    double millisPerBurst = MILLIS_PER_SECOND * data->framesPerBurst / data->parameters->sampleRate;
    while (elapsedTime < millis) {
        int callbackResult;
        if (data->parameters->mode == MODE_OUTPUT) {
            callbackResult = QaCallback(NULL /*inputBuffer */,
                                        data->audioBuffer,
                                        data->framesPerBurst,
                                        NULL /* timeInfo */, // TODO
                                        0, // stream flags
                                        data);
            if (callbackResult == 0) {
                result = Pa_WriteStream(stream, data->audioBuffer, data->framesPerBurst);
                ASSERT_EQ(paNoError, result);
            }
        } else if (data->parameters->mode == MODE_INPUT) {
            result = Pa_ReadStream(stream, data->audioBuffer, data->framesPerBurst);
            ASSERT_EQ(paNoError, result);
            callbackResult = QaCallback(data->audioBuffer,
                                        NULL /*outputBuffer */,
                                        data->framesPerBurst,
                                        NULL /* timeInfo */, // TODO
                                        0, // stream flags
                                        data);
        }
        elapsedTime += millisPerBurst;
    }
error:
    return result;
}

static void CheckDefaultCallbackRun(PaStream *stream,
                        PaQaData *data) {
    PaError result = paNoError;
    PaTime oldStreamTimeMillis = 0.0;
    PaTime startStreamTimeMillis = 0.0;
    unsigned long oldFramesLeft = INT32_MAX;

    oldStreamTimeMillis = Pa_GetStreamTime(stream) * MILLIS_PER_SECOND;

    ASSERT_EQ(0, Pa_IsStreamActive(stream));
    ASSERT_EQ(1, Pa_IsStreamStopped(stream));

    ASSERT_EQ(paNoError, result = Pa_StartStream( stream ));
    startStreamTimeMillis = Pa_GetStreamTime(stream) * MILLIS_PER_SECOND;

    ASSERT_EQ(1, Pa_IsStreamActive(stream));
    ASSERT_EQ(0, Pa_IsStreamStopped(stream));

    /* Sleep long enough for the stream callback to have stopped itself. */
    while ((oldStreamTimeMillis - startStreamTimeMillis) < ((RUN_TIME_SECONDS + 0.5) * MILLIS_PER_SECOND)
           && (data->framesLeft > 0))
    {
        if (data->parameters->useCallback) {
            Pa_Sleep(200);
        } else {
            result = CheckBlockingIO(stream,
                                     data,
                                     200);
            ASSERT_EQ(paNoError, result);
        }

        PaTime newStreamTime = Pa_GetStreamTime(stream) * MILLIS_PER_SECOND;
        //printf("oldStreamTime  = %9.6f, newStreamTime = %9.6f\n", oldStreamTime, newStreamTime ); /**/
        ASSERT_LE(oldStreamTimeMillis, newStreamTime);

        /* Check to make sure callback is decrementing framesLeft. */
        unsigned long newFramesLeft = data->framesLeft;
        //printf("oldFrames = %lu, newFrames = %lu\n", oldFramesLeft, newFramesLeft );
        ASSERT_GE(oldFramesLeft, newFramesLeft);

        oldStreamTimeMillis = newStreamTime;
        oldFramesLeft = newFramesLeft;
    }

    ASSERT_EQ(0, data->framesLeft);
    ASSERT_LE((1 * data->parameters->sampleRate), data->frameCounter);

    if (data->parameters->framesPerBuffer > 0) {
        ASSERT_EQ(data->parameters->framesPerBuffer, data->minFramesPerBuffer);
        ASSERT_EQ(data->parameters->framesPerBuffer, data->maxFramesPerBuffer);
    } else {
        ASSERT_GT(data->minFramesPerBuffer, 0);
        ASSERT_LT(data->maxFramesPerBuffer, data->parameters->sampleRate);
    }

    ASSERT_EQ(data->parameters->useCallback ? 0 : 1, Pa_IsStreamActive(stream));
    ASSERT_EQ(0, Pa_IsStreamStopped(stream));

    ASSERT_EQ(paNoError, result = Pa_StopStream( stream ));

    ASSERT_EQ(0, Pa_IsStreamActive(stream));
    ASSERT_EQ(1, Pa_IsStreamStopped(stream));

    ASSERT_EQ(paNoError, result = Pa_CloseStream( stream ));
    return;

error:
    printf("result = %d = for %s\n", result, Pa_GetErrorText(result));
    Pa_CloseStream(stream);
    return;
}

/*******************************************************************/
static int TestSingleStreamParameters(PaQaTestParameters testParameters)
{
    PaStreamParameters inputParameters, outputParameters, *ipp, *opp;
    PaStream *stream = NULL;
    PaQaData myData;
    int numChannels = 0;

    if( testParameters.mode == MODE_INPUT )
    {
        opp = NULL;
        numChannels = testParameters.numInputChannels;
        inputParameters.device       = testParameters.deviceID;
        inputParameters.channelCount = testParameters.numInputChannels;
        inputParameters.sampleFormat = testParameters.format
                | (testParameters.useNonInterleaved ? paNonInterleaved : 0);
        inputParameters.suggestedLatency = testParameters.suggestedLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;
        ipp = &inputParameters;
    }
    else if( testParameters.mode == MODE_OUTPUT )
    {
        ipp = NULL;
        numChannels = testParameters.numOutputChannels;
        outputParameters.device       = testParameters.deviceID;
        outputParameters.channelCount = testParameters.numOutputChannels;
        outputParameters.sampleFormat = testParameters.format
                | (testParameters.useNonInterleaved ? paNonInterleaved : 0);
        outputParameters.suggestedLatency = testParameters.suggestedLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;
        opp = &outputParameters;
    }

    printf("------ Test: %s, device = %d"
           ", #ch = %d"
           ", rate = %5g"
           ", format = %lu"
           ", %s, %s\n",
            ( testParameters.mode == MODE_INPUT ) ? "INPUT" : "OUTPUT",
           testParameters.deviceID,
           numChannels,
           testParameters.sampleRate,
           (unsigned long)testParameters.format,
           testParameters.useCallback ? "CALLBACK" : "BLOCKING",
           testParameters.useNonInterleaved ? "NON-INT" : "INTER"
           );

    if (BYPASS_TESTS) return 0;

    /* Setup data for callback thread. */
    PaQaSetupData(&myData, &testParameters);

    if(paFormatIsSupported == Pa_IsFormatSupported( ipp, opp, testParameters.sampleRate ))
    {
        PaError resultOpen = Pa_OpenStream( &stream,
                                ipp,
                                opp,
                                testParameters.sampleRate,
                                testParameters.framesPerBuffer,
                                paClipOff,  /* we won't output out of range samples so don't bother clipping them */
                                testParameters.useCallback ? QaCallback : NULL,
                                &myData
                               );

        if (resultOpen != paNoError) {
            printf("Pa_OpenStream() returned = %d = for %s\n",
                   resultOpen, Pa_GetErrorText(resultOpen));
        }
        ASSERT_EQ(paNoError, resultOpen);
        ASSERT_TRUE(stream != NULL);

        {
            const PaStreamInfo *streamInfo = Pa_GetStreamInfo(stream);
            ASSERT_EQ((int)(testParameters.sampleRate), (int)(streamInfo->sampleRate));
            if (testParameters.mode == MODE_INPUT) {
                ASSERT_EQ(0, (int)(streamInfo->outputLatency * 1000));
            } else {
                ASSERT_EQ(0, (int)(streamInfo->inputLatency * 1000));
            }
        }
        CheckDefaultCallbackRun(stream, &myData);

    } else {
        printf("    Parameters NOT supported.\n");
    }
    PaQaTeardownData(&myData, &testParameters);
    return 0;

error:
    if( stream != NULL ) Pa_CloseStream( stream );
    PaQaTeardownData(&myData, &testParameters);
    return -1;
}


static void RunQuickTest()
{
    PaQaTestParameters parameters = kDefaultTestParameters;

#if 1
    printf("\n=========== INPUT ==============\n");
    parameters.mode = MODE_INPUT;
    parameters.deviceID = Pa_GetDefaultInputDevice();
    parameters.format = paFloat32;

    parameters.sampleRate = 44100;
    parameters.numInputChannels = 1;
    TestSingleStreamParameters(parameters);
    parameters.sampleRate = 22050;
    TestSingleStreamParameters(parameters);

    parameters.sampleRate = 44100;
    parameters.numInputChannels = 2;
    TestSingleStreamParameters(parameters);

    parameters.useCallback = 0;
    TestSingleStreamParameters(parameters); /* Blocking */
    parameters.useNonInterleaved = 1;
    TestSingleStreamParameters(parameters); /* Blocking, NonInterleaved */
    parameters.useCallback = 1;
    TestSingleStreamParameters(parameters); /* NonInterleaved */
    parameters.useCallback = 1;
#endif

    printf("\n=========== OUTPUT =============\n");
    parameters = kDefaultTestParameters;
    parameters.mode = MODE_OUTPUT;
    parameters.deviceID = Pa_GetDefaultOutputDevice();
    parameters.sampleRate = 48000;
    parameters.numOutputChannels = 1;
    parameters.format = paFloat32;
    parameters.useCallback = 0;
    TestSingleStreamParameters(parameters);

    /* Interleaved */
    parameters = kDefaultTestParameters;
    parameters.deviceID = Pa_GetDefaultOutputDevice();
    parameters.useNonInterleaved = 0;
    parameters.numOutputChannels = 1;
    parameters.useCallback = 1;
    parameters.format = paFloat32;
    TestSingleStreamParameters(parameters);
    parameters.useCallback = 0;
    parameters.format = paFloat32;
    TestSingleStreamParameters(parameters); /* Blocking */
    parameters.useCallback = 1;

    parameters.sampleRate = 44100;
    parameters.numOutputChannels = 2;
    parameters.format = paFloat32;
    TestSingleStreamParameters(parameters);

    parameters.sampleRate = 22050;
    parameters.numOutputChannels = 2;
    parameters.format = paInt16;
    TestSingleStreamParameters(parameters);

    /* Non-Interleaved */
    parameters = kDefaultTestParameters;
    parameters.deviceID = Pa_GetDefaultOutputDevice();
    parameters.useNonInterleaved = 1;
    parameters.numOutputChannels = 2;
    parameters.format = paFloat32;
    parameters.useCallback = 0;
    TestSingleStreamParameters(parameters); /* Blocking */
    parameters.useCallback = 1;
    TestSingleStreamParameters(parameters);  /* Blocking */
    parameters.format = paInt16;
    TestSingleStreamParameters(parameters);
    parameters.format = paInt32;
    TestSingleStreamParameters(parameters);
}

static const double constStandardSampleRates_[] = {
        8000.0,  9600.0, 11025.0, 12000.0, 16000.0, 22050.0,
        24000.0, 32000.0, 44100.0, 48000.0, 88200.0, 96000.0,
        -1.0 }; /* Negative terminated list. */

static const PaSampleFormat constFormatsToTest_[] = {
    paFloat32, paInt32, paInt16, 0
}; /* Zero terminated list. */

/**
 * Iterate through each option with other options set to default.
 */
static void TestNormal( int mode, int allDevices )
{
    PaQaTestParameters parameters = kDefaultTestParameters;
    int id, jc, i;
    int maxChannels;
    int isDefault;
    const PaDeviceInfo *pdi;
    int numDevices = Pa_GetDeviceCount();
    parameters.mode = mode;

    for( id=0; id<numDevices; id++ )            /* Iterate through all devices. */
    {
        parameters.deviceID = id;

        pdi = Pa_GetDeviceInfo( id );

        if( mode == MODE_INPUT ) {
            maxChannels = pdi->maxInputChannels;
            isDefault = ( id == Pa_GetDefaultInputDevice());
        } else {
            maxChannels = pdi->maxOutputChannels;
            isDefault = ( id == Pa_GetDefaultOutputDevice());
        }
        if( maxChannels > MAX_TEST_CHANNELS )
            maxChannels = MAX_TEST_CHANNELS;
        if (maxChannels == 0) continue;  // skip this device, wrong direction

        if (!allDevices && !isDefault) continue; // skip this device

        printf("\n===========================================================\n");
        printf("            Device = %s\n", pdi->name );
        printf("===========================================================\n");
        for( jc=1; jc<=maxChannels; jc++ )
        {
            if (mode == MODE_INPUT) {
                parameters.numInputChannels = jc;
            } else {
                parameters.numOutputChannels = jc;
            }
            TestSingleStreamParameters(parameters);
        }

        /* Try each standard sample rate. */
        for( i=0; constStandardSampleRates_[i] > 0; i++ )
        {
            parameters.sampleRate = constStandardSampleRates_[i];
            TestSingleStreamParameters(parameters);
        }
        parameters.sampleRate = pdi->defaultSampleRate;

        if (mode == MODE_INPUT) {
            parameters.suggestedLatency = pdi->defaultHighInputLatency;
            TestSingleStreamParameters(parameters);
            parameters.suggestedLatency = pdi->defaultLowInputLatency;
            TestSingleStreamParameters(parameters);
        } else {
            parameters.suggestedLatency = pdi->defaultHighOutputLatency;
            TestSingleStreamParameters(parameters);
            parameters.suggestedLatency = pdi->defaultLowOutputLatency;
            TestSingleStreamParameters(parameters);
        }

        for (int callback = 0; callback < 2; callback++) {
            parameters.useCallback = callback;
            for (int nonInterleaved = 0; nonInterleaved < 2; nonInterleaved++) {
                parameters.useNonInterleaved = nonInterleaved;
                TestSingleStreamParameters(parameters);
            }
        }
        parameters.useCallback = 1;
        parameters.useNonInterleaved = 0;

        for (int jf = 0; constFormatsToTest_[jf] > 0; jf++) {
            parameters.format = constFormatsToTest_[jf];
            TestSingleStreamParameters(parameters);
        }
    }
}

/*******************************************************************
* Test each output device, through its full range of capabilities. */
static void TestExhaustive( int mode, int allDevices )
{
    PaQaTestParameters parameters = kDefaultTestParameters;
    int id, jc, i;
    int maxChannels;
    int isDefault;
    const PaDeviceInfo *pdi;
    int numDevices = Pa_GetDeviceCount();
    parameters.mode = mode;

    for( id=0; id<numDevices; id++ )            /* Iterate through all devices. */
    {
        parameters.deviceID = id;

        pdi = Pa_GetDeviceInfo( id );

        if( mode == MODE_INPUT ) {
            maxChannels = pdi->maxInputChannels;
            isDefault = ( id == Pa_GetDefaultInputDevice());
        } else {
            maxChannels = pdi->maxOutputChannels;
            isDefault = ( id == Pa_GetDefaultOutputDevice());
        }
        if( maxChannels > MAX_TEST_CHANNELS )
            maxChannels = MAX_TEST_CHANNELS;
        if (maxChannels == 0) continue;  // skip this device, wrong direction

        if (!allDevices && !isDefault) continue; // skip this device

        printf("\n===========================================================\n");
        printf("            Device = %s\n", pdi->name );
        printf("===========================================================\n");
        for( jc=1; jc<=maxChannels; jc++ )
        {
            printf("\n---------------------- NumChannels = %d ------------\n", jc );
            if (mode == MODE_INPUT) {
                parameters.numInputChannels = jc;
            } else {
                parameters.numOutputChannels = jc;
            }
            /* Try each standard sample rate. */
            for( i=0; constStandardSampleRates_[i] > 0; i++ )
            {
                parameters.sampleRate = constStandardSampleRates_[i];
                for (int callback = 0; callback < 2; callback++) {
                    parameters.useCallback = callback;
                    for (int nonInterleaved = 0; nonInterleaved < 2; nonInterleaved++) {
                        parameters.useNonInterleaved = nonInterleaved;
                        for (int jf = 0; constFormatsToTest_[jf] > 0; jf++) {
                            parameters.format = constFormatsToTest_[jf];
                            TestSingleStreamParameters(parameters);
                        }
                    }
                }
            }
        }
    }
}

/*******************************************************************/
static void usage( const char *name )
{
    printf("%s [-a] {-tN}\n", name);
    printf("  -a - Test ALL devices, otherwise just the default devices.\n");
    printf("  -i - test INPUT only.\n");
    printf("  -o - test OUTPUT only.\n");
    printf("  -t - Test level, 0=Quick, 1=Normal, 2=Exhaustive\n");
    printf("  -? - Help\n");
}

/*******************************************************************/
int main( int argc, char **argv );
int main( int argc, char **argv )
{
    int     i;
    PaError result;
    int     allDevices = 0;
    int     testOutput = 1;
    int     testInput = 1;
    int     testLevel = TEST_LEVEL_NORMAL;
    char   *executableName = argv[0];

    /* Parse command line parameters. */
    i = 1;
    while( i<argc )
    {
        char *arg = argv[i];
        if( arg[0] == '-' )
        {
            switch(arg[1])
            {
                case 'a':
                    allDevices = 1;
                    break;
                case 'i':
                    testOutput = 0;
                    break;
                case 'o':
                    testInput = 0;
                    break;
                case 't':
                    testLevel = atoi(&arg[2]);
                    break;

                default:
                    printf("Illegal option: %s\n", arg);
                case '?':
                    usage( executableName );
                    exit(1);
                    break;
            }
        }
        else
        {
            printf("Illegal argument: %s\n", arg);
            usage( executableName );
            return 1;
        }
        i += 1;
    }

    ASSERT_EQ(2, sizeof(short)); /* The callback assumes we have 16-bit shorts. */
    ASSERT_EQ(4, sizeof(int)); /* The callback assumes we have 32-bit ints. */
    ASSERT_EQ(paNoError, (result=Pa_Initialize()));

    if (testLevel == TEST_LEVEL_QUICK) {
        printf("\n---- Quick Test ---------------\n");
        RunQuickTest();
    } else {
        if( testInput )
        {
            printf("\n---- Test INPUT ---------------\n");
            if (testLevel == TEST_LEVEL_NORMAL) {
                TestNormal( MODE_INPUT, allDevices );
            } else {
                TestExhaustive( MODE_INPUT, allDevices );
            }
        }
        if( testOutput )
        {
            printf("\n---- Test OUTPUT ---------------\n");
            if (testLevel == TEST_LEVEL_NORMAL) {
                TestNormal( MODE_OUTPUT, allDevices );
            } else {
                TestExhaustive( MODE_OUTPUT, allDevices );
            }
        }
    }

error:
    ASSERT_EQ(paNoError, Pa_Terminate());

    PAQA_PRINT_RESULT;
    return PAQA_EXIT_RESULT;
}
