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
- Fix the display
- Dump Telcon and Gemini BIOSes
- Emulate the Co-Power-88 expansion (allows PC-DOS, CP/M-86, etc. to be used)
- Probably lots of other things


*************************************************************************************************************/

#include "emu.h"
#include "includes/zorba.h"

#include "machine/zorbakbd.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m68705.h"

#include "machine/input_merger.h"
#include "machine/latch.h"
#include "machine/pit8253.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"


ADDRESS_MAP_START(zorba_state::zorba_mem)
	AM_RANGE( 0x0000, 0x3fff ) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE( 0x4000, 0xffff ) AM_RAM
ADDRESS_MAP_END


ADDRESS_MAP_START(zorba_state::zorba_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("pit", pit8254_device, read, write)
	AM_RANGE(0x04, 0x04) AM_READWRITE(rom_r, rom_w)
	AM_RANGE(0x05, 0x05) AM_READWRITE(ram_r, ram_w)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("crtc", i8275_device, read, write)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("uart0", i8251_device, data_r, data_w)
	AM_RANGE(0x21, 0x21) AM_DEVREADWRITE("uart0", i8251_device, status_r, control_w)
	AM_RANGE(0x22, 0x22) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0x23, 0x23) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	AM_RANGE(0x24, 0x24) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	AM_RANGE(0x25, 0x25) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w)
	AM_RANGE(0x26, 0x26) AM_WRITE(intmask_w)
	AM_RANGE(0x30, 0x30) AM_DEVREADWRITE("dma", z80dma_device, read, write)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE("fdc", fd1793_device, read, write)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x60, 0x63) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
ADDRESS_MAP_END

namespace {

INPUT_PORTS_START( zorba )
	PORT_START("CNF")
	PORT_CONFNAME(0x01, 0x00, "Parallel Printer") PORT_CHANGED_MEMBER(DEVICE_SELF, zorba_state, printer_type, 0)
	PORT_CONFSETTING(0x00, "Centronics")
	PORT_CONFSETTING(0x01, "Prowriter")
INPUT_PORTS_END


SLOT_INTERFACE_START( zorba_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


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

GFXDECODE_START( zorba )
	GFXDECODE_ENTRY( "chargen", 0x0000, u5_charlayout, 0, 1 )
GFXDECODE_END

} // anonymous namespace


MACHINE_CONFIG_START(zorba_state::zorba)
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 24_MHz_XTAL / 6)
	MCFG_CPU_PROGRAM_MAP(zorba_mem)
	MCFG_CPU_IO_MAP(zorba_io)
	MCFG_MACHINE_RESET_OVERRIDE(zorba_state, zorba)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", i8275_device, screen_update)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", zorba)
	MCFG_PALETTE_ADD_MONOCHROME_HIGHLIGHT("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 800) // should be horizontal frequency / 16, so depends on CRTC parameters
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_INPUT_MERGER_ANY_HIGH("irq0")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(WRITELINE(zorba_state, irq_w<0>))
	MCFG_INPUT_MERGER_ANY_HIGH("irq1")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(WRITELINE(zorba_state, irq_w<1>))
	MCFG_INPUT_MERGER_ANY_HIGH("irq2")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(WRITELINE(zorba_state, irq_w<2>))

	/* devices */
	MCFG_DEVICE_ADD("dma", Z80DMA, 24_MHz_XTAL / 6)
	// busack on cpu connects to bai pin
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(zorba_state, busreq_w))  //connects to busreq on cpu
	MCFG_Z80DMA_OUT_INT_CB(DEVWRITELINE("irq0", input_merger_device, in_w<0>))
	//ba0 - not connected
	MCFG_Z80DMA_IN_MREQ_CB(READ8(zorba_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(zorba_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(zorba_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(zorba_state, io_write_byte))

	MCFG_DEVICE_ADD("uart0", I8251, 0) // U32 COM port J2
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd)) // TODO: this line has a LED attached
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(zorba_state, tx_rx_rdy_w<1>))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(zorba_state, tx_rx_rdy_w<0>))

	MCFG_DEVICE_ADD("uart1", I8251, 0) // U31 printer port J3
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("serprn", rs232_port_device, write_txd))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("serprn", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(zorba_state, tx_rx_rdy_w<3>))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(zorba_state, tx_rx_rdy_w<2>))

	MCFG_DEVICE_ADD("uart2", I8251, 0) // U30 serial keyboard J6
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("keyboard", zorba_keyboard_device, txd_w))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(zorba_state, tx_rx_rdy_w<5>))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(zorba_state, tx_rx_rdy_w<4>))

	// port A - disk select etc, beeper
	// port B - parallel interface
	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(zorba_state, pia0_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8("parprndata", output_latch_device, write))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("parprn", centronics_device, write_strobe))

	// IEEE488 interface
	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(DEVREAD8(IEEE488_TAG, ieee488_device, dio_r)) // TODO: gated with PB1
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w)) // TODO: gated with PB1
	MCFG_PIA_READPB_HANDLER(READ8(zorba_state, pia1_portb_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(zorba_state, pia1_portb_w))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, ifc_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE(IEEE488_TAG, ieee488_device, ren_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("irq1", input_merger_device, in_w<0>))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("irq1", input_merger_device, in_w<1>))

	// PIT
	MCFG_DEVICE_ADD("pit", PIT8254, 0)
	MCFG_PIT8253_CLK0(24_MHz_XTAL / 3)
	MCFG_PIT8253_CLK1(24_MHz_XTAL / 3)
	MCFG_PIT8253_CLK2(24_MHz_XTAL / 3)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(zorba_state, br1_w))
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("uart1", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart1", i8251_device, write_rxc))
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("uart2", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart2", i8251_device, write_rxc))

	// CRTC
	MCFG_DEVICE_ADD("crtc", I8275, 14.318'181_MHz_XTAL / 7)
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(zorba_state, zorba_update_chr)
	MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE("dma", z80dma_device, rdy_w))
	MCFG_I8275_IRQ_CALLBACK(DEVWRITELINE("irq0", input_merger_device, in_w<1>))
	MCFG_VIDEO_SET_SCREEN("screen")

	// Floppies
	MCFG_FD1793_ADD("fdc", 24_MHz_XTAL / 24)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("irq2", input_merger_device, in_w<0>))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("irq2", input_merger_device, in_w<1>))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", zorba_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", zorba_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	// J1 IEEE-488
	MCFG_IEEE488_BUS_ADD()
	MCFG_IEEE488_SRQ_CALLBACK(DEVWRITELINE("pia1", pia6821_device, ca2_w)) // TODO: gated with PB1 from PIA

	// J2 EIA RS232/internal modem
	// TODO: this has additional lines compared to a regular RS232 port (TxC in, RxC in, RxC out, speaker in, power)
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart0", i8251_device, write_rxd)) // TODO: this line has a LED attached
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart0", i8251_device, write_cts)) // TODO: this line has a LED attached
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart0", i8251_device, write_dsr))

	// J3 Parallel printer
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("parprndata", "parprn")
	MCFG_CENTRONICS_ADD("parprn", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("uart1", i8251_device, write_cts))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart1", i8251_device, write_dsr)) // TODO: shared with serial CTS
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(zorba_state, printer_fault_w))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(zorba_state, printer_select_w))

	// J3 Serial printer
	MCFG_RS232_PORT_ADD("serprn", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart1", i8251_device, write_rxd)) // TODO: this line has a LED attached

	// J6 TTL-level serial keyboard
	MCFG_DEVICE_ADD("keyboard", ZORBA_KEYBOARD, 0)
	MCFG_ZORBA_KEYBOARD_RXD_CB(DEVWRITELINE("uart2", i8251_device, write_rxd))

	MCFG_SOFTWARE_LIST_ADD("flop_list", "zorba")
MACHINE_CONFIG_END


//-------------------------------------------------
//  Initialise/reset
//-------------------------------------------------

DRIVER_INIT_MEMBER( zorba_state, zorba )
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(0, &main[0x0000]);
	membank("bankr0")->configure_entry(1, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);

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
}

MACHINE_RESET_MEMBER( zorba_state, zorba )
{
	m_uart2->write_cts(0); // always asserted

	m_intmask = 0x00;
	m_tx_rx_rdy = 0x00;
	m_irq = 0x00;

	m_printer_prowriter = BIT(m_config_port->read(), 0);
	m_pia0->cb1_w(m_printer_prowriter ? m_printer_select : m_printer_fault);

	m_read_bank->set_entry(1); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram

	m_maincpu->reset();
}


//-------------------------------------------------
// Memory banking control
//-------------------------------------------------

READ8_MEMBER( zorba_state::ram_r )
{
	if (!machine().side_effect_disabled())
		m_read_bank->set_entry(0);
	return 0;
}

WRITE8_MEMBER( zorba_state::ram_w )
{
	m_read_bank->set_entry(0);
}

READ8_MEMBER( zorba_state::rom_r )
{
	if (!machine().side_effect_disabled())
		m_read_bank->set_entry(1);
	return 0;
}

WRITE8_MEMBER( zorba_state::rom_w )
{
	m_read_bank->set_entry(1);
}


//-------------------------------------------------
//  Interrupt vectoring glue
//-------------------------------------------------

WRITE8_MEMBER( zorba_state::intmask_w )
{
	m_intmask = data & 0x3f; // only six lines physically present
	irq_w<3>(BIT(m_intmask & m_tx_rx_rdy, 0) | BIT(m_intmask & m_tx_rx_rdy, 1));
	irq_w<4>(BIT(m_intmask & m_tx_rx_rdy, 2) | BIT(m_intmask & m_tx_rx_rdy, 3));
	irq_w<5>(BIT(m_intmask & m_tx_rx_rdy, 4) | BIT(m_intmask & m_tx_rx_rdy, 5));
}

template <unsigned N> WRITE_LINE_MEMBER( zorba_state::tx_rx_rdy_w )
{
	m_tx_rx_rdy = (m_tx_rx_rdy & ~(1 << N)) | ((state ? 1 : 0) << N);
	irq_w<(N >> 1) + 3>(BIT(m_intmask & m_tx_rx_rdy, N & ~1) | BIT(m_intmask & m_tx_rx_rdy, N | 1));
}

template <unsigned N> WRITE_LINE_MEMBER( zorba_state::irq_w )
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

	m_maincpu->set_input_line_and_vector(
			INPUT_LINE_IRQ0,
			m_irq ? ASSERT_LINE : CLEAR_LINE,
			vector);
}


//-------------------------------------------------
//  DMA controller handlers
//-------------------------------------------------

WRITE_LINE_MEMBER( zorba_state::busreq_w )
{
// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_dma->bai_w(state); // tell dma that bus has been granted
}

READ8_MEMBER(zorba_state::memory_read_byte)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER(zorba_state::memory_write_byte)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

READ8_MEMBER(zorba_state::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(zorba_state::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);

	if (offset == 0x10)
		m_crtc->dack_w(space, 0, data);
	else
		prog_space.write_byte(offset, data);
}


//-------------------------------------------------
//  PIT handlers
//-------------------------------------------------

WRITE_LINE_MEMBER( zorba_state::br1_w )
{
	// TODO: these can be jumpered to inputs from J2 so a modem can generate Baud rates
	// TODO: receive clock is exposed on J2 for external devices without Baud rate generators
	m_uart0->write_txc(state);
	m_uart0->write_rxc(state);
}


//-------------------------------------------------
//  PIA handlers
//-------------------------------------------------

WRITE8_MEMBER( zorba_state::pia0_porta_w )
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

READ8_MEMBER( zorba_state::pia1_portb_r )
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

WRITE8_MEMBER( zorba_state::pia1_portb_w )
{
	// 0  DIO direction
	// 1  NDAC/NRFD data direction, SRQ gate
	// 2  EOI/DAV data direction
	// 3  EOI   gated with PB2 (active low)
	// 4  ATN
	// 5  DAV   gated with PB2 (active low)
	// 6  NDAC  gated with PB1 (active low)
	// 7  NRFD  gated with PB1 (active low)

	m_ieee->eoi_w(BIT(data, 3) & BIT(~data, 2));
	m_ieee->atn_w(BIT(data, 4));
	m_ieee->dav_w(BIT(data, 5) & BIT(~data, 2));
	m_ieee->ndac_w(BIT(data, 6) & BIT(~data, 1));
	m_ieee->nrfd_w(BIT(data, 7) & BIT(~data, 1));
}


//-------------------------------------------------
//  Video
//-------------------------------------------------

I8275_DRAW_CHARACTER_MEMBER( zorba_state::zorba_update_chr )
{
	int i;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	uint8_t gfx = m_p_chargen[(linecount & 15) + (charcode << 4) + ((gpa & 1) << 11)];

	if (vsp)
		gfx = 0;

	if (lten)
		gfx = 0xff;

	if (rvv)
		gfx ^= 0xff;

	for(i=0;i<8;i++)
		bitmap.pix32(y, x + 7 - i) = palette[BIT(gfx, i) ? (hlgt ? 2 : 1) : 0];
}


//-------------------------------------------------
//  Printer port glue
//-------------------------------------------------

WRITE_LINE_MEMBER( zorba_state::printer_fault_w )
{
	// connects to CB1 for Centronics
	m_printer_fault = state;
	if (!m_printer_prowriter)
		m_pia0->cb1_w(state);
}

WRITE_LINE_MEMBER( zorba_state::printer_select_w )
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
	ROM_REGION( 0x14000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "780000.u47", 0x10000, 0x1000, CRC(6d58f2c5) SHA1(7763f08c801cd36e5a761c6dc9f30a50b3bc482d) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "773000.u5", 0x0000, 0x1000, CRC(d0a2f8fc) SHA1(29aee7ee657778c46e9800abd4955e6d4b33ef68) )

	ROM_REGION( 0x60, "proms", 0 )
	ROM_LOAD( "74ls288.u37", 0x0000, 0x0020, CRC(0a67edd6) SHA1(c1ece8978a3a061e0130d43907fa63a71e75e75d) BAD_DUMP ) // looks like interrupt vector PROM dumped with scrambled lines
	ROM_LOAD( "74ls288.u38", 0x0020, 0x0020, CRC(5ec93ea7) SHA1(3a84c098474b05d5cbe1939a3e15f66d06470581) BAD_DUMP ) // looks like bad dump of address decode PROM
	ROM_LOAD( "74ls288.u77", 0x0040, 0x0020, CRC(946e03b0) SHA1(24240bdd7bdf507a5b51628fb36ad1266fc53a28) BAD_DUMP ) // looks like bad dump of address decode PROM
ROM_END

COMP( 1984?, zorba, 0, 0, zorba, zorba, zorba_state, zorba, "Modular Micros", "Zorba (Modular Micros)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// Undumped versions (see startup screen notes at top of file)
// COMP( 1983, zorbat, zorba, 0, zorba, zorba, zorba_state, zorba, "Telcon Industries",  "Zorba (Telcon Industries)",  MACHINE_NOT_WORKING )
// COMP( 1984, zorbag, zorba, 0, zorba, zorba, zorba_state, zorba, "Gemini Electronics", "Zorba (Gemini Electronics)", MACHINE_NOT_WORKING )
