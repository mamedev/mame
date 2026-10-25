// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    Novatek NT3580F-AM019, 6502 sound controller.

    Provisional peripheral implementation inferred from both Paomadi DATA
    ROMs and the M68000 host routines. No command or music interpreter is
    substituted for the original firmware. External RAM decoding, CPU clock
    division, timer prescalers and peripheral flag-clear semantics need PCB
    verification. Timers currently provide provisional 1 kHz/100 Hz sources.
*/

#include "emu.h"
#include "nt3580.h"

#include <algorithm>
#include <iterator>

#define LOG_HOST (1U << 1)
#define LOG_PORT (1U << 2)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NT3580, nt3580_device, "nt3580", "Novatek NT3580F sound controller")

nt3580_device::nt3580_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NT3580, tag, owner, clock)
	, m_cpu(*this, "cpu")
	, m_rom(*this, DEVICE_SELF)
	, m_sound_r(*this, 0xff)
	, m_sound_w(*this)
	, m_timers{ nullptr, nullptr }
	, m_host_control(0xff)
	, m_sound_irq(false)
	, m_reset(false)
{
}

void nt3580_device::device_add_mconfig(machine_config &config)
{
	// Divider is not established; keep the driver's original 8 MHz core rate.
	M6502(config, m_cpu, DERIVED_CLOCK(1, 1));
	m_cpu->set_addrmap(AS_PROGRAM, &nt3580_device::program_map);
}

void nt3580_device::program_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x00f2, 0x00f2).w(FUNC(nt3580_device::irq_enable_w));
	map(0x00f3, 0x00f3).r(FUNC(nt3580_device::irq_status_r));
	map(0x0200, 0x027f).rw(FUNC(nt3580_device::peripheral_r), FUNC(nt3580_device::peripheral_w));
	// The observed uploads require 4800-6b96 and b800-b9cf in addition to
	// 4200-46ff work RAM. Full external SRAM decoding is not yet established.
	map(0x4000, 0xbfff).ram();
	map(0xc000, 0xc001).rw(FUNC(nt3580_device::sound_r), FUNC(nt3580_device::sound_w));
	map(0xe000, 0xffff).rom().region(m_rom, 0);
}

void nt3580_device::device_start()
{
	m_timers[0] = timer_alloc(FUNC(nt3580_device::timer_tick), this);
	m_timers[1] = timer_alloc(FUNC(nt3580_device::timer_tick), this);
	save_item(NAME(m_ports));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_host_data));
	save_item(NAME(m_host_control));
	save_item(NAME(m_host_written));
	save_item(NAME(m_host_pending));
	save_item(NAME(m_sound_irq));
	save_item(NAME(m_reset));
}

void nt3580_device::reset_registers()
{
	std::fill(std::begin(m_ports), std::end(m_ports), 0);
	m_irq_enable = m_irq_pending = 0;
	m_host_data = 0;
	m_host_written = m_host_pending = false;
	update_irq();
}

void nt3580_device::device_reset()
{
	reset_registers();
	device_clock_changed();
}

void nt3580_device::device_clock_changed()
{
	if (m_timers[0])
	{
		// TODO: derive timer periods/enables from the configuration writes at
		// 0204/0206/0210/0220. Their bit meanings and prescalers are unknown;
		// these fixed divisors provisionally supply 1 kHz/100 Hz at 8 MHz.
		const attotime music = attotime::from_ticks(8'000, clock());
		const attotime effects = attotime::from_ticks(80'000, clock());
		m_timers[0]->adjust(music, 0x80, music);
		m_timers[1]->adjust(effects, 0x20, effects);
	}
}

void nt3580_device::update_irq()
{
	const u8 pending = m_irq_pending | (m_sound_irq ? 1 : 0);
	m_cpu->set_input_line(M6502_IRQ_LINE, !m_reset && (pending & m_irq_enable) ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(nt3580_device::timer_tick)
{
	if (!m_reset && (m_irq_enable & param))
	{
		m_irq_pending |= param;
		update_irq();
	}
}

void nt3580_device::irq_enable_w(u8 data)
{
	m_irq_enable = data;
	update_irq();
}

u8 nt3580_device::irq_status_r()
{
	const u8 data = m_irq_pending | (m_sound_irq ? 1 : 0);
	if (!machine().side_effects_disabled())
	{
		m_irq_pending = 0;
		update_irq();
	}
	return data;
}

void nt3580_device::sound_irq_w(int state)
{
	m_sound_irq = bool(state);
	update_irq();
}

void nt3580_device::host_data_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(nt3580_device::host_data_sync), this), data);
}

TIMER_CALLBACK_MEMBER(nt3580_device::host_data_sync)
{
	m_host_data = u8(param);
	m_host_written = true;
}

void nt3580_device::host_control_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(nt3580_device::host_control_sync), this), data);
}

TIMER_CALLBACK_MEMBER(nt3580_device::host_control_sync)
{
	const u8 data = u8(param);
	// Host writes FA -> FE to reset, FC -> FE around each data byte.
	const bool reset = !BIT(data, 2);
	if (reset != m_reset)
	{
		m_reset = reset;
		if (reset)
			reset_registers();
		m_cpu->set_input_line(INPUT_LINE_RESET, reset ? ASSERT_LINE : CLEAR_LINE);
	}
	if (!m_reset && !BIT(m_host_control, 1) && BIT(data, 1) && m_host_written)
	{
		m_host_written = false;
		m_host_pending = true;
		m_irq_pending |= 2;
		LOGMASKED(LOG_HOST, "%s: host byte %02x\n", machine().describe_context(), m_host_data);
		update_irq();
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
	}
	m_host_control = data;
}

u8 nt3580_device::host_status_r() const
{
	// Firmware toggles port 0248 bit 1 once for every received byte; the host
	// waits for its status bit 4 to change. Bit 2 signals an unfinished packet.
	return 0x80 | (m_ports[0x4c] & 0x0f) | ((m_ports[0x48] & 0x06) << 3) | ((m_ports[0x4c] & 0x20) << 1);
}

u8 nt3580_device::peripheral_r(offs_t offset)
{
	switch (offset)
	{
	case 0x46:
		if (!machine().side_effects_disabled())
			m_host_pending = false;
		return m_host_data;
	case 0x4a:
		return m_host_pending ? 0x08 : 0;
	case 0x4e:
		return m_sound_irq ? 0 : 0x10;
	default:
		return m_ports[offset];
	}
}

void nt3580_device::peripheral_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_PORT, "%s: peripheral %04x = %02x\n", machine().describe_context(), 0x200 + offset, data);
	if (offset == 0x48 || offset == 0x4c)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(nt3580_device::host_output_sync), this), (offset << 8) | data);
	else
		m_ports[offset] = data;
}

TIMER_CALLBACK_MEMBER(nt3580_device::host_output_sync)
{
	m_ports[param >> 8] = u8(param);
}
