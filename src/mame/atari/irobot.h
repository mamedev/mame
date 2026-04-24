// license:BSD-3-Clause
// copyright-holders:Dan Boris
/*************************************************************************

    Atari I, Robot hardware

*************************************************************************/
#ifndef MAME_ATARI_IROBOT_H
#define MAME_ATARI_IROBOT_H

#pragma once

#include "machine/timer.h"
#include "machine/x2212.h"
#include "sound/pokey.h"
#include "emupal.h"

#include "screen.h"
#include "tilemap.h"

#define IR_TIMING               1       /* try to emulate MB and VG running time */
#define DISASSEMBLE_MB_ROM      0       /* generate a disassembly of the mathbox ROMs */

class irobot_state : public driver_device
{
public:
	irobot_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
#if IR_TIMING
		m_irvg_timer(*this, "irvg_timer"),
		m_irmb_timer(*this, "irmb_timer"),
#endif
		m_novram(*this, "nvram"),
		m_pokey(*this, "pokey%u", 1U),
		m_videoram(*this, "videoram"),
		m_mathboxrom(*this, "mathbox"),
		m_mathboxram(*this, "mathboxram", 0x2000U, ENDIANNESS_BIG),
		m_commram(*this, "commram_%u", 0U, 0x1000U, ENDIANNESS_BIG),
		m_bankedram(*this, "bankedram", 0x800U*3U, ENDIANNESS_BIG),
		m_rombank(*this, "rombank"),
		m_rambank(*this, "rambank"),
		m_leds(*this, "led%u", 0U)
	{ }

	void init_irobot() ATTR_COLD;

	void irobot(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	enum mathbox_fl
	{
		FL_MULT = 0x01,
		FL_SHIFT = 0x02,
		FL_MBMEMDEC = 0x04,
		FL_ADDEN = 0x08,
		FL_DPSEL = 0x10,
		FL_CARRY = 0x20,
		FL_DIV = 0x40,
		FL_MBRW = 0x80
	};

	struct irmb_ops
	{
		const irmb_ops *nxtop;
		u32 func;
		u32 nxtadd;
		u32 diradd;
		u32 latchmask;
		u32 *areg;
		u32 *breg;
		u8 cycles;
		u8 diren;
		u8 flags;
		u8 ramsel;
	};

	tilemap_t *m_tilemap = nullptr;
	u8 m_vg_clear = 0U;
	u8 m_bufsel = 0U;
	u8 m_alphamap = 0U;
	u8 m_irvg_vblank = 0U;
	u8 m_irvg_running = 0U;
	u8 m_irmb_running = 0U;
	u8 m_commbank = 0U;
	u8 m_statwr = 0U;
	u8 m_out0 = 0U;
	u8 m_outx = 0U;
	u8 m_mpage = 0U;
	std::unique_ptr<irmb_ops[]> m_mbops{};
	const irmb_ops *m_irmb_stack[16]{};
	u32 m_irmb_regs[16]{};
	u32 m_irmb_latch = 0U;
	std::unique_ptr<u8[]> m_polybitmap[2]{};
	s32 m_ir_xmin = 0;
	s32 m_ir_ymin = 0;
	s32 m_ir_xmax = 0;
	s32 m_ir_ymax = 0;
	emu_timer *m_scanline_timer = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
#if IR_TIMING
	required_device<timer_device> m_irvg_timer;
	required_device<timer_device> m_irmb_timer;
#endif
	required_device<x2212_device> m_novram;
	required_device_array<pokey_device, 4> m_pokey;

	required_shared_ptr<u8> m_videoram;
	required_region_ptr<u16> m_mathboxrom;
	memory_share_creator<u8> m_mathboxram;
	memory_share_array_creator<u8, 2> m_commram;
	memory_share_creator<u8> m_bankedram;
	required_memory_bank m_rombank;
	required_memory_bank m_rambank;
	output_finder<2> m_leds;

#if DISASSEMBLE_MB_ROM
	void disassemble_instruction(u16 offset, const irmb_ops *op);
#endif

	void main_map(address_map &map) ATTR_COLD;

	template <int Line> void clearirq_w(u8 data);
	u8 sharedmem_r(offs_t offset);
	void sharedmem_w(offs_t offset, u8 data);
	void statwr_w(u8 data);
	void out0_w(u8 data);
	void rom_banksel_w(u8 data);
	u8 status_r();
	void paletteram_w(offs_t offset, u8 data);
	void videoram_w(offs_t offset, u8 data);
	u8 quad_pokeyn_r(offs_t offset);
	void quad_pokeyn_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(irvg_done_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(irmb_done_callback);
	void poly_clear(u8 *bitmap_base);
	void poly_clear();
	void draw_line(u8 *polybitmap, int x1, int y1, int x2, int y2, int col);
	void run_video();

	u32 irmb_din(const irmb_ops *curop);
	void irmb_dout(const irmb_ops *curop, u32 d);
	void load_oproms();
	void irmb_run();
};

#endif // MAME_ATARI_IROBOT_H
