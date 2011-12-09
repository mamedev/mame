/*************************************************************************

    Driver for Midway MCR games

**************************************************************************/

#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"

/* constants */
#define MAIN_OSC_MCR_I		XTAL_19_968MHz


class mcr_state : public driver_device
{
public:
	mcr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT8 *m_videoram;

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in drivers/mcr.c -----------*/

WRITE8_DEVICE_HANDLER( mcr_ipu_sio_transmit );


/*----------- defined in machine/mcr.c -----------*/

extern const z80_daisy_config mcr_daisy_chain[];
extern const z80_daisy_config mcr_ipu_daisy_chain[];
extern const z80ctc_interface mcr_ctc_intf;
extern const z80ctc_interface nflfoot_ctc_intf;
extern const z80pio_interface nflfoot_pio_intf;
extern const z80sio_interface nflfoot_sio_intf;
extern UINT8 mcr_cocktail_flip;

extern const gfx_layout mcr_bg_layout;
extern const gfx_layout mcr_sprite_layout;

extern UINT32 mcr_cpu_board;
extern UINT32 mcr_sprite_board;

MACHINE_START( mcr );
MACHINE_RESET( mcr );
MACHINE_START( nflfoot );

TIMER_DEVICE_CALLBACK( mcr_interrupt );
TIMER_DEVICE_CALLBACK( mcr_ipu_interrupt );

WRITE8_HANDLER( mcr_control_port_w );

WRITE8_HANDLER( mcr_ipu_laserdisk_w );
READ8_HANDLER( mcr_ipu_watchdog_r );
WRITE8_HANDLER( mcr_ipu_watchdog_w );


/*----------- defined in video/mcr.c -----------*/

extern INT8 mcr12_sprite_xoffs;
extern INT8 mcr12_sprite_xoffs_flip;

VIDEO_START( mcr );

WRITE8_HANDLER( mcr_91490_paletteram_w );

WRITE8_HANDLER( mcr_90009_videoram_w );
WRITE8_HANDLER( mcr_90010_videoram_w );
READ8_HANDLER( twotiger_videoram_r );
WRITE8_HANDLER( twotiger_videoram_w );
WRITE8_HANDLER( mcr_91490_videoram_w );

SCREEN_UPDATE( mcr );
