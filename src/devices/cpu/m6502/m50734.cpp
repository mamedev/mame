// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Mitsubishi M50734 emulation (preliminary)

    This microcontroller contains no internal memory, RAM or ROM. The
    external bus allows P05 to be programmed as a strobe (DME) for a
    separable "data" memory space. On-chip peripherals include an ADC,
    UART, various general and special-purpose timers and a stepper
    motor controller.

**********************************************************************/

#include "emu.h"
#include "m50734.h"

#define LOG_INIT  (1 << 1U)
#define LOG_TIMER (1 << 2U)
#define VERBOSE   (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(M50734, m50734_device, "m50734", "Mitsubishi M50734")

m50734_device::m50734_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m740_device(mconfig, M50734, tag, owner, clock)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_port_in_cb(*this)
	, m_port_out_cb(*this)
	, m_analog_in_cb(*this)
	, m_port_latch{0, 0, 0, 0}
	, m_port_3state{0, 0, 0, 0}
	, m_ad_control(0)
	, m_ad_register(0)
	, m_prescaler_reload{0xff, 0xff, 0xff}
	, m_timer_reload{0xff, 0xff, 0xff}
{
	program_config.m_internal_map = address_map_constructor(FUNC(m50734_device::internal_map), this);
}

device_memory_interface::space_config_vector m50734_device::memory_space_config() const
{
	space_config_vector scv = m740_device::memory_space_config();
	if (has_configured_map(AS_DATA))
		scv.emplace_back(AS_DATA, &m_data_config);
	return scv;
}

void m50734_device::device_resolve_objects()
{
	for (int n = 0; n < 4; n++)
		m_port_in_cb[n].resolve_safe(m_port_3state[n]);
	m_port_in_cb[4].resolve_safe(0);
	m_port_out_cb.resolve_all_safe();
	m_analog_in_cb.resolve_all_safe(0);
}

template <int N>
TIMER_CALLBACK_MEMBER(m50734_device::timer_interrupt)
{
	if (!BIT(m_interrupt_control[1], N * 2 + 1))
	{
		m_interrupt_control[1] |= 1 << (N * 2 + 1);
		if (BIT(m_interrupt_control[1], N * 2))
		{
			LOGMASKED(LOG_TIMER, "Timer %d interrupt asserted at %s\n", N + 1, machine().time().to_string());
			set_input_line(M740_INT2_LINE, ASSERT_LINE);
		}
	}

	// Reload timer and prescaler
	m_timer[N]->adjust(clocks_to_attotime(16 * u32(m_prescaler_reload[N] + 1) * (m_timer_reload[N] + 1)));
}

void m50734_device::device_start()
{
	m740_device::device_start();

	space(has_space(AS_DATA) ? AS_DATA : AS_PROGRAM).specific(m_data);

	m_ad_timer = timer_alloc(FUNC(m50734_device::ad_complete), this);
	m_timer[0] = timer_alloc(FUNC(m50734_device::timer_interrupt<0>), this);
	m_timer[1] = timer_alloc(FUNC(m50734_device::timer_interrupt<1>), this);
	m_timer[2] = timer_alloc(FUNC(m50734_device::timer_interrupt<2>), this);

	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_direction));
	save_item(NAME(m_p0_function));
	save_item(NAME(m_p2_p3_function));
	save_item(NAME(m_ad_control));
	save_item(NAME(m_ad_register));
	save_item(NAME(m_prescaler_reload));
	save_item(NAME(m_timer_reload));
	save_item(NAME(m_interrupt_control));
}

void m50734_device::device_reset()
{
	m740_device::device_reset();
	SP = 0x01ff;

	std::fill(std::begin(m_port_direction), std::end(m_port_direction), 0x00);
	for (int n = 0; n < 4; n++)
		m_port_out_cb[n](m_port_3state[n]);
	m_p0_function = 0x00;
	m_p2_p3_function = 0x00;
	m_ad_control |= 0x04;
	m_ad_timer->adjust(attotime::never);

	// Reset interrupts
	std::fill(std::begin(m_interrupt_control), std::end(m_interrupt_control), 0x00);
	set_input_line(M740_INT2_LINE, CLEAR_LINE);
}

void m50734_device::read_dummy(u16 adr)
{
	// M50734 outputs RD and WR strobes rather than R/W, so dummy accesses should do nothing
}

u8 m50734_device::read_data(u16 adr)
{
	if (BIT(m_p0_function, 5))
		return m_data.read_byte(adr);
	else
		return m740_device::read(adr);
}

void m50734_device::write_data(u16 adr, u8 val)
{
	if (BIT(m_p0_function, 5))
		m_data.write_byte(adr, val);
	else
		m740_device::write(adr, val);
}

u8 m50734_device::interrupt_control_r(offs_t offset)
{
	return m_interrupt_control[2 - offset];
}

void m50734_device::interrupt_control_w(offs_t offset, u8 data)
{
	if (offset == 1)
		data &= 0x3f;
	u8 old_control = std::exchange(m_interrupt_control[2 - offset], data);
	if (offset == 1)
	{
		bool timer_interrupt = (data & (data >> 1) & 0x15) != 0;
		bool old_interrupt = (old_control & (old_control >> 1) & 0x15) != 0;
		if (timer_interrupt != old_interrupt)
		{
			LOGMASKED(LOG_TIMER, "%s: Timer interrupt %sactivated by write to interrupt control register 2 ($%02X -> $%02X)\n", machine().describe_context(), timer_interrupt ? "": "de", old_control, data);
			set_input_line(M740_INT2_LINE, timer_interrupt ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

template <int N>
u8 m50734_device::port_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_port_direction[N];
	else if (m_port_direction[0] == 0xff)
		return m_port_latch[N];
	else
		return (m_port_in_cb[N]() & ~m_port_direction[N]) | (m_port_latch[N] & m_port_direction[N]);
}

template <int N>
void m50734_device::port_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
	{
		if (m_port_direction[N] != data)
		{
			LOGMASKED(LOG_INIT, "%s: Port P%d direction = $%02X\n", machine().describe_context(), N, data);
			m_port_direction[N] = data;
			m_port_out_cb[N]((data & m_port_latch[N]) | (m_port_3state[N] & ~m_port_direction[N]));
		}
	}
	else if (((std::exchange(m_port_latch[N], data) ^ data) & m_port_direction[N]) != 0)
		m_port_out_cb[N]((data & m_port_direction[N]) | (m_port_3state[N] & ~m_port_direction[N]));
}

u8 m50734_device::p4_r()
{
	// P4 has only 4 pins and no output drivers
	return m_port_in_cb[4]() & 0x0f;
}

u8 m50734_device::p0_function_r()
{
	return m_p0_function;
}

void m50734_device::p0_function_w(u8 data)
{
	LOGMASKED(LOG_INIT, "%s: Port P0 function = $%02X\n", machine().describe_context(), data);
	m_p0_function = data;
}

u8 m50734_device::p2_p3_function_r()
{
	return m_p2_p3_function;
}

void m50734_device::p2_p3_function_w(u8 data)
{
	LOGMASKED(LOG_INIT, "%s: Port P2/P3 function = $%02X\n", machine().describe_context(), data);
	m_p2_p3_function = data & 0xc7;
}

u8 m50734_device::ad_control_r()
{
	return m_ad_control;
}

void m50734_device::ad_control_w(u8 data)
{
	m_ad_control = data & 0x03;
	m_ad_timer->adjust(cycles_to_attotime(72)); // 36 µsec at 8 MHz
}

TIMER_CALLBACK_MEMBER(m50734_device::ad_complete)
{
	m_ad_register = m_analog_in_cb[m_ad_control & 0x03]();
	m_ad_control |= 0x04;
}

u8 m50734_device::ad_r()
{
	return m_ad_register;
}

u8 m50734_device::timer_r(offs_t offset)
{
	if (!m_timer[offset >> 1]->enabled())
		return 0;

	u32 ticks = attotime_to_clocks(m_timer[offset >> 1]->remaining()) / 16;
	u8 pre = m_prescaler_reload[offset >> 1];
	if (BIT(offset, 0))
		return std::min<u32>(ticks / (pre + 1), 0xff);
	else
		return ticks % (pre + 1);
}

void m50734_device::timer_w(offs_t offset, u8 data)
{
	u32 ticks = m_timer[offset >> 1]->enabled() ? attotime_to_clocks(m_timer[offset >> 1]->remaining()) / 16 : 0x10000;
	u8 pre = m_prescaler_reload[offset >> 1];
	if (BIT(offset, 0))
	{
		LOGMASKED(LOG_INIT, "%s: Reload timer %d latch = %d\n", machine().describe_context(), (offset >> 1) + 1, data);
		m_timer_reload[offset >> 1] = data;
		m_timer[offset >> 1]->adjust(clocks_to_attotime(16 * (data * pre + (ticks % (pre + 1)) + 1)));
	}
	else
	{
		LOGMASKED(LOG_INIT, "%s: Reload prescaler %d latch = %d\n", machine().describe_context(), (offset >> 1) + 1, data);
		m_prescaler_reload[offset >> 1] = data;
		m_timer[offset >> 1]->adjust(clocks_to_attotime(16 * (data + 1) * (std::min<u32>(ticks / (pre + 1), 0xff) + 1)));
	}
}

void m50734_device::internal_map(address_map &map)
{
	// TODO: other timers, UART, etc.
	map(0x00dc, 0x00e1).rw(FUNC(m50734_device::timer_r), FUNC(m50734_device::timer_w));
	map(0x00e9, 0x00e9).rw(FUNC(m50734_device::ad_control_r), FUNC(m50734_device::ad_control_w));
	map(0x00ea, 0x00ea).r(FUNC(m50734_device::ad_r));
	map(0x00eb, 0x00eb).r(FUNC(m50734_device::p4_r));
	map(0x00ed, 0x00ed).rw(FUNC(m50734_device::p2_p3_function_r), FUNC(m50734_device::p2_p3_function_w));
	map(0x00ee, 0x00ef).rw(FUNC(m50734_device::port_r<3>), FUNC(m50734_device::port_w<3>));
	map(0x00f0, 0x00f1).rw(FUNC(m50734_device::port_r<2>), FUNC(m50734_device::port_w<2>));
	map(0x00f3, 0x00f4).rw(FUNC(m50734_device::port_r<1>), FUNC(m50734_device::port_w<1>));
	map(0x00f5, 0x00f5).rw(FUNC(m50734_device::p0_function_r), FUNC(m50734_device::p0_function_w));
	map(0x00f6, 0x00f7).rw(FUNC(m50734_device::port_r<0>), FUNC(m50734_device::port_w<0>));
	map(0x00fd, 0x00ff).rw(FUNC(m50734_device::interrupt_control_r), FUNC(m50734_device::interrupt_control_w));
}
