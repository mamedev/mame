
#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"
#include "imagedev/cartslot.h"

// overclocked to 8 * NTSC burst frequency

#define VERBOSE_LEVEL	(0)

#define ENABLE_VERBOSE_LOG (0)

#if ENABLE_VERBOSE_LOG
INLINE void verboselog(running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", machine.device("maincpu")->safe_pc(), buf );
	}
}
#else
#define verboselog(x,y,z,...)
#endif

#define MASTER_CLOCK 28618180

class uzebox_state : public driver_device
{
public:
	uzebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu")
	{
	}

	virtual void machine_start();

	required_device<avr8_device> m_maincpu;

	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(port_w);
	DECLARE_DRIVER_INIT(uzebox);
	virtual void machine_reset();
	UINT32 screen_update_uzebox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void uzebox_state::machine_start()
{
}

READ8_MEMBER(uzebox_state::port_r)
{
	return 0;
}

WRITE8_MEMBER(uzebox_state::port_w)
{
}

/****************************************************\
* Address maps                                       *
\****************************************************/

static ADDRESS_MAP_START( uzebox_prg_map, AS_PROGRAM, 8, uzebox_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM // 64 KB internal eprom  ATmega644
ADDRESS_MAP_END

static ADDRESS_MAP_START( uzebox_data_map, AS_DATA, 8, uzebox_state )
	AM_RANGE(0x0100, 0x10ff) AM_RAM //  4KB RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( uzebox_io_map, AS_IO, 8, uzebox_state )
	AM_RANGE(0x00, 0x03) AM_READWRITE( port_r, port_w )
ADDRESS_MAP_END

/****************************************************\
* Input ports                                        *
\****************************************************/

static INPUT_PORTS_START( uzebox )
INPUT_PORTS_END

/****************************************************\
* Video hardware                                     *
\****************************************************/

UINT32 uzebox_state::screen_update_uzebox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/****************************************************\
* Machine definition                                 *
\****************************************************/

DRIVER_INIT_MEMBER(uzebox_state,uzebox)
{
}

void uzebox_state::machine_reset()
{
}

const avr8_config atmega644_config =
{
	"eeprom"
};

static MACHINE_CONFIG_START( uzebox, uzebox_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ATMEGA644, MASTER_CLOCK)
	MCFG_CPU_AVR8_CONFIG(atmega644_config)
	MCFG_CPU_PROGRAM_MAP(uzebox_prg_map)
	MCFG_CPU_DATA_MAP(uzebox_data_map)
	MCFG_CPU_IO_MAP(uzebox_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.08)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1395))
	MCFG_SCREEN_SIZE(634, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 633, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(uzebox_state, screen_update_uzebox)

	MCFG_PALETTE_LENGTH(0x1000)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("avr8")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(0, "avr8", 1.00)

	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("uzebox")
	MCFG_SOFTWARE_LIST_ADD("eprom_list","uzebox")
MACHINE_CONFIG_END

ROM_START( uzebox )
	ROM_REGION( 0x10000, "maincpu", 0 )  /* Main program store */
	ROM_CART_LOAD("cart1", 0x0000, 0x10000, ROM_OPTIONAL)

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASE00 )  /* on-die eeprom */
ROM_END

/*   YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY   FULLNAME */
CONS(2010, uzebox,   0,        0,        uzebox,   uzebox, uzebox_state,   uzebox,  "Belogic", "Uzebox", GAME_NO_SOUND | GAME_NOT_WORKING)
