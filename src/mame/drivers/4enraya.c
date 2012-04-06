/***************************************************************************

Driver by Tomasz Slanina

TODO:
- video and irq timings;
- there's a waitstate penalty on the VRAM apparently?

- unknown Pac-Man gambling game (whats the real title?) sound issues

***************************************************************************

RAM :
    1 x GM76c28-10 (6116) RAM
    3 x 2114  - VRAM (only 10 bits are used )

ROM:
  27256 + 27128 for code/data
  3x2764 for gfx

PROM:
 82S123 32x8
 Used for system control
    (d0 - connected to ROM5 /CS , d1 - ROM4 /CS, d2 - RAM /CS , d3 - to some logic(gfx control), and Z80 WAIT )

Memory Map :
  0x0000 - 0xbfff - ROM
  0xc000 - 0xcfff - RAM
  0xd000 - 0xdfff - VRAM mirrored write,
        tilemap offset = address & 0x3ff
        tile number =  bits 0-7 = data, bits 8,9  = address bits 10,11
  0xe000 - 0xefff - VRAM mirror
  0xf000 - 0xffff - (unconnected)

Video :
    No scrolling , no sprites.
    32x32 Tilemap stored in VRAM (10 bits/tile (tile numebr 0-1023))

    3 gfx ROMS
    ROM1 - R component (ROM ->(parallel in) shift register 74166 (serial out) -> jamma output
    ROM2 - G component
    ROM3 - B component

Sound :
 AY 3 8910

 sound_control :

  bit 0 - BC1
  bit 1 - BC2
  bit 2 - BDIR

  bits 3-7 - not connected

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "includes/4enraya.h"

#define MAIN_CLOCK XTAL_8MHz

WRITE8_MEMBER(_4enraya_state::sound_data_w)
{
	m_soundlatch = data;
}

static WRITE8_DEVICE_HANDLER( sound_control_w )
{
	_4enraya_state *state = device->machine().driver_data<_4enraya_state>();

	if ((state->m_last_snd_ctrl & state->m_snd_latch_bit ) == state->m_snd_latch_bit && (data & state->m_snd_latch_bit) == 0x00)
		ay8910_data_address_w(device, state->m_last_snd_ctrl, state->m_soundlatch);

	state->m_last_snd_ctrl = data;
}

READ8_MEMBER(_4enraya_state::fenraya_custom_map_r)
{
	UINT8 *prom = machine().region("pal_prom")->base();
	UINT8 prom_routing = (prom[offset >> 12] & 0xf) ^ 0xf;
	UINT8 res;

	res = 0;

	if(prom_routing & 1) //ROM5
	{
		UINT8 *rom = machine().region("maincpu")->base();
		res |= rom[offset & 0x7fff];
	}

	if(prom_routing & 2) //ROM4
	{
		UINT8 *rom = machine().region("maincpu")->base();
		res |= rom[(offset & 0x7fff) | 0x8000];
	}

	if(prom_routing & 4) //RAM
	{
		res |= m_workram[offset & 0xfff];
	}

	if(prom_routing & 8) //gfx control / RAM wait
	{
		res |= m_videoram[offset & 0xfff];
	}

	return res;
}

WRITE8_MEMBER(_4enraya_state::fenraya_custom_map_w)
{
	UINT8 *prom = machine().region("pal_prom")->base();
	UINT8 prom_routing = (prom[offset >> 12] & 0xf) ^ 0xf;

	if(prom_routing & 1) //ROM5
	{
		// ...
	}

	if(prom_routing & 2) //ROM4
	{
		// ...
	}

	if(prom_routing & 4) //RAM
	{
		m_workram[offset & 0xfff] = data;
	}

	if(prom_routing & 8) //gfx control / RAM wait
	{
		fenraya_videoram_w(space,offset & 0xfff,data);
	}
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, _4enraya_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(fenraya_custom_map_r,fenraya_custom_map_w)
	#if 0
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_WRITE(fenraya_videoram_w) AM_BASE_SIZE(m_videoram, m_videoram_size)
	AM_RANGE(0xe000, 0xefff) AM_WRITE(fenraya_videoram_w)
	AM_RANGE(0xf000, 0xffff) AM_NOP
	#endif
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, AS_IO, 8, _4enraya_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("INPUTS")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x23, 0x23) AM_WRITE(sound_data_w)
	AM_RANGE(0x33, 0x33) AM_DEVWRITE_LEGACY("aysnd", sound_control_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( unkpacg_main_map, AS_PROGRAM, 8, _4enraya_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x7000, 0x7fff) AM_WRITE(fenraya_videoram_w) AM_BASE_SIZE(m_videoram, m_videoram_size)
	AM_RANGE(0x8000, 0x9fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( unkpacg_main_portmap, AS_IO, 8, _4enraya_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x20, 0x20) AM_WRITE(sound_data_w)
	AM_RANGE(0x30, 0x30) AM_DEVWRITE_LEGACY("aysnd", sound_control_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( 4enraya )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Pieces" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPNAME( 0x08, 0x08, "Speed" )					PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Down")			// "drop" ("down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	PORT_NAME("P2 Down")			// "drop" ("down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Shot")			// "fire" ("shot")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	PORT_NAME("P2 Shot")			// "fire" ("shot")

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( unkpacg )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "0-0")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_DIPNAME( 0x02, 0x00, "0-1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x00, "0-2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )

	PORT_DIPNAME( 0x08, 0x00, "0-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

	PORT_DIPNAME( 0x10, 0x00, "0-4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_DIPNAME( 0x20, 0x00, "0-5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )

	PORT_DIPNAME( 0x40, 0x00, "0-6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_DIPNAME( 0x80, 0x00, "0-7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) //  1 credits / initiate minigame
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) //  5 credits / initiate gambling
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) // 10 credits
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_DIPNAME( 0x80, 0x80, "DIP8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(1,3), RGN_FRAC(2,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( 4enraya )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END

static MACHINE_START( 4enraya )
{
	_4enraya_state *state = machine.driver_data<_4enraya_state>();

	state->save_item(NAME(state->m_soundlatch));
	state->save_item(NAME(state->m_last_snd_ctrl));
}

static MACHINE_RESET( 4enraya )
{
	_4enraya_state *state = machine.driver_data<_4enraya_state>();

	state->m_soundlatch = 0;
	state->m_last_snd_ctrl = 0;
}

static PALETTE_INIT( 4enraya )
{
	int i;

	/* RGB format */
	for(i=0;i<8;i++)
		palette_set_color(machine, i, MAKE_RGB(pal1bit(i >> 0),pal1bit(i >> 1),pal1bit(i >> 2)));
}

static MACHINE_CONFIG_START( 4enraya, _4enraya_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_PERIODIC_INT(irq0_line_hold,4*60) // unknown timing

	MCFG_MACHINE_START(4enraya)
	MCFG_MACHINE_RESET(4enraya)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(4enraya)

	MCFG_GFXDECODE(4enraya)

	MCFG_PALETTE_INIT(4enraya)
	MCFG_PALETTE_LENGTH(8)

	MCFG_VIDEO_START(4enraya)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4) /* guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( unkpacg, 4enraya )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(unkpacg_main_map)
	MCFG_CPU_IO_MAP(unkpacg_main_portmap)
	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 4enraya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x8000, CRC(cf1cd151) SHA1(3920b0a6ed5798859158871b578b01ec742b0d13) )
	ROM_LOAD( "4.bin",   0x8000, 0x4000, CRC(f9ec1be7) SHA1(189159129ecbc4f6909c086867b0e02821f5b976) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(87f92552) SHA1(d16afd963c30f2e60951876b843e5c1dcbee1cfc) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(2b0a3793) SHA1(2c3d224251557824bb9641dc2f98a000ab72c4a2) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(f6940836) SHA1(afde21ffa0c141cf73243e50da62ecfd474aaac2) )

	ROM_REGION( 0x0020,  "pal_prom", 0 )
	ROM_LOAD( "1.bpr",   0x0000, 0x0020, CRC(dcbd2352) SHA1(ce72e84129ed1b455aaf648e1dfaa4333e7e7628) )	/* system control: used for memory mapping */
ROM_END


ROM_START(unkpacg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1.u14", 0x00000, 0x2000, CRC(848c4143) SHA1(3cff26181c58e5f52f1ac81df7d5d43e644585a2))
	ROM_LOAD("2.u46", 0x08000, 0x2000, CRC(9e6e0bd3) SHA1(f502132a0460108dad243632cc13d9116c534291))

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "3.u20",   0x2000, 0x2000, CRC(d00b04ea) SHA1(e65901d8586507257d74ab103001207e28fa28af) )
	ROM_LOAD( "4.u19",   0x4000, 0x2000, CRC(4a123a3d) SHA1(26300b8af0d0df0023a153a212699727311d1b74) )
	ROM_LOAD( "5.u18",   0x0000, 0x2000, CRC(44f272d2) SHA1(b39cbc1f290d9fb2453396906e4da4a682c41ef4) )
ROM_END

static DRIVER_INIT( unkpacg )
{
	_4enraya_state *state = machine.driver_data<_4enraya_state>();
	UINT8 *rom = machine.region("maincpu")->base();

	state->m_snd_latch_bit = 2;

	{
		for(int i=0x8000;i<0xa000;++i)
		{
			rom[i]=BITSWAP8(rom[i], 7,6,5,4,3,2,0,1);
		}
	}
}

GAME( 1990, 4enraya,  0,   4enraya,  4enraya,  0,       ROT0, "IDSA",      "4 En Raya", GAME_SUPPORTS_SAVE )
GAME( 199?, unkpacg,  0,   unkpacg,  unkpacg,  unkpacg, ROT0, "<unknown>", "unknown Pac-Man gambling game", GAME_IMPERFECT_SOUND )
