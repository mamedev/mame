// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************************

    "Mitchell hardware". Actually used mostly by Capcom.

    All games run on the same hardware except mgakuen, which runs on an
    earlier version, without RAM banking, not encrypted (standard Z80)
    and without EEPROM.

    Notes:
    - Super Pang has a protection which involves copying code stored in the
      EEPROM to RAM and execute it from there. The first time the game is run,
      you have to keep the player 1 start button pressed until the title screen
      appears. This forces the game to initialize the EEPROM, otherwise it will
      not work.

    TODO:
    - understand what bits 0 and 3 of input port 0x05 are
    - correct sound clocks for the MSM5205 bootlegs

******************************************************************************
Pang
Mitchell, 1989

PCB Layout
----------

89125-A-2
|------------------------------------------------------------------|
|HA13001   PW_01.1D PWJ_02.1E PWJ_04.1G          PWJ_09.1K         |
|  VOL VOL  M6295   PW_03.2E  PW_05.2G           PWJ_10.2K         |
|  LM324                                                           |
|             YM2413      CM^81300               |------|          |
|                                                |86S105|          |
|J  TD62064                                      |      |  BUNRAKU |
|A                                               |------|          |
|M                           4364                             16MHz|
|M                                                                 |
|A       5814                                          DL-010D-103 |
|                            4016                                  |
|        5814                                                      |
|                                                                  |
|                            PAL16L8      PWE_06.11H               |
|                            'POKER'      PWE_07.13H               |
|                                                      KABUKI      |
|TEST_SW  93C46              DL-020F-108U          HM6264   BATTERY|
|------------------------------------------------------------------|
Notes:
       KABUKI - Custom encrypted Z80 marked 'VC5006-0001 KABUKI DL-030P-110V KOREA' (DIP40)
      BUNRAKU - Custom chip marked 'BUNRAKU DL-050-115V' (QFP60)
       86S105 - Custom chip marked 'something(scratched) 86S105 RJ5C39 8M2 76' (PLCC84)
 DL-020F-108U - Custom chip marked 'DL-020F-108U' (SDIP64)
  DL-010D-103 - Custom chip marked 'DL-010D-103' (SDIP64)
     CM^81300 - Custom chip marked 'CM^81300' (SDIP28, ^ is a triangle symbol)
        POKER - PAL16L8 marked 'POKER' (DIP20)
         4364 - 8kx8 SRAM (DIP28)
         4016 - 2kx8 SRAM (DIP24)
         5814 - 2kx8 SRAM (DIP24)
       HM6264 - 8kx8 SRAM, battery-backed (DIP28)
        93C46 - AKM-J 93C46 128 bytes EEPROM (DIP8)
        LM324 - Texas Instruments LM324 Low Power Quad Operational Amplifier
      HA13001 - Hitachi HA13001 5.5W Dual / 17.5W BTL Audio Power Amplifier
      TD62064 - Toshiba TD62064 4 Channel High-Current Darlington Sink Driver
          Z80 - 8.000MHz [16/2]
       YM2413 - 4.000MHz [16/4]
        M6295 - 1.000MHz [16/16]
        VSync - 57.4450Hz
        HSync - 15.16452Hz
******************************************************************************

 Monsters World (c)1994 TCH

 Monsters World is basically a bootleg of Mitchell's Super Pang

 The code is a patched version of the current parent 'spang' set supported by
 MAME with many code changes and the majority of strings patched out.

 Super Pang is encrypted using the 'Kabuki' encryption system, so to decrypt
 the game decrypted code and decrypted data must be split.

 Monster World contains banks of decrypted data and decrypted code scrambled
 together in a single ROM, using a GAL to decode the addresses on the actual
 PCB.

 There are several other changes from Super Pang too.  Monsters World has no
 NVRAM / EEPROM, and has its own sound CPU driving only an OKI6925.  Video
 RAM Banking has also been changed.

 The actual Monsters World PCB is very close to the Speed Spin PCB but in terms
 of emulation the video etc. is closer to mitchell.cpp

******************************************************************************
Monsters World, from TCH (Spain)

Main CPU = Toshiba TMPZ84C00AP-6
Sound CPU = GS Z8400A PS - Z80A
OSC 12.000 MHz

Sound chip = Oki M6295

Graphics = TI 32005BWBL - TPC1020AFN-084C
OSC 10.000 MHz

ROMS

mw-1.rom = ST M27C4001    = Main CPU program
mw-2.rom = Intel D27256-1 = Sound CPU Program
mw-3.rom = AMD AM27C040   = Oki samples
mw-4.rom = ST M27C1001   \
mw-5.rom = TI TMS27C010A  |
mw-6.rom = ST M27C1001    | GFX
mw-7.rom = ST M27C1001   /
mw-8.rom = ST M27C1001 \
mw-9.rom = ST M27C1001 / GFX

*************************************************************************************/


#include "emu.h"

#include "kabuki.h"  // needed for decoding functions only

#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mitchell_state : public driver_device
{
public:
	mitchell_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_nvram(*this, "nvram"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_objram(*this, "objram", 0x1000, ENDIANNESS_LITTLE),
		m_databank(*this, "databank"),
		m_opbase(*this, "opbase"),
		m_opbank(*this, "opbank"),
		m_sys0(*this, "SYS0"),
		m_in(*this, "IN%u", 0U),
		m_dial_in(*this, "DIAL%u", 1U),
		m_key{ { *this, "KEY%u", 0U }, { *this, "KEY%u", 5U } } { }

	void mgakuen(machine_config &config);
	void marukin(machine_config &config);
	void pang(machine_config &config);
	void pangnv(machine_config &config);

	void init_mgakuen2();
	void init_block();
	void init_pangb();
	void init_qtono1();
	void init_mgakuen();
	void init_hatena();
	void init_spang();
	void init_cworld();
	void init_spangj();
	void init_qsangoku();
	void init_marukin();
	void init_pang();
	void init_sbbros();
	void init_pkladies();
	void init_blockbl();
	void init_dokaben();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	optional_device<nvram_device> m_nvram;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	memory_share_creator<uint8_t> m_objram; // Sprite RAM
	required_memory_bank m_databank;
	optional_memory_bank m_opbase;
	optional_memory_bank m_opbank;

	required_ioport m_sys0;
	optional_ioport_array<3> m_in;
	optional_ioport_array<2> m_dial_in;
	optional_ioport_array<10> m_key[2];

	// video-related
	tilemap_t *m_bg_tilemap;
	uint8_t m_flipscreen;
	uint8_t m_video_bank;
	uint8_t m_paletteram_bank;
	std::vector<uint8_t> m_paletteram;

	// misc
	uint8_t m_input_type;
	uint8_t m_dial[2];
	uint8_t m_dial_selected;
	uint8_t m_dir[2];
	uint8_t m_keymatrix;
	std::unique_ptr<uint8_t[]> m_decoded;
	uint8_t m_irq_source;

	uint8_t port5_r();
	void bankswitch_w(uint8_t data);
	uint8_t block_input_r(offs_t offset);
	void block_dial_control_w(uint8_t data);
	uint8_t mahjong_input_r(offs_t offset);
	uint8_t input_r(offs_t offset);
	void input_w(uint8_t data);
	void mgakuen_videoram_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t videoram_r(offs_t offset);
	void colorram_w(offs_t offset, uint8_t data);
	void gfxctrl_w(uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	uint8_t paletteram_r(offs_t offset);
	void eeprom_cs_w(uint8_t data);
	void eeprom_clock_w(uint8_t data);
	void eeprom_serial_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(mitchell_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void bootleg_decode();
	void configure_banks(void (*decode)(uint8_t *src, uint8_t *dst, int size));

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void mgakuen_map(address_map &map) ATTR_COLD;
	void mitchell_io_map(address_map &map) ATTR_COLD;
	void mitchell_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

// adds a Z80 as audio CPU and has an Oki M5205 instead of M6295
class spangbl_state : public mitchell_state
{
public:
	spangbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: mitchell_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_adpcm_select(*this, "adpcm_select"),
		m_soundbank(*this, "soundbank") { }

	void mstworld2(machine_config &config);
	void pangba(machine_config &config);
	void spangbl(machine_config &config);

	void init_spangbl();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_select;

	// memory pointers
	required_memory_bank m_soundbank;

	// sound-related
	bool m_sample_select;

	void sound_bankswitch_w(uint8_t data);
	void adpcm_int(int state);

	void mstworld2_io_map(address_map &map) ATTR_COLD;
	void pangba_sound_map(address_map &map) ATTR_COLD;
	void spangbl_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void spangbl_sound_map(address_map &map) ATTR_COLD;
};


// adds a Z80 as audio CPU to which the Oki M6295 is hooked up. No YM chip.
class mstworld_state : public mitchell_state
{
public:
	mstworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: mitchell_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu") { }

	void mstworld(machine_config &config);

	void init_mstworld();

private:
	required_device<cpu_device> m_audiocpu;

	void gfxctrl_w(uint8_t data);

	void io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

// has an Oki M5205 instead of an Oki M6295 without additional audio CPU
class pkladiesbl_state : public mitchell_state
{
public:
	pkladiesbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: mitchell_state(mconfig, type, tag),
		m_msm(*this, "msm") { }

	void pkladiesbl(machine_config &config);

	void init_pkladiesbl();

private:
	required_device<msm5205_device> m_msm;

	void io_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(mitchell_state::get_tile_info)
{
	uint8_t attr = m_colorram[tile_index];
	int code = m_videoram[2 * tile_index] + (m_videoram[2 * tile_index + 1] << 8);
	tileinfo.set(0,
			code,
			attr & 0x7f,
			(attr & 0x80) ? TILE_FLIPX : 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void mitchell_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mitchell_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap->set_transparent_pen(15);

	// Palette RAM
	m_paletteram.resize(2 * m_palette->entries());
	m_palette->basemem().set(m_paletteram, ENDIANNESS_LITTLE, 2);

	save_item(NAME(m_paletteram));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

/***************************************************************************
  OBJ / CHAR RAM HANDLERS (BANK 0 = CHAR, BANK 1=OBJ)
***************************************************************************/


void mitchell_state::mgakuen_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void mitchell_state::videoram_w(offs_t offset, uint8_t data)
{
	if (m_video_bank)
		m_objram[offset] = data;
	else
		mgakuen_videoram_w(offset, data);
}

uint8_t mitchell_state::videoram_r(offs_t offset)
{
	if (m_video_bank)
		return m_objram[offset];
	else
		return m_videoram[offset];
}

/***************************************************************************
  COLOUR RAM
****************************************************************************/

void mitchell_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

/***************************************************************************
  PALETTE HANDLERS (COLOURS: BANK 0 = 0x00-0x3f BANK 1=0x40-0xff)
****************************************************************************/

void mitchell_state::gfxctrl_w(uint8_t data)
{
	logerror("PC %04x: gfxctrl_w %02x\n", m_maincpu->pc(), data);

#if 0
	popmessage("%02x", data);
#endif

	// bit 0 is unknown (used, maybe back color enable?)

	// bit 1 is coin counter
	machine().bookkeeping().coin_counter_w(0, data & 2);

	// bit 2 is flip screen
	if (m_flipscreen != (data & 0x04))
	{
		m_flipscreen = data & 0x04;
		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}

	// bit 3 is unknown (used, e.g. marukin pulses it on the title screen)

	// bit 4 selects OKI M6295 bank, NOPed for the bootleg which don't have this sound chip
	if (m_oki != nullptr)
		m_oki->set_rom_bank((data >> 4) & 1);

	// bit 5 is palette RAM bank selector (doesn't apply to mgakuen)
	m_paletteram_bank = data & 0x20;

	/* bits 6 and 7 are unknown, used in several places. At first I thought */
	/* they were bg and sprites enable, but this screws up spang (screen flickers */
	/* every time you pop a bubble). However, not using them as enable bits screws */
	/* up marukin - you can see partially built up screens during attract mode. */
}

void mstworld_state::gfxctrl_w(uint8_t data)
{
	logerror("PC %04x: gfxctrl_w %02x\n", m_maincpu->pc(), data);

	// popmessage("%02x", data);

	// bit 0 is unknown (used, maybe back color enable?)

	// bit 1 is coin counter
	machine().bookkeeping().coin_counter_w(0, data & 2);

	// bit 2 is flip screen
	if (m_flipscreen != (data & 0x04))
	{
		m_flipscreen = data & 0x04;
		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}

	// bit 3 is unknown (used, e.g. marukin pulses it on the title screen)

	// bit 4 here does not select OKI M6295 bank: mstworld has its own z80 + sound banking

	// bit 5 is palette RAM bank selector (doesn't apply to mgakuen)
	m_paletteram_bank = data & 0x20;

	/* bits 6 and 7 are unknown, used in several places. At first I thought */
	/* they were bg and sprites enable, but this screws up spang (screen flickers */
	/* every time you pop a bubble). However, not using them as enable bits screws */
	/* up marukin - you can see partially built up screens during attract mode. */
}

void mitchell_state::paletteram_w(offs_t offset, uint8_t data)
{
	m_palette->write8(offset + (m_paletteram_bank ? 0x800 : 0x000), data);
}

uint8_t mitchell_state::paletteram_r(offs_t offset)
{
	return m_paletteram[offset + (m_paletteram_bank ? 0x800 : 0x000)];
}



/***************************************************************************

  Display refresh

***************************************************************************/

void mitchell_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// the last entry is not a sprite, we skip it otherwise spang shows a bubble moving diagonally across the screen
	for (int offs = 0x1000 - 0x40; offs >= 0; offs -= 0x20)
	{
		int code = m_objram[offs];
		int attr = m_objram[offs + 1];
		int color = attr & 0x0f;
		int sx = m_objram[offs + 3] + ((attr & 0x10) << 4);
		int sy = ((m_objram[offs + 2] + 8) & 0xff) - 8;
		code += (attr & 0xe0) << 3;
		if (m_flipscreen)
		{
			sx = 496 - sx;
			sy = 240 - sy;
		}
		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					code,
					color,
					m_flipscreen, m_flipscreen,
					sx, sy, 15);
	}
}

uint32_t mitchell_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  EEPROM
 *
 *************************************/

uint8_t mitchell_state::port5_r()
{
	/*  bits 0 and (sometimes) 3 are checked in the interrupt handler.
	    bit 3 is checked before updating the palette so it really seems to be vblank.
	    bit 0 may be vblank (or vblank irq flag) related too, but I'm not sure.
	    Many games require two interrupts per frame and for these bits to toggle,
	    otherwise music doesn't work.
	*/

	return (m_sys0->read() & 0xfe) | (m_irq_source & 1);
}

void mitchell_state::eeprom_cs_w(uint8_t data)
{
	m_eeprom->cs_write(data ? ASSERT_LINE : CLEAR_LINE);
}

void mitchell_state::eeprom_clock_w(uint8_t data)
{
	m_eeprom->clk_write(data ? ASSERT_LINE : CLEAR_LINE);
}

void mitchell_state::eeprom_serial_w(uint8_t data)
{
	m_eeprom->di_write(data & 1);
}


/*************************************
 *
 *  Bankswitch handling
 *
 *************************************/

void mitchell_state::bankswitch_w(uint8_t data)
{
	m_databank->set_entry(data & 0x0f);
	if(m_opbank)
		m_opbank->set_entry(data & 0x0f);
}

/*************************************
 *
 *  Input handling
 *
 *************************************/

uint8_t mitchell_state::block_input_r(offs_t offset)
{
	if (m_dial_selected)
	{
		int delta = (m_dial_in[offset]->read() - m_dial[offset]) & 0xff;

		if (delta & 0x80)
		{
			delta = (-delta) & 0xff;
			if (m_dir[offset])
			{
			// don't report movement on a direction change, otherwise it will stutter
				m_dir[offset] = 0;
				delta = 0;
			}
		}
		else if (delta > 0)
		{
			if (!m_dir[offset])
			{
			// don't report movement on a direction change, otherwise it will stutter
				m_dir[offset] = 1;
				delta = 0;
			}
		}
		if (delta > 0x3f)
			delta = 0x3f;

		return delta << 2;
	}
	else
	{
		int res = m_in[offset + 1]->read() & 0xf7;

		if (m_dir[offset])
			res |= 0x08;

		return res;
	}
}

void mitchell_state::block_dial_control_w(uint8_t data)
{
	if (data == 0x08)
	{
		// reset the dial counters
		m_dial[0] = m_dial_in[0]->read();
		m_dial[1] = m_dial_in[1]->read();
	}
	else if (data == 0x80)
		m_dial_selected = 0;
	else
		m_dial_selected = 1;
}


uint8_t mitchell_state::mahjong_input_r(offs_t offset)
{
	for (int i = 0; i < 5; i++)
	{
		if (m_keymatrix & (0x80 >> i))
			return m_key[offset][i]->read();
	}

	return 0xff;
}


uint8_t mitchell_state::input_r(offs_t offset)
{
	switch (m_input_type)
	{
		case 0:
		default:
			return m_in[offset]->read();
		case 1:     // Mahjong games
			if (offset)
				return mahjong_input_r(offset - 1);
			else
				return m_in[0]->read();
		case 2:     // Block Block - dial control
			if (offset)
				return block_input_r(offset - 1);
			else
				return m_in[0]->read();
		case 3:     // Super Pang - simulate START 1 press to initialize EEPROM
			return m_in[offset]->read();
	}
}


void mitchell_state::input_w(uint8_t data)
{
	switch (m_input_type)
	{
		case 0:
		default:
			logerror("PC %04x: write %02x to port 01\n", m_maincpu->pc(), data);
			break;
		case 1:
			m_keymatrix = data;
			break;
		case 2:
			block_dial_control_w(data);
			break;
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void mitchell_state::mgakuen_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_databank);
	map(0xc000, 0xc7ff).ram().w(m_palette, FUNC(palette_device::write8));
	map(0xc800, 0xcfff).ram().w(FUNC(mitchell_state::colorram_w)).share(m_colorram); // Attribute RAM
	map(0xd000, 0xdfff).ram().w(FUNC(mitchell_state::mgakuen_videoram_w)).share(m_videoram);
	map(0xe000, 0xefff).ram(); // Work RAM
	map(0xf000, 0xffff).ram().share(m_objram);
}

void mitchell_state::mitchell_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_databank);
	map(0xc000, 0xc7ff).rw(FUNC(mitchell_state::paletteram_r), FUNC(mitchell_state::paletteram_w)); // Banked palette RAM
	map(0xc800, 0xcfff).ram().w(FUNC(mitchell_state::colorram_w)).share(m_colorram); // Attribute RAM
	map(0xd000, 0xdfff).rw(FUNC(mitchell_state::videoram_r), FUNC(mitchell_state::videoram_w)).share(m_videoram); // Banked char / OBJ RAM
	map(0xe000, 0xffff).ram().share("nvram"); // Work RAM
}

void mitchell_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_opbase);
	map(0x8000, 0xbfff).bankr(m_opbank);
	map(0xe000, 0xffff).ram().share("nvram"); // Work RAM
}

void mitchell_state::mitchell_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(mitchell_state::gfxctrl_w));   // Palette bank, layer enable, coin counters, more
	map(0x00, 0x02).r(FUNC(mitchell_state::input_r));           // The Mahjong games and Block Block need special input treatment
	map(0x01, 0x01).w(FUNC(mitchell_state::input_w));
	map(0x02, 0x02).w(FUNC(mitchell_state::bankswitch_w));
	map(0x03, 0x03).w("ymsnd", FUNC(ym2413_device::data_w));
	map(0x04, 0x04).w("ymsnd", FUNC(ym2413_device::address_w));
	map(0x05, 0x05).r(FUNC(mitchell_state::port5_r)).w(m_oki, FUNC(okim6295_device::write));
	map(0x06, 0x06).noprw();                     // watchdog? IRQ ack? video buffering?
	map(0x07, 0x07).lw8(NAME([this] (uint8_t data) { m_video_bank = data; }));
	map(0x08, 0x08).w(FUNC(mitchell_state::eeprom_cs_w));
	map(0x10, 0x10).w(FUNC(mitchell_state::eeprom_clock_w));
	map(0x18, 0x18).w(FUNC(mitchell_state::eeprom_serial_w));
}

// spangbl
void spangbl_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_databank).nopw();
	map(0xc000, 0xc7ff).rw(FUNC(spangbl_state::paletteram_r), FUNC(spangbl_state::paletteram_w)); // Banked palette RAM
	map(0xc800, 0xcfff).ram().w(FUNC(spangbl_state::colorram_w)).share(m_colorram); // Attribute RAM
	map(0xd000, 0xdfff).rw(FUNC(spangbl_state::videoram_r), FUNC(spangbl_state::videoram_w)).share(m_videoram); // Banked char / OBJ RAM
	map(0xe000, 0xffff).ram().share("nvram");     // Work RAM
}

void spangbl_state::spangbl_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x02).r(FUNC(spangbl_state::input_r));
	map(0x00, 0x00).w(FUNC(spangbl_state::gfxctrl_w));    // Palette bank, layer enable, coin counters, more
	map(0x02, 0x02).w(FUNC(spangbl_state::bankswitch_w));
	map(0x03, 0x03).portr("DSW1").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x05, 0x05).portr("SYS0");
	map(0x06, 0x06).nopw();    // watchdog? irq ack?
	map(0x07, 0x07).lw8(NAME([this] (uint8_t data) { m_video_bank = data; }));
	map(0x08, 0x08).w(FUNC(spangbl_state::eeprom_cs_w));
	map(0x10, 0x10).w(FUNC(spangbl_state::eeprom_clock_w));
	map(0x18, 0x18).w(FUNC(spangbl_state::eeprom_serial_w));
}

void spangbl_state::mstworld2_io_map(address_map &map)
{
	spangbl_io_map(map);
	map(0x07, 0x07).lw8(NAME([this] (uint8_t data) { m_video_bank = data & 0x01; })); // for some reason mstworld2 freaks out if this isn't masked
}

void spangbl_state::sound_bankswitch_w(uint8_t data)
{
	m_msm->reset_w(BIT(data, 3));

	m_soundbank->set_entry(data & 7);
}

void spangbl_state::spangbl_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xe000, 0xe000).w(FUNC(spangbl_state::sound_bankswitch_w));
	map(0xe400, 0xe400).w(m_adpcm_select, FUNC(ls157_device::ba_w));
	map(0xec00, 0xec01).w("ymsnd", FUNC(ym2413_device::write));
	map(0xf000, 0xf4ff).ram();
	map(0xf800, 0xf800).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void spangbl_state::pangba_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xe000, 0xe000).w(FUNC(spangbl_state::sound_bankswitch_w));
	map(0xe400, 0xe400).w(m_adpcm_select, FUNC(ls157_device::ba_w));
	map(0xec00, 0xec01).w("ymsnd", FUNC(ym3812_device::write));
	map(0xf000, 0xf4ff).ram();
	map(0xf800, 0xf800).r("soundlatch", FUNC(generic_latch_8_device::read));
}


/**** Monsters World ****/

void mstworld_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).lw8(NAME([this] (uint8_t data) { m_oki->set_rom_bank(data & 3); }));
	map(0x9800, 0x9800).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void mstworld_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w(FUNC(mstworld_state::gfxctrl_w));   // Palette bank, layer enable, coin counters, more
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2").w(FUNC(mstworld_state::bankswitch_w));
	map(0x03, 0x03).portr("DSW0").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x04, 0x04).portr("DSW1");
	map(0x05, 0x05).portr("SYS0");
	map(0x06, 0x06).portr("DSW2");
	map(0x06, 0x06).nopw();        // watchdog? irq ack?
	map(0x07, 0x07).lw8(NAME([this] (uint8_t data) { m_video_bank = data & 0x01; })); // for some reason mstworld freaks out if this isn't masked
}

void pkladiesbl_state::io_map(address_map &map) // TODO: check everything, where is the MSM5205 hooked up?
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w(FUNC(pkladiesbl_state::gfxctrl_w));   // Palette bank, layer enable, coin counters, more
	map(0x01, 0x01).portr("IN1").w("ymsnd", FUNC(ym2413_device::address_w)); // TODO: hold buttons are here, multiplexed but not in the same way as the original
	map(0x02, 0x02).portr("IN2").w(FUNC(pkladiesbl_state::bankswitch_w));
	map(0x03, 0x03).portr("DSW0");
	map(0x04, 0x04).portr("DSW1");
	map(0x05, 0x05).r(FUNC(pkladiesbl_state::port5_r));
	map(0x06, 0x06).noprw();                     // watchdog? IRQ ack? video buffering?
	map(0x07, 0x07).lw8(NAME([this] (uint8_t data) { m_video_bank = data; }));
	map(0x08, 0x08).w(FUNC(pkladiesbl_state::eeprom_cs_w));
	map(0x09, 0x09).w("ymsnd", FUNC(ym2413_device::data_w));
	map(0x10, 0x10).w(FUNC(pkladiesbl_state::eeprom_clock_w));
	map(0x18, 0x18).w(FUNC(pkladiesbl_state::eeprom_serial_w));
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mj_common )
	PORT_START("SYS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // USED - handled in port5_r
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_A )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_B )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_C )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_D )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( mgakuen )
	PORT_INCLUDE( mj_common )

	PORT_MODIFY("SYS0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    // not PORT_VBLANK

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW0:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x08, 0x08, "Rules" )         PORT_DIPLOCATION("DSW0:4")
	PORT_DIPSETTING(    0x08, "Kantou" )
	PORT_DIPSETTING(    0x00, "Kansai" )
	PORT_DIPNAME( 0x10, 0x00, "Harness Type" )      PORT_DIPLOCATION("DSW0:5")
	PORT_DIPSETTING(    0x10, "Generic" )
	PORT_DIPSETTING(    0x00, "Royal Mahjong" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW0:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Freeze" )            PORT_DIPLOCATION("DSW0:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )         PORT_DIPLOCATION("DSW0:8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Player 1 Skill" )        PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "Weak" )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Strong" )
	PORT_DIPSETTING(    0x00, "Very Strong" )
	PORT_DIPNAME( 0x0c, 0x0c, "Player 2 Skill" )        PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x0c, "Weak" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Strong" )
	PORT_DIPSETTING(    0x00, "Very Strong" )
	PORT_DIPNAME( 0x10, 0x00, "Music" )         PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Help Mode" )         PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( marukin )
	PORT_INCLUDE( mj_common )

	PORT_MODIFY("SYS0")
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( pkladies )
	// same unknown ports as the mahjong games, so we include the following
	PORT_INCLUDE( marukin )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )    // same as the service mode farther down

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Deal")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold E / Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold A / Cancel")

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold B / Cancel")

	PORT_MODIFY("KEY2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Flip")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold C / Cancel")

	PORT_MODIFY("KEY3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold D / Cancel")

	PORT_MODIFY("KEY4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// P2 inputs seem to be a leftover from mgakuen
	PORT_MODIFY("KEY5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Deal")
	// PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_PLAYER(2) PORT_NAME("Hold E")
	// PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_PLAYER(2) PORT_NAME("Hold A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_PLAYER(2) PORT_NAME("Hold B")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Flip")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_PLAYER(2) PORT_NAME("Hold C")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY8")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_PLAYER(2) PORT_NAME("Hold D")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY9")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( pkladiesbl )
	PORT_START("SYS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // USED - handled in port5_r
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) // ok
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) // ok

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) // ok
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) // ok
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) // ok
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) // ok

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // this seems flipscreen?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) // this and the following seem to influence both Winning Percentage Level (although only levels 1, 3, 5 and 7) and Game BGM
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x04, "Maximum Hands" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x0b, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0d, "13" )
	PORT_DIPSETTING(    0x0e, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pang )
	PORT_START("SYS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // USED - handled in port5_r
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_OPTIONAL // "Shot B" in service mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_OPTIONAL // "Shot B" in service mode
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( pangdsw )
	PORT_INCLUDE( pang )

// the settings are shown if entering test mode. The game always respects the dip setting, changing the values in test mode has no effect
// it appears the bootleggers didn't care to add dips for flipscreen, free play and extend values (or at least they haven't been found yet)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:6,7,8")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( spangbl )
	PORT_INCLUDE( pang )

	PORT_MODIFY("SYS0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // this bootleg doesn't seem to allow entering test mode. It has a dip bank for settings, instead.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be high for game to boot..

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:6,7,8")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mstworld )
	// this port may not have the same role
	PORT_START("SYS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // don't think this one matters..
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // if not active high gfx aren't copied for game screen?! .. is this instead of a bit in port 5?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW0")      // coinage seems to be in here..
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A 1Coin 4Credits / B 1Coin 4Credits" )
	PORT_DIPSETTING(    0x02, "A 1Coin 3Credits / B 1Coin 3Credits" )
	PORT_DIPSETTING(    0x01, "A 1Coin 2Credits / B 1Coin 2Credits" )
	PORT_DIPSETTING(    0x00, "A 1Coin 1Credit / B 1Coin 4Credits" )
	PORT_DIPSETTING(    0x04, "A 2Coins 1Credit / B 1Coin 2Credits" )
	PORT_DIPSETTING(    0x05, "A 2Coins 1Credit / B 1Coin 3Credits" )
	PORT_DIPSETTING(    0x06, "A 3Coins 1Credit / B 1Coin 2Credits" )
	PORT_DIPSETTING(    0x07, "A 4Coins 1Credit / B 1Coin 1Credit" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "ds1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "ds2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mstworld2 )
	PORT_INCLUDE( mstworld )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "ds0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A 1Coin 4Credits / B 1Coin 4Credits" )
	PORT_DIPSETTING(    0x02, "A 1Coin 3Credits / B 1Coin 3Credits" )
	PORT_DIPSETTING(    0x01, "A 1Coin 2Credits / B 1Coin 2Credits" )
	PORT_DIPSETTING(    0x00, "A 1Coin 1Credit / B 1Coin 4Credits" )
	PORT_DIPSETTING(    0x04, "A 2Coins 1Credit / B 1Coin 2Credits" )
	PORT_DIPSETTING(    0x05, "A 2Coins 1Credit / B 1Coin 3Credits" )
	PORT_DIPSETTING(    0x06, "A 3Coins 1Credit / B 1Coin 2Credits" )
	PORT_DIPSETTING(    0x07, "A 4Coins 1Credit / B 1Coin 1Credit" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( qtono1 )
	PORT_START("SYS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // USED - handled in port5_r
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )    // same as the service mode farther down
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( block )
	PORT_START("SYS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // USED - handled in port5_r
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM )    // dial direction
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM )    // dial direction
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( blockjoy )
	PORT_START("SYS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // USED - handled in port5_r
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNKNOWN )    // unused?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM )    // dial direction
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM )    // dial direction
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	32768,  // 32768 characters
	4,      // 4 bits per pixel
	{ 32768*16*8+4, 32768*16*8+0,4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    // every char takes 16 consecutive bytes
};

static const gfx_layout marukin_charlayout =
{
	8,8,    // 8*8 characters
	65536,  // 65536 characters
	4,      // 4 bits per pixel
	{ 3*4, 2*4, 1*4, 0*4 },
	{ 0, 1, 2, 3, 16+0, 16+1, 16+2, 16+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    // every char takes 32 consecutive bytes
};


static const gfx_layout pkladiesbl_charlayout =
{
	8,8,    // 8*8 characters
	65536,  // 65536 characters
	4,      // 4 bits per pixel
	{ 0*8, 1*8, 2*8, 3*8 },
	{ 0, 1, 2, 3, 4,5,6,7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    // every char takes 32 consecutive bytes
};


static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	2048,   // 2048 sprites
	4,      // 4 bits per pixel
	{ 2048*64*8+4, 2048*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    // every sprite takes 64 consecutive bytes
};

static GFXDECODE_START( gfx_mgakuen )
	GFXDECODE_ENTRY( "chars",   0, marukin_charlayout, 0,  64 ) // colors 0-1023
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,       0,  16 ) // colors 0- 255
GFXDECODE_END

static GFXDECODE_START( gfx_marukin )
	GFXDECODE_ENTRY( "chars",   0, marukin_charlayout, 0, 128 ) // colors 0-2047
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,       0,  16 ) // colors 0- 255
GFXDECODE_END

static GFXDECODE_START( gfx_pkladiesbl )
	GFXDECODE_ENTRY( "chars",   0, pkladiesbl_charlayout, 0, 128 ) // colors 0-2047
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,       0,  16 ) // colors 0- 255
GFXDECODE_END

static GFXDECODE_START( gfx_mitchell )
	GFXDECODE_ENTRY( "chars",   0, charlayout,     0, 128 ) // colors 0-2047
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,   0,  16 ) // colors 0- 255
GFXDECODE_END

static const gfx_layout mstworld_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(2,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(1,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout mstworld_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0 },
	{ 0,1,2,3,8,9,10,11,
		16*16+0,16*16+1,16*16+2,16*16+3,16*16+8,16*16+9,16*16+10,16*16+11 },

	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
	8*16+0*16,8*16+1*16,8*16+2*16,8*16+3*16,8*16+4*16,8*16+5*16,8*16+6*16,8*16+7*16},
	32*16
};


static GFXDECODE_START( gfx_mstworld )
	GFXDECODE_ENTRY( "chars",   0, mstworld_charlayout,   0x000, 0x40 )
	GFXDECODE_ENTRY( "sprites", 0, mstworld_spritelayout, 0x000, 0x40 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mitchell_state::machine_start()
{
	save_item(NAME(m_dial_selected));
	save_item(NAME(m_keymatrix));
	save_item(NAME(m_dir));
	save_item(NAME(m_dial));
	save_item(NAME(m_irq_source));
}

void spangbl_state::machine_start()
{
	m_soundbank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
	m_soundbank->set_entry(0);

	save_item(NAME(m_sample_select));
	save_item(NAME(m_irq_source));
}

void mitchell_state::machine_reset()
{
	m_dial_selected = 0;
	m_dial[0] = 0;
	m_dial[1] = 0;
	m_dir[0] = 0;
	m_dir[1] = 0;
	m_keymatrix = 0;
}

void spangbl_state::machine_reset()
{
	mitchell_state::machine_reset();

	m_sample_select = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(mitchell_state::mitchell_irq)
{
	int scanline = param;

	if (scanline == 240 || scanline == 0)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);

		m_irq_source = (scanline == 240);
	}
}

void mitchell_state::mgakuen(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(16'000'000) / 2); // probably same clock as the other Mitchell hardware games
	m_maincpu->set_addrmap(AS_PROGRAM, &mitchell_state::mgakuen_map);
	m_maincpu->set_addrmap(AS_IO, &mitchell_state::mitchell_io_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(mitchell_state::mitchell_irq), "screen", 0, 1);

	EEPROM_93C46_16BIT(config, m_eeprom);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, (64-8)*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(mitchell_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mgakuen);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 1024); // less colors than the others

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH); // probably same clock as the other Mitchell hardware games
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.50);

	ym2413_device &ymsnd(YM2413(config, "ymsnd", XTAL(16'000'000) / 4)); // probably same clock as the other Mitchell hardware games
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void mitchell_state::pang(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(16'000'000 )/ 2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &mitchell_state::mitchell_map);
	m_maincpu->set_addrmap(AS_IO, &mitchell_state::mitchell_io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &mitchell_state::decrypted_opcodes_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(mitchell_state::mitchell_irq), "screen", 0, 1);

	EEPROM_93C46_16BIT(config, m_eeprom);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.42);   // verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, (64-8)*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(mitchell_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mitchell);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH); // verified on PCB
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.30);

	ym2413_device &ymsnd(YM2413(config, "ymsnd", XTAL(16'000'000) / 4)); // verified on PCB
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void mitchell_state::pangnv(machine_config &config)
{
	pang(config);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


static const gfx_layout blcharlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,2),  // 32768 characters
	4,      // 4 bits per pixel
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0,8, 0 },
	{ 0, 1, 2, 3, 4,5,6,7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    // every char takes 16 consecutive bytes
};


static GFXDECODE_START( gfx_spangbl )
	GFXDECODE_ENTRY( "chars", 0, blcharlayout,   0, 128 ) // colors 0-2047
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,   0,  16 ) // colors 0- 255
GFXDECODE_END

static GFXDECODE_START( gfx_mstworld2 )
	GFXDECODE_ENTRY( "chars", 0, blcharlayout,          0, 0x80 )
	GFXDECODE_ENTRY( "sprites", 0, mstworld_spritelayout, 0, 0x40 )
GFXDECODE_END

void spangbl_state::adpcm_int(int state)
{
	if (!state)
		return;

	m_sample_select = !m_sample_select;
	m_adpcm_select->select_w(m_sample_select);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, m_sample_select);
}

void spangbl_state::spangbl(machine_config &config)
{
	pangnv(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &spangbl_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &spangbl_state::spangbl_io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &spangbl_state::decrypted_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(spangbl_state::irq0_line_hold));

	config.device_remove("scantimer");

	Z80(config, m_audiocpu, 4000000); // Z80A CPU; clock unknown
	m_audiocpu->set_addrmap(AS_PROGRAM, &spangbl_state::spangbl_sound_map);

	m_gfxdecode->set_info(gfx_spangbl);

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	config.device_remove("oki");
	MSM5205(config, m_msm, 400000); // clock and prescaler unknown
	m_msm->vck_legacy_callback().set(FUNC(spangbl_state::adpcm_int));  // controls music as well as ADCPM rate
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);

	LS157(config, m_adpcm_select, 0);
	m_adpcm_select->out_callback().set("msm", FUNC(msm5205_device::data_w));
}

void spangbl_state::pangba(machine_config &config)
{
	spangbl(config);
	m_audiocpu->set_addrmap(AS_PROGRAM, &spangbl_state::pangba_sound_map);

	YM3812(config.replace(), "ymsnd", 4000000).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void spangbl_state::mstworld2(machine_config &config)
{
	spangbl(config);

	m_maincpu->set_addrmap(AS_IO, &spangbl_state::mstworld2_io_map);

	m_gfxdecode->set_info(gfx_mstworld2);
}

void mstworld_state::mstworld(machine_config &config)
{
	// basic machine hardware
	/* it doesn't glitch with the clock speed set to 4x normal, however this is incorrect..
	  the interrupt handling (and probably various irq flags / vbl flags handling etc.) is
	  more likely wrong.. the game appears to run too fast anyway .. */
	Z80(config, m_maincpu, 6000000 * 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &mstworld_state::mitchell_map);
	m_maincpu->set_addrmap(AS_IO, &mstworld_state::io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &mstworld_state::decrypted_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(mstworld_state::irq0_line_hold));

	Z80(config, m_audiocpu, 6000000);   // 6 MHz?
	m_audiocpu->set_addrmap(AS_PROGRAM, &mstworld_state::sound_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, (64-8)*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(mstworld_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mstworld);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	OKIM6295(config, m_oki, 990000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void mitchell_state::marukin(machine_config &config)
{
	pang(config);

	subdevice<screen_device>("screen")->set_refresh_hz(60);

	m_gfxdecode->set_info(gfx_marukin);
}

/*

Poker Ladies bootleg made by Playmark (PM stickers and pcb layout is same as many others Playmark bootlegs)
PCB is jamma standard while the original is mahjong pinout and require matrix inputs.
The bootleg has some gfx glitches (flickering of the text) and the colour of the background is totally black.

Encrypted custom z80 in epoxy block. Clock is 12mhz/2

YM2413 clock is 3.75mhz. The 7.5mhz clock which is divided by 2 by a 74ls74 before going to the YM2413 is too
difficult to follow.
It is derived from the 10mhz pixel clock since shorting it the video goes out of sync and the music change in pitch/speed.

Oki5205 clock is 384khz (resonator)

Vsync is 59.09hz

*/

void pkladiesbl_state::pkladiesbl(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &pkladiesbl_state::mitchell_map);
	m_maincpu->set_addrmap(AS_IO, &pkladiesbl_state::io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &pkladiesbl_state::decrypted_opcodes_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(pkladiesbl_state::mitchell_irq), "screen", 0, 1);

	EEPROM_93C46_16BIT(config, m_eeprom);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.09); // verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, (64-8)*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(pkladiesbl_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pkladiesbl);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	MSM5205(config, m_msm, 384000);
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);

	ym2413_device &ymsnd(YM2413(config, "ymsnd", 3750000)); // verified on PCB, read the comments
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mgakuen )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "mg-1.1j",      0x00000, 0x08000, CRC(bf02ea6b) SHA1(bb1f5fbb211a5ed181f1afbba6b39737639d3ee7) )
	ROM_LOAD( "mg-2.1l",      0x10000, 0x20000, CRC(64141b0c) SHA1(2de6bcd5cf2c042e5bf5c294dd7625393e99682b) )

	ROM_REGION( 0x200000, "chars", 0 )
	ROM_LOAD( "mg-1.13h",     0x000000, 0x80000, CRC(fd6a0805) SHA1(f3d4d402dd96b8e4297a074b01d803cac16ac0d3) )
	ROM_LOAD( "mg-2.14h",     0x080000, 0x80000, CRC(e26e871e) SHA1(00f9642ced5f1795e02b357a06deee3d093f6dc0) )
	ROM_LOAD( "mg-3.16h",     0x100000, 0x80000, CRC(dd781d9a) SHA1(db5568be7e5fc15497b979451c65d8448063e04b) )
	ROM_LOAD( "mg-4.17h",     0x180000, 0x80000, CRC(97afcc79) SHA1(a84ddf089db7d26a0043815648f1674b240b8289) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "mg-6.4l",      0x000000, 0x20000, CRC(34594e62) SHA1(a28493fc120ddfa6b51eeb3c111cc611cab54332) )
	ROM_LOAD( "mg-7.6l",      0x020000, 0x20000, CRC(f304c806) SHA1(a803a7be8702874fb547624be621a55f6ef5be1c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mg-5.1c",      0x00000, 0x80000, CRC(170332f1) SHA1(bc60f144a224f348fd5b8c0207e18a881f739fc1) )  // banked
ROM_END

ROM_START( 7toitsu )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "mc01.1j",      0x00000, 0x08000, CRC(0bebe45f) SHA1(24fadffd0033565441a75f36e2cb085a37e0f0e5) )
	ROM_LOAD( "mc02.1l",      0x10000, 0x20000, CRC(375378b0) SHA1(cbb5db5fda1d87902b22130243d579cb28803707) )

	ROM_REGION( 0x200000, "chars", 0 )
	ROM_LOAD( "mg-1.13h",     0x000000, 0x80000, CRC(fd6a0805) SHA1(f3d4d402dd96b8e4297a074b01d803cac16ac0d3) )
	ROM_LOAD( "mg-2.14h",     0x080000, 0x80000, CRC(e26e871e) SHA1(00f9642ced5f1795e02b357a06deee3d093f6dc0) )
	ROM_LOAD( "mg-3.16h",     0x100000, 0x80000, CRC(dd781d9a) SHA1(db5568be7e5fc15497b979451c65d8448063e04b) )
	ROM_LOAD( "mg-4.17h",     0x180000, 0x80000, CRC(97afcc79) SHA1(a84ddf089db7d26a0043815648f1674b240b8289) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "mc06.4l",      0x000000, 0x20000, CRC(0ef83926) SHA1(850b382d919c86ae09d802d5183edd37c81e7c97) )
	ROM_LOAD( "mc07.6l",      0x020000, 0x20000, CRC(59f9ffb1) SHA1(1c225a526860637a713d4b8add2fbc0a17c0a854) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mg-5.1c",      0x00000, 0x80000, CRC(170332f1) SHA1(bc60f144a224f348fd5b8c0207e18a881f739fc1) )  // banked
ROM_END

ROM_START( mgakuen2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "mg2-xf.1j",    0x00000, 0x08000, CRC(c8165d2d) SHA1(95146e293b2e005c4015590811119a4070dda65b) )
	ROM_LOAD( "mg2-y.1l",     0x10000, 0x20000, CRC(75bbcc14) SHA1(52ec279fda131c8de06d8c940df12d61ec6881cc) )
	ROM_LOAD( "mg2-z.3l",     0x30000, 0x20000, CRC(bfdba961) SHA1(75045562edbdef1eb599d6a6bfc4247c33c11258) )

	ROM_REGION( 0x200000, "chars", 0 )
	ROM_LOAD( "mg2-a.13h",    0x000000, 0x80000, CRC(31a0c55e) SHA1(2a6bd9f9d1fee17fd4798ba9aad05e05b3cfb210) )
	ROM_LOAD( "mg2-b.14h",    0x080000, 0x80000, CRC(c18488fa) SHA1(42efb2a51305dce86ec721c747ee13d82c4f6cd6) )
	ROM_LOAD( "mg2-c.16h",    0x100000, 0x80000, CRC(9425b364) SHA1(44373e137e0b820ad705ef1c299a9d31a1e8d0ca) )
	ROM_LOAD( "mg2-d.17h",    0x180000, 0x80000, CRC(6cc9eeba) SHA1(ef4a4f44abacc8b08576846d514765ac2eadf9a6) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "mg2-f.4l",     0x000000, 0x20000, CRC(3172c9fe) SHA1(7012bf2eb70c70b08f0204a4766dd8fce0bcc135) )
	ROM_LOAD( "mg2-g.6l",     0x020000, 0x20000, CRC(19b8b61c) SHA1(a9f5cea6f4788886719f5f9301ef172978b3b9a2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mg2-e.1c",     0x00000, 0x80000, CRC(70fd0809) SHA1(7f85fc5f575c925c3246b45fc041f57fc3eb7cc8) )  // banked
ROM_END

ROM_START( pkladies )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "pko-prg1.14f", 0x00000, 0x08000, CRC(86585a94) SHA1(067791da20556e6c47de26fbf85389d92f9709db) )
	ROM_LOAD( "pko-prg2.15f", 0x10000, 0x10000, CRC(86cbe82d) SHA1(3997a642004d1226cfce0f590123d4e407edf094) )

	ROM_REGION( 0x200000, "chars", 0 )
	ROM_LOAD16_BYTE( "pko-001.8h",   0x000000, 0x80000, CRC(1ead5d9b) SHA1(ac9b294ce1fcfb994f7c06e0e3f0ec8d86f2d908) )
	ROM_LOAD16_BYTE( "pko-003.8j",   0x000001, 0x80000, CRC(339ab4e6) SHA1(0dbe6801e72df1226a4df3f6911523c95cd2ac6a) )
	ROM_LOAD16_BYTE( "pko-002.9h",   0x100000, 0x80000, CRC(1cf02586) SHA1(d78fa4824c00b88049c36c1525031f3b8b5d36c8) )
	ROM_LOAD16_BYTE( "pko-004.9j",   0x100001, 0x80000, CRC(09ccb442) SHA1(c8deb7c29f75ad61237c8b737caded58f21f3bba) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "pko-chr1.2j",  0x000000, 0x20000, CRC(31ce33cd) SHA1(9e8cea7625e7436a8480c4114c9148c67ccbf247) )
	ROM_LOAD( "pko-chr2.3j",  0x020000, 0x20000, CRC(ad7e055f) SHA1(062f4d3b6e11ddce035bd0d5a279dc4489149cc4) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pko-voi1.2d",  0x00000, 0x20000, CRC(07e0f531) SHA1(315715f7686ae09c446029da36faec5bab7fcaf0) )
	ROM_LOAD( "pko-voi2.3d",  0x20000, 0x20000, CRC(18398bf6) SHA1(9e9ab85383350d01ba597951a48f18ecee1f46c6) )
ROM_END

ROM_START( pkladiesl )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "pk05.14f",     0x00000, 0x08000, CRC(ea1740a6) SHA1(eafd3fb0056a648dfc67b5d0a1dc93c4262e2a8b) )
	ROM_LOAD( "pk06.15f",     0x10000, 0x20000, CRC(3078ff5e) SHA1(5d91d68a07a968ee59f693841da165833a9fcf08) )  // larger than pkladies - 2nd half unused?

	ROM_REGION( 0x200000, "chars", 0 )
	ROM_LOAD16_BYTE( "pko-001.8h",   0x000000, 0x80000, CRC(1ead5d9b) SHA1(ac9b294ce1fcfb994f7c06e0e3f0ec8d86f2d908) )
	ROM_LOAD16_BYTE( "pko-003.8j",   0x000001, 0x80000, CRC(339ab4e6) SHA1(0dbe6801e72df1226a4df3f6911523c95cd2ac6a) )
	ROM_LOAD16_BYTE( "pko-002.9h",   0x100000, 0x80000, CRC(1cf02586) SHA1(d78fa4824c00b88049c36c1525031f3b8b5d36c8) )
	ROM_LOAD16_BYTE( "pko-004.9j",   0x100001, 0x80000, CRC(09ccb442) SHA1(c8deb7c29f75ad61237c8b737caded58f21f3bba) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "pko-chr1.2j",  0x000000, 0x20000, CRC(31ce33cd) SHA1(9e8cea7625e7436a8480c4114c9148c67ccbf247) )
	ROM_LOAD( "pko-chr2.3j",  0x020000, 0x20000, CRC(ad7e055f) SHA1(062f4d3b6e11ddce035bd0d5a279dc4489149cc4) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pko-voi1.2d",  0x00000, 0x20000, CRC(07e0f531) SHA1(315715f7686ae09c446029da36faec5bab7fcaf0) )
	ROM_LOAD( "pko-voi2.3d",  0x20000, 0x20000, CRC(18398bf6) SHA1(9e9ab85383350d01ba597951a48f18ecee1f46c6) )
ROM_END

ROM_START( pkladiesla )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "pk05.14f",       0x00000, 0x08000, CRC(fa18e16a) SHA1(05fff3335a55b9ebf13a0bc89216f00fba6b6b6d) )
	ROM_LOAD( "pk06.15f",       0x10000, 0x10000, CRC(a2fb7646) SHA1(778d3c1348efe6e46aed4ce968826ce73e320187) )

	ROM_REGION( 0x200000, "chars", 0 )
	ROM_LOAD16_BYTE( "pk0-001-t18.8h",   0x000000, 0x80000, CRC(1ead5d9b) SHA1(ac9b294ce1fcfb994f7c06e0e3f0ec8d86f2d908) )
	ROM_LOAD16_BYTE( "pk0-003-t20.8j",   0x000001, 0x80000, CRC(339ab4e6) SHA1(0dbe6801e72df1226a4df3f6911523c95cd2ac6a) )
	ROM_LOAD16_BYTE( "pk0-002-t19.9h",   0x100000, 0x80000, CRC(1cf02586) SHA1(d78fa4824c00b88049c36c1525031f3b8b5d36c8) )
	ROM_LOAD16_BYTE( "pk0-004-t21.9j",   0x100001, 0x80000, CRC(09ccb442) SHA1(c8deb7c29f75ad61237c8b737caded58f21f3bba) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "pk16.2j",  0x000000, 0x20000, CRC(31ce33cd) SHA1(9e8cea7625e7436a8480c4114c9148c67ccbf247) )
	ROM_LOAD( "pk17.3j",  0x020000, 0x20000, CRC(ad7e055f) SHA1(062f4d3b6e11ddce035bd0d5a279dc4489149cc4) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pk01.2d",  0x00000, 0x20000, CRC(07e0f531) SHA1(315715f7686ae09c446029da36faec5bab7fcaf0) )
	ROM_LOAD( "pk02.3d",  0x20000, 0x20000, CRC(18398bf6) SHA1(9e9ab85383350d01ba597951a48f18ecee1f46c6) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.14a", 0x0000, 0x0080, CRC(6856c4aa) SHA1(f767773f2a5930890273c992b044094c023affb7) )

	ROM_REGION( 0x400, "pals", 0 ) // not used by the emulation
	ROM_LOAD( "pal16l8cn.poker.10g", 0x000, 0x117, CRC(8e592f22) SHA1(abec57c811ea647768c96fd4258922c4662c005e) )
	ROM_LOAD( "epl16p8bp.pl-c4.5j",  0x200, 0x117, CRC(6cae00f7) SHA1(9826bed70d9ee223a1e4d72b765deb34350bed49) )
ROM_END

ROM_START( pkladiesbl )
	ROM_REGION( 0x50000*2, "maincpu", 0 )
	// only 1.ic112 is encrypted (only opcodes). Encryption scheme seems to involve XORs and bitswaps, based on addresses.
	ROM_LOAD( "1.ic112", 0x50000, 0x08000, CRC(ca4cfaf9) SHA1(97ad3c526e4494f347db45c986ba23aff07e6321) )
	ROM_CONTINUE(0x00000,0x08000)
	ROM_LOAD( "2.ic126", 0x60000, 0x10000, CRC(5c73e9b6) SHA1(5fbfb4c79e2df8e1edd3f29ac63f9961dd3724b1) )
	ROM_CONTINUE(0x10000,0x10000)

	ROM_REGION( 0x240000, "chars", ROMREGION_INVERT )
	ROM_LOAD32_BYTE("20.ic97",  0x000000, 0x20000, CRC(ea72f6b5) SHA1(f38e4c8c9acec754f34b3ac442c96919c321a277) )
	ROM_LOAD32_BYTE("14.ic96",  0x000001, 0x20000, CRC(d6fe4f36) SHA1(aa6250d4291ca67165898cec57b96ec830b89d8f) )
	ROM_LOAD32_BYTE("10.ic95",  0x000002, 0x20000, CRC(a2eff7a9) SHA1(290930ff93b20409cb99cbf12a7de493dce7baa2) )
	ROM_LOAD32_BYTE("6.ic110",  0x000003, 0x20000, CRC(0be240fc) SHA1(8c783ad87018fcbb03f98a4810372c5c6eb315c9) )
	ROM_LOAD32_BYTE("21.ic98",  0x080000, 0x20000, CRC(45bb438c) SHA1(2ba9a64d332e019f5eaa71ecb9f60681afd4930a) )
	ROM_LOAD32_BYTE("15.ic99",  0x080001, 0x20000, CRC(afb0fa4a) SHA1(f86df12bece344c29ff65cf2b00a64880f89c4bb) )
	ROM_LOAD32_BYTE("11.ic100", 0x080002, 0x20000, CRC(5d87135d) SHA1(b356edee8bec986446f6c91b5f9394a83bc1c094) )
	ROM_LOAD32_BYTE("7.ic101",  0x080003, 0x20000, CRC(ee822998) SHA1(1753a669a24f4152094489d073f87a18d5f36556) )
	ROM_LOAD32_BYTE("18.ic105", 0x100000, 0x20000, CRC(e088b5e2) SHA1(50950f6f94d6a6f646f5baf3faaceba95b837162) )
	ROM_LOAD32_BYTE("12.ic104", 0x100001, 0x20000, CRC(5339daa7) SHA1(66910bc1b6cfd51239b8c40d9ca34540fd73231b) )
	ROM_LOAD32_BYTE("8.ic103",  0x100002, 0x20000, CRC(fa117809) SHA1(4fffecdbf5adc4735d9680849e33556728c11997) )
	ROM_LOAD32_BYTE("4.ic102",  0x100003, 0x20000, CRC(09ba3171) SHA1(362be89c7b29de2f20ed1e28e73ed7a361f9f647) )
	ROM_LOAD32_BYTE("19.ic106", 0x180000, 0x20000, CRC(48540300) SHA1(cd25c1b7ddcf3883a5f77779d06c8b966acb0a99) )
	ROM_LOAD32_BYTE("13.ic107", 0x180001, 0x20000, CRC(5bcf710e) SHA1(3acf3b41b2002ee56a3452217dc99cbd36bd0273) )
	ROM_LOAD32_BYTE("9.ic108",  0x180002, 0x20000, CRC(edf4c0f4) SHA1(e1689bfaa1547fe8da5635808833cfe91b9d98bc) )
	ROM_LOAD32_BYTE("5.ic109",  0x180003, 0x20000, CRC(93422182) SHA1(72847f073ffaffb7ed34a0972598a3df929c25a3) )

	ROM_REGION( 0x040000, "sprites", ROMREGION_INVERT )
	ROM_LOAD("16.ic42", 0x020000, 0x20000, CRC(c6decb5e) SHA1(3d35cef348deb16a62a066acdfbabebcf11fa997) )
	ROM_LOAD("17.ic41", 0x000000, 0x20000, CRC(5a6efdcc) SHA1(04120dd4da0ff8df514f98a44d7eee7100e4c033) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD("3.ic127", 0x000000, 0x20000, CRC(16b79788) SHA1(6b796119d3c57229ba3d613ce8832c94e9616f76) )
ROM_END

ROM_START( pkladiesblu )  // uncensored encrypted bootleg. ROMs 1, 2, 3, 16 & 17 are identical to set pkladiesbl
	ROM_REGION( 0x50000*2, "maincpu", 0 )
	// only pklbu1.bin is encrypted (only opcodes). Encryption scheme seems to involve XORs and bitswaps, based on addresses.
	ROM_LOAD( "pklbu1.bin", 0x50000, 0x08000, CRC(ca4cfaf9) SHA1(97ad3c526e4494f347db45c986ba23aff07e6321) )
	ROM_CONTINUE(0x00000,0x08000)
	ROM_LOAD( "pklbu2.bin", 0x60000, 0x10000, CRC(5c73e9b6) SHA1(5fbfb4c79e2df8e1edd3f29ac63f9961dd3724b1) )
	ROM_CONTINUE(0x10000,0x10000)

	ROM_REGION( 0x240000, "chars", ROMREGION_INVERT )
	ROM_LOAD32_BYTE("pklbu20.bin", 0x000000, 0x20000, CRC(1f537087) SHA1(87ea5bee67bb2a1f8bbe191ed3e8a83491a3cd0f) )
	ROM_LOAD32_BYTE("pklbu14.bin", 0x000001, 0x20000, CRC(3fd2684d) SHA1(0f14cce27551af64ccc607fa14689cfceb0bb367) )
	ROM_LOAD32_BYTE("pklbu10.bin", 0x000002, 0x20000, CRC(97424238) SHA1(9c3ef3d5524133ba79929318288010ec5a15641a) )
	ROM_LOAD32_BYTE("pklbu6.bin",  0x000003, 0x20000, CRC(96c7eed0) SHA1(d435e6785cc447bee538dc89c1cbbe84071301d8) )
	ROM_LOAD32_BYTE("pklbu21.bin", 0x080000, 0x20000, CRC(6c18df0d) SHA1(eef87c710bbb2f643a7ea8ba2b2f609fb663b579) )
	ROM_LOAD32_BYTE("pklbu15.bin", 0x080001, 0x20000, CRC(0a52206a) SHA1(68f2aa9cda53fb9d545fd5ec29ac7e1ccf0faac7) )
	ROM_LOAD32_BYTE("pklbu11.bin", 0x080002, 0x20000, CRC(7f995c59) SHA1(cb533d524dd35d1058cb319e8f7b60c9d675c858) )
	ROM_LOAD32_BYTE("pklbu7.bin",  0x080003, 0x20000, CRC(d6a7a95b) SHA1(2a4b4ed65dbca88d38f5977f49b6800f8adc519b) )
	ROM_LOAD32_BYTE("pklbu18.bin", 0x100000, 0x20000, CRC(1280a069) SHA1(e2622f528d08eb05dd178fda003b4648828eaf06) )
	ROM_LOAD32_BYTE("pklbu12.bin", 0x100001, 0x20000, CRC(09aa3215) SHA1(d8bfd9eeba33e5b965b6f10421cfee675c44cd61) )
	ROM_LOAD32_BYTE("pklbu8.bin",  0x100002, 0x20000, CRC(2132c239) SHA1(fd770fc212476d32e08e2fae3af3fa42bce5abe2) )
	ROM_LOAD32_BYTE("pklbu4.bin",  0x100003, 0x20000, CRC(b798d926) SHA1(de21e5efff32ef421f68545ccbddfb18db54436b) )
	ROM_LOAD32_BYTE("pklbu19.bin", 0x180000, 0x20000, CRC(95d838cf) SHA1(179634dc6628f858e3f099070604599257aafbe9) )
	ROM_LOAD32_BYTE("pklbu13.bin", 0x180001, 0x20000, CRC(10be806b) SHA1(36cc07a93412be2a0e90bcc133b52febfed8bd90) )
	ROM_LOAD32_BYTE("pklbu9.bin",  0x180002, 0x20000, CRC(9059d383) SHA1(5e8a5a6079838b51363c3a763475b9ca4326d598) )
	ROM_LOAD32_BYTE("pklbu5.bin",  0x180003, 0x20000, CRC(8819affb) SHA1(095d951fa77ae09c4a68c4d971f532c517cb295a) )

	ROM_REGION( 0x040000, "sprites", ROMREGION_INVERT )
	ROM_LOAD("pklbu16.bin", 0x020000, 0x20000, CRC(c6decb5e) SHA1(3d35cef348deb16a62a066acdfbabebcf11fa997) )
	ROM_LOAD("pklbu17.bin", 0x000000, 0x20000, CRC(5a6efdcc) SHA1(04120dd4da0ff8df514f98a44d7eee7100e4c033) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD("pklbu3.bin", 0x000000, 0x20000, CRC(16b79788) SHA1(6b796119d3c57229ba3d613ce8832c94e9616f76) )
ROM_END

ROM_START( pkladiesbl2 ) // same as the above but without the z80 block, only 1.ic112 differs
	ROM_REGION( 0x50000*2, "maincpu", 0 )
	ROM_LOAD( "1.ic112", 0x50000, 0x08000, CRC(cadb9925) SHA1(d88353501a29ff855335f9c8822e095ef5196246) ) //sldh
	ROM_CONTINUE(0x00000,0x08000)
	ROM_LOAD( "2.ic126", 0x60000, 0x10000, CRC(5c73e9b6) SHA1(5fbfb4c79e2df8e1edd3f29ac63f9961dd3724b1) )
	ROM_CONTINUE(0x10000,0x10000)

	ROM_REGION( 0x240000, "chars", ROMREGION_INVERT )
	ROM_LOAD32_BYTE("20.ic97",  0x000000, 0x20000, CRC(ea72f6b5) SHA1(f38e4c8c9acec754f34b3ac442c96919c321a277) )
	ROM_LOAD32_BYTE("14.ic96",  0x000001, 0x20000, CRC(d6fe4f36) SHA1(aa6250d4291ca67165898cec57b96ec830b89d8f) )
	ROM_LOAD32_BYTE("10.ic95",  0x000002, 0x20000, CRC(a2eff7a9) SHA1(290930ff93b20409cb99cbf12a7de493dce7baa2) )
	ROM_LOAD32_BYTE("6.ic110",  0x000003, 0x20000, CRC(0be240fc) SHA1(8c783ad87018fcbb03f98a4810372c5c6eb315c9) )
	ROM_LOAD32_BYTE("21.ic98",  0x080000, 0x20000, CRC(45bb438c) SHA1(2ba9a64d332e019f5eaa71ecb9f60681afd4930a) )
	ROM_LOAD32_BYTE("15.ic99",  0x080001, 0x20000, CRC(afb0fa4a) SHA1(f86df12bece344c29ff65cf2b00a64880f89c4bb) )
	ROM_LOAD32_BYTE("11.ic100", 0x080002, 0x20000, CRC(5d87135d) SHA1(b356edee8bec986446f6c91b5f9394a83bc1c094) )
	ROM_LOAD32_BYTE("7.ic101",  0x080003, 0x20000, CRC(ee822998) SHA1(1753a669a24f4152094489d073f87a18d5f36556) )
	ROM_LOAD32_BYTE("18.ic105", 0x100000, 0x20000, CRC(e088b5e2) SHA1(50950f6f94d6a6f646f5baf3faaceba95b837162) )
	ROM_LOAD32_BYTE("12.ic104", 0x100001, 0x20000, CRC(5339daa7) SHA1(66910bc1b6cfd51239b8c40d9ca34540fd73231b) )
	ROM_LOAD32_BYTE("8.ic103",  0x100002, 0x20000, CRC(fa117809) SHA1(4fffecdbf5adc4735d9680849e33556728c11997) )
	ROM_LOAD32_BYTE("4.ic102",  0x100003, 0x20000, CRC(09ba3171) SHA1(362be89c7b29de2f20ed1e28e73ed7a361f9f647) )
	ROM_LOAD32_BYTE("19.ic106", 0x180000, 0x20000, CRC(48540300) SHA1(cd25c1b7ddcf3883a5f77779d06c8b966acb0a99) )
	ROM_LOAD32_BYTE("13.ic107", 0x180001, 0x20000, CRC(5bcf710e) SHA1(3acf3b41b2002ee56a3452217dc99cbd36bd0273) )
	ROM_LOAD32_BYTE("9.ic108",  0x180002, 0x20000, CRC(edf4c0f4) SHA1(e1689bfaa1547fe8da5635808833cfe91b9d98bc) )
	ROM_LOAD32_BYTE("5.ic109",  0x180003, 0x20000, CRC(93422182) SHA1(72847f073ffaffb7ed34a0972598a3df929c25a3) )

	ROM_REGION( 0x040000, "sprites", ROMREGION_INVERT )
	ROM_LOAD("16.ic42", 0x020000, 0x20000, CRC(c6decb5e) SHA1(3d35cef348deb16a62a066acdfbabebcf11fa997) )
	ROM_LOAD("17.ic41", 0x000000, 0x20000, CRC(5a6efdcc) SHA1(04120dd4da0ff8df514f98a44d7eee7100e4c033) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD("3.ic127", 0x000000, 0x20000, CRC(16b79788) SHA1(6b796119d3c57229ba3d613ce8832c94e9616f76) )
ROM_END

ROM_START( dokaben )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "db_06.11h",    0x00000, 0x08000, CRC(413e0886) SHA1(e9e6117fbbd980bc0f5448ada6c1856919bf92b5) )
	ROM_LOAD( "db_07.13h",    0x10000, 0x20000, CRC(8bdcf49e) SHA1(7d845ae2e640ec7d8d642e3aeef741d9f7b0a57c) )
	ROM_LOAD( "db_08.14h",    0x30000, 0x20000, CRC(1643bdd9) SHA1(5805e749713dbffacbb1238b1b4d42e8473d3656) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "db_02.1e",     0x000000, 0x20000, CRC(9aa8470c) SHA1(8acbed381d6140e70045da232dee9b4b165953f9) )
	ROM_LOAD( "db_03.2e",     0x020000, 0x20000, CRC(3324e43d) SHA1(ed273d4de56e382e24ab0f0a8bcd5e30a05a1c6d) )
	// 40000-7ffff empty
	ROM_LOAD( "db_04.1g",     0x080000, 0x20000, CRC(c0c5b6c2) SHA1(5d66d8b2a62ccab9574e04a867df9bbb8c0d15aa) )
	ROM_LOAD( "db_05.2g",     0x0a0000, 0x20000, CRC(d2ab25f2) SHA1(96eea06d1645e0aade4c1b3153c55e2b61fd52c7) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "db_10.2k",     0x000000, 0x20000, CRC(9e70f7ae) SHA1(ff3833a52d3d198f14e915ce52f7449cf04a0cca) )
	ROM_LOAD( "db_09.1k",     0x020000, 0x20000, CRC(2d9263f7) SHA1(fe2811ae47b9a250ea1485a91c2c3be742d90622) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "db_01.1d",     0x00000, 0x20000, CRC(62fa6b81) SHA1(0168b40df583f11cb28718aa8ab8be7cc08bf561) )
ROM_END

ROM_START( dokaben2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "d2_06.11h",    0x00000, 0x08000, CRC(9adcc38c) SHA1(0cacc58a14d63dfb1565ff517cc45f3d8fc9b77c) )
	ROM_LOAD( "d2_07.13h",    0x10000, 0x20000, CRC(43076e32) SHA1(fca84da82d427b3dca28ed2ec1e811eeddcee666) )
	ROM_LOAD( "d2_08.14h",    0x30000, 0x20000, CRC(cb9deb7a) SHA1(a3e359e991a64190e25cf1c589c82008af2cb9b5) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "d2_02.1e",     0x000000, 0x20000, CRC(5dd7b941) SHA1(b0e93e733b9bbabe68896c92af34b90daf8dcd7c) )
	ROM_LOAD( "d2_03.2e",     0x020000, 0x20000, CRC(b615e696) SHA1(f1ec11202fce23af4af15682b158795f7ff4234f) )
	// 40000-7ffff empty
	ROM_LOAD( "d2_04.1g",     0x080000, 0x20000, CRC(56b35605) SHA1(c065b03b5cb00ac75b8b439a4f35d9b04a886626) )
	ROM_LOAD( "d2_05.2g",     0x0a0000, 0x20000, CRC(ce98ff74) SHA1(ddae2e035369886ab03074e947405ef916cc425a) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "d2_10.2k",     0x000000, 0x20000, CRC(9b9bfb5f) SHA1(5969861e1fe900a3076785c7d1e304c10aa56435) )
	ROM_LOAD( "d2_09.1k",     0x020000, 0x20000, CRC(84de2e1d) SHA1(692304332b37ca3b26dc96bcad797ee81ab8b819) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "db_01.1d",     0x00000, 0x20000, CRC(62fa6b81) SHA1(0168b40df583f11cb28718aa8ab8be7cc08bf561) )
ROM_END

ROM_START( pang )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "pwe_06.11h", 0x00000, 0x08000, CRC(68be52cd) SHA1(67b9ac15f4cbd3959c417f979beae36ae17334c1) )
	ROM_LOAD( "pwe_07.13h", 0x10000, 0x20000, CRC(4a2e70f6) SHA1(039db1b51374e5637b5c2ba8e18ccd08816613a7) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "pwe_02.1e", 0x000000, 0x20000, CRC(3a5883f5) SHA1(a8a33071e10f5992e80afdb782c334829f9ae27f) )
	ROM_LOAD( "pw_03.2e",  0x020000, 0x20000, CRC(79a8ed08) SHA1(c1e43889e29b80c7fe2c09b11eecde24450a1ff5) ) // also found as PWJ 03
	// 40000-7ffff empty
	ROM_LOAD( "pwe_04.1g", 0x080000, 0x20000, CRC(166a16ae) SHA1(7f907c78b7ac8c99e3d79761a6ae689c77e3a1f5) )
	ROM_LOAD( "pw_05.2g",  0x0a0000, 0x20000, CRC(2fb3db6c) SHA1(328814d28569fec763975a8ae4c2767517a680af) ) // also found as PWJ 05
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "pw_10.2k", 0x000000, 0x20000, CRC(fdba4f6e) SHA1(9a2412a97682bbd25b8942520a0c02616bd59353) )
	ROM_LOAD( "pw_9.1k",  0x020000, 0x20000, CRC(39f47a63) SHA1(05675ad45909a7d723acaf4d53b4e588d4e048b9) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pw_01.1d", 0x00000, 0x20000, CRC(c52e5b8e) SHA1(933b954bfdd2d67e28b032ffabde192531249c1f) )
ROM_END

ROM_START( bbros )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "pwu_06.11h", 0x00000, 0x08000, CRC(a3041ca4) SHA1(2accb2151f621e4802211efe986969ebd3acb6d4) )
	ROM_LOAD( "pwu_07.13h", 0x10000, 0x20000, CRC(09231c68) SHA1(9e735487a99a5eb89a6abb81d5d9a20414ad75bf) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "pwu_02.1e", 0x000000, 0x20000, CRC(62f29992) SHA1(af4d43f76228e9908fbfbf83af2f577b84cc5e1d) )
	ROM_LOAD( "pw_03.2e",  0x020000, 0x20000, CRC(79a8ed08) SHA1(c1e43889e29b80c7fe2c09b11eecde24450a1ff5) )
	// 40000-7ffff empty
	ROM_LOAD( "pwu_04.1g", 0x080000, 0x20000, CRC(f705aa89) SHA1(cce2d90f7b767044e84bc22a16474a2f6496292e) )
	ROM_LOAD( "pw_05.2g",  0x0a0000, 0x20000, CRC(2fb3db6c) SHA1(328814d28569fec763975a8ae4c2767517a680af) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "pw_10.2k", 0x000000, 0x20000, CRC(fdba4f6e) SHA1(9a2412a97682bbd25b8942520a0c02616bd59353) )
	ROM_LOAD( "pw_9.1k",  0x020000, 0x20000, CRC(39f47a63) SHA1(05675ad45909a7d723acaf4d53b4e588d4e048b9) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pw_01.1d", 0x00000, 0x20000, CRC(c52e5b8e) SHA1(933b954bfdd2d67e28b032ffabde192531249c1f) )
ROM_END

ROM_START( pompingw )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "pwj_06.11h", 0x00000, 0x08000, CRC(4a0a6426) SHA1(c61346c5f80507bdf543e9ea32ee3f814be8e27f) )
	ROM_LOAD( "pwj_07.13h", 0x10000, 0x20000, CRC(a9402420) SHA1(2ca3aa59d561826477e3509fcaeeec753d64d419) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "pwj_02.1e", 0x000000, 0x20000, CRC(4b5992e4) SHA1(2071a1fcfc739d7ca837c03133909101b462d5a6) )
	ROM_LOAD( "pwj_03.2e", 0x020000, 0x20000, CRC(79a8ed08) SHA1(c1e43889e29b80c7fe2c09b11eecde24450a1ff5) )
	// 40000-7ffff empty
	ROM_LOAD( "pwj_04.1g", 0x080000, 0x20000, CRC(01e49081) SHA1(a29ffec199f196a2b3731e4863e863bdd04e2c58) )
	ROM_LOAD( "pwj_05.2g", 0x0a0000, 0x20000, CRC(2fb3db6c) SHA1(328814d28569fec763975a8ae4c2767517a680af) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "pwj_10.2k", 0x000000, 0x20000, CRC(83a81c02) SHA1(fced1b5798442b7633cef0c5f87546b1845df096) )
	ROM_LOAD( "pwj_9.1k",  0x020000, 0x20000, CRC(6b628232) SHA1(7b3848f289ad96f314076ddeb8b0a196d5102b36) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pwj_01.1d", 0x00000, 0x20000, CRC(c52e5b8e) SHA1(933b954bfdd2d67e28b032ffabde192531249c1f) )
ROM_END

ROM_START( pangb )
	ROM_REGION( 2*0x50000, "maincpu", 0 )
	ROM_LOAD( "pang_04.bin", 0x50000, 0x08000, CRC(f68f88a5) SHA1(6f57891d399a46d8d5a531771129552ed420d10a) )   // Decrypted opcode + data
	ROM_CONTINUE(            0x00000, 0x08000 )
	ROM_LOAD( "pang_02.bin", 0x60000, 0x20000, CRC(3f15bb61) SHA1(4f74ee25f32a201482840158b4d4c7aca1cda684) )   // Decrypted op codes
	ROM_LOAD( "pang_03.bin", 0x10000, 0x20000, CRC(0c8477ae) SHA1(a31a8c00407dfc3017d56e29fac6114b73248030) )   // Decrypted data

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "pang_9.bin",  0x000000, 0x20000, CRC(3a5883f5) SHA1(a8a33071e10f5992e80afdb782c334829f9ae27f) )
	ROM_LOAD( "bb3.bin",     0x020000, 0x20000, CRC(79a8ed08) SHA1(c1e43889e29b80c7fe2c09b11eecde24450a1ff5) )
	// 40000-7ffff empty
	ROM_LOAD( "bb11.bin",    0x080000, 0x20000, CRC(166a16ae) SHA1(7f907c78b7ac8c99e3d79761a6ae689c77e3a1f5) )
	ROM_LOAD( "bb5.bin",     0x0a0000, 0x20000, CRC(2fb3db6c) SHA1(328814d28569fec763975a8ae4c2767517a680af) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "bb10.bin",    0x000000, 0x20000, CRC(fdba4f6e) SHA1(9a2412a97682bbd25b8942520a0c02616bd59353) )
	ROM_LOAD( "bb9.bin",     0x020000, 0x20000, CRC(39f47a63) SHA1(05675ad45909a7d723acaf4d53b4e588d4e048b9) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "bb1.bin",     0x00000, 0x20000, CRC(c52e5b8e) SHA1(933b954bfdd2d67e28b032ffabde192531249c1f) )
ROM_END

ROM_START( pangb2 )
	ROM_REGION( 2*0x50000, "maincpu", 0 )
	ROM_LOAD( "27c512.11h", 0x50000, 0x08000, CRC(369a453e) SHA1(14acd8c2c2229a9af2aafda8e78f8f05d768b54a) )   // Decrypted opcode + data
	ROM_CONTINUE(           0x00000, 0x08000 )
	ROM_LOAD( "27c020.13h", 0x60000, 0x20000, CRC(5e7f24b1) SHA1(99d7365b6d9cc0afb8484c16536d33dc50f04676) )   // Decrypted op codes
	ROM_CONTINUE(           0x10000, 0x20000 )   // Decrypted data

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "pang_9.bin", 0x000000, 0x20000, CRC(3a5883f5) SHA1(a8a33071e10f5992e80afdb782c334829f9ae27f) )
	ROM_LOAD( "bb3.bin",    0x020000, 0x20000, CRC(79a8ed08) SHA1(c1e43889e29b80c7fe2c09b11eecde24450a1ff5) )
	// 40000-7ffff empty
	ROM_LOAD( "bb11.bin",   0x080000, 0x20000, CRC(166a16ae) SHA1(7f907c78b7ac8c99e3d79761a6ae689c77e3a1f5) )
	ROM_LOAD( "bb5.bin",    0x0a0000, 0x20000, CRC(2fb3db6c) SHA1(328814d28569fec763975a8ae4c2767517a680af) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "bb10.bin",   0x000000, 0x20000, CRC(fdba4f6e) SHA1(9a2412a97682bbd25b8942520a0c02616bd59353) )
	ROM_LOAD( "bb9.bin",    0x020000, 0x20000, CRC(39f47a63) SHA1(05675ad45909a7d723acaf4d53b4e588d4e048b9) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "bb1.bin",    0x00000, 0x20000, CRC(c52e5b8e) SHA1(933b954bfdd2d67e28b032ffabde192531249c1f) )
ROM_END

/* I suspect the only real difference in this set is that it doesn't have the date hacked to (c)1990 like
   the above bootleg, and it uses a different PCB layout.  Multiple PCBs with these ROMs have been found,
   so it's worth supporting anyway. */
ROM_START( pangbold )
	ROM_REGION( 2*0x50000, "maincpu", 0 )
	ROM_LOAD( "4.6l",  0x50000, 0x08000, CRC(f68f88a5) SHA1(6f57891d399a46d8d5a531771129552ed420d10a) )   // Decrypted opcode + data
	ROM_CONTINUE(      0x00000, 0x08000 )
	ROM_LOAD( "2.3l",  0x60000, 0x20000, CRC(3f15bb61) SHA1(4f74ee25f32a201482840158b4d4c7aca1cda684) )   // Decrypted op codes
	ROM_LOAD( "3.5l",  0x10000, 0x20000, CRC(ce6375e4) SHA1(fdd40d82553fcd4d2762ecfd898d0e3112dfde79) )   // Decrypted data

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "9.10o",  0x000000, 0x20000, CRC(3a5883f5) SHA1(a8a33071e10f5992e80afdb782c334829f9ae27f) )
	ROM_LOAD( "10.14o", 0x020000, 0x20000, CRC(79a8ed08) SHA1(c1e43889e29b80c7fe2c09b11eecde24450a1ff5) )
	// 40000-7ffff empty
	ROM_LOAD( "11.17j", 0x080000, 0x20000, CRC(166a16ae) SHA1(7f907c78b7ac8c99e3d79761a6ae689c77e3a1f5) )
	ROM_LOAD( "12.20j", 0x0a0000, 0x20000, CRC(2fb3db6c) SHA1(328814d28569fec763975a8ae4c2767517a680af) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "8.7o",     0x000000, 0x10000, CRC(f3188aa1) SHA1(f59da8986c0c7d74185211eae1d1cc3f59a54f82) )
	ROM_LOAD( "7.5o",     0x010000, 0x10000, CRC(011da14b) SHA1(3af9c5ca263b3df98b4f4c88d5428a115ddebef8) )
	ROM_LOAD( "6.3o",     0x020000, 0x10000, CRC(0e25e797) SHA1(88c99e544923142256c93ed2b71f06489f6a90a8) )
	ROM_LOAD( "5.1o",     0x030000, 0x10000, CRC(6daa4e27) SHA1(23411928de911b6303efa3a229646001459e4c70) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "1.1a",      0x00000, 0x10000, CRC(b6463907) SHA1(b79e0dca10c639b7f0ea9cbc49300b80708d46fa) )
ROM_END

// Similar to "pangbold" but with data on a battery backed 256Kbit RAM (undumped), on a small sub-board with the Z84 and a PLD (also undumped, was protected).
ROM_START( pangbp )
	ROM_REGION( 2*0x50000, "maincpu", 0 )
	ROM_LOAD( "pangbp_nvr.bin", 0x00000, 0x08000, NO_DUMP ) // Opcodes + data (?) on battery backed RAM (the battery was dead, so it's undumped)
	ROM_LOAD( "pangbp_4.6l",    0x50000, 0x08000, CRC(01bc7ecf) SHA1(7f3e3cf5f5d03d1c2d1ad624627feb941aea7414) ) // Opcodes + data on ROM (almost the same as the first half of "4.6l" on "pangbold")
	ROM_LOAD( "pangbp_2.3l",    0x60000, 0x20000, CRC(3f15bb61) SHA1(4f74ee25f32a201482840158b4d4c7aca1cda684) ) // Decrypted opcodes
	ROM_LOAD( "pangbp_3.5l",    0x10000, 0x20000, CRC(ce6375e4) SHA1(fdd40d82553fcd4d2762ecfd898d0e3112dfde79) ) // Decrypted data

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "pangbp_9.10o",  0x000000, 0x20000, CRC(3a5883f5) SHA1(a8a33071e10f5992e80afdb782c334829f9ae27f) )
	ROM_LOAD( "pangbp_10.14o", 0x020000, 0x20000, CRC(79a8ed08) SHA1(c1e43889e29b80c7fe2c09b11eecde24450a1ff5) )
	// 40000-7ffff empty
	ROM_LOAD( "pangbp_11.17j", 0x080000, 0x20000, CRC(166a16ae) SHA1(7f907c78b7ac8c99e3d79761a6ae689c77e3a1f5) )
	ROM_LOAD( "pangbp_12.20j", 0x0a0000, 0x20000, CRC(2fb3db6c) SHA1(328814d28569fec763975a8ae4c2767517a680af) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "pangbp_8.7o", 0x000000, 0x10000, CRC(f3188aa1) SHA1(f59da8986c0c7d74185211eae1d1cc3f59a54f82) )
	ROM_LOAD( "pangbp_7.5o", 0x010000, 0x10000, CRC(011da14b) SHA1(3af9c5ca263b3df98b4f4c88d5428a115ddebef8) )
	ROM_LOAD( "pangbp_6.3o", 0x020000, 0x10000, CRC(0e25e797) SHA1(88c99e544923142256c93ed2b71f06489f6a90a8) )
	ROM_LOAD( "pangbp_5.1o", 0x030000, 0x10000, CRC(6daa4e27) SHA1(23411928de911b6303efa3a229646001459e4c70) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pangbp_1.1a", 0x00000, 0x10000, CRC(b6463907) SHA1(b79e0dca10c639b7f0ea9cbc49300b80708d46fa) )

	ROM_REGION( 0x004b2, "plds", 0) // all protected
	ROM_LOAD( "pangbp_p_1-peel18cv8.i1",   0x0000, 0x0155, NO_DUMP )
	ROM_LOAD( "pangbp_p_2-peel18cv8.g20",  0x0155, 0x0155, NO_DUMP )
	ROM_LOAD( "pangbp_p_3-pal16r4acn.b21", 0x02aa, 0x0104, NO_DUMP )
	ROM_LOAD( "pangbp_p_z-pal16l8anc.bin", 0x03ae, 0x0104, NO_DUMP )
ROM_END

/* this bootleg has different sound hardware, the sound program is the same as 'rebus' by microhard
   I suspect it was produced by the same company as 'spangbl' */
ROM_START( pangba )
	ROM_REGION( 2*0x50000, "maincpu", 0 )
	ROM_LOAD( "pang.3",  0x50000, 0x08000, CRC(2548534f) SHA1(c67964e1d0b51ea7bb62685055dee1910e9f0fb9) )
	ROM_CONTINUE(        0x00000, 0x08000 )
	ROM_LOAD( "pang.2",  0x60000, 0x04000, CRC(8167b646) SHA1(db131cb53e81abd070db83721752a8f5473afbb9) )
	ROM_CONTINUE(        0x10000, 0x04000 )
	ROM_CONTINUE(        0x64000, 0x04000 )
	ROM_CONTINUE(        0x14000, 0x04000 )
	ROM_CONTINUE(        0x68000, 0x04000 )
	ROM_CONTINUE(        0x18000, 0x04000 )
	ROM_CONTINUE(        0x6c000, 0x04000 )
	ROM_CONTINUE(        0x1c000, 0x04000 )
	ROM_LOAD( "pang.1",  0x70000, 0x04000, CRC(5c3afca2) SHA1(130c801495d83e2336b8c5b04ca168e76e9e0da8) )
	ROM_CONTINUE(        0x20000, 0x04000 )
	ROM_CONTINUE(        0x74000, 0x04000 )
	ROM_CONTINUE(        0x24000, 0x04000 )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Sound Z80 + M5205(?) samples
	ROM_LOAD( "pang.4",   0x00000, 0x10000, CRC(88a7b1f8) SHA1(b34fa26dbc613bf3b525d19df90fa3ba4efb6e5d) ) // this is the same as the microhard game 'rebus' ...

	ROM_REGION( 0x100000, "chars", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "pang.14", 0x000001, 0x10000, CRC(c90095ee) SHA1(bf380f289eb42030a9f911aa5f697ba76f5723db) )
	ROM_LOAD16_BYTE( "pang.6",  0x000000, 0x10000, CRC(c0133cf3) SHA1(07916f7ce6bbaea75b68f5d1d2cb4486825fc397) )
	ROM_LOAD16_BYTE( "pang.13", 0x020001, 0x10000, CRC(a49e98ec) SHA1(8a3d13bd755b58b0bc1d1497363409a1eeade129) )
	ROM_LOAD16_BYTE( "pang.5",  0x020000, 0x10000, CRC(5804ae3e) SHA1(33de9aea7aa201aa650b0b6c5347713bf10cc13d) )

	ROM_LOAD16_BYTE( "pang.16", 0x080001, 0x10000, CRC(bc508935) SHA1(1a11144b563befc11015d75e3867c07329ee6f32) )
	ROM_LOAD16_BYTE( "pang.8",  0x080000, 0x10000, CRC(53a99bb6) SHA1(ffb75c5541d7c1478f05717b2cfa4bfe9b4654cd) )
	ROM_LOAD16_BYTE( "pang.15", 0x0a0001, 0x10000, CRC(bf5c09b9) SHA1(f66a901292b190aa39dc2460363307e94c358d4d) )
	ROM_LOAD16_BYTE( "pang.7",  0x0a0000, 0x10000, CRC(8b718670) SHA1(c22005a665a9e0bcfc3ddbc22ca4a2a261224ce1) )

	ROM_REGION( 0x040000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "pang.11", 0x020000, 0x10000, CRC(07191732) SHA1(7de03ddb07b2afad311b9ed5c84e04bef62d0050) )
	ROM_LOAD( "pang.9",  0x030000, 0x10000, CRC(6496be82) SHA1(9c7ef4c6c3a0361f3118339a0c63b0923045d6c3) )
	ROM_LOAD( "pang.12", 0x000000, 0x10000, CRC(fa247a04) SHA1(b5cab5f65eb3af3deeea6afba955056ca51f39af) )
	ROM_LOAD( "pang.10", 0x010000, 0x10000, CRC(082151ee) SHA1(0857b9f7430e0fc6217eafbaf008ff9da8e7a493) )
ROM_END

ROM_START( pangbb ) // Same bootleg hardware as pangba, but with original YM2413 music instead of YM3812 arrangement
	ROM_REGION( 2*0x50000, "maincpu", 0 )
	ROM_LOAD( "3", 0x50000, 0x08000, CRC(2548534f) SHA1(c67964e1d0b51ea7bb62685055dee1910e9f0fb9) )
	ROM_CONTINUE( 0x00000, 0x08000 )
	ROM_LOAD( "2", 0x60000, 0x04000, CRC(8167b646) SHA1(db131cb53e81abd070db83721752a8f5473afbb9) )
	ROM_CONTINUE( 0x10000, 0x04000 )
	ROM_CONTINUE( 0x64000, 0x04000 )
	ROM_CONTINUE( 0x14000, 0x04000 )
	ROM_CONTINUE( 0x68000, 0x04000 )
	ROM_CONTINUE( 0x18000, 0x04000 )
	ROM_CONTINUE( 0x6c000, 0x04000 )
	ROM_CONTINUE( 0x1c000, 0x04000 )
	ROM_LOAD( "1", 0x70000, 0x04000, CRC(5c3afca2) SHA1(130c801495d83e2336b8c5b04ca168e76e9e0da8) )
	ROM_CONTINUE( 0x20000, 0x04000 )
	ROM_CONTINUE( 0x74000, 0x04000 )
	ROM_CONTINUE( 0x24000, 0x04000 )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Sound Z80 + M5205(?) samples
	ROM_LOAD( "24", 0x00000, 0x10000, CRC(09c43210) SHA1(79b5aed2c5d6d9110129885e8979c1f13b7b8aac) )

	ROM_REGION( 0x100000, "chars", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "14", 0x000001, 0x10000, CRC(c90095ee) SHA1(bf380f289eb42030a9f911aa5f697ba76f5723db) )
	ROM_LOAD16_BYTE( "6",  0x000000, 0x10000, CRC(c0133cf3) SHA1(07916f7ce6bbaea75b68f5d1d2cb4486825fc397) )
	ROM_LOAD16_BYTE( "13", 0x020001, 0x10000, CRC(a49e98ec) SHA1(8a3d13bd755b58b0bc1d1497363409a1eeade129) )
	ROM_LOAD16_BYTE( "5",  0x020000, 0x10000, CRC(5804ae3e) SHA1(33de9aea7aa201aa650b0b6c5347713bf10cc13d) )

	ROM_LOAD16_BYTE( "16", 0x080001, 0x10000, CRC(bc508935) SHA1(1a11144b563befc11015d75e3867c07329ee6f32) )
	ROM_LOAD16_BYTE( "8",  0x080000, 0x10000, CRC(53a99bb6) SHA1(ffb75c5541d7c1478f05717b2cfa4bfe9b4654cd) )
	ROM_LOAD16_BYTE( "15", 0x0a0001, 0x10000, CRC(bf5c09b9) SHA1(f66a901292b190aa39dc2460363307e94c358d4d) )
	ROM_LOAD16_BYTE( "7",  0x0a0000, 0x10000, CRC(8b718670) SHA1(c22005a665a9e0bcfc3ddbc22ca4a2a261224ce1) )

	ROM_REGION( 0x040000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "11", 0x020000, 0x10000, CRC(07191732) SHA1(7de03ddb07b2afad311b9ed5c84e04bef62d0050) )
	ROM_LOAD( "9",  0x030000, 0x10000, CRC(6496be82) SHA1(9c7ef4c6c3a0361f3118339a0c63b0923045d6c3) )
	ROM_LOAD( "12", 0x000000, 0x10000, CRC(fa247a04) SHA1(b5cab5f65eb3af3deeea6afba955056ca51f39af) )
	ROM_LOAD( "10", 0x010000, 0x10000, CRC(082151ee) SHA1(0857b9f7430e0fc6217eafbaf008ff9da8e7a493) )
ROM_END

// Sound: Z80 (GoldStar Z8400A PS) + OKI M5205 + YM2413 + Xtal 10.000MHz
ROM_START( pangbc )
	ROM_REGION( 2*0x50000, "maincpu", 0 )
	ROM_LOAD( "27c512-1.bin", 0x50000, 0x08000, CRC(f5e4a6c3) SHA1(2679d67877769389e726d601294c986e4bafabe6) )
	ROM_CONTINUE( 0x00000, 0x08000 )
	ROM_LOAD( "27c010.bin",   0x60000, 0x04000, CRC(a128522f) SHA1(476adab8a5a4fae2c5022f89f36598ce275a070d) )
	ROM_CONTINUE( 0x10000, 0x04000 )
	ROM_CONTINUE( 0x64000, 0x04000 )
	ROM_CONTINUE( 0x14000, 0x04000 )
	ROM_CONTINUE( 0x68000, 0x04000 )
	ROM_CONTINUE( 0x18000, 0x04000 )
	ROM_CONTINUE( 0x6c000, 0x04000 )
	ROM_CONTINUE( 0x1c000, 0x04000 )
	ROM_LOAD( "27c512.bin",   0x70000, 0x04000, CRC(48d0e236) SHA1(d459bf1c500d5110c300212552449cbdae2a9dfd) )
	ROM_CONTINUE( 0x20000, 0x04000 )
	ROM_CONTINUE( 0x74000, 0x04000 )
	ROM_CONTINUE( 0x24000, 0x04000 )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Sound Z80 + M5205 samples
	ROM_LOAD( "27c512-2.bin", 0x00000, 0x10000, CRC(09c43210) SHA1(79b5aed2c5d6d9110129885e8979c1f13b7b8aac) )

	ROM_REGION( 0x100000, "chars", ROMREGION_INVERT | ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "pang.14", 0x000001, 0x10000, CRC(c90095ee) SHA1(bf380f289eb42030a9f911aa5f697ba76f5723db) )
	ROM_LOAD16_BYTE( "7.bin",   0x000000, 0x10000, CRC(0725d6ad) SHA1(de2efab47b4958d24c065ce52dcf4fab3c8d4274) )
	ROM_LOAD16_BYTE( "pang.13", 0x020001, 0x10000, CRC(a49e98ec) SHA1(8a3d13bd755b58b0bc1d1497363409a1eeade129) )
	ROM_LOAD16_BYTE( "pang.5",  0x020000, 0x10000, CRC(5804ae3e) SHA1(33de9aea7aa201aa650b0b6c5347713bf10cc13d) )

	ROM_LOAD16_BYTE( "pang.16", 0x080001, 0x10000, CRC(bc508935) SHA1(1a11144b563befc11015d75e3867c07329ee6f32) )
	ROM_LOAD16_BYTE( "pang.8",  0x080000, 0x10000, CRC(53a99bb6) SHA1(ffb75c5541d7c1478f05717b2cfa4bfe9b4654cd) )
	ROM_LOAD16_BYTE( "pang.15", 0x0a0001, 0x10000, CRC(bf5c09b9) SHA1(f66a901292b190aa39dc2460363307e94c358d4d) )
	ROM_LOAD16_BYTE( "pang.7",  0x0a0000, 0x10000, CRC(8b718670) SHA1(c22005a665a9e0bcfc3ddbc22ca4a2a261224ce1) )

	ROM_REGION( 0x040000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "pang.11", 0x020000, 0x10000, CRC(07191732) SHA1(7de03ddb07b2afad311b9ed5c84e04bef62d0050) )
	ROM_LOAD( "pang.9",  0x030000, 0x10000, CRC(6496be82) SHA1(9c7ef4c6c3a0361f3118339a0c63b0923045d6c3) )
	ROM_LOAD( "pang.12", 0x000000, 0x10000, CRC(fa247a04) SHA1(b5cab5f65eb3af3deeea6afba955056ca51f39af) )
	ROM_LOAD( "pang.10", 0x010000, 0x10000, CRC(082151ee) SHA1(0857b9f7430e0fc6217eafbaf008ff9da8e7a493) )
ROM_END

ROM_START( cworld )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "cw05.bin", 0x00000, 0x08000, CRC(d3c1723d) SHA1(b67f63e39f4301909c967555222820b54e98a205) )
	ROM_LOAD( "cw06.bin", 0x10000, 0x20000, CRC(d71ed4a3) SHA1(5b6d498810e6fc8041f4326087f3be56863e91d9) )
	ROM_LOAD( "cw07.bin", 0x30000, 0x20000, CRC(d419ce08) SHA1(f0a8265e839f6bdab2926f48aba88b6f9aaa3b29) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "cw08.bin", 0x000000, 0x20000, CRC(6c80da3c) SHA1(3ed8bc025703d6eccc88af0caeeb8e75a88ba5db) )
	ROM_LOAD( "cw09.bin", 0x020000, 0x20000, CRC(7607da71) SHA1(4486550aa96bf5be0294763a9585fafda3216b27) )
	ROM_LOAD( "cw10.bin", 0x040000, 0x20000, CRC(6f0e639f) SHA1(473804068479516694a864982e2a734f63cb1cce) )
	ROM_LOAD( "cw11.bin", 0x060000, 0x20000, CRC(130bd7c0) SHA1(fde2c358367577b7c51648610b978649424d7637) )
	ROM_LOAD( "cw18.bin", 0x080000, 0x20000, CRC(be6ee0c9) SHA1(1cff9333b32f66440cb6caca27137406d2c9493a) )
	ROM_LOAD( "cw19.bin", 0x0a0000, 0x20000, CRC(51fc5532) SHA1(bea3097492ddbe7842e37d31a633378298459511) )
	ROM_LOAD( "cw20.bin", 0x0c0000, 0x20000, CRC(58381d58) SHA1(aef01f628ad9f2280662610c58e5819611e3435a) )
	ROM_LOAD( "cw21.bin", 0x0e0000, 0x20000, CRC(910cc753) SHA1(971fe794511b336b188d3e2e6b5cda71ae16257f) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "cw16.bin", 0x000000, 0x20000, CRC(f90217d1) SHA1(1dbfeb0fd44928d9428a3798fe6d6862164fdf52) )
	ROM_LOAD( "cw17.bin", 0x020000, 0x20000, CRC(c953c702) SHA1(21d497dbb9ccccce3c440e6f0ba84c1e519d7fed) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "cw01.bin", 0x00000, 0x20000, CRC(f4368f5b) SHA1(7a8657dd4c5f3b60f5137af3c644793c479562a8) )
ROM_END

ROM_START( hatena )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "q2-05.rom", 0x00000, 0x08000, CRC(66c9e1da) SHA1(7ddbc4acf9d9d5b69f0bb60af65a171f3ba185b1) )
	ROM_LOAD( "q2-06.rom", 0x10000, 0x20000, CRC(5fc39916) SHA1(84ead43d8bad3f9c88fcb02171500298613646dc) )
	ROM_LOAD( "q2-07.rom", 0x30000, 0x20000, CRC(ec6d5e5e) SHA1(6269f5a5a3af91193afe85d34a764499877c2a24) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "q2-08.rom", 0x000000, 0x20000, CRC(6c80da3c) SHA1(3ed8bc025703d6eccc88af0caeeb8e75a88ba5db) )
	ROM_LOAD( "q2-09.rom", 0x020000, 0x20000, CRC(abe3e15c) SHA1(5af589e58b317758d1162913f6c104c8459546c0) )
	ROM_LOAD( "q2-10.rom", 0x040000, 0x20000, CRC(6963450d) SHA1(8fff6e9653b10194940b7a7a10f57995aafdd37c) )
	ROM_LOAD( "q2-11.rom", 0x060000, 0x20000, CRC(1e319fa2) SHA1(6064491d19cf9dd320535eb1807f4e5bf3e756ab) )
	ROM_LOAD( "q2-18.rom", 0x080000, 0x20000, CRC(be6ee0c9) SHA1(1cff9333b32f66440cb6caca27137406d2c9493a) )
	ROM_LOAD( "q2-19.rom", 0x0a0000, 0x20000, CRC(70300445) SHA1(499ba7e7cb3b41c858a346888547f98f8e7fe953) )
	ROM_LOAD( "q2-20.rom", 0x0c0000, 0x20000, CRC(21a6ff42) SHA1(d3ae3a5b898fa5202516e0f23e84255fb2164b52) )
	ROM_LOAD( "q2-21.rom", 0x0e0000, 0x20000, CRC(076280c9) SHA1(bdccbd8b169f7e19b955e0ede8bbe03d4009e354) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "q2-16.rom", 0x000000, 0x20000, CRC(ec19b2f0) SHA1(52d0a0b6e583103e0c8b73ecd27b03522accb3cb) )
	ROM_LOAD( "q2-17.rom", 0x020000, 0x20000, CRC(ecd69d92) SHA1(a3ac417bc93f9cb126bd0896f4d85b1bef1dc681) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "q2-01.rom",    0x00000, 0x20000, CRC(149e7a89) SHA1(103ab075b92c895e9991e7ef23df2b38d6a792c6) )
ROM_END

ROM_START( spang )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "spe_06.11h", 0x00000, 0x08000, CRC(1af106fb) SHA1(476ba5c95e090663a47d3f98451bf3b79bac7748) )
	ROM_LOAD( "spe_07.13h", 0x10000, 0x20000, CRC(208b5f54) SHA1(9d44f7240b56756dcb69d110036b1cb13b1bbc02) )
	ROM_LOAD( "spe_08.14h", 0x30000, 0x20000, CRC(2bc03ade) SHA1(3a8ee342b0556a8f6d5a417c98e5c3c43422713d) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "spe_02.1e", 0x000000, 0x20000, CRC(63c9dfd2) SHA1(ddc8ddee336855e857fb3124c8b64af33c2d0080) )
	ROM_LOAD( "spj_03.3e", 0x020000, 0x20000, CRC(3ae28bc1) SHA1(4f6d9a86f624598ebc0825b50941adfb7436e98a) )
	// 40000-7ffff empty
	ROM_LOAD( "spe_04.1g", 0x080000, 0x20000, CRC(9d7b225b) SHA1(d949c91da6ba6b82df0b3445499761a98c7e2703) )
	ROM_LOAD( "spj_05.2g", 0x0a0000, 0x20000, CRC(4a060884) SHA1(f83d713aee4230fc04a1d5f1d4d79c64a5bf2753) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "spj_10.2k", 0x000000, 0x20000, CRC(eedd0ade) SHA1(f2da2eb743c68c5c9a56a94709527110cef5d91d) )
	ROM_LOAD( "spj_09.1k", 0x020000, 0x20000, CRC(04b41b75) SHA1(946ed04a17f1f71085143d43905aa310ce1e05f4) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "spe_01.1d", 0x00000, 0x20000, CRC(2d19c133) SHA1(b3ec226f35494dfc259e910895cec8a49dd2f846) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-spang.bin", 0x0000, 0x0080, CRC(deae1291) SHA1(f62f2ad99852903f1cea3f8c1f69fc11e4e7b48b) )
ROM_END

ROM_START( sbbros )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "spu_06.11h", 0x00000, 0x08000, CRC(292eee6a) SHA1(d33368d2373a1ee9e24ada6aa045e0675c8e8160) )
	ROM_LOAD( "spu_07.13h", 0x10000, 0x20000, CRC(f46b698d) SHA1(6a1867f591aa0fb9e02dd472699df93f9d018793) )
	ROM_LOAD( "spu_08.14h", 0x30000, 0x20000, CRC(a75e7fbe) SHA1(0331d1a3e888678909f3e6d21f97896a5350e585) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "spu_02.1e", 0x000000, 0x20000, CRC(0c22ffc6) SHA1(f95b50617ef5cd8cffffacab0b96b4bfe8dd3a1e) )
	ROM_LOAD( "spj_03.3e", 0x020000, 0x20000, CRC(3ae28bc1) SHA1(4f6d9a86f624598ebc0825b50941adfb7436e98a) )
	// 40000-7ffff empty
	ROM_LOAD( "spu_04.1g", 0x080000, 0x20000, CRC(bb3dee5b) SHA1(e81875b9d9a56e91daa66375b22a4fa6dcd14faa) )
	ROM_LOAD( "spj_05.2g", 0x0a0000, 0x20000, CRC(4a060884) SHA1(f83d713aee4230fc04a1d5f1d4d79c64a5bf2753) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "spu_10.2k", 0x000000, 0x20000, CRC(d6675d8f) SHA1(1c65803fcce2305841e74772ae6ffb6e39edf5c6) )
	ROM_LOAD( "spu_09.1k", 0x020000, 0x20000, CRC(8f678bc8) SHA1(66dc7c14cc012ffa9320cd63bc84977fa76ad738) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "spj_01.1d", 0x00000, 0x20000, CRC(b96ea126) SHA1(83fa71994518d40b8938520faa8701c63b7f579e) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-sbbros.bin", 0x0000, 0x0080, CRC(ed69d3cd) SHA1(89eb0ca65ffe30f5cbe6427f767f1f0870c8a990) )
ROM_END

ROM_START( spangj )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "spj_06.11h", 0x00000, 0x08000, CRC(1a548b0b) SHA1(3aa65028876ab6e176f5b227366e65212c944888) )
	ROM_LOAD( "spj_07.13h", 0x10000, 0x20000, CRC(14c2b765) SHA1(af0f965dd13d878bae7850cf8419b26511090579) )
	ROM_LOAD( "spj_08.14h", 0x30000, 0x20000, CRC(4be4e5b7) SHA1(6273e8bf5d9f5b100ecda20001808dcf86411d83) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "spj_02.1e", 0x000000, 0x20000, CRC(419f69d7) SHA1(e3431b5ce3e687ba9a45cb6e0e0a2dfa3a9e5b29) )
	ROM_LOAD( "spj_03.3e", 0x020000, 0x20000, CRC(3ae28bc1) SHA1(4f6d9a86f624598ebc0825b50941adfb7436e98a) )
	// 40000-7ffff empty
	ROM_LOAD( "spj_04.1g", 0x080000, 0x20000, CRC(6870506f) SHA1(13a12c012ea2efb0c8cd9dcfb4b5757ac08ee912) )
	ROM_LOAD( "spj_05.2g", 0x0a0000, 0x20000, CRC(4a060884) SHA1(f83d713aee4230fc04a1d5f1d4d79c64a5bf2753) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "spj_10.2k", 0x000000, 0x20000, CRC(eedd0ade) SHA1(f2da2eb743c68c5c9a56a94709527110cef5d91d) )
	ROM_LOAD( "spj_09.1k", 0x020000, 0x20000, CRC(04b41b75) SHA1(946ed04a17f1f71085143d43905aa310ce1e05f4) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "spj_01.1d", 0x00000, 0x20000, CRC(b96ea126) SHA1(83fa71994518d40b8938520faa8701c63b7f579e) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-spangj.bin", 0x0000, 0x0080, CRC(237c00eb) SHA1(35a7fe793186e148c163adb04433b6a55ee21502) )
ROM_END

/*
1x Z0840006PSC (main)
1x Z0840006PSC (sound)
1x OKI M5205
1x YM2413
1x LM324N
1x oscillator 29.700 (close to sound)
1x oscillator 12.0 MHz (close to main)
ROMs    16x AM27C512 (1,3-17)
1x AM27C020 (2)
2x GAL16V8A (read protected - no dump available)
Note    1x JAMMA edge connector
1x trimmer (volume)
1x 8 switches dip
*/

ROM_START( spangbl )
	ROM_REGION( 0x50000*2, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ic17.1",   0x00000, 0x08000, CRC(f0b2bf86) SHA1(b42a6c0b98c7ccd1e8acd41066a25c7ed4a3aabe) )
	ROM_CONTINUE(0x50000,0x8000)
	ROM_LOAD( "ic18.2",   0x60000, 0x04000, CRC(6f377832) SHA1(25755ed77a797f50fdfbb4c42a04f51d3d08f87c) )
	ROM_CONTINUE(0x10000,0x4000)
	ROM_CONTINUE(0x64000,0x4000)
	ROM_CONTINUE(0x14000,0x4000)
	ROM_CONTINUE(0x68000,0x4000)
	ROM_CONTINUE(0x18000,0x4000)
	ROM_CONTINUE(0x6c000,0x4000)
	ROM_CONTINUE(0x1c000,0x4000)
	ROM_CONTINUE(0x70000,0x4000)
	ROM_CONTINUE(0x20000,0x4000)
	ROM_CONTINUE(0x74000,0x4000)
	ROM_CONTINUE(0x24000,0x4000)
	ROM_CONTINUE(0x78000,0x4000)
	ROM_CONTINUE(0x28000,0x4000)
	ROM_CONTINUE(0x7c000,0x4000)
	ROM_CONTINUE(0x2c000,0x4000)
	ROM_LOAD( "ic19.3",   0x40000, 0x04000, CRC(7c776309) SHA1(8861ed11484ca0727dfbc3003888a9de32ed8ecc) )
	ROM_CONTINUE(0x48000,0x4000)
	ROM_CONTINUE(0x44000,0x4000)
	ROM_CONTINUE(0x4c000,0x4000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Sound Z80 + M5205 samples
	ROM_LOAD( "ic28.4",   0x00000, 0x10000, CRC(02b07d0a) SHA1(77cb9bf1b0d93ebad1bd8cdbedb7fdbad23697be) )
	ROM_LOAD( "ic45.5",   0x10000, 0x10000, CRC(95c32824) SHA1(02de90a7bfbe89feb7708fda8dfac4ed32bc0773) )

	ROM_REGION( 0x100000, "chars", ROMREGION_INVERT| ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ic79.11",  0x000001, 0x10000, CRC(10839ddd) SHA1(bfb56aa5d6ee1d3aa19e346264bee90d64545e51) )
	ROM_LOAD16_BYTE( "ic78.7",   0x000000, 0x10000, CRC(c1d5df89) SHA1(a86e641af1b41c8f642fe3a14ebcbe6c27f80c79) )
	ROM_LOAD16_BYTE( "ic49.10",  0x020001, 0x10000, CRC(113c2753) SHA1(37b480b5d9c581d3c807c81924b4bbbc21d0698d) )
	ROM_LOAD16_BYTE( "ic48.6",   0x020000, 0x10000, CRC(4ffae6c9) SHA1(71df3c374a24d6a90e78d33929cb91d05bd10b78) )
	ROM_LOAD16_BYTE( "ic81.13",  0x080001, 0x10000, CRC(ebe9c63a) SHA1(1aeeea5051086405ceb803ca7a5bfd82a07ade0f) )
	ROM_LOAD16_BYTE( "ic80.9",   0x080000, 0x10000, CRC(f680051d) SHA1(b6e09e14baf839961f46e0986d2c17f7edfaf13d) )
	ROM_LOAD16_BYTE( "ic51.12",  0x0a0001, 0x10000, CRC(beb49dc9) SHA1(c93f65b0f4ce0a0f400202f2998b89abad1f6942) )
	ROM_LOAD16_BYTE( "ic50.8",   0x0a0000, 0x10000, CRC(3f91014c) SHA1(b3947caa0c667d871c19d7dda6536d043ad296f2) )

	ROM_REGION( 0x040000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "ic94.17",   0x000000, 0x10000, CRC(a56f3c20) SHA1(cb440e0e612da8b8a50fe25a6336869b62ab4cfd) )
	ROM_LOAD( "ic95.16",   0x020000, 0x10000, CRC(14df4659) SHA1(d73fab0a8c1e56a26cc15333a294e876f1552bc9) )
	ROM_LOAD( "ic124.15",  0x010000, 0x10000, CRC(4702c768) SHA1(ff996f1355f32451fa57836c2255027a8108eb40) )
	ROM_LOAD( "ic125.14",  0x030000, 0x10000, CRC(bd5c2f4b) SHA1(3c71d63637633a98ab513e4336e2954af3f964f4) )
ROM_END

ROM_START( spangbl2 )
	ROM_REGION( 0x50000*2, "maincpu", ROMREGION_ERASEFF )
	// IC2 can be found as 27C512 with 1st and 2nd half identical or as 27C256
	ROM_LOAD( "sp2-19.ic2", 0x00000, 0x08000, CRC(6f52f8df) SHA1(a203e5cee601ea660860c38ac8e377a54f619c12) ) // 27C256
	ROM_RELOAD(0x50000, 0x08000)
	ROM_LOAD( "sp-18.ic18", 0x60000, 0x04000, CRC(6f377832) SHA1(25755ed77a797f50fdfbb4c42a04f51d3d08f87c) ) // 27C020
	ROM_CONTINUE(0x10000,0x4000)
	ROM_CONTINUE(0x64000,0x4000)
	ROM_CONTINUE(0x14000,0x4000)
	ROM_CONTINUE(0x68000,0x4000)
	ROM_CONTINUE(0x18000,0x4000)
	ROM_CONTINUE(0x6c000,0x4000)
	ROM_CONTINUE(0x1c000,0x4000)
	ROM_CONTINUE(0x70000,0x4000)
	ROM_CONTINUE(0x20000,0x4000)
	ROM_CONTINUE(0x74000,0x4000)
	ROM_CONTINUE(0x24000,0x4000)
	ROM_CONTINUE(0x78000,0x4000)
	ROM_CONTINUE(0x28000,0x4000)
	ROM_CONTINUE(0x7c000,0x4000)
	ROM_CONTINUE(0x2c000,0x4000)
	ROM_LOAD( "sp-17.ic19", 0x40000, 0x04000, CRC(7c776309) SHA1(8861ed11484ca0727dfbc3003888a9de32ed8ecc) ) // 27C256
	ROM_CONTINUE(0x48000,0x4000)
	ROM_CONTINUE(0x44000,0x4000)
	ROM_CONTINUE(0x4c000,0x4000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Sound Z80 + M5205 samples
	ROM_LOAD( "sp-20.ic28", 0x00000, 0x10000, CRC(02b07d0a) SHA1(77cb9bf1b0d93ebad1bd8cdbedb7fdbad23697be) ) // 27C512
	ROM_LOAD( "sp-21.ic45", 0x10000, 0x10000, CRC(95c32824) SHA1(02de90a7bfbe89feb7708fda8dfac4ed32bc0773) ) // 27C512

	ROM_REGION( 0x100000, "chars", ROMREGION_INVERT| ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sp-31.ic79", 0x000001, 0x10000, CRC(10839ddd) SHA1(bfb56aa5d6ee1d3aa19e346264bee90d64545e51) ) // 27C512
	ROM_LOAD16_BYTE( "sp-23.ic78", 0x000000, 0x10000, CRC(c1d5df89) SHA1(a86e641af1b41c8f642fe3a14ebcbe6c27f80c79) ) // 27C512
	ROM_LOAD16_BYTE( "sp-30.ic49", 0x020001, 0x10000, CRC(113c2753) SHA1(37b480b5d9c581d3c807c81924b4bbbc21d0698d) ) // 27C512
	ROM_LOAD16_BYTE( "sp-22.ic46", 0x020000, 0x10000, CRC(4ffae6c9) SHA1(71df3c374a24d6a90e78d33929cb91d05bd10b78) ) // 27C512
	ROM_LOAD16_BYTE( "sp-33.ic81", 0x080001, 0x10000, CRC(ebe9c63a) SHA1(1aeeea5051086405ceb803ca7a5bfd82a07ade0f) ) // 27C512
	ROM_LOAD16_BYTE( "sp-25.ic80", 0x080000, 0x10000, CRC(f680051d) SHA1(b6e09e14baf839961f46e0986d2c17f7edfaf13d) ) // 27C512
	ROM_LOAD16_BYTE( "sp-32.ic51", 0x0a0001, 0x10000, CRC(beb49dc9) SHA1(c93f65b0f4ce0a0f400202f2998b89abad1f6942) ) // 27C512
	ROM_LOAD16_BYTE( "sp-24.ic50", 0x0a0000, 0x10000, CRC(3f91014c) SHA1(b3947caa0c667d871c19d7dda6536d043ad296f2) ) // 27C512

	ROM_REGION( 0x040000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "sp-29.ic94",  0x000000, 0x10000, CRC(a56f3c20) SHA1(cb440e0e612da8b8a50fe25a6336869b62ab4cfd) ) // 27C512
	ROM_LOAD( "sp-28.ic95",  0x020000, 0x10000, CRC(14df4659) SHA1(d73fab0a8c1e56a26cc15333a294e876f1552bc9) ) // 27C512
	ROM_LOAD( "sp-27.ic124", 0x010000, 0x10000, CRC(4702c768) SHA1(ff996f1355f32451fa57836c2255027a8108eb40) ) // 27C512
	ROM_LOAD( "sp-26.ic125", 0x030000, 0x10000, CRC(bd5c2f4b) SHA1(3c71d63637633a98ab513e4336e2954af3f964f4) ) // 27C512

	// Unused
	ROM_REGION( 0x008800, "extra", 0)
	// "sp2-15" contains the same pattern repeated several times
	ROM_LOAD( "sp2-15.bin", 0x00000, 0x08000, CRC(100dda13) SHA1(9a0b6d4439127abc4995c9df3839bfe1f13d8bc2) ) // 27C256
	// "sp2-16" can be found as a 76161 PROM or as a 27C256 with its first 6KB empty
	ROM_LOAD( "sp2-16.bin", 0x08000, 0x00800, CRC(16dbd461) SHA1(1759ad71df8deb5452b3ee92aa2ece1ee79ff469) ) // M1-76161-5

	// Unused
	ROM_REGION( 0x000505, "plds", 0)
	ROM_LOAD( "1-hy18cv8s.ic4",    0x0000, 0x0155, CRC(a93edbf5) SHA1(57ac314e6d501be903e313aec4e083d642bebebe) )
	ROM_LOAD( "2-pal16l8acn.ic32", 0x0155, 0x0104, CRC(b78ee715) SHA1(df9ed2bef394b4e26ac87bd39d81f4df2b5cefe5) )
	ROM_LOAD( "3-hy18cv8s.ic101",  0x0259, 0x0155, CRC(c5c1b9e2) SHA1(df10e706a212709bec415943a6409dd0eb8f72aa) )
	ROM_LOAD( "4-gal20v8.bin",     0x03ae, 0x0157, CRC(29a2653d) SHA1(38426af27d9c48e650c74fc3d9a9a612d11e413c) )
ROM_END

// seems to be the same basic hardware, but the memory map and io map are different at least..
ROM_START( mstworld )
	ROM_REGION( 0x50000*2, "maincpu", 0 )
	ROM_LOAD( "mw-1.rom", 0x00000, 0x080000, CRC(c4e51fb4) SHA1(60ad4ff2cec3a4d13b4aa0319dfcdab941404b1a) ) // fixed code

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mw-2.rom", 0x00000, 0x08000, CRC(12c4fea9) SHA1(4616f2d70022abcf89f244f3f365b39b96973368) )

	ROM_REGION( 0x080000, "user2", 0 )  // Samples
	ROM_LOAD( "mw-3.rom", 0x00000, 0x080000, CRC(110c6a68) SHA1(915758cd467fbcdfa18ca99df036dca40dfc4649) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0e0000, 0x020000)

	ROM_REGION( 0x80000, "chars", ROMREGION_INVERT )
	ROM_LOAD( "mw-4.rom", 0x00000, 0x020000, CRC(28a3af15) SHA1(99547966b2b5e06e097c55bbbb86a1c2809fa98c) )
	ROM_LOAD( "mw-5.rom", 0x20000, 0x020000, CRC(ffdf7e9f) SHA1(b7732837cc5606d4a868eeaaff438b1a86bd72d7) )
	ROM_LOAD( "mw-6.rom", 0x40000, 0x020000, CRC(1ed773a3) SHA1(0e8517a5c9bed57ecf3bb850152b8c1e1bd3faaa) )
	ROM_LOAD( "mw-7.rom", 0x60000, 0x020000, CRC(8eb7525c) SHA1(9c3fa9373803e9534c1ad7063d660abe130f7b49) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "mw-8.rom", 0x00000, 0x020000, CRC(b9b92a3c) SHA1(97191958a539c6f2eacb3956e8371acbaaa43795) )
	ROM_LOAD( "mw-9.rom", 0x20000, 0x020000, CRC(75fc3375) SHA1(b2e7551bdbe2b0f1c28f6e912a8efaa5645b2ff5) )
ROM_END

ROM_START( mstworld2 )
	ROM_REGION( 0x50000*2, "maincpu", 0 )
	ROM_LOAD( "3.ic2", 0x00000, 0x08000, CRC(34d4f302) SHA1(08802d4c5fb9801ef3f3e68a1c34241918d24931) ) // 0xxxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(      0x00000, 0x08000 )
	ROM_RELOAD(0x48000, 0x10000)
	ROM_LOAD( "4.ic18", 0x60000, 0x04000, CRC(31eed12e) SHA1(3194e3b73164b430ca9808a3b18e43373f46bb28) )
	ROM_CONTINUE(0x10000,0x4000)
	ROM_CONTINUE(0x64000,0x4000)
	ROM_CONTINUE(0x14000,0x4000)
	ROM_CONTINUE(0x68000,0x4000)
	ROM_CONTINUE(0x18000,0x4000)
	ROM_CONTINUE(0x6c000,0x4000)
	ROM_CONTINUE(0x1c000,0x4000)
	ROM_CONTINUE(0x70000,0x4000)
	ROM_CONTINUE(0x20000,0x4000)
	ROM_CONTINUE(0x74000,0x4000)
	ROM_CONTINUE(0x24000,0x4000)
	ROM_CONTINUE(0x78000,0x4000)
	ROM_CONTINUE(0x28000,0x4000)
	ROM_CONTINUE(0x7c000,0x4000)
	ROM_CONTINUE(0x2c000,0x4000)
	ROM_LOAD( "5.ic19", 0x40000, 0x04000, CRC(9bc3c079) SHA1(7ed73556a7b88feab22d5cff30d736c9c855f013) ) // 2c000 e 90000 too
	ROM_CONTINUE(0x48000,0x4000)
	ROM_CONTINUE(0x44000,0x4000)
	ROM_CONTINUE(0x4c000,0x4000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) // same as spangbl and spangbl2
	ROM_LOAD( "2.ic28", 0x00000, 0x10000, CRC(02b07d0a) SHA1(77cb9bf1b0d93ebad1bd8cdbedb7fdbad23697be) )
	ROM_LOAD( "1.ic45", 0x10000, 0x10000, CRC(95c32824) SHA1(02de90a7bfbe89feb7708fda8dfac4ed32bc0773) )

	ROM_REGION( 0x100000, "chars", ROMREGION_INVERT | ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "11.ic79", 0x00001, 0x020000, CRC(28fed01c) SHA1(b1c85ce4ec256fa8d76218f2887dac884dec4fc5) )
	ROM_LOAD16_BYTE( "9.ic78",  0x00000, 0x020000, CRC(e6b888b1) SHA1(365ec026cd6960b9309a429b5568f22462ef767e) )
	ROM_LOAD16_BYTE( "10.ic81", 0x80001, 0x020000, CRC(70d21e13) SHA1(ae8fa31a75556308f3ae61d2f4f3c399ba83e695) )
	ROM_LOAD16_BYTE( "8.ic80",  0x80000, 0x020000, CRC(d66638dd) SHA1(aa3b4aa8ed1d4a4532e5b4469b661f5f45e575b3) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "7.ic124", 0x00000, 0x020000, CRC(b9b92a3c) SHA1(97191958a539c6f2eacb3956e8371acbaaa43795) )
	ROM_LOAD( "6.ic95",  0x20000, 0x020000, CRC(75fc3375) SHA1(b2e7551bdbe2b0f1c28f6e912a8efaa5645b2ff5) )
ROM_END

ROM_START( marukin )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "mg3-01b.9d",    0x00000, 0x08000, CRC(529d4389) SHA1(ec2cc0c5da34706e9b93b430ccc63a297c337c4f) )
	ROM_LOAD( "mg3-02b.10d",   0x10000, 0x20000, CRC(e8a8f14e) SHA1(acfc60d1cc136e0d6e74a0983a16a8164c35333d) )

	ROM_REGION( 0x200000, "chars", 0 )
	ROM_LOAD( "mg3-a.3k",     0x000000, 0x80000, CRC(420f1de7) SHA1(bc2142175f93f96c45c5ee9d23da14f3eb91e58b) )
	ROM_LOAD( "mg3-b.4k",     0x080000, 0x80000, CRC(d8de13fa) SHA1(4420fb6fb42d40c0c84a6f4660bd0ffff429261a) )
	ROM_LOAD( "mg3-c.6k",     0x100000, 0x80000, CRC(fbeb66e8) SHA1(a9f13b3818187af05158dfea62ed46e28acf057b) )
	ROM_LOAD( "mg3-d.7k",     0x180000, 0x80000, CRC(8f6bd831) SHA1(8fe7aeab0ebe52fde269b320e9c797cb6c036eff) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "mg3-05.2g",    0x000000, 0x20000, CRC(7a738d2d) SHA1(4b2daf1824b40b961c1e18050197c817fccc2337) )
	ROM_LOAD( "mg3-04.1g",    0x020000, 0x20000, CRC(56f30515) SHA1(6af85c1bbebba37d3b0d4161bc2495237ddfc494) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mg3-e.1d",     0x00000, 0x80000, CRC(106c2fa9) SHA1(21d4579f41282dc69ea11fe2977c427543f1c69d) )  // banked

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "mj3c.10f", 0x000, 0x117, CRC(f59ba8d2) SHA1(c59c833674b6591885fd333a7d4715005957a3ab) ) // pal16l8bcn
	ROM_LOAD( "mj3p.14k", 0x000, 0x117, CRC(2c87be87) SHA1(0778f7939edf9e786839a8af2724e32d8cd4630b) ) // pal16l8bcn
ROM_END

ROM_START( marukina )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "mg3-01.9d",    0x00000, 0x08000, CRC(04357973) SHA1(61b0b347479126213c90ef6833c09537fab03093) )
	ROM_LOAD( "mg3-02.10d",   0x10000, 0x20000, CRC(50d08da0) SHA1(5d115eb646f34827d02219be3d5346f05c0c27b6) )

	ROM_REGION( 0x200000, "chars", 0 )
	ROM_LOAD( "mg3-a.3k",     0x000000, 0x80000, CRC(420f1de7) SHA1(bc2142175f93f96c45c5ee9d23da14f3eb91e58b) )
	ROM_LOAD( "mg3-b.4k",     0x080000, 0x80000, CRC(d8de13fa) SHA1(4420fb6fb42d40c0c84a6f4660bd0ffff429261a) )
	ROM_LOAD( "mg3-c.6k",     0x100000, 0x80000, CRC(fbeb66e8) SHA1(a9f13b3818187af05158dfea62ed46e28acf057b) )
	ROM_LOAD( "mg3-d.7k",     0x180000, 0x80000, CRC(8f6bd831) SHA1(8fe7aeab0ebe52fde269b320e9c797cb6c036eff) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "mg3-05.2g",    0x000000, 0x20000, CRC(7a738d2d) SHA1(4b2daf1824b40b961c1e18050197c817fccc2337) )
	ROM_LOAD( "mg3-04.1g",    0x020000, 0x20000, CRC(56f30515) SHA1(6af85c1bbebba37d3b0d4161bc2495237ddfc494) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mg3-e.1d",     0x00000, 0x80000, CRC(106c2fa9) SHA1(21d4579f41282dc69ea11fe2977c427543f1c69d) )  // banked
ROM_END

ROM_START( qtono1 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "q3-05.rom",    0x00000, 0x08000, CRC(1dd0a344) SHA1(814049bf957b78ff2d1c8da316dfe5303abee4df) )
	ROM_LOAD( "q3-06.rom",    0x10000, 0x20000, CRC(bd6a2110) SHA1(8c4d7a10dfaee0fcd18be21c80fc3d2ff9615eae) )
	ROM_LOAD( "q3-07.rom",    0x30000, 0x20000, CRC(61e53c4f) SHA1(bcde0029a217994561ae0a6fb0482bf1e3517913) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "q3-08.rom",    0x000000, 0x20000, CRC(1533b978) SHA1(586d3b93152cc78a3ae42987e66d984645cd2849) )
	ROM_LOAD( "q3-09.rom",    0x020000, 0x20000, CRC(a32db2f2) SHA1(df2243bff5fd44ebdfe02c5e0bbcccaff5c32628) )
	ROM_LOAD( "q3-10.rom",    0x040000, 0x20000, CRC(ed681aa8) SHA1(9f8dcebc384ca1582d509de94c194df9e3f81441) )
	ROM_LOAD( "q3-11.rom",    0x060000, 0x20000, CRC(38b2fd10) SHA1(2eee32e7c70f9f529a48d41fa886b3695228a7d3) )
	ROM_LOAD( "q3-18.rom",    0x080000, 0x20000, CRC(9e4292ac) SHA1(e1d96ef2bdb73c291734d0f8a4d7a7efbeef4fb2) )
	ROM_LOAD( "q3-19.rom",    0x0a0000, 0x20000, CRC(b7f6d40f) SHA1(40506ff901fd31a6f67ac23d2a3fdcaac5f7c8f9) )
	ROM_LOAD( "q3-20.rom",    0x0c0000, 0x20000, CRC(6cd7f38d) SHA1(cfc549331aa86a687bd9db8b3a926e490bbd4f55) )
	ROM_LOAD( "q3-21.rom",    0x0e0000, 0x20000, CRC(b4aa6b4b) SHA1(c7c771b69051fd820e9eb3faab62779b8df19209) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "q3-16.rom",    0x000000, 0x20000, CRC(863d6836) SHA1(ec78c462bb80e01f581673f2e9431efdf05599d7) )
	ROM_LOAD( "q3-17.rom",    0x020000, 0x20000, CRC(459bf59c) SHA1(89975c6ff259bf68ac0c25eb0c8afb6862f11c87) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "q3-01.rom",    0x00000, 0x20000, CRC(6c1be591) SHA1(7cab7121d78284dc95ae4218d1e7639a659dda8b) )
ROM_END

ROM_START( qsangoku )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "q4-05c.rom",   0x00000, 0x08000, CRC(e1d010b4) SHA1(7fca1ee45054331320abb6a99f10fa98dd4be994) )
	ROM_LOAD( "q4-06.rom",    0x10000, 0x20000, CRC(a0301849) SHA1(60910d84f869fd5735cd5500a93b761d8b8dbacb) )
	ROM_LOAD( "q4-07.rom",    0x30000, 0x20000, CRC(2941ef5b) SHA1(a86f5365edd315fcbb2a50489d63b4be9587ae29) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "q4-08.rom",    0x000000, 0x20000, CRC(dc84c6cb) SHA1(0fb5737bb2adeddde888d24974806d4c2ac5b2ee) )
	ROM_LOAD( "q4-09.rom",    0x020000, 0x20000, CRC(cbb6234c) SHA1(76b749cc39d3af1d9e4959ea513ed054723ffefd) )
	ROM_LOAD( "q4-10.rom",    0x040000, 0x20000, CRC(c20a27a8) SHA1(f462babb7090b2838326bb65e2cafab0fea12f99) )
	ROM_LOAD( "q4-11.rom",    0x060000, 0x20000, CRC(4ff66aed) SHA1(0d70aae5eb930647753650486c7f7eb56239f1ad) )
	ROM_LOAD( "q4-18.rom",    0x080000, 0x20000, CRC(ca3acea5) SHA1(2aba26a7886481691097e80ec7714a7df5873630) )
	ROM_LOAD( "q4-19.rom",    0x0a0000, 0x20000, CRC(1fd92b7d) SHA1(ca4ae05c97fcdec9f7fa024f09b797391e8b3c14) )
	ROM_LOAD( "q4-20.rom",    0x0c0000, 0x20000, CRC(b02dc6a1) SHA1(78d59ef4a3f7eaa3a003765060b8367348c4cfef) )
	ROM_LOAD( "q4-21.rom",    0x0e0000, 0x20000, CRC(432b1dc1) SHA1(9beb45fe95a2ef78401d50d70eba1e683102cd39) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "q4-16.rom",    0x000000, 0x20000, CRC(77342320) SHA1(a05684f6c75a19569350d6e14eb6cb9777fb1f09) )
	ROM_LOAD( "q4-17.rom",    0x020000, 0x20000, CRC(1275c436) SHA1(ed84fb07749b49066d1caf0c21e46ada94d4c213) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "q4-01.rom",    0x00000, 0x20000, CRC(5d0d07d8) SHA1(d36e42852dd1ec0955d19b16e7dfe157b3d48522) )
ROM_END

ROM_START( block )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "ble_05b.14f",   0x00000, 0x08000, CRC(fcdb7885) SHA1(500ee4b8344181e9ad348bd22344a1a942fe9fdc) )
	ROM_LOAD( "ble_06b.15f",   0x10000, 0x20000, CRC(e114ebde) SHA1(12362e809443644b43fbc72e7eead5f376fe11d3) )
	ROM_LOAD( "ble_07b.16f",   0x30000, 0x20000, CRC(61bef077) SHA1(92792f26305df1e5e66607bed391b84b4964ba3e) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "bl_08.8h",    0x000000, 0x20000, CRC(aa0f4ff1) SHA1(58f3c468f89d834caaf66d3c084ab87addbb75c0) )
	ROM_LOAD( "bl_09.9h",    0x020000, 0x20000, CRC(6fa8c186) SHA1(d4dd26d666f2accce871f70e7882e140d924dd07) )
	// 40000-7ffff empty
	ROM_LOAD( "bl_18.8j",    0x080000, 0x20000, CRC(c0acafaf) SHA1(7c44b2605da6a324d0c145202cb8bac7af7a9c68) )
	ROM_LOAD( "bl_19.9j",    0x0a0000, 0x20000, CRC(1ae942f5) SHA1(e9322790db0bf2a9e862b14e166ee3f36f9ea5ad) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "bl_16.2j",    0x000000, 0x20000, CRC(fadcaff7) SHA1(f4bd8e375fe6b1e6a07b4ec4e58f5807dbd738f8) )
	ROM_LOAD( "bl_17.3j",    0x020000, 0x20000, CRC(5f8cab42) SHA1(3a4c682a7938479e0be80c0494c2c8fc7303b663) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "bl_01.2d",    0x00000, 0x20000, CRC(c2ec2abb) SHA1(89981f2a887ace4c4580e2828cbdc962f89c215e) )
ROM_END

ROM_START( blockr1 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "ble_05a.14f",   0x00000, 0x08000, CRC(fa2a4536) SHA1(8f584745116bd0ced4d66719cd80c0372b797134) )
	ROM_LOAD( "ble_06a.15f",   0x10000, 0x20000, CRC(e114ebde) SHA1(12362e809443644b43fbc72e7eead5f376fe11d3) )
	ROM_LOAD( "ble_07.16f",    0x30000, 0x20000, CRC(1d114f13) SHA1(ee3588e1752b3432fd611e2d7d4fb43f942de580) )

	// the highscore table specifies an unused tile number, so we need ROMREGION_ERASEFF to ensure it is blank
	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "bl_08.8h",    0x000000, 0x20000, CRC(aa0f4ff1) SHA1(58f3c468f89d834caaf66d3c084ab87addbb75c0) )
	ROM_LOAD( "bl_09.9h",    0x020000, 0x20000, CRC(6fa8c186) SHA1(d4dd26d666f2accce871f70e7882e140d924dd07) )
	// 40000-7ffff empty
	ROM_LOAD( "bl_18.8j",    0x080000, 0x20000, CRC(c0acafaf) SHA1(7c44b2605da6a324d0c145202cb8bac7af7a9c68) )
	ROM_LOAD( "bl_19.9j",    0x0a0000, 0x20000, CRC(1ae942f5) SHA1(e9322790db0bf2a9e862b14e166ee3f36f9ea5ad) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "bl_16.2j",    0x000000, 0x20000, CRC(fadcaff7) SHA1(f4bd8e375fe6b1e6a07b4ec4e58f5807dbd738f8) )
	ROM_LOAD( "bl_17.3j",    0x020000, 0x20000, CRC(5f8cab42) SHA1(3a4c682a7938479e0be80c0494c2c8fc7303b663) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "bl_01.2d",    0x00000, 0x20000, CRC(c2ec2abb) SHA1(89981f2a887ace4c4580e2828cbdc962f89c215e) )
ROM_END

ROM_START( blockr2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "ble_05.14f",   0x00000, 0x08000, CRC(c12e7f4c) SHA1(335f4eab2323b942d5feeb3bab6f7286fabfffb4) )
	ROM_LOAD( "ble_06.15f",   0x10000, 0x20000, CRC(cdb13d55) SHA1(2e4489d12a603b4c7dfb90d246ebff9176e88a0b) )
	ROM_LOAD( "ble_07.16f",   0x30000, 0x20000, CRC(1d114f13) SHA1(ee3588e1752b3432fd611e2d7d4fb43f942de580) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "bl_08.8h",    0x000000, 0x20000, CRC(aa0f4ff1) SHA1(58f3c468f89d834caaf66d3c084ab87addbb75c0) )
	ROM_LOAD( "bl_09.9h",    0x020000, 0x20000, CRC(6fa8c186) SHA1(d4dd26d666f2accce871f70e7882e140d924dd07) )
	// 40000-7ffff empty
	ROM_LOAD( "bl_18.8j",    0x080000, 0x20000, CRC(c0acafaf) SHA1(7c44b2605da6a324d0c145202cb8bac7af7a9c68) )
	ROM_LOAD( "bl_19.9j",    0x0a0000, 0x20000, CRC(1ae942f5) SHA1(e9322790db0bf2a9e862b14e166ee3f36f9ea5ad) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "bl_16.2j",    0x000000, 0x20000, CRC(fadcaff7) SHA1(f4bd8e375fe6b1e6a07b4ec4e58f5807dbd738f8) )
	ROM_LOAD( "bl_17.3j",    0x020000, 0x20000, CRC(5f8cab42) SHA1(3a4c682a7938479e0be80c0494c2c8fc7303b663) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "bl_01.2d",    0x00000, 0x20000, CRC(c2ec2abb) SHA1(89981f2a887ace4c4580e2828cbdc962f89c215e) )
ROM_END

ROM_START( blockj )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "blj_05.14f",   0x00000, 0x08000, CRC(3b55969a) SHA1(86de2f1f5878de380a8b1e3935cffa146863f07f) )
	ROM_LOAD( "ble_06.15f",   0x10000, 0x20000, CRC(cdb13d55) SHA1(2e4489d12a603b4c7dfb90d246ebff9176e88a0b) )
	ROM_LOAD( "blj_07.16f",   0x30000, 0x20000, CRC(1723883c) SHA1(e6b7575a55c045b90fb41290a60306713121acfb) )

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "bl_08.8h",    0x000000, 0x20000, CRC(aa0f4ff1) SHA1(58f3c468f89d834caaf66d3c084ab87addbb75c0) )
	ROM_LOAD( "bl_09.9h",    0x020000, 0x20000, CRC(6fa8c186) SHA1(d4dd26d666f2accce871f70e7882e140d924dd07) )
	// 40000-7ffff empty
	ROM_LOAD( "bl_18.8j",    0x080000, 0x20000, CRC(c0acafaf) SHA1(7c44b2605da6a324d0c145202cb8bac7af7a9c68) )
	ROM_LOAD( "bl_19.9j",    0x0a0000, 0x20000, CRC(1ae942f5) SHA1(e9322790db0bf2a9e862b14e166ee3f36f9ea5ad) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "bl_16.2j",    0x000000, 0x20000, CRC(fadcaff7) SHA1(f4bd8e375fe6b1e6a07b4ec4e58f5807dbd738f8) )
	ROM_LOAD( "bl_17.3j",    0x020000, 0x20000, CRC(5f8cab42) SHA1(3a4c682a7938479e0be80c0494c2c8fc7303b663) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "bl_01.2d",    0x00000, 0x20000, CRC(c2ec2abb) SHA1(89981f2a887ace4c4580e2828cbdc962f89c215e) )
ROM_END

ROM_START( blockbl )
	ROM_REGION( 0x50000*2, "maincpu", 0 )
	ROM_LOAD( "m7.l6",        0x50000, 0x08000, CRC(3b576fd9) SHA1(99cf14eba089ed9c7d9f287277dab4a8a997a9a4) )   // Decrypted opcode + data
	ROM_CONTINUE(             0x00000, 0x08000 )
	ROM_LOAD( "m5.l3",        0x60000, 0x20000, CRC(7c988bb7) SHA1(138ffe62ef9186849c3db73b048132ad0349ccf7) )   // Decrypted opcode + data
	ROM_CONTINUE(             0x10000, 0x20000 )
	ROM_LOAD( "m6.l5",        0x30000, 0x20000, CRC(5768d8eb) SHA1(6aa9bc4e778c6a06444bba0f4022710cd2abf35c) )   // Decrypted data

	ROM_REGION( 0x100000, "chars", ROMREGION_ERASEFF )
	ROM_LOAD( "m12.o10",      0x000000, 0x20000, CRC(963154d9) SHA1(ef2d5bb4de3b17a2507f9656d924593edce0f3ed) )
	ROM_LOAD( "m13.o14",      0x020000, 0x20000, CRC(069480bb) SHA1(f33793822848c1c3589fd2f17bbb95254ab64736) )
	// 40000-7ffff empty
	ROM_LOAD( "m4.j17",       0x080000, 0x20000, CRC(9e3b6f4f) SHA1(d129ffd1689eaa21b354dcf60b471542ff434588) )
	ROM_LOAD( "m3.j20",       0x0a0000, 0x20000, CRC(629d58fe) SHA1(936ebc993f382a2cd138b6933d1bd1acd153bc01) )
	// c0000-fffff empty

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "m11.o7",       0x000000, 0x10000, CRC(255180a5) SHA1(8fde20c6c14b84d768ebe3634584f7d4e0702548) )
	ROM_LOAD( "m10.o5",       0x010000, 0x10000, CRC(3201c088) SHA1(df4f8e42eed22e67295131d2a4abf166a9ae4a6e) )
	ROM_LOAD( "m9.o3",        0x020000, 0x10000, CRC(29357fe4) SHA1(479f9a55895e2fd14ee88a65be99cf32ade1ca3d) )
	ROM_LOAD( "m8.o2",        0x030000, 0x10000, CRC(abd665d1) SHA1(a91d05ce1d5dcec2b1a933e4f5d335b05e4b3ec9) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "bl_01.rom",    0x00000, 0x20000, CRC(c2ec2abb) SHA1(89981f2a887ace4c4580e2828cbdc962f89c215e) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void mitchell_state::bootleg_decode()
{
	m_databank->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
	m_opbase->set_base(memregion("maincpu")->base() + 0x50000);
	m_opbank->configure_entries(0, 16, memregion("maincpu")->base() + 0x60000, 0x4000);
}


void mitchell_state::configure_banks(void (*decode)(uint8_t *src, uint8_t *dst, int size))
{
	uint8_t *src = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	m_decoded = std::make_unique<uint8_t[]>(size);
	decode(src, m_decoded.get(), size);
	m_databank->configure_entries(0, 16, src + 0x10000, 0x4000);
	m_opbase->set_base(m_decoded.get());
	m_opbank->configure_entries(0, 16, &m_decoded[0x10000], 0x4000);
}


void mitchell_state::init_dokaben()
{
	m_input_type = 0;
	configure_banks(mgakuen2_decode);
}
void mitchell_state::init_pang()
{
	m_input_type = 0;
	configure_banks(pang_decode);
}
void mitchell_state::init_pangb()
{
	m_input_type = 0;
	bootleg_decode();
}
void mitchell_state::init_cworld()
{
	m_input_type = 0;
	configure_banks(cworld_decode);
}
void mitchell_state::init_hatena()
{
	m_input_type = 0;
	configure_banks(hatena_decode);
}
void mitchell_state::init_spang()
{
	m_input_type = 3;
	configure_banks(spang_decode);
}

void spangbl_state::init_spangbl()
{
	m_input_type = 3;
	bootleg_decode();
}

void mitchell_state::init_spangj()
{
	m_input_type = 3;
	configure_banks(spangj_decode);
}
void mitchell_state::init_sbbros()
{
	m_input_type = 3;
	configure_banks(sbbros_decode);
}
void mitchell_state::init_qtono1()
{
	m_input_type = 0;
	configure_banks(qtono1_decode);
}
void mitchell_state::init_qsangoku()
{
	m_input_type = 0;
	configure_banks(qsangoku_decode);
}

void mitchell_state::init_mgakuen()
{
	m_input_type = 1;
	m_databank->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
	m_maincpu->space(AS_IO).install_read_port(0x03, 0x03, "DSW0");
	m_maincpu->space(AS_IO).install_read_port(0x04, 0x04, "DSW1");
}
void mitchell_state::init_mgakuen2()
{
	m_input_type = 1;
	configure_banks(mgakuen2_decode);
}
void mitchell_state::init_pkladies()
{
	m_input_type = 1;
	configure_banks(mgakuen2_decode);
}
void pkladiesbl_state::init_pkladiesbl()
{
	m_input_type = 0;
	bootleg_decode();
}
void mitchell_state::init_marukin()
{
	m_input_type = 1;
	configure_banks(marukin_decode);
}
void mitchell_state::init_block()
{
	m_input_type = 2;
	configure_banks(block_decode);
}
void mitchell_state::init_blockbl()
{
	m_input_type = 2;
	bootleg_decode();
}

void mstworld_state::init_mstworld()
{
	// descramble the program ROM ..
	int len = memregion("maincpu")->bytes();
	std::vector<uint8_t> source(len);
	uint8_t* dst = memregion("maincpu")->base() ;

	static const int tablebank[]=
	{
		/* fixed code */ 0,  0,
		/* fixed code */ 1,  1,
		/* RAM area   */-1, -1,
		/* RAM area   */-1, -1,
		/* bank 0     */10,  4,
		/* bank 1     */ 5, 13,
		/* bank 2     */ 7, 17,
		/* bank 3     */21,  2,
		/* bank 4     */18,  9,
		/* bank 5     */15,  3,
		/* bank 6     */ 6, 11,
		/* bank 7     */19,  8, /* bank a on spang! */
		/* bank 8     */-1, -1,
		/* bank 9     */-1, -1,
		/* bank a     */-1, -1,
		/* bank b     */-1, -1,
		/* bank c     */20, 20,
		/* bank d     */14, 14,
		/* bank e     */16, 16,
		/* bank f     */12, 12,
	};

	memcpy(&source[0], dst, len);
	for (int x = 0; x < 40; x += 2)
	{
		if (tablebank[x] != -1)
		{
			memcpy(&dst[(x / 2) * 0x4000], &source[tablebank[x] * 0x4000], 0x4000);
			memcpy(&dst[((x / 2) * 0x4000) + 0x50000],&source[tablebank[x + 1] * 0x4000], 0x4000);
		}
	}

	bootleg_decode();
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, mgakuen,     0,        mgakuen,    mgakuen,    mitchell_state,   init_mgakuen,    ROT0,   "Yuga",                      "Mahjong Gakuen", MACHINE_SUPPORTS_SAVE )
GAME( 1988, 7toitsu,     mgakuen,  mgakuen,    mgakuen,    mitchell_state,   init_mgakuen,    ROT0,   "Yuga",                      "Chi-Toitsu", MACHINE_SUPPORTS_SAVE )

GAME( 1989, mgakuen2,    0,        marukin,    marukin,    mitchell_state,   init_mgakuen2,   ROT0,   "Face",                      "Mahjong Gakuen 2 Gakuen-chou no Fukushuu", MACHINE_SUPPORTS_SAVE )

GAME( 1989, pkladies,    0,        marukin,    pkladies,   mitchell_state,   init_pkladies,   ROT0,   "Mitchell",                  "Poker Ladies", MACHINE_SUPPORTS_SAVE )
GAME( 1989, pkladiesl,   pkladies, marukin,    pkladies,   mitchell_state,   init_pkladies,   ROT0,   "Leprechaun",                "Poker Ladies (Leprechaun ver. 510)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, pkladiesla,  pkladies, marukin,    pkladies,   mitchell_state,   init_pkladies,   ROT0,   "Leprechaun",                "Poker Ladies (Leprechaun ver. 401)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, pkladiesbl,  pkladies, pkladiesbl, pkladiesbl, pkladiesbl_state, init_pkladiesbl, ROT0,   "bootleg",                   "Poker Ladies (Censored bootleg, encrypted)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // by Playmark? need to figure out CPU 'decryption' / ordering
GAME( 1989, pkladiesblu, pkladies, pkladiesbl, pkladiesbl, pkladiesbl_state, init_pkladiesbl, ROT0,   "bootleg",                   "Poker Ladies (Uncensored bootleg, encrypted)",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // by Playmark? need to figure out CPU 'decryption' / ordering
GAME( 1989, pkladiesbl2, pkladies, pkladiesbl, pkladiesbl, pkladiesbl_state, init_pkladiesbl, ROT0,   "bootleg",                   "Poker Ladies (Censored bootleg, not encrypted)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // by Playmark? needs inputs, EEPROM (?), MSM5205 hook up, GFX fixes

GAME( 1989, dokaben,     0,        pang,       pang,       mitchell_state,   init_dokaben,    ROT0,   "Capcom",                    "Dokaben (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, dokaben2,    0,        pang,       pang,       mitchell_state,   init_dokaben,    ROT0,   "Capcom",                    "Dokaben 2 (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, pang,        0,        pang,       pang,       mitchell_state,   init_pang,       ROT0,   "Mitchell",                  "Pang (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, bbros,       pang,     pang,       pang,       mitchell_state,   init_pang,       ROT0,   "Mitchell (Capcom license)", "Buster Bros. (USA)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, pompingw,    pang,     pang,       pang,       mitchell_state,   init_pang,       ROT0,   "Mitchell",                  "Pomping World (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, pangb,       pang,     pang,       pang,       mitchell_state,   init_pangb,      ROT0,   "bootleg",                   "Pang (bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, pangbold,    pang,     pang,       pang,       mitchell_state,   init_pangb,      ROT0,   "bootleg",                   "Pang (bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, pangba,      pang,     pangba,     pangdsw,    spangbl_state,    init_pangb,      ROT0,   "bootleg",                   "Pang (bootleg, set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, pangb2,      pang,     pang,       pang,       mitchell_state,   init_pangb,      ROT0,   "bootleg",                   "Pang (bootleg, set 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, pangbb,      pang,     spangbl,    pangdsw,    spangbl_state,    init_pangb,      ROT0,   "bootleg",                   "Pang (bootleg, set 5)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, pangbp,      pang,     pang,       pang,       mitchell_state,   init_pangb,      ROT0,   "bootleg",                   "Pang (bootleg, set 6)", MACHINE_NOT_WORKING ) // Missing the contents of a battery backed RAM
GAME( 1989, pangbc,      pang,     spangbl,    pangdsw,    spangbl_state,    init_pangb,      ROT0,   "bootleg",                   "Pang (bootleg, set 7)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, cworld,      0,        pang,       qtono1,     mitchell_state,   init_cworld,     ROT0,   "Capcom",                    "Capcom World (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, hatena,      0,        pang,       qtono1,     mitchell_state,   init_hatena,     ROT0,   "Capcom",                    "Adventure Quiz 2 - Hatena? no Daibouken (Japan 900228)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, spang,       0,        pangnv,     pang,       mitchell_state,   init_spang,      ROT0,   "Mitchell",                  "Super Pang (World 900914)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, sbbros,      spang,    pangnv,     pang,       mitchell_state,   init_sbbros,     ROT0,   "Mitchell (Capcom license)", "Super Buster Bros. (USA 901001)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, spangj,      spang,    pangnv,     pang,       mitchell_state,   init_spangj,     ROT0,   "Mitchell",                  "Super Pang (Japan 901023)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, spangbl,     spang,    spangbl,    spangbl,    spangbl_state,    init_spangbl,    ROT0,   "bootleg",                   "Super Pang (World 900914, bootleg, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // different sound hardware
GAME( 1990, spangbl2,    spang,    spangbl,    spangbl,    spangbl_state,    init_spangbl,    ROT0,   "bootleg",                   "Super Pang (World 900914, bootleg, set 2)", MACHINE_NOT_WORKING )

GAME( 1994, mstworld,    0,        mstworld,   mstworld,   mstworld_state,   init_mstworld,   ROT0,   "bootleg (TCH)",             "Monsters World (bootleg of Super Pang)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, mstworld2,   mstworld, mstworld2,  mstworld2,  spangbl_state,    init_spangbl,    ROT0,   "bootleg",                   "Monsters World 2 (bootleg of Super Pang)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // GFX garbage at title screen with clean NVRAM

GAME( 1990, marukin,     0,        marukin,    marukin,    mitchell_state,   init_marukin,    ROT0,   "Yuga",                      "Super Marukin-Ban (Japan 911128)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, marukina,    marukin,  marukin,    marukin,    mitchell_state,   init_marukin,    ROT0,   "Yuga",                      "Super Marukin-Ban (Japan 901017)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, qtono1,      0,        pang,       qtono1,     mitchell_state,   init_qtono1,     ROT0,   "Capcom",                    "Quiz Tonosama no Yabou (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, qsangoku,    0,        pang,       qtono1,     mitchell_state,   init_qsangoku,   ROT0,   "Capcom",                    "Quiz Sangokushi (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, block,       0,        pangnv,     blockjoy,   mitchell_state,   init_block,      ROT270, "Capcom",                    "Block Block (World 911219 Joystick)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, blockr1,     block,    pangnv,     blockjoy,   mitchell_state,   init_block,      ROT270, "Capcom",                    "Block Block (World 911106 Joystick)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, blockr2,     block,    pangnv,     block,      mitchell_state,   init_block,      ROT270, "Capcom",                    "Block Block (World 910910)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, blockj,      block,    pangnv,     block,      mitchell_state,   init_block,      ROT270, "Capcom",                    "Block Block (Japan 910910)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, blockbl,     block,    pangnv,     block,      mitchell_state,   init_blockbl,    ROT270, "bootleg",                   "Block Block (bootleg)", MACHINE_SUPPORTS_SAVE )
