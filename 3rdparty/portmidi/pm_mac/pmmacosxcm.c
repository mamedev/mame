/*
 * Platform interface to the MacOS X CoreMIDI framework
 * 
 * Jon Parise <jparise at cmu.edu>
 * and subsequent work by Andrew Zeldis and Zico Kolter
 * and Roger B. Dannenberg
 *
 * $Id: pmmacosx.c,v 1.17 2002/01/27 02:40:40 jon Exp $
 */
 
/* Notes:

    Since the input and output streams are represented by
    MIDIEndpointRef values and almost no other state, we store the
    MIDIEndpointRef on pm_descriptors[midi->device_id].descriptor.
    
    OS X does not seem to have an error-code-to-text function, so we
    will just use text messages instead of error codes.

    Virtual device input synchronization: Once we create a virtual
    device, it is always "on" and receiving messages, but it must drop
    messages unless the device has been opened with Pm_OpenInput. To
    open, the main thread should create all the data structures, then
    call OSMemoryBarrier so that writes are observed, then set
    is_opened = TRUE.  To close without locks, we need to get the
    callback to set is_opened to FALSE before we free data structures;
    otherwise, there's a race condition where closing could delete
    structures in use by the virtual_read_callback function. We send
    8 MIDI resets (FF) in a single packet to our own port to signal
    the virtual_read_callback to close it. Then, we wait for the
    callback to recognize the "close" packet and reset is_opened.
 
    Device scanning is done when you first open an application.
    PortMIDI does not actively update the devices. Instead, you must
    Pm_Terminate() and Pm_Initialize(), basically starting over. But
    CoreMIDI does not have a way to shut down(!), and even
    MIDIClientDispose() somehow retains state (and docs say do not
    call it even if it worked). The solution, apparently, is to
    call CFRunLoopRunInMode(), which somehow updates CoreMIDI
    state.
 
    But when do we call CFRunLoopRunInMode()? I tried calling it
    in midi_in_poll() which is called when you call Pm_Read() since
    that is called often. I observed that this caused the program
    to block for as long as 50ms and fairly often for 2 or 3ms.
    What was Apple thinking? Is it really OK to design systems that
    can only function with a tricky multi-threaded, non-blocking
    priority-based solution, and then not provide a proof of concept
    or documentation? Or is Apple's design really flawed? If anyone
    at Apple reads this, please let me know -- I'm curious.
 
    But I digress... Here's the PortMidi approach: Since
    CFRunLoopRunInMode() is potentially a non-realtime operation,
    we only call it in Pm_Initialize(), where other calls to look
    up devices and device names are quite slow to begin with. Again,
    PortMidi does not actively scan for new or deleted devices, so
    if devices change, you won't see it until the next Pm_Terminate
    and Pm_Initialize.
 
    Calling CFRunLoopRunInMode() once is probably not enough. There
    might be better way, but it seems to work to just call it 100
    times and insert 20 1ms delays (in case some inter-process
    communication or synchronization is going on).
    This adds 20ms to the wall time of Pm_Initialize(), but it
    typically runs 30ms to much more (~4s), so this has little impact.
 */

#include <stdlib.h>

/* turn on lots of debugging print statements */
#define CM_DEBUG if (0)
/* #define CM_DEBUG if (1) */

#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"
#include "porttime.h"
#include "pmmacosxcm.h"

#include <stdio.h>
#include <string.h>

#include <CoreServices/CoreServices.h>
#include <CoreMIDI/MIDIServices.h>
#include <CoreAudio/HostTime.h>
#include <unistd.h>
#include <libkern/OSAtomic.h>

#define PACKET_BUFFER_SIZE 1024
/* maximum overall data rate (OS X limits MIDI rate in case there
 * is a cycle among IAC ports.
 */

#define MAX_BYTES_PER_S 5400

/* Apple reports that packets are dropped when the MIDI bytes/sec
   exceeds 15000. This is computed by "tracking the number of MIDI
   bytes scheduled into 1-second buckets over the last six seconds and
   averaging these counts." This was confirmed in measurements
   (2021) with pm_test/fast.c and pm_test/fastrcv.c Now, in 2022, with
   macOS 12, pm_test/fast{rcv}.c show problems begin at 6000 bytes/sec.
   Previously, we set MAX_BYTES_PER_S to 14000. This is reduced to
   5400 based on testing (which shows 5700 is too high) to fix the
   packet loss problem that showed up with macOS 12.
 
   Experiments show this restriction applies to IAC bus MIDI, but not
   to hardware interfaces. (I measured 0.5 Mbps each way over USB to a
   Teensy 3.2 microcontroller implementing a USB MIDI loopback. Maybe
   it would get 1 Mbps one-way, which would make the CoreMIDI
   restriction 18x slower than USB. Maybe other USB MIDI
   implementations are faster -- USB top speed for other protocols is
   certainly higher than 1 Mbps!)

   This is apparently based on timestamps, not on real time, so we
   have to avoid constructing packets that schedule high speed output
   regardless of when writes occur. The solution is to alter
   timestamps to limit data rates.  This adds a slight time
   distortion, e.g. an 11 note chord with all notes on the same
   timestamp will be altered so that the last message is delayed by
   11 messages x 3 bytes/message / 5400 bytes/second = 6.1 ms.
   Note that this is about 2x MIDI speed, but at least 18x slower 
   than USB MIDI.
 
   Altering timestamps creates another problem, which is that a sender
   that exceeds the maximum rate can queue up an unbounded number of
   messages. With non-USB MIDI devices, you could be writing 5x faster
   to CoreMIDI than the hardware interface can send, causing an
   unbounded backlog, not to mention that the output stream will be a
   steady byte stream (e.g., one 3-byte MIDI message every 0.55 ms),
   losing any original timing or rhythm. PortMidi does not guarantee
   delivery if, over the long run, you write faster than the hardware
   can send.
   
   The LIMIT_RATE symbol, if defined (which is the default), enables
   code to modify timestamps for output to an IAC device as follows:

   Before a packet is formed, the message timestamp is set to the
   maximum of the PortMidi timestamp (converted to CoreMIDI time)
   and min_next_time. After each send, min_next_time is updated to
   the packet time + packet length * delay_per_byte, which limits
   the scheduled bytes-per-second. Also, after each packet list
   flush, min_next_time is updated to the maximum of min_next_time
   and the real time, which prevents many bytes to be scheduled in
   the past. (We could more directly just say packets are never
   scheduled in the past, but we prefer to get the current time -- a
   system call -- only when we perform the more expensive operation
   of flushing packets, so that's when we update min_next_time to
   the current real time. If we are sending a lot, we have to flush
   a lot, so the time will be updated frequently when it matters.)

   This possible adjustment to timestamps can distort accurate
   timestamps by up to 0.556 us per 3-byte MIDI message.
 
   Nothing blocks the sender from queueing up an arbitrary number of
   messages. Timestamps should be used for accurate timing by sending
   timestamped messages a little ahead of real time, not for
   scheduling an entire MIDI sequence at once!
 */
#define LIMIT_RATE 1

#define SYSEX_BUFFER_SIZE 128
/* What is the maximum PortMidi device number for an IAC device? A
 * cleaner design would be to not use the endpoint as our device
 * representation. Instead, we could have a private extensible struct
 * to keep all device information, including whether the device is
 * implemented with the AppleMIDIIACDriver, which we need because we
 * have to limit the data rate to this particular driver to avoid
 * dropping messages. Rather than rewrite a lot of code, I am just
 * allocating 64 bytes to flag which devices are IAC ones. If an IAC
 * device number is greater than 63, PortMidi will fail to limit
 * writes to it, but will not complain and will not access memory
 * outside the 64-element array of char.
 */
#define MAX_IAC_NUM 63

#define VERBOSE_ON 1
#define VERBOSE if (VERBOSE_ON)

#define MIDI_SYSEX       0xf0
#define MIDI_EOX         0xf7
#define MIDI_CLOCK       0xf8
#define MIDI_STATUS_MASK 0x80

/* "Ref"s are pointers on 32-bit machines and ints on 64 bit machines
   NULL_REF is our representation of either 0 or NULL
*/
#ifdef __LP64__
#define NULL_REF 0
#else
#define NULL_REF NULL
#endif

static MIDIClientRef client = NULL_REF;  /* Client handle to the MIDI server */
static MIDIPortRef   portIn = NULL_REF;  /* Input port handle */
static MIDIPortRef   portOut = NULL_REF;  /* Output port handle */
static char          isIAC[MAX_IAC_NUM + 1];  /* is device an IAC device */

extern pm_fns_node pm_macosx_in_dictionary;
extern pm_fns_node pm_macosx_out_dictionary;

typedef struct coremidi_info_struct {
    int is_virtual;     /* virtual device (TRUE) or actual device (FALSE)? */
    UInt64 delta;	/* difference between stream time and real time in ns */
    int sysex_mode;     /* middle of sending sysex */
    uint32_t sysex_word; /* accumulate data when receiving sysex */
    uint32_t sysex_byte_count; /* count how many received */
    char error[PM_HOST_ERROR_MSG_LEN];
    char callback_error[PM_HOST_ERROR_MSG_LEN];
    Byte packetBuffer[PACKET_BUFFER_SIZE];
    MIDIPacketList *packetList; /* a pointer to packetBuffer */
    MIDIPacket *packet;
    Byte sysex_buffer[SYSEX_BUFFER_SIZE]; /* temp storage for sysex data */
    MIDITimeStamp sysex_timestamp; /* host timestamp to use with sysex data */
    /* allow for running status (is running status possible here? -rbd): -cpr */
    UInt64 min_next_time; /* when can the next send take place? (host time) */
    int isIACdevice;
    Float64 us_per_host_tick; /* host clock frequency, units of min_next_time */
    UInt64 host_ticks_per_byte; /* host clock units per byte at maximum rate */
} coremidi_info_node, *coremidi_info_type;

/* private function declarations */
MIDITimeStamp timestamp_pm_to_cm(PmTimestamp timestamp); /* returns host time */
PmTimestamp timestamp_cm_to_pm(MIDITimeStamp timestamp); /* returns ms */

char* cm_get_full_endpoint_name(MIDIEndpointRef endpoint, int *iac_flag);

static PmError check_hosterror(OSStatus err, const char *msg)
{
    if (err != noErr) {
        snprintf(pm_hosterror_text, PM_HOST_ERROR_MSG_LEN, "Host error %ld: %s", (long) err, msg);
        pm_hosterror = TRUE;
        return pmHostError;
    }
    return pmNoError;
}


static PmTimestamp midi_synchronize(PmInternal *midi)
{
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    UInt64 pm_stream_time_2 = /* current time in ns */
            AudioConvertHostTimeToNanos(AudioGetCurrentHostTime());
    PmTimestamp real_time;  /* in ms */
    UInt64 pm_stream_time;  /* in ns */
    /* if latency is zero and this is an output, there is no 
       time reference and midi_synchronize should never be called */
    assert(midi->time_proc);
    assert(midi->is_input || midi->latency != 0);
    do {
         /* read real_time between two reads of stream time */
         pm_stream_time = pm_stream_time_2;
         real_time = (*midi->time_proc)(midi->time_info);
         pm_stream_time_2 = AudioConvertHostTimeToNanos(
                                    AudioGetCurrentHostTime());
         /* repeat if more than 0.5 ms has elapsed */
    } while (pm_stream_time_2 > pm_stream_time + 500000);
    info->delta = pm_stream_time - ((UInt64) real_time * (UInt64) 1000000);
    midi->sync_time = real_time;
    return real_time;
}


/* called when MIDI packets are received */
static void read_callback(const MIDIPacketList *newPackets, PmInternal *midi)
{
    PmTimestamp timestamp;
    MIDIPacket *packet;
    unsigned int packetIndex;
    uint32_t now;
    /* Retrieve the context for this connection */
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    assert(info);
    
    CM_DEBUG printf("read_callback: numPackets %d: ", newPackets->numPackets);
    
    /* synchronize time references every 100ms */
    now = (*midi->time_proc)(midi->time_info);
    if (midi->first_message || midi->sync_time + 100 /*ms*/ < now) {
        /* time to resync */
        now = midi_synchronize(midi);
        midi->first_message = FALSE;
    }
    
    packet = (MIDIPacket *) &newPackets->packet[0];
    /* hardware devices get untimed messages and apply timestamps. We
     * want to preserve them because they should be more accurate than
     * applying the current time here. virtual devices just pass on the
     * packet->timeStamp, which could be anything. PortMidi says the 
     * PortMidi timestamp is the time the message is received. We do not
     * know if we are receiving from a device driver or a virtual device.
     * PortMidi sends to virtual devices get a current timestamp, so we
     * can treat them as the receive time. If the timestamp is zero,
     * suggested by CoreMIDI as the value to use for immediate delivery,
     * then we plug in `now` which is obtained above. If another
     * application sends bogus non-zero timestamps, we will convert them
     * to this port's reference time and pass them as event.timestamp.
     * Receiver beware.
     */
    CM_DEBUG printf("read_callback packet @ %lld ns (host %lld) "
                    "status %x length %d\n",
                    AudioConvertHostTimeToNanos(AudioGetCurrentHostTime()),
                    AudioGetCurrentHostTime(),
                    packet->data[0], packet->length);
    for (packetIndex = 0; packetIndex < newPackets->numPackets; packetIndex++) {
        /* Set the timestamp and dispatch this message */
        CM_DEBUG printf("    packet->timeStamp %lld ns %lld host\n",
                        packet->timeStamp,
                        AudioConvertHostTimeToNanos(packet->timeStamp));
        if (packet->timeStamp == 0) {
            timestamp = now;
        } else {
            timestamp = (PmTimestamp) /* explicit conversion */ (
                (AudioConvertHostTimeToNanos(packet->timeStamp) - info->delta) /
                (UInt64) 1000000);
        }
        pm_read_bytes(midi, packet->data, packet->length, timestamp);
        packet = MIDIPacketNext(packet);
    }
}

/* callback for real devices - redirects to read_callback */
static void device_read_callback(const MIDIPacketList *newPackets, 
                                 void *refCon, void *connRefCon)
{
    read_callback(newPackets, (PmInternal *) connRefCon);
}


/* callback for virtual devices - redirects to read_callback */
static void virtual_read_callback(const MIDIPacketList *newPackets, 
                                  void *refCon, void *connRefCon)
{
    /* this refCon is the device ID -- if there is a valid ID and 
       the pm_descriptors table has a non-null pointer to a PmInternal,
       then then device is open and should receive this data */
    PmDeviceID id = (PmDeviceID) (size_t) refCon;
    if (id >= 0 && id < pm_descriptor_len) {
        if (pm_descriptors[id].pub.opened) {
            /* check for close request (7 reset status bytes): */
            if (newPackets->numPackets == 1 &&
                newPackets->packet[0].length == 8 &&
                /* CoreMIDI declares packets with 4-byte alignment, so we
                 * should be safe to test for 8 0xFF's as 2 32-bit values: */
                *(SInt32 *) &newPackets->packet[0].data[0] == -1 &&
                *(SInt32 *) &newPackets->packet[0].data[4] == -1) {
                CM_DEBUG printf("got close request packet\n");
                pm_descriptors[id].pub.opened = FALSE;
                return;
            } else {
                read_callback(newPackets, pm_descriptors[id].pm_internal);
            }
        }
    }
}


/* allocate and initialize our internal coremidi connection info */
static coremidi_info_type create_macosxcm_info(int is_virtual, int is_input)
{
    coremidi_info_type info = (coremidi_info_type)
            pm_alloc(sizeof(coremidi_info_node));
    if (!info) {
        return NULL;
    }
    info->is_virtual = is_virtual;
    info->delta = 0;
    info->sysex_mode = FALSE;
    info->sysex_word = 0;
    info->sysex_byte_count = 0;
    info->packet = NULL;
    info->min_next_time = 0;
    info->isIACdevice = FALSE;
    info->us_per_host_tick = 1000000.0 / AudioGetHostClockFrequency();
    info->host_ticks_per_byte =
            (UInt64) (1000000.0 / (info->us_per_host_tick * MAX_BYTES_PER_S));
    info->packetList = (is_input ? NULL :
                                   (MIDIPacketList *) info->packetBuffer);
    return info;
}


static PmError midi_in_open(PmInternal *midi, void *driverInfo)
{
    MIDIEndpointRef endpoint;
    coremidi_info_type info;
    OSStatus macHostError;
    int is_virtual = pm_descriptors[midi->device_id].pub.is_virtual;
    
    /* if this is an external device, descriptor is a MIDIEndpointRef.
     * if this is a virtual device for this application, descriptor is NULL.
     */
    if (!is_virtual) {
        endpoint = (MIDIEndpointRef) (intptr_t)
                   pm_descriptors[midi->device_id].descriptor;
        if (endpoint == NULL_REF) {
            return pmInvalidDeviceId;
        }
    }

    info = create_macosxcm_info(is_virtual, TRUE);
    midi->api_info = info;
    if (!info) {
        return pmInsufficientMemory;
    }
    if (!is_virtual) {
        macHostError = MIDIPortConnectSource(portIn, endpoint, midi);
        if (macHostError != noErr) {
            midi->api_info = NULL;
            pm_free(info);
            return  check_hosterror(macHostError,
                            "MIDIPortConnectSource() in midi_in_open()");
        }
    }
    return pmNoError;
}

static PmError midi_in_close(PmInternal *midi)
{
    MIDIEndpointRef endpoint;
    OSStatus macHostError;
    PmError err = pmNoError;
    
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    
    if (!info) return pmBadPtr;

    endpoint = (MIDIEndpointRef) (intptr_t)
               pm_descriptors[midi->device_id].descriptor;
    if (endpoint == NULL_REF) {
        return pmBadPtr;
    }
    
    if (!info->is_virtual) {
        /* shut off the incoming messages before freeing data structures */
        macHostError = MIDIPortDisconnectSource(portIn, endpoint);
        /* If the source closes, you get paramErr == -50 here. It seems
         * possible to monitor changes like sources closing by getting
         * notifications ALL changes, but the CoreMIDI documentation is
         * really terrible overall, and it seems easier to just ignore
         * this host error.
         */
        if (macHostError != noErr && macHostError != -50) {
            pm_hosterror = TRUE;
            err = check_hosterror(macHostError, 
                          "MIDIPortDisconnectSource() in midi_in_close()");
        }
    } else {
        /* make "close virtual port" message */
        SInt64 close_port_bytes = 0xFFFFFFFFFFFFFFFF;
        /* memory requirements: packet count (4), timestamp (8), length (2),
         *     data (8). Total: 22, but we allocate plenty more:
         */
        Byte packetBuffer[64];
        MIDIPacketList *plist = (MIDIPacketList *) packetBuffer;
        MIDIPacket *packet = MIDIPacketListInit(plist);
        MIDIPacketListAdd(plist, 64, packet, 0, 8,
                          (const Byte *) &close_port_bytes);
        macHostError = MIDISend(portOut, endpoint, plist);
        if (macHostError != noErr) {
            err = check_hosterror(macHostError, "MIDISend() (PortMidi close "
                                  "port packet) in midi_in_close()");
        }            
        /* when packet is delivered, callback thread will clear opened;
         * we must wait for that before removing the input queues etc.
         * Maybe this could use signals of some kind, but if signals use
         * locks, locks can cause priority inversion problems, so we will
         * just sleep as needed. On the MIDI timescale, inserting a 0.5ms
         * latency should be OK, as the application has no business
         * opening/closing devices during time-critical moments.
         *
         * We expect the MIDI thread to close the device quickly (<0.5ms),
         * but we wait up to 50ms in case something terrible happens like
         * getting paged out in the middle of deliving packets to this
         * virtual device. If there is still no response, we time out and
         * force the close without the MIDI thread (even this will probably
         * succeed - the problem would be: this thread proceeds to delete
         * the input queues, and the freed memory is reallocated and
         * overwritten so that queues are no longer usable. Meanwhile,
         * the MIDI thread has already begun to deliver packets, so the
         * check for opened == TRUE passed, but MIDI thread does not insert
         * into queue until queue is freed, reallocated and overwritten.
         */
        for (int i = 0; i < 100; i++) {  /* up to 50ms delay */
            if (!pm_descriptors[midi->device_id].pub.opened) {
                break;
            }
            usleep(500);  /* 0.5ms */
        }
        pm_descriptors[midi->device_id].pub.opened = FALSE;  /* force it */
    }
    midi->api_info = NULL;
    pm_free(info);
    return err;
}


static PmError midi_out_open(PmInternal *midi, void *driverInfo)
{
    coremidi_info_type info;
    int is_virtual = pm_descriptors[midi->device_id].pub.is_virtual;

    info = create_macosxcm_info(is_virtual, FALSE);
    if (midi->device_id <= MAX_IAC_NUM) {
        info->isIACdevice = isIAC[midi->device_id];
        CM_DEBUG printf("midi_out_open isIACdevice %d\n", info->isIACdevice);
    }
    midi->api_info = info;
    if (!info) {
        return pmInsufficientMemory;
    }
    return pmNoError;
}


static PmError midi_out_close(PmInternal *midi)
{
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    if (!info) return pmBadPtr;
    midi->api_info = NULL;
    pm_free(info);
    return pmNoError;
}


/* MIDIDestinationCreate apparently cannot create a virtual device
 * without a callback and a "refCon" parameter, but when we create
 * a virtual device, we do not want a PortMidi stream yet -- that
 * should wait for the user to open the stream. So, for the refCon,
 * use the PortMidi device ID. The callback will check if the
 * device is opened within PortMidi, and if so, use the pm_descriptors
 * table to locate the corresponding PmStream.
 */
static PmError midi_create_virtual(int is_input, const char *name,
                                   void *device_info)
{
    OSStatus macHostError;
    MIDIEndpointRef endpoint;
    CFStringRef nameRef;
    PmDeviceID id = pm_add_device("CoreMIDI", name, is_input, TRUE, NULL,
                                  (is_input ? &pm_macosx_in_dictionary :
                                              &pm_macosx_out_dictionary));
    if (id < 0) {  /* error -- out of memory or name conflict? */
        return id;
    }

    nameRef = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    if (is_input) {
        macHostError = MIDIDestinationCreate(client, nameRef, 
                virtual_read_callback, (void *) (intptr_t) id, &endpoint);
    } else {
        macHostError = MIDISourceCreate(client, nameRef, &endpoint);
    }
    CFRelease(nameRef);

    if (macHostError != noErr) {
        /* undo the device we just allocated */
        pm_undo_add_device(id);
        return check_hosterror(macHostError, (is_input ?
                "MIDIDestinationCreateWithProtocol() in midi_create_virtual()" :
                "MIDISourceCreateWithProtocol() in midi_create_virtual()"));
    }

    /* Do we have a manufacturer name? If not, set to "PortMidi" */
    const char *mfr_name = "PortMidi";
    PmSysDepInfo *info = (PmSysDepInfo *) device_info;
    /* the version where pmKeyCoreMidiManufacturer was introduced is 210 */
    if (info && info->structVersion >= 210) {
        int i;
        for (i = 0; i < info->length; i++) {  /* search for key */
            if (info->properties[i].key == pmKeyCoreMidiManufacturer) {
                mfr_name = info->properties[i].value;
                break;
            }  /* no other keys are recognized; they are ignored */
        }
    }
    nameRef = CFStringCreateWithCString(NULL, mfr_name, kCFStringEncodingUTF8);
    MIDIObjectSetStringProperty(endpoint, kMIDIPropertyManufacturer, nameRef);
    CFRelease(nameRef);

    pm_descriptors[id].descriptor = (void *) (intptr_t) endpoint;
    return id;
}


static PmError midi_delete_virtual(PmDeviceID id)
{
    MIDIEndpointRef endpoint;
    OSStatus macHostError;
    
    endpoint = (MIDIEndpointRef) (long) pm_descriptors[id].descriptor;
    if (endpoint == NULL_REF) {
        return pmBadPtr;
    }
    macHostError = MIDIEndpointDispose(endpoint);
    return check_hosterror(macHostError,
                           "MIDIEndpointDispose() in midi_in_close()");
}


static PmError midi_abort(PmInternal *midi)
{
    OSStatus macHostError;
    MIDIEndpointRef endpoint = (MIDIEndpointRef) (intptr_t)
                               pm_descriptors[midi->device_id].descriptor;
    macHostError = MIDIFlushOutput(endpoint);
    return check_hosterror(macHostError,
                           "MIDIFlushOutput() in midi_abort()");
}


static PmError midi_write_flush(PmInternal *midi, PmTimestamp timestamp)
{
    OSStatus macHostError = noErr;
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    MIDIEndpointRef endpoint = (MIDIEndpointRef) (intptr_t)
                               pm_descriptors[midi->device_id].descriptor;
    assert(info);
    assert(endpoint);
    if (info->packet != NULL) {
        /* out of space, send the buffer and start refilling it */
        /* update min_next_time each flush to support rate limit */
        UInt64 host_now =  AudioGetCurrentHostTime();
        if (host_now > info->min_next_time) 
            info->min_next_time = host_now;
        if (info->is_virtual) {
            macHostError = MIDIReceived(endpoint, info->packetList);
        } else {
            macHostError = MIDISend(portOut, endpoint, info->packetList);
        }
        info->packet = NULL; /* indicate no data in packetList now */
    }
    return check_hosterror(macHostError, (info->is_virtual ?
                                          "MIDIReceived() in midi_write()" :
                                          "MIDISend() in midi_write()"));
}


static PmError send_packet(PmInternal *midi, Byte *message,
                           unsigned int messageLength, MIDITimeStamp timestamp)
{
    PmError err;
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    assert(info);
    
    CM_DEBUG printf("add %d to packet %p len %d timestamp %lld @ %lld ns "
                    "(host %lld)\n",
                    message[0], info->packet, messageLength, timestamp,
                    AudioConvertHostTimeToNanos(AudioGetCurrentHostTime()),
                    AudioGetCurrentHostTime());
    info->packet = MIDIPacketListAdd(info->packetList,
                                     sizeof(info->packetBuffer), info->packet,
                                     timestamp, messageLength, message);
    if (info->packet == NULL) {
        /* out of space, send the buffer and start refilling it */
        /* make midi->packet non-null to fool midi_write_flush into sending */
        info->packet = (MIDIPacket *) 4;
        /* timestamp is 0 because midi_write_flush ignores timestamp since
         * timestamps are already in packets. The timestamp parameter is here
         * because other API's need it. midi_write_flush can be called 
         * from system-independent code that must be cross-API.
         */
        if ((err = midi_write_flush(midi, 0)) != pmNoError) return err;
        info->packet = MIDIPacketListInit(info->packetList);
        assert(info->packet); /* if this fails, it's a programming error */
        info->packet = MIDIPacketListAdd(info->packetList,
                               sizeof(info->packetBuffer), info->packet,
                               timestamp, messageLength, message);
        assert(info->packet); /* can't run out of space on first message */
    }
    return pmNoError;
}    


static PmError midi_write_short(PmInternal *midi, PmEvent *event)
{
    PmTimestamp when = event->timestamp;
    PmMessage what = event->message;
    MIDITimeStamp timestamp;
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    Byte message[4];
    unsigned int messageLength;

    if (info->packet == NULL) {
        info->packet = MIDIPacketListInit(info->packetList);
        /* this can never fail, right? failure would indicate something 
           unrecoverable */
        assert(info->packet);
    }
    
    /* PortMidi specifies that incoming timestamps are the receive
     * time.  Devices attach their receive times, but virtual devices
     * do not. Instead, they pass along whatever timestamp was sent to
     * them. We do not know if we are connected to real or virtual
     * device. To avoid wild timestamps on the receiving end, we
     * consider 2 cases: PortMidi timestamp is zero or latency is
     * zero. Both mean send immediately, so we attach the current time
     * which will go out immediately and arrive with a sensible
     * timestamp (not zero and not zero mapped to the client's local
     * time). Otherwise, we assume the timestamp is reasonable. It
     * might be slighly in the past, but we pass it along after
     * translation to MIDITimeStamp units. 
     *
     * Compute timestamp: use current time if timestamp is zero or
     * latency is zero. Both mean no timing and send immediately.
     */
    if (when == 0 || midi->latency == 0) {
        timestamp = AudioGetCurrentHostTime();
    } else {  /* translate PortMidi time + latency to CoreMIDI time */
        timestamp = ((UInt64) (when + midi->latency) * (UInt64) 1000000) +
                    info->delta;
        timestamp = AudioConvertNanosToHostTime(timestamp);
    }

    message[0] = Pm_MessageStatus(what);
    message[1] = Pm_MessageData1(what);
    message[2] = Pm_MessageData2(what);
    messageLength = pm_midi_length((int32_t) what);

#ifdef LIMIT_RATE
    /* Make sure we go forward in time. */
    if (timestamp < info->min_next_time) {
        timestamp = info->min_next_time;
    }
    /* Note that if application is way behind and slowly catching up, then
     * timestamps could be increasing faster than real time, and since 
     * timestamps are used to estimate data rate, our estimate could be
     * low, causing CoreMIDI to drop packets. This seems very unlikely.
     */
    if (info->isIACdevice || info->is_virtual) {
        info->min_next_time = timestamp + messageLength * 
                                          info->host_ticks_per_byte;
    }
#endif
    /* Add this message to the packet list */
    return send_packet(midi, message, messageLength, timestamp);
}


static PmError midi_begin_sysex(PmInternal *midi, PmTimestamp when)
{
    UInt64 when_ns;
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    assert(info);
    info->sysex_byte_count = 0;
    
    /* compute timestamp */
    if (when == 0) when = midi->now;
    /* if latency == 0, midi->now is not valid. We will just set it to zero */
    if (midi->latency == 0) when = 0;
    when_ns = ((UInt64) (when + midi->latency) * (UInt64) 1000000) +
              info->delta;
    info->sysex_timestamp =
              (MIDITimeStamp) AudioConvertNanosToHostTime(when_ns);
    UInt64 now; /* only make system time call when writing a virtual port */
    if (info->is_virtual && info->sysex_timestamp <
        (now = AudioGetCurrentHostTime())) {
        info->sysex_timestamp = now;
    }

    if (info->packet == NULL) {
        info->packet = MIDIPacketListInit(info->packetList);
        /* this can never fail, right? failure would indicate something 
           unrecoverable */
        assert(info->packet);
    }
    return pmNoError;
}


static PmError midi_end_sysex(PmInternal *midi, PmTimestamp when)
{
    PmError err;
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    assert(info);
    
#ifdef LIMIT_RATE
    /* make sure we go foreward in time */
    if (info->sysex_timestamp < info->min_next_time)
        info->sysex_timestamp = info->min_next_time;

    if (info->isIACdevice) {
        info->min_next_time = info->sysex_timestamp + info->sysex_byte_count *
                                                      info->host_ticks_per_byte;
    }
#endif
    
    /* now send what's in the buffer */
    err = send_packet(midi, info->sysex_buffer, info->sysex_byte_count,
                      info->sysex_timestamp);
    info->sysex_byte_count = 0;
    if (err != pmNoError) {
        info->packet = NULL; /* flush everything in the packet list */
    }
    return err;
}


static PmError midi_write_byte(PmInternal *midi, unsigned char byte, 
                               PmTimestamp timestamp)
{
    coremidi_info_type info = (coremidi_info_type) midi->api_info;
    assert(info);
    if (info->sysex_byte_count >= SYSEX_BUFFER_SIZE) {
        PmError err = midi_end_sysex(midi, timestamp);
        if (err != pmNoError) return err;
    }
    info->sysex_buffer[info->sysex_byte_count++] = byte;
    return pmNoError;
}


static PmError midi_write_realtime(PmInternal *midi, PmEvent *event)
{
    /* to send a realtime message during a sysex message, first
       flush all pending sysex bytes into packet list */
    PmError err = midi_end_sysex(midi, 0);
    if (err != pmNoError) return err;
    /* then we can just do a normal midi_write_short */
    return midi_write_short(midi, event);
}


static unsigned int midi_check_host_error(PmInternal *midi)
{
    return FALSE;
}


MIDITimeStamp timestamp_pm_to_cm(PmTimestamp timestamp)
{
    UInt64 nanos;
    if (timestamp <= 0) {
        return (MIDITimeStamp)0;
    } else {
        nanos = (UInt64)timestamp * (UInt64)1000000;
        return (MIDITimeStamp)AudioConvertNanosToHostTime(nanos);
    }
}


PmTimestamp timestamp_cm_to_pm(MIDITimeStamp timestamp)
{
    UInt64 nanos;
    nanos = AudioConvertHostTimeToNanos(timestamp);
    return (PmTimestamp)(nanos / (UInt64)1000000);
}


/* Code taken from http://developer.apple.com/qa/qa2004/qa1374.html
 *
 * Obtain the name of an endpoint without regard for whether it has connections.
 * The result should be released by the caller.
 */
CFStringRef EndpointName(MIDIEndpointRef endpoint, bool isExternal,
                         int *iac_flag)
{
    CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
    CFStringRef str;
    *iac_flag = FALSE;
  
    /* begin with the endpoint's name */
    str = NULL;
    MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &str);
    if (str != NULL) {
        CFStringAppend(result, str);
        CFRelease(str);
    }
    MIDIEntityRef entity = NULL_REF;
    MIDIEndpointGetEntity(endpoint, &entity);
    if (entity == NULL_REF) {
        /* probably virtual */
        return result;
    }
    if (!isExternal) { /* detect IAC devices */
        /* extern const CFStringRef kMIDIPropertyDriverOwner; */
        MIDIObjectGetStringProperty(entity, kMIDIPropertyDriverOwner, &str);
        if (str != NULL) {
            char s[32]; /* driver name may truncate, but that's OK */
            CFStringGetCString(str, s, 31, kCFStringEncodingUTF8);
            s[31] = 0;  /* make sure it is terminated just to be safe */
            CM_DEBUG printf("driver %s\n", s);
            *iac_flag = (strcmp(s, "com.apple.AppleMIDIIACDriver") == 0);
        }
    }

    if (CFStringGetLength(result) == 0) {
        /* endpoint name has zero length -- try the entity */
        str = NULL;
        MIDIObjectGetStringProperty(entity, kMIDIPropertyName, &str);
        if (str != NULL) {
            CFStringAppend(result, str);
            CFRelease(str);
        }
    }
    /* now consider the device's name */
    MIDIDeviceRef device = NULL_REF;
    MIDIEntityGetDevice(entity, &device);
    if (device == NULL_REF)
        return result;
  
    str = NULL;
    MIDIObjectGetStringProperty(device, kMIDIPropertyName, &str);
    if (CFStringGetLength(result) == 0) {
        CFRelease(result);
        return str;
    }
    if (str != NULL) {
        /* if an external device has only one entity, throw away
           the endpoint name and just use the device name
        */
        if (isExternal && MIDIDeviceGetNumberOfEntities(device) < 2) {
            CFRelease(result);
            return str;
        } else {
            if (CFStringGetLength(str) == 0) {
                CFRelease(str);
                return result;
            }
            /* does the entity name already start with the device name?
               (some drivers do this though they shouldn't)
               if so, do not prepend
            */
            if (CFStringCompareWithOptions(result, /* endpoint name */
                        str, /* device name */
                        CFRangeMake(0, CFStringGetLength(str)), 0) != 
                kCFCompareEqualTo) {
                /* prepend the device name to the entity name */
                if (CFStringGetLength(result) > 0)
                    CFStringInsert(result, 0, CFSTR(" "));
                CFStringInsert(result, 0, str);
            }
            CFRelease(str);
        }
    }
    return result;
}


/* Obtain the name of an endpoint, following connections.
   The result should be released by the caller.
*/
static CFStringRef ConnectedEndpointName(MIDIEndpointRef endpoint,
                                         int *iac_flag)
{
    CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
    CFStringRef str;
    OSStatus err;
    long i;
  
    /* Does the endpoint have connections? */
    CFDataRef connections = NULL;
    long nConnected = 0;
    bool anyStrings = false;
    err = MIDIObjectGetDataProperty(endpoint, kMIDIPropertyConnectionUniqueID,
                                    &connections);
    if (connections != NULL) {
        /* It has connections, follow them
           Concatenate the names of all connected devices
        */
        nConnected = CFDataGetLength(connections) / 
                     (int32_t) sizeof(MIDIUniqueID);
        if (nConnected) {
            const SInt32 *pid = (const SInt32 *)(CFDataGetBytePtr(connections));
            for (i = 0; i < nConnected; ++i, ++pid) {
                MIDIUniqueID id = EndianS32_BtoN(*pid);
                MIDIObjectRef connObject;
                MIDIObjectType connObjectType;
                err = MIDIObjectFindByUniqueID(id, &connObject, 
                                               &connObjectType);
                if (err == noErr) {
                    if (connObjectType == kMIDIObjectType_ExternalSource  ||
                        connObjectType == kMIDIObjectType_ExternalDestination) {
                        /* Connected to an external device's endpoint (>=10.3) */
                        str = EndpointName((MIDIEndpointRef)(connObject), true,
                                           iac_flag);
                    } else {
                        /* Connected to an external device (10.2) 
                           (or something else, catch-all)
                        */
                        str = NULL;
                        MIDIObjectGetStringProperty(connObject, 
                                                    kMIDIPropertyName, &str);
                    }
                    if (str != NULL) {
                        if (anyStrings)
                            CFStringAppend(result, CFSTR(", "));
                        else anyStrings = true;
                        CFStringAppend(result, str);
                        CFRelease(str);
                    }
                }
            }
        }
        CFRelease(connections);
    }
    if (anyStrings)
        return result; /* caller should release result */

    CFRelease(result);

    /* Here, either the endpoint had no connections, or we failed to
       obtain names for any of them.
    */
    return EndpointName(endpoint, false, iac_flag);
}


/* cm_get_full_endpoint_name -- returns UTF-8 namem for endpoint.
 *     caller is owner and responsible for pm_free'ing the string
 */
char *cm_get_full_endpoint_name(MIDIEndpointRef endpoint, int *iac_flag)
{
    /* Thanks to Dan Wilcox for fixes for Unicode handling
     * and kichikuou at github for fixing buffer size calculation.
     */
    CFStringRef fullName = ConnectedEndpointName(endpoint, iac_flag);
    CFIndex len = CFStringGetLength(fullName) + 1;
    /* (len seems to be length in Unicode characters, although docs add
     *  the confusing explanation "in terms of UTF-16 code pairs")
     */
    CFIndex max_byte_len = CFStringGetMaximumSizeForEncoding(
                                   len, kCFStringEncodingUTF8) + 1;
    char* pmname = (char *) pm_alloc(max_byte_len);

    /* copy the string into our buffer; note that there may be some wasted
       space, but the total waste is not large */
    CFStringGetCString(fullName, pmname, max_byte_len, kCFStringEncodingUTF8);

    /* clean up */
    if (fullName) CFRelease(fullName);
    return pmname;
}


pm_fns_node pm_macosx_in_dictionary = {
    none_write_short,
    none_sysex,
    none_sysex,
    none_write_byte,
    none_write_short,
    none_write_flush,
    none_synchronize,
    midi_in_open,
    midi_abort,
    midi_in_close,
    success_poll,
    midi_check_host_error
};

pm_fns_node pm_macosx_out_dictionary = {
    midi_write_short,
    midi_begin_sysex,
    midi_end_sysex,
    midi_write_byte,
    midi_write_realtime,
    midi_write_flush,
    midi_synchronize,
    midi_out_open,
    midi_abort,
    midi_out_close,
    success_poll,
    midi_check_host_error
};


/* We do nothing with callbacks, but generating the callbacks also
 * updates CoreMIDI state. Callback may not be essential, but calling
 * the CFRunLoopRunInMode is necessary.
 */
void cm_notify(const MIDINotification *msg, void *refCon)
{
    /*  for debugging, trace change notifications: 
    const char *descr[] = {
        "undefined (0)",
      	"kMIDIMsgSetupChanged",
        "kMIDIMsgObjectAdded",
        "kMIDIMsgObjectRemoved",
        "kMIDIMsgPropertyChanged",
        "kMIDIMsgThruConnectionsChanged",
        "kMIDIMsgSerialPortOwnerChanged",
        "kMIDIMsgIOError"};

    printf("MIDI Notify, messageID %d (%s)\n", (int) msg->messageID,
           descr[(int) msg->messageID]);
    */
    return;
}


PmError pm_macosxcm_init(void)
{
    ItemCount numInputs, numOutputs, numDevices;
    MIDIEndpointRef endpoint;
    OSStatus macHostError = noErr;
    const char *error_text;

    memset(isIAC, 0, sizeof(isIAC)); /* initialize all FALSE */

    /* Register interface CoreMIDI with create_virtual fn */
    pm_add_interf("CoreMIDI", &midi_create_virtual, &midi_delete_virtual);
    /* no check for error return because this always succeeds */

    /* Determine the number of MIDI devices on the system */
    numDevices = MIDIGetNumberOfDevices();

    /* Return prematurely if no devices exist on the system
       Note that this is not an error. There may be no devices.
       Pm_CountDevices() will return zero, which is correct and
       useful information
     */
    if (numDevices <= 0) {
        return pmNoError;
    }

    /* Initialize the client handle */
    if (client == NULL_REF) {
        macHostError = MIDIClientCreate(CFSTR("PortMidi"), &cm_notify, NULL,
                                        &client);
    } else {  /* see notes above on device scanning */
        for (int i = 0; i < 100; i++) {
            /* look for any changes before scanning for devices: */
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
            if (i % 5 == 0) Pt_Sleep(1);  /* insert 20 delays */
        }
    }
    if (macHostError != noErr) {
        error_text = "MIDIClientCreate() in pm_macosxcm_init()";
        goto error_return;
    }
    numInputs = MIDIGetNumberOfSources();
    numOutputs = MIDIGetNumberOfDestinations();

    /* Create the input port */
    macHostError = MIDIInputPortCreate(client, CFSTR("Input port"), 
                                       device_read_callback, NULL, &portIn);
    if (macHostError != noErr) {
        error_text = "MIDIInputPortCreate() in pm_macosxcm_init()";
        goto error_return;
    }

    /* Create the output port */
    macHostError = MIDIOutputPortCreate(client, CFSTR("Output port"), &portOut);
    if (macHostError != noErr) {
        error_text = "MIDIOutputPortCreate() in pm_macosxcm_init()";
        goto error_return;
    }

    /* Iterate over the MIDI input devices */
    for (int i = 0; i < numInputs; i++) {
        int iac_flag;
        endpoint = MIDIGetSource(i);
        if (endpoint == NULL_REF) {
            continue;
        }
        /* Register this device with PortMidi */
        char *name = cm_get_full_endpoint_name(endpoint, &iac_flag);
        pm_add_device("CoreMIDI", name, TRUE, FALSE,
                      (void *) (intptr_t) endpoint, &pm_macosx_in_dictionary);
        /* pm_add_device copies name, so it is no longer needed */
        pm_free(name);
    }

    /* Iterate over the MIDI output devices */
    for (int i = 0; i < numOutputs; i++) {
        int iac_flag;
        PmDeviceID id;
        endpoint = MIDIGetDestination(i);
        if (endpoint == NULL_REF) {
            continue;
        }
        /* Register this device with PortMidi */
        char *name = cm_get_full_endpoint_name(endpoint, &iac_flag);
        id = pm_add_device("CoreMIDI", name, FALSE, FALSE,
                (void *) (intptr_t) endpoint, &pm_macosx_out_dictionary);
        /* pm_add_device copies name, so it is no longer needed */
        pm_free(name);

        /* if this is an IAC device, tuck that info away for write functions */
        if (iac_flag && id <= MAX_IAC_NUM) {
            isIAC[id] = TRUE;
        }
    }
    return pmNoError;
    
error_return:
    pm_macosxcm_term(); /* clear out any opened ports */
    return check_hosterror(macHostError, error_text);
}

void pm_macosxcm_term(void)
{
    /* docs say do not explicitly dispose of client
       if (client != NULL_REF) MIDIClientDispose(client); */
    if (portIn != NULL_REF) MIDIPortDispose(portIn);
    if (portOut != NULL_REF) MIDIPortDispose(portOut);
}
