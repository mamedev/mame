/* Cuantro en Linea */

#include "emu.h"
#include "cpu/z80/z80.h"

#include "sound/ay8910.h"


class _4enlinea_state : public driver_device
{
public:
	_4enlinea_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ay(*this, "aysnd"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }


	required_device<ay8910_device> m_ay;
	required_shared_ptr<UINT8> m_videoram;

	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(_4enlinea);
	UINT32 screen_update_4enlinea(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(vram_w);
};



void _4enlinea_state::video_start()
{

	m_gfxdecode->gfx(0)->set_source(m_videoram);
}

UINT32 _4enlinea_state::screen_update_4enlinea(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offset = 0;
		
	for (int y = 0; y < 256; y++)
	{
		UINT16* dstptr_bitmap = &bitmap.pix16(y);

		for (int x = 0; x < 256; x+=4)
		{
			UINT8 pix = m_videoram[offset++];

			dstptr_bitmap[x+3] = (pix>>0)&0x3;
			dstptr_bitmap[x+2] = (pix>>2)&0x3;
			dstptr_bitmap[x+1] = (pix>>4)&0x3;
			dstptr_bitmap[x+0] = (pix>>6)&0x3;


		}
	}

	return 0;
}


#define MAIN_CLOCK XTAL_8MHz

WRITE8_MEMBER(_4enlinea_state::vram_w)
{
	m_videoram[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset/16);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, _4enlinea_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM

	AM_RANGE(0x8000, 0xbfff) AM_RAM_WRITE(vram_w) AM_SHARE("videoram")
	AM_RANGE(0xc000, 0xcfff) AM_RAM

	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM

ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, AS_IO, 8, _4enlinea_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, _4enlinea_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_portmap, AS_IO, 8, _4enlinea_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( 4enlinea )
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
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	0x4000/16,
	2,
	{ 0,1 },
	{ 0, 2, 4, 6, 8, 10, 12, 14 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static GFXDECODE_START( 4enlinea )
	GFXDECODE_ENTRY( NULL, 0, charlayout,     0, 1 )
GFXDECODE_END

void _4enlinea_state::machine_start()
{

}

void _4enlinea_state::machine_reset()
{

}


static MACHINE_CONFIG_START( 4enlinea, _4enlinea_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _4enlinea_state,  nmi_line_pulse)
	MCFG_CPU_PERIODIC_INT_DRIVER(_4enlinea_state, irq0_line_hold, 4*60)
	
	MCFG_CPU_ADD("audiocpu",Z80,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_IO_MAP(audio_portmap)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(_4enlinea_state, screen_update_4enlinea)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 4enlinea)

	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END



ROM_START( 4enlinea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c512_ic6_cicplay-2.bin",   0x0000, 0x10000, CRC(b6804274) SHA1(cea31b921b37e5adaade68ab92f07e103f60e9f7) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c512_ic19_cicplay-1.bin",   0x0000, 0x10000, CRC(a6181c1a) SHA1(d6c9d0353cdcb86500fe172d24d1bba7abcd5d42) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x10000, "unk", 0 )
	ROM_LOAD( "cuatro_en_linea_ic17_x24c16p.bin",   0x000, 0x800, CRC(21f81f5a) SHA1(00b10eee5af1ca79ced2878f4be4cac2bb8d26a0) )
ROM_END



GAME( 1991, 4enlinea,  0,       4enlinea,  4enlinea, driver_device,  0,       ROT0, "System Compumatic",      "Cuantro en Linea", GAME_NOT_WORKING )
        

