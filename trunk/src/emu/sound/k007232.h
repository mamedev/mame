/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/

#pragma once

#ifndef __K007232_H__
#define __K007232_H__

#include "devlegcy.h"

typedef struct _k007232_interface k007232_interface;
struct _k007232_interface
{
	void (*portwritehandler)(device_t *, int);
};

WRITE8_DEVICE_HANDLER( k007232_w );
READ8_DEVICE_HANDLER( k007232_r );

void k007232_set_bank( device_t *device, int chABank, int chBBank );

/*
  The 007232 has two channels and produces two outputs. The volume control
  is external, however to make it easier to use we handle that inside the
  emulation. You can control volume and panning: for each of the two channels
  you can set the volume of the two outputs. If panning is not required,
  then volumeB will be 0 for channel 0, and volumeA will be 0 for channel 1.
  Volume is in the range 0-255.
*/
void k007232_set_volume(device_t *device,int channel,int volumeA,int volumeB);

DECLARE_LEGACY_SOUND_DEVICE(K007232, k007232);

#endif /* __K007232_H__ */
