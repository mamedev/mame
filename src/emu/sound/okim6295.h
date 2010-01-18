#pragma once

#ifndef __OKIM6295_H__
#define __OKIM6295_H__

/* an interface for the OKIM6295 and similar chips */

/*
  Note about the playback frequency: the external clock is internally divided,
  depending on pin 7, by 132 (high) or 165 (low).
*/
typedef struct _okim6295_interface okim6295_interface;
struct _okim6295_interface
{
	int pin7;
};

extern const okim6295_interface okim6295_interface_pin7high;
extern const okim6295_interface okim6295_interface_pin7low;



void okim6295_set_bank_base(running_device *device, int base);
void okim6295_set_pin7(running_device *device, int pin7);

READ8_DEVICE_HANDLER( okim6295_r );
WRITE8_DEVICE_HANDLER( okim6295_w );

/*
    To help the various custom ADPCM generators out there,
    the following routines may be used.
*/
struct adpcm_state
{
	INT32	signal;
	INT32	step;
};
void reset_adpcm(struct adpcm_state *state);
INT16 clock_adpcm(struct adpcm_state *state, UINT8 nibble);

DEVICE_GET_INFO( okim6295 );
#define SOUND_OKIM6295 DEVICE_GET_INFO_NAME( okim6295 )

#endif /* __OKIM6295_H__ */
