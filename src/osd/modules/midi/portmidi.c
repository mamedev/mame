/***************************************************************************

    portmap.c

    Midi implementation based on portmidi library

*******************************************************************c********/

#include "portmidi/pm_common/portmidi.h"
#include "osdcore.h"

static const int RX_EVENT_BUF_SIZE = 512;

#define MIDI_SYSEX  0xf0
#define MIDI_EOX    0xf7

struct osd_midi_device
{
    PortMidiStream *pmStream;
    PmEvent rx_evBuf[RX_EVENT_BUF_SIZE];
    UINT8 xmit_in[4]; // Pm_Messages mean we can at most have 3 residue bytes
    int xmit_cnt;
    UINT8 last_status;
    bool rx_sysex;
};

bool osd_midi_init()
{
    Pm_Initialize();
    return true;
}

void osd_midi_exit()
{
    Pm_Terminate();
}

void osd_list_midi_devices(void)
{
    int num_devs = Pm_CountDevices();
    const PmDeviceInfo *pmInfo;

    printf("\n");

    if (num_devs == 0)
    {
        printf("No MIDI ports were found\n");
        return;
    }

    printf("MIDI input ports:\n");
    for (int i = 0; i < num_devs; i++)
    {
        pmInfo = Pm_GetDeviceInfo(i);

        if (pmInfo->input)
        {
            printf("%s %s\n", pmInfo->name, (i == Pm_GetDefaultInputDeviceID()) ? "(default)" : "");
        }
    }

    printf("\nMIDI output ports:\n");
    for (int i = 0; i < num_devs; i++)
    {
        pmInfo = Pm_GetDeviceInfo(i);

        if (pmInfo->output)
        {
            printf("%s %s\n", pmInfo->name, (i == Pm_GetDefaultOutputDeviceID()) ? "(default)" : "");
        }
    }
}

osd_midi_device *osd_open_midi_input(const char *devname)
{
    int num_devs = Pm_CountDevices();
    int found_dev = -1;
    const PmDeviceInfo *pmInfo;
    PortMidiStream *stm;
    osd_midi_device *ret;

    if (!strcmp("default", devname))
    {
        found_dev = Pm_GetDefaultInputDeviceID();
    }
    else
    {
        for (int i = 0; i < num_devs; i++)
        {
            pmInfo = Pm_GetDeviceInfo(i);

            if (pmInfo->input)
            {
                if (!strcmp(devname, pmInfo->name))
                {
                    found_dev = i;
                    break;
                }
            }
        }
    }

    if (found_dev >= 0)
    {
        if (Pm_OpenInput(&stm, found_dev, NULL, RX_EVENT_BUF_SIZE, NULL, NULL) == pmNoError)
        {
            ret = (osd_midi_device *)osd_malloc(sizeof(osd_midi_device));
            memset(ret, 0, sizeof(osd_midi_device));
            ret->pmStream = stm;
            return ret;
        }
        else
        {
            printf("Couldn't open PM device\n");
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

osd_midi_device *osd_open_midi_output(const char *devname)
{
    int num_devs = Pm_CountDevices();
    int found_dev = -1;
    const PmDeviceInfo *pmInfo;
    PortMidiStream *stm;
    osd_midi_device *ret;

    if (!strcmp("default", devname))
    {
        found_dev = Pm_GetDefaultOutputDeviceID();
    }
    else
    {
        for (int i = 0; i < num_devs; i++)
        {
            pmInfo = Pm_GetDeviceInfo(i);

            if (pmInfo->output)
            {
                if (!strcmp(devname, pmInfo->name))
                {
                    found_dev = i;
                    break;
                }
            }
        }
    }

    if (found_dev >= 0)
    {
        if (Pm_OpenOutput(&stm, found_dev, NULL, 100, NULL, NULL, 0) == pmNoError)
        {
            ret = (osd_midi_device *)osd_malloc(sizeof(osd_midi_device));
            memset(ret, 0, sizeof(osd_midi_device));
            ret->pmStream = stm;
            return ret;
        }
        else
        {
            printf("Couldn't open PM device\n");
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
    return NULL;
}

void osd_close_midi_channel(osd_midi_device *dev)
{
    Pm_Close(dev->pmStream);
    osd_free(dev);
}

bool osd_poll_midi_channel(osd_midi_device *dev)
{
    PmError chk = Pm_Poll(dev->pmStream);

    return (chk == pmGotData) ? true : false;
}

int osd_read_midi_channel(osd_midi_device *dev, UINT8 *pOut)
{
    int msgsRead = Pm_Read(dev->pmStream, dev->rx_evBuf, RX_EVENT_BUF_SIZE);
    int bytesOut = 0;

    if (msgsRead <= 0)
    {
        return 0;
    }

    for (int msg = 0; msg < msgsRead; msg++)
    {
        UINT8 status = Pm_MessageStatus(dev->rx_evBuf[msg].message);

        if (dev->rx_sysex)
        {
            if (status & 0x80)  // sys real-time imposing on us?
            {
                if ((status == 0xf2) || (status == 0xf3))
                {
                    *pOut++ = status;
                    *pOut++ = Pm_MessageData1(dev->rx_evBuf[msg].message);
                    *pOut++ = Pm_MessageData2(dev->rx_evBuf[msg].message);
                    bytesOut += 3;
                }
                else
                {
                    *pOut++ = status;
                    bytesOut++;
                    if (status == MIDI_EOX)
                    {
                        dev->rx_sysex = false;
                    }
                }
            }
            else    // shift out the sysex bytes
            {
                for (int i = 0; i < 4; i++)
                {
                    UINT8 byte = dev->rx_evBuf[msg].message & 0xff;
                    *pOut++ = byte;
                    bytesOut++;
                    if (byte == MIDI_EOX)
                    {
                        dev->rx_sysex = false;
                        break;
                    }
                    dev->rx_evBuf[msg].message >>= 8;
                }
            }
        }
        else
        {
            switch ((status>>4) & 0xf)
            {
                case 0xc:   // 2-byte messages
                case 0xd:
                    *pOut++ = status;
                    *pOut++ = Pm_MessageData1(dev->rx_evBuf[msg].message);
                    bytesOut += 2;
                    break;

                case 0xf:   // system common
                    switch (status & 0xf)
                    {
                        case 0: // System Exclusive
                        {
                            *pOut++ = status;   // this should be OK: the shortest legal sysex is F0 tt dd F7, I believe
                            *pOut++ = (dev->rx_evBuf[msg].message>>8) & 0xff;
                            *pOut++ = (dev->rx_evBuf[msg].message>>16) & 0xff;
                            UINT8 last = *pOut++ = (dev->rx_evBuf[msg].message>>24) & 0xff;
                            bytesOut += 4;
                            dev->rx_sysex = (last != MIDI_EOX);
                            break;
                        }

                        case 7: // End of System Exclusive
                            *pOut++ = status;
                            bytesOut += 1;
                            dev->rx_sysex = false;
                            break;

                        case 2: // song pos
                        case 3: // song select
                            *pOut++ = status;
                            *pOut++ = Pm_MessageData1(dev->rx_evBuf[msg].message);
                            *pOut++ = Pm_MessageData2(dev->rx_evBuf[msg].message);
                            bytesOut += 3;
                            break;

                        default:    // all other defined Fx messages are 1 byte
                            break;
                    }
                    break;

                default:
                    *pOut++ = status;
                    *pOut++ = Pm_MessageData1(dev->rx_evBuf[msg].message);
                    *pOut++ = Pm_MessageData2(dev->rx_evBuf[msg].message);
                    bytesOut += 3;
                    break;
            }
        }
    }

    return bytesOut;
}

void osd_write_midi_channel(osd_midi_device *dev, UINT8 data)
{
    int bytes_needed = 0;
    PmEvent ev;
    ev.timestamp = 0;   // use the current time

//  printf("write: %02x (%d)\n", data, dev->xmit_cnt);

    // reject data bytes when no valid status exists
    if ((dev->last_status == 0) && !(data & 0x80))
    {
        dev->xmit_cnt = 0;
        return;
    }

    if (dev->xmit_cnt >= 4)
    {
        printf("MIDI out: packet assembly overflow, contact MAMEdev!\n");
        return;
    }

    // handle sysex
    if (dev->last_status == MIDI_SYSEX)
    {
//      printf("sysex: %02x (%d)\n", data, dev->xmit_cnt);

        // if we get a status that isn't sysex, assume it's system common
        if ((data & 0x80) && (data != MIDI_EOX))
        {
//          printf("common during sysex!\n");
            ev.message = Pm_Message(data, 0, 0);
            Pm_Write(dev->pmStream, &ev, 1);
            return;
        }

        dev->xmit_in[dev->xmit_cnt++] = data;

        // if EOX or 4 bytes filled, transmit 4 bytes
        if ((dev->xmit_cnt == 4) || (data == MIDI_EOX))
        {
            ev.message = dev->xmit_in[0] | (dev->xmit_in[1]<<8) | (dev->xmit_in[2]<<16) | (dev->xmit_in[3]<<24);
            Pm_Write(dev->pmStream, &ev, 1);
            dev->xmit_in[0] = dev->xmit_in[1] = dev->xmit_in[2] = dev->xmit_in[3] = 0;
            dev->xmit_cnt = 0;

//          printf("SysEx packet: %08x\n", ev.message);

            // if this is EOX, kill the running status
            if (data == MIDI_EOX)
            {
                dev->last_status = 0;
            }
        }

        return;
    }

    // handle running status.  don't allow system real-time messages to be considered as running status.
    if ((dev->xmit_cnt == 0) && (data & 0x80) && (data < 0xf8))
    {
        dev->last_status = data;
    }

    if ((dev->xmit_cnt == 0) && !(data & 0x80))
    {
        dev->xmit_in[dev->xmit_cnt++] = dev->last_status;
        dev->xmit_in[dev->xmit_cnt++] = data;
//      printf("\trunning status: [%d] = %02x, [%d] = %02x, last_status = %02x\n", dev->xmit_cnt-2, dev->last_status, dev->xmit_cnt-1, data, dev->last_status);
    }
    else
    {
        dev->xmit_in[dev->xmit_cnt++] = data;
//      printf("\tNRS: [%d] = %02x\n", dev->xmit_cnt-1, data);
    }

    if ((dev->xmit_cnt == 1) && (dev->xmit_in[0] == MIDI_SYSEX))
    {
//      printf("Start SysEx!\n");
        dev->last_status = MIDI_SYSEX;
        return;
    }

    // are we there yet?
//  printf("status check: %02x\n", dev->xmit_in[0]);
    switch ((dev->xmit_in[0]>>4) & 0xf)
    {
        case 0xc:   // 2-byte messages
        case 0xd:
            bytes_needed = 2;
            break;

        case 0xf:   // system common
            switch (dev->xmit_in[0] & 0xf)
            {
                case 0: // System Exclusive is handled above
                    break;

                case 7: // End of System Exclusive
                    bytes_needed = 1;
                    break;

                case 2: // song pos
                case 3: // song select
                    bytes_needed = 3;
                    break;

                default:    // all other defined Fx messages are 1 byte
                    bytes_needed = 1;
                    break;
            }
            break;

        default:
            bytes_needed = 3;
            break;
    }

    if (dev->xmit_cnt == bytes_needed)
    {
        ev.message = Pm_Message(dev->xmit_in[0], dev->xmit_in[1], dev->xmit_in[2]);
        Pm_Write(dev->pmStream, &ev, 1);
        dev->xmit_cnt = 0;
    }

}
