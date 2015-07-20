// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    portmap.c

    Midi implementation based on portmidi library

*******************************************************************c********/

#ifndef NO_USE_MIDI

#ifndef USE_SYSTEM_PORTMIDI
#include "portmidi/pm_common/portmidi.h"
#else
#include <portmidi.h>
#endif
#include "osdcore.h"
#include "corealloc.h"
#include "modules/osdmodule.h"
#include "midi_module.h"

class pm_module : public osd_module, public midi_module
{
public:

	pm_module()
	: osd_module(OSD_MIDI_PROVIDER, "pm"), midi_module()
	{
	}
	virtual ~pm_module() { }

	virtual int init(const osd_options &options);
	virtual void exit();

	osd_midi_device *create_midi_device();
	void list_midi_devices(void);
};


static const int RX_EVENT_BUF_SIZE = 512;

#define MIDI_SYSEX  0xf0
#define MIDI_EOX    0xf7

class osd_midi_device_pm : public osd_midi_device
{
public:
	virtual ~osd_midi_device_pm() { }
	virtual bool open_input(const char *devname);
	virtual bool open_output(const char *devname);
	virtual void close();
	virtual bool poll();
	virtual int read(UINT8 *pOut);
	virtual void write(UINT8 data);

private:
	PortMidiStream *pmStream;
	PmEvent rx_evBuf[RX_EVENT_BUF_SIZE];
	UINT8 xmit_in[4]; // Pm_Messages mean we can at most have 3 residue bytes
	int xmit_cnt;
	UINT8 last_status;
	bool rx_sysex;
};

osd_midi_device *pm_module::create_midi_device()
{
	return global_alloc(osd_midi_device_pm());
}


int pm_module::init(const osd_options &options)
{
	Pm_Initialize();
	return 0;
}

void pm_module::exit()
{
	Pm_Terminate();
}

void pm_module::list_midi_devices(void)
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

bool osd_midi_device_pm::open_input(const char *devname)
{
	int num_devs = Pm_CountDevices();
	int found_dev = -1;
	const PmDeviceInfo *pmInfo;
	PortMidiStream *stm;

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
			pmStream = stm;
			return true;
		}
		else
		{
			printf("Couldn't open PM device\n");
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool osd_midi_device_pm::open_output(const char *devname)
{
	int num_devs = Pm_CountDevices();
	int found_dev = -1;
	const PmDeviceInfo *pmInfo;
	PortMidiStream *stm;

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
			pmStream = stm;
			return true;
		}
		else
		{
			printf("Couldn't open PM device\n");
			return false;
		}
	}
	else
	{
		return false;
	}
	return false;
}

void osd_midi_device_pm::close()
{
	Pm_Close(pmStream);
}

bool osd_midi_device_pm::poll()
{
	PmError chk = Pm_Poll(pmStream);

	return (chk == pmGotData) ? true : false;
}

int osd_midi_device_pm::read(UINT8 *pOut)
{
	int msgsRead = Pm_Read(pmStream, rx_evBuf, RX_EVENT_BUF_SIZE);
	int bytesOut = 0;

	if (msgsRead <= 0)
	{
		return 0;
	}

	for (int msg = 0; msg < msgsRead; msg++)
	{
		UINT8 status = Pm_MessageStatus(rx_evBuf[msg].message);

		if (rx_sysex)
		{
			if (status & 0x80)  // sys real-time imposing on us?
			{
				if ((status == 0xf2) || (status == 0xf3))
				{
					*pOut++ = status;
					*pOut++ = Pm_MessageData1(rx_evBuf[msg].message);
					*pOut++ = Pm_MessageData2(rx_evBuf[msg].message);
					bytesOut += 3;
				}
				else
				{
					*pOut++ = status;
					bytesOut++;
					if (status == MIDI_EOX)
					{
						rx_sysex = false;
					}
				}
			}
			else    // shift out the sysex bytes
			{
				for (int i = 0; i < 4; i++)
				{
					UINT8 byte = rx_evBuf[msg].message & 0xff;
					*pOut++ = byte;
					bytesOut++;
					if (byte == MIDI_EOX)
					{
						rx_sysex = false;
						break;
					}
					rx_evBuf[msg].message >>= 8;
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
					*pOut++ = Pm_MessageData1(rx_evBuf[msg].message);
					bytesOut += 2;
					break;

				case 0xf:   // system common
					switch (status & 0xf)
					{
						case 0: // System Exclusive
						{
							*pOut++ = status;   // this should be OK: the shortest legal sysex is F0 tt dd F7, I believe
							*pOut++ = (rx_evBuf[msg].message>>8) & 0xff;
							*pOut++ = (rx_evBuf[msg].message>>16) & 0xff;
							UINT8 last = *pOut++ = (rx_evBuf[msg].message>>24) & 0xff;
							bytesOut += 4;
							rx_sysex = (last != MIDI_EOX);
							break;
						}

						case 7: // End of System Exclusive
							*pOut++ = status;
							bytesOut += 1;
							rx_sysex = false;
							break;

						case 2: // song pos
						case 3: // song select
							*pOut++ = status;
							*pOut++ = Pm_MessageData1(rx_evBuf[msg].message);
							*pOut++ = Pm_MessageData2(rx_evBuf[msg].message);
							bytesOut += 3;
							break;

						default:    // all other defined Fx messages are 1 byte
							break;
					}
					break;

				default:
					*pOut++ = status;
					*pOut++ = Pm_MessageData1(rx_evBuf[msg].message);
					*pOut++ = Pm_MessageData2(rx_evBuf[msg].message);
					bytesOut += 3;
					break;
			}
		}
	}

	return bytesOut;
}

void osd_midi_device_pm::write(UINT8 data)
{
	int bytes_needed = 0;
	PmEvent ev;
	ev.timestamp = 0;   // use the current time

//  printf("write: %02x (%d)\n", data, xmit_cnt);

	// reject data bytes when no valid status exists
	if ((last_status == 0) && !(data & 0x80))
	{
		xmit_cnt = 0;
		return;
	}

	if (xmit_cnt >= 4)
	{
		printf("MIDI out: packet assembly overflow, contact MAMEdev!\n");
		return;
	}

	// handle sysex
	if (last_status == MIDI_SYSEX)
	{
//      printf("sysex: %02x (%d)\n", data, xmit_cnt);

		// if we get a status that isn't sysex, assume it's system common
		if ((data & 0x80) && (data != MIDI_EOX))
		{
//          printf("common during sysex!\n");
			ev.message = Pm_Message(data, 0, 0);
			Pm_Write(pmStream, &ev, 1);
			return;
		}

		xmit_in[xmit_cnt++] = data;

		// if EOX or 4 bytes filled, transmit 4 bytes
		if ((xmit_cnt == 4) || (data == MIDI_EOX))
		{
			ev.message = xmit_in[0] | (xmit_in[1]<<8) | (xmit_in[2]<<16) | (xmit_in[3]<<24);
			Pm_Write(pmStream, &ev, 1);
			xmit_in[0] = xmit_in[1] = xmit_in[2] = xmit_in[3] = 0;
			xmit_cnt = 0;

//          printf("SysEx packet: %08x\n", ev.message);

			// if this is EOX, kill the running status
			if (data == MIDI_EOX)
			{
				last_status = 0;
			}
		}

		return;
	}

	// handle running status.  don't allow system real-time messages to be considered as running status.
	if ((xmit_cnt == 0) && (data & 0x80) && (data < 0xf8))
	{
		last_status = data;
	}

	if ((xmit_cnt == 0) && !(data & 0x80))
	{
		xmit_in[xmit_cnt++] = last_status;
		xmit_in[xmit_cnt++] = data;
//      printf("\trunning status: [%d] = %02x, [%d] = %02x, last_status = %02x\n", xmit_cnt-2, last_status, xmit_cnt-1, data, last_status);
	}
	else
	{
		xmit_in[xmit_cnt++] = data;
//      printf("\tNRS: [%d] = %02x\n", xmit_cnt-1, data);
	}

	if ((xmit_cnt == 1) && (xmit_in[0] == MIDI_SYSEX))
	{
//      printf("Start SysEx!\n");
		last_status = MIDI_SYSEX;
		return;
	}

	// are we there yet?
//  printf("status check: %02x\n", xmit_in[0]);
	switch ((xmit_in[0]>>4) & 0xf)
	{
		case 0xc:   // 2-byte messages
		case 0xd:
			bytes_needed = 2;
			break;

		case 0xf:   // system common
			switch (xmit_in[0] & 0xf)
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

	if (xmit_cnt == bytes_needed)
	{
		ev.message = Pm_Message(xmit_in[0], xmit_in[1], xmit_in[2]);
		Pm_Write(pmStream, &ev, 1);
		xmit_cnt = 0;
	}

}
#else
	#include "modules/osdmodule.h"
	#include "midi_module.h"

	MODULE_NOT_SUPPORTED(pm_module, OSD_MIDI_PROVIDER, "pm")
#endif


MODULE_DEFINITION(MIDI_PM, pm_module)
