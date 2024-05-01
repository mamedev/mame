
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
 it has callbackmode and normal write mode support
*/

#include "pa_linux_pulseaudio_block_internal.h"
#include <unistd.h>

/*
    As separate stream interfaces are used for blocking and callback
    streams, the following functions can be guaranteed to only be called
    for blocking streams.
*/

PaError PaPulseAudio_ReadStreamBlock( PaStream * s,
                                      void *buffer,
                                      unsigned long frames )
{
    PaPulseAudio_Stream *pulseaudioStream = (PaPulseAudio_Stream *) s;
    PaPulseAudio_HostApiRepresentation *pulseaudioHostApi =
        pulseaudioStream->hostapi;
    PaError ret = 0;
    uint8_t *readableBuffer = (uint8_t *) buffer;
    long bufferLeftToRead = (frames * pulseaudioStream->inputFrameSize);

    while( bufferLeftToRead > 0 )
    {
        PA_PULSEAUDIO_IS_ERROR( pulseaudioStream, paStreamIsStopped )

        PaPulseAudio_Lock( pulseaudioStream->mainloop );
        long l_read = PaUtil_ReadRingBuffer( &pulseaudioStream->inputRing, readableBuffer,
                                             bufferLeftToRead );
        readableBuffer += l_read;
        bufferLeftToRead -= l_read;
        if( bufferLeftToRead > 0 )
            pa_threaded_mainloop_wait( pulseaudioStream->mainloop );

        PaPulseAudio_UnLock( pulseaudioStream->mainloop );

        if( bufferLeftToRead > 0 )
        {
            /* Sleep small amount of time not burn CPU
            * we block anyway so this is bearable
            */
            usleep(100);
        }
    }
    return paNoError;
}


PaError PaPulseAudio_WriteStreamBlock( PaStream * s,
                                       const void *buffer,
                                       unsigned long frames )
{
    PaPulseAudio_Stream *pulseaudioStream = (PaPulseAudio_Stream *) s;
    int ret = 0;
    size_t pulseaudioWritable = 0;
    uint8_t *writableBuffer = (uint8_t *) buffer;
    long bufferLeftToWrite = (frames * pulseaudioStream->outputFrameSize);
    pa_operation *pulseaudioOperation = NULL;

    PaUtil_BeginCpuLoadMeasurement( &pulseaudioStream->cpuLoadMeasurer );

    while( bufferLeftToWrite > 0)
    {
        PA_PULSEAUDIO_IS_ERROR( pulseaudioStream, paStreamIsStopped )

        PaPulseAudio_Lock( pulseaudioStream->mainloop );
        pulseaudioWritable = pa_stream_writable_size( pulseaudioStream->outputStream );
        PaPulseAudio_UnLock( pulseaudioStream->mainloop );

        if( pulseaudioWritable > 0 )
        {
            if( bufferLeftToWrite < pulseaudioWritable )
            {
                pulseaudioWritable = bufferLeftToWrite;
            }
            PaPulseAudio_Lock( pulseaudioStream->mainloop );
            ret = pa_stream_write( pulseaudioStream->outputStream,
                                     writableBuffer,
                                     pulseaudioWritable,
                                     NULL,
                                     0,
                                     PA_SEEK_RELATIVE );

            pulseaudioOperation = pa_stream_update_timing_info( pulseaudioStream->outputStream,
                                                                NULL,
                                                                NULL );
            PaPulseAudio_UnLock( pulseaudioStream->mainloop );

            ret = 0;

            if( pulseaudioOperation == NULL )
            {
                return paInsufficientMemory;
            }

            while( pa_operation_get_state( pulseaudioOperation ) == PA_OPERATION_RUNNING )
            {
                ret ++;
                PA_PULSEAUDIO_IS_ERROR( pulseaudioStream, paStreamIsStopped )

                /* As this shouldn never happen it's error if it does */
                if( ret >= 10000 )
                {
                    return paStreamIsStopped;
                }

                usleep(100);
            }

            PaPulseAudio_Lock( pulseaudioStream->mainloop );

            pa_operation_unref( pulseaudioOperation );
            pulseaudioOperation = NULL;

            PaPulseAudio_UnLock( pulseaudioStream->mainloop );

            writableBuffer += pulseaudioWritable;
            bufferLeftToWrite -= pulseaudioWritable;
        }

        if( bufferLeftToWrite > 0 )
        {
            /* Sleep small amount of time not burn CPU
            * we block anyway so this is bearable
            */
            usleep(100);
        }

    }
    PaUtil_EndCpuLoadMeasurement( &pulseaudioStream->cpuLoadMeasurer,
                                  frames );

    return paNoError;
}


signed long PaPulseAudio_GetStreamReadAvailableBlock( PaStream * s )
{
    PaPulseAudio_Stream *pulseaudioStream = (PaPulseAudio_Stream *) s;

    if( pulseaudioStream->inputStream == NULL )
    {
        return 0;
    }

    return ( PaUtil_GetRingBufferReadAvailable( &pulseaudioStream->inputRing ) /
             pulseaudioStream->inputFrameSize );
}
