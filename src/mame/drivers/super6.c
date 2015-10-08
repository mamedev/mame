// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - floppy (cannot be implemented currently since this is another case of halting the cpu mid-instruction)
    - interrupts
    - DMA
    - peripheral interfaces

*/

#include "bus/rs232/rs232.h"
#include "includes/super6.h"


//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void super6_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	// power on jump
	if (!BIT(m_bank0, 6)) { program.install_rom(0x0000, 0x07ff, 0, 0xf800, m_rom); return; }

	// first 64KB of memory
	program.install_ram(0x0000, 0xffff, ram);

	// second 64KB of memory
	int map = (m_bank1 >> 4) & 0x07;

	switch (map)
	{
	case 0:
		if (BIT(m_bank1, 0)) program.install_ram(0x0000, 0x3fff, ram + 0x10000);
		if (BIT(m_bank1, 1)) program.install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program.install_ram(0x8000, 0xbfff, ram + 0x18000);
		if (BIT(m_bank1, 3)) program.install_ram(0xc000, 0xffff, ram + 0x1c000);
		break;

	case 1:
		if (BIT(m_bank1, 0)) program.install_ram(0x0000, 0x3fff, ram + 0x10000);
		if (BIT(m_bank1, 1)) program.install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program.install_ram(0x8000, 0xbfff, ram + 0x18000);
		if (BIT(m_bank1, 3)) program.install_ram(0xc000, 0xffff, ram + 0x0000);
		break;

	case 2:
		if (BIT(m_bank1, 0)) program.install_ram(0x0000, 0x3fff, ram + 0x10000);
		if (BIT(m_bank1, 1)) program.install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program.install_ram(0x8000, 0xbfff, ram + 0x4000);
		if (BIT(m_bank1, 3)) program.install_ram(0xc000, 0xffff, ram + 0x1c000);
		break;

	case 3:
		if (BIT(m_bank1, 0)) program.install_ram(0x0000, 0x3fff, ram + 0x10000);
		if (BIT(m_bank1, 1)) program.install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program.install_ram(0x8000, 0xbfff, ram + 0x0000);
		if (BIT(m_bank1, 3)) program.install_ram(0xc000, 0xffff, ram + 0x4000);
		break;

	case 4:
		if (BIT(m_bank1, 0)) program.install_ram(0x0000, 0x3fff, ram + 0xc000);
		if (BIT(m_bank1, 1)) program.install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program.install_ram(0x8000, 0xbfff, ram + 0x18000);
		if (BIT(m_bank1, 3)) program.install_ram(0xc000, 0xffff, ram + 0x1c000);
		break;
	}

	// bank 0 overrides
	if (BIT(m_bank0, 0)) program.install_ram(0x0000, 0x3fff, ram + 0x0000);
	if (BIT(m_bank0, 1)) program.install_ram(0x4000, 0x7fff, ram + 0x4000);
	if (BIT(m_bank0, 2)) program.install_ram(0x8000, 0xbfff, ram + 0x8000);
	if (BIT(m_bank0, 3)) program.install_ram(0xc000, 0xffff, ram + 0xc000);

	// PROM enabled
	if (!BIT(m_bank0, 5)) program.install_rom(0xf000, 0xf7ff, 0, 0x800, m_rom);
}


//-------------------------------------------------
//  s100_w - S-100 bus extended address A16-A23
//-------------------------------------------------

WRITE8_MEMBER( super6_state::s100_w )
{
	/*

	    bit     description

	    0       A16
	    1       A17
	    2       A18
	    3       A19
	    4       A20
	    5       A21
	    6       A22
	    7       A23

	*/

	m_s100 = data;
}


//-------------------------------------------------
//  bank0_w - on-board memory control port #0
//-------------------------------------------------

WRITE8_MEMBER( super6_state::bank0_w )
{
	/*

	    bit     description

	    0       memory bank 0 (0000-3fff)
	    1       memory bank 1 (4000-7fff)
	    2       memory bank 2 (8000-bfff)
	    3       memory bank 3 (c000-ffff)
	    4
	    5       PROM enabled (0=enabled, 1=disabled)
	    6       power on jump reset
	    7       parity check enable

	*/

	m_bank0 = data;

	bankswitch();
}


//-------------------------------------------------
//  bank1_w - on-board memory control port #1
//-------------------------------------------------

WRITE8_MEMBER( super6_state::bank1_w )
{
	/*

	    bit     description

	    0       memory bank 4
	    1       memory bank 5
	    2       memory bank 6
	    3       memory bank 7
	    4       map select 0
	    5       map select 1
	    6       map select 2
	    7

	*/

	m_bank1 = data;

	bankswitch();
}



//**************************************************************************
//  PERIPHERALS
//**************************************************************************

//-------------------------------------------------
//  floppy_r - FDC synchronization/drive/density
//-------------------------------------------------

READ8_MEMBER( super6_state::fdc_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       FDC INTRQ

	*/

	fatalerror("Z80 WAIT not supported by MAME core\n");
	m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);

	return !m_fdc->intrq_r() << 7;
}


//-------------------------------------------------
//  floppy_w - FDC synchronization/drive/density
//-------------------------------------------------

WRITE8_MEMBER( super6_state::fdc_w )
{
	/*

	    bit     description

	    0       disk drive select 0
	    1       disk drive select 1
	    2       head select (0=head 1, 1=head 2)
	    3       disk density (0=single, 1=double)
	    4       size select (0=8", 1=5.25")
	    5
	    6
	    7

	*/

	// disk drive select
	floppy_image_device *m_floppy = NULL;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);
	if (m_floppy) m_floppy->mon_w(0);

	// head select
	if (m_floppy) m_floppy->ss_w(BIT(data, 2));

	// disk density
	m_fdc->dden_w(!BIT(data, 3));
}


//-------------------------------------------------
//  baud_w - baud rate
//-------------------------------------------------

WRITE8_MEMBER( super6_state::baud_w )
{
	/*

	    bit     description

	    0       SIO channel A baud bit A
	    1       SIO channel A baud bit B
	    2       SIO channel A baud bit C
	    3       SIO channel A baud bit D
	    4       SIO channel B baud bit A
	    5       SIO channel B baud bit B
	    6       SIO channel B baud bit C
	    7       SIO channel B baud bit D

	*/

	m_brg->str_w(data & 0x0f);
	m_brg->stt_w(data >> 4);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( super6_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( super6_mem, AS_PROGRAM, 8, super6_state )
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( super6_io )
//-------------------------------------------------

static ADDRESS_MAP_START( super6_io, AS_IO, 8, super6_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE(Z80DART_TAG, z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE(WD2793_TAG, wd2793_t, read, write)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x03) AM_DEVREADWRITE(Z80DMA_TAG, z80dma_device, read, write)
	AM_RANGE(0x14, 0x14) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x15, 0x15) AM_READ_PORT("J7") AM_WRITE(s100_w)
	AM_RANGE(0x16, 0x16) AM_WRITE(bank0_w)
	AM_RANGE(0x17, 0x17) AM_WRITE(bank1_w)
	AM_RANGE(0x18, 0x18) AM_MIRROR(0x03) AM_WRITE(baud_w)
//  AM_RANGE(0x40, 0x40) ?
//  AM_RANGE(0xe0, 0xe7) HDC?
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( super6 )
//-------------------------------------------------

static INPUT_PORTS_START( super6 )
	PORT_START("J7")
	PORT_DIPNAME( 0x0f, 0x0f, "SIO Channel A Baud Rate" ) PORT_DIPLOCATION("J7:1,2,3,4")
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "75" )
	PORT_DIPSETTING(    0x02, "110" )
	PORT_DIPSETTING(    0x03, "134.5" )
	PORT_DIPSETTING(    0x04, "150" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x06, "600" )
	PORT_DIPSETTING(    0x07, "1200" )
	PORT_DIPSETTING(    0x08, "1800" )
	PORT_DIPSETTING(    0x09, "2000" )
	PORT_DIPSETTING(    0x0a, "2400" )
	PORT_DIPSETTING(    0x0b, "3600" )
	PORT_DIPSETTING(    0x0c, "4800" )
	PORT_DIPSETTING(    0x0d, "7200" )
	PORT_DIPSETTING(    0x0e, "9600" )
	PORT_DIPSETTING(    0x0f, "19200" )
	PORT_DIPNAME( 0x70, 0x70, "SIO Channel B Baud Rate" ) PORT_DIPLOCATION("J7:5,6,7")
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x10, "75" )
	PORT_DIPSETTING(    0x20, "110" )
	PORT_DIPSETTING(    0x30, "134.5" )
	PORT_DIPSETTING(    0x40, "150" )
	PORT_DIPSETTING(    0x50, "300" )
	PORT_DIPSETTING(    0x60, "600" )
	PORT_DIPSETTING(    0x70, "1200" )
	PORT_DIPNAME( 0x80, 0x00, "Disk Drive Type" ) PORT_DIPLOCATION("J7:8")
	PORT_DIPSETTING(    0x80, "Single Sided" )
	PORT_DIPSETTING(    0x00, "Double Sided" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( super6_state::ctc_tick )
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);
}

//-------------------------------------------------
//  Z80DMA
//-------------------------------------------------

READ8_MEMBER(super6_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(super6_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

READ8_MEMBER(super6_state::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(super6_state::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

//-------------------------------------------------
//  COM8116_INTERFACE( brg_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( super6_state::fr_w )
{
	m_dart->rxca_w(state);
	m_dart->txca_w(state);

	m_ctc->trg1(state);
}


//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

static SLOT_INTERFACE_START( super6_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_QD )
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( super6_state::fdc_intrq_w )
{
	if (state) m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);

	m_ctc->trg3(!state);
}

WRITE_LINE_MEMBER( super6_state::fdc_drq_w )
{
	if (state) m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);

	m_dma->rdy_w(state);
}


//-------------------------------------------------
//  z80_daisy_config super6_daisy_chain
//-------------------------------------------------

static const z80_daisy_config super6_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80DART_TAG },
	{ Z80PIO_TAG },
	{ NULL }
};


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( super6 )
//-------------------------------------------------

void super6_state::machine_start()
{
	// state saving
	save_item(NAME(m_s100));
	save_item(NAME(m_bank0));
	save_item(NAME(m_bank1));
}


void super6_state::machine_reset()
{
	m_bank0 = m_bank1 = 0;

	bankswitch();

	UINT8 baud = m_j7->read();

	m_brg->str_w(baud & 0x0f);
	m_brg->stt_w((baud >> 4) & 0x07);
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( super6 )
//-------------------------------------------------

static MACHINE_CONFIG_START( super6, super6_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_24MHz/4)
	MCFG_CPU_PROGRAM_MAP(super6_mem)
	MCFG_CPU_IO_MAP(super6_io)
	MCFG_CPU_CONFIG(super6_daisy_chain)

	// devices
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_24MHz/4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", super6_state, ctc_tick, attotime::from_hz(XTAL_24MHz/16))

	MCFG_DEVICE_ADD(Z80DMA_TAG, Z80DMA, XTAL_24MHz/6)
	MCFG_Z80DMA_OUT_BUSREQ_CB(INPUTLINE(Z80_TAG, INPUT_LINE_HALT))
	MCFG_Z80DMA_OUT_INT_CB(DEVWRITELINE(Z80CTC_TAG, z80ctc_device, trg2))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(super6_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(super6_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(super6_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(super6_state, io_write_byte))

	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_24MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_WD2793_ADD(WD2793_TAG, 1000000)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(super6_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(super6_state, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD(WD2793_TAG":0", super6_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2793_TAG":1", super6_floppies, NULL,    floppy_image_device::default_floppy_formats)

	MCFG_Z80DART_ADD(Z80DART_TAG, XTAL_24MHz/4, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxa_w))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxb_w))

	MCFG_DEVICE_ADD(BR1945_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(super6_state, fr_w))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxtxcb_w))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "super6")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( super6 )
//-------------------------------------------------

ROM_START( super6 )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v36" )
	ROM_SYSTEM_BIOS( 0, "v36", "ADC S6 v3.6" )
	ROMX_LOAD( "adcs6_v3.6.u29", 0x000, 0x800, CRC(386fd22a) SHA1(9c177990aa180ab93be9c4641e92ae934627e661), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v12", "Digitex Monitor v1.2a" )
	ROMX_LOAD( "digitex monitor 1.2a 6oct1983.u29", 0x000, 0x800, CRC(a4c33ce4) SHA1(46dde43ea51d295f2b3202c2d0e1883bde1a8da7), ROM_BIOS(2) )

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "pal16l8.u16", 0x000, 0x800, NO_DUMP )
	ROM_LOAD( "pal16l8.u36", 0x000, 0x800, NO_DUMP )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    INIT    COMPANY                          FULLNAME        FLAGS
COMP( 1983, super6,  0,      0,      super6,  super6, driver_device,  0,      "Advanced Digital Corporation",   "Super Six",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
