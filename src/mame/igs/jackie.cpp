// license:BSD-3-Clause
// copyright-holders:David Haywood, Mirko Buffoni

/*
Happy Jackie (c) 1993 IGS.
Video Slot machine game for amusement only.
IGS PCB N0-T0039-4

Driver by David Haywood and Mirko Buffoni
*/
/*

Anno    199x
Produttore  IGS
N.revisione

CPU

1x Z0840006PSC (main)
2x D8255AC
1x unknown AMT001
1x unknown IGS002
1x UM3567 (sound)
1x oscillator 12.000MHz
1x oscillator 3.579645

ROMs

2x D27128A (1,3)
1x MBM27128 (2)
3x 27C010 (4,5,6)
1x D27512 (7sv)
1x MBM27C512 (v110)
1x unknown (DIP20 mil300)(jack3)
3x PEEL18CV8PC (read protected)
1x TIBPAL16L8 (read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector (payout system)
1x trimmer (volume)
1x pushbutton
1x battery
5x 8x2 switches dip

*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class jackie_state : public driver_device
{
public:
	jackie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this,"maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_bg_scroll(*this, "bg_scroll%u", 1U)
		, m_reel_ram(*this, "reel_ram%u", 1U)
		, m_fg_tile_ram(*this, "fg_tile_ram")
		, m_fg_color_ram(*this, "fg_color_ram")
		, m_exprom_bank(*this, "exprom_bank")
		, m_led(*this, "led")
		, m_lamps(*this, "lamp%u", 1U)
	{ }

	void jackie(machine_config &config);

	void init_jackie();
	void init_kungfu();

	int hopper_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void fg_tile_w(offs_t offset, uint8_t data);
	void fg_color_w(offs_t offset, uint8_t data);
	template<uint8_t Which> void reel_ram_w(offs_t offset, uint8_t data);

	void nmi_and_coins_w(uint8_t data);
	void lamps_w(uint8_t data);
	uint8_t irqack_r();
	void irqack_w(uint8_t data);

	template<uint8_t Which> void unk_reg_lo_w(offs_t offset, uint8_t data);
	template<uint8_t Which> void unk_reg_hi_w(offs_t offset, uint8_t data);
	void show_out();

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template<uint8_t Which> TILE_GET_INFO_MEMBER(get_reel_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint8_t, 2> m_bg_scroll;
	required_shared_ptr_array<uint8_t, 3> m_reel_ram;
	required_shared_ptr<uint8_t> m_fg_tile_ram;
	required_shared_ptr<uint8_t> m_fg_color_ram;
	required_memory_bank m_exprom_bank;
	output_finder<> m_led;
	output_finder<6> m_lamps;

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_reel_tilemap[3]{};
	uint8_t m_irq_enable = 0;
	uint8_t m_nmi_enable = 0;
	uint8_t m_bg_enable = 0;
	uint8_t m_hopper = 0;
	uint8_t m_out[3]{};
	uint16_t m_unk_reg[3][5]{};
};


TILE_GET_INFO_MEMBER(jackie_state::get_fg_tile_info)
{
	int const code = m_fg_tile_ram[tile_index] | (m_fg_color_ram[tile_index] << 8);
	int const tile = code & 0x1fff;
	tileinfo.set(0, code, tile != 0x1fff ? ((code >> 12) & 0xe) + 1 : 0, 0);
}

void jackie_state::fg_tile_w(offs_t offset, uint8_t data)
{
	m_fg_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void jackie_state::fg_color_w(offs_t offset, uint8_t data)
{
	m_fg_color_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

template<uint8_t Which>
void jackie_state::reel_ram_w(offs_t offset, uint8_t data)
{
	m_reel_ram[Which][offset] = data;
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}

template<uint8_t Which>
TILE_GET_INFO_MEMBER(jackie_state::get_reel_tile_info)
{
	int const code = m_reel_ram[Which][tile_index];
	tileinfo.set(1, code, 0, 0);
}

void jackie_state::video_start()
{
	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackie_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackie_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackie_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	for (int i = 0; i < 3; i++)
		m_reel_tilemap[i]->set_scroll_cols(64);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackie_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);
}


uint32_t jackie_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	for (int i = 0; i < 0x40; i++)
	{
		m_reel_tilemap[0]->set_scrolly(i, m_bg_scroll[0][i + 0x000]);
		m_reel_tilemap[1]->set_scrolly(i, m_bg_scroll[0][i + 0x040]);
		m_reel_tilemap[2]->set_scrolly(i, m_bg_scroll[0][i + 0x080]);
	}

	int startclipmin = 0;
	const rectangle &visarea = screen.visible_area();

	for (int j = 0; j < 0x100 - 1; j++)
	{
		rectangle clip;
		int const rowenable = m_bg_scroll[1][j];

		// draw top of screen
		clip.set(visarea.min_x, visarea.max_x, startclipmin, startclipmin + 1);

		if (rowenable == 0)
		{
			m_reel_tilemap[0]->draw(screen, bitmap, clip, 0, 0);
		}
		else if (rowenable == 1)
		{
			m_reel_tilemap[1]->draw(screen, bitmap, clip, 0, 0);
		}
		else if (rowenable == 2)
		{
			m_reel_tilemap[2]->draw(screen, bitmap, clip, 0, 0);
		}
		else if (rowenable == 3)
		{
		}

		startclipmin += 1;
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void jackie_state::machine_start()
{
	m_exprom_bank->configure_entries(0, 2, memregion("gfx3")->base(), 0x8000);

	m_led.resolve();
	m_lamps.resolve();

	// save_item(NAME(m_irq_enable)); //always 1?
	save_item(NAME(m_nmi_enable));
	// save_item(NAME(m_bg_enable)); //always 1?
	save_item(NAME(m_hopper));
	save_item(NAME(m_out));
	save_item(NAME(m_unk_reg));
}

void jackie_state::machine_reset()
{
	m_irq_enable = 1;
	m_nmi_enable = 0;
	m_hopper = 0;
	m_bg_enable = 1;
}


void jackie_state::show_out()
{
#ifdef MAME_DEBUG
//  popmessage("%02x %02x %02x", m_out[0], m_out[1], m_out[2]);
	popmessage("520: %04x %04x %04x %04x %04x\n560: %04x %04x %04x %04x %04x\n5A0: %04x %04x %04x %04x %04x",
		m_unk_reg[0][0],m_unk_reg[0][1],m_unk_reg[0][2],m_unk_reg[0][3],m_unk_reg[0][4],
		m_unk_reg[1][0],m_unk_reg[1][1],m_unk_reg[1][2],m_unk_reg[1][3],m_unk_reg[1][4],
		m_unk_reg[2][0],m_unk_reg[2][1],m_unk_reg[2][2],m_unk_reg[2][3],m_unk_reg[2][4]
	);
#endif
}

template<uint8_t Which>
void jackie_state::unk_reg_lo_w(offs_t offset, uint8_t data)
{
	m_unk_reg[Which][offset] &= 0xff00;
	m_unk_reg[Which][offset] |= data;
	show_out();
}

template<uint8_t Which>
void jackie_state::unk_reg_hi_w(offs_t offset, uint8_t data)
{
	m_unk_reg[Which][offset] &= 0xff;
	m_unk_reg[Which][offset] |= data << 8;
	show_out();
}

void jackie_state::nmi_and_coins_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);   // coin_a
	machine().bookkeeping().coin_counter_w(1, data & 0x04);   // coin_c
	machine().bookkeeping().coin_counter_w(2, data & 0x08);   // key in
	machine().bookkeeping().coin_counter_w(3, data & 0x10);   // coin m_out mech

	m_led = BIT(data, 5);   // led for coin m_out / m_hopper active

	m_exprom_bank->set_entry(BIT(data, 1));
	m_nmi_enable = data & 0x80;     // nmi enable?

	m_out[0] = data;
	show_out();
}

void jackie_state::lamps_w(uint8_t data)
{
/*
    - Lbits -
    7654 3210
    =========
    ---- --x-  Hold1 lamp.
    --x- ----  Hold2 lamp.
    ---x ----  Hold3 lamp.
    ---- x---  Hold4 lamp.
    ---- -x--  Hold5 lamp.
    ---- ---x  Start lamp.
*/
	m_lamps[0] = BIT(data, 1);      // Lamp 1 - HOLD 1
	m_lamps[1] = BIT(data, 5);      // Lamp 2 - HOLD 2
	m_lamps[2] = BIT(data, 4);      // Lamp 3 - HOLD 3
	m_lamps[3] = BIT(data, 3);      // Lamp 4 - HOLD 4
	m_lamps[4] = BIT(data, 2);      // Lamp 5 - HOLD 5
	m_lamps[5] = BIT(data, 0);      // Lamp 6 - START

	m_hopper = (~data) & 0x80;

	m_out[1] = data;
	show_out();
}

uint8_t jackie_state::irqack_r()
{
	m_irq_enable = 1;
	return 0;
}

void jackie_state::irqack_w(uint8_t data)
{
//  m_maincpu->set_input_line(0, CLEAR_LINE);
	m_out[2] = data;
	show_out();
}

void jackie_state::prg_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}

void jackie_state::io_map(address_map &map)
{
	map(0x0520, 0x0524).w(FUNC(jackie_state::unk_reg_lo_w<0>));
	map(0x0d20, 0x0d24).w(FUNC(jackie_state::unk_reg_hi_w<0>));
	map(0x0560, 0x0564).w(FUNC(jackie_state::unk_reg_lo_w<1>));
	map(0x0d60, 0x0d64).w(FUNC(jackie_state::unk_reg_hi_w<1>));
	map(0x05a0, 0x05a4).w(FUNC(jackie_state::unk_reg_lo_w<2>));
	map(0x0da0, 0x0da4).w(FUNC(jackie_state::unk_reg_hi_w<2>));
	map(0x1000, 0x1107).ram().share(m_bg_scroll[1]);
	map(0x2000, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2800, 0x2fff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x4000, 0x4000).portr("DSW1");
	map(0x4001, 0x4001).portr("DSW2");
	map(0x4002, 0x4002).portr("DSW3");
	map(0x4003, 0x4003).portr("DSW4");
	map(0x4004, 0x4004).portr("DSW5");
	map(0x5080, 0x5083).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5090, 0x5093).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x50a0, 0x50a0).portr("BUTTONS2");
	map(0x50b0, 0x50b1).w("ymsnd", FUNC(ym2413_device::write));
	map(0x50c0, 0x50c0).r(FUNC(jackie_state::irqack_r)).w(FUNC(jackie_state::irqack_w));
	map(0x6000, 0x60ff).ram().share(m_bg_scroll[0]);
	map(0x6800, 0x69ff).ram().w(FUNC(jackie_state::reel_ram_w<0>)).share(m_reel_ram[0]);
	map(0x6a00, 0x6bff).ram().w(FUNC(jackie_state::reel_ram_w<1>)).share(m_reel_ram[1]);
	map(0x6c00, 0x6dff).ram().w(FUNC(jackie_state::reel_ram_w<2>)).share(m_reel_ram[2]);
	map(0x7000, 0x77ff).ram().w(FUNC(jackie_state::fg_tile_w)).share(m_fg_tile_ram);
	map(0x7800, 0x7fff).ram().w(FUNC(jackie_state::fg_color_w)).share(m_fg_color_ram);
	map(0x8000, 0xffff).bankr(m_exprom_bank);
}

int jackie_state::hopper_r()
{
	if (m_hopper) return !(m_screen->frame_number() % 10);
	return machine().input().code_pressed(KEYCODE_H);
}

static INPUT_PORTS_START( jackie )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWB:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "45%" )
	PORT_DIPSETTING(    0x0e, "50%" )
	PORT_DIPSETTING(    0x0d, "55%" )
	PORT_DIPSETTING(    0x0c, "60%" )
	PORT_DIPSETTING(    0x0b, "65%" )
	PORT_DIPSETTING(    0x0a, "70%" )
	PORT_DIPSETTING(    0x09, "75%" )
	PORT_DIPSETTING(    0x08, "80%" )
	PORT_DIPSETTING(    0x07, "83%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x05, "88%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x03, "92%" )
	PORT_DIPSETTING(    0x02, "94%" )
	PORT_DIPSETTING(    0x01, "96%" )
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPNAME( 0x20, 0x00, "Double Up Rate" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0xC0, 0x00, "Max Bet" ) PORT_DIPLOCATION("SWC:2,1")
	PORT_DIPSETTING(    0xC0, "1" )
	PORT_DIPSETTING(    0x80, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x00, "32" )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW5")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("SERVICE")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9) PORT_NAME("Attendent")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jackie_state, hopper_r) PORT_NAME("HPSW")    // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )   // test (press during boot)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Key In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Clear")   // pays out
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Togglemode") // Used
	PORT_BIT( 0xC0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Stop")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS2")  // OK
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Small / Right Hammer")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Take/Left Hammer")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Big / Center Hammer")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( kungfu )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Display Card" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "A, J, Q, K" )
	PORT_DIPSETTING(    0x00, "Number" )
	PORT_DIPNAME( 0x04, 0x04, "Speed" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Slow" )
	PORT_DIPSETTING(    0x00, "Quick" )
	PORT_DIPNAME( 0x08, 0x08, "Double" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x30, 0x30, "Limit" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "10000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x00, "70000" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x40, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "40" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x30, 0x30, "Key In" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "Demo Game" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Anticlear" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Unanti" )
	PORT_DIPSETTING(    0x00, "Anti" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Double" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Royal Enable" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "5 Kind Enable" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Key Out Base" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Open Mode" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, "Demo" )
	PORT_DIPSETTING(    0x00, "Amuse" )
	PORT_DIPNAME( 0x20, 0x20, "Quick Get" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Bet Base" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x80, "Percent" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4") // not shown in test mode, possibly unused
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW4:8" )

	PORT_START("DSW5") // not shown in test mode, possibly unused
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW5:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW5:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW5:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW5:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW5:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW5:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW5:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW5:8" )

	PORT_START("SERVICE")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )   // test (press during boot)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("Amuse")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 1 / Extra")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Bet 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Low")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Double Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("Max Bet / High")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout layout_8x8x6 =
{
	8, 8,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2
};

static const gfx_layout layout_8x32x6 =
{
	8, 32,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static GFXDECODE_START( gfx_jackie )
	GFXDECODE_ENTRY( "tiles", 0, layout_8x8x6,  0, 16 )
	GFXDECODE_ENTRY( "reels", 0, layout_8x32x6, 0, 16 )
GFXDECODE_END

void jackie_state::init_jackie()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int A = 0; A < 0xf000; A++)
	{
		rom[A] = rom[A] ^ 0x21;

		if (((A & 0x0080) == 0x0000) && ((A & 0x0008) == 0x0000)) rom[A] = rom[A] ^ 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
	memset(&rom[0xf000], 0, 0x1000);

	// Patch trap
	rom[0x7e86] = 0xc3;
}

void jackie_state::init_kungfu()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		rom[A] ^= 0x01;
		if ((A & 0x0060) == 0x0020) rom[A] ^= 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}

	// Patch trap
	rom[0xbcc5] = 0xc3;
}


TIMER_DEVICE_CALLBACK_MEMBER(jackie_state::irq)
{
	int const scanline = param;

	if ((scanline % 32) != 0)
		return;

	if ((scanline % 64) == 32 && m_irq_enable)
		m_maincpu->set_input_line(0, HOLD_LINE);
	else if ((scanline % 64) == 0 && m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void jackie_state::jackie(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &jackie_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &jackie_state::io_map);
	TIMER(config, "scantimer", 0).configure_scanline(FUNC(jackie_state::irq), "screen", 0, 1);

	i8255_device &ppi1(I8255A(config, "ppi1")); // D8255AC
	ppi1.out_pa_callback().set(FUNC(jackie_state::nmi_and_coins_w));
	ppi1.in_pb_callback().set_ioport("SERVICE");
	ppi1.in_pc_callback().set_ioport("COINS");

	i8255_device &ppi2(I8255A(config, "ppi2")); // D8255AC
	ppi2.in_pa_callback().set_ioport("BUTTONS1");
	ppi2.out_pb_callback().set(FUNC(jackie_state::lamps_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 64*8-1, 0, 32*8-1);
	m_screen->set_screen_update(FUNC(jackie_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jackie);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( jackie ) // IGS PCB N0-T0039-4
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jackiev110.u23",   0x0000, 0x10000, CRC(1b78a619) SHA1(a6eb6b6e544efa55225f2e947483614afb6ece3b) )

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "6.u6",  0x00000, 0x20000, CRC(d2ed60a9) SHA1(40e2280384aa5c9e72e87a3b9e673172ff695676) )
	ROM_LOAD( "5.u5",  0x20000, 0x20000, CRC(dc01fe7c) SHA1(683834ce2f13a923c0467209b93fef693d9c3e38) )
	ROM_LOAD( "4.u4",  0x40000, 0x20000, CRC(38a42dcd) SHA1(8cc08ff4143281d9022210d6577146d725df9044) )

	ROM_REGION( 0x30000, "reels", 0 )
	ROM_LOAD( "3.u3",  0x00000, 0x4000, CRC(c69e962b) SHA1(492427ad1ac959cdf22d23439e0eb5932b60ec88) )
	ROM_LOAD( "2.u2",  0x10000, 0x4000, CRC(8900ffba) SHA1(065cf1810ec9738718e4c94613f726e85ba4314d) )
	ROM_LOAD( "1.u1",  0x20000, 0x4000, CRC(071d20f0) SHA1(77c87486803dccaa63732ff959c223b1313820e3) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "jackie7sv.u22",   0x0000, 0x10000, CRC(8b4eb6da) SHA1(480784917dfaf9a0343c1d56eb590b32bf5e94fd) )

	ROM_REGION( 0x10000, "misc", 0 )
	ROM_LOAD( "16l8.u31",   0x0000, 0x104, NO_DUMP )
	ROM_LOAD( "18cv8.u14",  0x0000, 0x155, NO_DUMP )
	ROM_LOAD( "18cv8.u8",   0x0000, 0x155, NO_DUMP )
	ROM_LOAD( "18cv8.u9",   0x0000, 0x155, NO_DUMP )
ROM_END

ROM_START( kungfu ) // IGS PCB N0- 0139
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kung fu v202n.u23", 0x00000, 0x10000, CRC(53396dd3) SHA1(1bab42394f016f800dbd80603c70defc25380fd7) )

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "kungfu-6.u6", 0x00000, 0x20000, CRC(f1ec5f0d) SHA1(0aa888e13312ed5d98953c81f03a61c6175c7fec) )
	ROM_LOAD( "kungfu-5.u5", 0x20000, 0x20000, CRC(25c9c98e) SHA1(2d3a399d8d53ee5cb8106d2b35d1ab1778439f81) )
	ROM_LOAD( "kungfu-4.u4", 0x40000, 0x20000, CRC(df4afedb) SHA1(56ab18c46a199653c284417a8e9edc9f32374318) )

	ROM_REGION( 0x30000, "reels", 0 )
	ROM_LOAD( "kungfu-3.u3", 0x00000, 0x4000, CRC(bbf78e03) SHA1(06fee093e75e2611d00c076c2e0a681938fa8b74) )
	ROM_LOAD( "kungfu-2.u2", 0x10000, 0x4000, CRC(927b3060) SHA1(a780ea5aaee04287cc9533c2d258dc18f8426530) )
	ROM_LOAD( "kungfu-1.u1", 0x20000, 0x4000, CRC(abaada6b) SHA1(a6b910db7451e8ca737f43f32dfc8fc5ecf865f4) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "kungfu-7.u22", 0x00000, 0x10000, CRC(0568f20b) SHA1(a51a10deee0d581b79d0fee354cedceaa660f55c) ) // 1ST AND 2ND HALF IDENTICAL, but confirmed same data as the kungfua set in igs/igspoker.cpp

	ROM_REGION( 0x155, "misc", 0 ) // all labels unreadable
	ROM_LOAD( "16l8.u31",   0x0000, 0x104, NO_DUMP )
	ROM_LOAD( "18cv8.u14",  0x0000, 0x155, NO_DUMP )
	ROM_LOAD( "18cv8.u8",   0x0000, 0x155, NO_DUMP )
	ROM_LOAD( "18cv8.u9",   0x0000, 0x155, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1993, jackie, 0, jackie, jackie, jackie_state, init_jackie, ROT0, "IGS", "Happy Jackie (v110U)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, kungfu, 0, jackie, kungfu, jackie_state, init_kungfu, ROT0, "IGS", "Kung Fu Fighters (IGS, v202N)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // inputs for the fighting part, incorrect decoding
