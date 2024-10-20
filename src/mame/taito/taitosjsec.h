// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_TAITO_TAITSJSEC_H
#define MAME_TAITO_TAITSJSEC_H

#pragma once

#include "cpu/m6805/m68705.h"


DECLARE_DEVICE_TYPE(TAITO_SJ_SECURITY_MCU, taito_sj_security_mcu_device)


class taito_sj_security_mcu_device : public device_t
{
public:
	enum class int_mode
	{
		NONE,
		LATCH,
		WRITE
	};

	void set_int_mode(int_mode mode) { m_int_mode = mode; }
	auto m68read_cb() { return m_68read_cb.bind(); }
	auto m68write_cb() { return m_68write_cb.bind(); }
	auto m68intrq_cb() { return m_68intrq_cb.bind(); }
	auto busrq_cb() { return m_busrq_cb.bind(); }

	taito_sj_security_mcu_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock);

	// uses two consecutive addresses
	u8 data_r(address_space &space, offs_t offset);
	void data_w(offs_t offset, u8 data);

	void busak_w(int state);
	void reset_w(int state);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override ATTR_COLD;

	u8 mcu_pa_r();
	u8 mcu_pc_r();
	void mcu_pa_w(u8 data);
	void mcu_pb_w(u8 data);

private:
	u8 const get_bus_val() const
	{ return (BIT(~m_pb_val, 1) ? m_host_data : 0xffU) & m_pa_val & ((m_busak && BIT(~m_pb_val, 5)) ? m_read_data : 0xffU); }

	TIMER_CALLBACK_MEMBER(do_mcu_read);
	TIMER_CALLBACK_MEMBER(do_mcu_write);
	TIMER_CALLBACK_MEMBER(do_host_write);

	required_device<m68705p_device> m_mcu;

	int_mode m_int_mode;
	devcb_read8 m_68read_cb;
	devcb_write8 m_68write_cb;
	devcb_write_line m_68intrq_cb;
	devcb_write_line m_busrq_cb;

	// IC6/IC10 latch/count the low byte, IC14 buffers low byte, IC3 latches high byte
	u16 m_addr;

	// latched by IC9
	u8 m_mcu_data;

	// latched by IC13
	u8 m_host_data;

	// buffered by IC16
	u8 m_read_data;

	// IC7 pin 6, indicates CPU has accepted data from MCU
	bool m_zaccept;

	// IC7 pin 9, indicates CPU has sent data to MCU
	bool m_zready;

	// previous MCU port outputs for detecting edges
	u8 m_pa_val, m_pb_val;

	// input state
	bool m_busak, m_reset;
};

#endif // MAME_TAITO_TATISJSEC_H
