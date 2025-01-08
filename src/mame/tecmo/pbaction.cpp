// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

/***************************************************************************

Pinball Action memory map (preliminary)

driver by Nicola Salmoria


0000-9fff ROM
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff Background Video RAM
dc00-dfff Background Color RAM
e000-e07f Sprites
e400-e5ff Palette RAM

read:
e600      IN0
e601      IN1
e602      IN2
e604      DSW1
e605      DSW2
e606      watchdog reset????

write:
e600      interrupt enable
e604      flip screen
e606      bg scroll? not sure
e800      command for the sound CPU


Notes:
- pbactiont (Tecfri) has a ROM on a small subboard with two Z80:

 2-pin conn   12-pin conn
____||________||||||||||||____________________
|   ||                       _______________  |
|                            |Z8430A PS     | |
|                            |______________| |
|                              _____________  |
|                              |TMM2016BP-10| |
|                              |____________| |
|               __________   _______________  |
|               |74HCT273 |  |ROM17 D27128D | |
|                            |______________| |
|           _________     __________________  |
|           |74LS139|     |MK3880N-4 Z80 CPU| |
|                         |_________________| |
|           _________     __________________  |
|           |74HCT259     |MK3880N-4 Z80 CPU| |
|                         |_________________| |
|                      _________  _________   |
|                      |74LS00P | |74LS32P |  |
|                                             |
|____________________________TECFRI S.A.______|

This subboard controls a vertical board panel with three 7-seg displays (like on a real pinball),
one for player 1 (7 digits), another for player 2 (7 digits) and the third for game scores (with
three groups: two digits - one digit - two digits).

One of the Z80s on this board is the main game Z80, the other is for the Pinball cabinet



Stephh's notes (based on the game Z80 code and some tests) :

  - There is an ingame bug that prevents you to get a bonus life at 1000000 points
    when you set the "Bonus Life" Dip Switch to "200k 1000k" :
      * Bonus life table index starts at 0x63c6 (8 * 2 bites, LSB first) :

          63C6: D6 63   "70k 200k"          -> 04 07 03 02 01 10
          63C8: DC 63   "70k 200k 1000k"    -> 04 07 03 02 02 01 01 10
          63CA: E4 63   "100k"              -> 03 01 01 10
          63CC: E8 63   "100k 300k"         -> 03 01 03 03 01 10
          63CE: EE 63   "100k 300k 1000k"   -> 03 01 03 03 02 01 01 10
          63D0: F6 63   "200k"              -> 03 02 01 10
          63D2: FA 63   "200k 1000k"        -> 03 02 01 10 !!!
          63D4: FE 63   "None"              -> 01 10

      * Each "pair" determines the digit number, then what shall be its value :

          digit  : 12 345 67
          number : 99.999.990

        Note that digit 1 is displayed outside the score box.
      * As each digit value can only be 00 to 09, 10 as a bonus life value
        means that you can't get anymore bonus life.
      * Now look at 7th table : first, you notice that it's the same as the
        6th table; then you find that the first bonus life at 200k and you see
        the end of table "marker" (01 10) instead of having 02 01.
      * As addresses and data are the same (after decryption in 'pbactio3'),
        this bug affects the 3 sets.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/segacrpt_device.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "pbactiont.lh"


namespace {

class pbaction_state : public driver_device
{
public:
	pbaction_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ctc(*this, "ctc"),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_work_ram(*this, "work_ram"),
		m_spriteram(*this, "spriteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void pbaction(machine_config &config);
	void pbactionx(machine_config &config);

	void init_pbaction2();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void sound_irq_clear(int state);

	void alt_sound_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;

private:
	// devices
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<z80ctc_device> m_ctc;

	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;
	required_shared_ptr<uint8_t> m_work_ram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	int16_t m_scroll = 0;

	emu_timer *m_soundcommand_timer = nullptr;
	uint8_t m_nmi_mask = 0;

	void sh_command_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(sound_trigger);
	void nmi_mask_w(uint8_t data);
	uint8_t sound_data_r();
	void sound_irq_ack_w(uint8_t data);
	uint8_t pbaction2_prot_kludge_r();
	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	void scroll_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

class pbaction_tecfri_state : public pbaction_state
{
public:
	pbaction_tecfri_state(const machine_config &mconfig, device_type type, const char *tag) :
		pbaction_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu"),
		m_ctc2(*this, "ctc2"),
		m_maintosublatch(*this, "maintosublatch"),
		//m_subtomainlatch(*this, "subtomainlatch"),
		m_digits(*this, "digit%u", 0U)
	{ }

	void pbactiont(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void sub_map(address_map &map) ATTR_COLD;
	void sub_io_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sub_trigger);
	emu_timer *m_subcommand_timer = nullptr;

	uint8_t subcpu_r();
	void subcpu_w(uint8_t data);

	void sub8000_w(int state);
	void sub8001_w(int state);
	void sub8008_w(uint8_t data);

	void subtomain_w(uint8_t data);

	required_device<z80_device> m_subcpu;
	required_device<z80ctc_device> m_ctc2;
	required_device<generic_latch_8_device> m_maintosublatch;
	//required_device<generic_latch_8_device> m_subtomainlatch;
	output_finder<24> m_digits;
	uint8_t m_outlatch = 0;
	uint32_t m_outdata = 0;
};


template <uint8_t Which>
void pbaction_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	Which ? m_fg_tilemap->mark_tile_dirty(offset) : m_bg_tilemap->mark_tile_dirty(offset);
}

template <uint8_t Which>
void pbaction_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	Which ? m_fg_tilemap->mark_tile_dirty(offset) : m_bg_tilemap->mark_tile_dirty(offset);
}

void pbaction_state::scroll_w(uint8_t data)
{
	m_scroll = data - 3;
	if (flip_screen())
		m_scroll = -m_scroll;

	m_bg_tilemap->set_scrollx(0, m_scroll);
	m_fg_tilemap->set_scrollx(0, m_scroll);
}

void pbaction_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 0x01);
}

TILE_GET_INFO_MEMBER(pbaction_state::get_bg_tile_info)
{
	int const attr = m_colorram[0][tile_index];
	int const code = m_videoram[0][tile_index] + 0x10 * (attr & 0x70);
	int const color = attr & 0x07;
	int const flags = (attr & 0x80) ? TILE_FLIPY : 0;

	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(pbaction_state::get_fg_tile_info)
{
	int const attr = m_colorram[1][tile_index];
	int const code = m_videoram[1][tile_index] + 0x10 * (attr & 0x30);
	int const color = attr & 0x0f;
	int const flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo.set(0, code, color, flags);
}

void pbaction_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pbaction_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pbaction_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void pbaction_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		// if next sprite is double size, skip this one
		if (offs > 0 && m_spriteram[offs - 4] & 0x80)
			continue;

		int sx = m_spriteram[offs + 3];
		int sy;

		if (m_spriteram[offs] & 0x80)
			sy = 225 - m_spriteram[offs + 2];
		else
			sy = 241 - m_spriteram[offs + 2];

		int flipx = m_spriteram[offs + 1] & 0x40;
		int flipy = m_spriteram[offs + 1] & 0x80;

		if (flip_screen())
		{
			if (m_spriteram[offs] & 0x80)
			{
				sx = 224 - sx;
				sy = 225 - sy;
			}
			else
			{
				sx = 240 - sx;
				sy = 241 - sy;
			}
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx((m_spriteram[offs] & 0x80) ? 3 : 2)->transpen(bitmap, cliprect, // normal or double size
				m_spriteram[offs],
				m_spriteram[offs + 1] & 0x0f,
				flipx, flipy,
				sx + (flip_screen() ? m_scroll : -m_scroll), sy, 0);
	}
}

uint32_t pbaction_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void pbaction_state::sh_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_soundcommand_timer->adjust(attotime::zero, 0);
}

TIMER_CALLBACK_MEMBER(pbaction_state::sound_trigger)
{
	m_ctc->trg0(0);
	m_ctc->trg0(1);
}

void pbaction_state::nmi_mask_w(uint8_t data)
{
	m_nmi_mask = data & 1;
	if (!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void pbaction_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram().share(m_work_ram);
	map(0xd000, 0xd3ff).ram().w(FUNC(pbaction_state::videoram_w<1>)).share(m_videoram[1]);
	map(0xd400, 0xd7ff).ram().w(FUNC(pbaction_state::colorram_w<1>)).share(m_colorram[1]);
	map(0xd800, 0xdbff).ram().w(FUNC(pbaction_state::videoram_w<0>)).share(m_videoram[0]);
	map(0xdc00, 0xdfff).ram().w(FUNC(pbaction_state::colorram_w<0>)).share(m_colorram[0]);
	map(0xe000, 0xe07f).ram().share(m_spriteram);
	map(0xe400, 0xe5ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xe600, 0xe600).portr("P1").w(FUNC(pbaction_state::nmi_mask_w));
	map(0xe601, 0xe601).portr("P2");
	map(0xe602, 0xe602).portr("SYSTEM");
	map(0xe604, 0xe604).portr("DSW1").w(FUNC(pbaction_state::flipscreen_w));
	map(0xe605, 0xe605).portr("DSW2");
	map(0xe606, 0xe606).nopr() /* ??? */ .w(FUNC(pbaction_state::scroll_w));
	map(0xe800, 0xe800).w(FUNC(pbaction_state::sh_command_w));
}

void pbaction_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
	map(0x8000, 0xbfff).rom().region("maincpu", 0x8000);
}


uint8_t pbaction_state::sound_data_r()
{
	if (!machine().side_effects_disabled())
		m_audiocpu->set_input_line(0, CLEAR_LINE);
	return m_soundlatch->read();
}

void pbaction_state::sound_irq_ack_w(uint8_t data)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	machine().scheduler().synchronize();
}

void pbaction_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8000).r(FUNC(pbaction_state::sound_data_r));
	map(0xffff, 0xffff).w(FUNC(pbaction_state::sound_irq_ack_w));
}

void pbaction_state::alt_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void pbaction_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x11).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x12, 0x13).nopw(); // unknown
	map(0x20, 0x21).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x30, 0x31).w("ay3", FUNC(ay8910_device::address_data_w));
}

void pbaction_tecfri_state::sub8000_w(int state)
{
	m_outlatch = state;

}

void pbaction_tecfri_state::sub8001_w(int state)
{
	// writes 01 , 00 to clock after writing data to 8000
	if (state)
	{
		m_outdata = (m_outdata << 1) | m_outlatch;
	}
}

void pbaction_tecfri_state::sub8008_w(uint8_t data)
{
	if (data != 0x00)
	{
		int base = 0;

		switch (data)
		{
		case 0x01: base = 6; break;
		case 0x02: base = 5; break;
		case 0x04: base = 4; break;
		case 0x08: base = 3; break;
		case 0x10: base = 2; break;
		case 0x20: base = 1; break;
		case 0x40: base = 0; break;

		default: break;
		}

		m_digits[base + 0] = (~m_outdata >> 0) & 0xff;
		m_digits[base + 7] = (~m_outdata >> 8) & 0xff;
		m_digits[base + 14] = (~m_outdata >> 16) & 0xff;

	}
	else
	{
		m_outdata = 0;
	}
}

void pbaction_tecfri_state::subtomain_w(uint8_t data)
{
	//m_subtomainlatch->write(data); // where does this go if it can't go to maincpu?
}

uint8_t pbaction_tecfri_state::subcpu_r()
{
	return 0x00; // other values stop the flippers from working? are there different inputs from the custom cabinet in here somehow?
//  return m_subtomainlatch->read();
}

void pbaction_tecfri_state::subcpu_w(uint8_t data)
{
	m_maintosublatch->write(data);
	m_subcommand_timer->adjust(attotime::zero, 0);
}

void pbaction_tecfri_state::sub_map(address_map &map)
{
	map(0x0000, 0x03ff).rom();
	map(0x4000, 0x47ff).ram();

	map(0x8000, 0x8007).w("suboutlatch", FUNC(hct259_device::write_d0));
	map(0x8008, 0x8008).w(FUNC(pbaction_tecfri_state::sub8008_w));

	map(0x8010, 0x8010).r(m_maintosublatch, FUNC(generic_latch_8_device::read));
	map(0x8018, 0x8018).w(FUNC(pbaction_tecfri_state::subtomain_w));
}

void pbaction_tecfri_state::sub_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ctc2, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void pbaction_tecfri_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(pbaction_tecfri_state::subcpu_r), FUNC(pbaction_tecfri_state::subcpu_w));
}

TIMER_CALLBACK_MEMBER(pbaction_tecfri_state::sub_trigger)
{
	m_ctc2->trg1(0);
	m_ctc2->trg1(1);
}


static INPUT_PORTS_START( pbaction )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x01, "70k 200k 1000k" )
	PORT_DIPSETTING(    0x04, "100k 300k 1000k" )
	PORT_DIPSETTING(    0x00, "70k 200k" )
	PORT_DIPSETTING(    0x03, "100k 300k" )
	PORT_DIPSETTING(    0x06, "200k 1000k" )                // see notes
	PORT_DIPSETTING(    0x02, "100k" )
	PORT_DIPSETTING(    0x05, "200k" )
	PORT_DIPSETTING(    0x07, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, "Extra" )         PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x00, "Difficulty (Flippers)" ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0x00, "Difficulty (Outlanes)" ) PORT_DIPLOCATION("SW2:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END

static const gfx_layout charlayout1 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};
static const gfx_layout charlayout2 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};
static const gfx_layout spritelayout1 =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(64,1) },
	{ STEP8(0,8), STEP8(128,8) },
	32*8
};
static const gfx_layout spritelayout2 =
{
	32,32,
	RGN_FRAC(1,6),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(64,1), STEP8(256,1), STEP8(320,1) },
	{ STEP8(0,8), STEP8(128,8), STEP8(512,8), STEP8(640,8) },
	128*8
};



static GFXDECODE_START( gfx_pbaction )
	GFXDECODE_ENTRY( "fgchars", 0x00000, charlayout1,    0, 16 )    //   0-127 characters
	GFXDECODE_ENTRY( "bgchars", 0x00000, charlayout2,  128,  8 )    // 128-255 background
	GFXDECODE_ENTRY( "sprites", 0x00000, spritelayout1,  0, 16 )    //   0-127 normal sprites
	GFXDECODE_ENTRY( "sprites", 0x01000, spritelayout2,  0, 16 )    //   0-127 large sprites
GFXDECODE_END


void pbaction_state::machine_start()
{
	m_soundcommand_timer = timer_alloc(FUNC(pbaction_state::sound_trigger), this);
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_scroll));
}

void pbaction_state::machine_reset()
{
	m_nmi_mask = 0;
	m_scroll = 0;
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void pbaction_state::vblank_irq(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void pbaction_state::sound_irq_clear(int state)
{
	if (state)
		m_audiocpu->set_input_line(0, CLEAR_LINE);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

void pbaction_state::pbaction(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pbaction_state::main_map);

	Z80(config, m_audiocpu, 12_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &pbaction_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &pbaction_state::sound_io_map);
	m_audiocpu->set_daisy_config(daisy_chain);

	Z80CTC(config, m_ctc, 12_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_audiocpu, 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(pbaction_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(pbaction_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pbaction);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, "ay1", 12_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.25);
	AY8910(config, "ay2", 12_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.25);
	AY8910(config, "ay3", 12_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void pbaction_state::pbactionx(machine_config &config)
{
	pbaction(config);

	SEGA_315_5128(config.replace(), m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pbaction_state::main_map);
	m_maincpu->set_addrmap(AS_OPCODES, &pbaction_state::decrypted_opcodes_map);
	downcast<sega_315_5128_device &>(*m_maincpu).set_decrypted_tag(":decrypted_opcodes");

	m_audiocpu->set_addrmap(AS_PROGRAM, &pbaction_state::alt_sound_map);
	m_audiocpu->irqack_cb().set(FUNC(pbaction_state::sound_irq_clear));
}

static const z80_daisy_config daisy_chain2[] =
{
	{ "ctc2" },
	{ nullptr }
};

void pbaction_tecfri_state::machine_start()
{
	pbaction_state::machine_start();

	m_subcommand_timer = timer_alloc(FUNC(pbaction_tecfri_state::sub_trigger), this);
	m_digits.resolve();
}

void pbaction_tecfri_state::pbactiont(machine_config &config)
{
	pbaction(config);

	m_maincpu->set_addrmap(AS_IO, &pbaction_tecfri_state::main_io_map);

	Z80(config, m_subcpu, 4_MHz_XTAL);
	m_subcpu->set_addrmap(AS_PROGRAM, &pbaction_tecfri_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &pbaction_tecfri_state::sub_io_map);
	m_subcpu->set_daisy_config(daisy_chain2);

	GENERIC_LATCH_8(config, "maintosublatch");
	//GENERIC_LATCH_8(config, "subtomainlatch");

	hct259_device &suboutlatch(HCT259(config, "suboutlatch"));
	suboutlatch.q_out_cb<0>().set(FUNC(pbaction_tecfri_state::sub8000_w));
	suboutlatch.q_out_cb<1>().set(FUNC(pbaction_tecfri_state::sub8001_w));
	// Q2-7 set on startup only

	Z80CTC(config, m_ctc2, 12_MHz_XTAL/4);
	m_ctc2->intr_callback().set_inputline(m_subcpu, 0);

	m_audiocpu->set_addrmap(AS_PROGRAM, &pbaction_tecfri_state::alt_sound_map);
	m_audiocpu->irqack_cb().set(FUNC(pbaction_tecfri_state::sound_irq_clear));
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pbaction )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b-p7.bin",     0x0000, 0x4000, CRC(8d6dcaae) SHA1(c9e605f9d291cb8c7163655ea96c605b7d30365f) )
	ROM_LOAD( "b-n7.bin",     0x4000, 0x4000, CRC(d54d5402) SHA1(a4c3205bfe5fba8bb1ff3ad15941a77c35b44a27) )
	ROM_LOAD( "b-l7.bin",     0x8000, 0x2000, CRC(e7412d68) SHA1(e75731d9bea80e0dc09798dd46e3b947fdb54aaa) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a-e3.bin",     0x0000,  0x2000, CRC(0e53a91f) SHA1(df2827197cd55c3685e5ac8b26c20800623cb932) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

ROM_START( pbaction2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "14.bin",     0x0000, 0x4000, CRC(f17a62eb) SHA1(8dabfc0ad127c154c0293a65df32d52d57dd9755) )
	ROM_LOAD( "12.bin",     0x4000, 0x4000, CRC(ec3c64c6) SHA1(6130b80606d717f95e219316c2d3fa0a1980ea1d) )
	ROM_LOAD( "13.bin",     0x8000, 0x4000, CRC(c93c851e) SHA1(b41077708fce4ccbcecdeae32af8821ca5322e87) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pba1.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

ROM_START( pbaction3 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "pinball_09.bin",     0x0000, 0x4000, CRC(c8e81ece) SHA1(04eafbd79263225f6c6fb5f04951b54179144f17) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "pinball_10.bin",     0x4000, 0x8000, CRC(04b56c7c) SHA1(d09c22fd0235e1c6a9b1978ba69338bb1ae5667d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pinball_01.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "pinball_06.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "pinball_07.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "pinball_08.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "pinball_02.bin",     0x00000, 0x4000, CRC(01ba32c9) SHA1(196f8c6e037a7ebdcefc80b453f3801b3b6eb075) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "pinball_03.bin",     0x04000, 0x4000, CRC(f605ae40) SHA1(9a28869014d2df513090d15ccb478de9f5d65b24) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "pinball_04.bin",     0x08000, 0x4000, CRC(9e23a780) SHA1(2d49ee79b9261bdb8367b28bfca9940b4527c87a) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "pinball_05.bin",     0x0c000, 0x4000, CRC(c9a4dfea) SHA1(38fb34f21773d652b14108e4a083d7c7acecdd03) )
	ROM_IGNORE(0x4000)

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "pinball_14.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "pinball_13.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "pinball_12.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

ROM_START( pbaction4 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "p16.bin",     0x0000, 0x4000, CRC(ad20b360) SHA1(91e3cdceb1c170580d926b2ed8359c3100f71b11) )
	ROM_LOAD( "c15.bin",     0x4000, 0x4000, CRC(057acfe3) SHA1(49c184d7caea0c0e9f0d0e163f2ef42bb9aebf16) )
	ROM_LOAD( "p14.bin",     0x8000, 0x2000, CRC(e7412d68) SHA1(e75731d9bea80e0dc09798dd46e3b947fdb54aaa) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p1.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "p7.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "p8.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "p9.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "p2.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "p3.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "p4.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "p5.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "p11.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "p12.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "p13.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

// PCB has Tehkan logo (6001-1A/1B) and also "Fabricado por Tecfri S.A. Made in Spain"
ROM_START( pbactiont )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pba16.bin",     0x0000, 0x4000, CRC(4a239ebd) SHA1(74e6da0485ac78093b4f09953fa3accb14bc3e43) )
	ROM_LOAD( "pba15.bin",     0x4000, 0x4000, CRC(3afef03a) SHA1(dec714415d2fd00c9021171a48f6c94b40888ae8) )
	ROM_LOAD( "pba14.bin",     0x8000, 0x2000, CRC(c0a98c8a) SHA1(442f37af31db13fd98602dd7f9eeae5529da0f44) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pba1.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "pba17.bin",    0x0000,  0x4000, CRC(2734ae60) SHA1(4edcdfac1611c49c4f890609efbe8352b8161f8e) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

uint8_t pbaction_state::pbaction2_prot_kludge_r()
{
	// on startup, the game expect this location to NOT act as RAM
	if (m_maincpu->pc() == 0xab80)
		return 0;

	return m_work_ram[0];
}

void pbaction_state::init_pbaction2()
{
	uint8_t *rom = memregion("maincpu")->base();

	// first of all, do a simple bitswap
	for (int i = 0; i < 0xc000; i++)
		rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0);

	// install a protection (?) workaround
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xc000, read8smo_delegate(*this, FUNC(pbaction_state::pbaction2_prot_kludge_r)) );
}

} // anonymous namespace


// some of these are probably bootlegs
GAME( 1985, pbaction,  0,        pbaction,  pbaction, pbaction_state, empty_init,     ROT90, "Tehkan",                  "Pinball Action (set 1)",            MACHINE_SUPPORTS_SAVE ) // possible bootleg due to not being encrypted + different sound map
GAME( 1985, pbaction2, pbaction, pbactionx, pbaction, pbaction_state, init_pbaction2, ROT90, "Tehkan",                  "Pinball Action (set 2, encrypted)", MACHINE_SUPPORTS_SAVE ) // likely bootleg due to extra protection on top of usual
GAME( 1985, pbaction3, pbaction, pbactionx, pbaction, pbaction_state, empty_init,     ROT90, "Tehkan",                  "Pinball Action (set 3, encrypted)", MACHINE_SUPPORTS_SAVE ) // possible bootleg due to oversized ROMs
GAME( 1985, pbaction4, pbaction, pbactionx, pbaction, pbaction_state, empty_init,     ROT90, "Tehkan",                  "Pinball Action (set 4, encrypted)", MACHINE_SUPPORTS_SAVE ) // original?
GAMEL(1985, pbactiont, pbaction, pbactiont, pbaction, pbaction_tecfri_state, empty_init,     ROT90, "Tehkan (Tecfri license)", "Pinball Action (Tecfri license)",   MACHINE_SUPPORTS_SAVE, layout_pbactiont )
