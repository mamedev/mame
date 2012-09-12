/*************************************************************************

    Driver for Midway MCR games

**************************************************************************/

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "audio/midway.h"

/* constants */
#define MAIN_OSC_MCR_I		XTAL_19_968MHz


class mcr_state : public driver_device
{
public:
	mcr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_ssio(*this, "ssio"),
		m_chip_squeak_deluxe(*this, "csd"),
		m_sounds_good(*this, "sg"),
		m_turbo_chip_squeak(*this, "tcs"),
		m_squawk_n_talk(*this, "snt"),
		m_dpoker_coin_in_timer(*this, "dp_coinin"),
		m_dpoker_hopper_timer(*this, "dp_hopper")
	{ }

	// these should be required but can't because mcr68 shares with us
	// once the sound boards are properly device-ified, fix this
	optional_device<z80_device> m_maincpu;
	optional_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_videoram;

	optional_device<midway_ssio_device> m_ssio;
	optional_device<midway_chip_squeak_deluxe_device> m_chip_squeak_deluxe;
	optional_device<midway_sounds_good_device> m_sounds_good;
	optional_device<midway_turbo_chip_squeak_device> m_turbo_chip_squeak;
	optional_device<midway_squawk_n_talk_device> m_squawk_n_talk;
	optional_device<timer_device> m_dpoker_coin_in_timer;
	optional_device<timer_device> m_dpoker_hopper_timer;

	DECLARE_WRITE8_MEMBER(mcr_control_port_w);
	DECLARE_WRITE8_MEMBER(mcr_ipu_laserdisk_w);
	DECLARE_READ8_MEMBER(mcr_ipu_watchdog_r);
	DECLARE_WRITE8_MEMBER(mcr_ipu_watchdog_w);
	DECLARE_WRITE8_MEMBER(mcr_91490_paletteram_w);
	DECLARE_WRITE8_MEMBER(mcr_90009_videoram_w);
	DECLARE_WRITE8_MEMBER(mcr_90010_videoram_w);
	DECLARE_READ8_MEMBER(twotiger_videoram_r);
	DECLARE_WRITE8_MEMBER(twotiger_videoram_w);
	DECLARE_WRITE8_MEMBER(mcr_91490_videoram_w);
	DECLARE_READ8_MEMBER(solarfox_ip0_r);
	DECLARE_READ8_MEMBER(solarfox_ip1_r);
	DECLARE_READ8_MEMBER(dpoker_ip0_r);
	DECLARE_WRITE8_MEMBER(dpoker_lamps1_w);
	DECLARE_WRITE8_MEMBER(dpoker_lamps2_w);
	DECLARE_WRITE8_MEMBER(dpoker_output_w);
	DECLARE_WRITE8_MEMBER(dpoker_meters_w);
	DECLARE_READ8_MEMBER(kick_ip1_r);

	DECLARE_WRITE8_MEMBER(wacko_op4_w);
	DECLARE_READ8_MEMBER(wacko_ip1_r);
	DECLARE_READ8_MEMBER(wacko_ip2_r);
	DECLARE_READ8_MEMBER(kroozr_ip1_r);
	DECLARE_WRITE8_MEMBER(kroozr_op4_w);
	DECLARE_WRITE8_MEMBER(journey_op4_w);
	DECLARE_WRITE8_MEMBER(twotiger_op4_w);
	DECLARE_WRITE8_MEMBER(dotron_op4_w);
	DECLARE_READ8_MEMBER(nflfoot_ip2_r);
	DECLARE_WRITE8_MEMBER(nflfoot_op4_w);
	DECLARE_READ8_MEMBER(demoderb_ip1_r);
	DECLARE_READ8_MEMBER(demoderb_ip2_r);
	DECLARE_WRITE8_MEMBER(demoderb_op4_w);

	DECLARE_INPUT_CHANGED_MEMBER(dpoker_coin_in_hit);

	DECLARE_DRIVER_INIT(mcr_91490);
	DECLARE_DRIVER_INIT(kroozr);
	DECLARE_DRIVER_INIT(solarfox);
	DECLARE_DRIVER_INIT(kick);
	DECLARE_DRIVER_INIT(dpoker);
	DECLARE_DRIVER_INIT(twotiger);
	DECLARE_DRIVER_INIT(demoderb);
	DECLARE_DRIVER_INIT(wacko);
	DECLARE_DRIVER_INIT(mcr_90010);
	DECLARE_DRIVER_INIT(dotrone);
	DECLARE_DRIVER_INIT(nflfoot);
	DECLARE_DRIVER_INIT(journey);

	TILE_GET_INFO_MEMBER(mcr_90009_get_tile_info);
	TILE_GET_INFO_MEMBER(mcr_90010_get_tile_info);
	TILE_GET_INFO_MEMBER(mcr_91490_get_tile_info);
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




/*----------- defined in video/mcr.c -----------*/

extern INT8 mcr12_sprite_xoffs;
extern INT8 mcr12_sprite_xoffs_flip;

VIDEO_START( mcr );



SCREEN_UPDATE_IND16( mcr );
