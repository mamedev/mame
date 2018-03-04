// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Sharp FontWriter series

    Skeleton driver by R. Belmont

    Main CPU: ROMless Mitsubishi M37720
    FDC: NEC 72068 (entire PC controller on a chip)
    512k RAM
    Custom gate array
    640x400 dot-matrix LCD

    Things to check
    - Hook up 37720 DMAC, it's used before this dies
    - Check if "stack in bank FF" bit is used
    - Verify timer implementation

****************************************************************************/

#include "emu.h"
#include "cpu/m37710/m37710.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

class fontwriter_state : public driver_device
{
public:
	fontwriter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_reset() override;
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(vbl_r)
	{
		m_vbl ^= 0xff;
		return m_vbl;
	}
	void fontwriter(machine_config &config);
	void io_map(address_map &map);
	void main_map(address_map &map);
protected:

	// devices
	required_device<m37720s1_device> m_maincpu;

	// driver_device overrides
	virtual void video_start() override;
	uint8_t m_vbl;
};

void fontwriter_state::machine_reset()
{
	m_vbl = 0;
}

void fontwriter_state::machine_start()
{
}

void fontwriter_state::video_start()
{
}

uint32_t fontwriter_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

ADDRESS_MAP_START(fontwriter_state::main_map)
	AM_RANGE(0x002000, 0x007fff) AM_RAM
	AM_RANGE(0x008000, 0x00ffff) AM_ROM AM_REGION("maincpu", 0x0000)
	AM_RANGE(0x020000, 0x04ffff) AM_RAM
	AM_RANGE(0x100000, 0x1007ff) AM_RAM
	AM_RANGE(0x200000, 0x3fffff) AM_ROM AM_REGION("maincpu", 0x0000)
ADDRESS_MAP_END

ADDRESS_MAP_START(fontwriter_state::io_map)
	AM_RANGE(M37710_PORT6, M37710_PORT6) AM_READ(vbl_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( fontwriter )
INPUT_PORTS_END

MACHINE_CONFIG_START(fontwriter_state::fontwriter)
	MCFG_CPU_ADD("maincpu", M37720S1, XTAL(16'000'000)) /* M37720S1 @ 16MHz - main CPU */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(fontwriter_state, screen_update)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 399)
MACHINE_CONFIG_END

ROM_START(fw700ger)
	ROM_REGION(0x200000, "maincpu", 0)       /* M37720 program ROM */
	ROM_LOAD( "lh5370pd.ic7", 0x000000, 0x200000, CRC(29083e13) SHA1(7e1605f91b53580e75f638f9e6b0917305c35f84) )
ROM_END

SYST( 1994, fw700ger, 0, 0, fontwriter, fontwriter, fontwriter_state, 0, "Sharp", "FontWriter FW-700 (German)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
