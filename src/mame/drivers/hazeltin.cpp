// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

	Hazeltine 1500
	original machine (c) 1977 Hazeltine Corporation

	perliminary driver by Ryan Holtz

TODO:
    - pretty much everything

References:
	[1]: Hazeltine_1500_Series_Maintenance_Manual_Dec77.pdf, on Bitsavers

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/ay31015.h"
#include "machine/com8116.h"
#include "machine/keyboard.h"

#define KEYBOARD_TAG	"keyboard"
#define CPU_TAG			"maincpu"
#define UART_TAG		"uart"
#define BAUDGEN_TAG		"baudgen"
#define BAUDPORT_TAG	"BAUD"
#define MISCPORT_TAG	"MISC"

#define SR2_FULL_DUPLEX	(0x01)
#define SR2_UPPER_ONLY	(0x08)

#define SR3_PB_RESET	(0x04)

class hazl1500_state : public driver_device
{
public:
	hazl1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CPU_TAG)
		, m_uart(*this, UART_TAG)
		, m_baud_dips(*this, BAUDPORT_TAG)
		, m_misc_dips(*this, MISCPORT_TAG)
		, m_status_reg_3(0)
		, m_keyboard_status_latch(0)
	{
	}

	virtual void machine_reset() override;

	uint32_t screen_update_hazl1500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER( com5016_fr_w );

	DECLARE_READ8_MEMBER( system_test_r ); // noted as "for use with auto test equip" in flowchart on pg. 30, ref[1], jumps to 0x8000 if bit 0 is unset
	DECLARE_READ8_MEMBER( status_reg_2_r );
	DECLARE_WRITE8_MEMBER( status_reg_3_w );
	DECLARE_READ8_MEMBER( keyboard_status_latch_r );

private:
	required_device<cpu_device> m_maincpu;
	required_device<ay31015_device> m_uart;
	required_ioport m_baud_dips;
	required_ioport m_misc_dips;

	uint8_t m_status_reg_3;
	uint8_t m_keyboard_status_latch;
};

void hazl1500_state::machine_reset()
{
	m_status_reg_3 = 0;
	m_keyboard_status_latch = 0;
}

WRITE_LINE_MEMBER( hazl1500_state::com5016_fr_w )
{
	m_uart->rx_process();
	m_uart->tx_process();
}

uint32_t hazl1500_state::screen_update_hazl1500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ8_MEMBER( hazl1500_state::keyboard_status_latch_r )
{
	return m_keyboard_status_latch ^ 0xff;
}

READ8_MEMBER( hazl1500_state::system_test_r )
{
	return 0xff;
}

READ8_MEMBER( hazl1500_state::status_reg_2_r )
{
	uint8_t misc_dips = m_misc_dips->read();
	uint8_t status = 0;

	if (misc_dips & 0x10)
		status |= SR2_FULL_DUPLEX;
	if (misc_dips & 0x40)
		status |= SR2_UPPER_ONLY;

	return status ^ 0xff;
}

WRITE8_MEMBER( hazl1500_state::status_reg_3_w )
{
	//printf("sr3: %02x\n", data);
	m_status_reg_3 = data;
}

static ADDRESS_MAP_START(hazl1500_mem, AS_PROGRAM, 8, hazl1500_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x3000, 0x377f) AM_RAM AM_SHARE("char_ram")
	AM_RANGE(0x3780, 0x37ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(hazl1500_io, AS_IO, 8, hazl1500_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf7, 0xf7) AM_READ(keyboard_status_latch_r)
	AM_RANGE(0xef, 0xef) AM_READ(system_test_r)
	AM_RANGE(0x7f, 0x7f) AM_READWRITE(status_reg_2_r, status_reg_3_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( hazl1500 )
	PORT_START(BAUDPORT_TAG)
	PORT_DIPNAME( 0xff, 0x08, "Baud Rate" )
	PORT_DIPSETTING( 0x01, "110" )
	PORT_DIPSETTING( 0x02, "300" )
	PORT_DIPSETTING( 0x04, "1200" )
	PORT_DIPSETTING( 0x08, "1800" )
	PORT_DIPSETTING( 0x10, "2400" )
	PORT_DIPSETTING( 0x20, "4800" )
	PORT_DIPSETTING( 0x40, "9600" )
	PORT_DIPSETTING( 0x80, "19.2K" )

	PORT_START(MISCPORT_TAG)
	PORT_DIPNAME( 0x0f, 0x01, "Parity" )
	PORT_DIPSETTING( 0x01, "Even" )
	PORT_DIPSETTING( 0x02, "Odd" )
	PORT_DIPSETTING( 0x04, "1" )
	PORT_DIPSETTING( 0x08, "0" )
	PORT_DIPNAME( 0x10, 0x10, "Duplex" )
	PORT_DIPSETTING( 0x00, "Half" )
	PORT_DIPSETTING( 0x10, "Full" )
	PORT_DIPNAME( 0x20, 0x20, "Auto" )
	PORT_DIPSETTING( 0x00, "LF" )
	PORT_DIPSETTING( 0x20, "CR" )
	PORT_DIPNAME( 0x40, 0x40, "Case" )
	PORT_DIPSETTING( 0x00, "Upper and Lower" )
	PORT_DIPSETTING( 0x40, "Upper" )
	PORT_DIPNAME( 0x80, 0x80, "Video" )
	PORT_DIPSETTING( 0x00, "Standard" )
	PORT_DIPSETTING( 0x80, "Reverse" )
INPUT_PORTS_END

/* F4 Character Displayer */
static const gfx_layout hazl1500_charlayout =
{
	8, 16,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START( hazl1500 )
	GFXDECODE_ENTRY( "chargen", 0x0000, hazl1500_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( hazl1500, hazl1500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_1MHz) //unknown clock
	MCFG_CPU_PROGRAM_MAP(hazl1500_mem)
	MCFG_CPU_IO_MAP(hazl1500_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 384-1)
	MCFG_SCREEN_UPDATE_DRIVER(hazl1500_state, screen_update_hazl1500)

	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hazl1500)

	/* Devices */
	MCFG_DEVICE_ADD(BAUDGEN_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(hazl1500_state, com5016_fr_w))

	MCFG_DEVICE_ADD(UART_TAG, AY51013, 0)

	//MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	//MCFG_GENERIC_KEYBOARD_CB(WRITE8(hazl1500_state, kbd_put))
MACHINE_CONFIG_END


ROM_START( hazl1500 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "h15s-00I-10-3.bin", 0x0000, 0x0800, CRC(a2015f72) SHA1(357cde517c3dcf693de580881add058c7b26dfaa))

	ROM_REGION( 0x800, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "u83_chr.bin", 0x0000, 0x0800, CRC(e0c6b734) SHA1(7c42947235c66c41059fd4384e09f4f3a17c9857))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT	COMPAT   MACHINE   INPUT     CLASS			INIT    COMPANY                		FULLNAME     		FLAGS */
COMP( 1977, hazl1500, 0,		0,       hazl1500, hazl1500, driver_device,	0,		"Hazeltine Corporation",	"Hazeltine 1500",	MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
