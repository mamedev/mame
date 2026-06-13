/* pmwinmm.c -- system specific definitions */

#ifndef _WIN32_WINNT
    /* without this define, InitializeCriticalSectionAndSpinCount is 
     * undefined. This version level means "Windows 2000 and higher" 
     */
    #define _WIN32_WINNT 0x0500
#endif

#define UNICODE 1
#include <wchar.h>
#include "windows.h"
#include "mmsystem.h"
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"
#include "pmwinmm.h"
#include <string.h>
#include "porttime.h"
#ifndef UNICODE
#error Expected UNICODE to be defined
#endif


/* asserts used to verify portMidi code logic is sound; later may want
    something more graceful */
#include <assert.h>
#ifdef MMDEBUG
/* this printf stuff really important for debugging client app w/host errors.
    probably want to do something else besides read/write from/to console
    for portability, however */
#define STRING_MAX 80
#include "stdio.h"
#endif

#define streql(x, y) (strcmp(x, y) == 0)

#define MIDI_SYSEX      0xf0
#define MIDI_EOX        0xf7

/* callback routines */
static void CALLBACK winmm_in_callback(HMIDIIN hMidiIn,
                                       UINT wMsg, DWORD_PTR dwInstance, 
                                       DWORD_PTR dwParam1, DWORD_PTR dwParam2);
static void CALLBACK winmm_streamout_callback(HMIDIOUT hmo, UINT wMsg,
                                              DWORD_PTR dwInstance, 
                                              DWORD_PTR dwParam1,
                                              DWORD_PTR dwParam2);

extern pm_fns_node pm_winmm_in_dictionary;
extern pm_fns_node pm_winmm_out_dictionary;

static void winmm_out_delete(PmInternal *midi); /* forward reference */

/*
A note about buffers: WinMM seems to hold onto buffers longer than
one would expect, e.g. when I tried using 2 small buffers to send
long sysex messages, at some point WinMM held both buffers. This problem
was fixed by making buffers bigger. Therefore, it seems that there should 
be enough buffer space to hold a whole sysex message. 

The bufferSize passed into Pm_OpenInput (passed into here as buffer_len)
will be used to estimate the largest sysex message (= buffer_len * 4 bytes).
Call that the max_sysex_len = buffer_len * 4.

For simple midi output (latency == 0), allocate 3 buffers, each with half
the size of max_sysex_len, but each at least 256 bytes.

For stream output, there will already be enough space in very short
buffers, so use them, but make sure there are at least 16.

For input, use many small buffers rather than 2 large ones so that when 
there are short sysex messages arriving frequently (as in control surfaces)
there will be more free buffers to fill. Use max_sysex_len / 64 buffers,
but at least 16, of size 64 bytes each.

The following constants help to represent these design parameters:
*/
#define NUM_SIMPLE_SYSEX_BUFFERS 3
#define MIN_SIMPLE_SYSEX_LEN 256

#define MIN_STREAM_BUFFERS 16
#define STREAM_BUFFER_LEN 24

#define INPUT_SYSEX_LEN 64
#define MIN_INPUT_BUFFERS 16

/* if we run out of space for output (assume this is due to a sysex msg,
   expand by up to NUM_EXPANSION_BUFFERS in increments of EXPANSION_BUFFER_LEN
 */
#define NUM_EXPANSION_BUFFERS 128
#define EXPANSION_BUFFER_LEN 1024

/* A sysex buffer has 3 DWORDS as a header plus the actual message size */
#define MIDIHDR_SYSEX_BUFFER_LENGTH(x) ((x) + sizeof(long)*3)
/* A MIDIHDR with a sysex message is the buffer length plus the header size */
#define MIDIHDR_SYSEX_SIZE(x) (MIDIHDR_SYSEX_BUFFER_LENGTH(x) + sizeof(MIDIHDR))

/*
==============================================================================
win32 mmedia system specific structure passed to midi callbacks
==============================================================================
*/

/* global winmm device info */
MIDIINCAPS *midi_in_caps = NULL;
MIDIINCAPS midi_in_mapper_caps;
UINT midi_num_inputs = 0;
MIDIOUTCAPS *midi_out_caps = NULL;
MIDIOUTCAPS midi_out_mapper_caps;
UINT midi_num_outputs = 0;

/* per device info */
typedef struct winmm_info_struct {
    union {
        HMIDISTRM stream;   /* windows handle for stream */
        HMIDIOUT out;       /* windows handle for out calls */
        HMIDIIN in;         /* windows handle for in calls */
    } handle;

    /* midi output messages are sent in these buffers, which are allocated
     * in a round-robin fashion, using next_buffer as an index
     */
    LPMIDIHDR *buffers;     /* pool of buffers for midi in or out data */
    int max_buffers;        /* length of buffers array */
    int buffers_expanded;   /* buffers array expanded for extra msgs? */
    int num_buffers;        /* how many buffers allocated in buffers array */
    int next_buffer;        /* index of next buffer to send */
    HANDLE buffer_signal;   /* used to wait for buffer to become free */
    unsigned long last_time;    /* last output time */
    int first_message;          /* flag: treat first message differently */
    int sysex_mode;             /* middle of sending sysex */
    unsigned long sysex_word;   /* accumulate data when receiving sysex */
    unsigned int sysex_byte_count; /* count how many received */
    LPMIDIHDR hdr;              /* the message accumulating sysex to send */
    unsigned long sync_time;    /* when did we last determine delta? */
    long delta;                 /* difference between stream time and
                                       real time */
    CRITICAL_SECTION lock;      /* prevents reentrant callbacks (input only) */
} winmm_info_node, *winmm_info_type;


/*
=============================================================================
general MIDI device queries
=============================================================================
*/

/* add a device after converting device (product) name to UTF-8 */
static void pm_add_device_w(char *api, WCHAR *device_name, int is_input,
                            int is_virtual, void *descriptor, pm_fns_type dictionary)
{
    char utf8name[4 * MAXPNAMELEN];
    WideCharToMultiByte(CP_UTF8, 0, device_name, -1, 
                        utf8name, 4 * MAXPNAMELEN - 1, NULL, NULL);
    /* ignore errors here -- if pm_descriptor_max is exceeded, 
       some devices will not be accessible. */
    pm_add_device(api, utf8name, is_input, is_virtual, descriptor, dictionary);
}


static void pm_winmm_general_inputs(void)
{
    UINT i;
    WORD wRtn;
    midi_num_inputs = midiInGetNumDevs();
    midi_in_caps = (MIDIINCAPS *) pm_alloc(sizeof(MIDIINCAPS) * 
                                           midi_num_inputs);
    if (midi_in_caps == NULL) {
        /* if you can't open a particular system-level midi interface
         * (such as winmm), we just consider that system or API to be
         * unavailable and move on without reporting an error.
         */
        return;
    }

    for (i = 0; i < midi_num_inputs; i++) {
        wRtn = midiInGetDevCaps(i, (LPMIDIINCAPS) & midi_in_caps[i],
                                sizeof(MIDIINCAPS));
        if (wRtn == MMSYSERR_NOERROR) {
            pm_add_device_w("MMSystem", midi_in_caps[i].szPname, TRUE, FALSE,
                              (void *) (intptr_t) i, &pm_winmm_in_dictionary);
        }
    }
}


static void pm_winmm_mapper_input(void)
{
    WORD wRtn;
    /* Note: if MIDIMAPPER opened as input (documentation implies you
        can, but current system fails to retrieve input mapper
        capabilities) then you still should retrieve some form of
        setup info. */
    wRtn = midiInGetDevCaps((UINT) MIDIMAPPER,
                            (LPMIDIINCAPS) & midi_in_mapper_caps, 
                            sizeof(MIDIINCAPS));
    if (wRtn == MMSYSERR_NOERROR) {
        pm_add_device_w("MMSystem", midi_in_mapper_caps.szPname, TRUE, FALSE,
                        (void *) (intptr_t) MIDIMAPPER, 
                        &pm_winmm_in_dictionary);
    }
}


static void pm_winmm_general_outputs(void)
{
    UINT i;
    DWORD wRtn;
    midi_num_outputs = midiOutGetNumDevs();
    midi_out_caps = pm_alloc(sizeof(MIDIOUTCAPS) * midi_num_outputs);

    if (midi_out_caps == NULL) {
        /* no error is reported -- see pm_winmm_general_inputs */
        return ;
    }

    for (i = 0; i < midi_num_outputs; i++) {
        wRtn = midiOutGetDevCaps(i, (LPMIDIOUTCAPS) & midi_out_caps[i],
                                 sizeof(MIDIOUTCAPS));
        if (wRtn == MMSYSERR_NOERROR) {
            pm_add_device_w("MMSystem", midi_out_caps[i].szPname, FALSE, FALSE,
                            (void *) (intptr_t) i, &pm_winmm_out_dictionary);
        }
    }
}


static void pm_winmm_mapper_output(void)
{
    WORD wRtn;
    /* Note: if MIDIMAPPER opened as output (pseudo MIDI device
        maps device independent messages into device dependant ones,
        via NT midimapper program) you still should get some setup info */
    wRtn = midiOutGetDevCaps((UINT) MIDIMAPPER, (LPMIDIOUTCAPS)
                             & midi_out_mapper_caps, sizeof(MIDIOUTCAPS));
    if (wRtn == MMSYSERR_NOERROR) {
        pm_add_device_w("MMSystem", midi_out_mapper_caps.szPname, FALSE, FALSE,
                        (void *) (intptr_t) MIDIMAPPER, 
                        &pm_winmm_out_dictionary);
    }
}


/*
============================================================================
host error handling
============================================================================
*/

static unsigned int winmm_check_host_error(PmInternal *midi)
{
    return FALSE;
}


/*
=============================================================================
buffer handling
=============================================================================
*/
static MIDIHDR *allocate_buffer(long data_size)
{
    LPMIDIHDR hdr = (LPMIDIHDR) pm_alloc(MIDIHDR_SYSEX_SIZE(data_size));
    MIDIEVENT *evt;
    if (!hdr) return NULL;
    evt = (MIDIEVENT *) (hdr + 1); /* place MIDIEVENT after header */
    hdr->lpData = (LPSTR) evt;
    hdr->dwBufferLength = MIDIHDR_SYSEX_BUFFER_LENGTH(data_size);
    hdr->dwBytesRecorded = 0;
    hdr->dwFlags = 0;
    hdr->dwUser = hdr->dwBufferLength;
    return hdr;
}


static PmError allocate_buffers(winmm_info_type info, long data_size, 
                                long count)
{
    int i;
    /* buffers is an array of count pointers to MIDIHDR/MIDIEVENT struct */
    info->num_buffers = 0; /* in case no memory can be allocated */
    info->buffers = (LPMIDIHDR *) pm_alloc(sizeof(LPMIDIHDR) * count);
    if (!info->buffers) return pmInsufficientMemory;
    info->max_buffers = count;
    for (i = 0; i < count; i++) {
        LPMIDIHDR hdr = allocate_buffer(data_size);
        if (!hdr) { /* free everything allocated so far and return */
            for (i = i - 1; i >= 0; i--) pm_free(info->buffers[i]);
            pm_free(info->buffers);
            info->max_buffers = 0;
            return pmInsufficientMemory;
        }
        info->buffers[i] = hdr; /* this may be NULL if allocation fails */
    }
    info->num_buffers = count;
    return pmNoError;
}


static LPMIDIHDR get_free_output_buffer(PmInternal *midi)
{
    LPMIDIHDR r = NULL;
    winmm_info_type info = (winmm_info_type) midi->api_info;
    while (TRUE) {
        int i;
        for (i = 0; i < info->num_buffers; i++) {
            /* cycle through buffers, modulo info->num_buffers */
            info->next_buffer++;
            if (info->next_buffer >= info->num_buffers) info->next_buffer = 0;
            r = info->buffers[info->next_buffer];
            if ((r->dwFlags & MHDR_PREPARED) == 0) goto found_buffer;
        }
        /* after scanning every buffer and not finding anything, block */
        if (WaitForSingleObject(info->buffer_signal, 1000) == WAIT_TIMEOUT) {
#ifdef MMDEBUG
            printf("PortMidi warning: get_free_output_buffer() "
                   "wait timed out after 1000ms\n");
#endif
            /* if we're trying to send a sysex message, maybe the 
             * message is too big and we need more message buffers.
             * Expand the buffer pool by 128KB using 1024-byte buffers.
             */
            /* first, expand the buffers array if necessary */
            if (!info->buffers_expanded) {
                LPMIDIHDR *new_buffers = (LPMIDIHDR *) pm_alloc(
                               (info->num_buffers + NUM_EXPANSION_BUFFERS) *
                               sizeof(LPMIDIHDR));
                /* if no memory, we could return a no-memory error, but user
                 * probably will be unprepared to deal with it. Maybe the
                 * MIDI driver is temporarily hung so we should just wait.
                 * I don't know the right answer, but waiting is easier.
                 */
                if (!new_buffers) continue;
                /* copy buffers to new_buffers and replace buffers */
                memcpy(new_buffers, info->buffers, 
                       info->num_buffers * sizeof(LPMIDIHDR));
                pm_free(info->buffers);
                info->buffers = new_buffers;
                info->max_buffers = info->num_buffers + NUM_EXPANSION_BUFFERS;
                info->buffers_expanded = TRUE;
            }
            /* next, add one buffer and return it */
            if (info->num_buffers < info->max_buffers) {
                r = allocate_buffer(EXPANSION_BUFFER_LEN);
                /* again, if there's no memory, we may not really be 
                 * dead -- maybe the system is temporarily hung and
                 * we can just wait longer for a message buffer */
                if (!r) continue;
                info->buffers[info->num_buffers++] = r;
                goto found_buffer; /* break out of 2 loops */
            }
            /* else, we've allocated all NUM_EXPANSION_BUFFERS buffers,
             * and we have no free buffers to send. We'll just keep
             * polling to see if any buffers show up.
             */
        }
    }
found_buffer:
    r->dwBytesRecorded = 0;
    /* actual buffer length is saved in dwUser field */
    r->dwBufferLength = (DWORD) r->dwUser;
    return r;
}

/*
============================================================================
begin midi input implementation
============================================================================
*/


static unsigned int allocate_input_buffer(HMIDIIN h, long buffer_len)
{
    LPMIDIHDR hdr = allocate_buffer(buffer_len);
    if (!hdr) return pmInsufficientMemory;
    /* note: pm_hosterror is normally a boolean, but here, we store Win
     * error code. The caller must test the value for nonzero, set
     * pm_hosterror_text, and then set pm_hosterror to TRUE */
    pm_hosterror = midiInPrepareHeader(h, hdr, sizeof(MIDIHDR));
    if (pm_hosterror) {
        pm_free(hdr);
        return pm_hosterror;
    }
    pm_hosterror = midiInAddBuffer(h, hdr, sizeof(MIDIHDR));
    return pm_hosterror;
}


static winmm_info_type winmm_info_create(void)
{
    winmm_info_type info = (winmm_info_type) pm_alloc(sizeof(winmm_info_node));
    info->handle.in = NULL;
    info->handle.out = NULL;
    info->buffers = NULL; /* not used for input */
    info->num_buffers = 0; /* not used for input */
    info->max_buffers = 0; /* not used for input */
    info->buffers_expanded = FALSE; /* not used for input */
    info->next_buffer = 0; /* not used for input */
    info->buffer_signal = 0; /* not used for input */
    info->last_time = 0;
    info->first_message = TRUE; /* not used for input */
    info->sysex_mode = FALSE;
    info->sysex_word = 0;
    info->sysex_byte_count = 0;
    info->hdr = NULL; /* not used for input */
    info->sync_time = 0;
    info->delta = 0;
    return info;
}


static void report_hosterror(LPWCH error_msg)
{
    WideCharToMultiByte(CP_UTF8, 0, error_msg, -1, pm_hosterror_text,
                        sizeof(pm_hosterror_text), NULL, NULL);
    if (pm_hosterror == MMSYSERR_NOMEM) {
        /* add explanation to Window's confusing error message */
        /* if there's room: */
        if (PM_HOST_ERROR_MSG_LEN - strlen(pm_hosterror_text) > 60) {
            strcat_s(pm_hosterror_text, PM_HOST_ERROR_MSG_LEN,
                     " Probably this MIDI device is open "
                     "in another application.");
        }
    }
    pm_hosterror = TRUE;
}


static void report_hosterror_in(void)
{
    WCHAR error_msg[PM_HOST_ERROR_MSG_LEN];
    int err = midiInGetErrorText(pm_hosterror, error_msg,
                                 PM_HOST_ERROR_MSG_LEN);
    assert(err == MMSYSERR_NOERROR);
    report_hosterror(error_msg);
}


static void report_hosterror_out(void)
{
    WCHAR error_msg[PM_HOST_ERROR_MSG_LEN];
    int err = midiOutGetErrorText(pm_hosterror, error_msg,
                                  PM_HOST_ERROR_MSG_LEN);
    assert(err == MMSYSERR_NOERROR);
    report_hosterror(error_msg);
}


static PmError winmm_in_open(PmInternal *midi, void *driverInfo)
{
    DWORD dwDevice;
    int i = midi->device_id;
    int max_sysex_len = midi->buffer_len * 4;
    int num_input_buffers = max_sysex_len / INPUT_SYSEX_LEN;
    winmm_info_type info;

    dwDevice = (DWORD) (intptr_t) pm_descriptors[i].descriptor;

    /* create system dependent device data */
    info = winmm_info_create();
    midi->api_info = info;
    if (!info) goto no_memory;
    /* 4000 is based on Windows documentation -- that's the value used
       in the memory manager. It's small enough that it should not
       hurt performance even if it's not optimal.
     */
    InitializeCriticalSectionAndSpinCount(&info->lock, 4000);
    /* open device */
    pm_hosterror = midiInOpen(
	                &(info->handle.in),  /* input device handle */
	                dwDevice,  /* device ID */
	                (DWORD_PTR) winmm_in_callback,  /* callback address */
	                (DWORD_PTR) midi,  /* callback instance data */
	                CALLBACK_FUNCTION); /* callback is a procedure */
    if (pm_hosterror) goto free_descriptor;

    if (num_input_buffers < MIN_INPUT_BUFFERS)
        num_input_buffers = MIN_INPUT_BUFFERS;
    for (i = 0; i < num_input_buffers; i++) {
        if (allocate_input_buffer(info->handle.in, INPUT_SYSEX_LEN)) {
            /* either pm_hosterror was set, or the proper return code
               is pmInsufficientMemory */
            goto close_device;
        }
    }
    /* start device */
    pm_hosterror = midiInStart(info->handle.in);
    if (!pm_hosterror) {
        return pmNoError;
    }

    /* undo steps leading up to the detected error */

    /* ignore return code (we already have an error to report) */
    midiInReset(info->handle.in);
close_device:
    midiInClose(info->handle.in); /* ignore return code */
free_descriptor:
    midi->api_info = NULL;
    pm_free(info);
no_memory:
    if (pm_hosterror) {
        report_hosterror_in();
        return pmHostError;
    }
    /* if !pm_hosterror, then the error must be pmInsufficientMemory */
    return pmInsufficientMemory;
    /* note: if we return an error code, the device will be
       closed and memory will be freed. It's up to the caller
       to free the parameter midi */
}


/* winmm_in_close -- close an open midi input device */
/*
 * assume midi is non-null (checked by caller)
 */
static PmError winmm_in_close(PmInternal *midi)
{
    winmm_info_type info = (winmm_info_type) midi->api_info;
    if (!info) return pmBadPtr;
    /* device to close */
    if ((pm_hosterror = midiInStop(info->handle.in))) {
        midiInReset(info->handle.in); /* try to reset and close port */
        midiInClose(info->handle.in);
    } else if ((pm_hosterror = midiInReset(info->handle.in))) {
        midiInClose(info->handle.in); /* best effort to close midi port */
    } else {
        pm_hosterror = midiInClose(info->handle.in);
    }
    midi->api_info = NULL;
    DeleteCriticalSection(&info->lock);
    pm_free(info); /* delete */
    if (pm_hosterror) {
        report_hosterror_in();
        return pmHostError;
    }
    return pmNoError;
}


/* Callback function executed via midiInput SW interrupt (via midiInOpen). */
static void FAR PASCAL winmm_in_callback(
    HMIDIIN hMidiIn,       /* midiInput device Handle */
    UINT wMsg,             /* midi msg */
    DWORD_PTR dwInstance,  /* application data */
    DWORD_PTR dwParam1,    /* MIDI data */
    DWORD_PTR dwParam2)    /* device timestamp (wrt most recent midiInStart) */
{
    PmInternal *midi = (PmInternal *) dwInstance;
    winmm_info_type info = (winmm_info_type) midi->api_info;

    /* NOTE: we do not just EnterCriticalSection() here because an
     * MIM_CLOSE message arrives when the port is closed, but then
     * the info->lock has been destroyed.
     */

    switch (wMsg) {
    case MIM_DATA: {
        /* if this callback is reentered with data, we're in trouble. 
         * It's hard to imagine that Microsoft would allow callbacks 
         * to be reentrant -- isn't the model that this is like a 
         * hardware interrupt? -- but I've seen reentrant behavior 
         * using a debugger, so it happens.
         */
        EnterCriticalSection(&info->lock);

        /* dwParam1 is MIDI data received, packed into DWORD w/ 1st byte of
                message LOB;
           dwParam2 is time message received by input device driver, specified
            in [ms] from when midiInStart called.
           each message is expanded to include the status byte */

        if ((dwParam1 & 0x80) == 0) {
            /* not a status byte -- ignore it. This happened running the
               sysex.c test under Win2K with MidiMan USB 1x1 interface,
               but I can't reproduce it. -RBD
             */
            /* printf("non-status byte found\n"); */
        } else { /* data to process */
            PmEvent event;
            if (midi->time_proc)
                dwParam2 = (*midi->time_proc)(midi->time_info);
            event.timestamp = (PmTimestamp)dwParam2;
            event.message = (PmMessage)dwParam1;
            pm_read_short(midi, &event);
        }
        LeaveCriticalSection(&info->lock);
        break;
    }
    case MIM_LONGDATA: {
        MIDIHDR *lpMidiHdr = (MIDIHDR *) dwParam1;
        unsigned char *data = (unsigned char *) lpMidiHdr->lpData;
        unsigned int processed = 0;
        int remaining = lpMidiHdr->dwBytesRecorded;

        EnterCriticalSection(&info->lock);
        /* printf("midi_in_callback -- lpMidiHdr %x, %d bytes, %2x...\n", 
                lpMidiHdr, lpMidiHdr->dwBytesRecorded, *data); */
        if (midi->time_proc)
            dwParam2 = (*midi->time_proc)(midi->time_info);
        /* can there be more than one message in one buffer? */
        /* assume yes and iterate through them */
        pm_read_bytes(midi, data + processed, remaining, (PmTimestamp)dwParam2);

        /* when a device is closed, the pending MIM_LONGDATA buffers are
           returned to this callback with dwBytesRecorded == 0. In this
           case, we do not want to send them back to the interface (if
           we do, the interface will not close, and Windows OS may hang). */
        if (lpMidiHdr->dwBytesRecorded > 0) {
            MMRESULT rslt;
            lpMidiHdr->dwBytesRecorded = 0;
            lpMidiHdr->dwFlags = 0;
			
            /* note: no error checking -- can this actually fail? */
            rslt = midiInPrepareHeader(hMidiIn, lpMidiHdr, sizeof(MIDIHDR));
            assert(rslt == MMSYSERR_NOERROR);
            /* note: I don't think this can fail except possibly for
             * MMSYSERR_NOMEM, but the pain of reporting this
             * unlikely but probably catastrophic error does not seem
             * worth it.
             */
            rslt = midiInAddBuffer(hMidiIn, lpMidiHdr, sizeof(MIDIHDR));
            assert(rslt == MMSYSERR_NOERROR);
            LeaveCriticalSection(&info->lock);
        } else {
            midiInUnprepareHeader(hMidiIn,lpMidiHdr,sizeof(MIDIHDR));
            LeaveCriticalSection(&info->lock);
            pm_free(lpMidiHdr);
        }
        break;
    }
    case MIM_OPEN:
        break;
    case MIM_CLOSE:
        break;
    case MIM_ERROR:
        /* printf("MIM_ERROR\n"); */
        break;
    case MIM_LONGERROR:
        /* printf("MIM_LONGERROR\n"); */
        break;
    default:
        break;
    }
}

/*
===========================================================================
begin midi output implementation
===========================================================================
*/

/* begin helper routines used by midiOutStream interface */

/* add_to_buffer -- adds timestamped short msg to buffer, returns fullp */
static int add_to_buffer(winmm_info_type m, LPMIDIHDR hdr,
                         unsigned long delta, unsigned long msg)
{
    unsigned long *ptr = (unsigned long *)
                         (hdr->lpData + hdr->dwBytesRecorded);
    *ptr++ = delta; /* dwDeltaTime */
    *ptr++ = 0;     /* dwStream */
    *ptr++ = msg;   /* dwEvent */
    hdr->dwBytesRecorded += 3 * sizeof(long);
    /* if the addition of three more words (a message) would extend beyond
       the buffer length, then return TRUE (full)
     */
    return hdr->dwBytesRecorded + 3 * sizeof(long) > hdr->dwBufferLength;
}


static PmTimestamp pm_time_get(winmm_info_type info)
{
    MMTIME mmtime;
    MMRESULT wRtn;
    mmtime.wType = TIME_TICKS;
    mmtime.u.ticks = 0;
    wRtn = midiStreamPosition(info->handle.stream, &mmtime, sizeof(mmtime));
    assert(wRtn == MMSYSERR_NOERROR);
    return mmtime.u.ticks;
}


/* end helper routines used by midiOutStream interface */


static PmError winmm_out_open(PmInternal *midi, void *driverInfo)
{
    DWORD dwDevice;
    int i = midi->device_id;
    winmm_info_type info;
    MIDIPROPTEMPO propdata;
    MIDIPROPTIMEDIV divdata;
    int max_sysex_len = midi->buffer_len * 4;
    int output_buffer_len;
    int num_buffers;
    dwDevice = (DWORD) (intptr_t) pm_descriptors[i].descriptor;

    /* create system dependent device data */
    info = winmm_info_create();
    midi->api_info = info;
    if (!info) goto no_memory;
    /* create a signal */
    info->buffer_signal = CreateEvent(NULL, FALSE, FALSE, NULL);
    /* this should only fail when there are very serious problems */
    assert(info->buffer_signal);
    /* open device */
    if (midi->latency == 0) {
        /* use simple midi out calls */
        pm_hosterror = midiOutOpen(
                (LPHMIDIOUT) & info->handle.out,  /* device Handle */
		        dwDevice,  /* device ID  */
		        /* note: same callback fn as for StreamOpen: */
		        (DWORD_PTR) winmm_streamout_callback, /* callback fn */
		        (DWORD_PTR) midi,  /* callback instance data */
		        CALLBACK_FUNCTION); /* callback type */
    } else {
        /* use stream-based midi output (schedulable in future) */
        pm_hosterror = midiStreamOpen(
	            &info->handle.stream,  /* device Handle */
		        (LPUINT) & dwDevice,  /* device ID pointer */
		        1,  /* reserved, must be 1 */
		        (DWORD_PTR) winmm_streamout_callback,
		        (DWORD_PTR) midi,  /* callback instance data */
		        CALLBACK_FUNCTION);
    }
    if (pm_hosterror != MMSYSERR_NOERROR) {
        goto free_descriptor;
    }

    if (midi->latency == 0) {
        num_buffers = NUM_SIMPLE_SYSEX_BUFFERS;
        output_buffer_len = max_sysex_len / num_buffers;
        if (output_buffer_len < MIN_SIMPLE_SYSEX_LEN)
            output_buffer_len = MIN_SIMPLE_SYSEX_LEN;
    } else {
        num_buffers = max(midi->buffer_len, midi->latency / 2);
        if (num_buffers < MIN_STREAM_BUFFERS)
            num_buffers = MIN_STREAM_BUFFERS;
        output_buffer_len = STREAM_BUFFER_LEN;

        propdata.cbStruct = sizeof(MIDIPROPTEMPO);
        propdata.dwTempo = 480000; /* microseconds per quarter */
        pm_hosterror = midiStreamProperty(info->handle.stream,
                                          (LPBYTE) & propdata,
                                          MIDIPROP_SET | MIDIPROP_TEMPO);
        if (pm_hosterror) goto close_device;

        divdata.cbStruct = sizeof(MIDIPROPTEMPO);
        divdata.dwTimeDiv = 480;   /* divisions per quarter */
        pm_hosterror = midiStreamProperty(info->handle.stream,
                                          (LPBYTE) & divdata,
                                          MIDIPROP_SET | MIDIPROP_TIMEDIV);
        if (pm_hosterror) goto close_device;
    }
    /* allocate buffers */
    if (allocate_buffers(info, output_buffer_len, num_buffers)) 
        goto free_buffers;
    /* start device */
    if (midi->latency != 0) {
        pm_hosterror = midiStreamRestart(info->handle.stream);
        if (pm_hosterror != MMSYSERR_NOERROR) goto free_buffers;
    }
    return pmNoError;

free_buffers:
    /* buffers are freed below by winmm_out_delete */
close_device:
    midiOutClose(info->handle.out);
free_descriptor:
    midi->api_info = NULL;
    winmm_out_delete(midi); /* frees buffers and m */
no_memory:
    if (pm_hosterror) {
        report_hosterror_out();
        return pmHostError;
    }
    return pmInsufficientMemory;
}


/* winmm_out_delete -- carefully free data associated with midi */
/**/
static void winmm_out_delete(PmInternal *midi)
{
    int i;
    /* delete system dependent device data */
    winmm_info_type info = (winmm_info_type) midi->api_info;
    if (info) {
        if (info->buffer_signal) {
            /* don't report errors -- better not to stop cleanup */
            CloseHandle(info->buffer_signal);
        }
        /* if using stream output, free buffers */
        for (i = 0; i < info->num_buffers; i++) {
            if (info->buffers[i]) pm_free(info->buffers[i]);
        }
        info->num_buffers = 0;
        pm_free(info->buffers);
        info->max_buffers = 0;
    }
    midi->api_info = NULL;
    pm_free(info); /* delete */
}


/* see comments for winmm_in_close */
static PmError winmm_out_close(PmInternal *midi)
{
    winmm_info_type info = (winmm_info_type) midi->api_info;
    if (info->handle.out) {
        /* device to close */
        if (midi->latency == 0) {
            pm_hosterror = midiOutClose(info->handle.out);
        } else {
            pm_hosterror = midiStreamClose(info->handle.stream);
        }
        /* regardless of outcome, free memory */
        winmm_out_delete(midi);
    }
    if (pm_hosterror) {
        report_hosterror_out();
        return pmHostError;
    }
    return pmNoError;
}


static PmError winmm_out_abort(PmInternal *midi)
{
    winmm_info_type info = (winmm_info_type) midi->api_info;

    /* only stop output streams */
    if (midi->latency > 0) {
        pm_hosterror = midiStreamStop(info->handle.stream);
        if (pm_hosterror) {
            report_hosterror_out();
            return pmHostError;
        }
    }
    return pmNoError;
}


static PmError winmm_write_flush(PmInternal *midi, PmTimestamp timestamp)
{
    winmm_info_type info = (winmm_info_type) midi->api_info;
    assert(info);
    if (info->hdr) {
        pm_hosterror = midiOutPrepareHeader(info->handle.out, info->hdr, 
                                            sizeof(MIDIHDR));
        if (pm_hosterror) {
            /* do not send message */
        } else if (midi->latency == 0) {
            /* As pointed out by Nigel Brown, 20Sep06, dwBytesRecorded
             * should be zero. This is set in get_free_sysex_buffer(). 
             * The msg length goes in dwBufferLength in spite of what
             * Microsoft documentation says (or doesn't say). */
            info->hdr->dwBufferLength = info->hdr->dwBytesRecorded;
            info->hdr->dwBytesRecorded = 0;
            pm_hosterror = midiOutLongMsg(info->handle.out, info->hdr, 
                                          sizeof(MIDIHDR));
        } else {
            pm_hosterror = midiStreamOut(info->handle.stream, info->hdr, 
                                         sizeof(MIDIHDR));
        }
        midi->fill_base = NULL;
        info->hdr = NULL;
        if (pm_hosterror) {
            report_hosterror_out();
            return pmHostError;
        }
    }
    return pmNoError;
}


static PmError winmm_write_short(PmInternal *midi, PmEvent *event)
{
    winmm_info_type info = (winmm_info_type) midi->api_info;
    PmError rslt = pmNoError;
    assert(info);

    if (midi->latency == 0) { /* use midiOut interface, ignore timestamps */
        pm_hosterror = midiOutShortMsg(info->handle.out, event->message);
        if (pm_hosterror) {
            if (info->hdr) {  /* device disconnect may delete hdr */
                info->hdr->dwFlags = 0; /* release the buffer */
            }
            report_hosterror_out();
            return pmHostError;
        }
    } else {  /* use midiStream interface -- pass data through buffers */
        unsigned long when = event->timestamp;
        unsigned long delta;
        int full;
        if (when == 0) when = midi->now;
        /* when is in real_time; translate to intended stream time */
        when = when + info->delta + midi->latency;
        /* make sure we don't go backward in time */
        if (when < info->last_time) when = info->last_time;
        delta = when - info->last_time;
        info->last_time = when;
        /* before we insert any data, we must have a buffer */
        if (info->hdr == NULL) {
            /* stream interface: buffers allocated when stream is opened */
            info->hdr = get_free_output_buffer(midi);
        }
        full = add_to_buffer(info, info->hdr, delta, event->message);
        /* note: winmm_write_flush sets pm_hosterror etc. on host error */
        if (full) rslt = winmm_write_flush(midi, when);
    }
    return rslt;
}

#define winmm_begin_sysex winmm_write_flush
#ifndef winmm_begin_sysex
static PmError winmm_begin_sysex(PmInternal *midi, PmTimestamp timestamp)
{
    winmm_info_type m = (winmm_info_type) midi->api_info;
    PmError rslt = pmNoError;

    if (midi->latency == 0) {
        /* do nothing -- it's handled in winmm_write_byte */
    } else {
        /* sysex expects an empty sysex buffer, so send whatever is here */
        rslt = winmm_write_flush(midi);
    }
    return rslt;
}
#endif

static PmError winmm_end_sysex(PmInternal *midi, PmTimestamp timestamp)
{
    /* could check for callback_error here, but I haven't checked
     * what happens if we exit early and don't finish the sysex msg
     * and clean up
     */
    winmm_info_type info = (winmm_info_type) midi->api_info;
    PmError rslt = pmNoError;
    LPMIDIHDR hdr = info->hdr;
    if (!hdr) return rslt; /* something bad happened earlier,
            do not report an error because it would have been 
            reported (at least) once already */
    /* a(n old) version of MIDI YOKE requires a zero byte after
     * the sysex message, but do not increment dwBytesRecorded: */
    hdr->lpData[hdr->dwBytesRecorded] = 0;
    if (midi->latency == 0) {
#ifdef DEBUG_PRINT_BEFORE_SENDING_SYSEX
        /* DEBUG CODE: */
        { int i; int len = info->hdr->dwBufferLength;
          printf("OutLongMsg %d ", len);
          for (i = 0; i < len; i++) {
              printf("%2x ", (unsigned char) (info->hdr->lpData[i]));
          }
        }
#endif
    } else {
        /* Using stream interface. There are accumulated bytes in info->hdr
           to send using midiStreamOut
         */
        /* add bytes recorded to MIDIEVENT length, but don't
           count the MIDIEVENT data (3 longs) */
        MIDIEVENT *evt = (MIDIEVENT *) (hdr->lpData);
        evt->dwEvent += hdr->dwBytesRecorded - 3 * sizeof(long);
        /* round up BytesRecorded to multiple of 4 */
        hdr->dwBytesRecorded = (hdr->dwBytesRecorded + 3) & ~3;
    }
    rslt = winmm_write_flush(midi, timestamp);
    return rslt;
}


static PmError winmm_write_byte(PmInternal *midi, unsigned char byte,
                                PmTimestamp timestamp)
{
    /* write a sysex byte */
    PmError rslt = pmNoError;
    winmm_info_type info = (winmm_info_type) midi->api_info;
    LPMIDIHDR hdr = info->hdr;
    unsigned char *msg_buffer;
    assert(info);
    if (!hdr) {
        info->hdr = hdr = get_free_output_buffer(midi);
        assert(hdr);
        midi->fill_base = (unsigned char *) info->hdr->lpData;
        midi->fill_offset_ptr = (uint32_t *) &(hdr->dwBytesRecorded);
        /* when buffer fills, Pm_WriteSysEx will revert to calling
         * pmwin_write_byte, which expect to have space, so leave
         * one byte free for pmwin_write_byte. Leave another byte
         * of space for zero after message to make early version of 
         * MIDI YOKE driver happy -- therefore dwBufferLength - 2 */
        midi->fill_length = hdr->dwBufferLength - 2;
        if (midi->latency != 0) {
            unsigned long when = (unsigned long) timestamp;
            unsigned long delta;
            unsigned long *ptr;
            if (when == 0) when = midi->now;
            /* when is in real_time; translate to intended stream time */
            when = when + info->delta + midi->latency;
            /* make sure we don't go backward in time */
            if (when < info->last_time) when = info->last_time;
            delta = when - info->last_time;
            info->last_time = when;

            ptr = (unsigned long *) hdr->lpData;
            *ptr++ = delta;
            *ptr++ = 0;
            *ptr = MEVT_F_LONG;
            hdr->dwBytesRecorded = 3 * sizeof(long);
            /* data will be added at an offset of dwBytesRecorded ... */
        }
    }
    /* add the data byte */
    msg_buffer = (unsigned char *) (hdr->lpData);
    msg_buffer[hdr->dwBytesRecorded++] = byte;

    /* see if buffer is full, leave one byte extra for pad */
    if (hdr->dwBytesRecorded >= hdr->dwBufferLength - 1) {
        /* write what we've got and continue */
        rslt = winmm_end_sysex(midi, timestamp); 
    }
    return rslt;
}


static PmTimestamp winmm_synchronize(PmInternal *midi)
{
    winmm_info_type info;
    unsigned long pm_stream_time_2;
    unsigned long real_time;
    unsigned long pm_stream_time;

    /* only synchronize if we are using stream interface */
    if (midi->latency == 0) return 0;

    /* figure out the time */
    info = (winmm_info_type) midi->api_info;
    pm_stream_time_2 = pm_time_get(info);

    do {
        /* read real_time between two reads of stream time */
        pm_stream_time = pm_stream_time_2;
        real_time = (*midi->time_proc)(midi->time_info);
        pm_stream_time_2 = pm_time_get(info);
        /* repeat if more than 1ms elapsed */
    } while (pm_stream_time_2 > pm_stream_time + 1);
    info->delta = pm_stream_time - real_time;
    info->sync_time = real_time;
    return real_time;
}


/* winmm_streamout_callback -- unprepare (free) buffer header */
static void CALLBACK winmm_streamout_callback(HMIDIOUT hmo, UINT wMsg,
        DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    PmInternal *midi = (PmInternal *) dwInstance;
    winmm_info_type info = (winmm_info_type) midi->api_info;
    LPMIDIHDR hdr = (LPMIDIHDR) dwParam1;
    int err;

    /* Even if an error is pending, I think we should unprepare msgs and
       signal their arrival
     */
    /* printf("streamout_callback: hdr %x, wMsg %x, MOM_DONE %x\n", 
           hdr, wMsg, MOM_DONE); */
    if (wMsg == MOM_DONE) {
        MMRESULT ret = midiOutUnprepareHeader(info->handle.out, hdr, 
                                              sizeof(MIDIHDR));
        assert(ret == MMSYSERR_NOERROR);
    } else if (wMsg == MOM_CLOSE) {
        /* The streaming API gets a callback when the device is closed.
         * The non-streaming API gets a callback when the device is
         * removed or closed. It is misleading to set is_removed when
         * the device is closed normally, but in that case, midi itself
         * will be freed immediately, so there should be no way to
         * observe is_removed == TRUE. On the other hand, if the device
         * is removed, setting is_removed will cause PortMidi to return
         * the pmDeviceRemoved error on attempts to output to the device.
         *     In the case of normal closing, due to midiOutClose(), 
         * the call below is reentrant (!), but for some reason this does
         * not cause an error or infinite recursion, so we are not taking
         * any precautions to flag midi as "in the process of closing."
         */
        midi->is_removed = TRUE;
        midiOutClose(info->handle.out);
    }
    /* signal client in case it is blocked waiting for buffer */
    err = SetEvent(info->buffer_signal);
    assert(err); /* false -> error */
}


/*
===========================================================================
begin exported functions
===========================================================================
*/

#define winmm_in_abort pm_fail_fn
pm_fns_node pm_winmm_in_dictionary = {
                                         none_write_short,
                                         none_sysex,
                                         none_sysex,
                                         none_write_byte,
                                         none_write_short,
                                         none_write_flush,
                                         winmm_synchronize,
                                         winmm_in_open,
                                         winmm_in_abort,
                                         winmm_in_close,
                                         success_poll,
                                         winmm_check_host_error
                                     };

pm_fns_node pm_winmm_out_dictionary = {
                                          winmm_write_short,
                                          winmm_begin_sysex,
                                          winmm_end_sysex,
                                          winmm_write_byte,
            /* short realtime message: */ winmm_write_short,  
                                          winmm_write_flush,
                                          winmm_synchronize,
                                          winmm_out_open,
                                          winmm_out_abort,
                                          winmm_out_close,
                                          none_poll,
                                          winmm_check_host_error
                                      };


/* initialize winmm interface. Note that if there is something wrong
   with winmm (e.g. it is not supported or installed), it is not an
   error. We should simply return without having added any devices to
   the table. Hence, no error code is returned. Furthermore, this init
   code is called along with every other supported interface, so the
   user would have a very hard time figuring out what hardware and API
   generated the error. Finally, it would add complexity to pmwin.c to
   remember where the error code came from in order to convert to text.
 */
void pm_winmm_init( void )
{
    pm_winmm_mapper_input();
    pm_winmm_mapper_output();
    pm_winmm_general_inputs();
    pm_winmm_general_outputs();
}


/* no error codes are returned, even if errors are encountered, because
   there is probably nothing the user could do (e.g. it would be an error
   to retry.
 */
void pm_winmm_term( void )
{
    int i;
#ifdef MMDEBUG
    int doneAny = 0;
    printf("pm_winmm_term called\n");
#endif
    for (i = 0; i < pm_descriptor_len; i++) {
        PmInternal *midi = pm_descriptors[i].pm_internal;
        if (midi) {
            winmm_info_type info = (winmm_info_type) midi->api_info;
            if (info->handle.out) {
                /* close next open device*/
#ifdef MMDEBUG
                if (doneAny == 0) {
                    printf("begin closing open devices...\n");
                    doneAny = 1;
                }
#endif
                /* close all open ports */
                (*midi->dictionary->close)(midi);
            }
        }
    }
    if (midi_in_caps) {
        pm_free(midi_in_caps);
        midi_in_caps = NULL;
    }
    if (midi_out_caps) {
        pm_free(midi_out_caps);
        midi_out_caps = NULL;
    }
#ifdef MMDEBUG
    if (doneAny) {
        printf("warning: devices were left open. They have been closed.\n");
    }
    printf("pm_winmm_term exiting\n");
#endif
    pm_descriptor_len = 0;
}
