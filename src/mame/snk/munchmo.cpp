// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************
  Munch Mobile
  (C) 1983 SNK

  2 Z80s
  2 AY-8910s
  15 MHz crystal

  Known Issues:
    - it's unclear if mirroring the videoram chunks is correct behavior
    - several unmapped registers

Stephh's notes (based on the game Z80 code and some tests) :

  - The "Continue after Game Over" Dip Switch (DSW1:1) allows the player
    to continue from where he lost his last life when he starts a new game.
    IMO, this is a debug feature (as often with SNK games) as there is
    NO continue routine nor text for it in the ROMs.
    See code at 0x013a ('joyfulr') or 0x013e ('mnchmobl') for more infos.
  - There is extra code at 0x1de2 in 'mnchmobl' but it doesn't seem to be used.

- DIPs are now verified from Munch Mobile manual and playtesting.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/74259.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class munchmo_state : public driver_device
{
public:
	munchmo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_sprite_xpos(*this, "sprite_xpos")
		, m_sprite_tile(*this, "sprite_tile")
		, m_sprite_attr(*this, "sprite_attr")
		, m_videoram(*this, "videoram")
		, m_status_vram(*this, "status_vram")
		, m_vreg(*this, "vreg")
		, m_tiles_rom(*this, "tiles")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_mainlatch(*this, "mainlatch")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_ay8910(*this, "ay%u", 1U)
	{
	}

	void mnchmobl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void nmi_enable_w(int state);
	void nmi_ack_w(u8 data);
	void sound_nmi_ack_w(u8 data);

	template <u8 Which> u8 ayreset_r();

	void palette_bank_0_w(int state);
	void palette_bank_1_w(int state);
	void flipscreen_w(int state);

	void palette(palette_device &palette) const;
	void vblank_irq(int state);

	IRQ_CALLBACK_MEMBER(generic_irq_ack);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_status(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// memory pointers
	required_shared_ptr<u8> m_sprite_xpos;
	required_shared_ptr<u8> m_sprite_tile;
	required_shared_ptr<u8> m_sprite_attr;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_status_vram;
	required_shared_ptr<u8> m_vreg;
	required_region_ptr<u8> m_tiles_rom;

	// video-related
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	u8 m_palette_bank = 0U;
	u8 m_flipscreen = 0U;

	// misc
	u8 m_nmi_enable = 0U;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ls259_device> m_mainlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device_array<ay8910_device, 2> m_ay8910;
};


void munchmo_state::palette(palette_device &palette) const
{
	u8 const *const color_prom = memregion("proms")->base();

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

void munchmo_state::palette_bank_0_w(int state)
{
	m_palette_bank = (state ? 1 : 0) | (m_palette_bank & 2);
}

void munchmo_state::palette_bank_1_w(int state)
{
	m_palette_bank = (state ? 2 : 0) | (m_palette_bank & 1);
}

void munchmo_state::flipscreen_w(int state)
{
	m_flipscreen = state;
}


void munchmo_state::video_start()
{
	m_tmpbitmap = std::make_unique<bitmap_ind16>(512, 512);
}

void munchmo_state::draw_status(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *const gfx = m_gfxdecode->gfx(0);

	for (int row = 0; row < 4; row++)
	{
		int sx = (row & 1) * 8;
		const u8 *source = m_status_vram + (~row & 1) * 32;
		if (row <= 1)
		{
			source += 2 * 32;
			sx += 256 + 32 + 16;
		}

		for (int sy = 0; sy < 256; sy += 8)
		{
				gfx->opaque(bitmap, cliprect,
				*source++,
				0, // color
				0, 0, // no flip
				sx, sy);
		}
	}
}

void munchmo_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
// ROM B1.2C contains 256 tilemaps defining 4x4 configurations of the tiles in ROM B2.2B

	gfx_element *const gfx = m_gfxdecode->gfx(1);

	for (int offs = 0; offs < 0x100; offs++)
	{
		int const sy = (offs % 16) * 32;
		int const sx = (offs / 16) * 32;
		int const tile_number = m_videoram[offs];

		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
					gfx->opaque(*m_tmpbitmap, m_tmpbitmap->cliprect(),
					m_tiles_rom[col + tile_number * 4 + row * 0x400],
					m_palette_bank,
					0, 0, // flip
					sx + col * 8, sy + row * 8);
			}
		}
	}

	int const scrollx = -(m_vreg[2] *2 + (m_vreg[3] >> 7)) - 64 - 128 - 16;
	int const scrolly = 0;

	copyscrollbitmap(bitmap, *m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
}

void munchmo_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const scroll = m_vreg[2];
	int const flags = m_vreg[3];                            //   XB??????
	int const xadjust = - 128 - 16 - ((flags & 0x80) ? 1 : 0);
	int const bank = (flags & 0x40) ? 1 : 0;
	gfx_element *const gfx = m_gfxdecode->gfx(2 + bank);
	int const color_base = m_palette_bank * 4 + 3;
	int const firstsprite = m_vreg[0] & 0x3f;
	for (int i = firstsprite; i < firstsprite + 0x40; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			int const offs = (j << 6) | (i & 0x3f);
			int const tile_number = m_sprite_tile[offs];    //   ETTTTTTT
			int const attributes = m_sprite_attr[offs];     //   XYYYYYCC
			int sx = m_sprite_xpos[offs];                   //   XXXXXXX?
			int sy = (offs >> 6) << 5;                      // Y YY------
			sy += (attributes >> 2) & 0x1f;
			if (attributes & 0x80)
			{
				sx = (sx >> 1) | (tile_number & 0x80);
				sx = 2 * ((- 32 - scroll - sx) & 0xff) + xadjust;
					gfx->transpen(bitmap, cliprect,
					0x7f - (tile_number & 0x7f),
					color_base - (attributes & 0x03),
					0, 0,                                   // no flip
					sx, sy, 7);
			}
		}
	}
}

u32 munchmo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	draw_status(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void munchmo_state::nmi_enable_w(int state)
{
	m_nmi_enable = state;
}

// trusted through schematics, NMI and IRQ triggers at vblank, at the same time (!)
void munchmo_state::vblank_irq(int state)
{
	if (state)
	{
		if (m_nmi_enable)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

IRQ_CALLBACK_MEMBER(munchmo_state::generic_irq_ack)
{
	device.execute().set_input_line(0, CLEAR_LINE);
	return 0xff;
}

void munchmo_state::nmi_ack_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void munchmo_state::sound_nmi_ack_w(u8 data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

template <u8 Which>
u8 munchmo_state::ayreset_r()
{
	m_ay8910[Which]->reset_w();
	return 0;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void munchmo_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x83ff).ram();
	map(0xa000, 0xa3ff).mirror(0x0400).ram().share(m_sprite_xpos);
	map(0xa800, 0xabff).mirror(0x0400).ram().share(m_sprite_tile);
	map(0xb000, 0xb3ff).mirror(0x0400).ram().share(m_sprite_attr);
	map(0xb800, 0xb8ff).mirror(0x0100).ram().share(m_videoram);
	map(0xbaba, 0xbaba).nopw(); // ?
	map(0xbc00, 0xbc7f).ram().share(m_status_vram);
	map(0xbe00, 0xbe00).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xbe01, 0xbe01).select(0x0070).lw8(NAME([this] (offs_t offset, u8 data){ m_mainlatch->write_d0(offset >> 4, data); }));
	map(0xbe02, 0xbe02).portr("DSW1");
	map(0xbe03, 0xbe03).portr("DSW2");
	map(0xbf00, 0xbf00).w(FUNC(munchmo_state::nmi_ack_w)); // CNI 1-8C
	map(0xbf01, 0xbf01).portr("SYSTEM");
	map(0xbf02, 0xbf02).portr("P1");
	map(0xbf03, 0xbf03).portr("P2");
	map(0xbf04, 0xbf07).writeonly().share(m_vreg); // MY0 1-8C
}

// memory map provided through schematics
void munchmo_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x4000, 0x4fff).w(m_ay8910[0], FUNC(ay8910_device::data_w));
	map(0x5000, 0x5fff).w(m_ay8910[0], FUNC(ay8910_device::address_w));
	map(0x6000, 0x6fff).w(m_ay8910[1], FUNC(ay8910_device::data_w));
	map(0x7000, 0x7fff).w(m_ay8910[1], FUNC(ay8910_device::address_w));
	map(0x8000, 0x9fff).r(FUNC(munchmo_state::ayreset_r<0>)).w(m_ay8910[0], FUNC(ay8910_device::reset_w));
	map(0xa000, 0xbfff).r(FUNC(munchmo_state::ayreset_r<1>)).w(m_ay8910[1], FUNC(ay8910_device::reset_w));
	map(0xc000, 0xdfff).w(FUNC(munchmo_state::sound_nmi_ack_w)); // NCL 1-8H
	map(0xe000, 0xe7ff).mirror(0x1800).ram(); // is mirror ok?
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mnchmobl )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )     PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )   PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )  PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )     PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )   PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
		// See notes about this DIP
	PORT_DIPNAME( 0x01, 0x00, "Continue after Game Over (Cheat)" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_DIPNAME( 0x1e, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:2,3,4,5")
	PORT_DIPSETTING(    0x14, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	// Duplicate Settings
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_8C ) )


	PORT_DIPNAME( 0xe0, 0x00, "1st Bonus" ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x60, "40000" )
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0xa0, "60000" )
	PORT_DIPSETTING(    0xc0, "70000" )
	PORT_DIPSETTING(    0xe0, "No Bonus" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "2nd Bonus (1st+)" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x01, "40000" )
	PORT_DIPSETTING(    0x02, "100000" )
	PORT_DIPSETTING(    0x03, "No Bonus" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Freeze" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Additional Bonus (2nd Bonus Value)" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout char_layout =
{
	8,8,
	256,
	4,
	{ 0, 8, 256*128,256*128+8 },
	{ 7,6,5,4,3,2,1,0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout tile_layout =
{
	8,8,
	0x100,
	4,
	{ 8,12,0,4 },
	{ 0,0,1,1,2,2,3,3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout sprite_layout1 =
{
	32,32,
	128,
	3,
	{ 0x4000*8,0x2000*8,0 },
	{
		7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,
		0x8000+7,0x8000+7,0x8000+6,0x8000+6,0x8000+5,0x8000+5,0x8000+4,0x8000+4,
		0x8000+3,0x8000+3,0x8000+2,0x8000+2,0x8000+1,0x8000+1,0x8000+0,0x8000+0
	},
	{
			0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,
		24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8
	},
	256
};

static const gfx_layout sprite_layout2 =
{
	32,32,
	128,
	3,
	{ 0,0,0 },
	{
		7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,
		0x8000+7,0x8000+7,0x8000+6,0x8000+6,0x8000+5,0x8000+5,0x8000+4,0x8000+4,
		0x8000+3,0x8000+3,0x8000+2,0x8000+2,0x8000+1,0x8000+1,0x8000+0,0x8000+0
	},
	{
			0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,
		24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8
	},
	256
};

static GFXDECODE_START( gfx_mnchmobl )
	GFXDECODE_ENTRY( "chars",              0,  char_layout,      0,  4 )  // colors   0- 63
	GFXDECODE_ENTRY( "tiles",         0x1000,  tile_layout,     64,  4 )  // colors  64-127
	GFXDECODE_ENTRY( "sprites",            0,  sprite_layout1, 128, 16 )  // colors 128-255
	GFXDECODE_ENTRY( "monochrome_sprites", 0,  sprite_layout2, 128, 16 )  // colors 128-255
GFXDECODE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void munchmo_state::machine_start()
{
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_nmi_enable));
}

void munchmo_state::mnchmobl(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(15'000'000) / 4); // from pin 13 of XTAL-driven 163
	m_maincpu->set_addrmap(AS_PROGRAM, &munchmo_state::main_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(munchmo_state::generic_irq_ack)); // IORQ clears flip-flop at 1-2C

	Z80(config, m_audiocpu, XTAL(15'000'000) / 8); // from pin 12 of XTAL-driven 163
	m_audiocpu->set_addrmap(AS_PROGRAM, &munchmo_state::sound_map);
	m_audiocpu->set_irq_acknowledge_callback(FUNC(munchmo_state::generic_irq_ack)); // IORQ clears flip-flop at 1-7H

	LS259(config, m_mainlatch, 0); // 12E
	m_mainlatch->q_out_cb<0>().set(FUNC(munchmo_state::palette_bank_0_w)); // BCL0 2-11E
	m_mainlatch->q_out_cb<1>().set(FUNC(munchmo_state::palette_bank_1_w)); // BCL1 2-11E
	m_mainlatch->q_out_cb<2>().set_nop(); // CL2 2-11E
	m_mainlatch->q_out_cb<3>().set_nop(); // CL3 2-11E
	m_mainlatch->q_out_cb<4>().set(FUNC(munchmo_state::flipscreen_w)); // INV
	m_mainlatch->q_out_cb<5>().set_nop(); // DISP
	m_mainlatch->q_out_cb<6>().set(FUNC(munchmo_state::nmi_enable_w)); // ENI 1-10C

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(256+32+32, 256);
	screen.set_visarea(0, 255+32+32,0, 255-16);
	screen.set_screen_update(FUNC(munchmo_state::screen_update));
	screen.screen_vblank().set(FUNC(munchmo_state::vblank_irq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mnchmobl);
	PALETTE(config, m_palette, FUNC(munchmo_state::palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set_inputline(m_audiocpu, 0, ASSERT_LINE);

	// AY clock speeds confirmed to match known recording
	AY8910(config, m_ay8910[0], XTAL(15'000'000) / 8);
	//m_ay8910[0]->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910[0]->add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, m_ay8910[1], XTAL(15'000'000) / 8);
	//m_ay8910[1]->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910[1]->add_route(ALL_OUTPUTS, "mono", 0.50);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( joyfulr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m1j.10e", 0x0000, 0x2000, CRC(1fe86e25) SHA1(e13abc20741dfd8a260f354efda3b3a25c820343) )
	ROM_LOAD( "m2j.10d", 0x2000, 0x2000, CRC(b144b9a6) SHA1(efed5fd6ba941b2baa7c8a17fe7323172c8fb17c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mu.2j",   0x0000, 0x2000, CRC(420adbd4) SHA1(3da18cda97ca604dc074b50c4f36287e0679224a) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "s1.10a",  0x0000, 0x1000, CRC(c0bcc301) SHA1(b8961e7bbced4dfe9c72f839ea9b89d3f2e629b2) )
	ROM_LOAD( "s2.10b",  0x1000, 0x1000, CRC(96aa11ca) SHA1(84438d6b27d520e95b8706c91c5c20de1785604c) )

	ROM_REGION( 0x2000, "tiles", 0 ) // 4x8
	ROM_LOAD( "b1.2c",   0x0000, 0x1000, CRC(8ce3a403) SHA1(eec5813076c31bb8534f7d1f83f2a397e552ed69) )
	ROM_LOAD( "b2.2b",   0x1000, 0x1000, CRC(0df28913) SHA1(485700d3b7f2bfcb970e8f9edb7d18ed9a708bd2) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "f1j.1g",  0x0000, 0x2000, CRC(93c3c17e) SHA1(902f458c4efe74187a58a3c1ecd146e343657977) )
	ROM_LOAD( "f2j.3g",  0x2000, 0x2000, CRC(b3fb5bd2) SHA1(51ff8b0bec092c9404944d6069c4493049604cb8) )
	ROM_LOAD( "f3j.5g",  0x4000, 0x2000, CRC(772a7527) SHA1(fe561d5323472e79051614a374e92aab17636055) )

	ROM_REGION( 0x2000, "monochrome_sprites", 0 )
	ROM_LOAD( "h",       0x0000, 0x2000, CRC(332584de) SHA1(9ef75a77e6cc298a315d80b7f2d24414827c7063) )

	ROM_REGION( 0x0100, "proms", 0 ) // color
	ROM_LOAD( "a2001.clr", 0x0000, 0x0100, CRC(1b16b907) SHA1(fc362174af128827b0b8119fdc1b5569598c087a) )
ROM_END

ROM_START( mnchmobl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m1.10e",  0x0000, 0x2000, CRC(a4bebc6a) SHA1(7c13b2b87168dee3c1b8e931487a56d0a528386e) )
	ROM_LOAD( "m2.10d",  0x2000, 0x2000, CRC(f502d466) SHA1(4da5a32b3903fb7fbef38fc385408b9390b5f57f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mu.2j",   0x0000, 0x2000, CRC(420adbd4) SHA1(3da18cda97ca604dc074b50c4f36287e0679224a) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "s1.10a",  0x0000, 0x1000, CRC(c0bcc301) SHA1(b8961e7bbced4dfe9c72f839ea9b89d3f2e629b2) )
	ROM_LOAD( "s2.10b",  0x1000, 0x1000, CRC(96aa11ca) SHA1(84438d6b27d520e95b8706c91c5c20de1785604c) )

	ROM_REGION( 0x2000, "tiles", 0 ) // 4x8
	ROM_LOAD( "b1.2c",   0x0000, 0x1000, CRC(8ce3a403) SHA1(eec5813076c31bb8534f7d1f83f2a397e552ed69) )
	ROM_LOAD( "b2.2b",   0x1000, 0x1000, CRC(0df28913) SHA1(485700d3b7f2bfcb970e8f9edb7d18ed9a708bd2) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "f1.1g",   0x0000, 0x2000, CRC(b75411d4) SHA1(d058a6c219676f8ba4e498215f5716c630bb1d20) )
	ROM_LOAD( "f2.3g",   0x2000, 0x2000, CRC(539a43ba) SHA1(a7b30c41d9fdb420ec8f0c6441432c1b2b69c4be) )
	ROM_LOAD( "f3.5g",   0x4000, 0x2000, CRC(ec996706) SHA1(e71e99061ce83068b0ec60ae97759a9d78c7cdf9) )

	ROM_REGION( 0x2000, "monochrome_sprites", 0 )
	ROM_LOAD( "h",       0x0000, 0x2000, CRC(332584de) SHA1(9ef75a77e6cc298a315d80b7f2d24414827c7063) )

	ROM_REGION( 0x0100, "proms", 0 ) // color
	ROM_LOAD( "a2001.clr", 0x0000, 0x0100, CRC(1b16b907) SHA1(fc362174af128827b0b8119fdc1b5569598c087a) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, joyfulr,  0,        mnchmobl, mnchmobl, munchmo_state, empty_init, ROT270, "SNK",                   "Joyful Road (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1983, mnchmobl, joyfulr,  mnchmobl, mnchmobl, munchmo_state, empty_init, ROT270, "SNK (Centuri license)", "Munch Mobile (US)",   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
