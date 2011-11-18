/*

    CES Classic wall games

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m68000/m68000.h"

class cesclassic_state : public driver_device
{
public:
	cesclassic_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }
	
	DECLARE_READ16_MEMBER(_60000_r);
	
protected:
	
	// devices
	required_device<cpu_device> m_maincpu;
	
	// driver_device overrides
	virtual void video_start();
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);
};


void cesclassic_state::video_start()
{
}

bool cesclassic_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ16_MEMBER( cesclassic_state::_60000_r )
{
	return 0x100 | 0x200;
}

static ADDRESS_MAP_START( cesclassic_map, AS_PROGRAM, 16, cesclassic_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM
	AM_RANGE(0x600000, 0x600001) AM_READ(_60000_r)
	AM_RANGE(0x640040, 0x640041) AM_WRITENOP
	AM_RANGE(0x904000, 0x904001) AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( cesclassic )
INPUT_PORTS_END

static MACHINE_CONFIG_START( cesclassic, cesclassic_state )

	MCFG_CPU_ADD("maincpu", M68000, 24000000/2 )
	MCFG_CPU_PROGRAM_MAP(cesclassic_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MCFG_PALETTE_LENGTH(16)
MACHINE_CONFIG_END


ROM_START(hrclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("hrclassic-v121-u44.bin", 0x000000, 0x80000, CRC(cbbbbbdb) SHA1(d406a27a823f5e530a9cf7615c396fe52df1c387) )
	ROM_LOAD16_BYTE("hrclassic-v121-u43.bin", 0x000001, 0x80000, CRC(f136aec3) SHA1(7823e81eb79c7575c1d6c2ae0848c2b9943ee6ef) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "hrclassic-v100-u28.bin", 0x00000, 0x80000, CRC(45d15b3a) SHA1(a11ce27a77ea353034c5f498cb46ef5ed787b0f9) )
ROM_END

ROM_START(ccclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("ccclassic-v110-u44.bin", 0x000000, 0x80000, CRC(63b63f3a) SHA1(d4b6f401815b05ac0c6c259bb066663d4c2ee132) )
	ROM_LOAD16_BYTE("ccclassic-v110-u43.bin", 0x000001, 0x80000, CRC(c1b420df) SHA1(9f1f22e6b27abcede6880a1a8ad5399ce582dab1) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ccclassic-v100-u28.bin", 0x00000, 0x80000, CRC(94190a55) SHA1(fb219401431747fc3840da02c4e933d4c23049b7) )
ROM_END

ROM_START(tsclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("tsclassic-v100-u43u44.bin", 0x000000, 0x100000, CRC(a820ec9a) SHA1(84e38c7e54bb9e80142ed4e7763c9e36df560f42) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "tsclassic-v100-u28.bin", 0x00000, 0x80000, CRC(5bf53ca3) SHA1(5767391175fa9488ba0fb17a16de6d5013712a01) )
ROM_END


GAME(1996, hrclass, 0, cesclassic, cesclassic, 0, ROT0, "Creative Electronics And Software", "Home Run Classic", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME(1996, ccclass, 0, cesclassic, cesclassic, 0, ROT0, "Creative Electronics And Software", "Country Club Classic", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME(1996, tsclass, 0, cesclassic, cesclassic, 0, ROT0, "Creative Electronics And Software", "Trap Shoot Classic", GAME_NOT_WORKING | GAME_NO_SOUND )
