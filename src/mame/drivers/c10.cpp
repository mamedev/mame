// license:BSD-3-Clause
// copyright-holders:Robbbert
/*****************************************************************************************************

Cromemco C-10 Personal Computer

2010-08-30 Skeleton driver

Photos show: Intersil 74954-1, Mostek MK3880N-4 (Z80A), CROMEMCO 011-0082-01, CROMEMCO 011-0095,
             Intel P8275-2, AM92128BPC (16K ROM), NEC D8257C-5, CROMEMCO 011-0083, WDC FD1793B-02,
             2x 8251. Crystals: 8MHz, 13.028MHz

Driver currently gets to a loop where it waits for an interrupt.
The interrupt routine presumably writes to FE69 which the loop is
constantly looking at.

*****************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"


class c10_state : public driver_device
{
public:
	enum
	{
		TIMER_RESET
	};

	c10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void init_c10();

	void c10(machine_config &config);
	void c10_io(address_map &map);
	void c10_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};



void c10_state::c10_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).bankrw("boot");
	map(0x1000, 0x7fff).ram();
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xf0a1).ram();
	map(0xf0a2, 0xffff).ram().share("videoram");
}

void c10_state::c10_io(address_map &map)
{
	map.global_mask(0xff);
}

/* Input ports */
static INPUT_PORTS_START( c10 )
INPUT_PORTS_END

void c10_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RESET:
		/* after the first 4 bytes have been read from ROM, switch the ram back in */
		membank("boot")->set_entry(0);
		break;
	default:
		assert_always(false, "Unknown id in c10_state::device_timer");
	}
}

void c10_state::machine_reset()
{
	membank("boot")->set_entry(1);
	timer_set(attotime::from_usec(4), TIMER_RESET);
}

/* This system appears to have inline attribute bytes of unknown meaning.
    Currently they are ignored. The word at FAB5 looks like it might be cursor location. */
uint32_t c10_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//static uint8_t framecnt=0;
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x,xx;

	//framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			xx = ma;
			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[xx++];

				//  /* Take care of flashing characters */
				//  if ((chr < 0x80) && (framecnt & 0x08))
				//      chr |= 0x80;

					if (BIT(chr, 7)) // ignore attribute bytes
						x--;
					else
						gfx = m_p_chargen[(chr<<4) | ra ];
				}
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
		ma+=96;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout c10_charlayout =
{
	8, 9,                   /* 8 x 9 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_c10 )
	GFXDECODE_ENTRY( "chargen", 0x0000, c10_charlayout, 0, 1 )
GFXDECODE_END


MACHINE_CONFIG_START(c10_state::c10)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(8'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(c10_mem)
	MCFG_DEVICE_IO_MAP(c10_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(c10_state, screen_update)
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)
	MCFG_SCREEN_PALETTE("palette")
	GFXDECODE(config, "gfxdecode", "palette", gfx_c10);
	PALETTE(config, "palette", palette_device::MONOCHROME);
MACHINE_CONFIG_END

void c10_state::init_c10()
{
	uint8_t *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0x8000);
}

/* ROM definition */
ROM_START( c10 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "502-0055.ic16", 0x8000, 0x4000, CRC(2ccf5983) SHA1(52f7c497f5284bf5df9eb0d6e9142bb1869d8c24))

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.ic9", 0x0000, 0x2000, CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

/*   YEAR   NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY     FULLNAME  FLAGS */
COMP( 1982, c10,  0,      0,      c10,     c10,   c10_state, init_c10,   "Cromemco", "C-10",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
