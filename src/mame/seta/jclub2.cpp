// license:BSD-3-Clause
// copyright-holders:Luca Elia
/************************************************************************************************************

  Seta's Jockey Club II (ジョッキークラブ II)

  Various hardware revisions, all use a MC68EC020FG16 CPU.

  ---------------------------------------------------------------------------------------------
  Year + Game                      PCB                   Video       Sound      Other
  ---------------------------------------------------------------------------------------------
  1994 Jockey Club II (older hw)   E06-00409/E06-00407*  ST-0020     ST-0016    ST-0013 ST-0017
  1996 Jockey Club II (newer hw)   E79-001 rev 01a       ST-0032     <--        ST-0013 ST-0017
  2001 Dark Horse (bootleg hw)     -                     3 x FPGA    M6295      -
  ---------------------------------------------------------------------------------------------
  * Spotted also on E48-0027 Terminal P.C.B.

  ST-0013: unknown function. Doesn't seem to be used anywhere else?
  ST-0016: Z80 with integrated video + sound capabilities. Video functionality is probably not used here.
  ST-0017: unknown function (maybe I/O or RLE). Also used in srmp6 (see srmp6.cpp).
  ST-0020: zooming sprites + blitter + tilemaps chip. Also used by gdfs in ssv.cpp (see st0020.cpp)
  ST-0032: similar to ST-0020 but the ram / list formats aren't the same. Maybe it handles sound as
           there doesn't seem to be any other dedicated sound chip. Doesn't seem to be used anywhere else?

  Notes:
  - Reset, Meter, Last Game and Config (in later games) are revolving keys, so once you turn them they stay "active".
  - Boot with Test switch on (F2) for service mode. Note that Config (if present) has to be off for this to work.
  - The EEPROM holds an ID at $14 e.g. SETA1997JC26203W:

      6 = config version, 203 = program version, W = fixed wide monitor (N = fixed normal, ' ' = configurable, E = Extended config)

    The ID can be reprogrammed by booting with a key combination (see below) that enables Write ID mode.
    Later games require to enter a valid Vender Code (up to 8 hex digits) which is saved (encoded as 8 bytes)
    at address $2a in the EEPROM (shown in service mode after the version) with a matching configuration id at $28 (2 bytes).
    The code affects the game settings (accounting for e.g. laws of the target market) and appearence (e.g. hiding the game title)
    Vender Codes and configuration ids for e.g. jclub2v203 are at 209B6C. Enter the digits using these inputs -> outputs:

      P1 start + 1..8 -> 1..8 | 7-8 -> A | P1 start + 7-8 -> F | P1 start + other keys -> 9 | any key -> F | payout -> end

  - Administrator Setup allows the setting of 3 rows of 8 characters (0-9 A-Z !). The meaning is unknown. Edit them with:

      6-7 -> left | 7-8 -> right |  start -> down | 1 -> increment | 1-2 -> decrement | payout -> end

  - The program can be upgraded by placing a ROM in the socket next to the main program ROM.
  - An undumped v4.00 exists, with copyright 1994-97 Seta Corporation on the E84-0001 Rev.B hardware.

  jclub2v100/jclub2v101 (these can generate a valid EEPROM without a vender code):
  - Boot with Reset + P1 Start               :  Configuration Setup             (operator options)
  - Boot with Reset             + P1 Cancel  :  Backup Data Clear               (needed to clear hardware/emergency/ID errors)
  - Boot with Test  + P1 Start               :  Write To ID Code                (writes ID to EEPROM, configurable monitor)
  - Boot with Test  + P1 Payout + P1 Cancel  :  Write To ID Code(Wide)          (writes ID to EEPROM, fixed wide monitor)

  jclub2v110 (from this version onwards a vender code is needed to generate a valid EEPROM):
  - All of the above plus...
  - Boot with Test  + P1 Start  + P1 Cancel  :  Write To ID Code(A)             (writes ID to EEPROM, configurable monitor, no game title)
  - Boot with invalid ID in the EEPROM i.e.  :  Now Update ID Enter Vender Code (update ID in EEPROM from a previous version)
    EEPROM does not contain SETA at $14 and JC2X at $1c, where X is from the current version:

      ' '(v100)  |  A(v101)  |  2/3(v110)  |  4/5(v112)  |  6(v203)  |  Z(v200-220)

    Bug: this will be triggered if the EEPROM is blank or from a newer version, but the update will fail even with a correct vender code!
         Simply use the key combination for Write ID (thus writing the ID from scratch).

  jclub2v112:
  - Vender codes are associated with a set of features encoded by up to 3 letters (e.g. AEW) shown after writing the ID:
      1) A     / empty   =   no game title (Anonymous?) / game title
      2) E     / empty   =   more configuration options (Extended?) if the monitor is not fixed (the last letter of the ID becomes E)
      3) W / N / empty   =   Fixed Wide / Fixed Normal / Configurable monitor (goes in the last letter of the ID, unless it's E)
  - Boot with Reset + P1 Start               :  Configuration Setup             (operator options)
  - Boot with Reset             + P1 Cancel  :  Backup Data Clear               (needed to clear hardware/emergency/ID errors)
  - Boot with Test  + P1 Start  + P1 Cancel  :  Write ID Enter Write Code       (writes ID to EEPROM, features depend on vender code)
  - They fixed the Update ID bug: it will only show if the EEPROM can be updated. Otherwise Broken ID will be shown (equivalent to Write ID).

  jclub2v200 and above:
  - Vender codes are associated with an hex number instead of the 3 letters above:
      $00 (on some sets produces Failed Write Sequence) .. $10, $20x (special with some settings shown but not changeable)
  - Boot with Test  + P1 Payout + P1 Cancel  :  Administrator Setup             (release payout quickly! Or it will end with a failure)

  jclub2v203 and above:
  - Boot with Reset + P1 Start               :  Configuration Setup             (press Test for operator options)
  - Boot with Reset + Test      + P1 Cancel  :  Backup All Clear                (needed to clear hardware/emergency/ID errors)

  darkhors (allegedly based on v400):
  - Sprite ram is parsed and converted from the original format (ST-0032 zooming sprites + tilemaps)
    to this hardware's plain sprites + 2 tilemaps.
  - Boot with Config                          :  System Setup (has printer and rtc setup)
  - Boot with Reset  + Test      + P1 Cancel  :  Backup All Clear
  - Boot with Config + P1 Payout + P1 Cancel  :  Administrator Setup (release payout quickly, or it will end with a failure)
  - Boot with Config + P1 Start  + P1 Cancel  :  Write ID Enter Write Code

  To do:
  - Emulate coin double sensor. At the moment you will need to enter the config menu and change to single sensor
  - Visible area under non-wide setting is $160 pixels wide instead of $150
  - Extend palette to 0x20000 entries (palette_device::allocate_palette() prevents more than 0x10000)
  - Correct clocks
  - darkhors: fix the disalignment between sprites and tilemap (gap in the fence) during play. Other screens are fine

************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68020.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "st0016.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/setapcm.h"
#include "sound/okim6295.h"
#include "st0020.h"
#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"

#include "jclub2o.lh"
#include "jclub2.lh"

namespace {

// Common between all hardware (jclub2o, jclub2 and darkhors)
class common_state : public driver_device
{
public:
	common_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_hopper1(*this, "hopper1"), m_hopper2(*this, "hopper2"),
		m_palette(*this, "palette"),
		m_key1(*this, "KEY1.%u", 0), m_key2(*this, "KEY2.%u", 0),
		m_input_sel1(0), m_input_sel2(0),
		m_out1(0), m_out2(0), m_out3(0)
	{ }

	uint8_t read_key(required_ioport_array<8> & key, uint8_t mask);

	uint8_t console_r();
	void console_w(uint8_t data);
	uint8_t console_status_r();

	void eeprom_93c46_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_irq);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ticket_dispenser_device> m_hopper1, m_hopper2;
	required_device<palette_device> m_palette;
	required_ioport_array<8> m_key1, m_key2;

	uint8_t m_input_sel1, m_input_sel2;

	uint32_t m_out1, m_out2, m_out3;

	void debug_out();
};


// Newer jclub2 hardware (without ST-0016)
class jclub2_state : public common_state
{
public:
	jclub2_state(const machine_config &mconfig, device_type type, const char *tag) :
		common_state(mconfig, type, tag),
		m_st0020(*this, "st0020")
	{ }

	void input_sel1_out3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void input_sel2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t p1_r();
	uint32_t p2_r();
	void out1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void out2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void jclub2(machine_config &config);
	void jclub2_map(address_map &map) ATTR_COLD;
protected:
	required_device<st0020_device> m_st0020;
};

// Older jclub2 hardware (with ST-0016)
class jclub2o_state : public jclub2_state
{
public:
	jclub2o_state(const machine_config &mconfig, device_type type, const char *tag) :
		jclub2_state(mconfig, type, tag),
		m_soundcpu(*this, "soundcpu"),
		m_soundbank(*this, "soundbank")
	{ }

	required_device<st0016_cpu_device> m_soundcpu;
	required_memory_bank m_soundbank;

	void eeprom_s29290_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// ST-0016 <-> 68EC020
	uint8_t cmd1_r();
	uint8_t cmd2_r();
	void cmd1_w(uint8_t data);
	void cmd2_w(uint8_t data);
	uint8_t cmd_stat_r();
	void st0016_rom_bank_w(uint8_t data); // temp?

	// 68EC020 <-> ST-0016
	uint32_t cmd1_word_r();
	uint32_t cmd2_word_r();
	void cmd1_word_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void cmd2_word_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t cmd_stat_word_r();

	void init_jclub2o();

	void jclub2o(machine_config &config);
	void jclub2o_map(address_map &map) ATTR_COLD;
	void st0016_io(address_map &map) ATTR_COLD;
	void st0016_mem(address_map &map) ATTR_COLD;
private:
	uint8_t m_cmd1;
	uint8_t m_cmd2;
	uint8_t m_cmd_stat;
};


// bootleg darkhors hardware
class darkhors_state : public common_state
{
public:
	darkhors_state(const machine_config &mconfig, device_type type, const char *tag) :
		common_state(mconfig, type, tag),
		m_tmapram(*this,     "tmapram"),
		m_tmapscroll(*this,  "tmapscroll"),
		m_tmapram2(*this,    "tmapram2"),
		m_tmapscroll2(*this, "tmapscroll2"),
		m_spriteram(*this,   "spriteram"),
		m_gfxdecode(*this,   "gfxdecode")
	{ }

	void init_darkhors();

	void darkhors(machine_config &config);
	void darkhorsa(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void input_sel_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t input_r();
	void out1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void tmapram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void tmapram2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void darkhors_map(address_map &map) ATTR_COLD;
	void darkhorsa_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint32_t> m_tmapram;
	required_shared_ptr<uint32_t> m_tmapscroll;
	required_shared_ptr<uint32_t> m_tmapram2;
	required_shared_ptr<uint32_t> m_tmapscroll2;
	required_shared_ptr<uint32_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;

	tilemap_t *m_tmap = nullptr;
	tilemap_t *m_tmap2 = nullptr;
};


/***************************************************************************

                                Common Functions

***************************************************************************/

uint8_t common_state::console_r()
{
	return 0;
}

void common_state::console_w(uint8_t data)
{
}

uint8_t common_state::console_status_r()
{
	// bit 0: 1 = can send to   debug console
	// bi1 1: 1 = can read from debug console
	return 0x3;
}

uint8_t common_state::read_key(required_ioport_array<8> & key, uint8_t mask)
{
	switch(mask)
	{
		case 0x01:  return key[0]->read();
		case 0x02:  return key[1]->read();
		case 0x04:  return key[2]->read();
		case 0x08:  return key[3]->read();
		case 0x10:  return key[4]->read();
		case 0x20:  return key[5]->read();
		case 0x40:  return key[6]->read();
		case 0x80:  return key[7]->read();
	}
	return 0xff;
}

void common_state::debug_out()
{
//  popmessage("OUT: %04X | %04X | %02X--", m_out1 >> 16, m_out2 >> 16, m_out3 >> 24);
}

/***************************************************************************

                                Video Hardware
                                   (jclub2)

***************************************************************************/

uint32_t jclub2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_st0020->update_screen(screen, bitmap, cliprect, true);

	return 0;
}

/***************************************************************************

                                Video Hardware
                                  (darkhors)

***************************************************************************/

TILE_GET_INFO_MEMBER(darkhors_state::get_tile_info_0)
{
	uint16_t tile     =   m_tmapram[tile_index] >> 16;
	uint16_t color    =   m_tmapram[tile_index] & 0xffff;
	tileinfo.set(0, tile/2, (color & 0x200) ? (color & 0x1ff) : ((color & 0x0ff) * 4) , 0);
}

TILE_GET_INFO_MEMBER(darkhors_state::get_tile_info_1)
{
	uint16_t tile     =   m_tmapram2[tile_index] >> 16;
	uint16_t color    =   m_tmapram2[tile_index] & 0xffff;
	tileinfo.set(0, tile/2, (color & 0x200) ? (color & 0x1ff) : ((color & 0x0ff) * 4) , 0);
}

void darkhors_state::tmapram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tmapram[offset]);
	m_tmap->mark_tile_dirty(offset);
}
void darkhors_state::tmapram2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tmapram2[offset]);
	m_tmap2->mark_tile_dirty(offset);
}

void darkhors_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint32_t *s       =   m_spriteram;
	uint32_t *end     =   m_spriteram + 0x02000/4;

	for ( ; s < end; s += 8/4 )
	{
		int sx      =   (s[ 0 ] >> 16);
		int sy      =   (s[ 0 ] & 0xffff);
		int attr    =   (s[ 1 ] >> 16);
		int code    =   (s[ 1 ] & 0xffff);

		// List end
		if (sx & 0x8000)
			break;

		int flipx   =   0;
		int flipy   =   0;
		int color   =   (attr & 0x0200) ? (attr & 0x1ff) : (attr & 0x1ff) * 4;

		// Sign extend the position
		sx  =   (sx & 0x1ff) - (sx & 0x200);
		sy  =   (sy & 0x1ff) - (sy & 0x200);

		sy  =   -sy;
		sy  +=  0xf8;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			code/2, color,
			flipx,  flipy,  sx, sy, 0);
	}
}

void darkhors_state::video_start()
{
	common_state::video_start();

	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darkhors_state::get_tile_info_0)), TILEMAP_SCAN_ROWS, 16,16, 0x40,0x40);
	m_tmap2= &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darkhors_state::get_tile_info_1)), TILEMAP_SCAN_ROWS, 16,16, 0x40,0x40);
	m_tmap->set_transparent_pen(0);
	m_tmap2->set_transparent_pen(0);

	m_gfxdecode->gfx(0)->set_granularity(64); // 256 colour sprites with palette selectable on 64 colour boundaries
}

uint32_t darkhors_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 2;
		if (machine().input().code_pressed(KEYCODE_A))  mask |= 4;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_tmap->set_scrollx(0, (m_tmapscroll[0] >> 16) - 5);
	m_tmap->set_scrolly(0, (m_tmapscroll[0] & 0xffff) - 0xff );
	if (layers_ctrl & 1)    m_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	m_tmap2->set_scrollx(0, (m_tmapscroll2[0] >> 16) - 5);
	m_tmap2->set_scrolly(0, (m_tmapscroll2[0] & 0xffff) - 0xff );
	if (layers_ctrl & 2)    m_tmap2->draw(screen, bitmap, cliprect, 0, 0);

	if (layers_ctrl & 4)    draw_sprites(bitmap,cliprect);

#ifdef MAME_DEBUG
#if 0
	popmessage("%04X-%04X %04X-%04X %04X-%04X %04X-%04X %04X-%04X %04X-%04X",
		m_tmapscroll[0] >> 16, m_tmapscroll[0] & 0xffff,
		m_tmapscroll[1] >> 16, m_tmapscroll[1] & 0xffff,
		m_tmapscroll[2] >> 16, m_tmapscroll[2] & 0xffff,
		m_tmapscroll[3] >> 16, m_tmapscroll[3] & 0xffff,
		m_tmapscroll[4] >> 16, m_tmapscroll[4] & 0xffff,
		m_tmapscroll[5] >> 16, m_tmapscroll[5] & 0xffff
	);
#endif
#endif

	return 0;
}

/***************************************************************************

                                Memory Map

***************************************************************************/

// I/O (both older and newer jclub2 hardware)

void jclub2_state::input_sel2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// sometimes 0x80000000 bit is set
	// (it is cleared before reading from 4d0000.b / 4d0004.b / 4d0008.b / 4d000c.b)
	if (ACCESSING_BITS_16_23)
		m_input_sel2 = data >> 16;
}

void jclub2_state::input_sel1_out3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_out3);

	if (ACCESSING_BITS_16_23)
		m_input_sel1 = data >> 16;
	if (ACCESSING_BITS_24_31)
	{
		// 0x0800 P2 divider coil
		// -
		debug_out();
	}
}

uint32_t jclub2_state::p1_r()
{
	uint32_t ret = ioport("P1LOW")->read() & 0x00ffffff;
	return ret | (read_key(m_key1, m_input_sel1) << 24);
}

uint32_t jclub2_state::p2_r()
{
	uint32_t ret = ioport("P2LOW")->read() & 0x00ffffff;
	return ret | (read_key(m_key2, m_input_sel2) << 24);
}

void jclub2_state::out1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_out1);
	if (ACCESSING_BITS_16_31)
	{
		// 0x1000 P2 hopper rotate
		// -
		// 0x0200 P1 divider coil
		// 0x0100 P2 lockout coil
		// 0x0080 P1 lockout coil
		// -
		// 0x0020 counter meter 6
		// 0x0010 counter meter 1
		// 0x0008 counter meter 4
		// 0x0004 counter meter 3
		// 0x0002 counter meter 5
		// 0x0001 counter meter 2

		m_hopper2->motor_w(                         data  & 0x10000000);
		machine().bookkeeping().coin_lockout_w(1, (~data) & 0x01000000);
		machine().bookkeeping().coin_lockout_w(0, (~data) & 0x00800000);

		debug_out();
	}
}

void jclub2_state::out2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_out2);
	if (ACCESSING_BITS_16_31)
	{
		// 0x8000 P1 hopper rotate

		m_hopper1->motor_w(data & 0x80000000);

		debug_out();
	}
}

// Older hardware (ST-0020 + ST-0016) only

void jclub2o_state::eeprom_s29290_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (data & ~0xff000000)
		logerror("%s: Unknown EEPROM bit written %08X\n", machine().describe_context(), data);

	if (ACCESSING_BITS_24_31)
	{
		// latch the bit
		m_eeprom->di_write((data & 0x01000000) >> 24);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x08000000) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x04000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}

// 68EC020 <-> ST-0016

uint32_t jclub2o_state::cmd1_word_r()
{
	m_cmd_stat &= ~0x02;
	return m_cmd1 << 24;
}
uint32_t jclub2o_state::cmd2_word_r()
{
	m_cmd_stat &= ~0x08;
	return m_cmd2 << 24;
}

uint32_t jclub2o_state::cmd_stat_word_r()
{
	return m_cmd_stat << 24;
}

void jclub2o_state::cmd1_word_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		m_cmd_stat |= 0x01;
		m_cmd1 = data >> 24;
		logerror("%s: cmd1_w %02x\n", machine().describe_context(), m_cmd1);
	}
}
void jclub2o_state::cmd2_word_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		m_cmd_stat |= 0x04;
		m_cmd2 = data >> 24;
		logerror("%s: cmd2_w %02x\n", machine().describe_context(), m_cmd2);
	}
}

void jclub2o_state::jclub2o_map(address_map &map)
{
	map(0x000000, 0x27ffff).rom();
	map(0x400000, 0x41ffff).ram().share("nvram"); // battery

	map(0x490000, 0x490003).w(FUNC(jclub2o_state::eeprom_s29290_w));

	map(0x4a0000, 0x4a0003).w(FUNC(jclub2o_state::out2_w));
//  map(0x4a0010, 0x4a0013).w(FUNC(jclub2o_map::));
//  map(0x4a0020, 0x4a0023).w(FUNC(jclub2o_map::));
//  map(0x4a0030, 0x4a0033).w(FUNC(jclub2o_map::));

	// ST-0016
	map(0x4b0000, 0x4b0003).rw(FUNC(jclub2o_state::cmd1_word_r), FUNC(jclub2o_state::cmd1_word_w));
	map(0x4b0004, 0x4b0007).rw(FUNC(jclub2o_state::cmd2_word_r), FUNC(jclub2o_state::cmd2_word_w));
	map(0x4b0008, 0x4b000b).r(FUNC(jclub2o_state::cmd_stat_word_r));

	map(0x4d0000, 0x4d0003).nopr().nopw(); // reads seem unused? this write would go to two 7-segs (but the code is never called)
	map(0x4d0004, 0x4d0007).nopr();
	map(0x4d0008, 0x4d000b).nopr();
	map(0x4d000c, 0x4d000f).nopr();

	map(0x4e0000, 0x4e0003).r(FUNC(jclub2o_state::p2_r)).w(FUNC(jclub2o_state::input_sel2_w));

	map(0x580000, 0x580003).portr("EEPROM");
	map(0x580004, 0x580007).r(FUNC(jclub2o_state::p1_r));
	map(0x580008, 0x58000b).portr("COIN");
	map(0x58000c, 0x58000f).w(FUNC(jclub2o_state::input_sel1_out3_w));
	map(0x580010, 0x580013).w(FUNC(jclub2o_state::out1_w));
//  map(0x580018, 0x58001b).w(FUNC(jclub2o_map::));
//  map(0x58001c, 0x58001f).w(FUNC(jclub2o_map::));

	map(0x580200, 0x580201).r("watchdog", FUNC(watchdog_timer_device::reset16_r));

	map(0x580401, 0x580401).rw(FUNC(jclub2o_state::console_r), FUNC(jclub2o_state::console_w));
	map(0x580421, 0x580421).r(FUNC(jclub2o_state::console_status_r)); //.w(FUNC(jclub2o_map::));
//  map(0x580440, 0x580443).w(FUNC(jclub2o_map::));

	// ST-0020
	map(0x600000, 0x67ffff).rw(m_st0020, FUNC(st0020_device::sprram_r), FUNC(st0020_device::sprram_w));
	map(0x680000, 0x69ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x6a0000, 0x6bffff).ram();
	map(0x6c0000, 0x6c00ff).rw(m_st0020, FUNC(st0020_device::regs_r), FUNC(st0020_device::regs_w));
	map(0x700000, 0x7fffff).rw(m_st0020, FUNC(st0020_device::gfxram_r), FUNC(st0020_device::gfxram_w));
}


// ST-0016 map

// common rombank? should go in machine/st0016 with larger address space exposed?
void jclub2o_state::st0016_rom_bank_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x1f);
}

uint8_t jclub2o_state::cmd1_r()
{
	m_cmd_stat &= ~0x01;
	return m_cmd1;
}
uint8_t jclub2o_state::cmd2_r()
{
	m_cmd_stat &= ~0x04;
	return m_cmd2;
}
uint8_t jclub2o_state::cmd_stat_r()
{
	return m_cmd_stat;
}

void jclub2o_state::cmd1_w(uint8_t data)
{
	m_cmd1 = data;
	m_cmd_stat |= 0x02;
	logerror("%s: cmd1_w %02x\n", machine().describe_context(), m_cmd1);
}
void jclub2o_state::cmd2_w(uint8_t data)
{
	m_cmd2 = data;
	m_cmd_stat |= 0x08;
	logerror("%s: cmd2_w %02x\n", machine().describe_context(), m_cmd2);
}

void jclub2o_state::st0016_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("soundbank");
	map(0xe800, 0xe8ff).ram();
	//map(0xe900, 0xe9ff) // sound - internal
	//map(0xec00, 0xec1f).rw(FUNC(jclub2o_state::st0016_character_ram_r), FUNC(jclub2o_state::st0016_character_ram_w));
	map(0xf000, 0xffff).ram();
}

void jclub2o_state::st0016_io(address_map &map)
{
	map.global_mask(0xff);
	//map(0x00, 0xbf).rw(FUNC(jclub2o_state::st0016_vregs_r), FUNC(jclub2o_state::st0016_vregs_w));
	map(0xc0, 0xc0).rw(FUNC(jclub2o_state::cmd1_r), FUNC(jclub2o_state::cmd1_w));
	map(0xc1, 0xc1).rw(FUNC(jclub2o_state::cmd2_r), FUNC(jclub2o_state::cmd2_w));
	map(0xc2, 0xc2).r(FUNC(jclub2o_state::cmd_stat_r));
	map(0xe1, 0xe1).w(FUNC(jclub2o_state::st0016_rom_bank_w));
	map(0xe7, 0xe7).nopw(); // watchdog?
	//map(0xf0, 0xf0).r(FUNC(jclub2o_state::st0016_dma_r));
}


// Newer hardware (ST-0032) only

void common_state::eeprom_93c46_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (data & ~0xff000000)
		logerror("%s: Unknown EEPROM bit written %08X\n", machine().describe_context(), data);

	if (ACCESSING_BITS_24_31)
	{
		// latch the bit
		m_eeprom->di_write((data & 0x04000000) >> 26);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x01000000) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x02000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}

void jclub2_state::jclub2_map(address_map &map)
{
	map(0x000000, 0x27ffff).rom();
	map(0x400000, 0x41ffff).ram().share("nvram"); // battery

	map(0x490000, 0x490003).w(FUNC(jclub2_state::eeprom_93c46_w));

	map(0x4a0000, 0x4a0003).w(FUNC(jclub2_state::out2_w));

	map(0x4d0000, 0x4d0003).nopr().nopw(); // reads seem unused? this write would go to two 7-segs (but the code is never called)
	map(0x4d0004, 0x4d0007).nopr();
	map(0x4d0008, 0x4d000b).nopr();
	map(0x4d000c, 0x4d000f).nopr();

	map(0x4e0000, 0x4e0003).r(FUNC(jclub2_state::p2_r)).w(FUNC(jclub2_state::input_sel2_w));

	map(0x580000, 0x580003).portr("EEPROM");
	map(0x580004, 0x580007).r(FUNC(jclub2_state::p1_r));
	map(0x580008, 0x58000b).portr("COIN");
	map(0x58000c, 0x58000f).w(FUNC(jclub2_state::input_sel1_out3_w));
	map(0x580010, 0x580013).w(FUNC(jclub2_state::out1_w));
//  map(0x580018, 0x58001b).w(FUNC(jclub2_map::));
//  map(0x58001c, 0x58001f).w(FUNC(jclub2_map::));

	map(0x580200, 0x580201).r("watchdog", FUNC(watchdog_timer_device::reset16_r));

	map(0x580401, 0x580401).rw(FUNC(jclub2_state::console_r), FUNC(jclub2_state::console_w));
	map(0x580421, 0x580421).r(FUNC(jclub2_state::console_status_r));  //.w(FUNC(jclub2_map::));
//  map(0x580440, 0x580443).w(FUNC(jclub2_map::));

	// ST-0032
	map(0x800000, 0x87ffff).rw(m_st0020, FUNC(st0020_device::sprram_r), FUNC(st0020_device::sprram_w));
	map(0x880000, 0x89ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x8a0000, 0x8bffff).ram();   // this should still be palette ram!
	map(0x8c0000, 0x8c00ff).rw(m_st0020, FUNC(st0020_device::regs_r), FUNC(st0020_device::regs_w));
	map(0x8e0000, 0x8e01ff).rw("st0032_snd", FUNC(st0032_sound_device::snd_r), FUNC(st0032_sound_device::snd_w));
	map(0x8e0200, 0x8e0201).rw("st0032_snd", FUNC(st0032_sound_device::key_r), FUNC(st0032_sound_device::key_w));
	map(0x8e0210, 0x8e0213).ram(); // sound?
	map(0x900000, 0x9fffff).rw(m_st0020, FUNC(st0020_device::gfxram_r), FUNC(st0020_device::gfxram_w));
}


// bootleg darkhors hardware

void darkhors_state::input_sel_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_23)
		m_input_sel1 = data >> 16;
	if (ACCESSING_BITS_24_31)
		m_input_sel2 = data >> 24;
}

uint32_t darkhors_state::input_r()
{
	return  (read_key(m_key1, m_input_sel1) << 16) |
			(read_key(m_key2, m_input_sel2) << 24) ;
}

void darkhors_state::out1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_out1);
	if (ACCESSING_BITS_16_31)
	{
		// note: the output test is buggy and crashes
		// when pressing start (rts @ 328b8)

		// 0x8000 P2 lockout coil
		// 0x4000 P2 divider coil
		// 0x2000 P2 hopper rotate
		// 0x1000 P1 lockout coil
		// 0x0800 P1 divider coil
		// 0x0400 P1 hopper rotate
		// -
		// 0x0100 tower lamp 3
		// 0x0080 tower lamp 2
		// 0x0040 tower lamp 1
		// 0x0020 counter meter 6
		// 0x0010 counter meter 5
		// 0x0008 counter meter 4
		// 0x0004 counter meter 3
		// 0x0002 counter meter 2
		// 0x0001 counter meter 1

		machine().bookkeeping().coin_lockout_w(1, (~data) & 0x80000000);
		machine().bookkeeping().coin_lockout_w(0, (~data) & 0x10000000);

		debug_out();
	}
}

void darkhors_state::darkhors_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x41ffff).ram().share("nvram"); // battery

	map(0x490040, 0x490043).w(FUNC(darkhors_state::eeprom_93c46_w));
	map(0x4e0080, 0x4e0083).portr("SERVICE").w(FUNC(darkhors_state::out1_w));

	map(0x580000, 0x580003).portr("UNKNOWN");
	map(0x580004, 0x580007).portr("COIN");
	map(0x580008, 0x58000b).r(FUNC(darkhors_state::input_r));
	map(0x58000c, 0x58000f).w(FUNC(darkhors_state::input_sel_w));
//  map(0x580010, 0x580013).w(FUNC(darkhors_state::));
//  map(0x580018, 0x58001b).w(FUNC(darkhors_state::));
//  map(0x58001c, 0x58001f).w(FUNC(darkhors_state::));
	map(0x580084, 0x580084).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//  map(0x58008c, 0x58008f).w(FUNC(darkhors_state::));
	map(0x580200, 0x580201).r("watchdog", FUNC(watchdog_timer_device::reset16_r));

	map(0x580401, 0x580401).rw(FUNC(darkhors_state::console_r), FUNC(darkhors_state::console_w));
	map(0x580421, 0x580421).r(FUNC(darkhors_state::console_status_r));

	map(0x800000, 0x86bfff).ram();
	map(0x86c000, 0x86ffff).ram().w(FUNC(darkhors_state::tmapram_w)).share("tmapram");
	map(0x870000, 0x873fff).ram().w(FUNC(darkhors_state::tmapram2_w)).share("tmapram2");
	map(0x874000, 0x87dfff).ram();
	map(0x87e000, 0x87ffff).ram().share("spriteram");
	map(0x880000, 0x89ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x8a0000, 0x8bffff).ram();   // this should still be palette ram!
	map(0x8c0120, 0x8c012f).writeonly().share("tmapscroll");
	map(0x8c0130, 0x8c013f).writeonly().share("tmapscroll2");
}

void darkhors_state::darkhorsa_map(address_map &map)
{
	darkhors_map(map);

	map(0x8c0020, 0x8c002f).writeonly().share("tmapscroll");
	map(0x8c0030, 0x8c003f).writeonly().share("tmapscroll2");
	map(0x8c0120, 0x8c012f).unmaprw();
	map(0x8c0130, 0x8c013f).unmaprw();
}

/***************************************************************************

                                Input Ports

***************************************************************************/

/***************************************************************************
                    Keyboards (39 pressure sensitive keys)

            white---------------------- cyan-------------------
            1-2 1-3 1-4 1-5 1-6 1-7 1-8 2-3 2-4 2-5 2-6 2-7 2-8

            orange------------- green---------- grey------- yel
            3-4 3-5 3-6 3-7 3-8 4-5 4-6 4-7 4-8 5-6 5-7 5-8 CAN

            cyan--- whi red---------------------------- yellow-
            6-7 6-8 7-8 1   2   3   4   5   6   7   8   PAY STA

***************************************************************************/

static INPUT_PORTS_START( keyboards )
	PORT_START("KEY1.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_START("KEY2.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 8")

	PORT_START("KEY1.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-2") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-4") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-6") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-7") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-8") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-7")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-3") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-5") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-6") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-7") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-8") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-7")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-8")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-4") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-6") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-7") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-8") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-7")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-8")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-5") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-7") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-8") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-8")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-6") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-7") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-8") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-7")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-8")
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6-7") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6-8") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6-7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6-8")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7-8") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 7-8")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( jclub2v100 )
	// P2 at 4e0000.b, P1 at 580004.b
	PORT_INCLUDE( keyboards )

	PORT_START("EEPROM") // 580000.w
	PORT_BIT( 0x74ff0000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_CONFNAME(0x01000000, 0x01000000, "Coin Chutes Polarity" )
	PORT_CONFSETTING(         0x01000000, DEF_STR( Normal) )
	PORT_CONFSETTING(         0x00000000, "Reverse" ) // warning message
	PORT_CONFNAME(0x02000000, 0x02000000, "Main Loop Down" )
	PORT_CONFSETTING(         0x02000000, DEF_STR( Off ) )
	PORT_CONFSETTING(         0x00000000, DEF_STR( On )  ) // Emergency Error 0001
	PORT_CONFNAME(0x08000000, 0x08000000, "System Int Down")
	PORT_CONFSETTING(         0x08000000, DEF_STR( Off ) )
	PORT_CONFSETTING(         0x00000000, DEF_STR( On )  ) // Emergency Error 0002
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("COIN") // 580008.w
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW,  IPT_OTHER   ) PORT_NAME("P1 Payout") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW,  IPT_START1  )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper2", ticket_dispenser_device, line_r) // P2 coin out
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper1", ticket_dispenser_device, line_r) // P1 coin out
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE(15) // P1 coin drop
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE(15) // P2 coin drop
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE( 5) // P1 coin in s1
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE(10) // P1 coin in s2
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE( 5) // P2 coin in s1
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE(10) // P2 coin in s2

	PORT_START("P1LOW") // 580004.w (low byte)
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset Key")     PORT_TOGGLE // reset error condition, e.g. coin time-out error
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter Key")     PORT_TOGGLE
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Last Game Key") PORT_TOGGLE
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("P1 Cancel") PORT_CODE(KEYCODE_LALT)
	PORT_SERVICE_NO_TOGGLE( 0x00100000, IP_ACTIVE_LOW ) // test switch (on during boot: service mode, but make sure Config Key is off!)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_CUSTOM  ) // P2 hopper full
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_CUSTOM  ) // P1 hopper full

	PORT_START("P2LOW") // 4e0000.w (low byte)
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Payout") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Cancel") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( jclub2v112 )
	PORT_INCLUDE(jclub2v100)

	PORT_MODIFY("EEPROM") // 580000.w
	PORT_BIT( 0x74ff0000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_CONFNAME(0x03000000, 0x03000000, "Backup Battery" )
	PORT_CONFSETTING(         0x00000000, "Off (0)" ) // Hardware Error 0001
	PORT_CONFSETTING(         0x01000000, "Off (1)" ) // ""
	PORT_CONFSETTING(         0x02000000, "Low" )     // warning message (checked at boot)
	PORT_CONFSETTING(         0x03000000, DEF_STR( On ) )
	PORT_CONFNAME(0x08000000, 0x08000000, "Disable Coins?") // causes lockout and coins to not register (same as an hardware error)
	PORT_CONFSETTING(         0x08000000, DEF_STR( Off ))
	PORT_CONFSETTING(         0x00000000, DEF_STR( On ))
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
INPUT_PORTS_END


static INPUT_PORTS_START( darkhors )
	// P2 at 580008.b, P1 at 580009.b
	PORT_INCLUDE( keyboards )

	// Battery status register reads have been replaced with a constant value (fbff) in the code

	PORT_START("UNKNOWN") // 580000.w
	PORT_BIT( 0xff7f0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("?") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("COIN") // 580004.w
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Bill 1") PORT_CODE(KEYCODE_BACKSPACE) // P1 bill-in
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE( 5) // P1 coin in s1
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(10) // P1 coin in s2
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(15) // P1 coin drop
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_CUSTOM )                  // P1 hopper full
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_OTHER   )                  // P1 coin out sensor
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Bill 2") PORT_CODE(KEYCODE_ENTER)  // P2 bill-in
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE( 5) // P2 coin in s1
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE(10) // P2 coin in s2
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE(15) // P2 coin drop
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_CUSTOM )                  // P2 hopper full
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_OTHER   )                  // P2 coin out sensor
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE") // 4e0080.w
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW,  IPT_SERVICE4 ) PORT_NAME("Config Key")    PORT_TOGGLE // on during boot: clear backup data (prevents service mode)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME("Reset Key")     PORT_TOGGLE // reset error condition, e.g. coin time-out error
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW,  IPT_SERVICE2 ) PORT_NAME("Meter Key")     PORT_TOGGLE
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW,  IPT_SERVICE3 ) PORT_NAME("Last Game Key") PORT_TOGGLE
	PORT_SERVICE_NO_TOGGLE( 0x00100000, IP_ACTIVE_LOW  ) // test switch (on during boot: service mode, but make sure Config Key is off!)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW,  IPT_OTHER    ) PORT_NAME("Door 1") PORT_CODE(KEYCODE_OPENBRACE)  PORT_TOGGLE
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW,  IPT_OTHER    ) PORT_NAME("Door 2") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_TOGGLE
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) // door 3 in service mode!
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW,  IPT_START1   )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW,  IPT_OTHER    ) PORT_NAME("P1 Payout") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW,  IPT_OTHER    ) PORT_NAME("P1 Cancel") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW,  IPT_START2   )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW,  IPT_OTHER    ) PORT_NAME("P2 Payout") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW,  IPT_OTHER    ) PORT_NAME("P2 Cancel") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW,  IPT_UNKNOWN  ) // tested during lev 3 (vblank) interrupt
INPUT_PORTS_END


/***************************************************************************

                                Gfx Layouts
                                 (darkhors)

***************************************************************************/

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{   RGN_FRAC(3,4)+8,RGN_FRAC(3,4)+0,
		RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8,RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0 },
	{ STEP8(0,1), STEP8(16,1) },
	{ STEP16(0,16*2)},
	16*16*2
};

static GFXDECODE_START( gfx_darkhors )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8, 0, 0x10000/64 ) // color codes should be doubled
GFXDECODE_END

/***************************************************************************

                                Machine Drivers

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(common_state::scanline_irq)
{
	int scanline = param;

	if (scanline == 248)
		m_maincpu->set_input_line(5, HOLD_LINE);

	if (scanline == 0)
		m_maincpu->set_input_line(3, HOLD_LINE);

	if (scanline == 128)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

// Older hardware (ST-0020 + ST-0016)
void jclub2o_state::jclub2o(machine_config &config)
{
	M68EC020(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &jclub2o_state::jclub2o_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(common_state::scanline_irq), "screen", 0, 1);

	ST0016_CPU(config, m_soundcpu, 8000000);
	m_soundcpu->set_addrmap(AS_PROGRAM, &jclub2o_state::st0016_mem);
	m_soundcpu->set_addrmap(AS_IO, &jclub2o_state::st0016_io);
	m_soundcpu->set_vblank_int("screen", FUNC(jclub2o_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	EEPROM_S29290_16BIT(config, "eeprom");
	WATCHDOG_TIMER(config, "watchdog");

	TICKET_DISPENSER(config, m_hopper1, attotime::from_msec(200));
	TICKET_DISPENSER(config, m_hopper2, attotime::from_msec(200));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(0x190, 0x100+16);
	screen.set_visarea(0, 0x190-1, 0x10, 0x100-1);
	screen.set_screen_update(FUNC(jclub2_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x10000);

	ST0020_SPRITES(config, m_st0020, 0);
	m_st0020->set_is_jclub2(1);
	m_st0020->set_palette(m_palette);

	// layout
	config.set_default_layout(layout_jclub2o);

	// TODO: Mono?
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	m_soundcpu->add_route(0, "lspeaker", 1.0);
	m_soundcpu->add_route(1, "rspeaker", 1.0);
}


// Newer hardware (ST-0032)
void jclub2_state::jclub2(machine_config &config)
{
	M68EC020(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &jclub2_state::jclub2_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(common_state::scanline_irq), "screen", 0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	EEPROM_93C46_8BIT(config, "eeprom");
	WATCHDOG_TIMER(config, "watchdog");

	TICKET_DISPENSER(config, m_hopper1, attotime::from_msec(200));
	TICKET_DISPENSER(config, m_hopper2, attotime::from_msec(200));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(0x190, 0x100+16);
	screen.set_visarea(0, 0x190-1, 8, 0x100-8-1);
	screen.set_screen_update(FUNC(jclub2_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x10000);

	// NOT an ST0020 but instead ST0032, ram format isn't compatible at least
	ST0020_SPRITES(config, m_st0020, 0);
	m_st0020->set_is_st0032(1);
	m_st0020->set_is_jclub2(1); // offsets
	m_st0020->set_palette(m_palette);

	// layout
	config.set_default_layout(layout_jclub2o);

	// sound hardware
	// TODO: Mono?
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	st0032_sound_device &st0032_snd(ST0032_SOUND(config, "st0032_snd", XTAL(42'954'545) / 3)); // 14.318181MHz (42.954545MHz / 3)
	st0032_snd.add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	st0032_snd.add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}


void darkhors_state::darkhors(machine_config &config)
{
	M68EC020(config, m_maincpu, 12000000); // 36MHz/3 ??
	m_maincpu->set_addrmap(AS_PROGRAM, &darkhors_state::darkhors_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(common_state::scanline_irq), "screen", 0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	EEPROM_93C46_8BIT(config, "eeprom");
	WATCHDOG_TIMER(config, "watchdog");

	TICKET_DISPENSER(config, m_hopper1, attotime::from_msec(200));
	TICKET_DISPENSER(config, m_hopper2, attotime::from_msec(200));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(0x190, 0x100+16);
	screen.set_visarea(0, 0x190-1, 8, 0x100-8-1);
	screen.set_screen_update(FUNC(darkhors_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x10000);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_darkhors);

	// layout
	config.set_default_layout(layout_jclub2);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 528000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void darkhors_state::darkhorsa(machine_config &config)
{
	darkhors(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &darkhors_state::darkhorsa_map);
}

/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

Jockey Club II by SETA 1994-1995

Other hardware version (older):

Main PCB   :  E06-00409
Sub  PCB   :  E06-00407 (I/O, nothing else)

Main CPU   :  MC68EC020FG16

Many XTALs :  48.0000 MHz, 33.3333 MHz, 4.91520 MHz, 42.9545 MHz(x2), 105.0000 MHz
              (the 105.0000 Xtal is sometimes replaced by a tiny pcb silkscreened 108.0000 MHz(!), with ICS ICS1494N, MB3771 and 14.3181 MHz Xtal)

Graphics   :  SETA ST-0020

Others     :  SETA ST-0013
              SETA ST-0016  <-- z80 core + simple gfx + sound, see st0016.cpp
              SETA ST-0017

Rams       :  Toshiba TC514800AJ-70
              Toshiba TC514000ASJ-70 (x8)
              Sharp LH5168D

Eproms     :  SX006A-01.u26, JC2-110x.u27, SX006-04.u87 (sound)
              SX006B-01.u26, JC2-110x.u27, SX006-04.u87 (sound)
              SX006A-01.u26, JC2-112x.u27, SX006-04.u87 (sound)
              SX006B-01.u26, JC2-112x.u27, SX006-04.u87 (sound)
              (u26 read as 5716200)

Provided to you by Belgium Dump Team Gerald (COY) on 18/01/2007.

***************************************************************************/

#define JCLUB2O_SOUND_ROMS \
	ROM_REGION( 0x80000, "soundcpu", 0 ) /* z80 core (used for sound) */ \
	ROM_LOAD( "sx006-04.u87", 0x00000, 0x80000, CRC(a87adedd) SHA1(1cd5af2d03738fff2230b46241659179467c828c) )  /* SoundDriverV1.26 */

ROM_START( jclub2v100 )
	JCLUB2O_SOUND_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sx006a-01.u26", 0x000000, 0x200000, CRC(55e249bc) SHA1(ed0f066ed17f047760b712cbbfba1a62d4b452ba) ) // v1.00 (1994 ' ' 100,  5 OCT. 1994)
	ROM_FILL(                              0x200000, 0x080000, 0xff) // no upgrade rom

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	// 1.00
	ROM_LOAD16_WORD_SWAP( "eeprom_jc2v100", 0x000, 0x100, CRC(1ced2e6c) SHA1(7bc7d40a9fde3256c52db5db2ad28a036557b99a) ) // from MAME (SETA1994JC2 100 )
ROM_END

ROM_START( jclub2v101 )
	JCLUB2O_SOUND_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sx006b-01.u26", 0x000000, 0x200000, CRC(f730dded) SHA1(efb966dcb98440a072d4825ef2788c85acdfd103) ) // v1.01 (1995 A 101, 20 FEB. 1995)
	ROM_FILL(                              0x200000, 0x080000, 0xff) // no upgrade rom

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	// 1.01 (JC2A)
	ROM_LOAD16_WORD_SWAP( "eeprom_jc2v101", 0x000, 0x100, CRC(2960b967) SHA1(8089ffea7d3fe5a558cccad9e285978c8099ba26) ) // from MAME (SETA1995JC2A101 )
ROM_END

ROM_START( jclub2v110 )
	JCLUB2O_SOUND_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sx006b-01.u26", 0x000000, 0x200000, CRC(f730dded) SHA1(efb966dcb98440a072d4825ef2788c85acdfd103) ) // v1.01  (1995 A   101, 20 FEB. 1995)
	ROM_LOAD16_WORD_SWAP( "jc2-110x.u27",  0x200000, 0x080000, CRC(03aa6882) SHA1(e0343bc77a19994ddafa614891663b40e1476332) ) // v1.10X (1996 2/3 110,  5 MAY. 1996)

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	// 1.10X (JC22110 )
	ROM_LOAD16_WORD_SWAP( "eeprom_jc2v110", 0x000, 0x100, CRC(ea23dfb2) SHA1(de3d771024c55f907e32bb313eaa9d28b3a9d533) ) // from MAME (SETA1996JC22110 )
ROM_END

ROM_START( jclub2v112 )
	JCLUB2O_SOUND_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sx006b-01.u26", 0x000000, 0x200000, CRC(f730dded) SHA1(efb966dcb98440a072d4825ef2788c85acdfd103) ) // v1.01  (1995 A   101, 20 FEB. 1995)
	ROM_LOAD16_WORD_SWAP( "jc2-112x.u27",  0x200000, 0x080000, CRC(e1ab93bd) SHA1(78b618b3f7819bd5351ebf949f328fec7795cec9) ) // v1.12X (1996 4/5 112,  3 JUN. 1996)

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	// 1.12X (JC24112E)
	ROM_LOAD16_WORD_SWAP( "eeprom_jc2v112", 0x000, 0x100, CRC(be1d26f9) SHA1(1b6c6fd1c3298439206d73cb934dd1c30aee8a6c) ) // from MAME (SETA1996JC24112E)
ROM_END

ROM_START( jclub2v203 )
	JCLUB2O_SOUND_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sx006b-01.u26", 0x000000, 0x200000, CRC(f730dded) SHA1(efb966dcb98440a072d4825ef2788c85acdfd103) ) // v1.01  (1995 A 101, 20 FEB. 1995)
	ROM_LOAD16_WORD_SWAP( "203x-rom1.u27", 0x200000, 0x080000, CRC(7446ed3e) SHA1(b0936e42549280e2965159270429c4fdacba114a) ) // v2.03X (1997 6 203, 26 MAR. 1997) Release Candidate

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	// 2.03X (JC26203 :00020400C5IK)
//  ROM_LOAD16_WORD_SWAP( "eeprom-jclub2o.bin", 0x000, 0x100, CRC(dd1c88ec) SHA1(acb67e41e832f203361e0f93afcd4eaf963fd13e) ) // dump      (SETA1997JC26203 )
	ROM_LOAD16_WORD_SWAP( "eeprom_jc2v203",     0x000, 0x100, CRC(c1bc58e7) SHA1(4670c94fd655d223f21254167e4334b81affdf8d) ) // from MAME (SETA1997JC26203 )
ROM_END

/***************************************************************************

Jockey Club II by SETA 1996

PCB       :  E79-001 rev 01a (Newer)

Main CPU  :  MC68EC020FG16

Graphics  :  SETA ST-0032 70C600JF505

Others    :  SETA ST-0013
             SETA ST-0017

XTALs     :  42.9545 MHz, 60.0000 MHz, 33.3333 MHz

Rams      :  Toshiba : TC5118160CJ-60 (x3)
             NKK N341256SJ-15 (x2)
             NEC D43001GU-70LL (x4)

Gals      :  gal16V8B(x2) ref : M88-03 M88-04

Eeprom    :  93c46

Eproms    :  M88-01.u38,  M88-02.u6              (1st set)
             M88-01A.u38, M88-02.u6              (2nd set)
             M88-01B.u38, M88-02.u6              (3rd set)
             M88-01.u38,  M88-02.u6, Z201x.u39   (4th set)
             M88-01A.u38, M88-02.u6, M88-03D.u39 (5th set)
             M88-01B.u38, M88-02.u6, M88-03D.u39 (6th set)
             (u6 read as 578200)

Provided to you by Belgium Dump Team Gerald (COY) on 18/01/2007.

***************************************************************************/

#define JCLUB2_OTHER_ROMS \
	ROM_REGION( 0x100000, "st0032_snd", 0 ) \
	ROM_LOAD( "m88-02.u6", 0x00000, 0x100000, CRC(0dd3436a) SHA1(809d3b7a26d36f71da04036fd8ab5d0c5089392a) ) \
	\
	ROM_REGION( 0x117, "pld", 0 ) \
	ROM_LOAD( "gal16v8b-m88-03.bin", 0x000, 0x117, CRC(6d9c882e) SHA1(84cb95ab540290c2f8b740668360e9c643a67dcf) ) \
	ROM_LOAD( "gal16v8b-m88-04.bin", 0x000, 0x117, CRC(5e79f292) SHA1(5e44c234e2b15d486a1af71fee986892aa245b4d) )

ROM_START( jclub2v200 )
	JCLUB2_OTHER_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "m88-01.u38", 0x000000, 0x200000, CRC(84476b68) SHA1(1014d23d3cebbfa9aa3bfb90505529989a8eedfa) ) // v2.00 (1996 Z 200 , 27 SEP. 1996)
	ROM_FILL(                           0x200000, 0x080000, 0xff) // no upgrade rom

	ROM_REGION( 0x80, "eeprom", 0 )
	// 2.00 (JC2Z200 :0000000004HQ)
	ROM_LOAD( "eeprom_jc2v200", 0x00, 0x80, CRC(90cc44da) SHA1(5c43d88009537884112950544fc74874cd0e745c) ) // from MAME (SETA1996JC2Z200 )
ROM_END

ROM_START( jclub2v201 )
	JCLUB2_OTHER_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "m88-01.u38", 0x000000, 0x200000, CRC(84476b68) SHA1(1014d23d3cebbfa9aa3bfb90505529989a8eedfa) ) // v2.00  (1996 Z 200 , 27 SEP. 1996)
	ROM_LOAD16_WORD_SWAP( "z201x.u39",  0x200000, 0x080000, CRC(1fb79c16) SHA1(c8914f7dfc17c412f6ca756f8eb6d6a35e3b6214) ) // v2.01X (1996 Z 201 , 20 NOV. 1996)

	ROM_REGION( 0x80, "eeprom", 0 )
	// 2.01X (JC2Z201 :00000400248Q)
	ROM_LOAD( "eeprom_jc2v201", 0x00, 0x80, CRC(e81deb16) SHA1(f96c7953e7c5221f6c1cfccc7aa331e7453eca00) ) // from MAME (SETA1996JC2Z201 )
ROM_END

ROM_START( jclub2v204 )
	JCLUB2_OTHER_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "m88-01a.u38", 0x000000, 0x200000, CRC(c1243e1c) SHA1(2a5857738b8950daf77ddaa8304b765f809f8241) ) // v2.04 (1997 Z 2040, 30 APR. 1997)
	ROM_FILL(                            0x200000, 0x080000, 0xff) // no upgrade rom

	ROM_REGION( 0x80, "eeprom", 0 )
	// 2.04 (JC2Z2040:00000400248Q)
	ROM_LOAD( "eeprom_jc2v204", 0x00, 0x80, CRC(fb25ba0f) SHA1(14c8951648e77cf9abe61ad4399d9be8abc93b2a) ) // from MAME (SETA1997JC2Z2040)
ROM_END

ROM_START( jclub2v205 )
	JCLUB2_OTHER_ROMS
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "m88-01b.u38", 0x000000, 0x200000, CRC(f1054c69) SHA1(be6d92653f0d3cc0a36a2ff0798043f4a95439bc) ) // v2.05 (1997 Z 2050, 21 JUL. 1997)
	ROM_FILL(                            0x200000, 0x080000, 0xff) // no upgrade rom

	ROM_REGION( 0x80, "eeprom", 0 )
	// 2.05 (JC2Z2050:00000400248Q)
	ROM_LOAD( "eeprom_jc2v205", 0x00, 0x80, CRC(9f1fdcd5) SHA1(8e492c6450c56859f582ed89b384caa979a912f0) ) // from MAME (SETA1997JC2Z2050)
ROM_END

ROM_START( jclub2v220 )
	JCLUB2_OTHER_ROMS
	ROM_REGION( 0x280000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "m88-01b.u38", 0x000000, 0x200000, CRC(f1054c69) SHA1(be6d92653f0d3cc0a36a2ff0798043f4a95439bc) ) // v2.05  (1997 Z 2050, 21 JUL. 1997)
	ROM_LOAD16_WORD_SWAP( "m88-03d.u39", 0x200000, 0x080000, CRC(723dd22b) SHA1(0ca622e0dd315f29e72dd9b82fb419d306ec5df8) ) // v2.20X (1997 Z 2201, 14 APR. 1998)

	ROM_REGION( 0x80, "eeprom", 0 )
	// 2.20X (JC2Z2201:00000400248Q)
	ROM_LOAD( "eeprom_jc2v220", 0x00, 0x80, CRC(bde1f064) SHA1(a9b6cfabcc63b1429c4e0c9b162ce7be60c3c515) ) // from MAME (SETA1997JC2Z2201)
ROM_END

/***************************************************************************

Dark Horse (2001)

A bootleg of Jockey Club II on inferior hardware

|-----------------------------------------------------------|
|  M6295    SND              GM76C512 GM76C512  |-------|   |
|                            GM76C512 GM76C512  |ACTEL  |   |
|      EEPROM         PRG1   GM76C512 GM76C512  |A40MX04|   |
|                            GM76C512 GM76C512  |       |   |
|                     PRG2                      |-------|   |
|   |-------|                          62256   GM71C4260    |
|   |ACTEL  |          3.6V_BATT       62256   GM71C4260    |
|   |A40MX04| 68EC020      GAL                              |
|   |       |              GAL                         GAL  |
|J  |-------|                                               |
|A                                           GFX1    GFX2   |
|M                                                          |
|M                                                   GFX3   |
|A                                                          |
|                                                    GFX4   |
|  62256                                                    |
|                                                    GFX5   |
|  62256                       |-------|                    |
|           62256              |ACTEL  |             GFX6   |
|                              |A42MX09|                    |
|           62256              |       |             GFX7   |
|                              |-------|                    |
|           GAL                                      GFX8   |
|           GAL                                             |
|           36MHz                                           |
|-----------------------------------------------------------|

***************************************************************************/

ROM_START( darkhors )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68EC020 code
	ROM_LOAD32_WORD_SWAP( "prg2", 0x00000, 0x80000, CRC(f2ec5818) SHA1(326937a331496880f517f41b0b8ab54e55fd7af7) ) // 27 JUN. 1997
	ROM_LOAD32_WORD_SWAP( "prg1", 0x00002, 0x80000, CRC(b80f8f59) SHA1(abc26dd8b36da0d510978364febe385f69fb317f) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "gfx1", 0x000000, 0x80000, CRC(e9fe9967) SHA1(a79d75c09f0eac6372de8d6e98c5eecf38ef750c) )
	ROM_LOAD( "gfx2", 0x080000, 0x80000, CRC(0853c5c5) SHA1(2b49ffe607278817f1f8219a79f5906be53ee6f4) )
	ROM_LOAD( "gfx3", 0x100000, 0x80000, CRC(6e89278f) SHA1(044c15e00ea95fd3f108fa916000a1000789c8e8) )
	ROM_LOAD( "gfx4", 0x180000, 0x80000, CRC(f28407ab) SHA1(47933719cff8099fc079fd736b4b08176f3aff66) )
	ROM_LOAD( "gfx5", 0x200000, 0x80000, CRC(281402cd) SHA1(77f8e5e02c6e7161299c06e65a078c1cdda1ba66) )
	ROM_LOAD( "gfx6", 0x280000, 0x80000, CRC(8ea0149b) SHA1(7792fd7e07a7baa4e15f50b6528c78fb15b40b40) )
	ROM_LOAD( "gfx7", 0x300000, 0x80000, BAD_DUMP CRC(504bf849) SHA1(13a184ec9e176371808938015111f8918cb4df7d) ) // FIXED BITS (11111111)
	ROM_FILL(         0x300000, 0x80000, 0x00 ) // a zero-fill seems fine
	ROM_LOAD( "gfx8", 0x380000, 0x80000, CRC(590bec2a) SHA1(7fdbb21f1a3eccde65e91eb2443a0e01487c59c3) ) // 000xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "oki", 0 ) // Samples
	ROM_LOAD( "snd", 0x00000, 0x80000, CRC(7aeb12d3) SHA1(3e81725fc206baa7559da87552a0cd73b7616155) )

	ROM_REGION( 0x80, "eeprom", 0 ) // EEPROM
	ROM_LOAD( "eeprom", 0x000, 0x080, CRC(1f434f66) SHA1(e1bee11d83fb72aed9c312bdc794d8b9a6645534) ) // (SETA1997JC2W400 )
ROM_END


ROM_START( jclub2bl )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68EC020 code
	ROM_LOAD32_WORD_SWAP( "154.4002", 0x00000, 0x80000, CRC(b892c254) SHA1(c1c31d878f41e4751048c07055d426803cd3332d) ) // 27 JUN. 1997
	ROM_LOAD32_WORD_SWAP( "155.4002", 0x00002, 0x80000, CRC(3d9061c0) SHA1(efd2c8d742d3ac097f0228c2b71453b3261a6d83) )

	ROM_REGION( 0x400000, "gfx1", 0 ) // not dumped yet, should be closer to the original (no Dark Horse GFX)
	ROM_LOAD( "gfx1", 0x000000, 0x80000, BAD_DUMP CRC(e9fe9967) SHA1(a79d75c09f0eac6372de8d6e98c5eecf38ef750c) )
	ROM_LOAD( "gfx2", 0x080000, 0x80000, BAD_DUMP CRC(0853c5c5) SHA1(2b49ffe607278817f1f8219a79f5906be53ee6f4) )
	ROM_LOAD( "gfx3", 0x100000, 0x80000, BAD_DUMP CRC(6e89278f) SHA1(044c15e00ea95fd3f108fa916000a1000789c8e8) )
	ROM_LOAD( "gfx4", 0x180000, 0x80000, BAD_DUMP CRC(f28407ab) SHA1(47933719cff8099fc079fd736b4b08176f3aff66) )
	ROM_LOAD( "gfx5", 0x200000, 0x80000, BAD_DUMP CRC(281402cd) SHA1(77f8e5e02c6e7161299c06e65a078c1cdda1ba66) )
	ROM_LOAD( "gfx6", 0x280000, 0x80000, BAD_DUMP CRC(8ea0149b) SHA1(7792fd7e07a7baa4e15f50b6528c78fb15b40b40) )
	ROM_LOAD( "gfx7", 0x300000, 0x80000, BAD_DUMP CRC(504bf849) SHA1(13a184ec9e176371808938015111f8918cb4df7d) ) // FIXED BITS (11111111)
	ROM_FILL(         0x300000, 0x80000, 0x00 ) // a zero-fill seems fine
	ROM_LOAD( "gfx8", 0x380000, 0x80000, BAD_DUMP CRC(590bec2a) SHA1(7fdbb21f1a3eccde65e91eb2443a0e01487c59c3) ) // 000xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "oki", 0 ) // Samples
	ROM_LOAD( "snd", 0x00000, 0x80000, CRC(7aeb12d3) SHA1(3e81725fc206baa7559da87552a0cd73b7616155) ) // same as darkhors

	ROM_REGION( 0x80, "eeprom", 0 ) // EEPROM
	ROM_LOAD( "93c46", 0x000, 0x080, CRC(32ea730a) SHA1(20a6a8259425c3c867319db49b27700b75e8e471) ) // (SETA1997JC2W400 )
ROM_END


/***************************************************************************

                                Game Drivers

***************************************************************************/

void jclub2o_state::init_jclub2o()
{
	m_soundbank->configure_entries(0, 32, memregion("soundcpu")->base(), 0x4000);
}

void darkhors_state::init_darkhors()
{
	// the dumped eeprom bytes are in a different order to how MAME expects them to be!?
	// (offset 0x00, 0x40, 0x01, 0x41, 0x02, 0x42 ..... )
	// The eeprom contains the game ID, which must be valid for it to boot
	uint8_t *eeprom = (uint8_t *)memregion("eeprom")->base();
	if (eeprom)
	{
		size_t len = memregion("eeprom")->bytes();
		std::vector<uint8_t> temp(len);

		for (int i = 0; i < len; i++)
			temp[i] = eeprom[bitswap<8>(i,7,5,4,3,2,1,0,6)];

		memcpy(eeprom, &temp[0], len);
	}
}

} // anonymous namespace


// Older hardware (ST-0020 + ST-0016)
GAME( 1994, jclub2v100, jclub2v112, jclub2o,  jclub2v100, jclub2o_state,  init_jclub2o,  ROT0, "Seta",    "Jockey Club II (v1.00, older hardware)",                MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, jclub2v101, jclub2v112, jclub2o,  jclub2v100, jclub2o_state,  init_jclub2o,  ROT0, "Seta",    "Jockey Club II (v1.01, older hardware)",                MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, jclub2v110, jclub2v112, jclub2o,  jclub2v100, jclub2o_state,  init_jclub2o,  ROT0, "Seta",    "Jockey Club II (v1.10X, older hardware)",               MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, jclub2v112, 0,          jclub2o,  jclub2v112, jclub2o_state,  init_jclub2o,  ROT0, "Seta",    "Jockey Club II (v1.12X, older hardware)",               MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, jclub2v203, jclub2v112, jclub2o,  jclub2v112, jclub2o_state,  init_jclub2o,  ROT0, "Seta",    "Jockey Club II (v2.03X RC, older hardware, prototype)", MACHINE_IMPERFECT_GRAPHICS )
// Newer hardware (ST-0032)
GAME( 1996, jclub2v200, jclub2v112, jclub2,   jclub2v112, jclub2_state,   empty_init,    ROT0, "Seta",    "Jockey Club II (v2.00, newer hardware)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, jclub2v201, jclub2v112, jclub2,   jclub2v112, jclub2_state,   empty_init,    ROT0, "Seta",    "Jockey Club II (v2.01X, newer hardware)",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, jclub2v204, jclub2v112, jclub2,   jclub2v112, jclub2_state,   empty_init,    ROT0, "Seta",    "Jockey Club II (v2.04, newer hardware)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, jclub2v205, jclub2v112, jclub2,   jclub2v112, jclub2_state,   empty_init,    ROT0, "Seta",    "Jockey Club II (v2.05, newer hardware)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, jclub2v220, jclub2v112, jclub2,   jclub2v112, jclub2_state,   empty_init,    ROT0, "Seta",    "Jockey Club II (v2.20X, newer hardware)",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
// Bootleg hardware
GAME( 2001, darkhors,   jclub2v112, darkhors, darkhors,   darkhors_state, init_darkhors, ROT0, "bootleg", "Dark Horse (USA v4.00, bootleg of Jockey Club II)",     MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, jclub2bl,   jclub2v112, darkhorsa,darkhors,   darkhors_state, init_darkhors, ROT0, "bootleg", "Jockey Club II (USA v4.00, bootleg)",                   MACHINE_IMPERFECT_GRAPHICS )
