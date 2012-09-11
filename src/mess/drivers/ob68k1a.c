/*

Omnibyte OB68K1A

PCB Layout
----------

REV-B

|-----------------------------------------------------------|
|   555     CN3     CN4         CN5   8116  SW5     10MHz   |
|                                       5.0688MHz           |
|                   PIA         PIA                         |
|           ACIA                            ROM1    ROM0    |
|                   ACIA        PTM         ROM3    ROM2    |
|           SW1                             ROM5    ROM4    |
|                       CPU                                 |
|           SW2                         4164 4164 4164 4164 |
|                                       4164 4164 4164 4164 |
|           SW3                         4164 4164 4164 4164 |
|                                   555 4164 4164 4164 4164 |
|           SW4                                 8409        |
|-|-------------CN1----------------|----|-------CN2-------|-|
  |--------------------------------|    |-----------------|

Notes:
    Relevant IC's shown.

    CPU     - Motorola MC68000L10
    PIA     - Motorola MC6821P
    ACIA    - Motorola MC6850P
    PTM     - Motorola MC6840P
    8116    - SMC COM8116
    8409    - National Semiconductor DP8409N DRAM Controller
    4164    - Fujitsu MB8264A-12 64Kx1 RAM
    555     - NE555N
    SW1     - RAM address DIP8
    SW2     - I/O address DIP8
    SW3     - ROM address DIP8
    SW4     - external RAM access address DIP8
    SW5     - push button
    CN1     - 2x43 PCB edge, IEEE 796 (Multibus)
    CN2     - 2x30 PCB edge, IEEE 796 (Multibus)
    CN3     - 2x25 header, ACIA 0
    CN4     - 2x25 header, ACIA 1
    CN5     - 2x50 header, PIA

*/

/*

    TODO:

    - COM8116 dividers are wrong
    - interrupts
    - configuration switches
    - PIA odd/even byte access

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/ram.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/com8116.h"
#include "machine/terminal.h"
#include "includes/ob68k1a.h"



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  com8116_w - baud rate selection
//-------------------------------------------------

WRITE8_MEMBER( ob68k1a_state::com8116_w )
{
	m_dbrg->stt_w(space, 0, data & 0x0f);
//  m_dbrg->str_w(space, 0, data >> 4); // HACK for terminal
}


//-------------------------------------------------
//  pia_r - trampoline for PIA odd/even access
//-------------------------------------------------

READ8_MEMBER( ob68k1a_state::pia_r )
{
	if (offset) {
		return m_pia1->read(space,0);
	} else {
		return m_pia0->read(space,0);
	}
}


//-------------------------------------------------
//  pia_w - trampoline for PIA odd/even access
//-------------------------------------------------

WRITE8_MEMBER( ob68k1a_state::pia_w )
{
	if (offset) {
		m_pia1->write(space,0,data);
	} else {
		m_pia0->write(space,0,data);
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( ob68k1a_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( ob68k1a_mem, AS_PROGRAM, 16, ob68k1a_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x01ffff) AM_RAM
	AM_RANGE(0xfe0000, 0xfeffff) AM_ROM AM_REGION(MC68000L10_TAG, 0)
	AM_RANGE(0xffff00, 0xffff01) AM_DEVREADWRITE8(MC6850_0_TAG, acia6850_device, status_read, control_write, 0x00ff)
	AM_RANGE(0xffff02, 0xffff03) AM_DEVREADWRITE8(MC6850_0_TAG, acia6850_device, data_read, data_write, 0x00ff)
	AM_RANGE(0xffff10, 0xffff11) AM_WRITE8(com8116_w, 0xff00)
	AM_RANGE(0xffff20, 0xffff21) AM_DEVREADWRITE8(MC6850_1_TAG, acia6850_device, status_read, control_write, 0x00ff)
	AM_RANGE(0xffff22, 0xffff23) AM_DEVREADWRITE8(MC6850_1_TAG, acia6850_device, data_read, data_write, 0x00ff)
//  AM_RANGE(0xffff40, 0xffff47) AM_DEVREADWRITE8(MC6821_0_TAG, pia6821_device, read, write, 0x00ff)
//  AM_RANGE(0xffff40, 0xffff47) AM_DEVREADWRITE8(MC6821_1_TAG, pia6821_device, read, write, 0xff00)
	AM_RANGE(0xffff40, 0xffff47) AM_READWRITE8(pia_r, pia_w, 0xffff)
	AM_RANGE(0xffff60, 0xffff6f) AM_DEVREADWRITE8(MC6840_TAG, ptm6840_device, read, write, 0x00ff)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( ob68k1a )
//-------------------------------------------------

INPUT_PORTS_START( ob68k1a )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  pia6821_interface pia0_intf
//-------------------------------------------------

static const pia6821_interface pia0_intf =
{
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
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  pia6821_interface pia1_intf
//-------------------------------------------------

static const pia6821_interface pia1_intf =
{
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
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  ptm6840_interface ptm_intf
//-------------------------------------------------

static const ptm6840_interface ptm_intf =
{
	XTAL_10MHz/10,
	{ 0, 0, 0 },
	{ DEVCB_NULL,
	  DEVCB_NULL,
	  DEVCB_NULL },
	DEVCB_NULL
};

//-------------------------------------------------
//  ACIA6850_INTERFACE( acia0_intf )
//-------------------------------------------------

static ACIA6850_INTERFACE( acia0_intf )
{
	9600*16, // HACK for terminal
	9600*16, // HACK for terminal
	DEVCB_DEVICE_LINE_MEMBER(TERMINAL_TAG, serial_terminal_device, tx_r),
	DEVCB_DEVICE_LINE_MEMBER(TERMINAL_TAG, serial_terminal_device, rx_w),
	DEVCB_LINE_GND, // HACK for terminal
	DEVCB_NULL,
	DEVCB_LINE_GND, // HACK for terminal
	DEVCB_NULL
};


//-------------------------------------------------
//  ACIA6850_INTERFACE( acia1_intf )
//-------------------------------------------------

static ACIA6850_INTERFACE( acia1_intf )
{
	0,
	0,
	DEVCB_LINE_VCC,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
};


//-------------------------------------------------
//  COM8116_INTERFACE( dbrg_intf )
//-------------------------------------------------

static WRITE_LINE_DEVICE_HANDLER( rx_tx_w )
{
	downcast<acia6850_device *>(device)->rx_clock_in();
	downcast<acia6850_device *>(device)->tx_clock_in();
}

static COM8116_INTERFACE( dbrg_intf )
{
	DEVCB_NULL,		/* fX/4 output */
	DEVCB_DEVICE_LINE(MC6850_0_TAG, rx_tx_w),
	DEVCB_DEVICE_LINE(MC6850_1_TAG, rx_tx_w),
	{ 101376, 67584, 46080, 37686, 33792, 16896, 8448, 4224, 2816, 2534, 2112, 1408, 1056, 704, 528, 264 },			/* receiver divisor ROM */
	{ 101376, 67584, 46080, 37686, 33792, 16896, 8448, 4224, 2816, 2534, 2112, 1408, 1056, 704, 528, 264 },			/* transmitter divisor ROM */
};


//-------------------------------------------------
//  GENERIC_TERMINAL_INTERFACE( terminal_intf )
//-------------------------------------------------

static serial_terminal_interface terminal_intf =
{
	DEVCB_NULL
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( ob68k1a )
//-------------------------------------------------

void ob68k1a_state::machine_start()
{
	address_space *program = m_maincpu->space(AS_PROGRAM);

	// configure RAM
	switch (m_ram->size())
	{
	case 32*1024:
		program->unmap_readwrite(0x008000, 0x01ffff);
		break;
	}
}


//-------------------------------------------------
//  MACHINE_RESET( ob68k1a )
//-------------------------------------------------

void ob68k1a_state::machine_reset()
{
	address_space *program = m_maincpu->space(AS_PROGRAM);

	// initialize COM8116
//  m_dbrg->stt_w(program, 0, 0x01);
//  m_dbrg->str_w(program, 0, 0x01);

	// set reset vector
	void *ram = program->get_write_ptr(0);
	UINT8 *rom = memregion(MC68000L10_TAG)->base();

	memcpy(ram, rom, 8);

	machine().firstcpu->reset();
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( ob68k1a )
//-------------------------------------------------

static MACHINE_CONFIG_START( ob68k1a, ob68k1a_state )
    // basic machine hardware
    MCFG_CPU_ADD(MC68000L10_TAG, M68000, XTAL_10MHz)
    MCFG_CPU_PROGRAM_MAP(ob68k1a_mem)

	// devices
	MCFG_PIA6821_ADD(MC6821_0_TAG, pia0_intf)
	MCFG_PIA6821_ADD(MC6821_1_TAG, pia1_intf)
	MCFG_PTM6840_ADD(MC6840_TAG, ptm_intf)
	MCFG_ACIA6850_ADD(MC6850_0_TAG, acia0_intf)
	MCFG_ACIA6850_ADD(MC6850_1_TAG, acia1_intf)
	MCFG_COM8116_ADD(COM8116_TAG, XTAL_5_0688MHz, dbrg_intf)
	MCFG_SERIAL_TERMINAL_ADD(TERMINAL_TAG, terminal_intf, 9600)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("128K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( ob68k1a )
//-------------------------------------------------

ROM_START( ob68k1a )
    ROM_REGION16_BE( 0x10000, MC68000L10_TAG, 0 )
	ROM_LOAD16_BYTE( "macsbug.u60",    0x0000, 0x2000, CRC(7c8905ff) SHA1(eba6c70f6b5b40d60e2885c2bd33dd93ec2aae48) )
	ROM_LOAD16_BYTE( "macsbug.u61",    0x0001, 0x2000, CRC(b5069252) SHA1(b310465d8ece944bd694cc9726d03fed0f4b2c0f) )
	ROM_LOAD16_BYTE( "idris_boot.u62", 0x4000, 0x2000, CRC(091e900e) SHA1(ea0c9f3ad5179eab2e743459c8afb707c059f0e2) )
	ROM_LOAD16_BYTE( "idris_boot.u63", 0x4001, 0x2000, CRC(a132259f) SHA1(34216bf1d22ff0f0af29699a1e4e0e57631f775d) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "1.0.u18", 0x000, 0x100, NO_DUMP ) // PAL16L8
	ROM_LOAD( "2.5.u20", 0x000, 0x100, NO_DUMP ) // PAL16L8
	ROM_LOAD( "3.2.u21", 0x000, 0x100, NO_DUMP ) // PAL16R4
	ROM_LOAD( "4.3.u49", 0x000, 0x100, NO_DUMP ) // PAL16R4
	ROM_LOAD( "5.1.u51", 0x000, 0x100, NO_DUMP ) // PAL16L8
	ROM_LOAD( "6.2.u55", 0x000, 0x100, NO_DUMP ) // PAL16L8
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS
COMP( 1982, ob68k1a,  0,       0,	ob68k1a,	ob68k1a, driver_device,	 0,  "Omnibyte",   "OB68K1A",	GAME_NO_SOUND_HW )
