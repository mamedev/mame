// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Advanced Digital Corporation Super Slave card emulation

    Monitor commands
    Dxxxx,yyyy      = Dump memory
    Fxxxx,yyyy,zz   = Fill memory
    Gxxxx           = Goto
    Ixx             = In port
    Lxxxx           = Load
    Mxxxx,yyyy,zzzz = Move x-y to z
    Oxx,yy          = Out port
    -               = Edit memory
    .               = Edit memory

    TODO:

    - master communications
    - S-100 bus access

**********************************************************************/

#include "emu.h"
#include "superslave.h"

#include "machine/am9519.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define Z80_TAG         "u45"
#define Z80DART_0_TAG   "u14"
#define Z80DART_1_TAG   "u30"
#define Z80PIO_TAG      "u43"
#define AM9519_TAG      "u13"
#define BR1941_TAG      "u12"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(S100_SUPERSLAVE, s100_superslave_device, "s100_superslave", "ADC Super Slave")


//-------------------------------------------------
//  ROM( superslave )
//-------------------------------------------------

ROM_START( superslave )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_LOAD( "adcs6_slave_v3.2.bin", 0x000, 0x800, CRC(7f39322d) SHA1(2e9621e09378a1bb6fc05317bb58ae7865e52744) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *s100_superslave_device::device_rom_region() const
{
	return ROM_NAME( superslave );
}


//-------------------------------------------------
//  INPUT_PORTS( superslave )
//-------------------------------------------------

static INPUT_PORTS_START( superslave )
	PORT_START("SW1")
	PORT_DIPNAME( 0x7f, 0x38, "Slave Address" ) PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x38, "70H (0)" )
	PORT_DIPSETTING(    0x39, "72H (1)" )
	PORT_DIPSETTING(    0x3a, "74H (2)" )
	PORT_DIPSETTING(    0x3b, "76H (3)" )
	PORT_DIPSETTING(    0x3c, "78H (4)" )
	PORT_DIPSETTING(    0x3d, "7AH (5)" )
	PORT_DIPSETTING(    0x3e, "7CH (6)" )
	PORT_DIPSETTING(    0x3f, "7EH (7)" )
	PORT_DIPSETTING(    0x40, "80H (8)" )
	PORT_DIPSETTING(    0x41, "82H (9)" )
	PORT_DIPSETTING(    0x42, "84H (10)" )
	PORT_DIPSETTING(    0x43, "86H (11)" )
	PORT_DIPSETTING(    0x44, "88H (12)" )
	PORT_DIPSETTING(    0x45, "8AH (13)" )
	PORT_DIPSETTING(    0x46, "8CH (14)" )
	PORT_DIPSETTING(    0x47, "8EH (15)" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor s100_superslave_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( superslave );
}


//-------------------------------------------------
//  ADDRESS_MAP( mem_map )
//-------------------------------------------------

void s100_superslave_device::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(s100_superslave_device::mem_r), FUNC(s100_superslave_device::mem_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( io_map )
//-------------------------------------------------

void s100_superslave_device::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(Z80DART_0_TAG, FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x0c, 0x0f).rw(Z80DART_1_TAG, FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x10, 0x10).mirror(0x03).w(m_dbrg, FUNC(com8116_device::stt_str_w));
	map(0x14, 0x17).rw(Z80PIO_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x18, 0x18).mirror(0x02).rw(AM9519_TAG, FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x19, 0x19).mirror(0x02).rw(AM9519_TAG, FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
	map(0x1d, 0x1d).w(FUNC(s100_superslave_device::memctrl_w));
	map(0x1e, 0x1e).noprw(); // master communications
	map(0x1f, 0x1f).rw(FUNC(s100_superslave_device::status_r), FUNC(s100_superslave_device::cmd_w));
}


//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS( terminal )
//-------------------------------------------------

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  z80_daisy_config daisy_chain
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ Z80DART_0_TAG },
	{ Z80DART_1_TAG },
	{ Z80PIO_TAG },
	{ nullptr }
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void s100_superslave_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &s100_superslave_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &s100_superslave_device::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	am9519_device &am9519(AM9519(config, AM9519_TAG));
	am9519.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device &pio(Z80PIO(config, Z80PIO_TAG, XTAL(8'000'000)/2));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device &dart0(Z80DART(config, Z80DART_0_TAG, XTAL(8'000'000)/2));
	dart0.out_txda_callback().set(m_rs232[0], FUNC(rs232_port_device::write_txd));
	dart0.out_dtra_callback().set(m_rs232[0], FUNC(rs232_port_device::write_dtr));
	dart0.out_rtsa_callback().set(m_rs232[0], FUNC(rs232_port_device::write_rts));
	dart0.out_txdb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_txd));
	dart0.out_dtrb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_dtr));
	dart0.out_rtsb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_rts));
	dart0.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	RS232_PORT(config, m_rs232[0], default_rs232_devices, "terminal");
	m_rs232[0]->rxd_handler().set(Z80DART_0_TAG, FUNC(z80dart_device::rxa_w));
	m_rs232[0]->dcd_handler().set(Z80DART_0_TAG, FUNC(z80dart_device::dcda_w));
	m_rs232[0]->cts_handler().set(Z80DART_0_TAG, FUNC(z80dart_device::ctsa_w));
	m_rs232[0]->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	RS232_PORT(config, m_rs232[1], default_rs232_devices, nullptr);
	m_rs232[1]->rxd_handler().set(Z80DART_0_TAG, FUNC(z80dart_device::rxb_w));
	m_rs232[1]->dcd_handler().set(Z80DART_0_TAG, FUNC(z80dart_device::dcdb_w));
	m_rs232[1]->cts_handler().set(Z80DART_0_TAG, FUNC(z80dart_device::ctsb_w));

	z80dart_device &dart1(Z80DART(config, Z80DART_1_TAG, XTAL(8'000'000)/2));
	dart1.out_txda_callback().set(m_rs232[2], FUNC(rs232_port_device::write_txd));
	dart1.out_dtra_callback().set(m_rs232[2], FUNC(rs232_port_device::write_dtr));
	dart1.out_rtsa_callback().set(m_rs232[2], FUNC(rs232_port_device::write_rts));
	dart1.out_txdb_callback().set(m_rs232[3], FUNC(rs232_port_device::write_txd));
	dart1.out_dtrb_callback().set(m_rs232[3], FUNC(rs232_port_device::write_dtr));
	dart1.out_rtsb_callback().set(m_rs232[3], FUNC(rs232_port_device::write_rts));
	dart1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	RS232_PORT(config, m_rs232[2], default_rs232_devices, nullptr);
	m_rs232[2]->rxd_handler().set(Z80DART_1_TAG, FUNC(z80dart_device::rxa_w));
	m_rs232[2]->dcd_handler().set(Z80DART_1_TAG, FUNC(z80dart_device::dcda_w));
	m_rs232[2]->cts_handler().set(Z80DART_1_TAG, FUNC(z80dart_device::ctsa_w));

	RS232_PORT(config, m_rs232[3], default_rs232_devices, nullptr);
	m_rs232[3]->rxd_handler().set(Z80DART_1_TAG, FUNC(z80dart_device::rxb_w));
	m_rs232[3]->dcd_handler().set(Z80DART_1_TAG, FUNC(z80dart_device::dcdb_w));
	m_rs232[3]->cts_handler().set(Z80DART_1_TAG, FUNC(z80dart_device::ctsb_w));

	COM8116(config, m_dbrg, XTAL(5'068'800));
	m_dbrg->fr_handler().set(Z80DART_0_TAG, FUNC(z80dart_device::txca_w));
	m_dbrg->fr_handler().append(Z80DART_0_TAG, FUNC(z80dart_device::rxca_w));
	m_dbrg->fr_handler().append(Z80DART_1_TAG, FUNC(z80dart_device::txca_w));
	m_dbrg->fr_handler().append(Z80DART_1_TAG, FUNC(z80dart_device::rxca_w));
	m_dbrg->ft_handler().set(Z80DART_0_TAG, FUNC(z80dart_device::rxtxcb_w));
	m_dbrg->ft_handler().append(Z80DART_1_TAG, FUNC(z80dart_device::rxtxcb_w));

	RAM(config, m_ram).set_default_size("64K").set_extra_options("128K");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s100_superslave_device - constructor
//-------------------------------------------------

s100_superslave_device::s100_superslave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, S100_SUPERSLAVE, tag, owner, clock)
	, device_s100_card_interface(mconfig, *this)
	, m_maincpu(*this, Z80_TAG)
	, m_dbrg(*this, BR1941_TAG)
	, m_ram(*this, RAM_TAG)
	, m_rs232(*this, "rs232%c", 'a')
	, m_rom(*this, Z80_TAG)
	, m_memctrl(0x01)
	, m_cmd(0x01)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_superslave_device::device_start()
{
	save_item(NAME(m_memctrl));
	save_item(NAME(m_cmd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s100_superslave_device::device_reset()
{
	m_memctrl = 0x01;
	m_cmd = 0x01;
}


//-------------------------------------------------
//  mem_r -
//-------------------------------------------------

uint8_t s100_superslave_device::mem_r(offs_t offset)
{
	uint8_t data = 0;

	offs_t boundary = 0xc000 | ((m_memctrl & 0xf0) << 6);

	if ((offset < 0x1000) && BIT(m_cmd, 0))
	{
		data = m_rom->base()[offset & 0x7ff];
	}
	else if (offset < boundary)
	{
		if (BIT(m_memctrl, 0))
		{
			data = m_ram->pointer()[offset];
		}
		else if (BIT(m_memctrl, 1) && (m_ram->size() > 0x10000))
		{
			data = m_ram->pointer()[0x10000 | offset];
		}
	}
	else
	{
		data = m_ram->pointer()[offset];
	}

	return data;
}


//-------------------------------------------------
//  mem_w -
//-------------------------------------------------

void s100_superslave_device::mem_w(offs_t offset, uint8_t data)
{
	offs_t boundary = 0xc000 | ((m_memctrl & 0xf0) << 6);

	if (offset < boundary)
	{
		if (BIT(m_memctrl, 0))
		{
			m_ram->pointer()[offset] = data;
		}
		else if (BIT(m_memctrl, 1) && (m_ram->size() > 0x10000))
		{
			m_ram->pointer()[0x10000 | offset] = data;
		}
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}
}


//-------------------------------------------------
//  memctrl_w -
//-------------------------------------------------

void s100_superslave_device::memctrl_w(uint8_t data)
{
	/*

	    bit     description

	    0       Memory bank 0 on
	    1       Memory bank 1 on
	    2
	    3
	    4       Unswitched memory boundary bit 0
	    5       Unswitched memory boundary bit 1
	    6       Unswitched memory boundary bit 2
	    7       Unswitched memory boundary bit 3

	*/

	m_memctrl = data;
}


//-------------------------------------------------
//  status_r -
//-------------------------------------------------

uint8_t s100_superslave_device::status_r()
{
	/*

	    bit     description

	    0       Sense Switch (0=closed)
	    1       Parity error (1=error)
	    2       Syncerr (1=error)
	    3       Service Request
	    4       Data Set Ready 3
	    5       Data Set Ready 2
	    6       Data Set Ready 1
	    7       Data Set Ready 0

	*/

	uint8_t data = 1;

	data |= m_rs232[3]->dsr_r() << 4;
	data |= m_rs232[2]->dsr_r() << 5;
	data |= m_rs232[1]->dsr_r() << 6;
	data |= m_rs232[0]->dsr_r() << 7;

	return data;
}


//-------------------------------------------------
//  cmd_w -
//-------------------------------------------------

void s100_superslave_device::cmd_w(uint8_t data)
{
	/*

	    bit     description

	    0       Prom enable (1=enabled)
	    1       Clear Parity (1=cleared)
	    2       Clear Syncerr (1=cleared)
	    3       Service Request
	    4       Wait Protocol (0=Wait)
	    5       Command bit 5
	    6       Command bit 6
	    7       Command bit 7

	*/

	m_cmd = data;
}
