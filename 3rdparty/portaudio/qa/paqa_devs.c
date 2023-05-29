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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "portaudio.h"
#include "pa_trace.h"

/****************************************** Definitions ***********/
#define MODE_INPUT         (0)
#define MODE_OUTPUT        (1)
#define MAX_TEST_CHANNELS  (4)
#define LOWEST_FREQUENCY   (300.0)
#define LOWEST_SAMPLE_RATE (8000.0)
#define PHASE_INCREMENT    (2.0 * M_PI * LOWEST_FREQUENCY / LOWEST_SAMPLE_RATE)
#define SINE_AMPLITUDE     (0.2)

typedef struct PaQaData
{
    unsigned long  framesLeft;
    int            numChannels;
    int            bytesPerSample;
    int            mode;
    float          phase;
    PaSampleFormat format;
}
PaQaData;

/****************************************** Prototypes ***********/
static void TestDevices( int mode, int allDevices );
static void TestFormats( int mode, PaDeviceIndex deviceID, double sampleRate,
                         int numChannels );
static int TestAdvance( int mode, PaDeviceIndex deviceID, double sampleRate,
                        int numChannels, PaSampleFormat format );
static int QaCallback( const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData );

/****************************************** Globals ***********/
static int gNumPassed = 0;
static int gNumFailed = 0;

/****************************************** Macros ***********/
/* Print ERROR if it fails. Tally success or failure. */
/* Odd do-while wrapper seems to be needed for some compilers. */
#define EXPECT(_exp) \
    do \
    { \
        if ((_exp)) {\
            /* printf("SUCCESS for %s\n", #_exp ); */ \
            gNumPassed++; \
        } \
        else { \
            printf("ERROR - 0x%x - %s for %s\n", result, \
                    ((result == 0) ? "-" : Pa_GetErrorText(result)), \
                    #_exp ); \
            gNumFailed++; \
            goto error; \
        } \
    } while(0)

static float NextSineSample( PaQaData *data )
{
    float phase = data->phase + PHASE_INCREMENT;
    if( phase > M_PI ) phase -= (float) (2.0 * M_PI);
    data->phase = phase;
    return sinf(phase) * SINE_AMPLITUDE;
}

/*******************************************************************/
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int QaCallback( const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData )
{
    unsigned long frameIndex;
    unsigned long channelIndex;
    float sample;
    PaQaData *data = (PaQaData *) userData;
    (void) inputBuffer;

    /* Play simple sine wave. */
    if( data->mode == MODE_OUTPUT )
    {
        switch( data->format )
        {
        case paFloat32:
            {
                float *out =  (float *) outputBuffer;
                for( frameIndex = 0; frameIndex < framesPerBuffer; frameIndex++ )
                {
                    sample = NextSineSample( data );
                    for( channelIndex = 0; channelIndex < data->numChannels; channelIndex++ )
                    {
                        *out++ = sample;
                    }
                }
            }
            break;

        case paInt32:
            {
                int *out =  (int *) outputBuffer;
                for( frameIndex = 0; frameIndex < framesPerBuffer; frameIndex++ )
                {
                    sample = NextSineSample( data );
                    for( channelIndex = 0; channelIndex < data->numChannels; channelIndex++ )
                    {
                        *out++ = ((int)(sample * 0x00800000)) << 8;
                    }
                }
            }
            break;

        case paInt16:
            {
                short *out =  (short *) outputBuffer;
                for( frameIndex = 0; frameIndex < framesPerBuffer; frameIndex++ )
                {
                    sample = NextSineSample( data );
                    for( channelIndex = 0; channelIndex < data->numChannels; channelIndex++ )
                    {
                        *out++ = (short)(sample * 32767);
                    }
                }
            }
            break;

        default:
            {
                unsigned char *out =  (unsigned char *) outputBuffer;
                unsigned long numBytes = framesPerBuffer * data->numChannels * data->bytesPerSample;
                memset(out, 0, numBytes);
            }
            break;
        }
    }
    /* Are we through yet? */
    if( data->framesLeft > framesPerBuffer )
    {
        PaUtil_AddTraceMessage("QaCallback: running. framesLeft", data->framesLeft );
        data->framesLeft -= framesPerBuffer;
        return 0;
    }
    else
    {
        PaUtil_AddTraceMessage("QaCallback: DONE! framesLeft", data->framesLeft );
        data->framesLeft = 0;
        return 1;
    }
}

/*******************************************************************/
static void usage( const char *name )
{
    printf("%s [-a]\n", name);
    printf("  -a - Test ALL devices, otherwise just the default devices.\n");
    printf("  -i - Test INPUT only.\n");
    printf("  -o - Test OUTPUT only.\n");
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

    EXPECT(sizeof(short) == 2); /* The callback assumes we have 16-bit shorts. */
    EXPECT(sizeof(int) == 4); /* The callback assumes we have 32-bit ints. */
    EXPECT( ((result=Pa_Initialize()) == 0) );

    if( testOutput )
    {
        printf("\n---- Test OUTPUT ---------------\n");
        TestDevices( MODE_OUTPUT, allDevices );
    }
    if( testInput )
    {
        printf("\n---- Test INPUT ---------------\n");
        TestDevices( MODE_INPUT, allDevices );
    }

error:
    Pa_Terminate();
    printf("QA Report: %d passed, %d failed.\n", gNumPassed, gNumFailed );
    return (gNumFailed > 0) ? 1 : 0;
}

/*******************************************************************
* Try each output device, through its full range of capabilities. */
static void TestDevices( int mode, int allDevices )
{
    int id, jc, i;
    int maxChannels;
    int isDefault;
    const PaDeviceInfo *pdi;
    static double standardSampleRates[] = {  8000.0,  9600.0, 11025.0, 12000.0,
                                            16000.0,          22050.0, 24000.0,
                                            32000.0,          44100.0, 48000.0,
                                                              88200.0, 96000.0,
                                               -1.0 }; /* Negative terminated list. */
    int numDevices = Pa_GetDeviceCount();
    for( id=0; id<numDevices; id++ )            /* Iterate through all devices. */
    {
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

        if (!allDevices && !isDefault) continue; // skip this device

        for( jc=1; jc<=maxChannels; jc++ )
        {
            printf("\n===========================================================\n");
            printf("            Device = %s\n", pdi->name );
            printf("===========================================================\n");
            /* Try each standard sample rate. */
            for( i=0; standardSampleRates[i] > 0; i++ )
            {
                TestFormats( mode, (PaDeviceIndex)id, standardSampleRates[i], jc );
            }
        }
    }
}

/*******************************************************************/
static void TestFormats( int mode, PaDeviceIndex deviceID, double sampleRate,
                         int numChannels )
{
    TestAdvance( mode, deviceID, sampleRate, numChannels, paFloat32 );
    TestAdvance( mode, deviceID, sampleRate, numChannels, paInt16 );
    TestAdvance( mode, deviceID, sampleRate, numChannels, paInt32 );
    /* TestAdvance( mode, deviceID, sampleRate, numChannels, paInt24 ); */
}

/*******************************************************************/
static int TestAdvance( int mode, PaDeviceIndex deviceID, double sampleRate,
                        int numChannels, PaSampleFormat format )
{
    PaStreamParameters inputParameters, outputParameters, *ipp, *opp;
    PaStream *stream = NULL;
    PaError result = paNoError;
    PaQaData myData;
    #define FRAMES_PER_BUFFER  (64)
    const int kNumSeconds = 100;

    /* Setup data for synthesis thread. */
    myData.framesLeft = (unsigned long) (sampleRate * kNumSeconds);
    myData.numChannels = numChannels;
    myData.mode = mode;
    myData.format = format;
    switch( format )
    {
    case paFloat32:
    case paInt32:
    case paInt24:
        myData.bytesPerSample = 4;
        break;
/*  case paPackedInt24:
        myData.bytesPerSample = 3;
        break; */
    default:
        myData.bytesPerSample = 2;
        break;
    }

    if( mode == MODE_INPUT )
    {
        inputParameters.device       = deviceID;
        inputParameters.channelCount = numChannels;
        inputParameters.sampleFormat = format;
        inputParameters.suggestedLatency =
                Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;
        ipp = &inputParameters;
    }
    else
    {
        ipp = NULL;
    }

    if( mode == MODE_OUTPUT )
    {
        outputParameters.device       = deviceID;
        outputParameters.channelCount = numChannels;
        outputParameters.sampleFormat = format;
        outputParameters.suggestedLatency =
                Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;
        opp = &outputParameters;
    }
    else
    {
        opp = NULL;
    }

    if(paFormatIsSupported == Pa_IsFormatSupported( ipp, opp, sampleRate ))
    {
        printf("------ TestAdvance: %s, device = %d, rate = %g"
               ", numChannels = %d, format = %lu -------\n",
                ( mode == MODE_INPUT ) ? "INPUT" : "OUTPUT",
                deviceID, sampleRate, numChannels, (unsigned long)format);
        EXPECT( ((result = Pa_OpenStream( &stream,
                                          ipp,
                                          opp,
                                          sampleRate,
                                          FRAMES_PER_BUFFER,
                                          paClipOff,  /* we won't output out of range samples so don't bother clipping them */
                                          QaCallback,
                                          &myData ) ) == 0) );
        if( stream )
        {
            PaTime oldStamp, newStamp;
            unsigned long oldFrames;
            int minDelay = ( mode == MODE_INPUT ) ? 1000 : 400;
            /* Was:
            int minNumBuffers = Pa_GetMinNumBuffers( FRAMES_PER_BUFFER, sampleRate );
            int msec = (int) ((minNumBuffers * 3 * 1000.0 * FRAMES_PER_BUFFER) / sampleRate);
            */
            int msec = (int)( 3.0 *
                       (( mode == MODE_INPUT ) ? inputParameters.suggestedLatency : outputParameters.suggestedLatency ));
            if( msec < minDelay ) msec = minDelay;
            printf("msec = %d\n", msec);  /**/
            EXPECT( ((result=Pa_StartStream( stream )) == 0) );
            /* Check to make sure PortAudio is advancing timeStamp. */
            oldStamp = Pa_GetStreamTime(stream);
            Pa_Sleep(msec);
            newStamp = Pa_GetStreamTime(stream);
            printf("oldStamp  = %9.6f, newStamp = %9.6f\n", oldStamp, newStamp ); /**/
            EXPECT( (oldStamp < newStamp) );
            /* Check to make sure callback is decrementing framesLeft. */
            oldFrames = myData.framesLeft;
            Pa_Sleep(msec);
            printf("oldFrames = %lu, myData.framesLeft = %lu\n", oldFrames,  myData.framesLeft ); /**/
            EXPECT( (oldFrames > myData.framesLeft) );
            EXPECT( ((result=Pa_CloseStream( stream )) == 0) );
            stream = NULL;
        }
    }
    return 0;
error:
    if( stream != NULL ) Pa_CloseStream( stream );
    return -1;
}
