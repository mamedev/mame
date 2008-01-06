/***************************************************************************

    streams.h

    Handle general purpose audio streams

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef STREAMS_H
#define STREAMS_H

#include "mamecore.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _sound_stream sound_stream;

typedef void (*stream_callback)(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void streams_init(running_machine *machine, attoseconds_t update_subseconds);
void streams_set_tag(running_machine *machine, void *streamtag);
void streams_update(running_machine *machine);

/* core stream configuration and operation */
sound_stream *stream_create(int inputs, int outputs, int sample_rate, void *param, stream_callback callback);
void stream_set_input(sound_stream *stream, int index, sound_stream *input_stream, int output_index, float gain);
void stream_update(sound_stream *stream);
const stream_sample_t *stream_get_output_since_last_update(sound_stream *stream, int outputnum, int *numsamples);

/* utilities for accessing a particular stream */
sound_stream *stream_find_by_tag(void *streamtag, int streamindex);
int stream_get_inputs(sound_stream *stream);
int stream_get_outputs(sound_stream *stream);
void stream_set_input_gain(sound_stream *stream, int input, float gain);
void stream_set_output_gain(sound_stream *stream, int output, float gain);
void stream_set_sample_rate(sound_stream *stream, int sample_rate);

#endif
