/*
 * $Id$
 * PortAudio Portable Real-Time Audio Library
 * Latest Version at: http://www.portaudio.com
 * Solaris/NetBSD implementation by:
 *   Nia Alarie
 *
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 1999-2002 Ross Bencina, Phil Burk
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

/** @file
 @ingroup hostapi_src
*/

#include <sys/audioio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "pa_util.h"
#include "pa_unix_util.h"
#include "pa_debugprint.h"
#include "pa_allocation.h"
#include "pa_hostapi.h"
#include "pa_stream.h"
#include "pa_cpuload.h"
#include "pa_process.h"

#ifndef AUDIOIO_MAX_DEVICES
#define AUDIOIO_MAX_DEVICES    (32)
#endif

#ifndef AUDIOIO_DEFAULT_FRAMES
#define AUDIOIO_DEFAULT_FRAMES (128)
#endif

#ifndef AUDIOIO_DEFAULT_MAX_CHANNELS
# ifdef __NetBSD__
#  define AUDIOIO_DEFAULT_MAX_CHANNELS (12)
# else
#  define AUDIOIO_DEFAULT_MAX_CHANNELS (2)
# endif
#endif

#ifndef AUDIOIO_DEV_PREFIX
# ifdef __sun
#  define AUDIOIO_DEV_PREFIX   "/dev/sound/"
# else
#  define AUDIOIO_DEV_PREFIX   "/dev/audio"
# endif
#endif

#ifndef AUDIOIO_DEV_DEFAULT
#define AUDIOIO_DEV_DEFAULT    "/dev/audio"
#endif

#ifndef AUDIO_ENCODING_SLINEAR
#define AUDIO_ENCODING_SLINEAR AUDIO_ENCODING_LINEAR
#endif

// AUDIO_GETBUFINFO is simply an optimization
#ifndef AUDIO_GETBUFINFO
#define AUDIO_GETBUFINFO AUDIO_GETINFO
#endif

PaError PaAudioIO_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
static void Terminate( struct PaUtilHostApiRepresentation *hostApi );
static bool AttemptEncoding( int fd, int encoding, int precision, bool record );
static PaSampleFormat GetSupportedEncodings( int fd, bool record );
static PaError PaFormatToAudioIOFormat( PaSampleFormat fmt, struct audio_prinfo *info );
static PaError IsFormatSupported( struct PaUtilHostApiRepresentation *hostApi,
                                  const PaStreamParameters *inputParameters,
                                  const PaStreamParameters *outputParameters,
                                  double sampleRate );
static PaError OpenStream( struct PaUtilHostApiRepresentation *hostApi,
                           PaStream** s,
                           const PaStreamParameters *inputParameters,
                           const PaStreamParameters *outputParameters,
                           double sampleRate,
                           unsigned long framesPerBuffer,
                           PaStreamFlags streamFlags,
                           PaStreamCallback *streamCallback,
                           void *userData );
static PaError CloseStream( PaStream* stream );
static PaError StartStream( PaStream *stream );
static PaError StopStream( PaStream *stream );
static PaError AbortStream( PaStream *stream );
static PaError IsStreamStopped( PaStream *s );
static PaError IsStreamActive( PaStream *stream );
static PaTime GetStreamTime( PaStream *stream );
static double GetStreamCpuLoad( PaStream* stream );
static PaError ReadStream( PaStream* stream, void *buffer, unsigned long frames );
static PaError WriteStream( PaStream* stream, const void *buffer, unsigned long frames );
static signed long GetStreamReadAvailable( PaStream* stream );
static signed long GetStreamWriteAvailable( PaStream* stream );

/* PaAudioIOHostApiRepresentation - host api datastructure specific to this implementation */

typedef struct
{
    PaUtilHostApiRepresentation inheritedHostApiRep;
    PaUtilStreamInterface callbackStreamInterface;
    PaUtilStreamInterface blockingStreamInterface;
    PaUtilAllocationGroup *allocations;
}
PaAudioIOHostApiRepresentation;


PaError PaAudioIO_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex hostApiIndex )
{
    PaError result = paNoError;
    int fd = -1, i, deviceCount = 0;
    PaAudioIOHostApiRepresentation *audioIOHostApi;
    PaUtilHostApiRepresentation *common;

    PA_UNLESS( audioIOHostApi = calloc(1, sizeof(PaAudioIOHostApiRepresentation)), paInsufficientMemory );

    common = &audioIOHostApi->inheritedHostApiRep;
    common->info.structVersion = 1;
    common->info.type = paAudioIO;
    common->info.name = "AudioIO";

    common->Terminate = Terminate;
    common->OpenStream = OpenStream;
    common->IsFormatSupported = IsFormatSupported;

    common->info.defaultInputDevice =
    common->info.defaultOutputDevice = paNoDevice;

    PA_UNLESS( audioIOHostApi->allocations = PaUtil_CreateAllocationGroup(), paInsufficientMemory );

    if( (common->deviceInfos = PaUtil_GroupAllocateZeroInitializedMemory(
         audioIOHostApi->allocations, sizeof(PaDeviceInfo *) * AUDIOIO_MAX_DEVICES)) == NULL )
    {
        result = paInsufficientMemory;
        goto error;
    }

    for( i=0; i < AUDIOIO_MAX_DEVICES; ++i )
    {
        PaDeviceInfo *dev;
        struct audio_info hwfmt, info;
        char path[16];
        int props;

        if( i > 0 )
            (void)snprintf(path, sizeof(path), AUDIOIO_DEV_PREFIX "%d", i - 1);
        else
            (void)snprintf(path, sizeof(path), AUDIOIO_DEV_DEFAULT);

        fd = open(path, O_WRONLY | O_NONBLOCK);
        if (fd < 0)
            fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0)
            continue;

        PA_UNLESS(dev = PaUtil_GroupAllocateZeroInitializedMemory(audioIOHostApi->allocations, sizeof(PaDeviceInfo)), paInsufficientMemory);

        dev->structVersion = 2;
        dev->hostApi = hostApiIndex;

        PA_UNLESS(dev->name = PaUtil_GroupAllocateZeroInitializedMemory(audioIOHostApi->allocations, sizeof(path)), paInsufficientMemory);

        memcpy((char *)dev->name, path, sizeof(path));

        dev->defaultSampleRate = 48000.;

        dev->maxInputChannels = dev->maxOutputChannels = AUDIOIO_DEFAULT_MAX_CHANNELS;

        dev->defaultLowInputLatency =
        dev->defaultLowOutputLatency =
        dev->defaultHighInputLatency =
        dev->defaultHighOutputLatency = .04;

#ifdef AUDIO_GETFORMAT
        if( ioctl(fd, AUDIO_GETFORMAT, &hwfmt) != -1 )
        {
            if( dev->maxInputChannels > hwfmt.record.channels )
                dev->maxInputChannels = hwfmt.record.channels;
            if( dev->maxOutputChannels > hwfmt.play.channels )
                dev->maxOutputChannels = hwfmt.play.channels;
            dev->defaultSampleRate = (dev->maxOutputChannels > 0) ?
                hwfmt.play.sample_rate : hwfmt.record.sample_rate;
        }
#endif

#ifdef AUDIO_GETPROPS
        if( ioctl(fd, AUDIO_GETPROPS, &props) != -1 )
        {
            if( (props & AUDIO_PROP_PLAYBACK) == 0 )
                dev->maxOutputChannels = 0;
            if( (props & AUDIO_PROP_CAPTURE) == 0 )
                dev->maxInputChannels = 0;
        }
#endif

#ifndef __sun
        AUDIO_INITINFO(&info);
        if( dev->maxOutputChannels > 0 )
        {
            info.play.encoding = AUDIO_ENCODING_SLINEAR;
            info.play.precision = 16;
            info.play.sample_rate = dev->defaultSampleRate;
            info.play.channels = dev->maxOutputChannels;
        }
        if( dev->maxInputChannels > 0 )
        {
            info.record.encoding = AUDIO_ENCODING_SLINEAR;
            info.record.precision = 16;
            info.record.sample_rate = dev->defaultSampleRate;
            info.record.channels = dev->maxInputChannels;
        }
        if( ioctl(fd, AUDIO_SETINFO, &info) != -1  &&
            ioctl(fd, AUDIO_GETBUFINFO, &info) != -1 )
        {
            if( dev->maxOutputChannels > 0 )
            {
                dev->defaultLowOutputLatency =
                dev->defaultHighOutputLatency =
                    info.blocksize / (info.play.channels * (info.play.precision / 8)) /
                    (double)info.play.sample_rate;
            }
            if( dev->maxInputChannels > 0 )
            {
                dev->defaultLowInputLatency =
                dev->defaultHighInputLatency =
                    info.blocksize / (info.record.channels * (info.record.precision / 8)) /
                    (double)info.record.sample_rate;
            }
        }
#endif

        if( common->info.defaultInputDevice == paNoDevice && dev->maxInputChannels > 0 )
            common->info.defaultInputDevice = deviceCount;

        if( common->info.defaultOutputDevice == paNoDevice && dev->maxOutputChannels > 0 )
            common->info.defaultOutputDevice = deviceCount;

        common->deviceInfos[deviceCount++] = dev;

        close(fd);
        fd = -1;
    }

    common->info.deviceCount = deviceCount;

    *hostApi = common;

    PaUtil_InitializeStreamInterface( &audioIOHostApi->callbackStreamInterface, CloseStream, StartStream,
                                      StopStream, AbortStream, IsStreamStopped, IsStreamActive,
                                      GetStreamTime, GetStreamCpuLoad,
                                      PaUtil_DummyRead, PaUtil_DummyWrite,
                                      PaUtil_DummyGetReadAvailable, PaUtil_DummyGetWriteAvailable );

    PaUtil_InitializeStreamInterface( &audioIOHostApi->blockingStreamInterface, CloseStream, StartStream,
                                      StopStream, AbortStream, IsStreamStopped, IsStreamActive,
                                      GetStreamTime, PaUtil_DummyGetCpuLoad,
                                      ReadStream, WriteStream, GetStreamReadAvailable, GetStreamWriteAvailable );
    return result;

error:
    if( fd != -1 )
        close(fd);

    if( audioIOHostApi )
    {
        if( audioIOHostApi->allocations )
        {
            PaUtil_FreeAllAllocations( audioIOHostApi->allocations );
            PaUtil_DestroyAllocationGroup( audioIOHostApi->allocations );
        }
        free( audioIOHostApi );
    }
    return result;
}


static void Terminate( struct PaUtilHostApiRepresentation *hostApi )
{
    PaAudioIOHostApiRepresentation *audioIOHostApi = (PaAudioIOHostApiRepresentation*)hostApi;

    if( audioIOHostApi )
    {
        if( audioIOHostApi->allocations )
        {
            PaUtil_FreeAllAllocations( audioIOHostApi->allocations );
            PaUtil_DestroyAllocationGroup( audioIOHostApi->allocations );
        }
        free( audioIOHostApi );
    }
}


static PaError IsFormatSupported( struct PaUtilHostApiRepresentation *hostApi,
                                  const PaStreamParameters *inputParameters,
                                  const PaStreamParameters *outputParameters,
                                  double sampleRate )
{
    int inputChannelCount, outputChannelCount;
    PaSampleFormat inputSampleFormat, outputSampleFormat;
    PaError result = paFormatIsSupported;
    PaDeviceInfo *indev, *outdev;
    struct audio_info info;
    int fd = -1;

    if( inputParameters )
    {
        inputChannelCount = inputParameters->channelCount;
        inputSampleFormat = inputParameters->sampleFormat;

        /* all standard sample formats are supported by the buffer adapter,
            this implementation doesn't support any custom sample formats */
        if( inputSampleFormat & paCustomFormat )
            return paSampleFormatNotSupported;

        /* unless alternate device specification is supported, reject the use of
            paUseHostApiSpecificDeviceSpecification */

        if( inputParameters->device == paUseHostApiSpecificDeviceSpecification )
            return paInvalidDevice;

        indev = hostApi->deviceInfos[ inputParameters->device ];

        /* check that input device can support inputChannelCount */
        if( inputChannelCount > indev->maxInputChannels )
            return paInvalidChannelCount;

        /* validate inputStreamInfo */
        if( inputParameters->hostApiSpecificStreamInfo )
            return paIncompatibleHostApiSpecificStreamInfo; /* this implementation doesn't use custom stream info */
    }
    else
    {
        inputChannelCount = 0;
    }

    if( outputParameters )
    {
        outputChannelCount = outputParameters->channelCount;
        outputSampleFormat = outputParameters->sampleFormat;

        /* all standard sample formats are supported by the buffer adapter,
            this implementation doesn't support any custom sample formats */
        if( outputSampleFormat & paCustomFormat )
            return paSampleFormatNotSupported;

        /* unless alternate device specification is supported, reject the use of
            paUseHostApiSpecificDeviceSpecification */

        if( outputParameters->device == paUseHostApiSpecificDeviceSpecification )
            return paInvalidDevice;

        outdev = hostApi->deviceInfos[ outputParameters->device ];

        /* check that output device can support outputChannelCount */
        if( outputChannelCount > outdev->maxOutputChannels )
            return paInvalidChannelCount;

        /* validate outputStreamInfo */
        if( outputParameters->hostApiSpecificStreamInfo )
            return paIncompatibleHostApiSpecificStreamInfo; /* this implementation doesn't use custom stream info */
    }
    else
    {
        outputChannelCount = 0;
    }

    if( inputChannelCount > 0 && outputChannelCount > 0 )
    {
        if( strcmp(indev->name, outdev->name) == 0 )
        {
            PA_UNLESS( (fd = open(indev->name, O_RDWR | O_NONBLOCK)) != -1, paDeviceUnavailable );
            AUDIO_INITINFO(&info);
            info.play.channels = outputChannelCount;
            info.record.channels = inputChannelCount;
            PA_UNLESS( ioctl(fd, AUDIO_SETINFO, &info) != -1, paInvalidChannelCount );
            AUDIO_INITINFO(&info);
            info.play.sample_rate = sampleRate;
            info.record.sample_rate = sampleRate;
            PA_UNLESS( ioctl(fd, AUDIO_SETINFO, &info) != -1, paInvalidSampleRate );
            close(fd);
            return result;
        }
    }

    if( inputChannelCount > 0 )
    {
        PA_UNLESS( (fd = open(indev->name, O_RDONLY | O_NONBLOCK)) != -1, paDeviceUnavailable );
        AUDIO_INITINFO(&info);
        info.record.channels = inputChannelCount;
        PA_UNLESS( ioctl(fd, AUDIO_SETINFO, &info) != -1, paInvalidChannelCount );
        AUDIO_INITINFO(&info);
        info.record.sample_rate = sampleRate;
        PA_UNLESS( ioctl(fd, AUDIO_SETINFO, &info) != -1, paInvalidSampleRate );
        close(fd);
        fd = -1;
    }

    if( outputChannelCount > 0 )
    {
        PA_UNLESS( (fd = open(outdev->name, O_WRONLY | O_NONBLOCK)) != -1, paDeviceUnavailable );
        AUDIO_INITINFO(&info);
        info.play.channels = outputChannelCount;
        PA_UNLESS( ioctl(fd, AUDIO_SETINFO, &info) != -1, paInvalidChannelCount );
        AUDIO_INITINFO(&info);
        info.play.sample_rate = sampleRate;
        PA_UNLESS( ioctl(fd, AUDIO_SETINFO, &info) != -1, paInvalidSampleRate );
        close(fd);
        fd = -1;
    }

error:
    if( result == paInvalidChannelCount )
        PA_DEBUG(("PaAudioIO %s: Invalid channels %d in %d out\n", __FUNCTION__, inputChannelCount, outputChannelCount));
    if( result == paInvalidSampleRate )
        PA_DEBUG(("PaAudioIO %s: Invalid sample rate %d Hz\n", __FUNCTION__, (int)sampleRate));
    if( fd != -1 )
        close(fd);
    return result;
}

typedef struct PaAudioIOComponent
{
    int fd;
    char name[16];
    void *chanbufs[16];
    void *buffer;
    unsigned int bufferSize; /* in frames */
    unsigned int channels;
    unsigned int frameSize; /* precision in bytes * channel count */
}
PaAudioIOComponent;

typedef struct PaAudioIOStream
{
    PaUtilStreamRepresentation streamRepresentation;
    PaUtilCpuLoadMeasurer cpuLoadMeasurer;
    PaUtilBufferProcessor bufferProcessor;
    PaUtilThreading threading;
    PaAudioIOComponent play, record;
    bool active, stopped;
    bool neverDropInput;
    uint64_t framesProcessed;
    uint64_t eof;
    double sampleRate;
}
PaAudioIOStream;

/* see pa_hostapi.h for a list of validity guarantees made about OpenStream parameters */

static PaError OpenStream( struct PaUtilHostApiRepresentation *hostApi,
                           PaStream** s,
                           const PaStreamParameters *inputParameters,
                           const PaStreamParameters *outputParameters,
                           double sampleRate,
                           unsigned long framesPerBuffer,
                           PaStreamFlags streamFlags,
                           PaStreamCallback *streamCallback,
                           void *userData )
{
    PaError result = paNoError;
    PaAudioIOHostApiRepresentation *audioIOHostApi = (PaAudioIOHostApiRepresentation*)hostApi;
    PaAudioIOStream *stream = 0;
    unsigned long framesPerHostBuffer = framesPerBuffer; /* these may not be equivalent for all implementations */
    int inputChannelCount, outputChannelCount;
    PaSampleFormat inputSampleFormat, outputSampleFormat;
    PaSampleFormat hostInputSampleFormat, hostOutputSampleFormat;
    PaTime inputLatency = .04, outputLatency = .04;
    PaDeviceInfo *indev, *outdev;
    struct audio_info info;

    /* validate platform specific flags */
    if( (streamFlags & paPlatformSpecificFlags) != 0 )
        return paInvalidFlag; /* unexpected platform specific flag */

    if( inputParameters )
    {
        inputChannelCount = inputParameters->channelCount;
        inputSampleFormat = inputParameters->sampleFormat;

        /* unless alternate device specification is supported, reject the use of
            paUseHostApiSpecificDeviceSpecification */

        if( inputParameters->device == paUseHostApiSpecificDeviceSpecification )
            return paInvalidDevice;

        indev = hostApi->deviceInfos[ inputParameters->device ];

        /* check that input device can support inputChannelCount */
        if( inputChannelCount > indev->maxInputChannels )
            return paInvalidChannelCount;

        /* validate inputStreamInfo */
        if( inputParameters->hostApiSpecificStreamInfo )
            return paIncompatibleHostApiSpecificStreamInfo; /* this implementation doesn't use custom stream info */
    }
    else
    {
        inputChannelCount = 0;
        inputSampleFormat = hostInputSampleFormat = paInt16; /* Surpress 'uninitialised var' warnings. */
    }

    if( outputParameters )
    {
        outputChannelCount = outputParameters->channelCount;
        outputSampleFormat = outputParameters->sampleFormat;

        /* unless alternate device specification is supported, reject the use of
            paUseHostApiSpecificDeviceSpecification */

        if( outputParameters->device == paUseHostApiSpecificDeviceSpecification )
            return paInvalidDevice;

        outdev = hostApi->deviceInfos[ outputParameters->device ];

        /* check that output device can support outputChannelCount */
        if( outputChannelCount > outdev->maxOutputChannels )
            return paInvalidChannelCount;

        /* validate outputStreamInfo */
        if( outputParameters->hostApiSpecificStreamInfo )
            return paIncompatibleHostApiSpecificStreamInfo; /* this implementation doesn't use custom stream info */
    }
    else
    {
        outputChannelCount = 0;
        outputSampleFormat = hostOutputSampleFormat = paInt16; /* Surpress 'uninitialized var' warnings. */
    }

    PA_UNLESS(stream = calloc( 1, sizeof(PaAudioIOStream) ), paInsufficientMemory);

    stream->play.fd = stream->record.fd = -1;
    stream->stopped = true;
    stream->neverDropInput = streamFlags & paNeverDropInput;

    if( inputChannelCount > 0 && outputChannelCount > 0)
    {
        if( inputParameters->device == outputParameters->device )
        {
            PA_DEBUG(("PaAudioIO %s: Opening device %s in full duplex\n", __FUNCTION__, indev->name));
            PA_UNLESS( (stream->play.fd = open(indev->name, O_RDWR)) != -1, paDeviceUnavailable );
            stream->record.fd = stream->play.fd;
        }
    }

    if( inputChannelCount > 0 )
    {
        if( stream->record.fd == -1 )
            PA_UNLESS((stream->record.fd = open(indev->name, O_RDONLY)) != -1, paDeviceUnavailable);

        memcpy(stream->record.name, indev->name, strlen(indev->name));

        hostInputSampleFormat =
            PaUtil_SelectClosestAvailableFormat( GetSupportedEncodings(stream->record.fd, true), inputSampleFormat );

        AUDIO_INITINFO(&info);
        if( PaFormatToAudioIOFormat(hostInputSampleFormat, &info.record ) != paNoError ||
            ioctl(stream->record.fd, AUDIO_SETINFO, &info) < 0 )
        {
            result = paSampleFormatNotSupported;
            goto error;
        }

        AUDIO_INITINFO(&info);
        info.record.sample_rate = sampleRate;
        PA_UNLESS( ioctl(stream->record.fd, AUDIO_SETINFO, &info) != -1, paInvalidSampleRate );

        AUDIO_INITINFO(&info);
        info.record.channels = inputChannelCount;
        PA_UNLESS( ioctl(stream->record.fd, AUDIO_SETINFO, &info) != -1, paInvalidChannelCount );

        PA_UNLESS( ioctl(stream->record.fd, AUDIO_GETINFO, &info) != -1, paDeviceUnavailable );

        PA_DEBUG(("PaAudioIO %s: %u-bit %u-channel recording stream at %u Hz\n", __FUNCTION__,
            info.record.precision, info.record.channels, info.record.sample_rate));

        sampleRate = info.record.sample_rate;

        stream->record.channels = inputChannelCount = info.record.channels;
        stream->record.frameSize = (info.record.precision / 8) * info.record.channels;
        stream->record.bufferSize = framesPerBuffer;

#ifndef __sun
        inputLatency = (info.blocksize / stream->record.frameSize) +
            PaUtil_GetBufferProcessorInputLatencyFrames( &stream->bufferProcessor ) / sampleRate;
#else
        inputLatency += PaUtil_GetBufferProcessorInputLatencyFrames( &stream->bufferProcessor ) / sampleRate;
#endif

        if( framesPerBuffer == paFramesPerBufferUnspecified )
        {
#ifndef __sun
            stream->record.bufferSize = info.blocksize / stream->record.frameSize;
#else
            stream->record.bufferSize = AUDIOIO_DEFAULT_FRAMES;
#endif
            framesPerBuffer = framesPerHostBuffer = stream->record.bufferSize;
        }

        PA_UNLESS(stream->record.buffer = calloc(stream->record.bufferSize, stream->record.frameSize), paInsufficientMemory);
    }

    if( outputChannelCount > 0 )
    {
        if( stream->play.fd == -1 )
            PA_UNLESS((stream->play.fd = open(outdev->name, O_WRONLY)) != -1, paDeviceUnavailable);

        memcpy(stream->play.name, outdev->name, strlen(outdev->name));

        hostOutputSampleFormat =
            PaUtil_SelectClosestAvailableFormat( GetSupportedEncodings(stream->play.fd, false), outputSampleFormat );

        AUDIO_INITINFO(&info);
        if( PaFormatToAudioIOFormat(hostOutputSampleFormat, &info.play ) != paNoError ||
            ioctl(stream->play.fd, AUDIO_SETINFO, &info) < 0 )
        {
            result = paSampleFormatNotSupported;
            goto error;
        }

        AUDIO_INITINFO(&info);
        info.play.sample_rate = sampleRate;
        PA_UNLESS( ioctl(stream->play.fd, AUDIO_SETINFO, &info) != -1, paInvalidSampleRate );

        AUDIO_INITINFO(&info);
        info.play.channels = outputChannelCount;
        PA_UNLESS( ioctl(stream->play.fd, AUDIO_SETINFO, &info) != -1, paInvalidChannelCount );

        PA_UNLESS( ioctl(stream->play.fd, AUDIO_GETINFO, &info) != -1, paDeviceUnavailable );

        PA_DEBUG(("PaAudioIO %s: %u-bit %u-channel playback stream at %u Hz\n", __FUNCTION__,
            info.play.precision, info.play.channels, info.play.sample_rate));

        sampleRate = info.play.sample_rate;

        stream->play.channels = outputChannelCount = info.play.channels;
        stream->play.frameSize = (info.play.precision / 8) * info.play.channels;
        stream->play.bufferSize = framesPerBuffer;

#ifndef __sun
        outputLatency = (info.blocksize / stream->play.frameSize) +
            PaUtil_GetBufferProcessorOutputLatencyFrames( &stream->bufferProcessor ) / sampleRate;
#else
        outputLatency += PaUtil_GetBufferProcessorInputLatencyFrames( &stream->bufferProcessor ) / sampleRate;
#endif

        if( framesPerBuffer == paFramesPerBufferUnspecified )
        {
#ifndef __sun
            stream->play.bufferSize = info.blocksize / stream->play.frameSize;
#else
            stream->play.bufferSize = AUDIOIO_DEFAULT_FRAMES;
#endif
            framesPerBuffer = framesPerHostBuffer = stream->play.bufferSize;
        }

        PA_UNLESS( stream->play.buffer = calloc(stream->play.bufferSize, stream->play.frameSize), paInsufficientMemory );
    }

    stream->sampleRate = sampleRate;

    if( streamCallback )
    {
        PaUtil_InitializeStreamRepresentation( &stream->streamRepresentation,
                                               &audioIOHostApi->callbackStreamInterface, streamCallback, userData );
    }
    else
    {
        PaUtil_InitializeStreamRepresentation( &stream->streamRepresentation,
                                               &audioIOHostApi->blockingStreamInterface, streamCallback, userData );
    }

    PA_ENSURE( PaUtil_InitializeThreading( &stream->threading ) );

    PaUtil_InitializeCpuLoadMeasurer( &stream->cpuLoadMeasurer, sampleRate );

    /* we assume a fixed host buffer size in this example, but the buffer processor
        can also support bounded and unknown host buffer sizes by passing
        paUtilBoundedHostBufferSize or paUtilUnknownHostBufferSize instead of
        paUtilFixedHostBufferSize below. */

    result =  PaUtil_InitializeBufferProcessor( &stream->bufferProcessor,
              inputChannelCount, inputSampleFormat, hostInputSampleFormat,
              outputChannelCount, outputSampleFormat, hostOutputSampleFormat,
              sampleRate, streamFlags, framesPerBuffer,
              framesPerHostBuffer, paUtilFixedHostBufferSize,
              streamCallback, userData );
    if( result != paNoError )
        goto error;

    stream->streamRepresentation.streamInfo.inputLatency = inputLatency;
    stream->streamRepresentation.streamInfo.outputLatency = outputLatency;
    stream->streamRepresentation.streamInfo.sampleRate = sampleRate;

    *s = (PaStream*)stream;

    return result;

error:
    if( stream != NULL )
    {
        if( stream->play.fd != -1)
            close( stream->play.fd );
        if( stream->record.fd != -1 && stream->record.fd != stream->play.fd )
            close( stream->record.fd );
        free( stream->play.buffer );
        free( stream->record.buffer );
        free( stream );
    }
    return result;
}

static void *PaAudioIO_AudioThreadProc( void *userData )
{
    PaError result = paNoError;
    PaAudioIOStream *stream = (PaAudioIOStream*)userData;
    PaStreamCallbackTimeInfo timeInfo = {0,0,0};
    int callbackResult;
    struct audio_info info;
    unsigned int pframes, rframes;
    size_t off;

    while(!stream->stopped)
    {
        pframes = rframes = 0;

        if( stream->play.channels > 0 )
            pframes = stream->play.bufferSize;

        if( stream->record.channels > 0 )
            rframes = stream->record.bufferSize;

        if( rframes > 0 )
        {
            ssize_t len = rframes * stream->record.frameSize;
            PA_ENSURE( (len = read(stream->record.fd, stream->record.buffer, len)) != -1 );
            rframes = len / stream->record.frameSize;
            if( pframes > rframes )
                pframes = rframes;
        }

        PaUtil_BeginCpuLoadMeasurement( &stream->cpuLoadMeasurer );
        PaUtil_BeginBufferProcessing( &stream->bufferProcessor, &timeInfo, 0 );

        if( rframes > 0 )
        {
            PaUtil_SetInputFrameCount( &stream->bufferProcessor, rframes );
            PaUtil_SetInterleavedInputChannels( &stream->bufferProcessor, 0, stream->record.buffer, stream->record.channels );
        }
        else if ( stream->record.channels > 0 )
        {
            PaUtil_SetNoInput( &stream->bufferProcessor );
        }

        if( pframes > 0 )
        {
            PaUtil_SetOutputFrameCount( &stream->bufferProcessor, pframes );
            PaUtil_SetInterleavedOutputChannels( &stream->bufferProcessor, 0, stream->play.buffer, stream->play.channels );
        }
        else if ( stream->play.channels > 0 && stream->neverDropInput )
        {
            PaUtil_SetNoOutput( &stream->bufferProcessor );
        }

        callbackResult = paContinue;
        pframes = PaUtil_EndBufferProcessing( &stream->bufferProcessor, &callbackResult );
        PaUtil_EndCpuLoadMeasurement( &stream->cpuLoadMeasurer, pframes );

        stream->framesProcessed += pframes;

        off = 0;

        while( stream->play.channels > 0 && pframes > 0 )
        {
            ssize_t len = pframes * stream->play.frameSize;
            PA_ENSURE( (len = write(stream->play.fd, stream->play.buffer + off, len)) != -1 );
            pframes -= (len / stream->play.frameSize);
            off += len;
        }

        if( callbackResult != paContinue )
            break;
    }

    if( callbackResult == paAbort )
    {
        /* once finished, call the finished callback */
        if( stream->streamRepresentation.streamFinishedCallback != 0 )
            stream->streamRepresentation.streamFinishedCallback( stream->streamRepresentation.userData );
    }
    else
    {
        /* once finished, call the finished callback */
        if( stream->streamRepresentation.streamFinishedCallback != 0 )
            stream->streamRepresentation.streamFinishedCallback( stream->streamRepresentation.userData );
    }

error:
    PaUtil_ResetCpuLoadMeasurer( &stream->cpuLoadMeasurer );
    stream->active = false;
    PA_DEBUG(("PaAudioIO %s: Thread exited\n", __FUNCTION__));
    pthread_exit( NULL );
}

/*
    When CloseStream() is called, the multi-api layer ensures that
    the stream has already been stopped or aborted.
*/
static PaError CloseStream( PaStream* s )
{
    PaError result = paNoError;
    PaAudioIOStream *stream = (PaAudioIOStream*)s;

    PaUtil_TerminateThreading ( &stream->threading );
    PaUtil_TerminateBufferProcessor( &stream->bufferProcessor );
    PaUtil_TerminateStreamRepresentation( &stream->streamRepresentation );

    if( stream->play.fd != -1 )
        close(stream->play.fd);

    if( stream->record.fd != -1 && stream->record.fd != stream->play.fd )
        close(stream->record.fd);

    free( stream->play.buffer );
    free( stream->record.buffer );
    free( stream );

    return result;
}


static PaError StartStream( PaStream *s )
{
    PaError result = paNoError;
    PaAudioIOStream *stream = (PaAudioIOStream*)s;

    PaUtil_ResetBufferProcessor( &stream->bufferProcessor );

    stream->active = true;
    stream->stopped = false;

    if( stream->bufferProcessor.streamCallback )
        PA_ENSURE( PaUtil_StartThreading( &stream->threading, &PaAudioIO_AudioThreadProc, stream ) );

error:
    return result;
}


static PaError StopStream( PaStream *s )
{
    PaError result = paNoError;
    PaAudioIOStream *stream = (PaAudioIOStream*)s;

    stream->stopped = true;

    if( stream->bufferProcessor.streamCallback )
        PA_ENSURE( PaUtil_CancelThreading( &stream->threading, 1, NULL ) );

    if( stream->play.fd != -1 )
        (void)ioctl(stream->play.fd, AUDIO_DRAIN);

error:
    return result;
}

static PaError AbortStream( PaStream *s )
{
    StopStream(s);
}

static PaError IsStreamStopped( PaStream *s )
{
    PaAudioIOStream *stream = (PaAudioIOStream*)s;

    return stream->stopped;
}


static PaError IsStreamActive( PaStream *s )
{
    PaAudioIOStream *stream = (PaAudioIOStream*)s;

    return stream->active;
}


static PaTime GetStreamTime( PaStream *s )
{
    PaAudioIOStream *stream = (PaAudioIOStream*)s;

    return stream->framesProcessed / stream->sampleRate;
}


static double GetStreamCpuLoad( PaStream* s )
{
    PaAudioIOStream *stream = (PaAudioIOStream*)s;

    return PaUtil_GetCpuLoad( &stream->cpuLoadMeasurer );
}


/*
    As separate stream interfaces are used for blocking and callback
    streams, the following functions can be guaranteed to only be called
    for blocking streams.
*/

static PaError ReadStream( PaStream* s,
                           void *buffer,
                           unsigned long frames )
{
    PaAudioIOStream *stream = (PaAudioIOStream*)s;
    void *chanbufs = buffer;
    unsigned wantframes, wantbytes;

    /* If user output is non-interleaved, PaUtil_CopyInput will manipulate the channel pointers,
     * so we copy the user provided pointers */
    if( !stream->bufferProcessor.userInputIsInterleaved )
    {
        /* Copy channels into local array */
        chanbufs = stream->play.chanbufs;
        memcpy( chanbufs, buffer, sizeof (void *) * stream->record.channels );
    }

    while( frames )
    {
        wantframes = PA_MIN( frames, stream->record.bufferSize );
        wantbytes = wantframes * stream->record.frameSize;

        if ( read(stream->record.fd, stream->record.buffer, wantbytes) < wantbytes )
            return paUnanticipatedHostError;

        PaUtil_SetInputFrameCount( &stream->bufferProcessor, stream->record.bufferSize );
        PaUtil_SetInterleavedInputChannels( &stream->bufferProcessor, 0, stream->record.buffer, stream->record.channels );
        PaUtil_CopyInput( &stream->bufferProcessor, &chanbufs, wantframes );

        frames -= wantframes;
    }

    return paNoError;
}


static PaError WriteStream( PaStream* s,
                            const void *buffer,
                            unsigned long frames )
{
    PaAudioIOStream *stream = (PaAudioIOStream*)s;
    const void *chanbufs = buffer;
    unsigned nbytes;
    unsigned long gotframes;

    /* If user output is non-interleaved, PaUtil_CopyOutput will manipulate the channel pointers,
     * so we copy the user provided pointers */
    if( !stream->bufferProcessor.userOutputIsInterleaved )
    {
        /* Copy channels into local array */
        chanbufs = stream->play.chanbufs;
        memcpy( (void *)chanbufs, buffer, sizeof (void *) * stream->play.channels );
    }

    while( frames )
    {
        PaUtil_SetOutputFrameCount( &stream->bufferProcessor, stream->play.bufferSize );
        PaUtil_SetInterleavedOutputChannels( &stream->bufferProcessor, 0, stream->play.buffer, stream->play.channels );

        gotframes = PaUtil_CopyOutput( &stream->bufferProcessor, &chanbufs, frames );

        nbytes = gotframes * stream->play.frameSize;

        if( write(stream->play.fd, stream->play.buffer, nbytes) < nbytes )
            return paUnanticipatedHostError;

        frames -= gotframes;
    }
    if( write(stream->play.fd, stream->play.buffer, 0) < 0 )
        return paUnanticipatedHostError;
    stream->eof++;
    return paNoError;
}


static signed long GetStreamReadAvailable( PaStream* s )
{
    PaAudioIOStream *stream = (PaAudioIOStream*)s;
#ifndef __sun
    struct audio_info info;

    if( ioctl(stream->record.fd, AUDIO_GETBUFINFO, &info) == -1 )
        return paDeviceUnavailable;

    return info.record.seek / stream->record.frameSize;
#else
    struct pollfd pfd[1];

    memset(pfd, 0, sizeof(pfd));
    pfd[0].fd = stream->record.fd;
    pfd[0].events = POLLIN;

    if( poll(pfd, 1, 0) == -1 )
        return paDeviceUnavailable;

    return (pfd[0].revents & POLLIN) ? AUDIOIO_DEFAULT_FRAMES : 0;
#endif
}


static signed long GetStreamWriteAvailable( PaStream* s )
{
    PaAudioIOStream *stream = (PaAudioIOStream*)s;
    struct audio_info info;

#ifndef __sun
    if( ioctl(stream->play.fd, AUDIO_GETBUFINFO, &info) == -1 )
        return paDeviceUnavailable;

    return ((info.hiwat * info.blocksize) - info.play.seek) / stream->play.frameSize;
#else
    if( ioctl(stream->play.fd, AUDIO_GETINFO, &info) == -1 )
        return paDeviceUnavailable;

    if( (stream->eof - info.play.eof) > 2 )
        return 0;

    return AUDIOIO_DEFAULT_FRAMES;
#endif
}

static PaError PaFormatToAudioIOFormat( PaSampleFormat fmt, struct audio_prinfo *info )
{
    PaError err = paNoError;

    switch( fmt )
    {
        case paInt32:
            info->encoding = AUDIO_ENCODING_SLINEAR;
            info->precision = 32;
            break;
        case paInt16:
            info->encoding = AUDIO_ENCODING_SLINEAR;
            info->precision = 16;
            break;
        case paInt8:
            info->encoding = AUDIO_ENCODING_SLINEAR;
            info->precision = 8;
            break;
        default:
            err = paSampleFormatNotSupported;
            break;
    }
    return err;
}

static bool AttemptEncoding( int fd, int encoding, int precision, bool record )
{
    struct audio_info info;
    struct audio_prinfo *prinfo = record ? &info.record : &info.play;

    AUDIO_INITINFO(&info);
    prinfo->encoding = encoding;
    prinfo->precision = precision;

    return ioctl(fd, AUDIO_SETINFO, &info) != -1;
}

static PaSampleFormat GetSupportedEncodings( int fd, bool record )
{
    PaSampleFormat fmts = 0;

    if( AttemptEncoding(fd, AUDIO_ENCODING_SLINEAR, 32, record) )
        fmts |= paInt32;

    if( AttemptEncoding(fd, AUDIO_ENCODING_SLINEAR, 16, record) )
        fmts |= paInt16;

    if( AttemptEncoding(fd, AUDIO_ENCODING_SLINEAR, 8, record) )
        fmts |= paInt8;

    return fmts;
}
