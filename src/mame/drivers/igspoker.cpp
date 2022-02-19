// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/*****************************************************************************

Champion Poker by IGS   (documented by Mirko Buffoni)
---

Memory Layout (refers to CSK227IT.  Others may have different addresses)

ROM:        0000-efff
RAM:        f000-ffff

---

I/O Ports

Palette:    2000-27ff   (low byte)
            2800-2fff   (high byte)
VideoRAM:   7000-77ff
ColorRAM:   7800-7fff
DSW1-5:     4000-4004   (see input ports section below)
InputPorts: 50a0        (unused in this game)
            5081-5082   (Coins and Keyboard)
            5091        (Keyboard)
Expansion:  8000-ffff   (R)     Used to read from an expansion rom

Unknown:    5080        (RW)    (possibly related to ticket/hopper)
            5090-5091   (RW)    (possibly related to eprom counters)
            50b0-50b1   (W)     (OPL2 compatible chip)
            5083        (W)     (used only at reset, maybe)
            1000-10ff   (W) ???
            6000-67ff   (W) ???
            6800-6fff   (W)     Expansion video layer (used with ability)

---

Timing:

Game is synchronized with VBLANK. It uses IRQ & NMI interrupts.
During a frame, there must be 4 IRQs and 4 NMIs in order to play
to the correct speed.

---

Notes about palette:
Charset is 6 bit depth (thus 64 colors of granularity)
Colortable is made up of 2 entries of 64 bytes for each palette,
splitted, and colorinfo is stored to form the following word:

xBBBBBGGGGGRRRRR    (Bit 15 is never used)

---

FIX:  csk227it has video issues, as after Ability game, bg_tilemap is not reset
    so there must be some bg_enable command which I couldn't find, or rom is
    from a beta version which has transparency issues.  This doesn't happen with
    csk234it or New Champion Skill.
    Insert credits with Key-In and press Pay-out to play ability game, and wait
    for attract-mode to show cubes (not cards), which are transparent and reveal
    background tilemap.

FIX: PK Tetris have an input named AMUSE which I couldn't map.  Maybe it is
    necessary for the Amuse game, because I can't understand how to play it.

TODO:

- Sets cpoker & cpokert spit 660K of whatever they have in the hopper when keyout...
- Check if the cpoker sets still lock at some point due to protection.
- Fix lamps to cpoker101.

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "igspoker.lh"


namespace {

#define VERBOSE 0


class igspoker_state : public driver_device
{
public:
	igspoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bg_tile_ram(*this, "bg_tile_ram")
		, m_fg_tile_ram(*this, "fg_tile_ram")
		, m_fg_color_ram(*this, "fg_color_ram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_led(*this, "led0")
		, m_lamps(*this, "lamp%u", 1U)
	{ }

	void csk234it(machine_config &config);
	void igs_ncs(machine_config &config);
	void csk227it(machine_config &config);
	void igspoker(machine_config &config);
	void pktetris(machine_config &config);
	void cpokerpk(machine_config &config);
	void number10(machine_config &config);

	void init_igs_ncs();
	void init_number10();
	void init_pktet346();
	void init_tet341();
	void init_cpokert();
	void init_cpoker101();
	void init_chleague();
	void init_cska();
	void init_cpoker();
	void init_cpoker300us();
	void init_igs_ncs2();
	void init_cpokerpk();
	void init_kungfu();
	void init_kungfua();

	DECLARE_READ_LINE_MEMBER(hopper_r);

protected:
	virtual void machine_start() override { m_led.resolve(); m_lamps.resolve(); }
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	uint8_t irqack_r();
	void irqack_w(uint8_t data);
	void bg_tile_w(offs_t offset, uint8_t data);
	void fg_tile_w(offs_t offset, uint8_t data);
	void fg_color_w(offs_t offset, uint8_t data);
	void nmi_and_coins_w(uint8_t data);
	void lamps_w(uint8_t data);
	uint8_t custom_io_r();
	void custom_io_w(uint8_t data);
	uint8_t exp_rom_r(offs_t offset);
	void show_out();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	DECLARE_VIDEO_START(cpokerpk);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cpokerpk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void cpokerpk_io_map(address_map &map);
	void igspoker_io_map(address_map &map);
	void igspoker_prg_map(address_map &map);
	void number10_io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<uint8_t> m_bg_tile_ram;
	required_shared_ptr<uint8_t> m_fg_tile_ram;
	required_shared_ptr<uint8_t> m_fg_color_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	output_finder<> m_led;
	output_finder<6> m_lamps;
	int m_nmi_enable;
	int m_bg_enable;
	int m_hopper;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	uint8_t m_out[3];
	uint8_t m_protection_res;
};


void igspoker_state::machine_reset()
{
	m_nmi_enable    =   0;
	m_hopper        =   0;
	m_bg_enable =   1;
}


TIMER_DEVICE_CALLBACK_MEMBER(igspoker_state::interrupt)
{
	int scanline = param;

	if((scanline % 32) != 0)
		return;

	if((scanline % 64) == 32)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	if((scanline % 64) == 0 && m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


uint8_t igspoker_state::irqack_r()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0;
}

void igspoker_state::irqack_w(uint8_t data)
{
//  m_maincpu->set_input_line(0, CLEAR_LINE);
}


TILE_GET_INFO_MEMBER(igspoker_state::get_bg_tile_info)
{
	int code = m_bg_tile_ram[tile_index];
	tileinfo.set(1 + (tile_index & 3), code, 0, 0);
}

TILE_GET_INFO_MEMBER(igspoker_state::get_fg_tile_info)
{
	int code = m_fg_tile_ram[tile_index] | (m_fg_color_ram[tile_index] << 8);
	int tile = code & 0x1fff;
	tileinfo.set(0, code, tile != 0x1fff ? ((code >> 12) & 0xe) + 1 : 0, 0);
}

void igspoker_state::bg_tile_w(offs_t offset, uint8_t data)
{
	m_bg_tile_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void igspoker_state::fg_tile_w(offs_t offset, uint8_t data)
{
	m_fg_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void igspoker_state::fg_color_w(offs_t offset, uint8_t data)
{
	m_fg_color_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void igspoker_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igspoker_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS,   8,  8,  64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igspoker_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,   8,  32, 64, 8);

	m_fg_tilemap->set_transparent_pen(0);
}

uint32_t igspoker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	// FIX: CSK227IT must have some way to disable background, or wrong gfx?
	if (m_bg_enable) m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

VIDEO_START_MEMBER(igspoker_state,cpokerpk)
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igspoker_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS,   8,  8,  64, 32);
}

uint32_t igspoker_state::screen_update_cpokerpk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void igspoker_state::show_out()
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x", m_out[0], m_out[1]);
#endif
}

void igspoker_state::nmi_and_coins_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0,        data & 0x01);   // coin_a
	machine().bookkeeping().coin_counter_w(1,        data & 0x04);   // coin_c
	machine().bookkeeping().coin_counter_w(2,        data & 0x08);   // key in
	machine().bookkeeping().coin_counter_w(3,        data & 0x10);   // coin m_out mech

	m_led = BIT(data, 5);   // led for coin m_out / m_hopper active

	m_nmi_enable = data & 0x80;     // nmi enable?
#if VERBOSE
	logerror("PC %06X: NMI change %02x\n",m_maincpu->pc(),m_nmi_enable);
#endif

	m_out[0] = data;
	show_out();
}

void igspoker_state::lamps_w(uint8_t data)
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

    cpokerx set has different layout:

    - Lbits -
    7654 3210
    =========
    ---- ---x  Start lamp.

    ---- ---x  Hold1 lamp.
    ---- --x-  Hold2 lamp.
    ---- -x--  Hold3 lamp.
    ---- x---  Hold4 lamp.
    ---x ----  Hold5 lamp.
    xx-- ----  one pulse once bet amount allows start.
*/
	m_lamps[0] = BIT(data, 1);      /* Lamp 1 - HOLD 1 */
	m_lamps[1] = BIT(data, 5);      /* Lamp 2 - HOLD 2  */
	m_lamps[2] = BIT(data, 4);      /* Lamp 3 - HOLD 3 */
	m_lamps[3] = BIT(data, 3);      /* Lamp 4 - HOLD 4 */
	m_lamps[4] = BIT(data, 2);      /* Lamp 5 - HOLD 5 */
	m_lamps[5] = BIT(data, 0);      /* Lamp 6 - START */

	m_hopper            =   (~data)& 0x80;

	m_out[1] = data;
	show_out();
}



uint8_t igspoker_state::custom_io_r()
{
#if VERBOSE
	logerror("PC %06X: Protection read %02x\n",m_maincpu->pc(), m_protection_res);
#endif
	return m_protection_res;
}

void igspoker_state::custom_io_w(uint8_t data)
{

	logerror("PC %06X: Protection write %02x\n",m_maincpu->pc(),data);

	switch (data)
	{
		case 0x00: m_protection_res = ioport("BUTTONS1")->read(); break;
		// CSK227
		case 0x20: m_protection_res = 0x49; break;
		case 0x21: m_protection_res = 0x47; break;
		case 0x22: m_protection_res = 0x53; break;
		case 0x24: m_protection_res = 0x41; break;
		case 0x25: m_protection_res = 0x41; break;
		case 0x26: m_protection_res = 0x7f; break;
		case 0x27: m_protection_res = 0x41; break;
		case 0x28: m_protection_res = 0x41; break;
		case 0x2a: m_protection_res = 0x3e; break;
		case 0x2b: m_protection_res = 0x41; break;
		// CSK227 and NUMBER10
		case 0x2c: m_protection_res = 0x49; break;
		case 0x2d: m_protection_res = 0xf9; break;
		case 0x2e: m_protection_res = 0x0a; break;
		case 0x30: m_protection_res = 0x26; break;
		case 0x31: m_protection_res = 0x49; break;
		case 0x32: m_protection_res = 0x49; break;
		case 0x33: m_protection_res = 0x49; break;
		case 0x34: m_protection_res = 0x32; break;
		// NUMBER10
		case 0x60: m_protection_res = 0x30; break;
		case 0x61: m_protection_res = 0x31; break;
		case 0x62: m_protection_res = 0x3e; break;
		case 0x64: m_protection_res = 0x3c; break;
		case 0x65: m_protection_res = 0x31; break;
		case 0x66: m_protection_res = 0x39; break;
		case 0x67: m_protection_res = 0x33; break;
		case 0x68: m_protection_res = 0x35; break;
		case 0x6a: m_protection_res = 0x40; break;
		case 0x6b: m_protection_res = 0x43; break;
		default:
			m_protection_res = data;
	}
}

READ_LINE_MEMBER(igspoker_state::hopper_r)
{
	if (m_hopper) return !(m_screen->frame_number()%10);
	return machine().input().code_pressed(KEYCODE_H);
}

uint8_t igspoker_state::exp_rom_r(offs_t offset)
{
	uint8_t *rom = memregion("maincpu")->base();
	return rom[offset+0x10000];
}

void igspoker_state::igspoker_prg_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}

void igspoker_state::igspoker_io_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(igspoker_state::exp_rom_r));
	map(0x2000, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2800, 0x2fff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x4000, 0x4000).portr("DSW1");           /* DSW1 */
	map(0x4001, 0x4001).portr("DSW2");           /* DSW2 */
	map(0x4002, 0x4002).portr("DSW3");           /* DSW3 */
	map(0x4003, 0x4003).portr("DSW4");           /* DSW4 */
	map(0x4004, 0x4004).portr("DSW5");           /* DSW5 */
	map(0x5080, 0x5083).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5090, 0x5090).w(FUNC(igspoker_state::custom_io_w));
	map(0x5091, 0x5091).r(FUNC(igspoker_state::custom_io_r)).w(FUNC(igspoker_state::lamps_w));            /* Keyboard */
	map(0x50a0, 0x50a0).portr("BUTTONS2");           /* Not connected */
	map(0x50b0, 0x50b1).w("ymsnd", FUNC(ym2413_device::write));
	map(0x50c0, 0x50c0).r(FUNC(igspoker_state::irqack_r)).w(FUNC(igspoker_state::irqack_w));
	map(0x6800, 0x6fff).ram().w(FUNC(igspoker_state::bg_tile_w)).share("bg_tile_ram");
	map(0x7000, 0x77ff).ram().w(FUNC(igspoker_state::fg_tile_w)).share("fg_tile_ram");
	map(0x7800, 0x7fff).ram().w(FUNC(igspoker_state::fg_color_w)).share("fg_color_ram");
}


/* MB: 05 Jun 99  Input ports and Dip switches are all verified! */

static INPUT_PORTS_START( cpoker )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Key Out Rate" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1:1" )
	PORT_DIPSETTING(    0x40, "10:1" )
	PORT_DIPSETTING(    0x20, "100:1" )
	PORT_DIPSETTING(    0x00, "100:1" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "W-UP Bonus Target" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x02, 0x02, "W-UP Bonus Rate" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )     /* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x00, "Strip Girl" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Anytime Key-in" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Hopper" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW") // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Low / Black")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / High / Red")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( cpokerx )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Key Out Rate" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1:1" )
	PORT_DIPSETTING(    0x40, "10:1" )
	PORT_DIPSETTING(    0x20, "100:1" )
	PORT_DIPSETTING(    0x00, "100:1" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "W-UP Bonus Target" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x02, 0x02, "W-UP Bonus Rate" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )     /* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x00, "Strip Girl" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Anytime Key-in" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Hopper" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_NAME("Attendent")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW") // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Operator")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Low / Black")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / High / Red")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( csk227 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:6,7")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x18, "Max Bet" ) PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "W-UP Bonus Target" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1500" )
	PORT_DIPSETTING(    0x40, "3000" )
	PORT_DIPSETTING(    0x20, "5000" )
	PORT_DIPSETTING(    0x00, "7500" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "W-UP Bonus Rate" ) PORT_DIPLOCATION("SWC:8,7")
	PORT_DIPSETTING(    0x03, "200" )
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "800" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )     /* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x00, "Strip Girl" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Ability" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Payout Select" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, "Ticket" )
	PORT_DIPSETTING(    0x00, "Hopper" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x07, 0x07, "Key Out Rate" ) PORT_DIPLOCATION("SWE:8,7,6")
	PORT_DIPSETTING(    0x07, "1:1" )
	PORT_DIPSETTING(    0x06, "10:1" )
	PORT_DIPSETTING(    0x05, "20:1" )
	PORT_DIPSETTING(    0x04, "50:1" )
	PORT_DIPSETTING(    0x03, "100:1" )     /* Bits 1-0 are all equivalents */
	PORT_DIPNAME( 0x08, 0x00, "Card Select" ) PORT_DIPLOCATION("SWE:5")
	PORT_DIPSETTING(    0x08, "Poker" )
	PORT_DIPSETTING(    0x00, "Tetris" )
	PORT_DIPNAME( 0x70, 0x70, "Ticket Rate" ) PORT_DIPLOCATION("SWE:4,3,2")
	PORT_DIPSETTING(    0x70, "1:1" )
	PORT_DIPSETTING(    0x60, "5:1" )
	PORT_DIPSETTING(    0x50, "10:1" )
	PORT_DIPSETTING(    0x40, "20:1" )
	PORT_DIPSETTING(    0x30, "25:1" )
	PORT_DIPSETTING(    0x20, "50:1" )
	PORT_DIPSETTING(    0x10, "100:1" )
	PORT_DIPSETTING(    0x00, "200:1" )
	PORT_DIPNAME( 0x80, 0x80, "Win Table" ) PORT_DIPLOCATION("SWE:1")
	PORT_DIPSETTING(    0x80, "Change" )
	PORT_DIPSETTING(    0x00, "Fixed" )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW")  // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")

	PORT_START("BUTTONS2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( csk234 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Key Out Rate" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1:1" )
	PORT_DIPSETTING(    0x40, "10:1" )
	PORT_DIPSETTING(    0x20, "100:1" )
	PORT_DIPSETTING(    0x00, "100:1" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "W-UP Bonus Target" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x02, 0x02, "W-UP Bonus Rate" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )     /* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x40, "Card Select" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, "Poker" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Anytime Key-in" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xC0, IP_ACTIVE_LOW, IPT_UNUSED )         /* Joker and Royal Flush are always enabled */

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Hopper" ) PORT_DIPLOCATION("SWE:8")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Payout Select" ) PORT_DIPLOCATION("SWE:7")
	PORT_DIPSETTING(    0x02, "Hopper" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x0c, 0x0c, "Ticket Rate" ) PORT_DIPLOCATION("SWE:6,5")
	PORT_DIPSETTING(    0x0c, "10:1" )
	PORT_DIPSETTING(    0x08, "20:1" )
	PORT_DIPSETTING(    0x04, "50:1" )
	PORT_DIPSETTING(    0x00, "100:1" )
	PORT_DIPNAME( 0x10, 0x00, "Ability" ) PORT_DIPLOCATION("SWE:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW")  // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( igs_ncs )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "W-UP Limit" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1500" )
	PORT_DIPSETTING(    0x40, "3000" )
	PORT_DIPSETTING(    0x20, "5000" )
	PORT_DIPSETTING(    0x00, "7500" )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "W-UP Pool" ) PORT_DIPLOCATION("SWC:8,7")
	PORT_DIPSETTING(    0x03, "200" )
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "800" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )     /* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x40, "Ability Pay" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, "All" )
	PORT_DIPSETTING(    0x00, "1/Time" )
	PORT_DIPNAME( 0x80, 0x80, "Ability" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Hopper" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x07, 0x07, "Key Out Rate" ) PORT_DIPLOCATION("SWE:8,7,6")
	PORT_DIPSETTING(    0x07, "1:1" )
	PORT_DIPSETTING(    0x06, "10:1" )
	PORT_DIPSETTING(    0x05, "20:1" )
	PORT_DIPSETTING(    0x04, "50:1" )
	PORT_DIPSETTING(    0x03, "100:1" )     /* latest 4 is 100 for ON/OFF */
	PORT_DIPNAME( 0x08, 0x08, "Card Select" ) PORT_DIPLOCATION("SWE:5")
	PORT_DIPSETTING(    0x08, "Poker" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_DIPNAME( 0x70, 0x70, "Ticket Rate" ) PORT_DIPLOCATION("SWE:4,3,2")
	PORT_DIPSETTING(    0x70, "1:1" )
	PORT_DIPSETTING(    0x60, "5:1" )
	PORT_DIPSETTING(    0x50, "10:1" )
	PORT_DIPSETTING(    0x40, "20:1" )
	PORT_DIPSETTING(    0x30, "25:1" )
	PORT_DIPSETTING(    0x20, "50:1" )
	PORT_DIPSETTING(    0x10, "100:1" )
	PORT_DIPSETTING(    0x00, "200:1" )
	PORT_DIPNAME( 0x80, 0x00, "Oddstab Fixed" ) PORT_DIPLOCATION("SWE:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("SERVICE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW") // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void igspoker_state::number10_io_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(igspoker_state::exp_rom_r));
	map(0x2000, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2800, 0x2fff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x4000, 0x4000).portr("DSW1");           /* DSW1 */
	map(0x4001, 0x4001).portr("DSW2");           /* DSW2 */
	map(0x4002, 0x4002).portr("DSW3");           /* DSW3 */
	map(0x4003, 0x4003).portr("DSW4");           /* DSW4 */
	map(0x4004, 0x4004).portr("DSW5");           /* DSW5 */
	map(0x4006, 0x4006).portr("DSW6");
	map(0x4007, 0x4007).portr("DSW7");
	map(0x50f0, 0x50f0).w(FUNC(igspoker_state::nmi_and_coins_w));
	map(0x5080, 0x5080).portr("SERVICE");            /* Services */
	map(0x5090, 0x5090).w(FUNC(igspoker_state::custom_io_w));
	map(0x5091, 0x5091).r(FUNC(igspoker_state::custom_io_r)).w(FUNC(igspoker_state::lamps_w));            /* Keyboard */
	map(0x50a0, 0x50a0).portr("BUTTONS2");
	/* Sound synthesys has been patched out, replaced by ADPCM samples */
	map(0x50b0, 0x50b0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x50c0, 0x50c0).r(FUNC(igspoker_state::irqack_r)).w(FUNC(igspoker_state::irqack_w));
	map(0x7000, 0x77ff).ram().w(FUNC(igspoker_state::fg_tile_w)).share("fg_tile_ram");
	map(0x7800, 0x7fff).ram().w(FUNC(igspoker_state::fg_color_w)).share("fg_color_ram");
}

void igspoker_state::cpokerpk_io_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(igspoker_state::exp_rom_r));
	map(0x2000, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2800, 0x2fff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x4000, 0x4000).portr("DSW1");           /* DSW1 */
	map(0x4001, 0x4001).portr("DSW2");           /* DSW2 */
	map(0x4002, 0x4002).portr("DSW3");           /* DSW3 */
	map(0x4003, 0x4003).portr("DSW4");           /* DSW4 */
	map(0x4004, 0x4004).portr("DSW5");           /* DSW5 */
	map(0x50f0, 0x50f0).w(FUNC(igspoker_state::nmi_and_coins_w));
	map(0x5081, 0x5081).portr("SERVICE");            /* Services */
	map(0x5082, 0x5082).portr("COINS");          /* Coing & Kbd */
	map(0x5090, 0x5090).w(FUNC(igspoker_state::custom_io_w));
	map(0x5091, 0x5091).r(FUNC(igspoker_state::custom_io_r)).w(FUNC(igspoker_state::lamps_w));            /* Keyboard */
	map(0x50a0, 0x50a0).portr("BUTTONS2");
	/* Sound synthesys has been patched out, replaced by ADPCM samples */
	map(0x50b0, 0x50b0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x50c0, 0x50c0).r(FUNC(igspoker_state::irqack_r)).w(FUNC(igspoker_state::irqack_w));
	map(0x7000, 0x77ff).ram().w(FUNC(igspoker_state::fg_tile_w)).share("fg_tile_ram");
	map(0x7800, 0x7fff).ram().w(FUNC(igspoker_state::fg_color_w)).share("fg_color_ram");
}

static INPUT_PORTS_START( number10 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0x00, "50000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Val Premio" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	/* Whatever value is selected, code will force ACTIVE_LOW, thus Manual mode */
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "W-UP Limit" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x02, 0x02, "W-UP Pool" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )     /* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x00, "Strip Girl" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	/* Whatever value is selected, code will force ACTIVE_LOW, thus Change */
	PORT_DIPNAME( 0x80, 0x00, "Win Table" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, "Change" )
	PORT_DIPSETTING(    0x00, "Change" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "50%" )
	PORT_DIPSETTING(    0x0e, "52%" )
	PORT_DIPSETTING(    0x0d, "54%" )
	PORT_DIPSETTING(    0x0c, "56%" )
	PORT_DIPSETTING(    0x0b, "58%" )
	PORT_DIPSETTING(    0x0a, "60%" )
	PORT_DIPSETTING(    0x09, "62%" )
	PORT_DIPSETTING(    0x08, "64%" )
	PORT_DIPSETTING(    0x07, "66%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "72%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x02, "76%" )
	PORT_DIPSETTING(    0x01, "78%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	/* Whatever value is selected, code will force ACTIVE_LOW, thus Bet Max */
	PORT_DIPNAME( 0x80, 0x00, "Pts Play" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, "Bet Max" )
	PORT_DIPSETTING(    0x00, "Bet Max" )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("HPSW")

	PORT_START("DSW7")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("HPSW")

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xC0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( cpokerpk )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0x00, "50000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Val Premio" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	/* Whatever value is selected, code will force ACTIVE_LOW, thus Manual mode */
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "W-UP Limit" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x02, 0x02, "W-UP Pool" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )     /* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x00, "Strip Girl" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "69%" )
	PORT_DIPSETTING(    0x0e, "72%" )
	PORT_DIPSETTING(    0x0d, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0b, "81%" )
	PORT_DIPSETTING(    0x0a, "83%" )
	PORT_DIPSETTING(    0x09, "85%" )
	PORT_DIPSETTING(    0x08, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x06, "91%" )
	PORT_DIPSETTING(    0x05, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x02, "99%" )
	PORT_DIPSETTING(    0x01, "101%" )
	PORT_DIPSETTING(    0x00, "103%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Anytime Key-in" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Hopper" ) PORT_DIPLOCATION("SWE:8")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Payout Select" ) PORT_DIPLOCATION("SWE:7")
	PORT_DIPSETTING(    0x02, "Hopper" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x0c, 0x0c, "Ticket Rate" ) PORT_DIPLOCATION("SWE:6,5")
	PORT_DIPSETTING(    0x0c, "10:1" )
	PORT_DIPSETTING(    0x08, "20:1" )
	PORT_DIPSETTING(    0x04, "50:1" )
	PORT_DIPSETTING(    0x00, "100:1" )

	PORT_START("SERVICE")
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW") // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xC0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( chleague )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:7,6")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x10, "Max Bet" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x60, 0x60, "Min Bet to play Fever" ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Credit Limit" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0x00, "50000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Val Premio" ) PORT_DIPLOCATION("SWB:3,2")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "1" )
	/* Whatever value is selected, code will force ACTIVE_LOW, thus Manual mode */
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "W-UP Limit" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x02, 0x02, "W-UP Pool" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x0c, 0x0c, "W-UP Chance" ) PORT_DIPLOCATION("SWC:6,5")
	PORT_DIPSETTING(    0x0c, "94%" )
	PORT_DIPSETTING(    0x08, "96%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x30, 0x20, "W-UP Type" ) PORT_DIPLOCATION("SWC:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "High-Low" )
	PORT_DIPSETTING(    0x10, "Red-Black" )     /* Bit 4 is equal for ON/OFF */
	PORT_DIPNAME( 0x40, 0x00, "Strip Girl" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	/* Whatever value is selected, code will force ACTIVE_LOW, thus Change */
	PORT_DIPNAME( 0x80, 0x00, "Win Table" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, "Change" )
	PORT_DIPSETTING(    0x00, "Change" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x07, "Main Game Chance" ) PORT_DIPLOCATION("SWD:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "50%" )
	PORT_DIPSETTING(    0x0e, "52%" )
	PORT_DIPSETTING(    0x0d, "54%" )
	PORT_DIPSETTING(    0x0c, "56%" )
	PORT_DIPSETTING(    0x0b, "58%" )
	PORT_DIPSETTING(    0x0a, "60%" )
	PORT_DIPSETTING(    0x09, "62%" )
	PORT_DIPSETTING(    0x08, "64%" )
	PORT_DIPSETTING(    0x07, "66%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "72%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x02, "76%" )
	PORT_DIPSETTING(    0x01, "78%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPNAME( 0x10, 0x00, "Five Jokers" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Royal Flush" ) PORT_DIPLOCATION("SWD:3")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" ) PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	/* Whatever value is selected, code will force ACTIVE_LOW, thus Bet Max */
	PORT_DIPNAME( 0x80, 0x00, "Pts Play" ) PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x80, "Bet Max" )
	PORT_DIPSETTING(    0x00, "Bet Max" )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW") // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pktet346 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Demo Game" ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Open Mode" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x04, "Demo" )
	PORT_DIPSETTING(    0x00, "Amuse" )
	PORT_DIPNAME( 0x18, 0x18, "Min Bet to Start" ) PORT_DIPLOCATION("SWA:5,4")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0xe0, 0xe0, "Max Bet" ) PORT_DIPLOCATION("SWA:3,2,1")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0xa0, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x60, "50" )
	PORT_DIPSETTING(    0x40, "75" )
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x00, "200" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Amuse Coin" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, "1:1" )
	PORT_DIPSETTING(    0x00, "5:1" )
	PORT_DIPNAME( 0x02, 0x02, "Amuse Game" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x02, "Free" )
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPNAME( 0x04, 0x04, "Display Card" ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x04, "Poker" )
	PORT_DIPSETTING(    0x00, "Numbers" )
	PORT_DIPNAME( 0x18, 0x18, "Key In Rate" ) PORT_DIPLOCATION("SWB:5,4")
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xe0, 0xe0, "Coin Setting" ) PORT_DIPLOCATION("SWB:3,2,1")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "5" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x60, "20" )
	PORT_DIPSETTING(    0x40, "40" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "100" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Speed" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, "Slow" )
	PORT_DIPSETTING(    0x00, "Quick" )
	PORT_DIPNAME( 0x02, 0x02, "Quick Get" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bet Base" ) PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x18, "System Limit" ) PORT_DIPLOCATION("SWC:5,4")
	PORT_DIPSETTING(    0x18, "30000" )
	PORT_DIPSETTING(    0x10, "50000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0xe0, 0xe0, "Key Out Base" ) PORT_DIPLOCATION("SWC:3,2,1")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0xa0, "20" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x60, "100" )
	PORT_DIPSETTING(    0x40, "100" )
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x00, "100" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Double Game" ) PORT_DIPLOCATION("SWD:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Royal Appear" ) PORT_DIPLOCATION("SWD:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "5 Kind Appear" ) PORT_DIPLOCATION("SWD:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Payout" ) PORT_DIPLOCATION("SWD:5")
	PORT_DIPSETTING(    0x08, "Manual" )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x10, 0x00, "Hopper" ) PORT_DIPLOCATION("SWD:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Set" ) PORT_DIPLOCATION("SWD:3,2,1")
	PORT_DIPSETTING(    0xe0, "1:1" )
	PORT_DIPSETTING(    0xc0, "5:1" )
	PORT_DIPSETTING(    0xa0, "10:1" )
	PORT_DIPSETTING(    0x80, "20:1" )
	PORT_DIPSETTING(    0x60, "25:1" )
	PORT_DIPSETTING(    0x40, "50:1" )
	PORT_DIPSETTING(    0x20, "100:1" )
	PORT_DIPSETTING(    0x00, "200:1" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Double Gate" ) PORT_DIPLOCATION("SWE:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Percentage" ) PORT_DIPLOCATION("SWE:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW") // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( igstet341 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Display Card" ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x02, "Poker" )
	PORT_DIPSETTING(    0x00, "Numbers" )
	PORT_DIPNAME( 0x04, 0x04, "Speed" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x04, "Slow" )
	PORT_DIPSETTING(    0x00, "Quick" )
	PORT_DIPNAME( 0x08, 0x08, "Double Gate" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x08, "Easy" )
	PORT_DIPSETTING(    0x00, "Difficult" )
	PORT_DIPNAME( 0x30, 0x30, "System Limit" ) PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x30, "10000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x00, "70000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Coin Setting" ) PORT_DIPLOCATION("SWA:2,1")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" ) PORT_DIPLOCATION("SWB:8,7")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" ) PORT_DIPLOCATION("SWB:6,5")
	PORT_DIPSETTING(    0x0c, "40" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x30, 0x30, "Key In" ) PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x10, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x40, 0x00, "Demo Game" ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Amuse Game" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, "Free" )
	PORT_DIPSETTING(    0x00, "1 Credit" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Double Game" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Royal Appear" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "5 Kind Appear" ) PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Key Out Base" ) PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Open Mode" ) PORT_DIPLOCATION("SWC:4")
	PORT_DIPSETTING(    0x10, "Demo" )
	PORT_DIPSETTING(    0x00, "Amuse" )
	PORT_DIPNAME( 0x20, 0x20, "Quick Get" ) PORT_DIPLOCATION("SWC:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Bet Base" ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x80, "Percentage" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Amuse Coin" ) PORT_DIPLOCATION("SWD:8")
	PORT_DIPSETTING(    0x01, "1:1" )
	PORT_DIPSETTING(    0x00, "5:1" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(igspoker_state, hopper_r) PORT_NAME("HPSW") // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Held 1 / Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Held 2 / Extra")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Held 3 / Bet 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Held 4 / Low")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Held 5 / W_Up")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Max Bet / High")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Move Left")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Move Right")
	PORT_BIT( 0x1e, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8, 8,   /* 8*8 characters */
	RGN_FRAC(1, 3),
	6,      /* 6 bits per pixel */
	{ RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2   /* every char takes 32 consecutive bytes */
};

static const gfx_layout charlayout2 =
{
	8, 32,   /* 8*32 characters */
	RGN_FRAC(1, 3*4),
	6,      /* 6 bits per pixel */
	{ RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};


static GFXDECODE_START( gfx_igspoker )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, charlayout2,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, charlayout2,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, charlayout2,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, charlayout2,  0, 16 )
GFXDECODE_END

static const gfx_layout charlayoutcpk =
{
	8, 8,   /* 8*8 characters */
	RGN_FRAC(1, 1),
	6,
	{ 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static GFXDECODE_START( gfx_cpokerpk )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayoutcpk,   0, 16 )
	/* these not used? */
	GFXDECODE_ENTRY( "gfx2", 0x04000, charlayout2,  0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, charlayout2,  0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, charlayout2,  0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, charlayout2,  0, 1 )
GFXDECODE_END

void igspoker_state::igspoker(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3579545);
	m_maincpu->set_addrmap(AS_PROGRAM, &igspoker_state::igspoker_prg_map);
	m_maincpu->set_addrmap(AS_IO, &igspoker_state::igspoker_io_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igspoker_state::interrupt), "screen", 0, 1);

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(igspoker_state::nmi_and_coins_w));
	ppi.in_pb_callback().set_ioport("SERVICE");
	ppi.in_pc_callback().set_ioport("COINS");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8); // TODO: wrong screen size!
	m_screen->set_visarea(0*8, 64*8-1, 0, 32*8-1);
	m_screen->set_screen_update(FUNC(igspoker_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_igspoker);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	YM2413(config, "ymsnd", 3579545).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void igspoker_state::csk227it(machine_config &config)
{
	igspoker(config);
}

void igspoker_state::csk234it(machine_config &config)
{
	igspoker(config);
}

void igspoker_state::igs_ncs(machine_config &config)
{
	igspoker(config);
}

void igspoker_state::number10(machine_config &config)
{
	igspoker(config);

	m_maincpu->set_addrmap(AS_IO, &igspoker_state::number10_io_map);

	config.device_remove("ppi");

	m_screen->set_screen_update(FUNC(igspoker_state::screen_update_cpokerpk));

	MCFG_VIDEO_START_OVERRIDE(igspoker_state,cpokerpk)

	OKIM6295(config, "oki", XTAL(12'000'000) / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void igspoker_state::cpokerpk(machine_config &config)
{
	number10(config);

	m_maincpu->set_addrmap(AS_IO, &igspoker_state::cpokerpk_io_map);
	m_gfxdecode->set_info(gfx_cpokerpk);
}


void igspoker_state::pktetris(machine_config &config)
{
	igspoker(config);
}



/*  ROM Regions definition
 */

ROM_START( cpoker )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "v220i1.bin",  0x0000, 0x8000, CRC(b7cae556) SHA1(bb43ee48634879029ed1a7cd4133d7f12413e2ac) )
	ROM_LOAD( "v220i2.bin",  0x8000, 0x8000, CRC(8245e42c) SHA1(b7e7b9f643e6dc2f4d5aaf7d50d0a9154ed9a4e7) )
	ROM_LOAD( "220i7.bin",   0x18000, 0x8000, CRC(8a2ff310) SHA1(a415a99dbb1448b4b2b94e17a3973e6347e3be18) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "220i1.bin",  0x40000, 0x20000, CRC(9c4c0af1) SHA1(7a9808b3093b23bde7ecc7405689b2a28ae34e61) )
	ROM_LOAD( "220i2.bin",  0x20000, 0x20000, CRC(331fa4b8) SHA1(ddac57251fa5dfecc0988a2ca01eec016ef47f20) )
	ROM_LOAD( "220i3.bin",  0x00000, 0x20000, CRC(bd2f797c) SHA1(5ca5adae44490dd109f630213a09a68c12f9bd1a) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
ROM_END

ROM_START( csk227it )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "v227i.bin",   0x0000, 0x10000, CRC(df1ebf49) SHA1(829c7575d3d3780557405b3a61859901df6dbe4f) )
	ROM_LOAD( "7.227",   0x10000, 0x10000, CRC(a10786ad) SHA1(82f5f81808ca70d67a2710cc66fbbf78588b33b5) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "6.227",  0x00000, 0x20000, CRC(e9aad93b) SHA1(72116759cd8ddd9828f534e8f8a3f9f96ad2e002) )
	ROM_LOAD( "5.227",  0x20000, 0x20000, CRC(e4c4c8da) SHA1(0442b0de68f3b69e613506348e00c3cf9139edcf) )
	ROM_LOAD( "4.227",  0x40000, 0x20000, CRC(afb365dd) SHA1(930a4cd516258e703a75afc25ef6b2655b8b696a) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3.bin",  0x00000, 0x10000, CRC(fcb115ac) SHA1(a9f2b9762413840669cd44f8e54b47a7c4350d11) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(848343a3) SHA1(b12f9bc2feb470d2fa8b085621fa60c0895109d4) )
	ROM_LOAD( "1.bin",  0x20000, 0x10000, CRC(921ad5de) SHA1(b06ab2e63b31361dcb0367110f47bf2453ecdca6) )
ROM_END

ROM_START( csk234it )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "v234it.bin",   0x0000, 0x10000, CRC(344b7059)  SHA1(990cb84e35c0c50d3be9fbb76a11395114dc6c9b) )
	ROM_LOAD( "7.234",   0x10000, 0x10000, CRC(ae6dd4ad) SHA1(4772d5c150d64d1ef3b68e16214f594eea0b3c1b) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "6.234",  0x00000, 0x20000, CRC(23b855a4) SHA1(8217bac61ad09483d8789113cf394d0e525ab28a) )
	ROM_LOAD( "5.234",  0x20000, 0x20000, CRC(189039d7) SHA1(146fd1ddb23ceaa4192e0382b0ab82f5cfbdabfe) )
	ROM_LOAD( "4.234",  0x40000, 0x20000, CRC(c82b0ffc) SHA1(5ebd7da76d402b7111cbe9012cfa3b8a8ff1a86e) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3.bin",  0x00000, 0x10000, CRC(fcb115ac) SHA1(a9f2b9762413840669cd44f8e54b47a7c4350d11) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(848343a3) SHA1(b12f9bc2feb470d2fa8b085621fa60c0895109d4) )
	ROM_LOAD( "1.bin",  0x20000, 0x10000, CRC(921ad5de) SHA1(b06ab2e63b31361dcb0367110f47bf2453ecdca6) )
ROM_END


/*

Stelle e Cubi

-- most of the roms on this seem to be the wrong size / missing data
   but its appears to be a hack based on Champion Skill

1x Z84c0006
1x 12mhz OSC
1x U6295 sound chip
1x Actel FPGA (gfx chip)

ROMs
Note    1x Battery
5x banks of dipswitch


--

This doesn't attempt to decode the gfx.

*/
ROM_START( stellecu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	/* there is data at 0x18000 which is probably mapped somewhere */
	ROM_LOAD( "u35.bin",   0x0000, 0x20000, CRC(914b7c59) SHA1(3275b5016524467199f32d653c757bfe4f9cfc60) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	/* seems to be missing half the gfx */
	ROM_LOAD( "u23.bin",   0x0000, 0x40000, BAD_DUMP CRC(9d95757d) SHA1(f7f44d684f1f3a5b1e9c0a82f4377c6d79eb4214) )
	ROM_LOAD( "u25.bin",   0x4000, 0x40000, BAD_DUMP CRC(63094010) SHA1(a781f1c529167dd0ab411c66b72105fc19e32f02) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASEFF )


	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	/* missing sample tables at start of rom */
	ROM_LOAD( "u15.bin",   0x0000, 0x40000, BAD_DUMP CRC(72e3e9c1) SHA1(6a8fb93059bee5a4e4b4deb9fee4b5869e53983b) )
ROM_END

/*  Decode a simple PAL encryption
 */

void igspoker_state::init_cpoker()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		rom[A] ^= 0x21;
		if ((A & 0x0030) == 0x0010) rom[A] ^= 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}

/*  Patch to avoid traps at $0eed and $206e
    that run subs in RAM, operate registers,
    and finally lock the game at $72c2.

    All these are triggered if RAM contents of $ff18
    matches the $ff19 (normally 0x20 due to an AND
    against the $ff1b contents)
*/
//      this NOP the $0eed call...
		rom[0x214a] = 0x00;
		rom[0x214b] = 0x00;
		rom[0x214c] = 0x00;

//      this NOP the conditional jump to $206e
		rom[0x214d] = 0x00;
		rom[0x214e] = 0x00;
		rom[0x214f] = 0x00;

}

void igspoker_state::init_cpoker300us()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		rom[A] ^= 0x01;
		if ((A & 0x00e0) == 0x00a0) rom[A] ^= 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
}

void igspoker_state::init_cpokert()
{
	uint8_t *rom = memregion("maincpu")->base();
	/* decrypt the program ROM */
	for (int i = 0; i < 0x10000; i++)
	{
		if((i & 0x200) && (i & 0x80))
		{
			rom[i] ^= ((~i & 2) >> 1);
		}
		else
		{
			rom[i] ^= 0x01;
		}

		if((i & 0x30) != 0x10)
		{
			rom[i] ^= 0x20;
		}

		if((i & 0x900) == 0x900 && ((i & 0xc0) == 0x40 || (i & 0xc0) == 0xc0))
		{
			rom[i] ^= 0x02;
		}
	}

/*  Patch to avoid traps at $0eed and $206e
    that run subs in RAM, operate registers,
    and finally lock the game at $72c2.

    All these are triggered if RAM contents of $ff18
    matches the $ff19 (normally 0x20 due to an AND
    against the $ff1b contents)
*/
//      this NOP the $0eed call...
		rom[0x214a] = 0x00;
		rom[0x214b] = 0x00;
		rom[0x214c] = 0x00;

//      this NOP the conditional jump to $206e
		rom[0x214d] = 0x00;
		rom[0x214e] = 0x00;
		rom[0x214f] = 0x00;
}

void igspoker_state::init_cpoker101()  // same decryption as cpokert
{
	uint8_t *rom = memregion("maincpu")->base();
	/* decrypt the program ROM */
	for (int i = 0; i < 0x10000; i++)
	{
		if((i & 0x200) && (i & 0x80))
		{
			rom[i] ^= ((~i & 2) >> 1);
		}
		else
		{
			rom[i] ^= 0x01;
		}

		if((i & 0x30) != 0x10)
		{
			rom[i] ^= 0x20;
		}

		if((i & 0x900) == 0x900 && ((i & 0xc0) == 0x40 || (i & 0xc0) == 0xc0))
		{
			rom[i] ^= 0x02;
		}
	}

/*  Patch to avoid traps at $0ec5 (cpoker101),
    $0ef0 (cpoker201f), $0f20 (cpoker210ks) and
    $206e (cpoker101, cpoker201f & cpoker210ks),
    that run subs in RAM, operate registers,
    and finally lock the game at $732e (cpoker101),
    $72c2 (cpoker201f) & $72c6 (cpoker210ks).

    All these are triggered if RAM contents of $ff18
    matches the $ff19 (normally 0x20 due to an AND
    against the $ff1b contents)
*/
//      this NOP the $0ec5 call...
		rom[0x214a] = 0x00;
		rom[0x214b] = 0x00;
		rom[0x214c] = 0x00;

//      this NOP the conditional jump to $206e
		rom[0x214d] = 0x00;
		rom[0x214e] = 0x00;
		rom[0x214f] = 0x00;
}

void igspoker_state::init_cska()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		if ((A & 0x0020) == 0x0000) rom[A] ^= 0x01;
		if ((A & 0x0020) == 0x0020) rom[A] ^= 0x21;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0028) == 0x0028) rom[A] ^= 0x20;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
}


void igspoker_state::init_igs_ncs()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		rom[A] ^= 0x21;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0140) == 0x0100) rom[A] ^= 0x20;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
}


/*

1x ZILOG Z0840006PSC-Z80CPU (main)
1x YM2413 (sound)
1x NEC D8255AC (label: ORIGINAL BY IGS 102986)
1x oscillator 12.000MHz (main)
1x oscillator 3.579545MHz (sound)

1x custom QFP80 label AMT001
1x custom QFP80 label IGS002
1x custom DIP40 label IGS003 (under chip label 8255)

ROMs

3x MX27C1000DC (4,5,6)
1x NM27C256Q (7)
1x 27C512 (200)
2x PEEL18CV8P (8,9)
1x PAL16L8ACN (31)
2x PEEL18CV8P (12,14) <-> UNREADABLE, protected!

Note

1x 10x2 edge connector (con1) (looks like a coin payout)
1x 36x2 edge connector (con2)
1x pushbutton (sw6)
5x 8 switches dips (sw1-5)
1x trimmer (volume)
----------------------
IGS PCB NO-T0039-8

*/

ROM_START( cpokert )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "champingv-200g.u23", 0x00000, 0x10000, CRC(696cb684) SHA1(ce9e5bed83d0bd3b115f556cc89e3293ac6b69c3) )
	ROM_LOAD( "cpoker7.u22", 0x18000, 0x8000, CRC(dae3ecda) SHA1(c881e143ec600c5a931f26cd097da6353e1da7c3) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "cpoker6.u6", 0x00000, 0x20000, CRC(f3e61b24) SHA1(b18998defb6e51daef4ac5a5865674565ffb9029) )
	ROM_LOAD( "cpoker5.u5", 0x20000, 0x20000, CRC(a68b305f) SHA1(f872d2bf7ab194145dffe6b254ae0ad66aa6a497) )
	ROM_LOAD( "cpoker4.u4", 0x40000, 0x20000, CRC(860be7c9) SHA1(41bc58713076276aeefc44c7ea903549692b0224) )

	//copy?
	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_COPY( "gfx1", 0x000000, 0, 0x60000 )

	// convert them to the pld format
	ROM_REGION( 0x2000, "plds", 0 )
	ROM_LOAD( "ag-u31.u31", 0x00000, 0x000b60, CRC(fd36baf2) SHA1(caac8bf47bc958395f97b6191569196efe3b3eaa) )
	ROM_LOAD( "ag-u8.u8",   0x00000, 0x0015e2, CRC(c0308c63) SHA1(16819a5c147fef38a235675fa4442da9fa8a6618) )
	ROM_LOAD( "ag-u9.u9",   0x00000, 0x0015e2, CRC(2e8039a3) SHA1(e39635ee9485a5ccd28526f1af7ec2e3294b0aec) )
ROM_END


/*
  Champion Poker (IGS) V100.

1x ZILOG Z0840006PSC-Z80CPU (main)

1x oscillator 12.000MHz (main)

1x custom QFP80 label IGS001A
1x custom QFP80 label IGS002

Note

1x 10x2 edge connector (con1) (looks like a coin payout)
1x 36x2 edge connector (con2)
1x switch (sw6)
5x 8 DIP switches (sw1-5)
1x trimmer (volume)
----------------------
IGS PCB NO-0139-3

*/

ROM_START( cpokerx )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "champion_v-100.bin", 0x00000, 0x10000, CRC(00fc9fc3) SHA1(ee6d4e156f0bf866a4b93272b92bb460dd7e73e1) )
	ROM_LOAD( "champion7.u22",      0x18000, 0x8000,  CRC(123ff157) SHA1(aa1d1dc589a2d1ca38b667ab88706280347088b4) )
	ROM_IGNORE(                              0x8000)

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "champion3.u6", 0x00000, 0x20000, CRC(f3e61b24) SHA1(b18998defb6e51daef4ac5a5865674565ffb9029) )
	ROM_LOAD( "champion2.u5", 0x20000, 0x20000, CRC(a68b305f) SHA1(f872d2bf7ab194145dffe6b254ae0ad66aa6a497) )
	ROM_LOAD( "champion1.u4", 0x40000, 0x20000, CRC(860be7c9) SHA1(41bc58713076276aeefc44c7ea903549692b0224) )

	//copy?
	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_COPY( "gfx1", 0x000000, 0, 0x60000 )

	ROM_REGION( 0x4000, "plds", 0 )
	ROM_LOAD( "16v8b.u31",  0x00000, 0x000892, BAD_DUMP CRC(33dec5f5) SHA1(f5c2e45513fa3657160ff38111a745f76cf679e1) )  // all 0's, seems protected
	ROM_LOAD( "16v8h.u14",  0x01000, 0x000892, CRC(123d539a) SHA1(cccf0cbae3175b091a998eedf4aa44a55b679400) )
	ROM_LOAD( "22v10b.u22", 0x02000, 0x001704, CRC(609a1aaa) SHA1(b123c93929f932e4ee343a7109f8b16717845d8b) )
ROM_END

ROM_START( cpoker101 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u20-v01.bin",        0x00000, 0x10000, CRC(ce99fe3c) SHA1(b5df1f2f5c086626b072b9978383484f699f628b) )
	ROM_LOAD( "champion7.u21",      0x18000, 0x8000,  CRC(123ff157) SHA1(aa1d1dc589a2d1ca38b667ab88706280347088b4) )
	ROM_IGNORE(                              0x8000)

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "champion3.u6", 0x00000, 0x20000, CRC(f3e61b24) SHA1(b18998defb6e51daef4ac5a5865674565ffb9029) )
	ROM_LOAD( "champion2.u5", 0x20000, 0x20000, CRC(a68b305f) SHA1(f872d2bf7ab194145dffe6b254ae0ad66aa6a497) )
	ROM_LOAD( "champion1.u4", 0x40000, 0x20000, CRC(860be7c9) SHA1(41bc58713076276aeefc44c7ea903549692b0224) )

	//copy?
	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_COPY( "gfx1", 0x000000, 0, 0x60000 )
ROM_END

ROM_START( cpoker201f )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u20-201f.bin",       0x00000, 0x10000, CRC(000e0f8d) SHA1(63ded0c3bfaeed6b57870706d379a975cc5790c4) )
	ROM_LOAD( "champion7.u21",      0x18000, 0x8000,  CRC(123ff157) SHA1(aa1d1dc589a2d1ca38b667ab88706280347088b4) )
	ROM_IGNORE(                              0x8000)

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "champion3.u6", 0x00000, 0x20000, CRC(f3e61b24) SHA1(b18998defb6e51daef4ac5a5865674565ffb9029) )
	ROM_LOAD( "champion2.u5", 0x20000, 0x20000, CRC(a68b305f) SHA1(f872d2bf7ab194145dffe6b254ae0ad66aa6a497) )
	ROM_LOAD( "champion1.u4", 0x40000, 0x20000, CRC(860be7c9) SHA1(41bc58713076276aeefc44c7ea903549692b0224) )

	//copy?
	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_COPY( "gfx1", 0x000000, 0, 0x60000 )
ROM_END

ROM_START( cpoker210ks )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u20-210ks.bin",      0x00000, 0x10000, CRC(8900ccba) SHA1(e8796602db0ab1c1e73ab37d380e7fe39060646d) )
	ROM_LOAD( "champion7.u21",      0x18000, 0x8000,  CRC(123ff157) SHA1(aa1d1dc589a2d1ca38b667ab88706280347088b4) )
	ROM_IGNORE(                              0x8000)

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "champion3.u6", 0x00000, 0x20000, CRC(f3e61b24) SHA1(b18998defb6e51daef4ac5a5865674565ffb9029) )
	ROM_LOAD( "champion2.u5", 0x20000, 0x20000, CRC(a68b305f) SHA1(f872d2bf7ab194145dffe6b254ae0ad66aa6a497) )
	ROM_LOAD( "champion1.u4", 0x40000, 0x20000, CRC(860be7c9) SHA1(41bc58713076276aeefc44c7ea903549692b0224) )

	//copy?
	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_COPY( "gfx1", 0x000000, 0, 0x60000 )
ROM_END

ROM_START( cpoker300us )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u20 v300us.bin",     0x00000, 0x10000, CRC(510dc75c) SHA1(0514135211ccada2aa7c1d87bf2bac64399cfc51) )
	ROM_LOAD( "champion7.u21",      0x18000, 0x8000,  CRC(123ff157) SHA1(aa1d1dc589a2d1ca38b667ab88706280347088b4) )
	ROM_IGNORE(                              0x8000)

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "u50-champion 3.bin", 0x00000, 0x20000, CRC(9f076732) SHA1(0d62db6a26e219032801ca90dc78470005a81ff2) )
	ROM_LOAD( "u51-champion 2.bin", 0x20000, 0x20000, CRC(9b775e3a) SHA1(df4a3ef284924900af8035059f167668f33fdb6a) )
	ROM_LOAD( "u52-champion 1.bin", 0x40000, 0x20000, CRC(c032f9a5) SHA1(468a4b8729ad3a4f0f2045bc4aa6ba853f103b70) )

	//copy?
	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_COPY( "gfx1", 0x000000, 0, 0x60000 )
ROM_END

ROM_START( igs_ncs )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "v.bin",   0x00000, 0x10000, CRC(8077724b) SHA1(1f6e01d5838e6ec4f91b07637c281a3f59631a51) )
	ROM_LOAD( "7.bin",   0x10000, 0x10000, CRC(678e412c) SHA1(dba031d3576d098d314d6589dd1aeda44d17c650) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "6.bin",  0x00000, 0x20000, CRC(d8e88148) SHA1(5f5c06d947027ef76026e8834f2090b96652006c) )
	ROM_LOAD( "5.bin",  0x20000, 0x20000, CRC(96c8a71c) SHA1(202d04850df9dfbd405c4b5372ef1b39850ac7f7) )
	ROM_LOAD( "4.bin",  0x40000, 0x20000, CRC(5480eae8) SHA1(93e35e8ba7d282cb93d51498420341a4e95acf78) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3.bin",  0x00000, 0x10000, CRC(fcb115ac) SHA1(a9f2b9762413840669cd44f8e54b47a7c4350d11) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(848343a3) SHA1(b12f9bc2feb470d2fa8b085621fa60c0895109d4) )
	ROM_LOAD( "1.bin",  0x20000, 0x10000, CRC(921ad5de) SHA1(b06ab2e63b31361dcb0367110f47bf2453ecdca6) )
ROM_END


/* New Champion Skill by IGS
 -- the dump MAY be incomplete, there were 3 empty positions on the PCB near
    the gfx roms

Chips of Note

IGS 003C (near chip with TEST OK E0069281 label)
IGS 002
IGA 001A


'file'
KC8255A
9941
(near CPU roms)

UM3567 9946

5x 8 switch dips

Clocks
3.579545Mhz (near sound)
12Mhz


--- what is the CPU, it looks like either Z80 or Z180 based
 -- CPU rom is lightly encrypted (usual IGS style, some xors)

*/

void igspoker_state::init_igs_ncs2()
{
	uint8_t *src = (uint8_t *) (memregion("maincpu")->base());
	for (int i = 0; i < 0x10000; i++)
	{
		/* bit 0 xor layer */
		if(i & 0x200)
		{
			if(i & 0x80)
			{
				if(~i & 0x02)
				{
					src[i] ^= 0x01;
				}
			}
			else
			{
				src[i] ^= 0x01;
			}
		}
		else
		{
			src[i] ^= 0x01;
		}

		/* bit 1 xor layer */
		if(i & 0x800)
		{
			if(i & 0x100)
			{
				if(i & 0x40)
				{
					src[i] ^= 0x02;
				}
			}
		}

		/* bit 5 xor layer */
		if(i & 0x100)
		{
			if(i & 0x40)
			{
				src[i] ^= 0x20;
			}
		}
		else
		{
			src[i] ^= 0x20;
		}
	}

}

ROM_START( igs_ncs2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ncs_v100n.u20", 0x00000, 0x10000, CRC(2bb91de5) SHA1(b0b7b3b9cee1ce4da10cf78ef1c8079f3d9cafbf) )
	ROM_LOAD( "ncs_v100n.u21", 0x10000, 0x10000, CRC(678e412c) SHA1(dba031d3576d098d314d6589dd1aeda44d17c650) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "ncs_v100n.u50", 0x00000, 0x40000, CRC(ff2bb3dc) SHA1(364c948504003b4230fbdac74227842c802d4c12) )
	ROM_LOAD( "ncs_v100n.u51", 0x40000, 0x40000, CRC(f8530313) SHA1(b21d6de7d5d4b902008ceea7e1227545e0d1701b) )
	ROM_LOAD( "ncs_v100n.u52", 0x80000, 0x40000, CRC(2fa5b6df) SHA1(5bfc651297440f73692079f1806b1e40b457b7b8) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASEFF )
	// looks like these are needed for pre-game screens, sockets were empty
	ROM_LOAD( "ncs_v100n.u55", 0x00000, 0x10000, NO_DUMP )
	ROM_LOAD( "ncs_v100n.u56", 0x10000, 0x10000, NO_DUMP )
	ROM_LOAD( "ncs_v100n.u57", 0x20000, 0x10000, NO_DUMP )
ROM_END


void igspoker_state::init_chleague()
{
	uint8_t *rom = memregion("maincpu")->base();
	int length = memregion("maincpu")->bytes();
	for (int A = 0; A < length; A++)
	{
		if ((A & 0x09C0) == 0x0880) rom[A] ^= 0x20;
		if ((A & 0x0B40) == 0x0140) rom[A] ^= 0x20;
	}

	/* Renable patched out DSW Display in test mode */
	rom[0xA835] = 0xcd;
	rom[0xA836] = 0x3a;
	rom[0xA837] = 0x48;

	rom[0xA863] = 0xcd;
	rom[0xA864] = 0x40;
	rom[0xA865] = 0xd3;

	rom[0xaade] = 0xcd;
	rom[0xaadf] = 0x17;
	rom[0xaae0] = 0xa5;

	/* Fix graphic glitch */
	rom[0x48e8] = 0x19;
	rom[0x48e9] = 0x5e;
	rom[0x48ea] = 0x23;

	/* Patch trap */
	rom[0x0eed] = 0xc3;
}

ROM_START( chleague )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "12b.bin", 0x00000, 0x10000, CRC(8b4fb718) SHA1(2ce7cf73aab8a644ecac4189c8ffe7dae9a21571) )
	ROM_LOAD( "12a.bin", 0x10000, 0x10000, CRC(bd3af488) SHA1(3c5e7a8623d11bd50a1949e870f1044eec7fc463) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "23.bin", 0x40000, 0x20000, CRC(4ac8cc41) SHA1(e4bfd63408511e7d21f140d315493af7fdeba373) )
	ROM_LOAD( "24.bin", 0x20000, 0x20000, CRC(6cb070f0) SHA1(27c34bb6463f3841e27fb61afe32fb94c9aedbd0) )
	ROM_LOAD( "25.bin", 0x00000, 0x20000, CRC(adebfda8) SHA1(32193f8553d70b15d77f6bc3f7c84ffeb5a60cc4) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
ROM_END

ROM_START( chleagul )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "12d.bin", 0x00000, 0x10000, CRC(7e143b05) SHA1(943a471fa16fd6c000f601ec8bdb35d70f12c033) )
	ROM_LOAD( "12c.bin", 0x10000, 0x10000, CRC(bd3af488) SHA1(3c5e7a8623d11bd50a1949e870f1044eec7fc463) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "23.bin", 0x40000, 0x20000, CRC(4ac8cc41) SHA1(e4bfd63408511e7d21f140d315493af7fdeba373) )
	ROM_LOAD( "24.bin", 0x20000, 0x20000, CRC(6cb070f0) SHA1(27c34bb6463f3841e27fb61afe32fb94c9aedbd0) )
	ROM_LOAD( "25.bin", 0x00000, 0x20000, CRC(adebfda8) SHA1(32193f8553d70b15d77f6bc3f7c84ffeb5a60cc4) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
ROM_END

/*
  Champion League (v220I, dual-program, set 1).

  This set has two different programs splitted in quarters.
  Both programs are intended to cover playing cards graphics (2nd quarter program),
  or cans (lattine) graphics to avoid some italian laws... (4th quarter program).

  Seems that there's no way to switch between them. They are harcoded through the
  involved PLD's. Addressing lines lower than A12 are driven normally.

  Even when the game has IGS copyright strings inside the program ROM, this set was
  manufactured/released by PlayMark SRL in 1998 to deal with the italian 1995 laws.


  Main program ROM banking through PLDs...

                                              27C2001
                                           .-----v-----.
                                     (VPP)-|01       32|-(VCC)
  PALCE22V10H (u16) pin 21 (IO7) <---(A16)-|02       31|-(/P)
  PALCE22V10H (u16) pin 05 (I4) <----(A15)-|03       30|-(A17)---> A40MX04-PL84 (u20) pin 37 (I/O)
  PALCE22V10H (u16) pin 02 (I1) <----(A12)-|04       29|-(A14)---> PALCE22V10H (u16) pin 04 (I3)
  RAM U6264ADC (u14) pin 03 (A7) <----(A7)-|05       28|-(A13)---> PALCE22V10H (u16) pin 03 (I2)
  RAM U6264ADC (u14) pin 04 (A6) <----(A6)-|06       27|-(A8)----> RAM U6264ADC (u14) pin 25 (A8)
  RAM U6264ADC (u14) pin 05 (A5) <----(A5)-|07       26|-(A9)----> RAM U6264ADC (u14) pin 24 (A9)
  RAM U6264ADC (u14) pin 06 (A4) <----(A4)-|08       25|-(A11)---> RAM U6264ADC (u14) pin 23 (A11)
  RAM U6264ADC (u14) pin 07 (A3) <----(A3)-|09       24|-(/G)
  RAM U6264ADC (u14) pin 08 (A2) <----(A2)-|10       23|-(A10)---> RAM U6264ADC (u14) pin 21 (A10)
  RAM U6264ADC (u14) pin 09 (A1) <----(A1)-|11       22|-(/E)
  RAM U6264ADC (u14) pin 10 (A0) <----(A0)-|12       21|-(Q7)
                                      (Q0)-|13       20|-(Q6)
                                      (Q1)-|14       19|-(Q5)
                                      (Q2)-|15       18|-(Q4)
                                     (VSS)-|16       17|-(Q3)
                                           '-----------'


  Specs...

  CPUs
  1x Z0840006PSC-Z80 CPU (u13) - 8-bit Microprocessor - main.
  1x PIC16C65A-20/P (u1) - 8bit CMOS Microcontroller (internal ROM not dumped).
  1x CP82C55A (u29) - Programmable Peripheral Interface.
  1x YM2413 (u3) - FM Operator Type-M (OPM) - sound.
  1x LM358 (u4) - Dual Operational Amplifier - sound.
  1x TDA2003 (u6) - Audio Amplifier - sound.

  1x 24.000000 MHz oscillator (x2).
  1x 3.579545 MHz oscillator (x1).

  ROMs
  1x 27C020 (u15) - dumped.
  3x M27C1001 (u9, u10, u11) - dumped.

  RAMs
  2x U6264ADC (u7, u14).
  2x HM3-65728BK-5 (u22, u23).

  PLDs
  2x A40MX04-PL84 (u20, u21) - not dumped.
  1x PALCE22V10H (u16) - not dumped.

  Others
  1x 28x2 edge connector.
  1x 6 legs connector (J3).
  1x pushbutton (S5 RESET).
  1x trimmer (volume).
  4x 8 DIP switches banks (DSW1-4).
  1x 4 DIP switches bank (SW5).
  1x 3.6V Battery (BT1).

*/
ROM_START( chleagxa )
	ROM_REGION( 0x40000, "maincpu", 0 )  // Each half contains a different set of data+program. The game needs banking.
	ROM_LOAD( "26.u15", 0x10000, 0x10000, CRC(84bf82db) SHA1(725ca115955cc68bc9a8b70fcf3b15ea47b6ffa2) )  // chleagxa, low combination. Cards GFX.
	ROM_CONTINUE(       0x00000, 0x10000)
	ROM_CONTINUE(       0x30000, 0x10000)  // chleagxa, high combination. Cans GFX.
	ROM_CONTINUE(       0x20000, 0x10000)

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "23.u9",  0x40000, 0x20000, CRC(4ac8cc41) SHA1(e4bfd63408511e7d21f140d315493af7fdeba373) )
	ROM_LOAD( "24.u10", 0x20000, 0x20000, CRC(6cb070f0) SHA1(27c34bb6463f3841e27fb61afe32fb94c9aedbd0) )
	ROM_LOAD( "25.u11", 0x00000, 0x20000, CRC(adebfda8) SHA1(32193f8553d70b15d77f6bc3f7c84ffeb5a60cc4) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
ROM_END


/*
  Champion League (v220I, dual-program, set 2).

  This set has two different programs splitted in quarters.
  Both programs are intended to cover playing cards graphics (2nd quarter program),
  or cans (lattine) graphics to avoid some italian laws... (4th quarter program).

  Similar to the above set. See tech notes there.

  Specs...

  CPUs
  1x Z0840006PSC (Z80 CPU) @5.997 MHz (u13) - 8-bit Microprocessor - main.
  1x PIC16C65A-20/P @5.997 MHz (u1) - 8bit CMOS Microcontroller (internal ROM not dumped).
  1x D71055C (u29) - Parallel Interface Unit.
  1x KA358 (u4) - Dual Operational Amplifier - sound.
  1x TDA2003 (u6) - Audio Amplifier - sound.
  1x YM2413 @3.578 MHz (u3) - FM Operator Type-LL - sound.

  1x 24.000000 MHz oscillator (x2).
  1x 3.579545 MHz oscillator (x1).

  ROMs
  3x M27C1001 (23, 24, 25) - dumped.
  1x M27C2001 (26) - dumped.

  RAMs
  2x HM3-65728BH-5 (u22, u23).
  2x V62C51864L-70P (u7, u14).

  PLDs
  1x PALCE22V10H-25PC/4 (u16) - read protected.
  2x A40MX04-PL84 (u20, u21) - read protected.

  Others
  1x 28x2 JAMMA edge connector.
  1x 6 legs connector(J3).
  1x pushbutton(S5).
  1x trimmer (volume)(PT1).
  4x 8 DIP switches banks (DSW1-4).
  1x 4 DIP switches bank (DSW5).
  1x 3.6V battery.

*/
ROM_START( chleagxb )
	ROM_REGION( 0x40000, "maincpu", 0 )  // Each half contains a different set of data+program.  The game needs banking.
	ROM_LOAD( "26.u15", 0x10000, 0x10000, CRC(e9555257) SHA1(8a20d8faf8520b928f1979239343bd9de9e66e70) )  // chleagxb, low combination. Cards GFX.
	ROM_CONTINUE(       0x00000, 0x10000)
	ROM_CONTINUE(       0x30000, 0x10000)  // chleagxb, high combination. Cans GFX.
	ROM_CONTINUE(       0x20000, 0x10000)

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "23.u9",  0x40000, 0x20000, CRC(2206fbbb) SHA1(c11e5f6fc460045ae93e44fba9662d8ac613581f) )
	ROM_LOAD( "24.u10", 0x20000, 0x20000, CRC(17a97591) SHA1(e314a02bc7f35386394f0ec78303f59f998fcca5) )
	ROM_LOAD( "25.u11", 0x00000, 0x20000, CRC(d0aba992) SHA1(42844a86bd583977c43582995f50a79d89d8687f) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
ROM_END


void igspoker_state::init_number10()
{
	uint8_t *rom = memregion("maincpu")->base();
	int length = memregion("maincpu")->bytes();
	for (int A = 0; A < length; A++)
	{
		if ((A & 0x09C0) == 0x0880) rom[A] ^= 0x20;
		if ((A & 0x0B40) == 0x0140) rom[A] ^= 0x20;
	}

	/* Renable patched out DSW Display in test mode */
	rom[0xA835] = 0xcd;
	rom[0xA836] = 0x3a;
	rom[0xA837] = 0x48;

	rom[0xA863] = 0xcd;
	rom[0xA864] = 0x40;
	rom[0xA865] = 0xd3;

	rom[0xaade] = 0xcd;
	rom[0xaadf] = 0x17;
	rom[0xaae0] = 0xa5;

	/* Fix graphic glitch */
	rom[0x48e8] = 0x19;
	rom[0x48e9] = 0x5e;
	rom[0x48ea] = 0x23;

	/* Patch trap */
	rom[0xeed] = 0xc3;

	/* Descramble graphic */
	rom = memregion("gfx1")->base();
	length = memregion("gfx1")->bytes();
	std::vector<uint8_t> tmp(length);
	memcpy(&tmp[0],rom,length);
	for (int A = 0; A < length; A++)
	{
		int addr = (A & ~0xffff) | bitswap<16>(A,15,14,13,12,11,10,9,8,7,6,5,4,3,0,1,2);
		rom[A] = tmp[addr];
	}
}

ROM_START( number10 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "10b.bin", 0x00000, 0x10000, CRC(149935d1) SHA1(8bb2f6bbe8fc5388e058cfce5c554ee9a5de2a6a) )
	ROM_LOAD( "10a.bin", 0x10000, 0x10000, CRC(73c6335b) SHA1(df2893c9ede5379afdd2ffbc50de90d715240a1f) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "11.bin", 0x00000, 0x20000, CRC(7095cc2a) SHA1(a831f4fc219d0660e1bef65bb6ae6b930795bfea) )
	ROM_LOAD( "12.bin", 0x20000, 0x20000, CRC(9cc00079) SHA1(60df16cbc005c3d249ff9342106c4354f47d9740) )
	ROM_LOAD( "13.bin", 0x40000, 0x20000, CRC(44f86441) SHA1(7fd4af167544bc5113e36647bfe2d2653f77f134) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "9.bin",   0x0000, 0x40000, CRC(dd213b5c) SHA1(82e32aa44eee227d7424553a743df48606bbd48e) )
ROM_END

ROM_START( numbr10l )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "10d.bin", 0x00000, 0x10000, CRC(e1c2b9cc) SHA1(a0943222531b5d0cdc44bd8e1a183107d2e1799d) )
	ROM_LOAD( "10c.bin", 0x10000, 0x10000, CRC(34620db9) SHA1(63bda238f55888d964bad3d70a0dff7d635b7441) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "11.bin", 0x00000, 0x20000, CRC(7095cc2a) SHA1(a831f4fc219d0660e1bef65bb6ae6b930795bfea) )
	ROM_LOAD( "12.bin", 0x20000, 0x20000, CRC(9cc00079) SHA1(60df16cbc005c3d249ff9342106c4354f47d9740) )
	ROM_LOAD( "13.bin", 0x40000, 0x20000, CRC(44f86441) SHA1(7fd4af167544bc5113e36647bfe2d2653f77f134) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "9.bin",   0x0000, 0x40000, CRC(dd213b5c) SHA1(82e32aa44eee227d7424553a743df48606bbd48e) )
ROM_END


void igspoker_state::init_cpokerpk()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0x0714; A < 0xF000; A += 0x1000)
		rom[A] ^= 0x20;
}

ROM_START( cpokerpk )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "blue.bin", 0x00000, 0x20000, CRC(3e987389) SHA1(ab154db89406590d04270d7b29e60efab15758ca) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "red.bin",   0x00000, 0x40000, CRC(b72fe1e0) SHA1(0507df7e1495aa265b276337c9c151478dd9d376) )
	ROM_LOAD16_BYTE( "white.bin", 0x00001, 0x40000, CRC(bdf55fa4) SHA1(487999d22941a0ef2f3874d31527f45d122aadb0) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "yellow.bin",   0x0000, 0x40000, CRC(dd213b5c) SHA1(82e32aa44eee227d7424553a743df48606bbd48e) )
ROM_END

ROM_START( cpokerpkg )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cp.u35", 0x00000, 0x20000, CRC(25e129b9) SHA1(01dc9e09603cef233da28e30194e53ef4cd04475) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "3.u23", 0x00000, 0x40000, CRC(b72fe1e0) SHA1(0507df7e1495aa265b276337c9c151478dd9d376) )
	ROM_LOAD16_BYTE( "2.u25", 0x00001, 0x40000, CRC(bdf55fa4) SHA1(487999d22941a0ef2f3874d31527f45d122aadb0) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "9.bin",   0x0000, 0x40000, CRC(dd213b5c) SHA1(82e32aa44eee227d7424553a743df48606bbd48e) )

	ROM_REGION( 0x2dd, "plds",0 )
	ROM_LOAD( "palce22v10h.u44.bad.dump", 0x000, 0x2dd, BAD_DUMP CRC(5c4e9024) SHA1(e9d1e4df3d79c21f4ce053a84bb7b7a43d650f91) )
ROM_END

ROM_START( citalcup )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic.u35", 0x00000, 0x20000, CRC(f120eb31) SHA1(b87f638d4eebe05323b6952956d44368077f27aa) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u23.bin", 0x00000, 0x40000, CRC(b8d2be66) SHA1(fc8cec6bbf7cd446e3388a7c0171643a8d8f3064) )
	ROM_LOAD16_BYTE( "u25.bin", 0x00001, 0x40000, CRC(b53b8830) SHA1(9854ab83300e7d79c9ab4e154941bfeb607ae8ff) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "9.bin",   0x0000, 0x40000, CRC(dd213b5c) SHA1(82e32aa44eee227d7424553a743df48606bbd48e) )
ROM_END

/*

IGS Tetris. PCB NO-T0039

Chips
1 x CPU not visible
1x 8255
1x IGS 003
1x IGS 002
1x AMT 001
1x YM2413 (sound)
1x oscillator 12.000MHz
1x oscillator 3.579545

*/

ROM_START( igstet341 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tetris_v-341r.u23",  0x00000, 0x10000, CRC(3a9762e6) SHA1(9307bfba4c715075edc4e3b892acf49d08b14266) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "tetris_1.u4",  0x40000, 0x20000, CRC(6bf90dd5) SHA1(280eb3a54cf5e4fbeeee25d87b10900bba360641) )
	ROM_LOAD( "tetris_2.u5",  0x20000, 0x20000, CRC(7079e79e) SHA1(bc44c446e8a7ee9cb75695ca1c1a27f78e4b3e30) )
	ROM_LOAD( "tetris_3.u6",  0x00000, 0x20000, CRC(8159768d) SHA1(b28026afa8206adbc381dfa461eea842354ea5b6) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
ROM_END

ROM_START( igstet342 ) // PCB NO- 0159
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tetris_v-342r.u23",  0x00000, 0x10000, CRC(f0414c5a) SHA1(e28e730de608f80815a4c6bdd846476093c3d846) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "tetris_1.u4",  0x40000, 0x20000, CRC(6bf90dd5) SHA1(280eb3a54cf5e4fbeeee25d87b10900bba360641) )
	ROM_LOAD( "tetris_2.u5",  0x20000, 0x20000, CRC(7079e79e) SHA1(bc44c446e8a7ee9cb75695ca1c1a27f78e4b3e30) )
	ROM_LOAD( "tetris_3.u6",  0x00000, 0x20000, CRC(8159768d) SHA1(b28026afa8206adbc381dfa461eea842354ea5b6) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
ROM_END

void igspoker_state::init_tet341()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		rom[A] ^= 0x01;
		if ((A & 0x0060) == 0x0020) rom[A] ^= 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
	memset(&rom[0xf000], 0, 0x1000);

	/* Patch trap */
	rom[0xbb86] = 0xc3;
}

ROM_START( pktet346 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v-346i.bin",  0x00000, 0x10000, CRC(8015ef13) SHA1(62841daff380d40c14ddb9c1b3fccdbb287e0b0d) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "346i-1.bin",  0x40000, 0x20000, CRC(1f8ae481) SHA1(259808422ae1c89f08deb982387b342a68afad7f) )
	ROM_LOAD( "346i-2.bin",  0x20000, 0x20000, CRC(f198a24f) SHA1(a4bc5936f8729b00dc3c5034ce5689e4d16284bf) )
	ROM_LOAD( "346i-3.bin",  0x00000, 0x20000, CRC(cfc4954d) SHA1(c68edbe0a7ce6a95d978756d2c1c8c5935786bcc) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )

ROM_END

void igspoker_state::init_pktet346()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		rom[A] ^= 0x21;
		if ((A & 0x0008) == 0x0008) rom[A] ^= 0x20;
		if ((A & 0x0098) == 0x0000) rom[A] ^= 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
	memset(&rom[0xf000], 0, 0x1000);

	/* Patch trap */
	rom[0xbb0c] = 0xc3;
}

ROM_START( kungfu ) // IGS PCB N0- 0139
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "kung fu v202n.u23", 0x00000, 0x10000, CRC(53396dd3) SHA1(1bab42394f016f800dbd80603c70defc25380fd7) )
	ROM_LOAD( "kungfu-7.u22",      0x10000, 0x08000, CRC(0568f20b) SHA1(a51a10deee0d581b79d0fee354cedceaa660f55c) ) // 1ST AND 2ND HALF IDENTICAL, otherwise same as the other set
	ROM_IGNORE(                             0x08000 )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "kungfu-4.u4", 0x00000, 0x20000, CRC(df4afedb) SHA1(56ab18c46a199653c284417a8e9edc9f32374318) )
	ROM_LOAD( "kungfu-5.u5", 0x20000, 0x20000, CRC(25c9c98e) SHA1(2d3a399d8d53ee5cb8106d2b35d1ab1778439f81) )
	ROM_LOAD( "kungfu-6.u6", 0x40000, 0x20000, CRC(f1ec5f0d) SHA1(0aa888e13312ed5d98953c81f03a61c6175c7fec) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD( "kungfu-1.u1", 0x00000, 0x4000, CRC(abaada6b) SHA1(a6b910db7451e8ca737f43f32dfc8fc5ecf865f4) )
	ROM_LOAD( "kungfu-2.u2", 0x10000, 0x4000, CRC(927b3060) SHA1(a780ea5aaee04287cc9533c2d258dc18f8426530) )
	ROM_LOAD( "kungfu-3.u3", 0x20000, 0x4000, CRC(bbf78e03) SHA1(06fee093e75e2611d00c076c2e0a681938fa8b74) )
ROM_END

/*

Cherry master looking board

Big chip with no markings at U80 stickered  KUNG FU
                                            V1.0
                                            1992

Board silkscreend on top                    PCB NO.0013-B

.45 27010   stickered   6
.44 27010   stickered   5
.43 27010   stickered   4
.42 27128   stickered   3
.41 27128   stickered   2
.40 27128   stickered   1
.98 27256   stickered   7   couldn't read chip, but board was silkscreened 27c256
.97 27512   stickered   ?   looked like Japanese writing
.38 74s287
.46 18cv8               <--- same checksum as .48
.47 pal16l8a            <--- checksum was 0
.48 18cv8               <--- same checksum as .46

unknown 24 pin chip @ u29
open 24 pin socket @ u54
12 MHz crystal

5 x DSW8
3 x NEC D8255AC

*/

ROM_START( kungfua )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	// u97 contains leftover x86 code at 0-3fff (compiled with Borland Turbo-C).
	// You can rename the rom to kungfu.exe and run it (DOS MZ executable)!
	// The rest is Z80 code, so the CPU at u80 is probably a variant with internal ROM.
	ROM_LOAD( "kungfu-internal.u80", 0x00000, 0x04000, NO_DUMP )
	ROM_LOAD( "kungfu.u97",          0x00000, 0x10000, CRC(5c8e16de) SHA1(4af3795753d6e08f528b861d3a771c782e173556) )
	ROM_LOAD( "kungfu-7.u98",        0x10000, 0x08000, CRC(1d3f0c79) SHA1(0a33798b69fbdc0fb7c47c51f5759e42acd2c608) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "kungfu-4.u43", 0x00000, 0x20000, CRC(df4afedb) SHA1(56ab18c46a199653c284417a8e9edc9f32374318) )
	ROM_LOAD( "kungfu-5.u44", 0x20000, 0x20000, CRC(25c9c98e) SHA1(2d3a399d8d53ee5cb8106d2b35d1ab1778439f81) )
	ROM_LOAD( "kungfu-6.u45", 0x40000, 0x20000, CRC(f1ec5f0d) SHA1(0aa888e13312ed5d98953c81f03a61c6175c7fec) )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD( "kungfu-1.u40", 0x00000, 0x4000, CRC(abaada6b) SHA1(a6b910db7451e8ca737f43f32dfc8fc5ecf865f4) )
	ROM_LOAD( "kungfu-2.u41", 0x10000, 0x4000, CRC(927b3060) SHA1(a780ea5aaee04287cc9533c2d258dc18f8426530) )
	ROM_LOAD( "kungfu-3.u42", 0x20000, 0x4000, CRC(bbf78e03) SHA1(06fee093e75e2611d00c076c2e0a681938fa8b74) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "kungfu.u38", 0x000, 0x100, CRC(2074f729) SHA1(eb9a60dec57a029ae6d3fc53aa7bc78e8ac34392) )
	ROM_LOAD( "kungfu.u46", 0x000, 0xde1, CRC(5d4aacaf) SHA1(733546ce0585c40833e1c34504c33219a2bea0a9) )
	ROM_LOAD( "kungfu.u47", 0x000, 0xaee, CRC(5c7e25b5) SHA1(7d37e4abfe1256bd9cb168e0f02e651118dfb304) )
	ROM_LOAD( "kungfu.u48", 0x000, 0xde1, CRC(5d4aacaf) SHA1(733546ce0585c40833e1c34504c33219a2bea0a9) )
ROM_END

void igspoker_state::init_kungfu()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
	{
		rom[A] ^= 0x01;
		if ((A & 0x0060) == 0x0020) rom[A] ^= 0x20;
		if ((A & 0x0282) == 0x0282) rom[A] ^= 0x01;
		if ((A & 0x0940) == 0x0940) rom[A] ^= 0x02;
	}
}

void igspoker_state::init_kungfua()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int A = 0x4000; A < 0x10000; A++)
	{
		rom[A] = rom[A] ^ 0x01;
	}
	memset( &rom[0xf000], 0, 0x1000);
}

} // Anonymous namespace


GAMEL( 1993?,cpoker,      0,      igspoker, cpoker,   igspoker_state, init_cpoker,      ROT0, "IGS",                  "Champion Poker (v220I)",                       0, layout_igspoker )
GAMEL( 1993?,cpokert,     cpoker, igspoker, cpoker,   igspoker_state, init_cpokert,     ROT0, "IGS (Tuning license)", "Champion Poker (v200G)",                       0, layout_igspoker )
GAMEL( 1993, cpokerx,     cpoker, igspoker, cpokerx,  igspoker_state, init_cpokert,     ROT0, "IGS",                  "Champion Poker (v100)",                        0, layout_igspoker )
GAMEL( 1993, cpoker101,   cpoker, igspoker, cpokerx,  igspoker_state, init_cpoker101,   ROT0, "IGS",                  "Champion Poker (v101)",                        0, layout_igspoker ) // need to fix lamps/layout
GAMEL( 1993, cpoker201f,  cpoker, igspoker, cpoker,   igspoker_state, init_cpoker101,   ROT0, "IGS",                  "Champion Poker (v201F)",                       0, layout_igspoker )
GAMEL( 1993, cpoker210ks, cpoker, igspoker, cpokerx,  igspoker_state, init_cpoker101,   ROT0, "IGS",                  "Champion Poker (v210KS)",                      MACHINE_NOT_WORKING, layout_igspoker ) // need to verify protection handling and inputs/outputs
GAMEL( 1993, cpoker300us, cpoker, igspoker, cpoker,   igspoker_state, init_cpoker300us, ROT0, "IGS",                  "Champion Poker (v300US)",                      MACHINE_NOT_WORKING, layout_igspoker ) // need to verify protection handling and inputs/outputs

GAMEL( 2000, chleague,    0,        igspoker, chleague, igspoker_state, init_chleague,  ROT0, "IGS",                  "Champion League (v220I, Poker)",               0, layout_igspoker )
GAMEL( 2000, chleagul,    chleague, igspoker, chleague, igspoker_state, init_chleague,  ROT0, "IGS",                  "Champion League (v220I, Lattine)",             0, layout_igspoker )
GAMEL( 1998, chleagxa,    chleague, igspoker, chleague, igspoker_state, init_chleague,  ROT0, "PlayMark SRL",         "Champion League (v220I, dual program, set 1)", 0, layout_igspoker )
GAMEL( 1998, chleagxb,    chleague, igspoker, chleague, igspoker_state, init_chleague,  ROT0, "PlayMark SRL",         "Champion League (v220I, dual program, set 2)", 0, layout_igspoker )

GAMEL( 198?, csk227it,    0,        csk227it, csk227,   igspoker_state, init_cska,      ROT0, "IGS",                  "Champion Skill (with Ability)",                0, layout_igspoker ) /* SU 062 */
GAMEL( 198?, csk234it,    csk227it, csk234it, csk234,   igspoker_state, init_cska,      ROT0, "IGS",                  "Champion Skill (Ability, Poker & Symbols)",    0, layout_igspoker ) /* SU 062 */

GAMEL( 2000, number10,    0,        number10, number10, igspoker_state, init_number10,  ROT0, "PlayMark SRL",         "Number Dieci (Poker)",                         0, layout_igspoker )
GAMEL( 2000, numbr10l,    number10, number10, number10, igspoker_state, init_number10,  ROT0, "PlayMark SRL",         "Number Dieci (Lattine)",                       0, layout_igspoker )

GAMEL( 198?, igs_ncs,     0,        igs_ncs,  igs_ncs,  igspoker_state, init_igs_ncs,   ROT0, "IGS",                  "New Champion Skill (v100n)",                   0, layout_igspoker ) /* SU 062 */

GAMEL( 199?, cpokerpk,    0,        cpokerpk, cpokerpk, igspoker_state, init_cpokerpk,  ROT0, "bootleg (SGS)",        "Champion Italian PK (bootleg, blue board)",    0, layout_igspoker )
GAMEL( 199?, cpokerpkg,   cpokerpk, cpokerpk, cpokerpk, igspoker_state, init_cpokerpk,  ROT0, "bootleg (SGS)",        "Champion Italian PK (bootleg, green board)",   0, layout_igspoker )
GAMEL( 199?, citalcup,    cpokerpk, cpokerpk, cpokerpk, igspoker_state, init_cpokerpk,  ROT0, "bootleg (SGS)",        "Champion Italian Cup (bootleg V220IT)",        0, layout_igspoker )

GAMEL( 2000, igs_ncs2,    0,        igs_ncs,  igs_ncs,  igspoker_state, init_igs_ncs2,  ROT0, "IGS",                  "New Champion Skill (v100n 2000)",              MACHINE_IMPERFECT_GRAPHICS, layout_igspoker )

GAMEL( 1998, stellecu,    0,        number10, number10, igspoker_state, empty_init,     ROT0, "Sure",                 "Stelle e Cubi (Italy)",                        MACHINE_NOT_WORKING, layout_igspoker )

GAMEL( 1993?,pktet346,    0,        pktetris, pktet346, igspoker_state, init_pktet346,  ROT0, "IGS",                  "PK Tetris (v346I)",                            0, layout_igspoker )
GAMEL( 199?, igstet341,   pktet346, pktetris, igstet341,igspoker_state, init_tet341,    ROT0, "IGS",                  "Tetris (v341R)",                               0, layout_igspoker )
GAMEL( 199?, igstet342,   pktet346, pktetris, igstet341,igspoker_state, init_tet341,    ROT0, "IGS",                  "Tetris (v342R)",                               0, layout_igspoker )

GAMEL( 199?, kungfu,     0,         igspoker, cpoker,   igspoker_state, init_kungfu,    ROT0, "IGS",                  "Kung Fu (IGS, v202N)",                         MACHINE_NOT_WORKING, layout_igspoker ) // decryption should be good, needs proper address map
GAMEL( 1992, kungfua,    kungfu,    igspoker, cpoker,   igspoker_state, init_kungfua,   ROT0, "IGS",                  "Kung Fu (IGS, v100)",                          MACHINE_NOT_WORKING, layout_igspoker ) // missing internal ROM dump
