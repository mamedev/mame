
/*
 * PulseAudio host to play natively in Linux based systems without
 * ALSA emulation
 *
 * Copyright (c) 2014-2023 Tuukka Pasanen <tuukka.pasanen@ilmi.fi>
 * Copyright (c) 2016 Sqweek
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
 @ingroup common_src

 @brief PulseAudio implementation of support for a host API.

 This host API implements PulseAudio support for portaudio
 it has callback mode and normal write mode support
*/


#include "pa_util.h"
#include "pa_allocation.h"
#include "pa_hostapi.h"
#include "pa_stream.h"
#include "pa_cpuload.h"
#include "pa_process.h"

#include "pa_unix_util.h"
#include "pa_ringbuffer.h"

#include "pa_linux_pulseaudio_cb_internal.h"


/* PulseAudio headers */
#include <string.h>
#include <unistd.h>

int PaPulseAudio_updateTimeInfo( pa_stream * s,
                                 PaStreamCallbackTimeInfo *timeInfo,
                                 int record )
{
    unsigned int isNegative = 0;
    pa_usec_t pulseaudioStreamTime = 0;
    pa_usec_t pulseaudioStreamLatency = 0;

    if( pa_stream_get_time( s,
                            &pulseaudioStreamTime ) == -PA_ERR_NODATA )
    {
        return -PA_ERR_NODATA;
    }
    else
    {
    timeInfo->currentTime = ((PaTime) pulseaudioStreamTime / (PaTime) 1000000);
    }

    if( pa_stream_get_latency( s,
                               &pulseaudioStreamLatency,
                               &isNegative ) == -PA_ERR_NODATA )
    {
        return -PA_ERR_NODATA;
    }
    else
    {
        if( record == 0 )
        {
            timeInfo->outputBufferDacTime = timeInfo->currentTime + ((PaTime) pulseaudioStreamLatency / (PaTime) 1000000);
        }
        else
        {
            timeInfo->inputBufferAdcTime = timeInfo->currentTime - ((PaTime) pulseaudioStreamLatency / (PaTime) 1000000);
        }
    }
    return 0;
}


/* locks the Pulse Main loop when not called from it */
void PaPulseAudio_Lock( pa_threaded_mainloop *mainloop )
{
    if( !pa_threaded_mainloop_in_thread( mainloop ) ) {
        pa_threaded_mainloop_lock( mainloop );
    }
}

/* unlocks the Pulse Main loop when not called from it */
void PaPulseAudio_UnLock( pa_threaded_mainloop *mainloop )
{
    if( !pa_threaded_mainloop_in_thread( mainloop ) ) {
        pa_threaded_mainloop_unlock( mainloop );
    }
}

void _PaPulseAudio_WriteRingBuffer( PaUtilRingBuffer *ringbuffer,
                                    const void *buffer,
                                    size_t length )
{
    /*
     * If there is not enough room. Read from ringbuffer to make
     * sure that if not full and audio will just underrun
     *
     * If you try to read too much and there is no room then this
     * will fail. But I don't know how to get into that?
     */
    if( PaUtil_GetRingBufferWriteAvailable( ringbuffer ) < length )
    {
        uint8_t tmpBuffer[ PULSEAUDIO_BUFFER_SIZE ];
        PaUtil_ReadRingBuffer( ringbuffer,
                               tmpBuffer,
                               length );
    }

    PaUtil_WriteRingBuffer( ringbuffer,
                            buffer,
                            length );

}

void _PaPulseAudio_Read( PaPulseAudio_Stream *stream,
                         size_t length )
{
    const void *pulseaudioData = NULL;

    /*
     * Idea behind this is that we fill ringbuffer with data
     * that comes from input device. When it's available
     * we'll fill it to callback or blocking reading
     */
    if( pa_stream_peek( stream->inputStream,
                        &pulseaudioData,
                        &length ))
    {
        PA_DEBUG( ("Portaudio %s: Can't read audio!\n",
                  __FUNCTION__) );
    }
    else
    {
        _PaPulseAudio_WriteRingBuffer( &stream->inputRing, pulseaudioData, length );
    }

    pa_stream_drop( stream->inputStream );

    pulseaudioData = NULL;

}

static int _PaPulseAudio_ProcessAudio(PaPulseAudio_Stream *stream,
                                      size_t length)
{
    uint8_t pulseaudioSampleBuffer[PULSEAUDIO_BUFFER_SIZE];
    size_t hostFramesPerBuffer = stream->bufferProcessor.framesPerHostBuffer;
    size_t pulseaudioOutputBytes = 0;
    size_t pulseaudioInputBytes = 0;
    size_t hostFrameCount = 0;
    int isOutputCb = 0;
    int isInputCb = 0;
    PaStreamCallbackTimeInfo timeInfo;
    int ret = paContinue;
    void *bufferData = NULL;
    size_t pulseaudioOutputWritten = 0;

    /* If there is no specified per host buffer then
     * just generate one or but correct one in place
     */
    if( hostFramesPerBuffer == paFramesPerBufferUnspecified )
    {
        if( !stream->framesPerHostCallback )
        {
            /* This just good enough and most
             * Pulseaudio server and ALSA can handle it
             *
             * We should never get here but this is ultimate
             * backup.
             */
            hostFramesPerBuffer = PAPULSEAUDIO_FRAMESPERBUFFERUNSPEC;

            stream->framesPerHostCallback = hostFramesPerBuffer;
        }
        else
        {
            hostFramesPerBuffer = stream->framesPerHostCallback;
        }
    }

    if( stream->outputStream )
    {
        /* Calculate how many bytes goes to one frame */
        pulseaudioInputBytes = pulseaudioOutputBytes = (hostFramesPerBuffer * stream->outputFrameSize);

        if( stream->bufferProcessor.streamCallback )
        {
            isOutputCb = 1;
        }
    }

    /* If we just want to have input but not output (Not Duplex)
     * Use this calculation
     */
    if( stream->inputStream )
    {
        pulseaudioInputBytes = pulseaudioOutputBytes = (hostFramesPerBuffer * stream->inputFrameSize);

        if( stream->bufferProcessor.streamCallback )
        {
            isInputCb = 1;
        }
    }

    /*
     * When stopped we should stop feeding or recording right away
     */
    if( stream->isStopped )
    {
        return paStreamIsStopped;
    }

    /*
     * This can be called before we have reached out
     * starting Portaudio stream or Portaudio stream
     * is stopped
     */
    if( !stream->pulseaudioIsActive )
    {
        if(stream->outputStream)
        {
            bufferData = pulseaudioSampleBuffer;
            memset( bufferData, 0x00, length);

            pa_stream_write( stream->outputStream,
                             bufferData,
                             length,
                             NULL,
                             0,
                             PA_SEEK_RELATIVE );
        }

        return paContinue;
    }

    /* If we have input which is mono and
     * output which is stereo. We have to copy
     * mono to monomono which is stereo.
     * Then just read half and copy
     */
    if( isOutputCb &&
        stream->outputSampleSpec.channels == 2 &&
        stream->inputSampleSpec.channels == 1)
    {
        pulseaudioInputBytes /= 2;
    }

    while(1)
    {
        /*
         * If everything fail like stream vanish or mainloop
         * is in error state try to handle error
         */
        PA_PULSEAUDIO_IS_ERROR( stream, paStreamIsStopped )

        /* There is only Record stream so
         * see if we have enough stuff to feed record stream
         * If not then bail out.
         */
        if( isInputCb &&
            PaUtil_GetRingBufferReadAvailable(&stream->inputRing) < pulseaudioInputBytes )
        {
            if(isOutputCb && (pulseaudioOutputWritten < length) && !stream->missedBytes)
            {
                stream->missedBytes = length - pulseaudioOutputWritten;
            }
            else
            {
                stream->missedBytes = 0;
            }
            break;
        }
        else if( pulseaudioOutputWritten >= length)
        {
            stream->missedBytes = 0;
            break;
        }

        if(  stream->outputStream )
        {
            PaPulseAudio_updateTimeInfo( stream->outputStream,
                                         &timeInfo,
                                         0 );
        }

        if(  stream->inputStream )
        {
            PaPulseAudio_updateTimeInfo( stream->inputStream,
                                         &timeInfo,
                                         1 );
        }

        PaUtil_BeginCpuLoadMeasurement( &stream->cpuLoadMeasurer );

        /* When doing Portaudio Duplex one has to write and read same amount of data
         * if not done that way Portaudio will go boo boo and nothing works.
         * This is why this is done as it's done
         *
         * Pulseaudio does not care and this is something that needs small adjusting
         */
        PaUtil_BeginBufferProcessing( &stream->bufferProcessor,
                                      &timeInfo,
                                      0 );

        /* Read of ther is something to read */
        if( isInputCb )
        {
            PaUtil_ReadRingBuffer( &stream->inputRing,
                                   pulseaudioSampleBuffer,
                                   pulseaudioInputBytes);

            PaUtil_SetInterleavedInputChannels( &stream->bufferProcessor,
                                                0,
                                                pulseaudioSampleBuffer,
                                                stream->inputSampleSpec.channels );

            PaUtil_SetInputFrameCount( &stream->bufferProcessor,
                                       hostFramesPerBuffer );
        }

        if( isOutputCb )
        {

            size_t tmpSize = pulseaudioOutputBytes;

            /* Allocate memory to make it faster to output stuff */
            pa_stream_begin_write( stream->outputStream, &bufferData, &tmpSize );

            /* If bufferData is NULL then output is not ready
             * and we have to wait for it
             */
            if(!bufferData)
            {
                return paNotInitialized;
            }

            PaUtil_SetInterleavedOutputChannels( &stream->bufferProcessor,
                                                 0,
                                                 bufferData,
                                                 stream->outputChannelCount );

            PaUtil_SetOutputFrameCount( &stream->bufferProcessor,
                                        hostFramesPerBuffer );

            if( pa_stream_write( stream->outputStream,
                                 bufferData,
                                 pulseaudioOutputBytes,
                                 NULL,
                                 0,
                                 PA_SEEK_RELATIVE ) )
            {
                PA_DEBUG( ("Portaudio %s: Can't write audio!\n",
                          __FUNCTION__) );
            }

            pulseaudioOutputWritten += pulseaudioOutputBytes;
        }

        hostFrameCount =
            PaUtil_EndBufferProcessing( &stream->bufferProcessor,
                                        &ret );

        PaUtil_EndCpuLoadMeasurement( &stream->cpuLoadMeasurer,
                                      hostFrameCount );
    }

    return ret;
}

void PaPulseAudio_StreamRecordCb( pa_stream * s,
                                  size_t length,
                                  void *userdata )
{
    PaPulseAudio_Stream *pulseaudioStream = (PaPulseAudio_Stream *) userdata;

    if( !pulseaudioStream->pulseaudioIsActive )
    {
        pulseaudioStream->pulseaudioIsActive = 1;
        pulseaudioStream->pulseaudioIsStopped= 0;
    }

    _PaPulseAudio_Read( pulseaudioStream, length );

    /* Let's handle when output happens if Duplex
     *
     * Also there is no callback there is no meaning to continue
     * as we have blocking reading
     */
    if( pulseaudioStream->bufferProcessor.streamCallback )
    {
        _PaPulseAudio_ProcessAudio( pulseaudioStream, length );
    }

    pa_threaded_mainloop_signal( pulseaudioStream->mainloop,
                                 0 );
}

void PaPulseAudio_StreamPlaybackCb( pa_stream * s,
                                    size_t length,
                                    void *userdata )
{
    PaPulseAudio_Stream *pulseaudioStream = (PaPulseAudio_Stream *) userdata;

    if( !pulseaudioStream->inputStream && !pulseaudioStream->pulseaudioIsActive )
    {
        pulseaudioStream->pulseaudioIsActive = 1;
        pulseaudioStream->pulseaudioIsStopped = 0;
    }

    if( pulseaudioStream->bufferProcessor.streamCallback )
    {
        _PaPulseAudio_ProcessAudio( pulseaudioStream, length );
    }

    pa_threaded_mainloop_signal( pulseaudioStream->mainloop,
                                 0 );
}

/* This is left for future use! */
static void PaPulseAudio_StreamSuccessCb( pa_stream * s,
                                          int success,
                                          void *userdata )
{
    PaPulseAudio_Stream *pulseaudioStream = (PaPulseAudio_Stream *) userdata;
    PA_DEBUG( ("Portaudio %s: %d\n", __FUNCTION__,
              success) );
    pa_threaded_mainloop_signal( pulseaudioStream->mainloop,
                                 0 );
}

/* This is left for future use! */
static void PaPulseAudio_CorkSuccessCb(
    pa_stream * s,
    int success,
    void *userdata
)
{
    PaPulseAudio_Stream *pulseaudioStream = (PaPulseAudio_Stream *) userdata;
    pa_threaded_mainloop_signal( pulseaudioStream->mainloop,
                                 0 );
}


/* This is left for future use! */
void PaPulseAudio_StreamStartedCb( pa_stream * stream,
                                   void *userdata )
{
    PaPulseAudio_Stream *pulseaudioStream = (PaPulseAudio_Stream *) userdata;
    pa_threaded_mainloop_signal( pulseaudioStream->mainloop,
                                 0 );
}

/*
    When CloseStream() is called, the multi-api layer ensures that
    the stream has already been stopped or aborted.
*/
PaError PaPulseAudio_CloseStreamCb( PaStream * s )
{
    PaError result = paNoError;
    PaPulseAudio_Stream *stream = (PaPulseAudio_Stream *) s;
    PaPulseAudio_HostApiRepresentation *pulseaudioHostApi = stream->hostapi;
    pa_operation *pulseaudioOperation = NULL;
    int waitLoop = 0;
    int pulseaudioError = 0;

    /* Wait for stream to be stopped */
    stream->isActive = 0;
    stream->isStopped = 1;
    stream->pulseaudioIsActive = 0;
    stream->pulseaudioIsStopped = 1;

    if( stream->outputStream != NULL
        && pa_stream_get_state( stream->outputStream ) == PA_STREAM_READY )
    {
        PaPulseAudio_Lock(stream->mainloop);
        /**
         * Pause stream so it stops faster
         */
        pulseaudioOperation = pa_stream_cork( stream->outputStream,
                                              1,
                                              PaPulseAudio_CorkSuccessCb,
                                              stream );

        PaPulseAudio_UnLock( stream->mainloop );

        while( pa_operation_get_state( pulseaudioOperation ) == PA_OPERATION_RUNNING )
        {
            pa_threaded_mainloop_wait( pulseaudioHostApi->mainloop );
            waitLoop ++;

            if(waitLoop > 250)
            {
                break;
            }
        }

        waitLoop = 0;

        PaPulseAudio_Lock(stream->mainloop);
        pa_operation_unref( pulseaudioOperation );
        pulseaudioOperation = NULL;

        pa_stream_disconnect( stream->outputStream );
        PaPulseAudio_UnLock( stream->mainloop );
    }

    if( stream->inputStream != NULL
        && pa_stream_get_state( stream->inputStream ) == PA_STREAM_READY )
    {
        PaPulseAudio_Lock( stream->mainloop );

        /**
         * Pause stream so it stops so it stops faster
         */
        pulseaudioOperation = pa_stream_cork( stream->inputStream,
                                              1,
                                              PaPulseAudio_CorkSuccessCb,
                                              stream );

        PaPulseAudio_UnLock( stream->mainloop );

        while( pa_operation_get_state( pulseaudioOperation ) == PA_OPERATION_RUNNING )
        {
            pa_threaded_mainloop_wait( pulseaudioHostApi->mainloop );
            waitLoop ++;

            if(waitLoop > 250)
            {
                break;
            }
        }

        waitLoop = 0;

        PaPulseAudio_Lock( stream->mainloop );
        pa_operation_unref( pulseaudioOperation );
        pulseaudioOperation = NULL;

        /* Then we disconnect stream and wait for
         * Termination
         */
        pa_stream_disconnect( stream->inputStream );

        PaPulseAudio_UnLock( stream->mainloop );

    }

    /* Wait for termination for both */
    while(!waitLoop)
    {
        PaPulseAudio_Lock( stream->mainloop );
        if( stream->inputStream != NULL
            && pa_stream_get_state( stream->inputStream ) == PA_STREAM_TERMINATED )
        {
            pa_stream_unref( stream->inputStream );
            stream->inputStream = NULL;
        }
        PaPulseAudio_UnLock( stream->mainloop );

        PaPulseAudio_Lock( stream->mainloop );
        if( stream->outputStream != NULL
            && pa_stream_get_state(stream->outputStream) == PA_STREAM_TERMINATED )
        {
            pa_stream_unref( stream->outputStream );
            stream->outputStream = NULL;
        }
        PaPulseAudio_UnLock( stream->mainloop );

        if((stream->outputStream == NULL
           && stream->inputStream == NULL)
           || pulseaudioError >= 5000 )
        {
            waitLoop = 1;
        }

        pulseaudioError ++;
        usleep(10000);
    }

    PaUtil_TerminateBufferProcessor( &stream->bufferProcessor );
    PaUtil_TerminateStreamRepresentation( &stream->streamRepresentation );

    PaUtil_FreeMemory( stream->inputStreamName );
    PaUtil_FreeMemory( stream->outputStreamName );
    PaUtil_FreeMemory( stream );

    return result;
}

PaError PaPulseAudio_StartStreamCb( PaStream * s )
{
    PaError ret = paNoError;
    PaPulseAudio_Stream *stream = (PaPulseAudio_Stream *) s;
    int pulseaudioPlaybackStarted = 0;
    int pulseaudioRecordStarted = 0;
    pa_stream_state_t pulseaudioState = PA_STREAM_UNCONNECTED;
    PaPulseAudio_HostApiRepresentation *pulseaudioHostApi = stream->hostapi;
    const char *pulseaudioName = NULL;
    pa_operation *pulseaudioOperation = NULL;
    int waitLoop = 0;
    unsigned int pulseaudioReqFrameSize = (1024 * 2);

    stream->isActive = 0;
    stream->isStopped = 1;
    stream->pulseaudioIsActive = 0;
    stream->pulseaudioIsStopped = 1;
    stream->missedBytes = 0;

    /* Ready the processor */
    PaUtil_ResetBufferProcessor( &stream->bufferProcessor );

    PaPulseAudio_Lock( pulseaudioHostApi->mainloop );
    /* Adjust latencies if that is wanted
     * https://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/Developer/Clients/LatencyControl/
     *
     * tlength is for Playback
     * fragsize if for Record
     */
    stream->outputBufferAttr.maxlength = (uint32_t)-1;
    stream->inputBufferAttr.maxlength = (uint32_t)-1;

    stream->outputBufferAttr.tlength = (uint32_t)-1;
    stream->inputBufferAttr.tlength = (uint32_t)-1;

    stream->outputBufferAttr.fragsize = (uint32_t)-1;
    stream->inputBufferAttr.fragsize = (uint32_t)-1;

    stream->outputBufferAttr.prebuf = (uint32_t)-1;
    stream->inputBufferAttr.prebuf = (uint32_t)-1;

    stream->outputBufferAttr.minreq = (uint32_t)-1;
    stream->inputBufferAttr.minreq = (uint32_t)-1;

    stream->outputUnderflows = 0;
    PaPulseAudio_UnLock( pulseaudioHostApi->mainloop );

    pa_stream_flags_t pulseaudioStreamFlags = PA_STREAM_INTERPOLATE_TIMING |
                                 PA_STREAM_AUTO_TIMING_UPDATE |
                                 PA_STREAM_ADJUST_LATENCY |
                                 PA_STREAM_NO_REMIX_CHANNELS |
                                 PA_STREAM_NO_REMAP_CHANNELS |
                                 PA_STREAM_DONT_MOVE;

    if( stream->inputStream )
    {
        /* Default input reads 65,535 bytes setting fragsize
         * fragments request to smaller chunks of data so it's
         * easier to get nicer looking timestamps with current
         * callback system
         */
        stream->inputBufferAttr.fragsize = pa_usec_to_bytes( pulseaudioReqFrameSize,
                                                             &stream->inputSampleSpec );

        if( stream->inputDevice != paNoDevice)
        {
            PA_DEBUG( ("Portaudio %s: %d (%s)\n", __FUNCTION__, stream->inputDevice,
                      pulseaudioHostApi->pulseaudioDeviceNames[stream->
                                                                    inputDevice]) );
        }

        pa_stream_set_read_callback( stream->inputStream,
                                     PaPulseAudio_StreamRecordCb,
                                     stream );

        PaDeviceIndex defaultInputDevice;
        PaError result = PaUtil_DeviceIndexToHostApiDeviceIndex(
                &defaultInputDevice,
                pulseaudioHostApi->inheritedHostApiRep.info.defaultInputDevice,
                &(pulseaudioHostApi->inheritedHostApiRep) );

        /* NULL means default device */
        pulseaudioName = NULL;

        /* If default device is not requested then change to wanted device */
        if( result == paNoError && stream->inputDevice != defaultInputDevice )
        {
            pulseaudioName = pulseaudioHostApi->
                        pulseaudioDeviceNames[stream->inputDevice];
        }

        if ( result == paNoError )
        {
            PaPulseAudio_Lock( pulseaudioHostApi->mainloop );
            /* Zero means success */
            if( ! pa_stream_connect_record( stream->inputStream,
                                            pulseaudioName,
                                            &stream->inputBufferAttr,
                                            pulseaudioStreamFlags ) )
            {
                pa_stream_set_started_callback( stream->inputStream,
                                                PaPulseAudio_StreamStartedCb,
                                                stream );
            }
            else
            {
                PA_DEBUG( ("Portaudio %s: Can't read audio!\n",
                          __FUNCTION__) );

                goto startstreamcb_error;
            }
            PaPulseAudio_UnLock( pulseaudioHostApi->mainloop );

            for( waitLoop = 0; waitLoop < 100; waitLoop ++ )
            {
                PaPulseAudio_Lock( pulseaudioHostApi->mainloop );
                pulseaudioState = pa_stream_get_state( stream->inputStream );
                PaPulseAudio_UnLock( pulseaudioHostApi->mainloop );

                if( pulseaudioState == PA_STREAM_READY )
                {
                    break;
                }
                else if( pulseaudioState == PA_STREAM_FAILED ||
                         pulseaudioState == PA_STREAM_TERMINATED )
                {
                    goto startstreamcb_error;
                }

                usleep(10000);
            }
        }
        else
        {
            goto startstreamcb_error;
        }

    }

    if( stream->outputStream )
    {
        /* tlength does almost the same as fragsize in record.
         * See reasoning up there in comments.
         *
         * In future this should we tuned when things changed
         * this just 'good' default
         */
        stream->outputBufferAttr.tlength = pa_usec_to_bytes( pulseaudioReqFrameSize,
                                                             &stream->outputSampleSpec );

        pa_stream_set_write_callback( stream->outputStream,
                                      PaPulseAudio_StreamPlaybackCb,
                                      stream );

        /* Just keep on trucking if we are just corked */
        if( pa_stream_get_state( stream->outputStream ) == PA_STREAM_READY
            && pa_stream_is_corked( stream->outputStream ) )
        {
            PaPulseAudio_Lock( pulseaudioHostApi->mainloop );
            pulseaudioOperation = pa_stream_cork( stream->outputStream,
                                            0,
                                            PaPulseAudio_CorkSuccessCb,
                                            stream );
            PaPulseAudio_UnLock( pulseaudioHostApi->mainloop );

            while( pa_operation_get_state( pulseaudioOperation ) == PA_OPERATION_RUNNING)
            {
                pa_threaded_mainloop_wait( pulseaudioHostApi->mainloop );
            }

            pa_operation_unref( pulseaudioOperation );
            pulseaudioOperation = NULL;
        }
        else
        {
            if( stream->outputDevice != paNoDevice )
            {
                PA_DEBUG( ("Portaudio %s: %d (%s)\n",
                          __FUNCTION__,
                          stream->outputDevice,
                          pulseaudioHostApi->pulseaudioDeviceNames[stream->
                                                            outputDevice]) );
            }

            PaDeviceIndex defaultOutputDevice;
            PaError result = PaUtil_DeviceIndexToHostApiDeviceIndex( &defaultOutputDevice,
                             pulseaudioHostApi->inheritedHostApiRep.info.defaultOutputDevice,
                             &(pulseaudioHostApi->inheritedHostApiRep) );

            /* NULL means default device */
            pulseaudioName = NULL;

            /* If default device is not requested then change to wanted device */
            if( result == paNoError && stream->outputDevice != defaultOutputDevice )
            {
                pulseaudioName = pulseaudioHostApi->
                                    pulseaudioDeviceNames[stream->outputDevice];
            }

            if(result == paNoError)
            {
                PaPulseAudio_Lock( pulseaudioHostApi->mainloop );

                if ( ! pa_stream_connect_playback( stream->outputStream,
                                                   pulseaudioName,
                                                   &stream->outputBufferAttr,
                                                   pulseaudioStreamFlags,
                                                   NULL,
                                                   NULL ) )
                {
                    pa_stream_set_underflow_callback( stream->outputStream,
                                                      PaPulseAudio_StreamUnderflowCb,
                                                      stream);
                    pa_stream_set_started_callback( stream->outputStream,
                                                    PaPulseAudio_StreamStartedCb,
                                                    stream );
                }
                else
                {
                    PA_DEBUG( ("Portaudio %s: Can't write audio!\n",
                              __FUNCTION__) );
                    goto startstreamcb_error;
                }
                PaPulseAudio_UnLock( pulseaudioHostApi->mainloop );

                for( waitLoop = 0; waitLoop < 100; waitLoop ++ )
                {
                    PaPulseAudio_Lock( pulseaudioHostApi->mainloop );
                    pulseaudioState = pa_stream_get_state( stream->outputStream );
                    PaPulseAudio_UnLock( pulseaudioHostApi->mainloop );

                    if( pulseaudioState == PA_STREAM_READY )
                    {
                        break;
                    }
                    else if( pulseaudioState == PA_STREAM_FAILED  ||
                             pulseaudioState == PA_STREAM_TERMINATED )
                    {
                        goto startstreamcb_error;
                    }

                    usleep(10000);
                }

            }
            else
            {
                goto startstreamcb_error;
            }
        }
    }

    if( !stream->outputStream &&
        !stream->inputStream )
    {
        PA_DEBUG( ("Portaudio %s: Streams not initialized!\n",
                  __FUNCTION__) );
        goto startstreamcb_error;
    }

    /* Make sure we pass no error on intialize */
    ret = paNoError;

    /* Stream is now active */
    stream->isActive = 1;
    stream->isStopped = 0;

    /* Allways unlock.. so we don't get locked */
    startstreamcb_end:
    return ret;

    error:
    startstreamcb_error:
    PA_DEBUG( ("Portaudio %s: Can't start audio!\n",
              __FUNCTION__) );

    if( pulseaudioPlaybackStarted || pulseaudioRecordStarted )
    {
        PaPulseAudio_AbortStreamCb( stream );
    }

    stream->isActive = 0;
    stream->isStopped = 1;
    ret = paNotInitialized;

    goto startstreamcb_end;
}

static PaError RequestStop( PaPulseAudio_Stream * stream,
                     int abort )
{
    PaError ret = paNoError;
    PaPulseAudio_HostApiRepresentation *pulseaudioHostApi = stream->hostapi;
    pa_operation *pulseaudioOperation = NULL;

    PaPulseAudio_Lock( pulseaudioHostApi->mainloop );

    /* Wait for stream to be stopped */
    stream->isActive = 0;
    stream->isStopped = 1;
    stream->pulseaudioIsActive = 0;
    stream->pulseaudioIsStopped = 1;

    stream->missedBytes = 0;

    /* Test if there is something that we can play */
    if( stream->outputStream
        && pa_stream_get_state( stream->outputStream ) == PA_STREAM_READY
        && !pa_stream_is_corked( stream->outputStream )
        && !abort )
    {
        pulseaudioOperation = pa_stream_cork( stream->outputStream,
                                              1,
                                              PaPulseAudio_CorkSuccessCb,
                                              stream );

        while( pa_operation_get_state( pulseaudioOperation ) == PA_OPERATION_RUNNING )
        {
            pa_threaded_mainloop_wait( pulseaudioHostApi->mainloop );
        }

        pa_operation_unref( pulseaudioOperation );

        pulseaudioOperation = NULL;
    }

    requeststop_error:
    PaPulseAudio_UnLock( pulseaudioHostApi->mainloop );
    stream->isActive = 0;
    stream->isStopped = 1;
    stream->pulseaudioIsActive = 0;
    stream->pulseaudioIsStopped = 1;

    return ret;
}

PaError PaPulseAudio_StopStreamCb( PaStream * s )
{
    return RequestStop( (PaPulseAudio_Stream *) s,
                        0 );
}

PaError PaPulseAudio_AbortStreamCb( PaStream * s )
{
    return RequestStop( (PaPulseAudio_Stream *) s,
                        1 );
}
