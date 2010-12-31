/***************************************************************************

    PSX SPU

    preliminary version by smf.

***************************************************************************/

#pragma once

#ifndef __SOUND_PSX_H__
#define __SOUND_PSX_H__

#include "devlegcy.h"

WRITE32_DEVICE_HANDLER( psx_spu_w );
READ32_DEVICE_HANDLER( psx_spu_r );
WRITE32_DEVICE_HANDLER( psx_spu_delay_w );
READ32_DEVICE_HANDLER( psx_spu_delay_r );

typedef void ( *spu_handler )( running_machine *, UINT32, INT32 );

typedef struct _psx_spu_interface psx_spu_interface;
struct _psx_spu_interface
{
	void (*irq_set)(device_t *,UINT32);
	void (*spu_install_read_handler)(running_machine *,int,spu_handler);
	void (*spu_install_write_handler)(running_machine *,int,spu_handler);
};

DECLARE_LEGACY_SOUND_DEVICE(PSXSPU, psxspu);

#endif /* __SOUND_PSX_H__ */
