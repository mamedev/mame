// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SONY_DMAC_0448_H
#define MAME_SONY_DMAC_0448_H

#pragma once

class dmac_0448_device : public device_t
{
public:
	dmac_0448_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }
	auto out_int_cb() { return m_out_int.bind(); }
	template <unsigned Channel> auto dma_r_cb() { return m_dma_r[Channel].bind(); }
	template <unsigned Channel> auto dma_w_cb() { return m_dma_w[Channel].bind(); }

	// line handlers
	template <unsigned IRQ> void irq(int state) { set_irq_line(IRQ, state); }
	template <unsigned DRQ> void drq(int state) { set_drq_line(DRQ, state); }

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void set_irq_line(int number, int state);
	void set_drq_line(int channel, int state);

	u8 cstat_r() { return m_channel[m_gsel].cstat; }
	u8 ctrcl_r() { return u8(m_channel[m_gsel].ctrc >> 0); }
	u8 ctrcm_r() { return u8(m_channel[m_gsel].ctrc >> 8); }
	u8 ctrch_r() { return u8(m_channel[m_gsel].ctrc >> 16); }
	u8 ctag_r() { return m_channel[m_gsel].ctag; }
	u8 cwid_r() { return m_channel[m_gsel].cwid; }
	u8 cofsl_r() { return u8(m_channel[m_gsel].cofs >> 0); }
	u8 cofsh_r() { return u8(m_channel[m_gsel].cofs >> 8); }
	u16 cmap_r() { return m_channel[m_gsel].cmap[m_channel[m_gsel].ctag]; }
	u8 gstat_r() { return m_gstat; }

	void cctl_w(u8 data);
	void ctrcl_w(u8 data) { m_channel[m_gsel].ctrc = (m_channel[m_gsel].ctrc & 0xffff00U) | (u32(data) << 0); }
	void ctrcm_w(u8 data) { m_channel[m_gsel].ctrc = (m_channel[m_gsel].ctrc & 0xff00ffU) | (u32(data) << 8); }
	void ctrch_w(u8 data) { m_channel[m_gsel].ctrc = (m_channel[m_gsel].ctrc & 0x00ffffU) | (u32(data) << 16); }
	void ctag_w(u8 data) { m_channel[m_gsel].ctag = data; }
	void cwid_w(u8 data) { m_channel[m_gsel].cwid = data; }
	void cofsl_w(u8 data) { m_channel[m_gsel].cofs = (m_channel[m_gsel].cofs & 0xff00U) | (u16(data) << 0); }
	void cofsh_w(u8 data) { m_channel[m_gsel].cofs = (m_channel[m_gsel].cofs & 0x00ffU) | (u16(data & 0x0f) << 8); }
	void cmap_w(offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_channel[m_gsel].cmap[m_channel[m_gsel].ctag]); }
	void gsel_w(u8 data) { m_gsel = data; }

	void irq_check(s32 param = 0);
	void dma_check(s32 param = 0);

	required_address_space m_bus;

	devcb_write_line m_out_int;
	devcb_read8::array<4> m_dma_r;
	devcb_write8::array<4> m_dma_w;

	emu_timer *m_irq_check;
	emu_timer *m_dma_check;

	enum cstat_mask : u8
	{
		CS_ENABLE = 0x01, // channel enable
		CS_MODE   = 0x02, // transfer to memory
		CS_RESET  = 0x04, // reset channel
		CS_ZINTEN = 0x08, // terminal count interrupt?
		CS_APAD   = 0x10, // auto pad
		CS_AFIX   = 0x20,
		CS_A28    = 0x40,
		CS_TCZ    = 0x80, // transfer count zero?
	};

	struct dma_channel
	{
		u8 cstat; // channel status
		u8 cctl;  // channel control
		u32 ctrc; // channel counter
		u8 ctag;  // channel tag
		u8 cwid;  // channel width
		u16 cofs; // channel offset
		u16 cmap[256];
	}
	m_channel[4];
	u8 m_gsel;  // channel select
	u8 m_gstat; // general status

	bool m_out_int_state;
};

DECLARE_DEVICE_TYPE(DMAC_0448, dmac_0448_device)

#endif // MAME_SONY_DMAC_0448_H
