// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**************************************************************************

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

    - all

****************************************************************************/

#include "includes/superslave.h"



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( superslave_state::read )
{
	UINT8 data = 0;

	offs_t boundry = 0xc000 | ((m_memctrl & 0xf0) << 6);

	if ((offset < 0x1000) && BIT(m_cmd, 0))
	{
		data = m_rom->base()[offset & 0x7ff];
	}
	else if (offset < boundry)
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
//  write -
//-------------------------------------------------

WRITE8_MEMBER( superslave_state::write )
{
	offs_t boundry = 0xc000 | ((m_memctrl & 0xf0) << 6);

	if (offset < boundry)
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
//  baud_w -
//-------------------------------------------------

WRITE8_MEMBER( superslave_state::baud_w )
{
	m_dbrg->str_w(data & 0x0f);
	m_dbrg->stt_w(data >> 4);
}


//-------------------------------------------------
//  memctrl_w -
//-------------------------------------------------

WRITE8_MEMBER( superslave_state::memctrl_w )
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

READ8_MEMBER( superslave_state::status_r)
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

	UINT8 data = 1;

	// RS-232
	data |= m_rs232d->dsr_r() << 4;
	data |= m_rs232c->dsr_r() << 5;
	data |= m_rs232b->dsr_r() << 6;
	data |= m_rs232a->dsr_r() << 7;

	return data;
}


//-------------------------------------------------
//  cmd_w -
//-------------------------------------------------

WRITE8_MEMBER( superslave_state::cmd_w )
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



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( superslave_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( superslave_mem, AS_PROGRAM, 8, superslave_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( superslave_io )
//-------------------------------------------------

static ADDRESS_MAP_START( superslave_io, AS_IO, 8, superslave_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE(Z80DART_0_TAG, z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE(Z80DART_1_TAG, z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x03) AM_WRITE(baud_w)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x18, 0x1c) AM_DEVREADWRITE(AM9519_TAG, pic8259_device, read, write)
	AM_RANGE(0x1d, 0x1d) AM_WRITE(memctrl_w)
	AM_RANGE(0x1e, 0x1e) AM_NOP // master communications
	AM_RANGE(0x1f, 0x1f) AM_READWRITE(status_r, cmd_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

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



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  COM8116_INTERFACE( dbrg_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( superslave_state::fr_w )
{
	m_dart0->rxca_w(state);
	m_dart0->txca_w(state);
	m_dart1->rxca_w(state);
	m_dart1->txca_w(state);
}

WRITE_LINE_MEMBER( superslave_state::ft_w )
{
	m_dart0->rxtxcb_w(state);
	m_dart1->rxtxcb_w(state);
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  z80_daisy_config superslave_daisy_chain
//-------------------------------------------------

static const z80_daisy_config superslave_daisy_chain[] =
{
	{ Z80DART_0_TAG },
	{ Z80DART_1_TAG },
	{ Z80PIO_TAG },
	{ NULL }
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( superslave )
//-------------------------------------------------

void superslave_state::machine_start()
{
	// state saving
	save_item(NAME(m_memctrl));
	save_item(NAME(m_cmd));
}


void superslave_state::machine_reset()
{
	m_memctrl = 0x01;
	m_cmd = 0x01;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( superslave )
//-------------------------------------------------

static MACHINE_CONFIG_START( superslave, superslave_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(superslave_mem)
	MCFG_CPU_IO_MAP(superslave_io)
	MCFG_CPU_CONFIG(superslave_daisy_chain)

	// devices
	MCFG_PIC8259_ADD(AM9519_TAG, INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0), VCC, NULL)

	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_8MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_Z80DART_ADD(Z80DART_0_TAG, XTAL_8MHz/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_0_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80DART_0_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80DART_0_TAG, z80dart_device, ctsa_w))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_0_TAG, z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80DART_0_TAG, z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80DART_0_TAG, z80dart_device, ctsb_w))

	MCFG_Z80DART_ADD(Z80DART_1_TAG, XTAL_8MHz/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_C_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_C_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_C_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_D_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_D_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_D_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD(RS232_C_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_1_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80DART_1_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80DART_1_TAG, z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232_D_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_1_TAG, z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80DART_1_TAG, z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80DART_1_TAG, z80dart_device, ctsb_w))

	MCFG_DEVICE_ADD(BR1941_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(superslave_state, fr_w))
	MCFG_COM8116_FT_HANDLER(WRITELINE(superslave_state, ft_w))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("128K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( superslv )
//-------------------------------------------------

ROM_START( superslv )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_LOAD( "adcs6_slave_v3.2.bin", 0x000, 0x800, CRC(7f39322d) SHA1(2e9621e09378a1bb6fc05317bb58ae7865e52744) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT       CLASS          INIT    COMPANY                          FULLNAME        FLAGS
COMP( 1983, superslv, 0,      0,      superslave, superslave, driver_device, 0, "Advanced Digital Corporation", "Super Slave",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
