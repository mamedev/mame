// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SONY_DMAC_0266_H
#define MAME_SONY_DMAC_0266_H

#pragma once

class dmac_0266_device : public device_t
{
public:
	dmac_0266_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }

	auto dma_r_cb() { return m_dma_r.bind(); }
	auto dma_w_cb() { return m_dma_w.bind(); }

	void map(address_map &map) ATTR_COLD;

	void eop_w(int state);
	void req_w(int state);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// register handlers
	u32 control_r() { return m_control; }
	u32 status_r() { return m_status | (m_control & (DIRECTION | ENABLE)); }
	u32 tcount_r() { return m_tcount; }

	void control_w(u32 data);
	void tcount_w(u32 data) { m_tcount = data; }
	void tag_w(u32 data)    { m_tag = data; }
	void offset_w(u32 data) { m_offset = data; }
	void entry_w(u32 data)  { m_map[m_tag & 0x7f] = data & 0x7fff; }

	// dma logic
	void soft_reset();
	void dma_check(s32 param);

private:
	required_address_space m_bus;

	devcb_read8 m_dma_r;
	devcb_write8 m_dma_w;

	emu_timer *m_dma_check;

	enum control_mask : u32
	{
		ENABLE    = 0x01,
		DIRECTION = 0x02,
		RESET     = 0x04,
	};

	enum status_mask : u32
	{
		INTERRUPT = 0x08,
		TCZERO    = 0x10,
	};

	// registers
	u32 m_control;
	u32 m_status;
	u32 m_tcount;
	u32 m_tag;
	u32 m_offset;
	u32 m_map[129];

	// internal state
	bool m_req_state;
};

DECLARE_DEVICE_TYPE(DMAC_0266, dmac_0266_device)

#endif // MAME_SONY_DMAC_0266_H
