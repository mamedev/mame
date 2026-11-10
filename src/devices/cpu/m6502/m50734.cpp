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
#define LOG_INT     (1U << 4)
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
	, m_sclk_cb(*this)
	, m_sio_out_cb(*this)
	, m_sio_in_cb(*this, 1)
	, m_port_latch{0, 0, 0, 0}
	, m_port_3state{0, 0, 0, 0}
	, m_p1_latch(0)
	, m_ad_control(0)
	, m_ad_register(0)
	, m_prescaler_reload{0xff, 0xff, 0xff}
	, m_timer_reload{0xff, 0xff, 0xff}
	, m_step_counter{0, 0}
	, m_phase_counter(0)
	, m_p2_output(0)
	, m_smcon{0, 0}
	, m_tx_count(0)
	, m_tx_reload(0xffff)
	, m_timer_w_count(0xff)
	, m_timer_w_base(0)
	, m_sio_data(0)
	, m_sio_counter(0)
	, m_sclk_state(true)
	, m_int_state{false, false}
	, m_cntr_state(false)
{
	m_program_config.m_internal_map = address_map_constructor(FUNC(m50734_device::internal_map), this);
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

u8 m50734_device::phase_output(int which) const
{
	// Phase decoder patterns, from Figure 11 (relation between the phase counter
	// and the phase output signals) of the M50734SP/FP datasheet in the 1989
	// Mitsubishi Single-Chip 8-Bit Microcomputers data book.  Ha is P20/P24 and Hd
	// is P23/P27, so as a nibble the 1-2 phase sequence is 9,1,3,2,6,4,c,8 and the
	// 2-2 phase sequence is 9,3,6,c, repeating every four counts.
	static constexpr u8 PHASE_2_2[4] = { 0x9, 0x3, 0x6, 0xc };
	static constexpr u8 PHASE_1_2[8] = { 0x9, 0x1, 0x3, 0x2, 0x6, 0x4, 0xc, 0x8 };

	const u8 phase = BIT(m_phase_counter, which * 4, 3);
	return BIT(m_smcon[which], 0) ? PHASE_1_2[phase] : PHASE_2_2[phase & 3];
}

template <int N>
void m50734_device::port_update()
{
	u8 data = (m_port_latch[N] & m_port_direction[N]) | (m_port_3state[N] & ~m_port_direction[N]);

	if constexpr (N == 2)
	{
		// the stepper motor phase outputs replace the port latch when selected
		if (BIT(m_p2_p3_function, 6))
			data = (data & 0xf0) | phase_output(0);
		if (BIT(m_p2_p3_function, 7))
			data = (data & 0x0f) | (phase_output(1) << 4);
		if (data == m_p2_output)
			return;
		m_p2_output = data;
	}

	m_port_out_cb[N](data);
}

void m50734_device::step_motor(int which)
{
	if (BIT(m_smcon[which], 2))
		clock_motor(which);
}

void m50734_device::clock_motor(int which)
{
	// Timer underflow (while linked) and single shot share one clock for HC/VC and HPHC/VPHC
	if (--m_step_counter[which] == 0)
	{
		LOGMASKED(LOG_TIMER, "%s counter empty at %s\n", which ? "Vertical" : "Horizontal", machine().time().to_string());
		request_interrupt(0, 0x20 << (which * 2));
	}

	// Increment or decrement HPHC/VPHC
	m_phase_counter = (m_phase_counter + ((BIT(m_smcon[which], 1) ? 7 : 1) << (which * 4))) & 0x77;
	port_update<2>();
}

template <int N>
TIMER_CALLBACK_MEMBER(m50734_device::timer_interrupt)
{
	LOGMASKED(LOG_TIMER, "Timer %d underflow at %s\n", N + 1, machine().time().to_string());
	request_interrupt(1, 1 << (N * 2 + 1));

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
	m_timer_s = timer_alloc(FUNC(m50734_device::timer_s_strobe), this);
	m_sio_timer = timer_alloc(FUNC(m50734_device::sio_clock), this);

	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_direction));
	save_item(NAME(m_p1_latch));
	save_item(NAME(m_p0_function));
	save_item(NAME(m_p2_p3_function));
	save_item(NAME(m_ad_control));
	save_item(NAME(m_ad_register));
	save_item(NAME(m_prescaler_reload));
	save_item(NAME(m_timer_reload));
	save_item(NAME(m_step_counter));
	save_item(NAME(m_phase_counter));
	save_item(NAME(m_p2_output));
	save_item(NAME(m_smcon));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_tx_reload));
	save_item(NAME(m_timer_w_count));
	save_item(NAME(m_timer_w_base));
	save_item(NAME(m_sio_data));
	save_item(NAME(m_sio_counter));
	save_item(NAME(m_sclk_state));
	save_item(NAME(m_int_state));
	save_item(NAME(m_cntr_state));
	save_item(NAME(m_interrupt_control));
	save_item(NAME(m_ic_pending));
}

void m50734_device::device_reset()
{
	m740_device::device_reset();
	m_SP = 0x01ff;

	// Reset port registers
	std::fill(std::begin(m_port_direction), std::end(m_port_direction), 0x00);
	for (int n = 0; n < 4; n++)
		m_port_out_cb[n](m_port_3state[n]);
	m_p0_function = 0x00;
	m_p2_p3_function = 0x00;
	m_p2_output = m_port_3state[2];

	// Reset A-D
	m_ad_control |= 0x04;
	m_ad_timer->adjust(attotime::never);

	// Reset stepper motor control registers (datasheet has no implication of RESET affecting these, but mps1200 never fully initializes SMCONH)
	m_smcon[0] = 0;
	m_smcon[1] = 0;

	// Reset interrupts
	std::fill(std::begin(m_interrupt_control), std::end(m_interrupt_control), 0x00);
	std::fill(std::begin(m_ic_pending), std::end(m_ic_pending), 0x00);
	m_int_state[0] = m_int_state[1] = false;
	m_cntr_state = false;
	set_input_line(M740_INT0_LINE, CLEAR_LINE);
	set_input_line(M740_INT1_LINE, CLEAR_LINE);
	set_input_line(M740_INT2_LINE, CLEAR_LINE);
	set_input_line(M740_INT3_LINE, CLEAR_LINE);
	set_input_line(M740_INT4_LINE, CLEAR_LINE);

	m_sio_counter = 0;
	m_sclk_state = true;
	m_sio_timer->adjust(attotime::never);
	m_timer_s->adjust(attotime::never);

	m_timer_w_count = 0xff;
	m_timer_w_base = total_cycles();

	// Initialize Timer X
	set_timer_x(0x0200);
}

void m50734_device::read_dummy(u16 adr)
{
	// M50734 outputs RD and WR strobes rather than R/W, so dummy accesses should do nothing
}

// P05 only strobes DME while it is both selected as that function and an output, which is
// how firmware reaches program memory through an addressing mode that would otherwise bank
bool m50734_device::dme_active() const
{
	return BIT(m_p0_function, 5) && BIT(m_port_direction[0], 5);
}

u8 m50734_device::read_data(u16 adr)
{
	if (dme_active())
		return m_data.read_interruptible(adr);
	else
		return m740_device::read(adr);
}

void m50734_device::write_data(u16 adr, u8 val)
{
	if (dme_active())
		m_data.write_interruptible(adr, val);
	else
		m740_device::write(adr, val);
}

void m50734_device::execute_set_input(int inputnum, int state)
{
	if (inputnum == M50734_CNTR_LINE)
	{
		bool level = state != CLEAR_LINE;
		if (m_cntr_state == level)
			return;
		m_cntr_state = level;

		if (level != BIT(m_interrupt_control[2], 3))
			return;
		LOGMASKED(LOG_INT, "CNTR interrupt requested at %s\n", machine().time().to_string());
		request_interrupt(2, 0x80);
		return;
	}

	if (inputnum != M50734_INT1_LINE && inputnum != M50734_INT2_LINE)
	{
		m740_device::execute_set_input(inputnum, state);
		return;
	}

	int n = inputnum - M50734_INT1_LINE;
	bool asserted = state != CLEAR_LINE;
	if (m_int_state[n] == asserted)
		return;
	m_int_state[n] = asserted;
	if (!asserted)
		return;

	if (n == 0)
		m_p1_latch = m_port_in_cb[1]();

	LOGMASKED(LOG_INT, "_INT%d interrupt requested at %s\n", n + 1, machine().time().to_string());
	request_interrupt(0, 1 << (n * 2 + 1));
}

// Request bits are set by hardware and cleared by software.  The 740 family writes a
// hardware request into the register at the end of the instruction during which it
// was raised (3804 Group datasheet Fig. 24, timing point IR1; Renesas TN-740-A111),
// so the write-back of a SEB/CLB on the same register cannot discard it.  The
// interrupt line is driven at once; only the bit's visibility to software waits.
void m50734_device::request_interrupt(int reg, u8 mask)
{
	const u8 old_effective = m_interrupt_control[reg] | m_ic_pending[reg];
	m_ic_pending[reg] |= mask;
	update_interrupt_lines(reg, old_effective, m_interrupt_control[reg] | m_ic_pending[reg]);
}

// Each request bit is paired with the enable bit below it
void m50734_device::update_interrupt_lines(int reg, u8 old_effective, u8 new_effective)
{
	const u8 old_active = old_effective & (old_effective >> 1);
	const u8 new_active = new_effective & (new_effective >> 1);
	if (old_active == new_active)
		return;

	LOGMASKED(LOG_INT, "%s: Interrupt control register %d active requests $%02X -> $%02X\n", machine().describe_context(), reg + 1, old_active, new_active);
	switch (reg)
	{
	case 0:
		if ((old_active ^ new_active) & 0x01)
			m740_device::execute_set_input(M740_INT1_LINE, BIT(new_active, 0) ? ASSERT_LINE : CLEAR_LINE);
		if ((old_active ^ new_active) & 0x04)
			m740_device::execute_set_input(M740_INT0_LINE, BIT(new_active, 2) ? ASSERT_LINE : CLEAR_LINE);
		if (bool(old_active & 0x50) != bool(new_active & 0x50))
			set_input_line(M740_INT3_LINE, (new_active & 0x50) ? ASSERT_LINE : CLEAR_LINE);
		break;

	case 1:
		if (bool(old_active & 0x15) != bool(new_active & 0x15))
			set_input_line(M740_INT2_LINE, (new_active & 0x15) ? ASSERT_LINE : CLEAR_LINE);
		break;

	case 2:
		if (bool(old_active & 0x50) != bool(new_active & 0x50))
			set_input_line(M740_INT4_LINE, (new_active & 0x50) ? ASSERT_LINE : CLEAR_LINE);
		break;
	}
}

void m50734_device::prefetch_end()
{
	for (int reg = 0; reg < 3; reg++)
		m_interrupt_control[reg] |= std::exchange(m_ic_pending[reg], 0);

	m740_device::prefetch_end();
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

	// Request bits (ICON1 7,5,3,1; ICON2 5,3,1; ICON3 7,5) can be cleared but not set by software
	static constexpr u8 REQUEST_BITS[3] = { 0xaa, 0x2a, 0xa0 };
	const int reg = 2 - offset;
	const u8 req = REQUEST_BITS[reg];
	data = (data & ~req) | (data & m_interrupt_control[reg] & req);

	const u8 old_effective = m_interrupt_control[reg] | m_ic_pending[reg];
	u8 old_control = std::exchange(m_interrupt_control[reg], data);
	if (offset == 0)
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
	}

	update_interrupt_lines(reg, old_effective, data | m_ic_pending[reg]);
}

template <int N>
u8 m50734_device::port_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_port_direction[N];
	else if (m_port_direction[N] == 0xff)
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
			port_update<N>();
		}
	}
	else if (((std::exchange(m_port_latch[N], data) ^ data) & m_port_direction[N]) != 0)
		port_update<N>();
}

u8 m50734_device::p4_r()
{
	// P4 has only 4 pins and no output drivers
	return m_port_in_cb[4]() & 0x0f;
}

u8 m50734_device::p1_latch_r()
{
	// transparent except while _INT1 is held low
	if (!m_int_state[0])
		m_p1_latch = m_port_in_cb[1]();
	return m_p1_latch;
}

u8 m50734_device::sio_r()
{
	return m_sio_data;
}

void m50734_device::sio_w(u8 data)
{
	m_sio_data = data;
	m_sio_counter = 8;
	m_sio_out_cb(BIT(data, 7));
	m_sclk_state = false;
	m_sclk_cb(0);

	m_sio_timer->adjust(clocks_to_attotime(2));
}

TIMER_CALLBACK_MEMBER(m50734_device::sio_clock)
{
	m_sclk_state = !m_sclk_state;
	if (m_sclk_state)
	{
		// MSB out first, the level on Sio re-enters at the LSB
		int in = m_sio_in_cb();
		m_sclk_cb(1);
		m_sio_data = (m_sio_data << 1) | (in & 1);
		if (--m_sio_counter == 0)
			return;
		m_sio_out_cb(BIT(m_sio_data, 7));
	}
	else
		m_sclk_cb(0);

	m_sio_timer->adjust(clocks_to_attotime(2));
}

unsigned m50734_device::timer_s_divider() const
{
	return BIT(m_p2_p3_function, 2) ? 16 : 64;
}

u8 m50734_device::timer_s_r()
{
	if (!m_timer_s->enabled())
		return 0;

	return std::min<u32>(attotime_to_clocks(m_timer_s->remaining()) / timer_s_divider(), 0xff);
}

void m50734_device::timer_s_w(u8 data)
{
	m_timer_s->adjust(clocks_to_attotime(timer_s_divider() * (data + 1)));
}

TIMER_CALLBACK_MEMBER(m50734_device::timer_s_strobe)
{
	// terminal count sets the P04 latch, ending the strobe the program began by clearing it
	if (BIT(m_p0_function, 4) && !BIT(m_port_latch[0], 4))
	{
		m_port_latch[0] |= 0x10;
		port_update<0>();
	}
}

u8 m50734_device::timer_w_r()
{
	// free-running 8-bit counter behind a 10-bit prescaler on f(Xin)/16
	return m_timer_w_count - u8((total_cycles() - m_timer_w_base) / (4 * 1024));
}

void m50734_device::timer_w_w(u8 data)
{
	m_timer_w_count = data;
	m_timer_w_base = total_cycles();
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
	port_update<2>();
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
	// HPHC and VPHC are 3-bit counters
	m_phase_counter = data & 0x77;
	port_update<2>();
}

u8 m50734_device::smcon_r(offs_t offset)
{
	return m_smcon[offset];
}

void m50734_device::smcon_w(offs_t offset, u8 data)
{
	if (m_smcon[offset] != (data & 0x07))
		LOGMASKED(LOG_INIT, "%s: SMCON%c = $%02X\n", machine().describe_context(), "HV"[offset], data);

	// Bit 3 triggers a single shot and is not stored, as SMCON is meant to be
	// changed with SEB/CLB, which would otherwise fire it again.  The datasheet
	// forbids single shots while bit 2 links the counters to their timer.
	m_smcon[offset] = data & 0x07;
	port_update<2>();

	if (BIT(data, 3))
	{
		LOGMASKED(LOG_TIMER, "%s: %s single shot at %s\n", machine().describe_context(), offset ? "Vertical" : "Horizontal", machine().time().to_string());
		clock_motor(offset);
	}
}

TIMER_CALLBACK_MEMBER(m50734_device::timer_x_interrupt)
{
	LOGMASKED(LOG_TIMER_X, "Timer X underflow at %s\n", machine().time().to_string());
	request_interrupt(2, 0x20);

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
	// TODO: UART, baud rate generator
	map(0x00da, 0x00db).rw(FUNC(m50734_device::timer_x_r), FUNC(m50734_device::timer_x_w));
	map(0x00dc, 0x00e1).rw(FUNC(m50734_device::timer_r), FUNC(m50734_device::timer_w));
	map(0x00e2, 0x00e3).rw(FUNC(m50734_device::step_counter_r), FUNC(m50734_device::step_counter_w));
	map(0x00e8, 0x00e8).rw(FUNC(m50734_device::sio_r), FUNC(m50734_device::sio_w));
	map(0x00e9, 0x00e9).rw(FUNC(m50734_device::ad_control_r), FUNC(m50734_device::ad_control_w));
	map(0x00ea, 0x00ea).r(FUNC(m50734_device::ad_r));
	map(0x00eb, 0x00eb).r(FUNC(m50734_device::p4_r));
	map(0x00ec, 0x00ec).rw(FUNC(m50734_device::phase_counter_r), FUNC(m50734_device::phase_counter_w));
	map(0x00ed, 0x00ed).rw(FUNC(m50734_device::p2_p3_function_r), FUNC(m50734_device::p2_p3_function_w));
	map(0x00ee, 0x00ef).rw(FUNC(m50734_device::port_r<3>), FUNC(m50734_device::port_w<3>));
	map(0x00f0, 0x00f1).rw(FUNC(m50734_device::port_r<2>), FUNC(m50734_device::port_w<2>));
	map(0x00f2, 0x00f2).r(FUNC(m50734_device::p1_latch_r));
	map(0x00f3, 0x00f4).rw(FUNC(m50734_device::port_r<1>), FUNC(m50734_device::port_w<1>));
	map(0x00f5, 0x00f5).rw(FUNC(m50734_device::p0_function_r), FUNC(m50734_device::p0_function_w));
	map(0x00f6, 0x00f7).rw(FUNC(m50734_device::port_r<0>), FUNC(m50734_device::port_w<0>));
	map(0x00f8, 0x00f9).rw(FUNC(m50734_device::smcon_r), FUNC(m50734_device::smcon_w));
	map(0x00fa, 0x00fa).rw(FUNC(m50734_device::timer_s_r), FUNC(m50734_device::timer_s_w));
	map(0x00fc, 0x00fc).rw(FUNC(m50734_device::timer_w_r), FUNC(m50734_device::timer_w_w));
	map(0x00fd, 0x00ff).rw(FUNC(m50734_device::interrupt_control_r), FUNC(m50734_device::interrupt_control_w));
}
