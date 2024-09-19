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

#define IR_TIMING               1       /* try to emulate MB and VG running time */

class irobot_state : public driver_device
{
public:
	irobot_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
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
		m_leds(*this, "led%u", 0U)
	{ }

	void init_irobot();

	void irobot(machine_config &config);

private:
	struct irmb_ops
	{
		const struct irmb_ops *nxtop;
		uint32_t func;
		uint32_t diradd;
		uint32_t latchmask;
		uint32_t *areg;
		uint32_t *breg;
		uint8_t cycles;
		uint8_t diren;
		uint8_t flags;
		uint8_t ramsel;
	};

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void irobot_map(address_map &map) ATTR_COLD;

	void irobot_clearirq_w(uint8_t data);
	void irobot_clearfirq_w(uint8_t data);
	uint8_t irobot_sharedmem_r(offs_t offset);
	void irobot_sharedmem_w(offs_t offset, uint8_t data);
	void irobot_statwr_w(uint8_t data);
	void irobot_out0_w(uint8_t data);
	void irobot_rom_banksel_w(uint8_t data);
	uint8_t irobot_status_r();
	void irobot_paletteram_w(offs_t offset, uint8_t data);
	uint8_t quad_pokeyn_r(offs_t offset);
	void quad_pokeyn_w(offs_t offset, uint8_t data);
	void irobot_palette(palette_device &palette) const;
	uint32_t screen_update_irobot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(irobot_irvg_done_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(irobot_irmb_done_callback);
	void irobot_poly_clear(uint8_t *bitmap_base);
	void irobot_poly_clear();
	void draw_line(uint8_t *polybitmap, int x1, int y1, int x2, int y2, int col);
	void irobot_run_video();
	uint32_t irmb_din(const irmb_ops *curop);
	void irmb_dout(const irmb_ops *curop, uint32_t d);
	void load_oproms();
	void irmb_run();

	required_shared_ptr<uint8_t> m_videoram;
	uint8_t m_vg_clear = 0U;
	uint8_t m_bufsel = 0U;
	uint8_t m_alphamap = 0U;
	uint8_t *m_combase = nullptr;
	uint8_t m_irvg_vblank = 0U;
	uint8_t m_irvg_running = 0U;
	uint8_t m_irmb_running = 0U;
	uint8_t *m_comRAM[2]{};
	uint8_t *m_mbRAM = nullptr;
	uint8_t *m_mbROM = nullptr;
	uint8_t m_statwr = 0U;
	uint8_t m_out0 = 0U;
	uint8_t m_outx = 0U;
	uint8_t m_mpage = 0U;
	uint8_t *m_combase_mb = nullptr;
	std::unique_ptr<irmb_ops[]> m_mbops{};
	const irmb_ops *m_irmb_stack[16]{};
	uint32_t m_irmb_regs[16]{};
	uint32_t m_irmb_latch = 0U;
	std::unique_ptr<uint8_t[]> m_polybitmap1{};
	std::unique_ptr<uint8_t[]> m_polybitmap2{};
	int m_ir_xmin = 0;
	int m_ir_ymin = 0;
	int m_ir_xmax = 0;
	int m_ir_ymax = 0;
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
	output_finder<2> m_leds;
};

#endif // MAME_ATARI_IROBOT_H
