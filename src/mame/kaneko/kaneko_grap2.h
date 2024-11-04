// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KANEKO_KANEKO_GRAP2_H
#define MAME_KANEKO_KANEKO_GRAP2_H

#pragma once

#include "dirom.h"
#include "emupal.h"

// TODO : Unknown Address Bits
class kaneko_grap2_device : public device_t, public device_rom_interface<32>, public device_palette_interface
{
public:
	static constexpr unsigned PALETTE_SIZE = 256 + 1; // 0x00-0xff is internal palette, 0x100 is background colour

	kaneko_grap2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t regs1_r(offs_t offset, uint16_t mem_mask = ~0);
	void regs1_go_w(uint16_t data);

	void grap2_map(address_map &map) ATTR_COLD;

	void do_rle(uint32_t address);
	void set_color_555(pen_t color, int rshift, int gshift, int bshift, uint16_t data);

	uint16_t m_framebuffer_scrolly = 0;
	uint16_t m_framebuffer_scrollx = 0;
	uint16_t m_framebuffer_enable = 0;
	int m_regs1_i = 0;

	uint16_t m_framebuffer_bright1 = 0;
	uint16_t m_framebuffer_bright2 = 0;

	uint16_t m_regs1_address_regs[0x2]{};
	uint16_t m_regs2 = 0;

	void framebuffer1_enable_w(uint16_t data) { m_framebuffer_enable = data; }

	void framebuffer1_scrolly_w(uint16_t data) { m_framebuffer_scrolly = data; }
	void framebuffer1_scrollx_w(uint16_t data) { m_framebuffer_scrollx = data; }


	uint16_t framebuffer1_fbbright1_r() { return m_framebuffer_bright1; }
	uint16_t framebuffer1_fbbright2_r() { return m_framebuffer_bright2; }


	void framebuffer1_fbbright1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_framebuffer_bright1); }
	void framebuffer1_fbbright2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_framebuffer_bright2); }

	void framebuffer1_bgcol_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void regs1_address_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_regs1_address_regs[offset]); }
	void regs2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_regs2); }

	uint16_t framebuffer_r(offs_t offset) { return m_framebuffer[offset]; }
	void framebuffer_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_framebuffer[offset]); }
	uint16_t pal_r(offs_t offset) { return m_framebuffer_palette[offset]; }
	void framebuffer1_palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t unk1_r(offs_t offset) { return m_framebuffer_unk1[offset]; }
	void unk1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_framebuffer_unk1[offset]); }
	uint16_t unk2_r(offs_t offset) { return m_framebuffer_unk2[offset]; }
	void unk2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { COMBINE_DATA(&m_framebuffer_unk2[offset]); }

	std::unique_ptr<uint16_t[]> m_framebuffer;
	std::unique_ptr<uint16_t[]> m_framebuffer_palette;
	std::unique_ptr<uint16_t[]> m_framebuffer_unk1;
	std::unique_ptr<uint16_t[]> m_framebuffer_unk2;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return PALETTE_SIZE; }
};


DECLARE_DEVICE_TYPE(KANEKO_GRAP2, kaneko_grap2_device)


#endif // MAME_KANEKO_KANEKO_GRAP2_H
