// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Mera-Elzab 79152pc

        This is terminal

        29/12/2011 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class m79152pc_state : public driver_device
{
public:
	m79152pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_p_videoram(*this, "p_videoram"),
		m_p_attributes(*this, "p_attributes"),
		m_maincpu(*this, "maincpu") { }

	UINT8 *m_p_chargen;
	required_shared_ptr<UINT8> m_p_videoram;
	required_shared_ptr<UINT8> m_p_attributes;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_m79152pc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(m79152pc_mem, AS_PROGRAM, 8, m79152pc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x8000, 0x8fff) AM_RAM AM_SHARE("p_videoram")
	AM_RANGE(0x9000, 0x9fff) AM_RAM AM_SHARE("p_attributes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( m79152pc_io, AS_IO, 8, m79152pc_state)
	//ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( m79152pc )
INPUT_PORTS_END


void m79152pc_state::machine_reset()
{
}

void m79152pc_state::video_start()
{
	m_p_chargen = memregion("chargen")->base()+4;
}

UINT32 m79152pc_state::screen_update_m79152pc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
// Attributes are unknown so are not implemented
	UINT8 y,ra,chr,gfx; //,attr;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				chr = m_p_videoram[x];
				//attr = m_p_attributes[x];
				gfx = m_p_chargen[(chr<<4) | ra ];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=80;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout m79152pc_charlayout =
{
	8, 12,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( m79152pc )
	GFXDECODE_ENTRY( "chargen", 0x0000, m79152pc_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( m79152pc, m79152pc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(m79152pc_mem)
	MCFG_CPU_IO_MAP(m79152pc_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 300-1)
	MCFG_SCREEN_UPDATE_DRIVER(m79152pc_state, screen_update_m79152pc)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", m79152pc)
	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( m79152pc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "left.bin", 0x0000, 0x4000, CRC(8cd677fc) SHA1(7ad28f3ba984383f24a36639ca27fc1eb5a5d002))

	ROM_REGION( 0x1800, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "right.bin", 0x0000, 0x1000, CRC(93f83fdc) SHA1(e8121b3d175c46c02828f43ec071a7d9c62e7c26)) // chargen
	ROM_LOAD( "char.bin",  0x1000, 0x0800, CRC(da3792a5) SHA1(b4a4f0d61d8082b7909a346a5b01494c53cf8d05)) // unknown
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT     INIT    COMPANY     FULLNAME       FLAGS */
COMP( ????, m79152pc,  0,       0,       m79152pc,  m79152pc, driver_device, 0,   "Mera-Elzab", "MERA 79152 PC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
