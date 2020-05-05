// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_DMAC_0266_H
#define MAME_MACHINE_DMAC_0266_H

#pragma once

class dmac_0266_device : public device_t
{
public:
	dmac_0266_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_bus(T &&tag, int spacenum) { m_bus.set_tag(std::forward<T>(tag), spacenum); }

	auto out_eop_cb() { return m_eop.bind(); }
	auto dma_r_cb() { return m_dma_r.bind(); }
	auto dma_w_cb() { return m_dma_w.bind(); }

	void map(address_map &map);

	void drq_w(int state);

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// register handlers
	u32 control_r() { return m_control; }
	u32 status_r() { return m_status; }
	u32 tcount_r() { return m_tcount; }

	void control_w(u32 data);
	void tcount_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_tcount); }
	void tag_w(offs_t offset, u32 data, u32 mem_mask)    { COMBINE_DATA(&m_tag); }
	void offset_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_offset); }
	void entry_w(offs_t offset, u32 data, u32 mem_mask)  { COMBINE_DATA(&m_map[m_tag & 0x7f]); }

	// dma logic
	void soft_reset();
	void set_eop(bool eop_state);
	void dma_check(void *ptr, s32 param);

private:
	required_address_space m_bus;

	devcb_write_line m_eop;
	devcb_read8 m_dma_r;
	devcb_write8 m_dma_w;

	emu_timer *m_dma_check;

	enum status_mask : u32
	{
		ENABLE    = 0x01,
		DIRECTION = 0x02,
		RESET     = 0x04,
		INTERRUPT = 0x08,
		TCZERO    = 0x10,
	};

	// registers
	u32 m_control;
	u32 m_status;
	u32 m_tcount;
	u32 m_tag;
	u32 m_offset;
	u32 m_map[128];

	// internal state
	bool m_eop_state;
	bool m_drq_state;
};

DECLARE_DEVICE_TYPE(DMAC_0266, dmac_0266_device)

#endif // MAME_MACHINE_DMAC_0266_H
