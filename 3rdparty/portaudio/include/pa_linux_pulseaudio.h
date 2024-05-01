#ifndef PA_LINUX_PULSEAUDIO_H
#define PA_LINUX_PULSEAUDIO_H

/*
 * $Id$
 * PortAudio Portable Real-Time Audio Library
 * PulseAudio-specific extensions
 *
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

/** @file
 *  @ingroup public_header
 *  @brief PulseAudio-specific PortAudio API extension header file.
 */

#include "portaudio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Renames the PulseAudio description for the source that is opened
 * by PortAudio.
 *
 * @param s The PortAudio stream to operate on.
 * @param streamName The new name/description of the source.
 *
 * @return paNoError on success or the error encountered otherwise.
 */
PaError PaPulseAudio_RenameSource( PaStream *s, const char *streamName );

/**
 * Renames the PulseAudio description for the sink that is opened
 * by PortAudio.
 *
 * @param s The PortAudio stream to operate on.
 * @param streamName The new name/description of the sink.
 *
 * @return paNoError on success or the error encountered otherwise.
 */
PaError PaPulseAudio_RenameSink( PaStream *s, const char *streamName );

#ifdef __cplusplus
}
#endif

#endif
