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
	return NULL;
}

osd_midi_device *osd_open_midi_output(const char *devname)
{
	return NULL;
}

void osd_close_midi_channel(osd_midi_device *dev)
{
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
