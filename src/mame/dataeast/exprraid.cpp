// license:BSD-3-Clause
// copyright-holders: Ernesto Corvi

/***************************************************************************

Express Raider - (c) 1986 Data East Corporation / Data East USA

Ernesto Corvi
ernesto@imagina.com

Memory Map:
Main CPU: ( DECO CPU-16 )
0000-05ff RAM
0600-07ff Sprites
0800-0bff Videoram
0c00-0fff Colorram
1800-1800 DSW 0
1801-1801 Controls
1802-1802 Coins
1803-1803 DSW 1
2100-2100 Sound latch write
2800-2801 Protection
3800-3800 VBlank ( bootleg 2 only )
4000-ffff ROM
ffc0-ffc0 VBlank ( bootleg 3 only )

Sound CPU: ( 6809 )
0000-1fff RAM
2000-2001 YM2203
4000-4001 YM3526
6000-6000 Sound latch read
8000-ffff ROM


NOTES:

The main 6502 CPU is a DECO CPU-16.  Also, there was some protection
circuitry which is now emulated.

The background tiles had a very ugly encoding.  It was so ugly that our
decode gfx routine will not be able to decode it without a little help.
So thats why gfx_expand() is there.  Many thanks to Phil Stroffolino,
who figured out the encoding.

If the second player obtains a high score, the game always reads the 2P
inputs for entering initials, even on upright cabinets that lack these
inputs, making it impossible for the second player to enter their
initials.  Data East released an update for the EEPROM at location 16A
on the top PCB (CZ00-6A) to fix this bug.  They also suggested wiring
the 1P controls to the 2P inputs to work around the issue.  The updated
EEPROM has not been dumped.


NOTES ON THE BOOTLEGS:

The bootleg version patched the ROM to get rid of the extra opcode
(bootlegs used a regular 6502), the vectors hard-coded in place, and
also had the protection cracked.

1st bootleg set expects to read vblank status from 0x3800, country
warning sign has been defaced by the bootleggers

2nd bootleg set expects to read vblank status from 0xFFC0, country
warning sign is intact, however Credit is spelt incorrectly.


Stephh's notes (based on the games M6502 code and some tests) :

1) 'exprrada'

  - "@ 1986 DATA EAST CORPORATION" + no code to display the Warning screen (World)
  - Same way to code number of enemies in "shoot" stages as in 'exprraidu'
    (code at 5ce4) and same ingame bug :

      5CF4: AD 03 18      lda  $1803
      5CF7: 49 FF         eor  #$FF
      5CF9: 4A            lsr  a
      5CFA: 4A            lsr  a
      5CFB: 4A            lsr  a
      5CFC: 29 06         and  #$06

    Correct code shall be :

      5CFC: 29 03         and  #$03

    You'll notice by looking at the tables that there are sometimes
    more enemies than in 'exprraidu'.
  - Time for each wagon on "shoot" stage is determined by the level
    (see code at 0x6834 where location 0x0e is level number-1).
    This time is also supposed to be determined by "Difficulty"
    settings (DSW1 bits 3 and 4).
    There is however an ingame bug that reads DSW1 bits 4 and 5 :

      683B: AD 03 18      lda  $1803
      683E: 49 FF         eor  #$FF
      6840: 4A            lsr  a
      6841: 4A            lsr  a
      6842: 4A            lsr  a
      6843: 4A            lsr  a
      6844: 29 03         and  #$03

    So Time is also determined by "Demo Sound" setting because of
    extra "lsr a" instruction at 0x6843 !
    Correct code shall be :

      6843: EA            nop

    Fortunately, table at 0x685f is filled with 0x30 so you'll
    always have 30 seconds to "clear" the wagon (which is more
    than the time you have in 'exprraid').
    For the locomotive, time is always set to 0x20 = 20 seconds
    (which is again more than the time you have in 'exprraid').
  - "Bonus lives" routine starts at 0xe49b.
  - Coinage related stuff starts at 0xe78e.
    Coinage tables :
      * 0xe7dc : COIN1 - 0xe7e4 : COIN2 (Mode 1)
      * 0xe7ec : COIN1 - 0xe7f4 : COIN2 (Mode 2)
  - At the beginning of each level, you have text in lower case
    which doesn't give you any hints to pass the level nor advice.
  - In this version, you always have 5 wagons for the "shoot" stages.
  - Continue play is always available but score is reset to 0.

2) 'exprraidu'

  - "@ 1986 DATA EAST USA, INC." (US)
  - Number of enemies on "shoot" stages is determined by the level
    (see code at 0x5d21 where location 0x0e is level number-1).
    Note that time tables are coded backwards (locomotive first,
    then 5th wagon, then 4th wagon ... up to 1st wagon).
    This number of enemies is also supposed to be determined
    by "Difficulty" settings (DSW1 bits 3 and 4).
    There is however an ingame bug that reads DSW1 bits 4 and 5 :

      5D2F: AD 03 18      lda  $1803
      5D32: 49 FF         eor  #$FF
      5D34: 4A            lsr  a
      5D35: 4A            lsr  a
      5D36: 4A            lsr  a
      5D37: 29 06         and  #$06

    So number of enemies is also determined by "Demo Sound" setting !
    Correct code shall be :

      5D37: 29 03         and  #$03

  - Time for each wagon on "shoot" stage is determined by the level
    (see code at 0x6873 where location 0x0e is level number-1).
    Note that time tables are coded backwards (locomotive first,
    then 5th wagon, then 4th wagon ... up to 1st wagon).
  - In the US manual, "bonus lives" settings are told be either
    "Every 50000" or "50000/80000".
    However, when you look at code at 0xe4a1, you'll notice that
    settings shall be "50000 only" and "50000/80000".
  - "Coin Mode" as well "Mode 2 Coinage" settings (DSW0 bits 0 to 4)
    are undocumented in the US manual.
    "Coin Mode" is tested though via code at 0xe7c5.
    Coinage tables :
      * 0xe7e2 : COIN1 - 0xe7ea : COIN2 (Mode 1)
      * 0xe7f2 : COIN1 - 0xe7fa : COIN2 (Mode 2)
  - "Force Coinage" (DSW1 bit 6) setting is undocumented in the US manual.
    It is tested though via code at 0xe794.
    When this Dip Switch is set to "On", pressing COIN1 or COIN2 always
    adds 1 credit regardless of the "Coinage" and "Coin Mode" settings.
  - At the beginning of each level, you have text in upper case
    which gives you some hints to pass the level or some advice.
  - In this version, due to extra code at 0xfd80, you only have 4 wagons
    for the "shoot" stages instead of 5.
  - Continue play is always available and score is NOT reset to 0.

3) 'wexpress'

  - "@ 1986 DATA EAST CORPORATION" + extra code to display the Warning screen (Japan)
  - This version is heavily based on 'exprrad'
    so all comments also fit for this set. The main difference is
    The other difference is that you can NOT continue a game.
  - "Bonus lives" routine starts at 0xe4e5.
  - Coinage related stuff starts at 0xe7d8.
  - Coinage tables :
      * 0xe826 : COIN1 - 0xe82e : COIN2 (Mode 1)
      * 0xe836 : COIN1 - 0xe83e : COIN2 (Mode 2)

4) 'wexpressb1'

  - "@ 1986 DATA EAST CORPORATION" + no code to display the Warning screen (World)
  - This version is based on 'exprrad' so all comments also fit
    for this set. The main difference is that reads from 0x2800
    and 0x2801 (protection) are either discarded (jumps are noped
    or patched) or changed to read what shall be the correct value
    (reads from 0x2801 occur almost all the time).
    So IMO this set looks like a World bootleg .

5) 'wexpressb2'

  - "@ 1986 DATA EAST CORPORATION" + extra code to display the Warning screen (Japan)
  - Modified Warning screen
  - This version is based on 'wexpress'
    so all comments also fit for this set. The main difference is
    the way protection is bypassed (in a different way than 'wexpressb1'
    as reads from 0x2801 only occur when a life is lost).

6) 'wexpressb3'

  - "@ 1986 DATA EAST CORPORATION" + extra code to display the Warning screen (Japan)
  - Original Warning screen
  - "CREDIT" misspelled to "CRDDIT".
  - This version is based on 'wexpress'
    so all comments also fit for this set. The main difference is
    the way protection is bypassed (in a different way than 'wexpressb1'
    but also in a different way than 'wexpressb2' as reads from 0x2801
    occur when you lose a life but also on "shoot" stages).

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/deco16.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class exprraid_state : public driver_device
{
public:
	exprraid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_tilemaprom(*this, "bgtilemap"),
		m_main_ram(*this, "main_ram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram")
	{ }

	void exprraid(machine_config &config) ATTR_COLD;
	void wexpressb2(machine_config &config) ATTR_COLD;
	void wexpressb3(machine_config &config) ATTR_COLD;

	void gfx_expand() ATTR_COLD;
	void init_wexpressb() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_deco16);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_region_ptr<uint8_t> m_tilemaprom;
	required_shared_ptr<uint8_t> m_main_ram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	// protection
	uint8_t m_prot_value = 0U;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_bg_index[4]{};

	void int_clear_w(uint8_t data);
	uint8_t prot_status_r();
	uint8_t prot_data_r();
	void prot_data_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);
	void bgselect_w(offs_t offset, uint8_t data);
	void scrollx_w(offs_t offset, uint8_t data);
	void scrolly_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void master_map(address_map &map) ATTR_COLD;
	template <offs_t Addr> void wexpressb_map(address_map &map) ATTR_COLD;
	void master_io_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
};


void exprraid_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void exprraid_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void exprraid_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 0x01);
}

void exprraid_state::bgselect_w(offs_t offset, uint8_t data)
{
	if (m_bg_index[offset] != data)
	{
		m_bg_index[offset] = data;
		m_bg_tilemap->mark_all_dirty();
	}
}

void exprraid_state::scrollx_w(offs_t offset, uint8_t data)
{
	m_bg_tilemap->set_scrollx(offset, data);
}

void exprraid_state::scrolly_w(offs_t offset, uint8_t data)
{
	m_bg_tilemap->set_scrolly(0, data);
}

TILE_GET_INFO_MEMBER(exprraid_state::get_bg_tile_info)
{
	int const sx = tile_index % 32;
	int const sy = tile_index / 32;

	int quadrant = 0;
	if (sx >= 16) quadrant++;
	if (sy >= 16) quadrant += 2;

	int const offs = (sy % 16) * 16 + (sx % 16) + (m_bg_index[quadrant] & 0x3f) * 0x100;

	int const data = m_tilemaprom[offs];
	int const attr = m_tilemaprom[offs + 0x4000];
	int const bank = (2 * (attr & 0x03) + ((data & 0x80) >> 7)) + 2;
	int const code = data & 0x7f;
	int const color = (attr & 0x18) >> 3;
	int const flags = (attr & 0x04) ? TILE_FLIPX : 0;

	tileinfo.category = ((attr & 0x80) ? 1 : 0);

	tileinfo.set(bank, code, color, flags);
}

TILE_GET_INFO_MEMBER(exprraid_state::get_fg_tile_info)
{
	int const attr = m_colorram[tile_index];
	int const code = m_videoram[tile_index] + ((attr & 0x07) << 8);
	int const color = (attr & 0x10) >> 4;

	tileinfo.set(0, code, color, 0);
}

void exprraid_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int const attr = m_spriteram[offs + 1];
		int const code = m_spriteram[offs + 3] + ((attr & 0xe0) << 3);
		int const color = (attr & 0x03) + ((attr & 0x08) >> 1);
		int flipx = (attr & 0x04);
		int flipy = 0;
		int sx = ((248 - m_spriteram[offs + 2]) & 0xff) - 8;
		int sy = m_spriteram[offs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(
				bitmap, cliprect,
				code, color,
				flipx, flipy,
				sx, sy, 0);

		// double height
		if (attr & 0x10)
		{
			m_gfxdecode->gfx(1)->transpen(
					bitmap, cliprect,
					code + 1, color,
					flipx, flipy,
					sx, sy + (flip_screen() ? -16 : 16), 0);
		}
	}
}

uint32_t exprraid_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*****************************************************************************************/
// Emulate DECO 291 protection (for original express raider, code is cracked on the bootleg)
/*****************************************************************************************/

uint8_t exprraid_state::prot_data_r()
{
	return m_prot_value;
}

uint8_t exprraid_state::prot_status_r()
{
	/*
	    76543210
	    .......x    ?
	    ......x.    Device data available
	    .....x..    CPU data available (cleared by device)
	*/

	return 0x02;
}

void exprraid_state::prot_data_w(uint8_t data)
{
	switch (data)
	{
		case 0x20:
			// Written when CPU times out waiting for status
			break;

		case 0x60:
			// ?
			break;

		case 0x80:
			++m_prot_value;
			break;

		case 0x90:
			m_prot_value = 0;
			break;

		default:
			logerror("Unknown protection write: %x at %s\n", data, machine().describe_context());
	}
}

void exprraid_state::int_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(DECO16_IRQ_LINE, CLEAR_LINE);
}


void exprraid_state::master_map(address_map &map)
{
	map(0x0000, 0x05ff).ram().share("main_ram");
	map(0x0600, 0x07ff).ram().share("spriteram");
	map(0x0800, 0x0bff).ram().w(FUNC(exprraid_state::videoram_w)).share("videoram");
	map(0x0c00, 0x0fff).ram().w(FUNC(exprraid_state::colorram_w)).share("colorram");
	map(0x1800, 0x1800).portr("DSW0");
	map(0x1801, 0x1801).portr("IN1");    // 1P controls, start buttons
	map(0x1802, 0x1802).portr("IN2");    // 2P controls, coins
	map(0x1803, 0x1803).portr("DSW1");
	map(0x2000, 0x2000).w(FUNC(exprraid_state::int_clear_w));
	map(0x2001, 0x2001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x2002, 0x2002).w(FUNC(exprraid_state::flipscreen_w));
	map(0x2003, 0x2003).nopw(); // DMA SWAP - Allow writes to video and sprite RAM
	map(0x2800, 0x2800).r(FUNC(exprraid_state::prot_data_r));
	map(0x2801, 0x2801).r(FUNC(exprraid_state::prot_status_r));
	map(0x2800, 0x2803).w(FUNC(exprraid_state::bgselect_w));
	map(0x2804, 0x2804).w(FUNC(exprraid_state::scrolly_w));
	map(0x2805, 0x2806).w(FUNC(exprraid_state::scrollx_w));
	map(0x2807, 0x2807).w(FUNC(exprraid_state::prot_data_w));
	map(0x4000, 0xffff).rom();
}

template <offs_t Addr>
void exprraid_state::wexpressb_map(address_map &map)
{
	master_map(map);

	map(Addr, Addr).portr("IN0");
}

void exprraid_state::master_io_map(address_map &map)
{
	map(0x01, 0x01).portr("IN0");
}

void exprraid_state::slave_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x4000, 0x4001).rw("ym2", FUNC(ym3526_device::read), FUNC(ym3526_device::write));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom();
}


INPUT_CHANGED_MEMBER(exprraid_state::coin_inserted_deco16)
{
	if (oldval && !newval)
		m_maincpu->set_input_line(DECO16_IRQ_LINE, ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(exprraid_state::coin_inserted_nmi)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, oldval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( exprraid )
	PORT_START("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x00)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x10)
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW0", 0x10, EQUALS, 0x10)
	PORT_DIPNAME( 0x10, 0x10, "Coin Mode" )                 PORT_DIPLOCATION("SW1:5")     // see notes
	PORT_DIPSETTING(    0x10, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_deco16, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_deco16, 0)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3")     // see notes
	PORT_DIPSETTING(    0x00, "50k 80k" )
	PORT_DIPSETTING(    0x04, "50k only" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,5")   // see notes
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:6")     // see notes
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_deco16, 0)
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( exprboot )
	PORT_INCLUDE( exprraid )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_nmi, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exprraid_state, coin_inserted_nmi, 0)
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	1024,   // 1024 characters
	2,  // 2 bits per pixel
	{ 0, 4 },   // the bitplanes are packed in the same byte
	{ (0x2000*8)+0, (0x2000*8)+1, (0x2000*8)+2, (0x2000*8)+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	2048,   // 2048 sprites
	3,  // 3 bits per pixel
	{ 2*2048*32*8, 2048*32*8, 0 },  // the bitplanes are separated
	{ 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every char takes 32 consecutive bytes
};

static const gfx_layout tile1 =
{
	16,16,  // 16*16 tiles
	128,    // 128 tiles
	3,  // 3 bits per pixel
	{ 4, 0x10000*8+0, 0x10000*8+4 },
	{ 0, 1, 2, 3, 1024*32*2,1024*32*2+1,1024*32*2+2,1024*32*2+3,
		128+0,128+1,128+2,128+3,128+1024*32*2,128+1024*32*2+1,128+1024*32*2+2,128+1024*32*2+3 }, // BOGUS
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		64+0*8,64+1*8,64+2*8,64+3*8,64+4*8,64+5*8,64+6*8,64+7*8 },
	32*8
};

static const gfx_layout tile2 =
{
	16,16,  // 16*16 tiles
	128,    // 128 tiles
	3,  // 3 bits per pixel
	{ 0, 0x11000*8+0, 0x11000*8+4  },
	{ 0, 1, 2, 3, 1024*32*2,1024*32*2+1,1024*32*2+2,1024*32*2+3,
		128+0,128+1,128+2,128+3,128+1024*32*2,128+1024*32*2+1,128+1024*32*2+2,128+1024*32*2+3 }, // BOGUS
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		64+0*8,64+1*8,64+2*8,64+3*8,64+4*8,64+5*8,64+6*8,64+7*8 },
	32*8
};


static GFXDECODE_START( gfx_exprraid )
	GFXDECODE_ENTRY( "chars",   0x00000, charlayout,   128, 2 )
	GFXDECODE_ENTRY( "sprites", 0x00000, spritelayout,  64, 8 )
	GFXDECODE_ENTRY( "bgtiles", 0x00000, tile1,          0, 4 )
	GFXDECODE_ENTRY( "bgtiles", 0x00000, tile2,          0, 4 )
	GFXDECODE_ENTRY( "bgtiles", 0x04000, tile1,          0, 4 )
	GFXDECODE_ENTRY( "bgtiles", 0x04000, tile2,          0, 4 )
	GFXDECODE_ENTRY( "bgtiles", 0x08000, tile1,          0, 4 )
	GFXDECODE_ENTRY( "bgtiles", 0x08000, tile2,          0, 4 )
	GFXDECODE_ENTRY( "bgtiles", 0x0c000, tile1,          0, 4 )
	GFXDECODE_ENTRY( "bgtiles", 0x0c000, tile2,          0, 4 )
GFXDECODE_END


void exprraid_state::machine_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exprraid_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exprraid_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(2);
	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_prot_value));
	save_item(NAME(m_bg_index));
}

void exprraid_state::machine_reset()
{
	m_bg_index[0] = 0;
	m_bg_index[1] = 0;
	m_bg_index[2] = 0;
	m_bg_index[3] = 0;
}


void exprraid_state::exprraid(machine_config &config)
{
	// basic machine hardware
	DECO16(config, m_maincpu, XTAL(12'000'000) / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &exprraid_state::master_map);
	m_maincpu->set_addrmap(AS_IO, &exprraid_state::master_io_map);

	MC6809(config, m_slave, XTAL(12'000'000) / 2); // MC68B09P
	m_slave->set_addrmap(AS_PROGRAM, &exprraid_state::slave_map);
	// IRQs are caused by the YM3526

	config.set_maximum_quantum(attotime::from_hz(12000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(12'000'000) / 2, 384, 0, 256, 262, 8, 256-8); // not accurate
	screen.set_screen_update(FUNC(exprraid_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_exprraid);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_slave, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(12'000'000) / 8));
	ym1.add_route(ALL_OUTPUTS, "mono", 0.30);

	ym3526_device &ym2(YM3526(config, "ym2", XTAL(12'000'000) / 4));
	ym2.irq_handler().set_inputline(m_slave, M6809_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.60);
}

void exprraid_state::wexpressb2(machine_config &config)
{
	exprraid(config);

	M6502(config.replace(), m_maincpu, 1'500'000);      // 1.5 MHz ???
	m_maincpu->set_addrmap(AS_PROGRAM, &exprraid_state::wexpressb_map<0x3800>);
}

void exprraid_state::wexpressb3(machine_config &config)
{
	exprraid(config);

	M6502(config.replace(), m_maincpu, 1'500'000);      // 1.5 MHz ???
	m_maincpu->set_addrmap(AS_PROGRAM, &exprraid_state::wexpressb_map<0xffc0>);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( exprraid )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz01-2e.16b", 0x4000, 0x4000, CRC(a0ae6756) SHA1(7f7ec1efddbb62e9d201c6013bca8ab72c3f75f6) )
	ROM_LOAD( "cz00-4e.15a", 0x8000, 0x8000, CRC(910f6ccc) SHA1(1dbf164a7add9335d90ee07b6db9a162a28e407b) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "cz02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "cz04.8e", 0x00000, 0x8000, CRC(643a1bd3) SHA1(b23631d96cb413808f65f3ebe8fe6539b6140606) )
	// Save 0x08000-0x0ffff to expand the previous so we can decode the thing
	ROM_LOAD( "cz05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )
	ROM_LOAD( "cz06.8h", 0x18000, 0x8000, CRC(b9bb448b) SHA1(84974b1f3a5b58cd427d874f805a6dd9244c1101) )

	ROM_REGION( 0x8000, "bgtilemap", 0 )
	ROM_LOAD( "cz03.12f", 0x0000, 0x8000, CRC(6ce11971) SHA1(16bfa69b3ad02253e81c8110c9b840be03952790) )

	ROM_REGION( 0x0400, "proms", 0 ) // All 4 PROMs are Fujitsu MB7114 or compatible
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) // red
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) // green
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) // blue
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) // priority

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c", 0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e", 0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( exprraidu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz01-5a.16b", 0x4000, 0x4000, CRC(dc8f9fba) SHA1(cae6af54fc0081d606b6884e8873aed356a37ba9) )
	ROM_LOAD( "cz00-5.15a",  0x8000, 0x8000, CRC(a81290bc) SHA1(ddb0acda6124427bee691f9926c41fda27ed816e) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "cz02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "cz04.8e", 0x00000, 0x8000, CRC(643a1bd3) SHA1(b23631d96cb413808f65f3ebe8fe6539b6140606) )
	// Save 0x08000-0x0ffff to expand the previous so we can decode the thing
	ROM_LOAD( "cz05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )
	ROM_LOAD( "cz06.8h", 0x18000, 0x8000, CRC(b9bb448b) SHA1(84974b1f3a5b58cd427d874f805a6dd9244c1101) )

	ROM_REGION( 0x8000, "bgtilemap", 0 )
	ROM_LOAD( "cz03.12f", 0x0000, 0x8000, CRC(6ce11971) SHA1(16bfa69b3ad02253e81c8110c9b840be03952790) )

	ROM_REGION( 0x0400, "proms", 0 ) // All 4 PROMs are Fujitsu MB7114 or compatible
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) // red
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) // green
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) // blue
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) // priority

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c", 0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e", 0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( exprraidi ) // PCB manufactured in Italy by Gecas under Data East license (custom ICs are all DECO original)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cz01-2e.16b",  0x4000, 0x4000, CRC(a0ae6756) SHA1(7f7ec1efddbb62e9d201c6013bca8ab72c3f75f6) )
	ROM_LOAD( "exraidi6.15a", 0x8000, 0x8000, CRC(a3d98118) SHA1(d35f0fcabef045afcec5119f95ae6da2cae547db) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "cz02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "cz04.8e", 0x00000, 0x8000, CRC(643a1bd3) SHA1(b23631d96cb413808f65f3ebe8fe6539b6140606) )
	// Save 0x08000-0x0ffff to expand the previous so we can decode the thing
	ROM_LOAD( "cz05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )
	ROM_LOAD( "cz06.8h", 0x18000, 0x8000, CRC(b9bb448b) SHA1(84974b1f3a5b58cd427d874f805a6dd9244c1101) )

	ROM_REGION( 0x8000, "bgtilemap", 0 )
	ROM_LOAD( "cz03.12f", 0x0000, 0x8000, CRC(6ce11971) SHA1(16bfa69b3ad02253e81c8110c9b840be03952790) )

	ROM_REGION( 0x0400, "proms", 0 ) // All 4 PROMs are Fujitsu MB7114 or compatible
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) // red
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) // green
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) // blue
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) // priority

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c", 0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e", 0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( wexpress )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cy01-2.16b", 0x4000, 0x4000, CRC(a0ae6756) SHA1(7f7ec1efddbb62e9d201c6013bca8ab72c3f75f6) )
	ROM_LOAD( "cy00-4.15a", 0x8000, 0x8000, CRC(c66d4dd3) SHA1(3c354e7379b3c3e709039ee2f3dbad7edddfc517) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "cy02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "cy04.8e", 0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )
	// Save 0x08000-0x0ffff to expand the previous so we can decode the thing
	ROM_LOAD( "cy05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )
	ROM_LOAD( "cy06.8h", 0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )

	ROM_REGION( 0x8000, "bgtilemap", 0 )
	ROM_LOAD( "cy03.12f", 0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, "proms", 0 ) // All 4 PROMs are Fujitsu MB7114 or compatible
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) // red
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) // green
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) // blue
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) // priority

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c",   0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e",   0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( wexpressb1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.16b", 0x4000, 0x4000, CRC(ea5e5a8f) SHA1(fa92bcb6b97c2966cd330b309eba73f9c059f14e) )
	ROM_LOAD( "1.15a", 0x8000, 0x8000, CRC(a7daae12) SHA1(a97f4bc05a3ec096d8c717bdf096f4b0e59dc2c2) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "cy02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "cy04.8e", 0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )
	// Save 0x08000-0x0ffff to expand the previous so we can decode the thing
	ROM_LOAD( "cy05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )
	ROM_LOAD( "cy06.8h", 0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )

	ROM_REGION( 0x8000, "bgtilemap", 0 )
	ROM_LOAD( "cy03.12f", 0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, "proms", 0 ) // All 4 PROMs are Fujitsu MB7114 or compatible
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) // red
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) // green
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) // blue
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) // priority

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16r4a.5c",   0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e",   0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( wexpressb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wexpress.3", 0x4000, 0x4000, CRC(b4dd0fa4) SHA1(8d17eb28ae92486c67859871ea2bef8f50f39dbd) )
	ROM_LOAD( "wexpress.1", 0x8000, 0x8000, CRC(e8466596) SHA1(dbbd3b84d0f017292595fc19f7412b984851221a) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "cy02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "cy04.8e", 0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )
	// Save 0x08000-0x0ffff to expand the previous so we can decode the thing
	ROM_LOAD( "cy05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )
	ROM_LOAD( "cy06.8h", 0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )

	ROM_REGION( 0x8000, "bgtilemap", 0 )
	ROM_LOAD( "cy03.12f", 0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, "proms", 0 ) // All 4 PROMs are Fujitsu MB7114 or compatible
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) // red
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) // green
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) // blue
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) // priority
ROM_END

ROM_START( wexpressb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s2.16b", 0x4000, 0x4000, CRC(40d70fcb) SHA1(1327d39f872a39e020972952e5756ca59c55f9d0) )
	ROM_LOAD( "s1.15a", 0x8000, 0x8000, CRC(7c573824) SHA1(f5e4d4f0866c08c88d012a77e8aa2e74a779f986) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "cy02-1.2a", 0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "cz07.5b", 0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "cz09.16h", 0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )
	ROM_LOAD( "cz08.14h", 0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k", 0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k", 0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k", 0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k", 0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "cy04.8e", 0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )
	// Save 0x08000-0x0ffff to expand the previous so we can decode the thing
	ROM_LOAD( "cy05.8f", 0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )
	ROM_LOAD( "cy06.8h", 0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )

	ROM_REGION( 0x8000, "bgtilemap", 0 )
	ROM_LOAD( "3.12f", 0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, "proms", 0 ) // PROMs weren't present in this set, using the one from the other
	ROM_LOAD( "cy-17.5b", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) // red
	ROM_LOAD( "cy-16.6b", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) // green
	ROM_LOAD( "cy-15.7b", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) // blue
	ROM_LOAD( "cy-14.9b", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) // priority
ROM_END


void exprraid_state::gfx_expand()
{
	// Expand the background ROM so we can use regular decode routines
	uint8_t *const gfx = memregion("bgtiles")->base();
	int offs = 0x10000 - 0x1000;

	for (int i = 0x8000 - 0x1000; i >= 0; i-= 0x1000)
	{
		memcpy(&gfx[offs], &gfx[i], 0x1000);
		offs -= 0x1000;

		memcpy(&gfx[offs], &gfx[i], 0x1000);
		offs -= 0x1000;
	}
}

void exprraid_state::init_wexpressb()
{
	uint8_t *rom = memregion("maincpu")->base();

	// HACK: this set uses M6502 IRQ vectors but DECO CPU-16 opcodes???
	rom[0xfff7] = rom[0xfffa];
	rom[0xfff6] = rom[0xfffb];

	rom[0xfff1] = rom[0xfffc];
	rom[0xfff0] = rom[0xfffd];

	rom[0xfff3] = rom[0xfffe];
	rom[0xfff2] = rom[0xffff];

	gfx_expand();
}

} // anonymous namespace


//    year  name        parent    machine     input     class           init            rot   company                  description                        flags
GAME( 1986, exprraid,   0,        exprraid,   exprraid, exprraid_state, gfx_expand,     ROT0, "Data East Corporation", "Express Raider (World, Rev 4)",   MACHINE_SUPPORTS_SAVE )
GAME( 1986, exprraidu,  exprraid, exprraid,   exprraid, exprraid_state, gfx_expand,     ROT0, "Data East USA",         "Express Raider (US, rev 5)",      MACHINE_SUPPORTS_SAVE )
GAME( 1986, exprraidi,  exprraid, exprraid,   exprraid, exprraid_state, gfx_expand,     ROT0, "Data East Corporation", "Express Raider (Italy)",          MACHINE_SUPPORTS_SAVE )
GAME( 1986, wexpress,   exprraid, exprraid,   exprraid, exprraid_state, gfx_expand,     ROT0, "Data East Corporation", "Western Express (Japan, rev 4)",  MACHINE_SUPPORTS_SAVE )
GAME( 1986, wexpressb1, exprraid, exprraid,   exprraid, exprraid_state, init_wexpressb, ROT0, "bootleg",               "Western Express (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wexpressb2, exprraid, wexpressb2, exprboot, exprraid_state, gfx_expand,     ROT0, "bootleg",               "Western Express (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wexpressb3, exprraid, wexpressb3, exprboot, exprraid_state, gfx_expand,     ROT0, "bootleg",               "Western Express (bootleg set 3)", MACHINE_SUPPORTS_SAVE )
