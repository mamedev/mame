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

	kaneko_grap2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// getters
	u16 *framebuffer() { return m_framebuffer.get(); }
	u16 framebuffer_scrollx() { return m_framebuffer_scrollx; }
	u16 framebuffer_scrolly() { return m_framebuffer_scrolly; }
	u16 framebuffer_enable() { return m_framebuffer_enable; }

	// handlers
	u16 regs1_r(offs_t offset, u16 mem_mask = ~0);
	void regs1_go_w(u16 data);

	void framebuffer1_enable_w(u16 data) { m_framebuffer_enable = data; }

	void framebuffer1_scrolly_w(u16 data) { m_framebuffer_scrolly = data; }
	void framebuffer1_scrollx_w(u16 data) { m_framebuffer_scrollx = data; }

	u16 framebuffer1_fbbright1_r() { return m_framebuffer_bright1; }
	u16 framebuffer1_fbbright2_r() { return m_framebuffer_bright2; }

	void framebuffer1_fbbright1_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_framebuffer_bright1); }
	void framebuffer1_fbbright2_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_framebuffer_bright2); }

	void framebuffer1_bgcol_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void regs1_address_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_regs1_address_regs[offset]); }
	void regs2_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_regs2); }

	u16 framebuffer_r(offs_t offset) { return m_framebuffer[offset]; }
	void framebuffer_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_framebuffer[offset]); }
	u16 pal_r(offs_t offset) { return m_framebuffer_palette[offset]; }
	void framebuffer1_palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 unk1_r(offs_t offset) { return m_framebuffer_unk1[offset]; }
	void unk1_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_framebuffer_unk1[offset]); }
	u16 unk2_r(offs_t offset) { return m_framebuffer_unk2[offset]; }
	void unk2_w(offs_t offset, u16 data, u16 mem_mask = ~0) { COMBINE_DATA(&m_framebuffer_unk2[offset]); }

	void grap2_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface overrides
	virtual u32 palette_entries() const noexcept override { return PALETTE_SIZE; }

private:
	void do_rle(u32 address, u32 dstaddress);
	void set_color_555(pen_t color, int rshift, int gshift, int bshift, u16 data);

	u16 m_framebuffer_scrolly;
	u16 m_framebuffer_scrollx;
	u16 m_framebuffer_enable;
	u8 m_regs1_i;

	u16 m_framebuffer_bright1;
	u16 m_framebuffer_bright2;

	u16 m_regs1_address_regs[0x2];
	u16 m_regs2;

	std::unique_ptr<u16[]> m_framebuffer;
	std::unique_ptr<u16[]> m_framebuffer_palette;
	std::unique_ptr<u16[]> m_framebuffer_unk1;
	std::unique_ptr<u16[]> m_framebuffer_unk2;
};


DECLARE_DEVICE_TYPE(KANEKO_GRAP2, kaneko_grap2_device)


#endif // MAME_KANEKO_KANEKO_GRAP2_H
