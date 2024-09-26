// license:BSD-3-Clause
// copyright-holders:Luca Elia
/************************************************************************************************************

                                 -= Subsino (Newer) Tilemaps Hardware =-

                                  driver by   Luca Elia (l.elia@tin.it)


Two 1024x512 tilemaps. 256 color tiles. Tiles are 8x8 or a multiple (dynamic tile size).
There is RAM for 512 scroll values (line scroll). Video RAM is mirrored on multiple ranges.
One peculiarity is that video RAM access is split into high and low byte. The former is mapped
in program space, the latter in I/O space.

----------------------------------------------------------------------------------------------------------------
Year  Game                CPU         Sound            Custom                            Other
----------------------------------------------------------------------------------------------------------------
1996  Magic Train         HD647180*   U6295            SS9601, SS9602                    HM86171 RAMDAC, Battery
1996  Water-Nymph         HD647180*   U6295            SS9601, SS9602                    HM86171 RAMDAC, Battery
1998  Express Card        AM188-EM    M6295            SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
1998  Ying Hua Lian       AM188-EM    M6295 + YM3812?  SS9601, SS9602                    HM86171 RAMDAC, Battery
1999  Bishou Jan          H8/3044**   SS9904           SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
1999  X-Train/P-Train     AM188-EM    M6295            SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
2000  New 2001            H8/3044**   SS9904           SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
2001  Queen Bee           H8/3044**   SS9804           SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
2001  Humlan's Lyckohjul  H8/3044**   SS9804           SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
2002  X-Reel              H8/3044**   SS9904           SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
2002  Super Queen Bee     H8/3044**   ?                ?                                 ?
2006  X-Plan              AM188-EM    M6295            SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
----------------------------------------------------------------------------------------------------------------
*   SS9600
**  SS9689 6433044A22F
*** SP006  6433044A65F

All other large QFPs are gate arrays. Die labels below:
- SS9601: (M) NEC, 65650-302
- SS9802/SS9803: (C) (M) 1993 Goldstar, 8406, GVC10032
- SS9904: (C) (M) 1997?, LG SEMICON, GVS693Q5

Graphics for the H8-based games are stored in either four socketed DIP28 8-bit EPROMs, two socketed DIP40 16-bit EPROMs or one
surface-mounted SSOP70 32-bit ROM. Later H8-based PCBs have a custom QFP device labeled "SG 003" instead of the off-the-shelf
RAMDAC.

TODO:
- Add sound to SS9804/SS9904 games.
- ptrain: missing scroll in race screens.
- humlan: empty reels when bonus image should scroll in via L0 scroll. The image (crown/fruits) is at y > 0x100 in the tilemap.
- bishjan, new2001, humlan, saklove, squeenb: game is sometimes too fast (can bishjan read the VBLANK state? saklove and xplan can).
- xtrain: it runs faster than a video from the real thing. It doesn't use vblank irqs (but reads the vblank bit).
- mtrain: implement hopper.
- xplan: starts with 4 credits, no controls to move the aircraft
- which PCBs have the newer SP006 H8 instead of SS9689? is it the ones with a SG 003?

Protection seems to work the same way on every game in this driver, using a bitbanged Dallas 1-Wire EEPROM. First a Read ROM
command is issued, and only the first 8 bits returned are examined to determine whether they match the expected device code (0x14).
If this test passes, the EEPROM contents are recalled and the first 64 bits are read out. These 64 bits are then unscrambled using
a permutation table common to all games. The second and final protection check compares byte 6 in the unscrambled data buffer
against a game-specific ID code. This byte is composed of EEPROM bits 22 (MSB), 27, 52, 50, 42, 9, 38 and 35 (LSB). This EEPROM
comes in a small TO-92 package which is very difficult to spot on PCB photos, though it appears to be typically positioned near
one of the edge connectors.

Timings in the Z180-based and H8-based games consistently fail to meet 1-Wire specifications. In the case of the H8-based games,
this likely has to do with CPU clocks and emulated cycle timings being both too fast. There may also be wait states programmed
by the otherwise seemingly unnecessary internal ROMs.

************************************************************************************************************/

#include "emu.h"
#include "subsino_crypt.h"
#include "subsino_io.h"

#include "cpu/h8/h83048.h"
#include "cpu/i86/i186.h"
#include "cpu/z180/hd647180x.h"
#include "machine/ds2430a.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


enum tilesize_t : uint8_t
{
	TILE_8x8,
	TILE_8x32,
	TILE_64x32
};

ALLOW_SAVE_TYPE(tilesize_t);

enum vram_t
{
	VRAM_LO,
	VRAM_HI
};


// Layers
struct layer_t
{
	std::unique_ptr<uint16_t[]> videoram;

	std::unique_ptr<uint16_t[]> scrollram;
	int scroll_x;
	int scroll_y;

	tilemap_t *tmap;
	tilesize_t tilesize;

};

class subsino2_state : public driver_device
{
public:
	subsino2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_hopper(*this, "hopper")
		, m_ticket(*this, "ticket")
		, m_eeprom(*this, "eeprom")
		, m_keyb(*this, "KEYB_%u", 0U)
		, m_dsw(*this, "DSW%u", 1U)
		, m_leds(*this, "led%u", 0U)
	{ }

	void bishjan(machine_config &config);
	void xiaoao(machine_config &config);
	void saklove(machine_config &config);
	void mtrain(machine_config &config);
	void tbonusal(machine_config &config);
	void humlan(machine_config &config);
	void new2001(machine_config &config);
	void expcard(machine_config &config);
	void xplan(machine_config &config);
	void xtrain(machine_config &config);
	void ptrain(machine_config &config);

	void init_wtrnymph();
	void init_mtrain();
	void init_tbonusal();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void ss9601_byte_lo_w(uint8_t data);
	void ss9601_byte_lo2_w(uint8_t data);
	void ss9601_videoram_0_hi_w(offs_t offset, uint8_t data);
	void ss9601_videoram_0_lo_w(offs_t offset, uint8_t data);
	void ss9601_videoram_0_hi_lo_w(offs_t offset, uint8_t data);
	void ss9601_videoram_0_hi_lo2_w(offs_t offset, uint8_t data);
	uint8_t ss9601_videoram_0_hi_r(offs_t offset);
	uint8_t ss9601_videoram_0_lo_r(offs_t offset);
	void ss9601_videoram_1_hi_w(offs_t offset, uint8_t data);
	void ss9601_videoram_1_lo_w(offs_t offset, uint8_t data);
	void ss9601_videoram_1_hi_lo_w(offs_t offset, uint8_t data);
	void ss9601_videoram_1_hi_lo2_w(offs_t offset, uint8_t data);
	uint8_t ss9601_videoram_1_hi_r(offs_t offset);
	uint8_t ss9601_videoram_1_lo_r(offs_t offset);
	void ss9601_reelram_hi_lo_w(offs_t offset, uint8_t data);
	uint8_t ss9601_reelram_hi_r(offs_t offset);
	uint8_t ss9601_reelram_lo_r(offs_t offset);
	void ss9601_scrollctrl_w(uint8_t data);
	void ss9601_tilesize_w(uint8_t data);
	void ss9601_scroll_w(offs_t offset, uint8_t data);
	void ss9601_scrollram_0_hi_w(offs_t offset, uint8_t data);
	void ss9601_scrollram_0_lo_w(offs_t offset, uint8_t data);
	void ss9601_scrollram_0_hi_lo_w(offs_t offset, uint8_t data);
	uint8_t ss9601_scrollram_0_hi_r(offs_t offset);
	uint8_t ss9601_scrollram_0_lo_r(offs_t offset);
	void ss9601_scrollram_1_hi_w(offs_t offset, uint8_t data);
	void ss9601_scrollram_1_lo_w(offs_t offset, uint8_t data);
	void ss9601_scrollram_1_hi_lo_w(offs_t offset, uint8_t data);
	uint8_t ss9601_scrollram_1_hi_r(offs_t offset);
	uint8_t ss9601_scrollram_1_lo_r(offs_t offset);
	void ss9601_disable_w(uint8_t data);
	void dsw_mask_w(uint8_t data);
	uint8_t dsw_r();
	uint8_t vblank_bit2_r();
	uint8_t vblank_bit6_r();
	uint8_t bishjan_sound_r();
	void bishjan_sound_w(uint8_t data);
	uint8_t bishjan_serial_r();
	uint8_t xiaoao_serial_r();
	uint8_t bishjan_unknown_r();
	void bishjan_input_w(uint8_t data);
	uint8_t bishjan_input_r();
	void bishjan_outputs_w(uint8_t data);
	uint8_t new2001_sound_ready_r();
	void new2001_output0_w(uint8_t data);
	void new2001_output1_w(uint8_t data);
	void humlan_output0_w(uint8_t data);
	void humlan_output1_w(uint8_t data);
	void expcard_out_b_w(uint8_t data);
	void expcard_out_a_w(uint8_t data);
	void mtrain_output0_w(uint8_t data);
	void mtrain_output1_w(uint8_t data);
	void mtrain_output2_w(uint8_t data);
	void mtrain_output3_w(uint8_t data);
	void mtrain_videoram_w(offs_t offset, uint8_t data);
	void mtrain_tilesize_w(uint8_t data);
	void saklove_output0_w(uint8_t data);
	void saklove_output1_w(uint8_t data);
	void saklove_output2_w(uint8_t data);
	void saklove_output3_w(uint8_t data);
	void xplan_out_d_w(uint8_t data);
	void xplan_out_c_w(uint8_t data);
	void xplan_out_b_w(uint8_t data);
	void xplan_out_a_w(uint8_t data);
	void xtrain_out_d_w(uint8_t data);
	void xtrain_out_c_w(uint8_t data);
	void xtrain_out_b_w(uint8_t data);
	void xtrain_out_a_w(uint8_t data);
	void oki_bank_bit0_w(uint8_t data);
	void oki_bank_bit4_w(uint8_t data);

	TILE_GET_INFO_MEMBER(ss9601_get_tile_info_0);
	TILE_GET_INFO_MEMBER(ss9601_get_tile_info_1);
	uint32_t screen_update_subsino2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bishjan_map(address_map &map) ATTR_COLD;
	void mtrain_io(address_map &map) ATTR_COLD;
	void mtrain_base_map(address_map &map) ATTR_COLD;
	void mtrain_map(address_map &map) ATTR_COLD;
	void tbonusal_map(address_map &map) ATTR_COLD;
	void new2001_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void saklove_io(address_map &map) ATTR_COLD;
	void saklove_map(address_map &map) ATTR_COLD;
	void xplan_io(address_map &map) ATTR_COLD;
	void xplan_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override { m_leds.resolve(); }

	layer_t m_layers[2];
	uint8_t m_ss9601_byte_lo;
	uint8_t m_ss9601_byte_lo2;
	std::unique_ptr<uint16_t[]> m_ss9601_reelram;
	bitmap_ind16 m_reelbitmap;
	uint8_t m_ss9601_scrollctrl;
	uint8_t m_ss9601_tilesize;
	uint8_t m_ss9601_disable;
	uint8_t m_dsw_mask;
	uint8_t m_bishjan_sound;
	uint8_t m_bishjan_input;

	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<ticket_dispenser_device> m_hopper;
	optional_device<ticket_dispenser_device> m_ticket;
	required_device<ds2430a_device> m_eeprom;
	optional_ioport_array<5> m_keyb;
	optional_ioport_array<4> m_dsw;
	output_finder<9> m_leds;

	inline void ss9601_get_tile_info(layer_t *l, tile_data &tileinfo, tilemap_memory_index tile_index);
};



/***************************************************************************
                              Tilemaps Access
***************************************************************************/

inline void subsino2_state::ss9601_get_tile_info(layer_t *l, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int addr;
	uint16_t offs;
	switch (l->tilesize)
	{
		default:
		case TILE_8x8:      addr = tile_index;              offs = 0;                                               break;
		case TILE_8x32:     addr = tile_index & (~0x180);   offs = (tile_index/0x80) & 3;                           break;
		case TILE_64x32:    addr = tile_index & (~0x187);   offs = ((tile_index/0x80) & 3) + (tile_index & 7) * 4;  break;
	}
	tileinfo.set(0, l->videoram[addr] + offs, 0, 0);
}

// Layer 0
TILE_GET_INFO_MEMBER(subsino2_state::ss9601_get_tile_info_0)
{
	ss9601_get_tile_info(&m_layers[0], tileinfo, tile_index);
}

// Layer 1
TILE_GET_INFO_MEMBER(subsino2_state::ss9601_get_tile_info_1)
{
	ss9601_get_tile_info(&m_layers[1], tileinfo, tile_index);
}


void subsino2_state::ss9601_byte_lo_w(uint8_t data)
{
	m_ss9601_byte_lo = data;
}
void subsino2_state::ss9601_byte_lo2_w(uint8_t data)
{
	m_ss9601_byte_lo2 = data;
}


static inline void ss9601_videoram_w(layer_t *l, vram_t vram, offs_t offset, uint8_t data)
{
	switch (vram)
	{
	case VRAM_HI:
		l->videoram[offset] = uint8_t(data) << 8 | (l->videoram[offset] & 0xff);
		break;

	case VRAM_LO:
		l->videoram[offset] = data | (l->videoram[offset] & 0xff00);
		break;
	}

	switch (l->tilesize)
	{
	default:
	case TILE_8x8:
		l->tmap->mark_tile_dirty(offset);
		break;

	case TILE_8x32:
		offset &= ~0x180;
		for (int y = 0; y < 0x80*4; y += 0x80)
			l->tmap->mark_tile_dirty(offset + y);
		break;

	case TILE_64x32:
		offset &= ~0x187;
		for (int x = 0; x < 8; x++)
			for (int y = 0; y < 0x80*4; y += 0x80)
				l->tmap->mark_tile_dirty(offset + y + x);
		break;
	}
}

// Layer 0
void subsino2_state::ss9601_videoram_0_hi_w(offs_t offset, uint8_t data)
{
	ss9601_videoram_w(&m_layers[0], VRAM_HI, offset, data);
}

void subsino2_state::ss9601_videoram_0_lo_w(offs_t offset, uint8_t data)
{
	ss9601_videoram_w(&m_layers[0], VRAM_LO, offset, data);
}

void subsino2_state::ss9601_videoram_0_hi_lo_w(offs_t offset, uint8_t data)
{
	ss9601_videoram_w(&m_layers[0], VRAM_HI, offset, data);
	ss9601_videoram_w(&m_layers[0], VRAM_LO, offset, m_ss9601_byte_lo);
}

void subsino2_state::ss9601_videoram_0_hi_lo2_w(offs_t offset, uint8_t data)
{
	ss9601_videoram_w(&m_layers[0], VRAM_HI, offset, data);
	ss9601_videoram_w(&m_layers[0], VRAM_LO, offset, m_ss9601_byte_lo2);
}

uint8_t subsino2_state::ss9601_videoram_0_hi_r(offs_t offset)
{
	return m_layers[0].videoram[offset] >> 8;
}

uint8_t subsino2_state::ss9601_videoram_0_lo_r(offs_t offset)
{
	return m_layers[0].videoram[offset] & 0xff;
}

// Layer 1
void subsino2_state::ss9601_videoram_1_hi_w(offs_t offset, uint8_t data)
{
	ss9601_videoram_w(&m_layers[1], VRAM_HI, offset, data);
}

void subsino2_state::ss9601_videoram_1_lo_w(offs_t offset, uint8_t data)
{
	ss9601_videoram_w(&m_layers[1], VRAM_LO, offset, data);
}

void subsino2_state::ss9601_videoram_1_hi_lo_w(offs_t offset, uint8_t data)
{
	ss9601_videoram_w(&m_layers[1], VRAM_HI, offset, data);
	ss9601_videoram_w(&m_layers[1], VRAM_LO, offset, m_ss9601_byte_lo);
}

void subsino2_state::ss9601_videoram_1_hi_lo2_w(offs_t offset, uint8_t data)
{
	ss9601_videoram_w(&m_layers[1], VRAM_HI, offset, data);
	ss9601_videoram_w(&m_layers[1], VRAM_LO, offset, m_ss9601_byte_lo2);
}

uint8_t subsino2_state::ss9601_videoram_1_hi_r(offs_t offset)
{
	return m_layers[1].videoram[offset] >> 8;
}

uint8_t subsino2_state::ss9601_videoram_1_lo_r(offs_t offset)
{
	return m_layers[1].videoram[offset] & 0xff;
}

// Layer 0 Reels

void subsino2_state::ss9601_reelram_hi_lo_w(offs_t offset, uint8_t data)
{
	m_ss9601_reelram[offset] = uint16_t(data) << 8 | m_ss9601_byte_lo;
}
uint8_t subsino2_state::ss9601_reelram_hi_r(offs_t offset)
{
	return m_ss9601_reelram[offset] >> 8;
}
uint8_t subsino2_state::ss9601_reelram_lo_r(offs_t offset)
{
	return m_ss9601_reelram[offset] & 0xff;
}


/***************************************************************************
                              Tilemaps Tile Size


mtrain:

80 = 00     40 = -      L0 = 8x8    L1 = 8x8    ; title screen
80 = 01     40 = -      L0 = REEL   L1 = 8x8    ; gameplay (center reel)
80 = 01     40 = -      L0 = REEL   L1 = 8x8    ; girl slot (top, bottom reels)

xtrain:

80 = 00     40 = FD     L0 = -      L1 = 8x8    ; ram test (disable = 01 -> L0 disabled)
80 = 40     40 = FD     L0 = REEL   L1 = 8x8    ; game play (center reel) as well as "BONUS" screen (top reel)
80 = 40     40 = FD     L0 = REEL   L1 = 8x8    ; lose/treasure slot (center reel)
80 = 00     40 = BF     L0 = 8x8    L1 = 8x8    ; car screen [L0 line scroll every 32]
80 = 00     40 = EF     L0 = 8x8    L1 = 8x8    ; title screen, L0 normal scroll (scrollram is 0) [L1 line scroll every 8]
80 = 40     40 = 00     L0 = 8x16?  L1 = 8x8    ; girl dancing
80 = 00     40 = FF     L0 = 8x8    L1 = 8x8    ; soft dsw screen

xplan:

80 = 00     40 = FD     L0 = -      L1 = 8x8    ; ram test (disable = 01 -> L0 disabled)
80 = 40     40 = BF     L0 = 8x32   L1 = 8x8    ; title screen [L0 line scroll every 32, L1 line scroll disabled?] / 3 planes with scrolling clouds (before title screen)
80 = 00     40 = EF     L0 = 8x8    L1 = 8x8    ; parachutist and cars demo [L1 line scroll every 8]
80 = 70     40 = FF     L0 = 64x32  L1 = 8x8    ; shoot'em up demo / gambling demo
80 = 00     40 = FF     L0 = 8x8    L1 = 8x8    ; test mode and stat screens

bishjan:

80 = 00     40 = FD     L0 = -      L1 = 8x8    ; ram test (disable = 01 -> L0 disabled)
80 = 00     40 = 0F     L0 = 8x8    L1 = 8x8    ; soft dsw screen
80 = 00     40 = 0x     L0 = 8x8    L1 = 8x8    ; stat screens and gameplay (40 = 07/0d, seems a don't care)

saklove:

80 = 00     40 = 0D     L0 = -      L1 = 8x8    ; ram test (disable = 01 -> L0 disabled)
80 = 00     40 = 0D     L0 = 8x8    L1 = 8x8    ; gameplay / square of tiles screen
80 = 00     40 = 0F     L0 = 8x8    L1 = 8x8    ; title screen / instructions / double up screen
80 = 00     40 = 07     L0 = 8x8    L1 = 8x8    ; pool [L0 line scroll]

-----

More registers, at boot (never changed AFAICT):

mtrain,     saklove     xtrain,      expcard
wtrnymph                xplan

                         C0 = 0A     C0 = 0A
                         E0 = 18     E0 = 18

9100 = 13   200 = 13    200 = 13    200 = 13
9101 = 20   201 = 20    201 = 20    201 = 20
9102 = 20   202 = 20    202 = 20    202 = 20
9103 = 33   203 = 33    203 = 33    203 = 33
9104 = DC   204 = DC    204 = DC    204 = DC
9105 = A0   205 = A0    205 = A0    205 = A0
9106 = FA   206 = FA    206 = FA    206 = FA
9107 = 00   207 = 00    207 = 00    207 = 00
9108 = 33   208 = 33    208 = 33    208 = 33
9109 = 16   209 = 0E    209 = 0E    209 = 0E
910A = F6   20A = FD    20A = FD    20A = FD    *F5 in wtrnymph
910B = 22   20B = 22    20B = 22    20B = 22
910C = FE   20C = FE    20C = FE    20C = FE
910D = FB   20D = FB    20D = FB    20D = FB
910E = F8   20E = F8    20E = F8    20E = F8
910F = 02   20F = 02    20F = 02    20F = 02
9110 = 13   210 = 13    210 = 13    210 = 13
9111 = 10   211 = 00    211 = 00    211 = 00
9112 = 62   212 = 82    212 = 82    212 = 82
9113 = 13   213 = 33    213 = 33    213 = 33
9114 = 80   214 = 0E    214 = 0E    214 = 0E
9115 = 80   215 = FD    215 = FD    215 = FD
9116 = 13   216 = 13    216 = 13    216 = 13
9117 = 42   217 = 42    217 = 42    217 = 42
9118 = 42   218 = 42    218 = 42    218 = 42
9119 = 13   219 = 13    219 = 13    219 = 13
911A = 20   21A = 20    21A = 20    21A = 20
911B = 42   21B = 62    21B = 62    21B = 62
9126 = 0C   226 = 08    226 = 08    226 = 08
9127 = 0C   227 = 0C    227 = 0C    227 = 0C
9130 = 00   230 = 00    230 = 00    230 = 00
9131 = 00   231 = 00    231 = 00    231 = 00
9132 = 00   232 = FB    232 = FB    232 = FB
9133 = 00   233 = FF    233 = FF    233 = FF
9134 = 00   234 = 22    234 = 20    234 = 20
9135 = 00   235 = 02
9136 = 00
9137 = 00   237 = 00    237 = 00    237 = 00

9149 = FF   312 = FF    309 = FF    312 = FF
914A = FF   311 = FF    30A = FF    311 = FF
914B = FF   310 = FF    30B = FF    310 = FF
914C = 47   30F = 47    30C = 47    30F = 47
914D = 00   30E = 00    30D = 00    30E = 00
914E = 00   30D = 00    30E = 00    30D = 00
914F = 00   30C = 00    30F = 00    30C = 00
9150 = C0   30B = C0    310 = C0    30B = C0
9151 = FF   30A = FF    311 = FF    30A = FF
9153 = 00               313 = 00

***************************************************************************/

// These are written in sequence

void subsino2_state::ss9601_scrollctrl_w(uint8_t data)
{
	m_ss9601_scrollctrl = data;
}

void subsino2_state::ss9601_tilesize_w(uint8_t data)
{
	m_ss9601_tilesize = data;

	tilesize_t sizes[2];
	switch ((data&0xf0)>>4)
	{
		case 0x0:
			sizes[0] = TILE_8x8;
			break;

		case 0x4:
			sizes[0] = TILE_8x32;
			break;

		case 0x7:
			sizes[0] = TILE_64x32;
			break;

		default:
			sizes[0] = TILE_8x8;

			logerror("%s: warning, layer 0 unknown tilesize = %02x\n", machine().describe_context(), data);
			popmessage("layer 0 UNKNOWN TILESIZE %02X", data);
			break;
	}

	switch (data&0x0f)
	{
		case 0x0:
			sizes[1] = TILE_8x8;
			break;

		case 0x4:
			sizes[1] = TILE_8x32;
			break;

		case 0x7:
			sizes[1] = TILE_64x32;
			break;

		default:
			sizes[1] = TILE_8x8;

			logerror("%s: warning, layer 1 unknown tilesize = %02x\n", machine().describe_context(), data);
			popmessage("layer 1 UNKNOWN TILESIZE %02X", data);
			break;
	}


	for (int i = 0; i < 2; i++)
	{
		layer_t *l = &m_layers[i];

		if (l->tilesize != sizes[i])
		{
			l->tilesize = sizes[i];
			l->tmap->mark_all_dirty();
		}
	}
}

/***************************************************************************
                              Tilemaps Scroll
***************************************************************************/

void subsino2_state::ss9601_scroll_w(offs_t offset, uint8_t data)
{
	layer_t *layers = m_layers;
	switch ( offset )
	{
		// Layer 0
		case 0: layers[0].scroll_x = (layers[0].scroll_x & 0xf00) | data;                   break;  // x low
		case 1: layers[0].scroll_y = (layers[0].scroll_y & 0xf00) | data;                   break;  // y low
		case 2: layers[0].scroll_x = (layers[0].scroll_x & 0x0ff) | ((data & 0x0f) << 8);           // y|x high bits
				layers[0].scroll_y = (layers[0].scroll_y & 0x0ff) | ((data & 0xf0) << 4);   break;

		// Layer 1
		case 3: layers[1].scroll_x = (layers[1].scroll_x & 0xf00) | data;                   break;  // x low
		case 4: layers[1].scroll_y = (layers[1].scroll_y & 0xf00) | data;                   break;  // y low
		case 5: layers[1].scroll_x = (layers[1].scroll_x & 0x0ff) | ((data & 0x0f) << 8);           // y|x high bits
				layers[1].scroll_y = (layers[1].scroll_y & 0x0ff) | ((data & 0xf0) << 4);   break;
	}
}

// Layer 0
void subsino2_state::ss9601_scrollram_0_hi_w(offs_t offset, uint8_t data)
{
	m_layers[0].scrollram[offset] = uint16_t(data) << 8 | (m_layers[0].scrollram[offset] & 0xff);
}

void subsino2_state::ss9601_scrollram_0_lo_w(offs_t offset, uint8_t data)
{
	m_layers[0].scrollram[offset] = data | (m_layers[0].scrollram[offset] & 0xff00);
}

void subsino2_state::ss9601_scrollram_0_hi_lo_w(offs_t offset, uint8_t data)
{
	m_layers[0].scrollram[offset] = uint16_t(data) << 8 | m_ss9601_byte_lo;
}

uint8_t subsino2_state::ss9601_scrollram_0_hi_r(offs_t offset)
{
	return m_layers[0].scrollram[offset] >> 8;
}

uint8_t subsino2_state::ss9601_scrollram_0_lo_r(offs_t offset)
{
	return m_layers[0].scrollram[offset] & 0xff;
}

// Layer 1
void subsino2_state::ss9601_scrollram_1_hi_w(offs_t offset, uint8_t data)
{
	m_layers[1].scrollram[offset] = uint16_t(data) << 8 | (m_layers[1].scrollram[offset] & 0xff);
}

void subsino2_state::ss9601_scrollram_1_lo_w(offs_t offset, uint8_t data)
{
	m_layers[1].scrollram[offset] = data | (m_layers[1].scrollram[offset] & 0xff00);
}

void subsino2_state::ss9601_scrollram_1_hi_lo_w(offs_t offset, uint8_t data)
{
	m_layers[1].scrollram[offset] = uint16_t(data) << 8 | m_ss9601_byte_lo;
}

uint8_t subsino2_state::ss9601_scrollram_1_hi_r(offs_t offset)
{
	return m_layers[1].scrollram[offset] >> 8;
}

uint8_t subsino2_state::ss9601_scrollram_1_lo_r(offs_t offset)
{
	return m_layers[1].scrollram[offset] & 0xff;
}


/***************************************************************************
                              Tilemaps Disable
***************************************************************************/

void subsino2_state::ss9601_disable_w(uint8_t data)
{
	m_ss9601_disable = data;
}


/***************************************************************************
                                Video Update
***************************************************************************/

void subsino2_state::video_start()
{
	// SS9601 Regs:

	m_ss9601_tilesize       =   TILE_8x8;
	m_ss9601_scrollctrl     =   0xfd;   // not written by mtrain, default to reels on
	m_ss9601_disable        =   0x00;

	save_item(NAME(m_ss9601_byte_lo));
	save_item(NAME(m_ss9601_byte_lo2));
	save_item(NAME(m_ss9601_tilesize));
	save_item(NAME(m_ss9601_scrollctrl));
	save_item(NAME(m_ss9601_disable));

	// SS9601 Layers:

	for (int i = 0; i < 2; i++)
	{
		layer_t *l = &m_layers[i];

		l->tmap = &machine().tilemap().create(*m_gfxdecode, i ?
												tilemap_get_info_delegate(*this, FUNC(subsino2_state::ss9601_get_tile_info_1)) :
												tilemap_get_info_delegate(*this, FUNC(subsino2_state::ss9601_get_tile_info_0)),
												TILEMAP_SCAN_ROWS, 8,8, 0x80,0x40);

		l->tmap->set_transparent_pen(0);

		// line scroll
		l->tmap->set_scroll_rows(0x200);

		l->videoram = std::make_unique<uint16_t[]>(0x80 * 0x40);

		l->scrollram = make_unique_clear<uint16_t[]>(0x200);

		save_pointer(NAME(l->videoram), 0x80 * 0x40, i);
		save_pointer(NAME(l->scrollram), 0x200, i);

		save_item(NAME(l->scroll_x), i);
		save_item(NAME(l->scroll_y), i);
		save_item(NAME(l->tilesize), i);
	}

	// SS9601 Reels:

	m_ss9601_reelram = make_unique_clear<uint16_t[]>(0x2000);

	m_reelbitmap.allocate(0x80*8, 0x40*8);

	save_pointer(NAME(m_ss9601_reelram), 0x2000);

	save_item(NAME(m_dsw_mask));
	save_item(NAME(m_bishjan_sound));
	save_item(NAME(m_bishjan_input));
}

uint32_t subsino2_state::screen_update_subsino2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = ~m_ss9601_disable;
	int y;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	// Line Scroll / Reel Control
	int mask_y[2] = {0, 0};
	bool l0_reel = false;
	switch ( m_ss9601_scrollctrl )
	{
		case 0xbf:
			mask_y[0] = ~(32-1);
			break;
		case 0xef:
			mask_y[1] = ~(8-1);
			break;
		case 0x07:
			mask_y[0] = ~(8-1);
			break;
//      case 0x7f:  // ptrain
//          break;
		case 0xfd:
			l0_reel = true;
			break;
	}

	// Scroll
	for (int i = 0; i < 2; i++)
	{
		layer_t *l = &m_layers[i];

		l->tmap->set_scroll_cols(1);
		l->tmap->set_scroll_rows(0x200);
		l->tmap->set_scrolly(0, l->scroll_y + 1);

		// line scroll

		uint16_t scroll_dx = 0;
		for (y = 0; y < 0x200; y++)
		{
			if (mask_y[i])
				scroll_dx = l->scrollram[y & mask_y[i]];

			l->tmap->set_scrollx(y, l->scroll_x + scroll_dx);
		}
	}

	bitmap.fill(m_palette->black_pen(), cliprect);

	if (layers_ctrl & 1)
	{
		layer_t *l = &m_layers[0];

		if (l0_reel)
		{
			l->tmap->set_scroll_rows(1);
			l->tmap->set_scroll_cols(1);

			for (int y = 0; y < 0x20/4; y++)
			{
				for (int x = 0; x < 0x80; x++)
				{
					rectangle visible;
					visible.min_x = 8 * x;
					visible.max_x = 8 * (x+1) - 1;
					visible.min_y = 4 * 0x10 * y;
					visible.max_y = 4 * 0x10 * (y+1) - 1;

					int reeladdr = y * 0x80 * 4 + x;
					uint16_t reelscroll = m_ss9601_reelram[reeladdr];

					l->tmap->set_scrollx(0, (reelscroll >> 9) * 8 - visible.min_x);

					// wrap around at half tilemap (0x100)
					int reelscroll_y = (reelscroll & 0x100) + ((reelscroll - visible.min_y) & 0xff);
					int reelwrap_y = 0x100 - (reelscroll_y & 0xff);

					//visible &= cliprect;
					rectangle tmp = visible;

					// draw above the wrap around y
					if ( reelwrap_y-1 >= visible.min_y )
					{
						if ( reelwrap_y-1 <= visible.max_y )
							tmp.max_y = reelwrap_y-1;
						l->tmap->set_scrolly(0, reelscroll_y);
						l->tmap->draw(screen, m_reelbitmap, tmp, TILEMAP_DRAW_OPAQUE);
						tmp.max_y = visible.max_y;
					}

					// draw below the wrap around y
					if ( reelwrap_y <= visible.max_y )
					{
						if ( reelwrap_y >= visible.min_y )
							tmp.min_y = reelwrap_y;
						l->tmap->set_scrolly(0, -((reelwrap_y &0xff) | (reelscroll_y & 0x100)));
						l->tmap->draw(screen, m_reelbitmap, tmp, TILEMAP_DRAW_OPAQUE);
						tmp.min_y = visible.min_y;
					}
				}
			}

			int32_t sx = -l->scroll_x;
			int32_t sy = -(l->scroll_y + 1);
			copyscrollbitmap(bitmap, m_reelbitmap, 1, &sx, 1, &sy, cliprect);
		}
		else
		{
			l->tmap->draw(screen, bitmap, cliprect, 0, 0);
		}
	}

	if (layers_ctrl & 2)    m_layers[1].tmap->draw(screen, bitmap, cliprect, 0, 0);

//  popmessage("scrl: %03x,%03x - %03x,%03x dis: %02x siz: %02x ctrl: %02x", m_layers[0].scroll_x,m_layers[0].scroll_y, m_layers[1].scroll_x,m_layers[1].scroll_y, m_ss9601_disable, m_ss9601_tilesize, m_ss9601_scrollctrl);

	return 0;
}

/***************************************************************************
                                Input / Output
***************************************************************************/

void subsino2_state::dsw_mask_w(uint8_t data)
{
	m_dsw_mask = data;
}

uint8_t subsino2_state::dsw_r()
{
	return  ( (m_dsw[0]->read() & m_dsw_mask) ? 0x01 : 0 ) |
			( (m_dsw[1]->read() & m_dsw_mask) ? 0x02 : 0 ) |
			( (m_dsw[2]->read() & m_dsw_mask) ? 0x04 : 0 ) |
			( (m_dsw[3]->read() & m_dsw_mask) ? 0x08 : 0 ) ;
}


uint8_t subsino2_state::vblank_bit2_r()
{
	return m_screen->vblank() ? 0x04 : 0x00;
}
uint8_t subsino2_state::vblank_bit6_r()
{
	return m_screen->vblank() ? 0x40 : 0x00;
}

void subsino2_state::oki_bank_bit0_w(uint8_t data)
{
	// it writes 0x32 or 0x33
	m_oki->set_rom_bank(data & 1);
}

void subsino2_state::oki_bank_bit4_w(uint8_t data)
{
	// it writes 0x23 or 0x33
	m_oki->set_rom_bank((data >> 4) & 1);
}


/***************************************************************************
                                Memory Maps
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/

uint8_t subsino2_state::bishjan_sound_r()
{
	return 0;
}

void subsino2_state::bishjan_sound_w(uint8_t data)
{
	/*
	    sound writes in service mode:
	    01 88 04 00 (coin in)
	    02 89 04 0v (v = voice = 0..3)
	*/
	switch (data)
	{
	case 0x10:
		m_eeprom->data_w(1);
		break;

	case 0x13:
		m_eeprom->data_w(0);
		break;
	}
	m_bishjan_sound = data;
}

uint8_t subsino2_state::bishjan_serial_r()
{
	return
		(m_eeprom->data_r() ? 0x80 : 0) |             // bit 7 - serial communication
		(machine().rand() & 0x18) |
		((m_bishjan_sound == 0x12) ? 0x40:0x00);     // bit 6 - sound communication
}

uint8_t subsino2_state::xiaoao_serial_r()
{
	return
		(m_eeprom->data_r() ? 0x80 : 0) |             // bit 7 - serial communication
		(machine().rand() & 0x18) |
		((m_bishjan_sound == 0x11) ? 0x40:0x00);     // bit 6 - sound communication
}

uint8_t subsino2_state::bishjan_unknown_r()
{
	return
//      (machine().rand() & 0xff);
//      (((m_screen->frame_number()%60)==0)?0x18:0x00);
		0x18;
}

void subsino2_state::bishjan_input_w(uint8_t data)
{
	m_bishjan_input = data;
}

uint8_t subsino2_state::bishjan_input_r()
{
	uint8_t res = 0xff;

	for (int i = 0; i < 5; i++)
		if (m_bishjan_input & (1 << i))
			res = m_keyb[i]->read();

	return res;
}

void subsino2_state::bishjan_outputs_w(uint8_t data)
{
	// coin out         BIT(data, 0)
	m_hopper->motor_w(BIT(data, 1));   // hopper
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
}


void subsino2_state::bishjan_map(address_map &map)
{
	map.global_mask(0xffffff);

	map(0x000000, 0x007fff).rom();
	map(0x080000, 0x0fffff).rom();

	map(0x200000, 0x207fff).ram().share("nvram"); // battery

	// read lo (L1)   (only half tilemap?)
	map(0x412000, 0x412fff).r(FUNC(subsino2_state::ss9601_videoram_1_lo_r));
	map(0x413000, 0x4131ff).rw(FUNC(subsino2_state::ss9601_scrollram_1_lo_r), FUNC(subsino2_state::ss9601_scrollram_1_lo_w));
	// read lo (REEL)
	map(0x416000, 0x416fff).r(FUNC(subsino2_state::ss9601_reelram_lo_r));
	map(0x417000, 0x4171ff).rw(FUNC(subsino2_state::ss9601_scrollram_0_lo_r), FUNC(subsino2_state::ss9601_scrollram_0_lo_w));

	// read hi (L1)
	map(0x422000, 0x422fff).r(FUNC(subsino2_state::ss9601_videoram_1_hi_r));
	map(0x423000, 0x4231ff).rw(FUNC(subsino2_state::ss9601_scrollram_1_hi_r), FUNC(subsino2_state::ss9601_scrollram_1_hi_w));
	// read hi (REEL)
	map(0x426000, 0x426fff).r(FUNC(subsino2_state::ss9601_reelram_hi_r));
	map(0x427000, 0x4271ff).rw(FUNC(subsino2_state::ss9601_scrollram_0_hi_r), FUNC(subsino2_state::ss9601_scrollram_0_hi_w));

	// write both (L1)
	map(0x430000, 0x431fff).w(FUNC(subsino2_state::ss9601_videoram_1_hi_lo_w));
	map(0x432000, 0x432fff).w(FUNC(subsino2_state::ss9601_videoram_1_hi_lo_w));
	map(0x433000, 0x4331ff).w(FUNC(subsino2_state::ss9601_scrollram_1_hi_lo_w));
	// write both (L0 & REEL)
	map(0x434000, 0x435fff).w(FUNC(subsino2_state::ss9601_videoram_0_hi_lo_w));
	map(0x436000, 0x436fff).w(FUNC(subsino2_state::ss9601_reelram_hi_lo_w));
	map(0x437000, 0x4371ff).w(FUNC(subsino2_state::ss9601_scrollram_0_hi_lo_w));

	map(0x600000, 0x600000).rw(FUNC(subsino2_state::bishjan_sound_r), FUNC(subsino2_state::bishjan_sound_w));
	map(0x600040, 0x600040).w(FUNC(subsino2_state::ss9601_scrollctrl_w));
	map(0x600060, 0x600060).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x600061, 0x600061).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x600062, 0x600062).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x600080, 0x600080).w(FUNC(subsino2_state::ss9601_tilesize_w));
	map(0x6000a0, 0x6000a0).w(FUNC(subsino2_state::ss9601_byte_lo_w));

	map(0xa0001f, 0xa0001f).w(FUNC(subsino2_state::ss9601_disable_w));
	map(0xa00020, 0xa00025).w(FUNC(subsino2_state::ss9601_scroll_w));

	map(0xc00000, 0xc0001f).rw("io", FUNC(ss9802_device::read), FUNC(ss9802_device::write));
}

void subsino2_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

/***************************************************************************
                                  New 2001
***************************************************************************/

uint8_t subsino2_state::new2001_sound_ready_r()
{
	return 0x20;
}

void subsino2_state::new2001_output0_w(uint8_t data)
{
	m_leds[0] = BIT(data, 6); // record?
	m_leds[1] = BIT(data, 5); // shoot now
	m_leds[2] = BIT(data, 4); // double
	m_leds[3] = BIT(data, 3); // black/red
}

void subsino2_state::new2001_output1_w(uint8_t data)
{
	m_leds[4] = BIT(data, 7); // start
	m_leds[5] = BIT(data, 6); // take
	m_leds[6] = BIT(data, 5); // black/red

	machine().bookkeeping().coin_counter_w(0, data & 0x0010); // coin in / key in
	m_leds[7] = BIT(data, 2); // ?
	m_leds[8] = BIT(data, 1); // ?
}

// Same as bishjan (except for i/o and lo2 usage like xplan)
void subsino2_state::new2001_map(address_map &map)
{
	bishjan_map(map);

	// write both (L1, byte_lo2)
	map(0x410000, 0x411fff).w(FUNC(subsino2_state::ss9601_videoram_1_hi_lo2_w));
	// write both (L0 & REEL, byte_lo2)
	map(0x414000, 0x415fff).w(FUNC(subsino2_state::ss9601_videoram_0_hi_lo2_w));

	map(0x600020, 0x600020).w(FUNC(subsino2_state::ss9601_byte_lo2_w));
}

/***************************************************************************
                             Humlan's Lyckohjul
***************************************************************************/

void subsino2_state::humlan_output0_w(uint8_t data)
{
	m_leds[5] = BIT(data, 5); // big or small
	m_leds[4] = BIT(data, 2); // double
	m_leds[3] = BIT(data, 1); // big or small
	m_leds[2] = BIT(data, 0); // bet
}

void subsino2_state::humlan_output1_w(uint8_t data)
{
	m_leds[1] = BIT(data, 7); // take
	m_leds[0] = BIT(data, 6); // start
	machine().bookkeeping().coin_counter_w(1, data & 0x04); // key in
	machine().bookkeeping().coin_counter_w(0, data & 0x02); // coin in
}

/***************************************************************************
                       Express Card / Top Card
***************************************************************************/

void subsino2_state::expcard_out_b_w(uint8_t data)
{
	m_leds[1] = BIT(data, 2);   // hold 4 / small & hold 5 / big ?
	m_leds[2] = BIT(data, 3);   // hold 1 / bet
	m_leds[3] = BIT(data, 4);   // hold 2 / take ?
	m_leds[4] = BIT(data, 5);   // hold 3 / double up ?
}

void subsino2_state::expcard_out_a_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0,    data & 0x01 );  // coin in
	machine().bookkeeping().coin_counter_w(1,    data & 0x02 );  // key in

	m_leds[5] = BIT(data, 4);   // start
}

/***************************************************************************
                                Magic Train
***************************************************************************/

void subsino2_state::mtrain_output0_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0,    data & 0x01 );  // key in
	machine().bookkeeping().coin_counter_w(1,    data & 0x02 );  // coin in
	machine().bookkeeping().coin_counter_w(2,    data & 0x10 );  // pay out
//  machine().bookkeeping().coin_counter_w(3,   data & 0x20 );  // hopper motor
}

void subsino2_state::mtrain_output1_w(uint8_t data)
{
	m_leds[0] = BIT(data, 0);   // stop reel?
	m_leds[1] = BIT(data, 1);   // stop reel? (double or take)
	m_leds[2] = BIT(data, 2);   // start all
	m_leds[3] = BIT(data, 3);   // bet / stop all
	m_leds[4] = BIT(data, 5);   // stop reel? (double or take)
}

void subsino2_state::mtrain_output2_w(uint8_t data)
{
}

void subsino2_state::mtrain_output3_w(uint8_t data)
{
	m_eeprom->data_w(!BIT(data, 6));
}

void subsino2_state::mtrain_videoram_w(offs_t offset, uint8_t data)
{
	vram_t vram = (m_ss9601_byte_lo & 0x08) ? VRAM_HI : VRAM_LO;
	switch (m_ss9601_byte_lo & (~0x08))
	{
	case 0x00:
		ss9601_videoram_w(&m_layers[1], vram, offset,        data);
		ss9601_videoram_w(&m_layers[1], vram, offset+0x1000, data);
		break;

	case 0x04:
		ss9601_videoram_w(&m_layers[0], vram, offset,        data);
		ss9601_videoram_w(&m_layers[0], vram, offset+0x1000, data);
		break;

	case 0x06:
		if (vram == VRAM_HI)
			m_ss9601_reelram[offset] = uint16_t(data) << 8 | (m_ss9601_reelram[offset] & 0xff);
		else
			m_ss9601_reelram[offset] = uint16_t(data) | (m_ss9601_reelram[offset] & 0xff00);
		break;
	}
}

void subsino2_state::mtrain_tilesize_w(uint8_t data)
{
	m_ss9601_tilesize = data;

	tilesize_t sizes[2];
	switch (data)
	{
		case 0x00:
			sizes[0] = TILE_8x8;
			sizes[1] = TILE_8x8;
			break;

		case 0x01:
			sizes[0] = TILE_8x32;
			sizes[1] = TILE_8x8;
			break;

		default:
			sizes[0] = TILE_8x8;
			sizes[1] = TILE_8x8;

			logerror("%s: warning, unknown tilesize = %02x\n", machine().describe_context(), data);
			popmessage("UNKNOWN TILESIZE %02X", data);
			break;
	}

	for (int i = 0; i < 2; i++)
	{
		layer_t *l = &m_layers[i];

		if (l->tilesize != sizes[i])
		{
			l->tilesize = sizes[i];
			l->tmap->mark_all_dirty();
		}
	}
}

void subsino2_state::mtrain_base_map(address_map &map)
{
	map(0x06000, 0x0d7ff).rom().region("program", 0x8100);

	map(0x0d800, 0x0dfff).ram().share("nvram");   // battery

	map(0x0e000, 0x0efff).w(FUNC(subsino2_state::mtrain_videoram_w));

	map(0x0f11f, 0x0f11f).w(FUNC(subsino2_state::ss9601_disable_w));
	map(0x0f120, 0x0f125).w(FUNC(subsino2_state::ss9601_scroll_w));

	map(0x0f12f, 0x0f12f).w(FUNC(subsino2_state::ss9601_byte_lo_w));

	map(0x0f140, 0x0f15f).rw("io", FUNC(ss9602_device::read), FUNC(ss9602_device::write));

	map(0x0f160, 0x0f160).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0f161, 0x0f161).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0f162, 0x0f162).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x0f168, 0x0f168).w(FUNC(subsino2_state::mtrain_tilesize_w));

	map(0x10000, 0x180ff).rom().region("program", 0);
}

void subsino2_state::mtrain_map(address_map &map)
{
	mtrain_base_map(map);
	map(0x0f164, 0x0f164).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void subsino2_state::tbonusal_map(address_map &map)
{
	mtrain_base_map(map);
	map(0x0f166, 0x0f167).w("ymsnd", FUNC(ym3812_device::write));
}

void subsino2_state::mtrain_io(address_map &map)
{
	map(0x0000, 0x003f).ram(); // internal regs
}

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

void subsino2_state::saklove_output0_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0,    data & 0x01 );  // coin in
	machine().bookkeeping().coin_counter_w(1,    data & 0x02 );  // key in
}

void subsino2_state::saklove_output1_w(uint8_t data)
{
}

void subsino2_state::saklove_output2_w(uint8_t data)
{
}

void subsino2_state::saklove_output3_w(uint8_t data)
{
	// 1, 2, 4
	m_eeprom->data_w(!BIT(data, 6));
}

void subsino2_state::saklove_map(address_map &map)
{
	map(0x00000, 0x07fff).ram().share("nvram"); // battery

	// read lo (L1)   (only half tilemap?)
	map(0x12000, 0x12fff).rw(FUNC(subsino2_state::ss9601_videoram_1_lo_r), FUNC(subsino2_state::ss9601_videoram_1_lo_w));
	map(0x13000, 0x131ff).rw(FUNC(subsino2_state::ss9601_scrollram_1_lo_r), FUNC(subsino2_state::ss9601_scrollram_1_lo_w));
	// read lo (L0)
	map(0x16000, 0x16fff).rw(FUNC(subsino2_state::ss9601_videoram_0_lo_r), FUNC(subsino2_state::ss9601_videoram_0_lo_w));
	map(0x17000, 0x171ff).rw(FUNC(subsino2_state::ss9601_scrollram_0_lo_r), FUNC(subsino2_state::ss9601_scrollram_0_lo_w));

	// read hi (L1)
	map(0x22000, 0x22fff).rw(FUNC(subsino2_state::ss9601_videoram_1_hi_r), FUNC(subsino2_state::ss9601_videoram_1_hi_w));
	map(0x23000, 0x231ff).rw(FUNC(subsino2_state::ss9601_scrollram_1_hi_r), FUNC(subsino2_state::ss9601_scrollram_1_hi_w));
	// read hi (L0)
	map(0x26000, 0x26fff).rw(FUNC(subsino2_state::ss9601_videoram_0_hi_r), FUNC(subsino2_state::ss9601_videoram_0_hi_w));
	map(0x27000, 0x271ff).rw(FUNC(subsino2_state::ss9601_scrollram_0_hi_r), FUNC(subsino2_state::ss9601_scrollram_0_hi_w));

	// write both (L1)
	map(0x30000, 0x31fff).rw(FUNC(subsino2_state::ss9601_videoram_1_hi_r), FUNC(subsino2_state::ss9601_videoram_1_hi_lo_w));
	// write both (L0)
	map(0x34000, 0x35fff).rw(FUNC(subsino2_state::ss9601_videoram_0_hi_r), FUNC(subsino2_state::ss9601_videoram_0_hi_lo_w));

	map(0xe0000, 0xfffff).rom().region("maincpu", 0);
}

void subsino2_state::saklove_io(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(subsino2_state::ss9601_scrollctrl_w));

	map(0x0020, 0x0020).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0040, 0x0041).w("ymsnd", FUNC(ym3812_device::write));

	map(0x0060, 0x0060).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0061, 0x0061).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0062, 0x0062).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x0080, 0x0080).w(FUNC(subsino2_state::ss9601_tilesize_w));
	map(0x00a0, 0x00a0).w(FUNC(subsino2_state::ss9601_byte_lo_w));
	map(0x021f, 0x021f).w(FUNC(subsino2_state::ss9601_disable_w));
	map(0x0220, 0x0225).w(FUNC(subsino2_state::ss9601_scroll_w));

	map(0x0300, 0x031f).rw("io", FUNC(ss9602_device::read), FUNC(ss9602_device::write));
}

/***************************************************************************
                                X-Plan
***************************************************************************/

void subsino2_state::xplan_out_d_w(uint8_t data)
{
	m_eeprom->data_w(!BIT(data, 6));
}

void subsino2_state::xplan_out_c_w(uint8_t data)
{
	m_leds[0] = BIT(data, 1);   // raise
}

void subsino2_state::xplan_out_b_w(uint8_t data)
{
	m_leds[1] = BIT(data, 2);   // hold 1 / big ?
	m_leds[2] = BIT(data, 3);   // hold 5 / bet
	m_leds[3] = BIT(data, 4);   // hold 4 ?
	m_leds[4] = BIT(data, 5);   // hold 2 / double up
	m_leds[5] = BIT(data, 6);   // hold 3 / small ?
}

void subsino2_state::xplan_out_a_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0,    data & 0x01 );
	machine().bookkeeping().coin_counter_w(1,    data & 0x02 );

	m_leds[6] = BIT(data, 4);   // start / take
}

void subsino2_state::xplan_map(address_map &map)
{
	map(0x00000, 0x07fff).ram().share("nvram"); // battery

	// write both (L1, byte_lo2)
	map(0x10000, 0x11fff).w(FUNC(subsino2_state::ss9601_videoram_1_hi_lo2_w));
	// read lo (L1)   (only half tilemap?)
	map(0x12000, 0x12fff).r(FUNC(subsino2_state::ss9601_videoram_1_lo_r));
	map(0x13000, 0x131ff).rw(FUNC(subsino2_state::ss9601_scrollram_1_lo_r), FUNC(subsino2_state::ss9601_scrollram_1_lo_w));

	// write both (L0, byte_lo2)
	map(0x14000, 0x15fff).w(FUNC(subsino2_state::ss9601_videoram_0_hi_lo2_w));
	// read lo (REEL)
	map(0x16000, 0x16fff).r(FUNC(subsino2_state::ss9601_reelram_lo_r));
	map(0x17000, 0x171ff).rw(FUNC(subsino2_state::ss9601_scrollram_0_lo_r), FUNC(subsino2_state::ss9601_scrollram_0_lo_w));

	// read hi (L1)
	map(0x22000, 0x22fff).r(FUNC(subsino2_state::ss9601_videoram_1_hi_r));
	map(0x23000, 0x231ff).rw(FUNC(subsino2_state::ss9601_scrollram_1_hi_r), FUNC(subsino2_state::ss9601_scrollram_1_hi_w));
	// read hi (REEL)
	map(0x26000, 0x26fff).r(FUNC(subsino2_state::ss9601_reelram_hi_r));
	map(0x27000, 0x271ff).rw(FUNC(subsino2_state::ss9601_scrollram_0_hi_r), FUNC(subsino2_state::ss9601_scrollram_0_hi_w));

	// write both (L1, byte_lo)
	map(0x30000, 0x31fff).w(FUNC(subsino2_state::ss9601_videoram_1_hi_lo_w));
	map(0x32000, 0x32fff).w(FUNC(subsino2_state::ss9601_videoram_1_hi_lo_w));
	map(0x33000, 0x331ff).w(FUNC(subsino2_state::ss9601_scrollram_1_hi_lo_w));
	// write both (L0 & REEL, byte_lo)
	map(0x34000, 0x35fff).w(FUNC(subsino2_state::ss9601_videoram_0_hi_lo_w));
	map(0x36000, 0x36fff).w(FUNC(subsino2_state::ss9601_reelram_hi_lo_w));
	map(0x37000, 0x371ff).w(FUNC(subsino2_state::ss9601_scrollram_0_hi_lo_w));

	map(0xc0000, 0xfffff).rom().region("maincpu", 0);
}

void subsino2_state::xplan_io(address_map &map)
{
	map(0x0000, 0x0000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x0020, 0x0020).w(FUNC(subsino2_state::ss9601_byte_lo2_w));

	map(0x0040, 0x0040).w(FUNC(subsino2_state::ss9601_scrollctrl_w));

	map(0x0060, 0x0060).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0061, 0x0061).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0062, 0x0062).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x0080, 0x0080).w(FUNC(subsino2_state::ss9601_tilesize_w));
	map(0x00a0, 0x00a0).w(FUNC(subsino2_state::ss9601_byte_lo_w));

	map(0x021f, 0x021f).w(FUNC(subsino2_state::ss9601_disable_w));
	map(0x0220, 0x0225).w(FUNC(subsino2_state::ss9601_scroll_w));

	map(0x0235, 0x0235).noprw(); // INT0 Ack.?

	map(0x0300, 0x031f).rw("io", FUNC(ss9802_device::read), FUNC(ss9802_device::write));
}

/***************************************************************************
                                X-Train
***************************************************************************/

void subsino2_state::xtrain_out_d_w(uint8_t data)
{
	m_hopper->motor_w(BIT(data, 2));
	m_eeprom->data_w(!BIT(data, 6));
}

void subsino2_state::xtrain_out_c_w(uint8_t data)
{
	if (m_ticket.found())
		m_ticket->motor_w(BIT(data, 0));

	m_leds[0] = BIT(data, 1);   // re-double
	m_leds[1] = BIT(data, 2);   // half double
}

void subsino2_state::xtrain_out_b_w(uint8_t data)
{
	m_leds[2] = BIT(data, 1);   // hold 3 / small
	m_leds[3] = BIT(data, 2);   // hold 2 / big
	m_leds[4] = BIT(data, 3);   // bet
	m_leds[5] = BIT(data, 4);   // hold1 / take
	m_leds[6] = BIT(data, 5);   // double up
}

void subsino2_state::xtrain_out_a_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0)); // coin in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1)); // key in
	machine().bookkeeping().coin_counter_w(2, BIT(data, 2)); // hopper out
	machine().bookkeeping().coin_counter_w(3, BIT(data, 3)); // ticket out

	m_leds[7] = BIT(data, 4);   // start
}


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout ss9601_8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ 0, 16, 8, 24, 32, 48, 40, 56 },
	{ STEP8(0,8 * 8) },
	8 * 8 * 8
};

static GFXDECODE_START( gfx_ss9601 )
	GFXDECODE_ENTRY( "tilemap", 0, ss9601_8x8_layout, 0, 1 )
GFXDECODE_END


/***************************************************************************
                                Input Ports
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/

static INPUT_PORTS_START( bishjan )
	PORT_START("RESET")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)

	PORT_START("DSW")   // SW1
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Controls ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("JOY")   // IN C
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) PORT_NAME("1 Player Start (Joy Mode)")    // start (joy)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )   // down (joy)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )   // left (joy)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )   // right (joy)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        )   // n (joy)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_BET    ) PORT_NAME("P1 Mahjong Bet (Joy Mode)")    // bet (joy)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2        )   // select (joy)

	PORT_START("SYSTEM") // IN A
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE        )   PORT_IMPULSE(1) // service mode (press twice for inputs)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM        )   PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1       )   // stats
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2       )   // pay out? "hopper empty"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1          )   PORT_IMPULSE(2) // coin
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3       )   // pay out? "hopper empty"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2          )   PORT_IMPULSE(2) // coin

	PORT_START("KEYB_0")    // IN B(0)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A      )   // a
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E      )   // e
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I      )   // i
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M      )   // m
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN    )   // i2 (kan)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1         )   // b2 (start)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEYB_1")    // IN B(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B      )   // b
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F      )   // f
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J      )   // j
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N      )   // n
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH  )   // l2 (reach)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET    )   // c2 (bet)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEYB_2")    // IN B(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C      )   // c
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G      )   // g
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K      )   // k
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI    )   // k2 (chi)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON    )   // m2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEYB_3")    // IN B(3)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D      )   // d
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H      )   // h
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L      )   // l
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON    )   // j2 (pon)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEYB_4")    // IN B(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // g2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // e2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // d2
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // f2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )
INPUT_PORTS_END

/***************************************************************************
                                  New 2001
***************************************************************************/

static INPUT_PORTS_START( new2001 )
	PORT_START("DSW") // c00001
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )
	// high byte related to sound communication

	// JAMMA inputs:
	PORT_START("IN-C") // c00003
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	// high byte not read

	PORT_START("IN-B") // c00004
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1        ) PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   ) // stats (keep pressed during boot for service mode)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       )

	PORT_START("IN-A") // c00005
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE       ) PORT_IMPULSE(1) // service mode (press twice for inputs)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) PORT_NAME("Hold 3 / Black")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   ) PORT_NAME("Double Up / Help")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) PORT_NAME("Hold 2 / Red")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD1   ) PORT_NAME("Hold 1 / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET    ) PORT_NAME("Bet (Shoot)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(1)
INPUT_PORTS_END

/***************************************************************************
                             Humlan's Lyckohjul
***************************************************************************/

static INPUT_PORTS_START( humlan )
	PORT_START("DSW") // c00001
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" ) // used
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )
	// high byte related to sound communication

	// JAMMA inputs:
	PORT_START("IN-C") // c00003
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE       ) PORT_IMPULSE(1) // service mode (press twice for inputs)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // ?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	// high byte not read

	PORT_START("IN-B") // c00004
	// 1st-type panel
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1   ) PORT_NAME("Hold 1 / Take")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   ) PORT_NAME("Double Up / Help")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) PORT_NAME("Hold 2 / Big")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   ) // stats (keep pressed during boot for service mode)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // ?

	PORT_START("IN-A") // c00005
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1        ) PORT_NAME("Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) PORT_NAME("Hold 3 / Small")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(1)
INPUT_PORTS_END

/***************************************************************************
                               Queen Bee
***************************************************************************/

static INPUT_PORTS_START( queenbee )
	PORT_INCLUDE( humlan )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Win Wave" )
	PORT_DIPSETTING(    0x01, "Small" )
	PORT_DIPSETTING(    0x00, "Big"   )
	PORT_DIPNAME( 0x40, 0x40, "Free 2nd Spin" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Title" )
	PORT_DIPSETTING(    0x80, "Queen Bee" )
	PORT_DIPSETTING(    0x00, "Fruit Holders" )
INPUT_PORTS_END

/***************************************************************************
                       Express Card / Top Card
***************************************************************************/

static INPUT_PORTS_START( expcard )
	PORT_START("DSW1")
	// unused?
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	// not populated

	PORT_START("DSW3")
	// not populated

	PORT_START("DSW4")
	// not populated

	PORT_START("IN-A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN     ) PORT_NAME("Raise") PORT_CODE(KEYCODE_M)    // raise
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1      ) PORT_NAME("Start")                 // start
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Small")        // hold 4 / small / decrease sample in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Bet")          // hold 1 / bet

	PORT_START("IN-B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Take" )        // hold 2 / take
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Double Up" )   // hold 3 / double up
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Big")          // hold 5 / big / increase sample in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1       )                                    // coin in
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN     )

	PORT_START("IN-C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK  )                                   // stats (keep pressed during boot for service mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                                   // cancel?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )                                   // key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE      ) PORT_IMPULSE(1)                   // service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN      )

	PORT_START("IN-D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER        ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)  // reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER       ) // serial out
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM      ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r)
INPUT_PORTS_END

/***************************************************************************
                               Magic Train
***************************************************************************/

static INPUT_PORTS_START( mtrain )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x00, "Key Coinage" )           PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x08, "1 Key / 1 Credits" )
	PORT_DIPSETTING(    0x10, "1 Key / 2 Credits" )
	PORT_DIPSETTING(    0x18, "1 Key / 5 Credits" )
	PORT_DIPSETTING(    0x00, "1 Key / 10 Credits" )
	PORT_DIPSETTING(    0x20, "1 Key / 20 Credits" )
	PORT_DIPSETTING(    0x28, "1 Key / 25 Credits" )
	PORT_DIPSETTING(    0x30, "1 Key / 50 Credits" )
	PORT_DIPSETTING(    0x38, "1 Key / 100 Credits" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Minimum Bet" )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x03, "40" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )               PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPSETTING(    0x0c, "80" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Game Limit" )            PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x20, "10k" )
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x40, "30k" )
	PORT_DIPSETTING(    0x60, "60k" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Win Rate" )      PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:5")   // used
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Double-Up Rate" )        PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x00, "82%" )
	PORT_DIPSETTING(    0x01, "84%" )
	PORT_DIPSETTING(    0x02, "86%" )
	PORT_DIPSETTING(    0x03, "88%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x05, "92%" )
	PORT_DIPSETTING(    0x06, "94%" )
	PORT_DIPSETTING(    0x07, "96%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x00, "5k" )
	PORT_DIPSETTING(    0x08, "10k" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN-A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START      ) PORT_CODE(KEYCODE_N)    PORT_NAME("Start All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_CODE(KEYCODE_Z)    PORT_NAME("Info / Double?")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN-B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  ) PORT_IMPULSE(5)          // key in
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1         )                          // coin in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )  // stats
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE       )  // service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout (hopper error)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("IN-C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN-D")  // not shown in test mode
	PORT_BIT( 0x47, IP_ACTIVE_HIGH, IPT_UNUSED  ) // outputs
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER   ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r) // serial in
INPUT_PORTS_END

/***************************************************************************
                          Super Train
***************************************************************************/

static INPUT_PORTS_START( strain ) // inputs need verifying
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )                  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )                  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )                  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )                  PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Show Demo" )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN-A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START      ) PORT_CODE(KEYCODE_N)    PORT_NAME("Start All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_CODE(KEYCODE_Z)    PORT_NAME("Info / Double?")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN-B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  ) PORT_IMPULSE(5)          // key in
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1         )                          // coin in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )  // stats
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE       )  // service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout (hopper error)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("IN-C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN-D")  // not shown in test mode
	PORT_BIT( 0x47, IP_ACTIVE_HIGH, IPT_UNUSED  ) // outputs
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER   ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r) // serial in
INPUT_PORTS_END

/***************************************************************************
                          Treasure Bonus
***************************************************************************/

static INPUT_PORTS_START( tbonusal ) // inputs need verifying
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )                  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )                  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )                  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )                  PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN-A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START      ) PORT_CODE(KEYCODE_N)    PORT_NAME("Start All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_CODE(KEYCODE_Z)    PORT_NAME("Info / Double?")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN-B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  ) PORT_IMPULSE(5)          // key in
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1         )                          // coin in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )  // stats
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE       )  // service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout (hopper error)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("IN-C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN-D")  // not shown in test mode
	PORT_BIT( 0x47, IP_ACTIVE_HIGH, IPT_UNUSED  ) // outputs
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER   ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r) // serial in
INPUT_PORTS_END

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

static INPUT_PORTS_START( saklove )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Coin" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x04, "25" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "100" )
	PORT_DIPSETTING(    0x07, "300" )
	PORT_DIPNAME( 0x38, 0x00, "Key In" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x18, "25" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x28, "300" )
	PORT_DIPSETTING(    0x30, "500" )
	PORT_DIPSETTING(    0x38, "1000" )
	PORT_DIPNAME( 0x40, 0x00, "Pay Out" )
	PORT_DIPSETTING(    0x00, "Coin" )
	PORT_DIPSETTING(    0x40, "Key In" )
	PORT_DIPNAME( 0x80, 0x00, "Key Out" )
	PORT_DIPSETTING(    0x80, "Coin" )
	PORT_DIPSETTING(    0x00, "Key In" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Min Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPNAME( 0x0c, 0x00, "Max Bet" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPUNKNOWN( 0x10, 0x00 )
	PORT_DIPUNKNOWN( 0x20, 0x00 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Double Up" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x00, "Win Rate (%)" )
	PORT_DIPSETTING(    0x01, "55" )
	PORT_DIPSETTING(    0x02, "60" )
	PORT_DIPSETTING(    0x03, "65" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x05, "75" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPSETTING(    0x06, "85" )
	PORT_DIPSETTING(    0x07, "90" )
	PORT_DIPNAME( 0x18, 0x00, "Game Limit" )
	PORT_DIPSETTING(    0x08, "10k" )
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x10, "60k" )
	PORT_DIPSETTING(    0x18, "80k" )
	PORT_DIPUNKNOWN( 0x20, 0x00 )
	PORT_DIPUNKNOWN( 0x40, 0x00 )
	PORT_DIPUNKNOWN( 0x80, 0x00 )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x03, 0x00, "Double Up Level" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x00, "Double Up Limit" )
	PORT_DIPSETTING(    0x00, "5k" )
	PORT_DIPSETTING(    0x04, "10k" )
	PORT_DIPSETTING(    0x08, "20k" )
	PORT_DIPSETTING(    0x0c, "30k" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPNAME( 0x20, 0x00, "Coin Type" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPNAME( 0x80, 0x00, "JAMMA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN-A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_NAME("Bet 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_NAME("Bet 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_NAME("Bet 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1   ) PORT_NAME("Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5  ) PORT_NAME("Big or Small 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4  ) PORT_NAME("Bet Amount")   // 1-5-10

	PORT_START("IN-B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2   )           // selects music in system test / exit
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )  // top 10? / double up?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6  ) PORT_NAME("Big or Small 2")   // plays sample or advances music in system test / big or small?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?

	PORT_START("IN-C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Statistics")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2    )   // key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_IMPULSE(2)   // service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?

	PORT_START("IN-D")  // bits 3 and 4 shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER   ) // serial out
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r) // serial in
INPUT_PORTS_END

/***************************************************************************
                             Treasure City
***************************************************************************/

static INPUT_PORTS_START( treacity )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )                  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )                  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )                  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )                  PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN-A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_NAME("Bet 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_NAME("Bet 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_NAME("Bet 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1   ) PORT_NAME("Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5  ) PORT_NAME("Big or Small 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4  ) PORT_NAME("Bet Amount")   // 1-5-10

	PORT_START("IN-B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2   )           // selects music in system test / exit
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )  // top 10? / double up?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6  ) PORT_NAME("Big or Small 2")   // plays sample or advances music in system test / big or small?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?

	PORT_START("IN-C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Statistics")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2    )   // key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_IMPULSE(2)   // service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?

	PORT_START("IN-D")  // bits 3 and 4 shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER   ) // serial out
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r) // serial in
INPUT_PORTS_END

/***************************************************************************
                                X-Plan
***************************************************************************/

static INPUT_PORTS_START( xplan )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Pinout" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "JAMMA (28 pin)" )
	PORT_DIPSETTING(    0x00, "Lucky 8 Liner (36 pin & 10 pin)" )       // not implemented
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	// not populated

	PORT_START("DSW3")
	// not populated

	PORT_START("DSW4")
	// not populated

	// JAMMA inputs:
	PORT_START("IN-A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1       ) PORT_NAME("A / Play Gambling 1")         // A \__ play gambling game
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2       ) PORT_NAME("C / Play Gambling 2")         // C /
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3       ) PORT_NAME("B / Play Shoot'Em Up")        // B ___ play shoot'em up game
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1        ) PORT_NAME("Start / Take")                // start / take
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) PORT_NAME("Hold 3 / Small")              // hold 3 / small / decrease sample in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5   ) PORT_NAME("Hold 5 / Bet")                // hold 5 / bet

	PORT_START("IN-B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4   ) PORT_NAME("Hold 4 / Re-Double" )         // hold 4 / re-double?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) PORT_NAME("Hold 2 / Double Up / Right")  // hold 2 / double up? / right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Raise") PORT_CODE(KEYCODE_N)  // raise
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1   ) PORT_NAME("Hold 1 / Big / Left")         // hold 1 / big / increase sample in test mode / left
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1         )                                          // coin in
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       )

	PORT_START("IN-C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )                      // stats (keep pressed during boot for service mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )                      // key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE       )  PORT_IMPULSE(1)     // service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                      // pay-out

	PORT_START("IN-D")  // bits 3 and 4 shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )                      // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER        ) // serial out
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM       ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r) // serial in
INPUT_PORTS_END

/***************************************************************************
                                X-Train
***************************************************************************/

static INPUT_PORTS_START( xtrain )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	// not populated

	PORT_START("DSW3")
	// not populated

	PORT_START("DSW4")
	// not populated

	// JAMMA inputs:
	PORT_START("IN-A")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)       PORT_NAME("Re-Double") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF) PORT_NAME("Half Double")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1)      PORT_NAME("Start")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3) PORT_NAME("Hold 3 / Small")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BET)  PORT_NAME("Bet")

	PORT_START("IN-B")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1) PORT_NAME("Hold 1 / Take" )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP) PORT_NAME("Double Up / Help")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2) PORT_NAME("Hold 2 / Big")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN1)       PORT_NAME("Coin In")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN-C")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK) // keep pressed during boot for service mode
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT)

	PORT_START("IN-D")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_CUSTOM)  PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)   PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) // serial out
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r) // serial in
INPUT_PORTS_END

static INPUT_PORTS_START( ptrain )
	PORT_INCLUDE(xtrain)

	PORT_MODIFY("IN-B")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
INPUT_PORTS_END


/***************************************************************************
                               Water-Nymph
***************************************************************************/

static INPUT_PORTS_START( wtrnymph )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x00, "Key Coinage" )           PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x08, "1 Key / 1 Credits" )
	PORT_DIPSETTING(    0x10, "1 Key / 2 Credits" )
	PORT_DIPSETTING(    0x18, "1 Key / 5 Credits" )
	PORT_DIPSETTING(    0x00, "1 Key / 10 Credits" )
	PORT_DIPSETTING(    0x20, "1 Key / 20 Credits" )
	PORT_DIPSETTING(    0x28, "1 Key / 25 Credits" )
	PORT_DIPSETTING(    0x30, "1 Key / 50 Credits" )
	PORT_DIPSETTING(    0x38, "1 Key / 100 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out" )               PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Coin" )
	PORT_DIPSETTING(    0x00, "Key" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Minimum Bet" )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )               PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x04, "20" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x0c, "60" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Game Limit" )            PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x20, "10k" )
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x40, "30k" )
	PORT_DIPSETTING(    0x60, "40k" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Win Rate" )      PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Double-Up Rate" )        PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x00, "82%" )
	PORT_DIPSETTING(    0x01, "84%" )
	PORT_DIPSETTING(    0x02, "88%" )
	PORT_DIPSETTING(    0x03, "90%" )
	PORT_DIPSETTING(    0x04, "92%" )
	PORT_DIPSETTING(    0x05, "94%" )
	PORT_DIPSETTING(    0x06, "96%" )
	PORT_DIPSETTING(    0x07, "98%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x00, "5k" )
	PORT_DIPSETTING(    0x08, "10k" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN-A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START      ) PORT_CODE(KEYCODE_N)    PORT_NAME("Start All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_CODE(KEYCODE_Z)    PORT_NAME("Info / Double?") // down
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN-B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  ) PORT_IMPULSE(5)          // key in
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1         )                          // coin in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )  // stats
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE       )  // service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout (hopper error)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("IN-C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Left / Play Gambling 1")        // C \__ play gambling game
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_NAME("Play Gambling 2") PORT_CODE(KEYCODE_D)   // D /
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take / Rotate")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_NAME("Play Tetris")     PORT_CODE(KEYCODE_T)   // T |__ play Tetris game

	PORT_START("IN-D")  // not shown in test mode
	PORT_BIT( 0x47, IP_ACTIVE_HIGH, IPT_UNUSED  ) // outputs
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER   ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r) // serial in
INPUT_PORTS_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/

void subsino2_state::bishjan(machine_config &config)
{
	H83044(config, m_maincpu, XTAL(44'100'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino2_state::bishjan_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ss9802_device &io(SS9802(config, "io"));
	io.in_port_callback<1>().set_ioport("DSW");   // SW1
	io.out_port_callback<2>().set(FUNC(subsino2_state::bishjan_input_w));
	io.in_port_callback<3>().set_ioport("JOY");   // IN C
	io.in_port_callback<4>().set(FUNC(subsino2_state::bishjan_input_r));  // IN B
	io.in_port_callback<5>().set_ioport("SYSTEM");  // IN A
	io.in_port_callback<6>().set(FUNC(subsino2_state::bishjan_serial_r));  // IN D
	io.in_port_callback<7>().set(FUNC(subsino2_state::bishjan_unknown_r));
	io.in_port_callback<9>().set_ioport("RESET");
	io.out_port_callback<9>().set(FUNC(subsino2_state::bishjan_outputs_w));

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	DS2430A(config, m_eeprom).set_timing_scale(0.24);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-16-1);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(subsino2_state::screen_update_subsino2));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0); // edge-triggered interrupt

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ss9601);
	PALETTE(config, m_palette).set_entries(256);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette)); // HMC HM86171 VGA 256 colour RAMDAC
	ramdac.set_addrmap(0, &subsino2_state::ramdac_map);

	// sound hardware
	// SS9904
}

void subsino2_state::xiaoao(machine_config &config)
{
	bishjan(config);

	ss9802_device &io(*subdevice<ss9802_device>("io"));
	io.in_port_callback<6>().set(FUNC(subsino2_state::xiaoao_serial_r));
}

void subsino2_state::new2001(machine_config &config)
{
	bishjan(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino2_state::new2001_map);

	ss9802_device &io(*subdevice<ss9802_device>("io"));
	io.in_port_callback<0>().set(FUNC(subsino2_state::new2001_sound_ready_r));
	io.in_port_callback<3>().set_ioport("IN-C");
	io.in_port_callback<4>().set_ioport("IN-B");
	io.in_port_callback<5>().set_ioport("IN-A");
	io.out_port_callback<8>().set(FUNC(subsino2_state::new2001_output0_w));
	io.out_port_callback<9>().set(FUNC(subsino2_state::new2001_output1_w));
	io.in_port_callback<9>().set_constant(0);

	m_screen->set_size(640, 256);
	m_screen->set_visarea(0, 640-1, 0, 256-16-1);
}

void subsino2_state::humlan(machine_config &config)
{
	bishjan(config);
	H83044(config.replace(), m_maincpu, XTAL(48'000'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino2_state::new2001_map);

	ss9802_device &io(*subdevice<ss9802_device>("io"));
	io.in_port_callback<0>().set(FUNC(subsino2_state::new2001_sound_ready_r));
	io.in_port_callback<3>().set_ioport("IN-C");
	io.in_port_callback<4>().set_ioport("IN-B");
	io.in_port_callback<5>().set_ioport("IN-A");
	io.out_port_callback<8>().set(FUNC(subsino2_state::humlan_output0_w));
	io.out_port_callback<9>().set(FUNC(subsino2_state::humlan_output1_w));
	io.in_port_callback<9>().set_constant(0);

	m_eeprom->set_timing_scale(0.32);

	// sound hardware
	// SS9804
}

/***************************************************************************
                                Magic Train
***************************************************************************/

void subsino2_state::mtrain(machine_config &config)
{
	HD647180X(config, m_maincpu, XTAL(12'000'000));   /* Unknown clock */
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino2_state::mtrain_map);
	m_maincpu->set_addrmap(AS_IO, &subsino2_state::mtrain_io);

	ss9602_device &io(SS9602(config, "io"));
	io.out_port_callback<0>().set(FUNC(subsino2_state::mtrain_output0_w));
	io.out_port_callback<1>().set(FUNC(subsino2_state::mtrain_output1_w));
	io.out_port_callback<2>().set(FUNC(subsino2_state::mtrain_output2_w));
	io.out_port_callback<3>().set(FUNC(subsino2_state::mtrain_output3_w));
	io.in_port_callback<3>().set_ioport("IN-D"); // (not shown in system test) 0x40 serial out, 0x80 serial in
	io.in_port_callback<4>().set_ioport("IN-A"); // A
	io.in_port_callback<5>().set_ioport("IN-B"); // B
	io.in_port_callback<6>().set_ioport("IN-C"); // C
	io.in_port_callback<7>().set(FUNC(subsino2_state::dsw_r));
	io.out_port_callback<8>().set(FUNC(subsino2_state::dsw_mask_w));
	io.in_port_callback<9>().set(FUNC(subsino2_state::vblank_bit2_r));
	io.out_port_callback<9>().set(FUNC(subsino2_state::oki_bank_bit0_w));

	DS2430A(config, m_eeprom).set_timing_scale(0.73);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-32-1);
	m_screen->set_refresh_hz(58.7270);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_screen_update(FUNC(subsino2_state::screen_update_subsino2));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ss9601);
	PALETTE(config, m_palette).set_entries(256);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette)); // HMC HM86171 VGA 256 colour RAMDAC
	ramdac.set_addrmap(0, &subsino2_state::ramdac_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, XTAL(8'467'200) / 8, okim6295_device::PIN7_HIGH);    // probably
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void subsino2_state::tbonusal(machine_config &config)
{
	mtrain(config);
	config.device_remove("oki");
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino2_state::tbonusal_map);
	subdevice<ss9602_device>("io")->out_port_callback<9>().set_nop();

	YM3812(config, "ymsnd", 3'000'000).add_route(ALL_OUTPUTS, "mono", 0.80); // ? chip and clock unknown
}

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

void subsino2_state::saklove(machine_config &config)
{
	AM188EM(config, m_maincpu, XTAL(20'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino2_state::saklove_map);
	m_maincpu->set_addrmap(AS_IO, &subsino2_state::saklove_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ss9602_device &io(SS9602(config, "io"));
	io.out_port_callback<0>().set(FUNC(subsino2_state::saklove_output0_w));
	io.out_port_callback<1>().set(FUNC(subsino2_state::saklove_output1_w));
	io.out_port_callback<2>().set(FUNC(subsino2_state::saklove_output2_w));
	io.out_port_callback<3>().set(FUNC(subsino2_state::saklove_output3_w));
	io.in_port_callback<3>().set_ioport("IN-D"); // 0x40 serial out, 0x80 serial in
	io.in_port_callback<4>().set_ioport("IN-A");
	io.in_port_callback<5>().set_ioport("IN-B");
	io.in_port_callback<6>().set_ioport("IN-C");
	io.in_port_callback<7>().set(FUNC(subsino2_state::dsw_r));
	io.out_port_callback<8>().set(FUNC(subsino2_state::dsw_mask_w));
	io.in_port_callback<9>().set(FUNC(subsino2_state::vblank_bit2_r));
	io.out_port_callback<9>().set(FUNC(subsino2_state::oki_bank_bit0_w));

	DS2430A(config, m_eeprom);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-16-1);
	m_screen->set_refresh_hz(58.7270);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_screen_update(FUNC(subsino2_state::screen_update_subsino2));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ss9601);
	PALETTE(config, m_palette).set_entries(256);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette)); // HMC HM86171 VGA 256 colour RAMDAC
	ramdac.set_addrmap(0, &subsino2_state::ramdac_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, XTAL(8'467'200) / 8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.80);

	YM3812(config, "ymsnd", XTAL(12'000'000) / 4).add_route(ALL_OUTPUTS, "mono", 0.80); // ? chip and clock unknown
}

/***************************************************************************
                                X-Plan
***************************************************************************/

void subsino2_state::xplan(machine_config &config)
{
	AM188EM(config, m_maincpu, XTAL(20'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino2_state::xplan_map);
	m_maincpu->set_addrmap(AS_IO, &subsino2_state::xplan_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ss9802_device &io(SS9802(config, "io"));
	io.in_port_callback<0>().set(FUNC(subsino2_state::vblank_bit6_r));
	io.out_port_callback<0>().set(FUNC(subsino2_state::oki_bank_bit4_w));
	io.out_port_callback<1>().set(FUNC(subsino2_state::dsw_mask_w));
	io.in_port_callback<2>().set(FUNC(subsino2_state::dsw_r));
	io.in_port_callback<3>().set_ioport("IN-C");
	io.in_port_callback<4>().set_ioport("IN-B");
	io.in_port_callback<5>().set_ioport("IN-A");
	io.in_port_callback<6>().set_ioport("IN-D"); // 0x40 serial out, 0x80 serial in
	io.out_port_callback<6>().set(FUNC(subsino2_state::xplan_out_d_w));
	io.out_port_callback<7>().set(FUNC(subsino2_state::xplan_out_c_w));
	io.out_port_callback<8>().set(FUNC(subsino2_state::xplan_out_b_w)); // B
	io.out_port_callback<9>().set(FUNC(subsino2_state::xplan_out_a_w)); // A

	DS2430A(config, m_eeprom);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-16-1);
	m_screen->set_refresh_hz(58.7270);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);   // game reads vblank state
	m_screen->set_screen_update(FUNC(subsino2_state::screen_update_subsino2));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set("maincpu", FUNC(am188em_device::int0_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ss9601);
	PALETTE(config, m_palette).set_entries(256);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette)); // HMC HM86171 VGA 256 colour RAMDAC
	ramdac.set_addrmap(0, &subsino2_state::ramdac_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, XTAL(8'467'200) / 8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);    // probably
}

void subsino2_state::xtrain(machine_config &config)
{
	xplan(config);

	ss9802_device &io(*subdevice<ss9802_device>("io"));
	io.out_port_callback<6>().set(FUNC(subsino2_state::xtrain_out_d_w)); // D
	io.out_port_callback<7>().set(FUNC(subsino2_state::xtrain_out_c_w)); // C
	io.out_port_callback<8>().set(FUNC(subsino2_state::xtrain_out_b_w)); // B
	io.out_port_callback<9>().set(FUNC(subsino2_state::xtrain_out_a_w)); // A

	HOPPER(config, m_hopper, attotime::from_msec(200));
}

void subsino2_state::ptrain(machine_config &config)
{
	xtrain(config);

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(200));
}

void subsino2_state::expcard(machine_config &config)
{
	xplan(config);

	ss9802_device &io(*subdevice<ss9802_device>("io"));
	io.out_port_callback<8>().set(FUNC(subsino2_state::expcard_out_b_w)); // B
	io.out_port_callback<9>().set(FUNC(subsino2_state::expcard_out_a_w)); // A
}


/***************************************************************************
                                ROMs Loading
***************************************************************************/

#define HD647180X_FAKE_INTERNAL_ROM \
	ROM_FILL( 0x0000, 1, 0x3e ) \
	ROM_FILL( 0x0001, 1, 0x09 ) \
	ROM_FILL( 0x0002, 1, 0xed ) \
	ROM_FILL( 0x0003, 1, 0x39 ) \
	ROM_FILL( 0x0004, 1, 0x38 ) \
	ROM_FILL( 0x0005, 1, 0xc3 ) \
	ROM_FILL( 0x0006, 1, 0x16 ) \
	ROM_FILL( 0x0007, 1, 0xf0 )

/***************************************************************************

Bishou Jan (Laugh World)
(C)1999 Subsino

PCB Layout
----------

|------------------------------------------------------|
|TDA1519A           28-WAY                             |
|     VOL                                              |
|                HM86171                       ULN2003 |
|   LM324                                              |
|           S-1                                ULN2003 |
|                                                      |
|                                   |-------|  DSW1(8) |
|                       |-------|   |SUBSINO|          |
|            2-V201.U9  |SUBSINO|   |SS9802 |          |
|                       |SS9904 |   |       |          |
|                       |       |   |-------|          |
|                       |-------|                      |
|                                                      |
|                         44.1MHz             CXK58257 |
|  3-V201.U25                                          |
|                                  1-V203.U21          |
|  4-V201.U26                                       SW1|
|             |-------|    |-------|   |-----|         |
|  5-V201.U27 |SUBSINO|    |SUBSINO|   |H8   |         |
|             |SS9601 |    |SS9803 |   |3044 |         |
|  6-V201.U28 |       |    |       |   |-----|         |
|             |-------|    |-------|                   |
|          62256  62256   BATTERY                      |
|------------------------------------------------------|
Notes:
      H8/3044 - Subsino re-badged Hitachi H8/3044 HD6433044A22F Microcontroller (QFP100)
                The H8/3044 is a H8/3002 with 24bit address bus and has 32k mask ROM and 2k RAM, clock input is 14.7MHz [44.1/3]
                MD0,MD1 & MD2 are configured to MODE 6 16MByte Expanded Mode with the on-chip 32k mask ROM enabled.
     CXK58257 - Sony CXK58257 32k x8 SRAM (SOP28)
      HM86171 - Hualon Microelectronics HMC HM86171 VGA 256 colour RAMDAC (DIP28)
          S-1 - ?? Probably some kind of audio OP AMP or DAC? (DIP8)
          SW1 - Push Button Test Switch
        HSync - 15.75kHz
        VSync - 60Hz

***************************************************************************/

ROM_START( bishjan )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "1-v203.u21",             0x080000, 0x080000, CRC(1f891d48) SHA1(0b6a5aa8b781ba8fc133289790419aa8ea21c400) )

	ROM_REGION( 0x400000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "3-v201.u25", 0x000000, 0x100000, CRC(e013e647) SHA1(a5b0f82f3454393c1ea5e635b0d37735a25e2ea5) )
	ROM_LOAD32_BYTE( "4-v201.u26", 0x000002, 0x100000, CRC(e0d40ef1) SHA1(95f80889103a7b93080b46387274cb1ffe0c8768) )
	ROM_LOAD32_BYTE( "5-v201.u27", 0x000001, 0x100000, CRC(85067d40) SHA1(3ecf7851311a77a0dfca90775fcbf6faabe9c2ab) )
	ROM_LOAD32_BYTE( "6-v201.u28", 0x000003, 0x100000, CRC(430bd9d7) SHA1(dadf5a7eb90cf2dc20f97dbf20a4b6c8e7734fb1) )

	ROM_REGION( 0x100000, "samples", 0 ) // SS9904
	ROM_LOAD( "2-v201.u9", 0x000000, 0x100000, CRC(ea42764d) SHA1(13fe1cd30e474f4b092949c440068e9ddca79976) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "bishoujan-ds2430a.q3", 0x00, 0x28, CRC(7366d9d5) SHA1(1b276015f70bdc8cc7ba8380be19a821e728b617) )
ROM_END

// Uses newer PCB type, same as Humlan's Lyckohjul
ROM_START( xiaoao )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "1-v100.u21",             0x080000, 0x080000, CRC(728b4597) SHA1(97f92b9a6c455d2d906d55482166fd9704253615) )

	ROM_REGION( 0x400000, "tilemap", 0 )
	ROM_LOAD( "mj-gc1.u24", 0x000000, 0x400000, CRC(ed3eaaea) SHA1(941ef99dfb2ba0e26112dcd992f7690a1dba8d9c) )

	ROM_REGION( 0x100000, "samples", 0 )
	ROM_LOAD( "mj-v1.u10", 0x000000, 0x100000, CRC(4d797394) SHA1(fa40a410f903cd81f15c3a86a60ad405b5db8168) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "xiaoaojianghu-ds2430a.q3", 0x00, 0x28, CRC(518e4ba3) SHA1(704fb6f8ff9966d1b90af849b2b7c6df06d3e4a0) )
ROM_END


/***************************************************************************

New 2001 (Italy, V200N)
(C)2000 Subsino

CPU:
  Subsino SS9689 (TQFP100, U16, H8/3044)

Video:
  Subsino SS9601      (TQFP12, U23)
  Subsino SS9802 9948 (QFP100, U1)
  Subsino SS9803      (TQFP100, U29)

Sound:
  Subsino SS9904 (QFP100, U8)
  Subsino S-1    (DIP8, U11, audio OP AMP or DAC?)
  ST324          (U12, Quad Operational Amplifier)
  TDA1519        (U14, Audio Amplifier)
  Oscill. 44.100 (OSC48)

ROMs:
  27C020     (1)
  27C4001    (2)
  4x 27C2001 (3,4,5,6)

RAMs:
  2x HM62H256AK-15  (U30,U31)
  CXK58257AM-10L    (U22)
  RAMDAC HM86171-80 (U35)

Others:
  28x2 JAMMA edge connector
  36x2 edge connector
  10x2 edge connector
  6 legs connector
  pushbutton (SW1)
  trimmer (volume)(VR1)
  8x2 switches DIP(DS1)
  3x 3 legs jumper (JP1,JP2,JP4)
  battery 3V

***************************************************************************/

ROM_START( new2001 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16",     0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "new_2001_italy_1_v200n.u21", 0x080000, 0x040000, CRC(bacc8c01) SHA1(e820bc53fa297c3f543a1d65d47eb7b5ee85a6e2) )
	ROM_RELOAD(                             0x0c0000, 0x040000 )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "new_2001_italy_3_v200.0.u25", 0x00000, 0x40000, CRC(621452d6) SHA1(a9654bb98df16b13e8bbc6dd4dada2e63ee05dc9) )
	ROM_LOAD32_BYTE( "new_2001_italy_4_v200.1.u26", 0x00002, 0x40000, CRC(3073e2d2) SHA1(fb257c625e177d7aa12f1b176a3d1b93d5891cab) )
	ROM_LOAD32_BYTE( "new_2001_italy_5_v200.2.u27", 0x00001, 0x40000, CRC(d028696b) SHA1(ebb047e7cafaefbdeb479c3877aea4fce0c47ad2) )
	ROM_LOAD32_BYTE( "new_2001_italy_6_v200.3.u28", 0x00003, 0x40000, CRC(085599e3) SHA1(afd4bed369a96ba12037e6b8cf3a4cab84d12b21) )

	ROM_REGION( 0x80000, "samples", 0 ) // SS9904
	ROM_LOAD( "new_2001_italy_2_v200.u9", 0x00000, 0x80000, CRC(9d522d04) SHA1(68f314b077a62598f3de8ef753bdedc93d6eca71) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(71281d72) SHA1(1661181a5a5331083d649b10a7d3a36062e617c0) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

Queen Bee
(c) 2001 Subsino

no ROM labels available

***************************************************************************/

ROM_START( queenbee )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "27c020 u21.bin",         0x080000, 0x040000, CRC(baec0241) SHA1(345cfee7bdb4f4c61caa828372a121f3917bb4eb) )
	ROM_FILL(                           0x0c0000, 0x040000, 0xff )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "27c4001 u25.bin", 0x000000, 0x80000, CRC(628ed650) SHA1(dadbc5f73f6a5773303d834a44d2eab836874cfe) )
	ROM_LOAD32_BYTE( "27c4001 u26.bin", 0x000002, 0x80000, CRC(27a169df) SHA1(d36989c300051a0c41752638ab5134a9b04c50a4) )
	ROM_LOAD32_BYTE( "27c4001 u27.bin", 0x000001, 0x80000, CRC(27e8c4b9) SHA1(b010b9dcadb357cf4e79d97ce84b86f792bd8ecf) )
	ROM_LOAD32_BYTE( "27c4001 u28.bin", 0x000003, 0x80000, CRC(7f139a04) SHA1(595a114806756e6f77a6fe20a13515b211ffdf2a) )

	ROM_REGION( 0x80000, "samples", 0 )
	ROM_LOAD( "27c4001 u9.bin", 0x000000, 0x80000, CRC(c7cda990) SHA1(193144fe0c31fc8342bd44aa4899bf15f0bc399d) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(f64b92e5) SHA1(fbef61b1046c6559d5ac71e665e822f9a6704461) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( queenbeeb )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "u21",                    0x080000, 0x040000, CRC(23e0ad8f) SHA1(d913ebd249c471ab36aabe515a8b36bb3590c1ca) )
	ROM_FILL(                           0x0c0000, 0x040000, 0xff )

	ROM_REGION( 0x200000, "tilemap", 0 ) // this PCB has a single surface mounted ROM, which hasn't been dumped.
	ROM_LOAD( "gfx", 0x000000, 0x200000, NO_DUMP )
	// following ROMs are taken from humlan for testing, it doesn't seem to be a case of just differently split ROMs.
	// ROM_LOAD32_BYTE( "hlj__truemax_3_v402.u25", 0x000000, 0x80000, CRC(dfc8d795) SHA1(93e0fe271c7390596f73092720befe11d8354838) )
	// ROM_LOAD32_BYTE( "hlj__truemax_4_v402.u26", 0x000002, 0x80000, CRC(31c774d6) SHA1(13fcdb42f5fd7d0cadd3fd7030037c21b7585f0f) )
	// ROM_LOAD32_BYTE( "hlj__truemax_5_v402.u27", 0x000001, 0x80000, CRC(28e14be8) SHA1(778906427175ca50ad5b0a7c5978c36ed29ef994) )
	// ROM_LOAD32_BYTE( "hlj__truemax_6_v402.u28", 0x000003, 0x80000, CRC(d1c7ae17) SHA1(3ddb8ad38eeb5ab0a944d7d26cfb890a4327ef2e) )

	ROM_REGION( 0x40000, "samples", 0 )
	ROM_LOAD( "u9", 0x000000, 0x40000, NO_DUMP )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(b6d57e98) SHA1(6bbed2613c667369e74c417917c1c36d36f03739) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

// make sure these are really queenbee
ROM_START( queenbeei )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "u21 9ac9 v100",          0x080000, 0x040000, CRC(061b406f) SHA1(2a5433817e41610e9ba90302a6b9608f769176a0) )
	ROM_FILL(                           0x0c0000, 0x040000, 0xff )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, NO_DUMP )

	ROM_REGION( 0x80000, "samples", 0 )
	ROM_LOAD( "u9", 0x000000, 0x80000, NO_DUMP )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(25d37d36) SHA1(9d7130328be80c1b9376ac6923300122ee1b9399) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( queenbeesa )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "00b0 u21 1v101",         0x080000, 0x040000, CRC(19e31fd7) SHA1(01cf507958b0411d21dd660280f45668d7c5b9d9) )
	ROM_FILL(                           0x0c0000, 0x040000, 0xff )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, NO_DUMP )

	ROM_REGION( 0x80000, "samples", 0 )
	ROM_LOAD( "u9", 0x000000, 0x80000, NO_DUMP )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(a084e2c9) SHA1(18ba0577ab61d89816b157ee24532c4a3f8d0b6f) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

Humlan's Lyckohjul (Sweden, V402)
(c) 2001 Subsino & Truemax

Swedish version of Queen Bee.

The PCB is similar to bishjan and new2001, but with a 48MHz crystal
and a 9804 (instead of 9904) for sound.

***************************************************************************/

ROM_START( humlan )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16",  0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "hlj__truemax_1_v402.u21", 0x080000, 0x040000, CRC(5b4a7113) SHA1(9a9511aa79a6e90e8ac1b267e058c8696d13d84f) )
	ROM_FILL(                            0x0c0000, 0x040000, 0xff )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "hlj__truemax_3_v402.u25", 0x000000, 0x80000, CRC(dfc8d795) SHA1(93e0fe271c7390596f73092720befe11d8354838) )
	ROM_LOAD32_BYTE( "hlj__truemax_4_v402.u26", 0x000002, 0x80000, CRC(31c774d6) SHA1(13fcdb42f5fd7d0cadd3fd7030037c21b7585f0f) )
	ROM_LOAD32_BYTE( "hlj__truemax_5_v402.u27", 0x000001, 0x80000, CRC(28e14be8) SHA1(778906427175ca50ad5b0a7c5978c36ed29ef994) )
	ROM_LOAD32_BYTE( "hlj__truemax_6_v402.u28", 0x000003, 0x80000, CRC(d1c7ae17) SHA1(3ddb8ad38eeb5ab0a944d7d26cfb890a4327ef2e) )

	ROM_REGION( 0x40000, "samples", 0 ) // SS9804
	// clearly samples, might be different from the SS9904 case
	ROM_LOAD( "subsino__qb-v1.u9", 0x000000, 0x40000, CRC(c5dfed44) SHA1(3f5effb85de10c0804efee9bce769d916268bfc9) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(281eb16b) SHA1(db62a7004e2bc9a052d6f154cb4c6d645d00f768) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

X-Reel (c) 2002 Subsino & ECM

Same PCB as bishjan and new2001, but with a 48MHz crystal

***************************************************************************/

ROM_START( xreel )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "x-reel_ecm_1_v105.u21",  0x080000, 0x040000, CRC(f9307deb) SHA1(dff5e47d7bbf4ec96aba479f350c7891d97b86c8) )
	ROM_FILL(                           0x0c0000, 0x040000, 0xff )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "x-reel_ecm_3_v103.u25", 0x000000, 0x80000, CRC(00dda66e) SHA1(f87e60cbe6b328fa285f118eee3652873ef4a45f) )
	ROM_LOAD32_BYTE( "x-reel_ecm_4_v103.u26", 0x000002, 0x80000, CRC(3848d12c) SHA1(5e20c631a8d14f6b58077278bd52be0d6d416d20) )
	ROM_LOAD32_BYTE( "x-reel_ecm_5_v103.u27", 0x000001, 0x80000, CRC(f203d41f) SHA1(4e666ffbb5a3a6545c89cbb4516c2e918b5a96f2) )
	ROM_LOAD32_BYTE( "x-reel_ecm_6_v103.u28", 0x000003, 0x80000, CRC(a9c39698) SHA1(dedc366dec836ad3c43146633850a702ca46f722) )

	ROM_REGION( 0x80000, "samples", 0 ) // SS9904
	ROM_LOAD( "subsino_qb-vi.u9", 0x000000, 0x80000, CRC(aa4edabb) SHA1(b117ad5bba2e410e20b5cbdb606688c6e2112450) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.q3", 0x00, 0x28, CRC(39bbd2c5) SHA1(52eb2fa124e176650015389e7b04eddc49ce6e8e) )
ROM_END


/***************************************************************************

Super Queen Bee
(c) 2002 Subsino

no ROM labels available

***************************************************************************/

ROM_START( squeenb )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "u21",                    0x080000, 0x040000, CRC(9edc4062) SHA1(515c8e648f839c99905fd5a861688fc62a45c4ed) )
	ROM_FILL(                           0x0c0000, 0x040000, 0xff )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "u25", 0x000000, 0x80000, CRC(842c0a33) SHA1(defb79c158d5091ca8830e9f03dda382d03d51ef) )
	ROM_LOAD32_BYTE( "u26", 0x000002, 0x80000, CRC(11b67abb) SHA1(e388e3aefbcceda1390c00e6590cbdd686982b2e) )
	ROM_LOAD32_BYTE( "u27", 0x000001, 0x80000, CRC(d713131a) SHA1(74a95e1ef0d30da53a91a5232574687f816df2eb) )
	ROM_LOAD32_BYTE( "u28", 0x000003, 0x80000, CRC(dfa39f39) SHA1(992f74c04cbf4af06a02812052ce701228d4e174) )

	ROM_REGION( 0x80000, "samples", 0 )
	ROM_LOAD( "u9", 0x000000, 0x80000, CRC(c7cda990) SHA1(193144fe0c31fc8342bd44aa4899bf15f0bc399d) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(c861db4a) SHA1(3109031239328a167f80082ec70b62630f8316ab) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( qbeebing )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "rom 2 27c040",           0x080000, 0x080000, CRC(03ea15cd) SHA1(19d3c3dd9e0c57066a6bd854964fd6a9f43c989f) )

	ROM_REGION( 0x400000, "tilemap", 0 )
	ROM_LOAD16_BYTE( "rom 4 27c160 3374h", 0x000001, 0x200000, CRC(a01527a0) SHA1(41ea384dd9c15c58246856f104b7dce68be1737c) )
	ROM_LOAD16_BYTE( "rom 3 27c160 08d7h", 0x000000, 0x200000, CRC(1fdf0fcb) SHA1(ed54172521f8d05bad37b670548106e4c4deb8af) )

	ROM_REGION( 0x80000, "samples", ROMREGION_ERASE00 ) // no samples, missing?

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(0d8db9ef) SHA1(eef0c8debbb2cb20af180c5c6a8ba998104fa24e) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( treamary )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044
	ROM_LOAD( "ss9689_6433044a22f.u16", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "27c040_u21.bin",         0x080000, 0x080000, CRC(b9163830) SHA1(853ccba636c4ee806602ca92a61d4c53ee3108b7) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "27c040_u25.bin", 0x000000, 0x80000, CRC(d17e5286) SHA1(a538a3b010eb0c7b5c16a4188f32f340fc890850) )
	ROM_LOAD32_BYTE( "27c040_u26.bin", 0x000002, 0x80000, CRC(fdc6c45b) SHA1(bb37badeba975630fb09b98104fbc757bd39538c) )
	ROM_LOAD32_BYTE( "27c040_u27.bin", 0x000001, 0x80000, CRC(dc3a477e) SHA1(6268872257f1b513b80a58a9e29861f3f2e2c177) )
	ROM_LOAD32_BYTE( "27c040_u28.bin", 0x000003, 0x80000, CRC(58d88d8d) SHA1(4551121691e958d280dfd437e47c6e331b66ede6) )

	ROM_REGION( 0x80000, "samples", 0 )
	ROM_LOAD( "27c040_u9.bin", 0x000000, 0x80000, CRC(5345ca39) SHA1(2b8f1dfeebb93a1d99c06912d89b268c642163df) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(0c068400) SHA1(7892443b04a987da944e36d6a528e1fdfbc68a39) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

Express Card / Top Card
(c) 1998 American Alpha

PCB:
  SUBSINO SFWO2

CPU:
  AMD Am188 EM-20KC? (@U10)
  Osc. 20.000000 MHz (@OSC20)
  ?????              (@U13) - 32k x 8 Bit High Speed CMOS Static RAM?

Video:
  Subsino SS9601? (@U16)
  Subsino SS9802  (@U1)
  Subsino SS9803? (@U29)
  HM86171-80 (@U26) - RAMDAC
  2 x HMC HM62H256AK-15 (@U21-U22) - 32k x 8 Low Voltage CMOS Static RAM

Sound:
  U6295 9838 (@U6)
  Osc. 8.4672 MHz (@X1)
  Philips TDA1519A (@U9) - 22 W BTL or 2 x 11 W stereo power amplifier

Other:
  Osc. 12.000 MHz (@OSC12)
  Battery (button cell)
  Reset switch (@SW5)
  Volume trimmer (@VR1)
  4 x DSW8 (@SW1-SW4, only SW1 is populated)
  4 x Toshiba TD62003A (@U3-U5) - High Voltage, High Current Darlington Transistor Arrays
  10 pin edge connector
  36 pin edge connector
  28 pin JAMMA connector

***************************************************************************/

ROM_START( expcard )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "top_card_alpha_1_v1.5.u14", 0x00000, 0x40000, CRC(c6de12fb) SHA1(e807880809dd71243caf993216d8d0baf5f678df) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "top_card_alpha_3_v1.4.u20", 0x00000, 0x80000, CRC(6e666d51) SHA1(924ac4fefa30cbe8bebe4f0d8ba6fff42fdd233e) )
	ROM_LOAD32_BYTE( "top_card_alpha_4_v1.4.u19", 0x00002, 0x80000, CRC(1382fd45) SHA1(1d81b7e72e702f5a254e1ec5ec6adb5d8af5d467) )
	ROM_LOAD32_BYTE( "top_card_alpha_5_v1.4.u18", 0x00001, 0x80000, CRC(bbe465ac) SHA1(7a5ee6f7696e5f768ac56ccfaf0914dd56a83339) )
	ROM_LOAD32_BYTE( "top_card_alpha_6_v1.4.u17", 0x00003, 0x80000, CRC(315d7a81) SHA1(8dafa1d422d8fe306765413084e35f16e4c17d27) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "top_card-ve1.u7", 0x00000, 0x80000, CRC(0ca9bd18) SHA1(af791c78ae321104afa738564bc23f520f37e7d5) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(622a8862) SHA1(fae60a326e6905aefc36275d505147e1860a71d0) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

  Magic Train
  -----------

  Board silkscreened: "SUBSINO" (logo), "CS186P012". Stickered "1056439".

  CPU:   1x Hitachi HD647180X0CP6 - 6D1R (Subsino - SS9600) (U23).
  SND:   1x U6295 (OKI compatible) (U25).
         1x TDA1519A (PHILIPS, 22W BTL or 2x 11W stereo car radio power amplifier (U34).

  NVRAM:     1x SANYO LC36256AML (SMD) (U16).
  VRAM:      2x UMC UM62256E-70LL (U7-U8, next to gfx ROMs).
  Other RAM: 1x HMC HM86171-80 (U29, next to sound ROM).

  Video: Subsino (SMD-40PX40P) SS9601 - 9732WX011 (U1).
  I/O:   Subsino (SMD-30PX20P) SS9602 - 9732LX006 (U11).

  PRG ROM:  Stickered "M-TRAIN-N OUT_1 V1.31".

  GFX ROMs: 1x 27C2000DC-12  Stickered "M-TRAIN-N ROM_1 V1.0" (U5).
            1x 27C2000DC-12  Stickered "M-TRAIN-N ROM_2 V1.0" (U4).
            1x 27C2000DC-12  Stickered "M-TRAIN-N ROM_3 V1.0" (U3).
            1x 27C2000DC-12  Stickered "M-TRAIN-N ROM_4 V1.0" (U2).

  SND ROM:  1x 27C2000DC-12 (U27, no sticker).

  PLDs: 1x GAL16V8D (U31, next to sound ROM).
        3x GAL16V8D (U18-U19-U6, next to CPU, program ROM and NVRAM).
        1x GAL16V8D (U26, near sound amp)

  Battery: 1x VARTA 3.6v, 60mAh.

  Xtal: 12 MHz.

  4x 8 DIP switches banks (SW1-SW2-SW3-SW4).
  1x Push button (S1, next to battery).

  1x 2x36 Edge connector.
  1x 2x10 Edge connector.

  U12, U13 & U14 are Darlington arrays.

***************************************************************************/

ROM_START( mtrain )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	// code starts at 0x8100!
	ROM_LOAD( "v1.4_27c512.u17", 0x00000, 0x10000, CRC(38d5340d) SHA1(6c32b7cd42e2ad0ad56ac308d008d649ebb10684) )  // year 1997 inside

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "v1.4_27c2001.u2", 0x00000, 0x40000, CRC(b7e65d04) SHA1(5eea1b8c1129963b3b83a59410cd0e1de70621e4) )
	ROM_LOAD32_BYTE( "v1.4_27c2001.u3", 0x00002, 0x40000, CRC(cef2c079) SHA1(9ee54a08ef8db90a80a4b3568bb82ce09ee41e65) )
	ROM_LOAD32_BYTE( "v1.4_27c2001.u4", 0x00001, 0x40000, CRC(a794f287) SHA1(7b9c0d57224a700f49e55ba5aeb7ed9d35a71e02) )
	ROM_LOAD32_BYTE( "v1.4_27c2001.u5", 0x00003, 0x40000, CRC(96067e95) SHA1(bec7dffaf6920ff2bd85a43fb001a997583e25ee) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "v1.4_27c2001.u27", 0x00000, 0x40000, CRC(51cae476) SHA1(d1da4e5c3d53d18d8b69dfb57796d0ae311d99bf) )
	ROM_RELOAD(                   0x40000, 0x40000 )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8d.u6",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u18", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u19", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u26", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u31", 0x000, 0x117, NO_DUMP )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(a73211f7) SHA1(ebe175b9b8ea3fffcc9dd03ea51ccef36b016eb8) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( mtraina )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	// code starts at 0x8100!
	ROM_LOAD( "out_1v131.u17", 0x00000, 0x10000, CRC(6761be7f) SHA1(a492f8179d461a454516dde33ff04473d4cfbb27) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "rom_4.u02", 0x00000, 0x40000, CRC(b7e65d04) SHA1(5eea1b8c1129963b3b83a59410cd0e1de70621e4) )
	ROM_LOAD32_BYTE( "rom_3.u03", 0x00002, 0x40000, CRC(cef2c079) SHA1(9ee54a08ef8db90a80a4b3568bb82ce09ee41e65) )
	ROM_LOAD32_BYTE( "rom_2.u04", 0x00001, 0x40000, CRC(a794f287) SHA1(7b9c0d57224a700f49e55ba5aeb7ed9d35a71e02) )
	ROM_LOAD32_BYTE( "rom_1.u05", 0x00003, 0x40000, CRC(96067e95) SHA1(bec7dffaf6920ff2bd85a43fb001a997583e25ee) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom_5.u27", 0x00000, 0x40000, CRC(51cae476) SHA1(d1da4e5c3d53d18d8b69dfb57796d0ae311d99bf) )
	ROM_RELOAD(            0x40000, 0x40000 )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8d.u6",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u18", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u19", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u26", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u31", 0x000, 0x117, NO_DUMP )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(a73211f7) SHA1(ebe175b9b8ea3fffcc9dd03ea51ccef36b016eb8) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( strain )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	// code starts at 0x8100!
	ROM_LOAD( "v1.9_27c512_u17.bin", 0x00000, 0x10000, CRC(36379ab2) SHA1(b48374f80ffa107a7ea3e08eb432259e443dc4a6) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "v1.0_mx27c4000_u2.bin", 0x00000, 0x80000, CRC(0b77b3be) SHA1(daf1180cabce3e1bbb9a8f91c02e0fe4f0fd811e) )
	ROM_LOAD32_BYTE( "v1.0_mx27c4000_u3.bin", 0x00002, 0x80000, CRC(c003661d) SHA1(49d76a9273928c35dcd6a6ab114d798f5553d79a) )
	ROM_LOAD32_BYTE( "v1.0_mx27c4000_u4.bin", 0x00001, 0x80000, CRC(6392f562) SHA1(83881ec85a3dff82f32214b2654ee79e5e9a2d2a) )
	ROM_LOAD32_BYTE( "v1.0_mx27c4000_u5.bin", 0x00003, 0x80000, CRC(85abe66c) SHA1(32698faf75bd0c42ab99b0c53b3ffa0891eedaca) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "v1.0_mx27c4000_u27.bin", 0x00000, 0x80000, CRC(d5bbebc7) SHA1(59fa804caa991dc2ad7f735b7c171defd836140a) )

	ROM_REGION( 0x117, "plds", ROMREGION_ERASE00 )
	// TODO: list these

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(133705eb) SHA1(974b7fd5f7eaa84c4ba2a5ba9e014ac459fa7d23) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

  Decryption of mtrain (same as crsbingo)

  Notes:

  0000-8100? is not encrypted

  rom     addr
  8100 -> 0000  ; after decryption (code start)
  8200 -> 0100  ; after decryption
  8d97 -> 0c97  ; after decryption
  1346 -> b346
  ec40 -> 6b40  ; after decryption

***************************************************************************/

void subsino2_state::init_mtrain()
{
	subsino_decrypt(memregion("program")->base() + 0x8100, crsbingo_bitswaps, crsbingo_xors, 0x7f00);
}



ROM_START( tbonusal )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x1a000, "program", 0 )
	// code starts at 0x8100
	ROM_LOAD( "n-alpha 1.6-u17.bin", 0x00000, 0x10000, CRC(1bdc1c92) SHA1(2cd7ec5a89865b76df2cfe9d18b2ab42923f8def) )

	// there is a clear HD647180X0FS6 on the PCB, but is it operating in external mode? there is a program rom above at least
	// if the internal ROM is unused / irrelevant then this doesn't need to be marked
	//ROM_REGION( 0x10000, "mcu", 0 )
	//ROM_LOAD( "hd647180", 0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "n-alpha 1.6-u2.bin", 0x000000, 0x80000, CRC(392943be) SHA1(776d36a94b8a70ec2eaf88dfd2804517656b53a0) )
	ROM_LOAD32_BYTE( "n-alpha 1.6-u3.bin", 0x000002, 0x80000, CRC(24c8f62e) SHA1(692a96f233d8576a6921bfe23c97502ff26c62db) )
	ROM_LOAD32_BYTE( "n-alpha 1.6-u4.bin", 0x000001, 0x80000, CRC(bed035a9) SHA1(6b141bb8fb7969338faa702bd03970331bbbe6e1) )
	ROM_LOAD32_BYTE( "n-alpha 1.6-u5.bin", 0x000003, 0x80000, CRC(d00d48c6) SHA1(28b505a3f07c5d5bb8e8609c6d6e883260594588) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASE00 )
	// not populated on 4 different PCBs

	ROM_REGION( 0x117, "plds", ROMREGION_ERASEFF )
	// TODO list of GALs

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(7c832409) SHA1(fe16074490fe4edab2be2de5fa83941dac9969b0) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

void subsino2_state::init_tbonusal()
{
	subsino_decrypt(memregion("program")->base() + 0x8100, sharkpy_bitswaps, sharkpy_xors, 0x7f00);
}


/***************************************************************************

Sakura Love
Subsino, 1998

PCB Layout
----------

|------------------------------------|
|     ULN2003 ULN2003 ULN2003 ULN2003|
|LM358     HM86171     |-----|    PAL|
| M6295       8.4672MHz|SS9602    PAL|
| LM324                |-----|    PAL|
|J              |-------|         PAL|
|A              |SUBSINO|         PAL|
|M  2   12MHz   |SS9601 |         SW5|
|M  3   20MHz   |       |            |
|A  4           |-------|            |
|   5        1  LC36256              |
|   6      AM188-EM    62256  SW3 SW4|
|BATTERY               62256  SW1 SW2|
|------------------------------------|
Notes:
      AM188-EM - AMD AM188 Main CPU (QFP100) Clock 20.0MHz
      M6295    - clock 1.0584MHz [8.4672/8]. Pin 7 HIGH
      SW1-SW4  - Dip switches with 8 positions
      SW5      - Reset switch
      VSync    - 58.7270Hz
      HSync    - 15.3234kHz

***************************************************************************/

ROM_START( saklove )
	ROM_REGION( 0x20000, "maincpu", 0 ) // AM188-EM
	ROM_LOAD( "1.u23", 0x00000, 0x20000, CRC(02319bfb) SHA1(1a425dcdeecae92d8b7457d1897c700ac7856a9d) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "3.u27", 0x000000, 0x80000, CRC(01aa8fbd) SHA1(d1d19ef52c8077ccf17cc2fde96fd56c626e33db) )
	ROM_LOAD32_BYTE( "4.u28", 0x000002, 0x80000, CRC(f8db7ab6) SHA1(3af4e92ab27edc980eccecdbbbb431e1d2101059) )
	ROM_LOAD32_BYTE( "5.u29", 0x000001, 0x80000, CRC(c6ca1764) SHA1(92bfa19e116d358b03164f2448a28e7524e3cc62) )
	ROM_LOAD32_BYTE( "6.u30", 0x000003, 0x80000, CRC(5823c39e) SHA1(257e862ac736ff403ce9c70bbdeed340dfe168af) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "2.u10", 0x00000, 0x80000, CRC(4f70125c) SHA1(edd5e6bd47b9a4fa3c4057cb4a85544241fe483d) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.q3", 0x00, 0x28, CRC(225136fb) SHA1(86095f7c98b579282605730de820d17ac2c8a141) )
ROM_END


/***************************************************************************

X-Plan
(c) 2006 Subsino

PCB:
  SUBSINO SFWO2

CPU:
  AMD Am188 EM-20KC F 9912F6B (@U10)
  Osc. 20.00000 MHz (@OSC20)
  Sony CXK58257AM-10L (@U13) - 32k x 8 Bit High Speed CMOS Static RAM

Video:
  Subsino SS9601 0035WK007 (@U16)
  Subsino SS9802 0448 (@U1)
  Subsino SS9803 0020 (@U29)
  HM86171-80 (@U26) - RAMDAC
  2 x Winbond W24M257AK-15 (@U21-U22) - 32k x 8 Low Voltage CMOS Static RAM

Sound:
  U6295 0214 B923826 (@U6)
  Osc. 8.4672 MHz (@X1)
  Philips TDA1519A? (@U9) - 22 W BTL or 2 x 11 W stereo power amplifier

Other:
  Osc. 12.000 MHz (@OSC12)
  Battery (button cell)
  Reset switch (@SW5)
  Volume trimmer (@VR1)
  4 x DSW8 (@SW1-SW4, only SW1 is populated)
  4 x ULN2003A (@U3-U5) - High Voltage, High Current Darlington Transistor Arrays
  10 pin edge connector
  36 pin edge connector
  28 pin JAMMA connector

***************************************************************************/

ROM_START( xplan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "x-plan_v101.u14", 0x00000, 0x40000, CRC(5a05fcb3) SHA1(9dffffd868e777f9436c38df76fa5247f4dd6daf) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "x-plan_rom_3_v102b.u20", 0x00000, 0x80000, CRC(a027cbd1) SHA1(dac4226014794ef5bff84ddafee7da6691c00ece) )
	ROM_LOAD32_BYTE( "x-plan_rom_4_v102b.u19", 0x00002, 0x80000, CRC(744be318) SHA1(1c1f2a9e1da77d9bc1bf897072df44a681a53079) )
	ROM_LOAD32_BYTE( "x-plan_rom_5_v102b.u18", 0x00001, 0x80000, CRC(7e89c9b3) SHA1(9e3fea0d74cac48c068a15595f2342a2b0b3f747) )
	ROM_LOAD32_BYTE( "x-plan_rom_6_v102b.u17", 0x00003, 0x80000, CRC(a86ca3b9) SHA1(46aa86b9c62aa0a4e519eb06c72c2d540489afee) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "x-plan_rom_2_v100.u7", 0x00000, 0x80000, CRC(c742b5c8) SHA1(646960508be738824bfc578c1b21355c17e05010) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(ac70474d) SHA1(120362665af4ab361197795c6be51c8fed5a3506) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

X-Train
(c) 1999 Subsino

PCB:
  SUBSINO

CPU:
  AMD Am188 EM-20KC (@U10)
  Osc. 20.000 MHz (@OSC20)
  Sony CXK58257AM-10L (@U13) - 32k x 8 Bit High Speed CMOS Static RAM

Video:
  Subsino SS9601 9948WK007 (@U16)
  Subsino SS9802 9933 (@U1)
  Subsino SS9803 9933 (@U29)
  HM86171-80 (@U26) - RAMDAC
  2 x Alliance AS7C256-15PC (@U21-U22) - High Performance 32Kx8 CMOS SRAM

Sound:
  U6295 (@U6)
  Osc. 8.4672 MHz (@X1)
  Philips TDA1519A (@U9) - 22 W BTL or 2 x 11 W stereo power amplifier

Other:
  Osc. 12.000 MHz (@OSC12)
  Battery (button cell)
  Reset switch (@SW5)
  Volume trimmer (@VR1)
  4 x DSW8 (@SW1-SW4, only SW1 is populated)
  4 x ULN2003A? (@U3-U5) - High Voltage, High Current Darlington Transistor Arrays
  10 pin edge connector
  36 pin edge connector
  28 pin JAMMA connector

***************************************************************************/

ROM_START( xtrain )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "x-train=top=_out_1_v1.3.u14", 0x00000, 0x40000, CRC(019812b4) SHA1(33c73c53f8cf730c35fa310868f5b8360dfaad9e) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "x-train_rom_3_v1.2.u20", 0x00000, 0x80000, CRC(0e18ca82) SHA1(8fbc62a16ab109994086f58c9b9915a92bda0448) )
	ROM_LOAD32_BYTE( "x-train_rom_4_v1.2.u19", 0x00002, 0x80000, CRC(959fa749) SHA1(d39fcedd1d13d9f86c1915d7dcff7d024739a6fa) )
	ROM_LOAD32_BYTE( "x-train_rom_5_v1.2.u18", 0x00001, 0x80000, CRC(d0e8279f) SHA1(174483871c9e98936b37cc6cede71b64e19cae90) )
	ROM_LOAD32_BYTE( "x-train_rom_6_v1.2.u17", 0x00003, 0x80000, CRC(289ae881) SHA1(b3f8db43d86078688ad56a04d1e7d7a825df60d7) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "x-train_rom_2_v1.2.u7", 0x00000, 0x80000, CRC(aae563ff) SHA1(97db845d7e3d343bd70352371cb27b16faacca7f) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(9c5973b7) SHA1(ba79b2971cfa5d0183b1be5d54c5e7f13f0e8243) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

Panda Train (Novamatic 1.7)
(c) 1999 Subsino

Note: It's the same hardware as X-Train

PCB:
  SUBSINO

CPU:
  AMD Am188 EM-20KC (@U10)
  Osc. 20.000 MHz (@OSC20)
  MB84256C-10L (@U13)

Video:
  Subsino SS9601 9901WK002 (@U16)
  Subsino SS9802 (@U1)
  Subsino SS9803 (@U29)
  HM86171-80 (@U26) - RAMDAC
  2 x HM62H256DK-12 (@U21-U22)

Sound:
  U6295 (@U6)
  Osc. 8.4672 MHz (@X1)
  Philips TDA1519 (@U9)
  LM324 (@U8) - Quad Operational Amplifier

Other:
  Osc. 12.000 MHz (@OSC12)
  Battery (3V button cell)
  Reset switch (@SW5)
  Volume trimmer (@VR1)
  4 x DSW8 (@SW1-SW4, only SW1 is populated)
  4 x ULN2003A? (@U3-U5) - High Voltage, High Current Darlington Transistor Arrays
  10 pin edge connector
  36 pin edge connector
  28 pin JAMMA connector

***************************************************************************/

ROM_START( ptrain )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "panda=top=-novam_1-v1.4.u14", 0x00000, 0x40000, CRC(75b12734) SHA1(d05d0cba2de9d7021736bbd7c67d9b3c552374ee) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "panda-novam_3-v1.4.0.u20", 0x00000, 0x80000, CRC(2d5ab471) SHA1(3df42b7f762d738a4409498984e90c80625fae1f) )
	ROM_LOAD32_BYTE( "panda-novam_4-v1.4.1.u19", 0x00002, 0x80000, CRC(a4b6985c) SHA1(1d3d23f7c9e775439a2d1a4c68b703bf51b0350f) )
	ROM_LOAD32_BYTE( "panda-novam_5-v1.4.2.u18", 0x00001, 0x80000, CRC(716f7500) SHA1(971589a2530a0d4152bb68dbc7794985525a837d) )
	ROM_LOAD32_BYTE( "panda-novam_6-v1.4.3.u17", 0x00003, 0x80000, CRC(10f0c21a) SHA1(400e53bf3dd6fe6f2dd679ed5151fb4400a6ec9f) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "panda-novam_2-v1.4.u7", 0x00000, 0x80000, CRC(d1debec8) SHA1(9086975e5bef2066a688ab3c1df3b384f59e507d) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(a19d7b78) SHA1(e32a33a953d2523a558c395debbf85ee1df8965b) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( ptraina )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "27c020.u14", 0x00000, 0x40000, CRC(618123a6) SHA1(6c52797c1af264bab0fb28686e15fa8ff756d4b9) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "27c040.u20", 0x00000, 0x80000, CRC(d2b07a34) SHA1(a1f76545bdede0f48d26782960d1f0f666dbe5b8) )
	ROM_LOAD32_BYTE( "27c040.u19", 0x00002, 0x80000, CRC(304c992b) SHA1(fe6f6b2ba33eb81177387a8d65555f88ab87648d) )
	ROM_LOAD32_BYTE( "27c040.u18", 0x00001, 0x80000, CRC(7faaeba0) SHA1(ac7c82eed27f444cffd8feb37f12acbeb2a448b9) )
	ROM_LOAD32_BYTE( "27c040.u17", 0x00003, 0x80000, CRC(71478da4) SHA1(ee5bed03249805211e0500a8df3aae7987db9f9c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "27c040.u7", 0x00000, 0x80000, CRC(d1debec8) SHA1(9086975e5bef2066a688ab3c1df3b384f59e507d) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(4c2fdd04) SHA1(1a173524f37275944b5cc5bc6b85a2b4a6043629) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

Treasure City

unknown hardware

***************************************************************************/

ROM_START( treacity )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "alpha 208_27c1001_u33.bin", 0x00000, 0x20000, CRC(e743aac3) SHA1(762575000463a126df561c959dfa06180e955822) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "alpha 207_27c4001_u7.bin",  0x00000, 0x80000, CRC(88d4d1f2) SHA1(35bc70904ceadeb7b1ccc35bb92585419da50fe1) )
	ROM_LOAD32_BYTE( "alpha 207_27c4001_u8.bin",  0x00002, 0x80000, CRC(7140638f) SHA1(a6072286b453e1290b2fc46060a0d777ad4ae1a8) )
	ROM_LOAD32_BYTE( "alpha 207_27c4001_u9.bin",  0x00001, 0x80000, CRC(57241f44) SHA1(f055488710ae624c1c7e92b2adf6b497c72514ea) )
	ROM_LOAD32_BYTE( "alpha 207_27c4001_u10.bin", 0x00003, 0x80000, CRC(338370f9) SHA1(0e06ed1b71fb44bfd617f4d5112f6d34f0b759bc) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASE00 ) // samples, missing or not used / other hardware here?

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(8c9906fd) SHA1(8afaaf80dbaf5d9763da5fa0c6f95d20887bc336) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( treacity202 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "alpha 202_27c1001_u33.bin", 0x00000, 0x20000, CRC(1a698c3d) SHA1(c2107b67d86783b04d1ebdf78d1f358916c51219) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "alpha 142_27c4001_u7.bin",  0x00000, 0x80000, CRC(c8e4e4d3) SHA1(b5dabfe2e8e5a19d218e3d58bbebbe83803feb23) )
	ROM_LOAD32_BYTE( "alpha 142_27c4001_u8.bin",  0x00002, 0x80000, CRC(a8fb65b4) SHA1(047fa2ccd08ce5282c015239f0f22d0ba20ea67b) )
	ROM_LOAD32_BYTE( "alpha 142_27c4001_u9.bin",  0x00001, 0x80000, CRC(b0c50891) SHA1(66ebebc327e00d5e8e9eb0a427d34683c4cca8aa) )
	ROM_LOAD32_BYTE( "alpha 142_27c4001_u10.bin", 0x00003, 0x80000, CRC(8545e8cd) SHA1(0d122a532df81fe2150c1eaf49b5a4e35c8134eb) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASE00 ) // samples, missing or not used / other hardware here?

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(8c9906fd) SHA1(8afaaf80dbaf5d9763da5fa0c6f95d20887bc336) BAD_DUMP ) // handcrafted to pass protection check
ROM_END


/***************************************************************************

Water-Nymph (Ver. 1.4)
(c) 1996 Subsino

Same PCB as Magic Train

***************************************************************************/

ROM_START( wtrnymph )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	// code starts at 0x8100!
	ROM_LOAD( "ocean-n tetris_1 v1.4.u17", 0x00000, 0x10000, CRC(c7499123) SHA1(39a9ea6d927ee839cfb127747e5e3df3535af098) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "ocean-n tetris_2 v1.21.u2", 0x00000, 0x40000, CRC(813aac90) SHA1(4555adf8dc363359b10f1d5cfae2dcebed411679) )
	ROM_LOAD32_BYTE( "ocean-n tetris_3 v1.21.u3", 0x00002, 0x40000, CRC(83c39379) SHA1(e7f9315d19370c18b664b759e433052a88f8c146) )
	ROM_LOAD32_BYTE( "ocean-n tetris_4 v1.21.u4", 0x00001, 0x40000, CRC(6fc64b42) SHA1(80110d7dae28cca5e39c8a7c2ceebf589116ae23) )
	ROM_LOAD32_BYTE( "ocean-n tetris_5 v1.21.u5", 0x00003, 0x40000, CRC(8c7515ee) SHA1(a67b21c1e8ca8a098fe558c73561bca13962893e) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ocean-n tetris_6 v1.21.u27", 0x00000, 0x40000, CRC(1c8a886d) SHA1(faa983801b368a6d04ef80e359c6fb67b240c60d) )
	ROM_RELOAD(                             0x40000, 0x40000 )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8d.u6",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u18", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u19", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u26", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d.u31", 0x000, 0x117, NO_DUMP )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(1b585f27) SHA1(ee89dfc731d867507c15009910e9f743c652a399) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

void subsino2_state::init_wtrnymph()
{
	subsino_decrypt(memregion("program")->base() + 0x8100, victor5_bitswaps, victor5_xors, 0x7f00);
}


GAME( 1997, mtrain,      0,        mtrain,   mtrain,   subsino2_state, init_mtrain,   ROT0, "Subsino",                          "Magic Train (Ver. 1.4)",                0 )  // inside the program ROM says 1997, but on screen shows 1996
GAME( 1996, mtraina,     mtrain,   mtrain,   mtrain,   subsino2_state, init_mtrain,   ROT0, "Subsino",                          "Magic Train (Ver. 1.31)",               0 )

GAME( 1996, strain,      0,        mtrain,   strain,   subsino2_state, init_mtrain,   ROT0, "Subsino",                          "Super Train (Ver. 1.9)",                MACHINE_NOT_WORKING )

GAME( 1995, tbonusal,    0,        tbonusal, tbonusal, subsino2_state, init_tbonusal, ROT0, "Subsino (American Alpha license)", "Treasure Bonus (American Alpha, Ver. 1.6)", MACHINE_NOT_WORKING )

GAME( 1996, wtrnymph,    0,        mtrain,   wtrnymph, subsino2_state, init_wtrnymph, ROT0, "Subsino",                          "Water-Nymph (Ver. 1.4)",                0 )

GAME( 1998, expcard,     0,        expcard,  expcard,  subsino2_state, empty_init,    ROT0, "Subsino (American Alpha license)", "Express Card / Top Card (Ver. 1.5)",    0 )

GAME( 1998, saklove,     0,        saklove,  saklove,  subsino2_state, empty_init,    ROT0, "Subsino",                          "Ying Hua Lian 2.0 (China, Ver. 1.02)",  0 )

GAME( 1999, xtrain,      0,        xtrain,   xtrain,   subsino2_state, empty_init,    ROT0, "Subsino",                          "X-Train (Ver. 1.3)",                    0 )

GAME( 1999, ptrain,      0,        ptrain,   ptrain,   subsino2_state, empty_init,    ROT0, "Subsino",                          "Panda Train (Novamatic 1.7)",           MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, ptraina,     ptrain,   ptrain,   ptrain,   subsino2_state, empty_init,    ROT0, "Subsino",                          "Panda Train (Ver. 1.3)",                MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, treacity,    0,        saklove,  treacity, subsino2_state, empty_init,    ROT0, "Subsino (American Alpha license)", "Treasure City (Ver. 208)",              MACHINE_NOT_WORKING )
GAME( 1997, treacity202, treacity, saklove,  treacity, subsino2_state, empty_init,    ROT0, "Subsino (American Alpha license)", "Treasure City (Ver. 202)",              MACHINE_NOT_WORKING )

GAME( 1999, bishjan,     0,        bishjan,  bishjan,  subsino2_state, empty_init,    ROT0, "Subsino",                          "Bishou Jan (Japan, Ver. 203)",          MACHINE_NO_SOUND )
GAME( 1999, xiaoao,      bishjan,  xiaoao,   bishjan,  subsino2_state, empty_init,    ROT0, "Subsino",                          "Xiao Ao Jiang Hu (China, Ver. 1.00)",   MACHINE_NO_SOUND )

GAME( 2000, new2001,     0,        new2001,  new2001,  subsino2_state, empty_init,    ROT0, "Subsino",                          "New 2001 (Italy, Ver. 200N)",           MACHINE_NO_SOUND )

GAME( 2006, xplan,       0,        xplan,    xplan,    subsino2_state, empty_init,    ROT0, "Subsino",                          "X-Plan (Ver. 101)",                     MACHINE_NOT_WORKING )

GAME( 2001, queenbee,    0,        humlan,   queenbee, subsino2_state, empty_init,    ROT0, "Subsino (American Alpha license)", "Queen Bee (Ver. 114)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // severe timing issues
GAME( 2001, queenbeeb,   queenbee, humlan,   queenbee, subsino2_state, empty_init,    ROT0, "Subsino",                          "Queen Bee (Brazil, Ver. 202)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // severe timing issues, only program ROM available
GAME( 2001, queenbeei,   queenbee, humlan,   queenbee, subsino2_state, empty_init,    ROT0, "Subsino",                          "Queen Bee (Israel, Ver. 100)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // severe timing issues, only program ROM available
GAME( 2001, queenbeesa,  queenbee, humlan,   queenbee, subsino2_state, empty_init,    ROT0, "Subsino",                          "Queen Bee (SA-101-HARD)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // severe timing issues, only program ROM available

GAME( 2001, humlan,      queenbee, humlan,   humlan,   subsino2_state, empty_init,    ROT0, "Subsino (Truemax license)",        "Humlan's Lyckohjul (Sweden, Ver. 402)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // severe timing issues

GAME( 2002, xreel,       queenbee, humlan,   humlan,   subsino2_state, empty_init,    ROT0, "Subsino (ECM license)",            "X-Reel",                                MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // severe timing issues

GAME( 2002, squeenb,     0,        humlan,   humlan,   subsino2_state, empty_init,    ROT0, "Subsino",                          "Super Queen Bee (Ver. 101)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // severe timing issues

GAME( 2003, qbeebing,    0,        humlan,   humlan,   subsino2_state, empty_init,    ROT0, "Subsino",                          "Queen Bee Bingo",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

GAME( 200?, treamary,    0,        bishjan,  bishjan,  subsino2_state, empty_init,    ROT0, "Subsino",                          "Treasure Mary",                         MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
