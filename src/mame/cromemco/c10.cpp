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


namespace {

class c10_state : public driver_device
{
public:
	c10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_vram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void c10(machine_config &config);

private:
	void machine_reset() override;
	void machine_start() override;

	void io_map(address_map &map);
	void mem_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_p_chargen;
};



void c10_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("mainram");
	map(0x8000, 0xbfff).rom().region("maincpu", 0);
	map(0xc000, 0xf0a1).ram();
	map(0xf0a2, 0xffff).ram().share("videoram");
}

void c10_state::io_map(address_map &map)
{
	map.global_mask(0xff);
}

/* Input ports */
static INPUT_PORTS_START( c10 )
INPUT_PORTS_END

void c10_state::machine_start()
{
}

void c10_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x0fff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0x8000, 0x8fff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x0fff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

/* This system appears to have inline attribute bytes of unknown meaning.
    Currently they are ignored. The word at FAB5 looks like it might be cursor location. */
uint32_t c10_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//static uint8_t framecnt=0;
	uint16_t sy=0,ma=0;

	//framecnt++;

	for (uint8_t y = 0; y < 25; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			uint16_t xx = ma;
			for (uint16_t x = ma; x < ma + 80; x++)
			{
				uint8_t gfx = 0;
				if (ra < 9)
				{
					uint8_t chr = m_vram[xx++];

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
static const gfx_layout charlayout =
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
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END


void c10_state::c10(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &c10_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &c10_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(c10_state::screen_update));
	screen.set_size(640, 250);
	screen.set_visarea_full();
	screen.set_palette("palette");
	GFXDECODE(config, "gfxdecode", "palette", gfx_c10);
	PALETTE(config, "palette", palette_device::MONOCHROME);
}


/* ROM definition */
ROM_START( c10 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "502-0055.ic16", 0x0000, 0x4000, CRC(2ccf5983) SHA1(52f7c497f5284bf5df9eb0d6e9142bb1869d8c24))

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.ic9", 0x0000, 0x2000, CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

} // anonymous namespace

/* Driver */

/*   YEAR   NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY     FULLNAME  FLAGS */
COMP( 1982, c10,  0,      0,      c10,     c10,   c10_state, empty_init, "Cromemco", "C-10",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

