/***************************************************************************

    streams.h

    Handle general purpose audio streams

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef STREAMS_H
#define STREAMS_H



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define STREAMS_UPDATE_FREQUENCY	(50)
#define STREAMS_UPDATE_ATTOTIME		ATTOTIME_IN_HZ(STREAMS_UPDATE_FREQUENCY)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _sound_stream sound_stream;

typedef void (*stream_update_func)(const device_config *device, void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

#define STREAM_UPDATE(name) void name(const device_config *device, void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- system-level management ----- */

/* initialize the streams engine */
void streams_init(running_machine *machine);

/* update all the streams periodically */
void streams_update(running_machine *machine);



/* ----- stream configuration and setup ----- */

/* create a new stream */
sound_stream *stream_create(const device_config *device, int inputs, int outputs, int sample_rate, void *param, stream_update_func callback);

/* convert a device/output pair to a stream/output pair */
int stream_device_output_to_stream_output(const device_config *device, int outputnum, sound_stream **streamptr, int *streamoutputptr);

/* convert a device/input pair to a stream/input pair */
int stream_device_input_to_stream_input(const device_config *device, int inputnum, sound_stream **streamptr, int *streaminputptr);

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

/* return the total number of outputs for the given device */
int stream_get_device_outputs(const device_config *device);

/* find a stream using a device and index */
sound_stream *stream_find_by_device(const device_config *device, int streamindex);

/* return the number of inputs for a given stream */
int stream_get_inputs(sound_stream *stream);

/* return the number of outputs for a given stream */
int stream_get_outputs(sound_stream *stream);

/* set the input gain on a given stream */
void stream_set_input_gain(sound_stream *stream, int input, float gain);

/* set the output gain on a given stream */
void stream_set_output_gain(sound_stream *stream, int output, float gain);


#endif
