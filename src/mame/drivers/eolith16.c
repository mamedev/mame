/********************************************************************

 Eolith 16 bits hardware

 Supported Games:
 - KlonDike+      (c) 1999 Eolith

 driver by Pierpaolo Prazzoli

**********************************************************************/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "includes/eolithsp.h"


class eolith16_state : public driver_device
{
public:
	eolith16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_vram;
	int m_vbuffer;
};



// It's configured for 512 bytes
static const eeprom_interface eeprom_interface_93C66 =
{
	9,				// address bits 9
	8,				// data bits    8
	"*110",			// read         110 aaaaaaaaa
	"*101",			// write        101 aaaaaaaaa dddddddd
	"*111",			// erase        111 aaaaaaaaa
	"*10000xxxxxx",	// lock         100 00xxxxxxx
	"*10011xxxxxx"	// unlock       100 11xxxxxxx
};

static WRITE16_HANDLER( eeprom_w )
{
	eolith16_state *state = space->machine().driver_data<eolith16_state>();
	state->m_vbuffer = (data & 0x80) >> 7;
	coin_counter_w(space->machine(), 0, data & 1);

	input_port_write(space->machine(), "EEPROMOUT", data, 0xff);

	//data & 0x100 and data & 0x004 always set
}

static READ16_HANDLER( eolith16_custom_r )
{
	eolith_speedup_read(space);
	return input_port_read(space->machine(), "SPECIAL");
}



static WRITE16_HANDLER( vram_w )
{
	eolith16_state *state = space->machine().driver_data<eolith16_state>();
	COMBINE_DATA(&state->m_vram[offset + (0x10000/2) * state->m_vbuffer]);
}

static READ16_HANDLER( vram_r )
{
	eolith16_state *state = space->machine().driver_data<eolith16_state>();
	return state->m_vram[offset + (0x10000/2) * state->m_vbuffer];
}

static ADDRESS_MAP_START( eolith16_map, AS_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM
	AM_RANGE(0x50000000, 0x5000ffff) AM_READWRITE(vram_r, vram_w)
	AM_RANGE(0x90000000, 0x9000002f) AM_WRITENOP //?
	AM_RANGE(0xff000000, 0xff1fffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0xffe40000, 0xffe40001) AM_DEVREADWRITE8_MODERN("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0xffe80000, 0xffe80001) AM_WRITE(eeprom_w)
	AM_RANGE(0xffea0000, 0xffea0001) AM_READ(eolith16_custom_r)
	AM_RANGE(0xffea0002, 0xffea0003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xffec0000, 0xffec0001) AM_READNOP // not used?
	AM_RANGE(0xffec0002, 0xffec0003) AM_READ_PORT("INPUTS")
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( eolith16 )
	PORT_START("SPECIAL")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(eolith_speedup_getvblank, NULL)
	PORT_BIT( 0xff6f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
INPUT_PORTS_END

static VIDEO_START( eolith16 )
{
	eolith16_state *state = machine.driver_data<eolith16_state>();
	state->m_vram = auto_alloc_array(machine, UINT16, 0x10000);
}

static SCREEN_UPDATE( eolith16 )
{
	eolith16_state *state = screen->machine().driver_data<eolith16_state>();
	int x,y,count;
	int color;

	count = 0;
	for (y=0;y < 204;y++)
	{
		for (x=0;x < 320/2;x++)
		{
			color = state->m_vram[count + (0x10000/2) * (state->m_vbuffer ^ 1)] & 0xff;
			*BITMAP_ADDR16(bitmap, y, x*2 + 0) = color;

			color = (state->m_vram[count + (0x10000/2) * (state->m_vbuffer ^ 1)] & 0xff00) >> 8;
			*BITMAP_ADDR16(bitmap, y, x*2 + 1) = color;

			count++;
		}
	}
	return 0;
}


// setup a custom palette because pixels use 8 bits per color
static PALETTE_INIT( eolith16 )
{
	int c;

	for (c = 0; c < 256; c++)
	{
		int bit0,bit1,bit2,r,g,b;
		bit0 = (c >> 0) & 0x01;
		bit1 = (c >> 1) & 0x01;
		bit2 = (c >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (c >> 3) & 0x01;
		bit1 = (c >> 4) & 0x01;
		bit2 = (c >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (c >> 6) & 0x01;
		bit1 = (c >> 7) & 0x01;
		b = 0x55 * bit0 + 0xaa * bit1;

		palette_set_color(machine,c,MAKE_RGB(r,g,b));
	}
}



static MACHINE_CONFIG_START( eolith16, eolith16_state )
	MCFG_CPU_ADD("maincpu", E116T, 60000000)		/* no internal multiplier */
	MCFG_CPU_PROGRAM_MAP(eolith16_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", eolith_speedup, "screen", 0, 1)

	MCFG_EEPROM_ADD("eeprom", eeprom_interface_93C66)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 199)
	MCFG_SCREEN_UPDATE(eolith16)

	MCFG_PALETTE_LENGTH(256)

	MCFG_PALETTE_INIT(eolith16)
	MCFG_VIDEO_START(eolith16)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

/*

KlonDike+
Eolith, 1999

This game is like Freecell which comes with Windows XP etc

PCB Layout
----------

9812C
|-------------------------------------------|
|TDA1519A      KD.U28  PAL        RESET_SW  |
|     VOL  M6295   1MHz                60MHz|
|               EV0514-001                  |
|        14.31818MHz            E1-16T      |
|                                           |
|                          GM71C18160       |
|   TEST_SW                                 |
|J                                          |
|A                  KM6161002               |
|M  SERV_SW                                 |
|M                              KD.U5       |
|A                                          |
|                               93C66       |
|                                           |
|                                           |
|                                     TL7705|
|                                           |
|    *    *    *    *    *    KD.U31        |
|                                           |
|-------------------------------------------|
Notes:
      E1-16T - Main CPU, HyperStone E1-16T, clock input 60.000MHz (TQFP100)
      M6294  - Oki M6295 sound chip, clock 1.000MHz, sample rate = 1000000 / 132 (QFP44)
      93C66  - Atmel 93C66 4096bit Serial EEPROM (DIP8)
   KM6161002 - Samsung Electronics KM6161002BJ-10 64k x16 High Speed CMOS Static RAM (SOJ44)
  GM71C18160 - LG Semiconductor GM71C18160CJ6 1M x16 DRAM (SOJ42)
  EV0514-001 - Custom Eolith IC (QFP100)
      VSync  - 60Hz
      HSync  - 15.64kHz
          *  - Empty DIP42 sockets
       ROMs  -
               KD.U28 - TMS27C040 EPROM, M6295 samples (DIP32)
               KD.U5  - TMS27C040 EPROM, Main Program (DIP32)
               KD.U31 - ST M27C160 EPROM, Graphics Data (DIP42)
*/

ROM_START( klondkp )
	ROM_REGION16_BE( 0x80000, "user1", 0 ) /* E1-16T program code */
	ROM_LOAD( "kd.u5",  0x000000, 0x080000, CRC(591f0c73) SHA1(a9f338204c77a724fa6a6e08d78ca89bd5191aba) )

	ROM_REGION16_BE( 0x200000, "user2", 0 ) /* gfx data */
	ROM_LOAD16_WORD_SWAP( "kd.u31", 0x000000, 0x200000, CRC(e5dd12b5) SHA1(0a0cd75cbcdccce3575e5a58ba09c88452e1a5ee) )

	ROM_REGION( 0x80000, "oki", 0 ) /* oki samples */
	ROM_LOAD( "kd.u28", 0x000000, 0x080000, CRC(c12112a1) SHA1(729bbaca6db933a730099a4a560a10ed99cae1c3) )
ROM_END

static DRIVER_INIT( eolith16 )
{
	init_eolith_speedup(machine);
}

GAME( 1999, klondkp, 0, eolith16, eolith16, eolith16, ROT0, "Eolith", "KlonDike+", 0 )
