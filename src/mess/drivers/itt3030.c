/***************************************************************************

    ITT 3030

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"

#define MAIN_CLOCK XTAL_8MHz

class itt3030_state : public driver_device
{
public:
	itt3030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
public:

	DECLARE_READ8_MEMBER(unk1_r);
	DECLARE_READ8_MEMBER(unk2_r);
private:
	UINT8 m_unk;
};

void itt3030_state::video_start()
{
	m_unk = 0x80;
}

READ8_MEMBER(itt3030_state::unk1_r)
{
	UINT8 ret = m_unk;
	m_unk ^= 0x80;
	return ret;
}

READ8_MEMBER(itt3030_state::unk2_r)
{
	return 0x40;
}

UINT32 itt3030_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = *machine().device("maincpu")->memory().space(AS_PROGRAM);

	for(int y = 0; y < 24; y++ )
	{
		for(int x = 0; x < 80; x++ )
		{
			UINT8 code = space.read_byte(0x3000 + x + y*128);
			drawgfx_opaque(bitmap, cliprect, machine().gfx[0],  code , 0, 0,0, x*8,y*16);
		}
	}

	return 0;
}

static ADDRESS_MAP_START( itt3030_map, AS_PROGRAM, 8, itt3030_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( itt3030_io, AS_IO, 8, itt3030_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0031, 0x0031) AM_READ(unk2_r)
	AM_RANGE(0x0035, 0x0035) AM_READ(unk1_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( itt3030 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8, 16,				/* 8x16 characters */
	128,				/* 128 characters */
	1,				  /* 1 bits per pixel */
	{0},				/* no bitplanes; 1 bit per pixel */
	{7, 6, 5, 4, 3, 2, 1, 0},
	{ 0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16					/* size of one char */
};

static GFXDECODE_START( itt3030 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void itt3030_state::machine_start()
{
}

void itt3030_state::machine_reset()
{
}

static MACHINE_CONFIG_START( itt3030, itt3030_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(itt3030_map)
	MCFG_CPU_IO_MAP(itt3030_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(itt3030_state, screen_update)
	MCFG_SCREEN_SIZE(80*8, 24*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*16-1)

	MCFG_GFXDECODE(itt3030)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( itt3030 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bootv1.2.bin", 0x0000, 0x0800, CRC(90279d45) SHA1(a39a3f31f4f98980b1ef50805870837fbf72261d))
	ROM_REGION( 0x0800, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "gb136-0.bin", 0x0000, 0x0800, CRC(6a3895a8) SHA1(f3b977ffa2f54c346521c9ef034830de8f404621))
	ROM_REGION( 0x0400, "gfxcpu", ROMREGION_ERASE00 )
	ROM_LOAD( "8741ad.bin", 0x0000, 0x0400, CRC(cabf4394) SHA1(e5d1416b568efa32b578ca295a29b7b5d20c0def))
ROM_END

GAME( 1982, itt3030,  0,   itt3030,  itt3030,  driver_device, 0,      ROT0, "ITT RFA",      "ITT3030", GAME_NOT_WORKING | GAME_NO_SOUND )
