// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Altos 5-15

    ToDo:
    - When running MP/M, dir command crashes the system

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/wd_fdc.h"
#include "machine/clock.h"
#include "softlist.h"

#include "imagedev/snapquik.h"


class altos5_state : public driver_device
{
public:
	altos5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio0(*this, "pio0")
		, m_dma (*this, "dma")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	void init_altos5();
	void altos5(machine_config &config);

	DECLARE_QUICKLOAD_LOAD_MEMBER(altos5);

protected:
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_READ8_MEMBER(port09_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port09_w);
	DECLARE_WRITE8_MEMBER(port14_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);

	void io_map(address_map &map);
	void mem_map(address_map &map);

private:
	uint8_t m_port08;
	uint8_t m_port09;
	uint8_t *m_p_prom;
	bool m_ipl;
	offs_t m_curr_bank;
	floppy_image_device *m_floppy;
	uint8_t convert(offs_t offset, bool state);
	void setup_banks(uint8_t source);
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio0;
	required_device<z80dma_device> m_dma;
	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};


void altos5_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).bankr("bankr0").bankw("bankw0");
	map(0x1000, 0x1fff).bankr("bankr1").bankw("bankw1");
	map(0x2000, 0x2fff).bankr("bankr2").bankw("bankw2");
	map(0x3000, 0x3fff).bankr("bankr3").bankw("bankw3");
	map(0x4000, 0x4fff).bankr("bankr4").bankw("bankw4");
	map(0x5000, 0x5fff).bankr("bankr5").bankw("bankw5");
	map(0x6000, 0x6fff).bankr("bankr6").bankw("bankw6");
	map(0x7000, 0x7fff).bankr("bankr7").bankw("bankw7");
	map(0x8000, 0x8fff).bankr("bankr8").bankw("bankw8");
	map(0x9000, 0x9fff).bankr("bankr9").bankw("bankw9");
	map(0xa000, 0xafff).bankr("bankra").bankw("bankwa");
	map(0xb000, 0xbfff).bankr("bankrb").bankw("bankwb");
	map(0xc000, 0xcfff).bankr("bankrc").bankw("bankwc");
	map(0xd000, 0xdfff).bankr("bankrd").bankw("bankwd");
	map(0xe000, 0xefff).bankr("bankre").bankw("bankwe");
	map(0xf000, 0xffff).bankr("bankrf").bankw("bankwf");
}

void altos5_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x04, 0x07).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0x08, 0x0b).rw(m_pio0, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0f).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x14, 0x17).w(this, FUNC(altos5_state::port14_w));
	map(0x1c, 0x1f).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	//AM_RANGE(0x20, 0x23) // Hard drive
	map(0x2c, 0x2f).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
}

/* Input ports */
static INPUT_PORTS_START( altos5 )
INPUT_PORTS_END

uint8_t altos5_state::convert(offs_t offset, bool state)
{
	uint8_t data = m_p_prom[offset];

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

void altos5_state::setup_banks(uint8_t source)
{
	offs_t offs,temp;
	// WPRT | template | dma bank / cpu bank

	if (source == 1) // use DMA banks only if BUSACK is asserted
		offs = ((bitswap<8>(m_port09, 0, 0, 0, 5, 1, 2, 7, 6) << 4) & 0x1f0) ^ 0x100;
	else
		offs = ((bitswap<8>(m_port09, 0, 0, 0, 5, 1, 2, 4, 3) << 4) & 0x1f0) ^ 0x100;

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
	m_floppy = nullptr;
	m_maincpu->reset();
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dma" },
	{ "pio0" },
	{ "pio1" },
	{ "ctc" },
	{ "dart" },
	{ "sio" },
	{ nullptr }
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
	m_dma->bai_w(state); // tell dma that bus has been granted
	setup_banks(state); // adjust banking for dma or cpu
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
	uint8_t data = m_port08 | 0x87;
	if (m_floppy)
		data |= ((uint8_t)m_floppy->twosid_r() << 3); // get number of sides
	return data;
}

/*
d0: HD IRQ
*/
READ8_MEMBER( altos5_state::port09_r )
{
	return m_port09 & 0xfe;
}

/*
d4: DDEN (H = double density)
d5: DS (H = drive 2)
d6: SS (H = side 2)
*/
WRITE8_MEMBER( altos5_state::port08_w )
{
	m_port08 = data & 0x70;

	m_floppy = nullptr;
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


/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER( altos5_state, altos5 )
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	if (quickload_size >= 0xfd00)
		return image_init_result::FAIL;

	setup_banks(2);

	/* Avoid loading a program if CP/M-80 is not in memory */
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return image_init_result::FAIL;
	}

	/* Load image to the TPA (Transient Program Area) */
	for (uint16_t i = 0; i < quickload_size; i++)
	{
		uint8_t data;

		if (image.fread( &data, 1) != 1)
			return image_init_result::FAIL;
		prog_space.write_byte(i+0x100, data);
	}

	/* clear out command tail */
	prog_space.write_byte(0x80, 0);   prog_space.write_byte(0x81, 0);

	/* Roughly set SP basing on the BDOS position */
	m_maincpu->set_state_int(Z80_SP, 256 * prog_space.read_byte(7) - 300);
	m_maincpu->set_pc(0x100);       // start program

	return image_init_result::PASS;
}


static void altos5_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

WRITE_LINE_MEMBER( altos5_state::fdc_intrq_w )
{
	uint8_t data = m_port08 | ((uint8_t)(state) << 7);
	m_pio0->port_a_write(data);
}

void altos5_state::init_altos5()
{
	m_p_prom =  memregion("proms")->base();

	uint8_t *RAM = memregion("maincpu")->base();

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

MACHINE_CONFIG_START(altos5_state::altos5)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(8'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)
	MCFG_Z80_DAISY_CHAIN(daisy_chain_intf)

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, XTAL(8'000'000) / 4) // 2MHz
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("ctc" ,z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("ctc" ,z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("ctc" ,z80ctc_device, trg2))

	/* devices */
	MCFG_DEVICE_ADD("dma", Z80DMA, XTAL(8'000'000) / 2)
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(*this, altos5_state, busreq_w))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	// BAO, not used
	MCFG_Z80DMA_IN_MREQ_CB(READ8(*this, altos5_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(*this, altos5_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(*this, altos5_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(*this, altos5_state, io_write_byte))

	MCFG_DEVICE_ADD("pio0", Z80PIO, XTAL(8'000'000) / 2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(*this, altos5_state, port08_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(*this, altos5_state, port08_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(*this, altos5_state, port09_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(*this, altos5_state, port09_w))

	MCFG_DEVICE_ADD("pio1", Z80PIO, XTAL(8'000'000) / 2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("dart", Z80DART, XTAL(8'000'000) / 2)
	// Channel A - console #3
	// Channel B - printer
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("sio", Z80SIO, XTAL(8'000'000) / 2)
	// Channel A - console #2
	// WRDY connects to (altos5_state, fdc_intrq_w)
	// Channel B - console #1
	MCFG_Z80SIO_OUT_TXDB_CB(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(WRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(WRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL(8'000'000) / 2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE("sio", z80sio_device, rxtxcb_w))    // SIO Ch B
	MCFG_Z80CTC_ZC1_CB(WRITELINE("dart", z80dart_device, txca_w))       // Z80DART Ch A, SIO Ch A
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("dart" ,z80dart_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio" ,z80sio_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio" ,z80sio_device, rxca_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE("dart", z80dart_device, rxtxcb_w))       // Z80DART Ch B

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio", z80sio_device, ctsb_w))

	MCFG_FD1797_ADD("fdc", XTAL(8'000'000) / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, altos5_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE("dma", z80dma_device, rdy_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", altos5_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", altos5_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_SOFTWARE_LIST_ADD("flop_list", "altos5")
	MCFG_QUICKLOAD_ADD("quickload", altos5_state, altos5, "com,cpm", 3)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( altos5 ) // 00000-2FFFF = ram banks; 30000-30FFF wprt space; 31000-31FFF ROM
	ROM_REGION( 0x32000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("2732.bin",   0x31000, 0x1000, CRC(15fdc7eb) SHA1(e15bdf5d5414ad56f8c4bb84edc6f967a5f01ba9)) // bios

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("82s141.bin", 0x0000, 0x0200, CRC(35c8078c) SHA1(dce24374bfcc5d23959e2c03485d82a119c0c3c9)) // banking control
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY  FULLNAME      FLAGS */
COMP( 1982, altos5, 0,      0,      altos5,  altos5, altos5_state, init_altos5, "Altos", "Altos 5-15", MACHINE_NOT_WORKING )
