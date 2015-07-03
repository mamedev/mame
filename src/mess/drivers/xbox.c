// license:BSD-3-Clause
// copyright-holders:?
/***************************************************************************

	XBOX (c) 2001 Microsoft

	Skeleton driver
	
***************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
//#include "machine/lpci.h"
//#include "machine/pic8259.h"
//#include "machine/pit8253.h"
//#include "machine/idectrl.h"
//#include "machine/idehd.h"
//#include "machine/naomigd.h"
//#include "video/poly.h"
//#include "bitmap.h"
//#include "debug/debugcon.h"
//#include "debug/debugcmd.h"
//#include "debug/debugcpu.h"
//#include "includes/chihiro.h"


#define CPU_DIV 64

/*!
  @todo - Inheritance with chihiro_state
  */
class xbox_state : public driver_device
{
public:
	xbox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(xbox);
protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};

void xbox_state::video_start()
{
}

UINT32 xbox_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

static ADDRESS_MAP_START(xbox_map, AS_PROGRAM, 32, xbox_state)
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM // 128 megabytes
	AM_RANGE(0xf0000000, 0xf0ffffff) AM_RAM
//	AM_RANGE(0xfd000000, 0xfdffffff) AM_RAM AM_READWRITE(geforce_r, geforce_w)
//	AM_RANGE(0xfed00000, 0xfed003ff) AM_READWRITE(usbctrl_r, usbctrl_w)
//	AM_RANGE(0xfe800000, 0xfe85ffff) AM_READWRITE(audio_apu_r, audio_apu_w)
//	AM_RANGE(0xfec00000, 0xfec001ff) AM_READWRITE(audio_ac93_r, audio_ac93_w)
	AM_RANGE(0xff000000, 0xff07ffff) AM_ROM AM_REGION("bios", 0) AM_MIRROR(0x00f80000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(xbox_map_io, AS_IO, 32, xbox_state)
//	AM_RANGE(0x0020, 0x0023) AM_DEVREADWRITE8("pic8259_1", pic8259_device, read, write, 0xffffffff)
//	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8254", pit8254_device, read, write, 0xffffffff)
//	AM_RANGE(0x00a0, 0x00a3) AM_DEVREADWRITE8("pic8259_2", pic8259_device, read, write, 0xffffffff)
//	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", bus_master_ide_controller_device, read_cs0, write_cs0)
//	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
//	AM_RANGE(0x4000, 0x40ff) AM_READWRITE(mediaboard_r, mediaboard_w)
//	AM_RANGE(0x8000, 0x80ff) AM_READWRITE(dummy_r, dummy_w)
//	AM_RANGE(0xc000, 0xc0ff) AM_READWRITE(smbus_r, smbus_w)
//	AM_RANGE(0xff60, 0xff67) AM_DEVREADWRITE("ide", bus_master_ide_controller_device, bmdma_r, bmdma_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( xbox )
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




void xbox_state::machine_start()
{
}

void xbox_state::machine_reset()
{
}


PALETTE_INIT_MEMBER(xbox_state, xbox)
{
}

static MACHINE_CONFIG_START( xbox, xbox_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM3, 733333333/CPU_DIV) /* Wrong! family 6 model 8 stepping 10 */
	MCFG_CPU_PROGRAM_MAP(xbox_map)
	MCFG_CPU_IO_MAP(xbox_map_io)
//	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(chihiro_state, irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(xbox_state, screen_update)
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_RAW_PARAMS(8000000/2, 442, 0, 320, 264, 0, 240) /* generic NTSC video timing, change accordingly */
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(xbox_state, xbox)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( xbox )
	ROM_REGION( 0x200, "mcpx", 0 )
	ROM_LOAD( "mcpx_1_0.bin", 0, 0x200, CRC(f31429fc) SHA1(a9ecbf8896d10db81594923e485862aa3aac7b58) )
	ROM_LOAD( "mcpx_1_1.bin", 0, 0x200, CRC(94ce376b) SHA1(6c875f17f773aaec51eb434068bb6c657c4343c0) )
	
	ROM_REGION( 0x80000, "bios", 0)
    ROM_LOAD( "xbox-5530.bin", 0x000000, 0x040000, CRC(9569c4d3) SHA1(40fa73277013be3168135e1768b09623a987ff63) )
    ROM_LOAD( "xbox-5713.bin", 0x040000, 0x040000, CRC(58fd8173) SHA1(8b7ccc4648ccd78cdb7b65cfca09621eaf2d4238) )
	ROM_COPY( "mcpx", 0, 0x7fe00, 0x200 )
	

	ROM_REGION( 0x1000000, "tbp", 0 ) // To Be Processed, of course
	ROM_LOAD( "3944_1024k.bin", 0x000000, 0x100000, CRC(32a9ecb6) SHA1(67054fc88bda94e33e86f1b19be60efec0724fb6) )
    ROM_LOAD( "4034_1024k.bin", 0x000000, 0x100000, CRC(0d6fc88f) SHA1(ab676b712204fb1728bf89f9cd541a8f5a64ab97) )
    ROM_LOAD( "4134_1024k.bin", 0x000000, 0x100000, CRC(49d8055a) SHA1(d46cef771a63dc8024fe36d7ab5b959087ac999f) )
    ROM_LOAD( "4817_1024k.bin", 0x000000, 0x100000, CRC(3f30863a) SHA1(dc955bd4d3ca71e01214a49e5d0aba615270c03c) )
    ROM_LOAD( "5101_256k.bin", 0x000000, 0x040000, CRC(e8a9224e) SHA1(5108e1025f48071c07a6823661d708c66dee97a9) )
    ROM_LOAD( "5838_256k.bin", 0x000000, 0x040000, CRC(5be2413d) SHA1(b9489e883c650b5e5fe2f83a32237dbf74f0e9f1) )
ROM_END
// See src/emu/gamedrv.h for details
// For a game:
// GAME(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)

// For a console:
// CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)

// For a computer:
// COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)

// For a generic system:
// SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)

CONS( 2001, xbox,  0,  0,   xbox,  xbox, driver_device,  0,       "Microsoft",      "XBOX", GAME_IS_SKELETON )
