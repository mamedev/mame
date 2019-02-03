// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_KANEKO_GRAP2_H
#define MAME_VIDEO_KANEKO_GRAP2_H

#pragma once

#include "emupal.h"

class kaneko_grap2_device : public device_t, public device_rom_interface, public device_palette_interface
{
public:
	static constexpr unsigned PALETTE_SIZE = 256 + 1; // 0x00-0xff is internal palette, 0x100 is background colour

	kaneko_grap2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(regs1_r);
	DECLARE_WRITE16_MEMBER(regs1_go_w);

	void grap2_map(address_map &map);

	void do_rle(uint32_t address);
	void set_color_555(pen_t color, int rshift, int gshift, int bshift, uint16_t data);

	uint16_t m_framebuffer_scrolly;
	uint16_t m_framebuffer_scrollx;
	uint16_t m_framebuffer_enable;
	int m_regs1_i;

	uint16_t m_framebuffer_bright1;
	uint16_t m_framebuffer_bright2;

	uint16_t m_regs1_address_regs[0x2];
	uint16_t m_regs2;

	DECLARE_WRITE16_MEMBER(framebuffer1_enable_w) { m_framebuffer_enable = data; }

	DECLARE_WRITE16_MEMBER(framebuffer1_scrolly_w) { m_framebuffer_scrolly = data; }
	DECLARE_WRITE16_MEMBER(framebuffer1_scrollx_w) { m_framebuffer_scrollx = data; }


	DECLARE_READ16_MEMBER(framebuffer1_fbbright1_r) { return m_framebuffer_bright1; }
	DECLARE_READ16_MEMBER(framebuffer1_fbbright2_r) { return m_framebuffer_bright2; }


	DECLARE_WRITE16_MEMBER(framebuffer1_fbbright1_w) { COMBINE_DATA(&m_framebuffer_bright1); }
	DECLARE_WRITE16_MEMBER(framebuffer1_fbbright2_w) { COMBINE_DATA(&m_framebuffer_bright2); }

	DECLARE_WRITE16_MEMBER(framebuffer1_bgcol_w);

	DECLARE_WRITE16_MEMBER(regs1_address_w) { COMBINE_DATA(&m_regs1_address_regs[offset]); }
	DECLARE_WRITE16_MEMBER(regs2_w) { COMBINE_DATA(&m_regs2); }

	DECLARE_READ16_MEMBER(  framebuffer_r ) { return m_framebuffer[offset]; }
	DECLARE_WRITE16_MEMBER( framebuffer_w ) { COMBINE_DATA(&m_framebuffer[offset]); }
	DECLARE_READ16_MEMBER(  pal_r ) { return m_framebuffer_palette[offset]; }
	DECLARE_WRITE16_MEMBER(framebuffer1_palette_w);
	DECLARE_READ16_MEMBER(  unk1_r ) { return m_framebuffer_unk1[offset]; }
	DECLARE_WRITE16_MEMBER( unk1_w ) { COMBINE_DATA(&m_framebuffer_unk1[offset]); }
	DECLARE_READ16_MEMBER(  unk2_r ) { return m_framebuffer_unk2[offset]; }
	DECLARE_WRITE16_MEMBER( unk2_w ) { COMBINE_DATA(&m_framebuffer_unk2[offset]); }

	std::unique_ptr<uint16_t[]> m_framebuffer;
	std::unique_ptr<uint16_t[]> m_framebuffer_palette;
	std::unique_ptr<uint16_t[]> m_framebuffer_unk1;
	std::unique_ptr<uint16_t[]> m_framebuffer_unk2;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void rom_bank_updated() override;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const override { return PALETTE_SIZE; }
};


DECLARE_DEVICE_TYPE(KANEKO_GRAP2, kaneko_grap2_device)


#endif // MAME_VIDEO_KANEKO_GRAP2_H
