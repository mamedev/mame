// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    r6511.cpp

    Rockwell R6511Q one-chip microprocessor (R6500/11 family):
    * NMOS 6502 CPU with the RMB/SMB/BBR/BBS bit instructions
    * 192*8 static RAM at 0040-00FF, stack in page zero
    * Four eight-bit I/O ports; A-C have active pull-down only
    * Edge detection on PA0/PA1 (positive) and PA2/PA3 (negative)
    * Port B input latch strobed by PA0 (port B is input only while
      the latch is enabled)
    * Two sixteen-bit counter/timers
    * Full-duplex serial channel
    * Full 16-bit external address bus and 8-bit data bus

    The internal clock is XTLI divided by two.

    Read-modify-write instructions read the port output registers;
    all other reads return the pin states.

    TODO:
    - serial channel shift timing and pin I/O
    - abbreviated and multiplexed bus modes
    - counter A forced to interval timer mode by the serial channel

***************************************************************************/

#include "emu.h"
#include "r6511.h"
#include "r6511d.h"

#include "m6502mcu.ipp"

#include <algorithm>

#define VERBOSE 0
#include "logmacro.h"


namespace {

constexpr u8 MCR_PB_LATCH   = 0x10;
constexpr u8 MCR_PD_OUTPUT  = 0x20;

constexpr u8 IFR_PA0        = 0x01;
constexpr u8 IFR_PA1        = 0x02;
constexpr u8 IFR_PA2        = 0x04;
constexpr u8 IFR_PA3        = 0x08;
constexpr u8 IFR_CA         = 0x10;
constexpr u8 IFR_CB         = 0x20;
constexpr u8 IFR_RCVR       = 0x40;
constexpr u8 IFR_XMTR       = 0x80;

constexpr u8 SCCR_RCVR      = 0x40;
constexpr u8 SCCR_XMTR      = 0x80;

constexpr u8 SCSR_RDRF      = 0x01;
constexpr u8 SCSR_RCVR_MASK = 0x0f;
constexpr u8 SCSR_EOT       = 0x20;
constexpr u8 SCSR_TDRE      = 0x40;
constexpr u8 SCSR_UNDERRUN  = 0x80;

enum : unsigned
{
	MODE_INTERVAL = 0,
	MODE_PULSE,
	MODE_EVENT,
	MODE_PWM_RETRIGGER
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(R6511, r6511_device, "r6511", "Rockwell R6511Q")


r6511_device::r6511_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	m6502_mcu_device_base<m6502_device>(mconfig, R6511, tag, owner, clock),
	m_port_in_cb(*this, 0xff),
	m_port_out_cb(*this),
	m_pa_in(0xff),
	m_pa_out(0xff),
	m_pb_latch(0xff),
	m_mcr(0),
	m_ier(0),
	m_ifr(0),
	m_sccr(0),
	m_scsr(SCSR_TDRE),
	m_rdr(0),
	m_counter_a_base(0),
	m_counter_a(0xffff),
	m_latch_a(0xffff),
	m_cnta_out(1),
	m_counter_b_base(0),
	m_counter_b(0xffff),
	m_latch_b(0xffff),
	m_latch_c(0xffff),
	m_cntb_out(1)
{
	m_program_config.m_internal_map = address_map_constructor(FUNC(r6511_device::internal_map), this);
}

std::unique_ptr<util::disasm_interface> r6511_device::create_disassembler()
{
	return std::make_unique<r6511_disassembler>();
}


void r6511_device::device_start()
{
	m6502_mcu_device_base<m6502_device>::device_start();

	m_SP = 0x0000;

	std::fill(std::begin(m_port_latch), std::end(m_port_latch), 0xff);

	state_add(R6511_MCR, "MCR", m_mcr);
	state_add(R6511_IER, "IER", m_ier);
	state_add(R6511_IFR, "IFR", m_ifr);
	state_add(R6511_SCCR, "SCCR", m_sccr);
	state_add(R6511_SCSR, "SCSR", m_scsr);
	state_add<u16>(R6511_CA, "CA",
		[this] () { internal_update(); return m_counter_a; },
		[this] (u16 data) { internal_update(); m_counter_a = data; internal_update(); });
	state_add(R6511_LA, "LA", m_latch_a);
	state_add<u16>(R6511_CB, "CB",
		[this] () { internal_update(); return m_counter_b; },
		[this] (u16 data) { internal_update(); m_counter_b = data; internal_update(); });
	state_add(R6511_LB, "LB", m_latch_b);
	state_add(R6511_LC, "LC", m_latch_c);

	save_item(NAME(m_port_latch));
	save_item(NAME(m_pa_in));
	save_item(NAME(m_pa_out));
	save_item(NAME(m_pb_latch));
	save_item(NAME(m_mcr));
	save_item(NAME(m_ier));
	save_item(NAME(m_ifr));
	save_item(NAME(m_sccr));
	save_item(NAME(m_scsr));
	save_item(NAME(m_rdr));
	save_item(NAME(m_counter_a_base));
	save_item(NAME(m_counter_a));
	save_item(NAME(m_latch_a));
	save_item(NAME(m_cnta_out));
	save_item(NAME(m_counter_b_base));
	save_item(NAME(m_counter_b));
	save_item(NAME(m_latch_b));
	save_item(NAME(m_latch_c));
	save_item(NAME(m_cntb_out));
}

void r6511_device::device_reset()
{
	m6502_mcu_device_base<m6502_device>::device_reset();

	internal_update();

	u8 const prev_pins = pa_pins();

	m_mcr = 0;
	m_ier = 0;
	m_ifr = 0;
	m_sccr = 0;
	m_scsr = SCSR_TDRE;
	m_cnta_out = 1;
	m_cntb_out = 1;

	for (int i = 0; i < 4; i++)
		m_port_latch[i] = 0xff;

	m_port_out_cb[1](0xff);
	m_port_out_cb[2](0xff);
	m_port_out_cb[3](0xff);
	pa_update(prev_pins);

	internal_update();
	update_irq();
}


u64 r6511_device::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return (clocks + 1) / 2;
}

u64 r6511_device::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles * 2;
}


void r6511_device::internal_update(u64 current_time)
{
	u64 event_time = 0;
	add_event(event_time, update_counter_a(current_time));
	add_event(event_time, update_counter_b(current_time));
	recompute_bcount(event_time);
}


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

void r6511_device::update_irq()
{
	set_input_line(M6502_IRQ_LINE, (m_ifr & m_ier) ? ASSERT_LINE : CLEAR_LINE);
}

u8 r6511_device::ifr_r()
{
	internal_update();
	return m_ifr;
}

void r6511_device::ifr_clear_w(u8 data)
{
	m_ifr &= data | 0xf0;
	update_irq();
}

u8 r6511_device::ier_r()
{
	return m_ier;
}

void r6511_device::ier_w(u8 data)
{
	internal_update();
	m_ier = data;
	update_irq();
	internal_update();
}

u8 r6511_device::mcr_r()
{
	return m_mcr;
}

void r6511_device::mcr_w(u8 data)
{
	internal_update();

	u8 const prev_mcr = m_mcr;
	u8 const prev_pins = pa_pins();
	m_mcr = data;

	if (counter_a_mode() == MODE_PULSE && (prev_mcr & 0x03) != MODE_PULSE)
		m_cnta_out = 1;
	if (counter_b_mode() == MODE_PULSE && ((prev_mcr >> 2) & 0x03) != MODE_PULSE)
		m_cntb_out = 1;

	pa_update(prev_pins);

	if ((prev_mcr ^ m_mcr) & MCR_PB_LATCH)
		m_port_out_cb[1]((m_mcr & MCR_PB_LATCH) ? 0xff : m_port_latch[1]);

	if ((prev_mcr ^ m_mcr) & MCR_PD_OUTPUT)
		m_port_out_cb[3]((m_mcr & MCR_PD_OUTPUT) ? m_port_latch[3] : 0xff);

	if ((m_mcr & 0xc0) != 0)
		LOG("%s: unsupported bus mode %u\n", machine().describe_context(), m_mcr >> 6);

	internal_update();
}


//**************************************************************************
//  I/O PORTS
//**************************************************************************

bool r6511_device::rmw_read() const
{
	if (m_inst_state >= 0x100)
		return false;

	u8 const op = m_inst_state & 0xff;

	// RMB/SMB
	if ((op & 0x0f) == 0x07)
		return true;

	// ASL/ROL/LSR/ROR/DEC/INC, excluding the STX/LDX rows
	if ((op & 0x07) != 0x06)
		return false;

	return ((op >> 5) != 4) && ((op >> 5) != 5);
}

u8 r6511_device::pa_output() const
{
	u8 data = m_port_latch[0];

	if (counter_a_mode() == MODE_PULSE)
		data = (data & ~0x10) | (m_cnta_out << 4);
	else if (counter_a_mode() == MODE_EVENT || counter_a_mode() == MODE_PWM_RETRIGGER)
		data |= 0x10;
	if (counter_b_mode() == MODE_PULSE)
		data = (data & ~0x20) | (m_cntb_out << 5);
	else if (counter_b_mode() == MODE_EVENT || counter_b_mode() == MODE_PWM_RETRIGGER)
		data |= 0x20;

	return data;
}

u8 r6511_device::pa_pins() const
{
	return m_pa_out & m_pa_in;
}

void r6511_device::pa_update(u8 prev_pins)
{
	u8 const out = pa_output();
	if (out != m_pa_out)
	{
		m_pa_out = out;
		m_port_out_cb[0](out);
	}

	u8 const pins = pa_pins();
	u8 const rise = ~prev_pins & pins;
	u8 const fall = prev_pins & ~pins;

	if (!(rise | fall))
		return;

	u8 const prev_ifr = m_ifr;
	m_ifr |= rise & (IFR_PA0 | IFR_PA1);
	m_ifr |= fall & (IFR_PA2 | IFR_PA3);

	if (BIT(rise, 0) && (m_mcr & MCR_PB_LATCH))
		m_pb_latch = m_port_in_cb[1]();

	if (BIT(rise, 4) && counter_a_mode() == MODE_EVENT)
	{
		if (m_counter_a)
		{
			m_counter_a--;
		}
		else
		{
			m_counter_a = m_latch_a;
			m_ifr |= IFR_CA;
		}
	}

	if (BIT(rise, 5))
	{
		if (counter_b_mode() == MODE_EVENT)
		{
			if (m_counter_b)
			{
				m_counter_b--;
			}
			else
			{
				m_counter_b = m_latch_b;
				m_ifr |= IFR_CB;
			}
		}
		else if (counter_b_mode() == MODE_PWM_RETRIGGER)
		{
			m_counter_b = m_latch_b;
		}
	}

	if (m_ifr != prev_ifr)
		update_irq();
}

TIMER_CALLBACK_MEMBER(r6511_device::set_pa_in)
{
	u8 const mask = 1 << (param >> 1);
	u8 const data = (param & 1) ? mask : 0;

	if ((m_pa_in & mask) == data)
		return;

	internal_update();
	u8 const prev_pins = pa_pins();
	m_pa_in = (m_pa_in & ~mask) | data;
	pa_update(prev_pins);
	internal_update();
}

u8 r6511_device::port_r(offs_t offset)
{
	if (rmw_read())
		return m_port_latch[offset];

	switch (offset)
	{
	case 0:
		return pa_pins() & m_port_in_cb[0]();

	case 1:
		if (m_mcr & MCR_PB_LATCH)
			return m_pb_latch;
		return m_port_in_cb[1]() & m_port_latch[1];

	case 2:
		return m_port_in_cb[2]() & m_port_latch[2];

	default:
		if (m_mcr & MCR_PD_OUTPUT)
			return m_port_latch[3];
		return m_port_in_cb[3]();
	}
}

void r6511_device::port_w(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		internal_update();
		u8 const prev_pins = pa_pins();
		m_port_latch[0] = data;
		pa_update(prev_pins);
		internal_update();
		return;
	}

	m_port_latch[offset] = data;

	if ((offset == 1 && (m_mcr & MCR_PB_LATCH)) || (offset == 3 && !(m_mcr & MCR_PD_OUTPUT)))
		return;

	m_port_out_cb[offset](data);
}


//**************************************************************************
//  SERIAL CHANNEL
//**************************************************************************

void r6511_device::update_serial_irq()
{
	u8 const prev_ifr = m_ifr;
	m_ifr &= ~(IFR_RCVR | IFR_XMTR);

	if ((m_sccr & SCCR_RCVR) && (m_scsr & SCSR_RCVR_MASK))
		m_ifr |= IFR_RCVR;

	if ((m_sccr & SCCR_XMTR) && (((m_scsr & SCSR_TDRE) && !(m_scsr & SCSR_EOT)) || (m_scsr & SCSR_UNDERRUN)))
		m_ifr |= IFR_XMTR;

	if (m_ifr != prev_ifr)
		update_irq();
}

u8 r6511_device::sccr_r()
{
	return m_sccr;
}

void r6511_device::sccr_w(u8 data)
{
	LOG("%s: SCCR %02x\n", machine().describe_context(), data);
	m_sccr = data;
	update_serial_irq();
}

u8 r6511_device::scsr_r()
{
	return m_scsr;
}

void r6511_device::scsr_w(u8 data)
{
	m_scsr |= data & 0x30;
	update_serial_irq();
}

u8 r6511_device::serial_data_r()
{
	if (!machine().side_effects_disabled())
	{
		m_scsr &= ~SCSR_RCVR_MASK;
		update_serial_irq();
	}

	return m_rdr;
}

void r6511_device::serial_data_w(u8 data)
{
	LOG("%s: serial transmit %02x\n", machine().describe_context(), data);

	m_scsr &= ~(SCSR_EOT | SCSR_UNDERRUN);
	if (m_sccr & SCCR_XMTR)
		m_scsr |= SCSR_TDRE;
	else
		m_scsr &= ~SCSR_TDRE;

	update_serial_irq();
}


//**************************************************************************
//  COUNTERS
//**************************************************************************

u64 r6511_device::update_counter_a(u64 current_time)
{
	u64 elapsed = current_time - m_counter_a_base;
	m_counter_a_base = current_time;

	switch (counter_a_mode())
	{
	case MODE_EVENT:
		return 0;

	case MODE_PWM_RETRIGGER:
		if (BIT(m_pa_in, 4))
			return 0;
		break;
	}

	if (elapsed <= m_counter_a)
	{
		m_counter_a -= elapsed;
	}
	else
	{
		elapsed -= m_counter_a + 1;
		u32 const period = u32(m_latch_a) + 1;
		u64 const events = elapsed / period + 1;
		m_counter_a = m_latch_a - (elapsed % period);
		m_ifr |= IFR_CA;
		update_irq();

		if (counter_a_mode() == MODE_PULSE && (events & 1))
		{
			u8 const prev_pins = pa_pins();
			m_cnta_out ^= 1;
			pa_update(prev_pins);
		}
	}

	if (counter_a_mode() == MODE_PULSE || (m_ier & IFR_CA))
		return current_time + m_counter_a + 1;

	return 0;
}

u64 r6511_device::update_counter_b(u64 current_time)
{
	u64 elapsed = current_time - m_counter_b_base;
	m_counter_b_base = current_time;

	if (counter_b_mode() == MODE_EVENT)
		return 0;

	if (counter_b_mode() == MODE_PULSE)
	{
		u8 const prev_pins = pa_pins();
		bool underflow = false;

		while (elapsed > m_counter_b)
		{
			elapsed -= m_counter_b + 1;
			underflow = true;
			m_cntb_out ^= 1;
			m_counter_b = m_cntb_out ? m_latch_c : m_latch_b;
		}
		m_counter_b -= elapsed;

		if (underflow)
		{
			m_ifr |= IFR_CB;
			update_irq();
			pa_update(prev_pins);
		}

		return current_time + m_counter_b + 1;
	}

	if (elapsed <= m_counter_b)
	{
		m_counter_b -= elapsed;
	}
	else
	{
		elapsed -= m_counter_b + 1;
		u32 const period = u32(m_latch_b) + 1;
		m_counter_b = m_latch_b - (elapsed % period);
		m_ifr |= IFR_CB;
		update_irq();
	}

	if (m_ier & IFR_CB)
		return current_time + m_counter_b + 1;

	return 0;
}

u8 r6511_device::lca_r()
{
	internal_update();

	if (!machine().side_effects_disabled())
	{
		m_ifr &= ~IFR_CA;
		update_irq();
		internal_update();
	}

	return u8(m_counter_a);
}

u8 r6511_device::uca_r()
{
	internal_update();
	return u8(m_counter_a >> 8);
}

u8 r6511_device::lca_noclear_r()
{
	internal_update();
	return u8(m_counter_a);
}

void r6511_device::lla_w(u8 data)
{
	internal_update();
	m_latch_a = (m_latch_a & 0xff00) | data;
}

void r6511_device::ula_w(u8 data)
{
	internal_update();
	m_latch_a = (m_latch_a & 0x00ff) | (u16(data) << 8);
}

void r6511_device::ula_start_w(u8 data)
{
	internal_update();

	m_latch_a = (m_latch_a & 0x00ff) | (u16(data) << 8);
	m_counter_a = m_latch_a;
	m_ifr &= ~IFR_CA;
	update_irq();

	if (counter_a_mode() == MODE_PULSE)
	{
		u8 const prev_pins = pa_pins();
		m_cnta_out = 0;
		pa_update(prev_pins);
	}

	internal_update();
}

u8 r6511_device::lcb_r()
{
	internal_update();

	if (!machine().side_effects_disabled())
	{
		m_ifr &= ~IFR_CB;
		update_irq();
		internal_update();
	}

	return u8(m_counter_b);
}

u8 r6511_device::ucb_r()
{
	internal_update();
	return u8(m_counter_b >> 8);
}

u8 r6511_device::lcb_noclear_r()
{
	internal_update();
	return u8(m_counter_b);
}

void r6511_device::llb_w(u8 data)
{
	internal_update();
	m_latch_b = (m_latch_b & 0xff00) | data;
}

void r6511_device::ulb_latch_c_w(u8 data)
{
	internal_update();
	m_latch_b = (m_latch_b & 0x00ff) | (u16(data) << 8);
	m_latch_c = m_latch_b;
}

void r6511_device::ulb_start_w(u8 data)
{
	internal_update();

	m_latch_b = (m_latch_b & 0x00ff) | (u16(data) << 8);
	m_counter_b = m_latch_b;
	m_ifr &= ~IFR_CB;
	update_irq();

	if (counter_b_mode() == MODE_PULSE)
	{
		u8 const prev_pins = pa_pins();
		m_cntb_out = 0;
		pa_update(prev_pins);
	}

	internal_update();
}


//**************************************************************************
//  ADDRESS MAP
//**************************************************************************

void r6511_device::internal_map(address_map &map)
{
	map(0x0000, 0x003f).noprw();
	map(0x0000, 0x0003).rw(FUNC(r6511_device::port_r), FUNC(r6511_device::port_w));
	map(0x0010, 0x0010).lr8(NAME([] () -> u8 { return 0xff; })).w(FUNC(r6511_device::ifr_clear_w));
	map(0x0011, 0x0011).r(FUNC(r6511_device::ifr_r));
	map(0x0012, 0x0012).rw(FUNC(r6511_device::ier_r), FUNC(r6511_device::ier_w));
	map(0x0014, 0x0014).rw(FUNC(r6511_device::mcr_r), FUNC(r6511_device::mcr_w));
	map(0x0015, 0x0015).rw(FUNC(r6511_device::sccr_r), FUNC(r6511_device::sccr_w));
	map(0x0016, 0x0016).rw(FUNC(r6511_device::scsr_r), FUNC(r6511_device::scsr_w));
	map(0x0017, 0x0017).rw(FUNC(r6511_device::serial_data_r), FUNC(r6511_device::serial_data_w));
	map(0x0018, 0x0018).rw(FUNC(r6511_device::lca_r), FUNC(r6511_device::lla_w));
	map(0x0019, 0x0019).rw(FUNC(r6511_device::uca_r), FUNC(r6511_device::ula_w));
	map(0x001a, 0x001a).rw(FUNC(r6511_device::lca_noclear_r), FUNC(r6511_device::ula_start_w));
	map(0x001c, 0x001c).rw(FUNC(r6511_device::lcb_r), FUNC(r6511_device::llb_w));
	map(0x001d, 0x001d).rw(FUNC(r6511_device::ucb_r), FUNC(r6511_device::ulb_latch_c_w));
	map(0x001e, 0x001e).rw(FUNC(r6511_device::lcb_noclear_r), FUNC(r6511_device::ulb_start_w));
	map(0x0040, 0x00ff).ram();
}


#include "cpu/m6502/r6511.hxx"
