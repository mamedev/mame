// license:BSD-3-Clause
// copyright-holders: Luca Elia

/***************************************************************************

                              -= Blomby Car =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  68000
Video Chips  :  TI TPC1020AFN-084 (= Actel A1020A PL84C 9548)
Sound Chips  :  K-665 9546 (= M6295)

Blomby Car is effectively a bootleg/hack of Gaelco's World Rally given how:
- HW is very similar, down to dip-switches;
- How the game is a straight (inferior) clone;
- rips off most fonts;

Ranking screen BGM is a rip-off of Vapor Trail attract mode.

Service mode claims "Press P1 and P2 buttons to Exit", but that doesn't work.
The effective procedure is to hold P1 start and button 1 then release b1
and finally start;

To Do:

- Flip screen unused ?
- Better driving wheel(s) support, merge with World Rally implementation
- watrball: Check game speed, it depends on a bit we toggle.

***************************************************************************/

#include "emu.h"

#include "gaelco_wrally_sprites.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_okibank(*this, "okibank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprites(*this, "sprites"),
		m_vram(*this, "vram_%u", 0U),
		m_scroll(*this, "scroll_%u", 0U),
		m_spriteram(*this, "spriteram")
	{
	}

	void base(machine_config &config);

protected:
	virtual void video_start() override;

	void common_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_okibank;

private:
	void okibank_w(uint8_t data);
	template<int Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );

	void oki_map(address_map &map);

	// devices
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<gaelco_wrally_sprites_device> m_sprites;

	// memory pointers
	required_shared_ptr_array<uint16_t, 2> m_vram;
	required_shared_ptr_array<uint16_t, 2> m_scroll;
	required_shared_ptr<uint16_t> m_spriteram;

	// video-related
	tilemap_t *m_tilemap[2]{};

};

class blmbycar_state : public base_state
{
public:
	blmbycar_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_pot_wheel_io(*this, "POT_WHEEL"),
		m_opt_wheel_io(*this, "OPT_WHEEL")
	{
	}

	void blmbycar(machine_config &config);

	void init_blmbycar();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void pot_wheel_reset_w(uint8_t data);
	void pot_wheel_shift_w(uint8_t data);
	uint16_t pot_wheel_r();
	uint16_t opt_wheel_r();

	void prg_map(address_map &map);

	required_ioport m_pot_wheel_io;
	required_ioport m_opt_wheel_io;

	// input-related
	uint8_t m_pot_wheel = 0;
	uint8_t m_old_val = 0;
};

class watrball_state : public base_state
{
public:
	watrball_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag)
	{
	}

	void watrball(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint16_t unk_r();

	void prg_map(address_map &map);

	uint8_t m_retvalue = 0;
};


/***************************************************************************

Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows the background
        W       shows the foreground
        A       shows the sprites

        Keys can be used together!


    [ 2 Scrolling Layers ]

    The Tilemaps are 64 x 32 tiles in size (1024 x 512).
    Tiles are 16 x 16 x 4, with 32 color codes and 2 priority
    levels (wrt sprites). Each tile needs 4 bytes.

    [ 1024? Sprites ]

    They use the same graphics the tilemaps use (16 x 16 x 4 tiles)
    with 16 color codes and 2 levels of priority


***************************************************************************/

/***************************************************************************


                                Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code
        2.w     fedc ba98 ---- ----
                ---- ---- 7--- ----     Flip Y
                ---- ---- -6-- ----     Flip X
                ---- ---- --5- ----     Priority (0 = Low)
                ---- ---- ---4 3210     Color

***************************************************************************/

static constexpr uint8_t DIM_NX = 0x40;
static constexpr uint8_t DIM_NY = 0x20;

template<int Layer>
TILE_GET_INFO_MEMBER(base_state::get_tile_info)
{
	uint16_t const code = m_vram[Layer][tile_index * 2 + 0];
	uint16_t const attr = m_vram[Layer][tile_index * 2 + 1];
	tileinfo.set(0,
			code,
			attr & 0x1f,
			TILE_FLIPYX((attr >> 6) & 3));

	tileinfo.category = BIT(attr, 5);
}

/***************************************************************************


                                Video Init


***************************************************************************/

void base_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(base_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, DIM_NX, DIM_NY );
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(base_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, DIM_NX, DIM_NY );
	m_tilemap[1]->set_transparent_pen(0);
}

/***************************************************************************


                                Screen Drawing


***************************************************************************/

uint32_t base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprites->draw_sprites(cliprect, m_spriteram, flip_screen());

	m_tilemap[0]->set_scrolly(0, m_scroll[0][0]);
	m_tilemap[0]->set_scrollx(0, m_scroll[0][1]);

	m_tilemap[1]->set_scrolly(0, m_scroll[1][0] + 1);
	m_tilemap[1]->set_scrollx(0, m_scroll[1][1] + 5);

	screen.priority().fill(0, cliprect);

	bitmap.fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	m_sprites->mix_sprites(bitmap, cliprect, 0);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 1, 1);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 1, 1);

	m_sprites->mix_sprites(bitmap, cliprect, 1);

	return 0;
}


/***************************************************************************


                                Sound Banking


***************************************************************************/

// The top 64k of samples are banked (16 banks total)

void base_state::okibank_w(uint8_t data)
{
	m_okibank->set_entry(data & 0x0f);
}

/***************************************************************************


                                Input Handling


***************************************************************************/

// Preliminary potentiometric wheel support

void blmbycar_state::pot_wheel_reset_w(uint8_t data)
{
	m_pot_wheel = m_pot_wheel_io->read() & 0xff;
}

void blmbycar_state::pot_wheel_shift_w(uint8_t data)
{
	if ((m_old_val == 0xff) && (data == 0))
		m_pot_wheel <<= 1;
	m_old_val = data;
}

uint16_t blmbycar_state::pot_wheel_r()
{
	return ((m_pot_wheel & 0x80) ? 0x04 : 0) | (machine().rand() & 0x08);
}


// Preliminary optical wheel support

uint16_t blmbycar_state::opt_wheel_r()
{
	return ((m_opt_wheel_io->read() & 0xff) << 8) | 0xff;
}


/***************************************************************************


                                Video Handling


***************************************************************************/

template<int Layer>
void base_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset / 2);
}


/***************************************************************************


                                Memory Maps


***************************************************************************/

void base_state::common_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).writeonly().share("unk_100000");                          // ???
	map(0x104000, 0x105fff).ram().w(FUNC(base_state::vram_w<1>)).share(m_vram[1]); // Layer 1
	map(0x106000, 0x107fff).ram().w(FUNC(base_state::vram_w<0>)).share(m_vram[0]); // Layer 0
	map(0x108000, 0x10bfff).writeonly().share("unk_108000");                          // ???
	map(0x10c000, 0x10c003).writeonly().share(m_scroll[1]);
	map(0x10c004, 0x10c007).writeonly().share(m_scroll[0]);
	map(0x200000, 0x203fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette").mirror(0x4000);
	map(0x440000, 0x441fff).ram();
	map(0x444000, 0x445fff).writeonly().share(m_spriteram); // (size?)
	map(0x700000, 0x700001).portr("DSW");
	map(0x700002, 0x700003).portr("P1_P2");
	map(0x70000d, 0x70000d).w(FUNC(base_state::okibank_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfec000, 0xfeffff).ram();
}

void blmbycar_state::prg_map(address_map &map)
{
	common_map(map);

	map(0x700004, 0x700005).r(FUNC(blmbycar_state::opt_wheel_r));
	map(0x700006, 0x700007).portr("UNK");
	map(0x700008, 0x700009).r(FUNC(blmbycar_state::pot_wheel_r));
	map(0x70000a, 0x70000b).nopw();                                                // ? Wheel
	map(0x70006a, 0x70006b).nopr();                                                // Wheel (potentiometer)
	map(0x70006b, 0x70006b).w(FUNC(blmbycar_state::pot_wheel_reset_w));
	map(0x70007a, 0x70007b).nopr();
	map(0x70007b, 0x70007b).w(FUNC(blmbycar_state::pot_wheel_shift_w));
}

uint16_t watrball_state::unk_r()
{
	if (!machine().side_effects_disabled())
		m_retvalue ^= 0x0008; // must toggle.. but not vblank?
	return m_retvalue;
}

void watrball_state::prg_map(address_map &map)
{
	common_map(map);

	map(0x700006, 0x700007).nopr();                                                 // read
	map(0x700008, 0x700009).r(FUNC(watrball_state::unk_r));                                   // 0x0008 must toggle
	map(0x70000a, 0x70000b).nopw();                                               // ?? busy
}

void base_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}

/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( blmbycar )

	PORT_START("DSW")       // $700000.w
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Joysticks" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(      0x0018, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0010, "Pot Wheel" ) // Preliminary
	PORT_DIPSETTING(      0x0008, "Opt Wheel" ) // Preliminary
	PORT_DIPSETTING(      0x0000, "invalid, breaks game" )   // Time goes to 0 rally fast!
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 1-2" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:1" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Credits To Start" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x4000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1_P2") // $700002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("DSW", 0x18, EQUALS, 0x18)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(1) PORT_CONDITION("DSW", 0x18, EQUALS, 0x18)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("DSW", 0x18, EQUALS, 0x18)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("DSW", 0x18, EQUALS, 0x18)
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW", 0x18, NOTEQUALS, 0x18)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Gear Shift") PORT_TOGGLE PORT_CONDITION("DSW", 0x18, NOTEQUALS, 0x18)
	// Service mode only, unused in-game
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("DSW", 0x18, EQUALS, 0x18)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Accelerator")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2  )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("OPT_WHEEL") // $700004.w
	PORT_BIT ( 0x00ff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1) PORT_REVERSE PORT_CONDITION("DSW", 0x18, EQUALS, 0x08) PORT_NAME("P1 Opt Wheel")

	PORT_START("POT_WHEEL")
	PORT_BIT ( 0x00ff, 0x0080, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1) PORT_REVERSE PORT_CONDITION("DSW", 0x18, EQUALS, 0x10) PORT_NAME("P1 Pot Wheel")

	PORT_START("UNK")       // $700006.w
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


static INPUT_PORTS_START( watrball )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:8,7") // Affects timer
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )    // 180 Seconds
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )    // 150 Seconds
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )    // 120 Seconds
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )    // 100 Seconds
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:4" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 1-2" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:1" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:1" )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2  )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2  )
INPUT_PORTS_END

/***************************************************************************


                                Graphics Layouts


***************************************************************************/

// 16x16x4 tiles (made of four 8x8 tiles)/
static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(8*8*2,1) },
	{ STEP8(0,8), STEP8(8*8*1,8) },
	16*16
};

// Layers both use the first $20 color codes. Sprites the next $10
static GFXDECODE_START( gfx_blmbycar )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4, 0x0, 64*8 ) // [0] Layers + Sprites
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/

void blmbycar_state::machine_start()
{
	save_item(NAME(m_pot_wheel));
	save_item(NAME(m_old_val));

	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

void blmbycar_state::machine_reset()
{
	m_pot_wheel = 0;
	m_old_val = 0;
}


void base_state::base(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);   // 12MHz
	m_maincpu->set_vblank_int("screen", FUNC(blmbycar_state::irq1_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(0x180, 0x100);
	screen.set_screen_update(FUNC(blmbycar_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_blmbycar);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 0x2000);

	BLMBYCAR_SPRITES(config, m_sprites, 0, m_palette, gfx_blmbycar);
	m_sprites->set_screen("screen");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.set_addrmap(0, &blmbycar_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void blmbycar_state::blmbycar(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &blmbycar_state::prg_map);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_visarea_full();
}

void watrball_state::machine_start()
{
	save_item(NAME(m_retvalue));

	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

void watrball_state::machine_reset()
{
	m_retvalue = 0;
}

void watrball_state::watrball(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &watrball_state::prg_map);

	// video hardware
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_visarea(0, 0x180-1, 16, 0x100-1);
}


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

                                Blomby Car
Abm & Gecas, 1990.

CPU : MC68000P12
SND : Oki M6295 (samples only)
OSC : 30.000MHz, 24.000MHz & 1.00MHz resonator
DSW : 2 x 8
GFX : TI TPC1020AFN-084

***************************************************************************/

ROM_START( blmbycar )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 code
	ROM_LOAD16_BYTE( "bcrom4.bin", 0x000000, 0x080000, CRC(06d490ba) SHA1(6d113561b474bf613c6b91c9525a52025ae65ab7) )
	ROM_LOAD16_BYTE( "bcrom6.bin", 0x000001, 0x080000, CRC(33aca664) SHA1(04fff492654d3edac62e9d35808e5946bcc78cbb) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "bc_rom7",   0x000000, 0x080000, CRC(e55ca79b) SHA1(4453a6ae0518832f437ab701c28cb2f32920f8ba) )
	ROM_LOAD( "bc_rom8",   0x080000, 0x080000, CRC(cdf38c96) SHA1(3273c29b6a01a7296d06fc653120f8c615195d2c) )
	ROM_LOAD( "bc_rom9",   0x100000, 0x080000, CRC(0337ab3d) SHA1(18c72cd640c7b599390dffaeb670f5832202bf06) )
	ROM_LOAD( "bc_rom10",  0x180000, 0x080000, CRC(5458917e) SHA1(c8dd5a391cc20a573e27a140b185893a8c04859e) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM (banked)
	ROM_LOAD( "bc_rom1",     0x000000, 0x080000, CRC(ac6f8ba1) SHA1(69d2d47cdd331bde5a8973d29659b3f8520452e7) )
	ROM_LOAD( "bc_rom2",     0x080000, 0x080000, CRC(a4bc31bf) SHA1(f3d60141a91449a73f6cec9f4bc5d95ca7911e19) )
ROM_END

ROM_START( blmbycaru )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 code
	ROM_LOAD16_BYTE( "bc_rom4", 0x000000, 0x080000, CRC(76f054a2) SHA1(198efd152b13033e5249119ca48b9e0f6351b0b9) )
	ROM_LOAD16_BYTE( "bc_rom6", 0x000001, 0x080000, CRC(2570b4c5) SHA1(706465950023a6ef7c85ceb9c76246d7556b3859) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "bc_rom7",   0x000000, 0x080000, CRC(e55ca79b) SHA1(4453a6ae0518832f437ab701c28cb2f32920f8ba) )
	ROM_LOAD( "bc_rom8",   0x080000, 0x080000, CRC(cdf38c96) SHA1(3273c29b6a01a7296d06fc653120f8c615195d2c) )
	ROM_LOAD( "bc_rom9",   0x100000, 0x080000, CRC(0337ab3d) SHA1(18c72cd640c7b599390dffaeb670f5832202bf06) )
	ROM_LOAD( "bc_rom10",  0x180000, 0x080000, CRC(5458917e) SHA1(c8dd5a391cc20a573e27a140b185893a8c04859e) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM (banked)
	ROM_LOAD( "bc_rom1",     0x000000, 0x080000, CRC(ac6f8ba1) SHA1(69d2d47cdd331bde5a8973d29659b3f8520452e7) )
	ROM_LOAD( "bc_rom2",     0x080000, 0x080000, CRC(a4bc31bf) SHA1(f3d60141a91449a73f6cec9f4bc5d95ca7911e19) )
ROM_END

/*

Waterball by ABM (sticker on the PCB 12-3-96)
The PCB has some empty sockets, maybe it was used for other games since it has no markings.

The game has fonts identical to World Rally and obviously Blomby Car ;)

1x 68k
1x oki 6295
1x OSC 30mhz
1x OSC 24mhz
1x FPGA
1x dispswitch

*/

ROM_START( watrball )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 code
	ROM_LOAD16_BYTE( "rom4.bin", 0x000000, 0x020000, CRC(bfbfa720) SHA1(d6d14c0ba545eb7adee7190da2d3db1c6dd00d75) )
	ROM_LOAD16_BYTE( "rom6.bin", 0x000001, 0x020000, CRC(acff9b01) SHA1(b85671bcc4f03fdf05eb1c9b5d4143e33ec1d7db) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "rom7.bin",   0x000000, 0x080000, CRC(e7e5c311) SHA1(5af1a666bf23c5505d120d81fb942f5c49341861) )
	ROM_LOAD( "rom8.bin",   0x080000, 0x080000, CRC(fd27ce6e) SHA1(a472a8cc25818427d2870518649780146e51835b) )
	ROM_LOAD( "rom9.bin",   0x100000, 0x080000, CRC(122cc0ad) SHA1(27cdb19fa082089e47c5cdb44742cfd93aa23a00) )
	ROM_LOAD( "rom10.bin",  0x180000, 0x080000, CRC(22a2a706) SHA1(c7350a94a857e0007d7fc0076b44a3d62693cb6c) )

	ROM_REGION( 0x100000, "oki", 0 )    // ADPCM (banked)
	ROM_LOAD( "rom1.bin",     0x000000, 0x080000, CRC(7f88dee7) SHA1(d493b961fa4631185a33faee7f61786430707209))
//  ROM_LOAD( "rom2.bin",     0x080000, 0x080000, // not populated for this game
ROM_END


void blmbycar_state::init_blmbycar()
{
	uint16_t *RAM = (uint16_t *) memregion("maincpu")->base();
	size_t size = memregion("maincpu")->bytes() / 2;
	for (int i = 0; i < size; i++)
	{
		uint16_t x = RAM[i];
		x = (x & ~0x0606) | ((x & 0x0202) << 1) | ((x & 0x0404) >> 1);
		RAM[i] = x;
	}
}

} // anonymous namespace


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1994, blmbycar,  0,        blmbycar, blmbycar, blmbycar_state, init_blmbycar, ROT0, "ABM & Gecas", "Blomby Car (Version 1P0)",                MACHINE_SUPPORTS_SAVE )
GAME( 1994, blmbycaru, blmbycar, blmbycar, blmbycar, blmbycar_state, empty_init,    ROT0, "ABM & Gecas", "Blomby Car (Version 1P0, not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, watrball,  0,        watrball, watrball, watrball_state, empty_init,    ROT0, "ABM",         "Water Balls",                        MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )
