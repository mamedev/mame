/*

    Commodore C900
    UNIX prototype

    http://www.zimmers.net/cbmpics/c900.html
    http://www.zimmers.net/cbmpics/cbm/900/c900-chips.txt

*/


#include "emu.h"
#include "cpu/z8000/z8000.h"
#include "machine/terminal.h"


class c900_state : public driver_device
{
public:
	c900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{ }

//  UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ16_MEMBER(key_r);
	DECLARE_READ16_MEMBER(stat_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(c900_mem, AS_PROGRAM, 16, c900_state)
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(c900_io, AS_IO, 16, c900_state)
	AM_RANGE(0x100, 0x101) AM_READ(stat_r)
	AM_RANGE(0x110, 0x111) AM_READ(key_r) AM_DEVWRITE8(TERMINAL_TAG, generic_terminal_device, write, 0x00ff)
ADDRESS_MAP_END

static INPUT_PORTS_START( c900 )
INPUT_PORTS_END

//UINT32 c900_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
//{
//  return 0;
//}


READ16_MEMBER( c900_state::key_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ16_MEMBER( c900_state::stat_r )
{
	return (m_term_data) ? 5 : 4;
}

WRITE8_MEMBER( c900_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(c900_state, kbd_put)
};

static MACHINE_CONFIG_START( c900, c900_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z8001, 4000000)
	MCFG_CPU_PROGRAM_MAP(c900_mem)
	MCFG_CPU_IO_MAP(c900_io)

	/* video hardware */
//  MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
//  MCFG_SCREEN_SIZE(512, 256)
//  MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
//  MCFG_PALETTE_LENGTH(4)
//  MCFG_SCREEN_UPDATE_DRIVER(c900_state, screen_update)
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

ROM_START( c900 )
	ROM_REGION16_LE( 0x8000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "c 900 boot-h v 1.0.bin.u17", 0x0000, 0x4000, CRC(c3aa7fc1) SHA1(ff12dd100fa7b1e7e931e9a8ef4c4f5cc056e099) )
	ROM_LOAD16_BYTE( "c 900 boot-l v 1.0.bin.u18", 0x0001, 0x4000, CRC(0aa39272) SHA1(b2c5da4586d38fc66bb33aafeae4dbda36080f1e) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "380217-01.u2", 0x0000, 0x1000, CRC(64cb4171) SHA1(e60d796170addfd27e2c33090f9c512c7e3f99f5) )
ROM_END

/*    YEAR  NAME   PARENT  COMPAT  MACHINE INPUT   INIT COMPANY     FULLNAME        FLAGS */
COMP( 1985, c900,   0,      0,      c900,    c900, driver_device,    0, "Commodore", "Commodore 900", GAME_NOT_WORKING | GAME_NO_SOUND)
