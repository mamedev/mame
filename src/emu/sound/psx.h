/***************************************************************************

    PSX SPU

    preliminary version by smf.

***************************************************************************/

#pragma once

#ifndef __SOUND_PSX_H__
#define __SOUND_PSX_H__

WRITE32_DEVICE_HANDLER( psx_spu_w );
READ32_DEVICE_HANDLER( psx_spu_r );
WRITE32_DEVICE_HANDLER( psx_spu_delay_w );
READ32_DEVICE_HANDLER( psx_spu_delay_r );

typedef void ( *spu_handler )( running_machine *, UINT32, INT32 );

typedef struct _psx_spu_interface psx_spu_interface;
struct _psx_spu_interface
{
	UINT32 **p_psxram;
	void (*irq_set)(running_device *,UINT32);
	void (*spu_install_read_handler)(int,spu_handler);
	void (*spu_install_write_handler)(int,spu_handler);
};

DEVICE_GET_INFO( psxspu );
#define SOUND_PSXSPU DEVICE_GET_INFO_NAME( psxspu )

#endif /* __SOUND_PSX_H__ */
