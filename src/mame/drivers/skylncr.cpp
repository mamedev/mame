// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, David Haywood
/***************************************************************************************************

  Sky Lancer / Butterfly / Mad Zoo / Super Star 97 and others...
  Bordun International.

  Original preliminary driver by Luca Elia.
  Additional Work: Roberto Fresca & David Haywood.

****************************************************************************************************

  Notes:

  - There are at least 3 different PCBs: Rolla, Sky and Cleco.

  - Some of the tiles look badly scaled down, and others appear to have columns swapped.
    This might actually be correct due to different gfx encoding for different PCBs.

  - Skylncr and madzoo can run with the same program roms, they're basically graphics swaps.

  - To enter the Service Mode, press F2. All the game settings are there.
    Change regular values using DIP switches, and jackpot value using STOP2 and STOP3.
    To exit the mode, press START. You must reset the machine (F3) to update the changes.

  - Press key 0 to navigate between statistics pages. Press START to exit the mode.

****************************************************************************************************

  Settings:

  Pressing F2, you can enter the DIP switches settings.
  Here the translated items:

  .---------------------------------------.
  |        DIP Switches Settings          |
  |                                       |
  |   Main Game %      Double-Up %        |
  |   Clown %          Reels Speed        |
  |   Coin Scores      Key In Scores      |
  |   Payout Limit     Key Out Score      |
  |   Max Bet          Min Bet            |
  |   Special Bonus %  Super Star %       |
  |   Bonus Base       Max Win Bonus      |
  |   Double-Up Y/N    Bonus Scores       |
  |                                       |
  '---------------------------------------'

  'Special Bonus' and 'Super Star' appearance, are per 1000.
  You also can find the MAME DIP switches menu already translated.
  The <unknown> items still need translation.

  Press START (key 1) to exit the mode.


  Bookkeeping:

  Pressing BOOKKEEPING (key 0), you enter the Record Mode.
  Here the translated items:

  .---------------------------------------.
  |             Record Menu               |
  |                                       |
  |   Play Scores      Key In Total       |
  |   Win Scores       Key Out Total      |
  |   Play Times       Coin In Total      |
  |   Win Times        Coin Out Total     |
  |   Bonus Scores     Short Total        |
  |   Double Play      Special Times      |
  |   Double Win       Super Show Up      |
  |   Win Times        Power On Times     |
  |   Loss Times       Working Time  H M  |
  |                                       |
  |            Version XXXXX              |
  '---------------------------------------'

  Pressing BOOKKEEPING key again, you can find 2 screens showing
  all statistics and the whole historial by winning hand.

  Press START (key 1) to exit the mode.

****************************************************************************************************

  Game specific notes...

  * Sonik Fighter

  The game is encrypted, and runs with an obfuscated daughterboard in place of the CPU.
  Even when I have the attract working, accepting coins, getting sounds and accurate inputs,
  the game is still not working. Once coined, there's no way to start a game.

  The PPI0 port B, D5 input line behaves like a reset, when the Attract/Girls DSW is set OFF.
  Need more investigation about...

  Colors are wrong due to can't find a way to set the palette.

  * Magical Butterfly

  Some of the inputs are merged, and there is an extra output port that seems to handle lamps and
  a rather simple form of protection that also involves one of the PPI1 input lines.

  To enter the Service Mode or Bookkeeping Menu, hold down Stop Reel 2/Up and Take along with the
  specific inputs.

  * 蝴蝶梦 97 (Húdié Mèng 97)

  The program code has trivial opcode encryption which has been broken. However, the inputs tend
  to act out of control at many times. This may have to do with the large number of unknown reads
  and writes to an I/O port at $66.

****************************************************************************************************

  TODO:

  - Proper M5M82C255 device emulation.
  - Colors: Find the palette in Sonik Fighter.

***************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>


#define MASTER_CLOCK        XTAL(12'000'000)  /* confirmed */
#define HOPPER_PULSE        50 // guessed


class skylncr_state : public driver_device
{
public:
	skylncr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_reeltiles_1_ram(*this, "reeltiles_1_ram"),
		m_reeltiles_2_ram(*this, "reeltiles_2_ram"),
		m_reeltiles_3_ram(*this, "reeltiles_3_ram"),
		m_reeltiles_4_ram(*this, "reeltiles_4_ram"),
		m_reeltileshigh_1_ram(*this, "rthigh_1_ram"),
		m_reeltileshigh_2_ram(*this, "rthigh_2_ram"),
		m_reeltileshigh_3_ram(*this, "rthigh_3_ram"),
		m_reeltileshigh_4_ram(*this, "rthigh_4_ram"),
		m_reelscroll1(*this, "reelscroll1"),
		m_reelscroll2(*this, "reelscroll2"),
		m_reelscroll3(*this, "reelscroll3"),
		m_reelscroll4(*this, "reelscroll4"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 1U)
	{ }

	void neraidou(machine_config &config);
	void sstar97(machine_config &config);
	void bdream97(machine_config &config);
	void skylncr(machine_config &config);
	void mbutrfly(machine_config &config);

	void init_miaction();
	void init_sonikfig();

	READ_LINE_MEMBER(mbutrfly_prot_r);

private:
	DECLARE_WRITE8_MEMBER(skylncr_videoram_w);
	DECLARE_WRITE8_MEMBER(skylncr_colorram_w);
	DECLARE_WRITE8_MEMBER(reeltiles_1_w);
	DECLARE_WRITE8_MEMBER(reeltiles_2_w);
	DECLARE_WRITE8_MEMBER(reeltiles_3_w);
	DECLARE_WRITE8_MEMBER(reeltiles_4_w);
	DECLARE_WRITE8_MEMBER(reeltileshigh_1_w);
	DECLARE_WRITE8_MEMBER(reeltileshigh_2_w);
	DECLARE_WRITE8_MEMBER(reeltileshigh_3_w);
	DECLARE_WRITE8_MEMBER(reeltileshigh_4_w);
	DECLARE_WRITE8_MEMBER(reelscroll1_w);
	DECLARE_WRITE8_MEMBER(reelscroll2_w);
	DECLARE_WRITE8_MEMBER(reelscroll3_w);
	DECLARE_WRITE8_MEMBER(reelscroll4_w);
	DECLARE_WRITE8_MEMBER(skylncr_coin_w);
	DECLARE_READ8_MEMBER(ret_ff);
	DECLARE_WRITE8_MEMBER(skylncr_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(mbutrfly_prot_w);
	DECLARE_READ8_MEMBER(bdream97_opcode_r);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_1_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_2_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_3_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_4_tile_info);
	uint32_t screen_update_skylncr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(skylncr_vblank_interrupt);
	void bdream97_opcode_map(address_map &map);
	void io_map_mbutrfly(address_map &map);
	void io_map_skylncr(address_map &map);
	void mem_map_skylncr(address_map &map);
	void ramdac2_map(address_map &map);
	void ramdac_map(address_map &map);

	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override;

	tilemap_t *m_tmap;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_reeltiles_1_ram;
	required_shared_ptr<uint8_t> m_reeltiles_2_ram;
	required_shared_ptr<uint8_t> m_reeltiles_3_ram;
	required_shared_ptr<uint8_t> m_reeltiles_4_ram;
	required_shared_ptr<uint8_t> m_reeltileshigh_1_ram;
	required_shared_ptr<uint8_t> m_reeltileshigh_2_ram;
	required_shared_ptr<uint8_t> m_reeltileshigh_3_ram;
	required_shared_ptr<uint8_t> m_reeltileshigh_4_ram;
	tilemap_t *m_reel_1_tilemap;
	tilemap_t *m_reel_2_tilemap;
	tilemap_t *m_reel_3_tilemap;
	tilemap_t *m_reel_4_tilemap;
	required_shared_ptr<uint8_t> m_reelscroll1;
	required_shared_ptr<uint8_t> m_reelscroll2;
	required_shared_ptr<uint8_t> m_reelscroll3;
	required_shared_ptr<uint8_t> m_reelscroll4;
	uint8_t m_nmi_enable;
	bool m_mbutrfly_prot;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ticket_dispenser_device> m_hopper;
	output_finder<7> m_lamps;
};


/**************************************
*           Video Hardware            *
**************************************/

WRITE8_MEMBER(skylncr_state::skylncr_videoram_w)
{
	m_videoram[offset] = data;
	m_tmap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::skylncr_colorram_w)
{
	m_colorram[offset] = data;
	m_tmap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(skylncr_state::get_tile_info)
{
	uint16_t code = m_videoram[ tile_index ] + (m_colorram[ tile_index ] << 8);
	int pal = (code & 0x8000) >> 15;
	SET_TILE_INFO_MEMBER(0, code, pal^1, TILE_FLIPYX( 0 ));
}

TILE_GET_INFO_MEMBER(skylncr_state::get_reel_1_tile_info)
{
	uint16_t code = m_reeltiles_1_ram[ tile_index ] + (m_reeltileshigh_1_ram[ tile_index ] << 8);
	int pal = (code & 0x8000) >> 15;
	SET_TILE_INFO_MEMBER(1, code&0x7fff, pal^1, TILE_FLIPYX( 0 ));
}

TILE_GET_INFO_MEMBER(skylncr_state::get_reel_2_tile_info)
{
	uint16_t code = m_reeltiles_2_ram[ tile_index ] + (m_reeltileshigh_2_ram[ tile_index ] << 8);
	int pal = (code & 0x8000) >> 15;
	SET_TILE_INFO_MEMBER(1, code, pal^1, TILE_FLIPYX( 0 ));
}

TILE_GET_INFO_MEMBER(skylncr_state::get_reel_3_tile_info)
{
	uint16_t code = m_reeltiles_3_ram[ tile_index ] + (m_reeltileshigh_3_ram[ tile_index ] << 8);
	int pal = (code & 0x8000) >> 15;
	SET_TILE_INFO_MEMBER(1, code, pal^1, TILE_FLIPYX( 0 ));
}

TILE_GET_INFO_MEMBER(skylncr_state::get_reel_4_tile_info)
{
	uint16_t code = m_reeltiles_4_ram[ tile_index ] + (m_reeltileshigh_4_ram[ tile_index ] << 8);
	int pal = (code & 0x8000) >> 15;
	SET_TILE_INFO_MEMBER(1, code, pal^1, TILE_FLIPYX( 0 ));
}


void skylncr_state::video_start()
{
	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 0x40, 0x20    );

	m_reel_1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_reel_1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 32, 64, 8 );
	m_reel_2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_reel_2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 32, 64, 8 );
	m_reel_3_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_reel_3_tile_info),this), TILEMAP_SCAN_ROWS, 8, 32, 64, 8 );
	m_reel_4_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_reel_4_tile_info),this), TILEMAP_SCAN_ROWS, 8, 32, 64, 8 );

	m_reel_2_tilemap->set_scroll_cols(0x40);
	m_reel_3_tilemap->set_scroll_cols(0x40);
	m_reel_4_tilemap->set_scroll_cols(0x40);

	m_reel_2_tilemap->set_transparent_pen(0);
	m_reel_3_tilemap->set_transparent_pen(0);
	m_reel_4_tilemap->set_transparent_pen(0);


	m_tmap->set_transparent_pen(0);
}


uint32_t skylncr_state::screen_update_skylncr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i;

	bitmap.fill(rgb_t::black(), cliprect);
	m_reel_1_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// are these hardcoded, or registers?
	const rectangle visible1(0*8, (20+48)*8-1,  4*8,  (4+7)*8-1);
	const rectangle visible2(0*8, (20+48)*8-1, 12*8, (12+7)*8-1);
	const rectangle visible3(0*8, (20+48)*8-1, 20*8, (20+7)*8-1);

	for (i= 0;i < 64;i++)
	{
		m_reel_2_tilemap->set_scrolly(i, m_reelscroll2[i]);
		m_reel_3_tilemap->set_scrolly(i, m_reelscroll3[i]);
		m_reel_4_tilemap->set_scrolly(i, m_reelscroll4[i]);
	}

	m_reel_2_tilemap->draw(screen, bitmap, visible1, 0, 0);
	m_reel_3_tilemap->draw(screen, bitmap, visible2, 0, 0);
	m_reel_4_tilemap->draw(screen, bitmap, visible3, 0, 0);


	m_tmap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

WRITE8_MEMBER(skylncr_state::reeltiles_1_w)
{
	m_reeltiles_1_ram[offset] = data;
	m_reel_1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::reeltiles_2_w)
{
	m_reeltiles_2_ram[offset] = data;
	m_reel_2_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::reeltiles_3_w)
{
	m_reeltiles_3_ram[offset] = data;
	m_reel_3_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::reeltiles_4_w)
{
	m_reeltiles_4_ram[offset] = data;
	m_reel_4_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::reeltileshigh_1_w)
{
	m_reeltileshigh_1_ram[offset] = data;
	m_reel_1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::reeltileshigh_2_w)
{
	m_reeltileshigh_2_ram[offset] = data;
	m_reel_2_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::reeltileshigh_3_w)
{
	m_reeltileshigh_3_ram[offset] = data;
	m_reel_3_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::reeltileshigh_4_w)
{
	m_reeltileshigh_4_ram[offset] = data;
	m_reel_4_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(skylncr_state::reelscroll1_w)
{
	m_reelscroll1[offset] = data;
}

WRITE8_MEMBER(skylncr_state::reelscroll2_w)
{
	m_reelscroll2[offset] = data;
}

WRITE8_MEMBER(skylncr_state::reelscroll3_w)
{
	m_reelscroll3[offset] = data;
}

WRITE8_MEMBER(skylncr_state::reelscroll4_w)
{
	m_reelscroll4[offset] = data;
}


/************************************
*         Other Handlers            *
************************************/

WRITE8_MEMBER(skylncr_state::skylncr_coin_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	m_hopper->motor_w(data & 0x20);
}

READ8_MEMBER(skylncr_state::ret_ff)
{
	return 0xff;
}

#ifdef UNUSED_FUNCTION
READ8_MEMBER(skylncr_state::ret_00)
{
	return 0x00;
}
#endif

WRITE8_MEMBER(skylncr_state::skylncr_nmi_enable_w)
{
	m_nmi_enable = data & 0x10;
}

WRITE8_MEMBER(skylncr_state::mbutrfly_prot_w)
{
	m_lamps[0] = BIT(data, 0); // Slot Stop 2
	m_lamps[1] = BIT(data, 1); // Slot Stop 1
	m_lamps[2] = BIT(data, 2); // Take
	m_lamps[3] = BIT(data, 3); // Bet
	m_lamps[4] = BIT(data, 4); // Slot Stop 3
	m_lamps[5] = BIT(data, 5); // Start
	m_lamps[6] = BIT(data, 6); // Payout
	m_mbutrfly_prot = BIT(data, 7);
}

READ_LINE_MEMBER(skylncr_state::mbutrfly_prot_r)
{
	return m_mbutrfly_prot;
}

READ8_MEMBER(skylncr_state::bdream97_opcode_r)
{
	auto dis = machine().disable_side_effects();
	return m_maincpu->space(AS_PROGRAM).read_byte(offset) ^ 0x80;
}


/**************************************
*             Memory Map              *
**************************************/

void skylncr_state::mem_map_skylncr(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");

	map(0x8800, 0x8fff).ram().w(FUNC(skylncr_state::skylncr_videoram_w)).share("videoram");
	map(0x9000, 0x97ff).ram().w(FUNC(skylncr_state::skylncr_colorram_w)).share("colorram");

	map(0x9800, 0x99ff).ram().w(FUNC(skylncr_state::reeltiles_1_w)).share("reeltiles_1_ram");
	map(0x9a00, 0x9bff).ram().w(FUNC(skylncr_state::reeltiles_2_w)).share("reeltiles_2_ram");
	map(0x9c00, 0x9dff).ram().w(FUNC(skylncr_state::reeltiles_3_w)).share("reeltiles_3_ram");
	map(0x9e00, 0x9fff).ram().w(FUNC(skylncr_state::reeltiles_4_w)).share("reeltiles_4_ram");
	map(0xa000, 0xa1ff).ram().w(FUNC(skylncr_state::reeltileshigh_1_w)).share("rthigh_1_ram");
	map(0xa200, 0xa3ff).ram().w(FUNC(skylncr_state::reeltileshigh_2_w)).share("rthigh_2_ram");
	map(0xa400, 0xa5ff).ram().w(FUNC(skylncr_state::reeltileshigh_3_w)).share("rthigh_3_ram");
	map(0xa600, 0xa7ff).ram().w(FUNC(skylncr_state::reeltileshigh_4_w)).share("rthigh_4_ram");

	map(0xaa55, 0xaa55).r(FUNC(skylncr_state::ret_ff));

	map(0xb000, 0xb03f).ram().w(FUNC(skylncr_state::reelscroll1_w)).share("reelscroll1");
	map(0xb040, 0xb07f).ram().w(FUNC(skylncr_state::reelscroll1_w));
	map(0xb080, 0xb0bf).ram().w(FUNC(skylncr_state::reelscroll1_w));
	map(0xb0c0, 0xb0ff).ram().w(FUNC(skylncr_state::reelscroll1_w));
	map(0xb100, 0xb13f).ram().w(FUNC(skylncr_state::reelscroll1_w));
	map(0xb140, 0xb17f).ram().w(FUNC(skylncr_state::reelscroll1_w));
	map(0xb180, 0xb1bf).ram().w(FUNC(skylncr_state::reelscroll1_w));
	map(0xb1c0, 0xb1ff).ram().w(FUNC(skylncr_state::reelscroll1_w));

	map(0xb200, 0xb23f).ram().w(FUNC(skylncr_state::reelscroll2_w)).share("reelscroll2");
	map(0xb240, 0xb27f).ram().w(FUNC(skylncr_state::reelscroll2_w));
	map(0xb280, 0xb2bf).ram().w(FUNC(skylncr_state::reelscroll2_w));
	map(0xb2c0, 0xb2ff).ram().w(FUNC(skylncr_state::reelscroll2_w));
	map(0xb300, 0xb33f).ram().w(FUNC(skylncr_state::reelscroll2_w));
	map(0xb340, 0xb37f).ram().w(FUNC(skylncr_state::reelscroll2_w));
	map(0xb380, 0xb3bf).ram().w(FUNC(skylncr_state::reelscroll2_w));
	map(0xb3c0, 0xb3ff).ram().w(FUNC(skylncr_state::reelscroll2_w));

	map(0xb400, 0xb43f).ram().w(FUNC(skylncr_state::reelscroll3_w)).share("reelscroll3");
	map(0xb440, 0xb47f).ram().w(FUNC(skylncr_state::reelscroll3_w));
	map(0xb480, 0xb4bf).ram().w(FUNC(skylncr_state::reelscroll3_w));
	map(0xb4c0, 0xb4ff).ram().w(FUNC(skylncr_state::reelscroll3_w));
	map(0xb500, 0xb53f).ram().w(FUNC(skylncr_state::reelscroll3_w));
	map(0xb540, 0xb57f).ram().w(FUNC(skylncr_state::reelscroll3_w));
	map(0xb580, 0xb5bf).ram().w(FUNC(skylncr_state::reelscroll3_w));
	map(0xb5c0, 0xb5ff).ram().w(FUNC(skylncr_state::reelscroll3_w));

	map(0xb600, 0xb63f).ram().w(FUNC(skylncr_state::reelscroll4_w)).share("reelscroll4");
	map(0xb640, 0xb67f).ram().w(FUNC(skylncr_state::reelscroll4_w));
	map(0xb680, 0xb6bf).ram().w(FUNC(skylncr_state::reelscroll4_w));
	map(0xb6c0, 0xb6ff).ram().w(FUNC(skylncr_state::reelscroll4_w));
	map(0xb700, 0xb73f).ram().w(FUNC(skylncr_state::reelscroll4_w));
	map(0xb740, 0xb77f).ram().w(FUNC(skylncr_state::reelscroll4_w));
	map(0xb780, 0xb7bf).ram().w(FUNC(skylncr_state::reelscroll4_w));
	map(0xb7c0, 0xb7ff).ram().w(FUNC(skylncr_state::reelscroll4_w));

	map(0xc000, 0xffff).rom();
}


void skylncr_state::io_map_skylncr(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */
	map(0x10, 0x13).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    /* Input Ports */

	map(0x20, 0x20).w(FUNC(skylncr_state::skylncr_coin_w));

	map(0x30, 0x31).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x31, 0x31).r("aysnd", FUNC(ay8910_device::data_r));

	map(0x40, 0x40).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x41, 0x41).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x42, 0x42).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x50, 0x50).w("ramdac2", FUNC(ramdac_device::index_w));
	map(0x51, 0x51).w("ramdac2", FUNC(ramdac_device::pal_w));
	map(0x52, 0x52).w("ramdac2", FUNC(ramdac_device::mask_w));

	map(0x70, 0x70).w(FUNC(skylncr_state::skylncr_nmi_enable_w));
}


void skylncr_state::io_map_mbutrfly(address_map &map)
{
	map.global_mask(0xff);
	io_map_skylncr(map);
	map(0x60, 0x60).w(FUNC(skylncr_state::mbutrfly_prot_w));
}


void skylncr_state::bdream97_opcode_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(skylncr_state::bdream97_opcode_r));
}


void skylncr_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


void skylncr_state::ramdac2_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac2", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}



/***************************************
*           Graphics Layouts           *
***************************************/

static const gfx_layout layout8x8x8 =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*0,8*1,
		RGN_FRAC(1,2)+8*0,RGN_FRAC(1,2)+8*1,
		8*2,8*3,
		RGN_FRAC(1,2)+8*2,RGN_FRAC(1,2)+8*3
	},
	{ STEP8(0,8*4) },
	8*8*4
};

static const gfx_layout layout8x8x8_alt =   /* for sstar97 */
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*0,RGN_FRAC(1,2)+8*0,
		8*1,RGN_FRAC(1,2)+8*1,
		8*2,RGN_FRAC(1,2)+8*2,
		8*3,RGN_FRAC(1,2)+8*3
	},
	{ STEP8(0,8*4) },
	8*8*4
};

static const gfx_layout layout8x8x8_bdream97 =   /* for bdream97 */
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*0,RGN_FRAC(1,2)+8*0,
		RGN_FRAC(1,2)+8*1,8*1,
		8*2,RGN_FRAC(1,2)+8*2,
		RGN_FRAC(1,2)+8*3,8*3
	},
	{ STEP8(0,8*4) },
	8*8*4
};

static const gfx_layout layout8x32x8 =
{
	8,32,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*0, 8*1,
		RGN_FRAC(1,2)+8*0, RGN_FRAC(1,2)+8*1,
		8*2, 8*3,
		RGN_FRAC(1,2)+8*2, RGN_FRAC(1,2)+8*3
	},
	{
		STEP16(0,8*4),
		STEP16(16*8*4,8*4)
	},
	8*32*8/2
};

/* this will decode the big x2 x3 'correctly' however, maybe they're
   simply not meant to appear correct? */
static const gfx_layout layout8x32x8_rot =
{
	8,32,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*0, 8*1,
		RGN_FRAC(1,2)+8*1, RGN_FRAC(1,2)+8*0,
		8*2, 8*3,
		RGN_FRAC(1,2)+8*3, RGN_FRAC(1,2)+8*2
	},
	{
		STEP16(0,8*4),
		STEP16(16*8*4,8*4)
	},
	8*32*8/2
};

static const gfx_layout layout8x32x8_alt =  /* for sstar97 */
{
	8,32,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		RGN_FRAC(1,2)+8*1, 8*1,
		8*0, RGN_FRAC(1,2)+8*0,
		RGN_FRAC(1,2)+8*3, 8*3,
		8*2, RGN_FRAC(1,2)+8*2
	},
	{
		STEP16(0,8*4),
		STEP16(16*8*4,8*4)
	},
	8*32*8/2
};

static const gfx_layout layout8x32x8_alt2 =  /* for neraidov */
{
	8,32,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		RGN_FRAC(1,2)+8*1, 8*1,
		RGN_FRAC(1,2)+8*0, 8*0,
		RGN_FRAC(1,2)+8*3, 8*3,
		RGN_FRAC(1,2)+8*2, 8*2
	},
	{
		STEP16(0,8*4),
		STEP16(16*8*4,8*4)
	},
	8*32*8/2
};

static const gfx_layout layout8x32x8_bdream97 =  /* for bdream97 */
{
	8,32,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*1, RGN_FRAC(1,2)+8*1,
		8*0, RGN_FRAC(1,2)+8*0,
		8*3, RGN_FRAC(1,2)+8*3,
		8*2, RGN_FRAC(1,2)+8*2
	},
	{
		STEP16(0,8*4),
		STEP16(16*8*4,8*4)
	},
	8*32*8/2
};


/**************************************
*           Graphics Decode           *
**************************************/

static GFXDECODE_START( gfx_skylncr )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x8,        0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8,       0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_rot,   0, 2 )
GFXDECODE_END

static GFXDECODE_START( gfx_neraidou )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x8_alt,    0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_alt2,  0, 2 )
//  GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_alt,   0x100, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_sstar97 )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x8_alt,    0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_alt,   0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_alt,   0x100, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_bdream97 )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x8_bdream97,    0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_bdream97,   0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_bdream97,   0x100, 1 )
GFXDECODE_END


/***********************************
*           Input Ports            *
***********************************/

static INPUT_PORTS_START( skylncr )
	PORT_START("IN1")   /* $00 (PPI0 port A) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")   /* $01 (PPI0 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW) PORT_NAME("Down/Low")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN3")   /* $11 (PPI1 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH) PORT_NAME("Up/High")

	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")

	PORT_START("IN4")   /* $12 (PPI1 port C) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* Settings */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("DSW1")  /* $02 (PPI0 port C) */
	PORT_DIPNAME( 0x11, 0x11, "D-UP Percentage" )
	PORT_DIPSETTING(    0x11, "60%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x0e, 0x0e, "Main Game Percentage" )
	PORT_DIPSETTING(    0x0e, "75%" )
	PORT_DIPSETTING(    0x0c, "78%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x08, "84%" )
	PORT_DIPSETTING(    0x06, "87%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x02, "93%" )
	PORT_DIPSETTING(    0x00, "96%" )
	PORT_DIPNAME( 0x20, 0x20, "Reels Speed" )
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Score" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "24" )
	PORT_DIPNAME( 0x80, 0x00, "Key Out" )
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x80, "x100" )

	PORT_START("DSW2")  /* $10 (PPI1 port A) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x08, "Payout Limit" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x18, "1000" )
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Clown Percentage" )
	PORT_DIPSETTING(    0xc0, "60%" )
	PORT_DIPSETTING(    0x80, "70%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )

	PORT_START("DSW3")  /* AY8910 port A */
	PORT_DIPNAME( 0x07, 0x07, "Coinage A, B & C" )
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Credit" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin / 30 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 40 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credit" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Max Win Bonus" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x00, "32" )

	PORT_START("DSW4")  /* AY8910 port B */
	PORT_DIPNAME( 0x07, 0x07, "Remote Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x01, "1 Pulse / 110 Credits" )
	PORT_DIPSETTING(    0x02, "1 Pulse / 120 Credits" )
	PORT_DIPSETTING(    0x03, "1 Pulse / 130 Credits" )
	PORT_DIPSETTING(    0x04, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x05, "1 Pulse / 400 Credits" )
	PORT_DIPSETTING(    0x06, "1 Pulse / 500 Credits" )
	PORT_DIPSETTING(    0x07, "1 Pulse / 1000 Credits" )
	PORT_DIPNAME( 0x18, 0x00, "Max Bet" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x10, "64" )
	PORT_DIPSETTING(    0x08, "72" )
	PORT_DIPSETTING(    0x00, "80" )
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


static INPUT_PORTS_START( mbutrfly )
	PORT_INCLUDE(skylncr)

	PORT_MODIFY("IN1")   // $00 (PPI0 port A)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_MODIFY("IN2")   // $01 (PPI0 port B)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3) PORT_NAME("Stop Reel 3, Down/Low")

	PORT_MODIFY("IN3")   // $11 (PPI1 port B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP2) PORT_NAME("Stop Reel 2, Up/High")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP1) PORT_NAME("Stop Reel 1, Double Up")

	PORT_MODIFY("IN4")   // $12 (PPI1 port C)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, skylncr_state, mbutrfly_prot_r)
INPUT_PORTS_END


static INPUT_PORTS_START( leader )
	PORT_START("IN1")   /* $00 (PPI0 port A) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")   /* $01 (PPI0 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET) PORT_NAME("Bet/Throttle")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW) PORT_NAME("Down/Low")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN3")   /* $11 (PPI1 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH) PORT_NAME("Up/High")

	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")

	PORT_START("IN4")   /* $12 (PPI1 port C) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* Settings */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("DSW1")  /* $02 (PPI0 port C) */
	PORT_DIPNAME( 0x11, 0x11, "Butterfly Max Mul" )     PORT_DIPLOCATION("DSW-A:!4,!5")
	PORT_DIPSETTING(    0x11, "5" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "12" )
	PORT_DIPNAME( 0x0e, 0x00, "Main Win Rate" )         PORT_DIPLOCATION("DSW-A:!6,!7,!8")
	PORT_DIPSETTING(    0x0e, "55%" )
	PORT_DIPSETTING(    0x0c, "60%" )
	PORT_DIPSETTING(    0x0a, "65%" )
	PORT_DIPSETTING(    0x08, "70%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x04, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "Reels Speed" )           PORT_DIPLOCATION("DSW-A:!3")
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x00, "Bonus Score" )           PORT_DIPLOCATION("DSW-A:!2")
	PORT_DIPSETTING(    0x40, "24" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x80, 0x00, "Key Out" )               PORT_DIPLOCATION("DSW-A:!1")
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x80, "x100" )

	PORT_START("DSW2")  /* $10 (PPI1 port A) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW-B:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW-B:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )             PORT_DIPLOCATION("DSW-B:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, "Refund Coin Limit" )     PORT_DIPLOCATION("DSW-B:4,5")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x18, "1000" )
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW-B:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Butterfly Win Rate" )    PORT_DIPLOCATION("DSW-B:7,8")
	PORT_DIPSETTING(    0xc0, "15%" )
	PORT_DIPSETTING(    0x80, "20%" )
	PORT_DIPSETTING(    0x40, "25%" )
	PORT_DIPSETTING(    0x00, "30%" )

	PORT_START("DSW3")  /* AY8910 port A */
	PORT_DIPNAME( 0x07, 0x07, "Coinage A, B & C" )      PORT_DIPLOCATION("DSW-D:1,2,3")
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Credit" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin / 30 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 40 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credit" )
	PORT_DIPNAME( 0x18, 0x00, "Credit Limit" )          PORT_DIPLOCATION("DSW-D:4,5")
	PORT_DIPSETTING(    0x00, "120000" )
	PORT_DIPSETTING(    0x08, "100000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPNAME( 0x20, 0x20, "Max Win Bonus" )         PORT_DIPLOCATION("DSW-D:6")
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" )           PORT_DIPLOCATION("DSW-D:7,8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )

	PORT_START("DSW4")  /* AY8910 port B */
	PORT_DIPNAME( 0x07, 0x07, "Remote Credits" )            PORT_DIPLOCATION("DSW-C:1,2,3")
	PORT_DIPSETTING(    0x00, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x01, "1 Pulse / 110 Credits" )
	PORT_DIPSETTING(    0x02, "1 Pulse / 120 Credits" )
	PORT_DIPSETTING(    0x03, "1 Pulse / 130 Credits" )
	PORT_DIPSETTING(    0x04, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x05, "1 Pulse / 400 Credits" )
	PORT_DIPSETTING(    0x06, "1 Pulse / 500 Credits" )
	PORT_DIPSETTING(    0x07, "1 Pulse / 1000 Credits" )
	PORT_DIPNAME( 0x18, 0x18, "High Bet Limit" )            PORT_DIPLOCATION("DSW-C:4,5")
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x10, "50" )
	PORT_DIPSETTING(    0x08, "72" )
	PORT_DIPSETTING(    0x00, "96" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW-C:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW-C:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Coin Lock" )                 PORT_DIPLOCATION("DSW-C:8")
	PORT_DIPSETTING(    0x80, "Locked" )
	PORT_DIPSETTING(    0x00, "Normal" )
INPUT_PORTS_END


static INPUT_PORTS_START( neraidou )
	PORT_START("IN1")   /* $00 (PPI0 port A) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")   /* $01 (PPI0 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET) PORT_NAME("Bet/Throttle")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW) PORT_NAME("Down/Low")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN3")   /* $11 (PPI1 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH) PORT_NAME("Up/High")

	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")

	PORT_START("IN4")   /* $12 (PPI1 port C) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* Settings */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("DSW1")  /* $02 (PPI0 port C) */
	PORT_DIPNAME( 0x01, 0x01, "Hardware Type (could be inverted)" ) // leave it off, otherwise the game works bad and resets
	PORT_DIPSETTING(    0x01, "Rolla" )
	PORT_DIPSETTING(    0x00, "Sky" )
	PORT_DIPNAME( 0x0e, 0x0e, "Main Game Percentage" )
	PORT_DIPSETTING(    0x0e, "91%" )
	PORT_DIPSETTING(    0x0c, "92%" )
	PORT_DIPSETTING(    0x0a, "93%" )
	PORT_DIPSETTING(    0x08, "94%" )
	PORT_DIPSETTING(    0x06, "95%" )
	PORT_DIPSETTING(    0x04, "96%" )
	PORT_DIPSETTING(    0x02, "97%" )
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Reels Speed" )
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Rate" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x80, 0x00, "Key Out" )
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x80, "x100" )

	PORT_START("DSW2")  /* $10 (PPI1 port A) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Butterfly Win Rate" )
	PORT_DIPSETTING(    0xc0, "25%" )
	PORT_DIPSETTING(    0x80, "30%" )
	PORT_DIPSETTING(    0x40, "40%" )
	PORT_DIPSETTING(    0x00, "50%" )

	PORT_START("DSW3")  /* AY8910 port A */
	PORT_DIPNAME( 0x07, 0x07, "Coinage A, B & C" )
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Credit" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin / 30 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 40 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credit" )
	PORT_DIPNAME( 0x18, 0x18, "Credit Limit" )
	PORT_DIPSETTING(    0x00, "120000" )
	PORT_DIPSETTING(    0x08, "100000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPNAME( 0x20, 0x20, "Max Win Bonus" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" )
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )

	PORT_START("DSW4")  /* AY8910 port B */
	PORT_DIPNAME( 0x07, 0x07, "Remote Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x01, "1 Pulse / 110 Credits" )
	PORT_DIPSETTING(    0x02, "1 Pulse / 120 Credits" )
	PORT_DIPSETTING(    0x03, "1 Pulse / 130 Credits" )
	PORT_DIPSETTING(    0x04, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x05, "1 Pulse / 400 Credits" )
	PORT_DIPSETTING(    0x06, "1 Pulse / 500 Credits" )
	PORT_DIPSETTING(    0x07, "1 Pulse / 1000 Credits" )
	PORT_DIPNAME( 0x18, 0x00, "High Bet Limit" )
	PORT_DIPSETTING(    0x18, "104" )
	PORT_DIPSETTING(    0x10, "120" )
	PORT_DIPSETTING(    0x08, "160" )
	PORT_DIPSETTING(    0x00, "240" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Bonus? (Left Side)" )
	PORT_DIPSETTING(    0x80, "x3" )
	PORT_DIPSETTING(    0x00, "No Bonus" )
INPUT_PORTS_END


static INPUT_PORTS_START( gallag50 )
	PORT_START("IN1")   /* $00 (PPI0 port A) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")   /* $01 (PPI0 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET) PORT_NAME("Bet/Throttle")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW) PORT_NAME("Down/Low")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN3")   /* $11 (PPI1 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH) PORT_NAME("Up/High")

	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")

	PORT_START("IN4")   /* $12 (PPI1 port C) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* Settings */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("DSW1")  /* $02 (PPI0 port C) */
	PORT_DIPNAME( 0x01, 0x01, "Hardware Type (could be inverted)" ) // leave it off, otherwise the game works bad and resets
	PORT_DIPSETTING(    0x01, "Rolla" )
	PORT_DIPSETTING(    0x00, "Sky" )
	PORT_DIPNAME( 0x0e, 0x0e, "Main Game Percentage" )
	PORT_DIPSETTING(    0x0e, "91%" )
	PORT_DIPSETTING(    0x0c, "92%" )
	PORT_DIPSETTING(    0x0a, "93%" )
	PORT_DIPSETTING(    0x08, "94%" )
	PORT_DIPSETTING(    0x06, "95%" )
	PORT_DIPSETTING(    0x04, "96%" )
	PORT_DIPSETTING(    0x02, "97%" )
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Reels Speed" )
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Rate" )
	PORT_DIPSETTING(    0x40, "24" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x80, 0x00, "Key Out" )
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x80, "x100" )

	PORT_START("DSW2")  /* $10 (PPI1 port A) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Rolla GFX" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Title" )
	PORT_DIPSETTING(    0x10, "Petalouda (Greek for Butterfly)" )
	PORT_DIPSETTING(    0x00, "Gallag" )
	PORT_DIPNAME( 0x20, 0x20, "Alt GFX Decode (For different HW)" )     // could be a mix with 08 'Rolla GFX'
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Butterfly Win Rate" )
	PORT_DIPSETTING(    0xc0, "15%" )
	PORT_DIPSETTING(    0x80, "20%" )
	PORT_DIPSETTING(    0x40, "25%" )
	PORT_DIPSETTING(    0x00, "30%" )

	PORT_START("DSW3")  /* AY8910 port A */
	PORT_DIPNAME( 0x07, 0x07, "Coinage A, B & C" )
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Credit" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin / 30 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 40 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credit" )
	PORT_DIPNAME( 0x18, 0x18, "Credit Limit" )
	PORT_DIPSETTING(    0x00, "120000" )
	PORT_DIPSETTING(    0x08, "100000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPNAME( 0x20, 0x20, "Max Win Bonus" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" )
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )

	PORT_START("DSW4")  /* AY8910 port B */
	PORT_DIPNAME( 0x07, 0x07, "Remote Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x01, "1 Pulse / 110 Credits" )
	PORT_DIPSETTING(    0x02, "1 Pulse / 120 Credits" )
	PORT_DIPSETTING(    0x03, "1 Pulse / 130 Credits" )
	PORT_DIPSETTING(    0x04, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x05, "1 Pulse / 400 Credits" )
	PORT_DIPSETTING(    0x06, "1 Pulse / 500 Credits" )
	PORT_DIPSETTING(    0x07, "1 Pulse / 1000 Credits" )
	PORT_DIPNAME( 0x18, 0x00, "High Bet Limit" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x10, "50" )
	PORT_DIPSETTING(    0x08, "72" )
	PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPNAME( 0x20, 0x00, "Rolla HW" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown Feature" )
	PORT_DIPSETTING(    0x80, "Full Cutted" )
	PORT_DIPSETTING(    0x00, "Full Open" )
INPUT_PORTS_END


static INPUT_PORTS_START( sstar97 )
	PORT_START("IN1")   /* $00 (PPI0 port A) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")   /* $01 (PPI0 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN3")   /* $11 (PPI1 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")

	PORT_START("IN4")   /* $12 (PPI1 port C) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* Settings */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("DSW1")  /* $02 (PPI0 port C) */
	PORT_DIPNAME( 0x11, 0x11, "D-UP Percentage" )
	PORT_DIPSETTING(    0x11, "60%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x0e, 0x0e, "Special Bonus Appearance (per 1000)" )
	PORT_DIPSETTING(    0x0e, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x0a, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x06, "9" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x02, "11" )
	PORT_DIPSETTING(    0x00, "12" )
	PORT_DIPNAME( 0x20, 0x20, "Reels Speed" )
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Score" )
	PORT_DIPSETTING(    0x00, "24" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPNAME( 0x80, 0x00, "Key Out" )
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x80, "x100" )

	PORT_START("DSW2")  /* $10 (PPI1 port A) */
	PORT_DIPNAME( 0x03, 0x03, "Main Game Percentage" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, "Payout Limit" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x18, "1000" )
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Clown Percentage" )
	PORT_DIPSETTING(    0xc0, "60%" )
	PORT_DIPSETTING(    0x80, "70%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )

	PORT_START("DSW3")  /* AY8910 port A */
	PORT_DIPNAME( 0x07, 0x07, "Coinage A, B & C" )
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Credit" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin / 30 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 40 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credit" )
	PORT_DIPNAME( 0x18, 0x18, "Base Bonus (Bonus Bottom)" )
	PORT_DIPSETTING(    0x18, "200" )
	PORT_DIPSETTING(    0x10, "400" )
	PORT_DIPSETTING(    0x08, "600" )
	PORT_DIPSETTING(    0x00, "800" )
	PORT_DIPNAME( 0x20, 0x20, "Max Win Bonus" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x00, "32" )

	PORT_START("DSW4")  /* AY8910 port B */
	PORT_DIPNAME( 0x07, 0x07, "Remote Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x01, "1 Pulse / 110 Credits" )
	PORT_DIPSETTING(    0x02, "1 Pulse / 120 Credits" )
	PORT_DIPSETTING(    0x03, "1 Pulse / 130 Credits" )
	PORT_DIPSETTING(    0x04, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x05, "1 Pulse / 400 Credits" )
	PORT_DIPSETTING(    0x06, "1 Pulse / 500 Credits" )
	PORT_DIPSETTING(    0x07, "1 Pulse / 1000 Credits" )
	PORT_DIPNAME( 0x18, 0x18, "Max Bet" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x10, "64" )
	PORT_DIPSETTING(    0x08, "72" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0xe0, 0xe0, "Super Star Appearance (per 1000)" )
	PORT_DIPSETTING(    0xe0, "6" )
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0xa0, "10" )
	PORT_DIPSETTING(    0x80, "12" )
	PORT_DIPSETTING(    0x60, "14" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x20, "18" )
	PORT_DIPSETTING(    0x00, "20" )
INPUT_PORTS_END


static INPUT_PORTS_START( sonikfig )
	PORT_START("IN1")   /* $00 (PPI0 port A) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_D) PORT_NAME("IN1-02")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_F) PORT_NAME("IN1-08")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_G) PORT_NAME("IN1-20")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_H) PORT_NAME("IN1-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_J) PORT_NAME("IN1-80")

	PORT_START("IN2")   /* $01 (PPI0 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_K) PORT_NAME("IN2-02")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW) PORT_NAME("Down/Low") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_L) PORT_NAME("IN2-08")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start")  // OK
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_T) PORT_NAME("Reset #2") // Behaves like a reset, only when attract DSW is off...
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_Y) PORT_NAME("IN2-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_U) PORT_NAME("IN2-80")

	PORT_START("IN3")   /* $11 (PPI1 port B) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)     // OK
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)     // OK
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)     // OK
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )              // OK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH) PORT_NAME("Up/High") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_I) PORT_NAME("IN3-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")

	PORT_START("IN4")   /* $12 (PPI1 port C) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-01")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset #1")  // OK
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   // Settings OK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-20")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START("DSW1")  /* $02 (PPI0 port C) */
	PORT_DIPNAME( 0x11, 0x00, "D-UP Percentage" )  // OK
	PORT_DIPSETTING(    0x11, "40%" )  // OK
	PORT_DIPSETTING(    0x01, "50%" )  // OK
	PORT_DIPSETTING(    0x10, "60%" )  // OK
	PORT_DIPSETTING(    0x00, "70%" )  // OK
	PORT_DIPNAME( 0x0e, 0x00, "Main Game Percentage" )  // OK
	PORT_DIPSETTING(    0x0e, "55%" )  // OK
	PORT_DIPSETTING(    0x0c, "60%" )  // OK
	PORT_DIPSETTING(    0x0a, "65%" )  // OK
	PORT_DIPSETTING(    0x08, "70%" )  // OK
	PORT_DIPSETTING(    0x06, "75%" )  // OK
	PORT_DIPSETTING(    0x04, "80%" )  // OK
	PORT_DIPSETTING(    0x02, "85%" )  // OK
	PORT_DIPSETTING(    0x00, "90%" )  // OK
	PORT_DIPNAME( 0x20, 0x00, "Reels Speed" )  // OK
	PORT_DIPSETTING(    0x20, "Low" )  // OK
	PORT_DIPSETTING(    0x00, "Hi" )  // OK
	PORT_DIPNAME( 0x40, 0x00, "Bonus Score" )  // OK
	PORT_DIPSETTING(    0x40, "32" )  // OK
	PORT_DIPSETTING(    0x00, "24" )  // OK
	PORT_DIPNAME( 0x80, 0x00, "Payout" )  // OK
	PORT_DIPSETTING(    0x00, "x1" )  // OK
	PORT_DIPSETTING(    0x80, "x100" )  // OK

	PORT_START("DSW2")  /* $10 (PPI1 port A) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )  // OK
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )  // OK
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )  // OK
	PORT_DIPNAME( 0x18, 0x18, "Payout Limit" )  // OK
	PORT_DIPSETTING(    0x00, "0" )  // OK
	PORT_DIPSETTING(    0x18, "1000" )  // OK
	PORT_DIPSETTING(    0x10, "2000" )  // OK
	PORT_DIPSETTING(    0x08, "5000" )  // OK
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* AY8910 port A */
	PORT_DIPNAME( 0x07, 0x07, "Coinage A, B & C" )  // OK on test
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Credit" )  // OK on test, always 1c-1c in game...
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Credits" )  // OK on test, always 1c-1c in game...
	PORT_DIPSETTING(    0x02, "1 Coin / 10 Credits" )  // OK on test, always 1c-1c in game...
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )  // OK on test, always 1c-1c in game...
	PORT_DIPSETTING(    0x04, "1 Coin / 30 Credits" )  // OK on test, always 1c-1c in game...
	PORT_DIPSETTING(    0x05, "1 Coin / 40 Credits" )  // OK on test, always 1c-1c in game...
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )  // OK on test, always 1c-1c in game...
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credit" )  // OK on test, always 1c-1c in game...
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )  // This input should be turned ON, otherwise you can't enter the setup (F2)
	PORT_DIPNAME( 0x10, 0x00, "Attract / Girls" )  // OK... Could be either or both. Need the game working to check it.
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Max Bonus" )  // OK
	PORT_DIPSETTING(    0x20, "10000" )  // OK
	PORT_DIPSETTING(    0x00, "20000" )  // OK
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" )  // OK
	PORT_DIPSETTING(    0xc0, "0" )  // OK
	PORT_DIPSETTING(    0x80, "8" )  // OK
	PORT_DIPSETTING(    0x40, "16" )  // OK
	PORT_DIPSETTING(    0x00, "32" )  // OK

	PORT_START("DSW4")  /* AY8910 port B */
	PORT_DIPNAME( 0x07, 0x00, "Key In" )  // OK on test
	PORT_DIPSETTING(    0x00, "1 Pulse / 100 Credits" )  // OK on test, always 1 credit in game...
	PORT_DIPSETTING(    0x01, "1 Pulse / 110 Credits" )  // OK on test, always 1 credit in game...
	PORT_DIPSETTING(    0x02, "1 Pulse / 120 Credits" )  // OK on test, always 1 credit in game...
	PORT_DIPSETTING(    0x03, "1 Pulse / 130 Credits" )  // OK on test, always 1 credit in game...
	PORT_DIPSETTING(    0x04, "1 Pulse / 200 Credits" )  // OK on test, always 1 credit in game...
	PORT_DIPSETTING(    0x05, "1 Pulse / 400 Credits" )  // OK on test, always 1 credit in game...
	PORT_DIPSETTING(    0x06, "1 Pulse / 500 Credits" )  // OK on test, always 1 credit in game...
	PORT_DIPSETTING(    0x07, "1 Pulse / 1000 Credits" )  // OK on test, always 1 credit in game...
	PORT_DIPNAME( 0x18, 0x00, "Max Bet" )  // OK
	PORT_DIPSETTING(    0x18, "64" )  // OK
	PORT_DIPSETTING(    0x10, "72" )  // OK
	PORT_DIPSETTING(    0x08, "80" )  // OK
	PORT_DIPSETTING(    0x00, "96" )  // OK
	PORT_DIPNAME( 0x20, 0x00, "Lit" )  // OK
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )  // OK
	PORT_DIPSETTING(    0x00, "Yes (50.000)" )  // OK, 50.000
	PORT_DIPNAME( 0x40, 0x00, "Control" )  // OK
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )  // OK
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )  // OK
	PORT_DIPNAME( 0x80, 0x00, "Reel Cover" )  // OK
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )  // OK
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )  // OK
INPUT_PORTS_END



// It runs in IM 0, thus needs an opcode on the data bus
INTERRUPT_GEN_MEMBER(skylncr_state::skylncr_vblank_interrupt)
{
	if (m_nmi_enable) device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


/*************************************
*           Machine Driver           *
*************************************/

void skylncr_state::skylncr(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &skylncr_state::mem_map_skylncr);
	m_maincpu->set_addrmap(AS_IO, &skylncr_state::io_map_skylncr);
	m_maincpu->set_vblank_int("screen", FUNC(skylncr_state::skylncr_vblank_interrupt));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* 1x M5M82C255, or 2x PPI8255 */
	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("IN1");
	ppi0.in_pb_callback().set_ioport("IN2");
	ppi0.in_pc_callback().set_ioport("DSW1");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("DSW2");
	ppi1.in_pb_callback().set_ioport("IN3");
	ppi1.in_pc_callback().set_ioport("IN4");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(HOPPER_PULSE), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(skylncr_state::screen_update_skylncr));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_skylncr);
	PALETTE(config, m_palette).set_entries(0x200);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &skylncr_state::ramdac_map);
	ramdac.set_color_base(0);

	ramdac_device &ramdac2(RAMDAC(config, "ramdac2", 0, m_palette));
	ramdac2.set_addrmap(0, &skylncr_state::ramdac2_map);
	ramdac2.set_color_base(0x100);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK/8));
	aysnd.port_a_read_callback().set_ioport("DSW3");
	aysnd.port_b_read_callback().set_ioport("DSW4");
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


void skylncr_state::mbutrfly(machine_config &config)
{
	skylncr(config);

	m_maincpu->set_addrmap(AS_IO, &skylncr_state::io_map_mbutrfly);
}


void skylncr_state::neraidou(machine_config &config)
{
	skylncr(config);

	m_gfxdecode->set_info(gfx_neraidou);
}


void skylncr_state::sstar97(machine_config &config)
{
	skylncr(config);

	m_gfxdecode->set_info(gfx_sstar97);
}


void skylncr_state::bdream97(machine_config &config)
{
	skylncr(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_OPCODES, &skylncr_state::bdream97_opcode_map);

	m_gfxdecode->set_info(gfx_bdream97);
}


/**********************************
*            ROM Load             *
**********************************/
/*

Sky Lancer PCB Layout
---------------------

  |--------------------------------------------|
 _|                          ROM.U33           |
|                                              |
|                            ROM.U32           |
|    WF19054                                   |
|                                              |
|_                                             |
  |                                  6264      |
  |                     |------|     6116      |
 _|           DSW4(8)   |ACTEL |               |
|             DSW3(8)   |A1010B|               |
|             DSW2(8)   |      |          6264 |
|             DSW1(8)   |------|               |
|                                         6264 |
|    M5M82C255                                 |
|                                              |
|       ROM.U35                                |
|3.6V_BATT                                     |
|_          6116              Z80        12MHz |
  |--------------------------------------------|
Notes:
      Z80 @ 3.0MHz [12/4]
      WF19054 = AY-3-8910 @ 1.5MHz [12/8]
*/

ROM_START( skylncr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27512.u35",  0x00000, 0x10000, CRC(98b1c9fe) SHA1(9ca1706d25038a078fb07ba5c2e6681ed468bc88) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "574200.u32", 0x00000, 0x80000, CRC(b36f11fe) SHA1(1d8660ac1ca44e33976ac14210e4a3a201f8f3c4) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "574200.u33", 0x00000, 0x80000, CRC(19b25221) SHA1(2f32d337125a9fd0bc7f50713b05e564fd4f81b2) )
ROM_END

ROM_START( butrfly )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "butterfly.prg",  0x00000, 0x10000, CRC(b35b289c) SHA1(5a02bfb6e1fb608099b9f491c10795ef888a3b36) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u29", 0x00000, 0x20000, CRC(2ff775ea) SHA1(2219c75cbac2969485607446ab116587bdee7278) )
	ROM_LOAD16_BYTE( "u31", 0x00001, 0x20000, CRC(029d2214) SHA1(cf8256157db0b297ed457b3da6b6517907128843) )
	ROM_LOAD16_BYTE( "u33", 0x40000, 0x20000, CRC(37bad677) SHA1(c077f0c07b097b376a01e5637446e4c4f82d9e28) )
	ROM_LOAD16_BYTE( "u35", 0x40001, 0x20000, CRC(d14c7713) SHA1(c229ef64f3b0a04ff8e27bc56cff6a55ca34b80c) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "u52", 0x00000, 0x20000, CRC(15051537) SHA1(086c38c05c605f297a7bc470eb51763a7648e72c) )
	ROM_LOAD16_BYTE( "u54", 0x00001, 0x20000, CRC(8e34d029) SHA1(ae316f2f34768938a07d62db110ce59d2751abaa) )
	ROM_LOAD16_BYTE( "u56", 0x40000, 0x20000, CRC(a53daaef) SHA1(7b88bb986bd5e47576163d6999f8770c720c5bfc) )
	ROM_LOAD16_BYTE( "u58", 0x40001, 0x20000, CRC(21ca47f8) SHA1(b192be06a2eb817776309580dc64fd76772a8d50) )
ROM_END

ROM_START( mbutrfly )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "magical_butterfy_x4_cleco.bin",  0x00000, 0x10000, CRC(2391778f) SHA1(f82ee9fb571547fda70867e091317779e2fe6e80) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "mb.u29", 0x00000, 0x20000, CRC(294b1cc5) SHA1(56f143d7d96b9ace0973d7001a13e7e55967e70a) )
	ROM_LOAD16_BYTE( "mb.u31", 0x00001, 0x20000, CRC(c6f4e629) SHA1(97334c7dcfea9a405996c06a79cf3c34a360f807) )
	ROM_LOAD16_BYTE( "mb.u33", 0x40000, 0x20000, CRC(72d22790) SHA1(d7a995e95f17bd4324f02aa16d23bfd78f95b5c5) )
	ROM_LOAD16_BYTE( "mb.u35", 0x40001, 0x20000, CRC(fdaa2288) SHA1(199323c2bd2af0d9b1d254a330670e2845f21dd9) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mb.u52", 0x00000, 0x20000, CRC(15051537) SHA1(086c38c05c605f297a7bc470eb51763a7648e72c) ) // identical to butterfly.
	ROM_LOAD16_BYTE( "mb.u54", 0x00001, 0x20000, CRC(8e34d029) SHA1(ae316f2f34768938a07d62db110ce59d2751abaa) ) // identical to butterfly.
	ROM_LOAD16_BYTE( "mb.u56", 0x40000, 0x20000, CRC(21ca47f8) SHA1(b192be06a2eb817776309580dc64fd76772a8d50) ) // this one is straight fixed for reel tiles 6C-DF.
	ROM_LOAD16_BYTE( "mb.u58", 0x40001, 0x20000, CRC(21ca47f8) SHA1(b192be06a2eb817776309580dc64fd76772a8d50) ) // identical to butterfly.
ROM_END

ROM_START( gallag50 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27512.u9",  0x00000, 0x10000, CRC(355f8c3b) SHA1(d419fcb96bf936eaf7afb1a4b38a9e1d2a191686) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "27c301.u29", 0x00000, 0x20000, CRC(adf3208a) SHA1(251e94efe90b8250cb8d75255bd5b58a6b7825a7) )
	ROM_LOAD16_BYTE( "27c301.u31", 0x00001, 0x20000, CRC(24f20819) SHA1(a6fd1febe061f84e2bf3609bbf7c95912a04db70) )
	ROM_LOAD16_BYTE( "27c301.u33", 0x40000, 0x20000, CRC(7ec63f5a) SHA1(47e91f21a674a949b0085cf2d8463da245328db3) )
	ROM_LOAD16_BYTE( "27c301.u35", 0x40001, 0x20000, CRC(68c186d6) SHA1(6e8b5d489c4ba2a65dadf3a8cb3c07c2fedc1cb1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "27c301.u52", 0x00000, 0x20000, CRC(f3de59f4) SHA1(3b70a8677647a54333049a59cf01ba2c63c33d87) )
	ROM_LOAD16_BYTE( "27c301.u54", 0x00001, 0x20000, CRC(f399751b) SHA1(2b7f3ed4181d654b67154b9587ee971ee881f35c) )
	ROM_LOAD16_BYTE( "27c301.u56", 0x40000, 0x20000, CRC(7eec4edf) SHA1(897fbbcda8d1c42b3a59f0f2f68e295e054d062d) )
	ROM_LOAD16_BYTE( "27c301.u58", 0x40001, 0x20000, CRC(afd0d391) SHA1(cc024d37cc23f94ef09a8a543cb1604e3f82c306) )
ROM_END

/*

Mad Zoo PCB Layout
------------------

|-----|  |------|  |---------------------------|
|     |--|      |--|ROM.U29           ROM.U52  |
|                                              |
| DSW3(8)           ROM.U31           ROM.U54  |
|    KC89C72                                   |
| DSW4(8)           ROM.U33           ROM.U56  |
|_                                             |
  |       PAL       ROM.U35           ROM.U58  |
  |                     |-------|              |
 _|                     |LATTICE|         6116 |
|             12MHz     |1016   |              |
|                       |       |         6116 |
|       8255            |-------|              |
|                                         6116 |
| DSW1(8)  DSW2(8)                             |
|       8255   PAL  ROM.U9                6116 |
|                                              |
|    6264      Z80  6116                  6116 |
|_   6264                          PAL  BATTERY|
  |--------------------------------------------|
Notes:
      Z80 @ 3.0MHz [12/4]
      KC89C72 = AY-3-8910 @ 1.5MHz [12/8]
*/

ROM_START( madzoo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27512.u9",  0x00000, 0x10000, CRC(98b1c9fe) SHA1(9ca1706d25038a078fb07ba5c2e6681ed468bc88) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "27c301.u29", 0x00000, 0x20000, CRC(44645bb8) SHA1(efaf88d63e09029aa023ddaf72dbd9ee1df10315) )
	ROM_LOAD16_BYTE( "27c301.u31", 0x00001, 0x20000, CRC(58267dbc) SHA1(dd64e4b44d10e2d93ded255622891f058b2b8bb9) )
	ROM_LOAD16_BYTE( "27c301.u33", 0x40000, 0x20000, CRC(6adb1c2c) SHA1(d782a778a34e6240a3ae09cd11124790864a9149) )
	ROM_LOAD16_BYTE( "27c301.u35", 0x40001, 0x20000, CRC(a8d3a174) SHA1(b668bb1db1d27aff52e808aa9b972f24693161b3) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "27c301.u52", 0x00000, 0x20000, CRC(dd1997ed) SHA1(9197a0b4a0b6284ae7eeb6364c87589f6f8a614d) )
	ROM_LOAD16_BYTE( "27c301.u54", 0x00001, 0x20000, CRC(a654a6df) SHA1(54292953df1103ad830e1f40fdf96c48e0e13be7) )
	ROM_LOAD16_BYTE( "27c301.u56", 0x40000, 0x20000, CRC(f2e3c394) SHA1(8e09516fe822d7c125be57b154c896ab3e024f98) )
	ROM_LOAD16_BYTE( "27c301.u58", 0x40001, 0x20000, CRC(65d2015b) SHA1(121494a2684276276e2504d6f853718e93f4d458) )
ROM_END

ROM_START( leader )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "leader.prg",  0x00000, 0x10000, CRC(1a6e1129) SHA1(639f687e7720bab89628b377dca0475f17a35041) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "leadergfx1.dmp11", 0x00000, 0x20000, CRC(08acae31) SHA1(8b93066a2159e56607499fe1b1748a70a73a326c) )
	ROM_LOAD16_BYTE( "leadergfx1.dmp21", 0x00001, 0x20000, CRC(88cd7a49) SHA1(f7187c7e3e584180de03998f376001f8d5966882) )
	ROM_LOAD16_BYTE( "leadergfx1.dmp12", 0x40000, 0x20000, CRC(e57e145e) SHA1(3f6169ed1d907de3438787c02dc53c73ca6bdb73) )
	ROM_LOAD16_BYTE( "leadergfx1.dmp22", 0x40001, 0x20000, CRC(e8368d29) SHA1(19e7d7d6e320f5f06e91013cb4c92b3987dbe24e) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "leadergfx2.dmp11", 0x00000, 0x20000, CRC(1d62edf4) SHA1(7ba43bf0d0d0cadd5c7fcbe940ecf3fab5c9127b) )
	ROM_LOAD16_BYTE( "leadergfx2.dmp21", 0x00001, 0x20000, CRC(57b9d159) SHA1(ee98aea160653d55017bd893cc253d23c7b1faf4) )
	ROM_LOAD16_BYTE( "leadergfx2.dmp12", 0x40000, 0x20000, CRC(91e73bf9) SHA1(90a9c1119ae05bbd66a4d3c2266ec02cc53969bd) )
	ROM_LOAD16_BYTE( "leadergfx2.dmp22", 0x40001, 0x20000, CRC(04cc0118) SHA1(016ccbe7daf8c4676830aadcc906a64e2826d11a) )
ROM_END

/*
  Neraidoula

  There is a complete screen of the game Out Run
  inside the graphics ROMs.
  Maybe it's a leftover, or it's a sort of stealth game.
*/
ROM_START( neraidou )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "aepi.prg",  0x00000, 0x10000, CRC(7ac74830) SHA1(1e3322341711e329b40d94ac6ec25fbafb1d4d62) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "ldrbfl4.bin", 0x00000, 0x20000, CRC(9424c24d) SHA1(4fcf66b641db14b5096d0de75a134d2d35c6eb9b) )
	ROM_LOAD16_BYTE( "ldrbfl2.bin", 0x00001, 0x20000, CRC(467dd56b) SHA1(5c64ee7ff2f4cc127b57342daf63c392c5155344) )
	ROM_LOAD16_BYTE( "ldrbfl3.bin", 0x40000, 0x20000, CRC(810ac7f5) SHA1(f0e680a1813d01e4ca4da97c3c45e9373361620b) )
	ROM_LOAD16_BYTE( "ldrbfl1.bin", 0x40001, 0x20000, CRC(c3bd4dc0) SHA1(2696321846e09359122447e6b60db29c5742a36a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "ldrbfl8.bin", 0x00000, 0x20000, CRC(74992877) SHA1(f10f90f844198bba49fc3c74b1e8f40821cd1b56) )
	ROM_LOAD16_BYTE( "ldrbfl6.bin", 0x00001, 0x20000, CRC(4b9fb756) SHA1(21d5abbc19a7e3277316d0ac616bdf0819e563b7) )
	ROM_LOAD16_BYTE( "ldrbfl7.bin", 0x40000, 0x20000, CRC(a1842082) SHA1(0790c1c1c268fe13f2613e594fdf09daae19bbd0) )
	ROM_LOAD16_BYTE( "ldrbfl5.bin", 0x40001, 0x20000, CRC(aa0a9b4e) SHA1(e09e6d3c5283ace1f1c6999cdc97e7dde9105338) )
ROM_END

/*
  Missing In Action.
  from Vegas.

  People call it Ypovrixio (Submarine).
  But is not the real title.
*/
ROM_START( miaction )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27c512_sub_board_miaction.bin",  0x00000, 0x10000, CRC(4865a6de) SHA1(cfa23eef004f9a29d462676d9b9b94a1e84064d6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u29.bin", 0x00000, 0x20000, CRC(14d2a342) SHA1(5745a4db5af55072e49de80fdc29a33dd78af68e) )
	ROM_LOAD16_BYTE( "u31.bin", 0x00001, 0x20000, CRC(9079a3d2) SHA1(b4c624cf7bf45d7879118dac7d999d36717e0395) )
	ROM_LOAD16_BYTE( "u33.bin", 0x40000, 0x20000, CRC(9cf17008) SHA1(9fe1d1522ef0ca271cbe4105f79c55462cec9078) )
	ROM_LOAD16_BYTE( "u35.bin", 0x40001, 0x20000, CRC(e04d0ae8) SHA1(36ae96302225e7485882ec911b04e24cc2bd9e8d) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "u52.bin", 0x00000, 0x20000, CRC(e02ed758) SHA1(a90a3889522f3a8b3f1a4642c191dea8ec6cad21) )
	ROM_LOAD16_BYTE( "u54.bin", 0x00001, 0x20000, CRC(4724f35b) SHA1(a8117adf903238a1aca16c1e1468d0684a1d5b95) )
	ROM_LOAD16_BYTE( "u56.bin", 0x40000, 0x20000, CRC(62ca6bc8) SHA1(2aa2a04c18bf25da509e18de7a32d945a084f37a) )
	ROM_LOAD16_BYTE( "u58.bin", 0x40001, 0x20000, CRC(54e49fa5) SHA1(31e7fb65b78ddf429c794fe94ff63fe5650a9787) )
ROM_END

/*
  Tiger (slot).
  Unknown manufacturer.

  Encrypted program.
  Program seems close to Missing In Action.

*/
ROM_START( tigerslt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27c512_sub_board_tiger.bin",  0x00000, 0x10000, CRC(3c4181bf) SHA1(afc4fcd7ec9a48406242fe7e01a32e1a20216330) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "27c301.u29", 0x00000, 0x20000, CRC(adfdc5d6) SHA1(cdbdf43c081fe5bf6c5435fcd68329930f486b9b) )
	ROM_LOAD16_BYTE( "27c301.u31", 0x00001, 0x20000, CRC(e5054f16) SHA1(c6652fd48cf9ec0dde7ab67da24938e5a845e62f) )
	ROM_LOAD16_BYTE( "27c301.u33", 0x40000, 0x20000, CRC(8f162664) SHA1(1777262177820f0be91e5a77e8ff4c3dae819049) )
	ROM_LOAD16_BYTE( "27c301.u35", 0x40001, 0x20000, CRC(c46d51b6) SHA1(340a627d33433791d3b17736c04267fb3b53d12b) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "27c301.u52", 0x00000, 0x20000, CRC(fba16aa8) SHA1(750a462c507363eca6e9dbcdf5fa07581165758a) )
	ROM_LOAD16_BYTE( "27c301.u54", 0x00001, 0x20000, CRC(6631f487) SHA1(9ac5c2bc177479297b605c4c54ad91fc2e3358e6) )
	ROM_LOAD16_BYTE( "27c301.u56", 0x40000, 0x20000, CRC(9168854f) SHA1(2b9b9b9aa23cb5521b4b25b878911e5029506e47) )
	ROM_LOAD16_BYTE( "27c301.u58", 0x40001, 0x20000, CRC(1d4e4ae8) SHA1(0a1a0b68174bb8954e302f75200fbd642d5275da) )
ROM_END


/*
  明星 97 (Ming Xing 97) / Super Star 97
  Bordun International.

  For amusement only (as seen in the title).
  PCB looks similar to Sky Lancer.

  1x M5M82C255ASP for I/O,
  1x daughterboard with Z80 CPU,
  1x AY-3-8910A

  1x Xilinx XC2064-33 CPLD...

  1x 12.000 Mhz crystal

  2x UM70C171-66
  1x HM6116LP-4
  5x HM6116L-120

  Unfortunately, one extra ROM (u48) is blank.
  Seems to be the one that store the palette at offset $C000.

  BP 170 to see the palette registers...

*/
ROM_START( sstar97 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27256.u15",    0x0000, 0x8000, CRC(a5da4f92) SHA1(82ac70bd379649f130db017aa226d0247db0f3cd) )
	ROM_LOAD( "unknown.u48",  0x8000, 0x8000, BAD_DUMP CRC(9f4c02e3) SHA1(05975184130ea7dd3bb5d32eff77b585bd53e6b5) )   // palette borrowed from other game

	ROM_REGION( 0x80000, "gfx1", 0 )    // All ROMs are 28-pins mask ROMs dumped as Fujitsu MB831000 or TC531000 (mask ROM).
	ROM_LOAD16_BYTE( "bor_dun_4.u23", 0x00000, 0x20000, CRC(d0d0ead1) SHA1(00bfe691cb9020c5d7e21d80a1e059ea2155aad8) )
	ROM_LOAD16_BYTE( "bor_dun_2.u25", 0x00001, 0x20000, CRC(2b0f07b5) SHA1(9bcde623e53697c4b68d2f083f6254596aee64eb) )
	ROM_LOAD16_BYTE( "bor_dun_3.u24", 0x40000, 0x20000, CRC(3c7da3f1) SHA1(8098b33a779fb697984b97f2d7edb9874e6e19d9) )
	ROM_LOAD16_BYTE( "bor_dun_1.u26", 0x40001, 0x20000, CRC(36efdca6) SHA1(e614fbba77e5c7a1e7a1d2970b4f945ee0468196) )

	ROM_REGION( 0x80000, "gfx2", 0 )    // All ROMs are 28-pins mask ROMs dumped as Fujitsu MB831000 or TC531000 (mask ROM).
	ROM_LOAD16_BYTE( "bor_dun_8.u19", 0x00000, 0x20000, CRC(adf0b7ce) SHA1(41d9fb16eb20e1fd2960117b7e4ea23a97b88961) )
	ROM_LOAD16_BYTE( "bor_dun_6.u21", 0x00001, 0x20000, CRC(37be2cbe) SHA1(78acda58aab605cb992c3b9fbaf18d38f768ed1a) )
	ROM_LOAD16_BYTE( "bor_dun_7.u20", 0x40000, 0x20000, CRC(43908665) SHA1(41b9cee0723d9da6934ab7934012fb1625a8f080) )
	ROM_LOAD16_BYTE( "bor_dun_5.u22", 0x40001, 0x20000, CRC(ca17a632) SHA1(d491310ccdbe9b59a1e607f9254646f20700d79d) )
ROM_END

/*
  蝴蝶梦 97 (Húdié Mèng 97)
  Game is encrypted and needs better decoded graphics.
*/
ROM_START( bdream97 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_subboard.bin",    0x00000, 0x08000, CRC(b0056324) SHA1(8299198d5e7ed50967f380ba0fddff5a39eee857) )
	ROM_CONTINUE( 0x0c000, 0x04000 )
	ROM_CONTINUE( 0x08000, 0x04000 )

	ROM_REGION( 0x80000, "gfx1", 0 )    // All ROMs are 27010.
	ROM_LOAD16_BYTE( "27c010.u20", 0x00000, 0x20000, CRC(df1fd438) SHA1(8da4d116e768a3c269a2031db6bf38a5b7707029) )
	ROM_LOAD16_BYTE( "27c010.u22", 0x00001, 0x20000, CRC(e66910eb) SHA1(454dc47caf4ae9408c3d0b759f4a32d346f86ffe) )
	ROM_LOAD16_BYTE( "27c010.u21", 0x40000, 0x20000, CRC(b984f5bd) SHA1(9a21b46d6d497271cd589e01af3f3143946980b1) )
	ROM_LOAD16_BYTE( "27c010.u23", 0x40001, 0x20000, CRC(2b6e6fd7) SHA1(cf0c66c90c6ab3ebc69ebe1e4f29b69b72edfdc2) )

	ROM_REGION( 0x80000, "gfx2", 0 )    // All ROMs are 27010.
	ROM_LOAD16_BYTE( "27c010.u24", 0x00000, 0x20000, CRC(b77f3fc5) SHA1(63f7b46ed19f256a6d84624e191dc1719a57cbed) )
	ROM_LOAD16_BYTE( "27c010.u26", 0x00001, 0x20000, CRC(27cd64ef) SHA1(6b39b2919aca967f72fa16cd61b1641d0fb98d88) )
	ROM_LOAD16_BYTE( "27c010.u25", 0x40000, 0x20000, CRC(bdebdf35) SHA1(245247b23ddeded32519608f4696205bb5541ccc) )
	ROM_LOAD16_BYTE( "27c010.u27", 0x40001, 0x20000, CRC(0a266de4) SHA1(0ff9ad793e77d5419bd446cb73d4968e42305353) )
ROM_END

/*
  Sonik Fighter.
  Greek Version By ZBOUNOS (Z GAMES).
  Year 2000.

  Multiple Butterfly type with naked girls.
  + new features and hold a pair.
  + jackpot.
  + tetris game?.

  Program ROM is encrypted.
*/
ROM_START( sonikfig )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "subboard_27c512.bin",  0x00000, 0x10000, CRC(f9b5b03e) SHA1(3832a7d70b41052f9dca46faa6f311ccc5a817b7) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "1__am27c100.u29", 0x00000, 0x20000, CRC(69824294) SHA1(a221ab3e5d50435c10e3cd4601cfda1c87038a74) )
	ROM_LOAD16_BYTE( "2__am27c100.u31", 0x00001, 0x20000, CRC(5224ed08) SHA1(3ba9af7557f13bf31c529bff7b2b8cfd6e71552c) )
	ROM_LOAD16_BYTE( "3__am27c100.u33", 0x40000, 0x20000, CRC(38fef15c) SHA1(00f7578bac395421fa3289748f85cf3b9d80e04b) )
	ROM_LOAD16_BYTE( "4__am27c100.u35", 0x40001, 0x20000, CRC(6193d30d) SHA1(390b3f451224eb4236c6c921d34b25b702d366e0) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "5__am27c100.u52", 0x00000, 0x20000, CRC(56921033) SHA1(25086b0f5978df04b28b60a30e271f4364112c96) )
	ROM_LOAD16_BYTE( "6__am27c100.u54", 0x00001, 0x20000, CRC(72802c6c) SHA1(c552d438399fb8c983e995d5e78e591982ef96e5) )
	ROM_LOAD16_BYTE( "7__am27c100.u56", 0x40000, 0x20000, CRC(90a09327) SHA1(3bb1150ec397627cc04b46d2bf07538c55e7f116) )
	ROM_LOAD16_BYTE( "8__am27c100.u58", 0x40001, 0x20000, CRC(b02ed0ce) SHA1(ec8ab64210a1b10cdba5ee179e46fc8d6a1a67b6) )
ROM_END


/**********************************
*           Driver Init           *
**********************************/

void skylncr_state::init_sonikfig()
/*
  Encryption: For each 8 bytes group,
  swap byte #1 with #4 and #3 with #6.

       SWAPPED
      /       \
  00 01 02 03 04 05 06 07
            \       /
             SWAPPED

  00 01 02 03 04 05 06 07
      \     \ /     /
       \     X     /
        \   / \   /
         \ /   \ /
          X     X
         / \   / \
        /   \ /   \
       /     X     \
      /     / \     \
  00 04 02 06 01 05 03 07
*/
{
	uint8_t *const ROM = memregion("maincpu")->base();
	for (unsigned x = 0; x < 0x10000; x += 8)
	{
		std::swap(ROM[x + 1], ROM[x + 4]);
		std::swap(ROM[x + 3], ROM[x + 6]);
	}
}

void skylncr_state::init_miaction()
/*
  Encryption:

  0000-0006  ---> unencrypted
  0007-4485  ---> xor'ed with 0x19
  4486-7fff  ---> xor'ed with 0x44
  8000-bfff  ---> seems unencrypted
  c000-d25f  ---> xor'ed with 0x44
  d260-dfff  ---> seems unencrypted
  e000-ffff  ---> xor'ed with 0x19

*/
{
	uint8_t *const ROM = memregion("maincpu")->base();

	for (int x = 0x0007; x < 0x4485; x++)
	{
		ROM[x] = ROM[x] ^ 0x19;
	}

	for (int x = 0x4486; x < 0x7fff; x++)
	{
		ROM[x] = ROM[x] ^ 0x44;
	}

	for (int x = 0xc000; x < 0xd25f; x++)
	{
		ROM[x] = ROM[x] ^ 0x44;
	}

	for (int x = 0xe000; x < 0xffff; x++)
	{
		ROM[x] = ROM[x] ^ 0x19;
	}
}


/****************************************************
*                  Game Drivers                     *
****************************************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT           ROT   COMPANY                 FULLNAME                                          FLAGS
GAME( 1995, skylncr,   0,        skylncr,  skylncr,  skylncr_state,  empty_init,    ROT0, "Bordun International", "Sky Lancer (Bordun, version U450C)",             0 )
GAME( 1995, butrfly,   0,        skylncr,  skylncr,  skylncr_state,  empty_init,    ROT0, "Bordun International", "Butterfly Video Game (version U350C)",           0 )
GAME( 1999, mbutrfly,  0,        mbutrfly, mbutrfly, skylncr_state,  empty_init,    ROT0, "Bordun International", "Magical Butterfly (version U350C, protected)",   0 )
GAME( 1995, madzoo,    0,        skylncr,  skylncr,  skylncr_state,  empty_init,    ROT0, "Bordun International", "Mad Zoo (version U450C)",                        0 )
GAME( 1995, leader,    0,        skylncr,  leader,   skylncr_state,  empty_init,    ROT0, "bootleg",              "Leader (version Z 2E, Greece)",                  0 )
GAME( 199?, gallag50,  0,        skylncr,  gallag50, skylncr_state,  empty_init,    ROT0, "bootleg",              "Gallag Video Game / Petalouda (Butterfly, x50)", 0 )
GAME( 199?, neraidou,  0,        neraidou, neraidou, skylncr_state,  empty_init,    ROT0, "bootleg",              "Neraidoula",                                     0 )
GAME( 199?, miaction,  0,        skylncr,  skylncr,  skylncr_state,  init_miaction, ROT0, "Vegas",                "Missing In Action",                              MACHINE_NOT_WORKING )
GAME( 199?, tigerslt,  0,        skylncr,  skylncr,  skylncr_state,  init_miaction, ROT0, "bootleg",              "Tiger (slot)",                                   MACHINE_NOT_WORKING )
GAME( 199?, sstar97,   0,        sstar97,  sstar97,  skylncr_state,  empty_init,    ROT0, "Bordun International", "Super Star 97 / Ming Xing 97 (version V153B)",   0 )
GAME( 1995, bdream97,  0,        bdream97, skylncr,  skylncr_state,  empty_init,    ROT0, "bootleg (KKK)",        "Hudie Meng 97",                                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2000, sonikfig,  0,        skylncr,  sonikfig, skylncr_state,  init_sonikfig, ROT0, "Z Games",              "Sonik Fighter (version 02, encrypted)",          MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING )
