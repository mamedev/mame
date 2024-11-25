// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Angel Kids / Space Position hardware driver

 driver by David Haywood
 with some help from Steph (DSWs, Inputs, other
 bits here and there)

2 Board System, Uses Boards X090-PC-A & X090-PC-B

Both games appear to be joint Sega / Nasco efforts
(although all I see in Angel Kids is 'Exa Planning'
 but I think that has something to do with Nasco   )

Space Position is encrypted, the main processor is
D317-0005 (NEC Z80 Custom), see machine/segacrpt.cpp
for details on this encryption scheme

*/

// started 23/01/2002

/* notes / TODO:

* Unknown Reads / Writes
* Whats the PROM for? nothing important?
  the progress sprite on the side of the screen re-appears at the bottom when you get
  to the top, but the wrap-around is needed for other things, actual game bug?
* Angel Kids service mode doesn't seem to work, did it ever?
* Enable / disable tilemap bits might be wrong

*/

/* readme's

------------------------------------------------------------------------

Angel Kids
833-6599-01
Sega 1988

Nasco X090-PC-A  (Sega 837-6600)

 SW1   SW2


 8255

 8255

       11429 6116 Z80   YM2203 YM2203


 11424 11425 11426 11427  -  -  -  -  5M5165 11428  Z80
                                                         4MHz

                                                         6MHz


Nasco X090-PC-B

                                  2016-55
11437  11445    2016-55  2016-55             U5
11436  11444
11435  11443
11434  11442
11433  11441                  2016-55    2016-55
11432  11440
11431  11439    11446         2016-55

                                             11148
                                             11147
   2016-55 2016-55 2016-55

                                               18.432MHz

11430  11438

------------------------------------------------------------------------

Space Position (JPN Ver.)
(c)1986 Sega / Nasco
X090-PC-A 171-5383
X090-PC-B 171-5384
834-6089 SPACE POSITION
Sticker reading 860723.0883E


CPU :D317-0005 (NEC Z80 Custom)
Sound   :NEC D780C-1
    :YM2203C x 2
OSC :4.000MHz 6.000MHz
    :18.432MHz


EPR-10120.C1 prg
EPR-10121.C2  |
EPR-10122.C3  |
EPR-10123.C4  |
EPR-10124.C5  |
EPR-10125.C10/

EPR-10126.D4 snd

EPR-10127.06
EPR-10128.07
EPR-10129.08
EPR-10130.14
EPR-10131.15
EPR-10132.16

EPR-10133.17

EPR-10134.18
EPR-10135.19

63S081N.U5


--- Team Japump!!! ---
Dumped by Chackn
02/25/2000

------------------------------------------------------------------------

*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/segacrp2_device.h"
#include "machine/i8255.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class angelkds_state : public driver_device
{
public:
	angelkds_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram%u", 0U),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void angelkds(machine_config &config);
	void spcpostn(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_bgvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	required_memory_bank m_mainbank;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;

	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap[2]{};
	uint8_t m_txbank = 0;
	uint8_t m_bgbank[2]{};

	uint8_t m_sound[4]{};
	uint8_t m_sound2[4]{};
	uint8_t m_layer_ctrl = 0;

	void cpu_bank_w(uint8_t data);
	void main_sound_w(offs_t offset, uint8_t data);
	uint8_t main_sound_r(offs_t offset);
	void audio_sound_w(offs_t offset, uint8_t data);
	uint8_t audio_sound_r(offs_t offset);
	void txvideoram_w(offs_t offset, uint8_t data);
	void txbank_w(uint8_t data);
	template <uint8_t Which> void bgvideoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void bgbank_w(uint8_t data);
	template <uint8_t Which> void bgscroll_w(uint8_t data);
	void layer_ctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int enable_n);

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
};



// Text Layer Tilemap

TILE_GET_INFO_MEMBER(angelkds_state::get_tx_tile_info)
{
	int tileno = m_txvideoram[tile_index] + (m_txbank * 0x100);
	tileinfo.set(0, tileno, 0, 0);
}

void angelkds_state::txvideoram_w(offs_t offset, uint8_t data)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

void angelkds_state::txbank_w(uint8_t data)
{
	if (m_txbank != data)
	{
		m_txbank = data;
		m_tx_tilemap->mark_all_dirty();
	}
}

// Top (0) and Bottom (1) Half Background Tilemap

template <uint8_t Which>
TILE_GET_INFO_MEMBER(angelkds_state::get_bg_tile_info)
{
	int tileno = m_bgvideoram[Which][tile_index];

	tileno += m_bgbank[Which] * 0x100 ;
	tileinfo.set(Which + 1, tileno, Which, 0);
}

template <uint8_t Which>
void angelkds_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[Which][offset] = data;
	m_bg_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void angelkds_state::bgbank_w(uint8_t data)
{
	if (m_bgbank[Which] != data)
	{
		m_bgbank[Which] = data;
		m_bg_tilemap[Which]->mark_all_dirty();
	}
}

template <uint8_t Which>
void angelkds_state::bgscroll_w(uint8_t data)
{
	m_bg_tilemap[Which]->set_scrollx(0, data);
}

void angelkds_state::layer_ctrl_w(uint8_t data)
{
	m_layer_ctrl = data;
}

/* Sprites

the sprites are similar to the tilemaps in the sense that there is
a split down the middle of the screen

*/

void angelkds_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int enable_n)
{
	const uint8_t *source = m_spriteram + 0x100 - 4;
	const uint8_t *finish = m_spriteram;
	gfx_element *gfx = m_gfxdecode->gfx(3);

	while (source >= finish)
	{
	/*

	nnnn nnnn - EeFf B?cc - yyyy yyyy - xxxx xxxx

	n = sprite number
	E = Sprite Enabled in Top Half of Screen
	e = Sprite Enabled in Bottom Half of Screen
	F = Flip Y
	f = Flip X
	B = Tile Bank
	? = unknown, nothing / unused? recheck
	c = color
	y = Y position
	x = X position

	*/
		uint16_t tile_no = source[0];
		uint8_t attr = source[1];
		uint8_t ypos = source[2];
		uint8_t xpos = source[3];

		uint8_t enable = attr & 0xc0;
		uint8_t flipx = (attr & 0x10) >> 4;
		uint8_t flipy = (attr & 0x20) >> 5;
		uint8_t bank = attr & 0x08;
		uint8_t color = attr & 0x03;

		if (bank)
			tile_no += 0x100;

		ypos = 0xff - ypos;

		if (enable & enable_n)
		{
			gfx->transpen(bitmap, cliprect, tile_no, color * 4, flipx, flipy, xpos, ypos, 15);
			// wraparound
			if (xpos > 240)
				gfx->transpen(bitmap, cliprect, tile_no, color * 4, flipx, flipy, xpos - 256, ypos, 15);
			// wraparound
			if (ypos > 240)
			{
				gfx->transpen(bitmap, cliprect, tile_no, color * 4, flipx, flipy, xpos, ypos - 256, 15);
				// wraparound
				if (xpos > 240)
					gfx->transpen(bitmap, cliprect, tile_no, color * 4, flipx, flipy, xpos - 256, ypos - 256, 15);
			}
		}

		source -= 0x04;
	}
}


// Video Start & Update

void angelkds_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(angelkds_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tx_tilemap->set_transparent_pen(0);

	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(angelkds_state::get_bg_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap[1]->set_transparent_pen(15);

	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(angelkds_state::get_bg_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap[0]->set_transparent_pen(15);
}

// enable bits are uncertain

uint32_t angelkds_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = screen.visible_area();
	rectangle clip;

	bitmap.fill(0x3f, cliprect); // is there a register controlling the colour?, we currently use the last colour of the tx palette

	// draw top of screen
	clip.set(8 * 0, 8 * 16 - 1, visarea.min_y, visarea.max_y);

	if ((m_layer_ctrl & 0x80) == 0x00)
		m_bg_tilemap[0]->draw(screen, bitmap, clip, 0, 0);

	draw_sprites(bitmap, clip, 0x80);

	if ((m_layer_ctrl & 0x20) == 0x00)
		m_tx_tilemap->draw(screen, bitmap, clip, 0, 0);

	// draw bottom of screen
	clip.set(8 * 16, 8 * 32 - 1, visarea.min_y, visarea.max_y);

	if ((m_layer_ctrl & 0x40) == 0x00)
		m_bg_tilemap[1]->draw(screen, bitmap, clip, 0, 0);

	draw_sprites(bitmap, clip, 0x40);

	if ((m_layer_ctrl & 0x20) == 0x00)
		m_tx_tilemap->draw(screen, bitmap, clip, 0, 0);

	return 0;
}

// CPU Banking

void angelkds_state::cpu_bank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x0f);   // shall we check (data & 0x0f) < # of available banks (8 or 10 resp.)?
}


/*** Memory Structures

Angel Kids:
I would have expected f003 to be the scroll register for the bottom
part of the screen, in the attract mode this works fine, but in the
game it doesn't, so maybe it wasn't really hooked up and instead
only one of the register (f001) is used for both part?

 update, it is correct, the screen is meant to split in two when
 the kid goes what would be offscreen, just looked kinda odd

Interesting note, each Bank in the 0x8000 - 0xbfff appears to
contain a level.

*/

void angelkds_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe3ff).ram().w(FUNC(angelkds_state::bgvideoram_w<0>)).share(m_bgvideoram[0]); // Top Half of Screen
	map(0xe400, 0xe7ff).ram().w(FUNC(angelkds_state::bgvideoram_w<1>)).share(m_bgvideoram[1]); // Bottom Half of Screen
	map(0xe800, 0xebff).ram().w(FUNC(angelkds_state::txvideoram_w)).share(m_txvideoram);
	map(0xec00, 0xecff).ram().share(m_spriteram);
	map(0xed00, 0xedff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xee00, 0xeeff).ram().w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xef00, 0xefff).ram();
	map(0xf000, 0xf000).w(FUNC(angelkds_state::bgbank_w<0>));
	map(0xf001, 0xf001).w(FUNC(angelkds_state::bgscroll_w<0>));
	map(0xf002, 0xf002).w(FUNC(angelkds_state::bgbank_w<1>));
	map(0xf003, 0xf003).w(FUNC(angelkds_state::bgscroll_w<1>));
	map(0xf004, 0xf004).w(FUNC(angelkds_state::txbank_w));
	map(0xf005, 0xf005).w(FUNC(angelkds_state::layer_ctrl_w));
}

void angelkds_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
	map(0x8000, 0xbfff).bankr(m_mainbank);
}

void angelkds_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw(); // 00 on start-up, not again

	map(0x40, 0x43).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x80, 0x83).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0xc0, 0xc3).rw(FUNC(angelkds_state::main_sound_r), FUNC(angelkds_state::main_sound_w)); // 02 various points
}





// audio CPU

void angelkds_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xaaa9, 0xaaa9).nopr();
	map(0xaaab, 0xaaab).nopr();
	map(0xaaac, 0xaaac).nopr();
}

void angelkds_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x40, 0x41).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x80, 0x83).rw(FUNC(angelkds_state::audio_sound_r), FUNC(angelkds_state::audio_sound_w)); // spcpostn
}


// Input Ports

#define ANGELDSK_PLAYERS_INPUT( player ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(player) PORT_8WAY


static INPUT_PORTS_START( angelkds )
	/*
	    Free Play: Set SW1:1-8 ON (A:Free Play & B:Free Play).
	    Sound Test: Set SW1:1-8 ON (A:Free Play & B:Free Play), hold test switch and reboot.
	    Joystick Test: Set SW1:1-7 ON & SW1:8 OFF (A:Free Play & B:3C_1C), hold test switch and reboot.
	    Joystick Test Coin_A & Coin_B seem to be switched, only works when setting A to 3C_1C and B to Free Play.
	*/
	PORT_START("I40")       // inport $40
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("I41")       // inport $41
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, "High Score Characters" )         PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "20k, 50k, 100k, 200k and 500k" )
	PORT_DIPSETTING(    0x08, "50k, 100k, 200k and 500k" )
	PORT_DIPSETTING(    0x04, "100k, 200k and 500k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:7,8") // Stored at 0xc023
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )


	PORT_START("I80")       // inport $80
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("I81")       // inport $81
	ANGELDSK_PLAYERS_INPUT( 1 )

	PORT_START("I82")       // inport $82
	ANGELDSK_PLAYERS_INPUT( 2 )

INPUT_PORTS_END

static INPUT_PORTS_START( spcpostn )
	PORT_START("I40")       // inport $40
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )

	PORT_START("I41")       // inport $41
	PORT_DIPNAME( 0x01, 0x01, DEF_STR(Allow_Continue ) )    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Obstruction Car" )           PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x08, "Time Limit" )                PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1:10" )
	PORT_DIPSETTING(    0x04, "1:20" )
	PORT_DIPSETTING(    0x08, "1:30" )
	PORT_DIPSETTING(    0x0c, "1:40" )
	PORT_DIPNAME( 0x30, 0x20, "Power Down" )                PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "Slow" )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )            // Listed as "Unused"

	PORT_START("I80")       // inport $80
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("I81")       // inport $81
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1) // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1) // probably unused

	PORT_START("I82")       // inport $82
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2) // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2) // probably unused

INPUT_PORTS_END

/* Sound Hardware

TODO: verify / correct things
seems a bit strange are all the addresses really
sound related ?

*/

void angelkds_state::main_sound_w(offs_t offset, uint8_t data)
{
	m_sound[offset] = data;
}

uint8_t angelkds_state::main_sound_r(offs_t offset)
{
	return m_sound2[offset];
}

void angelkds_state::audio_sound_w(offs_t offset, uint8_t data)
{
	m_sound2[offset] = data;
}

uint8_t angelkds_state::audio_sound_r(offs_t offset)
{
	return m_sound[offset];
}


/* Graphics Decoding

all the 8x8 tiles are in one format, the 16x16 sprites in another

*/

static const gfx_layout angelkds_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0,4,  RGN_FRAC(1,2)+0,    RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11, 16,17,18,19, 24,25,26,27 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32  },
	16*32
};

static GFXDECODE_START( gfx_angelkds )
	GFXDECODE_ENTRY( "chars",          0, gfx_8x8x4_packed_msb,  0x30, 1  )
	GFXDECODE_ENTRY( "bgtiles_top",    0, gfx_8x8x4_packed_msb,  0,    16 )
	GFXDECODE_ENTRY( "bgtiles_bottom", 0, gfx_8x8x4_packed_msb,  0,    16 )
	GFXDECODE_ENTRY( "sprites",        0, angelkds_spritelayout, 0x20, 0x0d )
GFXDECODE_END

/* Machine Driver

 2 x Z80 (one for game, one for sound)
 2 x YM2203 (for sound)

 all fairly straightforward

*/

void angelkds_state::machine_start()
{
	uint8_t *rom = memregion("banked_roms")->base();
	m_mainbank->configure_entries(0, 16, &rom[0x0000], 0x4000);

	save_item(NAME(m_layer_ctrl));
	save_item(NAME(m_txbank));
	save_item(NAME(m_bgbank));
	save_item(NAME(m_sound));
	save_item(NAME(m_sound2));
}

void angelkds_state::machine_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_sound[i] = 0;
		m_sound2[i] = 0;
	}

	m_layer_ctrl = 0;
	m_txbank = 0;
	m_bgbank[0] = m_bgbank[1] = 0;
}

void angelkds_state::angelkds(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(6'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &angelkds_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &angelkds_state::main_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(angelkds_state::irq0_line_hold));

	Z80(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &angelkds_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &angelkds_state::sound_portmap);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("I40");
	ppi0.in_pb_callback().set_ioport("I41");
	ppi0.in_pc_callback().set_constant(0xff); // or left inputs don't work
	ppi0.out_pc_callback().set(FUNC(angelkds_state::cpu_bank_w));

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("I80");
	ppi1.in_pb_callback().set_ioport("I81");
	ppi1.in_pc_callback().set_ioport("I82");

	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(angelkds_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_angelkds);
	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 0x100);

	SPEAKER(config, "mono").front_center();

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(4'000'000)));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.65);
	ym1.add_route(1, "mono", 0.65);
	ym1.add_route(2, "mono", 0.65);
	ym1.add_route(3, "mono", 0.45);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(4'000'000)));
	ym2.add_route(0, "mono", 0.65);
	ym2.add_route(1, "mono", 0.65);
	ym2.add_route(2, "mono", 0.65);
	ym2.add_route(3, "mono", 0.45);
}

void angelkds_state::spcpostn(machine_config &config)
{
	angelkds(config);

	// encryption
	sega_317_0005_device &maincpu(SEGA_317_0005(config.replace(), m_maincpu, XTAL(6'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &angelkds_state::main_map);
	maincpu.set_addrmap(AS_IO, &angelkds_state::main_portmap);
	maincpu.set_vblank_int("screen", FUNC(angelkds_state::irq0_line_hold));
	maincpu.set_addrmap(AS_OPCODES, &angelkds_state::decrypted_opcodes_map);
	maincpu.set_decrypted_tag(m_decrypted_opcodes);
}


ROM_START( angelkds )
	// Nasco X090-PC-A  (Sega 837-6600)
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "epr-11428.c10", 0x00000, 0x08000, CRC(90daacd2) SHA1(7e50ad1cbed0c1e6bad04ef1611cad25538c905f) )

	ROM_REGION( 0x40000, "banked_roms", 0 )
	ROM_LOAD( "epr-11424.c1",  0x00000, 0x08000, CRC(b55997f6) SHA1(7ed746becac1851f39591f1fdbeff64aa97d6206) )
	ROM_LOAD( "epr-11425.c2",  0x08000, 0x08000, CRC(299359de) SHA1(f531dd3bfe6f64e9e043cb4f85d5657455241dc7) )
	ROM_LOAD( "epr-11426.c3",  0x10000, 0x08000, CRC(5fad8bd3) SHA1(4d865342eb10dcfb779eee4ac1e159bb9ec140cb) )
	ROM_LOAD( "epr-11427.c4",  0x18000, 0x08000, CRC(ef920c74) SHA1(81c0fbe4ace5441e4cd99ba423e0190cc541da31) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-11429.d4",  0x00000, 0x08000, CRC(0ca50a66) SHA1(cccb081b447419138b1ebd309e7f291e392a44d5) )

	// Nasco X090-PC-B
	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "epr-11446",  0x00000, 0x08000, CRC(45052470) SHA1(c2312a9f814d6dbe42aa465147a04a2bd9b2aa1b) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-11447.f7",  0x08000, 0x08000, CRC(b3afc5b3) SHA1(376d527f60e9044f18d19a5535bca77606efbd4c) )
	ROM_LOAD( "epr-11448.h7",  0x00000, 0x08000, CRC(05dab626) SHA1(73feaca6e23c673a7d8c9e972714b20bd8f2d51e) )

	// both tilemaps on angelkds use the same GFX
	ROM_REGION( 0x40000, "bgtiles_top", 0 )
	ROM_LOAD( "epr-11437",  0x00000, 0x08000, CRC(a520b628) SHA1(2b51f59e760e740e5e6b06dad61bbc23fc84a72b) )
	ROM_LOAD( "epr-11436",  0x08000, 0x08000, CRC(469ab216) SHA1(8223f072a6f9135ff84841c95410368bcea073d8) )
	ROM_LOAD( "epr-11435",  0x10000, 0x08000, CRC(b0f8c245) SHA1(882e27eaceac46c397fdae8427a082caa7d6b7dc) )
	ROM_LOAD( "epr-11434",  0x18000, 0x08000, CRC(cbde81f5) SHA1(5d5b8e709c9dd09a45dfced6f3d4a9c52500da6b) )
	ROM_LOAD( "epr-11433",  0x20000, 0x08000, CRC(b63fa414) SHA1(25adcafd7e17ab0be0fed2ec44245124febd74b3) )
	ROM_LOAD( "epr-11432",  0x28000, 0x08000, CRC(00dc747b) SHA1(041b73aa48b45162af33b5f416ccc0c0dbbd995b) )
	ROM_LOAD( "epr-11431",  0x30000, 0x08000, CRC(ac2025af) SHA1(2aba145df3ccdb1a7f0fec524bd2de3f9aab4161) )
	ROM_LOAD( "epr-11430",  0x38000, 0x08000, CRC(d640f89e) SHA1(38fb67bcb2a3d1ad614fc62e42f22a66bc757137) )

	ROM_REGION( 0x40000, "bgtiles_bottom", 0 )
	ROM_LOAD( "epr-11445",  0x00000, 0x08000, CRC(a520b628) SHA1(2b51f59e760e740e5e6b06dad61bbc23fc84a72b) )
	ROM_LOAD( "epr-11444",  0x08000, 0x08000, CRC(469ab216) SHA1(8223f072a6f9135ff84841c95410368bcea073d8) )
	ROM_LOAD( "epr-11443",  0x10000, 0x08000, CRC(b0f8c245) SHA1(882e27eaceac46c397fdae8427a082caa7d6b7dc) )
	ROM_LOAD( "epr-11442",  0x18000, 0x08000, CRC(cbde81f5) SHA1(5d5b8e709c9dd09a45dfced6f3d4a9c52500da6b) )
	ROM_LOAD( "epr-11441",  0x20000, 0x08000, CRC(b63fa414) SHA1(25adcafd7e17ab0be0fed2ec44245124febd74b3) )
	ROM_LOAD( "epr-11440",  0x28000, 0x08000, CRC(00dc747b) SHA1(041b73aa48b45162af33b5f416ccc0c0dbbd995b) )
	ROM_LOAD( "epr-11439",  0x30000, 0x08000, CRC(ac2025af) SHA1(2aba145df3ccdb1a7f0fec524bd2de3f9aab4161) )
	ROM_LOAD( "epr-11438",  0x38000, 0x08000, CRC(d640f89e) SHA1(38fb67bcb2a3d1ad614fc62e42f22a66bc757137) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "63s081n.u5",   0x00,    0x20,    CRC(36b98627) SHA1(d2d54d92d1d47e7cc85104989ee421ce5d80a42a) )
ROM_END

ROM_START( spcpostn )
	// X090-PC-A 171-5383  (Sega 834-6089 SPACE POSITION)
	ROM_REGION( 0x8000, "maincpu", 0 ) // D317-0005 (NEC Z80 Custom)
	ROM_LOAD( "epr-10125.c10", 0x00000, 0x08000, CRC(bffd38c6) SHA1(af02907124343ddecd21439d25f1ebb81ef9f51a) ) // encrypted

	ROM_REGION( 0x40000, "banked_roms", 0 )
	ROM_LOAD( "epr-10120.c1",  0x00000, 0x08000, CRC(d6399f99) SHA1(4c7d19a8798e5a10b688bf793ca74f5170fd9b51) )
	ROM_LOAD( "epr-10121.c2",  0x08000, 0x08000, CRC(d4861560) SHA1(74d28c36a08880abbd3c398cc3e990e8986caccb) )
	ROM_LOAD( "epr-10122.c3",  0x10000, 0x08000, CRC(7a1bff1b) SHA1(e1bda8430fd632c1813dd78e0f210a358e1b0d2f) )
	ROM_LOAD( "epr-10123.c4",  0x18000, 0x08000, CRC(6aed2925) SHA1(75848c8086c460b72494da2367f592d7d5dcf9f1) )
	ROM_LOAD( "epr-10124.c5",  0x20000, 0x08000, CRC(a1d7ae6b) SHA1(ec81fecf63e0515cae2077e2623262227adfdf37) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // NEC D780C-1
	ROM_LOAD( "epr-10126.d4",  0x00000, 0x08000, CRC(ab17f852) SHA1(dc0db427ddb4df97bb40dfb6fc65cb9354a6b9ad) )

	// X090-PC-B 171-5384
	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "epr-10133.17",  0x00000, 0x08000, CRC(642e6609) SHA1(2dfb4cc66f89543b55ed2a5b914e2c9304e821ca) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-10134.18",  0x08000, 0x08000, CRC(c674ff88) SHA1(9f240910a1ffb7c9e09d2326de280e6a5dd84565) )
	ROM_LOAD( "epr-10135.19",  0x00000, 0x08000, CRC(0685c4fa) SHA1(6950d9ad9ec13236cf24e83e87adb62aa53af7bb) )

	ROM_REGION( 0x30000, "bgtiles_top", 0 )
	ROM_LOAD( "epr-10130.14",  0x10000, 0x08000, CRC(b68fcb36) SHA1(3943dd550b13f2911d56d8dad675410da79196e6) )
	ROM_LOAD( "epr-10131.15",  0x08000, 0x08000, CRC(de223817) SHA1(1860db0a19c926fcfaabe676cb57fff38c4df8e6) )
	ROM_LOAD( "epr-10132.16",  0x00000, 0x08000, CRC(2df8b1bd) SHA1(cad8befa3f2c158d2aa74073066ccd2b54e68825) )

	ROM_REGION( 0x18000, "bgtiles_bottom", 0 )
	ROM_LOAD( "epr-10127.06",  0x10000, 0x08000, CRC(b68fcb36) SHA1(3943dd550b13f2911d56d8dad675410da79196e6) )
	ROM_LOAD( "epr-10128.07",  0x08000, 0x08000, CRC(de223817) SHA1(1860db0a19c926fcfaabe676cb57fff38c4df8e6) )
	ROM_LOAD( "epr-10129.08",  0x00000, 0x08000, CRC(a6f21023) SHA1(8d573446a2d3d3428409707d0c59b118d1463131) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "63s081n.u5",   0x00,    0x20,    CRC(36b98627) SHA1(d2d54d92d1d47e7cc85104989ee421ce5d80a42a) )
ROM_END

} // anonymous namespace


GAME( 1988, angelkds, 0, angelkds, angelkds, angelkds_state, empty_init, ROT90, "Sega / Nasco?", "Angel Kids (Japan)" ,     MACHINE_SUPPORTS_SAVE) // Nasco not displayed but 'Exa Planning' is
GAME( 1986, spcpostn, 0, spcpostn, spcpostn, angelkds_state, empty_init, ROT90, "Sega / Nasco",  "Space Position (Japan)" , MACHINE_SUPPORTS_SAVE) // encrypted
