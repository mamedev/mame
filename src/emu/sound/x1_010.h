#pragma once

#ifndef __X1_010_H__
#define __X1_010_H__


typedef struct _x1_010_interface x1_010_interface;
struct _x1_010_interface
{
	int adr;	/* address */
};


READ8_DEVICE_HANDLER ( seta_sound_r );
WRITE8_DEVICE_HANDLER( seta_sound_w );

READ16_DEVICE_HANDLER ( seta_sound_word_r );
WRITE16_DEVICE_HANDLER( seta_sound_word_w );

void seta_sound_enable_w(running_device *device, int data);

DEVICE_GET_INFO( x1_010 );
#define SOUND_X1_010 DEVICE_GET_INFO_NAME( x1_010 )

#endif /* __X1_010_H__ */
