// license:BSD-3-Clause
// copyright-holders: Scott Stone
/***************************************************************************

Magic Reels - Casino Slots
Play System

1x XILINX XCS30 PQ240CKN9825 A2015802A
1x MC68HC000FN8
1x XILINX XC9572 TQ100AEM9917
1x XILINX XC95108 TQ100AEM9909
1x XILINX XC9536 VQ44ASJ9917
2x GM76C8128CLLFW70
4x GM76C256CLLFW70
2x TD62083AP
1x TL7705ACP
1x HIN232CP

1x XTAL: 58.00000Mhz

UNDUMPED:
1x PIC16C621A  ==> PROTECTED

***************************************************************************/

#include "emu.h"
//#include "cpu/m68000/m68000.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK XTAL(58'000'000) // Actual Xtal on board

class magreel_state : public driver_device
{
public:
	magreel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}
	void magreel(machine_config &config);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


static INPUT_PORTS_START( magreel )
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


void magreel_state::machine_start()
{
}


void magreel_state::machine_reset()
{
}


MACHINE_CONFIG_START(magreel_state::magreel)

	/* basic machine hardware - all information unknown */
//  MCFG_DEVICE_ADD("maincpu",m68000,MAIN_CLOCK/12)
//  MCFG_DEVICE_PROGRAM_MAP(magreel_map)

	/* video hardware */
//  MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
//  MCFG_SCREEN_UPDATE_DRIVER(magreel_state, screen_update)
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
//  MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK/2, 442, 0, 320, 264, 0, 240)          /* generic NTSC video timing at 320x240 */
//  MCFG_SCREEN_PALETTE("palette")

//  GFXDECODE(config, "gfxdecode", "palette", gfx_magreel);

//  MCFG_PALETTE_ADD("palette", 8)
//  MCFG_PALETTE_INIT_OWNER(magreel_state, magreel)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
MACHINE_CONFIG_END


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( magreel ) // roms have not been looked at
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m27c160.ic3",  0x000000, 0x200000, CRC(707a835a) SHA1(4edbb2279298f330514512147166b9382c79861d) )
	ROM_LOAD( "m27c160.ic4",  0x200000, 0x200000, CRC(d5590a3c) SHA1(69dacb370b630fd7fee3ddd4beeb34a336dd2d16) )
	ROM_LOAD( "m27c160.ic5",  0x400000, 0x200000, CRC(72da9809) SHA1(19516432c4cfc33c3db20aab0c64fafb72ed1a19) )
	ROM_LOAD( "m27c160.ic6",  0x600000, 0x200000, CRC(0f3274d0) SHA1(1abb45ebc74a09f1832cf80775a35966e8d5cd84) )
	ROM_LOAD( "mx29f161.ic24",0x800000, 0x200000, CRC(61accab0) SHA1(0fee6bf6071849d1b00fbfc248ab654a8abc3b99) )
	ROM_LOAD( "m27c800ic18",  0xa00000, 0x100000, CRC(2af3d8e7) SHA1(729cd2c1011d8018cf8d77c2d118d1815e30f475) )
	ROM_LOAD( "m28c64.ic19",  0xb00000, 0x002000, CRC(d0238e5c) SHA1(513bb97487d33c3b844877104bb2af3220851583) )
	ROM_LOAD( "m28c64.ic20",  0xb02000, 0x002000, CRC(4e6abd42) SHA1(5b1741b755f0fddd94e16d41d5d39a03f37fb23b) )
ROM_END

GAME( 199?, magreel, 0, magreel, magreel, magreel_state, empty_init, ROT0, "Play System",      "Magic Reels", MACHINE_IS_SKELETON )
