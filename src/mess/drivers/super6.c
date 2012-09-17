/*

    ADC Super Six SBC

    Skeleton driver

*/

/*

    TODO:

    - floppy (cannot be implemented currently since this is another case of halting the cpu mid-instruction)
    - interrupts
    - DMA
    - peripheral interfaces

*/

#include "includes/super6.h"


//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

//-------------------------------------------------
//  bankswitch -
//-------------------------------------------------

void super6_state::bankswitch()
{
	address_space *program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();
	UINT8 *rom = memregion(Z80_TAG)->base();

	// power on jump
	if (!BIT(m_bank0, 6)) {	program->install_rom(0x0000, 0x07ff, 0, 0xf800, rom); return; }

	// first 64KB of memory
	program->install_ram(0x0000, 0xffff, ram);

	// second 64KB of memory
	int map = (m_bank1 >> 4) & 0x07;

	switch (map)
	{
	case 0:
		if (BIT(m_bank1, 0)) program->install_ram(0x0000, 0x3fff, ram + 0x10000);
		if (BIT(m_bank1, 1)) program->install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program->install_ram(0x8000, 0xbfff, ram + 0x18000);
		if (BIT(m_bank1, 3)) program->install_ram(0xc000, 0xffff, ram + 0x1c000);
		break;

	case 1:
		if (BIT(m_bank1, 0)) program->install_ram(0x0000, 0x3fff, ram + 0x10000);
		if (BIT(m_bank1, 1)) program->install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program->install_ram(0x8000, 0xbfff, ram + 0x18000);
		if (BIT(m_bank1, 3)) program->install_ram(0xc000, 0xffff, ram + 0x0000);
		break;

	case 2:
		if (BIT(m_bank1, 0)) program->install_ram(0x0000, 0x3fff, ram + 0x10000);
		if (BIT(m_bank1, 1)) program->install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program->install_ram(0x8000, 0xbfff, ram + 0x4000);
		if (BIT(m_bank1, 3)) program->install_ram(0xc000, 0xffff, ram + 0x1c000);
		break;

	case 3:
		if (BIT(m_bank1, 0)) program->install_ram(0x0000, 0x3fff, ram + 0x10000);
		if (BIT(m_bank1, 1)) program->install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program->install_ram(0x8000, 0xbfff, ram + 0x0000);
		if (BIT(m_bank1, 3)) program->install_ram(0xc000, 0xffff, ram + 0x4000);
		break;

	case 4:
		if (BIT(m_bank1, 0)) program->install_ram(0x0000, 0x3fff, ram + 0xc000);
		if (BIT(m_bank1, 1)) program->install_ram(0x4000, 0x7fff, ram + 0x14000);
		if (BIT(m_bank1, 2)) program->install_ram(0x8000, 0xbfff, ram + 0x18000);
		if (BIT(m_bank1, 3)) program->install_ram(0xc000, 0xffff, ram + 0x1c000);
		break;
	}

	// bank 0 overrides
	if (BIT(m_bank0, 0)) program->install_ram(0x0000, 0x3fff, ram + 0x0000);
	if (BIT(m_bank0, 1)) program->install_ram(0x4000, 0x7fff, ram + 0x4000);
	if (BIT(m_bank0, 2)) program->install_ram(0x8000, 0xbfff, ram + 0x8000);
	if (BIT(m_bank0, 3)) program->install_ram(0xc000, 0xffff, ram + 0xc000);

	// PROM enabled
	if (!BIT(m_bank0, 5)) program->install_rom(0xf000, 0xf7ff, 0, 0x800, rom);
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

	m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);

	return !wd17xx_intrq_r(m_fdc) << 7;
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
	wd17xx_set_drive(m_fdc, data & 0x03);
	floppy_mon_w(m_floppy0, 0);
	floppy_mon_w(m_floppy1, 0);

	// head select
	wd17xx_set_side(m_fdc, BIT(data, 2));

	// disk density
	wd17xx_dden_w(m_fdc, !BIT(data, 3));
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

	m_brg->str_w(space, 0, data & 0x0f);
	m_brg->stt_w(space, 0, data >> 4);
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
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE_LEGACY(Z80DART_TAG, z80dart_ba_cd_r, z80dart_ba_cd_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE_LEGACY(WD2793_TAG, wd17xx_r, wd17xx_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x03) AM_DEVREADWRITE_LEGACY(Z80DMA_TAG, z80dma_r, z80dma_w)
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
	PORT_DIPNAME( 0x0f, 0x0e, "SIO Channel A Baud Rate" ) PORT_DIPLOCATION("J7:1,2,3,4")
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
//  Z80CTC_INTERFACE( ctc_intf )
//-------------------------------------------------

static TIMER_DEVICE_CALLBACK( ctc_tick )
{
	super6_state *state = timer.machine().driver_data<super6_state>();

	state->m_ctc->trg0(1);
	state->m_ctc->trg0(0);
}

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  Z80DART_INTERFACE( dart_intf )
//-------------------------------------------------

static Z80DART_INTERFACE( dart_intf )
{
	0, 0, 0, 0,

	DEVCB_DEVICE_LINE_MEMBER(TERMINAL_TAG, serial_terminal_device, tx_r),
	DEVCB_DEVICE_LINE_MEMBER(TERMINAL_TAG, serial_terminal_device, rx_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0)
};


//-------------------------------------------------
//  Z80DMA_INTERFACE( dma_intf )
//-------------------------------------------------

static UINT8 memory_read_byte(address_space &space, offs_t address, UINT8 mem_mask) { return space.read_byte(address); }
static void memory_write_byte(address_space &space, offs_t address, UINT8 data, UINT8 mem_mask) { space.write_byte(address, data); }

static Z80DMA_INTERFACE( dma_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_HALT),
	DEVCB_DEVICE_LINE_MEMBER(Z80CTC_TAG, z80ctc_device, trg2),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER(Z80_TAG, PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, PROGRAM, memory_write_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, IO, memory_read_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, IO, memory_write_byte)
};


//-------------------------------------------------
//  Z80PIO_INTERFACE( pio_intf )
//-------------------------------------------------

static Z80PIO_INTERFACE( pio_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  COM8116_INTERFACE( brg_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( super6_state::fr_w )
{
	z80dart_rxca_w(m_dart, state);
	z80dart_txca_w(m_dart, state);

	m_ctc->trg1(state);
}

static COM8116_INTERFACE( brg_intf )
{
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(super6_state, fr_w),
	DEVCB_DEVICE_LINE(Z80DART_TAG, z80dart_rxtxcb_w),
	{ 6336, 4224, 2880, 2355, 2112, 1056, 528, 264, 176, 158, 132, 88, 66, 44, 33, 16 }, // from WD1943-00 datasheet
	{ 6336, 4224, 2880, 2355, 2112, 1056, 528, 264, 176, 158, 132, 88, 66, 44, 33, 16 },
};


//-------------------------------------------------
//  floppy_interface super6_floppy_interface
//-------------------------------------------------

static const floppy_interface super6_floppy_interface =
{
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    FLOPPY_STANDARD_5_25_DSHD,
    LEGACY_FLOPPY_OPTIONS_NAME(default),
    "floppy_5_25",
	NULL
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( super6_state::intrq_w )
{
	if (state) m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);

	m_ctc->trg3(!state);
}

WRITE_LINE_MEMBER( super6_state::drq_w )
{
	if (state) m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);

	m_dma->rdy_w(state);
}

static const wd17xx_interface fdc_intf =
{
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(super6_state, intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(super6_state, drq_w),
	{ FLOPPY_0, FLOPPY_1, NULL, NULL }
};


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


//-------------------------------------------------
//  GENERIC_TERMINAL_INTERFACE( terminal_intf )
//-------------------------------------------------

static WRITE8_DEVICE_HANDLER( dummy_w )
{
	// handled in Z80DART_INTERFACE
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_HANDLER(dummy_w)
};



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


//-------------------------------------------------
//  MACHINE_RESET( super6 )
//-------------------------------------------------

void super6_state::machine_reset()
{
	m_bank0 = m_bank1 = 0;

	bankswitch();
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
	MCFG_Z80CTC_ADD(Z80CTC_TAG, XTAL_24MHz/4, ctc_intf)
	MCFG_TIMER_ADD_PERIODIC("ctc", ctc_tick, attotime::from_hz(XTAL_24MHz/16))
	MCFG_Z80DART_ADD(Z80DART_TAG, XTAL_24MHz/4, dart_intf)
	MCFG_Z80DMA_ADD(Z80DMA_TAG, XTAL_24MHz/6, dma_intf)
	MCFG_Z80PIO_ADD(Z80PIO_TAG, XTAL_24MHz/4, pio_intf)
	MCFG_WD2793_ADD(WD2793_TAG, fdc_intf)
	MCFG_COM8116_ADD(BR1945_TAG, XTAL_5_0688MHz, brg_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(super6_floppy_interface)
	MCFG_SERIAL_TERMINAL_ADD(TERMINAL_TAG, terminal_intf, 4800)

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
	ROM_LOAD( "digitex monitor 1.2a 6oct1983.u29", 0x000, 0x800, CRC(a4c33ce4) SHA1(46dde43ea51d295f2b3202c2d0e1883bde1a8da7) )

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "pal16l8.u16", 0x000, 0x800, NO_DUMP )
	ROM_LOAD( "pal16l8.u36", 0x000, 0x800, NO_DUMP )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    INIT    COMPANY                          FULLNAME        FLAGS
COMP( 1983, super6,  0,      0,      super6,  super6, driver_device,  0,      "Advanced Digital Corporation",	"Super Six",	GAME_NOT_WORKING | GAME_NO_SOUND_HW )
