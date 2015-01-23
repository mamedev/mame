// license: MAME
// copyright-holders: Angelo Salese
/***************************************************************************

C=65 / C=64DX (c) 1991 Commodore

Attempt at rewriting the driver ...

Note:
- VIC-4567 will be eventually be added via compile switch, once that I
  get the hang of the system (and checking where the old code fails 
  eventually)

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m4510.h"

#define MAIN_CLOCK XTAL_3_5MHz

class c65_state : public driver_device
{
public:
	c65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_screen(*this, "screen")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	DECLARE_READ8_MEMBER(vic4567_dummy_r);
	DECLARE_WRITE8_MEMBER(vic4567_dummy_w);

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(c65);
	DECLARE_DRIVER_INIT(c65);
	DECLARE_DRIVER_INIT(c65pal);
protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};

void c65_state::video_start()
{
}

UINT32 c65_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

READ8_MEMBER(c65_state::vic4567_dummy_r)
{
	UINT8 res;

	res=0xff;
	switch(offset)
	{
		case 0x11:
			res = (m_screen->vpos() & 0x100) >> 1;
			return res;
		case 0x12:
			res = (m_screen->vpos() & 0xff);
			return res;
	}
	
	printf("%02x\n",offset); // TODO: PC
	return res;
}

WRITE8_MEMBER(c65_state::vic4567_dummy_w)
{
	switch(offset)
	{
		/* KEY register, handles vic-iii and vic-ii modes via two consecutive writes 
		  0xa5 -> 0x96 vic-iii mode
          any other write vic-ii mode
		  */
		//case 0x2f: break;
		default: printf("%02x %02x\n",offset,data); break;
	}

}

static ADDRESS_MAP_START( c65_map, AS_PROGRAM, 8, c65_state )
	AM_RANGE(0x00000, 0x01fff) AM_RAM // TODO: bank
	AM_RANGE(0x0c800, 0x0cfff) AM_ROM AM_REGION("maincpu", 0xc800)
	AM_RANGE(0x0d000, 0x0d07f) AM_READWRITE(vic4567_dummy_r,vic4567_dummy_w)
	// 0x0d000, 0x0d07f VIC-4567
	// 0x0d080, 0x0d09f FDC
	// 0x0d0a0, 0x0d0ff Ram Expansion Control (REC)
	// 0x0d100, 0x0d1ff Red Palette
	// 0x0d200, 0x0d2ff Green Palette
	// 0x0d300, 0x0d3ff Blue Palette
	// 0x0d400, 0x0d4*f Right SID
	// 0x0d440, 0x0d4*f Left  SID
	// 0x0d600, 0x0d6** UART
	// 0x0d700, 0x0d7** DMAgic
	// 0x0d800, 0x0d8** Color matrix
	// 0x0dc00, 0x0dc** CIA-1
	// 0x0dd00, 0x0dd** CIA-2
	// 0x0de00, 0x0de** Ext I/O Select 1
	// 0x0df00, 0x0df** Ext I/O Select 2
	AM_RANGE(0x20000, 0x3ffff) AM_ROM AM_REGION("maincpu",0)
ADDRESS_MAP_END



static INPUT_PORTS_START( c65 )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void c65_state::machine_start()
{
}

void c65_state::machine_reset()
{
}


PALETTE_INIT_MEMBER(c65_state, c65)
{
}

static MACHINE_CONFIG_START( c65, c65_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M4510,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(c65_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(c65_state, screen_update)
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK, 525, 0, 320, 525, 0, 240) // mods needed
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(c65_state, c65)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( c65 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "910111", "V0.9.910111" )
	ROMX_LOAD( "910111.bin", 0x0000, 0x20000, CRC(c5d8d32e) SHA1(71c05f098eff29d306b0170e2c1cdeadb1a5f206), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "910523", "V0.9.910523" )
	ROMX_LOAD( "910523.bin", 0x0000, 0x20000, CRC(e8235dd4) SHA1(e453a8e7e5b95de65a70952e9d48012191e1b3e7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "910626", "V0.9.910626" )
	ROMX_LOAD( "910626.bin", 0x0000, 0x20000, CRC(12527742) SHA1(07c185b3bc58410183422f7ac13a37ddd330881b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "910828", "V0.9.910828" )
	ROMX_LOAD( "910828.bin", 0x0000, 0x20000, CRC(3ee40b06) SHA1(b63d970727a2b8da72a0a8e234f3c30a20cbcb26), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "911001", "V0.9.911001" )
	ROMX_LOAD( "911001.bin", 0x0000, 0x20000, CRC(0888b50f) SHA1(129b9a2611edaebaa028ac3e3f444927c8b1fc5d), ROM_BIOS(5) )
ROM_END

ROM_START( c64dx )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "910429.bin", 0x0000, 0x20000, CRC(b025805c) SHA1(c3b05665684f74adbe33052a2d10170a1063ee7d) )
ROM_END

DRIVER_INIT_MEMBER(c65_state,c65)
{
//	m_dma.version = 2;
//	c65_common_driver_init();
}

DRIVER_INIT_MEMBER(c65_state,c65pal)
{
//	m_dma.version = 1;
//	c65_common_driver_init();
//	m_pal = 1;
}

COMP( 1991, c65,    0,      0,      c65,    c65, c65_state, c65,    "Commodore Business Machines",  "Commodore 65 Development System (Prototype, NTSC)", GAME_NOT_WORKING )
COMP( 1991, c64dx,  c65,    0,      c65,    c65, c65_state, c65pal, "Commodore Business Machines",  "Commodore 64DX Development System (Prototype, PAL, German)", GAME_NOT_WORKING )
