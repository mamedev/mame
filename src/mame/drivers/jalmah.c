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
-Back layer pens looks ugly in some circumstances (i.e. suchipi when you win, mjzoomin when coined up),
 static or controlled by something else?
-daireika: the ranking screen on the original pcb shows some hearts instead of the "0".
 Some investigation indicates that the game reads area "fe100" onwards for these to be filled.
 These are likely to be provided by one of the mcu snippets...
-kakumei/kakumei2:has weird text layer strings in test mode (like "#p control panel"),
 unsure if this one is somehow related to the above daireika bug, it's a BTANB or something else.
-Check if urashima has a "mode 3" for the layer 0 tilemap;
-There could be timing issues caused by MCU simulation at $80004;
-Fix the sound banking, protection-related for the first version of the MCU
 (should be somewhere on the work ram/shared ram)
-suchipi: I need a side-by-side to understand if the PAL shuffling is correct with the OKI bgm rom.
-urashima: might use three/four layers instead of two.It can be checked when you win
 a match in particular circumstances because there's a write in the 94000-9bfff region;
-Massive clean-ups needed for the MCU snippet programs and the input-ports, also check if
 the programs are actually into the m68k program itself (like hachamf/tdragon/ddealer);

Notes (1st MCU ver.):
-$f000e is bogus,maybe the program snippets can modify this value,or the MCU itself can
 do that,returning the contents of D0 register seems enough for now...
 Update: Improved it for the new mcu simulation,now it pulls all the values from 0x00 to
 0x0f, it seems to be a MCU call snippet for the $f0000 work ram;
-$f030e is a mirror for $f000e in urashima.
-I need more space for MCU code...that's why I've used an extra jmp when entering
 into mcu code,so we can debug the first version freely without being teased about
 memory space.
 BTW,the real HW is using a sort of bankswitch or I'm missing something?
-$f0020 is for the sound program,same for all games, for example mjzoomin hasn't any clear
 write to $80040 area and the program jumps to $f0020 when there should be a sample.

============================================================================================
Debug cheats:

-(suchipi)
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

kakumei/kakumei2/suchipi 68k irq table vectors
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
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"


class jalmah_state : public driver_device
{
public:
	jalmah_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sc0_vram(*this, "sc0_vram"),
		m_sc1_vram(*this, "sc1_vram"),
		m_sc2_vram(*this, "sc2_vram"),
		m_sc3_vram(*this, "sc3_vram"),
		m_jm_shared_ram(*this, "jshared_ram"),
		m_jm_mcu_code(*this, "jmcu_code"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	tilemap_t *m_sc0_tilemap_0;
	tilemap_t *m_sc0_tilemap_1;
	tilemap_t *m_sc0_tilemap_2;
	tilemap_t *m_sc0_tilemap_3;
	tilemap_t *m_sc1_tilemap_0;
	tilemap_t *m_sc1_tilemap_1;
	tilemap_t *m_sc1_tilemap_2;
	tilemap_t *m_sc1_tilemap_3;
	tilemap_t *m_sc2_tilemap_0;
	tilemap_t *m_sc2_tilemap_1;
	tilemap_t *m_sc2_tilemap_2;
	tilemap_t *m_sc2_tilemap_3;
	tilemap_t *m_sc3_tilemap_0;
	tilemap_t *m_sc3_tilemap_2;
	tilemap_t *m_sc3_tilemap_3;
	required_shared_ptr<UINT16> m_sc0_vram;
	optional_shared_ptr<UINT16> m_sc1_vram;
	optional_shared_ptr<UINT16> m_sc2_vram;
	required_shared_ptr<UINT16> m_sc3_vram;
	UINT16 *m_jm_scrollram;
	UINT16 *m_jm_vregs;
	UINT16 m_sc0bank;
	UINT16 m_pri;
	UINT8 m_sc0_prin;
	UINT8 m_sc1_prin;
	UINT8 m_sc2_prin;
	UINT8 m_sc3_prin;
	required_shared_ptr<UINT16> m_jm_shared_ram;
	required_shared_ptr<UINT16> m_jm_mcu_code;
	UINT8 m_mcu_prg;
	int m_respcount;
	UINT8 m_test_mode;
	UINT16 m_dma_old;
	UINT16 m_prg_prot;
	UINT8 m_oki_rom;
	UINT8 m_oki_bank;
	UINT8 m_oki_za;
	DECLARE_WRITE16_MEMBER(sc0_vram_w);
	DECLARE_WRITE16_MEMBER(sc3_vram_w);
	DECLARE_WRITE16_MEMBER(sc1_vram_w);
	DECLARE_WRITE16_MEMBER(sc2_vram_w);
	DECLARE_WRITE16_MEMBER(jalmah_tilebank_w);
	DECLARE_WRITE16_MEMBER(jalmah_scroll_w);
	DECLARE_WRITE16_MEMBER(urashima_bank_w);
	DECLARE_WRITE16_MEMBER(urashima_sc0_vram_w);
	DECLARE_WRITE16_MEMBER(urashima_sc3_vram_w);
	DECLARE_WRITE16_MEMBER(urashima_vregs_w);
	DECLARE_WRITE16_MEMBER(urashima_dma_w);
	DECLARE_WRITE16_MEMBER(jalmah_okirom_w);
	DECLARE_WRITE16_MEMBER(jalmah_okibank_w);
	DECLARE_WRITE16_MEMBER(jalmah_flip_screen_w);
	DECLARE_READ16_MEMBER(urashima_mcu_r);
	DECLARE_WRITE16_MEMBER(urashima_mcu_w);
	DECLARE_READ16_MEMBER(daireika_mcu_r);
	DECLARE_WRITE16_MEMBER(daireika_mcu_w);
	DECLARE_READ16_MEMBER(mjzoomin_mcu_r);
	DECLARE_WRITE16_MEMBER(mjzoomin_mcu_w);
	DECLARE_READ16_MEMBER(kakumei_mcu_r);
	DECLARE_READ16_MEMBER(suchipi_mcu_r);
	DECLARE_DRIVER_INIT(suchipi);
	DECLARE_DRIVER_INIT(kakumei);
	DECLARE_DRIVER_INIT(urashima);
	DECLARE_DRIVER_INIT(kakumei2);
	DECLARE_DRIVER_INIT(daireika);
	DECLARE_DRIVER_INIT(mjzoomin);
	TILEMAP_MAPPER_MEMBER(range0_16x16);
	TILEMAP_MAPPER_MEMBER(range1_16x16);
	TILEMAP_MAPPER_MEMBER(range2_16x16);
	TILEMAP_MAPPER_MEMBER(range3_16x16);
	TILEMAP_MAPPER_MEMBER(range2_8x8);
	TILEMAP_MAPPER_MEMBER(range3_8x8);
	TILE_GET_INFO_MEMBER(get_sc0_tile_info);
	TILE_GET_INFO_MEMBER(get_sc1_tile_info);
	TILE_GET_INFO_MEMBER(get_sc2_tile_info);
	TILE_GET_INFO_MEMBER(get_sc3_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_VIDEO_START(urashima);
	UINT32 screen_update_jalmah(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_urashima(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(jalmah_mcu_sim);
	void jalmah_priority_system();
	void draw_sc0_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sc1_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sc2_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sc3_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void daireika_palette_dma(UINT16 val);
	void daireika_mcu_run();
	void mjzoomin_mcu_run();
	void urashima_mcu_run();
	void second_mcu_run();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/******************************************************************************************

Video Hardware start

******************************************************************************************/

/*4096x512 tilemap*/
TILEMAP_MAPPER_MEMBER(jalmah_state::range0_16x16)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0xff) << 4) + ((row & 0x70) << 8);
}

/*2048x1024 tilemap*/
TILEMAP_MAPPER_MEMBER(jalmah_state::range1_16x16)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x7f) << 4) + ((row & 0xf0) << 7);
}

/*1024x2048 tilemap*/
TILEMAP_MAPPER_MEMBER(jalmah_state::range2_16x16)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x1f0) << 6);
}

/*512x4096 tilemap*/
TILEMAP_MAPPER_MEMBER(jalmah_state::range3_16x16)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x1f) << 4) + ((row & 0x3f0) << 5);
}


/*1024x512 tilemap*/
TILEMAP_MAPPER_MEMBER(jalmah_state::range2_8x8)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x1f) + ((col & 0x7f) * 0x20) + ((row & 0x20) * 0x80);
}

/*512x1024 tilemap*/
TILEMAP_MAPPER_MEMBER(jalmah_state::range3_8x8)
{
	return (row & 0x1f) + ((col & 0x3f) * 0x20) + ((row & 0x60) * 0x40);
}

TILE_GET_INFO_MEMBER(jalmah_state::get_sc0_tile_info)
{
	int code = m_sc0_vram[tile_index];
	SET_TILE_INFO_MEMBER(3,
			(code & 0xfff) + ((m_sc0bank & 3) << 12),
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(jalmah_state::get_sc1_tile_info)
{
	int code = m_sc1_vram[tile_index];
	SET_TILE_INFO_MEMBER(2,
			code & 0xfff,
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(jalmah_state::get_sc2_tile_info)
{
	int code = m_sc2_vram[tile_index];
	SET_TILE_INFO_MEMBER(1,
			code & 0xfff,
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(jalmah_state::get_sc3_tile_info)
{
	int code = m_sc3_vram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			code & 0xfff,
			code >> 12,
			0);
}

void jalmah_state::video_start()
{
	m_sc0_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc0_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range0_16x16),this),16,16,256,32);
	m_sc0_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc0_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range1_16x16),this),16,16,128,64);
	m_sc0_tilemap_2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc0_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range2_16x16),this),16,16,64,128);
	m_sc0_tilemap_3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc0_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range3_16x16),this),16,16,32,256);

	m_sc1_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc1_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range0_16x16),this),16,16,256,32);
	m_sc1_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc1_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range1_16x16),this),16,16,128,64);
	m_sc1_tilemap_2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc1_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range2_16x16),this),16,16,64,128);
	m_sc1_tilemap_3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc1_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range3_16x16),this),16,16,32,256);

	m_sc2_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc2_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range0_16x16),this),16,16,256,32);
	m_sc2_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc2_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range1_16x16),this),16,16,128,64);
	m_sc2_tilemap_2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc2_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range2_16x16),this),16,16,64,128);
	m_sc2_tilemap_3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc2_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range3_16x16),this),16,16,32,256);

	m_sc3_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc3_tile_info),this),TILEMAP_SCAN_COLS,8,8,256,32);
	//m_sc3_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc3_tile_info),this),TILEMAP_SCAN_COLS,8,8,256,32);
	m_sc3_tilemap_2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc3_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range2_8x8),this),8,8,128,64);
	m_sc3_tilemap_3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc3_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range3_8x8),this),8,8,64,128);

	m_jm_scrollram = auto_alloc_array(machine(), UINT16, 0x80/2);
	m_jm_vregs = auto_alloc_array(machine(), UINT16, 0x40/2);

	m_sc0_tilemap_0->set_transparent_pen(15);
	m_sc0_tilemap_1->set_transparent_pen(15);
	m_sc0_tilemap_2->set_transparent_pen(15);
	m_sc0_tilemap_3->set_transparent_pen(15);

	m_sc1_tilemap_0->set_transparent_pen(15);
	m_sc1_tilemap_1->set_transparent_pen(15);
	m_sc1_tilemap_2->set_transparent_pen(15);
	m_sc1_tilemap_3->set_transparent_pen(15);

	m_sc2_tilemap_0->set_transparent_pen(15);
	m_sc2_tilemap_1->set_transparent_pen(15);
	m_sc2_tilemap_2->set_transparent_pen(15);
	m_sc2_tilemap_3->set_transparent_pen(15);

	m_sc3_tilemap_0->set_transparent_pen(15);
	//m_sc3_tilemap_1->set_transparent_pen(15);
	m_sc3_tilemap_2->set_transparent_pen(15);
	m_sc3_tilemap_3->set_transparent_pen(15);
}

VIDEO_START_MEMBER(jalmah_state,urashima)
{
	m_sc0_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc0_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range0_16x16),this),16,16,256,32);
	m_sc3_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jalmah_state::get_sc3_tile_info),this),tilemap_mapper_delegate(FUNC(jalmah_state::range2_8x8),this),8,8,128,64);

	m_jm_scrollram = auto_alloc_array(machine(), UINT16, 0x80/2);
	m_jm_vregs = auto_alloc_array(machine(), UINT16, 0x40/2);

	m_sc0_tilemap_0->set_transparent_pen(15);
	m_sc3_tilemap_0->set_transparent_pen(15);
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
void jalmah_state::jalmah_priority_system()
{
	UINT8 *pri_rom = memregion("user1")->base();
	UINT8 i;
	UINT8 prinum[0x10];

	m_sc0_prin = 0;
	m_sc1_prin = 0;
	m_sc2_prin = 0;
	m_sc3_prin = 0;

	for(i=0;i<0x10;i++)
	{
		prinum[i] = pri_rom[i+m_pri*0x10];

		if(prinum[i] == 0) { m_sc0_prin++; }
		if(prinum[i] == 1) { m_sc1_prin++; }
		if(prinum[i] == 2) { m_sc2_prin++; }
		if(prinum[i] == 3) { m_sc3_prin++; }
	}

	//popmessage("%02x %02x %02x %02x",m_sc0_prin,m_sc1_prin,m_sc2_prin,m_sc3_prin);
}

void jalmah_state::draw_sc0_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch(m_jm_vregs[0] & 3)
	{
		case 0: m_sc0_tilemap_0->draw(screen, bitmap, cliprect, 0,0); break;
		case 1: m_sc0_tilemap_1->draw(screen, bitmap, cliprect, 0,0); break;
		case 2: m_sc0_tilemap_2->draw(screen, bitmap, cliprect, 0,0); break;
		case 3: m_sc0_tilemap_3->draw(screen, bitmap, cliprect, 0,0); break;
	}
}

void jalmah_state::draw_sc1_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch(m_jm_vregs[1] & 3)
	{
		case 0: m_sc1_tilemap_0->draw(screen, bitmap, cliprect, 0,0); break;
		case 1: m_sc1_tilemap_1->draw(screen, bitmap, cliprect, 0,0); break;
		case 2: m_sc1_tilemap_2->draw(screen, bitmap, cliprect, 0,0); break;
		case 3: m_sc1_tilemap_3->draw(screen, bitmap, cliprect, 0,0); break;
	}
}

void jalmah_state::draw_sc2_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch(m_jm_vregs[2] & 3)
	{
		case 0: m_sc2_tilemap_0->draw(screen, bitmap, cliprect, 0,0); break;
		case 1: m_sc2_tilemap_1->draw(screen, bitmap, cliprect, 0,0); break;
		case 2: m_sc2_tilemap_2->draw(screen, bitmap, cliprect, 0,0); break;
		case 3: m_sc2_tilemap_3->draw(screen, bitmap, cliprect, 0,0); break;
	}
}

void jalmah_state::draw_sc3_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch(m_jm_vregs[3] & 3)
	{
		case 0:
		case 1: m_sc3_tilemap_0->draw(screen, bitmap, cliprect, 0,0); break;
		case 2: m_sc3_tilemap_2->draw(screen, bitmap, cliprect, 0,0); break;
		case 3: m_sc3_tilemap_3->draw(screen, bitmap, cliprect, 0,0); break;
	}
}

UINT32 jalmah_state::screen_update_jalmah(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *jm_scrollram = m_jm_scrollram;
	UINT8 cur_prin;
	jalmah_priority_system();

	m_sc0_tilemap_0->set_scrollx(0, jm_scrollram[0] & 0xfff);
	m_sc0_tilemap_1->set_scrollx(0, jm_scrollram[0] & 0x7ff);
	m_sc0_tilemap_2->set_scrollx(0, jm_scrollram[0] & 0x3ff);
	m_sc0_tilemap_3->set_scrollx(0, jm_scrollram[0] & 0x1ff);

	m_sc1_tilemap_0->set_scrollx(0, jm_scrollram[1] & 0xfff);
	m_sc1_tilemap_1->set_scrollx(0, jm_scrollram[1] & 0x7ff);
	m_sc1_tilemap_2->set_scrollx(0, jm_scrollram[1] & 0x3ff);
	m_sc1_tilemap_3->set_scrollx(0, jm_scrollram[1] & 0x1ff);

	m_sc2_tilemap_0->set_scrollx(0, jm_scrollram[2] & 0xfff);
	m_sc2_tilemap_1->set_scrollx(0, jm_scrollram[2] & 0x7ff);
	m_sc2_tilemap_2->set_scrollx(0, jm_scrollram[2] & 0x3ff);
	m_sc2_tilemap_3->set_scrollx(0, jm_scrollram[2] & 0x1ff);

	m_sc3_tilemap_0->set_scrollx(0, jm_scrollram[3] & 0x7ff);
//  empty
	m_sc3_tilemap_2->set_scrollx(0, jm_scrollram[3] & 0x3ff);
	m_sc3_tilemap_3->set_scrollx(0, jm_scrollram[3] & 0x1ff);


	m_sc0_tilemap_0->set_scrolly(0, jm_scrollram[4] & 0x1ff);
	m_sc0_tilemap_1->set_scrolly(0, jm_scrollram[4] & 0x3ff);
	m_sc0_tilemap_2->set_scrolly(0, jm_scrollram[4] & 0x7ff);
	m_sc0_tilemap_3->set_scrolly(0, jm_scrollram[4] & 0xfff);

	m_sc1_tilemap_0->set_scrolly(0, jm_scrollram[5] & 0x1ff);
	m_sc1_tilemap_1->set_scrolly(0, jm_scrollram[5] & 0x3ff);
	m_sc1_tilemap_2->set_scrolly(0, jm_scrollram[5] & 0x7ff);
	m_sc1_tilemap_3->set_scrolly(0, jm_scrollram[5] & 0xfff);

	m_sc2_tilemap_0->set_scrolly(0, jm_scrollram[6] & 0x1ff);
	m_sc2_tilemap_1->set_scrolly(0, jm_scrollram[6] & 0x3ff);
	m_sc2_tilemap_2->set_scrolly(0, jm_scrollram[6] & 0x7ff);
	m_sc2_tilemap_3->set_scrolly(0, jm_scrollram[6] & 0xfff);

	m_sc3_tilemap_0->set_scrolly(0, jm_scrollram[7] & 0xff);
//  empty
	m_sc3_tilemap_2->set_scrolly(0, jm_scrollram[7] & 0x1ff);
	m_sc3_tilemap_3->set_scrolly(0, jm_scrollram[7] & 0x3ff);

	bitmap.fill(m_palette->pen(0xff), cliprect); //selectable by a ram address?

	for(cur_prin=1;cur_prin<=0x8;cur_prin<<=1)
	{
		if(cur_prin==m_sc0_prin) { draw_sc0_layer(screen,bitmap,cliprect); }
		if(cur_prin==m_sc1_prin) { draw_sc1_layer(screen,bitmap,cliprect); }
		if(cur_prin==m_sc2_prin) { draw_sc2_layer(screen,bitmap,cliprect); }
		if(cur_prin==m_sc3_prin) { draw_sc3_layer(screen,bitmap,cliprect); }
	}

	return 0;
}

UINT32 jalmah_state::screen_update_urashima(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *jm_scrollram = m_jm_scrollram;
	/*this game doesn't use the RANGE register at all.*/
	m_sc0_tilemap_0->set_scrollx(0, jm_scrollram[0]);
	m_sc3_tilemap_0->set_scrollx(0, jm_scrollram[3]);
	m_sc0_tilemap_0->set_scrolly(0, jm_scrollram[4]);
	m_sc3_tilemap_0->set_scrolly(0, jm_scrollram[7]);

	bitmap.fill(m_palette->pen(0x1ff), cliprect);//selectable by a ram address?
	if(m_jm_vregs[0] & 1) { m_sc0_tilemap_0->draw(screen, bitmap, cliprect, 0,0); }
	if(m_jm_vregs[3] & 1) { m_sc3_tilemap_0->draw(screen, bitmap, cliprect, 0,0); }
	return 0;
}

WRITE16_MEMBER(jalmah_state::sc0_vram_w)
{
	COMBINE_DATA(&m_sc0_vram[offset]);
	/*2048x256 tilemap*/
	m_sc0_tilemap_0->mark_tile_dirty(offset);
	/*1024x512 tilemap*/
	m_sc0_tilemap_1->mark_tile_dirty(offset);
	/*512x1024 tilemap*/
	m_sc0_tilemap_2->mark_tile_dirty(offset);
	/*256x2048 tilemap*/
	m_sc0_tilemap_3->mark_tile_dirty(offset);
}

WRITE16_MEMBER(jalmah_state::sc3_vram_w)
{
	COMBINE_DATA(&m_sc3_vram[offset]);
	/*2048x256 tilemap*/
	m_sc3_tilemap_0->mark_tile_dirty(offset);
	/*1024x512 tilemap*/
	m_sc3_tilemap_2->mark_tile_dirty(offset);
	/*512x1024 tilemap*/
	m_sc3_tilemap_3->mark_tile_dirty(offset);
}

WRITE16_MEMBER(jalmah_state::sc1_vram_w)
{
	COMBINE_DATA(&m_sc1_vram[offset]);
	/*2048x256 tilemap*/
	m_sc1_tilemap_0->mark_tile_dirty(offset);
	/*1024x512 tilemap*/
	m_sc1_tilemap_1->mark_tile_dirty(offset);
	/*512x1024 tilemap*/
	m_sc1_tilemap_2->mark_tile_dirty(offset);
	/*256x2048 tilemap*/
	m_sc1_tilemap_3->mark_tile_dirty(offset);
}

WRITE16_MEMBER(jalmah_state::sc2_vram_w)
{
	COMBINE_DATA(&m_sc2_vram[offset]);
	/*2048x256 tilemap*/
	m_sc2_tilemap_0->mark_tile_dirty(offset);
	/*1024x512 tilemap*/
	m_sc2_tilemap_1->mark_tile_dirty(offset);
	/*512x1024 tilemap*/
	m_sc2_tilemap_2->mark_tile_dirty(offset);
	/*256x2048 tilemap*/
	m_sc2_tilemap_3->mark_tile_dirty(offset);
}

WRITE16_MEMBER(jalmah_state::jalmah_tilebank_w)
{
	/*
	 xxxx ---- fg bank (used by suchipi)
	 ---- xxxx Priority number (trusted,see mjzoomin)
	*/
	//popmessage("Write to tilebank %02x",data);
	if (ACCESSING_BITS_0_7)
	{
		if (m_sc0bank != ((data & 0xf0) >> 4))
		{
			m_sc0bank = (data & 0xf0) >> 4;
			m_sc0_tilemap_0->mark_all_dirty();
			m_sc0_tilemap_1->mark_all_dirty();
			m_sc0_tilemap_2->mark_all_dirty();
			m_sc0_tilemap_3->mark_all_dirty();
		}
		if (m_pri != (data & 0x0f))
			m_pri = data & 0x0f;
	}
}

WRITE16_MEMBER(jalmah_state::jalmah_scroll_w)
{
	UINT16 *jm_scrollram = m_jm_scrollram;
	UINT16 *jm_vregs = m_jm_vregs;
	//logerror("[%04x]<-%04x\n",(offset+0x10)*2,data);
	switch(offset+(0x10))
	{
		/*These 4 are just video regs,see mjzoomin test*/
		/*
		    ---x ---- Always on with layer 3, 8x8 tiles switch?
		    ---- --xx RANGE registers
		*/
		case (0x24/2): jm_vregs[0] = data; break;
		case (0x2c/2): jm_vregs[1] = data; break;
		case (0x34/2): jm_vregs[2] = data; break;
		case (0x3c/2): jm_vregs[3] = data; break;

		case (0x20/2): jm_scrollram[0] = data; break;
		case (0x28/2): jm_scrollram[1] = data; break;
		case (0x30/2): jm_scrollram[2] = data; break;
		case (0x38/2): jm_scrollram[3] = data; break;
		case (0x22/2): jm_scrollram[4] = data; break;
		case (0x2a/2): jm_scrollram[5] = data; break;
		case (0x32/2): jm_scrollram[6] = data; break;
		case (0x3a/2): jm_scrollram[7] = data; break;
		//default:    popmessage("[%04x]<-%04x",offset+0x10,data);
	}
}

WRITE16_MEMBER(jalmah_state::urashima_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (m_sc0bank != (data & 0x0f))
		{
			m_sc0bank = (data & 0x0f);
			m_sc0_tilemap_0->mark_all_dirty();
			//m_sc0_tilemap_2->mark_all_dirty();
			//m_sc0_tilemap_3->mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(jalmah_state::urashima_sc0_vram_w)
{
	COMBINE_DATA(&m_sc0_vram[offset]);
	m_sc0_tilemap_0->mark_tile_dirty(offset);
}

WRITE16_MEMBER(jalmah_state::urashima_sc3_vram_w)
{
	COMBINE_DATA(&m_sc3_vram[offset]);
	m_sc3_tilemap_0->mark_tile_dirty(offset);
}

/*Urashima Mahjong uses a bigger (and mostly unused/wasted) video register ram.*/
WRITE16_MEMBER(jalmah_state::urashima_vregs_w)
{
	UINT16 *jm_scrollram = m_jm_scrollram;
	UINT16 *jm_vregs = m_jm_vregs;
	//logerror("[%04x]<-%04x\n",(offset)*2,data);
	switch(offset)
	{
		case 0x082/2: jm_vregs[0] = data;     break; //sc0 plane enable
		case 0x084/2: jm_scrollram[0] = data; break; //sc0 x offset
		case 0x086/2: jm_scrollram[4] = data; break; //sc0 y offset

//      case 0x182/2: jm_vregs[0] = data;     break;
//      case 0x184/2: jm_scrollram[0] = data; break;
//      case 0x186/2: jm_scrollram[4] = data; break;

//      case 0x382/2: jm_vregs[0] = data;     break;
//      case 0x384/2: jm_scrollram[0] = data; break;
//      case 0x386/2: jm_scrollram[4] = data; break;

		case 0x882/2: jm_vregs[3] = data;     break; //sc3 plane enable
		case 0x884/2: jm_scrollram[3] = data; break; //sc3 x offset
		case 0x886/2: jm_scrollram[7] = data; break; //sc3 y offset

		/*WRONG!*/
		case 0x602/2: jm_vregs[0] = data;     break;
		case 0x604/2: jm_scrollram[0] = data; break;
		case 0x606/2: jm_scrollram[4] = data; break;

		case 0x77a/2: jm_vregs[0] = data;     break; //sc0 plane enable,flip screen
		case 0x77c/2: jm_scrollram[0] = data; break; //sc0 x offset,flip screen
		case 0x77e/2: jm_scrollram[4] = data; break; //sc0 y offset,flip screen

		case 0xf7a/2: jm_vregs[3] = data;     break; //sc3 plane enable,flip screen
		case 0xf7c/2: jm_scrollram[3] = data; break; //sc3 x offset,flip screen
		case 0xf7e/2: jm_scrollram[7] = data; break; //sc3 y offset,flip screen
		default: break;
	}
	//popmessage("%04x %04x %04x %04x %02x %02x",jm_scrollram[0],jm_scrollram[4],jm_scrollram[3],jm_scrollram[7],jm_vregs[0],jm_vregs[3]);
}

/******************************************************************************************

Protection file start

******************************************************************************************/


/*
MCU program number,different for each game(n.b. the numbering scheme is *mine*,do not
take it seriously...):
0x11 = daireika
0x12 = urashima
0x13 = mjzoomin
0x21 = kakumei
0x22 = kakumei2
0x23 = suchipi

xxxx ---- MCU program revision
---- xxxx MCU program number assignment for each game.
*/

#define DAIREIKA_MCU (0x11)
#define URASHIMA_MCU (0x12)
#define MJZOOMIN_MCU (0x13)
#define KAKUMEI_MCU  (0x21)
#define KAKUMEI2_MCU (0x22)
#define SUCHIPI_MCU  (0x23)


#define MCU_READ(tag, _bit_, _offset_, _retval_) \
if((0xffff - ioport(tag)->read()) & _bit_) { jm_shared_ram[_offset_] = _retval_; }

/*Funky "DMA" / protection thing*/
/*---- -x-- "DMA" execute.*/
/*---- ---x used too very often,I don't have any clue of what it is,it might just be the source or the destination address.*/
WRITE16_MEMBER(jalmah_state::urashima_dma_w)
{
	if(data & 4)
	{
		UINT32 i;
		for(i = 0; i < 0x200; i += 2)
			space.write_word(0x88200 + i, space.read_word(0x88400 + i));
	}
}

/*same as $f00c0 sub-routine,but with additional work-around,to remove from here...*/
void jalmah_state::daireika_palette_dma(UINT16 val)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT32 index_1, index_2, src_addr, tmp_addr;
	/*a0=301c0+jm_shared_ram[0x540/2] & 0xf00 */
	/*a1=88000*/
	src_addr = 0x301c0 + (val * 0x40);
//  popmessage("%08x",src_addr);
	for(index_1 = 0; index_1 < 0x200; index_1 += 0x20)
	{
		tmp_addr = src_addr;
		src_addr = space.read_dword(src_addr);

		for(index_2 = 0; index_2 < 0x20; index_2 += 2)
			space.write_word(0x88000 + index_2 + index_1, space.read_word(src_addr + index_2));

		src_addr = tmp_addr + 4;
	}
}

/*RAM-based protection handlings*/
void jalmah_state::daireika_mcu_run()
{
	UINT16 *jm_shared_ram = m_jm_shared_ram;

	if(((jm_shared_ram[0x550/2] & 0xf00) == 0x700) && ((jm_shared_ram[0x540/2] & 0xf00) != m_dma_old))
	{
		m_dma_old = jm_shared_ram[0x540/2] & 0xf00;
		daireika_palette_dma(((jm_shared_ram[0x540/2] & 0x0f00) >> 8));
	}

	if(m_test_mode)  //service_mode
	{
		jm_shared_ram[0x000/2] = ioport("KEY0")->read();
		jm_shared_ram[0x002/2] = ioport("KEY1")->read();
		jm_shared_ram[0x004/2] = ioport("KEY2")->read();
		jm_shared_ram[0x006/2] = ioport("KEY3")->read();
		jm_shared_ram[0x008/2] = ioport("KEY4")->read();
		jm_shared_ram[0x00a/2] = ioport("KEY5")->read();
	}
	else
	{
		jm_shared_ram[0x000/2] = 0x0000;
		MCU_READ("KEY1", 0x0001, 0x000/2, 0x00);        /*FF*/
		MCU_READ("KEY2", 0x0400, 0x000/2, 0x01);        /*A*/
		MCU_READ("KEY2", 0x1000, 0x000/2, 0x02);        /*B*/
		MCU_READ("KEY2", 0x0200, 0x000/2, 0x03);        /*C*/
		MCU_READ("KEY2", 0x0800, 0x000/2, 0x04);        /*D*/
		MCU_READ("KEY2", 0x0004, 0x000/2, 0x05);        /*E*/
		MCU_READ("KEY2", 0x0010, 0x000/2, 0x06);        /*F*/
		MCU_READ("KEY2", 0x0002, 0x000/2, 0x07);        /*G*/
		MCU_READ("KEY2", 0x0008, 0x000/2, 0x08);        /*H*/
		MCU_READ("KEY1", 0x0400, 0x000/2, 0x09);        /*I*/
		MCU_READ("KEY1", 0x1000, 0x000/2, 0x0a);        /*J*/
		MCU_READ("KEY1", 0x0200, 0x000/2, 0x0b);        /*K*/
		MCU_READ("KEY1", 0x0800, 0x000/2, 0x0c);        /*L*/
		MCU_READ("KEY1", 0x0004, 0x000/2, 0x0d);        /*M*/
		MCU_READ("KEY1", 0x0010, 0x000/2, 0x0e);        /*N*/
		MCU_READ("KEY0", 0x0200, 0x000/2, 0x0f);        /*RON   (trusted)*/
		MCU_READ("KEY0", 0x1000, 0x000/2, 0x10);        /*REACH (trusted)*/
		MCU_READ("KEY0", 0x0400, 0x000/2, 0x11);        /*KAN            */
		MCU_READ("KEY1", 0x0008, 0x000/2, 0x12);        /*PON            */
		MCU_READ("KEY1", 0x0002, 0x000/2, 0x13);        /*CHI   (trusted)*/
		MCU_READ("KEY0", 0x0004, 0x000/2, 0x14);        /*START1*/
	}
	m_prg_prot++;
	if(m_prg_prot > 0x10) { m_prg_prot = 0; }
	jm_shared_ram[0x00e/2] = m_prg_prot;
}

void jalmah_state::mjzoomin_mcu_run()
{
	UINT16 *jm_shared_ram = m_jm_shared_ram;

	if(m_test_mode)  //service_mode
	{
		jm_shared_ram[0x000/2] = ioport("KEY0")->read();
		jm_shared_ram[0x002/2] = ioport("KEY1")->read();
		jm_shared_ram[0x004/2] = ioport("KEY2")->read();
		jm_shared_ram[0x006/2] = ioport("KEY3")->read();
		jm_shared_ram[0x008/2] = ioport("KEY4")->read();
		jm_shared_ram[0x00a/2] = ioport("KEY5")->read();
	}
	else
	{
		jm_shared_ram[0x000/2] = 0x0000;
		MCU_READ("KEY1", 0x0001, 0x000/2, 0x00);        /*FF*/
		MCU_READ("KEY2", 0x0400, 0x000/2, 0x01);        /*A*/
		MCU_READ("KEY2", 0x1000, 0x000/2, 0x02);        /*B*/
		MCU_READ("KEY2", 0x0200, 0x000/2, 0x03);        /*C*/
		MCU_READ("KEY2", 0x0800, 0x000/2, 0x04);        /*D*/
		MCU_READ("KEY2", 0x0004, 0x000/2, 0x05);        /*E*/
		MCU_READ("KEY2", 0x0010, 0x000/2, 0x06);        /*F*/
		MCU_READ("KEY2", 0x0002, 0x000/2, 0x07);        /*G*/
		MCU_READ("KEY2", 0x0008, 0x000/2, 0x08);        /*H*/
		MCU_READ("KEY1", 0x0400, 0x000/2, 0x09);        /*I*/
		MCU_READ("KEY1", 0x1000, 0x000/2, 0x0a);        /*J*/
		MCU_READ("KEY1", 0x0200, 0x000/2, 0x0b);        /*K*/
		MCU_READ("KEY1", 0x0800, 0x000/2, 0x0c);        /*L*/
		MCU_READ("KEY1", 0x0004, 0x000/2, 0x0d);        /*M*/
		MCU_READ("KEY1", 0x0010, 0x000/2, 0x0e);        /*N*/
		MCU_READ("KEY0", 0x0200, 0x000/2, 0x0f);        /*RON   (trusted)*/
		MCU_READ("KEY0", 0x1000, 0x000/2, 0x10);        /*REACH (trusted)*/
		MCU_READ("KEY0", 0x0400, 0x000/2, 0x11);        /*KAN            */
		MCU_READ("KEY1", 0x0008, 0x000/2, 0x12);        /*PON            */
		MCU_READ("KEY1", 0x0002, 0x000/2, 0x13);        /*CHI   (trusted)*/
		MCU_READ("KEY0", 0x0004, 0x000/2, 0x14);        /*START1*/
	}
	jm_shared_ram[0x00c/2] = machine().rand() & 0xffff;
	m_prg_prot++;
	if(m_prg_prot > 0x10) { m_prg_prot = 0; }
	jm_shared_ram[0x00e/2] = m_prg_prot;
}

void jalmah_state::urashima_mcu_run()
{
	UINT16 *jm_shared_ram = m_jm_shared_ram;

	if(m_test_mode)  //service_mode
	{
		jm_shared_ram[0x300/2] = ioport("KEY0")->read();
		jm_shared_ram[0x302/2] = ioport("KEY1")->read();
		jm_shared_ram[0x304/2] = ioport("KEY2")->read();
		jm_shared_ram[0x306/2] = ioport("KEY3")->read();
		jm_shared_ram[0x308/2] = ioport("KEY4")->read();
		jm_shared_ram[0x30a/2] = ioport("KEY5")->read();
	}
	else
	{
		jm_shared_ram[0x300/2] = 0x0000;
		MCU_READ("KEY1", 0x0001, 0x300/2, 0x00);        /*FF*/
		MCU_READ("KEY2", 0x0400, 0x300/2, 0x01);        /*A*/
		MCU_READ("KEY2", 0x1000, 0x300/2, 0x02);        /*B*/
		MCU_READ("KEY2", 0x0200, 0x300/2, 0x03);        /*C*/
		MCU_READ("KEY2", 0x0800, 0x300/2, 0x04);        /*D*/
		MCU_READ("KEY2", 0x0004, 0x300/2, 0x05);        /*E*/
		MCU_READ("KEY2", 0x0010, 0x300/2, 0x06);        /*F*/
		MCU_READ("KEY2", 0x0002, 0x300/2, 0x07);        /*G*/
		MCU_READ("KEY2", 0x0008, 0x300/2, 0x08);        /*H*/
		MCU_READ("KEY1", 0x0400, 0x300/2, 0x09);        /*I*/
		MCU_READ("KEY1", 0x1000, 0x300/2, 0x0a);        /*J*/
		MCU_READ("KEY1", 0x0200, 0x300/2, 0x0b);        /*K*/
		MCU_READ("KEY1", 0x0800, 0x300/2, 0x0c);        /*L*/
		MCU_READ("KEY1", 0x0004, 0x300/2, 0x0d);        /*M*/
		MCU_READ("KEY1", 0x0010, 0x300/2, 0x0e);        /*N*/
		MCU_READ("KEY0", 0x0200, 0x300/2, 0x0f);        /*RON   (trusted)*/
		MCU_READ("KEY0", 0x1000, 0x300/2, 0x10);        /*REACH (trusted)*/
		MCU_READ("KEY0", 0x0400, 0x300/2, 0x11);        /*KAN            */
		MCU_READ("KEY1", 0x0008, 0x300/2, 0x12);        /*PON            */
		MCU_READ("KEY1", 0x0002, 0x300/2, 0x13);        /*CHI   (trusted)*/
		MCU_READ("KEY0", 0x0004, 0x300/2, 0x14);        /*START1*/
	}
	jm_shared_ram[0x30c/2] = machine().rand() & 0xffff;
	m_prg_prot++;
	if(m_prg_prot > 0x10) { m_prg_prot = 0; }
	jm_shared_ram[0x30e/2] = m_prg_prot;
}

void jalmah_state::second_mcu_run()
{
	UINT16 *jm_shared_ram = m_jm_shared_ram;
	if(m_test_mode)  //service_mode
	{
		jm_shared_ram[0x200/2] = ioport("KEY0")->read();
		jm_shared_ram[0x202/2] = ioport("KEY1")->read();
		jm_shared_ram[0x204/2] = ioport("KEY2")->read();
	}
	else
	{
		jm_shared_ram[0x200/2] = 0x0000;
		MCU_READ("KEY1", 0x0001, 0x200/2, 0x00);        /*FF*/
		MCU_READ("KEY2", 0x0400, 0x200/2, 0x01);        /*A*/
		MCU_READ("KEY2", 0x1000, 0x200/2, 0x02);        /*B*/
		MCU_READ("KEY2", 0x0200, 0x200/2, 0x03);        /*C*/
		MCU_READ("KEY2", 0x0800, 0x200/2, 0x04);        /*D*/
		MCU_READ("KEY2", 0x0004, 0x200/2, 0x05);        /*E*/
		MCU_READ("KEY2", 0x0010, 0x200/2, 0x06);        /*F*/
		MCU_READ("KEY2", 0x0002, 0x200/2, 0x07);        /*G*/
		MCU_READ("KEY2", 0x0008, 0x200/2, 0x08);        /*H*/
		MCU_READ("KEY1", 0x0400, 0x200/2, 0x09);        /*I*/
		MCU_READ("KEY1", 0x1000, 0x200/2, 0x0a);        /*J*/
		MCU_READ("KEY1", 0x0200, 0x200/2, 0x0b);        /*K*/
		MCU_READ("KEY1", 0x0800, 0x200/2, 0x0c);        /*L*/
		MCU_READ("KEY1", 0x0004, 0x200/2, 0x0d);        /*M*/
		MCU_READ("KEY1", 0x0010, 0x200/2, 0x0e);        /*N*/
		MCU_READ("KEY0", 0x0200, 0x200/2, 0x0f);        /*RON*/
		MCU_READ("KEY0", 0x1000, 0x200/2, 0x10);        /*REACH*/
		MCU_READ("KEY0", 0x0400, 0x200/2, 0x11);        /*KAN*/
		MCU_READ("KEY1", 0x0008, 0x200/2, 0x12);        /*PON*/
		MCU_READ("KEY1", 0x0002, 0x200/2, 0x13);        /*CHI*/
		MCU_READ("KEY0", 0x0004, 0x200/2, 0x14);        /*START1*/

//      MCU_READ("KEY0", 0x0004, 0x7b8/2, 0x03);        /*START1(correct?)  */
	}
	jm_shared_ram[0x20c/2] = machine().rand() & 0xffff; //kakumei2

}

TIMER_DEVICE_CALLBACK_MEMBER(jalmah_state::jalmah_mcu_sim)
{
	switch(m_mcu_prg)
	{
		/*
		    #define DAIREIKA_MCU (0x11)
		    #define URASHIMA_MCU (0x12)
		    #define MJZOOMIN_MCU (0x13)
		    #define KAKUMEI_MCU  (0x21)
		    #define KAKUMEI2_MCU (0x22)
		    #define SUCHIPI_MCU  (0x23)
		*/
			case MJZOOMIN_MCU: mjzoomin_mcu_run(); break;
			case DAIREIKA_MCU: daireika_mcu_run(); break;
			case URASHIMA_MCU: urashima_mcu_run(); break;
			case KAKUMEI_MCU:
			case KAKUMEI2_MCU:
			case SUCHIPI_MCU:  second_mcu_run(); break;
	}
}

/******************************************************************************************

Basic driver start

******************************************************************************************/


WRITE16_MEMBER(jalmah_state::jalmah_okirom_w)
{
	if(ACCESSING_BITS_0_7)
	{
		UINT8 *oki = memregion("oki")->base();

		m_oki_rom = data & 1;

		/* ZA appears to be related to the banking, or maybe kakumei2 uses PAL shuffling and this is for something else? */
		m_oki_za = (data & 2) ? 1 : 0;

		memcpy(&oki[0x20000], &oki[(m_oki_rom * 0x80000) + ((m_oki_bank+m_oki_za) * 0x20000) + 0x40000], 0x20000);
	}

	//popmessage("PC=%06x %02x %02x %02x %08x",space.device().safe_pc(),m_oki_rom,m_oki_za,m_oki_bank,(m_oki_rom * 0x80000) + ((m_oki_bank+m_oki_za) * 0x20000) + 0x40000);
}

WRITE16_MEMBER(jalmah_state::jalmah_okibank_w)
{
	if(ACCESSING_BITS_0_7)
	{
		UINT8 *oki = memregion("oki")->base();

		m_oki_bank = data & 3;

		memcpy(&oki[0x20000], &oki[(m_oki_rom * 0x80000) + ((m_oki_bank+m_oki_za) * 0x20000) + 0x40000], 0x20000);
	}

	//popmessage("PC=%06x %02x %02x %02x %08x",space.device().safe_pc(),m_oki_rom,m_oki_za,m_oki_bank,(m_oki_rom * 0x80000) + ((m_oki_bank+m_oki_za) * 0x20000) + 0x40000);
}

WRITE16_MEMBER(jalmah_state::jalmah_flip_screen_w)
{
	/*---- ----x flip screen*/
	flip_screen_set(data & 1);

//  popmessage("%04x",data);
}

static ADDRESS_MAP_START( jalmah, AS_PROGRAM, 16, jalmah_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("DSW")
	//       0x080004, 0x080005  MCU read,different for each game
	AM_RANGE(0x080010, 0x080011) AM_WRITE(jalmah_flip_screen_w)
	//       0x080012, 0x080013  MCU write related,same for each game
	//       0x080014, 0x080015  MCU write related,same for each game
/**/AM_RANGE(0x080016, 0x080017) AM_RAM_WRITE(jalmah_tilebank_w)
	AM_RANGE(0x080018, 0x080019) AM_WRITE(jalmah_okibank_w)
	AM_RANGE(0x08001a, 0x08001b) AM_WRITE(jalmah_okirom_w)
/**/AM_RANGE(0x080020, 0x08003f) AM_RAM_WRITE(jalmah_scroll_w)
	AM_RANGE(0x080040, 0x080041) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	//       0x084000, 0x084001  ?
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") /* Palette RAM */
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(sc0_vram_w) AM_SHARE("sc0_vram")
	AM_RANGE(0x094000, 0x097fff) AM_RAM_WRITE(sc1_vram_w) AM_SHARE("sc1_vram")
	AM_RANGE(0x098000, 0x09bfff) AM_RAM_WRITE(sc2_vram_w) AM_SHARE("sc2_vram")
	AM_RANGE(0x09c000, 0x09ffff) AM_RAM_WRITE(sc3_vram_w) AM_SHARE("sc3_vram")
	AM_RANGE(0x0f0000, 0x0f0fff) AM_RAM AM_SHARE("jshared_ram")/*shared with MCU*/
	AM_RANGE(0x0f1000, 0x0fffff) AM_RAM /*Work Ram*/
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_SHARE("jmcu_code")/*extra RAM for MCU code prg (NOT ON REAL HW!!!)*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( urashima, AS_PROGRAM, 16, jalmah_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("DSW")
	//       0x080004, 0x080005  MCU read,different for each game
	AM_RANGE(0x080010, 0x080011) AM_WRITE(jalmah_flip_screen_w)
	//       0x080012, 0x080013  MCU write related,same for each game
	//       0x080014, 0x080015  MCU write related,same for each game
/**/AM_RANGE(0x080016, 0x080017) AM_RAM_WRITE(urashima_dma_w)
	AM_RANGE(0x080018, 0x080019) AM_WRITE(jalmah_okibank_w)
	AM_RANGE(0x08001a, 0x08001b) AM_WRITE(jalmah_okirom_w)
/**/AM_RANGE(0x08001c, 0x08001d) AM_RAM_WRITE(urashima_bank_w)
	AM_RANGE(0x080040, 0x080041) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	//       0x084000, 0x084001  ?
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") /* Palette RAM */
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(urashima_sc0_vram_w) AM_SHARE("sc0_vram")
	AM_RANGE(0x094000, 0x097fff) AM_RAM_WRITE(urashima_sc0_vram_w)
	AM_RANGE(0x098000, 0x09bfff) AM_RAM_WRITE(urashima_sc0_vram_w)
//  AM_RANGE(0x094000, 0x097fff) AM_RAM_WRITE(urashima_sc1_vram_w) AM_SHARE("sc1_vram")/*unused*/
//  AM_RANGE(0x098000, 0x09bfff) AM_RAM_WRITE(urashima_sc2_vram_w) AM_SHARE("sc2_vram")/*unused*/
	/*$9c000-$9cfff Video Registers*/
/**/AM_RANGE(0x09c000, 0x09dfff) AM_WRITE(urashima_vregs_w)
/**///AM_RANGE(0x09c480, 0x09c49f) AM_RAM_WRITE(urashima_sc2vregs_w)
	AM_RANGE(0x09e000, 0x0a1fff) AM_RAM_WRITE(urashima_sc3_vram_w) AM_SHARE("sc3_vram")
	AM_RANGE(0x0f0000, 0x0f0fff) AM_RAM AM_SHARE("jshared_ram")/*shared with MCU*/
	AM_RANGE(0x0f1000, 0x0fffff) AM_RAM /*Work Ram*/
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_SHARE("jmcu_code")/*extra RAM for MCU code prg (NOT ON REAL HW!!!)*/
ADDRESS_MAP_END

static INPUT_PORTS_START( common )
	PORT_START("SYSTEM")
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( ctrl_mj1 )
	PORT_START("KEY0")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xe9fb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0xe1e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0xe1e1, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ctrl_mj2 )
	PORT_START("KEY3")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0xe9fb, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2)

	PORT_START("KEY4")
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

	PORT_START("KEY5")
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
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW )

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
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Harder ) )
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

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P1 Mahjong Flip Flop")  PORT_CODE(KEYCODE_2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW2:2" )   // Unused according to the manual
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW2:4" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW2:5" )   // Unused according to the manual
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
	PORT_DIPSETTING(      0x1800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW1:6" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW1:7" )   // Unused according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW1:8" )   // Unused according to the manual
INPUT_PORTS_END

static INPUT_PORTS_START( kakumei2 )
	PORT_INCLUDE( common )
	PORT_INCLUDE( ctrl_mj1 )

	PORT_MODIFY("KEY1")
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

static INPUT_PORTS_START( suchipi )
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

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			16*32+0*4, 16*32+1*4, 16*32+2*4, 16*32+3*4, 16*32+4*4, 16*32+5*4, 16*32+6*4, 16*32+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static GFXDECODE_START( jalmah )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0x300, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0x200, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 0x100, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 0x000, 16 )
GFXDECODE_END

/*different color offsets*/
static GFXDECODE_START( urashima )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0x000, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0x100, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 0x100, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 0x100, 16 )
GFXDECODE_END

void jalmah_state::machine_reset()
{
	m_respcount = 0;
	/*check if we are into service or normal mode*/
	switch(m_mcu_prg)
	{
		case MJZOOMIN_MCU:
		case DAIREIKA_MCU:
			m_test_mode = (~(ioport("SYSTEM")->read()) & 0x0008) ? (1) : (0);
			break;
		case URASHIMA_MCU:
			m_test_mode = ((~(ioport("SYSTEM")->read()) & 0x0008) || (~(ioport("DSW")->read()) & 0x8000)) ? (1) : (0);
			break;
		case KAKUMEI_MCU:
		case KAKUMEI2_MCU:
		case SUCHIPI_MCU:
			m_test_mode = (~(ioport("DSW")->read()) & 0x0004) ? (1) : (0);
			break;
	}
}

static MACHINE_CONFIG_START( jalmah, jalmah_state )
	MCFG_CPU_ADD("maincpu" , M68000, 12000000) /* 68000-8 */
	MCFG_CPU_PROGRAM_MAP(jalmah)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jalmah_state,  irq2_line_hold)

	//M50747 MCU

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jalmah)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(jalmah_state, screen_update_jalmah)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("mcusim", jalmah_state, jalmah_mcu_sim, attotime::from_hz(10000))

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 4000000, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( urashima, jalmah )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(urashima)

	MCFG_GFXDECODE_MODIFY("gfxdecode", urashima)

	MCFG_VIDEO_START_OVERRIDE(jalmah_state,urashima)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(jalmah_state, screen_update_urashima)
MACHINE_CONFIG_END

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

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "um-3.22c",      0x40000, 0x80000, CRC(9fd8c8fa) SHA1(0346f74c03a4daa7a84b64c9edf0e54297c82fd9) )
	ROM_COPY( "oki" ,          0x40000, 0x00000, 0x40000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "um-5.22j",       0x000000, 0x020000, CRC(991776a2) SHA1(56740553d7d26aaeb9bec8557727030950bb01f7) )  /* 8x8 tiles */

	ROM_REGION( 0x080000, "gfx2", 0 ) /* 16x16 Tiles */
	ROM_LOAD( "um-6.2l",    0x000000, 0x080000, CRC(076be5b5) SHA1(77444025f149a960137d3c79abecf9b30defa341) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "um-7.4l",    0x000000, 0x080000, CRC(d2a68cfb) SHA1(eb6cb1fad306b697b2035a31ad48e8996722a032) )

	ROM_REGION( 0x200000, "gfx4", 0 ) /* BG3 */
	/*0*/
	ROM_COPY( "gfx2" , 0x000000, 0x000000, 0x40000 )
	ROM_COPY( "gfx2",  0x040000, 0x040000, 0x40000 )
	/*1*/
	ROM_COPY( "gfx2",  0x000000, 0x080000, 0x40000 )
	ROM_COPY( "gfx3",  0x000000, 0x0c0000, 0x40000 )
	/*2*/
	ROM_COPY( "gfx2",  0x000000, 0x100000, 0x40000 )
	ROM_COPY( "gfx3",  0x040000, 0x140000, 0x40000 )

	ROM_REGION( 0x0240, "user1", 0 )
	ROM_LOAD( "um-10.2b",      0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )   /* unknown */
	ROM_LOAD( "um-11.2c",      0x0100, 0x0100, CRC(ff5660cf) SHA1(a4635dcf9d6dd637ea4f36f1ad233db0bd039731) )   /* unknown */
	ROM_LOAD( "um-12.20c",     0x0200, 0x0020, CRC(bdb66b02) SHA1(8755244de638d7e835e35e08c62b0612958e6ca5) )   /* unknown */
	ROM_LOAD( "um-13.10l",     0x0220, 0x0020, CRC(4ce07ec0) SHA1(5f5744ddc7f258307f036fde4c0a8e6271b2d1f9) )   /* unknown */
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

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "mj3.bin", 0x40000, 0x80000, CRC(65bb350c) SHA1(e77866f2d612a0973adc616717e7c89a37d6c48e) )
	ROM_COPY( "oki" ,    0x40000, 0x00000, 0x40000 )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* BG0 */
	ROM_LOAD( "mj14.bin", 0x00000, 0x10000, CRC(c84c5577) SHA1(6437368d3be39739d62158590ecd373aa070a9b2) )
	ROM_LOAD( "mj13.bin", 0x10000, 0x10000, CRC(c54bca14) SHA1(ee9c99858817aedd70bd6266b7a71c3c5ad00607) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* BG3 */
	ROM_LOAD( "mj10.bin", 0x00000, 0x80000, CRC(1f5509a5) SHA1(4dcdee0e159956cf73f5f85ce278479be2a9ca9f) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* BG2 */
//  ROM_COPY( "gfx4",     0x20000, 0x20000, 0x20000 )/*mj10.bin*/
	ROM_LOAD( "mj11.bin", 0x00000, 0x20000, CRC(14867c51) SHA1(b282b5048a55c9ad72ceb0d23f010a0fee78704f) )
	ROM_LOAD( "mj12.bin", 0x20000, 0x20000, CRC(236f809f) SHA1(9e15dd8a810a9d4f7f75f084d6bd277ea7d0e40a) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* BG1 */
	ROM_COPY( "gfx1",     0x10000, 0x00000, 0x10000 )/*mj12.bin*/

	ROM_REGION( 0x220, "user1", 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
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

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "zoomin-3.bin", 0x40000, 0x80000, CRC(07d7b8cd) SHA1(e05ce80ffb945b04f93f8c49d0c840b0bff6310b) )
	ROM_COPY( "oki" ,         0x40000, 0x00000, 0x40000 )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* BG0 */
	ROM_LOAD( "zoomin14.bin", 0x00000, 0x20000, CRC(4e32aa45) SHA1(450a3449ca8b4f0dfe8b62cceaee9366eaf3dc3d) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* BG1 */
	ROM_LOAD( "zoomin13.bin", 0x00000, 0x20000, CRC(888d79fe) SHA1(eb9671d4c7608edd1231dc0cae47aab2430cbd66) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* BG2 */
	ROM_LOAD( "zoomin12.bin", 0x00000, 0x40000, CRC(b0b94554) SHA1(10490b7475810910140ce075e62f604b914e5511) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* BG3 */
	ROM_LOAD( "zoomin10.bin", 0x00000, 0x80000, CRC(40aec575) SHA1(ef7a3c7a94523c5967ab774936b873c9629e0e44) )

	ROM_REGION( 0x220, "user1", 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
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

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "rom3.bin", 0x00000, 0x40000, CRC(c9b7a526) SHA1(edec57e66d4ff601c8fdef7b1405af84a3f3d883) )
	ROM_RELOAD(           0x40000, 0x40000 )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* BG0 */
	ROM_LOAD( "rom14.bin", 0x00000, 0x20000, CRC(63e88dd6) SHA1(58734c8caf1b1ddc4cf0437ffd8109292b76c4e1) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* BG1 */
	ROM_LOAD( "rom13.bin", 0x00000, 0x20000, CRC(9bef4fc2) SHA1(6598ab9dba513efcda01e47cc7752b47a97f2c6a) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* BG2 */
	ROM_LOAD( "rom12.bin", 0x00000, 0x40000, CRC(31620a61) SHA1(11593ca7760e1a628e63aa48d9ad3800cf7af275) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* BG3 */
	ROM_LOAD( "rom10.bin", 0x00000, 0x80000, CRC(88366377) SHA1(163a08415a631c8a09a0a55bc2819988d850f2ad) )

	ROM_REGION( 0x220, "user1", 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Kakumei2 Princess League (JPN Ver.)
(c)1992 Jaleco

*/

ROM_START( kakumei2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj-8956.1", 0x00001, 0x40000, CRC(db4ce32f) SHA1(1ae13627b9922143f462b1c3bbed87374f6e1667) )
	ROM_LOAD16_BYTE( "mj-8956.2", 0x00000, 0x40000, CRC(0f942507) SHA1(7ec2fbeb9a34dfc80c4df3de8397388db13f5c7c) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "92000-01.3", 0x040000, 0x80000, CRC(4b0ed440) SHA1(11961d217a41f92b60d5083a5e346c245f7db620) )
	ROM_COPY( "oki" ,       0x040000, 0x00000, 0x40000 )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* BG0 */
	ROM_LOAD( "mj-8956.14", 0x00000, 0x20000, CRC(2b2fe999) SHA1(d9d601e2c008791f5bff6e7b1340f754dd094201) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* BG1 */
	ROM_LOAD( "mj-8956.13", 0x00000, 0x20000, CRC(afe93cf4) SHA1(1973dc5821c6df68e20f8a84b5c9ae281dd3f85f) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* BG2 */
	ROM_LOAD( "mj-8956.12", 0x00000, 0x40000, CRC(4a088f69) SHA1(468c446d1f345dfd628cdd66ca71cf82e02abe6f) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* BG3 */
	ROM_LOAD( "92000-02.10", 0x00000, 0x80000, CRC(338fa9b2) SHA1(05ba4b3c44249cf92be238bf53d6345dc49b0881) )

	ROM_REGION( 0x220, "user1", 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
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

ROM_START( suchipi )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x40000, CRC(e37cc745) SHA1(73b3314d27a0332068e0d2bbc08d7401e371da1b) )
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x40000, CRC(42ecf88a) SHA1(7bb85470bc9f94c867646afeb91c4730599ea299) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "oki_data", ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "3.bin", 0x00000, 0x80000, CRC(691b5387) SHA1(b8bc9f904eab7653566042b18d89276d537ba586) )
	ROM_LOAD( "4.bin", 0x80000, 0x80000, CRC(3fe932a1) SHA1(9e768b901738ee9eba207a67c4fd19efb0035a68) )

	ROM_REGION( 0x140000, "oki", ROMREGION_ERASEFF ) /* Samples */
	ROM_COPY( "oki_data" , 0x00000, 0x000000+0x00000, 0x40000 )

	/* PAL address shuffling for the BGM data (TODO: check this with a side-by-side test)*/
	ROM_COPY( "oki_data" , 0x20000, 0x000000+0x40000, 0x20000 ) // 0
	ROM_COPY( "oki_data" , 0x40000, 0x020000+0x40000, 0x20000 ) // 1
	ROM_COPY( "oki_data" , 0x60000, 0x040000+0x40000, 0x20000 ) // 2
	ROM_COPY( "oki_data" , 0x00000, 0x060000+0x40000, 0x20000 ) // 3

	ROM_COPY( "oki_data" , 0x80000, 0x080000+0x40000, 0x40000 )
	ROM_COPY( "oki_data" , 0xc0000, 0x0c0000+0x40000, 0x40000 )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* BG0 */
	ROM_LOAD( "14.bin", 0x00000, 0x20000, CRC(e465a540) SHA1(10e19599ab90b0c0b6ef6ee41f16620bd1ba6800) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* BG1 */
	ROM_LOAD( "13.bin", 0x00000, 0x20000, CRC(99466044) SHA1(ca31b58a5d4656f95d80ddb9bc1f9a53f5f2446c) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* BG2 */
	ROM_LOAD( "12.bin", 0x00000, 0x40000, CRC(146596eb) SHA1(f85e92e6dc9ebef5e67d28f1d450225cd2a2abaa) )

	ROM_REGION( 0x200000, "gfx4", 0 ) /* BG3 */
	ROM_LOAD( "7.bin",  0x000000, 0x80000, CRC(18caf6f3) SHA1(3df6b257867487adcba1a05c8745413d9a15c3d7) )
	ROM_LOAD( "8.bin",  0x080000, 0x80000, CRC(0403399a) SHA1(8d39a68b3a1a431afe93ff485e837389a4502d0c) )
	ROM_LOAD( "9.bin",  0x100000, 0x80000, CRC(8a348246) SHA1(13516c48bdbe8d78e7517473ef2835a4dea2ce93) )
	ROM_LOAD( "10.bin", 0x180000, 0x80000, CRC(2b0d1afd) SHA1(40009b450901567052aa63c4629a2f7a10343e63) )

	ROM_REGION( 0x220, "user1", 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "pr93035.17", 0x200, 0x020, CRC(ab28ae42) SHA1(e05652c4bd5db4c7d7a1bfdeb63841e8b019f24c) )
ROM_END


/******************************************************************************************

MCU code snippets

******************************************************************************************/

READ16_MEMBER(jalmah_state::urashima_mcu_r)
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
	if (m_respcount >= ARRAY_LENGTH(resp)) m_respcount = 0;

//  logerror("%04x: mcu_r %02x\n",space.device().safe_pc(),res);

	return res;
}

/*
data value is REQ under mjzoomin video test menu.It is related to the MCU?
*/
WRITE16_MEMBER(jalmah_state::urashima_mcu_w)
{
	UINT16 *jm_shared_ram = m_jm_shared_ram;
	UINT16 *jm_mcu_code = m_jm_mcu_code;
	if(ACCESSING_BITS_0_7 && data)
	{
		/*******************************************************
		1st M68k code uploaded by the MCU (sound prg)
		*******************************************************/
		jm_shared_ram[0x0320/2] = 0x4ef9;
		jm_shared_ram[0x0322/2] = 0x0010;
		jm_shared_ram[0x0324/2] = 0x2000;//jmp $102000

		jm_mcu_code[0x2000/2] = 0x0040;
		jm_mcu_code[0x2002/2] = 0x0080;//ori $80,D0
		jm_mcu_code[0x2004/2] = 0x33c0;
		jm_mcu_code[0x2006/2] = 0x0008;
		jm_mcu_code[0x2008/2] = 0x0040;
		jm_mcu_code[0x200a/2] = 0x4e71;//0x6100;
		jm_mcu_code[0x200c/2] = 0x4e71;//0x000c;
		jm_mcu_code[0x200e/2] = 0x33fc;
		jm_mcu_code[0x2010/2] = 0x0010;
		jm_mcu_code[0x2012/2] = 0x0008;
		jm_mcu_code[0x2014/2] = 0x0040;
		jm_mcu_code[0x2016/2] = 0x4e75;
		jm_mcu_code[0x2018/2] = 0x3239;
		jm_mcu_code[0x201a/2] = 0x0008;
		jm_mcu_code[0x201c/2] = 0x0040;
		jm_mcu_code[0x201e/2] = 0x0241;
		jm_mcu_code[0x2020/2] = 0x0001;
		jm_mcu_code[0x2022/2] = 0x66f4;
		jm_mcu_code[0x2024/2] = 0x4e75;
		/*******************************************************
		1st alt M68k code uploaded by the MCU (Input test mode)
		*******************************************************/
		/*similar to mjzoomin but with offset summed with 0x300?*/
		/*tx scrollx = $200*/
		jm_shared_ram[0x03c6/2] = 0x6008;//bra $+10
		jm_shared_ram[0x03d0/2] = 0x4ef9;
		jm_shared_ram[0x03d2/2] = 0x0010;
		jm_shared_ram[0x03d4/2] = 0x0000;//jmp $100000
		jm_mcu_code[0x0000/2] = 0x4E71;//0x33fc;//CAREFUL!!!
		jm_mcu_code[0x0002/2] = 0x4E71;//0x0400;
		jm_mcu_code[0x0004/2] = 0x4E71;//0x0008;
		jm_mcu_code[0x0006/2] = 0x4E71;//0x0038;
		/*priority = 5(Something that shows the text layer,to be checked after that the priority works )*/
		jm_mcu_code[0x0008/2] = 0x33fc;
		jm_mcu_code[0x000a/2] = 0x0005;
		jm_mcu_code[0x000c/2] = 0x0008;
		jm_mcu_code[0x000e/2] = 0x0016;//move.w #
		jm_mcu_code[0x0010/2] = 0xd0fc;
		jm_mcu_code[0x0012/2] = 0x0060;//adda.w $60,A0
		jm_mcu_code[0x0014/2] = 0x92fc;
		jm_mcu_code[0x0016/2] = 0x0200;//suba.w $200,A1
		jm_mcu_code[0x0018/2] = 0x32d8;//move.w (A0)+,(A1)+
		jm_mcu_code[0x001a/2] = 0x51c9;
		jm_mcu_code[0x001c/2] = 0xfffc;//dbra D1,f00ca
		jm_mcu_code[0x001e/2] = 0x4e75;//rts

		/*******************************************************
		2nd M68k code uploaded by the MCU (tile upload)
		*******************************************************/
		jm_shared_ram[0x03ca/2] = 0x4ef9;
		jm_shared_ram[0x03cc/2] = 0x0010;
		jm_shared_ram[0x03ce/2] = 0x0800;//jmp $100800
		jm_mcu_code[0x0800/2] = 0x32da;
		jm_mcu_code[0x0802/2] = 0x51c8;
		jm_mcu_code[0x0804/2] = 0xfffc;
		jm_mcu_code[0x0806/2] = 0x4e75;
		/*******************************************************
		3rd M68k code uploaded by the MCU (palette upload)
		*******************************************************/
		jm_shared_ram[0x03c0/2] = 0x4ef9;
		jm_shared_ram[0x03c2/2] = 0x0010;
		jm_shared_ram[0x03c4/2] = 0x1000;//jmp $101000


		jm_mcu_code[0x1000/2] = 0x4e71;//0x92fc;
		jm_mcu_code[0x1002/2] = 0x4e71;//0x0200;//suba.w $200,A1
		jm_mcu_code[0x1004/2] = 0xb3fc;
		jm_mcu_code[0x1006/2] = 0x0008;
		jm_mcu_code[0x1008/2] = 0x8200;
		jm_mcu_code[0x100a/2] = 0x673c;//beq :x//0x6d00;
		jm_mcu_code[0x100c/2] = 0x4e71;//nop//0x003c;
		jm_mcu_code[0x100e/2] = 0x33c2;     jm_mcu_code[0x1010/2] = 0x0010;     jm_mcu_code[0x1012/2] = 0x17fe; //move.w D2,$1017fe
		jm_mcu_code[0x1014/2] = 0x33c1;     jm_mcu_code[0x1016/2] = 0x0010;     jm_mcu_code[0x1018/2] = 0x17fc; //move.w D1,$1017fc
		jm_mcu_code[0x101a/2] = 0x720f;
		jm_mcu_code[0x101c/2] = 0x740f; //moveq $07,D2
		jm_mcu_code[0x101e/2] = 0x23c8;
		jm_mcu_code[0x1020/2] = 0x0010;
		jm_mcu_code[0x1022/2] = 0x17f0;
		jm_mcu_code[0x1024/2] = 0x2050; //movea (A0),A0
		jm_mcu_code[0x1026/2] = 0x32d8;
		jm_mcu_code[0x1028/2] = 0x51ca;
		jm_mcu_code[0x102a/2] = 0xfffc;
		jm_mcu_code[0x102c/2] = 0x2079;
		jm_mcu_code[0x102e/2] = 0x0010;
		jm_mcu_code[0x1030/2] = 0x17f0;
		jm_mcu_code[0x1032/2] = 0xd0fc;
		jm_mcu_code[0x1034/2] = 0x0004;//adda.w $4,A0
		jm_mcu_code[0x1036/2] = 0x51c9;
		jm_mcu_code[0x1038/2] = 0xffe4;
		jm_mcu_code[0x103a/2] = 0x3439;
		jm_mcu_code[0x103c/2] = 0x0010;
		jm_mcu_code[0x103e/2] = 0x17fe;
		jm_mcu_code[0x1040/2] = 0x3239;
		jm_mcu_code[0x1042/2] = 0x0010;
		jm_mcu_code[0x1044/2] = 0x17fc;
		jm_mcu_code[0x1046/2] = 0x4e75;

		jm_mcu_code[0x1048/2] = 0x23CD;
		jm_mcu_code[0x104a/2] = 0x0010;
		jm_mcu_code[0x104c/2] = 0x17C0;
		jm_mcu_code[0x104e/2] = 0x2A7C;
		jm_mcu_code[0x1050/2] = 0x0010;
		jm_mcu_code[0x1052/2] = 0x17E0;
		jm_mcu_code[0x1054/2] = 0x33C6;
		jm_mcu_code[0x1056/2] = 0x0010;
		jm_mcu_code[0x1058/2] = 0x17B0;
		jm_mcu_code[0x105a/2] = 0x3C39;
		jm_mcu_code[0x105c/2] = 0x000F;
		jm_mcu_code[0x105e/2] = 0x201A;
		jm_mcu_code[0x1060/2] = 0x3A86;
		jm_mcu_code[0x1062/2] = 0x6700;
		jm_mcu_code[0x1064/2] = 0x0074;
		jm_mcu_code[0x1066/2] = 0x33C2;
		jm_mcu_code[0x1068/2] = 0x0010;
		jm_mcu_code[0x106a/2] = 0x17FE;
		jm_mcu_code[0x106c/2] = 0x33C1;
		jm_mcu_code[0x106e/2] = 0x0010;
		jm_mcu_code[0x1070/2] = 0x17FC;
		jm_mcu_code[0x1072/2] = 0x23C8;
		jm_mcu_code[0x1074/2] = 0x0010;
		jm_mcu_code[0x1076/2] = 0x17F0;
		jm_mcu_code[0x1078/2] = 0x23C9;
		jm_mcu_code[0x107a/2] = 0x0010;
		jm_mcu_code[0x107c/2] = 0x17D0;
		jm_mcu_code[0x107e/2] = 0x41F9;
		jm_mcu_code[0x1080/2] = 0x0002;
		jm_mcu_code[0x1082/2] = 0xA2C0;
		jm_mcu_code[0x1084/2] = 0x3c15;
		jm_mcu_code[0x1086/2] = 0xd1fc;
		jm_mcu_code[0x1088/2] = 0x0000;
		jm_mcu_code[0x108a/2] = 0x0040;
		jm_mcu_code[0x108c/2] = 0x5346;
		jm_mcu_code[0x108e/2] = 0x6704;
		jm_mcu_code[0x1090/2] = 0x4e71;
		jm_mcu_code[0x1092/2] = 0x60F2;
		jm_mcu_code[0x1094/2] = 0x23c8;
		jm_mcu_code[0x1096/2] = 0x0010;
		jm_mcu_code[0x1098/2] = 0x17a0;//store here the A0 reg,1017a0
		jm_mcu_code[0x109a/2] = 0xd3fc;
		jm_mcu_code[0x109c/2] = 0x0000;
		jm_mcu_code[0x109e/2] = 0x0200;
		jm_mcu_code[0x10a0/2] = 0x720F;
		jm_mcu_code[0x10a2/2] = 0x740F;
		jm_mcu_code[0x10a4/2] = 0x2050;
		jm_mcu_code[0x10a6/2] = 0x32D8;
		jm_mcu_code[0x10a8/2] = 0x51CA;
		jm_mcu_code[0x10aa/2] = 0xFFFC;
		jm_mcu_code[0x10ac/2] = 0x2079;//1017a0,not 1017f0
		jm_mcu_code[0x10ae/2] = 0x0010;
		jm_mcu_code[0x10b0/2] = 0x17a0;
		jm_mcu_code[0x10b2/2] = 0xD0FC;
		jm_mcu_code[0x10b4/2] = 0x0004;
		jm_mcu_code[0x10b6/2] = 0x23c8;
		jm_mcu_code[0x10b8/2] = 0x0010;
		jm_mcu_code[0x10ba/2] = 0x17a0;
		jm_mcu_code[0x10bc/2] = 0x51C9;
		jm_mcu_code[0x10be/2] = 0xffe4;
		jm_mcu_code[0x10c0/2] = 0x3439;
		jm_mcu_code[0x10c2/2] = 0x0010;
		jm_mcu_code[0x10c4/2] = 0x17FE;
		jm_mcu_code[0x10c6/2] = 0x3239;
		jm_mcu_code[0x10c8/2] = 0x0010;
		jm_mcu_code[0x10ca/2] = 0x17FC;
		jm_mcu_code[0x10cc/2] = 0x2079;
		jm_mcu_code[0x10ce/2] = 0x0010;
		jm_mcu_code[0x10d0/2] = 0x17F0;
		jm_mcu_code[0x10d2/2] = 0x2279;
		jm_mcu_code[0x10d4/2] = 0x0010;
		jm_mcu_code[0x10d6/2] = 0x17D0;
		jm_mcu_code[0x10d8/2] = 0x2A79;
		jm_mcu_code[0x10da/2] = 0x0010;
		jm_mcu_code[0x10dc/2] = 0x17c0;
		jm_mcu_code[0x10de/2] = 0x3C39;
		jm_mcu_code[0x10e0/2] = 0x0010;
		jm_mcu_code[0x10e2/2] = 0x17B0;
		jm_mcu_code[0x10e4/2] = 0x6000;
		jm_mcu_code[0x10e6/2] = 0xFF26;
	}
}

READ16_MEMBER(jalmah_state::daireika_mcu_r)
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
	if (m_respcount >= ARRAY_LENGTH(resp)) m_respcount = 0;

//  logerror("%04x: mcu_r %02x\n",space.device().safe_pc(),res);

	return res;
}

/*
data value is REQ under mjzoomin video test menu.It is related to the MCU?
*/
static const UINT16 dai_mcu_code[0x11] = { 0x33c5, 0x0010, 0x07fe, 0x3a39,0x000f,0x000c,0xda86,0x0245,
											0x003f, 0x33c5, 0x000f, 0x000c,0x3a39,0x0010,0x07fe,0x4e75    };

WRITE16_MEMBER(jalmah_state::daireika_mcu_w)
{
	UINT16 *jm_shared_ram = m_jm_shared_ram;
	UINT16 *jm_mcu_code = m_jm_mcu_code;
	UINT16 i;

	if(ACCESSING_BITS_0_7 && data)
	{
		/*******************************************************
		1st M68k code uploaded by the MCU.
		random number generator (guess)
		*******************************************************/
		jm_shared_ram[0x0140/2] = 0x4ef9;
		jm_shared_ram[0x0142/2] = 0x0010;
		jm_shared_ram[0x0144/2] = 0x0800;

		/*
		33c5 0010 07fe move.w  D5, $1007fe.l
		3A39 000F 000C move.w  $f000c, D5
		da86           add.w   D6,D5
		0245 003f      and.w   $3f,D5
		33C5 000F 000C move.w  D5, $f000c.l
		3a39 0010 07fe move.w  $1007fe, D5
		4e75           rts
		*/
		for(i=0;i<0x11;i++)
			jm_mcu_code[(0x0800/2)+i] = dai_mcu_code[i];

		/*******************************************************
		2nd M68k code uploaded by the MCU.
		*******************************************************/
		jm_shared_ram[0x0020/2] = 0x4ef9;
		jm_shared_ram[0x0022/2] = 0x0010;
		jm_shared_ram[0x0024/2] = 0x2000;//jmp $102000
		jm_mcu_code[0x2000/2] = 0x0040;
		jm_mcu_code[0x2002/2] = 0x0080;//ori $80,D0
		jm_mcu_code[0x2004/2] = 0x33c0;
		jm_mcu_code[0x2006/2] = 0x0008;
		jm_mcu_code[0x2008/2] = 0x0040;
		jm_mcu_code[0x200a/2] = 0x6100;
		jm_mcu_code[0x200c/2] = 0x000c;
		jm_mcu_code[0x200e/2] = 0x33fc;
		jm_mcu_code[0x2010/2] = 0x0010;
		jm_mcu_code[0x2012/2] = 0x0008;
		jm_mcu_code[0x2014/2] = 0x0040;
		jm_mcu_code[0x2016/2] = 0x4e75;
		jm_mcu_code[0x2018/2] = 0x3239;
		jm_mcu_code[0x201a/2] = 0x0008;
		jm_mcu_code[0x201c/2] = 0x0040;
		jm_mcu_code[0x201e/2] = 0x0241;
		jm_mcu_code[0x2020/2] = 0x0001;
		jm_mcu_code[0x2022/2] = 0x66f4;
		jm_mcu_code[0x2024/2] = 0x4e75;
		/*******************************************************
		3rd M68k code uploaded by the MCU.
		see mjzoomin_mcu_w
		*******************************************************/
		jm_shared_ram[0x00c6/2] = 0x6000;
		jm_shared_ram[0x00c8/2] = 0x0008;//bra +$8,needed because we have only two bytes here
									//and we need three...
		jm_shared_ram[0x00d0/2] = 0x4ef9;
		jm_shared_ram[0x00d2/2] = 0x0010;
		jm_shared_ram[0x00d4/2] = 0x0000;//jmp $100000
		//jm_shared_ram[0x00cc/2] = 0x4e75;//rts //needed? we can use jmp instead of jsr...
		jm_mcu_code[0x0000/2] = 0x2050;//movea.l (A0),A0
		jm_mcu_code[0x0002/2] = 0x32d8;//move.w (A0)+,(A1)+
		jm_mcu_code[0x0004/2] = 0x51c9;
		jm_mcu_code[0x0006/2] = 0xfffc;//dbra D1,f00ca
		jm_mcu_code[0x0008/2] = 0x4e75;//rts
		/*******************************************************
		4th M68k code uploaded by the MCU
		They seem video code cleaning functions
		*******************************************************/
		//108800
		jm_shared_ram[0x0100/2] = 0x4ef9;
		jm_shared_ram[0x0102/2] = 0x0010;
		jm_shared_ram[0x0104/2] = 0x8800;

		jm_mcu_code[0x8800/2] = 0x4df9;
		jm_mcu_code[0x8802/2] = 0x0009;
		jm_mcu_code[0x8804/2] = 0x0000; //lea.w #90000,A6
		jm_mcu_code[0x8806/2] = 0x323c;
		jm_mcu_code[0x8808/2] = 0x1fff; //move.w #$1fff,D1
		jm_mcu_code[0x880a/2] = 0x3cbc;
		jm_mcu_code[0x880c/2] = 0x00ff; //move.w #$0000,(A6)
		jm_mcu_code[0x880e/2] = 0xdcfc;
		jm_mcu_code[0x8810/2] = 0x0002; //adda.w #$0002,A6
		jm_mcu_code[0x8812/2] = 0x51c9;
		jm_mcu_code[0x8814/2] = 0xfff6; //dbra D1
		jm_mcu_code[0x8816/2] = 0x4df9;
		jm_mcu_code[0x8818/2] = 0x0000;
		jm_mcu_code[0x881a/2] = 0x0000; //lea.w #0,A6
		jm_mcu_code[0x881c/2] = 0x323c;
		jm_mcu_code[0x881e/2] = 0x0000; //move.w #$0,D1
		jm_mcu_code[0x8820/2] = 0x4e75; //rts

		//108880
		jm_shared_ram[0x0108/2] = 0x4ef9;
		jm_shared_ram[0x010a/2] = 0x0010;
		jm_shared_ram[0x010c/2] = 0x8880;

		jm_mcu_code[0x8880/2] = 0x4df9;
		jm_mcu_code[0x8882/2] = 0x0009;
		jm_mcu_code[0x8884/2] = 0x4000; //lea.w #94000,A6
		jm_mcu_code[0x8886/2] = 0x323c;
		jm_mcu_code[0x8888/2] = 0x1fff; //move.w #$1fff,D1
		jm_mcu_code[0x888a/2] = 0x3cbc;
		jm_mcu_code[0x888c/2] = 0x00ff; //move.w #$0000,(A6)
		jm_mcu_code[0x888e/2] = 0xdcfc;
		jm_mcu_code[0x8890/2] = 0x0002; //adda.w #$0002,A6
		jm_mcu_code[0x8892/2] = 0x51c9;
		jm_mcu_code[0x8894/2] = 0xfff6; //dbra D1
		jm_mcu_code[0x8896/2] = 0x4df9;
		jm_mcu_code[0x8898/2] = 0x0000;
		jm_mcu_code[0x889a/2] = 0x0000; //lea.w #0,A6
		jm_mcu_code[0x889c/2] = 0x323c;
		jm_mcu_code[0x889e/2] = 0x0000; //move.w #$0,D1
		jm_mcu_code[0x88a0/2] = 0x4e75; //rts

		//108900
		jm_shared_ram[0x0110/2] = 0x4ef9;
		jm_shared_ram[0x0112/2] = 0x0010;
		jm_shared_ram[0x0114/2] = 0x8900;

		jm_mcu_code[0x8900/2] = 0x4df9;
		jm_mcu_code[0x8902/2] = 0x0009;
		jm_mcu_code[0x8904/2] = 0x8000; //lea.w #98000,A6
		jm_mcu_code[0x8906/2] = 0x323c;
		jm_mcu_code[0x8908/2] = 0x1fff; //move.w #$1fff,D1
		jm_mcu_code[0x890a/2] = 0x3cbc;
		jm_mcu_code[0x890c/2] = 0xf0ff; //move.w #$f0ff,(A6)
		jm_mcu_code[0x890e/2] = 0xdcfc;
		jm_mcu_code[0x8910/2] = 0x0002; //adda.w #$0002,A6
		jm_mcu_code[0x8912/2] = 0x51c9;
		jm_mcu_code[0x8914/2] = 0xfff6; //dbra D1
		jm_mcu_code[0x8916/2] = 0x4df9;
		jm_mcu_code[0x8918/2] = 0x0000;
		jm_mcu_code[0x891a/2] = 0x0000; //lea.w #0,A6
		jm_mcu_code[0x891c/2] = 0x323c;
		jm_mcu_code[0x891e/2] = 0x0000; //move.w #$0,D1
		jm_mcu_code[0x8920/2] = 0x4e75; //rts

		/*TX function?*/
		jm_shared_ram[0x0126/2] = 0x4ef9;
		jm_shared_ram[0x0128/2] = 0x0010;
		jm_shared_ram[0x012a/2] = 0x8980;

		//m_pri $f0590
		jm_mcu_code[0x8980/2] = 0x33fc;
		jm_mcu_code[0x8982/2] = 0x0006;
		jm_mcu_code[0x8984/2] = 0x000f;
		jm_mcu_code[0x8986/2] = 0x0590; //move.w #$6,$f0590 (m_pri n)
		jm_mcu_code[0x8988/2] = 0x4df9;
		jm_mcu_code[0x898a/2] = 0x0009;
		jm_mcu_code[0x898c/2] = 0xc000; //lea.w #9c000,A6
		jm_mcu_code[0x898e/2] = 0x323c;
		jm_mcu_code[0x8990/2] = 0x1fff; //move.w #$1fff,D1
		jm_mcu_code[0x8992/2] = 0x3cbc;
		jm_mcu_code[0x8994/2] = 0x0020; //move.w #$0020,(A6)
		jm_mcu_code[0x8996/2] = 0xdcfc;
		jm_mcu_code[0x8998/2] = 0x0002; //adda.w #$0002,A6
		jm_mcu_code[0x899a/2] = 0x51c9;
		jm_mcu_code[0x899c/2] = 0xfff6; //dbra D1
		jm_mcu_code[0x899e/2] = 0x4df9;
		jm_mcu_code[0x89a0/2] = 0x0000;
		jm_mcu_code[0x89a2/2] = 0x0000; //lea.w #0,A6
		jm_mcu_code[0x89a4/2] = 0x323c;
		jm_mcu_code[0x89a6/2] = 0x0000; //move.w #$0,D1
		jm_mcu_code[0x89a8/2] = 0x4e75; //rts

/*
        jm_shared_ram[0x0100/2] = 0x4ef9;
        jm_shared_ram[0x0102/2] = 0x0010;
        jm_shared_ram[0x0104/2] = 0x1000;//jmp $101000
        //jm_shared_ram[0x00c6/2] = 0x4e75;//rts
        jm_mcu_code[0x1000/2] = 0x33c2;
        jm_mcu_code[0x1002/2] = 0x0010;
        jm_mcu_code[0x1004/2] = 0x17fe; //move.w D2,$1017fe
        jm_mcu_code[0x1006/2] = 0x23c8;
        jm_mcu_code[0x1008/2] = 0x0010;
        jm_mcu_code[0x100a/2] = 0x17f0;
        jm_mcu_code[0x100c/2] = 0x2050; //movea (A0),A0
        jm_mcu_code[0x100e/2] = 0x22d8;
        jm_mcu_code[0x1010/2] = 0x51ca;
        jm_mcu_code[0x1012/2] = 0xfffc;
        jm_mcu_code[0x1014/2] = 0x3439;
        jm_mcu_code[0x1016/2] = 0x0010;
        jm_mcu_code[0x1018/2] = 0x17fe;
        jm_mcu_code[0x101a/2] = 0x2079;
        jm_mcu_code[0x101c/2] = 0x0010;
        jm_mcu_code[0x101e/2] = 0x17f0;
        jm_mcu_code[0x1020/2] = 0xd0fc;
        jm_mcu_code[0x1022/2] = 0x0004;//adda.w $4,A0
        jm_mcu_code[0x1024/2] = 0x4e75;*/
		/*******************************************************
		5th M68k code uploaded by the MCU (palette upload)
		*******************************************************/
		jm_shared_ram[0x00c0/2] = 0x4ef9;
		jm_shared_ram[0x00c2/2] = 0x0010;
		jm_shared_ram[0x00c4/2] = 0x1000;//jmp $101000
		//jm_shared_ram[0x00c6/2] = 0x4e75;//rts
		jm_mcu_code[0x1000/2] = 0x33c2;
		jm_mcu_code[0x1002/2] = 0x0010;
		jm_mcu_code[0x1004/2] = 0x17fe; //move.w D2,$1017fe
		jm_mcu_code[0x1006/2] = 0x33c1;
		jm_mcu_code[0x1008/2] = 0x0010;
		jm_mcu_code[0x100a/2] = 0x17fc; //move.w D1,$1017fc
		jm_mcu_code[0x100c/2] = 0x720f;
		jm_mcu_code[0x100e/2] = 0x740f; //moveq $0f,D2
		jm_mcu_code[0x1010/2] = 0x23c8;
		jm_mcu_code[0x1012/2] = 0x0010;
		jm_mcu_code[0x1014/2] = 0x17f0;
		jm_mcu_code[0x1016/2] = 0x2050; //movea (A0),A0
		jm_mcu_code[0x1018/2] = 0x32d8;
		jm_mcu_code[0x101a/2] = 0x51ca;
		jm_mcu_code[0x101c/2] = 0xfffc;
		jm_mcu_code[0x101e/2] = 0x2079;
		jm_mcu_code[0x1020/2] = 0x0010;
		jm_mcu_code[0x1022/2] = 0x17f0;
		jm_mcu_code[0x1024/2] = 0xd0fc;
		jm_mcu_code[0x1026/2] = 0x0004;//adda.w $4,A0
		jm_mcu_code[0x1028/2] = 0x51c9;
		jm_mcu_code[0x102a/2] = 0xffe4;
		jm_mcu_code[0x102c/2] = 0x3439;
		jm_mcu_code[0x102e/2] = 0x0010;
		jm_mcu_code[0x1030/2] = 0x17fe;
		jm_mcu_code[0x1032/2] = 0x3239;
		jm_mcu_code[0x1034/2] = 0x0010;
		jm_mcu_code[0x1036/2] = 0x17fc;
		jm_mcu_code[0x1038/2] = 0x4e75;
		/*******************************************************
		6th M68k code uploaded by the MCU (tile upload)
		*******************************************************/
		jm_shared_ram[0x00ca/2] = 0x4ef9;
		jm_shared_ram[0x00cc/2] = 0x0010;
		jm_shared_ram[0x00ce/2] = 0x1800;//jmp $101800
		//jm_shared_ram[0x00c6/2] = 0x4e75;//rts

		jm_mcu_code[0x1800/2] = 0x22da;//move.l (A2)+,(A1)+
		jm_mcu_code[0x1802/2] = 0xb5fc;
		jm_mcu_code[0x1804/2] = 0x0002;
		jm_mcu_code[0x1806/2] = 0x6600;
		jm_mcu_code[0x1808/2] = 0x6706;
		jm_mcu_code[0x180a/2] = 0x51c8;
		jm_mcu_code[0x180c/2] = 0xfff4;//dbra D0,f00ca
		jm_mcu_code[0x180e/2] = 0x4e75;//rts
		jm_mcu_code[0x1810/2] = 0xd4fc;
		jm_mcu_code[0x1812/2] = 0x0a00;
		jm_mcu_code[0x1814/2] = 0x60ea;
	}
}

READ16_MEMBER(jalmah_state::mjzoomin_mcu_r)
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
	if (m_respcount >= ARRAY_LENGTH(resp)) m_respcount = 0;

//  logerror("%04x: mcu_r %02x\n",space.device().safe_pc(),res);

	return res;
}

/*
data value is REQ under mjzoomin video test menu.It is related to the MCU?
*/
WRITE16_MEMBER(jalmah_state::mjzoomin_mcu_w)
{
	UINT16 *jm_shared_ram = m_jm_shared_ram;
	UINT16 *jm_mcu_code = m_jm_mcu_code;
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
		jm_shared_ram[0x00c6/2] = 0x4ef9;
		jm_shared_ram[0x00c8/2] = 0x0010;
		jm_shared_ram[0x00ca/2] = 0x0000;//jsr $100000
		//jm_shared_ram[0x00cc/2] = 0x4e75;//rts //needed? we can use jmp instead of jsr...
		jm_mcu_code[0x0000/2] = 0x2050;//movea.l (A0),A0
		jm_mcu_code[0x0002/2] = 0x32d8;//move.w (A0)+,(A1)+
		jm_mcu_code[0x0004/2] = 0x51c9;
		jm_mcu_code[0x0006/2] = 0xfffc;//dbra D1,f00ca
		jm_mcu_code[0x0008/2] = 0x4e75;//rts
		/*******************************************************
		2nd M68k code uploaded by the MCU (Sound read/write)
		(Note:copied from suchipi,check here the sound banking)
		*******************************************************/
		jm_shared_ram[0x0020/2] = 0x4ef9;
		jm_shared_ram[0x0022/2] = 0x0010;
		jm_shared_ram[0x0024/2] = 0x1800;//jmp $101800

		/*Wrong,it reads from the rom...*/
		//0x33c2 0011 80fe move.w D2,$1180fe
		//0x0642 addi.w $1,D2
		//0x0001
		//0x0242 andi.w $3,D2 ;might be not needed
		//0x0003

		//0x33c2 move.w D2,$80018 ;sound bank
		//0x0008
		//0x0018
		//0x3439 0011 80fe move.w $1180fe,D2
		jm_mcu_code[0x1800/2] = 0x33c2;
		jm_mcu_code[0x1802/2] = 0x0011;
		jm_mcu_code[0x1804/2] = 0x80fe;
		jm_mcu_code[0x1806/2] = 0x0642;
		jm_mcu_code[0x1808/2] = 0x0001;
		jm_mcu_code[0x180a/2] = 0x0242;
		jm_mcu_code[0x180c/2] = 0x0003;
		jm_mcu_code[0x180e/2] = 0x33c2;
		jm_mcu_code[0x1810/2] = 0x0008;
		jm_mcu_code[0x1812/2] = 0x0018;
		jm_mcu_code[0x1814/2] = 0x0040;
		jm_mcu_code[0x1816/2] = 0x0080;//ori $80,D0
		jm_mcu_code[0x1818/2] = 0x33c0;//move.w D0,$80040
		jm_mcu_code[0x181a/2] = 0x0008;
		jm_mcu_code[0x181c/2] = 0x0040;
		jm_mcu_code[0x181e/2] = 0x33fc;//move.w $10,$80040
		jm_mcu_code[0x1820/2] = 0x0010;
		jm_mcu_code[0x1822/2] = 0x0008;
		jm_mcu_code[0x1824/2] = 0x0040;
		jm_mcu_code[0x1826/2] = 0x3439;
		jm_mcu_code[0x1828/2] = 0x0011;
		jm_mcu_code[0x182a/2] = 0x80fe;
		jm_mcu_code[0x182c/2] = 0x4e75;
		//jm_mcu_code[0x1818/2] = 0x3239;
		//jm_mcu_code[0x181a/2] = 0x0008;
		//jm_mcu_code[0x181c/2] = 0x0040;
		//jm_mcu_code[0x181e/2] = 0x0241;
		//jm_mcu_code[0x1820/2] = 0x0001;
		//jm_mcu_code[0x1822/2] = 0x66f4;
		//jm_mcu_code[0x1824/2] = 0x4e75;
		/*******************************************************
		3rd M68k code uploaded by the MCU(palette upload)
		*******************************************************/
		jm_shared_ram[0x00c0/2] = 0x4ef9;
		jm_shared_ram[0x00c2/2] = 0x0010;
		jm_shared_ram[0x00c4/2] = 0x1000;//jmp $101000
		//jm_shared_ram[0x00c6/2] = 0x4e75;//rts
		jm_mcu_code[0x1000/2] = 0x33c2;
		jm_mcu_code[0x1002/2] = 0x0010;
		jm_mcu_code[0x1004/2] = 0x17fe; //move.w D2,$1017fe
		jm_mcu_code[0x1006/2] = 0x33c1;
		jm_mcu_code[0x1008/2] = 0x0010;
		jm_mcu_code[0x100a/2] = 0x17fc; //move.w D1,$1017fc
		jm_mcu_code[0x100c/2] = 0x720f;
		jm_mcu_code[0x100e/2] = 0x740f; //moveq $07,D2
		jm_mcu_code[0x1010/2] = 0x23c8;
		jm_mcu_code[0x1012/2] = 0x0010;
		jm_mcu_code[0x1014/2] = 0x17f0;
		jm_mcu_code[0x1016/2] = 0x2050; //movea (A0),A0
		jm_mcu_code[0x1018/2] = 0x32d8;
		jm_mcu_code[0x101a/2] = 0x51ca;
		jm_mcu_code[0x101c/2] = 0xfffc;
		jm_mcu_code[0x101e/2] = 0x2079;
		jm_mcu_code[0x1020/2] = 0x0010;
		jm_mcu_code[0x1022/2] = 0x17f0;
		jm_mcu_code[0x1024/2] = 0xd0fc;
		jm_mcu_code[0x1026/2] = 0x0004;//adda.w $4,A0
		jm_mcu_code[0x1028/2] = 0x51c9;
		jm_mcu_code[0x102a/2] = 0xffe4;
		jm_mcu_code[0x102c/2] = 0x3439;
		jm_mcu_code[0x102e/2] = 0x0010;
		jm_mcu_code[0x1030/2] = 0x17fe;
		jm_mcu_code[0x1032/2] = 0x3239;
		jm_mcu_code[0x1034/2] = 0x0010;
		jm_mcu_code[0x1036/2] = 0x17fc;
		jm_mcu_code[0x1038/2] = 0x4e75;
	}
}

READ16_MEMBER(jalmah_state::kakumei_mcu_r)
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
	if (m_respcount >= ARRAY_LENGTH(resp)) m_respcount = 0;

//  popmessage("%04x: mcu_r %02x",space.device().safe_pc(),res);

	return res;
}

READ16_MEMBER(jalmah_state::suchipi_mcu_r)
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
	if (m_respcount >= ARRAY_LENGTH(resp)) m_respcount = 0;

//  popmessage("%04x: mcu_r %02x",space.device().safe_pc(),res);

	return res;
}

DRIVER_INIT_MEMBER(jalmah_state,urashima)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16_delegate(FUNC(jalmah_state::urashima_mcu_r), this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x80012, 0x80013, write16_delegate(FUNC(jalmah_state::urashima_mcu_w), this));

	m_mcu_prg = 0x12;
}

DRIVER_INIT_MEMBER(jalmah_state,daireika)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16_delegate(FUNC(jalmah_state::daireika_mcu_r), this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x80012, 0x80013, write16_delegate(FUNC(jalmah_state::daireika_mcu_w), this));

	m_mcu_prg = 0x11;
}

DRIVER_INIT_MEMBER(jalmah_state,mjzoomin)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16_delegate(FUNC(jalmah_state::mjzoomin_mcu_r), this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x80012, 0x80013, write16_delegate(FUNC(jalmah_state::mjzoomin_mcu_w), this));

	m_mcu_prg = 0x13;
}

DRIVER_INIT_MEMBER(jalmah_state,kakumei)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16_delegate(FUNC(jalmah_state::kakumei_mcu_r), this));
	m_mcu_prg = 0x21;
}

DRIVER_INIT_MEMBER(jalmah_state,kakumei2)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16_delegate(FUNC(jalmah_state::kakumei_mcu_r), this));

	m_mcu_prg = 0x22;
}

DRIVER_INIT_MEMBER(jalmah_state,suchipi)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80004, 0x80005, read16_delegate(FUNC(jalmah_state::suchipi_mcu_r), this));

	m_mcu_prg = 0x23;
}

/*First version of the MCU*/
GAME( 1989, urashima, 0, urashima,  urashima, jalmah_state,   urashima, ROT0, "UPL",          "Otogizoushi Urashima Mahjong (Japan)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1989, daireika, 0, jalmah,    daireika, jalmah_state,   daireika, ROT0, "Jaleco / NMK", "Mahjong Daireikai (Japan)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1990, mjzoomin, 0, jalmah,    mjzoomin, jalmah_state,   mjzoomin, ROT0, "Jaleco",       "Mahjong Channel Zoom In (Japan)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
/*Second version of the MCU*/
GAME( 1990, kakumei,  0, jalmah,    kakumei, jalmah_state,    kakumei,  ROT0, "Jaleco",       "Mahjong Kakumei (Japan)",                      MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, kakumei2, 0, jalmah,    kakumei2, jalmah_state,   kakumei2, ROT0, "Jaleco",       "Mahjong Kakumei 2 - Princess League (Japan)",  MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, suchipi,  0, jalmah,    suchipi, jalmah_state,    suchipi,  ROT0, "Jaleco",       "Idol Janshi Suchie-Pai Special (Japan)",       MACHINE_IMPERFECT_GRAPHICS )
