/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "sound/discrete.h"

#define QIX_CHARACTER_CLOCK		(20000000/2/16)


/*----------- defined in machine/qix.c -----------*/

extern UINT8 *qix_68705_port_out;
extern UINT8 *qix_68705_ddr;

MACHINE_START( qix );
MACHINE_START( qixmcu );
MACHINE_START( slither );
MACHINE_RESET( qix );

WRITE8_HANDLER( zoo_bankswitch_w );

READ8_HANDLER( qix_data_firq_r );
READ8_HANDLER( qix_data_firq_ack_r );
WRITE8_HANDLER( qix_data_firq_w );
WRITE8_HANDLER( qix_data_firq_ack_w );

READ8_HANDLER( qix_video_firq_r );
READ8_HANDLER( qix_video_firq_ack_r );
WRITE8_HANDLER( qix_video_firq_w );
WRITE8_HANDLER( qix_video_firq_ack_w );

READ8_HANDLER( qix_68705_portA_r );
READ8_HANDLER( qix_68705_portB_r );
READ8_HANDLER( qix_68705_portC_r );
WRITE8_HANDLER( qix_68705_portA_w );
WRITE8_HANDLER( qix_68705_portB_w );
WRITE8_HANDLER( qix_68705_portC_w );

WRITE8_HANDLER( qix_pia_0_w );

INTERRUPT_GEN( qix_vblank_start );

/*----------- defined in video/qix.c -----------*/

extern UINT8 *qix_videoaddress;
extern UINT8 qix_cocktail_flip;

VIDEO_START( qix );
VIDEO_UPDATE( qix );

WRITE8_HANDLER( qix_crtc6845_address_w );
READ8_HANDLER( qix_crtc6845_register_r );
WRITE8_HANDLER( qix_crtc6845_register_w );
READ8_HANDLER( qix_scanline_r );
READ8_HANDLER( qix_videoram_r );
WRITE8_HANDLER( qix_videoram_w );
READ8_HANDLER( qix_addresslatch_r );
WRITE8_HANDLER( qix_addresslatch_w );
WRITE8_HANDLER( slither_vram_mask_w );
WRITE8_HANDLER( qix_paletteram_w );
WRITE8_HANDLER( qix_palettebank_w );

/*----------- defined in audio/qix.c -----------*/

DISCRETE_SOUND_EXTERN( qix );

WRITE8_HANDLER( qix_dac_w );
WRITE8_HANDLER( qix_vol_w );
