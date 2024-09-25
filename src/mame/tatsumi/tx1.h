// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

    TX-1/Buggy Boy hardware

*************************************************************************/
#ifndef MAME_TATSUMI_TX1_H
#define MAME_TATSUMI_TX1_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "tx1_a.h"

#define TX1_PIXEL_CLOCK     (XTAL(18'000'000) / 3)
#define TX1_HBSTART         256
#define TX1_HBEND           0
#define TX1_HTOTAL          384
#define TX1_VBSTART         240
#define TX1_VBEND           0
#define TX1_VTOTAL          264

/*
 * HACK! Increased VTOTAL to 'fix' a timing issue
 * that prevents one of the start countdown tones
 * from playing.
 */
#define BB_PIXEL_CLOCK      (XTAL(18'000'000) / 3)
#define BB_HBSTART          256
#define BB_HBEND            0
#define BB_HTOTAL           384
#define BB_VBSTART          240
#define BB_VBEND            0
#define BB_VTOTAL           288 + 1

#define CPU_MASTER_CLOCK    (XTAL(15'000'000))
#define BUGGYBOY_ZCLK       (CPU_MASTER_CLOCK / 2)


class tx1_state : public driver_device
{
public:
	tx1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mathcpu(*this, "math_cpu"),
		m_maincpu(*this, "main_cpu"),
		m_math_ram(*this, "math_ram"),
		m_vram(*this, "vram"),
		m_objram(*this, "objram"),
		m_rcram(*this, "rcram"),
		m_char_tiles(*this, "char_tiles"),
		m_obj_tiles(*this, "obj_tiles"),
		m_road_rom(*this, "road"),
		m_obj_map(*this, "obj_map"),
		m_obj_luts(*this, "obj_luts"),
		m_proms(*this, "proms"),
		m_screen(*this, "screen"),
		m_sound(*this, "soundbrd")
	{ }

	void tx1(machine_config &config);
	void tx1j(machine_config &config);
	void buggyboy(machine_config &config);
	void buggybjr(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	struct math_t
	{
		uint16_t    cpulatch = 0;
		uint16_t    promaddr = 0;
		uint16_t    inslatch = 0;
		uint32_t    mux = 0;
		uint16_t    ppshift = 0;
		uint32_t    i0ff = 0;
		uint16_t    retval = 0;
		uint16_t    muxlatch = 0;   // TX-1
		int         dbgaddr = 0;
		int         dbgpc = 0;

		uint16_t get_datarom_addr() const;
		uint16_t get_bb_datarom_addr() const;
	};

	// SN74S516 16x16 Multiplier/Divider
	class sn74s516_t
	{
	public:
		int16_t   X = 0;
		int16_t   Y = 0;

		union
		{
#ifdef LSB_FIRST
			struct { uint16_t W; int16_t Z; } as16bit;
#else
			struct { int16_t Z; uint16_t W; } as16bit;
#endif
			int32_t ZW32 = 0;
		} ZW;

		int     code = 0;
		int     state = 0;
		int     ZWfl = 0;

		void kick(running_machine &machine, math_t &math, uint16_t *data, int ins);

	private:
		void multiply(running_machine &machine);
		void divide(running_machine &machine);
		void update(running_machine &machine, int ins);
	};

	struct vregs_t
	{
		uint16_t  scol = 0;       /* Road colours */
		uint32_t  slock = 0;      /* Scroll lock */
		uint8_t   flags = 0;      /* Road flags */

		uint32_t  ba_val = 0;     /* Accumulator */
		uint32_t  ba_inc = 0;
		uint32_t  bank_mode = 0;

		uint16_t  h_val = 0;      /* Accumulator */
		uint16_t  h_inc = 0;
		uint16_t  h_init = 0;

		uint8_t   slin_val = 0;   /* Accumulator */
		uint8_t   slin_inc = 0;

		/* Buggyboy only */
		uint8_t   wa8 = 0;
		uint8_t   wa4 = 0;

		uint16_t  wave_lfsr = 0;
		uint8_t   sky = 0;
		uint16_t  gas = 0;
		uint8_t   shift = 0;
	};

	math_t m_math;
	sn74s516_t m_sn74s516;

	required_device<cpu_device> m_mathcpu;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_math_ram;
	required_shared_ptr<uint16_t> m_vram;
	required_shared_ptr<uint16_t> m_objram;
	required_shared_ptr<uint16_t> m_rcram;

	required_region_ptr<uint8_t> m_char_tiles;
	required_region_ptr<uint8_t> m_obj_tiles;
	required_region_ptr<uint8_t> m_road_rom;
	required_region_ptr<uint8_t> m_obj_map;
	required_region_ptr<uint8_t> m_obj_luts;
	required_region_ptr<uint8_t> m_proms;

	required_device<screen_device> m_screen;
	required_device<tx1_sound_device> m_sound;

	emu_timer *m_interrupt_timer = nullptr;

	vregs_t m_vregs;
	std::unique_ptr<uint8_t[]> m_chr_bmp;
	std::unique_ptr<uint8_t[]> m_obj_bmp;
	std::unique_ptr<uint8_t[]> m_rod_bmp;
	std::unique_ptr<bitmap_ind16> m_bitmap;

	bool m_needs_update = false;

	void kick_sn74s516(uint16_t *data, int ins);
	void tx1_update_state();
	void buggyboy_update_state();

	uint16_t tx1_math_r(offs_t offset);
	void tx1_math_w(offs_t offset, uint16_t data);
	uint16_t tx1_spcs_rom_r(offs_t offset);
	uint16_t tx1_spcs_ram_r(offs_t offset);
	void tx1_spcs_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t buggyboy_math_r(offs_t offset);
	void buggyboy_math_w(offs_t offset, uint16_t data);
	uint16_t buggyboy_spcs_rom_r(offs_t offset);
	void buggyboy_spcs_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t buggyboy_spcs_ram_r(offs_t offset);
	uint16_t tx1_crtc_r();
	void tx1_crtc_w(offs_t offset, uint16_t data);
	void tx1_bankcs_w(offs_t offset, uint16_t data);
	void tx1_slincs_w(offs_t offset, uint16_t data);
	void tx1_slock_w(uint16_t data);
	void tx1_scolst_w(uint16_t data);
	void tx1_flgcs_w(uint16_t data);
	void buggyboy_gas_w(offs_t offset, uint16_t data);
	void buggyboy_sky_w(uint16_t data);
	void buggyboy_scolst_w(uint16_t data);
	void resume_math_w(uint16_t data);
	void halt_math_w(uint16_t data);
	u16 dipswitches_r();
	DECLARE_VIDEO_START(tx1);
	void tx1_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(buggyboy);
	void buggyboy_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(buggybjr);

	void tx1_draw_char(uint8_t *bitmap);
	void tx1_draw_road_pixel(int screen, uint8_t *bmpaddr,
			uint8_t apix[3], uint8_t bpix[3], uint32_t pixnuma, uint32_t pixnumb,
			uint8_t stl, uint8_t sld, uint8_t selb,
			uint8_t bnk, uint8_t rorev, uint8_t eb, uint8_t r, uint8_t delr);
	void tx1_draw_road(uint8_t *bitmap);
	void tx1_draw_objects(uint8_t *bitmap);
	void tx1_update_layers();
	void tx1_combine_layers(bitmap_ind16 &bitmap, int screen);

	void buggyboy_draw_char(uint8_t *bitmap, bool wide);
	void buggyboy_get_roadpix(int screen, int ls161, uint8_t rva0_6, uint8_t sld, uint32_t *_rorev,
			uint8_t *rc0, uint8_t *rc1, uint8_t *rc2, uint8_t *rc3);
	void buggyboy_draw_road(uint8_t *bitmap);
	void buggybjr_draw_road(uint8_t *bitmap);
	void buggyboy_draw_objs(uint8_t *bitmap, bool wide);
	void bb_combine_layers(bitmap_ind16 &bitmap, int screen);
	void bb_update_layers();

	uint32_t screen_update_tx1_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tx1_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tx1_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_buggyboy_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_buggyboy_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_buggyboy_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_buggybjr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_tx1(int state);
	void screen_vblank_buggyboy(int state);
	TIMER_CALLBACK_MEMBER(interrupt_callback);

	void buggybjr_main(address_map &map) ATTR_COLD;
	void buggyboy_main(address_map &map) ATTR_COLD;
	void buggyboy_math(address_map &map) ATTR_COLD;
	void tx1_main(address_map &map) ATTR_COLD;
	void tx1_math(address_map &map) ATTR_COLD;
};

#endif // MAME_TATSUMI_TX1_H
