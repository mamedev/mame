/* portmidi.c -- cross-platform MIDI I/O library */
/* see license.txt for license */

#include "stdlib.h"
#include "string.h"
#include "portmidi.h"
#include "porttime.h"
#include "pmutil.h"
#include "pminternal.h"
#include <assert.h>

#define MIDI_CLOCK      0xf8
#define MIDI_ACTIVE     0xfe
#define MIDI_STATUS_MASK 0x80
#define MIDI_SYSEX      0xf0
#define MIDI_EOX        0xf7
#define MIDI_START      0xFA
#define MIDI_STOP       0xFC
#define MIDI_CONTINUE   0xFB
#define MIDI_F9         0xF9
#define MIDI_FD         0xFD
#define MIDI_RESET      0xFF
#define MIDI_NOTE_ON    0x90
#define MIDI_NOTE_OFF   0x80
#define MIDI_CHANNEL_AT 0xD0
#define MIDI_POLY_AT    0xA0
#define MIDI_PROGRAM    0xC0
#define MIDI_CONTROL    0xB0
#define MIDI_PITCHBEND  0xE0
#define MIDI_MTC        0xF1
#define MIDI_SONGPOS    0xF2
#define MIDI_SONGSEL    0xF3
#define MIDI_TUNE       0xF6

#define is_empty(midi) ((midi)->tail == (midi)->head)

/* these are not static so that (possibly) some system-dependent code
 * could override the portmidi.c default which is to use the first
 * device added using pm_add_device()
 */
PmDeviceID pm_default_input_device_id = -1;
PmDeviceID pm_default_output_device_id = -1;

/* this is not static so that pm_init can set it directly
 *   (see pmmac.c:pm_init())
 */
int pm_initialized = FALSE;

int pm_hosterror;  /* boolean */

/* if PM_CHECK_ERRORS is enabled, but the caller wants to
 * handle an error condition, declare this as extern and 
 * set to FALSE (this override is provided specifically
 * for the test program virttest.c, where pmNameConflict 
 * is expected in a call to Pm_CreateVirtualInput()):
 */
int pm_check_errors = TRUE;

char pm_hosterror_text[PM_HOST_ERROR_MSG_LEN];

#ifdef PM_CHECK_ERRORS

#include <stdio.h>

#define STRING_MAX 80

static void prompt_and_exit(void)
{
    char line[STRING_MAX];
    printf("type ENTER...");
    char *rslt = fgets(line, STRING_MAX, stdin);
    /* this will clean up open ports: */
    exit(-1);
}

static PmError pm_errmsg(PmError err)
{
    if (!pm_check_errors) { /* see pm_check_errors declaration above */
        ;
    } else if (err == pmHostError) {
        /* it seems pointless to allocate memory and copy the string,
         * so I will do the work of Pm_GetHostErrorText directly
         */
        printf("PortMidi found host error...\n  %s\n", pm_hosterror_text);
        pm_hosterror = FALSE;
        pm_hosterror_text[0] = 0; /* clear the message */
        prompt_and_exit();
    } else if (err < 0) {
        printf("PortMidi call failed...\n  %s\n", Pm_GetErrorText(err));
        prompt_and_exit();
    }
    return err;
}
#else
#define pm_errmsg(err) err
#endif


int pm_midi_length(PmMessage msg)
{
    int status, high, low;
    static int high_lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1,         /* 0x00 through 0x70 */
        3, 3, 3, 3, 2, 2, 3, 1          /* 0x80 through 0xf0 */
    };
    static int low_lengths[] = {
        1, 2, 3, 2, 1, 1, 1, 1,         /* 0xf0 through 0xf8 */
        1, 1, 1, 1, 1, 1, 1, 1          /* 0xf9 through 0xff */
    };

    status = msg & 0xFF;
    high = status >> 4;
    low = status & 15;

    return (high != 0xF) ? high_lengths[high] : low_lengths[low];
}


/*
====================================================================
system implementation of portmidi interface
====================================================================
*/

int pm_descriptor_max = 0;
int pm_descriptor_len = 0;
descriptor_type pm_descriptors = NULL;

/* interface pm_descriptors are simple: an array of string/fnptr pairs: */
#define MAX_INTERF 4
static struct {
    const char *interf;
    pm_create_fn create_fn;
    pm_delete_fn delete_fn;
} pm_interf_list[MAX_INTERF];

static int pm_interf_list_len = 0;


/* pm_add_interf -- describe an interface to library
 *
 * This is called at initialization time, once for each
 * supported interface (e.g., CoreMIDI). The strings
 * are retained but NOT COPIED, so do not destroy them!
 *
 * The purpose is to register functions that create/delete
 * a virtual input or output device.
 *
 * returns pmInsufficientMemor if interface memory is
 * exceeded, otherwise returns pmNoError.
 */
PmError pm_add_interf(const char *interf, pm_create_fn create_fn,
                      pm_delete_fn delete_fn)
{
    if (pm_interf_list_len >= MAX_INTERF) {
        return pmInsufficientMemory;
    }
    pm_interf_list[pm_interf_list_len].interf = interf;
    pm_interf_list[pm_interf_list_len].create_fn = create_fn;
    pm_interf_list[pm_interf_list_len].delete_fn = delete_fn;
    pm_interf_list_len++;
    return pmNoError;
}


PmError pm_create_virtual(PmInternal *midi, int is_input, const char *interf,
                          const char *name, void *device_info)
{
    int i;
    if (pm_interf_list_len == 0) {
        return pmNotImplemented;
    }
    if (!interf) {
        /* default interface is the first one */
        interf = pm_interf_list[0].interf;
    }
    for (i = 0; i < pm_interf_list_len; i++) {
        if (strcmp(pm_interf_list[i].interf,
                   interf) == 0) {
            int id = (*pm_interf_list[i].create_fn)(is_input, name,
                                                        device_info);
            pm_descriptors[id].pub.is_virtual = TRUE;
            return id;
        }
    }
    return pmInterfaceNotSupported;
}


/* pm_add_device -- describe interface/device pair to library 
 *
 * This is called at intialization time, once for each 
 * interface (e.g. DirectSound) and device (e.g. SoundBlaster 1).
 * This is also called when user creates a virtual device.
 * 
 * Normally, increasing integer indices are returned. If the device
 * is virtual, a linear search is performed to ensure that the name
 * is unique. If the name is already taken, the call will fail and
 * no device is added.
 *
 * interf is assumed to be static memory, so it is NOT COPIED and 
 * NOT FREED.
 * name is owned by caller, COPIED if needed, and FREED by PortMidi.
 * Caller is resposible for freeing name when pm_add_device returns.
 *
 * returns pmInvalidDeviceId if device memory is exceeded or a virtual
 * device would take the name of an existing device.
 * otherwise returns index (portmidi device_id) of the added device
 */
PmError pm_add_device(const char *interf, const char *name, int is_input, 
                int is_virtual, void *descriptor, pm_fns_type dictionary) {
    /* printf("pm_add_device: %s %s %d %p %p\n",
           interf, name, is_input, descriptor, dictionary); */
    int device_id;
    PmDeviceInfo *d;
    /* if virtual, search for duplicate name or unused ID; otherwise,
     * just add a new device at the next integer available:
     */
    for (device_id = (is_virtual ? 0 : pm_descriptor_len);
         device_id < pm_descriptor_len; device_id++) {
        d = &pm_descriptors[device_id].pub;
        d->structVersion = PM_DEVICEINFO_VERS;
        if (strcmp(d->interf, interf) == 0 && strcmp(d->name, name) == 0) {
            /* only reuse a name if it is a deleted virtual device with
             * a matching direction (input or output) */
            if (pm_descriptors[device_id].deleted && is_input == d->input) {
                /* here, we know d->is_virtual because only virtual devices
                 * can be deleted, and we know is_virtual because we are
                 * in this loop. 
                 */
                pm_free((void *) d->name);  /* reuse this device entry */
                d->name = NULL;
                break;
            /* name conflict exists if the new device appears to others as
             * the same direction (input or output) as the existing device.
             * Note that virtual inputs appear to others as outputs and
             * vice versa.
             * The direction of the new virtual device to others is "output" 
             * if is_input, i.e., virtual inputs appear to others as outputs. 
             * The existing device appears to others as "output" if
             *     (d->is_virtual == d->input) by the same logic.
             * The compare will detect if device directions are the same:
             */
            } else if (is_input == (d->is_virtual == d->input)) {
                return pmNameConflict;
            }
        }
    }
    if (device_id >= pm_descriptor_max) {
        /* expand pm_descriptors */
        descriptor_type new_descriptors = (descriptor_type) 
            pm_alloc(sizeof(descriptor_node) * (pm_descriptor_max + 32));
        if (!new_descriptors) return pmInsufficientMemory;
        if (pm_descriptors) {
            memcpy(new_descriptors, pm_descriptors, 
                   sizeof(descriptor_node) * pm_descriptor_max);
            pm_free(pm_descriptors);
        }
        pm_descriptor_max += 32;
        pm_descriptors = new_descriptors;
    }
    if (device_id == pm_descriptor_len) {
        pm_descriptor_len++;  /* extending array of pm_descriptors */
    }
    d = &pm_descriptors[device_id].pub;
    d->interf = interf;
    d->name = pm_alloc(strlen(name) + 1);
    if (!d->name) {
        return pmInsufficientMemory;
    }
#if defined(WIN32) && !defined(_WIN32)
#pragma warning(suppress: 4996)  /* don't use suggested strncpy_s */
#endif
    strcpy(d->name, name);
    d->input = is_input;
    d->output = !is_input;
    d->is_virtual = FALSE;  /* caller should set to TRUE if this is virtual */

    /* default state: nothing to close (for automatic device closing) */
    d->opened = FALSE;

    pm_descriptors[device_id].deleted = FALSE;

    /* ID number passed to win32 multimedia API open */
    pm_descriptors[device_id].descriptor = descriptor;
    
    /* points to PmInternal, allows automatic device closing */
    pm_descriptors[device_id].pm_internal = NULL;

    pm_descriptors[device_id].dictionary = dictionary;

    /* set the defaults to the first input and output we see */
    if (is_input && pm_default_input_device_id == -1) {
        pm_default_input_device_id = device_id;
    } else if (!is_input && pm_default_output_device_id == -1) {
        pm_default_output_device_id = device_id;
    }
    
    return device_id;
}


/* Undo a successful call to pm_add_device(). If a new device was
 * allocated, it must be the last device in pm_descriptors, so it is
 * easy to delete by decrementing the length of pm_descriptors, but
 * first free the name (which was copied to the heap). Otherwise,
 * the device must be a virtual device that was created previously
 * and is in the interior of the array of pm_descriptors. Leave it,
 * but mark it as deleted.
 */
void pm_undo_add_device(int id)
{
    /* Clear some fields (not all are strictly necessary) */ 
    pm_descriptors[id].deleted = TRUE;
    pm_descriptors[id].descriptor = NULL;
    pm_descriptors[id].pm_internal = NULL;

    if (id == pm_descriptor_len - 1) {
        pm_free(pm_descriptors[id].pub.name);
        pm_descriptor_len--;
    }
}


/* utility to look up device, given a pattern, 
   note: pattern is modified
 */
int Pm_FindDevice(char *pattern, int is_input)
{
    int id = pmNoDevice;
    int i;
    /* first parse pattern into name, interf parts */
    const char *interf_pref = ""; /* initially assume it is not there */
    char *name_pref = strstr(pattern, ", ");

    if (name_pref) { /* found separator, adjust the pointer */
        interf_pref = pattern;
        name_pref[0] = 0;
        name_pref += 2;
    } else {
        name_pref = pattern; /* whole string is the name pattern */
    }
    for (i = 0; i < pm_descriptor_len; i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (info &&
            info->input == is_input &&
            strstr(info->name, name_pref) &&
            strstr(info->interf, interf_pref)) {
            id = i;
            break;
        }
    }    
    return id;
}


/*
====================================================================
portmidi implementation
====================================================================
*/

PMEXPORT int Pm_CountDevices(void)
{
    Pm_Initialize();
    /* no error checking -- Pm_Initialize() does not fail */
    return pm_descriptor_len;
}


PMEXPORT const PmDeviceInfo* Pm_GetDeviceInfo(PmDeviceID id)
{
    Pm_Initialize(); /* no error check needed */
    if (id >= 0 && id < pm_descriptor_len && !pm_descriptors[id].deleted) {
        return &pm_descriptors[id].pub;
    }
    return NULL;
}

/* pm_success_fn -- "noop" function pointer */
PmError pm_success_fn(PmInternal *midi)
{
    return pmNoError;
}

/* none_write -- returns an error if called */
PmError none_write_short(PmInternal *midi, PmEvent *buffer)
{
    return pmBadPtr;
}

/* pm_fail_timestamp_fn -- placeholder for begin_sysex and flush */
PmError pm_fail_timestamp_fn(PmInternal *midi, PmTimestamp timestamp)
{
    return pmBadPtr;
}

PmError none_write_byte(PmInternal *midi, unsigned char byte, 
                        PmTimestamp timestamp)
{
    return pmBadPtr;
}

/* pm_fail_fn -- generic function, returns error if called */
PmError pm_fail_fn(PmInternal *midi)
{
    return pmBadPtr;
}

static PmError none_open(PmInternal *midi, void *driverInfo)
{
    return pmBadPtr;
}

static unsigned int none_check_host_error(PmInternal * midi)
{
    return FALSE;
}

PmTimestamp none_synchronize(PmInternal *midi)
{
    return 0;
}

#define none_abort pm_fail_fn
#define none_close pm_fail_fn

pm_fns_node pm_none_dictionary = {
    none_write_short,
    none_sysex,
    none_sysex,
    none_write_byte,
    none_write_short,
    none_write_flush,
    none_synchronize,
    none_open,
    none_abort,
    none_close,
    none_poll,
    none_check_host_error,
};


PMEXPORT const char *Pm_GetErrorText(PmError errnum)
{
    const char *msg;

    switch(errnum)
    {
    case pmNoError:                  
        msg = ""; 
        break;
    case pmHostError:                
        msg = "PortMidi: Host error"; 
        break;
    case pmInvalidDeviceId:          
        msg = "PortMidi: Invalid device ID"; 
        break;
    case pmInsufficientMemory:       
        msg = "PortMidi: Insufficient memory"; 
        break;
    case pmBufferTooSmall:           
        msg = "PortMidi: Buffer too small"; 
        break;
    case pmBadPtr:                   
        msg = "PortMidi: Bad pointer";
        break;
    case pmInternalError:            
        msg = "PortMidi: Internal PortMidi Error";
        break;
    case pmBufferOverflow:
        msg = "PortMidi: Buffer overflow";
        break;
    case pmBadData:
        msg = "PortMidi: Invalid MIDI message Data";
        break;
    case pmBufferMaxSize:
        msg = "PortMidi: Buffer cannot be made larger";
        break;
    case pmNotImplemented:
        msg = "PortMidi: Function is not implemented";
        break;
    case pmInterfaceNotSupported:
        msg = "PortMidi: Interface not supported";
        break;
    case pmNameConflict:
        msg = "PortMidi: Cannot create virtual device: name is taken";
        break;
    case pmDeviceRemoved:
        msg = "PortMidi: Output attempted after (USB) device removed";
        break;
    default:
        msg = "PortMidi: Illegal error number";
        break;
    }
    return msg;
}


/* This can be called whenever you get a pmHostError return value
 * or TRUE from Pm_HasHostError().
 * The error will always be in the global pm_hosterror_text.
 */
PMEXPORT void Pm_GetHostErrorText(char * msg, unsigned int len)
{
    assert(msg);
    assert(len > 0);
    if (pm_hosterror) {
#if defined(WIN32) && !defined(_WIN32)
#pragma warning(suppress: 4996)  /* don't use suggested strncpy_s */
#endif
        strncpy(msg, (char *) pm_hosterror_text, len);
        pm_hosterror = FALSE;
        pm_hosterror_text[0] = 0; /* clear the message; not necessary, but it
                                     might help with debugging */
        msg[len - 1] = 0; /* make sure string is terminated */
    } else {
        msg[0] = 0; /* no string to return */
    }
}


PMEXPORT int Pm_HasHostError(PortMidiStream * stream)
{
    if (pm_hosterror) return TRUE;
    if (stream) {
        PmInternal * midi = (PmInternal *) stream;
        return (*midi->dictionary->check_host_error)(midi);
    }
    return FALSE;
}


PMEXPORT PmError Pm_Initialize(void)
{
    if (!pm_initialized) {
        pm_descriptor_len = 0;
        pm_interf_list_len = 0;
        pm_hosterror = FALSE;
        pm_hosterror_text[0] = 0; /* the null string */
        pm_init();
        pm_initialized = TRUE;
    }
    return pmNoError;
}


PMEXPORT PmError Pm_Terminate(void)
{
    if (pm_initialized) {
        pm_term();
        /* if there are no devices, pm_descriptors might still be NULL */
        if (pm_descriptors != NULL) {
            int i;  /* free names copied into pm_descriptors */
            for (i = 0; i < pm_descriptor_len; i++) {
                if (pm_descriptors[i].pub.name) {
                    pm_free(pm_descriptors[i].pub.name);
                }
            }
            pm_free(pm_descriptors);
            pm_descriptors = NULL;
        }
        pm_descriptor_len = 0;
        pm_descriptor_max = 0;
        pm_interf_list_len = 0;
        pm_initialized = FALSE;
    }
    return pmNoError;
}


/* Pm_Read -- read up to length messages from source into buffer */
/*
 * returns number of messages actually read, or error code
 */
PMEXPORT int Pm_Read(PortMidiStream *stream, PmEvent *buffer, int32_t length)
{
    PmInternal *midi = (PmInternal *) stream;
    int n = 0;
    PmError err = pmNoError;
    pm_hosterror = FALSE;
    /* arg checking */
    if(midi == NULL)
        err = pmBadPtr;
    else if(!pm_descriptors[midi->device_id].pub.opened)
        err = pmBadPtr;
    else if(!pm_descriptors[midi->device_id].pub.input)
        err = pmBadPtr;    
    /* First poll for data in the buffer...
     * This either simply checks for data, or attempts first to fill the buffer
     * with data from the MIDI hardware; this depends on the implementation.
     * We could call Pm_Poll here, but that would redo a lot of redundant
     * parameter checking, so I copied some code from Pm_Poll to here: */
    else err = (*(midi->dictionary->poll))(midi);

    if (err != pmNoError) {
        if (err == pmHostError) {
            midi->dictionary->check_host_error(midi);
        }
        return pm_errmsg(err);
    }

    while (n < length) {
        err = Pm_Dequeue(midi->queue, buffer++);
        if (err == pmBufferOverflow) {
            /* ignore the data we have retreived so far */
            return pm_errmsg(pmBufferOverflow);
        } else if (err == 0) { /* empty queue */
            break;
        }
        n++;
    }
    return n;
}

PMEXPORT PmError Pm_Poll(PortMidiStream *stream)
{
    PmInternal *midi = (PmInternal *) stream;
    PmError err;

    pm_hosterror = FALSE;
    /* arg checking */
    if(midi == NULL)
        err = pmBadPtr;
    else if (!pm_descriptors[midi->device_id].pub.opened)
        err = pmBadPtr;
    else if (!pm_descriptors[midi->device_id].pub.input)
        err = pmBadPtr;
    else
        err = (*(midi->dictionary->poll))(midi);

    if (err != pmNoError) {
        return pm_errmsg(err);
    }

    return (PmError) !Pm_QueueEmpty(midi->queue);
}


/* this is called from Pm_Write and Pm_WriteSysEx to issue a
 * call to the system-dependent end_sysex function and handle 
 * the error return
 */
static PmError pm_end_sysex(PmInternal *midi)
{
    PmError err = (*midi->dictionary->end_sysex)(midi, 0);
    midi->sysex_in_progress = FALSE;
    return err;
}


/* to facilitate correct error-handling, Pm_Write, Pm_WriteShort, and
   Pm_WriteSysEx all operate a state machine that "outputs" calls to
   write_short, begin_sysex, write_byte, end_sysex, and write_realtime */

PMEXPORT PmError Pm_Write(PortMidiStream *stream, PmEvent *buffer,
                          int32_t length)
{
    PmInternal *midi = (PmInternal *) stream;
    PmError err = pmNoError;
    int i;
    int bits;
    
    pm_hosterror = FALSE;
    /* arg checking */
    if (midi == NULL) {
        err = pmBadPtr;
    } else {
        descriptor_type desc = &pm_descriptors[midi->device_id]; 
        if (!desc || !desc->pub.opened ||
            !desc->pub.output || !desc->pm_internal) {
            err = pmBadPtr;
        } else if (desc->pm_internal->is_removed) {
            err = pmDeviceRemoved;
        }
    }
    if (err != pmNoError) goto pm_write_error;
    
    if (midi->latency == 0) {
        midi->now = 0;
    } else {
        midi->now = (*(midi->time_proc))(midi->time_info);
        if (midi->first_message || midi->sync_time + 100 /*ms*/ < midi->now) {
            /* time to resync */
            midi->now = (*midi->dictionary->synchronize)(midi);
            midi->first_message = FALSE;
        }
    }
    /* error recovery: when a sysex is detected, we call
     *   dictionary->begin_sysex() followed by calls to
     *   dictionary->write_byte() and dictionary->write_realtime()
     *   until an end-of-sysex is detected, when we call
     *   dictionary->end_sysex(). After an error occurs, 
     *   Pm_Write() continues to call functions. For example,
     *   it will continue to call write_byte() even after
     *   an error sending a sysex message, and end_sysex() will be
     *   called when an EOX or non-real-time status is found.
     * When errors are detected, Pm_Write() returns immediately, 
     *   so it is possible that this will drop data and leave
     *   sysex messages in a partially transmitted state.
     */
    for (i = 0; i < length; i++) {
        uint32_t msg = buffer[i].message;
        bits = 0;
        /* is this a sysex message? */
        if (Pm_MessageStatus(msg) == MIDI_SYSEX) {
            if (midi->sysex_in_progress) {
                /* error: previous sysex was not terminated by EOX */
                midi->sysex_in_progress = FALSE;
                err = pmBadData;
                goto pm_write_error;
            }
            midi->sysex_in_progress = TRUE;
            if ((err = (*midi->dictionary->begin_sysex)(midi, 
                               buffer[i].timestamp)) != pmNoError)
                goto pm_write_error;
            if ((err = (*midi->dictionary->write_byte)(midi, MIDI_SYSEX,
                               buffer[i].timestamp)) != pmNoError) 
                goto pm_write_error;
            bits = 8;
            /* fall through to continue sysex processing */
        } else if ((msg & MIDI_STATUS_MASK) && 
                   (Pm_MessageStatus(msg) != MIDI_EOX)) {
            /* a non-sysex message */
            if (midi->sysex_in_progress) {
                /* this should be a realtime message */
                if (is_real_time(msg)) {
                    if ((err = (*midi->dictionary->write_realtime)(midi, 
                                       &(buffer[i]))) != pmNoError)
                        goto pm_write_error;
                } else {
                    midi->sysex_in_progress = FALSE;
                    err = pmBadData;
                    /* ignore any error from this, because we already have one */
                    /* pass 0 as timestamp -- it's ignored */
                    (*midi->dictionary->end_sysex)(midi, 0);
                    goto pm_write_error;
                }
            } else { /* regular short midi message */
                if ((err = (*midi->dictionary->write_short)(midi, 
                                   &(buffer[i]))) != pmNoError)
                    goto pm_write_error;
                continue;
            }
        }
        if (midi->sysex_in_progress) { /* send sysex bytes until EOX */
            /* see if we can accelerate data transfer */
            if (bits == 0 && midi->fill_base && /* 4 bytes to copy */
                (*midi->fill_offset_ptr) + 4 <= midi->fill_length &&
                (msg & 0x80808080) == 0) { /* all data */
                    /* copy 4 bytes from msg to fill_base + fill_offset */
                    unsigned char *ptr = midi->fill_base + 
                                         *(midi->fill_offset_ptr);
                    ptr[0] = msg; ptr[1] = msg >> 8; 
                    ptr[2] = msg >> 16; ptr[3] = msg >> 24;
                    (*midi->fill_offset_ptr) += 4;
                     continue;
            }
            /* no acceleration, so do byte-by-byte copying */
            while (bits < 32) {
                unsigned char midi_byte = (unsigned char) (msg >> bits);
                if ((err = (*midi->dictionary->write_byte)(midi, midi_byte, 
                                   buffer[i].timestamp)) != pmNoError)
                    goto pm_write_error;
                if (midi_byte == MIDI_EOX) {
                    err = pm_end_sysex(midi);
                    if (err != pmNoError) goto error_exit;
                    break; /* from while loop */
                }
                bits += 8;
            }
        } else {
            /* not in sysex mode, but message did not start with status */
            err = pmBadData;
            goto pm_write_error;
        }
    }
    /* after all messages are processed, send the data */
    if (!midi->sysex_in_progress)
        err = (*midi->dictionary->write_flush)(midi, 0);
pm_write_error:
    if (err == pmHostError) {
        midi->dictionary->check_host_error(midi);
    }
error_exit:
    return pm_errmsg(err);
}


PMEXPORT PmError Pm_WriteShort(PortMidiStream *stream, PmTimestamp when,
                               PmMessage msg)
{
    PmEvent event;
    
    event.timestamp = when;
    event.message = msg;
    return Pm_Write(stream, &event, 1);
}


PMEXPORT PmError Pm_WriteSysEx(PortMidiStream *stream, PmTimestamp when, 
                      unsigned char *msg)
{
    /* allocate buffer space for PM_DEFAULT_SYSEX_BUFFER_SIZE bytes */
    /* each PmEvent holds sizeof(PmMessage) bytes of sysex data */
    #define BUFLEN ((int) (PM_DEFAULT_SYSEX_BUFFER_SIZE / sizeof(PmMessage)))
    PmEvent buffer[BUFLEN];
    int buffer_size = 1; /* first time, send 1. After that, it's BUFLEN */
    PmInternal *midi = (PmInternal *) stream;
    PmError err = pmNoError;
    /* the next byte in the buffer is represented by an index, bufx, and
       a shift in bits */
    int shift = 0;
    int bufx = 0;
    buffer[0].message = 0;
    buffer[0].timestamp = when;

    while (1) {
        /* insert next byte into buffer */
        buffer[bufx].message |= ((*msg) << shift);
        shift += 8;
        if (*msg++ == MIDI_EOX) break;
        if (shift == 32) {
            shift = 0;
            bufx++;
            if (bufx == buffer_size) {
                err = Pm_Write(stream, buffer, buffer_size);
                /* note: Pm_Write has already called errmsg() */
                if (err) return err;
                /* prepare to fill another buffer */
                bufx = 0;
                buffer_size = BUFLEN;
                /* optimization: maybe we can just copy bytes */
                if (midi->fill_base) {
                    while (*(midi->fill_offset_ptr) < midi->fill_length) {
                        midi->fill_base[(*midi->fill_offset_ptr)++] = *msg;
                        if (*msg++ == MIDI_EOX) {
                            err = pm_end_sysex(midi);
                            if (err != pmNoError) return pm_errmsg(err);
                            goto end_of_sysex;
                        }
                    }
                    /* I thought that I could do a pm_Write here and
                     * change this if to a loop, avoiding calls in Pm_Write
                     * to the slower write_byte, but since 
                     * sysex_in_progress is true, this will not flush
                     * the buffer, and we'll infinite loop: */
                    /* err = Pm_Write(stream, buffer, 0);
                       if (err) return err; */
                    /* instead, the way this works is that Pm_Write calls
                     * write_byte on 4 bytes. The first, since the buffer
                     * is full, will flush the buffer and allocate a new
                     * one. This primes the buffer so
                     * that we can return to the loop above and fill it
                     * efficiently without a lot of function calls.
                     */
                    buffer_size = 1; /* get another message started */
                }
            }
            buffer[bufx].message = 0;
            buffer[bufx].timestamp = when;
        } 
        /* keep inserting bytes until you find MIDI_EOX */
    }
end_of_sysex:
    /* we're finished sending full buffers, but there may
     * be a partial one left.
     */
    if (shift != 0) bufx++; /* add partial message to buffer len */
    if (bufx) { /* bufx is number of PmEvents to send from buffer */
        err = Pm_Write(stream, buffer, bufx);
        if (err) return err;
    }
    return pmNoError;
}



PmError pm_create_internal(PmInternal **stream, PmDeviceID device_id,
                           int is_input, int latency, PmTimeProcPtr time_proc,
                           void *time_info, int buffer_size)
{
    PmInternal *midi;  /* initialized below */
    if (device_id < 0 || device_id >= pm_descriptor_len) {
       return pmInvalidDeviceId;
    }
    if (latency < 0) {  /* force a legal value */
        latency = 0;
    }
    /* create portMidi internal data */
    midi = (PmInternal *) pm_alloc(sizeof(PmInternal)); 
    *stream = midi;
    if (!midi) {
        return pmInsufficientMemory;
    }
    midi->device_id = device_id;
    midi->is_input = is_input;
    midi->is_removed = FALSE;
    midi->time_proc = time_proc;
    /* if latency != 0, we need a time reference for output.
       we always need a time reference for input.
       If none is provided, use PortTime library */
    if (time_proc == NULL && (latency != 0 || is_input)) {
        if (!Pt_Started()) 
            Pt_Start(1, 0, 0);
        /* time_get does not take a parameter, so coerce */
        midi->time_proc = (PmTimeProcPtr) Pt_Time;
    }
    midi->time_info = time_info;
    if (is_input) {
        midi->latency = 0;  /* unused by input */
        if (buffer_size <= 0) buffer_size = 256; /* default buffer size */
        midi->queue = Pm_QueueCreate(buffer_size, (int32_t) sizeof(PmEvent));
        if (!midi->queue) {
            /* free portMidi data */
            *stream = NULL;
            pm_free(midi);
            return pmInsufficientMemory;
        }
    } else {
        /* if latency zero, output immediate (timestamps ignored) */
        /* if latency < 0, use 0 but don't return an error */
        if (latency < 0) latency = 0;
        midi->latency = latency;
        midi->queue = NULL;  /* unused by output; input needs to allocate: */
    }
    midi->buffer_len = buffer_size; /* portMidi input storage */
    midi->sysex_in_progress = FALSE;
    midi->message = 0; 
    midi->message_count = 0; 
    midi->filters = (is_input ? PM_FILT_ACTIVE : 0);
    midi->channel_mask = 0xFFFF;
    midi->sync_time = 0;
    midi->first_message = TRUE;
    midi->api_info = NULL;
    midi->fill_base = NULL;
    midi->fill_offset_ptr = NULL;
    midi->fill_length = 0;
    midi->dictionary = pm_descriptors[device_id].dictionary;
    pm_descriptors[device_id].pm_internal = midi;
    return pmNoError;
}


PMEXPORT PmError Pm_OpenInput(PortMidiStream** stream,
                              PmDeviceID inputDevice,
                              void *inputDriverInfo,
                              int32_t bufferSize,
                              PmTimeProcPtr time_proc,
                              void *time_info)
{
    PmInternal *midi;
    PmError err = pmNoError;
    pm_hosterror = FALSE;
    *stream = NULL;  /* invariant: *stream == midi */
    
    /* arg checking */
    if (!pm_descriptors[inputDevice].pub.input) 
        err =  pmInvalidDeviceId;
    else if (pm_descriptors[inputDevice].pub.opened)
        err =  pmInvalidDeviceId;
    if (err != pmNoError) 
        goto error_return;

    /* common initialization of PmInternal structure (midi): */
    err = pm_create_internal(&midi, inputDevice, TRUE, 0, time_proc,
                             time_info, bufferSize);
    if (err) {
        goto error_return;  /* will return with *stream == NULL */
    }
    *stream = midi;  /* no error, so midi is valid */

    /* open system dependent input device */
    err = (*midi->dictionary->open)(midi, inputDriverInfo);
    if (err) {
        *stream = NULL;
        pm_descriptors[inputDevice].pm_internal = NULL;
        /* free portMidi data */
        Pm_QueueDestroy(midi->queue);
        pm_free(midi);
    } else {
        /* portMidi input open successful */
        pm_descriptors[inputDevice].pub.opened = TRUE;
    }
error_return:
    /* note: if there is a pmHostError, it is the responsibility
     * of the system-dependent code (*midi->dictionary->open)()
     * to set pm_hosterror and pm_hosterror_text
     */
    return pm_errmsg(err);
}


PMEXPORT PmError Pm_OpenOutput(PortMidiStream** stream,
                               PmDeviceID outputDevice,
                               void *outputDriverInfo,
                               int32_t bufferSize,
                               PmTimeProcPtr time_proc,
                               void *time_info,
                               int32_t latency)
{
    PmInternal *midi;
    PmError err = pmNoError;
    pm_hosterror = FALSE;
    *stream =  NULL;
    
    /* arg checking */
    if (outputDevice < 0 || outputDevice >= pm_descriptor_len)
        err = pmInvalidDeviceId;
    else if (!pm_descriptors[outputDevice].pub.output) 
        err = pmInvalidDeviceId;
    else if (pm_descriptors[outputDevice].pub.opened)
        err = pmInvalidDeviceId;
    if (err != pmNoError) 
        goto error_return;

    /* common initialization of PmInternal structure (midi): */
    err = pm_create_internal(&midi, outputDevice, FALSE, latency, time_proc,
                             time_info, bufferSize);
    *stream = midi;
    if (err) {
        goto error_return;
    }

    /* open system dependent output device */
    err = (*midi->dictionary->open)(midi, outputDriverInfo);
    if (err) {
        *stream = NULL;
        pm_descriptors[outputDevice].pm_internal = NULL;
        /* free portMidi data */
        pm_free(midi); 
    } else {
        /* portMidi input open successful */
        pm_descriptors[outputDevice].pub.opened = TRUE;
    }
error_return:
    /* note: system-dependent code must set pm_hosterror and
     * pm_hosterror_text if a pmHostError occurs
     */
    return pm_errmsg(err);
}


static PmError create_virtual_device(const char *name, const char *interf,
                                     void *device_info, int is_input)
{
    PmError err = pmNoError;
    int i;
    pm_hosterror = FALSE;
    
    /* arg checking */
    if (!name) {
        err = pmInvalidDeviceId;
        goto error_return;
    }

    Pm_Initialize();  /* just in case */

    /* create the virtual device */
    if (pm_interf_list_len == 0) {
        return pmNotImplemented;
    }
    if (!interf) {
        /* default interface is the first one */
        interf = pm_interf_list[0].interf;
    }
    /* look up and call the create_fn for interf(ace), e.g. "CoreMIDI" */
    for (i = 0; i < pm_interf_list_len; i++) {
        if (strcmp(pm_interf_list[i].interf, interf) == 0) {
            int id = (*pm_interf_list[i].create_fn)(is_input,
                                                        name, device_info);
            /* id could be pmNameConflict or an actual descriptor index */
            if (id >= 0) {
                pm_descriptors[id].pub.is_virtual = TRUE;
            }
            err = id;
            goto error_return;
        }
    }
    err = pmInterfaceNotSupported;

error_return:
    /* note: if there is a pmHostError, it is the responsibility
     * of the system-dependent code (*midi->dictionary->open)()
     * to set pm_hosterror and pm_hosterror_text
     */
    return pm_errmsg(err);
}


PMEXPORT PmError Pm_CreateVirtualInput(const char *name,
                                       const char *interf,
                                       void *deviceInfo)
{
    return create_virtual_device(name, interf, deviceInfo, TRUE);
}

PMEXPORT PmError Pm_CreateVirtualOutput(const char *name, const char *interf,
                                        void *deviceInfo)
{
    return create_virtual_device(name, interf, deviceInfo, FALSE);
}

PmError Pm_DeleteVirtualDevice(PmDeviceID id)
{
    int i;
    const char *interf = pm_descriptors[id].pub.interf;
    PmError err = pmBadData;  /* returned if we cannot find the interface-
                               * specific delete function */
    /* arg checking */
    if (id < 0 || id >= pm_descriptor_len ||
        pm_descriptors[id].pub.opened || pm_descriptors[id].deleted) {
        return pmInvalidDeviceId;
    }
    /* delete function pointer is in interfaces list */
    for (i = 0; i < pm_interf_list_len; i++) {
        if (strcmp(pm_interf_list[i].interf, interf) == 0) {
            err = (*pm_interf_list[i].delete_fn)(id);
            break;
        }
    }
    pm_descriptors[id].deleted = TRUE;
    /* (pm_internal should already be NULL because !pub.opened) */
    pm_descriptors[id].pm_internal = NULL;
    pm_descriptors[id].descriptor = NULL;
    return err;
}

PMEXPORT PmError Pm_SetChannelMask(PortMidiStream *stream, int mask)
{
    PmInternal *midi = (PmInternal *) stream;
    PmError err = pmNoError;

    if (midi == NULL)
        err = pmBadPtr;
    else
        midi->channel_mask = mask;

    return pm_errmsg(err);
}


PMEXPORT PmError Pm_SetFilter(PortMidiStream *stream, int32_t filters)
{
    PmInternal *midi = (PmInternal *) stream;
    PmError err = pmNoError;

    /* arg checking */
    if (midi == NULL)
        err = pmBadPtr;
    else if (!pm_descriptors[midi->device_id].pub.opened)
        err = pmBadPtr;
    else
        midi->filters = filters;
    return pm_errmsg(err);
}


PMEXPORT PmError Pm_Close(PortMidiStream *stream)
{
    PmInternal *midi = (PmInternal *) stream;
    PmError err = pmNoError;

    pm_hosterror = FALSE;
    /* arg checking */
    if (midi == NULL) /* midi must point to something */
        err = pmBadPtr;
    /* if it is an open device, the device_id will be valid */
    else if (midi->device_id < 0 || midi->device_id >= pm_descriptor_len)
        err = pmBadPtr;
    /* and the device should be in the opened state */
    else if (!pm_descriptors[midi->device_id].pub.opened)
        err = pmBadPtr;
    
    if (err != pmNoError) 
        goto error_return;

    /* close the device */
    err = (*midi->dictionary->close)(midi);
    /* even if an error occurred, continue with cleanup */
    pm_descriptors[midi->device_id].pm_internal = NULL;
    pm_descriptors[midi->device_id].pub.opened = FALSE;
    if (midi->queue) Pm_QueueDestroy(midi->queue);
    pm_free(midi); 
error_return:
    /* system dependent code must set pm_hosterror and
     * pm_hosterror_text if a pmHostError occurs.
     */
    return pm_errmsg(err);
}

PmError Pm_Synchronize(PortMidiStream* stream)
{
    PmInternal *midi = (PmInternal *) stream;
    PmError err = pmNoError;
    if (midi == NULL)
        err = pmBadPtr;
    else if (!pm_descriptors[midi->device_id].pub.output)
        err = pmBadPtr;
    else if (!pm_descriptors[midi->device_id].pub.opened)
        err = pmBadPtr;
    else
        midi->first_message = TRUE;
    return err;
}

PMEXPORT PmError Pm_Abort(PortMidiStream* stream)
{
    PmInternal *midi = (PmInternal *) stream;
    PmError err;
    /* arg checking */
    if (midi == NULL)
        err = pmBadPtr;
    else if (!pm_descriptors[midi->device_id].pub.output)
        err = pmBadPtr;
    else if (!pm_descriptors[midi->device_id].pub.opened)
        err = pmBadPtr;
    else
        err = (*midi->dictionary->abort)(midi);

    if (err == pmHostError) {
        midi->dictionary->check_host_error(midi);
    }
    return pm_errmsg(err);
}



/* pm_channel_filtered returns non-zero if the channel mask is
   blocking the current channel */
#define pm_channel_filtered(status, mask) \
    ((((status) & 0xF0) != 0xF0) && (!(Pm_Channel((status) & 0x0F) & (mask))))


/* The following two functions will checks to see if a MIDI message
   matches the filtering criteria.  Since the sysex routines only want
   to filter realtime messages, we need to have separate routines.
 */


/* pm_realtime_filtered returns non-zero if the filter will kill the
   current message. Note that only realtime messages are checked here.
 */
#define pm_realtime_filtered(status, filters) \
    ((((status) & 0xF0) == 0xF0) && ((1 << ((status) & 0xF)) & (filters)))

/*
    return ((status == MIDI_ACTIVE) && (filters & PM_FILT_ACTIVE))
            ||  ((status == MIDI_CLOCK) && (filters & PM_FILT_CLOCK))
            ||  ((status == MIDI_START) && (filters & PM_FILT_PLAY))
            ||  ((status == MIDI_STOP) && (filters & PM_FILT_PLAY))
            ||  ((status == MIDI_CONTINUE) && (filters & PM_FILT_PLAY))
            ||  ((status == MIDI_F9) && (filters & PM_FILT_F9))
            ||  ((status == MIDI_FD) && (filters & PM_FILT_FD))
            ||  ((status == MIDI_RESET) && (filters & PM_FILT_RESET))
            ||  ((status == MIDI_MTC) && (filters & PM_FILT_MTC))
            ||  ((status == MIDI_SONGPOS) && (filters & PM_FILT_SONG_POSITION))
            ||  ((status == MIDI_SONGSEL) && (filters & PM_FILT_SONG_SELECT))
            ||  ((status == MIDI_TUNE) && (filters & PM_FILT_TUNE));
}*/


/* pm_status_filtered returns non-zero if a filter will kill the
   current message, based on status. Note that sysex and real time are
   not checked.  It is up to the subsystem (winmm, core midi, alsa) to
   filter sysex, as it is handled more easily and efficiently at that
   level. Realtime message are filtered in pm_realtime_filtered.
 */
#define pm_status_filtered(status, filters) \
                ((1 << (16 + ((status) >> 4))) & (filters))


/*
    return  ((status == MIDI_NOTE_ON) && (filters & PM_FILT_NOTE))
            ||  ((status == MIDI_NOTE_OFF) && (filters & PM_FILT_NOTE))
            ||  ((status == MIDI_CHANNEL_AT) &&
                 (filters & PM_FILT_CHANNEL_AFTERTOUCH))
            ||  ((status == MIDI_POLY_AT) && (filters & PM_FILT_POLY_AFTERTOUCH))
            ||  ((status == MIDI_PROGRAM) && (filters & PM_FILT_PROGRAM))
            ||  ((status == MIDI_CONTROL) && (filters & PM_FILT_CONTROL))
            ||  ((status == MIDI_PITCHBEND) && (filters & PM_FILT_PITCHBEND));

}
*/

static void pm_flush_sysex(PmInternal *midi, PmTimestamp timestamp)
{
    PmEvent event;
    
    /* there may be nothing in the buffer */
    if (midi->message_count == 0) return; /* nothing to flush */
    
    event.message = midi->message;
    event.timestamp = timestamp;
    /* copied from pm_read_short, avoids filtering */
    if (Pm_Enqueue(midi->queue, &event) == pmBufferOverflow) {
        midi->sysex_in_progress = FALSE;
    }
    midi->message_count = 0;
    midi->message = 0;
}


/* pm_read_short and pm_read_bytes
   are the interface between system-dependent MIDI input handlers
   and the system-independent PortMIDI code.
   The input handler MUST obey these rules:
   1) all short input messages must be sent to pm_read_short, which
      enqueues them to a FIFO for the application.
   2) each buffer of sysex bytes should be reported by calling pm_read_bytes
      (which sets midi->sysex_in_progress). After the eox byte, 
      pm_read_bytes will clear sysex_in_progress
 */

/* pm_read_short is the place where all input messages arrive from 
   system-dependent code such as pmwinmm.c. Here, the messages
   are entered into the PortMidi input buffer. 
 */
void pm_read_short(PmInternal *midi, PmEvent *event)
{ 
    int status;
    /* arg checking */
    assert(midi != NULL);
    /* midi filtering is applied here */
    status = Pm_MessageStatus(event->message);
    if (!pm_status_filtered(status, midi->filters)
        && (!is_real_time(status) || 
            !pm_realtime_filtered(status, midi->filters))
        && !pm_channel_filtered(status, midi->channel_mask)) {
        /* if sysex is in progress and we get a status byte, it had
           better be a realtime message or the starting SYSEX byte;
           otherwise, we exit the sysex_in_progress state
         */
        if (midi->sysex_in_progress && (status & MIDI_STATUS_MASK)) {
            /* two choices: real-time or not. If it's real-time, then
             * this should be delivered as a sysex byte because it is
             * embedded in a sysex message
             */
            if (is_real_time(status)) {
                midi->message |= (status << (8 * midi->message_count++));
                if (midi->message_count == 4) {
                    pm_flush_sysex(midi, event->timestamp);
                }
            } else { /* otherwise, it's not real-time. This interrupts
                      * a sysex message in progress */
                midi->sysex_in_progress = FALSE;
            }
        } else if (Pm_Enqueue(midi->queue, event) == pmBufferOverflow) {
            midi->sysex_in_progress = FALSE;
        }
    }
}


/* pm_read_bytes -- a sequence of bytes has been read from a device.
 *        parse the bytes into PmEvents and put them in the queue.
 * midi - the midi device
 * data - the bytes
 * len - the number of bytes
 * timestamp - when were the bytes received?
 *
 * returns how many bytes processed, which is always the len parameter
 */
unsigned int pm_read_bytes(PmInternal *midi, const unsigned char *data, 
                    int len, PmTimestamp timestamp)
{
    int i = 0; /* index into data, must not be unsigned (!) */
    PmEvent event;
    event.timestamp = timestamp;
    assert(midi);

    /* Since sysex messages may have embedded real-time messages, we
     * cannot simply send every consecutive group of 4 bytes as sysex
     * data. Instead, we insert each data byte into midi->message and
     * keep count using midi->message_count. If we encounter a
     * real-time message, it is sent immediately as a 1-byte PmEvent,
     * while sysex bytes are sent as PmEvents in groups of 4 bytes
     * until the sysex is either terminated by EOX (F7) or a
     * non-real-time message is encountered, indicating that the EOX
     * was dropped.
     */

    /* This is a finite state machine so that we can accept any number
     * of bytes, even if they contain partial messages.
     *
     * midi->sysex_in_progress says we are expecting sysex message bytes
     *    (otherwise, expect a short message or real-time message)
     * midi->message accumulates bytes to enqueue for application
     * midi->message_count is the number of bytes accumulated
     * midi->short_message_count is how many bytes we need in midi->message,
     *    therefore midi->message_count, before we have a complete message
     * midi->running_status is running status or 0 if there is none
     *
     * Set running status when: A status byte < F0 is received.
     * Clear running status when: A status byte from F0 through F7 is
     * received.
     * Ignore (drop) data bytes when running status is 0.
     *
     * Our output buffer (the application input buffer) can overflow
     * at any time. If that occurs, we have to clear sysex_in_progress
     * (otherwise, the buffer could be flushed and we could resume
     * inserting sysex bytes into the buffer, resulting in a continuation
     * of a sysex message even though a buffer full of bytes was dropped.)
     *
     * Since we have to parse everything and form <=4-byte PmMessages,
     * we send all messages via pm_read_short, which filters messages
     * according to midi->filters and clears sysex_in_progress on
     * buffer overflow. This also provides a "short cut" for short
     * messages that are already parsed, allowing API-specific code
     * to bypass this more expensive state machine. What if we are
     * getting a sysex message, but it is interrupted by a short
     * message (status 80-EF) and a direct call to pm_read_short?
     * Without some care, the state machine would still be in
     * sysex_in_progress mode, and subsequent data bytes would be
     * accumulated as more sysex data, which is wrong since you
     * cannot have a short message in the middle of a sysex message.
     * To avoid this problem, pm_read_short clears sysex_in_progress
     * when a non-real-time short message arrives.
     */

    while (i < len) {
        unsigned char byte = data[i++];
        if (is_real_time(byte)) {
            event.message = byte;
            pm_read_short(midi, &event);
        } else if (byte & MIDI_STATUS_MASK && byte != MIDI_EOX) {
            midi->message = byte;
            midi->message_count = 1;
            if (byte == MIDI_SYSEX) {
                midi->sysex_in_progress = TRUE;
            } else {
                /* If Sysex is in progress, we cancel it because we encountered
                   a status byte:
                */
                midi->sysex_in_progress = FALSE;
                midi->short_message_count = pm_midi_length(midi->message);
                /* maybe we're done already with a 1-byte message: */
                if (midi->short_message_count == 1) {
                    event.message = byte;
                    pm_read_short(midi, &event);
                    midi->message_count = 0;
                }
            }
        } else if (midi->sysex_in_progress) {  /* sysex data byte */
            /* accumulate sysex message data or EOX */
            midi->message |= (byte << (8 * midi->message_count++));
            if (midi->message_count == 4 || byte == MIDI_EOX) {
                event.message = midi->message;
                /* enqueue if not filtered, and then if there is overflow,
                   stop sysex_in_progress */
                if (!(midi->filters & PM_FILT_SYSEX) &&
                    Pm_Enqueue(midi->queue, &event) == pmBufferOverflow) {
                    midi->sysex_in_progress = FALSE;
                } else if (byte == MIDI_EOX) {  /* continue unless EOX */
                    midi->sysex_in_progress = FALSE;
                }
                midi->message_count = 0;
                midi->message = 0;
            }
        } else {  /* no sysex in progress, must be short message */
            if (midi->message_count == 0) {  /* need a running status */
                if (midi->running_status) {
                    midi->message = midi->running_status;
                    midi->message_count = 1;
                } else {  /* drop data byte - not sysex and no status byte */
                    continue;
                }
            }
            midi->message |= (byte << (8 * midi->message_count++));
            if (midi->message_count == midi->short_message_count) {
                event.message = midi->message;
                pm_read_short(midi, &event);
            }
        }
    }
    return i;
}
