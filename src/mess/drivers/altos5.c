// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Altos 5-15

    Boots, terminal works, memory banking works.

    Error when reading floppy disk (bdos error)

    ToDo:
    - Get floppy to read the disk (only ones found are .TD0 format)
    - Further work once the floppy is fixed

****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/wd_fdc.h"


class altos5_state : public driver_device
{
public:
	altos5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio0(*this, "z80pio_0"),
		m_pio1(*this, "z80pio_1"),
		m_dart(*this, "z80dart"),
		m_sio (*this, "z80sio"),
		m_dma (*this, "z80dma"),
		m_ctc (*this, "z80ctc"),
		m_fdc (*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1")
	{ }

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_READ8_MEMBER(port09_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port09_w);
	DECLARE_WRITE8_MEMBER(port14_w);
	DECLARE_DRIVER_INIT(altos5);
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
private:
	UINT8 m_port08;
	UINT8 m_port09;
	UINT8 *m_p_prom;
	bool m_ipl;
	offs_t m_curr_bank;
	floppy_image_device *m_floppy;
	UINT8 convert(offs_t offset, bool state);
	void setup_banks(UINT8 source);
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio0;
	required_device<z80pio_device> m_pio1;
	required_device<z80dart_device> m_dart;
	required_device<z80sio0_device> m_sio;
	required_device<z80dma_device> m_dma;
	required_device<z80ctc_device> m_ctc;
	required_device<fd1797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};

static ADDRESS_MAP_START(altos5_mem, AS_PROGRAM, 8, altos5_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE( 0x1000, 0x1fff ) AM_READ_BANK("bankr1") AM_WRITE_BANK("bankw1")
	AM_RANGE( 0x2000, 0x2fff ) AM_READ_BANK("bankr2") AM_WRITE_BANK("bankw2")
	AM_RANGE( 0x3000, 0x3fff ) AM_READ_BANK("bankr3") AM_WRITE_BANK("bankw3")
	AM_RANGE( 0x4000, 0x4fff ) AM_READ_BANK("bankr4") AM_WRITE_BANK("bankw4")
	AM_RANGE( 0x5000, 0x5fff ) AM_READ_BANK("bankr5") AM_WRITE_BANK("bankw5")
	AM_RANGE( 0x6000, 0x6fff ) AM_READ_BANK("bankr6") AM_WRITE_BANK("bankw6")
	AM_RANGE( 0x7000, 0x7fff ) AM_READ_BANK("bankr7") AM_WRITE_BANK("bankw7")
	AM_RANGE( 0x8000, 0x8fff ) AM_READ_BANK("bankr8") AM_WRITE_BANK("bankw8")
	AM_RANGE( 0x9000, 0x9fff ) AM_READ_BANK("bankr9") AM_WRITE_BANK("bankw9")
	AM_RANGE( 0xa000, 0xafff ) AM_READ_BANK("bankra") AM_WRITE_BANK("bankwa")
	AM_RANGE( 0xb000, 0xbfff ) AM_READ_BANK("bankrb") AM_WRITE_BANK("bankwb")
	AM_RANGE( 0xc000, 0xcfff ) AM_READ_BANK("bankrc") AM_WRITE_BANK("bankwc")
	AM_RANGE( 0xd000, 0xdfff ) AM_READ_BANK("bankrd") AM_WRITE_BANK("bankwd")
	AM_RANGE( 0xe000, 0xefff ) AM_READ_BANK("bankre") AM_WRITE_BANK("bankwe")
	AM_RANGE( 0xf000, 0xffff ) AM_READ_BANK("bankrf") AM_WRITE_BANK("bankwf")
ADDRESS_MAP_END

static ADDRESS_MAP_START(altos5_io, AS_IO, 8, altos5_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("z80dma", z80dma_device, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("fdc", fd1797_t, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("z80pio_0", z80pio_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("z80pio_1", z80pio_device, read, write)
	AM_RANGE(0x14, 0x17) AM_WRITE(port14_w)
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE("z80dart", z80dart_device, ba_cd_r, ba_cd_w)
	//AM_RANGE(0x20, 0x23) // Hard drive
	AM_RANGE(0x2c, 0x2f) AM_DEVREADWRITE("z80sio", z80sio0_device, ba_cd_r, ba_cd_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( altos5 )
INPUT_PORTS_END

UINT8 altos5_state::convert(offs_t offset, bool state)
{
	UINT8 data = m_p_prom[offset];

	// if IPL and /A12, point at rom
	if (!state && m_ipl && !BIT(offset, 0))
		data = 0x31;
	else
	// if WPRT point at nothing
	if (state && BIT(data, 7))
		data = 0x30;

	// mask off wprt (no longer needed)
	// normalise bank number (4x becomes 0x; 2x and 1x are already ok)
	return data & 0x3f;
}

void altos5_state::setup_banks(UINT8 source)
{
	offs_t offs,temp;
	// WPRT | template | dma bank / cpu bank

	if (source == 1) // use DMA banks only if BUSACK is asserted
		offs = ((~m_port09 & 0x20) << 3) | ((m_port09 & 0x06) << 5) | ((m_port09 & 0xc0) >> 2);
	else
		offs = ((~m_port09 & 0x20) << 3) | ((m_port09 & 0x06) << 5) | ((m_port09 & 0x18) << 1);

	temp = offs;
	if ((source == 2) || (temp != m_curr_bank))
	{
		membank("bankr0")->set_entry(convert(offs++, 0));
		membank("bankr1")->set_entry(convert(offs++, 0));
		membank("bankr2")->set_entry(convert(offs++, 0));
		membank("bankr3")->set_entry(convert(offs++, 0));
		membank("bankr4")->set_entry(convert(offs++, 0));
		membank("bankr5")->set_entry(convert(offs++, 0));
		membank("bankr6")->set_entry(convert(offs++, 0));
		membank("bankr7")->set_entry(convert(offs++, 0));
		membank("bankr8")->set_entry(convert(offs++, 0));
		membank("bankr9")->set_entry(convert(offs++, 0));
		membank("bankra")->set_entry(convert(offs++, 0));
		membank("bankrb")->set_entry(convert(offs++, 0));
		membank("bankrc")->set_entry(convert(offs++, 0));
		membank("bankrd")->set_entry(convert(offs++, 0));
		membank("bankre")->set_entry(convert(offs++, 0));
		membank("bankrf")->set_entry(convert(offs++, 0));

		offs = temp;
		membank("bankw0")->set_entry(convert(offs++, 1));
		membank("bankw1")->set_entry(convert(offs++, 1));
		membank("bankw2")->set_entry(convert(offs++, 1));
		membank("bankw3")->set_entry(convert(offs++, 1));
		membank("bankw4")->set_entry(convert(offs++, 1));
		membank("bankw5")->set_entry(convert(offs++, 1));
		membank("bankw6")->set_entry(convert(offs++, 1));
		membank("bankw7")->set_entry(convert(offs++, 1));
		membank("bankw8")->set_entry(convert(offs++, 1));
		membank("bankw9")->set_entry(convert(offs++, 1));
		membank("bankwa")->set_entry(convert(offs++, 1));
		membank("bankwb")->set_entry(convert(offs++, 1));
		membank("bankwc")->set_entry(convert(offs++, 1));
		membank("bankwd")->set_entry(convert(offs++, 1));
		membank("bankwe")->set_entry(convert(offs++, 1));
		membank("bankwf")->set_entry(convert(offs++, 1));
	}

	m_curr_bank = temp;
}

void altos5_state::machine_reset()
{
	m_curr_bank = 0;
	m_port08 = 0;
	m_port09 = 0;
	m_ipl = 1;
	setup_banks(2);
	m_maincpu->reset();
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "z80dma" },
	{ "z80pio_0" },
	{ "z80pio_1" },
	{ "z80ctc" },
	{ "z80dart" },
	{ "z80sio" },
	{ NULL }
};


// turns off IPL mode, removes boot rom from memory map
WRITE8_MEMBER( altos5_state::port14_w )
{
	m_ipl = 0;
	setup_banks(2);
}

READ8_MEMBER(altos5_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(altos5_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

READ8_MEMBER(altos5_state::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(altos5_state::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

WRITE_LINE_MEMBER( altos5_state::busreq_w )
{
// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_maincpu->set_input_line(INPUT_LINE_HALT, state); // do we need this?
	m_dma->bai_w(state); // tell dma that bus has been granted
	setup_banks(state); // adjust banking for dma or cpu
}

// baud rate generator and RTC. All inputs are 2MHz.
TIMER_DEVICE_CALLBACK_MEMBER(altos5_state::ctc_tick)
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);
	m_ctc->trg1(1);
	m_ctc->trg1(0);
	m_ctc->trg2(1);
	m_ctc->trg2(0);
	m_ctc->trg3(1);
	m_ctc->trg3(0);
}

WRITE_LINE_MEMBER( altos5_state::ctc_z1_w )
{
	m_dart->rxca_w(state);
	m_dart->txca_w(state);
	m_sio->rxca_w(state);
	m_sio->txca_w(state);
}

/*
d0: L = a HD is present
d1: L = a 2nd hard drive is present
d2: unused configuration input (must be H to skip HD boot)
d3: selected floppy is single(L) or double sided(H)
d7: IRQ from FDC
*/
READ8_MEMBER( altos5_state::port08_r )
{
	UINT8 data = m_port08 | 0x87;
	if (m_floppy)
		data |= ((UINT8)m_floppy->twosid_r() << 3); // get number of sides
	return data;
}

/*
d0: HD IRQ
*/
READ8_MEMBER( altos5_state::port09_r )
{
	return m_port09 | 0x01;
}

/*
d4: DDEN (H = double density)
d5: DS (H = drive 2)
d6: SS (H = side 2)
*/
WRITE8_MEMBER( altos5_state::port08_w )
{
	m_port08 = data & 0x70;

	m_floppy = NULL;
	if (BIT(data, 5))
		m_floppy = m_floppy1->get_device();
	else
		m_floppy = m_floppy0->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(0);
		m_floppy->ss_w(BIT(data, 6));
		m_fdc->dden_w(!BIT(data, 4));
	}
}

/*
d1, 2: Memory Map template selection (0 = diag; 1 = oasis; 2 = mp/m)
d3, 4: CPU bank select
d5:    H = Write protect of common area
d6, 7: DMA bank select (not emulated)
*/
WRITE8_MEMBER( altos5_state::port09_w )
{
	m_port09 = data;
	setup_banks(2);
}

static SLOT_INTERFACE_START( altos5_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( altos5_state::fdc_intrq_w )
{
	UINT8 data = m_port08 | ((UINT8)(state) << 7);
	m_pio0->port_a_write(data);
}

DRIVER_INIT_MEMBER( altos5_state, altos5 )
{
	m_p_prom =  memregion("proms")->base();

	UINT8 *RAM = memregion("maincpu")->base();

	membank("bankr0")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr1")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr2")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr3")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr4")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr5")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr6")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr7")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr8")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankr9")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankra")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankrb")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankrc")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankrd")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankre")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankrf")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw0")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw1")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw2")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw3")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw4")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw5")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw6")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw7")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw8")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankw9")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwa")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwb")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwc")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwd")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwe")->configure_entries(0, 50, &RAM[0], 0x1000);
	membank("bankwf")->configure_entries(0, 50, &RAM[0], 0x1000);
}

static MACHINE_CONFIG_START( altos5, altos5_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz / 2)
	MCFG_CPU_PROGRAM_MAP(altos5_mem)
	MCFG_CPU_IO_MAP(altos5_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)

	/* devices */
	MCFG_DEVICE_ADD("z80dma", Z80DMA, XTAL_8MHz / 2)
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(altos5_state, busreq_w))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	// BAO, not used
	MCFG_Z80DMA_IN_MREQ_CB(READ8(altos5_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(altos5_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(altos5_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(altos5_state, io_write_byte))

	MCFG_DEVICE_ADD("z80pio_0", Z80PIO, XTAL_8MHz / 2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(altos5_state, port08_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(altos5_state, port08_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(altos5_state, port09_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(altos5_state, port09_w))

	MCFG_DEVICE_ADD("z80pio_1", Z80PIO, XTAL_8MHz / 2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_Z80DART_ADD("z80dart", XTAL_8MHz / 2, 0, 0, 0, 0 )
	// Channel A - console #3
	// Channel B - printer
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD("z80sio", XTAL_8MHz / 2, 0, 0, 0, 0 )
	// Channel A - console #2
	// WRDY connects to (altos5_state, fdc_intrq_w)
	// Channel B - console #1
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, XTAL_8MHz / 2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("z80sio", z80dart_device, rxtxcb_w))    // SIO Ch B
	MCFG_Z80CTC_ZC1_CB(WRITELINE(altos5_state, ctc_z1_w))       // Z80DART Ch A, SIO Ch A
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("z80dart", z80dart_device, rxtxcb_w))       // Z80DART Ch B

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("z80sio", z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("z80sio", z80dart_device, dcdb_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("z80sio", z80dart_device, rib_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("z80sio", z80dart_device, ctsb_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc_tick", altos5_state, ctc_tick, attotime::from_hz(XTAL_8MHz / 4))
	MCFG_FD1797_ADD("fdc", XTAL_8MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(altos5_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("z80dma", z80dma_device, rdy_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", altos5_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", altos5_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_SOFTWARE_LIST_ADD("flop_list", "altos5")
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( altos5 ) // 00000-2FFFF = ram banks; 30000-30FFF wprt space; 31000-31FFF ROM
	ROM_REGION( 0x32000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("2732.bin",   0x31000, 0x1000, CRC(15fdc7eb) SHA1(e15bdf5d5414ad56f8c4bb84edc6f967a5f01ba9)) // bios

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("82s141.bin", 0x0000, 0x0200, CRC(35c8078c) SHA1(dce24374bfcc5d23959e2c03485d82a119c0c3c9)) // banking control
ROM_END

/* Driver */

/*   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   CLASS           INIT    COMPANY    FULLNAME       FLAGS */
COMP(1982, altos5, 0,      0,       altos5,  altos5, altos5_state,  altos5, "Altos", "Altos 5-15", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
