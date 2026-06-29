// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_TAOS_H
#define MAME_APPLE_TAOS_H

#pragma once

#include "emupal.h"
#include "screen.h"

class taos_device : public device_t
{
public:
	taos_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~taos_device() = default;

	void map(address_map &map) ATTR_COLD;

	void set_pixclock(u32 pclock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// Taos registers (many names from DAFB, which they share a resemblance to)
	enum
	{
		FB_BASE = 0,
		ROW_WORDS,
		COLOR_DEPTH,
		VIDEO_MODE,
		EVEN_FIELD_START,
		ODD_FIELD_START,
		HSTART,
		CONVOLUTION_END,
		CONTROL,
		TEST,

		HEQ = 0x0b,
		HBWAY,
		HAL,
		HSERR,
		HFP,
		HPIX,
		HSP,
		HLFLN,

		VBPEQ = 0x13,
		VBP,
		VAL,
		VFPEQ,
		VFP,
		VSYNC,
		VHLINE,

		GPIO_IN = 0x1a,
		GPIO_DDR,
		GPIO_OUT,
		INT_FLAG,
		INT_ENABLE,
		INT_CLEAR,

		VERSION = 0x23,
		INT_SET,
		HEB,

		TAOS_NUM_REGS
	};

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	std::unique_ptr<u32[]> m_vram;
	u32 m_hres, m_vres, m_htotal, m_vtotal;
	u32 m_taos_regs[TAOS_NUM_REGS];
	u32 m_pixel_clock;
	u8 m_pal_idx;

	u32 screen_update_taos(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);
	u32 taos_r(offs_t offset);
	void taos_w(offs_t offset, u32 data, u32 mem_mask);
	u32 clut_r(offs_t offset);
	void clut_w(offs_t offset, u32 data, u32 mem_mask);

	void rebuild_params();
};

DECLARE_DEVICE_TYPE(APPLE_TAOS, taos_device)

#endif  /* MAME_APPLE_TAOS_H */
