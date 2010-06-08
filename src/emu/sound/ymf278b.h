#pragma once

#ifndef __YMF278B_H__
#define __YMF278B_H__

#include "devlegcy.h"

#define YMF278B_STD_CLOCK (33868800)			/* standard clock for OPL4 */


typedef struct _ymf278b_interface ymf278b_interface;
struct _ymf278b_interface
{
	void (*irq_callback)(running_device *device, int state);	/* irq callback */
};

READ8_DEVICE_HANDLER( ymf278b_r );
WRITE8_DEVICE_HANDLER( ymf278b_w );

DECLARE_LEGACY_SOUND_DEVICE(YMF278B, ymf278b);

#endif /* __YMF278B_H__ */
