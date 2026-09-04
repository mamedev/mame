/* pmsndio.c -- PortMidi os-dependent code */

#include <stdlib.h>
#include <stdio.h>
#include <sndio.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <pthread.h>
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"
#include "porttime.h"

#define NDEVS 9
#define SYSEX_MAXLEN 1024

#define SYSEX_START     0xf0
#define SYSEX_END       0xf7

extern pm_fns_node pm_sndio_in_dictionary;
extern pm_fns_node pm_sndio_out_dictionary;

/* length of voice and common messages (status byte included) */
unsigned int voice_len[] = { 3, 3, 3, 3, 2, 2, 3 };
unsigned int common_len[] = { 0, 2, 3, 2, 0, 0, 1, 1 };

struct mio_dev {
    char name[16];
    struct mio_hdl *hdl;
    int mode;
    char errmsg[PM_HOST_ERROR_MSG_LEN];
    pthread_t thread;
} devs[NDEVS];

static void set_mode(struct mio_dev *, unsigned int);

void pm_init(void)
{
    int i, j, k = 0;
    char devices[][16] = {"midithru", "rmidi", "midi", "snd"};

    /* default */
    strcpy(devs[0].name, MIO_PORTANY);
    pm_add_device("SNDIO", devs[k].name, TRUE, FALSE, (void *) &devs[k],
        &pm_sndio_in_dictionary);
    pm_add_device("SNDIO", devs[k].name, FALSE, FALSE, (void *) &devs[k],
        &pm_sndio_out_dictionary);
    k++;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            sprintf(devs[k].name, "%s/%d", devices[i], j);
            pm_add_device("SNDIO", devs[k].name, TRUE, FALSE, (void *) &devs[k],
              &pm_sndio_in_dictionary);
            pm_add_device("SNDIO", devs[k].name, FALSE, FALSE, (void *) &devs[k],
              &pm_sndio_out_dictionary);
            k++;
        }
    }

    // this is set when we return to Pm_Initialize, but we need it
    // now in order to (successfully) call Pm_CountDevices()
    pm_initialized = TRUE;
    pm_default_input_device_id = 0;
    pm_default_output_device_id = 1;
}

void pm_term(void)
{
    int i;
    for(i = 0; i < NDEVS; i++) {
        if (devs[i].mode != 0) {
            set_mode(&devs[i], 0);
            if (devs[i].thread) {
                pthread_join(devs[i].thread, NULL);
                devs[i].thread = NULL;
            }
        }
    }
}

PmDeviceID Pm_GetDefaultInputDeviceID(void) {
    Pm_Initialize();
    return pm_default_input_device_id;
}

PmDeviceID Pm_GetDefaultOutputDeviceID(void) {
    Pm_Initialize();
    return pm_default_output_device_id;
}

void *pm_alloc(size_t s) { return malloc(s); }

void pm_free(void *ptr) { free(ptr); }

/* midi_message_length -- how many bytes in a message? */
static int midi_message_length(PmMessage message)
{
    unsigned char st = message & 0xff;
    if (st >= 0xf8)
	return 1;
    else if (st >= 0xf0)
        return common_len[st & 7];
    else if (st >= 0x80)
        return voice_len[(st >> 4) & 7];
    else
        return 0;
}

void* input_thread(void *param)
{
    PmInternal *midi = (PmInternal*)param;
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;
    struct pollfd pfd[1];
    nfds_t nfds;
    unsigned char st = 0, c = 0;
    int rc, revents, idx = 0, len = 0;
    size_t todo = 0;
    unsigned char buf[0x200], *p;
    PmEvent pm_ev, pm_ev_rt;
    unsigned char sysex_data[SYSEX_MAXLEN];

    while(dev->mode & MIO_IN) {
        if (todo == 0) {
            nfds = mio_pollfd(dev->hdl, pfd, POLLIN);
            rc = poll(pfd, nfds, 100);
            if (rc < 0) {
                if (errno == EINTR)
                    continue;
                break;
            }
            revents = mio_revents(dev->hdl, pfd);
            if (!(revents & POLLIN))
                continue;

            todo = mio_read(dev->hdl, buf, sizeof(buf));
            if (todo == 0)
                continue;
            p = buf;
        }
        c = *p++;
        todo--;

        if (c >= 0xf8) {
            pm_ev_rt.message = c;
            pm_ev_rt.timestamp = Pt_Time();
            pm_read_short(midi, &pm_ev_rt);
        } else if (c == SYSEX_END) {
            /* note: PortMidi is designed to avoid the need for SYSEX_MAXLEN.
               With the new implementation of pm_read_bytes, it would be
               better to simply call pm_read_bytes() and let it parse buf,
               which can contain any number of whole or partial messages with
               interleaved realtime messages. I did not change the code because
               I cannot test it. -RBD */
            if (st == SYSEX_START) {
                sysex_data[idx++] = c;
                pm_read_bytes(midi, sysex_data, idx, Pt_Time());
            }
            st = 0;
            idx = 0;
        } else if (c == SYSEX_START) {
            st = c;
            idx = 0;
            sysex_data[idx++] = c;
        } else if (c >= 0xf0) {
            pm_ev.message = c;
            len = common_len[c & 7];
            st = c;
            idx = 1;
        } else if (c >= 0x80) {
            pm_ev.message = c;
            len = voice_len[(c >> 4) & 7];
            st = c;
            idx = 1;
        } else if (st == SYSEX_START) {
            if (idx == SYSEX_MAXLEN) {
                fprintf(stderr, "the message is too long\n");
                idx = st = 0;
            } else {
                sysex_data[idx++] = c;
            }
        } else if (st) {
            if (idx == 0 && st != SYSEX_START)
                pm_ev.message |= (c << (8 * idx++));
            pm_ev.message |= (c << (8 * idx++));
            if (idx == len) {
                pm_read_short(midi, &pm_ev);
                if (st >= 0xf0)
                    st = 0;
                idx = 0;
            }
        }
    }

    pthread_exit(NULL);
    return NULL;
}

static void set_mode(struct mio_dev *dev, unsigned int mode) {
    if (dev->mode != 0)
        mio_close(dev->hdl);
    dev->mode = 0;
    if (mode != 0)
        dev->hdl = mio_open(dev->name, mode, 0);
    if (dev->hdl)
        dev->mode = mode;
}

static PmError sndio_out_open(PmInternal *midi, void *driverInfo)
{
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;

    if (dev->mode & MIO_OUT)
        return pmNoError;

    set_mode(dev, dev->mode | MIO_OUT);
    if (!(dev->mode & MIO_OUT)) {
        snprintf(dev->errmsg, PM_HOST_ERROR_MSG_LEN,
          "mio_open (output) failed: %s\n", dev->name);
        return pmHostError;
    }

    return pmNoError;
}

static PmError sndio_in_open(PmInternal *midi, void *driverInfo)
{
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;

    if (dev->mode & MIO_IN)
        return pmNoError;

    set_mode(dev, dev->mode | MIO_IN);
    if (!(dev->mode & MIO_IN)) {
        snprintf(dev->errmsg, PM_HOST_ERROR_MSG_LEN,
          "mio_open (input) failed: %s\n", dev->name);
        return pmHostError;
    }
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&dev->thread, &attr, input_thread, ( void* )midi);
    return pmNoError;
}

static PmError sndio_out_close(PmInternal *midi)
{
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;

    if (dev->mode & MIO_OUT)
        set_mode(dev, dev->mode & ~MIO_OUT);
    return pmNoError;
}

static PmError sndio_in_close(PmInternal *midi)
{
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;

    if (dev->mode & MIO_IN) {
        set_mode(dev, dev->mode & ~MIO_IN);
        pthread_join(dev->thread, NULL);
        dev->thread = NULL;
    }
    return pmNoError;
}

static PmError sndio_abort(PmInternal *midi)
{
    return pmNoError;
}

static PmTimestamp sndio_synchronize(PmInternal *midi)
{
    return 0;
}

static PmError do_write(struct mio_dev *dev, const void *addr, size_t nbytes)
{
    size_t w = mio_write(dev->hdl, addr, nbytes);

    if (w != nbytes) {
        snprintf(dev->errmsg, PM_HOST_ERROR_MSG_LEN,
          "mio_write failed, bytes written:%zu\n", w);
        return pmHostError;
    }
    return pmNoError;
}

static PmError sndio_write_byte(PmInternal *midi, unsigned char byte,
                        PmTimestamp timestamp)
{
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;

    return do_write(dev, &byte, 1);
}

static PmError sndio_write_short(PmInternal *midi, PmEvent *event)
{
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;
    int nbytes = midi_message_length(event->message);

    if (midi->latency > 0) {
        /* XXX the event should be queued for later playback */
        return do_write(dev, &event->message, nbytes);
    } else {
        return do_write(dev, &event->message, nbytes);
    }
    return pmNoError;
}

static PmError sndio_write_flush(PmInternal *midi, PmTimestamp timestamp)
{
    return pmNoError;
}

PmError sndio_sysex(PmInternal *midi, PmTimestamp timestamp)
{
    return pmNoError;
}

static unsigned int sndio_has_host_error(PmInternal *midi)
{
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;

    return (dev->errmsg[0] != '\0');
}

static void sndio_get_host_error(PmInternal *midi, char *msg, unsigned int len)
{
    struct mio_dev *dev = pm_descriptors[midi->device_id].descriptor;

    strlcpy(msg, dev->errmsg, len);
    dev->errmsg[0] = '\0';
}

pm_fns_node pm_sndio_in_dictionary = {
    none_write_short,
    none_sysex,
    none_sysex,
    none_write_byte,
    none_write_short,
    none_write_flush,
    sndio_synchronize,
    sndio_in_open,
    sndio_abort,
    sndio_in_close,
    success_poll,
    sndio_has_host_error,
};

pm_fns_node pm_sndio_out_dictionary = {
    sndio_write_short,
    sndio_sysex,
    sndio_sysex,
    sndio_write_byte,
    sndio_write_short,
    sndio_write_flush,
    sndio_synchronize,
    sndio_out_open,
    sndio_abort,
    sndio_out_close,
    none_poll,
    sndio_has_host_error,
};

