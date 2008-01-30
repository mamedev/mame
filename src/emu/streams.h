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


/* ----- system-level management ----- */

/* initialize the streams engine */
void streams_init(running_machine *machine, attoseconds_t update_subseconds);

/* set the tag to be associated with all streams allocated from now on */
void streams_set_tag(running_machine *machine, void *streamtag);

/* update all the streams periodically */
void streams_update(running_machine *machine);



/* ----- stream configuration and setup ----- */

/* create a new stream */
sound_stream *stream_create(int inputs, int outputs, int sample_rate, void *param, stream_callback callback);

/* configure a stream's input */
void stream_set_input(sound_stream *stream, int index, sound_stream *input_stream, int output_index, float gain);

/* force a stream to update to the current emulated time */
void stream_update(sound_stream *stream);

/* return a pointer to the output buffer and the number of samples since the last global update */
const stream_sample_t *stream_get_output_since_last_update(sound_stream *stream, int outputnum, int *numsamples);



/* ----- stream timing ----- */

/* return the currently set sample rate on a given stream */
int stream_get_sample_rate(sound_stream *stream);

/* set the sample rate on a given stream */
void stream_set_sample_rate(sound_stream *stream, int sample_rate);

/* return the emulation time of the next sample to be generated on the stream */
attotime stream_get_time(sound_stream *stream);

/* return the duration of a single sample for a stream */
attotime stream_get_sample_period(sound_stream *stream);



/* ----- stream information and control ----- */

/* find a stream using a tag and index */
sound_stream *stream_find_by_tag(void *streamtag, int streamindex);

/* return the number of inputs for a given stream */
int stream_get_inputs(sound_stream *stream);

/* return the number of outputs for a given stream */
int stream_get_outputs(sound_stream *stream);

/* set the input gain on a given stream */
void stream_set_input_gain(sound_stream *stream, int input, float gain);

/* set the output gain on a given stream */
void stream_set_output_gain(sound_stream *stream, int output, float gain);


#endif
