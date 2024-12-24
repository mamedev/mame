// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb, Olivier Galibert
/*

Hitachi HD6305 series

TODO:
- HD6305xx has a 14-bit address space, not 13 (memory_access in m6805.h)
- add unimplemented opcodes: STOP, WAIT, DAA, and HD63705Z0 also has MUL
- add HD63705Z0 peripherals

*/

#include "emu.h"
#include "hd6305.h"
#include "m6805defs.h"

hd6305_device::hd6305_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock,
		device_type const type,
		configuration_params const &params,
		address_map_constructor internal_map) :
	m6805_base_device(mconfig, tag, owner, clock, type, params, internal_map),
	m_sci_clk(*this),
	m_sci_tx(*this),
	m_read_port(*this, 0xff),
	m_write_port(*this)
{
	std::fill(m_port_ddr_override.begin(), m_port_ddr_override.end(), 0x00);
}

// common peripherals

void hd6305_device::device_start()
{
	m6805_base_device::device_start();

	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_tcr));
	save_item(NAME(m_ssr));
	save_item(NAME(m_scr));
	save_item(NAME(m_mr));
	save_item(NAME(m_timer_last_update));
	save_item(NAME(m_sci_tx_data));
	save_item(NAME(m_sci_tx_filled));
	save_item(NAME(m_sci_tx_byte));
	save_item(NAME(m_sci_tx_step));

	m_timer_timer = timer_alloc(FUNC(hd6305_device::timer_cb), this);
	m_timer_sci = timer_alloc(FUNC(hd6305_device::sci_cb), this);
}

void hd6305_device::device_reset()
{
	m6805_base_device::device_reset();

	std::fill(m_port_data.begin(), m_port_data.end(), 0x00);
	std::fill(m_port_ddr.begin(), m_port_ddr.end(), 0x00);

	m_tdr = 0xf0;
	m_prescaler = 0x7f;
	m_tcr = 0x50;
	m_ssr = 0x3f;
	m_scr = 0x00;
	m_mr = 0x5f;

	m_timer_last_update = total_cycles();

	m_sci_tx_data = 0;

	m_sci_tx_filled = false;
	m_sci_tx_byte = 0;
	m_sci_tx_step = 0;

	m_sci_clk(1);
	m_sci_tx(1);
}

template<int Port>
u8 hd6305_device::port_r()
{
	const u8 ddr = m_port_ddr[Port] & ~m_port_ddr_override[Port];
	if(ddr == 0xff)
		return m_port_data[Port];

	return (m_port_data[Port] & ddr) | (m_read_port[Port]() & ~ddr);
}

template<int Port>
void hd6305_device::port_w(u8 data)
{
	if(data != m_port_data[Port]) {
		m_port_data[Port] = data;
		m_write_port[Port](Port, data | ~m_port_ddr[Port], m_port_ddr[Port]);
	}
}

template<int Port>
u8 hd6305_device::port_ddr_r()
{
	return m_port_ddr[Port];
}

template<int Port>
void hd6305_device::port_ddr_w(u8 data)
{
	if(data != m_port_ddr[Port]) {
		logerror("port %d ddr %c%c%c%c%c%c%c%c\n",
				Port,
				BIT(data, 7) ? 'o': 'i',
				BIT(data, 6) ? 'o': 'i',
				BIT(data, 5) ? 'o': 'i',
				BIT(data, 4) ? 'o': 'i',
				BIT(data, 3) ? 'o': 'i',
				BIT(data, 2) ? 'o': 'i',
				BIT(data, 1) ? 'o': 'i',
				BIT(data, 0) ? 'o': 'i');

		m_port_ddr[Port] = data;
		m_write_port[Port](Port, m_port_data[Port] | ~data, data);
	}
}

void hd6305_device::timer_update_regs()
{
	if(machine().side_effects_disabled())
		return;

	u32 counter = m_prescaler;
	u64 tc = machine().time().as_ticks(clock())/4;
	u32 cycles = tc - m_timer_last_update;

	u32 next_counter = counter + cycles;
	u32 shift = BIT(m_tcr, 0, 3);
	u32 steps = shift ? (((next_counter >> (shift-1)) + 1) >> 1) - (((counter >> (shift-1)) + 1) >> 1) : cycles;

	m_tdr -= steps;
	m_prescaler = next_counter & 0x7f;
	m_timer_last_update = tc;
	if(!m_tdr && steps) {
		m_tcr |= 0x80;
		if(!BIT(m_tcr, 6))
			m_pending_interrupts |= 1 << HD6305_INT_TIMER1;
	}
}

void hd6305_device::timer_wait_next_timeout()
{
	u32 shift = BIT(m_tcr, 0, 3);
	u32 cycles;
	if(shift == 0)
		cycles = 1;
	else {
		u32 masked_prescaler = ((1 << shift) - 1) & m_prescaler;
		if(masked_prescaler >= (1 << (shift-1)))
			cycles = (3 << (shift-1)) - masked_prescaler;
		else
			cycles = (1 << (shift-1)) - masked_prescaler;
	}
	if(m_tdr != 1)
		cycles += (m_tdr ? m_tdr - 1 : 0xff) << shift;
	m_timer_timer->adjust(attotime::from_ticks(cycles*4, clock()));
}

u8 hd6305_device::timer_data_r()
{
	timer_update_regs();
	return m_tdr;
}

void hd6305_device::timer_data_w(u8 data)
{
	timer_update_regs();
	m_tdr = data;
	timer_wait_next_timeout();
}

u8 hd6305_device::timer_ctrl_r()
{
	timer_update_regs();
	return m_tcr;
}

void hd6305_device::timer_ctrl_w(u8 data)
{
	timer_update_regs();
	u8 old = m_tcr;
	m_tcr = (m_tcr & data & 0x80) | (data & 0x7f);
	if((old ^ m_tcr) & 0x7f) {
		logerror("timer ctrl %02x irq=%s, %s clock=%s%s prescale=%d\n", data,
				BIT(m_tcr, 7) ? "on" : "off",
				BIT(m_tcr, 6) ? "disabled" : "enabled",
				BIT(m_tcr, 4, 2) == 0 ? "e" : BIT(m_tcr, 4, 2) == 1 ? "e&timer" : BIT(m_tcr, 4, 2) == 2 ? "off" : "timer",
				BIT(m_tcr, 3) ? " reset prescaler" : "",
				1 << BIT(m_tcr, 0, 3));

		if(BIT(m_tcr, 4, 2) != 0)
			logerror("WARNING: timer mode not implemented\n");
	}
	if((m_tcr & 0xc0) == 0x80)
		m_pending_interrupts |= 1 << HD6305_INT_TIMER1;
	else
		m_pending_interrupts &= ~(1 << HD6305_INT_TIMER1);
	if(BIT(m_tcr, 3)) {
		m_prescaler = 0x7f;
		m_tcr &= ~0x08;
	}

	timer_wait_next_timeout();
}


TIMER_CALLBACK_MEMBER(hd6305_device::timer_cb)
{
	timer_update_regs();
	timer_wait_next_timeout();
}

u8 hd6305_device::misc_r()
{
	return m_mr;
}

void hd6305_device::misc_w(u8 data)
{
	m_mr = (m_mr & data & 0x80) | (data & 0x60) | 0x1f;
	logerror("misc %02x  int2=%s, %s  int=%s\n", data,
			BIT(m_mr, 7) ? "on" : "off",
			BIT(m_mr, 6) ? "disabled" : "enabled",
			BIT(m_mr, 5) ? "edge" : "level");
}

void hd6305_device::sci_timer_step()
{
	m_timer_sci->adjust(attotime::from_ticks(1 << (BIT(m_scr, 0, 4) + 2), clock()));
}

TIMER_CALLBACK_MEMBER(hd6305_device::sci_cb)
{
	if(m_sci_tx_step == 16) {
		m_ssr |= 0x80;
		if(!BIT(m_ssr, 5))
			m_pending_interrupts |= 1 << HD6305_INT_SCI;

		if(m_sci_tx_filled) {
			m_sci_tx_filled = false;
			m_sci_clk(0);
			m_sci_tx_byte = m_sci_tx_data;
			logerror("transmit %02x\n", m_sci_tx_byte);
			m_sci_tx(m_sci_tx_byte & 0x01);
			m_sci_tx_step = 1;
		} else {
			m_sci_tx_step = 0;
			return;
		}
	} else {
		if(m_sci_tx_step & 1)
			m_sci_clk(1);
		else {
			m_sci_clk(0);
			m_sci_tx((m_sci_tx_byte >> (m_sci_tx_step >> 1)) & 0x01);
		}
		m_sci_tx_step++;
	}

	sci_timer_step();
}

u8 hd6305_device::sci_ctrl_r()
{
	return m_scr;
}

void hd6305_device::sci_ctrl_w(u8 data)
{
	m_scr = data;
	logerror("sci ctrl %02x  d3=%s d4=%s d5=%s rate=%d\n", data,
			BIT(data, 7) ? "data_out" : "gpio",
			BIT(data, 6) ? "data_in" : "gpio",
			BIT(data, 5) == 0 ? "gpio" : BIT(data, 4) ? "clock_in" : "clock_out",
			clock()/(1 << (BIT(data, 0, 4) + 3)));
}

u8 hd6305_device::sci_ssr_r()
{
	return m_ssr;
}

void hd6305_device::sci_ssr_w(u8 data)
{
	m_ssr = ((m_ssr & data) & 0xc0) | (data & 0x38) | 7;
	logerror("sci ssr w %02x sci irq=%s, %s  timer2 irq=%s, %s%s\n", data,
			BIT(m_ssr, 7) ? "on" : "off",
			BIT(m_ssr, 5) ? "disabled" : "enabled",
			BIT(m_ssr, 6) ? "on" : "off",
			BIT(m_ssr, 4) ? "disabled" : "enabled",
			BIT(m_ssr, 3) ? "reset sci prescaler" : "");

	if((m_ssr & 0xa0) == 0x80)
		m_pending_interrupts |= 1 << HD6305_INT_SCI;
	else
		m_pending_interrupts &= ~(1 << HD6305_INT_SCI);
}

u8 hd6305_device::sci_data_r()
{
	if(!machine().side_effects_disabled())
		logerror("sci data r\n");
	return 0x00;
}

void hd6305_device::sci_data_w(u8 data)
{
	m_sci_tx_data = data;
	if(m_sci_tx_step == 0) {
		m_sci_tx_filled = false;
		m_sci_clk(0);
		m_sci_tx_byte = m_sci_tx_data;
		logerror("transmit %02x\n", m_sci_tx_byte);
		m_sci_tx(m_sci_tx_byte & 0x01);
		m_sci_tx_step = 1;
		sci_timer_step();
	} else
		m_sci_tx_filled = true;
}

void hd6305_device::execute_set_input(int inputnum, int state)
{
	// TODO: edge vs. level on int1
	if(inputnum == HD6305_INT_IRQ1 || inputnum == HD6305_INT_IRQ2) {
		if(m_irq_state[inputnum] != state) {
			m_irq_state[inputnum] = state;

			if(state != CLEAR_LINE)
				m_pending_interrupts |= 1 << inputnum;
		}
	}
}

void hd6305_device::interrupt_vector()
{
	if((m_pending_interrupts & (1 << HD6305_INT_IRQ1)) != 0) {
		m_pending_interrupts &= ~(1 << HD6305_INT_IRQ1);
		rm16<false>(0x1ffa, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD6305_INT_IRQ2)) != 0) {
		m_pending_interrupts &= ~(1 << HD6305_INT_IRQ2);
		rm16<false>(0x1ff8, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD6305_INT_TIMER1)) != 0) {
		// TODO: 1ff6 when in wait...
		m_pending_interrupts &= ~(1 << HD6305_INT_TIMER1);
		rm16<false>(0x1ff8, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD6305_INT_TIMER2)) != 0) {
		m_pending_interrupts &= ~(1 << HD6305_INT_TIMER2);
		rm16<false>(0x1ff4, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD6305_INT_SCI)) != 0) {
		m_pending_interrupts &= ~(1 << HD6305_INT_SCI);
		rm16<false>(0x1ff4, m_pc);
	}
}


/****************************************************************************
 * HD6305V0 section
 ****************************************************************************/

hd6305v0_device::hd6305v0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	hd6305_device(
			mconfig,
			tag,
			owner,
			clock,
			HD6305V0,
			{ s_hmos_s_ops, s_hd6305_cycles, 14, 0x00ff, 0x00c0, 0x1fff, 0x1ffc },
			address_map_constructor(FUNC(hd6305v0_device::internal_map), this))
{
}

void hd6305v0_device::internal_map(address_map &map)
{
	map(0x0000, 0x0000).rw(FUNC(hd6305v0_device::port_r<0>), FUNC(hd6305v0_device::port_w<0>));
	map(0x0001, 0x0001).rw(FUNC(hd6305v0_device::port_r<1>), FUNC(hd6305v0_device::port_w<1>));
	map(0x0002, 0x0002).rw(FUNC(hd6305v0_device::port_r<2>), FUNC(hd6305v0_device::port_w<2>));
	map(0x0003, 0x0003).rw(FUNC(hd6305v0_device::port_r<3>), FUNC(hd6305v0_device::port_w<3>));
	map(0x0004, 0x0004).rw(FUNC(hd6305v0_device::port_ddr_r<0>), FUNC(hd6305v0_device::port_ddr_w<0>));
	map(0x0005, 0x0005).rw(FUNC(hd6305v0_device::port_ddr_r<1>), FUNC(hd6305v0_device::port_ddr_w<1>));
	map(0x0006, 0x0006).rw(FUNC(hd6305v0_device::port_ddr_r<2>), FUNC(hd6305v0_device::port_ddr_w<2>));
	map(0x0007, 0x0007).rw(FUNC(hd6305v0_device::port_ddr_r<3>), FUNC(hd6305v0_device::port_ddr_w<3>));
	map(0x0008, 0x0008).rw(FUNC(hd6305v0_device::timer_data_r), FUNC(hd6305v0_device::timer_data_w));
	map(0x0009, 0x0009).rw(FUNC(hd6305v0_device::timer_ctrl_r), FUNC(hd6305v0_device::timer_ctrl_w));
	map(0x000a, 0x000a).rw(FUNC(hd6305v0_device::misc_r), FUNC(hd6305v0_device::misc_w));
	map(0x0010, 0x0010).rw(FUNC(hd6305v0_device::sci_ctrl_r), FUNC(hd6305v0_device::sci_ctrl_w));
	map(0x0011, 0x0011).rw(FUNC(hd6305v0_device::sci_ssr_r), FUNC(hd6305v0_device::sci_ssr_w));
	map(0x0012, 0x0012).rw(FUNC(hd6305v0_device::sci_data_r), FUNC(hd6305v0_device::sci_data_w));
	map(0x0040, 0x00ff).ram();
	map(0x1000, 0x1fff).rom().region(DEVICE_SELF, 0);
}


/****************************************************************************
 * HD6305Y0 section
 ****************************************************************************/

hd6305y0_device::hd6305y0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	hd6305_device(
			mconfig,
			tag,
			owner,
			clock,
			HD6305Y0,
			{ s_hmos_s_ops, s_hd6305_cycles, 14, 0x00ff, 0x00c0, 0x1fff, 0x1ffc },
			address_map_constructor(FUNC(hd6305y0_device::internal_map), this))
{
}

void hd6305y0_device::internal_map(address_map &map)
{
	map(0x0000, 0x0000).rw(FUNC(hd6305y0_device::port_r<0>), FUNC(hd6305y0_device::port_w<0>));
	map(0x0001, 0x0001).rw(FUNC(hd6305y0_device::port_r<1>), FUNC(hd6305y0_device::port_w<1>));
	map(0x0002, 0x0002).rw(FUNC(hd6305y0_device::port_r<2>), FUNC(hd6305y0_device::port_w<2>));
	map(0x0003, 0x0003).r(FUNC(hd6305y0_device::port_r<3>));
	map(0x0004, 0x0004).w(FUNC(hd6305y0_device::port_ddr_w<0>));
	map(0x0005, 0x0005).w(FUNC(hd6305y0_device::port_ddr_w<1>));
	map(0x0006, 0x0006).w(FUNC(hd6305y0_device::port_ddr_w<2>));
	map(0x0007, 0x0007).w(FUNC(hd6305y0_device::port_ddr_w<6>));
	map(0x0008, 0x0008).rw(FUNC(hd6305y0_device::timer_data_r), FUNC(hd6305y0_device::timer_data_w));
	map(0x0009, 0x0009).rw(FUNC(hd6305y0_device::timer_ctrl_r), FUNC(hd6305y0_device::timer_ctrl_w));
	map(0x000a, 0x000a).rw(FUNC(hd6305y0_device::misc_r), FUNC(hd6305y0_device::misc_w));
	map(0x000b, 0x000b).rw(FUNC(hd6305y0_device::port_r<4>), FUNC(hd6305y0_device::port_w<4>));
	map(0x000c, 0x000c).rw(FUNC(hd6305y0_device::port_r<5>), FUNC(hd6305y0_device::port_w<5>));
	map(0x000d, 0x000d).rw(FUNC(hd6305y0_device::port_r<6>), FUNC(hd6305y0_device::port_w<6>));
	map(0x0010, 0x0010).rw(FUNC(hd6305y0_device::sci_ctrl_r), FUNC(hd6305y0_device::sci_ctrl_w));
	map(0x0011, 0x0011).rw(FUNC(hd6305y0_device::sci_ssr_r), FUNC(hd6305y0_device::sci_ssr_w));
	map(0x0012, 0x0012).rw(FUNC(hd6305y0_device::sci_data_r), FUNC(hd6305y0_device::sci_data_w));
	map(0x0040, 0x013f).ram();
	map(0x0140, 0x1fff).rom().region(DEVICE_SELF, 0);
}

void hd6305y0_device::device_reset()
{
	hd6305_device::device_reset();

	// ports E and F are write-only
	m_port_ddr[4] = m_port_ddr[5] = 0xff;
}


/****************************************************************************
 * HD6305Y2 section
 ****************************************************************************/

hd6305y2_device::hd6305y2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	hd6305_device(
			mconfig,
			tag,
			owner,
			clock,
			HD6305Y2,
			{ s_hmos_s_ops, s_hd6305_cycles, 14, 0x00ff, 0x00c0, 0x1fff, 0x1ffc },
			address_map_constructor(FUNC(hd6305y2_device::internal_map), this))
{
}

void hd6305y2_device::internal_map(address_map &map)
{
	map(0x0000, 0x0000).rw(FUNC(hd6305y2_device::port_r<0>), FUNC(hd6305y2_device::port_w<0>));
	map(0x0001, 0x0001).rw(FUNC(hd6305y2_device::port_r<1>), FUNC(hd6305y2_device::port_w<1>));
	map(0x0002, 0x0002).rw(FUNC(hd6305y2_device::port_r<2>), FUNC(hd6305y2_device::port_w<2>));
	map(0x0003, 0x0003).r(FUNC(hd6305y2_device::port_r<3>));
	map(0x0004, 0x0004).w(FUNC(hd6305y2_device::port_ddr_w<0>));
	map(0x0005, 0x0005).w(FUNC(hd6305y2_device::port_ddr_w<1>));
	map(0x0006, 0x0006).w(FUNC(hd6305y2_device::port_ddr_w<2>));
	map(0x0008, 0x0008).rw(FUNC(hd6305y2_device::timer_data_r), FUNC(hd6305y2_device::timer_data_w));
	map(0x0009, 0x0009).rw(FUNC(hd6305y2_device::timer_ctrl_r), FUNC(hd6305y2_device::timer_ctrl_w));
	map(0x000a, 0x000a).rw(FUNC(hd6305y2_device::misc_r), FUNC(hd6305y2_device::misc_w));
	map(0x0010, 0x0010).rw(FUNC(hd6305y2_device::sci_ctrl_r), FUNC(hd6305y2_device::sci_ctrl_w));
	map(0x0011, 0x0011).rw(FUNC(hd6305y2_device::sci_ssr_r), FUNC(hd6305y2_device::sci_ssr_w));
	map(0x0012, 0x0012).rw(FUNC(hd6305y2_device::sci_data_r), FUNC(hd6305y2_device::sci_data_w));
	// All other page zero addresses up to 0x001f are reserved
	map(0x0040, 0x013f).ram();
}


/****************************************************************************
 * HD63705Z0 section
 ****************************************************************************/

hd63705z0_device::hd63705z0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	hd6305_device(
			mconfig,
			tag,
			owner,
			clock,
			HD63705Z0,
			{ s_hmos_b_ops, s_hd63705_cycles, 16, 0x017f, 0x0100, 0x1fff, 0x1ffa },
			address_map_constructor(FUNC(hd63705z0_device::internal_map), this))
{
}

void hd63705z0_device::internal_map(address_map &map)
{
	// TODO: ports, timer, SCI
	map(0x0040, 0x01bf).ram();
	map(0x01c0, 0x1fff).rom().region(DEVICE_SELF, 0x01c0);
}

void hd63705z0_device::execute_set_input(int inputnum, int state)
{
	if(inputnum == INPUT_LINE_NMI) {
		if(m_nmi_state != state) {
			m_nmi_state = state;

			if(state != CLEAR_LINE)
				m_pending_interrupts |= 1 << HD63705_INT_NMI;
		}
	}
	else if(inputnum <= HD63705_INT_ADCONV) {
		if(m_irq_state[inputnum] != state) {
			m_irq_state[inputnum] = state;

			if(state != CLEAR_LINE)
				m_pending_interrupts |= 1 << inputnum;
		}
	}
}

void hd63705z0_device::interrupt_vector()
{
	// Need to add emulation of other interrupt sources here KW-2/4/99
	// This is just a quick patch for Namco System 2 operation

	if((m_pending_interrupts & (1 << HD63705_INT_IRQ1)) != 0) {
		m_pending_interrupts &= ~(1 << HD63705_INT_IRQ1);
		rm16<true>(0x1ff8, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD63705_INT_IRQ2)) != 0) {
		m_pending_interrupts &= ~(1 << HD63705_INT_IRQ2);
		rm16<true>(0x1fec, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD63705_INT_ADCONV)) != 0) {
		m_pending_interrupts &= ~(1 << HD63705_INT_ADCONV);
		rm16<true>(0x1fea, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD63705_INT_TIMER1)) != 0) {
		m_pending_interrupts &= ~(1 << HD63705_INT_TIMER1);
		rm16<true>(0x1ff6, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD63705_INT_TIMER2)) != 0) {
		m_pending_interrupts &= ~(1 << HD63705_INT_TIMER2);
		rm16<true>(0x1ff4, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD63705_INT_TIMER3)) != 0) {
		m_pending_interrupts &= ~(1<<HD63705_INT_TIMER3);
		rm16<true>(0x1ff2, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD63705_INT_PCI)) != 0) {
		m_pending_interrupts &= ~(1 << HD63705_INT_PCI);
		rm16<true>(0x1ff0, m_pc);
	}
	else if((m_pending_interrupts & (1 << HD63705_INT_SCI)) != 0) {
		m_pending_interrupts &= ~(1 << HD63705_INT_SCI);
		rm16<true>(0x1fee, m_pc);
	}
}

void hd63705z0_device::interrupt()
{
	// the 6805 latches interrupt requests internally, so we don't clear
	// pending_interrupts until the interrupt is taken, no matter what the
	// external IRQ pin does.

	if(BIT(m_pending_interrupts, HD63705_INT_NMI)) {
		pushword<true>(m_pc);
		pushbyte<true>(m_x);
		pushbyte<true>(m_a);
		pushbyte<true>(m_cc);
		SEI;

		// no vectors supported, just do the callback to clear irq_state if needed
		standard_irq_callback(0, m_pc.w.l);

		rm16<true>(0x1ffc, m_pc);
		m_pending_interrupts &= ~(1 << HD63705_INT_NMI);

		m_icount -= 11;
		burn_cycles(11);
	}
	else if((m_pending_interrupts & ((1 << M6805_IRQ_LINE) | HD63705_INT_MASK)) != 0) {
		if((CC & IFLAG) == 0) {
			// standard IRQ
			pushword<true>(m_pc);
			pushbyte<true>(m_x);
			pushbyte<true>(m_a);
			pushbyte<true>(m_cc);
			SEI;

			// no vectors supported, just do the callback to clear irq_state if needed
			standard_irq_callback(0, m_pc.w.l);

			interrupt_vector();

			m_pending_interrupts &= ~(1 << M6805_IRQ_LINE);

			m_icount -= 11;
			burn_cycles(11);
		}
	}
}


DEFINE_DEVICE_TYPE(HD6305V0,  hd6305v0_device,  "hd6305v0",  "Hitachi HD6305V0")
DEFINE_DEVICE_TYPE(HD6305Y0,  hd6305y0_device,  "hd6305y0",  "Hitachi HD6305Y0")
DEFINE_DEVICE_TYPE(HD6305Y2,  hd6305y2_device,  "hd6305y2",  "Hitachi HD6305Y2")
DEFINE_DEVICE_TYPE(HD63705Z0, hd63705z0_device, "hd63705z0", "Hitachi HD63705Z0")
