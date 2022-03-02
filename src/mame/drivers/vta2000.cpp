// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

VTA-2000 Terminal

Board images : http://fotki.yandex.ru/users/lodedome/album/93699?p=0

BDP-15 board only

2010-11-29 Skeleton driver.

Better known on the net as BTA2000-15m.
It is a green-screen terminal, using RS232, and supposedly VT100 compatible.
The top line is a status line.

Note: port 0 bit 4 is NOT a speaker bit. See code at 027B.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class vta2000_state : public driver_device
{
public:
	vta2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainpit(*this, "mainpit")
		, m_speaker(*this, "speaker")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void vta2000(machine_config &config);
private:
	void output_00(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);

	uint32_t screen_update_vta2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	virtual void machine_reset() override;
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<pit8253_device> m_mainpit;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};

void vta2000_state::output_00(uint8_t data)
{
	m_mainpit->write_gate0(BIT(data, 4));
}

WRITE_LINE_MEMBER(vta2000_state::speaker_w)
{
	m_speaker->level_w(state);
}

void vta2000_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom().region("roms", 0);
	map(0x8000, 0xc7ff).ram().share("videoram");
	map(0xc800, 0xc8ff).rom().region("roms", 0x5000);
}

void vta2000_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).w(FUNC(vta2000_state::output_00));
	map(0x20, 0x21).rw("pic", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xc0, 0xc0).rw("usart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xc3, 0xc3).rw("usart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xc8, 0xcb).w("brgpit", FUNC(pit8253_device::write));
	map(0xe0, 0xe3).rw("mainpit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

/* Input ports */
static INPUT_PORTS_START( vta2000 )
INPUT_PORTS_END


void vta2000_state::machine_reset()
{
}

uint32_t vta2000_state::screen_update_vta2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
/* Cursor is missing. */
{
	static uint8_t framecnt=0; // FIXME: static variable
	uint16_t sy=0,ma=0;

	framecnt++;

	for (uint8_t y = 0; y < 25; y++)
	{
		for (uint8_t ra = 0; ra < 12; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			uint16_t xx = ma << 1;
			for (uint16_t x = ma; x < ma + 80; x++)
			{
				uint16_t chr = m_p_videoram[xx++];
				uint8_t const attr = m_p_videoram[xx++];

				if ((chr & 0x60)==0x60)
					chr+=256;

				uint8_t gfx = m_p_chargen[(chr<<4) | ra ];
				uint8_t fg, bg = 0;

				/* Process attributes */
				if (BIT(attr, 4))
				{
					gfx ^= 0xff; // reverse video
					bg = 2;
				}
				if (BIT(attr, 0))
					fg = 2; // highlight
				else
					fg = 1;
				if ((BIT(attr, 1)) && (BIT(framecnt, 5)))
					gfx = 0; // blink
				if ((BIT(attr, 5)) && (ra == 10))
				{
					gfx = 0xff; // underline
					fg = 2;
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : bg;
				*p++ = BIT(gfx, 6) ? fg : bg;
				*p++ = BIT(gfx, 5) ? fg : bg;
				*p++ = BIT(gfx, 4) ? fg : bg;
				*p++ = BIT(gfx, 3) ? fg : bg;
				*p++ = BIT(gfx, 2) ? fg : bg;
				*p++ = BIT(gfx, 1) ? fg : bg;
				*p++ = BIT(gfx, 0) ? fg : bg;
			}
		}
		if (y)
			ma+=132;
		else
			ma+=80;
	}
	return 0;
}


/* F4 Character Displayer */
static const gfx_layout vta2000_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	512,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_vta2000 )
	GFXDECODE_ENTRY( "chargen", 0x0000, vta2000_charlayout, 0, 1 )
GFXDECODE_END

void vta2000_state::vta2000(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(4'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &vta2000_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vta2000_state::io_map);
	m_maincpu->in_inta_func().set("pic", FUNC(pic8259_device::acknowledge));

	PIT8253(config, m_mainpit, 0);
	m_mainpit->set_clk<0>(500'000);
	m_mainpit->out_handler<0>().set(FUNC(vta2000_state::speaker_w));

	pic8259_device &pic(PIC8259(config, "pic", 0));
	pic.in_sp_callback().set_constant(0);
	pic.out_int_callback().set_inputline(m_maincpu, 0);

	i8251_device &usart(I8251(config, "usart", XTAL(4'000'000) / 4));
	usart.rxrdy_handler().set("pic", FUNC(pic8259_device::ir4_w));

	pit8253_device &brgpit(PIT8253(config, "brgpit", 0));
	brgpit.set_clk<0>(1'228'800); // maybe
	brgpit.set_clk<1>(1'228'800);
	brgpit.out_handler<0>().set("usart", FUNC(i8251_device::write_rxc));
	brgpit.out_handler<1>().set("usart", FUNC(i8251_device::write_txc)); // or vice versa?

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(80*8, 25*12);
	screen.set_visarea(0, 80*8-1, 0, 25*12-1);
	screen.set_screen_update(FUNC(vta2000_state::screen_update_vta2000));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);
	GFXDECODE(config, "gfxdecode", "palette", gfx_vta2000);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.5);
}


/* ROM definition */
ROM_START( vta2000 )
	ROM_REGION( 0x6000, "roms", 0 )
	ROM_LOAD( "bdp-15_11.rom", 0x4000, 0x2000, CRC(d4abe3e9) SHA1(ab1973306e263b0f66f2e1ede50cb5230f8d69d5) )
	ROM_LOAD( "bdp-15_12.rom", 0x2000, 0x2000, CRC(4a5fe332) SHA1(f1401c26687236184fec0558cc890e796d7d5c77) )
	ROM_LOAD( "bdp-15_13.rom", 0x0000, 0x2000, CRC(b6b89d90) SHA1(0356d7ba77013b8a79986689fb22ef4107ef885b) )

	ROM_REGION(0x2000, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "bdp-15_14.rom", 0x0000, 0x2000, CRC(a1dc4f8e) SHA1(873fd211f44713b713d73163de2d8b5db83d2143) )
ROM_END

} // Anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME    FLAGS
COMP( 19??, vta2000, 0,      0,      vta2000, vta2000, vta2000_state, empty_init, "<unknown>", "VTA2000-15m", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
