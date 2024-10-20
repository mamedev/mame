// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/*******************************************************************************

     Mouser

     Driver by Frank Palazzolo (palazzol@comcast.net)

    - This driver was done with only flyer shots to go by.
    - Colors are a good guess (might be perfect)
    - Clock and interrupt speeds for the sound CPU is a guess, but seem
      reasonable, especially because the graphics seem to be synched
    - Sprite priorities are unknown

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class mouser_state : public driver_device
{
public:
	mouser_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void mouser(machine_config &config);

	void init_mouser();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	// misc
	bool m_nmi_enable = false;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_decrypted_opcodes;

	void nmi_enable_w(int state);
	void sound_nmi_clear_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nmi_interrupt(int state);
	INTERRUPT_GEN_MEMBER(sound_nmi_assert);
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/*******************************************************************************

     Mouser - Video Hardware:

     Character map with scrollable rows, 1024 possible characters.
        - index = byte from videoram + 2 bits from colorram)
        - (if row is scrolled, videoram is offset, colorram is not)
        - 16 4-color combinations for each char, from colorram

     15 Sprites controlled by 4-byte records
        - 16 4-color combinations
        - 2 banks of 64 sprite characters each

*******************************************************************************/

void mouser_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

uint32_t mouser_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const spriteram = m_spriteram;

	// for every character in the Video RAM
	for (int offs = 0x3ff; offs >= 0; offs--)
	{
		int sx = offs % 32;
		if (flip_screen_x())
			sx = 31 - sx;

		int sy = offs / 32;
		if (flip_screen_y())
			sy = 31 - sy;

		/* This bit of spriteram appears to be for row scrolling
		   Note: this is dependant on flipping in y */
		int const scrolled_y_position = (256 + 8 * sy - spriteram[offs % 32]) % 256;
		/* I think we still need to fetch the colorram bits to from the ram underneath, which is not scrolled
		   Ideally we would merge these on a pixel-by-pixel basis, but it's ok to do this char-by-char,
		   Since it's only for the MOUSER logo and it looks fine
		   Note: this is _not_ dependant on flipping */
		int const color_offs = offs % 32 + ((256 + 8 * (offs / 32) - spriteram[offs % 32] )% 256) / 8 * 32;

		m_gfxdecode->gfx(0)->opaque(bitmap, cliprect,
				m_videoram[offs] | (m_colorram[color_offs] >> 5) * 256 | ((m_colorram[color_offs] >> 4) & 1) * 512,
				m_colorram[color_offs] % 16,
				flip_screen_x(), flip_screen_y(),
				8 * sx, scrolled_y_position);
	}

	// There seem to be two sets of sprites, each decoded identically

	// This is the first set of 7 sprites
	for (int offs = 0x0084; offs < 0x00a0; offs += 4)
	{
		if (BIT(spriteram[offs + 1], 4))
		{
			int flipx = BIT(spriteram[offs], 6);
			int flipy = BIT(spriteram[offs], 7);

			int sx = spriteram[offs + 3];
			if (flip_screen_x())
			{
				flipx = !flipx;
				sx = 240 - sx;
			}

			int sy = 0xef - spriteram[offs + 2];
			if (flip_screen_y())
			{
				flipy = !flipy;
				sy = 238 - sy;
			}

			m_gfxdecode->gfx(1 + ((spriteram[offs + 1] & 0x20) >> 5))->transpen(bitmap, cliprect,
					spriteram[offs] & 0x3f,
					spriteram[offs + 1] % 16,
					flipx, flipy,
					sx, sy, 0);
		}
	}

	// This is the second set of 8 sprites
	for (int offs = 0x00c4; offs < 0x00e4; offs += 4)
	{
		if (BIT(spriteram[offs + 1], 4))
		{
			int flipx = BIT(spriteram[offs], 6);
			int flipy = BIT(spriteram[offs], 7);

			int sx = spriteram[offs + 3];
			if (flip_screen_x())
			{
				flipx = !flipx;
				sx = 240 - sx;
			}

			int sy = 0xef - spriteram[offs + 2];
			if (flip_screen_y())
			{
				flipy = !flipy;
				sy = 238 - sy;
			}

			m_gfxdecode->gfx(1 + ((spriteram[offs + 1] & 0x20) >> 5))->transpen(bitmap, cliprect,
					spriteram[offs] & 0x3f,
					spriteram[offs + 1] % 16,
					flipx, flipy,
					sx, sy, 0);
		}
	}

	return 0;
}


// Mouser has external masking circuitry around the NMI input on the main CPU
void mouser_state::nmi_enable_w(int state)
{
	m_nmi_enable = state;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void mouser_state::nmi_interrupt(int state)
{
	if (state && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

// Sound CPU interrupted on write

void mouser_state::sound_nmi_clear_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(mouser_state::sound_nmi_assert)
{
	if (m_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void mouser_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6bff).ram();
	map(0x8800, 0x88ff).nopw(); // unknown
	map(0x9000, 0x93ff).ram().share(m_videoram);
	map(0x9800, 0x9bff).ram().share(m_spriteram);
	map(0x9c00, 0x9fff).ram().share(m_colorram);
	map(0xa000, 0xa000).portr("P1");
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xa800, 0xa800).portr("SYSTEM");
	map(0xb000, 0xb000).portr("DSW");
	map(0xb800, 0xb800).portr("P2").w("soundlatch", FUNC(generic_latch_8_device::write)); // byte to sound CPU
}

void mouser_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().share(m_decrypted_opcodes);
}

void mouser_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x3000, 0x3000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x4000, 0x4000).w(FUNC(mouser_state::sound_nmi_clear_w));
}

void mouser_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x80, 0x81).w("ay2", FUNC(ay8910_device::data_address_w));
}

static INPUT_PORTS_START( mouser )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )       // guess ! - check code at 0x29a1
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x08, "60000" )
	PORT_DIPSETTING(    0x0c, "80000" )
	PORT_DIPNAME( 0x70, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,     /* 8*8 characters */
	1024,    /* 1024 characters */
	2,       /* 2 bits per pixel */
	{ 8192*8, 0 },
	{0, 1, 2, 3, 4, 5, 6, 7},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	8*8
};


static const gfx_layout spritelayout =
{
	16,16,   /* 16*16 characters */
	64,      /* 64 sprites (2 banks) */
	2,       /* 2 bits per pixel */
	{ 8192*8, 0 },
	{0,  1,  2,  3,  4,  5,  6,  7,
		64+0, 64+1, 64+2, 64+3, 64+4, 64+5, 64+6, 64+7},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7,
		128+8*0, 128+8*1, 128+8*2, 128+8*3, 128+8*4, 128+8*5, 128+8*6, 128+8*7},
	16*16
};


static GFXDECODE_START( gfx_mouser )
	GFXDECODE_ENTRY( "gfx", 0x0000, charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x1000, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x1800, spritelayout,     0, 16 )
GFXDECODE_END


void mouser_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
}

void mouser_state::mouser(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4_MHz_XTAL); // NEC D780C-1 - 4.000MHz OSC
	m_maincpu->set_addrmap(AS_PROGRAM, &mouser_state::main_map);
	m_maincpu->set_addrmap(AS_OPCODES, &mouser_state::decrypted_opcodes_map);

	Z80(config, m_audiocpu, 4_MHz_XTAL); // NEC D780C-1 - 4.000MHz OSC
	m_audiocpu->set_addrmap(AS_PROGRAM, &mouser_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &mouser_state::sound_io_map);
	m_audiocpu->set_periodic_int(FUNC(mouser_state::sound_nmi_assert), attotime::from_hz(4 * 60)); // ??? This controls the sound tempo

	ls259_device &mainlatch(LS259(config, "mainlatch")); // Fairchild MB74LS259 @ 4L
	mainlatch.q_out_cb<0>().set(FUNC(mouser_state::nmi_enable_w));
	mainlatch.q_out_cb<1>().set(FUNC(mouser_state::flip_screen_x_set)).invert();
	mainlatch.q_out_cb<2>().set(FUNC(mouser_state::flip_screen_y_set)).invert();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(mouser_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(mouser_state::nmi_interrupt));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mouser);
	PALETTE(config, m_palette, FUNC(mouser_state::palette), 64);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", 4_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "mono", 0.50);
	AY8910(config, "ay2", 4_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( mouser ) // UPL-83001 PCB
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "m0.5e", 0x0000, 0x2000, CRC(b56e00bc) SHA1(f3b23212590d91f1d19b1c7a98c560fbe5943185) )
	ROM_LOAD( "m1.5f", 0x2000, 0x2000, CRC(ae375d49) SHA1(8422f5a4d8560425f0c8612cf6f76029fcfe267c) )
	ROM_LOAD( "m2.5j", 0x4000, 0x2000, CRC(ef5817e4) SHA1(5cadc19f20fadf97c95852b280305fe4c75f1d19) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "m5.3v", 0x0000, 0x1000, CRC(50705eec) SHA1(252cea3498722318638f0c98ae929463ffd7d0d6) )

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "m3.11h", 0x0000, 0x2000, CRC(aca2834e) SHA1(c4f457fd8ea46386431ef8dffe54a232631870be) )
	ROM_LOAD( "m4.11k", 0x2000, 0x2000, CRC(943ab2e2) SHA1(ef9fc31dc8fe7a62f7bc6c817ce0d65091cb9a03) )

	ROM_REGION( 0x0100, "user1", 0 ) // Opcode Decryption PROMS
	ROM_LOAD_NIB_HIGH( "tbp24s10n.4b", 0x0000, 0x0100, CRC(dd233851) SHA1(25eab1ec2227910c6fcd2803986f1cf206624da7) ) // TBP24S10N BPROM
	ROM_LOAD_NIB_LOW(  "tbp24s10n.4c", 0x0000, 0x0100, CRC(60aaa686) SHA1(bb2ad555da51f6b30ab8b55833fe8d461a1e67f4) ) // TBP24S10N BPROM

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "tbp18s030n.5v", 0x0000, 0x0020, CRC(7f8930b2) SHA1(8d0fe14b770fcf7088696d7b80d64507c6ee7364) ) // TBP18S030N BPROM
	ROM_LOAD( "tbp18s030n.5u", 0x0020, 0x0020, CRC(0086feed) SHA1(b0b368e5fb7380cf09abd60c0933b405daf1c36a) ) // TBP18S030N BPROM
ROM_END


ROM_START( mouserc )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "83001.0", 0x0000, 0x2000, CRC(e20f9601) SHA1(f559a470784bda0bee9cab257a548238365acaa6) )
	ROM_LOAD( "83001.1", 0x2000, 0x2000, CRC(ae375d49) SHA1(8422f5a4d8560425f0c8612cf6f76029fcfe267c) ) // == m1.5f
	ROM_LOAD( "83001.2", 0x4000, 0x2000, CRC(ef5817e4) SHA1(5cadc19f20fadf97c95852b280305fe4c75f1d19) ) // == m2.5j

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "83001.5", 0x0000, 0x1000, CRC(50705eec) SHA1(252cea3498722318638f0c98ae929463ffd7d0d6) ) // == m5.3v

	ROM_REGION( 0x4000, "gfx", 0 )
	ROM_LOAD( "83001.3", 0x0000, 0x2000, CRC(aca2834e) SHA1(c4f457fd8ea46386431ef8dffe54a232631870be) ) // == m3.11h
	ROM_LOAD( "83001.4", 0x2000, 0x2000, CRC(943ab2e2) SHA1(ef9fc31dc8fe7a62f7bc6c817ce0d65091cb9a03) ) // == m4.11k

	ROM_REGION( 0x0100, "user1", 0 ) // Opcode Decryption PROMS (originally from the UPL romset!)
	ROM_LOAD_NIB_HIGH( "tbp24s10n.4b", 0x0000, 0x0100, CRC(dd233851) SHA1(25eab1ec2227910c6fcd2803986f1cf206624da7) ) // TBP24S10N BPROM
	ROM_LOAD_NIB_LOW(  "tbp24s10n.4c", 0x0000, 0x0100, CRC(60aaa686) SHA1(bb2ad555da51f6b30ab8b55833fe8d461a1e67f4) ) // TBP24S10N BPROM

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "tbp18s030n.5v", 0x0000, 0x0020, CRC(7f8930b2) SHA1(8d0fe14b770fcf7088696d7b80d64507c6ee7364) ) // TBP18S030N BPROM
	ROM_LOAD( "tbp18s030n.5u", 0x0020, 0x0020, CRC(0086feed) SHA1(b0b368e5fb7380cf09abd60c0933b405daf1c36a) ) // TBP18S030N BPROM
ROM_END


void mouser_state::init_mouser()
{
	// Decode the opcodes
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *table = memregion("user1")->base();

	for (offs_t i = 0; i < 0x6000; i++)
	{
		m_decrypted_opcodes[i] = table[rom[i]];
	}
}

} // anonymous namespace


GAME( 1983, mouser,   0,      mouser, mouser, mouser_state, init_mouser, ROT90, "UPL",                  "Mouser",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, mouserc,  mouser, mouser, mouser, mouser_state, init_mouser, ROT90, "UPL (Cosmos license)", "Mouser (Cosmos)", MACHINE_SUPPORTS_SAVE )
