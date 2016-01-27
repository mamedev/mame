// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    NCR DMV K220 Diagnostic module

    to be inserted into slot 7 only

    all semiconductors shown:

   |-----------------------------------------------------------------------|
   |                                                                       |
   |                  S     S                     NEC D8255AC-5            |
   |   PB1            E     E                                              |
   | X1               G     G                                              |
   |                  1     2      SN84LS247N         ROM1                 |
   | LED1                                                                  |
   | LED2                SW1       SN74LS247N                            C |
   | LED3   TCA965       SW2                                             N |
   |          8302       SW3       PROM1              ROM2               1 |
   | LED4                SW4                                               |
   | LED5        TCA965  SW5                                               |
   | LED6          8240  SW6                                               |
   |                     SW7       M5L8253P-5         RAM                  |
   | LED7                SW8                                               |
   | LED8        SN7407N                                                   |
   | LED9                                                                  |
   |                                                                       |
   |                   SN74LS14N   DM74LS05N DM74LS00M DM74LS04N           |
   |-----------------------------------------------------------------------|

   PB1:             Push Button
   X1:              Crystal 24 MHz
   LED 1:           red, 5V>
   LED 2:           green, 5V-
   LED 3:           red, 5V<
   LED 4:           red, 12V>
   LED 5:           green, 12V-
   LED 6:           red, 12V<
   LED 7:           green, PCLK/
   LED 8:           green, MEMR/
   LED 9:           green, HLDA
   SEG 1&2          Seven segment displays
   SW1-8            Switches marked top=>bottom Off / Start, S Run / Loop, M Mes / D Mes, CRT / LED, SEL 1, SEL 2, SEL 3, SEL 4
   PROM 1:          TBP24S10N marked 32084
   ROM 1:           MBM2764-30 marked 32564
   ROM 2:           M5L2764K marked 32563
   RAM:             HM6116LP-3

   Inserting the diagnostics module into Slot 7 changes the DMV's memory map:

   The diagnostics ROM is inserted between 0x2000 and 0x3FFF, the diagnostics RAM between 0xF000 and 0xF7FF with the diagnostics stack at 0xF700

***************************************************************************/

#include "emu.h"
#include "k220.h"
#include "render.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

ROM_START( dmv_k220 )
	ROM_REGION(0x4000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "v4", "V 04.00")
	ROMX_LOAD("34014.u17", 0x0000, 0x2000, CRC(552c2247) SHA1(7babd264ead2e04afe624c3035f211279c203f41), ROM_BIOS(1))
	ROMX_LOAD("34015.u18", 0x2000, 0x2000, CRC(d714f2d8) SHA1(1a7095401d63951ba9189bc3e384c26996113815), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "v2", "V 02.00")
	ROMX_LOAD("32563.u17", 0x0000, 0x2000, CRC(57445768) SHA1(59b615437444789bf10ba6768cd3c43a69c7ed7b), ROM_BIOS(2))
	ROMX_LOAD("32564.u18", 0x2000, 0x2000, CRC(172e0c60) SHA1(eedae538636009a5b86fc78e50a03c72eeb0e73b), ROM_BIOS(2))

	ROM_REGION(0x0080, "prom", 0)
	ROM_LOAD( "u11.bin", 0x0000, 0x0080, NO_DUMP)   // used for address decoding

	ROM_REGION(0x0800, "ram", ROMREGION_ERASE)
ROM_END

static MACHINE_CONFIG_FRAGMENT( dmv_k220 )
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(dmv_k220_device, porta_w))
	MCFG_I8255_IN_PORTB_CB(IOPORT("SWITCH"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(dmv_k220_device, portc_w))

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_1MHz)  // CLK1
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(dmv_k220_device, write_out0))
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(dmv_k220_device, write_out1))
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(dmv_k220_device, write_out2))
MACHINE_CONFIG_END

static INPUT_PORTS_START( dmv_k220 )
	PORT_START("SWITCH")
	PORT_DIPNAME( 0x01, 0x00, "Select 1" )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_DIPSETTING( 0x01, DEF_STR(On) )
	PORT_DIPNAME( 0x02, 0x00, "Select 2" )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_DIPSETTING( 0x02, DEF_STR(On) )
	PORT_DIPNAME( 0x04, 0x00, "Select 3" )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_DIPSETTING( 0x04, DEF_STR(On) )
	PORT_DIPNAME( 0x08, 0x00, "Select 4" )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_DIPSETTING( 0x08, DEF_STR(On) )
	PORT_DIPNAME( 0x10, 0x10, "Maintenance" )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_DIPSETTING( 0x10, DEF_STR(On) )
	PORT_DIPNAME( 0x20, 0x20, "Detail Message" )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_DIPSETTING( 0x20, DEF_STR(On) )
	PORT_DIPNAME( 0x40, 0x40, "Continuous Run" )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_DIPSETTING( 0x40, DEF_STR(On) )
	PORT_DIPNAME( 0x80, 0x80, "Diagnostic module" )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_DIPSETTING( 0x80, DEF_STR(On) )
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type DMV_K220 = &device_creator<dmv_k220_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k220_device - constructor
//-------------------------------------------------

dmv_k220_device::dmv_k220_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, DMV_K220, "K220 diagnostic", tag, owner, clock, "dmv_k220", __FILE__),
		device_dmvslot_interface( mconfig, *this ),
		m_pit(*this, "pit8253"),
		m_ppi(*this, "ppi8255"),
		m_ram(*this, "ram"),
		m_rom(*this, "rom"), m_portc(0)
	{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k220_device::device_start()
{
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_readwrite_handler(0x08, 0x0b, 0, 0, read8_delegate(FUNC(pit8253_device::read), &(*m_pit)), write8_delegate(FUNC(pit8253_device::write), &(*m_pit)), 0);
	space.install_readwrite_handler(0x0c, 0x0f, 0, 0, read8_delegate(FUNC(i8255_device::read), &(*m_ppi)), write8_delegate(FUNC(i8255_device::write), &(*m_ppi)), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmv_k220_device::device_reset()
{
	// active the correct layout
	machine().render().first_target()->set_view(1);
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor dmv_k220_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k220 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dmv_k220_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k220 );
}

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const rom_entry *dmv_k220_device::device_rom_region() const
{
	return ROM_NAME( dmv_k220 );
}

//-------------------------------------------------
//  read
//-------------------------------------------------

bool dmv_k220_device::read(offs_t offset, UINT8 &data)
{
	if ((m_portc & 0x01) && offset >= 0x2000 && offset < 0x6000)
	{
		data = m_rom->base()[offset - 0x2000];
		return true;
	}
	else if ((m_portc & 0x02) && offset >= 0xf000 && offset < 0xf800)
	{
		data = m_ram->base()[offset & 0x7ff];
		return true;
	}

	return false;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

bool dmv_k220_device::write(offs_t offset, UINT8 data)
{
	if ((m_portc & 0x01) && offset >= 0x2000 && offset < 0x4000)
	{
		logerror("k220: write on ROM %x %x\n", offset, data);
		return true;
	}
	else if ((m_portc & 0x02) && offset >= 0xf000 && offset < 0xf800)
	{
		m_ram->base()[offset & 0x7ff] = data;
		return true;
	}

	return false;
}

WRITE8_MEMBER( dmv_k220_device::porta_w )
{
	// 74LS247 BCD-to-Seven-Segment Decoder
	static UINT8 bcd2hex[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00 };

	machine().output().set_digit_value(0, bcd2hex[(data >> 4) & 0x0f]);
	machine().output().set_digit_value(1, bcd2hex[data & 0x0f]);
}


WRITE8_MEMBER( dmv_k220_device::portc_w )
{
	/*
	    xxxx ---- not connected
	    ---- x--- PIT gate 2
	    ---- -x-- PIT gate 1
	    ---- --x- enable RAM
	    ---- ---x enable ROM

	*/
	m_pit->write_gate1(BIT(data, 2));
	m_pit->write_gate2(BIT(data, 3));

	m_portc = data;
}


WRITE_LINE_MEMBER( dmv_k220_device::write_out0 )
{
	m_pit->write_clk1(state);
	m_pit->write_clk2(state);
}


WRITE_LINE_MEMBER( dmv_k220_device::write_out1 )
{
}


WRITE_LINE_MEMBER( dmv_k220_device::write_out2 )
{
}
