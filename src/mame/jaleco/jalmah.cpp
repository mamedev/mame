// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Uki
/*******************************************************************************************

MJ-8956 HW games (c) 1989 Jaleco / NMK / UPL

driver by Angelo Salese, based on early work by David Haywood
Special thanks to Uki and Yasu for the priority system explanation.

Similar to the NMK16 / Jaleco Mega System 1 boards but without sprites.

Notes(general):
-I think that the 0xf0000-0xfffff area is shared with the MCU because:
\-The first version of the MCU programs (daireika/mjzoomin) jump in that area.
  Then the MCU upload a 68k code;the programs are likely to be the same for all the games,
  for example if the program jumps to $f0020 it means a sample request.
\-Input ports located there.Program doesn't check these locations at P.O.S.T. and doesn't
  give any work ram error.
\-Additionally all the games have a MCU protection which involves various RAM areas,
  that controls mahjong panel inputs.Protection is simulated by
  adding a value to these RAM areas according to what button is pressed.

TODO:
- kakumei2: unemulated RNG;
- daireika: the ranking screen on the original pcb shows some hearts instead of the "0".
  Some investigation indicates that the game reads area "fe100" onwards for these to be filled.
  These are likely to be provided by one of the mcu snippets...
- kakumei/kakumei2: has weird text layer strings in test mode (like "#p control panel"),
  unsure if this one is somehow related to the above daireika bug, it's a BTANB or something else.
- Check if urashima has a "mode 3" for the layer 0 tilemap (doesn't seem so);
- There could be timing issues caused by MCU simulation at $80004;
- Imperfect sound banking for 1st MCU version (especially noticeable for daireika);
- suchiesp: I need a side-by-side to understand if the PAL shuffling is correct with the OKI BGM ROM.
- urashima: doesn't use the mega system 1 tilemap devices, MCU might actually be responsible
  for that too (cfr. notes).

Notes (1st MCU ver.):
- $f000e is bogus,maybe the program snippets can modify this value,or the MCU itself can
  do that,returning the contents of D0 register seems enough for now...
  Update: Improved it for the new mcu simulation,now it pulls all the values from 0x00 to
  0x0f, it seems to be a MCU call snippet for the $f0000 work ram;
- $f030e is a mirror for $f000e in urashima.
- I need more space for MCU code...that's why I've used an extra jmp when entering
  into mcu code,so we can debug the first version freely without being teased about
  memory space.
  BTW,the real HW is using a sort of bankswitch or I'm missing something?
- $f0020 is for the sound program,same for all games, for example mjzoomin hasn't any clear
  write to $80040 area and the program jumps to $f0020 when there should be a sample.
- Likewise, D0 upper byte is used, not 100% sure about its meaning (banking? voice channel control?).

============================================================================================
Debug cheats:

-(suchiesp)
*
$fe87e: bonus timer,used as a count-down.
*
$f079a: finish match now
*
During gameplay,set $f0400 to 6 then set $f07d4 to 1 to advance to next
level.
*
$f06a6-$f06c0: Your tiles
$f06c6-$f06e0: COM tiles
---- ---- --xx ----: Defines kind
---- ---- ---- xxxx: Defines number
*
$f0434: priority number

============================================================================================
daireika 68k irq table vectors
lev 1 : 0x64 : 0000 049e -
lev 2 : 0x68 : 0000 04ae -
lev 3 : 0x6c : 0000 049e -
lev 4 : 0x70 : 0000 091a -
lev 5 : 0x74 : 0000 0924 -
lev 6 : 0x78 : 0000 092e -
lev 7 : 0x7c : 0000 0938 -

mjzoomin 68k irq table vectors
lev 1 : 0x64 : 0000 048a -
lev 2 : 0x68 : 0000 049a - vblank
lev 3 : 0x6c : 0000 048a -
lev 4 : 0x70 : 0000 09ba - "write to Text RAM" (?)
lev 5 : 0x74 : 0000 09c4 - "write to Text RAM" (?)
lev 6 : 0x78 : 0000 09ce - "write to Text RAM" (?)
lev 7 : 0x7c : 0000 09d8 - "write to Text RAM" (?)

kakumei/kakumei2/suchiesp 68k irq table vectors
lev 1 : 0x64 : 0000 0506 - rte
lev 2 : 0x68 : 0000 050a - vblank
lev 3 : 0x6c : 0000 051c - rte
lev 4 : 0x70 : 0000 0520 - rte
lev 5 : 0x74 : 0000 0524 - rte
lev 6 : 0x78 : 0000 0524 - rte
lev 7 : 0x7c : 0000 0524 - rte

Board:  MJ-8956

CPU:    68000-8
        M50747 (not dumped)
Sound:  M6295
OSC:    12.000MHz
        4.000MHz


2009-04: Verified DipLocations and Default settings with manual (thanks to Uki)

*******************************************************************************************/

#include "emu.h"

#include "ms1_tmap.h"

#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class jalmah_state : public driver_device
{
public:
	jalmah_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_tmap(*this, "scroll%u", 0),
		m_jmcu_rom_region(*this, "jmcu_rom"),
		m_jmcu_rom_share(*this, "jmcu_rom"),
		m_sharedram(*this, "sharedram"),
		m_prirom(*this, "prirom"),
		m_p1_key_io(*this, "P1_KEY%u", 0U),
		m_p2_key_io(*this, "P2_KEY%u", 0U),
		m_okibank(*this, "okibank"),
		m_system_io(*this, "SYSTEM"),
		m_dsw_io(*this, "DSW")
	{ }

	void tilebank_w(uint8_t data);
	void okirom_w(uint8_t data);
	void okibank_w(uint8_t data);
	void flip_screen_w(uint8_t data);
	uint16_t urashima_mcu_r();
	void urashima_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t daireika_mcu_r();
	void daireika_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mjzoomin_mcu_r();
	void mjzoomin_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t kakumei_mcu_r();
	uint16_t suchiesp_mcu_r();
	void init_suchiesp();
	void init_kakumei();
	void init_urashima();
	void init_kakumei2();
	void init_daireika();
	void init_mjzoomin();
	uint32_t screen_update_jalmah(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_sim);
	void refresh_priority_system();
	void daireika_mcu_run();
	void mjzoomin_mcu_run();
	void urashima_mcu_run();
	void second_mcu_run();
	void jalmah(machine_config &config);
	void jalmahv1(machine_config &config);
	void jalmah_map(address_map &map) ATTR_COLD;
	void jalmahv1_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	optional_device_array<megasys1_tilemap_device, 4> m_tmap;
	optional_region_ptr<uint16_t> m_jmcu_rom_region;
	optional_shared_ptr<uint16_t> m_jmcu_rom_share;

	uint16_t m_tile_bank;
	uint16_t m_pri;
	void mcu_check_test_mode();

private:
	required_shared_ptr<uint16_t> m_sharedram;
	optional_region_ptr<uint8_t> m_prirom;
	required_ioport_array<3> m_p1_key_io;
	optional_ioport_array<3> m_p2_key_io;
	required_memory_bank m_okibank;
	required_ioport m_system_io;
	required_ioport m_dsw_io;
	uint8_t m_layer_prin[4];
	void mcu_fetch_input_polling(required_ioport_array<3> &port, uint16_t workram_offset);

	uint8_t m_mcu_prg;
	int m_respcount;
	uint8_t m_test_mode;
	uint16_t m_prg_prot;
	uint8_t m_oki_rom;
	uint8_t m_oki_bank;
	uint8_t m_oki_za;

	// arbitrary numbering scheme for the MCU sims
	enum {
		URASHIMA_MCU = 0x11,
		DAIREIKA_MCU = 0x12,
		MJZOOMIN_MCU = 0x13,
		KAKUMEI_MCU = 0x21,
		KAKUMEI2_MCU = 0x22,
		SUCHIESP_MCU = 0x23
	};

	// base values for the MCU code snippets (arbitrary)
	enum {
		SNIPPET_PALETTE1 = 0x0000,
		SNIPPET_SOUND = 0x0800,
		SNIPPET_TILE = 0x1000,
		SNIPPET_PALETTE2 = 0x1800,
		SNIPPET_RNG = 0x2000,
		SNIPPET_CLR_LAYER0 = 0x2800,
		SNIPPET_CLR_LAYER1 = 0x2880,
		SNIPPET_CLR_LAYER2 = 0x2900
	};
};

class urashima_state : public jalmah_state
{
public:
	urashima_state(const machine_config &mconfig, device_type type, const char *tag)
		: jalmah_state(mconfig, type, tag),
		  m_videoram(*this, "videoram%u", 0U),
		  m_vreg(*this, "vreg%u", 0U),
		  m_gfxdecode(*this, "gfxdecode")
	{}

	template<int TileChip> uint16_t urashima_vregs_r(offs_t offset);
	template<int TileChip> void urashima_vregs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	template<int TileChip> uint16_t urashima_vram_r(offs_t offset);
	template<int TileChip> void urashima_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void urashima_vreg_log_w(offs_t offset, uint16_t data);
	void urashima_priority_w(uint8_t data);

	template<int TileChip> TILE_GET_INFO_MEMBER(get_tile_info_urashima);

	void urashima(machine_config &config);
	void urashima_map(address_map &map) ATTR_COLD;

	uint32_t screen_update_urashima(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void urashima_bank_w(uint8_t data);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr_array<uint16_t, 2> m_videoram;
	required_shared_ptr_array<uint16_t, 2> m_vreg;
	required_device<gfxdecode_device> m_gfxdecode;

	tilemap_t *m_layer[2]{};

	TILEMAP_MAPPER_MEMBER(range0_16x16);
	TILEMAP_MAPPER_MEMBER(range3_8x8);
};

/******************************************************************************************

Video Hardware start

******************************************************************************************/

void jalmah_state::video_start()
{
	m_tile_bank = 0;
	// ...
}

template<int TileChip>
TILE_GET_INFO_MEMBER(urashima_state::get_tile_info_urashima)
{
	int code = m_videoram[TileChip][tile_index];
	uint16_t tile = code & 0xfff;
	int region = (TileChip == 0) ? m_tile_bank : 3;

	tileinfo.set(region,
			tile,
			code >> 12,
			0);
}

TILEMAP_MAPPER_MEMBER(urashima_state::range0_16x16)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0xff) << 4) + ((row & 0x70) << 8);
}

TILEMAP_MAPPER_MEMBER(urashima_state::range3_8x8)
{
	return (row & 0x1f) + ((col & 0x3f) * 0x20) + ((row & 0x20) * 0x40);
}

void urashima_state::video_start()
{
	jalmah_state::video_start();

	m_layer[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(urashima_state::get_tile_info_urashima<0>)), tilemap_mapper_delegate(*this, FUNC(urashima_state::range0_16x16)), 16,16,256,32);
	// range confirmed with title screen transition in attract mode
	// also it's confirmed to be 64 x 64 with 2nd tier girls stripping
	m_layer[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(urashima_state::get_tile_info_urashima<1>)), tilemap_mapper_delegate(*this, FUNC(urashima_state::range3_8x8)), 8,8,64,64);

	for(int i=0;i<2;i++)
		m_layer[i]->set_transparent_pen(15);
}


/***************************************************************************************
The priority system is a combination between one prom and the priority number.
The priority number is a pointer to an array of 16 bytes of the prom ( addresses bits 4-7
0x*0-0x*f). These 16 bytes are read and added and every number is directly hooked up
to the equivalent layer (i.e. read 0 == +1 for the layer 0, read 1 == +1 for the layer 1
etc.)
In the end the final results always are one bit assigned to each priority (i.e. most
priority = 8, then 4, 2 and finally 1).
***************************************************************************************/
void jalmah_state::refresh_priority_system()
{
	uint8_t prinum;
	int i;

	// clear old priority buffer
	for(i=0; i<4; i++)
		m_layer_prin[i] = 0;

	// now read from the Priority PROM and hookup layer priority values accordingly
	for(int i=0; i<0x10; i++)
	{
		prinum = m_prirom[i|(m_pri<<4)];

		if(prinum == 0) { m_layer_prin[0]++; }
		if(prinum == 1) { m_layer_prin[1]++; }
		if(prinum == 2) { m_layer_prin[2]++; }
		if(prinum == 3) { m_layer_prin[3]++; }
	}

	//popmessage("%02x %02x %02x %02x",m_sc0_prin,m_sc1_prin,m_sc2_prin,m_sc3_prin);
}


uint32_t jalmah_state::screen_update_jalmah(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t cur_prin;
	int layer_num;

	bitmap.fill(m_palette->pen(0xff), cliprect); //selectable by a ram address?

	for(cur_prin=1; cur_prin<=0x8; cur_prin<<=1)
	{
		for(layer_num=0; layer_num<4; layer_num++)
		{
			if(cur_prin == m_layer_prin[layer_num])
				m_tmap[layer_num]->draw(screen,bitmap,cliprect,0,0);
		}
	}

	return 0;
}

uint32_t urashima_state::screen_update_urashima(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	uint16_t sx[2],sy[2];

	// reset scroll latches
	sx[0] = sx[1] = sy[0] = sy[1] = 0;

	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;

	bitmap.fill(m_palette->pen(0x1ff), cliprect); //selectable by a ram address?

	for(int y = cliprect.min_y; y < cliprect.max_y+1; y++)
	{
		clip.min_y = clip.max_y = y;

		// Urashima tilemaps interrogate the video register area to make row and column scrolling
		// for every scanline the format is:
		// ---- ---- ---- ---- [0] unused?
		// ---- ---- ---- ---x [1] latches new scroll values
		// ---- xxxx xxxx xxxx [2] new scroll x value
		// ---- yyyy yyyy yyyy [3] new scroll y value
		// this is notably used by the big puntos winning animations.
		for(int layer_num=0;layer_num<2;layer_num++)
		{
			// latches fetch go in reverse order when flip screen is enabled
			int y_base = flip_screen() ? 0x3fc-(y*4) : y*4;

			// is there a new latch?
			if(m_vreg[layer_num][1+y_base] & 1)
			{
				sx[layer_num] = m_vreg[layer_num][2+y_base];
				sy[layer_num] = m_vreg[layer_num][3+y_base];
			}

			// set scroll values for this layer
			m_layer[layer_num]->set_scrollx(0,sx[layer_num]);
			m_layer[layer_num]->set_scrolly(0,sy[layer_num]);
		}

		if(m_pri & 1)
		{
			m_layer[0]->draw(screen,bitmap,clip,0,0);
			m_layer[1]->draw(screen,bitmap,clip,0,0);
		}
		else
		{
			m_layer[1]->draw(screen,bitmap,clip,0,0);
			m_layer[0]->draw(screen,bitmap,clip,0,0);
		}
	}

	return 0;
}

void jalmah_state::tilebank_w(uint8_t data)
{
	/*
	 --xx ---- fg bank (used by suchiesp)
	 ---- xxxx Priority number (trusted,see mjzoomin)
	*/
	if (m_tile_bank != ((data & 0x30) >> 4))
	{
		m_tile_bank = (data & 0x30) >> 4;
		m_tmap[0]->set_tilebank(m_tile_bank);
	}
	if (m_pri != (data & 0x0f))
	{
		m_pri = data & 0x0f;
		refresh_priority_system();
	}
}

void urashima_state::urashima_bank_w(uint8_t data)
{
	if (m_tile_bank != (data & 0x03))
	{
		m_tile_bank = (data & 0x03);
		m_layer[0]->mark_all_dirty();

		if(m_tile_bank == 3)
			popmessage("layer 0 bank == 3, contact MAMEdev");
	}
}

template<int TileChip>
uint16_t urashima_state::urashima_vram_r(offs_t offset)
{
	return m_videoram[TileChip][offset];
}

template<int TileChip>
void urashima_state::urashima_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[TileChip][offset]);
	m_layer[TileChip]->mark_tile_dirty(offset);
}

template<int TileChip>
uint16_t urashima_state::urashima_vregs_r(offs_t offset)
{
	return m_vreg[TileChip][offset];
}

template<int TileChip>
void urashima_state::urashima_vregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vreg[TileChip][offset]);
}

// Urashima Mahjong uses a bigger video register area (mostly mirrored?)
// we use this fallback so that it doesn't pollute logerror and get the info we want
void urashima_state::urashima_vreg_log_w(offs_t offset, uint16_t data)
{
	if(data)
		logerror("Warning vreg write to [%04x] -> %04x\n",offset*2,data);
}

void urashima_state::urashima_priority_w(uint8_t data)
{
	m_pri = data & 1;
}

/******************************************************************************************

Protection file start

******************************************************************************************/

#define MCU_READ(_ioport_, _bit_, _offset_, _retval_) \
	if((0xffff - port[_ioport_]->read()) & _bit_) \
		{ m_sharedram[_offset_] = _retval_; }


/* RAM-based protection handlings (input) */
// TODO: unemulated polling mode for player 2 ingame, is it even used?
// TODO: urashima (at least) reads upper byte of the workram buffer, perhaps for checking previous frame input
// TODO: RBSDTL in 1st version MCU stands for Bet/Big/Small/Double Up/Take Score/Last Chance
//      (not hooked up cause none of the dumped games actually uses it)
void jalmah_state::mcu_fetch_input_polling(required_ioport_array<3> &port, uint16_t workram_offset)
{
	MCU_READ(1, 0x0001, workram_offset, 0x00);        // FF (correct?)
	MCU_READ(2, 0x0400, workram_offset, 0x01);        // A
	MCU_READ(2, 0x1000, workram_offset, 0x02);        // B
	MCU_READ(2, 0x0200, workram_offset, 0x03);        // C
	MCU_READ(2, 0x0800, workram_offset, 0x04);        // D
	MCU_READ(2, 0x0004, workram_offset, 0x05);        // E
	MCU_READ(2, 0x0010, workram_offset, 0x06);        // F
	MCU_READ(2, 0x0002, workram_offset, 0x07);        // G
	MCU_READ(2, 0x0008, workram_offset, 0x08);        // H
	MCU_READ(1, 0x0400, workram_offset, 0x09);        // I
	MCU_READ(1, 0x1000, workram_offset, 0x0a);        // J
	MCU_READ(1, 0x0200, workram_offset, 0x0b);        // K
	MCU_READ(1, 0x0800, workram_offset, 0x0c);        // L
	MCU_READ(1, 0x0004, workram_offset, 0x0d);        // M
	MCU_READ(1, 0x0010, workram_offset, 0x0e);        // N
	MCU_READ(0, 0x0200, workram_offset, 0x0f);        // RON
	MCU_READ(0, 0x1000, workram_offset, 0x10);        // REACH
	MCU_READ(0, 0x0400, workram_offset, 0x11);        // KAN
	MCU_READ(1, 0x0008, workram_offset, 0x12);        // PON
	MCU_READ(1, 0x0002, workram_offset, 0x13);        // CHI
	MCU_READ(0, 0x0004, workram_offset, 0x14);        // START1

}

void jalmah_state::daireika_mcu_run()
{
	if(m_test_mode)  //service_mode
	{
		for(int keynum = 0; keynum < 3; keynum++)
		{
			m_sharedram[keynum+0] = m_p1_key_io[keynum]->read();
			m_sharedram[keynum+3] = m_p2_key_io[keynum]->read();
		}
	}
	else
	{
		m_sharedram[0x000/2] = 0x0000;
		mcu_fetch_input_polling(m_p1_key_io, 0x000/2);
	}
	m_prg_prot++;
	if(m_prg_prot > 0x10) { m_prg_prot = 0; }
	m_sharedram[0x00e/2] = m_prg_prot;
}

void jalmah_state::mjzoomin_mcu_run()
{
	if(m_test_mode)  //service_mode
	{
		for(int keynum = 0; keynum < 3; keynum++)
		{
			m_sharedram[keynum+0] = m_p1_key_io[keynum]->read();
			m_sharedram[keynum+3] = m_p2_key_io[keynum]->read();
		}
	}
	else
	{
		m_sharedram[0x000/2] = 0x0000;
		mcu_fetch_input_polling(m_p1_key_io, 0x000/2);
	}

	m_sharedram[0x00c/2] = machine().rand() & 0xffff;
	m_prg_prot++;
	if(m_prg_prot > 0x10) { m_prg_prot = 0; }
	m_sharedram[0x00e/2] = m_prg_prot;
}

void jalmah_state::urashima_mcu_run()
{
	if(m_test_mode)  //service_mode
	{
		for(int keynum = 0; keynum < 3; keynum++)
		{
			m_sharedram[0x300/2+keynum+0] = m_p1_key_io[keynum]->read();
			m_sharedram[0x300/2+keynum+3] = m_p2_key_io[keynum]->read();
		}
	}
	else
	{
		m_sharedram[0x300/2] = 0x0000;
		mcu_fetch_input_polling(m_p1_key_io, 0x300/2);
	}
	m_sharedram[0x30c/2] = machine().rand() & 0xffff;
	m_prg_prot++;
	if(m_prg_prot > 0x10) { m_prg_prot = 0; }
	m_sharedram[0x30e/2] = m_prg_prot;
}

void jalmah_state::second_mcu_run()
{
	if(m_test_mode)  //service_mode
	{
		for(int keynum = 0; keynum < 3; keynum++)
			m_sharedram[0x200/2+keynum] = m_p1_key_io[keynum]->read();
	}
	else
	{
		m_sharedram[0x200/2] = 0x0000;
		mcu_fetch_input_polling(m_p1_key_io, 0x200/2);
	}

	m_sharedram[0x20c/2] = machine().rand() & 0xffff; //kakumei2
}

TIMER_DEVICE_CALLBACK_MEMBER(jalmah_state::mcu_sim)
{
	switch(m_mcu_prg)
	{
			case MJZOOMIN_MCU: mjzoomin_mcu_run(); break;
			case DAIREIKA_MCU: daireika_mcu_run(); break;
			case URASHIMA_MCU: urashima_mcu_run(); break;
			case KAKUMEI_MCU:
			case KAKUMEI2_MCU:
			case SUCHIESP_MCU:  second_mcu_run(); break;
	}
}

/******************************************************************************************

Basic driver start

******************************************************************************************/


void jalmah_state::okirom_w(uint8_t data)
{
	m_oki_rom = data & 1;

	/* ZA appears to be related to the banking, or maybe kakumei2 uses PAL shuffling and this is for something else? */
	m_oki_za = (data & 2) ? 1 : 0;

	//memcpy(&oki[0x20000], &oki[(m_oki_rom * 0x80000) + ((m_oki_bank+m_oki_za) * 0x20000) + 0x40000], 0x20000);
	m_okibank->set_entry((m_oki_rom << 2) + (m_oki_bank+m_oki_za));

	//popmessage("PC=%06x %02x %02x %02x %08x",m_maincpu->pc(),m_oki_rom,m_oki_za,m_oki_bank,(m_oki_rom * 0x80000) + ((m_oki_bank+m_oki_za) * 0x20000) + 0x40000);
}

void jalmah_state::okibank_w(uint8_t data)
{
	m_oki_bank = data & 3;

	//memcpy(&oki[0x20000], &oki[(m_oki_rom * 0x80000) + ((m_oki_bank+m_oki_za) * 0x20000) + 0x40000], 0x20000);
	m_okibank->set_entry((m_oki_rom << 2) + (m_oki_bank+m_oki_za));

	//popmessage("PC=%06x %02x %02x %02x %08x",m_maincpu->pc(),m_oki_rom,m_oki_za,m_oki_bank,(m_oki_rom * 0x80000) + ((m_oki_bank+m_oki_za) * 0x20000) + 0x40000);
}

void jalmah_state::flip_screen_w(uint8_t data)
{
	/*---- ----x flip screen*/
	flip_screen_set(data & 1);

	if(data & 0xfe)
		popmessage("Flip data %02x, contact MAMEdev",data);
}

void jalmah_state::jalmah_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x080001).portr("SYSTEM");
	map(0x080002, 0x080003).portr("DSW");
//      0x080004, 0x080005  MCU read, different for each game
	map(0x080011, 0x080011).w(FUNC(jalmah_state::flip_screen_w));
//      0x080012, 0x080013  MCU write related,same for each game
	map(0x080014, 0x080015).noprw(); // MCU handshake
	map(0x080017, 0x080017).w(FUNC(jalmah_state::tilebank_w));
	map(0x080019, 0x080019).w(FUNC(jalmah_state::okibank_w));
	map(0x08001b, 0x08001b).w(FUNC(jalmah_state::okirom_w));
	map(0x080020, 0x080025).rw("scroll0", FUNC(megasys1_tilemap_device::scroll_r), FUNC(megasys1_tilemap_device::scroll_w));
	map(0x080028, 0x08002d).rw("scroll1", FUNC(megasys1_tilemap_device::scroll_r), FUNC(megasys1_tilemap_device::scroll_w));
	map(0x080030, 0x080035).rw("scroll2", FUNC(megasys1_tilemap_device::scroll_r), FUNC(megasys1_tilemap_device::scroll_w));
	map(0x080038, 0x08003d).rw("scroll3", FUNC(megasys1_tilemap_device::scroll_r), FUNC(megasys1_tilemap_device::scroll_w));
	map(0x080041, 0x080041).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//      0x084000, 0x084001  ?
	map(0x088000, 0x0887ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); /* Palette RAM */
	map(0x090000, 0x093fff).ram().w("scroll0", FUNC(megasys1_tilemap_device::write)).share("scroll0"); // Scroll RAM 0
	map(0x094000, 0x097fff).ram().w("scroll1", FUNC(megasys1_tilemap_device::write)).share("scroll1"); // Scroll RAM 1
	map(0x098000, 0x09bfff).ram().w("scroll2", FUNC(megasys1_tilemap_device::write)).share("scroll2"); // Scroll RAM 2
	map(0x09c000, 0x09ffff).ram().w("scroll3", FUNC(megasys1_tilemap_device::write)).share("scroll3"); // Scroll RAM 3
	map(0x0f0000, 0x0f0fff).ram().share("sharedram");/*shared with MCU*/
	map(0x0f1000, 0x0fffff).ram(); /*Work Ram*/
}

void jalmah_state::jalmahv1_map(address_map &map)
{
	jalmah_map(map);
	map(0x100000, 0x10ffff).ram().share("jmcu_rom"); // extended ROM functions (not on real HW)
}

void urashima_state::urashima_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x080001).portr("SYSTEM");
	map(0x080002, 0x080003).portr("DSW");
//      0x080004, 0x080005  MCU read, different for each game
	map(0x080011, 0x080011).w(FUNC(jalmah_state::flip_screen_w));
//      0x080012, 0x080013  MCU write related, same for each game
	map(0x080014, 0x080015).noprw(); // MCU handshake
/**/map(0x080017, 0x080017).w(FUNC(urashima_state::urashima_priority_w));
	map(0x080019, 0x080019).w(FUNC(jalmah_state::okibank_w));
	map(0x08001b, 0x08001b).w(FUNC(jalmah_state::okirom_w));
/**/map(0x08001c, 0x08001d).ram().w(FUNC(urashima_state::urashima_bank_w)).umask16(0x00ff);
	map(0x080041, 0x080041).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//      0x084000, 0x084001  ?
	map(0x088000, 0x0887ff).mirror(0x800).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); /* Palette RAM */
	map(0x090000, 0x093fff).mirror(0x4000).rw(FUNC(urashima_state::urashima_vram_r<0>),FUNC(urashima_state::urashima_vram_w<0>)).share("videoram0");
	map(0x098000, 0x09bfff).noprw(); // unused mirror? set when coin up then never set anymore
	map(0x09c000, 0x09c7ff).rw(FUNC(urashima_state::urashima_vregs_r<0>), FUNC(urashima_state::urashima_vregs_w<0>)).share("vreg0");
	map(0x09c800, 0x09cfff).rw(FUNC(urashima_state::urashima_vregs_r<1>), FUNC(urashima_state::urashima_vregs_w<1>)).share("vreg1");
	map(0x09d000, 0x09dfff).nopr().w(FUNC(urashima_state::urashima_vreg_log_w)); // cleared at POST then unused

	// likely only 0x9e000-0x9ffff is connected (0xa0000-0xa1fff cleared at POST and nowhere else)
	map(0x09e000, 0x09ffff).rw(FUNC(urashima_state::urashima_vram_r<1>),FUNC(urashima_state::urashima_vram_w<1>)).share("videoram1");
	map(0x0a0000, 0x0a1fff).noprw();
	map(0x0f0000, 0x0f0fff).ram().share("sharedram");/*shared with MCU*/
	map(0x0f1000, 0x0fffff).ram(); /*Work Ram*/
	map(0x100000, 0x10ffff).ram().share("jmcu_rom");/*extra RAM for MCU code prg (NOT ON REAL HW!!!)*/
}

void jalmah_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("okibank");
}

static INPUT_PORTS_START( common )
	PORT_START("SYSTEM")
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( ctrl_mj1 )
	PORT_START("P1_KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN ) // (S) Small
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // (R) Bet?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // (B) Big?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xe8ea, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // (D) Double Up
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0xe0e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN ) // (T) Take Score
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // (L) Last Chance
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0xe0e0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ctrl_mj2 )
	PORT_START("P2_KEY0")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0xe9fb, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2)

	PORT_START("P2_KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0xe1e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY2")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0xe1e1, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( urashima )
	PORT_INCLUDE( common )
	PORT_INCLUDE( ctrl_mj1 )
	PORT_INCLUDE( ctrl_mj2 )

	PORT_MODIFY("SYSTEM")
	// TODO: this probably sends game in test mode while playing (unemulated)
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW ) PORT_NAME("Test Button") PORT_CODE(KEYCODE_F1)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Display Tenpai/Noten" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Pinfu with Tsumo" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, "Number of Chips (Start - Continue)" ) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(      0x0300, "1 - 1" )
	PORT_DIPSETTING(      0x0200, "1 - 2" )
	PORT_DIPSETTING(      0x0100, "2 - 1" )
	PORT_DIPSETTING(      0x0000, "2 - 2" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Number of Players" ) PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(      0x0800, "0" )
	PORT_DIPSETTING(      0x0c00, "1" )
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Chip Added After Win" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x1000, "Less" )
	PORT_DIPSETTING(      0x0000, "More" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW1:3" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW1:2" )   // Unused according to the manual
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( daireika )
	PORT_INCLUDE( common )
	PORT_INCLUDE( ctrl_mj1 )
	PORT_INCLUDE( ctrl_mj2 )

	PORT_MODIFY("SYSTEM")
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW2:6" )   // Unused according to the manual
	PORT_DIPNAME( 0x0040, 0x0040, "Pinfu with Tsumo" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW2:8" )   // Unused according to the manual
	PORT_DIPNAME( 0x0300, 0x0300, "Number of Chips (Start - Continue)" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0300, "1 - 1" )
	PORT_DIPSETTING(      0x0200, "1 - 2" )
	PORT_DIPSETTING(      0x0100, "2 - 1" )
	PORT_DIPSETTING(      0x0000, "2 - 2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW1:3" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW1:4" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW1:5" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW1:6" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW1:7" )   // Unused according to the manual
	/* SW1:8 should be "Switch Control Panel" off: no - on : yes -> likely to be controlled by the MCU. */
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjzoomin )
	PORT_INCLUDE( daireika )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0010, "0 (Easy)" )
	PORT_DIPSETTING(      0x0008, "1 (Easier)" )
	PORT_DIPSETTING(      0x0000, "2 (Easiest)" )
	PORT_DIPSETTING(      0x0038, "3 (Normal)" )
	PORT_DIPSETTING(      0x0030, "4 (Little Hard)" )
	PORT_DIPSETTING(      0x0028, "5 (Hard)" )
	PORT_DIPSETTING(      0x0020, "6 (Harder)" )
	PORT_DIPSETTING(      0x0018, "7 (Hardest)" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, "Start Score Type" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0300, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW1:3" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW1:4" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW1:5" )   // Unused according to the manual
	PORT_DIPNAME( 0x2000, 0x2000, "Item Availability" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW1:7" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW1:8" )   // Unused according to the manual
INPUT_PORTS_END

static INPUT_PORTS_START( kakumei )
	PORT_INCLUDE( common )
	PORT_INCLUDE( ctrl_mj1 )

	PORT_MODIFY("P1_KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P1 Mahjong Flip Flop")  PORT_CODE(KEYCODE_2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW2:2" )   // Unused according to the manual
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )   // Unused according to the manual
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )   // Unused according to the manual
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )   // Unused according to the manual
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )   // Unused according to the manual
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" )   // Unused according to the manual
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin / 99 Credits" )   // Free Play according to the manual
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW1:6" )   // Unused according to the manual
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW1:7" )   // Unused according to the manual
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:8" )   // Unused according to the manual
INPUT_PORTS_END

static INPUT_PORTS_START( kakumei2 )
	PORT_INCLUDE( common )
	PORT_INCLUDE( ctrl_mj1 )

	PORT_MODIFY("P1_KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P1 Mahjong Flip Flop")  PORT_CODE(KEYCODE_2)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW2:1" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW2:2" )   // Unused according to the manual
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW2:4" )   // Unused according to the manual
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")    // Should default to OFF according to manual
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW2:6" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW2:7" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW2:8" )   // Unused according to the manual
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin / 99 Credits" )   // Free Play according to the manual
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW1:6" )   // Unused according to the manual
	PORT_DIPNAME( 0x4000, 0x4000, "Pinfu with Tsumo" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( suchiesp )
	PORT_INCLUDE( kakumei2 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Campaign Mode" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
INPUT_PORTS_END

// check if we are into service or normal mode
// TODO: this should be set via internal work RAM buffer
void jalmah_state::mcu_check_test_mode()
{
	m_respcount = 0;

	switch(m_mcu_prg)
	{
		case MJZOOMIN_MCU:
		case DAIREIKA_MCU:
			m_test_mode = (ioport("SYSTEM")->read() & 0x0008) ? 0 : 1;
			break;
		case URASHIMA_MCU:
			m_test_mode = ((~(ioport("SYSTEM")->read()) & 0x0008) || (~(ioport("DSW")->read()) & 0x8000)) ? (1) : (0);
			break;
		case KAKUMEI_MCU:
		case KAKUMEI2_MCU:
		case SUCHIESP_MCU:
			m_test_mode = (ioport("DSW")->read() & 0x0004) ? (0) : (1);
			break;
	}
}

void jalmah_state::machine_start()
{
	const int okimax = (memregion("oki")->bytes() - 0x40000) / 0x20000;
	m_okibank->configure_entries(0,okimax,memregion("oki")->base() + 0x40000,0x20000);

	m_prg_prot = 0;
	m_oki_rom = 0;
	m_oki_bank = 0;
	m_oki_za = 0;

	save_item(NAME(m_respcount));
	save_item(NAME(m_test_mode));
	save_item(NAME(m_prg_prot));
	save_item(NAME(m_oki_rom));
	save_item(NAME(m_oki_bank));
	save_item(NAME(m_oki_za));
}

void jalmah_state::machine_reset()
{
	if(m_jmcu_rom_share)
		memcpy(m_jmcu_rom_share, m_jmcu_rom_region, 0x10000);

	m_pri = 0;
	refresh_priority_system();

	mcu_check_test_mode();
}

void urashima_state::machine_reset()
{
	if(m_jmcu_rom_share)
		memcpy(m_jmcu_rom_share, m_jmcu_rom_region, 0x10000);

//  m_pri = 0;

	// initialize tilemap vram to sane defaults (test mode cares)
	for(int i=0;i<0x4000/2;i++)
		m_videoram[0][i] = 0xffff;

	for(int i=0;i<0x2000/2;i++)
		m_videoram[1][i] = 0xffff;

	mcu_check_test_mode();
}

void jalmah_state::jalmah(machine_config &config)
{
	M68000(config, m_maincpu, 12000000/2); // assume same as Mega System 1
	m_maincpu->set_addrmap(AS_PROGRAM, &jalmah_state::jalmah_map);
	m_maincpu->set_vblank_int("screen", FUNC(jalmah_state::irq2_line_hold));

	//M50747 MCU

	MEGASYS1_TILEMAP(config, m_tmap[0], m_palette, 0x0000);
	MEGASYS1_TILEMAP(config, m_tmap[1], m_palette, 0x0100);
	MEGASYS1_TILEMAP(config, m_tmap[2], m_palette, 0x0200);
	MEGASYS1_TILEMAP(config, m_tmap[3], m_palette, 0x0300);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12000000/2,406,0,256,263,16,240); // assume same as nmk16 & mega system 1
	screen.set_screen_update(FUNC(jalmah_state::screen_update_jalmah));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x400);

	TIMER(config, "mcusim").configure_periodic(FUNC(jalmah_state::mcu_sim), attotime::from_hz(10000));

	SPEAKER(config, "mono").front_center();
	okim6295_device &oki(OKIM6295(config, "oki", 4000000, okim6295_device::PIN7_LOW));
	oki.set_addrmap(0, &jalmah_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 0.5);
}

static GFXDECODE_START( gfx_urashima )
	GFXDECODE_ENTRY( "scroll0", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x100, 16 )
	GFXDECODE_ENTRY( "scroll1", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x100, 16 )
	GFXDECODE_ENTRY( "scroll2", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x100, 16 )
	GFXDECODE_ENTRY( "scroll3", 0, gfx_8x8x4_packed_msb,               0x000, 16 )
GFXDECODE_END

void jalmah_state::jalmahv1(machine_config &config)
{
	jalmah(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jalmah_state::jalmahv1_map);
}

void urashima_state::urashima(machine_config &config)
{
	jalmah(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &urashima_state::urashima_map);

	// Urashima seems to use an earlier version of the Jaleco tilemaps (without range etc.)
	// and with per-scanline video registers
	config.device_remove("scroll0");
	config.device_remove("scroll1");
	config.device_remove("scroll2");
	config.device_remove("scroll3");

	GFXDECODE(config, "gfxdecode", m_palette, gfx_urashima);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(urashima_state::screen_update_urashima));
}

// fake ROM containing 68k snippets
// the original MCU actually uploads these into shared work ram area
// we actually compile it using EASy68k tool
#define LOAD_FAKE_MCU_ROM \
	ROM_REGION16_BE( 0x10000, "jmcu_rom", 0 ) \
	ROM_LOAD16_WORD( "mcu.bin", 0, 0x10000, BAD_DUMP CRC(35425d2f) SHA1(9a9914d4e50a665d4eb0efb80552f357fc719e7e))


/*

Urashima Mahjong
(c) 1989 UPL

*/

ROM_START ( urashima )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "um-2.15d",  0x00000, 0x20000, CRC(a90a47e3) SHA1(2f912001e9177cce8c3795f3d299115b80fdca4e) )
	ROM_RELOAD(                   0x40000, 0x20000 )
	ROM_LOAD16_BYTE( "um-1.15c",  0x00001, 0x20000, CRC(5f5c8f39) SHA1(cef663965c3112f87788d6a871e609c0b10ef9a2) )
	ROM_RELOAD(                   0x40001, 0x20000 )

	ROM_REGION( 0x2000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747-b84sp.11b", 0x0000, 0x2000, NO_DUMP )

	LOAD_FAKE_MCU_ROM

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "um-3.22c",      0x40000, 0x80000, CRC(9fd8c8fa) SHA1(0346f74c03a4daa7a84b64c9edf0e54297c82fd9) )
	ROM_COPY( "oki" ,          0x40000, 0x00000, 0x40000 )

	// 16x16 tiles
	// layer 0 bankswitches tiles 0x800-0xfff by reading following two roms.
	// we copy them into three regions for simplicity in tilemap code.
	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "um-6.2l",    0x000000, 0x080000, CRC(076be5b5) SHA1(77444025f149a960137d3c79abecf9b30defa341) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "um-7.4l",    0x000000, 0x080000, CRC(d2a68cfb) SHA1(eb6cb1fad306b697b2035a31ad48e8996722a032) )

	ROM_REGION( 0x080000, "scroll0", 0 )
	ROM_COPY( "gfx1" , 0x000000, 0x000000, 0x80000 )

	ROM_REGION( 0x080000, "scroll1", 0 )
	ROM_COPY( "gfx1",  0x000000, 0x000000, 0x40000 )
	ROM_COPY( "gfx2",  0x000000, 0x040000, 0x40000 )

	ROM_REGION( 0x080000, "scroll2", 0 )
	ROM_COPY( "gfx1",  0x000000, 0x000000, 0x40000 )
	ROM_COPY( "gfx2",  0x040000, 0x040000, 0x40000 )

	ROM_REGION( 0x020000, "scroll3", 0 )
	// 8x8 tiles
	ROM_LOAD( "um-5.22j",       0x000000, 0x020000, CRC(991776a2) SHA1(56740553d7d26aaeb9bec8557727030950bb01f7) )

	ROM_REGION( 0x240, "proms", 0 ) /* Misc PROMs (unknown) */
	ROM_LOAD( "um-10.2b",      0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )
	ROM_LOAD( "um-11.2c",      0x0100, 0x0100, CRC(ff5660cf) SHA1(a4635dcf9d6dd637ea4f36f1ad233db0bd039731) )
	ROM_LOAD( "um-12.20c",     0x0200, 0x0020, CRC(bdb66b02) SHA1(8755244de638d7e835e35e08c62b0612958e6ca5) )
	ROM_LOAD( "um-13.10l",     0x0220, 0x0020, CRC(4ce07ec0) SHA1(5f5744ddc7f258307f036fde4c0a8e6271b2d1f9) )
ROM_END

/*

Mahjong Daireikai (JPN Ver.)
(c)1989 Jaleco / NMK

*/

ROM_START( daireika )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj1.bin", 0x00001, 0x20000, CRC(3b4e8357) SHA1(1ad3e40ec6b6ff4f1c9c09d7b530f67b460151d8) )
	ROM_RELOAD(                 0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "mj2.bin", 0x00000, 0x20000, CRC(c54d2f9b) SHA1(d59fc5a9e5bbb96b3b6a43378f4f2215c368b671) )
	ROM_RELOAD(                 0x40000, 0x20000 )

	ROM_REGION( 0x2000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747-a46sp.bin", 0x0000, 0x2000, NO_DUMP )

	LOAD_FAKE_MCU_ROM

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "mj3.bin", 0x40000, 0x80000, CRC(65bb350c) SHA1(e77866f2d612a0973adc616717e7c89a37d6c48e) )
	ROM_COPY( "oki" ,    0x40000, 0x00000, 0x40000 )

	ROM_REGION( 0x80000, "scroll0", 0 ) /* BG3 */
	ROM_LOAD( "mj10.bin", 0x00000, 0x80000, CRC(1f5509a5) SHA1(4dcdee0e159956cf73f5f85ce278479be2a9ca9f) )

	ROM_REGION( 0x40000, "scroll1", 0 ) /* BG2 */
//  ROM_COPY( "scroll3",     0x20000, 0x20000, 0x20000 )/*mj10.bin*/
	ROM_LOAD( "mj11.bin", 0x00000, 0x20000, CRC(14867c51) SHA1(b282b5048a55c9ad72ceb0d23f010a0fee78704f) )
	ROM_LOAD( "mj12.bin", 0x20000, 0x20000, CRC(236f809f) SHA1(9e15dd8a810a9d4f7f75f084d6bd277ea7d0e40a) )

	ROM_REGION( 0x10000, "scroll2", 0 ) /* BG1 */
	ROM_LOAD( "mj13.bin", 0x00000, 0x10000, CRC(c54bca14) SHA1(ee9c99858817aedd70bd6266b7a71c3c5ad00607) )

	ROM_REGION( 0x20000, "scroll3", 0 ) /* BG0 */
	ROM_LOAD( "mj14.bin", 0x00000, 0x10000, CRC(c84c5577) SHA1(6437368d3be39739d62158590ecd373aa070a9b2) )
	ROM_COPY( "scroll2",     0x00000, 0x10000, 0x10000 )

	ROM_REGION( 0x100, "prirom", 0 ) /* Priority PROM */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )

	ROM_REGION( 0x120, "proms", 0 ) /* Misc PROMs (unknown) */
	ROM_LOAD( "mj16.bpr", 0x000, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x100, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Channel Zoom In (JPN Ver.)
(c)1990 Jaleco

*/

ROM_START( mjzoomin )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "zoomin-1.bin", 0x00001, 0x20000, CRC(b8b04d30) SHA1(abb163a9965421b4d92114bba974ccb13bb57f5a) )
	ROM_RELOAD(                      0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "zoomin-2.bin", 0x00000, 0x20000, CRC(c7eb982c) SHA1(9006ded2aa1fef38bde114110d76b20747c32658) )
	ROM_RELOAD(                      0x40000, 0x20000 )

	ROM_REGION( 0x2000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x2000, NO_DUMP )

	LOAD_FAKE_MCU_ROM

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "zoomin-3.bin", 0x40000, 0x80000, CRC(07d7b8cd) SHA1(e05ce80ffb945b04f93f8c49d0c840b0bff6310b) )
	ROM_COPY( "oki" ,         0x40000, 0x00000, 0x40000 )

	ROM_REGION( 0x80000, "scroll0", 0 )
	ROM_LOAD( "zoomin10.bin", 0x00000, 0x80000, CRC(40aec575) SHA1(ef7a3c7a94523c5967ab774936b873c9629e0e44) )

	ROM_REGION( 0x40000, "scroll1", 0 )
	ROM_LOAD( "zoomin12.bin", 0x00000, 0x40000, CRC(b0b94554) SHA1(10490b7475810910140ce075e62f604b914e5511) )

	ROM_REGION( 0x20000, "scroll2", 0 )
	ROM_LOAD( "zoomin13.bin", 0x00000, 0x20000, CRC(888d79fe) SHA1(eb9671d4c7608edd1231dc0cae47aab2430cbd66) )

	ROM_REGION( 0x20000, "scroll3", 0 )
	ROM_LOAD( "zoomin14.bin", 0x00000, 0x20000, CRC(4e32aa45) SHA1(450a3449ca8b4f0dfe8b62cceaee9366eaf3dc3d) )

	ROM_REGION( 0x100, "prirom", 0 ) /* Priority PROM */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )

	ROM_REGION( 0x120, "proms", 0 ) /* Misc PROMs (unknown) */
	ROM_LOAD( "mj16.bpr", 0x000, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x100, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Kakumei (JPN Ver.)
(c)1990 Jaleco


*/

ROM_START( kakumei )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj-re-1.bin", 0x00001, 0x20000, CRC(b90215be) SHA1(10384237f734836acefb4b5f53a6ddd9054d63ff) )
	ROM_RELOAD(                     0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "mj-re-2.bin", 0x00000, 0x20000, CRC(37eff266) SHA1(1d9e88c0270daadfafff1f73eb617e77b1d199d6) )
	ROM_RELOAD(                     0x40000, 0x20000 )

	ROM_REGION( 0x2000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747-b84sp.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "rom3.bin", 0x00000, 0x40000, CRC(c9b7a526) SHA1(edec57e66d4ff601c8fdef7b1405af84a3f3d883) )
	ROM_RELOAD(           0x40000, 0x40000 )

	ROM_REGION( 0x80000, "scroll0", 0 )
	ROM_LOAD( "rom10.bin", 0x00000, 0x80000, CRC(88366377) SHA1(163a08415a631c8a09a0a55bc2819988d850f2ad) )

	ROM_REGION( 0x40000, "scroll1", 0 )
	ROM_LOAD( "rom12.bin", 0x00000, 0x40000, CRC(31620a61) SHA1(11593ca7760e1a628e63aa48d9ad3800cf7af275) )

	ROM_REGION( 0x20000, "scroll2", 0 )
	ROM_LOAD( "rom13.bin", 0x00000, 0x20000, CRC(9bef4fc2) SHA1(6598ab9dba513efcda01e47cc7752b47a97f2c6a) )

	ROM_REGION( 0x20000, "scroll3", 0 )
	ROM_LOAD( "rom14.bin", 0x00000, 0x20000, CRC(63e88dd6) SHA1(58734c8caf1b1ddc4cf0437ffd8109292b76c4e1) )

	ROM_REGION( 0x100, "prirom", 0 ) /* Priority PROM */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )

	ROM_REGION( 0x120, "proms", 0 ) /* Misc PROMs (unknown) */
	ROM_LOAD( "mj16.bpr", 0x000, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x100, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Kakumei2 Princess League (JPN Ver.)
(c)1992 Jaleco

*/

ROM_START( kakumei2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj-8956.1", 0x00001, 0x40000, CRC(db4ce32f) SHA1(1ae13627b9922143f462b1c3bbed87374f6e1667) )
	ROM_LOAD16_BYTE( "mj-8956.2", 0x00000, 0x40000, CRC(0f942507) SHA1(7ec2fbeb9a34dfc80c4df3de8397388db13f5c7c) )

	ROM_REGION( 0x2000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "92000-01.3", 0x040000, 0x80000, CRC(4b0ed440) SHA1(11961d217a41f92b60d5083a5e346c245f7db620) )
	ROM_COPY( "oki" ,       0x040000, 0x00000, 0x40000 )

	ROM_REGION( 0x80000, "scroll0", 0 )
	ROM_LOAD( "92000-02.10", 0x00000, 0x80000, CRC(338fa9b2) SHA1(05ba4b3c44249cf92be238bf53d6345dc49b0881) )

	ROM_REGION( 0x40000, "scroll1", 0 )
	ROM_LOAD( "mj-8956.12", 0x00000, 0x40000, CRC(4a088f69) SHA1(468c446d1f345dfd628cdd66ca71cf82e02abe6f) )

	ROM_REGION( 0x20000, "scroll2", 0 )
	ROM_LOAD( "mj-8956.13", 0x00000, 0x20000, CRC(afe93cf4) SHA1(1973dc5821c6df68e20f8a84b5c9ae281dd3f85f) )

	ROM_REGION( 0x20000, "scroll3", 0 )
	ROM_LOAD( "mj-8956.14", 0x00000, 0x20000, CRC(2b2fe999) SHA1(d9d601e2c008791f5bff6e7b1340f754dd094201) )

	ROM_REGION( 0x100, "prirom", 0 ) /* Priority PROM */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )

	ROM_REGION( 0x120, "proms", 0 ) /* Misc PROMs (unknown) */
	ROM_LOAD( "mj16.bpr", 0x000, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x100, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Idol Janshi Su-Chi-Pi Special
(c)Jaleco 1994

CPU  : M68000P10
Sound: OKI M6295
OSC  : 12.000MHz 4.000MHz

MJ-8956
YSP-40101 171

1.bin - Main program ver.1.2 (27c2001)
2.bin - Main program ver.1.2 (27c2001)

3.bin - Sound data (27c4000)
4.bin - Sound data (27c4000)

7.bin  (27c4000) \
8.bin  (27c4000) |
9.bin  (27c4000) |
10.bin (27c4000) |
                 |- Graphics
12.bin (27c2001) |
                 |
13.bin (27c1001) |
                 |
14.bin (27c1001) /

pr92000a.prm (82s129) \
pr92000b.prm (82s129) |- Not dumped
pr93035.prm  (82s123) /

Custom chips:
GS-9000406 9345K5005 (80pin QFP) x4
GS-9000404 9248EP004 (44pin QFP)

Other chips:
MO-92000 (64pin DIP)
NEC D65012GF303 9050KX016 (80pin QFP) x4

*/

ROM_START( suchiesp )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x40000, CRC(e37cc745) SHA1(73b3314d27a0332068e0d2bbc08d7401e371da1b) )
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x40000, CRC(42ecf88a) SHA1(7bb85470bc9f94c867646afeb91c4730599ea299) )

	ROM_REGION( 0x2000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x100000, "oki_data", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "3.bin", 0x00000, 0x80000, CRC(691b5387) SHA1(b8bc9f904eab7653566042b18d89276d537ba586) )
	ROM_LOAD( "4.bin", 0x80000, 0x80000, CRC(3fe932a1) SHA1(9e768b901738ee9eba207a67c4fd19efb0035a68) )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_COPY( "oki_data" , 0x000000, 0x000000+0x00000, 0x40000 )

	/* PAL address shuffling for the BGM data (TODO: check this with a side-by-side test)*/
	ROM_COPY( "oki_data" , 0x20000, 0x000000+0x40000, 0x20000 ) // 0
	ROM_COPY( "oki_data" , 0x40000, 0x020000+0x40000, 0x20000 ) // 1
	ROM_COPY( "oki_data" , 0x60000, 0x040000+0x40000, 0x20000 ) // 2
	ROM_COPY( "oki_data" , 0x000000, 0x060000+0x40000, 0x20000 ) // 3

	ROM_COPY( "oki_data" , 0x80000, 0x080000+0x40000, 0x40000 )
	ROM_COPY( "oki_data" , 0xc0000, 0x0c0000+0x40000, 0x40000 )

	ROM_REGION( 0x200000, "scroll0", 0 )
	ROM_LOAD( "7.bin",  0x000000, 0x80000, CRC(18caf6f3) SHA1(3df6b257867487adcba1a05c8745413d9a15c3d7) )
	ROM_LOAD( "8.bin",  0x080000, 0x80000, CRC(0403399a) SHA1(8d39a68b3a1a431afe93ff485e837389a4502d0c) )
	ROM_LOAD( "9.bin",  0x100000, 0x80000, CRC(8a348246) SHA1(13516c48bdbe8d78e7517473ef2835a4dea2ce93) )
	ROM_LOAD( "10.bin", 0x180000, 0x80000, CRC(2b0d1afd) SHA1(40009b450901567052aa63c4629a2f7a10343e63) )

	ROM_REGION( 0x40000, "scroll1", 0 )
	ROM_LOAD( "12.bin", 0x00000, 0x40000, CRC(146596eb) SHA1(f85e92e6dc9ebef5e67d28f1d450225cd2a2abaa) )

	ROM_REGION( 0x20000, "scroll2", 0 )
	ROM_LOAD( "13.bin", 0x00000, 0x20000, CRC(99466044) SHA1(ca31b58a5d4656f95d80ddb9bc1f9a53f5f2446c) )

	ROM_REGION( 0x20000, "scroll3", 0 )
	ROM_LOAD( "14.bin", 0x00000, 0x20000, CRC(e465a540) SHA1(10e19599ab90b0c0b6ef6ee41f16620bd1ba6800) )

	ROM_REGION( 0x100, "prirom", 0 ) /* Priority PROM */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )

	ROM_REGION( 0x120, "proms", 0 ) /* Misc PROMs (unknown) */
	ROM_LOAD( "mj16.bpr", 0x000, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "pr93035.17", 0x100, 0x020, CRC(ab28ae42) SHA1(e05652c4bd5db4c7d7a1bfdeb63841e8b019f24c) )
ROM_END


/******************************************************************************************

MCU code snippets

******************************************************************************************/

uint16_t jalmah_state::urashima_mcu_r()
{
	static const int resp[] = { 0x99, 0xd8, 0x00,
							0x2a, 0x6a, 0x00,
							0x9c, 0xd8, 0x00,
							0x2f, 0x6f, 0x00,
							0x22, 0x62, 0x00,
							0x25, 0x65, 0x00,
							0x23, 0x63, 0x00,
							0x3e, 0x7e, 0x00,
							0x35, 0x75, 0x00,
							0x21, 0x61, 0x00 };
	int res;

	res = resp[m_respcount++];
	if (m_respcount >= std::size(resp)) m_respcount = 0;

//  logerror("%s: mcu_r %02x\n",machine().dscribe_context(),res);

	return res;
}

#define MCU_JMP(_workram_,_data_) \
	m_sharedram[_workram_]   = 0x4ef9; \
	m_sharedram[_workram_+1] = 0x0010; \
	m_sharedram[_workram_+2] = _data_;


/*
data value is REQ under mjzoomin video test menu. Is it related to the MCU?
*/
void jalmah_state::urashima_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(ACCESSING_BITS_0_7 && data)
	{
		/*******************************************************
		1st M68k code uploaded by the MCU (sound prg)
		*******************************************************/
		MCU_JMP(0x320/2,SNIPPET_SOUND);
		/*******************************************************
		1st alt M68k code uploaded by the MCU (Input test mode)
		*******************************************************/
		/*similar to mjzoomin but with offset summed with 0x300?*/
		/*tx scrollx = $200*/
		m_sharedram[0x03c6/2] = 0x6008; //bra $+10
		MCU_JMP(0x3d0/2,SNIPPET_PALETTE1);

		/*******************************************************
		2nd M68k code uploaded by the MCU (tile upload)
		*******************************************************/
		MCU_JMP(0x3ca/2,SNIPPET_TILE);

		/*******************************************************
		3rd M68k code uploaded by the MCU (palette upload)
		*******************************************************/
		MCU_JMP(0x3c0/2,SNIPPET_PALETTE2);
	}
}

uint16_t jalmah_state::daireika_mcu_r()
{
	static const int resp[] = { 0x99, 0xd8, 0x00,
							0x2a, 0x6a, 0x00,
							0x9c, 0xd8, 0x00,
							0x2f, 0x6f, 0x00,
							0x22, 0x62, 0x00,
							0x25, 0x65, 0x00,
							0x23, 0x63, 0x00,
							0x3e, 0x7e, 0x00,
							0x35, 0x75, 0x00,
							0x21, 0x61, 0x00 };
	int res;

	res = resp[m_respcount++];
	if (m_respcount >= std::size(resp)) m_respcount = 0;

//  logerror("%s: mcu_r %02x\n",machine().describe_context(),res);

	return res;
}

void jalmah_state::daireika_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(ACCESSING_BITS_0_7 && data)
	{
		/*******************************************************
		1st M68k code uploaded by the MCU.
		random number generator (guess)
		*******************************************************/
		MCU_JMP(0x0140/2,SNIPPET_RNG);

		/*******************************************************
		2nd M68k code uploaded by the MCU. (sound playback)
		*******************************************************/
		MCU_JMP(0x0020/2,SNIPPET_SOUND);

		/*******************************************************
		3rd M68k code uploaded by the MCU.
		see mjzoomin_mcu_w
		*******************************************************/
		m_sharedram[0x00c6/2] = 0x6008; //bra +$8, we need this due of clash with SNIPPET_TILE
		MCU_JMP(0x00d0/2,SNIPPET_PALETTE1);

		/*******************************************************
		4th M68k code uploaded by the MCU
		They seem video code cleaning functions
		*******************************************************/
		MCU_JMP(0x100/2,SNIPPET_CLR_LAYER0);
		MCU_JMP(0x108/2,SNIPPET_CLR_LAYER1);
		MCU_JMP(0x110/2,SNIPPET_CLR_LAYER2);

		/* layer 3 clear function is already in 68k ROM program???*/
		m_sharedram[0x0126/2] = 0x4ef9;
		m_sharedram[0x0128/2] = 0x0000;
		m_sharedram[0x012a/2] = 0x2684;

		/*******************************************************
		5th M68k code uploaded by the MCU (palette upload)
		*******************************************************/
		MCU_JMP(0x00c0/2,SNIPPET_PALETTE2);

		/*******************************************************
		6th M68k code uploaded by the MCU (tile upload)
		*******************************************************/
		MCU_JMP(0x00ca/2,SNIPPET_TILE);
	}
}

uint16_t jalmah_state::mjzoomin_mcu_r()
{
	static const int resp[] = { 0x9c, 0xd8, 0x00,
							0x2a, 0x6a, 0x00,
							0x99, 0xd8, 0x00,
							0x2f, 0x6f, 0x00,
							0x22, 0x62, 0x00,
							0x25, 0x65, 0x00,
							0x35, 0x75, 0x00,
							0x36, 0x36, 0x00,
							0x21, 0x61, 0x00 };
	int res;

	res = resp[m_respcount++];
	if (m_respcount >= std::size(resp)) m_respcount = 0;

//  logerror("%04x: mcu_r %02x\n",machine().describe_context(),res);

	return res;
}

/*
data value is REQ under mjzoomin video test menu.It is related to the MCU?
*/
void jalmah_state::mjzoomin_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(ACCESSING_BITS_0_7 && data)
	{
		/******************************************************
		1st M68k code uploaded by the MCU(Service Mode PC=2a56).
		Program passes some parameters before entering into
		the sub-routine (jsr)
		D1 = 0xf
		A0 = 1026e
		A1 = 88600
		(A0) is the vector number for take the real palette
		address.
		******************************************************/
		MCU_JMP(0x00c6/2,SNIPPET_PALETTE1);

		/*******************************************************
		2nd M68k code uploaded by the MCU (Sound read/write)
		(Note:copied from suchiesp,check here the sound banking)
		*******************************************************/
		MCU_JMP(0x0020/2,SNIPPET_SOUND);

		/*******************************************************
		3rd M68k code uploaded by the MCU (in-game palette upload)
		*******************************************************/
		MCU_JMP(0x00c0/2,SNIPPET_PALETTE2);
	}
}

uint16_t jalmah_state::kakumei_mcu_r()
{
	static const int resp[] = { 0x8a, 0xd8, 0x00,
							0x3c, 0x7c, 0x00,
							0x99, 0xd8, 0x00,
							0x25, 0x65, 0x00,
							0x36, 0x76, 0x00,
							0x35, 0x75, 0x00,
							0x2f, 0x6f, 0x00,
							0x31, 0x71, 0x00,
							0x3e, 0x7e, 0x00 };
	int res;

	res = resp[m_respcount++];
	if (m_respcount >= std::size(resp)) m_respcount = 0;

//  popmessage("%s: mcu_r %02x",machine().describe_context(),res);

	return res;
}

uint16_t jalmah_state::suchiesp_mcu_r()
{
	static const int resp[] = { 0x8a, 0xd8, 0x00,
							0x3c, 0x7c, 0x00,
							0x99, 0xd8, 0x00,
							0x25, 0x65, 0x00,
							0x36, 0x76, 0x00,
							0x35, 0x75, 0x00,
							0x2f, 0x6f, 0x00,
							0x31, 0x71, 0x00,
							0x3e, 0x7e, 0x00 };
	int res;

	res = resp[m_respcount++];
	if (m_respcount >= std::size(resp)) m_respcount = 0;

//  popmessage("%s: mcu_r %02x",machine().describe_context(),res);

	return res;
}

void jalmah_state::init_urashima()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16smo_delegate(*this, FUNC(jalmah_state::urashima_mcu_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x80012, 0x80013, write16s_delegate(*this, FUNC(jalmah_state::urashima_mcu_w)));

	m_mcu_prg = URASHIMA_MCU;
}

void jalmah_state::init_daireika()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16smo_delegate(*this, FUNC(jalmah_state::daireika_mcu_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x80012, 0x80013, write16s_delegate(*this, FUNC(jalmah_state::daireika_mcu_w)));

	m_mcu_prg = DAIREIKA_MCU;
}

void jalmah_state::init_mjzoomin()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16smo_delegate(*this, FUNC(jalmah_state::mjzoomin_mcu_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x80012, 0x80013, write16s_delegate(*this, FUNC(jalmah_state::mjzoomin_mcu_w)));

	m_mcu_prg = MJZOOMIN_MCU;
}

void jalmah_state::init_kakumei()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16smo_delegate(*this, FUNC(jalmah_state::kakumei_mcu_r)));

	m_mcu_prg = KAKUMEI_MCU;
}

void jalmah_state::init_kakumei2()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16smo_delegate(*this, FUNC(jalmah_state::kakumei_mcu_r)));

	m_mcu_prg = KAKUMEI2_MCU;
}

void jalmah_state::init_suchiesp()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16smo_delegate(*this, FUNC(jalmah_state::suchiesp_mcu_r)));

	m_mcu_prg = SUCHIESP_MCU;
}

} // Anonymous namespace

/*First version of the MCU*/
GAME( 1989, urashima, 0, urashima,  urashima, urashima_state, init_urashima, ROT0, "UPL",          "Otogizoushi Urashima Mahjong (Japan)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1989, daireika, 0, jalmahv1,  daireika, jalmah_state,   init_daireika, ROT0, "Jaleco / NMK", "Mahjong Daireikai (Japan)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
GAME( 1990, mjzoomin, 0, jalmahv1,  mjzoomin, jalmah_state,   init_mjzoomin, ROT0, "Jaleco",       "Mahjong Channel Zoom In (Japan)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION )
/*Second version of the MCU*/
GAME( 1990, kakumei,  0, jalmah,    kakumei,  jalmah_state,   init_kakumei,  ROT0, "Jaleco",       "Mahjong Kakumei (Japan)",                      MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, kakumei2, 0, jalmah,    kakumei2, jalmah_state,   init_kakumei2, ROT0, "Jaleco",       "Mahjong Kakumei 2 - Princess League (Japan)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION )
GAME( 1993, suchiesp, 0, jalmah,    suchiesp, jalmah_state,   init_suchiesp, ROT0, "Jaleco",       "Idol Janshi Suchie-Pai Special (Japan)",       MACHINE_IMPERFECT_GRAPHICS )
