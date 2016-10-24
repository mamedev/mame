// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Midway MCR system

**************************************************************************/

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/watchdog.h"
#include "audio/midway.h"
#include "sound/samples.h"

/* constants */
#define MAIN_OSC_MCR_I      XTAL_19_968MHz


class mcr_state : public driver_device
{
public:
	mcr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ipu(*this, "ipu"),
		m_watchdog(*this, "watchdog"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_sio(*this, "ipu_sio"),
		m_ssio(*this, "ssio"),
		m_chip_squeak_deluxe(*this, "csd"),
		m_sounds_good(*this, "sg"),
		m_turbo_chip_squeak(*this, "tcs"),
		m_squawk_n_talk(*this, "snt"),
		m_dpoker_coin_in_timer(*this, "dp_coinin"),
		m_dpoker_hopper_timer(*this, "dp_hopper"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<z80_device> m_maincpu;
	optional_device<cpu_device> m_ipu;
	required_device<watchdog_timer_device> m_watchdog;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_paletteram;

	optional_device<z80dart_device> m_sio;
	optional_device<midway_ssio_device> m_ssio;
	optional_device<midway_chip_squeak_deluxe_device> m_chip_squeak_deluxe;
	optional_device<midway_sounds_good_device> m_sounds_good;
	optional_device<midway_turbo_chip_squeak_device> m_turbo_chip_squeak;
	optional_device<midway_squawk_n_talk_device> m_squawk_n_talk;
	optional_device<timer_device> m_dpoker_coin_in_timer;
	optional_device<timer_device> m_dpoker_hopper_timer;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_sio_txda;
	int m_sio_txdb;

	void mcr_control_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcr_ipu_laserdisk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcr_ipu_watchdog_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcr_ipu_watchdog_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcr_paletteram9_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcr_90009_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcr_90010_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t twotiger_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void twotiger_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcr_91490_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t solarfox_ip0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t solarfox_ip1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dpoker_ip0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dpoker_lamps1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dpoker_lamps2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dpoker_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dpoker_meters_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t kick_ip1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wacko_op4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t wacko_ip1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t wacko_ip2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t kroozr_ip1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kroozr_op4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void journey_op4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void twotiger_op4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dotron_op4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nflfoot_ip2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nflfoot_op4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t demoderb_ip1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t demoderb_ip2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void demoderb_op4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void dpoker_coin_in_hit(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void init_mcr_91490();
	void init_kroozr();
	void init_solarfox();
	void init_kick();
	void init_dpoker();
	void init_twotiger();
	void init_demoderb();
	void init_wacko();
	void init_mcr_90010();
	void init_dotrone();
	void init_nflfoot();
	void init_journey();

	void mcr_90009_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mcr_90010_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mcr_91490_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_mcr();
	void machine_reset_mcr();
	void video_start_mcr();
	void machine_start_nflfoot();
	uint32_t screen_update_mcr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ipu_watchdog_reset(void *ptr, int32_t param);
	void dpoker_hopper_callback(timer_device &timer, void *ptr, int32_t param);
	void dpoker_coin_in_callback(timer_device &timer, void *ptr, int32_t param);
	void mcr_interrupt(timer_device &timer, void *ptr, int32_t param);
	void mcr_ipu_interrupt(timer_device &timer, void *ptr, int32_t param);
	void sio_txda_w(int state);
	void sio_txdb_w(int state);
	void mcr_set_color(int index, int data);
	void journey_set_color(int index, int data);
	void render_sprites_91399(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void render_sprites_91464(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int sprmask, int colormask);
	void mcr_init(int cpuboard, int vidboard, int ssioboard);
};

/*----------- defined in machine/mcr.c -----------*/

extern const z80_daisy_config mcr_daisy_chain[];
extern const z80_daisy_config mcr_ipu_daisy_chain[];
extern uint8_t mcr_cocktail_flip;

extern const gfx_layout mcr_bg_layout;
extern const gfx_layout mcr_sprite_layout;

extern uint32_t mcr_cpu_board;
extern uint32_t mcr_sprite_board;

/*----------- defined in video/mcr.c -----------*/

extern int8_t mcr12_sprite_xoffs;
extern int8_t mcr12_sprite_xoffs_flip;
