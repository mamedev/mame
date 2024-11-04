// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// HLE emulation of the MKS3 keyboard scanning PCB used in the psr340
// and psr540 among others.

// Uses a 63B05 with a not-yet-dumped internal rom


#ifndef MAME_YAMAHA_MKS3_H
#define MAME_YAMAHA_MKS3_H

#pragma once

DECLARE_DEVICE_TYPE(MKS3, mks3_device)

class mks3_device : public device_t
{
public:
	mks3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void ic_w(int state);
	void req_w(int state);
	auto write_da() { return m_write_da.bind(); }
	auto write_clk() { return m_write_clk.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<4> m_port;
	devcb_write_line m_write_da;
	devcb_write_line m_write_clk;
	emu_timer *m_scan_timer, *m_transmit_timer;

	std::array<u8, 3> m_bytes;

	std::array<u32, 4> m_sent_state;
	std::array<u32, 4> m_current_state;

	int m_ic, m_req;
	u8 m_step, m_byte, m_byte_count;

	u32 find_next();
	void send_next();
	void transmit_next();
	TIMER_CALLBACK_MEMBER(transmit_tick);
	TIMER_CALLBACK_MEMBER(scan_tick);
};

#endif
