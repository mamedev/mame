// license:BSD-3-Clause
// copyright-holders:Curt Coder
/******************************************************************************************

    Commodore C900
    UNIX prototype

    http://www.zimmers.net/cbmpics/c900.html
    http://www.zimmers.net/cbmpics/cbm/900/c900-chips.txt

    Chips: Z8001 CPU, Z8010 MMU, Z8030 SCC, Z8036 CIO. Crystal: 12MHz

    The Z8030 runs 2 serial ports. The Z8036 runs the IEEE interface and the speaker.

    The FDC is an intelligent device that communicates with the main board via the MMU.
    It has a 6508 CPU.

    Disk drive is a Matsushita JA-560-012

*******************************************************************************************/


#include "emu.h"
#include "cpu/z8000/z8000.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class c900_state : public driver_device
{
public:
	c900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_READ16_MEMBER(port1e_r);
	DECLARE_READ16_MEMBER(key_r);
	DECLARE_READ16_MEMBER(stat_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(c900_mem, AS_PROGRAM, 16, c900_state)
	AM_RANGE(0x00000, 0x07fff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(c900_data, AS_DATA, 16, c900_state)
	AM_RANGE(0x00000, 0x07fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x08000, 0x6ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(c900_io, AS_IO, 16, c900_state)
	AM_RANGE(0x0010, 0x0011) AM_READ(stat_r)
	AM_RANGE(0x001A, 0x001B) AM_READ(key_r)
	AM_RANGE(0x001E, 0x001F) AM_READ(port1e_r)
	AM_RANGE(0x0100, 0x0101) AM_READ(stat_r)
	AM_RANGE(0x0110, 0x0111) AM_READ(key_r) AM_DEVWRITE8(TERMINAL_TAG, generic_terminal_device, write, 0x00ff)
ADDRESS_MAP_END

static INPUT_PORTS_START( c900 )
INPUT_PORTS_END

READ16_MEMBER( c900_state::port1e_r )
{
	return 0;
}

READ16_MEMBER( c900_state::key_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ16_MEMBER( c900_state::stat_r )
{
	return (m_term_data) ? 6 : 4;
}

WRITE8_MEMBER( c900_state::kbd_put )
{
	m_term_data = data;
}

/* F4 Character Displayer */
static const gfx_layout c900_charlayout =
{
	8, 16,                   /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( c900 )
	GFXDECODE_ENTRY( "chargen", 0x0000, c900_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( c900, c900_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z8001, XTAL_12MHz / 2)
	MCFG_CPU_PROGRAM_MAP(c900_mem)
	MCFG_CPU_DATA_MAP(c900_data)
	MCFG_CPU_IO_MAP(c900_io)

	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(c900_state, kbd_put))
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", c900)
	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

ROM_START( c900 )
	ROM_REGION16_LE( 0x8000, "roms", 0 )
	ROM_LOAD16_BYTE( "c 900 boot-h v 1.0.bin.u17", 0x0001, 0x4000, CRC(c3aa7fc1) SHA1(ff12dd100fa7b1e7e931e9a8ef4c4f5cc056e099) )
	ROM_LOAD16_BYTE( "c 900 boot-l v 1.0.bin.u18", 0x0000, 0x4000, CRC(0aa39272) SHA1(b2c5da4586d38fc66bb33aafeae4dbda36080f1e) )

	ROM_REGION( 0x2000, "fdc", 0 )
	ROM_LOAD( "s41_6-20-85.bin", 0x0000, 0x2000, CRC(ec245721) SHA1(4cc19014b4887833a56b1236dc5fe39cc5d7b5c3) )

	ROM_REGION( 0x1000, "chargen", 0 ) // this must be for the c900 terminal as the mainframe has no video output
	ROM_LOAD( "380217-01.u2", 0x0000, 0x1000, CRC(64cb4171) SHA1(e60d796170addfd27e2c33090f9c512c7e3f99f5) )
ROM_END

/*    YEAR  NAME   PARENT  COMPAT  MACHINE INPUT   INIT COMPANY     FULLNAME        FLAGS */
COMP( 1985, c900,   0,      0,      c900,    c900, driver_device,    0, "Commodore", "Commodore 900", MACHINE_IS_SKELETON )
