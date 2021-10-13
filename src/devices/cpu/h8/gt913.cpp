// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************
	Casio GT913

	This chip powers several late-90s/early-2000s Casio keyboards.
	It's based on the H8/300 instruction set, but with different encoding
	for many opcodes, as well as:

	- Dedicated bank switching instructions (20-bit external address bus)
	- Two timers, three 8-bit ports, two 8-bit ADCs
	- Keyboard controller w/ key velocity detection
	- MIDI UART
	- 24-voice PCM sound (currently not emulated / fully understood)

	Earlier and later Casio keyboard models contain "uPD912" and "uPD914" chips,
	which are presumably similar.

***************************************************************************/

#include "emu.h"
#include "gt913.h"
#include "gt913d.h"

DEFINE_DEVICE_TYPE(GT913, gt913_device, "gt913", "Casio GT913F")

gt913_device::gt913_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8_device(mconfig, GT913, tag, owner, clock, address_map_constructor(FUNC(gt913_device::map), this)),
	m_rom(*this, DEVICE_SELF),
	m_bank(*this, "bank"),
	m_intc(*this, "intc"),
	m_sound(*this, "gt_sound"),
	m_kbd(*this, "kbd"),
	m_io_hle(*this, "io_hle"),
	m_sci(*this, "sci"),
	m_port(*this, "port%u", 1)
{
	has_hc = false;
}

std::unique_ptr<util::disasm_interface> gt913_device::create_disassembler()
{
	return std::make_unique<gt913_disassembler>();
}

void gt913_device::map(address_map &map)
{
//	map.unmap_value_high();

	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank");
	map(0xfac0, 0xffbf).ram(); // CTK-551 zeroes out this range at $0418

	/* ffc0-ffcb: sound */
	map(0xffc0, 0xffc5).rw(m_sound, FUNC(gt913_sound_hle_device::data_r), FUNC(gt913_sound_hle_device::data_w));
	map(0xffc6, 0xffc7).w(m_sound, FUNC(gt913_sound_hle_device::command_w));
	map(0xffca, 0xffcb).r(m_sound, FUNC(gt913_sound_hle_device::status_r));

	/* ffd0-ffd5: key controller */
	map(0xffd0, 0xffd1).r(m_kbd, FUNC(gt913_kbd_hle_device::read));
	map(0xffd2, 0xffd3).rw(m_kbd, FUNC(gt913_kbd_hle_device::status_r), FUNC(gt913_kbd_hle_device::status_w));

	/* ffd8-ffdf: timers */
	map(0xffd8, 0xffd9).rw(m_io_hle, FUNC(gt913_io_hle_device::timer_control_r), FUNC(gt913_io_hle_device::timer_control_w));
	map(0xffdc, 0xffdd).w(m_io_hle, FUNC(gt913_io_hle_device::timer_rate0_w));
	map(0xffdf, 0xffdf).w(m_io_hle, FUNC(gt913_io_hle_device::timer_rate1_w));

	/* ffe0-ffe3: serial */
	map(0xffe0, 0xffe0).w(FUNC(gt913_device::uart_rate_w));
	map(0xffe1, 0xffe1).w(m_sci, FUNC(h8_sci_device::tdr_w));
	map(0xffe2, 0xffe2).rw(FUNC(gt913_device::uart_control_r), FUNC(gt913_device::uart_control_w));
	map(0xffe3, 0xffe3).r(m_sci, FUNC(h8_sci_device::rdr_r));

	/* ffe9-ffea: ADC */
	map(0xffe9, 0xffe9).rw(m_io_hle, FUNC(gt913_io_hle_device::adc_control_r), FUNC(gt913_io_hle_device::adc_control_w));
	map(0xffea, 0xffea).r(m_io_hle, FUNC(gt913_io_hle_device::adc_data_r));

	/* fff0-fff5: I/O ports */
	map(0xfff0, 0xfff0).rw(m_port[0], FUNC(h8_port_device::ddr_r), FUNC(h8_port_device::ddr_w));
	map(0xfff1, 0xfff1).rw(m_port[1], FUNC(h8_port_device::ddr_r), FUNC(h8_port_device::ddr_w));
	map(0xfff2, 0xfff2).rw(m_port[0], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfff3, 0xfff3).rw(m_port[1], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
//	map(0xfff4, 0xfff4).nopw(); probably not port 3 DDR - ctk551 writes 0x00 but uses port 3 for output only
	map(0xfff5, 0xfff5).rw(m_port[2], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
}

void gt913_device::device_add_mconfig(machine_config &config)
{
	GT913_INTC(config, "intc");

	GT913_SOUND_HLE(config, m_sound, 0);
	GT913_KBD_HLE(config, m_kbd, "intc", 5);
	GT913_IO_HLE(config, m_io_hle, "intc", 6, 7);
	H8_SCI(config, m_sci, "intc", 8, 9, 10, 0);

	H8_PORT(config, m_port[0], h8_device::PORT_1, 0xff, 0x00);
	H8_PORT(config, m_port[1], h8_device::PORT_2, 0xff, 0x00);
	H8_PORT(config, m_port[2], h8_device::PORT_3, 0xff, 0x00);
}


void gt913_device::uart_rate_w(uint8_t data)
{
	m_sci->brr_w(data >> 1);
}

void gt913_device::uart_control_w(uint8_t data)
{
	/*
	upper 4 bits seem to correspond to the upper bits of SSR (Tx/Rx/error status)
	lower 4 bits seem to correspond to the upper bits of SCR (Tx/Rx IRQ enable, Tx/Rx enable(?))
	*/
	m_sci->ssr_w(data & 0xf0);
	m_sci->scr_w((data & 0x0f) << 4);
}

uint8_t gt913_device::uart_control_r()
{
	return (m_sci->ssr_r() & 0xf0) | (m_sci->scr_r() >> 4);
}

void gt913_device::irq_setup()
{
	CCR |= F_H;
}

void gt913_device::update_irq_filter()
{
	if (CCR & F_H)
		m_intc->set_filter(2, -1);
	else
		m_intc->set_filter(0, -1);
}

void gt913_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(taken_irq_vector));
}

void gt913_device::internal_update(uint64_t current_time)
{
	uint64_t event_time = 0;

	add_event(event_time, m_sci->internal_update(current_time));

	recompute_bcount(event_time);
}

void gt913_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

void gt913_device::device_start()
{
	h8_device::device_start();

	m_bank->configure_entries(0, m_rom->bytes() >> 12, m_rom->base(), 1 << 12);

	save_item(NAME(m_banknum));
}

void gt913_device::device_reset()
{
	h8_device::device_reset();

	m_banknum = 0;
}

#include "cpu/h8/gt913.hxx"
