/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#pragma once

#ifndef __RF5C68_H__
#define __RF5C68_H__

#include "devlegcy.h"

/******************************************/
WRITE8_DEVICE_HANDLER( rf5c68_w );

READ8_DEVICE_HANDLER( rf5c68_mem_r );
WRITE8_DEVICE_HANDLER( rf5c68_mem_w );

typedef struct _rf5c68_interface rf5c68_interface;
struct _rf5c68_interface
{
	void (*sample_end_callback)(device_t* device, int channel);
};

DECLARE_LEGACY_SOUND_DEVICE(RF5C68, rf5c68);

#endif /* __RF5C68_H__ */
