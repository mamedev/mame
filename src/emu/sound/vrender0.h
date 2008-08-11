#pragma once

#ifndef __VRENDER0_H__
#define __VRENDER0_H__


typedef struct _vr0_interface vr0_interface;
struct _vr0_interface
{
	UINT32 RegBase;
};

void vr0_snd_set_areas(UINT32 *texture,UINT32 *frame);

READ32_HANDLER(vr0_snd_read);
WRITE32_HANDLER(vr0_snd_write);

#endif /* __VRENDER0_H__ */
