// license:BSD-3-Clause
// copyright-holders:XingXing, Vas Crabb

#include "emu.h"
#include "igs027a.h"

#include "cpu/arm7/arm7core.h"


namespace {

constexpr u32 IN_PORT_MASK = 0x000f'ffff; // 20-bit input port
constexpr u8 OUT_PORT_MASK = 0x1f; // 5-bit output port

} // anonymous namespace


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
	m_in_port_cb(*this, IN_PORT_MASK),
	m_out_port_cb(*this),
	m_irq_timers{ nullptr, nullptr },
	m_irq_enable(0xff),
	m_irq_pending(0xff)
{
}

igs027a_cpu_device::~igs027a_cpu_device()
{
}


void igs027a_cpu_device::device_resolve_objects()
{
	m_ext_fiq = 0;
	m_ext_irq = 0;
}

void igs027a_cpu_device::device_start()
{
	arm7_cpu_device::device_start();

	m_irq_timers[0] = timer_alloc(FUNC(igs027a_cpu_device::timer_irq<0>), this);
	m_irq_timers[1] = timer_alloc(FUNC(igs027a_cpu_device::timer_irq<1>), this);

	save_item(NAME(m_ext_fiq));
	save_item(NAME(m_ext_irq));
	save_item(NAME(m_fiq_enable));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_pending));
}

void igs027a_cpu_device::device_reset()
{
	arm7_cpu_device::device_reset();

	if (!BIT(m_fiq_enable, 0))
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(igs027a_cpu_device::check_fiq), this));
	m_fiq_enable = 0x01; // superkds and fearless never write to 0x4000'0014 but expect FIQ to be triggered
	m_irq_enable = 0xff;
	m_irq_pending = 0xff;

	set_irq(0);
}


void igs027a_cpu_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case ARM7_IRQ_LINE:
		if (state && !m_ext_irq && !BIT(m_irq_enable, 3))
		{
			m_irq_pending &= ~(u8(1) << 3);
			set_irq(1);
		}
		m_ext_irq = state ? 1 : 0;
		break;

	case ARM7_FIRQ_LINE:
		if (state && !m_ext_fiq && BIT(m_fiq_enable, 0))
		{
			set_fiq(1);
			set_fiq(0); // hacky - depends on ARM7 core checking interrupts outside execute loop; how is this actually acknowledged?
		}
		m_ext_fiq = state ? 1 : 0;
		break;

	default:
		arm7_cpu_device::execute_set_input(irqline, state);
	}
}


void igs027a_cpu_device::onboard_peripherals(address_map &map)
{
	map(0x0000'0000, 0x0000'3fff).rom().region(DEVICE_SELF, 0);

	map(0x1000'0000, 0x1000'03ff).ram(); // onboard RAM?

	map(0x4000'000c, 0x4000'000f).r(FUNC(igs027a_cpu_device::in_port_r));
	map(0x4000'0014, 0x4000'0017).umask32(0x0000'00ff).w(FUNC(igs027a_cpu_device::fiq_enable_w)); // sets bit 0 before waiting on FIRQ, maybe it's an enable here?
	map(0x4000'0018, 0x4000'001b).umask32(0x0000'00ff).w(FUNC(igs027a_cpu_device::out_port_w));

	map(0x7000'0100, 0x7000'0107).umask32(0x0000'00ff).w(FUNC(igs027a_cpu_device::timer_rate_w));
	map(0x7000'0200, 0x7000'0203).umask32(0x0000'00ff).rw(FUNC(igs027a_cpu_device::irq_pending_r), FUNC(igs027a_cpu_device::irq_enable_w));

	map(0xf000'0008, 0xf000'000b).umask32(0x0000'00ff).w(FUNC(igs027a_cpu_device::bus_sizing_w));
}


u32 igs027a_cpu_device::in_port_r()
{
	return
			0xff80'0000 | // unused bits?
			((m_in_port_cb(0, IN_PORT_MASK) & IN_PORT_MASK) << 3) | // 20-bit input port
			0x0000'0007; // TODO: FIRQ, IRQ and UART Rx pins
}

void igs027a_cpu_device::out_port_w(u8 data)
{
	// 5-bit output port
	m_out_port_cb(0, data & OUT_PORT_MASK, OUT_PORT_MASK);
}

void igs027a_cpu_device::timer_rate_w(offs_t offset, u8 data)
{
	// TODO: determine how timer intervals are derived from clocks
	if (data)
	{
		constexpr u32 TIMER_DIVISOR = 4263;
		auto const period = attotime::from_ticks(TIMER_DIVISOR * (data + 1), clock());
		m_irq_timers[offset]->adjust(period, 0, period);
	}
	else
	{
		m_irq_timers[offset]->adjust(attotime::never, 0, attotime::never);
	}
}

u8 igs027a_cpu_device::irq_pending_r()
{
	u8 const result = m_irq_pending;
	if (!machine().side_effects_disabled())
	{
		m_irq_pending = 0xff;
		set_irq(0);
	}
	return result;
}

void igs027a_cpu_device::fiq_enable_w(u8 data)
{
	if (BIT(~m_fiq_enable & data, 0))
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(igs027a_cpu_device::check_fiq), this));
	else if (BIT(m_fiq_enable & ~data, 0))
		set_fiq(0);
	m_fiq_enable = data;
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
	if (!BIT(m_irq_enable, N))
	{
		m_irq_pending &= ~(u8(1) << N);
		set_irq(1);
	}
}

TIMER_CALLBACK_MEMBER(igs027a_cpu_device::check_fiq)
{
	if (m_ext_fiq && BIT(m_fiq_enable, 0))
	{
		set_fiq(1);
		set_fiq(0); // hacky - depends on ARM7 core checking interrupts outside execute loop; how is this actually acknowledged?
	}
}
