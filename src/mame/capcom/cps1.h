// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

   Capcom CPS1/2 hardware

***************************************************************************/

#ifndef MAME_CAPCOM_CPS1_H
#define MAME_CAPCOM_CPS1_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "machine/timekpr.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/qsound.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


// Video raw params
// measured clocks:
// CPS2(Guru): V = 59.6376Hz, H = 15,4445kHz *H is probably measured too low!
// CPS1 GNG: V = 59.61Hz
/* CPS1(Charles MacDonald):
    Pixel clock: 8.00 MHz
    Total pixel clocks per scanline: 512 clocks
    Horizontal sync pulse width : 36 clocks
    Horizontal display and blanking period: 476 clocks
    Frame size: 262 scanlines
    Refresh rate: 59.63 MHz.
*/
#define CPS_PIXEL_CLOCK  (XTAL(16'000'000)/2)

#define CPS_HTOTAL       (512)
#define CPS_HBEND        (64)
#define CPS_HBSTART      (448)

#define CPS_VTOTAL       (262)
#define CPS_VBEND        (16)
#define CPS_VBSTART      (240)


struct gfx_range
{
	// start and end are as passed by the game (shift adjusted to be all
	// in the same scale a 8x8 tiles): they don't necessarily match the
	// position in ROM.
	int type = 0;
	int start = 0;
	int end = 0;
	int bank = 0;
};

struct CPS1config
{
	const char *name = nullptr;             /* game driver name */

	/* Some games interrogate a couple of registers on bootup. */
	/* These are CPS1 board B self test checks. They wander from game to */
	/* game. */
	int cpsb_addr = 0;        /* CPS board B test register address */
	int cpsb_value = 0;       /* CPS board B test register expected value */

	/* some games use as a protection check the ability to do 16-bit multiplies */
	/* with a 32-bit result, by writing the factors to two ports and reading the */
	/* result from two other ports. */
	/* It looks like this feature was introduced with 3wonders (CPSB ID = 08xx) */
	int mult_factor1 = 0;
	int mult_factor2 = 0;
	int mult_result_lo = 0;
	int mult_result_hi = 0;

	/* unknown registers which might be related to the multiply protection */
	int unknown1 = 0;
	int unknown2 = 0;
	int unknown3 = 0;

	int layer_control = 0;
	int priority[4]{};
	int palette_control = 0;

	/* ideally, the layer enable masks should consist of only one bit, */
	/* but in many cases it is unknown which bit is which. */
	int layer_enable_mask[5]{};

	/* these depend on the B-board model and PAL */
	int bank_sizes[4]{};
	const struct gfx_range *bank_mapper;

	/* some C-boards have additional I/O for extra buttons/extra players */
	int in2_addr = 0;
	int in3_addr = 0;
	int out2_addr = 0;

	int bootleg_kludge = 0;
};


class cps_state : public driver_device
{
public:
	cps_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mainram(*this, "mainram")
		, m_gfxram(*this, "gfxram")
		, m_cps_a_regs(*this, "cps_a_regs")
		, m_cps_b_regs(*this, "cps_b_regs")
		, m_qsound_sharedram(*this, "qsound_ram%u", 1U)
		, m_audiorom_raw(*this, "audiorom_raw")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_qsound(*this, "qsound")
		, m_m48t35(*this,"m48t35")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch%u", 1U)
		, m_region_stars(*this, "stars")
		, m_audioregion(*this, "audiocpu")
		, m_audiobank(*this, "audiobank")
		, m_io_in(*this, "IN%u", 0U)
		, m_dsw(*this, "DSW%c", 'A')
		, m_eepromout(*this, "EEPROMOUT")
		, m_dial(*this, "DIAL%u", 0U)
		, m_led_cboard(*this, "led_cboard%u", 0U)
	{ }

public:
	void cps1_10MHz(machine_config &config);
	void forgottn(machine_config &config);
	void cps1_12MHz(machine_config &config);
	void pang3(machine_config &config);
	void ganbare(machine_config &config);
	void qsound(machine_config &config);
	void wofhfh(machine_config &config);
	void sf2m3(machine_config &config);
	void sf2cems6(machine_config &config);
	void sf2m10(machine_config &config);
	void varthb2(machine_config &config);
	void varthb3(machine_config &config);

	void init_cps1mult();
	void init_sf2ee();
	void init_wof();
	void init_dino();
	void init_punisher();
	void init_slammast();
	void init_pang3();
	void init_rasters();
	void init_ganbare();
	void init_pang3b();
	void init_pang3b4();
	void init_sf2rb();
	void init_sf2rb2();
	void init_sf2thndr();
	void init_sf2hack();
	void init_sf2rk();
	void init_sf2dongb();
	void init_sf2ceblp();
	void init_sf2m8();
	void init_dinohunt();

protected:
	// Offset of each palette entry
	static constexpr uint32_t CPS1_PALETTE_ENTRIES = 32 * 6;      // Number colour schemes in palette

	// CPS-A registers
	static constexpr unsigned CPS1_OBJ_BASE        = 0x00 / 2;    // Base address of objects
	static constexpr unsigned CPS1_SCROLL1_BASE    = 0x02 / 2;    // Base address of scroll 1
	static constexpr unsigned CPS1_SCROLL2_BASE    = 0x04 / 2;    // Base address of scroll 2
	static constexpr unsigned CPS1_SCROLL3_BASE    = 0x06 / 2;    // Base address of scroll 3
	static constexpr unsigned CPS1_OTHER_BASE      = 0x08 / 2;    // Base address of other video
	static constexpr unsigned CPS1_PALETTE_BASE    = 0x0a / 2;    // Base address of palette
	static constexpr unsigned CPS1_SCROLL1_SCROLLX = 0x0c / 2;    // Scroll 1 X
	static constexpr unsigned CPS1_SCROLL1_SCROLLY = 0x0e / 2;    // Scroll 1 Y
	static constexpr unsigned CPS1_SCROLL2_SCROLLX = 0x10 / 2;    // Scroll 2 X
	static constexpr unsigned CPS1_SCROLL2_SCROLLY = 0x12 / 2;    // Scroll 2 Y
	static constexpr unsigned CPS1_SCROLL3_SCROLLX = 0x14 / 2;    // Scroll 3 X
	static constexpr unsigned CPS1_SCROLL3_SCROLLY = 0x16 / 2;    // Scroll 3 Y
	static constexpr unsigned CPS1_STARS1_SCROLLX  = 0x18 / 2;    // Stars 1 X
	static constexpr unsigned CPS1_STARS1_SCROLLY  = 0x1a / 2;    // Stars 1 Y
	static constexpr unsigned CPS1_STARS2_SCROLLX  = 0x1c / 2;    // Stars 2 X
	static constexpr unsigned CPS1_STARS2_SCROLLY  = 0x1e / 2;    // Stars 2 Y
	static constexpr unsigned CPS1_ROWSCROLL_OFFS  = 0x20 / 2;    // base of row scroll offsets in other RAM
	static constexpr unsigned CPS1_VIDEOCONTROL    = 0x22 / 2;    // flip screen, rowscroll enable

	virtual void device_post_load() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	DECLARE_MACHINE_START(common);
	DECLARE_MACHINE_START(cps1);
	DECLARE_MACHINE_START(qsound);
	DECLARE_MACHINE_START(ganbare);
	DECLARE_MACHINE_RESET(cps);

	uint16_t cps1_dsw_r(offs_t offset);
	template <unsigned Which> uint16_t cps1_in_r()
	{
		const int in = m_io_in[Which]->read();
		return (in << 8) | in;
	}
	void cps1_coinctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cpsq_coinctrl2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_cps_a_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cps1_cps_b_r(offs_t offset);
	void cps1_cps_b_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_soundlatch2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_snd_bankswitch_w(uint8_t data);
	void cps1_oki_pin7_w(uint8_t data);
	uint16_t qsound_rom_r(offs_t offset);
	template <unsigned Which> uint16_t qsound_sharedram_r(offs_t offset)
	{
		return m_qsound_sharedram[Which][offset] | 0xff00;
	}
	template <unsigned Which> void qsound_sharedram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0)
	{
		if (ACCESSING_BITS_0_7)
			m_qsound_sharedram[Which][offset] = data;
	}
	void qsound_banksw_w(uint8_t data);
	uint16_t ganbare_ram_r(offs_t offset, uint16_t mem_mask = ~0);
	void ganbare_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cps1_hack_dsw_r(offs_t offset);
	uint16_t sf2rb_prot_r(offs_t offset);
	uint16_t sf2rb2_prot_r(offs_t offset);
	uint16_t sf2dongb_prot_r(offs_t offset);
	uint16_t sf2ceblp_prot_r();
	void sf2ceblp_prot_w(uint16_t data);
	void sf2m3_layer_w(offs_t offset, uint16_t data);
	uint16_t dinohunt_sound_r();
	void varthb2_cps_a_w(offs_t offset, uint16_t data);
	uint16_t pang3b4_prot_r();
	void pang3b4_prot_w(uint16_t data);

	TILEMAP_MAPPER_MEMBER(tilemap0_scan);
	TILEMAP_MAPPER_MEMBER(tilemap1_scan);
	TILEMAP_MAPPER_MEMBER(tilemap2_scan);
	TILE_GET_INFO_MEMBER(get_tile0_info);
	TILE_GET_INFO_MEMBER(get_tile1_info);
	TILE_GET_INFO_MEMBER(get_tile2_info);

	INTERRUPT_GEN_MEMBER(cps1_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(raster_scanline);
	TIMER_CALLBACK_MEMBER(raster_irq);
	uint16_t irqack_r(offs_t offset);

	virtual void render_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cps1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_cps1(int state);
	void cps1_objram_latch(int state);

	/* capcom/cps1_v.cpp */
	inline uint16_t *cps1_base(int offset, int boundary);
	void cps1_get_video_base();
	int gfxrom_bank_mapper(int type, int code);
	void cps1_update_transmasks();
	void cps1_build_palette(const uint16_t* const palette_base);
	virtual void find_last_sprite();
	void cps1_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cps1_render_stars(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cps1_render_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask);
	void cps1_render_high_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);

	void kabuki_setup(void (*decode)(uint8_t *src, uint8_t *dst));

	/* maps */
	void main_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
	void forgottn_map(address_map &map) ATTR_COLD;
	void qsound_main_map(address_map &map) ATTR_COLD;
	void qsound_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void qsound_sub_map(address_map &map) ATTR_COLD;
	void sf2m3_map(address_map &map) ATTR_COLD;
	void sf2cems6_map(address_map &map) ATTR_COLD;
	void sf2m10_map(address_map &map) ATTR_COLD;
	void varthb2_map(address_map &map) ATTR_COLD;
	void varthb3_map(address_map &map) ATTR_COLD;

	// game-specific
	uint16_t m_sf2ceblp_prot = 0;
	uint16_t m_pang3b4_prot = 0;
	std::unique_ptr<uint8_t[]> m_decrypt_kabuki;

	/* video-related */
	std::unique_ptr<uint16_t[]> m_buffered_obj;
	tilemap_t *m_bg_tilemap[3]{};
	uint16_t m_raster_counter[3]{};
	uint16_t m_raster_reload[3]{};
	emu_timer *m_raster_irq = nullptr;

	int32_t m_scrollx[3]{};
	int32_t m_scrolly[3]{};

	int32_t m_stars_enabled[2]{};        /* Layer enabled [Y/N] */
	int32_t m_starsx[2]{};
	int32_t m_starsy[2]{};
	int32_t m_last_sprite_offset = 0;      /* Offset of the last sprite */

	bitmap_ind16 m_dummy_bitmap;

	/* video config (never changed after video_start) */
	const struct CPS1config *m_game_config;
	int32_t m_scroll_size = 0;
	int32_t m_obj_size = 0;
	int32_t m_other_size = 0;
	int32_t m_palette_align = 0;
	int32_t m_palette_size = 0;
	int32_t m_stars_rom_size = 0;
	uint8_t m_empty_tile[32*32]{};

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_mainram;
	required_shared_ptr<uint16_t> m_gfxram;
	required_shared_ptr<uint16_t> m_cps_a_regs;
	required_shared_ptr<uint16_t> m_cps_b_regs;
	uint16_t *m_scroll[3]{};
	uint16_t *m_obj = nullptr;
	uint16_t *m_other = nullptr;
	optional_shared_ptr_array<uint8_t, 2> m_qsound_sharedram;

	optional_region_ptr<uint8_t> m_audiorom_raw;

	/* devices */
	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<qsound_device> m_qsound;
	optional_device<m48t35_device> m_m48t35;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device_array<generic_latch_8_device, 2> m_soundlatch;
	optional_memory_region m_region_stars;
	optional_memory_region m_audioregion;

	optional_memory_bank m_audiobank;

	optional_ioport_array<4> m_io_in;
	optional_ioport_array<3> m_dsw;
	optional_ioport m_eepromout;
	optional_ioport_array<2> m_dial;

	output_finder<3> m_led_cboard;
};


/*----------- defined in capcom/cps1.cpp -----------*/

extern gfx_decode_entry const gfx_cps1[];

INPUT_PORTS_EXTERN( dino );
INPUT_PORTS_EXTERN( knights );
INPUT_PORTS_EXTERN( mtwins );
INPUT_PORTS_EXTERN( punisher );
INPUT_PORTS_EXTERN( sf2 );
INPUT_PORTS_EXTERN( slammast );
INPUT_PORTS_EXTERN( varth );
INPUT_PORTS_EXTERN( captcomm );
INPUT_PORTS_EXTERN( wof );

#endif // MAME_CAPCOM_CPS1_H
