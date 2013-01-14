//============================================================
//
//  pmmidi.c - OSD interface for PortMidi
//
//  Copyright (c) 1996-2013, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#include "portmidi/portmidi.h"
#include "osdcore.h"

struct osd_midi_device
{
	#ifndef DISABLE_MIDI
	PortMidiStream *pmStream;
	PmEvent rx_evBuf[20];		// up to 20 events
	#endif
	UINT8 xmit_in[4]; // Pm_Messages mean we can at most have 3 residue bytes
	int xmit_cnt;
};

void osd_list_midi_devices(void)
{
	#ifndef DISABLE_MIDI
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
	#else
	printf("\nMIDI is not supported in this build\n");
	#endif
}

osd_midi_device *osd_open_midi_input(const char *devname)
{
	#ifndef DISABLE_MIDI
	int num_devs = Pm_CountDevices();
	int found_dev = -1;
	const PmDeviceInfo *pmInfo;
	PortMidiStream *stm;
	osd_midi_device *ret;

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

	if (found_dev >= 0)
	{
		if (Pm_OpenInput(&stm, found_dev, NULL, 20, NULL, NULL) == pmNoError)
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
	#else
	return NULL;
	#endif
}

osd_midi_device *osd_open_midi_output(const char *devname)
{
	#ifndef DISABLE_MIDI
	int num_devs = Pm_CountDevices();
	int found_dev = -1;
	const PmDeviceInfo *pmInfo;
	PortMidiStream *stm;
	osd_midi_device *ret;

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

	if (found_dev >= 0)
	{
		if (Pm_OpenOutput(&stm, found_dev, NULL, 20, NULL, NULL, 0) == pmNoError)
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
	#endif
	return NULL;
}

void osd_close_midi_channel(osd_midi_device *dev)
{
	#ifndef DISABLE_MIDI
	Pm_Close(dev->pmStream);
	osd_free(dev);
	#endif
}

bool osd_poll_midi_channel(osd_midi_device *dev)
{
	#ifndef DISABLE_MIDI
	PmError chk = Pm_Poll(dev->pmStream);

	return (chk == pmGotData) ? true : false;
	#else
	return false;
	#endif
}

int osd_read_midi_channel(osd_midi_device *dev, UINT8 *pOut)
{
	#ifndef DISABLE_MIDI
	int msgsRead = Pm_Read(dev->pmStream, dev->rx_evBuf, 20);
	int bytesOut = 0;

	if (msgsRead <= 0)
	{
		return 0;
	}

	for (int msg = 0; msg < msgsRead; msg++)
	{
		UINT8 status = Pm_MessageStatus(dev->rx_evBuf[msg].message);
		switch ((status>>4) & 0xf)
		{
			case 0xc:	// 2-byte messages
			case 0xd:
				*pOut++ = status;
				*pOut++ = Pm_MessageData1(dev->rx_evBuf[msg].message); 
				bytesOut += 2;
				break;

			case 0xf:	// system common
				switch (status & 0xf)
				{
					case 0:	// System Exclusive
						printf("No SEx please!\n");
						break;

					case 7:	// End of System Exclusive
						*pOut++ = status;
						bytesOut += 1;
						break;

					case 2:	// song pos
					case 3:	// song select
						*pOut++ = status;
						*pOut++ = Pm_MessageData1(dev->rx_evBuf[msg].message); 
						*pOut++ = Pm_MessageData2(dev->rx_evBuf[msg].message); 
						bytesOut += 3;
						break;

					default:	// all other defined Fx messages are 1 byte
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

	return bytesOut;
	#else
	return 0;
	#endif
}

void osd_write_midi_channel(osd_midi_device *dev, UINT8 data)
{
	#ifndef DISABLE_MIDI
	int bytes_needed = 0;

	dev->xmit_in[dev->xmit_cnt++] = data;

	// are we there yet?
	switch ((dev->xmit_in[0]>>4) & 0xf)
	{
		case 0xc:	// 2-byte messages
		case 0xd:
			bytes_needed = 2;
			break;

		case 0xf:	// system common
			switch (dev->xmit_in[0] & 0xf)
			{
				case 0:	// System Exclusive
					printf("No SEx please!\n");
					break;

				case 7:	// End of System Exclusive
					bytes_needed = 1;
					break;

				case 2:	// song pos
				case 3:	// song select
					bytes_needed = 3;
					break;

				default:	// all other defined Fx messages are 1 byte
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
		PmEvent ev;
		ev.message = Pm_Message(dev->xmit_in[0], dev->xmit_in[1], dev->xmit_in[2]);
		ev.timestamp = 0;	// use the current time
		Pm_Write(dev->pmStream, &ev, 1);
		dev->xmit_cnt = 0;
	}

	#endif
}

void osd_init_midi(void)
{
	#ifndef DISABLE_MIDI
	Pm_Initialize();
	#endif
}

void osd_shutdown_midi(void)
{
	#ifndef DISABLE_MIDI
	Pm_Terminate();
	#endif
}
