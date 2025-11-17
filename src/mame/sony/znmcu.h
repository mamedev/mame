// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_SONY_ZNMCU_H
#define MAME_SONY_ZNMCU_H

#pragma once

class znmcu_device : public device_t
{
public:
	znmcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<unsigned N> auto analog() { return m_analog_cb[N].bind(); }
	template<unsigned N> auto trackball() { return m_trackball_cb[N].bind(); }
	auto dsw() { return m_dsw_cb.bind(); }
	auto dsr() { return m_dsr_cb.bind(); }
	auto txd() { return m_txd_cb.bind(); }

	void select(int state);
	void sck(int state);
	void analog_read(int state);
	void trackball_read(int state);

protected:
	// device_t
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(mcu_tick);

private:
	devcb_read8::array<8> m_analog_cb;
	devcb_read16::array<4> m_trackball_cb;
	devcb_read8 m_dsw_cb;
	devcb_write_line m_dsr_cb;
	devcb_write_line m_txd_cb;

	emu_timer *m_mcu_timer;
	int m_sck;
	int m_select;
	int m_bit;
	int m_byte;
	int m_databytes;
	int m_analog_read;
	int m_trackball_read;
	std::array<uint16_t, 4> m_trackball;
	std::array<uint8_t, 9> m_send;
};

DECLARE_DEVICE_TYPE(ZNMCU, znmcu_device)

#endif // MAME_SONY_ZNMCU_H
