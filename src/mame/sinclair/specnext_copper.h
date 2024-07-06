// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_COPPER_H
#define MAME_SINCLAIR_SPECNEXT_COPPER_H

#pragma once

#include <utility>


class specnext_copper_device : public device_t
{

public:
	specnext_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_nextreg_cb() { return m_out_nextreg_cb.bind(); }
	template <typename... T> void set_in_until_pos_cb(T &&... args) { return m_in_until_pos_cb.set(std::forward<T>(args)...); }

	u8 data_r(u16 addr) { return m_listram[addr]; }
	void data_w(u16 addr, u8 data);

	void copper_en_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_callback);
	TIMER_CALLBACK_MEMBER(frame_timer_callback);

	emu_timer *m_timer;
	emu_timer *m_frame_timer;

private:
	memory_share_creator<u8> m_listram;

	devcb_write8 m_out_nextreg_cb;
	device_delegate<attotime (u16)> m_in_until_pos_cb;

	u8 m_copper_en; // u2
	u16 m_copper_list_addr; // u10
	u16 m_copper_list_data; // u16
	bool m_copper_dout;
};


DECLARE_DEVICE_TYPE(SPECNEXT_COPPER, specnext_copper_device)

#endif // MAME_SINCLAIR_SPECNEXT_COPPER_H
