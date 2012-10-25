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
};


// mame/machine/psx.c
extern void psx_driver_init( running_machine &machine );
DECLARE_WRITE32_HANDLER( psx_com_delay_w );
DECLARE_READ32_HANDLER( psx_com_delay_r );
extern void psx_sio_install_handler( running_machine &, int, psx_sio_handler );
extern void psx_sio_input( running_machine &, int, int, int );

// emu/video/psx.c
PALETTE_INIT( psx );
SCREEN_UPDATE_IND16( psx );
INTERRUPT_GEN( psx_vblank );

#define PSX_H ( 1 )
#endif
