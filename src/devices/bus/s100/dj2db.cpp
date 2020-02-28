// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Disk Jockey 2D/B floppy controller board emulation

**********************************************************************/

/*

    TODO:

    - stall logic (read from fdc data register halts CPU until intrq/drq from FDC)

*/

#include "emu.h"
#include "dj2db.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MB8866_TAG      "14b"
#define S1602_TAG       "14d"
#define BR1941_TAG      "13d"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(S100_DJ2DB, s100_dj2db_device, "s100_sj2db", "Morrow Disk Jockey 2D/B")


//-------------------------------------------------
//  ROM( dj2db )
//-------------------------------------------------

ROM_START( dj2db )
	ROM_REGION( 0x400, "dj2db", ROMREGION_INVERT ) // 2708, inverted data outputs
	ROM_LOAD( "bv-2 f8.11d", 0x000, 0x400, CRC(b6218d0b) SHA1(e4b2ae886c0dd7717e2e02ae2e202115d8ec2def) )

	ROM_REGION( 0x220, "proms", 0 )
	ROM_LOAD( "8c-b f8.8c", 0x000, 0x200, NO_DUMP ) // 6301
	ROM_LOAD( "3d.3d", 0x200, 0x20, NO_DUMP ) // 6331
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *s100_dj2db_device::device_rom_region() const
{
	return ROM_NAME( dj2db );
}


//-------------------------------------------------
//  COM8116_INTERFACE( brg_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( s100_dj2db_device::fr_w )
{
	// S1602 RRC/TRC
}

static void s100_dj2db_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

WRITE_LINE_MEMBER( s100_dj2db_device::fdc_intrq_w )
{
	if (state) m_bus->rdy_w(CLEAR_LINE);

	switch (m_j1a->read())
	{
	case 0: m_bus->vi0_w(state); break;
	case 1: m_bus->vi1_w(state); break;
	case 2: m_bus->vi2_w(state); break;
	case 3: m_bus->vi3_w(state); break;
	case 4: m_bus->vi4_w(state); break;
	case 5: m_bus->vi5_w(state); break;
	case 6: m_bus->vi6_w(state); break;
	case 7: m_bus->vi7_w(state); break;
	case 8: m_bus->irq_w(state); break;
	}
}

WRITE_LINE_MEMBER( s100_dj2db_device::fdc_drq_w )
{
	if (state) m_bus->rdy_w(CLEAR_LINE);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void s100_dj2db_device::device_add_mconfig(machine_config &config)
{
	COM8116(config, m_dbrg, 5.0688_MHz_XTAL);
	m_dbrg->fr_handler().set(FUNC(s100_dj2db_device::fr_w));

	MB8866(config, m_fdc, 10_MHz_XTAL / 5);
	m_fdc->intrq_wr_callback().set(FUNC(s100_dj2db_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(s100_dj2db_device::fdc_drq_w));

	FLOPPY_CONNECTOR(config, m_floppy0, s100_dj2db_floppies, "8dsdd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, s100_dj2db_floppies, nullptr, floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy2, s100_dj2db_floppies, nullptr, floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy3, s100_dj2db_floppies, nullptr, floppy_image_device::default_floppy_formats);
}


//-------------------------------------------------
//  INPUT_PORTS( mm65k16s )
//-------------------------------------------------

static INPUT_PORTS_START( dj2db )
	PORT_START("SW1")
	PORT_DIPNAME( 0xf8, 0xf8, "Power-On Jump Address" ) PORT_DIPLOCATION("SW1:1,2,3,4,5") PORT_CONDITION("SW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0xf8, "F800H" )
	PORT_DIPSETTING(    0xf0, "F000H" )
	PORT_DIPSETTING(    0xe8, "E800H" )
	PORT_DIPSETTING(    0xe0, "E000H" )
	PORT_DIPSETTING(    0xd8, "D800H" )
	PORT_DIPSETTING(    0xd0, "D000H" )
	PORT_DIPSETTING(    0xc8, "C800H" )
	PORT_DIPSETTING(    0xc0, "C000H" )
	PORT_DIPSETTING(    0xb8, "B800H" )
	PORT_DIPSETTING(    0xb0, "B000H" )
	PORT_DIPSETTING(    0xa8, "A800H" )
	PORT_DIPSETTING(    0xa0, "A000H" )
	PORT_DIPSETTING(    0x98, "9800H" )
	PORT_DIPSETTING(    0x90, "9000H" )
	PORT_DIPSETTING(    0x88, "8800H" )
	PORT_DIPSETTING(    0x80, "8000H" )
	PORT_DIPSETTING(    0x78, "7800H" )
	PORT_DIPSETTING(    0x70, "7000H" )
	PORT_DIPSETTING(    0x68, "6800H" )
	PORT_DIPSETTING(    0x60, "6000H" )
	PORT_DIPSETTING(    0x58, "5800H" )
	PORT_DIPSETTING(    0x50, "5000H" )
	PORT_DIPSETTING(    0x48, "4800H" )
	PORT_DIPSETTING(    0x40, "4000H" )
	PORT_DIPSETTING(    0x38, "3800H" )
	PORT_DIPSETTING(    0x30, "3000H" )
	PORT_DIPSETTING(    0x28, "2800H" )
	PORT_DIPSETTING(    0x20, "2000H" )
	PORT_DIPSETTING(    0x18, "1800H" )
	PORT_DIPSETTING(    0x10, "1000H" )
	PORT_DIPSETTING(    0x08, "0800H" )
	PORT_DIPSETTING(    0x00, "0000H" )
	PORT_DIPNAME( 0x04, 0x04, "Phantom Line" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPNAME( 0x02, 0x02, "Bus Speed" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "2 MHz" )
	PORT_DIPSETTING(    0x00, "4/6 MHz" )
	PORT_DIPNAME( 0x01, 0x01, "Power-On Jump" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x0f, 0x0f, "Baud Rate" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x08, "110" )
	PORT_DIPSETTING(    0x0e, "1200" )
	PORT_DIPSETTING(    0x07, "9600" )
	PORT_DIPSETTING(    0x0f, "19200" )
	PORT_DIPNAME( 0x10, 0x00, "Word Length" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "8 Bits" )
	PORT_DIPSETTING(    0x00, "7 Bits" )
	PORT_DIPNAME( 0x20, 0x20, "Stop Bit Count" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "2 Stop Bits" )
	PORT_DIPSETTING(    0x00, "1 Stop Bit" )
	PORT_DIPNAME( 0x40, 0x40, "Parity" ) PORT_DIPLOCATION("SW2:7") PORT_CONDITION("SW2", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, "Even Parity" )
	PORT_DIPSETTING(    0x00, "Odd Parity" )
	PORT_DIPNAME( 0x80, 0x80, "Parity" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("J4")
	PORT_DIPNAME( 0x01, 0x00, "Power Up" )
	PORT_DIPSETTING(    0x00, "Inactive" )
	PORT_DIPSETTING(    0x01, "Active" )

	PORT_START("J2")
	PORT_DIPNAME( 0x01, 0x01, "Generate PHANTOM Signal" )
	PORT_DIPSETTING(    0x01, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )

	PORT_START("J3A")
	PORT_DIPNAME( 0xff, 0x00, "Bank Select" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x01, "DATA 0" )
	PORT_DIPSETTING(    0x02, "DATA 1" )
	PORT_DIPSETTING(    0x04, "DATA 2" )
	PORT_DIPSETTING(    0x08, "DATA 2" )
	PORT_DIPSETTING(    0x10, "DATA 3" )
	PORT_DIPSETTING(    0x20, "DATA 4" )
	PORT_DIPSETTING(    0x40, "DATA 6" )
	PORT_DIPSETTING(    0x80, "DATA 7" )

	PORT_START("J1A")
	PORT_DIPNAME( 0x0f, 0x09, "Interrupt" )
	PORT_DIPSETTING(    0x09, "Disabled")
	PORT_DIPSETTING(    0x00, "VI0")
	PORT_DIPSETTING(    0x01, "VI1")
	PORT_DIPSETTING(    0x02, "VI2")
	PORT_DIPSETTING(    0x03, "VI3")
	PORT_DIPSETTING(    0x04, "VI4")
	PORT_DIPSETTING(    0x05, "VI5")
	PORT_DIPSETTING(    0x06, "VI6")
	PORT_DIPSETTING(    0x07, "VI7")
	PORT_DIPSETTING(    0x08, "PINT")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor s100_dj2db_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dj2db );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s100_dj2db_device - constructor
//-------------------------------------------------

s100_dj2db_device::s100_dj2db_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, S100_DJ2DB, tag, owner, clock),
	device_s100_card_interface(mconfig, *this),
	m_fdc(*this, MB8866_TAG),
	m_dbrg(*this, BR1941_TAG),
	m_floppy0(*this, MB8866_TAG":0"),
	m_floppy1(*this, MB8866_TAG":1"),
	m_floppy2(*this, MB8866_TAG":2"),
	m_floppy3(*this, MB8866_TAG":3"),
	m_floppy(nullptr),
	m_rom(*this, "dj2db"),
	m_ram(*this, "ram"),
	m_j1a(*this, "J1A"),
	m_j3a(*this, "J3A"),
	m_j4(*this, "J4"),
	m_sw1(*this, "SW1"),
	m_drive(0),
	m_head(1),
	m_int_enbl(0),
	m_access_enbl(0),
	m_board_enbl(1),
	m_phantom(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_dj2db_device::device_start()
{
	// allocate memory
	m_ram.allocate(0x400);

	// state saving
	save_item(NAME(m_drive));
	save_item(NAME(m_head));
	save_item(NAME(m_int_enbl));
	save_item(NAME(m_access_enbl));
	save_item(NAME(m_board_enbl));
	save_item(NAME(m_phantom));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s100_dj2db_device::device_reset()
{
	m_board_enbl = m_j4->read();
}


//-------------------------------------------------
//  s100_smemr_r - memory read
//-------------------------------------------------

uint8_t s100_dj2db_device::s100_smemr_r(offs_t offset)
{
	uint8_t data = 0;

//  if (!(m_board_enbl & m_phantom)) return 0xff;

	if ((offset >= 0xf800) && (offset < 0xfbf8))
	{
		data = m_rom->base()[offset & 0x3ff] ^ 0xff;
	}
	else if (offset == 0xfbf8) // SERIAL IN
	{
		// UART inverted data
	}
	else if (offset == 0xfbf9) // SERIAL STAT
	{
		/*

		    bit     description

		    0       PE
		    1       OE
		    2       DR
		    3       TBRE
		    4       FE
		    5
		    6
		    7

		*/
	}
	else if (offset == 0xfbfa) // DISK STAT
	{
		/*

		    bit     description

		    0       HEAD
		    1       DATA RQ
		    2       INT RQ
		    3       _TWO SIDED
		    4       _INDEX
		    5
		    6
		    7       _READY

		*/

		data |= !m_head;
		data |= !m_fdc->drq_r() << 1;
		data |= !m_fdc->intrq_r() << 2;
		data |= (m_floppy ? m_floppy->twosid_r() : 1) << 3;
		data |= (m_floppy ? m_floppy->idx_r() : 1) << 4;
		data |= (m_floppy ? m_floppy->ready_r() : 1) << 7;
	}
	else if ((offset >= 0xfbfc) && (offset < 0xfc00))
	{
		m_bus->rdy_w(ASSERT_LINE);

		data = m_fdc->read(offset & 0x03);
	}
	else if ((offset >= 0xfc00) && (offset < 0x10000))
	{
		data = m_ram[offset & 0x3ff];
	}
	else
	{
		return 0xff;
	}

	// LS241 inverts data
	return data ^ 0xff;
}


//-------------------------------------------------
//  s100_mwrt_w - memory write
//-------------------------------------------------

void s100_dj2db_device::s100_mwrt_w(offs_t offset, uint8_t data)
{
//  if (!(m_board_enbl & m_phantom)) return;

	// LS96 inverts data
	data ^= 0xff;

	if (offset == 0xfbf8) // SERIAL OUT
	{
		// UART inverted data
	}
	else if (offset == 0xfbf9) // DRIVE SEL
	{
		/*

		    bit     description

		    0       DRIVE 1
		    1       DRIVE 2
		    2       DRIVE 3
		    3       DRIVE 4
		    4       IN USE / SIDE SELECT
		    5       INT ENBL
		    6       _ACCESS ENBL
		    7       START

		*/

		// drive select
		m_floppy = nullptr;

		if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
		if (BIT(data, 1)) m_floppy = m_floppy1->get_device();
		if (BIT(data, 2)) m_floppy = m_floppy2->get_device();
		if (BIT(data, 3)) m_floppy = m_floppy3->get_device();

		m_fdc->set_floppy(m_floppy);

		// side select
		if (m_floppy)
		{
			m_floppy->ss_w(BIT(data, 4));
			m_floppy->mon_w(0);
		}

		// interrupt enable
		m_int_enbl = BIT(data, 5);

		// access enable
		m_access_enbl = BIT(data, 6);

		// master reset
		if (!BIT(data, 7)) m_fdc->soft_reset();
	}
	else if (offset == 0xfbfa) // FUNCTION SEL
	{
		/*

		    bit     description

		    0       DOUBLE
		    1       8A SET
		    2       8A CLEAR
		    3       LEDOFF
		    4
		    5
		    6
		    7

		*/

		// density select
		m_fdc->dden_w(BIT(data, 0));
	}
	else if (offset == 0xfbfb) // WAIT ENBL
	{
		fatalerror("Z80 WAIT not supported by MAME core\n");
	}
	else if ((offset >= 0xfbfc) && (offset < 0xfc00))
	{
		m_fdc->write(offset & 0x03, data);
	}
	else if ((offset >= 0xfc00) && (offset < 0x10000))
	{
		m_ram[offset & 0x3ff] = data;
	}
}


//-------------------------------------------------
//  s100_sinp_r - I/O read
//-------------------------------------------------

uint8_t s100_dj2db_device::s100_sinp_r(offs_t offset)
{
	return 0xff;
}


//-------------------------------------------------
//  s100_sout_w - I/O write
//-------------------------------------------------

void s100_dj2db_device::s100_sout_w(offs_t offset, uint8_t data)
{
	if (offset == 0x41)
	{
		m_board_enbl = (data & m_j3a->read()) ? 1 : 0;
	}
}


//-------------------------------------------------
//  s100_phantom_w - phantom
//-------------------------------------------------

void s100_dj2db_device::s100_phantom_w(int state)
{
	if (!BIT(m_sw1->read(), 2))
	{
		m_phantom = state;
	}
	else
	{
		m_phantom = 1;
	}
}
