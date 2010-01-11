/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#pragma once

#ifndef __RF5C68_H__
#define __RF5C68_H__

/******************************************/
WRITE8_DEVICE_HANDLER( rf5c68_w );

READ8_DEVICE_HANDLER( rf5c68_mem_r );
WRITE8_DEVICE_HANDLER( rf5c68_mem_w );

typedef struct _rf5c68_interface rf5c68_interface;
struct _rf5c68_interface
{
	void (*sample_end_callback)(const device_config* device, int channel);
};

DEVICE_GET_INFO( rf5c68 );
#define SOUND_RF5C68 DEVICE_GET_INFO_NAME( rf5c68 )

#endif /* __RF5C68_H__ */
