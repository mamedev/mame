// license:BSD-3-Clause
// copyright-holders:XingXing, Vas Crabb

#include "emu.h"
#include "igs027a.h"

#include "cpu/arm7/arm7core.h"


DEFINE_DEVICE_TYPE(IGS027A, igs027a_cpu_device, "igs027a", "IGS 027A ARM CPU (little)")


igs027a_cpu_device::igs027a_cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	arm7_cpu_device(
			mconfig,
			IGS027A,
			tag,
			owner,
			clock,
			4,
			ARCHFLAG_T,
			ENDIANNESS_LITTLE,
			address_map_constructor(FUNC(igs027a_cpu_device::onboard_peripherals), this)),
	m_irq_timers{ nullptr, nullptr },
	m_irq_enable(0xff),
	m_irq_pending(0xff)
{
}

igs027a_cpu_device::~igs027a_cpu_device()
{
}


void igs027a_cpu_device::trigger_irq(unsigned num)
{
	if (!BIT(m_irq_enable, num))
	{
		m_irq_pending &= ~(u8(1) << num);
		pulse_input_line(ARM7_IRQ_LINE, minimum_quantum_time());
	}
}


void igs027a_cpu_device::device_start()
{
	arm7_cpu_device::device_start();

	m_irq_timers[0] = timer_alloc(FUNC(igs027a_cpu_device::timer_irq<0>), this);
	m_irq_timers[1] = timer_alloc(FUNC(igs027a_cpu_device::timer_irq<1>), this);

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_pending));
}

void igs027a_cpu_device::device_reset()
{
	arm7_cpu_device::device_reset();

	m_irq_enable = 0xff;
	m_irq_pending = 0xff;
}


void igs027a_cpu_device::onboard_peripherals(address_map &map)
{
	map(0x0000'0000, 0x0000'3fff).rom().region(DEVICE_SELF, 0);

	map(0x7000'0100, 0x7000'0103).umask32(0x0000'00ff).w(FUNC(igs027a_cpu_device::timer_rate_w<0>));
	map(0x7000'0104, 0x7000'0107).umask32(0x0000'00ff).w(FUNC(igs027a_cpu_device::timer_rate_w<1>));
	map(0x7000'0200, 0x7000'0203).umask32(0x0000'00ff).rw(FUNC(igs027a_cpu_device::irq_pending_r), FUNC(igs027a_cpu_device::irq_enable_w));

	map(0xf000'0008, 0xf000'000b).umask32(0x0000'00ff).w(FUNC(igs027a_cpu_device::bus_sizing_w));
}


template <unsigned N>
void igs027a_cpu_device::timer_rate_w(u8 data)
{
	// TODO: determine how timer intervals are derived from clocks
	m_irq_timers[N]->adjust(attotime::from_hz(data / 2), 0, attotime::from_hz(data / 2));
}

u8 igs027a_cpu_device::irq_pending_r()
{
	u8 const result = m_irq_pending;
	if (!machine().side_effects_disabled())
		m_irq_pending = 0xff;
	return result;
}

void igs027a_cpu_device::irq_enable_w(u8 data)
{
	m_irq_enable = data;
}


void igs027a_cpu_device::bus_sizing_w(u8 data)
{
	logerror("Bus sizing configuration: 0x%02x\n", data);
}


template <unsigned N>
TIMER_CALLBACK_MEMBER(igs027a_cpu_device::timer_irq)
{
	trigger_irq(N);
}
