// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    Casio GT913 (uPD913)

    This chip powers several late-90s/early-2000s Casio keyboards.
    It's based on the H8/300 instruction set, but with different encoding
    for many opcodes, as well as:

    - Dedicated bank switching instructions
      (20-bit external address bus + 3 chip select outputs, can address a total of 4MB)
    - Two timers, three 8-bit ports, two 8-bit ADCs
    - Keyboard controller w/ key velocity detection
    - MIDI UART
    - 24-voice DPCM sound

    Variants include the uPD912 and GT915/uPD915.
    These were later succeeded by the uPD914.

***************************************************************************/

#include "emu.h"
#include "gt913.h"
#include "gt913d.h"

DEFINE_DEVICE_TYPE(GT913, gt913_device, "gt913", "Casio GT913F")

gt913_device::gt913_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8_device(mconfig, GT913, tag, owner, clock, address_map_constructor(FUNC(gt913_device::map), this)),
	device_mixer_interface(mconfig, *this),
	m_rom(*this, DEVICE_SELF),
	m_data_config("data", ENDIANNESS_BIG, 16, 22, 0),
	m_write_ple(*this),
	m_intc(*this, "intc"),
	m_sound(*this, "gt_sound"),
	m_kbd(*this, "kbd"),
	m_io_hle(*this, "io_hle"),
	m_port(*this, "port%u", 1)
{
	m_has_hc = false;
}

std::unique_ptr<util::disasm_interface> gt913_device::create_disassembler()
{
	return std::make_unique<gt913_disassembler>();
}

void gt913_device::map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).rw(FUNC(gt913_device::data_r), FUNC(gt913_device::data_w));
	map(0xc000, 0xf7ff).rom();

	/* ctk530 writes here to latch LED matrix data, which generates an active high strobe on pin 99 (PLE/P16)
	   there's otherwise no external address decoding (or the usual read/write strobes) used for the LED latches.
	   just treat as a 16-bit write-only port for now */
	map(0xe000, 0xe001).lw16(NAME([this](u16 data) { m_write_ple(data); }));

	map(0xfac0, 0xffbf).ram();

	/* ffc0-ffcb: sound */
	map(0xffc0, 0xffc5).rw(m_sound, FUNC(gt913_sound_device::data_r), FUNC(gt913_sound_device::data_w));
	map(0xffc6, 0xffc7).w(m_sound, FUNC(gt913_sound_device::command_w));
	map(0xffca, 0xffcb).r(m_sound, FUNC(gt913_sound_device::status_r));

	/* ffd0-ffd5: key controller */
	map(0xffd0, 0xffd1).r(m_kbd, FUNC(gt913_kbd_hle_device::read));
	map(0xffd2, 0xffd3).rw(m_kbd, FUNC(gt913_kbd_hle_device::status_r), FUNC(gt913_kbd_hle_device::status_w));

	/* ffd8-ffdf: timers */
	map(0xffd8, 0xffd9).rw(m_io_hle, FUNC(gt913_io_hle_device::timer_control_r), FUNC(gt913_io_hle_device::timer_control_w));
	map(0xffdc, 0xffdd).w(m_io_hle, FUNC(gt913_io_hle_device::timer_rate0_w));
	map(0xffdf, 0xffdf).w(m_io_hle, FUNC(gt913_io_hle_device::timer_rate1_w));

	/* ffe0-ffe7: serial */
	map(0xffe0, 0xffe0).w(FUNC(gt913_device::uart_rate_w));
	map(0xffe1, 0xffe1).w(m_sci[0], FUNC(h8_sci_device::tdr_w));
	map(0xffe2, 0xffe2).select(0x04).rw(FUNC(gt913_device::uart_control_r), FUNC(gt913_device::uart_control_w));
	map(0xffe3, 0xffe3).r(m_sci[0], FUNC(h8_sci_device::rdr_r));
	map(0xffe5, 0xffe5).w(m_sci[1], FUNC(h8_sci_device::tdr_w));
	map(0xffe7, 0xffe7).r(m_sci[1], FUNC(h8_sci_device::rdr_r));

	/* ffe9-ffea: ADC */
	map(0xffe9, 0xffe9).rw(m_io_hle, FUNC(gt913_io_hle_device::adc_control_r), FUNC(gt913_io_hle_device::adc_control_w));
	map(0xffea, 0xffea).r(m_io_hle, FUNC(gt913_io_hle_device::adc_data_r));

	/* fff0-fff5: I/O ports */
	map(0xfff0, 0xfff0).rw(m_port[0], FUNC(h8_port_device::ddr_r), FUNC(h8_port_device::ddr_w));
	// port 2 DDR - ctk601 and gz70sp both seem to use only bit 0 to indicate either all inputs or all outputs
//  map(0xfff1, 0xfff1).rw(m_port[1], FUNC(h8_port_device::ddr_r), FUNC(h8_port_device::ddr_w));
	map(0xfff1, 0xfff1).lw8(NAME([this](u8 data) { m_port[1]->ddr_w(BIT(data, 0) ? 0xff : 0x00); }));
	map(0xfff2, 0xfff2).rw(m_port[0], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfff3, 0xfff3).rw(m_port[1], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfff4, 0xfff4).rw(m_port[2], FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfff5, 0xfff5).rw(FUNC(gt913_device::syscr_r), FUNC(gt913_device::syscr_w));
}

void gt913_device::device_add_mconfig(machine_config &config)
{
	GT913_INTC(config, m_intc, *this);

	GT913_SOUND(config, m_sound, DERIVED_CLOCK(1, 1));
	m_sound->set_device_rom_tag(m_rom);
	m_sound->add_route(0, *this, 1.0, 0);
	m_sound->add_route(1, *this, 1.0, 1);

	GT913_KBD_HLE(config, m_kbd, 0);
	m_kbd->irq_cb().set([this] (int val) {
							if(val)
								m_intc->internal_interrupt(5);
							else
								m_intc->clear_interrupt(5);
						});
	GT913_IO_HLE(config, m_io_hle, *this, m_intc, 6, 7);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 8, 9, 10, 0);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 11, 12, 13, 0);

	H8_PORT(config, m_port[0], *this, h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, m_port[1], *this, h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, m_port[2], *this, h8_device::PORT_3, 0x00, 0x00);
}


void gt913_device::uart_rate_w(u8 data)
{
	// TODO: how is SCI1 baud rate actually selected?
	// gz70sp writes 0x7e to ffe4 to select 31250 baud for MIDI, which doesn't seem right
	m_sci[0]->brr_w(data >> 2);
	m_sci[1]->brr_w(data >> 2);
}

void gt913_device::uart_control_w(offs_t offset, u8 data)
{
	const unsigned num = BIT(offset, 2);
	/*
	upper 4 bits seem to correspond to the upper bits of SSR (Tx/Rx/error status)
	lower 4 bits seem to correspond to the upper bits of SCR (Tx/Rx IRQ enable, Tx/Rx enable(?))
	*/
	m_sci[num]->ssr_w(data & 0xf0);
	m_sci[num]->scr_w((data & 0x0f) << 4);
}

u8 gt913_device::uart_control_r(offs_t offset)
{
	const unsigned num = BIT(offset, 2);
	return (m_sci[num]->ssr_r() & 0xf0) | (m_sci[num]->scr_r() >> 4);
}

void gt913_device::syscr_w(u8 data)
{
	// NMI active edge
	m_intc->set_nmi_edge(BIT(data, 2));

	m_syscr = data;
}

u8 gt913_device::syscr_r()
{
	return m_syscr;
}

void gt913_device::data_w(offs_t offset, u8 data)
{
	m_data.write_byte(offset | (m_banknum & 0xff) << 14, data);
}

u8 gt913_device::data_r(offs_t offset)
{
	return m_data.read_byte(offset | (m_banknum & 0xff) << 14);
}

u8 gt913_device::read8ib(u32 adr)
{
	if(BIT(m_syscr, 0))
		// indirect bank disabled
		return m_program.read_byte(adr);
	else if((m_IR[0] & 0x0070) == 0)
		// indirect bank enabled, using bankh for r0
		return m_data.read_byte(adr | ((m_banknum >> 6) << 16));
	else
		// indirect bank enabled, using bankl for other regs
		return m_data.read_byte(adr | ((m_banknum & 0x3f) << 16));
}

void gt913_device::write8ib(u32 adr, u8 data)
{
	if(BIT(m_syscr, 0))
		// indirect bank disabled
		m_program.write_byte(adr, data);
	else if((m_IR[0] & 0x0070) == 0)
		// indirect bank enabled, using bankh for r0
		m_data.write_byte(adr | ((m_banknum >> 6) << 16), data);
	else
		// indirect bank enabled, using bankl for other regs
		m_data.write_byte(adr | ((m_banknum & 0x3f) << 16), data);
}

u16 gt913_device::read16ib(u32 adr)
{
	adr &= ~1;

	if(BIT(m_syscr, 0))
		// indirect bank disabled
		return m_program.read_word(adr);
	else if((m_IR[0] & 0x0070) == 0)
		// indirect bank enabled, using bankh for r0
		return m_data.read_word(adr | ((m_banknum >> 6) << 16));
	else
		// indirect bank enabled, using bankl for other regs
		return m_data.read_word(adr | ((m_banknum & 0x3f) << 16));
}

void gt913_device::write16ib(u32 adr, u16 data)
{
	adr &= ~1;

	if(BIT(m_syscr, 0))
		// indirect bank disabled
		m_program.write_word(adr, data);
	else if((m_IR[0] & 0x0070) == 0)
		// indirect bank enabled, using bankh for r0
		m_data.write_word(adr | ((m_banknum >> 6) << 16), data);
	else
		// indirect bank enabled, using bankl for other regs
		m_data.write_word(adr | ((m_banknum & 0x3f) << 16), data);
}

void gt913_device::irq_setup()
{
	m_CCR |= F_H;
}

void gt913_device::update_irq_filter()
{
	if(m_CCR & F_H)
		m_intc->set_filter(2, -1);
	else
		m_intc->set_filter(0, -1);
}

void gt913_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_NPC);
}

void gt913_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));

	recompute_bcount(event_time);
}

void gt913_device::notify_standby(int state)
{
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
}

void gt913_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

device_memory_interface::space_config_vector gt913_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

void gt913_device::device_start()
{
	h8_device::device_start();

	space(AS_DATA).specific(m_data);

	save_item(NAME(m_banknum));
	save_item(NAME(m_syscr));
}

void gt913_device::device_reset()
{
	h8_device::device_reset();

	m_banknum = 0;
	m_syscr = 0;
}

#include "cpu/h8/gt913.hxx"
