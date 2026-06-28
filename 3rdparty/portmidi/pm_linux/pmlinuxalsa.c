/*
 * pmlinuxalsa.c -- system specific definitions
 * 
 * written by:
 *  Roger Dannenberg (port to Alsa 0.9.x)
 *  Clemens Ladisch (provided code examples and invaluable consulting)
 *  Jason Cohen, Rico Colon, Matt Filippone (Alsa 0.5.x implementation)
 */ 

/* omit this code if PMALSA is not defined -- this mechanism allows
 * selection of different MIDI interfaces (at compile time).
 */
#ifdef PMALSA

#include "stdlib.h"
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"
#include "pmlinuxalsa.h"
#include "string.h"
#include "porttime.h"

#include <alsa/asoundlib.h>

/* I used many print statements to debug this code. I left them in the
 * source, and you can turn them on by changing false to true below:
 */
#define VERBOSE_ON 0
#define VERBOSE if (VERBOSE_ON)

#define MIDI_SYSEX      0xf0
#define MIDI_EOX        0xf7

#if SND_LIB_MAJOR == 0 && SND_LIB_MINOR < 9
#error needs ALSA 0.9.0 or later
#endif

/* to store client/port in the device descriptor */
#define MAKE_DESCRIPTOR(client, port) ((void*)(long)(((client) << 8) | (port)))
#define GET_DESCRIPTOR_CLIENT(info) ((((long)(info)) >> 8) & 0xff)
#define GET_DESCRIPTOR_PORT(info) (((long)(info)) & 0xff)

#define BYTE unsigned char

extern pm_fns_node pm_linuxalsa_in_dictionary;
extern pm_fns_node pm_linuxalsa_out_dictionary;

static snd_seq_t *seq = NULL; /* all input comes here, 
                                 output queue allocated on seq */
static int queue, queue_used; /* one for all ports, reference counted */

#define PORT_IS_CLOSED -999999

typedef struct alsa_info_struct {
    int is_virtual;
    int client;
    int port;
    int this_port;
    int in_sysex;
    snd_midi_event_t *parser;
} alsa_info_node, *alsa_info_type;


/* get_alsa_error_text -- copy error text to potentially short string */
/**/
static void get_alsa_error_text(char *msg, int len, int err)
{
    int errlen = strlen(snd_strerror(err));
    if (errlen > 0 && errlen < len) {
        strcpy(msg, snd_strerror(err));
    } else if (len > 20) {
        sprintf(msg, "Alsa error %d", err);
    } else {
        msg[0] = 0;
    }
}


static PmError check_hosterror(int err)
{
    if (err < 0) {
        get_alsa_error_text(pm_hosterror_text, PM_HOST_ERROR_MSG_LEN, err);
        pm_hosterror = TRUE;
        return pmHostError;
    }
    return pmNoError;
}


/* queue is shared by both input and output, reference counted */
static PmError alsa_use_queue(void)
{
    int err = 0;
    if (queue_used == 0) {
        snd_seq_queue_tempo_t *tempo;

        queue = snd_seq_alloc_queue(seq);
        if (queue < 0) {
            return check_hosterror(queue);
        }
        snd_seq_queue_tempo_alloca(&tempo);
        snd_seq_queue_tempo_set_tempo(tempo, 480000);
        snd_seq_queue_tempo_set_ppq(tempo, 480);
        err = snd_seq_set_queue_tempo(seq, queue, tempo);
        if (err < 0) {
            return check_hosterror(err);
        }
        snd_seq_start_queue(seq, queue, NULL);
        snd_seq_drain_output(seq);
    }
    ++queue_used;
    return pmNoError;
}


static void alsa_unuse_queue(void)
{
    if (--queue_used == 0) {
        snd_seq_stop_queue(seq, queue, NULL);
        snd_seq_drain_output(seq);
        snd_seq_free_queue(seq, queue);
        VERBOSE printf("queue freed\n");
    }
}


/* midi_message_length -- how many bytes in a message? */
static int midi_message_length(PmMessage message)
{
    message &= 0xff;
    if (message < 0x80) {
        return 0;
    } else if (message < 0xf0) {
        static const int length[] = {3, 3, 3, 3, 2, 2, 3};
        return length[(message - 0x80) >> 4];
    } else {
        static const int length[] = {
            -1, 2, 3, 2, 0, 0, 1, -1, 1, 0, 1, 1, 1, 0, 1, 1};
        return length[message - 0xf0];
    }
}


static alsa_info_type alsa_info_create(int client_port, long id, int is_virtual)
{
    alsa_info_type info = (alsa_info_type) pm_alloc(sizeof(alsa_info_node));
    info->is_virtual = is_virtual;
    info->this_port = id;
    info->client = GET_DESCRIPTOR_CLIENT(client_port);
    info->port = GET_DESCRIPTOR_PORT(client_port);
    info->in_sysex = 0;
    return info;
}    


/* search system dependent extra parameters for string */
static const char *get_sysdep_name(enum PmSysDepPropertyKey key,
                                   PmSysDepInfo *info)
{
    /* the version where all current properties were introduced is 210 */
    if (info && info->structVersion >= 210) {
        int i;
        for (i = 0; i < info->length; i++) {  /* search for key */
            if (info->properties[i].key == key) {
                return info->properties[i].value;
            }
        }
    }
    return NULL;
}


static void maybe_set_client_name(PmSysDepInfo *driverInfo)
{
    /* make sure seq is created and we have info */
    if (!seq || !driverInfo) {
        return;
    }
    
    const char *client_name = get_sysdep_name(pmKeyAlsaClientName,
                                              (PmSysDepInfo *) driverInfo);
    if (client_name) {
        snd_seq_set_client_name(seq, client_name);
        VERBOSE printf("maybe_set_client_name set client to %s\n", client_name);
    }
}    


static PmError alsa_out_open(PmInternal *midi, void *driverInfo) 
{
    int id = midi->device_id;
    void *client_port = pm_descriptors[id].descriptor;
    alsa_info_type ainfo = alsa_info_create((long) client_port, id,
                                            pm_descriptors[id].pub.is_virtual);
    snd_seq_port_info_t *pinfo;
    int err = 0;
    int using_the_queue = 0;

    if (!ainfo) return pmInsufficientMemory;
    midi->api_info = ainfo;

    snd_seq_port_info_alloca(&pinfo);
    if (!ainfo->is_virtual) {
        snd_seq_port_info_set_port(pinfo, id);
        snd_seq_port_info_set_capability(pinfo, SND_SEQ_PORT_CAP_WRITE |
                SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ);
        snd_seq_port_info_set_type(pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC | 
                                          SND_SEQ_PORT_TYPE_APPLICATION);
        const char *port_name = get_sysdep_name(pmKeyAlsaPortName,
                                                (PmSysDepInfo *) driverInfo);
        if (port_name) {
            snd_seq_port_info_set_name(pinfo, port_name);
        }
        snd_seq_port_info_set_port_specified(pinfo, 1);

        err = snd_seq_create_port(seq, pinfo);
        if (err < 0) goto free_ainfo;

    }
    
    err = snd_midi_event_new(PM_DEFAULT_SYSEX_BUFFER_SIZE, &ainfo->parser);
    if (err < 0) goto free_this_port;

    if (midi->latency > 0) { /* must delay output using a queue */
        err = alsa_use_queue();
        if (err < 0) goto free_parser;
        using_the_queue++;
    }

    if (!ainfo->is_virtual) {
        err = snd_seq_connect_to(seq, ainfo->this_port, ainfo->client,
                                 ainfo->port);
        if (err < 0) goto unuse_queue;  /* clean up and return on error */
    }

    maybe_set_client_name(driverInfo);

    return pmNoError;

 unuse_queue:
    if (using_the_queue > 0)  /* only for latency>0 case */
        alsa_unuse_queue();
 free_parser:
    snd_midi_event_free(ainfo->parser);
 free_this_port:
    snd_seq_delete_port(seq, ainfo->this_port);
 free_ainfo:
    pm_free(ainfo);
    return check_hosterror(err);
}
    

static PmError alsa_write_byte(PmInternal *midi, unsigned char byte, 
                               PmTimestamp timestamp)
{
    alsa_info_type info = (alsa_info_type) midi->api_info;
    snd_seq_event_t ev;
    int err = 0;

    snd_seq_ev_clear(&ev);
    if (snd_midi_event_encode_byte(info->parser, byte, &ev) == 1) {
        if (info->is_virtual) {
            snd_seq_ev_set_subs(&ev);
        } else {
            snd_seq_ev_set_dest(&ev, info->client, info->port);
        }
        snd_seq_ev_set_source(&ev, info->this_port);
        if (midi->latency > 0) {
            /* compute relative time of event = timestamp - now + latency */
            PmTimestamp now = (midi->time_proc ? 
                               midi->time_proc(midi->time_info) : 
                               Pt_Time());
            int when = timestamp;
            /* if timestamp is zero, send immediately */
            /* otherwise compute time delay and use delay if positive */
            if (when == 0) when = now;
            when = (when - now) + midi->latency;
            if (when < 0) when = 0;
            VERBOSE printf("timestamp %d now %d latency %d, ", 
                           (int) timestamp, (int) now, midi->latency);
            VERBOSE printf("scheduling event after %d\n", when);
            /* message is sent in relative ticks, where 1 tick = 1 ms */
            snd_seq_ev_schedule_tick(&ev, queue, 1, when);
            /* NOTE: for cases where the user does not supply a time function,
               we could optimize the code by not starting Pt_Time and using
               the alsa tick time instead. I didn't do this because it would
               entail changing the queue management to start the queue tick
               count when PortMidi is initialized and keep it running until
               PortMidi is terminated. (This should be simple, but it's not
               how the code works now.) -RBD */
        } else { /* send event out without queueing */
            VERBOSE printf("direct\n");
            /* ev.queue = SND_SEQ_QUEUE_DIRECT;
               ev.dest.client = SND_SEQ_ADDRESS_SUBSCRIBERS; */
            snd_seq_ev_set_direct(&ev);
        }
        VERBOSE printf("sending event, timestamp %d (%d+%dns) (%s, %s)\n",
                       ev.time.tick, ev.time.time.tv_sec, ev.time.time.tv_nsec,
                       (ev.flags & SND_SEQ_TIME_STAMP_MASK ? "real" : "tick"),
                       (ev.flags & SND_SEQ_TIME_MODE_MASK ? "rel" : "abs"));
        err = snd_seq_event_output(seq, &ev);
    }
    return check_hosterror(err);
}


static PmError alsa_out_close(PmInternal *midi)
{
    alsa_info_type info = (alsa_info_type) midi->api_info;
    int err = 0;
    int error2 = 0;
    if (!info) return pmBadPtr;

    if (info->this_port != PORT_IS_CLOSED) {
        if (!info->is_virtual) {
            err = snd_seq_disconnect_to(seq, info->this_port,
                                        info->client, info->port);
            /* even if there was an error, we still try to delete the port */
            error2 = snd_seq_delete_port(seq, info->this_port);

            if (!err) {  /* retain original error if there was one */
                err = error2; /* otherwise, use port delete status */
            }
        }
    }
    if (midi->latency > 0) alsa_unuse_queue();
    snd_midi_event_free(info->parser);
    midi->api_info = NULL; /* destroy the pointer to signify "closed" */
    pm_free(info);
    return check_hosterror(err);
}


static PmError alsa_create_virtual(int is_input, const char *name,
                                   void *device_info)
{
    snd_seq_port_info_t *pinfo;
    int err;
    int client, port;
    
    /* we need the id to set the port. */
    PmDeviceID id = pm_add_device("ALSA", name, is_input, TRUE, NULL,
                                  (is_input ? &pm_linuxalsa_in_dictionary :
                                              &pm_linuxalsa_out_dictionary));
    if (id < 0) { /* error -- out of memory? */
        return id;
    }
    snd_seq_port_info_alloca(&pinfo);
    snd_seq_port_info_set_capability(pinfo,
            (is_input ? SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE :
                        SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ));
    snd_seq_port_info_set_type(pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                     SND_SEQ_PORT_TYPE_APPLICATION);
    snd_seq_port_info_set_name(pinfo, name);
    snd_seq_port_info_set_port(pinfo, id);
    snd_seq_port_info_set_port_specified(pinfo, 1);
    /* next 3 lines needed to generate timestamp - PaulLiu */
    snd_seq_port_info_set_timestamping(pinfo, 1);
    snd_seq_port_info_set_timestamp_real(pinfo, 0);
    snd_seq_port_info_set_timestamp_queue(pinfo, queue);

    err = snd_seq_create_port(seq, pinfo);
    if (err < 0) {
        pm_undo_add_device(id);
        return check_hosterror(err);
    }

    client = snd_seq_port_info_get_client(pinfo);
    port = snd_seq_port_info_get_port(pinfo);
    pm_descriptors[id].descriptor = MAKE_DESCRIPTOR(client, port);
    return id;
}


 static PmError alsa_delete_virtual(PmDeviceID id)
 {
     int err = snd_seq_delete_port(seq, id);
     return check_hosterror(err);
 }
 

static PmError alsa_in_open(PmInternal *midi, void *driverInfo)
{
    int id = midi->device_id;
    void *client_port = pm_descriptors[id].descriptor;
    alsa_info_type ainfo = alsa_info_create((long) client_port, id,
                                            pm_descriptors[id].pub.is_virtual);
    snd_seq_port_info_t *pinfo;
    snd_seq_port_subscribe_t *sub;
    snd_seq_addr_t addr;
    int err = 0;
    int is_virtual = pm_descriptors[id].pub.is_virtual;
    
    if (!ainfo) return pmInsufficientMemory;
    midi->api_info = ainfo;

    err = alsa_use_queue();
    if (err < 0) goto free_ainfo;

    snd_seq_port_info_alloca(&pinfo);
    if (is_virtual) {
        ainfo->is_virtual = TRUE;
        if (snd_seq_get_port_info(seq, ainfo->port, pinfo)) {
            pinfo = NULL;
            goto free_ainfo;
        }
    } else {
        /* create a port for this alsa client (seq) where the port
           number matches the portmidi device ID of the input device */
        snd_seq_port_info_set_port(pinfo, id);
        snd_seq_port_info_set_capability(pinfo, SND_SEQ_PORT_CAP_WRITE |
                SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_WRITE);

        snd_seq_port_info_set_type(pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC | 
                                          SND_SEQ_PORT_TYPE_APPLICATION);
        snd_seq_port_info_set_port_specified(pinfo, 1);
        
        const char *port_name = get_sysdep_name(pmKeyAlsaPortName,
                                                (PmSysDepInfo *) driverInfo);
        if (port_name) {
            snd_seq_port_info_set_name(pinfo, port_name);
        }
        
        err = snd_seq_create_port(seq, pinfo);
        if (err < 0) goto free_queue;

        /* forward messages from input to this alsa client, so this
         * alsa client is the destination, and the destination port is the
         * port we just created using the device ID as port number
         */
        snd_seq_port_subscribe_alloca(&sub);
        addr.client = snd_seq_client_id(seq);
        addr.port = ainfo->this_port;
        snd_seq_port_subscribe_set_dest(sub, &addr);

        /* forward from the sender which is the device named by 
           client and port */
        addr.client = ainfo->client;
        addr.port = ainfo->port;
        snd_seq_port_subscribe_set_sender(sub, &addr);
        snd_seq_port_subscribe_set_time_update(sub, 1);
        /* this doesn't seem to work: messages come in with real timestamps */
        snd_seq_port_subscribe_set_time_real(sub, 0);
        err = snd_seq_subscribe_port(seq, sub);
        if (err < 0) goto free_this_port;  /* clean up and return on error */
    }

    maybe_set_client_name(driverInfo);

    return pmNoError;
 free_this_port:
    snd_seq_delete_port(seq, ainfo->this_port);
 free_queue:
    alsa_unuse_queue();
 free_ainfo:
    pm_free(ainfo);
    return check_hosterror(err);
}

static PmError alsa_in_close(PmInternal *midi)
{
    int err = 0;
    alsa_info_type info = (alsa_info_type) midi->api_info;
    if (!info) return pmBadPtr;
    /* virtual ports stay open because the represent devices */
    if (!info->is_virtual && info->this_port != PORT_IS_CLOSED) {
        err = snd_seq_delete_port(seq, info->this_port);
    }
    alsa_unuse_queue();
    midi->api_info = NULL;
    pm_free(info);
    return check_hosterror(err);
}
        

static PmError alsa_abort(PmInternal *midi)
{
    /* NOTE: ALSA documentation is vague. This is supposed to 
     * remove any pending output messages. If you can test and 
     * confirm this code is correct, please update this comment. -RBD
     */
    /* Unfortunately, I can't even compile it -- my ALSA version 
     * does not implement snd_seq_remove_events_t, so this does
     * not compile. I'll try again, but it looks like I'll need to
     * upgrade my entire Linux OS -RBD
     */
    /*
    info_type info = (info_type) midi->api_info;
    snd_seq_remove_events_t info;
    snd_seq_addr_t addr;
    addr.client = info->client;
    addr.port = info->port;
    snd_seq_remove_events_set_dest(&info, &addr);
    snd_seq_remove_events_set_condition(&info, SND_SEQ_REMOVE_DEST);
    pm_hosterror = snd_seq_remove_events(seq, &info);
    if (pm_hosterror) {
        get_alsa_error_text(pm_hosterror_text, PM_HOST_ERROR_MSG_LEN, 
                            pm_hosterror);
        return pmHostError;
    }
    */
    printf("WARNING: alsa_abort not implemented\n");
    return pmNoError;
}


static PmError alsa_write_flush(PmInternal *midi, PmTimestamp timestamp)
{
    int err;
    alsa_info_type info = (alsa_info_type) midi->api_info;
    if (!info) return pmBadPtr;
    VERBOSE printf("snd_seq_drain_output: %p\n", seq);
    err = snd_seq_drain_output(seq);
    return check_hosterror(err);
}


static PmError alsa_write_short(PmInternal *midi, PmEvent *event)
{
    int bytes = midi_message_length(event->message);
    PmMessage msg = event->message;
    int i;
    alsa_info_type info = (alsa_info_type) midi->api_info;
    if (!info) return pmBadPtr;
    for (i = 0; i < bytes; i++) {
        unsigned char byte = msg;
        VERBOSE printf("sending 0x%x\n", byte);
        alsa_write_byte(midi, byte, event->timestamp);
        if (pm_hosterror) break;
        msg >>= 8; /* shift next byte into position */
    }
    if (pm_hosterror) return pmHostError;
    return pmNoError;
}


/* alsa_sysex -- implements begin_sysex and end_sysex */
PmError alsa_sysex(PmInternal *midi, PmTimestamp timestamp) {
    return pmNoError;
}


static PmTimestamp alsa_synchronize(PmInternal *midi)
{
    return 0; /* linux implementation does not use this synchronize function */
    /* Apparently, Alsa data is relative to the time you send it, and there
       is no reference. If this is true, this is a serious shortcoming of
       Alsa. If not true, then PortMidi has a serious shortcoming -- it 
       should be scheduling relative to Alsa's time reference. */
}


static void handle_event(snd_seq_event_t *ev)
{
    int device_id = ev->dest.port;

    assert(device_id >= 0 && device_id < pm_descriptor_len);
    assert(pm_descriptors[device_id].pub.name);

    PmInternal *midi = pm_descriptors[device_id].pm_internal;
    /* There is a race condition when closing a device and
       continuing to poll other open devices. The closed device may
       have outstanding events from before the close operation.
    */
    if (!midi) {
        return;
    }
    PmEvent pm_ev;

    /* Copilot says: "snd_seq_event_input() will not return events for a
     * MIDI output device." However, this does not seem to be true, e.g.,
     * for MIDIFLEX 4 on Arch Linux, so we want to check and ignore the
     * call if this is an output device.
     */
    if (!midi->is_input) {
        return;
    }

    if (!midi->time_proc) {  /* extra sanity check */
        return;
    }

    PmTimestamp timestamp = midi->time_proc(midi->time_info);

    /* time stamp should be in ticks, using our queue where 1 tick = 1ms */
    /* assert((ev->flags & SND_SEQ_TIME_STAMP_MASK) == SND_SEQ_TIME_STAMP_TICK);
       Currently, event timestamp is ignored. See long note below. 
    */
    VERBOSE {
        /* translate time to time_proc basis */
        snd_seq_queue_status_t *queue_status;
        snd_seq_queue_status_alloca(&queue_status);
        snd_seq_get_queue_status(seq, queue, queue_status);
        printf("handle_event: alsa_now %d, "
               "event timestamp %d (%d+%dns) (%s, %s)\n",
               snd_seq_queue_status_get_tick_time(queue_status),
               ev->time.tick, ev->time.time.tv_sec, ev->time.time.tv_nsec,
               (ev->flags & SND_SEQ_TIME_STAMP_MASK ? "real" : "tick"),
               (ev->flags & SND_SEQ_TIME_MODE_MASK ? "rel" : "abs"));
        /* OLD: portmidi timestamp is (now - alsa_now) + alsa_timestamp */
        /* timestamp = (*time_proc)(midi->time_info) + ev->time.tick -
                       snd_seq_queue_status_get_tick_time(queue_status); */
    }
    /* CURRENT: portmidi timestamp is "now". In a test, timestamps from
     * hardware (MIDI over USB) were timestamped with the current ALSA
     * time (snd_seq_queue_status_get_tick_time) and flags indicating 
     * absolute ticks, but timestamps from another application's virtual
     * port, sent direct with 0 absolute ticks, were received with a 
     * large value that is apparently the time since the start time of
     * the other application. Without any reference to our local time,
     * this seems useless. PortMidi is supposed to return the local
     * PortMidi time of the arrival of the message, so the best we can
     * do is set the timestamp to our local clock.  This seems to be a
     * design flaw in ALSA -- I pointed this out a decade ago, but if
     * there is a workaround, I'd still like to know. Maybe there is a
     * way to use absolute real time and maybe that's sharable across
     * applications by referencing the system time?
     */
    pm_ev.timestamp = timestamp;
    switch (ev->type) {
    case SND_SEQ_EVENT_NOTEON:
        pm_ev.message = Pm_Message(0x90 | ev->data.note.channel,
                                   ev->data.note.note & 0x7f,
                                   ev->data.note.velocity & 0x7f);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        pm_ev.message = Pm_Message(0x80 | ev->data.note.channel,
                                   ev->data.note.note & 0x7f,
                                   ev->data.note.velocity & 0x7f);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_KEYPRESS:
        pm_ev.message = Pm_Message(0xa0 | ev->data.note.channel,
                                   ev->data.note.note & 0x7f,
                                   ev->data.note.velocity & 0x7f);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_CONTROLLER:
        pm_ev.message = Pm_Message(0xb0 | ev->data.note.channel,
                                   ev->data.control.param & 0x7f,
                                   ev->data.control.value & 0x7f);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_PGMCHANGE:
        pm_ev.message = Pm_Message(0xc0 | ev->data.note.channel,
                                   ev->data.control.value & 0x7f, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_CHANPRESS:
        pm_ev.message = Pm_Message(0xd0 | ev->data.note.channel,
                                   ev->data.control.value & 0x7f, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_PITCHBEND:
        pm_ev.message = Pm_Message(0xe0 | ev->data.note.channel,
                            (ev->data.control.value + 0x2000) & 0x7f,
                            ((ev->data.control.value + 0x2000) >> 7) & 0x7f);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_CONTROL14:
        if (ev->data.control.param < 0x20) {
            pm_ev.message = Pm_Message(0xb0 | ev->data.note.channel,
                                       ev->data.control.param,
                                       (ev->data.control.value >> 7) & 0x7f);
            pm_read_short(midi, &pm_ev);
            pm_ev.message = Pm_Message(0xb0 | ev->data.note.channel,
                                       ev->data.control.param + 0x20,
                                       ev->data.control.value & 0x7f);
            pm_read_short(midi, &pm_ev);
        } else {
            pm_ev.message = Pm_Message(0xb0 | ev->data.note.channel,
                                       ev->data.control.param & 0x7f,
                                       ev->data.control.value & 0x7f);

            pm_read_short(midi, &pm_ev);
        }
        break;
    case SND_SEQ_EVENT_SONGPOS:
        pm_ev.message = Pm_Message(0xf2,
                                   ev->data.control.value & 0x7f,
                                   (ev->data.control.value >> 7) & 0x7f);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_SONGSEL:
        pm_ev.message = Pm_Message(0xf3,
                                   ev->data.control.value & 0x7f, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_QFRAME:
        pm_ev.message = Pm_Message(0xf1,
                                   ev->data.control.value & 0x7f, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_START:
        pm_ev.message = Pm_Message(0xfa, 0, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_CONTINUE:
        pm_ev.message = Pm_Message(0xfb, 0, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_STOP:
        pm_ev.message = Pm_Message(0xfc, 0, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_CLOCK:
        pm_ev.message = Pm_Message(0xf8, 0, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_TUNE_REQUEST:
        pm_ev.message = Pm_Message(0xf6, 0, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_RESET:
        pm_ev.message = Pm_Message(0xff, 0, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_SENSING:
        pm_ev.message = Pm_Message(0xfe, 0, 0);
        pm_read_short(midi, &pm_ev);
        break;
    case SND_SEQ_EVENT_SYSEX: {
        const BYTE *ptr = (const BYTE *) ev->data.ext.ptr;
        /* assume there is one sysex byte to process */
        pm_read_bytes(midi, ptr, ev->data.ext.len, timestamp);
        break;
    }
    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED: {
        /* this happens if you have an input port open and the 
         * device or application with virtual ports closes. We
         * mark the port as closed to avoid closing a 2nd time
         * when Pm_Close() is called.
         */
        alsa_info_type info = (alsa_info_type) midi->api_info;
        /* printf("SND_SEQ_EVENT_UNSUBSCRIBE message\n"); */
        info->this_port = PORT_IS_CLOSED;
        break;
    }
    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        break;  /* someone connected to a virtual output port, not reported */
    default:
        printf("portmidi handle_event: not handled type %x\n", ev->type);
        break;
    }
}


static PmError alsa_poll(PmInternal *midi)
{
    if (!midi) {
        return pmBadPtr;
    }
    snd_seq_event_t *ev;
    /* expensive check for input data, gets data from device: */
    while (snd_seq_event_input_pending(seq, TRUE) > 0) {
        /* cheap check on local input buffer */
        while (snd_seq_event_input_pending(seq, FALSE) > 0) {
            /* check for and ignore errors, e.g. input overflow */
            /* note: if there's overflow, this should be reported
             * all the way through to client. Since input from all
             * devices is merged, we need to find all input devices
             * and set all to the overflow state.
             * NOTE: this assumes every input is ALSA based.
             */
            int rslt = snd_seq_event_input(seq, &ev);
            if (rslt >= 0) {
                handle_event(ev);
            } else if (rslt == -ENOSPC) {
                int i;
                for (i = 0; i < pm_descriptor_len; i++) {
                    if (pm_descriptors[i].pub.input) {
                        PmInternal *midi_i = pm_descriptors[i].pm_internal;
                        /* careful, device may not be open! */
                        if (midi_i) Pm_SetOverflow(midi_i->queue);
                    }
                }
            }
        }
    }
    return pmNoError;
}


static unsigned int alsa_check_host_error(PmInternal *midi)
{
    return FALSE;
}


pm_fns_node pm_linuxalsa_in_dictionary = {
    none_write_short,
    none_sysex,
    none_sysex,
    none_write_byte,
    none_write_short,
    none_write_flush,
    alsa_synchronize,
    alsa_in_open,
    alsa_abort,
    alsa_in_close,
    alsa_poll,
    alsa_check_host_error
};

pm_fns_node pm_linuxalsa_out_dictionary = {
    alsa_write_short,
    alsa_sysex,
    alsa_sysex,
    alsa_write_byte,
    alsa_write_short, /* short realtime message */
    alsa_write_flush,
    alsa_synchronize,
    alsa_out_open, 
    alsa_abort, 
    alsa_out_close,
    none_poll,
    alsa_check_host_error
};


/* pm_strdup -- copy a string to the heap. Use this rather than strdup so 
 *    that we call pm_alloc, not malloc. This allows portmidi to avoid 
 *    malloc which might cause priority inversion. Probably ALSA is going
 *    to call malloc anyway, so this extra work here may be pointless.
 */
char *pm_strdup(const char *s)
{
    int len = strlen(s);
    char *dup = (char *) pm_alloc(len + 1);
    strcpy(dup, s);
    return dup;
}


PmError pm_linuxalsa_init(void)
{
    int  err;
    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;
    unsigned int caps;

    /* Register interface ALSA with create_virtual fn */
    pm_add_interf("ALSA", &alsa_create_virtual, &alsa_delete_virtual);

    /* Previously, the last parameter was SND_SEQ_NONBLOCK, but this 
     * would cause messages to be dropped if the ALSA buffer fills up.
     * The correct behavior is for writes to block until there is 
     * room to send all the data. The client should normally allocate
     * a large enough buffer to avoid blocking on output. 
     * Now that blocking is enabled, the seq_event_input() will block
     * if there is no input data. This is not what we want, so must
     * call seq_event_input_pending() to avoid blocking.
     */
    err = snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0) goto error_return;
    
    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);

    snd_seq_client_info_set_client(cinfo, -1);
    while (snd_seq_query_next_client(seq, cinfo) == 0) {
        snd_seq_port_info_set_client(pinfo,
                                     snd_seq_client_info_get_client(cinfo));
        snd_seq_port_info_set_port(pinfo, -1);
        while (snd_seq_query_next_port(seq, pinfo) == 0) {
            if (snd_seq_port_info_get_client(pinfo) == SND_SEQ_CLIENT_SYSTEM)
                continue; /* ignore Timer and Announce ports on client 0 */
            caps = snd_seq_port_info_get_capability(pinfo);
            if (!(caps & (SND_SEQ_PORT_CAP_SUBS_READ |
                          SND_SEQ_PORT_CAP_SUBS_WRITE)))
                continue; /* ignore if you cannot read or write port */
            if (caps & SND_SEQ_PORT_CAP_SUBS_WRITE) {
                if (pm_default_output_device_id == -1) 
                    pm_default_output_device_id = pm_descriptor_len;
                pm_add_device("ALSA",
                        pm_strdup(snd_seq_port_info_get_name(pinfo)),
                        FALSE, FALSE,
                        MAKE_DESCRIPTOR(snd_seq_port_info_get_client(pinfo),
                                        snd_seq_port_info_get_port(pinfo)),
                        &pm_linuxalsa_out_dictionary);
            }
            if (caps & SND_SEQ_PORT_CAP_SUBS_READ) {
                if (pm_default_input_device_id == -1) 
                    pm_default_input_device_id = pm_descriptor_len;
                pm_add_device("ALSA",
                        pm_strdup(snd_seq_port_info_get_name(pinfo)),
                        TRUE, FALSE,
                        MAKE_DESCRIPTOR(snd_seq_port_info_get_client(pinfo),
                                        snd_seq_port_info_get_port(pinfo)),
                        &pm_linuxalsa_in_dictionary);
            }
        }
    }
    return pmNoError;
 error_return:
    pm_linuxalsa_term();  /* clean up */
    return check_hosterror(err);
}


void pm_linuxalsa_term(void)
{
    if (seq) {
        snd_seq_close(seq);
        pm_free(pm_descriptors);
        pm_descriptors = NULL;
        pm_descriptor_len = 0;
        pm_descriptor_max = 0;
    }
}

#endif
