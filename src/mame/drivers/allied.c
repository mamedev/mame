#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6502/m6502.h"

class allied_state : public driver_device
{
public:
	allied_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( allied_map, AS_PROGRAM, 8, allied_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1400, 0x1fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( allied )
INPUT_PORTS_END

void allied_state::machine_reset()
{
}

static DRIVER_INIT( allied )
{
}

static MACHINE_CONFIG_START( allied, allied_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6504, 3572549/4)
	MCFG_CPU_PROGRAM_MAP(allied_map)
MACHINE_CONFIG_END


ROM_START( allied )
    ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "alliedu5.bin", 0x1400, 0x0400, CRC(e4fb64fb) SHA1(a3d9de7cbfb42180a860e0bbbeaeba96d8bd1e20))
	ROM_LOAD( "alliedu6.bin", 0x1800, 0x0400, CRC(dca980dd) SHA1(3817d75413854d889fc1ce4fd6a51d820d1e0534))
	ROM_LOAD( "alliedu3.bin", 0x1c00, 0x0400, CRC(13f42789) SHA1(baa0f73fda08a3c5d6f1423fb329e4febb07ef97))
ROM_END

#define rom_suprpick    rom_allied
#define rom_royclark    rom_allied
#define rom_thndbolt    rom_allied
#define rom_hoedown     rom_allied
#define rom_takefive    rom_allied
#define rom_heartspd    rom_allied
#define rom_foathens    rom_allied
#define rom_disco79     rom_allied
#define rom_erosone     rom_allied
#define rom_circa33     rom_allied
#define rom_starshot    rom_allied


GAME(1977,	allied, 	0,			allied, allied, allied, ROT0,	"Allied Leisure",				"Allied System",				GAME_IS_BIOS_ROOT)
GAME(1977,	suprpick,	allied,		allied,	allied,	allied,	ROT0,	"Allied Leisure",				"Super Picker",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1977,	royclark,	allied,		allied,	allied,	allied,	ROT0,	"Fascination Int.",				"Roy Clark - The Entertainer",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1977,	thndbolt,	allied,		allied,	allied,	allied,	ROT0,	"Allied Leisure",				"Thunderbolt",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	hoedown,	allied,		allied,	allied,	allied,	ROT0,	"Allied Leisure",				"Hoe Down",						GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	takefive,	allied,		allied,	allied,	allied,	ROT0,	"Allied Leisure",				"Take Five",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	heartspd,	allied,		allied,	allied,	allied,	ROT0,	"Allied Leisure",				"Hearts & Spades",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1978,	foathens,	allied,		allied,	allied,	allied,	ROT0,	"Allied Leisure",				"Flame of Athens",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	disco79,	allied,		allied,	allied,	allied,	ROT0,	"Allied Leisure",				"Disco '79",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	erosone,	allied,		allied,	allied,	allied,	ROT0,	"Fascination Int.",				"Eros One",						GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	circa33,	allied,		allied,	allied,	allied,	ROT0,	"Fascination Int.",				"Circa 1933",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1979,	starshot,	allied,		allied,	allied,	allied,	ROT0,	"Allied Leisure",				"Star Shooter",					GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
