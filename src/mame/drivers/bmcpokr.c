/*

 similar to koftball.c?


An (unknown) gambling / poker game with sub-games
BMC 1999

PCB Layout
----------

BMC-A81212
|---------------------------------------|
|          CH-A-401  CH-M-301  CH-M-701 |
|M11B416256A                   UM3567   |
|42MHz     CH-M-201  CH-M-101     M6295 |
|      VDB40817                        1|
|              HM86171-80        VOL   0|
|      SYA70521                        W|
|                        LM324   7805  A|
|DSW1                                  Y|
|      68000                   TDA2003  |
|DSW2       CH-M-505  CH-M-605          |
|            6264     6264             2|
|DSW3  CPLD   74HC132 74LS05           2|
|                 555                  W|
|DSW4                                  A|
|        BATT  SW       JAMMA          Y|
|---------------------------------------|
Notes:
      RAM - M11B416256, 6264(x2)
      VDB40817/SYA70521 - Unknown QFP100
      CPLD - unknown PLCC44 chip labelled 'BMC B816140'
      BATT - 5.5 volt 0.047F super cap
      68000 @10.50MHz (42/4)
      M6295 @1.05MHz (42/40)
      UM3567 @3.50MHz (42/12)
      HSync - 15.440kHz
      VSync - 58.935Hz

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"

class bmcpokr_state : public driver_device
{
public:
	bmcpokr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_videoram(*this, "videoram")
		{ }

	DECLARE_READ16_MEMBER( bmcpokr_unk_r )
	{
		return space.machine().rand();
	}

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_videoram;
	virtual void video_start();
	UINT32 screen_update_bmcpokr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



static ADDRESS_MAP_START( bmcpokr_mem, AS_PROGRAM, 16, bmcpokr_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM

	AM_RANGE(0x2c0000, 0x2dffff) AM_RAM AM_SHARE("videoram")

	AM_RANGE(0x2FF800, 0x2FFFFF) AM_RAM
	AM_RANGE(0x34001A, 0x34001B) AM_READ(bmcpokr_unk_r)

ADDRESS_MAP_END

static INPUT_PORTS_START( bmcpokr )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW1:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW1:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW1:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW1:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW1:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW1:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW1:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW2:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW2:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DSW3:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW3:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW3:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW3:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW3:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW3:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW3:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW3:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, "DSW4:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW4:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW4:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW4:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW4:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW4:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW4:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW4:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3  },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( bmcpokr )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

UINT32 bmcpokr_state::screen_update_bmcpokr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = machine().gfx[0];

	int count = 0;
	for (int y=0;y<32;y++)
	{
		for (int x=0;x<64;x++)
		{
			UINT16 data = m_videoram[count];
			count++;

			drawgfx_opaque(bitmap,cliprect,gfx,data,0,0,0,x*8,y*8);

		}
	}



	return 0;
}

void bmcpokr_state::video_start()
{
}


static MACHINE_CONFIG_START( bmcpokr, bmcpokr_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_42MHz/4)
	MCFG_CPU_PROGRAM_MAP(bmcpokr_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bmcpokr_state, irq3_line_hold)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(bmcpokr_state, screen_update_bmcpokr)

	MCFG_GFXDECODE(bmcpokr)

	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_PALETTE_LENGTH(256)

MACHINE_CONFIG_END


ROM_START( bmcpokr )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ch-m-505.u12", 0x000001, 0x20000, CRC(d6effaf1) SHA1(b446d3beb3393bc8b3bcd0d543945e6fb6a375b9) )
	ROM_LOAD16_BYTE( "ch-m-605.u13", 0x000000, 0x20000, CRC(c5c3fcd1) SHA1(b77fef734c290d52ae877a24bb3ee42b24eb5cb8) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE(    "ch-m-101.u39", 0x000000, 0x80000, CRC(f4b82e0a) SHA1(f545c6ab1375518de06900f02a0eb5af1edeeb47) )
	ROM_LOAD16_BYTE(    "ch-m-201.u40", 0x000001, 0x80000, CRC(520571cb) SHA1(5c006f10d6192939003f8197e8bb64908a826fc1) )
	ROM_LOAD16_BYTE(    "ch-m-301.u45", 0x100000, 0x80000, CRC(daba09c3) SHA1(e5d2f92b63288c36faa367a3306d1999264843e8) )
	ROM_LOAD16_BYTE(    "ch-a-401.u29", 0x100001, 0x80000, CRC(5ee5d39f) SHA1(f6881aa5c755831d885f7adf35a5a094f7302205) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ch-m-701.u10", 0x00000, 0x40000,  CRC(e01be644) SHA1(b68682786d5b40cb5672cfd7f717adcfb8fac7d3) )
ROM_END




GAME( 1999, bmcpokr,    0, bmcpokr,    bmcpokr, driver_device,    0, ROT0,  "BMC", "unknown BMC poker game", GAME_NOT_WORKING | GAME_IS_SKELETON )
