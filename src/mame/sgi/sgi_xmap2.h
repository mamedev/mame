// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_SGI_XMAP2_H
#define MAME_SGI_SGI_XMAP2_H

#pragma once

class sgi_xmap2_device
	: public device_t
	, public device_palette_interface
{
public:
	sgi_xmap2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface overrides
	virtual u32 palette_entries() const noexcept override { return 4096 + 16; }

	u8 reg_r(offs_t offset);
	void reg_w(offs_t offset, u8 data);

	void map_select_w(int state);

	u16 mode_r(unsigned const index) const { return m_mode[index]; }
	rgb_t overlay_r(u8 data) const { return m_wid_aux ? m_overlay[data & 0x3] : m_overlay[data & 0xf]; }

	enum mode_mask : u16
	{
		MODE_MC = 0x3c00, // multimap constant
		MODE_ME = 0x0200, // multimap enable
		MODE_UE = 0x0100, // underlay enable
		MODE_OE = 0x00f0, // overlay enable
		MODE_BS = 0x0008, // buffer select
		MODE_DM = 0x0007, // display mode
	};
	enum mode_bits : unsigned
	{
		BIT_ME = 9, // multimap enable
		BIT_UE = 8, // underlay enable
		BIT_BS = 3, // buffer select
	};
private:
	required_ioport m_options_port;

	u16 m_addr;
	rgb_t m_color[8192];
	rgb_t m_overlay[16];
	u16 m_mode[16];
	bool m_wid_aux;
	bool m_map_select;
	u8 m_options;
};

DECLARE_DEVICE_TYPE(SGI_XMAP2, sgi_xmap2_device)

#endif // MAME_SGI_SGI_XMAP2_H
