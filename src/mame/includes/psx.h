/***************************************************************************

  includes/psx.h

***************************************************************************/

#if !defined( PSX_H )

#include "cpu/psx/dma.h"
#include "cpu/psx/irq.h"
#include "cpu/psx/sio.h"

class psx_state : public driver_device
{
public:
	psx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 *m_p_n_psxram;
	size_t m_n_psxramsize;

	UINT32 n_com_delay;
	int b_need_sianniv_vblank_hack;
};


/*----------- defined in machine/psx.c -----------*/

PALETTE_INIT( psx );
VIDEO_START( psx_type1 );
VIDEO_START( psx_type2 );
SCREEN_UPDATE( psx );
INTERRUPT_GEN( psx_vblank );
extern void psx_gpu_reset( running_machine &machine );
READ32_HANDLER( psx_gpu_r );
WRITE32_HANDLER( psx_gpu_w );
extern void psx_lightgun_set( running_machine &, int, int );

WRITE32_HANDLER( psx_com_delay_w );
READ32_HANDLER( psx_com_delay_r );
extern void psx_irq_set( running_machine &, UINT32 );
extern void psx_dma_install_read_handler( running_machine &, int, psx_dma_read_delegate );
extern void psx_dma_install_write_handler( running_machine &, int, psx_dma_read_delegate );
WRITE32_HANDLER( psx_counter_w );
READ32_HANDLER( psx_counter_r );
extern void psx_sio_install_handler( running_machine &, int, psx_sio_handler );
extern void psx_sio_input( running_machine &, int, int, int );

extern void psx_driver_init( running_machine &machine );

#define PSX_H ( 1 )
#endif
