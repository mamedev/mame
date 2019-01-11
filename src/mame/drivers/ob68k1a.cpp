// license:BSD-3-Clause
// copyright-holders:Curt Coder
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

    - interrupts
    - configuration switches
    - PIA odd/even byte access

*/


#include "emu.h"
#include "includes/ob68k1a.h"



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  com8116_w - baud rate selection
//-------------------------------------------------

WRITE8_MEMBER( ob68k1a_state::com8116_w )
{
	m_dbrg->stt_w(data & 0x0f);
	m_dbrg->str_w(data >> 4);
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

void ob68k1a_state::ob68k1a_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x01ffff).ram();
	map(0xfe0000, 0xfeffff).rom().region(MC68000L10_TAG, 0);
	map(0xffff00, 0xffff03).rw(m_acia0, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0xffff10, 0xffff10).w(this, FUNC(ob68k1a_state::com8116_w));
	map(0xffff20, 0xffff23).rw(m_acia1, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
//  AM_RANGE(0xffff40, 0xffff47) AM_DEVREADWRITE8(MC6821_0_TAG, pia6821_device, read, write, 0x00ff)
//  AM_RANGE(0xffff40, 0xffff47) AM_DEVREADWRITE8(MC6821_1_TAG, pia6821_device, read, write, 0xff00)
	map(0xffff40, 0xffff47).rw(this, FUNC(ob68k1a_state::pia_r), FUNC(ob68k1a_state::pia_w));
	map(0xffff60, 0xffff6f).rw(MC6840_TAG, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( ob68k1a )
//-------------------------------------------------

INPUT_PORTS_START( ob68k1a )
INPUT_PORTS_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( ob68k1a )
//-------------------------------------------------

void ob68k1a_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// configure RAM
	switch (m_ram->size())
	{
	case 32*1024:
		program.unmap_readwrite(0x008000, 0x01ffff);
		break;
	}
}


void ob68k1a_state::machine_reset()
{
	// initialize COM8116
	m_dbrg->stt_w(0x0e);
	m_dbrg->str_w(0x0e);

	// set reset vector
	void *ram = m_maincpu->space(AS_PROGRAM).get_write_ptr(0);
	uint8_t *rom = memregion(MC68000L10_TAG)->base();

	memcpy(ram, rom, 8);

	m_maincpu->reset();
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( ob68k1a )
//-------------------------------------------------

MACHINE_CONFIG_START(ob68k1a_state::ob68k1a)
	// basic machine hardware
	MCFG_DEVICE_ADD(MC68000L10_TAG, M68000, XTAL(10'000'000))
	MCFG_DEVICE_PROGRAM_MAP(ob68k1a_mem)

	// devices
	MCFG_DEVICE_ADD(MC6821_0_TAG, PIA6821, 0)
	MCFG_DEVICE_ADD(MC6821_1_TAG, PIA6821, 0)
	MCFG_DEVICE_ADD(MC6840_TAG, PTM6840, XTAL(10'000'000)/10)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)

	MCFG_DEVICE_ADD(MC6850_0_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE(RS232_A_TAG, rs232_port_device, write_rts))

	MCFG_DEVICE_ADD(RS232_A_TAG, RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE(MC6850_0_TAG, acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(MC6850_0_TAG, acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(WRITELINE(MC6850_0_TAG, acia6850_device, write_cts))

	MCFG_DEVICE_ADD(MC6850_1_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE(RS232_B_TAG, rs232_port_device, write_rts))

	MCFG_DEVICE_ADD(RS232_B_TAG, RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(MC6850_1_TAG, acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(MC6850_1_TAG, acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(WRITELINE(MC6850_1_TAG, acia6850_device, write_cts))

	MCFG_DEVICE_ADD(COM8116_TAG, COM8116, XTAL(5'068'800))
	MCFG_COM8116_FR_HANDLER(WRITELINE(MC6850_0_TAG, acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE(MC6850_0_TAG, acia6850_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(WRITELINE(MC6850_1_TAG, acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE(MC6850_1_TAG, acia6850_device, write_rxc))

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

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME   FLAGS
COMP( 1982, ob68k1a, 0,      0,      ob68k1a, ob68k1a, ob68k1a_state, empty_init, "Omnibyte", "OB68K1A", MACHINE_NO_SOUND_HW )
