// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************************************

Control Data Corporation CDC 721 Terminal (Viking)

2013-08-13 Skeleton


*************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9927.h"
#include "screen.h"


class cdc721_state : public driver_device
{
public:
	cdc721_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DRIVER_INIT(init);
	DECLARE_PALETTE_INIT(cdc721);
//  DECLARE_WRITE8_MEMBER(port70_w) { membank("bankr0")->set_entry(BIT(data, 3)); }

void cdc721(machine_config &config);
void io_map(address_map &map);
void mem_map(address_map &map);
private:
	u8 m_flashcnt;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
};

ADDRESS_MAP_START(cdc721_state::mem_map)
	AM_RANGE(0x0000, 0x4fff) AM_ROM AM_REGION("maincpu", 0x10000)
//  AM_RANGE(0x0000, 0x4fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x8000, 0xe10f) AM_RAM
	AM_RANGE(0xe110, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

ADDRESS_MAP_START(cdc721_state::io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x1f) AM_DEVREADWRITE("crtc", tms9927_device, read, write)
//  AM_RANGE(0x70, 0x70) AM_WRITE(port70_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( cdc721 )
INPUT_PORTS_END

void cdc721_state::machine_reset()
{
//  membank("bankr0")->set_entry(0);
//  membank("bankw0")->set_entry(0);
}

DRIVER_INIT_MEMBER( cdc721_state, init )
{
//  uint8_t *main = memregion("maincpu")->base();

//  membank("bankr0")->configure_entry(1, &main[0x14000]);
//  membank("bankr0")->configure_entry(0, &main[0x4000]);
//  membank("bankw0")->configure_entry(0, &main[0x4000]);
}

/* F4 Character Displayer */
static const gfx_layout cdc721_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 0x100*8, 0x200*8, 0x300*8, 0x400*8, 0x500*8, 0x600*8, 0x700*8,
	0x800*8, 0x900*8, 0xa00*8, 0xb00*8, 0xc00*8, 0xd00*8, 0xe00*8, 0xf00*8 },
	8                   /* every char takes 16 x 1 bytes */
};

static GFXDECODE_START( cdc721 )
	GFXDECODE_ENTRY( "chargen", 0x0000, cdc721_charlayout, 0, 1 )
GFXDECODE_END

PALETTE_INIT_MEMBER( cdc721_state, cdc721 )
{
	palette.set_pen_color(0, 0, 0, 0 ); /* Black */
	palette.set_pen_color(1, 0, 255, 0 );   /* Full */
	palette.set_pen_color(2, 0, 128, 0 );   /* Dimmed */
}

uint32_t cdc721_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx,attr,pen;
	uint16_t sy=0,ma=0,x;
	m_flashcnt++;

	for (y = 0; y < 30; y++)
	{
		for (ra = 0; ra < 16; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = 0; x < 160; x+=2)
			{
				pen = 1;
				chr = m_p_videoram[x+ma];
				attr = m_p_videoram[x+ma+1];
				gfx = m_p_chargen[chr | (ra << 8) ];
				if (BIT(attr, 0))  // blank
					pen = 0;
				if (BIT(attr, 1) && (ra == 14)) // underline
					gfx = 0xff;
				if (BIT(attr, 4)) // dim
					pen = 2;
				if (BIT(attr, 2)) // rv
					gfx ^= 0xff;
				if (BIT(attr, 3) && BIT(m_flashcnt, 6)) // blink
					gfx = 0;

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 0) ? pen : 0;
				*p++ = BIT(gfx, 1) ? pen : 0;
				*p++ = BIT(gfx, 2) ? pen : 0;
				*p++ = BIT(gfx, 3) ? pen : 0;
				*p++ = BIT(gfx, 4) ? pen : 0;
				*p++ = BIT(gfx, 5) ? pen : 0;
				*p++ = BIT(gfx, 6) ? pen : 0;
				*p++ = BIT(gfx, 7) ? pen : 0;
			}
		}
		ma+=264;
	}
	return 0;
}

MACHINE_CONFIG_START(cdc721_state::cdc721)
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(cdc721_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(cdc721_state, cdc721)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cdc721)

	MCFG_DEVICE_ADD("crtc", TMS9927, 2000000) // clock guess
	MCFG_TMS9927_CHAR_WIDTH(8)
MACHINE_CONFIG_END

ROM_START( cdc721 )
	ROM_REGION( 0x15000, "maincpu", 0 )
	ROM_LOAD( "66315359", 0x10000, 0x2000, CRC(20ff3eb4) SHA1(5f15cb14893d75a46dc66d3042356bb054d632c2) )
	ROM_LOAD( "66315361", 0x12000, 0x2000, CRC(21d59d09) SHA1(9c087537d68c600ddf1eb9b009cf458231c279f4) )
	ROM_LOAD( "66315360", 0x14000, 0x1000, CRC(feaa0fc5) SHA1(f06196553a1f10c07b2f7e495823daf7ea26edee) )
	//ROM_FILL(0x14157,1,0xe0)

	ROM_REGION( 0x1000, "keyboard", 0 )
	ROM_LOAD( "66307828", 0x0000, 0x1000, CRC(ac97136f) SHA1(0d280e1aa4b9502bd390d260f83af19bf24905cd) ) // keyboard lookup

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "66315039", 0x0000, 0x1000, CRC(5c9aa968) SHA1(3ec7c5f25562579e6ed3fda7562428ff5e6b9550) )
ROM_END

COMP( 1981, cdc721, 0, 0, cdc721, cdc721, cdc721_state, init, "Control Data Corporation",  "CDC721 Terminal", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
