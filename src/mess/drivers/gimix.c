// license:MAME
// copyright-holders:
/*
    Gimix 6809-Based Computers

    Representative group of GIMIX users included:  Government Research and Scientific Organizations, Universities, Industrial, Computer Mainframe and
    Peripheral Manufacturers and Software Houses.

    This system is most notable for being the development base of the "Vid Kidz", a pair of programmers (Eugene Jarvis and Larry DeMar) who formerly
    worked for Williams Electronics on the game, Defender.  They left Willams and continued work on other games eventually making a deal with Williams
    to continue to design games producing the titles: Stargate, Robotron: 2084 and Blaster.

    Information Link:  http://www.backglass.org/duncan/gimix/

    TODO:  Everything
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/terminal.h"


class gimix_state : public driver_device
{
public:
	gimix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_READ8_MEMBER(status_r);

private:
	UINT8 m_term_data;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START( gimix_mem, AS_PROGRAM, 8, gimix_state )
	AM_RANGE(0x0000, 0xdfff) AM_RAM
	AM_RANGE(0xe004, 0xe004) AM_READ(status_r)
	AM_RANGE(0xe005, 0xe005) AM_READ(keyin_r) AM_DEVWRITE("terminal", generic_terminal_device, write)
	AM_RANGE(0xe400, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION("maincpu", 0xf800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gimix_io, AS_IO, 8, gimix_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( gimix )
INPUT_PORTS_END

READ8_MEMBER( gimix_state::keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( gimix_state::status_r )
{
	return (m_term_data) ? 3 : 2;
}

WRITE8_MEMBER( gimix_state::kbd_put )
{
	m_term_data = data;
}

void gimix_state::machine_reset()
{
	m_term_data = 0;
}

static MACHINE_CONFIG_START( gimix, gimix_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M6809, 2000000)
	MCFG_CPU_PROGRAM_MAP(gimix_mem)
	MCFG_CPU_IO_MAP(gimix_io)

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(gimix_state, kbd_put))
MACHINE_CONFIG_END

ROM_START( gimix )
	ROM_REGION( 0x10000, "maincpu", 0)

/* CPU board U5: gimixv14.bin - checksum 97E2 - 2716 - GIMIX 6809 | AUTOBOOT | V1.4 I2716 */
        ROM_LOAD( "gimixv14.u5", 0x000000, 0x000800, CRC(f795b8b9) SHA1(eda2de51cc298d94b36605437d900ce971b3b276) )

/* CPU board U4: gimixf8.bin  - checksum 68DB - 2716 - GMXBUG09 V2.1 | (c)1981 GIMIX | $F800 I2716 */
	ROM_LOAD( "gimixf8.u4",  0x00f800, 0x000800, CRC(7d60f838) SHA1(eb7546e8bbf50d33e181f3e86c3e4c5c9032cab2) )

/* CPU board U6: os9l1v11.bin - checksum 2C84 - 2716 - OS-9tmL1 V1 | GIMIX P1 " (c)1982 MSC
   CPU board U7: os9l1v12.bin - checksum 7694 - 2716 - OS-9tmL1 V1 | GIMIX P2-68 | (c)1982 MSC */

        ROM_LOAD( "os9l1v11.u6", 0x002000, 0x000800, CRC(0d6527a0) SHA1(1435a22581c6e9e0ae338071a72eed646f429530) )
        ROM_LOAD( "os9l1v12.u7", 0x003000, 0x000800, CRC(b3c65feb) SHA1(19d1ea1e84473b25c95cbb8449e6b9828567e998) )

/* Hard drive controller board 2 (XEBEC board) 11H: gimixhd.bin - checksum 2436 - 2732 - 104521D */

        ROM_LOAD( "gimixhd.h11",  0x004000, 0x001000, CRC(35c12201) SHA1(51ac9052f9757d79c7f5bd3aa5d8421e98cfcc37) )
ROM_END

COMP( 1980, gimix,    0,      0,      gimix,        gimix, driver_device, 0,      "Gimix",  "Gimix 6809 System",  GAME_IS_SKELETON | GAME_NOT_WORKING | GAME_NO_SOUND )
