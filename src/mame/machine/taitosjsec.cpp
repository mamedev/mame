// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "taitosjsec.h"

DEFINE_DEVICE_TYPE(TAITO_SJ_SECURITY_MCU,  taito_sj_security_mcu_device, "taitosjsecmcu", "Taito SJ Security MCU Interface")


taito_sj_security_mcu_device::taito_sj_security_mcu_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig,TAITO_SJ_SECURITY_MCU, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_int_mode(int_mode::NONE)
	, m_68read_cb(*this)
	, m_68write_cb(*this)
	, m_68intrq_cb(*this)
	, m_busrq_cb(*this)
	, m_addr(0U)
	, m_mcu_data(0U)
	, m_host_data(0U)
	, m_read_data(0U)
	, m_zaccept(false)
	, m_zready(false)
	, m_pb_val(0U)
	, m_busak(false)
	, m_reset(false)
{
}

READ8_MEMBER(taito_sj_security_mcu_device::data_r)
{
	if (BIT(offset, 0))
	{
		// ZLSTATUS
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(10));
		return
				(u8(space.unmap()) & 0xfc) |
				u8(m_zaccept ? 0x00 : 0x02) |
				u8(m_zready ? 0x00 : 0x01);
	}
	else
	{
		// ZLREAD
		if (!machine().side_effects_disabled())
			m_zaccept = true;
		return m_mcu_data;
	}
}

WRITE8_MEMBER(taito_sj_security_mcu_device::data_w)
{
	if (BIT(offset, 0))
	{
		// ZINTRQ
		// if jumpered this way, the Z80 write strobe pulses the MCU interrupt line
		// should be PULSE_LINE because it's edge sensitive, but diexec only allows PULSE_LINE on reset and NMI
		if (int_mode::WRITE == m_int_mode)
			m_mcu->set_input_line(M68705_IRQ_LINE, HOLD_LINE);
	}
	else
	{
		// ZLWRITE
		device_scheduler &sched(machine().scheduler());
		sched.synchronize(timer_expired_delegate(FUNC(taito_sj_security_mcu_device::do_host_write), this), data);
		sched.boost_interleave(attotime::zero, attotime::from_usec(10));
	}
}

WRITE_LINE_MEMBER(taito_sj_security_mcu_device::busak_w)
{
	m_busak = (ASSERT_LINE == state);
}

WRITE_LINE_MEMBER(taito_sj_security_mcu_device::reset_w)
{
	m_reset = (ASSERT_LINE == state);
	if (CLEAR_LINE != state)
	{
		m_zaccept = true;
		m_zready = false;
		if (int_mode::LATCH == m_int_mode)
			m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	}
	m_mcu->set_input_line(INPUT_LINE_RESET, state);
}

void taito_sj_security_mcu_device::device_start()
{
	m_68read_cb.resolve_safe(0xff);
	m_68write_cb.resolve_safe();
	m_68intrq_cb.resolve_safe();
	m_busrq_cb.resolve_safe();

	save_item(NAME(m_addr));
	save_item(NAME(m_mcu_data));
	save_item(NAME(m_host_data));
	save_item(NAME(m_read_data));
	save_item(NAME(m_zaccept));
	save_item(NAME(m_zready));
	save_item(NAME(m_pa_val));
	save_item(NAME(m_pb_val));
	save_item(NAME(m_busak));
	save_item(NAME(m_reset));

	m_addr = 0xffffU;
	m_mcu_data = 0xffU;
	m_host_data = 0xffU;
	m_read_data = 0xffU;
	m_pb_val = 0xffU;
	m_busak = false;
	m_reset = false;
}

void taito_sj_security_mcu_device::device_reset()
{
	m_zaccept = true;
	m_zready = false;
	if (int_mode::LATCH == m_int_mode)
		m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
}

void taito_sj_security_mcu_device::device_add_mconfig(machine_config &config)
{
	M68705P5(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->porta_r().set(FUNC(taito_sj_security_mcu_device::mcu_pa_r));
	m_mcu->portc_r().set(FUNC(taito_sj_security_mcu_device::mcu_pc_r));
	m_mcu->porta_w().set(FUNC(taito_sj_security_mcu_device::mcu_pa_w));
	m_mcu->portb_w().set(FUNC(taito_sj_security_mcu_device::mcu_pb_w));
}

READ8_MEMBER(taito_sj_security_mcu_device::mcu_pa_r)
{
	return get_bus_val();
}

READ8_MEMBER(taito_sj_security_mcu_device::mcu_pc_r)
{
	// FIXME 68INTAK is on PC3 but we're ignoring it
	return
			(m_zready ? 0x01U : 0x00U) |
			(m_zaccept ? 0x02U : 0x00U) |
			(m_busak ? 0x00U : 0x04U);
}

WRITE8_MEMBER(taito_sj_security_mcu_device::mcu_pa_w)
{
	m_pa_val = data;
	if (BIT(~m_pb_val, 6))
		m_addr = (m_addr & 0xff00U) | u16(get_bus_val());
}

WRITE8_MEMBER(taito_sj_security_mcu_device::mcu_pb_w)
{
	bool inc_addr(false);
	u8 const diff(m_pb_val ^ data);

	// 68INTRQ
	if (BIT(diff, 0))
		m_68intrq_cb(BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);

	// 68LRD
	u8 const bus_val(get_bus_val());
	if (BIT(diff & data, 1))
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(taito_sj_security_mcu_device::do_mcu_read), this));
		if (int_mode::LATCH == m_int_mode)
			m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	}

	// 68LWR
	if (BIT(diff & data, 2))
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(taito_sj_security_mcu_device::do_mcu_write), this), bus_val);

	// BUSRQ
	if (BIT(diff, 3))
		m_busrq_cb(BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);

	// 68WRITE
	if (BIT(diff, 4))
	{
		if (BIT(~data, 4))
			m_68write_cb(space, m_addr, bus_val);
		else if (BIT(data, 5))
			inc_addr = true;
	}

	// 68READ
	if (BIT(diff, 5))
	{
		if (BIT(~data, 5))
			m_read_data = m_68read_cb(space, m_addr);
		else if (BIT(data, 4))
			inc_addr = true;
	}

	// LAL
	if (BIT(~data, 6))
		m_addr = (m_addr & 0xff00U) | u16(bus_val);
	else if (inc_addr)
		m_addr = (m_addr & 0xff00U) | ((m_addr + 1) & 0x00ffU);

	// UAL
	if (BIT(~data, 7))
		m_addr = (m_addr & 0x00ffU) | (u16(bus_val) << 8);

	m_pb_val = data;
}

TIMER_CALLBACK_MEMBER(taito_sj_security_mcu_device::do_mcu_read)
{
	m_zready = false;
}

TIMER_CALLBACK_MEMBER(taito_sj_security_mcu_device::do_mcu_write)
{
	m_mcu_data = u8(param);
	if (!m_reset)
		m_zaccept = false;
}

TIMER_CALLBACK_MEMBER(taito_sj_security_mcu_device::do_host_write)
{
	m_host_data = u8(param);
	if (!m_reset)
	{
		m_zready = true;
		if (int_mode::LATCH == m_int_mode)
			m_mcu->set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
	}
}
