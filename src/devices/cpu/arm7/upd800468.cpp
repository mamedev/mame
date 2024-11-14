// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

 NEC uPD800468
 ARM7TDMI core with internal peripherals and external ROM/flash

 Used in late 2000s/early 2010s Casio keyboards, like the CTK-2000 series.

***************************************************************************/

#include "emu.h"
#include "upd800468.h"

#include "arm7core.h"


DEFINE_DEVICE_TYPE(UPD800468_TIMER, upd800468_timer_device, "upd800468_timer", "NEC uPD800468 timer")

upd800468_timer_device::upd800468_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UPD800468_TIMER, tag, owner, clock)
	, m_irq_cb(*this)
{
}

u32 upd800468_timer_device::rate_r()
{
	return m_rate;
}

void upd800468_timer_device::rate_w(u32 data)
{
	m_rate = data;
}

u8 upd800468_timer_device::control_r()
{
	return m_control;
}

void upd800468_timer_device::control_w(u8 data)
{
	if (BIT(data, 1) != BIT(m_control, 1))
	{
		if (BIT(data, 1))
		{
			attotime period = clocks_to_attotime(m_rate);
			m_timer->adjust(period, 0, period);
		}
		else
		{
			m_timer->adjust(attotime::never);
			m_irq_cb(0);
		}
	}

	if (!BIT(data, 0))
	{
		m_irq_cb(0);
	}

	m_control = data;
}

void upd800468_timer_device::device_start()
{
	m_timer = timer_alloc(FUNC(upd800468_timer_device::irq_timer_tick), this);

	save_item(NAME(m_rate));
	save_item(NAME(m_control));
}

void upd800468_timer_device::device_reset()
{
	m_rate = m_control = 0;
}

TIMER_CALLBACK_MEMBER(upd800468_timer_device::irq_timer_tick)
{
	if (BIT(m_control, 0))
		m_irq_cb(1);
}

DEFINE_DEVICE_TYPE(UPD800468, upd800468_device, "upd800468", "NEC uPD800468")

void upd800468_device::upd800468_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).view(m_ram_view);
	m_ram_view[0](0x00000000, 0x0000ffff).ram();

	// RAM used for storing user samples (TODO: is this actually supposed to be part of the main 64kb RAM?)
	map(0x1ffe8400, 0x1ffedfff).ram();

	map(0x1fff00a0, 0x1fff00a1).r(m_kbd, FUNC(gt913_kbd_hle_device::read)).umask32(0x0000ffff);
	map(0x1fff00a2, 0x1fff00a3).r(m_kbd, FUNC(gt913_kbd_hle_device::status_r)).umask32(0xffff0000);
	map(0x1fff00a4, 0x1fff00a5).w(m_kbd, FUNC(gt913_kbd_hle_device::status_w)).umask32(0x0000ffff);

	map(0x1fff00c0, 0x1fff00cf).r(FUNC(upd800468_device::adc_r));

	map(0x1fff0141, 0x1fff0141).rw(FUNC(upd800468_device::port_ddr_r<0>), FUNC(upd800468_device::port_ddr_w<0>)).umask32(0x0000ff00);
	map(0x1fff0142, 0x1fff0142).rw(FUNC(upd800468_device::port_r<0>), FUNC(upd800468_device::port_w<0>)).umask32(0x00ff0000);
	map(0x1fff0145, 0x1fff0145).rw(FUNC(upd800468_device::port_ddr_r<1>), FUNC(upd800468_device::port_ddr_w<1>)).umask32(0x0000ff00);
	map(0x1fff0146, 0x1fff0146).rw(FUNC(upd800468_device::port_r<1>), FUNC(upd800468_device::port_w<1>)).umask32(0x00ff0000);
	map(0x1fff0149, 0x1fff0149).rw(FUNC(upd800468_device::port_ddr_r<2>), FUNC(upd800468_device::port_ddr_w<2>)).umask32(0x0000ff00);
	map(0x1fff014a, 0x1fff014a).rw(FUNC(upd800468_device::port_r<2>), FUNC(upd800468_device::port_w<2>)).umask32(0x00ff0000);
	map(0x1fff014d, 0x1fff014d).rw(FUNC(upd800468_device::port_ddr_r<3>), FUNC(upd800468_device::port_ddr_w<3>)).umask32(0x0000ff00);
	map(0x1fff014e, 0x1fff014e).rw(FUNC(upd800468_device::port_r<3>), FUNC(upd800468_device::port_w<3>)).umask32(0x00ff0000);

	map(0x2a003504, 0x2a003507).rw(m_timer[0], FUNC(upd800468_timer_device::rate_r), FUNC(upd800468_timer_device::rate_w));
	map(0x2a003508, 0x2a003508).rw(m_timer[0], FUNC(upd800468_timer_device::control_r), FUNC(upd800468_timer_device::control_w)).umask32(0x000000ff);
	map(0x2a003514, 0x2a003517).rw(m_timer[1], FUNC(upd800468_timer_device::rate_r), FUNC(upd800468_timer_device::rate_w));
	map(0x2a003518, 0x2a003518).rw(m_timer[1], FUNC(upd800468_timer_device::control_r), FUNC(upd800468_timer_device::control_w)).umask32(0x000000ff);
	map(0x2a003524, 0x2a003527).rw(m_timer[2], FUNC(upd800468_timer_device::rate_r), FUNC(upd800468_timer_device::rate_w));
	map(0x2a003528, 0x2a003528).rw(m_timer[2], FUNC(upd800468_timer_device::control_r), FUNC(upd800468_timer_device::control_w)).umask32(0x000000ff);

	map(0x2a003e00, 0x2a003e03).rw(FUNC(upd800468_device::ram_enable_r), FUNC(upd800468_device::ram_enable_w));

	// keep whatever this is from spamming the error log for now
	map(0x50000070, 0x50000077).noprw();

	// TODO: USB controller at 0x50000400

	map(0xfffff000, 0xffffffff).m(m_vic, FUNC(vic_upd800468_device::map));
}

upd800468_device::upd800468_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, UPD800468, tag, owner, clock, 4, ARCHFLAG_T, ENDIANNESS_LITTLE)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor(FUNC(upd800468_device::upd800468_map), this))
	, m_vic(*this, "vic")
	, m_timer(*this, "timer%u", 0)
	, m_kbd(*this, "kbd")
	, m_adc_cb(*this, 0x00), m_in_cb(*this, 0x00), m_out_cb(*this)
	, m_ram_view(*this, "ramview")
{
}

device_memory_interface::space_config_vector upd800468_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void upd800468_device::device_add_mconfig(machine_config &config)
{
	UPD800468_VIC(config, m_vic, 0);
	m_vic->out_irq_cb().set_inputline(*this, ARM7_IRQ_LINE);
	m_vic->out_fiq_cb().set_inputline(*this, ARM7_FIRQ_LINE);

	// this is probably not 100% accurate timing-wise
	// for the ctk2100 it works ok for e.g. MIDI tempo and the auto power off interval
	// it's unclear if there's supposed to be a register for setting a divider for each timer
	UPD800468_TIMER(config, m_timer[0], clock() >> 3);
	UPD800468_TIMER(config, m_timer[1], clock() >> 3);
	UPD800468_TIMER(config, m_timer[2], clock() >> 3);
	m_timer[0]->irq_cb().set(m_vic, FUNC(vic_upd800468_device::irq_w<21>));
	m_timer[1]->irq_cb().set(m_vic, FUNC(vic_upd800468_device::irq_w<22>));
	m_timer[2]->irq_cb().set(m_vic, FUNC(vic_upd800468_device::irq_w<23>));

	// key/button controller is compatible with the one from earlier keyboards
	GT913_KBD_HLE(config, m_kbd, 0);
	m_kbd->irq_cb().set(m_vic, FUNC(vic_upd800468_device::irq_w<31>));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd800468_device::device_start()
{
	arm7_cpu_device::device_start();

	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
	save_item(NAME(m_ram_enable));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd800468_device::device_reset()
{
	arm7_cpu_device::device_reset();

	for (offs_t i = 0; i < 4; i++)
	{
		m_port_data[i] = 0;
		m_port_ddr[i] = 0xff;
		port_update(i);
	}

	m_ram_enable = 0;
	m_ram_view.disable();
}

u16 upd800468_device::adc_r(offs_t num)
{
	// TODO: verify strange-seeming ADC behavior
	// ctk2100 reads a 10-bit value, then inverts the highest bit, and seemingly treats the result as unsigned
	return m_adc_cb[num]() ^ 0x200;
}

u8 upd800468_device::port_ddr_r(offs_t num)
{
	return m_port_ddr[num];
}

void upd800468_device::port_ddr_w(offs_t num, u8 data)
{
	m_port_ddr[num] = data;
//  logerror("port %u ddr_w: %02x\n", num, data);
	port_update(num);
}

u8 upd800468_device::port_r(offs_t num)
{
	// port input seemingly does not use the ddr value
	// (ctk2100 port 2 ddr lower 4 bits are always high, but the corresponding data bits
	//  are apparently expected to be bidirectional, otherwise reading LCD status fails)
	return m_in_cb[num]();
}

void upd800468_device::port_w(offs_t num, u8 data)
{
	m_port_data[num] = data;
	port_update(num);
}

void upd800468_device::port_update(offs_t num)
{
//  logerror("port %u out: %02x\n", num, m_port_data[num] & m_port_ddr[num]);
	m_out_cb[num](m_port_data[num] & m_port_ddr[num]);
}

u32 upd800468_device::ram_enable_r()
{
	return m_ram_enable;
}

void upd800468_device::ram_enable_w(u32 data)
{
	m_ram_enable = data;

	if (BIT(m_ram_enable, 0))
		m_ram_view.select(0);
	else
		m_ram_view.disable();
}
