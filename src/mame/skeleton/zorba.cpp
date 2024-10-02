// license:BSD-3-Clause
// copyright-holders:Robbbert, Vas Crabb
/************************************************************************************************************

Telcon Industries/Modular Micros/Gemini Electronics Zorba
http://www.zorba.z80.de

2013-08-25 Skeleton
2015-02-20 Boots from floppy, is now usable.

This was one of the last CP/M-based systems, already out of date when it was released.
Because it doesn't use the standard Z80 peripherals, it uses a homebrew interrupt controller to make use
of the Z80's IM2.

The keyboard is an intelligent serial device like the Kaypro's keyboard. They even have the same plug,
and might be swappable. Need a schematic.

Instead of using a daisy chain, the IM2 vectors are calculated by a prom (u77). Unfortunately, the prom
contents make no sense at all (mostly FF), so the vectors for IRQ0 and IRQ2 are hard-coded. Other IRQ
vectors are not used as yet.

Three companies are known to have sold the Zorba over its lifetime: Telcon Industries, Modular Micros
(a subsidiary of Modular Computers (ModComp)), and Gemini Electronics. 7-inch and 9-inch models were
available from Telcon and Modular Micros, while Gemini exclusively sold the 9-inch version. The ROM dumps
currently used in this emulation originate from a Modular Micros Zorba.

The two versions of the Zorba were sold by Modular Micros were:
- Zorba 7: 7" CRT, 2 410K floppies, 22 lbs, $1595
- Zorba 2000: 9" CRT, 2 820K floppies, 10M HD optional, 25 lbs, ~$2000

The 7-inch version has the screen on the left, the floppy drives on the right, and a Zorba logo on the
far right; on the 9-inch version this arrangement is reversed and the logo is removed.

The startup screen varies across each company:
- Telcon: "TELCON ZORBA" graphical logo
- Modular Micros: "ZORBA" graphical logo with "MODULAR MICROS, INC." below in normal text
- Gemini: "GEMINI ZORBA" graphical logo

Status:
- Boots up, and the keyboard works

ToDo:
- Need software that does more than plain text (such as games)
- Add masking feature (only used for the UARTs)
- Connect devices to the above hardware
- Dump Telcon and Gemini BIOSes
- Emulate the Co-Power-88 expansion (allows PC-DOS, CP/M-86, etc. to be used)
- Probably lots of other things


*************************************************************************************************************/

#include "emu.h"

#include "zorbakbd.h"

#include "bus/centronics/ctronics.h"
#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m68705.h"

#include "imagedev/floppy.h"

#include "machine/6821pia.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/output_latch.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"

#include "sound/beep.h"

#include "video/i8275.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class zorba_state : public driver_device
{
public:
	zorba_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_config_port(*this, "CNF")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_p_chargen(*this, "chargen")
		, m_maincpu(*this, "maincpu")
		, m_dma(*this, "dma")
		, m_uart0(*this, "uart0")
		, m_uart1(*this, "uart1")
		, m_uart2(*this, "uart2")
		, m_pia0(*this, "pia0")
		, m_pia1(*this, "pia1")
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_beep(*this, "beeper")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_ieee(*this, IEEE488_TAG)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(printer_type);
	void zorba(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void zorba_io(address_map &map) ATTR_COLD;
	void zorba_mem(address_map &map) ATTR_COLD;

	// Memory banking control
	uint8_t ram_r();
	void ram_w(uint8_t data);
	uint8_t rom_r();
	void rom_w(uint8_t data);

	// Interrupt vectoring glue
	void intmask_w(uint8_t data);
	template <unsigned N> void tx_rx_rdy_w(int state);
	template <unsigned N> void irq_w(int state);

	// DMA controller handlers
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);

	// PIT handlers
	void br1_w(int state);

	// PIA handlers
	void pia0_porta_w(uint8_t data);
	uint8_t pia1_portb_r();
	void pia1_portb_w(uint8_t data);

	// Video
	I8275_DRAW_CHARACTER_MEMBER(zorba_update_chr);

	// Printer port glue
	void printer_fault_w(int state);
	void printer_select_w(int state);

	required_ioport                     m_config_port;

	required_region_ptr<u8>             m_rom;
	required_shared_ptr<u8>             m_ram;
	required_memory_bank                m_bank1;
	required_region_ptr<uint8_t>        m_p_chargen;

	required_device<z80_device>         m_maincpu;
	required_device<z80dma_device>      m_dma;
	required_device<i8251_device>       m_uart0;
	required_device<i8251_device>       m_uart1;
	required_device<i8251_device>       m_uart2;
	required_device<pia6821_device>     m_pia0;
	required_device<pia6821_device>     m_pia1;

	required_device<palette_device>     m_palette;
	required_device<i8275_device>       m_crtc;

	required_device<beep_device>        m_beep;

	required_device<fd1793_device>      m_fdc;
	required_device<floppy_connector>   m_floppy0;
	required_device<floppy_connector>   m_floppy1;

	required_device<ieee488_device>     m_ieee;

	uint8_t m_intmask = 0U;
	uint8_t m_tx_rx_rdy = 0U;
	uint8_t m_irq = 0U;

	bool    m_printer_prowriter = false;
	int     m_printer_fault = 0;
	int     m_printer_select = 0;

	uint8_t m_term_data = 0U;
};

void zorba_state::zorba_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0x0000, 0x3fff).bankr("bank1");
}


void zorba_state::zorba_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x04, 0x04).rw(FUNC(zorba_state::rom_r), FUNC(zorba_state::rom_w));
	map(0x05, 0x05).rw(FUNC(zorba_state::ram_r), FUNC(zorba_state::ram_w));
	map(0x10, 0x11).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x20, 0x21).rw(m_uart0, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x22, 0x23).rw(m_uart1, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x24, 0x25).rw(m_uart2, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x26, 0x26).w(FUNC(zorba_state::intmask_w));
	map(0x30, 0x30).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x40, 0x43).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x50, 0x53).rw(m_pia0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x60, 0x63).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}

namespace {

INPUT_PORTS_START( zorba )
	PORT_START("CNF")
	PORT_CONFNAME(0x01, 0x00, "Parallel Printer") PORT_CHANGED_MEMBER(DEVICE_SELF, zorba_state, printer_type, 0)
	PORT_CONFSETTING(0x00, "Centronics")
	PORT_CONFSETTING(0x01, "Prowriter")
INPUT_PORTS_END


void zorba_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}


// F4 Character Displayer
const gfx_layout u5_charlayout =
{
	8, 11,                  // 8 x 11 characters
	256,                    // 256 characters
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8 },
	8*16                    // every char takes 16 bytes
};

GFXDECODE_START( gfx_zorba )
	GFXDECODE_ENTRY( "chargen", 0x0000, u5_charlayout, 0, 1 )
GFXDECODE_END

} // anonymous namespace


void zorba_state::zorba(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 24_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &zorba_state::zorba_mem);
	m_maincpu->set_addrmap(AS_IO, &zorba_state::zorba_io);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(50);
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, gfx_zorba);
	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 800).add_route(ALL_OUTPUTS, "mono", 1.00); // should be horizontal frequency / 16, so depends on CRTC parameters

	INPUT_MERGER_ANY_HIGH(config, "irq0").output_handler().set(FUNC(zorba_state::irq_w<0>));
	INPUT_MERGER_ANY_HIGH(config, "irq1").output_handler().set(FUNC(zorba_state::irq_w<1>));
	INPUT_MERGER_ANY_HIGH(config, "irq2").output_handler().set(FUNC(zorba_state::irq_w<2>));

	/* devices */
	Z80DMA(config, m_dma, 24_MHz_XTAL / 6);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set("irq0", FUNC(input_merger_device::in_w<0>));
	//ba0 - not connected
	m_dma->in_mreq_callback().set(FUNC(zorba_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(zorba_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(zorba_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(zorba_state::io_write_byte));

	I8251(config, m_uart0, 24_MHz_XTAL / 12); // U32 COM port J2
	m_uart0->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd)); // TODO: this line has a LED attached
	m_uart0->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart0->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_uart0->rxrdy_handler().set(FUNC(zorba_state::tx_rx_rdy_w<1>));
	m_uart0->txrdy_handler().set(FUNC(zorba_state::tx_rx_rdy_w<0>));

	I8251(config, m_uart1, 24_MHz_XTAL / 12); // U31 printer port J3
	m_uart1->txd_handler().set("serprn", FUNC(rs232_port_device::write_txd));
	m_uart1->rts_handler().set("serprn", FUNC(rs232_port_device::write_rts));
	m_uart1->rxrdy_handler().set(FUNC(zorba_state::tx_rx_rdy_w<3>));
	m_uart1->txrdy_handler().set(FUNC(zorba_state::tx_rx_rdy_w<2>));

	I8251(config, m_uart2, 24_MHz_XTAL / 12); // U30 serial keyboard J6
	m_uart2->txd_handler().set("keyboard", FUNC(zorba_keyboard_device::txd_w));
	m_uart2->rxrdy_handler().set(FUNC(zorba_state::tx_rx_rdy_w<5>));
	m_uart2->txrdy_handler().set(FUNC(zorba_state::tx_rx_rdy_w<4>));

	// port A - disk select etc, beeper
	// port B - parallel interface
	PIA6821(config, m_pia0);
	m_pia0->writepa_handler().set(FUNC(zorba_state::pia0_porta_w));
	m_pia0->writepb_handler().set("parprndata", FUNC(output_latch_device::write));
	m_pia0->cb2_handler().set("parprn", FUNC(centronics_device::write_strobe));

	// IEEE488 interface
	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set(m_ieee, FUNC(ieee488_device::dio_r)); // TODO: gated with PB1
	m_pia1->writepa_handler().set(m_ieee, FUNC(ieee488_device::host_dio_w)); // TODO: gated with PB1
	m_pia1->readpb_handler().set(FUNC(zorba_state::pia1_portb_r));
	m_pia1->writepb_handler().set(FUNC(zorba_state::pia1_portb_w));
	m_pia1->ca2_handler().set(m_ieee, FUNC(ieee488_device::host_ifc_w));
	m_pia1->cb2_handler().set(m_ieee, FUNC(ieee488_device::host_ren_w));
	m_pia1->irqa_handler().set("irq1", FUNC(input_merger_device::in_w<0>));
	m_pia1->irqb_handler().set("irq1", FUNC(input_merger_device::in_w<1>));

	// PIT
	pit8254_device &pit(PIT8254(config, "pit", 0));
	pit.set_clk<0>(24_MHz_XTAL / 3);
	pit.set_clk<1>(24_MHz_XTAL / 3);
	pit.set_clk<2>(24_MHz_XTAL / 3);
	pit.out_handler<0>().set(FUNC(zorba_state::br1_w));
	pit.out_handler<1>().set(m_uart1, FUNC(i8251_device::write_txc));
	pit.out_handler<1>().append(m_uart1, FUNC(i8251_device::write_rxc));
	pit.out_handler<2>().set(m_uart2, FUNC(i8251_device::write_txc));
	pit.out_handler<2>().append(m_uart2, FUNC(i8251_device::write_rxc));

	// CRTC
	I8275(config, m_crtc, 14.318'181_MHz_XTAL / 8); // TODO: character clock divider is 9 during HRTC
	m_crtc->set_character_width(8);
	m_crtc->set_refresh_hack(true);
	m_crtc->set_display_callback(FUNC(zorba_state::zorba_update_chr));
	m_crtc->drq_wr_callback().set(m_dma, FUNC(z80dma_device::rdy_w));
	m_crtc->irq_wr_callback().set("irq0", FUNC(input_merger_device::in_w<1>));
	m_crtc->set_screen("screen");

	// Floppies
	FD1793(config, m_fdc, 24_MHz_XTAL / 24);
	m_fdc->intrq_wr_callback().set("irq2", FUNC(input_merger_device::in_w<0>));
	m_fdc->drq_wr_callback().set("irq2", FUNC(input_merger_device::in_w<1>));
	FLOPPY_CONNECTOR(config, m_floppy0, zorba_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, zorba_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// J1 IEEE-488
	IEEE488(config, m_ieee);
	m_ieee->srq_callback().set(m_pia1, FUNC(pia6821_device::ca2_w)); // TODO: gated with PB1 from PIA

	// J2 EIA RS232/internal modem
	// TODO: this has additional lines compared to a regular RS232 port (TxC in, RxC in, RxC out, speaker in, power)
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart0, FUNC(i8251_device::write_rxd)); // TODO: this line has a LED attached
	rs232.cts_handler().set(m_uart0, FUNC(i8251_device::write_cts)); // TODO: this line has a LED attached
	rs232.dsr_handler().set(m_uart0, FUNC(i8251_device::write_dsr));

	// J3 Parallel printer
	centronics_device &parprn(CENTRONICS(config, "parprn", centronics_devices, "printer"));
	parprn.busy_handler().set(m_uart1, FUNC(i8251_device::write_cts));
	parprn.busy_handler().append(m_uart1, FUNC(i8251_device::write_dsr)); // TODO: shared with serial CTS
	parprn.fault_handler().set(FUNC(zorba_state::printer_fault_w));
	parprn.select_handler().set(FUNC(zorba_state::printer_select_w));

	output_latch_device &parprndata(OUTPUT_LATCH(config, "parprndata"));
	parprn.set_output_latch(parprndata);

	// J3 Serial printer
	rs232_port_device &serprn(RS232_PORT(config, "serprn", default_rs232_devices, nullptr));
	serprn.rxd_handler().set(m_uart1, FUNC(i8251_device::write_rxd)); // TODO: this line has a LED attached

	// J6 TTL-level serial keyboard
	ZORBA_KEYBOARD(config, "keyboard").rxd_cb().set(m_uart2, FUNC(i8251_device::write_rxd));

	SOFTWARE_LIST(config, "flop_list").set_original("zorba");
}


//-------------------------------------------------
//  Initialise/reset
//-------------------------------------------------

void zorba_state::machine_start()
{
	save_item(NAME(m_intmask));
	save_item(NAME(m_tx_rx_rdy));
	save_item(NAME(m_irq));

	save_item(NAME(m_printer_prowriter));
	save_item(NAME(m_printer_fault));
	save_item(NAME(m_printer_select));

	save_item(NAME(m_term_data));

	m_printer_prowriter = false;
	m_printer_fault = 0;
	m_printer_select = 0;
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
}

void zorba_state::machine_reset()
{
	m_uart2->write_cts(0); // always asserted

	m_intmask = 0x00;
	m_tx_rx_rdy = 0x00;
	m_irq = 0x00;

	m_printer_prowriter = BIT(m_config_port->read(), 0);
	m_pia0->cb1_w(m_printer_prowriter ? m_printer_select : m_printer_fault);

	m_bank1->set_entry(1);

	m_maincpu->reset();
}


//-------------------------------------------------
// Memory banking control
//-------------------------------------------------

uint8_t zorba_state::ram_r()
{
	if (!machine().side_effects_disabled())
		m_bank1->set_entry(0);
	return 0;
}

void zorba_state::ram_w(uint8_t data)
{
	m_bank1->set_entry(0);
}

uint8_t zorba_state::rom_r()
{
	if (!machine().side_effects_disabled())
		m_bank1->set_entry(1);
	return 0;
}

void zorba_state::rom_w(uint8_t data)
{
	m_bank1->set_entry(1);
}


//-------------------------------------------------
//  Interrupt vectoring glue
//-------------------------------------------------

void zorba_state::intmask_w(uint8_t data)
{
	m_intmask = data & 0x3f; // only six lines physically present
	irq_w<3>(BIT(m_intmask & m_tx_rx_rdy, 0) | BIT(m_intmask & m_tx_rx_rdy, 1));
	irq_w<4>(BIT(m_intmask & m_tx_rx_rdy, 2) | BIT(m_intmask & m_tx_rx_rdy, 3));
	irq_w<5>(BIT(m_intmask & m_tx_rx_rdy, 4) | BIT(m_intmask & m_tx_rx_rdy, 5));
}

template <unsigned N> void zorba_state::tx_rx_rdy_w(int state)
{
	m_tx_rx_rdy = (m_tx_rx_rdy & ~(1 << N)) | ((state ? 1 : 0) << N);
	irq_w<(N >> 1) + 3>(BIT(m_intmask & m_tx_rx_rdy, N & ~1) | BIT(m_intmask & m_tx_rx_rdy, N | 1));
}

template <unsigned N> void zorba_state::irq_w(int state)
{
	m_irq = (m_irq & ~(1 << N)) | ((state ? 1 : 0) << N);

	// TODO: get a good dump of the interrupt vector PROM and get rid of this hack
	// There are six interrupt sources:
	// * IRQ0: DMAC/CRTC shared
	// * IRQ1: IEEE488 (PIA IRQA/IRQB shared)
	// * IRQ2: FDC INTRQ/DRQ shared
	// * IRQ3: RS232 TXRDY/RXRDY selectable
	// * IRQ4: Serial printer TXRDY/RXRDY selectable
	// * IRQ5: Keyboard TXRDY/RXRDY selectable
	// Valid vectors are 0x80, 0x84, 0x88, 0x8c, 0x90 and 0x94
	// Floppy needs to be on 0x80, and RS232 is apparently on 0x88
	// Everything else, including priority, is a complete guess
	u8 vector;
	if (BIT(m_irq, 2))      vector = 0x80;
	else if (BIT(m_irq, 3)) vector = 0x88;
	else                    vector = 0x84; // very wrong, need test cases for other things

	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, m_irq ? ASSERT_LINE : CLEAR_LINE, vector); // Z80
}


//-------------------------------------------------
//  DMA controller handlers
//-------------------------------------------------

uint8_t zorba_state::memory_read_byte(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void zorba_state::memory_write_byte(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

uint8_t zorba_state::io_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

void zorba_state::io_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_IO);

	if (offset == 0x10)
		m_crtc->dack_w(data);
	else
		prog_space.write_byte(offset, data);
}


//-------------------------------------------------
//  PIT handlers
//-------------------------------------------------

void zorba_state::br1_w(int state)
{
	// TODO: these can be jumpered to inputs from J2 so a modem can generate Baud rates
	// TODO: receive clock is exposed on J2 for external devices without Baud rate generators
	m_uart0->write_txc(state);
	m_uart0->write_rxc(state);
}


//-------------------------------------------------
//  PIA handlers
//-------------------------------------------------

void zorba_state::pia0_porta_w(uint8_t data)
{
	m_beep->set_state(BIT(data, 7));
	m_fdc->dden_w(BIT(data, 6));

	floppy_image_device *floppy = nullptr;
	if (!BIT(data, 0)) floppy = m_floppy0->get_device();
	if (!BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(!BIT(data, 5));

	m_floppy0->get_device()->mon_w(BIT(data, 4));
	m_floppy1->get_device()->mon_w(BIT(data, 4));
}

uint8_t zorba_state::pia1_portb_r()
{
	// 0  (output only)
	// 1  (output only)
	// 2  (output only)
	// 3  EOI   gated with PB2 (active high)
	// 4  (output only)
	// 5  DAV   gated with PB2 (active high)
	// 6  NDAC  gated with PB1 (active high)
	// 7  NRFD  gated with PB1 (active high)

	const uint8_t outputs(m_pia0->b_output());
	return
			(m_ieee->eoi_r() << 3) |
			(m_ieee->dav_r() << 5) |
			(m_ieee->ndac_r() << 6) |
			(m_ieee->nrfd_r() << 7) |
			(BIT(outputs, 1) ? 0x00 : 0xc0) |
			(BIT(outputs, 2) ? 0x00 : 0x28);
}

void zorba_state::pia1_portb_w(uint8_t data)
{
	// 0  DIO direction
	// 1  NDAC/NRFD data direction, SRQ gate
	// 2  EOI/DAV data direction
	// 3  EOI   gated with PB2 (active low)
	// 4  ATN
	// 5  DAV   gated with PB2 (active low)
	// 6  NDAC  gated with PB1 (active low)
	// 7  NRFD  gated with PB1 (active low)

	m_ieee->host_eoi_w(BIT(data, 3) & BIT(~data, 2));
	m_ieee->host_atn_w(BIT(data, 4));
	m_ieee->host_dav_w(BIT(data, 5) & BIT(~data, 2));
	m_ieee->host_ndac_w(BIT(data, 6) & BIT(~data, 1));
	m_ieee->host_nrfd_w(BIT(data, 7) & BIT(~data, 1));
}


//-------------------------------------------------
//  Video
//-------------------------------------------------

I8275_DRAW_CHARACTER_MEMBER( zorba_state::zorba_update_chr )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	using namespace i8275_attributes;
	uint8_t gfx = m_p_chargen[(linecount & 15) + (charcode << 4) + (BIT(attrcode, GPA0) ? 0x800 : 0)];

	if (BIT(attrcode, RVV))
		gfx ^= 0xff;

	// VSP actually overrides reverse video here
	if (BIT(attrcode, VSP))
		gfx = 0;

	if (BIT(attrcode, LTEN))
		gfx = 0xff;

	bool hlgt = BIT(attrcode, HLGT);
	for (int i = 0; i < 8; i++)
		bitmap.pix(y, x + 7 - i) = palette[BIT(gfx, i) ? (hlgt ? 2 : 1) : 0];
}


//-------------------------------------------------
//  Printer port glue
//-------------------------------------------------

void zorba_state::printer_fault_w(int state)
{
	// connects to CB1 for Centronics
	m_printer_fault = state;
	if (!m_printer_prowriter)
		m_pia0->cb1_w(state);
}

void zorba_state::printer_select_w(int state)
{
	// connects to CB1 for Prowriter
	m_printer_select = state;
	if (m_printer_prowriter)
		m_pia0->cb1_w(state);
}

INPUT_CHANGED_MEMBER( zorba_state::printer_type )
{
	m_printer_prowriter = newval;
	m_pia0->cb1_w(m_printer_prowriter ? m_printer_select : m_printer_fault);
}


ROM_START( zorba )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "780000.u47", 0x0000, 0x1000, CRC(6d58f2c5) SHA1(7763f08c801cd36e5a761c6dc9f30a50b3bc482d) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "773000.u5", 0x0000, 0x1000, CRC(d0a2f8fc) SHA1(29aee7ee657778c46e9800abd4955e6d4b33ef68) )

	ROM_REGION( 0x60, "proms", 0 )
	ROM_LOAD( "74ls288.u37", 0x0000, 0x0020, CRC(0a67edd6) SHA1(c1ece8978a3a061e0130d43907fa63a71e75e75d) BAD_DUMP ) // looks like interrupt vector PROM dumped with scrambled lines
	ROM_LOAD( "74ls288.u38", 0x0020, 0x0020, CRC(5ec93ea7) SHA1(3a84c098474b05d5cbe1939a3e15f66d06470581) BAD_DUMP ) // looks like bad dump of address decode PROM
	ROM_LOAD( "74ls288.u77", 0x0040, 0x0020, CRC(946e03b0) SHA1(24240bdd7bdf507a5b51628fb36ad1266fc53a28) BAD_DUMP ) // looks like bad dump of address decode PROM
ROM_END

} // anonymous namespace

COMP( 1984?, zorba, 0, 0, zorba, zorba, zorba_state, empty_init, "Modular Micros", "Zorba (Modular Micros)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// Undumped versions (see startup screen notes at top of file)
// COMP( 1983, zorbat, zorba, 0, zorba, zorba, zorba_state, empty_init, "Telcon Industries",  "Zorba (Telcon Industries)",  MACHINE_NOT_WORKING )
// COMP( 1984, zorbag, zorba, 0, zorba, zorba, zorba_state, empty_init, "Gemini Electronics", "Zorba (Gemini Electronics)", MACHINE_NOT_WORKING )
