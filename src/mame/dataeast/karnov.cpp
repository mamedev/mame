// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Dirk Best
/***************************************************************************

    Karnov (USA version)                   (c) 1987 Data East USA
    Karnov (Japanese version)              (c) 1987 Data East Corporation
    Wonder Planet (Japanese version)       (c) 1987 Data East Corporation
    Chelnov (World version)                (c) 1987 Data East Corporation
    Chelnov (USA version)                  (c) 1988 Data East USA
    Chelnov (Japanese version)             (c) 1987 Data East Corporation

    Main board:  DE-0248-1 (all games)
    Video board: DE-0249-0 (all games)

    Emulation by Bryan McPhail, mish@tendril.co.uk


    NOTE!  Karnov USA & Karnov Japan sets have different gameplay!
      and Chelnov USA & Chelnov Japan sets have different gameplay!

    These games use a 68000 main processor with a 6502, YM2203C and YM3526 for
    sound.  Karnov was a major pain to get going because of the
    'protection' on the main player sprite, probably connected to the Intel
    microcontroller on the board.  The game is very sensitive to the wrong values
    at the input ports...

    There is another Karnov rom set - a bootleg version of the Japanese roms with
    the Data East copyright removed - not supported because the original Japanese
    roms work fine.
    ^^ This should be added (DH, 30/03/11)

    Thanks to Oliver Stabel <stabel@rhein-neckar.netsurf.de> for confirming some
    of the sprite & control information :)

    Cheats:

    Karnov - put 0x30 at 0x60201 to skip a level
    Chelnov - level number at 0x60189 - enter a value at cartoon intro


Stephh's notes (based on the games M68000 code and some tests) :

1) 'karnov' and its clones :

  - DSW1 bit 7 is called "No Die Mode" in the manual. It used to give invulnerability
    to shots (but not to falls), but it has no effect due to the "bra" instruction
    at 0x001334 ('karnov') or 0x00131a ('karnovj').

2) 'wndrplnt'

  - There is code at 0x01c000 which tests DSW2 bit 6 which seems to act as a "Freeze"
    Dip Switch, but this address doesn't seem to be reached. Leftover from another game ?
  - DSW2 bit 7 used to give invulnerability, but it has no effect due to
    the "andi.w  #$7fff, D5" instruction at 0x0011a2.

3) 'chelnov' and its clones :

3a) 'chelnov'

  - DSW2 bit 6 isn't tested in this set.
  - DSW2 bit 7 used to give invulnerability, but it has no effect due to
    the "andi.w  #$3fff, D5" instruction at 0x000ed0.

3b) 'chelnovu'

  - DSW2 bit 6 freezes the game (code at 0x000654), but when you turn
    the Dip Switch back to "Off", it adds credits as if COIN1 was pressed.
    Is that the correct behaviour ?
  - Even if there is a "andi.w  #$ffff, D5" instruction at 0x000ef0,
    DSW2 bit 7 isn't tested in this set.

3c) 'chelnovj'

  - DSW2 bit 6 isn't tested in this set.
  - DSW2 bit 7 used to give invulnerability, but it has no effect due to
    the "andi.w  #$3fff, D5" instruction at 0x000ed8.

*******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "video/bufsprite.h"
#include "deckarn.h"
#include "decrmc3.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {


/*************************************
 *
 *  Type definitions
 *
 *************************************/

class karnov_state : public driver_device
{
public:
	karnov_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_screen(*this, "screen"),
		m_spriteram(*this, "spriteram") ,
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ram(*this, "ram"),
		m_videoram(*this, "videoram"),
		m_pf_data(*this, "pf_data"),
		m_scroll(*this, "scroll") { }

	void chelnovjbl(machine_config &config);
	void karnov(machine_config &config);
	void wndrplnt(machine_config &config);
	void karnovjbl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<mcs51_cpu_device> m_mcu;
	required_device<screen_device> m_screen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<deco_karnovsprites_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<deco_rmc3_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_ram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_pf_data;
	required_shared_ptr<uint16_t> m_scroll;

	// video
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fix_tilemap = nullptr;

	void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void playfield_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vintctl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	DECLARE_VIDEO_START(karnov);
	DECLARE_VIDEO_START(wndrplnt);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void base_sound_map(address_map &map) ATTR_COLD;
	void chelnovjbl_mcu_map(address_map &map) ATTR_COLD;
	void chelnovjbl_mcu_io_map(address_map &map) ATTR_COLD;
	void karnov_map(address_map &map) ATTR_COLD;
	void karnovjbl_map(address_map &map) ATTR_COLD;
	void karnov_sound_map(address_map &map) ATTR_COLD;
	void karnovjbl_sound_map(address_map &map) ATTR_COLD;

	void screen_vblank(int state);
	// protection mcu
	void mcu_coin_irq(int state);
	void mcu_ack_w(uint16_t data);
	uint16_t mcu_r();
	void mcu_w(uint16_t data);
	void mcu_p2_w(uint8_t data);

	// protection mcu (bootleg specific)
	uint8_t mcu_data_l_r();
	void mcu_data_l_w(uint8_t data);
	uint8_t mcu_data_h_r();
	void mcu_data_h_w(uint8_t data);
	void mcubl_p1_w(uint8_t data);

	uint8_t m_mcu_p0 = 0;
	uint8_t m_mcu_p1 = 0;
	uint8_t m_mcu_p2 = 0;
	uint16_t m_mcu_to_maincpu = 0;
	uint16_t m_maincpu_to_mcu = 0;
	bool m_coin_state = false;
	bool m_vint_en = false;
};


/*************************************
 *
 *  Microcontroller emulation
 *
 *************************************/

void karnov_state::mcu_coin_irq(int state)
{
	if (state && !m_coin_state)
		m_mcu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);

	m_coin_state = bool(state);
}

void karnov_state::mcu_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(6, CLEAR_LINE);
}

uint16_t karnov_state::mcu_r()
{
	return m_mcu_to_maincpu;
}

void karnov_state::mcu_w(uint16_t data)
{
	m_maincpu_to_mcu = data;
	m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
}

void karnov_state::mcu_p2_w(uint8_t data)
{
	// 7-------  output latch 9k (d8-d15)
	// -6------  output latch 11k (d0-d7)
	// --5-----  input latch 10k (d8-d15)
	// ---4----  input latch 12k (d0-d7)
	// ----3---  unused
	// -----2--  secirq to maincpu
	// ------1-  secreq ack
	// -------0  cinclr

	if (BIT(m_mcu_p2, 0) == 1 && BIT(data, 0) == 0)
		m_mcu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);

	if (BIT(m_mcu_p2, 1) == 1 && BIT(data, 1) == 0)
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);

	if (BIT(m_mcu_p2, 2) == 1 && BIT(data, 2) == 0)
		m_maincpu->set_input_line(6, ASSERT_LINE);

	if (BIT(m_mcu_p2, 4) == 1 && BIT(data, 4) == 0)
		m_mcu_p0 = m_maincpu_to_mcu >> 0;

	if (BIT(m_mcu_p2, 5) == 1 && BIT(data, 5) == 0)
		m_mcu_p1 = m_maincpu_to_mcu >> 8;

	if (BIT(m_mcu_p2, 6) == 1 && BIT(data, 6) == 0)
		m_mcu_to_maincpu = (m_mcu_to_maincpu & 0xff00) | (m_mcu_p0 << 0);

	if (BIT(m_mcu_p2, 7) == 1 && BIT(data, 7) == 0)
		m_mcu_to_maincpu = (m_mcu_to_maincpu & 0x00ff) | (m_mcu_p1 << 8);

	m_mcu_p2 = data;
}

// i8031 for bootleg emulation

void karnov_state::chelnovjbl_mcu_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void karnov_state::chelnovjbl_mcu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(karnov_state::mcu_data_l_r), FUNC(karnov_state::mcu_data_l_w));
	map(0x01, 0x01).rw(FUNC(karnov_state::mcu_data_h_r), FUNC(karnov_state::mcu_data_h_w));
}

uint8_t karnov_state::mcu_data_l_r()
{
	return m_maincpu_to_mcu >> 0;
}

void karnov_state::mcu_data_l_w(uint8_t data)
{
	m_mcu_to_maincpu = (m_mcu_to_maincpu & 0xff00) | (data << 0);
}

uint8_t karnov_state::mcu_data_h_r()
{
	return m_maincpu_to_mcu >> 8;
}

void karnov_state::mcu_data_h_w(uint8_t data)
{
	m_mcu_to_maincpu = (m_mcu_to_maincpu & 0x00ff) | (data << 8);
}

void karnov_state::mcubl_p1_w(uint8_t data)
{
	if (BIT(m_mcu_p1, 0) == 1 && BIT(data, 0) == 0)
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);

	if (BIT(m_mcu_p1, 1) == 1 && BIT(data, 1) == 0)
		m_maincpu->set_input_line(6, ASSERT_LINE);

	m_mcu_p1 = data;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void karnov_state::karnov_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x060000, 0x063fff).ram().share("ram");
	map(0x080000, 0x080fff).ram().share("spriteram");
	map(0x0a0000, 0x0a07ff).ram().w(FUNC(karnov_state::videoram_w)).share("videoram");
	map(0x0a0800, 0x0a0fff).w(FUNC(karnov_state::videoram_w)); /* Wndrplnt Mirror */
	map(0x0a1000, 0x0a17ff).w(FUNC(karnov_state::playfield_w)).share("pf_data");
	map(0x0a1800, 0x0a1fff).lw16([this](offs_t offset, u16 data, u16 mem_mask)
							{ playfield_w(((offset & 0x1f) << 5) | ((offset & 0x3e0) >> 5), data, mem_mask); }, "pf_col_w");
	map(0x0c0000, 0x0c0001).portr("P1_P2").w(FUNC(karnov_state::mcu_ack_w));
	map(0x0c0002, 0x0c0003).portr("SYSTEM");
	map(0x0c0003, 0x0c0003).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0c0004, 0x0c0005).portr("DSW").w(m_spriteram, FUNC(buffered_spriteram16_device::write));
	map(0x0c0006, 0x0c0007).rw(FUNC(karnov_state::mcu_r), FUNC(karnov_state::mcu_w));
	map(0x0c0008, 0x0c000b).writeonly().share("scroll");
	map(0x0c000c, 0x0c000f).nopr().w(FUNC(karnov_state::vintctl_w));
}

void karnov_state::karnovjbl_map(address_map &map)
{
	karnov_map(map);
	map(0x0c0000, 0x0c0001).portr("P1_P2").nopw();
	map(0x0c0006, 0x0c0007).lr16(NAME([]() { return 0x56a; })).lw16(NAME([this](u16 data) { m_maincpu->set_input_line(6, HOLD_LINE); }));
}

void karnov_state::base_sound_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x0800, 0x0800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x1000, 0x1001).w("ym1", FUNC(ym2203_device::write));
	map(0x8000, 0xffff).rom();
}

void karnov_state::karnov_sound_map(address_map &map)
{
	base_sound_map(map);
	map(0x1800, 0x1801).w("ym2", FUNC(ym3526_device::write));
}

void karnov_state::karnovjbl_sound_map(address_map &map)
{
	base_sound_map(map);
	map(0x1800, 0x1801).w("ym2", FUNC(ym3812_device::write));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( common )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED ) /* Button 4 on karnov schematics */

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED ) /* Button 4 on karnov schematics */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) /* PL1 Button 5 on karnov schematics */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* PL2 Button 5 on karnov schematics */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("COIN")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )    PORT_WRITE_LINE_DEVICE_MEMBER("coin", input_merger_device, in_w<0>)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )    PORT_WRITE_LINE_DEVICE_MEMBER("coin", input_merger_device, in_w<1>)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", input_merger_device, in_w<2>)
INPUT_PORTS_END

/* verified from M68000 code */
static INPUT_PORTS_START( karnov )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )    /* see notes */

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "50 'K'" )
	PORT_DIPSETTING(      0x0800, "70 'K'" )
	PORT_DIPSETTING(      0x0400, "90 'K'" )
	PORT_DIPSETTING(      0x0000, "100 'K'" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Timer Speed" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
INPUT_PORTS_END

static INPUT_PORTS_START( karnovjbl )
	PORT_INCLUDE(karnov)

	// no interrupt on coin input here
	PORT_MODIFY("COIN")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE1)
INPUT_PORTS_END

/* verified from M68000 code */
static INPUT_PORTS_START( wndrplnt )
	PORT_INCLUDE( common )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_COCKTAIL */

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )    /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )    /* see notes */
INPUT_PORTS_END

/* verified from M68000 code */
static INPUT_PORTS_START( chelnov )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")   /* also determines "Bonus Life" settings */
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )         /* bonus life at 30k 60k 100k 150k 250k 100k+ */
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )       /* bonus life at 50k 120k 200k 300k 100k+ */
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )         /* bonus life at 80k 160k 260k 100k+ */
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      /* bonus life at every 100k */
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )    /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )    /* see notes */
INPUT_PORTS_END

/* verified from M68000 code */
static INPUT_PORTS_START( chelnovj )
	PORT_INCLUDE( chelnov )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

/* verified from M68000 code */
static INPUT_PORTS_START( chelnovu )
	PORT_INCLUDE( chelnovj )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x4000, 0x4000, "Freeze" )        PORT_DIPLOCATION("SW2:7") /* see notes */
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chelnovjbl )
	PORT_INCLUDE(chelnovj)

	// no interrupt on coin input here
	PORT_MODIFY("COIN")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE1)
INPUT_PORTS_END


/*************************************
 *
 *  Video emulation
 *
 *************************************/

uint32_t karnov_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const flip = BIT(m_scroll[0], 15);

	m_bg_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_fix_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_spritegen->set_flip_screen(flip);

	m_bg_tilemap->set_scrollx(m_scroll[0]);
	m_bg_tilemap->set_scrolly(m_scroll[1]);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram->buffer(), 0x800);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

TILE_GET_INFO_MEMBER(karnov_state::get_fix_tile_info)
{
	int tile = m_videoram[tile_index];
	tileinfo.set(0, tile & 0xfff, tile >> 14, 0);
}

TILE_GET_INFO_MEMBER(karnov_state::get_bg_tile_info)
{
	int tile = m_pf_data[tile_index];
	tileinfo.set(1, tile & 0x7ff, tile >> 12, 0);
}

void karnov_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_fix_tilemap->mark_tile_dirty(offset);
}

void karnov_state::playfield_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pf_data[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void karnov_state::vintctl_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_vint_en = bool(offset & 1);
	// writing to any position in the range will clear the line
	m_maincpu->set_input_line(7, CLEAR_LINE);
}

VIDEO_START_MEMBER(karnov_state, karnov)
{
	/* Allocate bitmap & tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(karnov_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(karnov_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(karnov_state, wndrplnt)
{
	/* Allocate bitmap & tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(karnov_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(karnov_state::get_fix_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
}

void karnov_state::screen_vblank(int state)
{
	// rising edge
	if (state && m_vint_en)
	{
		m_maincpu->set_input_line(7, ASSERT_LINE);
	}
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout chars =
{
	8,8,
	RGN_FRAC(1,4),
	3,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8 /* every sprite takes 8 consecutive bytes */
};

/* 16x16 tiles, 4 Planes, each plane is 0x10000 bytes */
static const gfx_layout tiles =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),0,RGN_FRAC(1,4),RGN_FRAC(2,4) },
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	16*16
};

static GFXDECODE_START( gfx_karnov )
	GFXDECODE_ENTRY( "char",    0, chars,   0,  4 )  /* colors 0-31 */
	GFXDECODE_ENTRY( "tiles",   0, tiles, 512, 16 )  /* colors 512-767 */
GFXDECODE_END

static GFXDECODE_START( gfx_karnov_spr )
	GFXDECODE_ENTRY( "sprites", 0, tiles, 256, 16 )  /* colors 256-511 */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void karnov_state::machine_start()
{
	save_item(NAME(m_mcu_p0));
	save_item(NAME(m_mcu_p1));
	save_item(NAME(m_mcu_p2));
	save_item(NAME(m_mcu_to_maincpu));
	save_item(NAME(m_maincpu_to_mcu));
	save_item(NAME(m_coin_state));
	save_item(NAME(m_vint_en));
}

void karnov_state::machine_reset()
{
	memset(m_ram, 0, 0x4000 / 2); /* Chelnov likes ram clear on reset.. */

	m_scroll[0] = 0;
	m_scroll[1] = 0;
}


void karnov_state::karnov(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2);    /* 10 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &karnov_state::karnov_map);

	// needs a tight sync with the mcu
	config.set_perfect_quantum(m_maincpu);

	M6502(config, m_audiocpu, 12_MHz_XTAL/8);     /* Accurate */
	m_audiocpu->set_addrmap(AS_PROGRAM, &karnov_state::karnov_sound_map);

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set([this](){ return m_mcu_p0; });
	m_mcu->port_out_cb<0>().set([this](u8 data){ m_mcu_p0 = data; });
	m_mcu->port_in_cb<1>().set([this](){ return m_mcu_p1; });
	m_mcu->port_out_cb<1>().set([this](u8 data){ m_mcu_p1 = data; });
	m_mcu->port_out_cb<2>().set(FUNC(karnov_state::mcu_p2_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	INPUT_MERGER_ANY_LOW(config, "coin").output_handler().set(FUNC(karnov_state::mcu_coin_irq));

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(karnov_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(karnov_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_karnov);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting
	m_palette->set_prom_region("proms");
	m_palette->set_init("palette", FUNC(deco_rmc3_device::palette_init_proms));

	DECO_KARNOVSPRITES(config, m_spritegen, 0, m_palette, gfx_karnov_spr);

	MCFG_VIDEO_START_OVERRIDE(karnov_state,karnov)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL/8)); // 1.5 MHz
	ym1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL/4)); // 3 MHz
	ym2.irq_handler().set_inputline(m_audiocpu, M6502_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void karnov_state::karnovjbl(machine_config &config)
{
	karnov(config);

	/* X-TALs:
	Top board next to #9 is 20.000 MHz
	Top board next to the microcontroller is 6.000 MHz
	Bottom board next to the ribbon cable is 12.000 MHz*/

	m_maincpu->set_addrmap(AS_PROGRAM, &karnov_state::karnovjbl_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &karnov_state::karnovjbl_sound_map);

	// different MCU
	config.device_remove("mcu");
	config.device_remove("coin");

	ym3812_device &ym2(YM3812(config.replace(), "ym2", 3000000));
	ym2.irq_handler().set_inputline(m_audiocpu, M6502_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void karnov_state::wndrplnt(machine_config &config)
{
	karnov(config);

	MCFG_VIDEO_START_OVERRIDE(karnov_state, wndrplnt)
}

void karnov_state::chelnovjbl(machine_config &config)
{
	karnov(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &karnov_state::karnov_map);

	config.device_remove("mcu");
	config.device_remove("coin");

	I8031(config, m_mcu, 8_MHz_XTAL); // info below states 8MHz for MCU
	m_mcu->set_addrmap(AS_PROGRAM, &karnov_state::chelnovjbl_mcu_map);
	m_mcu->set_addrmap(AS_IO, &karnov_state::chelnovjbl_mcu_io_map);
	m_mcu->port_out_cb<1>().set(FUNC(karnov_state::mcubl_p1_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( karnov ) // DE-0248-3 main board, DE-259-0 sub/rom board
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "dn08-6.j15", 0x00000, 0x10000, CRC(4c60837f) SHA1(6886e6ee1d1563c3011b8fea79e7435f983a3ee0) )
	ROM_LOAD16_BYTE( "dn11-6.j20", 0x00001, 0x10000, CRC(cd4abb99) SHA1(b4482175f5d90941ad3aec6c2269a50f57a465ed) )
	ROM_LOAD16_BYTE( "dn07-.j14",  0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "dn10-.j18",  0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "dn06-5.j13", 0x40000, 0x10000, CRC(29d64e42) SHA1(c07ff5f29b7ccd5fc97b5086bcae57ab6eb29330) )
	ROM_LOAD16_BYTE( "dn09-5.j17", 0x40001, 0x10000, CRC(072d7c49) SHA1(92195b89274d066a9c1f87dd810683ea66edaff4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502 Sound CPU
	ROM_LOAD( "dn05-5.f3", 0x8000, 0x8000, CRC(fa1a31a8) SHA1(5007a625be03c546d2a78444d72c28761b10cdb0) )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 MCU (Note: Dump taken from a Rev 5 board)
	ROM_LOAD( "dn-5.k14", 0x0000, 0x1000, CRC(d056de4e) SHA1(621587ed949ff46e5ccb0d0603612655a38b69a3) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dn00-.c5", 0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dn04-.d18", 0x00000, 0x10000, CRC(a9121653) SHA1(04a67ba6fcf551719734ba2b86ee49c37ee1b842) )
	ROM_LOAD( "dn01-.c15", 0x10000, 0x10000, CRC(18697c9e) SHA1(b454af7922c4b1a651d303a3d8d89e5cc102f9ca) )
	ROM_LOAD( "dn03-.d15", 0x20000, 0x10000, CRC(90d9dd9c) SHA1(00a3bed276927f099d57e90f28fd77bd41a3c360) )
	ROM_LOAD( "dn02-.c18", 0x30000, 0x10000, CRC(1e04d7b9) SHA1(a2c6fde42569a52cc6d9a86715dea4a8bea80092) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "dn12-.f8",   0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )  // 2 sets of 4, interleaved here
	ROM_LOAD( "dn14-5.f11", 0x10000, 0x08000, CRC(ac9e6732) SHA1(6f61344eb8a13349471145dee252a01aadb8cdf0) )
	ROM_LOAD( "dn13-.f9",   0x18000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "dn15-5.f12", 0x28000, 0x08000, CRC(8933fcb8) SHA1(0dbda4b032ed3776d7633264f39e6f00ace7a238) )
	ROM_LOAD( "dn16-.f13",  0x30000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "dn17-5.f15", 0x40000, 0x08000, CRC(b70ae950) SHA1(1ec833bdad12710ea846ef48dddbe2e1ae6b8ce1) )
	ROM_LOAD( "dn18-.f16",  0x48000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "dn19-5.f18", 0x58000, 0x08000, CRC(8fd4fa40) SHA1(1870fb0c5c64fbc53a10115f0f3c7624cf2465db) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dn-21.k8", 0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) ) // MB7132E BPROM
	ROM_LOAD( "dn-20.l6", 0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) ) // MB7122E BPROM
ROM_END

ROM_START( karnova ) // DE-0248-3 main board, DE-259-0 sub/rom board
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "dn08-5.j15", 0x00000, 0x10000, CRC(db92c264) SHA1(bd4bcd984a3455eedd2b78dc2090c9d625025671) ) // also known to be labeled DN08-5E
	ROM_LOAD16_BYTE( "dn11-5.j20", 0x00001, 0x10000, CRC(05669b4b) SHA1(c78d0da5afc66750dd9841a7d4f8f244d878c081) ) // also known to be labeled DN11-5E
	ROM_LOAD16_BYTE( "dn07-.j14",  0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "dn10-.j18",  0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "dn06-5.j13", 0x40000, 0x10000, CRC(29d64e42) SHA1(c07ff5f29b7ccd5fc97b5086bcae57ab6eb29330) )
	ROM_LOAD16_BYTE( "dn09-5.j17", 0x40001, 0x10000, CRC(072d7c49) SHA1(92195b89274d066a9c1f87dd810683ea66edaff4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "dn05-5.f3", 0x8000, 0x8000, CRC(fa1a31a8) SHA1(5007a625be03c546d2a78444d72c28761b10cdb0) )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 MCU
	ROM_LOAD( "dn-5.k14", 0x0000, 0x1000, CRC(d056de4e) SHA1(621587ed949ff46e5ccb0d0603612655a38b69a3) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dn00-.c5", 0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dn04-.d18", 0x00000, 0x10000, CRC(a9121653) SHA1(04a67ba6fcf551719734ba2b86ee49c37ee1b842) )
	ROM_LOAD( "dn01-.c15", 0x10000, 0x10000, CRC(18697c9e) SHA1(b454af7922c4b1a651d303a3d8d89e5cc102f9ca) )
	ROM_LOAD( "dn03-.d15", 0x20000, 0x10000, CRC(90d9dd9c) SHA1(00a3bed276927f099d57e90f28fd77bd41a3c360) )
	ROM_LOAD( "dn02-.c18", 0x30000, 0x10000, CRC(1e04d7b9) SHA1(a2c6fde42569a52cc6d9a86715dea4a8bea80092) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "dn12-.f8",   0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )  // 2 sets of 4, interleaved here
	ROM_LOAD( "dn14-5.f11", 0x10000, 0x08000, CRC(ac9e6732) SHA1(6f61344eb8a13349471145dee252a01aadb8cdf0) )
	ROM_LOAD( "dn13-.f9",   0x18000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "dn15-5.f12", 0x28000, 0x08000, CRC(8933fcb8) SHA1(0dbda4b032ed3776d7633264f39e6f00ace7a238) )
	ROM_LOAD( "dn16-.f13",  0x30000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "dn17-5.f15", 0x40000, 0x08000, CRC(b70ae950) SHA1(1ec833bdad12710ea846ef48dddbe2e1ae6b8ce1) )
	ROM_LOAD( "dn18-.f16",  0x48000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "dn19-5.f18", 0x58000, 0x08000, CRC(8fd4fa40) SHA1(1870fb0c5c64fbc53a10115f0f3c7624cf2465db) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dn-21.k8", 0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) ) // MB7132E BPROM
	ROM_LOAD( "dn-20.l6", 0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) ) // MB7122E BPROM
ROM_END

ROM_START( karnovj ) // DE-0248-3 main board, DE-259-0 sub/rom board
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "dn08-.j15",  0x00000, 0x10000, CRC(3e17e268) SHA1(3a63928bb0148175519540f9d891b03590094dfb) )
	ROM_LOAD16_BYTE( "dn11-.j20", 0x00001, 0x10000, CRC(417c936d) SHA1(d31f9291f18c3d5e3c4430768396e1ac10fd9ea3) )
	ROM_LOAD16_BYTE( "dn07-.j14", 0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "dn10-.j18", 0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "dn06-.j13",  0x40000, 0x10000, CRC(c641e195) SHA1(fa7a2eba70e730f72a8d868160af9c41f9b2e5b0) )
	ROM_LOAD16_BYTE( "dn09-.j17",  0x40001, 0x10000, CRC(d420658d) SHA1(4c7e67a80e419b8b94eb015f7f0af0a01f00c28e) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502 Sound CPU
	ROM_LOAD( "dn05-.f3", 0x8000, 0x8000, CRC(7c9158f1) SHA1(dfba7b3abd6b8d6991f0207cd252ee652a6050c2) )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 MCU - seen with DN-3 label on Japanese PCB, "3" is handwritten
	ROM_LOAD( "dn-3.k14", 0x0000, 0x1000, BAD_DUMP CRC(5a8c4d28) SHA1(58cc912d91e569503d5a20fa3180fbdca595e39f) ) // hand-crafted based on US version

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dn00-.c5",        0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "dn04-.d18", 0x00000, 0x10000, CRC(a9121653) SHA1(04a67ba6fcf551719734ba2b86ee49c37ee1b842) )
	ROM_LOAD( "dn01-.c15", 0x10000, 0x10000, CRC(18697c9e) SHA1(b454af7922c4b1a651d303a3d8d89e5cc102f9ca) )
	ROM_LOAD( "dn03-.d15", 0x20000, 0x10000, CRC(90d9dd9c) SHA1(00a3bed276927f099d57e90f28fd77bd41a3c360) )
	ROM_LOAD( "dn02-.c18", 0x30000, 0x10000, CRC(1e04d7b9) SHA1(a2c6fde42569a52cc6d9a86715dea4a8bea80092) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "dn12-.f8",  0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )  // 2 sets of 4, interleaved here
	ROM_LOAD( "kar14.f11", 0x10000, 0x08000, CRC(c6b39595) SHA1(3bc2d0a613cc1b5d255cccc3b26e21ea1c23e75b) )
	ROM_LOAD( "dn13-.f9",  0x18000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "kar15.f12", 0x28000, 0x08000, CRC(2f72cac0) SHA1(a71e61eea77ecd3240c5217ae84e7aa3ef21288a) )
	ROM_LOAD( "dn16-.f13", 0x30000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "kar17.f15", 0x40000, 0x08000, CRC(7851c70f) SHA1(47b7a64dd8230e95cd7ae7f661c7586c7598c356) )
	ROM_LOAD( "dn18-.f16", 0x48000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "kar19.f18", 0x58000, 0x08000, CRC(7bc174bb) SHA1(d8bc320169fc3a9cdd3f271ea523fb0486abae2c) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dn-21.k8", 0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) ) /* MB7132E BPROM */
	ROM_LOAD( "dn-20.l6", 0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) ) /* MB7122E BPROM */
ROM_END

ROM_START( karnovjbl )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "3.bin",        0x00000, 0x10000, CRC(3e17e268) SHA1(3a63928bb0148175519540f9d891b03590094dfb) )
	ROM_LOAD16_BYTE( "6.bin",        0x00001, 0x10000, CRC(509eb27e) SHA1(ff2866791f7e220132224ccb2db1bbee21b6ff2b) )
	ROM_LOAD16_BYTE( "2.bin",        0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "5.bin",        0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "1.bin",        0x40000, 0x08000, CRC(6c66b30c) SHA1(0db0441cb55b88db328237f62f22882d486cb76f) )
	ROM_FILL(0x48000, 0x08000, 0xff)
	ROM_LOAD16_BYTE( "4.bin",        0x40001, 0x08000, CRC(015e74ad) SHA1(62850776e064a22dd14d0a31d091d295c65ebc0d) )
	ROM_FILL(0x48001, 0x08000, 0xff)

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "7.bin",         0x8000, 0x8000, CRC(7c9158f1) SHA1(dfba7b3abd6b8d6991f0207cd252ee652a6050c2) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* NEC D8748HD MCU */
	ROM_LOAD( "mcu.bin", 0x0000, 0x1000, NO_DUMP ) // labeled 19 on PCB (yes, they labeled two chips as 19)

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "8.bin",       0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "11.bin",       0x00000, 0x08000, CRC(fd18a75c) SHA1(21f753bd7062c00afde48a1585c9aeba247b68b8) )
	ROM_LOAD( "12.bin",       0x08000, 0x08000, CRC(a47791d7) SHA1(c242afc80bb885b5d612cc4e7780a8a0ed1f0879) )
	ROM_LOAD( "13.bin",       0x10000, 0x08000, CRC(e8baf220) SHA1(34ad6453757f87defde5d2f1946fda920a10b1f6) )
	ROM_LOAD( "14.bin",       0x18000, 0x08000, CRC(27cecdec) SHA1(529f2f14c88cbefd3b630c9b5fa3bbb6bdb8dabf) )
	ROM_LOAD( "9.bin",        0x20000, 0x08000, CRC(4c403259) SHA1(375feab295be3a540ea3a1d281c2f2cec2d91a21) )
	ROM_LOAD( "10.bin",       0x28000, 0x08000, CRC(14821a07) SHA1(63504313c117dbeb0d3b425fcb4d216078ef7b82) )
	ROM_LOAD( "15.bin",       0x30000, 0x08000, CRC(04551cc8) SHA1(d0b89b55b8e139e11b79efd26edeffd8022ee385) )
	ROM_LOAD( "16.bin",       0x38000, 0x08000, CRC(12bc9e09) SHA1(d2f527138d475fc7d798aaf48e08b133be090543) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "17.bin",       0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )  // 2 sets of 4, interleaved here
	ROM_LOAD( "19.bin",       0x10000, 0x08000, CRC(c6b39595) SHA1(3bc2d0a613cc1b5d255cccc3b26e21ea1c23e75b) )
	ROM_LOAD( "18.bin",       0x18000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "20.bin",       0x28000, 0x08000, CRC(2f72cac0) SHA1(a71e61eea77ecd3240c5217ae84e7aa3ef21288a) )
	ROM_LOAD( "21.bin",       0x30000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "22.bin",       0x40000, 0x08000, CRC(7851c70f) SHA1(47b7a64dd8230e95cd7ae7f661c7586c7598c356) )
	ROM_LOAD( "23.bin",       0x48000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "24.bin",       0x58000, 0x08000, CRC(7bc174bb) SHA1(d8bc320169fc3a9cdd3f271ea523fb0486abae2c) )

	ROM_REGION( 0x0800, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "dn-21.k8", 0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) ) /* MB7132E BPROM */
	ROM_LOAD( "dn-20.l6", 0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) ) /* MB7122E BPROM */
ROM_END

ROM_START( wndrplnt )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ea08.j16",   0x00000, 0x10000, CRC(b0578a14) SHA1(a420d1e8f80405161c86a123610ddf17c7ff07ff) )
	ROM_LOAD16_BYTE( "ea11.j19",   0x00001, 0x10000, CRC(271edc6c) SHA1(6aa411fa4a3613018e7d971c5675f54d5765904d) )
	ROM_LOAD16_BYTE( "ea07.j14",   0x20000, 0x10000, CRC(7095a7d5) SHA1(a7ee88cad03690a72a52b8ea2310416aa53febdd) )
	ROM_LOAD16_BYTE( "ea10.j18",   0x20001, 0x10000, CRC(81a96475) SHA1(2d2e647ed7867b1a7f0dc24544e241e4b1c9fa92) )
	ROM_LOAD16_BYTE( "ea06.j13",   0x40000, 0x10000, CRC(5951add3) SHA1(394552c29a6266becbdb36c3bd65fc1f56701d11) )
	ROM_LOAD16_BYTE( "ea09.j17",   0x40001, 0x10000, CRC(c4b3cb1e) SHA1(006becbcdbbb3e666382e59e8fa5a5ebe06e5724) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ea05.f3",     0x8000, 0x8000, CRC(8dbb6231) SHA1(342faa020448ce916e820b3df18d44191983f7a6) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ea.k14", 0x0000, 0x1000, CRC(b481f6a9) SHA1(e2c4376662fc7b209cd6a2f4e9a85807c8af2548) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ea00.c5",    0x00000, 0x08000, CRC(9f3cac4c) SHA1(af8a275ff531029dbada3c820c9f660fef383100) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "ea04.d18",    0x00000, 0x10000, CRC(7d701344) SHA1(4efaa73a4b2534078ee25111a2f5143c7c7e846f) )
	ROM_LOAD( "ea01.c18",    0x10000, 0x10000, CRC(18df55fb) SHA1(406ea47365ff8372bb2588c97c438ea02aa17538) )
	ROM_LOAD( "ea03.d15",    0x20000, 0x10000, CRC(922ef050) SHA1(e33aea6df2e1a14bd371ed0a2b172f58edcc0e8e) )
	ROM_LOAD( "ea02.c18",    0x30000, 0x10000, CRC(700fde70) SHA1(9b5b59aaffac091622329dc6ebedb24806b69964) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "ea12.f8",     0x00000, 0x10000, CRC(a6d4e99d) SHA1(a85dbb23d05d1e386d8a66f505fa9dfcc554327b) )   // 2 sets of 4, interleaved here
	ROM_LOAD( "ea14.f9",     0x10000, 0x10000, CRC(915ffdc9) SHA1(b65cdc8ee953494f2b69e06cd6c97ee142d83c3e) )
	ROM_LOAD( "ea13.f13",    0x20000, 0x10000, CRC(cd839f3a) SHA1(7eae3a1e080b7db22968d556e80b620cb07976b0) )
	ROM_LOAD( "ea15.f15",    0x30000, 0x10000, CRC(a1f14f16) SHA1(5beb2b8967aa34271f734865704c6bab07d76a8c) )
	ROM_LOAD( "ea16.bin",    0x40000, 0x10000, CRC(7a1d8a9c) SHA1(2b924a7e5a2490a7144b981155f2503d3737875d) )
	ROM_LOAD( "ea17.bin",    0x50000, 0x10000, CRC(21a3223d) SHA1(7754ed9cbe4eed94b49130af6108e919be18e5b3) )
	ROM_LOAD( "ea18.bin",    0x60000, 0x10000, CRC(3fb2cec7) SHA1(7231bb728f1009186d41e177402e84b63f25a44f) )
	ROM_LOAD( "ea19.bin",    0x70000, 0x10000, CRC(87cf03b5) SHA1(29bc25642be1dd7e25f13e96dae90572f7a09d21) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ea-21.k8",      0x0000, 0x0400, CRC(c8beab49) SHA1(970c2bad3cbf2d7fc313997ae0fe11dd04383b40) ) /* MB7132E BPROM */
	ROM_LOAD( "ea-20.l6",      0x0400, 0x0400, CRC(619f9d1e) SHA1(17fe49b6c9ce17be4a03e3400229e3ef4998a46f) ) /* MB7122E BPROM */
ROM_END

ROM_START( chelnov ) /* DE-0248-1 main board, DE-259-0 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ee08-e.j16",   0x00000, 0x10000, CRC(8275cc3a) SHA1(961166226b68744eef15fed6a306010757b83556) )
	ROM_LOAD16_BYTE( "ee11-e.j19",   0x00001, 0x10000, CRC(889e40a0) SHA1(e927f32d9bc448a331fb7b3478b2d07154f5013b) )
	ROM_LOAD16_BYTE( "ee07.j14",     0x20000, 0x10000, CRC(51465486) SHA1(e165e754eb756db3abc1f8477171ab817d03a890) )
	ROM_LOAD16_BYTE( "ee10.j18",     0x20001, 0x10000, CRC(d09dda33) SHA1(1764215606eec61e4fe30c0fc82ea2faf17821dc) )
	ROM_LOAD16_BYTE( "ee06-e.j13",   0x40000, 0x10000, CRC(55acafdb) SHA1(9dc0528c888dd73617f8cab76690b9296715680a) )
	ROM_LOAD16_BYTE( "ee09-e.j17",   0x40001, 0x10000, CRC(303e252c) SHA1(d5d2570e42aa1e1b3600d14cc694677248e12750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ee-e.k14", 0x0000, 0x1000, CRC(b7045395) SHA1(a873de0978cbd169b481ee4c4512e47e7745df77) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ee00-e.c5",    0x00000, 0x08000, CRC(e06e5c6b) SHA1(70166257da5be428cb8404d8e1063c59c7722365) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )
	ROM_LOAD( "ee13-.f9",     0x10000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x20000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x30000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee-17.k8",      0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) ) /* MB7132E BPROM */
	ROM_LOAD( "ee-16.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) ) /* MB7122E BPROM */
ROM_END

ROM_START( chelnovu ) /* DE-0248-1 main board, DE-259-0 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ee08-a.j15",   0x00000, 0x10000, CRC(2f2fb37b) SHA1(f89b424099097a95cf184d20a15b876c5b639552) )
	ROM_LOAD16_BYTE( "ee11-a.j20",   0x00001, 0x10000, CRC(f306d05f) SHA1(e523ffd17fb0104fe28eac288b6ebf7fc0ea2908) )
	ROM_LOAD16_BYTE( "ee07-a.j14",   0x20000, 0x10000, CRC(9c69ed56) SHA1(23606d2fc7c550eaddf0fd4b0da1a4e2c9263e14) )
	ROM_LOAD16_BYTE( "ee10-a.j18",   0x20001, 0x10000, CRC(d5c5fe4b) SHA1(183b2f5dfa4e0a9067674a29abab2744a887fd19) )
	ROM_LOAD16_BYTE( "ee06-e.j13",   0x40000, 0x10000, CRC(55acafdb) SHA1(9dc0528c888dd73617f8cab76690b9296715680a) )
	ROM_LOAD16_BYTE( "ee09-e.j17",   0x40001, 0x10000, CRC(303e252c) SHA1(d5d2570e42aa1e1b3600d14cc694677248e12750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ee-a.k14", 0x0000, 0x1000, CRC(95ea1e7b) SHA1(6d9e3107a2b90734c826c6915c1a3443a7eddfdb) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ee00-e.c5",    0x00000, 0x08000, CRC(e06e5c6b) SHA1(70166257da5be428cb8404d8e1063c59c7722365) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )
	ROM_LOAD( "ee13-.f9",     0x10000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x20000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x30000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee-17.k8",      0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) ) /* MB7132E BPROM */
	ROM_LOAD( "ee-16.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) ) /* MB7122E BPROM */
ROM_END

ROM_START( chelnovj ) /* DE-0248-1 main board, DE-259-0 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ee08-1.j15",  0x00000, 0x10000, CRC(1978cb52) SHA1(833b8e80445ec2384e0479afb7430b32d6a14441) )/* at least 1 PCB found with all labels as 'EPR-EExx' like Sega labels */
	ROM_LOAD16_BYTE( "ee11-1.j20",  0x00001, 0x10000, CRC(e0ed3d99) SHA1(f47aaec5c72ecc308c32cdcf117ef4965ac5ea61) )
	ROM_LOAD16_BYTE( "ee07.j14",    0x20000, 0x10000, CRC(51465486) SHA1(e165e754eb756db3abc1f8477171ab817d03a890) )
	ROM_LOAD16_BYTE( "ee10.j18",    0x20001, 0x10000, CRC(d09dda33) SHA1(1764215606eec61e4fe30c0fc82ea2faf17821dc) )
	ROM_LOAD16_BYTE( "ee06.j13",    0x40000, 0x10000, CRC(cd991507) SHA1(9da858ea41bfbce78496c086e3b462ea9f3722e8) )
	ROM_LOAD16_BYTE( "ee09.j17",    0x40001, 0x10000, CRC(977f601c) SHA1(b40a37160b493dcb614922c2a9b4b5f140b62aca) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ee.k14", 0x0000, 0x1000, CRC(b3dc380c) SHA1(81cc4ded918da9f232481f4e67cf71de814efc48) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ee00.c5",     0x00000, 0x08000, CRC(1abf2c6d) SHA1(86d625ae94cd9ea69e4e613895410640efb175b3) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )
	ROM_LOAD( "ee13-.f9",     0x10000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x20000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x30000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee-17.k8",      0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) ) /* MB7132E BPROM */
	ROM_LOAD( "ee-16.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) ) /* MB7122E BPROM */
ROM_END

// bootleg of chelnovj, only interesting because it uses a SCM8031HCCN40 instead of the usual 8051 for protection
// matching roms had been stripped out so chip labels and locations unknown :-(
ROM_START( chelnovjbl ) // code is the same as the regular chelnovj set
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "13.bin",       0x00000, 0x10000, CRC(1978cb52) SHA1(833b8e80445ec2384e0479afb7430b32d6a14441) )
	ROM_LOAD16_BYTE( "16.bin",       0x00001, 0x10000, CRC(e0ed3d99) SHA1(f47aaec5c72ecc308c32cdcf117ef4965ac5ea61) )
	ROM_LOAD16_BYTE( "12.bin",       0x20000, 0x08000, CRC(dcb65089) SHA1(1f63044073b429f5f750e170036d5d8763972051) ) // same content but without FF filled 2nd half
	ROM_LOAD16_BYTE( "15.bin",       0x20001, 0x08000, CRC(2aed4c90) SHA1(74d2a03872f75c731c2472fc8cd497a17b2d590d) ) // ^^
	ROM_LOAD16_BYTE( "11.bin",       0x40000, 0x10000, CRC(cd991507) SHA1(9da858ea41bfbce78496c086e3b462ea9f3722e8) )
	ROM_LOAD16_BYTE( "14.bin",       0x40001, 0x10000, CRC(977f601c) SHA1(b40a37160b493dcb614922c2a9b4b5f140b62aca) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x2000, "mcu", 0 )    /* SCM8031HCCN40  */ // unique to the bootlegs (rewritten or adjusted to be 8031 compatible)
	ROM_LOAD( "17o.bin", 0x0000, 0x2000, CRC(9af64150) SHA1(0f478d9f79baebd2ad90615c98c6bc2d73c0056a) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "a-c5.bin",     0x00000, 0x08000, CRC(1abf2c6d) SHA1(86d625ae94cd9ea69e4e613895410640efb175b3) )

	ROM_REGION( 0x40000, "tiles", 0 ) // same content split into more roms
	ROM_LOAD( "8.bin",    0x00000, 0x08000, CRC(a78b174a) SHA1(e0d82b600a154b81d7e1a787f0e20eb1a341894f) )
	ROM_LOAD( "9.bin",    0x08000, 0x08000, CRC(97d2c146) SHA1(075bb9afc4f0623cd413883ec2bca574d7ff88d4) )
	ROM_LOAD( "2.bin",    0x10000, 0x08000, CRC(8c45e7de) SHA1(d843b7dcc64ed3a5b8717af172a1f22c4c599480) )
	ROM_LOAD( "3.bin",    0x18000, 0x08000, CRC(504cc95c) SHA1(97e5e9f8cd8ebf5e0c18f27f2988a45c4d3809b3) )
	ROM_LOAD( "6.bin",    0x20000, 0x08000, CRC(8f146815) SHA1(c0330b0ced8d12234d71a9d4cdb8a73f4caa61af) )
	ROM_LOAD( "7.bin",    0x28000, 0x08000, CRC(97bf8061) SHA1(16abb3f65bee2ab93b0adfc1558b5c4ceec726a4) )
	ROM_LOAD( "4.bin",    0x30000, 0x08000, CRC(276a46de) SHA1(5b8932dec0e10be128f5ed41798a8928c0aa506b) )
	ROM_LOAD( "5.bin",    0x38000, 0x08000, CRC(99cee6cd) SHA1(b2cd0a1aef04fd63ad27ac8a61d17a6bb4c8b600) )

	ROM_REGION( 0x40000, "sprites", 0 )
//  ROM_LOAD( "17.bin",       0x00000, 0x10000, CRC(47c857f8) SHA1(59f50365cee266c0e4075c989dc7fde50e43667a) ) // probably bad, 1 byte difference: byte 0x55CC == 0x30 vs 0xF0 in ee12-.f8
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )
	ROM_LOAD( "ee13-.f9",     0x10000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x20000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x30000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee-17.k8",      0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) ) /* MB7132E BPROM */
	ROM_LOAD( "ee-16.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) ) /* MB7122E BPROM */
ROM_END

/*

Main cpu 68000
Sound cpu 6502
Sound ics: ym2203 and ym3812
Other ic: Philips MAB8031AH MCU
OSC: 20 mhz, 12 mhz,8 mhz (for mcu)

*/

// same pcb as above?
// this is a further hacked set of the above, with the copyright messages removed etc. (black screens for several seconds instead)
ROM_START( chelnovjbla )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "cheljb13.bin", 0x00000, 0x10000, CRC(1a099586) SHA1(d37d482190b0c1ad38fab5a0351cbf12ad543221) )
	ROM_LOAD16_BYTE( "cheljb16.bin", 0x00001, 0x10000, CRC(a798e7b2) SHA1(35d610f5f86f09981cd6e4120ed3604d87aceba7) )
	ROM_LOAD16_BYTE( "12.bin",       0x20000, 0x08000, CRC(dcb65089) SHA1(1f63044073b429f5f750e170036d5d8763972051) ) // same content but without FF filled 2nd half
	ROM_LOAD16_BYTE( "15.bin",       0x20001, 0x08000, CRC(2aed4c90) SHA1(74d2a03872f75c731c2472fc8cd497a17b2d590d) ) // ^^
	ROM_LOAD16_BYTE( "11.bin",       0x40000, 0x10000, CRC(cd991507) SHA1(9da858ea41bfbce78496c086e3b462ea9f3722e8) )
	ROM_LOAD16_BYTE( "14.bin",       0x40001, 0x10000, CRC(977f601c) SHA1(b40a37160b493dcb614922c2a9b4b5f140b62aca) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x2000, "mcu", 0 )    /* MAB8031AH */ // unique to the bootlegs (rewritten or adjusted to be 8031 compatible)
	ROM_LOAD( "17o.bin", 0x0000, 0x2000, CRC(9af64150) SHA1(0f478d9f79baebd2ad90615c98c6bc2d73c0056a) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "a-c5.bin",     0x00000, 0x08000, CRC(1abf2c6d) SHA1(86d625ae94cd9ea69e4e613895410640efb175b3) )

	ROM_REGION( 0x40000, "tiles", 0 ) // same content split into more roms
	ROM_LOAD( "8.bin",    0x00000, 0x08000, CRC(a78b174a) SHA1(e0d82b600a154b81d7e1a787f0e20eb1a341894f) )
	ROM_LOAD( "9.bin",    0x08000, 0x08000, CRC(97d2c146) SHA1(075bb9afc4f0623cd413883ec2bca574d7ff88d4) )
	ROM_LOAD( "2.bin",    0x10000, 0x08000, CRC(8c45e7de) SHA1(d843b7dcc64ed3a5b8717af172a1f22c4c599480) )
	ROM_LOAD( "3.bin",    0x18000, 0x08000, CRC(504cc95c) SHA1(97e5e9f8cd8ebf5e0c18f27f2988a45c4d3809b3) )
	ROM_LOAD( "6.bin",    0x20000, 0x08000, CRC(8f146815) SHA1(c0330b0ced8d12234d71a9d4cdb8a73f4caa61af) )
	ROM_LOAD( "7.bin",    0x28000, 0x08000, CRC(97bf8061) SHA1(16abb3f65bee2ab93b0adfc1558b5c4ceec726a4) )
	ROM_LOAD( "4.bin",    0x30000, 0x08000, CRC(276a46de) SHA1(5b8932dec0e10be128f5ed41798a8928c0aa506b) )
	ROM_LOAD( "5.bin",    0x38000, 0x08000, CRC(99cee6cd) SHA1(b2cd0a1aef04fd63ad27ac8a61d17a6bb4c8b600) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )
	ROM_LOAD( "ee13-.f9",     0x10000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x20000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x30000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee-17.k8",     0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) )    /* not dumped here, taken from parent */
	ROM_LOAD( "ee-16.l6",     0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END


} // Anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, karnov,      0,       karnov,     karnov,     karnov_state, empty_init, ROT0,   "Data East USA",               "Karnov (US, rev 6)",                                         MACHINE_SUPPORTS_SAVE )
GAME( 1987, karnova,     karnov,  karnov,     karnov,     karnov_state, empty_init, ROT0,   "Data East USA",               "Karnov (US, rev 5)",                                         MACHINE_SUPPORTS_SAVE )
GAME( 1987, karnovj,     karnov,  karnov,     karnov,     karnov_state, empty_init, ROT0,   "Data East Corporation",       "Karnov (Japan)",                                             MACHINE_SUPPORTS_SAVE )
GAME( 1987, karnovjbl,   karnov,  karnovjbl,  karnovjbl,  karnov_state, empty_init, ROT0,   "bootleg (K. J. Corporation)", "Karnov (Japan, bootleg with NEC D8748HD)",                   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1987, wndrplnt,    0,       wndrplnt,   wndrplnt,   karnov_state, empty_init, ROT270, "Data East Corporation",       "Wonder Planet (Japan)",                                      MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnov,     0,       karnov,     chelnov,    karnov_state, empty_init, ROT0,   "Data East Corporation",       "Chelnov - Atomic Runner (World)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnovu,    chelnov, karnov,     chelnovu,   karnov_state, empty_init, ROT0,   "Data East USA",               "Chelnov - Atomic Runner (US)",                               MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnovj,    chelnov, karnov,     chelnovj,   karnov_state, empty_init, ROT0,   "Data East Corporation",       "Chelnov - Atomic Runner (Japan)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnovjbl,  chelnov, chelnovjbl, chelnovjbl, karnov_state, empty_init, ROT0,   "bootleg",                     "Chelnov - Atomic Runner (Japan, bootleg with I8031, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, chelnovjbla, chelnov, chelnovjbl, chelnovjbl, karnov_state, empty_init, ROT0,   "bootleg",                     "Chelnov - Atomic Runner (Japan, bootleg with I8031, set 2)", MACHINE_SUPPORTS_SAVE )
