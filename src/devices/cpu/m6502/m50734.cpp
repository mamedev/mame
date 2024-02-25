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

#define LOG_INIT    (1U << 1)
#define LOG_TIMER   (1U << 2)
#define LOG_TIMER_X (1U << 3)
#define VERBOSE     (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(M50734, m50734_device, "m50734", "Mitsubishi M50734")

m50734_device::m50734_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m740_device(mconfig, M50734, tag, owner, clock)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_port_in_cb(*this, 0)
	, m_port_out_cb(*this)
	, m_analog_in_cb(*this, 0)
	, m_port_latch{0, 0, 0, 0}
	, m_port_3state{0, 0, 0, 0}
	, m_ad_control(0)
	, m_ad_register(0)
	, m_prescaler_reload{0xff, 0xff, 0xff}
	, m_timer_reload{0xff, 0xff, 0xff}
	, m_step_counter{0, 0}
	, m_phase_counter(0)
	, m_smcon{0, 0}
	, m_tx_count(0)
	, m_tx_reload(0xffff)
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

void m50734_device::device_config_complete()
{
	m740_device::device_config_complete();

	for (int n = 0; n < 4; n++)
	{
		if (m_port_in_cb[n].isunset())
			m_port_in_cb[n].bind().set_constant(m_port_3state[n]);
	}
}

void m50734_device::step_motor(int which)
{
	if (!BIT(m_smcon[which], 2))
		return;

	if (--m_step_counter[which] == 0 && !BIT(m_interrupt_control[0], which * 2 + 5))
	{
		LOGMASKED(LOG_TIMER, "%s counter empty at %s\n", which ? "Vertical" : "Horizontal", machine().time().to_string());
		m_interrupt_control[0] |= 0x20 << (which * 2);
		if (BIT(m_interrupt_control[0], which * 2 + 4))
			set_input_line(M740_INT3_LINE, ASSERT_LINE);
	}

	// Increment or decrement HPHC/VPHC
	m_phase_counter = (m_phase_counter + ((BIT(m_smcon[which], 1) ? 7 : 1) << (which * 4))) & 0x77;

	// TODO: single shot mode, phase decoder and P2 output
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

	if (N == 2)
		step_motor(0); // Timer 3 steps HC
	else if (N == 0)
		step_motor(1); // Timer 0 steps VC

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
	m_timer_x = timer_alloc(FUNC(m50734_device::timer_x_interrupt), this);

	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_direction));
	save_item(NAME(m_p0_function));
	save_item(NAME(m_p2_p3_function));
	save_item(NAME(m_ad_control));
	save_item(NAME(m_ad_register));
	save_item(NAME(m_prescaler_reload));
	save_item(NAME(m_timer_reload));
	save_item(NAME(m_step_counter));
	save_item(NAME(m_phase_counter));
	save_item(NAME(m_smcon));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_tx_reload));
	save_item(NAME(m_interrupt_control));
}

void m50734_device::device_reset()
{
	m740_device::device_reset();
	SP = 0x01ff;

	// Reset port registers
	std::fill(std::begin(m_port_direction), std::end(m_port_direction), 0x00);
	for (int n = 0; n < 4; n++)
		m_port_out_cb[n](m_port_3state[n]);
	m_p0_function = 0x00;
	m_p2_p3_function = 0x00;

	// Reset A-D
	m_ad_control |= 0x04;
	m_ad_timer->adjust(attotime::never);

	// Reset stepper motor control registers (datasheet has no implication of RESET affecting these, but mps1200 never fully initializes SMCONH)
	m_smcon[0] = 0;
	m_smcon[1] = 0;

	// Reset interrupts
	std::fill(std::begin(m_interrupt_control), std::end(m_interrupt_control), 0x00);
	set_input_line(M740_INT2_LINE, CLEAR_LINE);
	set_input_line(M740_INT3_LINE, CLEAR_LINE);
	set_input_line(M740_INT4_LINE, CLEAR_LINE);

	// Initialize Timer X
	set_timer_x(0x0200);
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

// TODO: emulate modes other than timer mode
static const char *const s_timer_x_modes[4] =
{
	"timer",
	"pulse output",
	"event count",
	"PWM"
};

void m50734_device::interrupt_control_w(offs_t offset, u8 data)
{
	if (offset == 1)
		data &= 0x3f;
	u8 old_control = std::exchange(m_interrupt_control[2 - offset], data);
	if (offset == 2)
	{
		bool he_ve_interrupt = (data & (data >> 1) & 0x50) != 0;
		bool old_interrupt = (old_control & (old_control >> 1) & 0x50) != 0;
		if (he_ve_interrupt != old_interrupt)
		{
			LOGMASKED(LOG_TIMER, "%s: HE/VE interrupt %sactivated by write to interrupt control register 1 ($%02X -> $%02X)\n", machine().describe_context(), he_ve_interrupt ? "": "de", old_control, data);
			set_input_line(M740_INT3_LINE, he_ve_interrupt ? ASSERT_LINE : CLEAR_LINE);
		}
	}
	else if (offset == 1)
	{
		bool timer_interrupt = (data & (data >> 1) & 0x15) != 0;
		bool old_interrupt = (old_control & (old_control >> 1) & 0x15) != 0;
		if (timer_interrupt != old_interrupt)
		{
			LOGMASKED(LOG_TIMER, "%s: Timer interrupt %sactivated by write to interrupt control register 2 ($%02X -> $%02X)\n", machine().describe_context(), timer_interrupt ? "": "de", old_control, data);
			set_input_line(M740_INT2_LINE, timer_interrupt ? ASSERT_LINE : CLEAR_LINE);
		}
	}
	else
	{
		bool tx_run = BIT(data, 0, 2) != 2 && !BIT(data, 2);
		if (!tx_run && m_timer_x->enabled())
		{
			m_tx_count = get_timer_x();
			LOGMASKED(LOG_TIMER_X, "%s: Timer X count stopped at $%04X in %s mode by write to interrupt control register 3 ($%02X -> $%02X)\n", machine().describe_context(),
									m_tx_count, s_timer_x_modes[BIT(data, 0, 2)], old_control, data);
			m_timer_x->enable(false);
		}
		else if (tx_run && !m_timer_x->enabled() && (m_tx_count != 0 || m_tx_reload != 0))
		{
			LOGMASKED(LOG_TIMER_X, "%s: Timer X count restarted from $%04X in %s mode by write to interrupt control register 3 ($%02X -> $%02X)\n", machine().describe_context(),
									m_tx_count, s_timer_x_modes[BIT(data, 0, 2)], old_control, data);
			m_timer_x->adjust(clocks_to_attotime(16 * (m_tx_count != 0 ? m_tx_count : m_tx_reload + 1)));
			m_timer_x->enable(true);
		}
		bool tx_interrupt = (data & (data >> 1) & 0x15) != 0;
		bool old_interrupt = (old_control & (old_control >> 1) & 0x15) != 0;
		if (tx_interrupt != old_interrupt)
		{
			LOGMASKED(LOG_TIMER_X, "%s: Timer X interrupt %sactivated by write to interrupt control register 3 ($%02X -> $%02X)\n", machine().describe_context(), tx_interrupt ? "": "de", old_control, data);
			set_input_line(M740_INT4_LINE, tx_interrupt ? ASSERT_LINE : CLEAR_LINE);
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
		attotime expire_time = clocks_to_attotime(16 * (data * pre + (ticks % (pre + 1)) + 1));
		LOGMASKED(LOG_INIT, "%s: Reload timer %d latch = %u (expires in %.1f usec)\n", machine().describe_context(), (offset >> 1) + 1, data, expire_time.as_double() * 1.0E6);
		m_timer_reload[offset >> 1] = data;
		m_timer[offset >> 1]->adjust(expire_time);
	}
	else
	{
		attotime expire_time = clocks_to_attotime(16 * (data + 1) * (std::min<u32>(ticks / (pre + 1), 0xff) + 1));
		LOGMASKED(LOG_INIT, "%s: Reload prescaler %d latch = %u (expires in %.1f usec)\n", machine().describe_context(), (offset >> 1) + 1, data, expire_time.as_double() * 1.0E6);
		m_prescaler_reload[offset >> 1] = data;
		m_timer[offset >> 1]->adjust(expire_time);
	}
}

u8 m50734_device::step_counter_r(offs_t offset)
{
	return m_step_counter[offset];
}

void m50734_device::step_counter_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_INIT, "%s: %s counter = %u steps\n", machine().describe_context(), BIT(offset, 0) ? "Vertical" : "Horizontal", data);
	m_step_counter[offset] = data;
}

u8 m50734_device::phase_counter_r()
{
	return m_phase_counter;
}

void m50734_device::phase_counter_w(u8 data)
{
	// HPHC and VPHC are 3-bit counters (TODO: P2 output)
	m_phase_counter = data & 0x77;
}

u8 m50734_device::smcon_r(offs_t offset)
{
	return m_smcon[offset];
}

void m50734_device::smcon_w(offs_t offset, u8 data)
{
	if (m_smcon[offset] != (data & 0x0f))
		LOGMASKED(LOG_INIT, "%s: SMCON%c = $%02X\n", machine().describe_context(), "HV"[offset], data);
	m_smcon[offset] = data & 0x0f;
}

TIMER_CALLBACK_MEMBER(m50734_device::timer_x_interrupt)
{
	if (!BIT(m_interrupt_control[2], 5))
	{
		m_interrupt_control[2] |= 0x20;
		if (BIT(m_interrupt_control[2], 4))
		{
			LOGMASKED(LOG_TIMER_X, "Timer X interrupt asserted at %s\n", machine().time().to_string());
			set_input_line(M740_INT4_LINE, ASSERT_LINE);
		}
	}

	if (m_tx_reload != 0)
		m_timer_x->adjust(clocks_to_attotime(16 * (m_tx_reload + 1)));
	else
	{
		m_tx_count = 0;
		m_timer_x->enable(false);
	}
}

u16 m50734_device::get_timer_x() const
{
	if (m_timer_x->enabled())
	{
		unsigned count = attotime_to_clocks(m_timer_x->remaining()) / 16;
		return count > m_tx_reload ? 0 : count;
	}
	else
		return m_tx_count;
}

void m50734_device::set_timer_x(u16 count)
{
	LOGMASKED(LOG_TIMER_X, "%s: Timer X reload value = $%04X (%s mode)\n", machine().describe_context(), count, s_timer_x_modes[BIT(m_interrupt_control[2], 0, 2)]);
	m_tx_reload = count;
	if (BIT(m_interrupt_control[2], 0, 2) != 2 && !BIT(m_interrupt_control[2], 2))
	{
		m_timer_x->adjust(clocks_to_attotime(16 * count));
		m_timer_x->enable(true);
	}
	else
		m_tx_count = count;
}

u8 m50734_device::timer_x_r(offs_t offset)
{
	return BIT(get_timer_x(), BIT(offset, 0) ? 8 : 0, 8);
}

void m50734_device::timer_x_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		set_timer_x(data << 8 | (m_tx_reload & 0x00ff));
	else
		set_timer_x(data | (m_tx_reload & 0xff00));
}

void m50734_device::internal_map(address_map &map)
{
	// TODO: other timers, UART, etc.
	map(0x00da, 0x00db).rw(FUNC(m50734_device::timer_x_r), FUNC(m50734_device::timer_x_w));
	map(0x00dc, 0x00e1).rw(FUNC(m50734_device::timer_r), FUNC(m50734_device::timer_w));
	map(0x00e2, 0x00e3).rw(FUNC(m50734_device::step_counter_r), FUNC(m50734_device::step_counter_w));
	map(0x00e9, 0x00e9).rw(FUNC(m50734_device::ad_control_r), FUNC(m50734_device::ad_control_w));
	map(0x00ea, 0x00ea).r(FUNC(m50734_device::ad_r));
	map(0x00eb, 0x00eb).r(FUNC(m50734_device::p4_r));
	map(0x00ec, 0x00ec).rw(FUNC(m50734_device::phase_counter_r), FUNC(m50734_device::phase_counter_w));
	map(0x00ed, 0x00ed).rw(FUNC(m50734_device::p2_p3_function_r), FUNC(m50734_device::p2_p3_function_w));
	map(0x00ee, 0x00ef).rw(FUNC(m50734_device::port_r<3>), FUNC(m50734_device::port_w<3>));
	map(0x00f0, 0x00f1).rw(FUNC(m50734_device::port_r<2>), FUNC(m50734_device::port_w<2>));
	map(0x00f3, 0x00f4).rw(FUNC(m50734_device::port_r<1>), FUNC(m50734_device::port_w<1>));
	map(0x00f5, 0x00f5).rw(FUNC(m50734_device::p0_function_r), FUNC(m50734_device::p0_function_w));
	map(0x00f6, 0x00f7).rw(FUNC(m50734_device::port_r<0>), FUNC(m50734_device::port_w<0>));
	map(0x00f8, 0x00f9).rw(FUNC(m50734_device::smcon_r), FUNC(m50734_device::smcon_w));
	map(0x00fd, 0x00ff).rw(FUNC(m50734_device::interrupt_control_r), FUNC(m50734_device::interrupt_control_w));
}
