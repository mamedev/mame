/***************************************************************************

	NEC APC

	front ^
	      |
	card
	----
	69PFCU 7220               PFCU1R 2764
	69PTS  7220
	-
	69PFB2 8086/8087   DFBU2J PFBU2L 2732
	69SNB RAM

***************************************************************************/


#include "emu.h"
#include "cpu/i86/i86.h"
#include "video/upd7220.h"
//#include "sound/ay8910.h"


class apc_state : public driver_device
{
public:
	apc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_hgdc1(*this, "upd7220_chr"),
		  m_hgdc2(*this, "upd7220_btm")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<upd7220_device> m_hgdc1;
	required_device<upd7220_device> m_hgdc2;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

void apc_state::video_start()
{

}

UINT32 apc_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(0, cliprect);

	/* graphics */
	m_hgdc2->screen_update(screen, bitmap, cliprect);
	m_hgdc1->screen_update(screen, bitmap, cliprect);

	return 0;
}


static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	// ...
}

static UPD7220_DRAW_TEXT_LINE( hgdc_draw_text )
{
	// ...
}

static ADDRESS_MAP_START( apc_map, AS_PROGRAM, 16, apc_state )
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
//	AM_RANGE(0xa0000, 0xaffff) AM_ROM AM_REGION("file", 0)
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( apc_io, AS_IO, 16, apc_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
//	AM_RANGE(0x00, 0x1f) i8237
//	AM_RANGE(0x20, 0x23) i8259 master
//	AM_RANGE(0x28, 0x2b) i8259 slave
//	0x2b RTC counter port 0
//	0x2f RTC counter mode 0 (w)
//	0x30, 0x37 serial port 0/1 (i8251) (even/odd)
//	0x38, 0x3f DMA extended address
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE8("upd7220_chr", upd7220_device, read, write, 0x00ff) // odd address UPD7220 #2!
//  0x46 UPD7220 reset interrupt
//	0x48, 0x4f keyboard controller
//	0x50, 0x53 upd765
//	0x5a  APU data (Arithmetic Processing Unit!)
//	0x5e  APU status/command
//	0x60 Melody Processing Unit
//	0x61 RTC counter port 1
//	0x67 RTC counter mode 1 (w)
//	AM_RANGE(0x68, 0x6f) i8255 , printer port (A: status (R) B: data (W) C: command (W))
//	AM_DEVREADWRITE8("upd7220_btm", upd7220_device, read, write, 0x00ff)

ADDRESS_MAP_END

static INPUT_PORTS_START( apc )
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


void apc_state::machine_start()
{
}

void apc_state::machine_reset()
{
}


void apc_state::palette_init()
{
}

static UPD7220_INTERFACE( hgdc_1_intf )
{
	"screen",
	NULL,
	hgdc_draw_text,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static UPD7220_INTERFACE( hgdc_2_intf )
{
	"screen",
	hgdc_display_pixels,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const gfx_layout apc_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 1 },
	{ 0, 1*2, 2*2, 3*2, 4*2, 5*2, 6*2, 7*2 },
	{ 0*8*2, 1*8*2, 2*8*2, 3*8*2, 4*8*2, 5*8*2, 6*8*2, 7*8*2 },
	8*8*2
};

static GFXDECODE_START( apc )
	GFXDECODE_ENTRY( "gfx", 0x0000, apc_chars_8x8, 0, 8 )
GFXDECODE_END


static ADDRESS_MAP_START( upd7220_1_map, AS_0, 8, apc_state)
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram_1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( upd7220_2_map, AS_0, 8, apc_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram_2")
ADDRESS_MAP_END

#define MAIN_CLOCK XTAL_5MHz

static MACHINE_CONFIG_START( apc, apc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8086,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(apc_map)
	MCFG_CPU_IO_MAP(apc_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(apc_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_GFXDECODE(apc)

	MCFG_UPD7220_ADD("upd7220_chr", 5000000/2, hgdc_1_intf, upd7220_1_map)
	MCFG_UPD7220_ADD("upd7220_btm", 5000000/2, hgdc_2_intf, upd7220_2_map)

	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( apc )
	ROM_REGION( 0x2000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "pfbu2j.bin",   0x00000, 0x001000, CRC(86970df5) SHA1(be59c5dad3bd8afc21e9f2f1404553d4371978be) )
    ROM_LOAD16_BYTE( "pfbu2l.bin",   0x00001, 0x001000, CRC(38df2e70) SHA1(a37ccaea00c2b290610d354de08b489fa897ec48) )

//	ROM_REGION( 0x10000, "file", ROMREGION_ERASE00 )
//	ROM_LOAD( "sioapc.o", 0, 0x10000, CRC(1) SHA1(1) )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
    ROM_LOAD( "pfcu1r.bin",   0x000000, 0x002000, CRC(683efa94) SHA1(43157984a1746b2e448f3236f571011af9a3aa73) )
ROM_END

GAME( 198?, apc,  0,   apc,  apc, driver_device,  0,       ROT0, "NEC",      "APC", GAME_IS_SKELETON )
