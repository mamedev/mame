// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_TSCONF_COPPER_H
#define MAME_SINCLAIR_TSCONF_COPPER_H

#pragma once

class tsconf_copper_device : public device_t
{

public:
	tsconf_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_wreg_cb() { return m_out_wreg_cb.bind(); }
	template <typename... T> void set_in_until_pos_cb(T &&... args) { return m_in_until_pos_cb.set(std::forward<T>(args)...); }

	void copper_en_w(u8 data);
	void cl_data_w(u16 addr, u8 data);
	void dma_ready_w(int status);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_callback);

	emu_timer *m_timer;

private:
	memory_share_creator<u16> m_cl_data;

	devcb_write8 m_out_wreg_cb;
	devcb_read_line m_in_dma_ready_cb;
	device_delegate<attotime (u16)> m_in_until_pos_cb;

	bool m_en;
	u8 m_pc;
	u8 m_ret_pc;
	u8 m_a;
	u16 m_b; // u9
};


DECLARE_DEVICE_TYPE(TSCONF_COPPER, tsconf_copper_device)

#endif // MAME_SINCLAIR_TSCONF_COPPER_H
