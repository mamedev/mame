// license:BSD-3-Clause
// copyright-holders:Luca Elia
/************************************************************************************************************

                                 -= Subsino (Newer) Tilemaps Hardware =-

                                  driver by   Luca Elia (l.elia@tin.it)


Two 1024x512 tilemaps. 256 color tiles. Tiles are 8x8 or a multiple (dynamic tile size).
There is RAM for 512 scroll values (line scroll). Video RAM is mirrored on multiple ranges.
One peculiarity is that video RAM access is split into high and low byte. The former is mapped
in program space, the latter in I/O space.

-------------------------------------------------------------------------------------------------------------
Year  Game            CPU         Sound            Custom                            Other
-------------------------------------------------------------------------------------------------------------
1996  Magic Train     HD647180*   U6295            SS9601, SS9602                    HM86171 RAMDAC, Battery
1996  Water-Nymph     HD647180*   U6295            SS9601, SS9602                    HM86171 RAMDAC, Battery
1998  Express Card    AM188-EM    M6295            SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
1998  Ying Hua Lian   AM188-EM    M6295 + YM3812?  SS9601, SS9602                    HM86171 RAMDAC, Battery
1999  Bishou Jan      H8/3044     SS9904?          SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
1999  X-Train         AM188-EM    M6295            SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
2006  X-Plan          AM188-EM    M6295            SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
-------------------------------------------------------------------------------------------------------------
*SS9600

To do:

- Implement serial communication (used for protection).
- saklove, xplan: remove IRQ hacks (when an AM188-EM core will be available).
- bishjan, saklove: game is sometimes too fast (can bishjan read the VBLANK state? saklove and xplan can).
- bishjan: add sound (does it send sound commands to another device? SS9904?).
- xtrain: it runs faster than a video from the real thing. It doesn't use vblank irqs (but reads the vblank bit).
- mtrain: implement hopper. Double up does not work?

************************************************************************************************************/

#include "emu.h"
#include "cpu/h8/h83048.h"
#include "cpu/i86/i186.h"
#include "cpu/z180/z180.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "machine/subsino.h"
#include "machine/ticket.h"


enum tilesize_t
{
	TILE_8x8,
	TILE_8x32,
	TILE_64x32
};

enum vram_t
{
	VRAM_LO,
	VRAM_HI
};


// Layers
struct layer_t
{
	UINT8 *videorams[2];

	UINT8 *scrollrams[2];
	int scroll_x;
	int scroll_y;

	tilemap_t *tmap;
	tilesize_t tilesize;

};

class subsino2_state : public driver_device
{
public:
	subsino2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_outputs16(*this, "outputs16"),
		m_outputs(*this, "outputs"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	UINT8 *m_hm86171_colorram;
	layer_t m_layers[2];
	UINT8 m_ss9601_byte_lo;
	UINT8 m_ss9601_byte_lo2;
	UINT8 *m_ss9601_reelrams[2];
	rectangle m_ss9601_reelrects[3];
	UINT8 m_ss9601_scrollctrl;
	UINT8 m_ss9601_tilesize;
	UINT8 m_ss9601_disable;
	int m_hm86171_offs;
	UINT8 m_dsw_mask;
	optional_shared_ptr<UINT16> m_outputs16;
	optional_shared_ptr<UINT8> m_outputs;
	UINT16 m_bishjan_sel;
	UINT16 m_bishjan_input;
	DECLARE_WRITE8_MEMBER(ss9601_byte_lo_w);
	DECLARE_WRITE8_MEMBER(ss9601_byte_lo2_w);
	DECLARE_WRITE8_MEMBER(ss9601_videoram_0_hi_w);
	DECLARE_WRITE8_MEMBER(ss9601_videoram_0_lo_w);
	DECLARE_WRITE8_MEMBER(ss9601_videoram_0_hi_lo_w);
	DECLARE_WRITE8_MEMBER(ss9601_videoram_0_hi_lo2_w);
	DECLARE_READ8_MEMBER(ss9601_videoram_0_hi_r);
	DECLARE_READ8_MEMBER(ss9601_videoram_0_lo_r);
	DECLARE_WRITE8_MEMBER(ss9601_videoram_1_hi_w);
	DECLARE_WRITE8_MEMBER(ss9601_videoram_1_lo_w);
	DECLARE_WRITE8_MEMBER(ss9601_videoram_1_hi_lo_w);
	DECLARE_WRITE8_MEMBER(ss9601_videoram_1_hi_lo2_w);
	DECLARE_READ8_MEMBER(ss9601_videoram_1_hi_r);
	DECLARE_READ8_MEMBER(ss9601_videoram_1_lo_r);
	DECLARE_WRITE8_MEMBER(ss9601_reelram_hi_lo_w);
	DECLARE_READ8_MEMBER(ss9601_reelram_hi_r);
	DECLARE_READ8_MEMBER(ss9601_reelram_lo_r);
	DECLARE_WRITE8_MEMBER(ss9601_scrollctrl_w);
	DECLARE_WRITE8_MEMBER(ss9601_tilesize_w);
	DECLARE_WRITE8_MEMBER(ss9601_scroll_w);
	DECLARE_WRITE8_MEMBER(ss9601_scrollram_0_hi_w);
	DECLARE_WRITE8_MEMBER(ss9601_scrollram_0_lo_w);
	DECLARE_WRITE8_MEMBER(ss9601_scrollram_0_hi_lo_w);
	DECLARE_READ8_MEMBER(ss9601_scrollram_0_hi_r);
	DECLARE_READ8_MEMBER(ss9601_scrollram_0_lo_r);
	DECLARE_WRITE8_MEMBER(ss9601_scrollram_1_hi_w);
	DECLARE_WRITE8_MEMBER(ss9601_scrollram_1_lo_w);
	DECLARE_WRITE8_MEMBER(ss9601_scrollram_1_hi_lo_w);
	DECLARE_READ8_MEMBER(ss9601_scrollram_1_hi_r);
	DECLARE_READ8_MEMBER(ss9601_scrollram_1_lo_r);
	DECLARE_WRITE8_MEMBER(ss9601_disable_w);
	DECLARE_WRITE8_MEMBER(hm86171_colorram_w);
	DECLARE_WRITE8_MEMBER(dsw_mask_w);
	DECLARE_READ8_MEMBER(dsw_r);
	DECLARE_READ8_MEMBER(vblank_bit2_r);
	DECLARE_READ8_MEMBER(vblank_bit6_r);
	DECLARE_WRITE16_MEMBER(bishjan_sel_w);
	DECLARE_READ16_MEMBER(bishjan_serial_r);
	DECLARE_WRITE16_MEMBER(bishjan_input_w);
	DECLARE_READ16_MEMBER(bishjan_input_r);
	DECLARE_WRITE16_MEMBER(bishjan_outputs_w);
	DECLARE_WRITE8_MEMBER(expcard_outputs_w);
	DECLARE_WRITE8_MEMBER(mtrain_outputs_w);
	DECLARE_WRITE8_MEMBER(mtrain_videoram_w);
	DECLARE_WRITE8_MEMBER(mtrain_tilesize_w);
	DECLARE_READ8_MEMBER(mtrain_prot_r);
	DECLARE_WRITE8_MEMBER(saklove_outputs_w);
	DECLARE_WRITE8_MEMBER(xplan_outputs_w);
	DECLARE_WRITE8_MEMBER(xtrain_outputs_w);
	DECLARE_WRITE8_MEMBER(oki_bank_bit0_w);
	DECLARE_WRITE8_MEMBER(oki_bank_bit4_w);
	DECLARE_DRIVER_INIT(bishjan);
	DECLARE_DRIVER_INIT(xtrain);
	DECLARE_DRIVER_INIT(expcard);
	DECLARE_DRIVER_INIT(wtrnymph);
	DECLARE_DRIVER_INIT(mtrain);
	DECLARE_DRIVER_INIT(saklove);
	DECLARE_DRIVER_INIT(xplan);
	DECLARE_DRIVER_INIT(ptrain);
	TILE_GET_INFO_MEMBER(ss9601_get_tile_info_0);
	TILE_GET_INFO_MEMBER(ss9601_get_tile_info_1);
	DECLARE_VIDEO_START(subsino2);
	DECLARE_VIDEO_START(mtrain);
	DECLARE_VIDEO_START(xtrain);
	UINT32 screen_update_subsino2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(h8_timer_irq);
	INTERRUPT_GEN_MEMBER(am188em_int0_irq);
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

private:
	inline void ss9601_get_tile_info(layer_t *l, tile_data &tileinfo, tilemap_memory_index tile_index);
};



/***************************************************************************
                              Tilemaps Access
***************************************************************************/

inline void subsino2_state::ss9601_get_tile_info(layer_t *l, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int addr;
	UINT16 offs;
	switch (l->tilesize)
	{
		default:
		case TILE_8x8:      addr = tile_index;              offs = 0;                                               break;
		case TILE_8x32:     addr = tile_index & (~0x180);   offs = (tile_index/0x80) & 3;                           break;
		case TILE_64x32:    addr = tile_index & (~0x187);   offs = ((tile_index/0x80) & 3) + (tile_index & 7) * 4;  break;
	}
	SET_TILE_INFO_MEMBER(0, (l->videorams[VRAM_HI][addr] << 8) + l->videorams[VRAM_LO][addr] + offs, 0, 0);
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


WRITE8_MEMBER(subsino2_state::ss9601_byte_lo_w)
{
	m_ss9601_byte_lo = data;
}
WRITE8_MEMBER(subsino2_state::ss9601_byte_lo2_w)
{
	m_ss9601_byte_lo2 = data;
}


static inline void ss9601_videoram_w(layer_t *l, vram_t vram, address_space &space, offs_t offset, UINT8 data)
{
	l->videorams[vram][offset] = data;

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
WRITE8_MEMBER(subsino2_state::ss9601_videoram_0_hi_w)
{
	ss9601_videoram_w(&m_layers[0], VRAM_HI, space, offset, data);
}

WRITE8_MEMBER(subsino2_state::ss9601_videoram_0_lo_w)
{
	ss9601_videoram_w(&m_layers[0], VRAM_LO, space, offset, data);
}

WRITE8_MEMBER(subsino2_state::ss9601_videoram_0_hi_lo_w)
{
	ss9601_videoram_w(&m_layers[0], VRAM_HI, space, offset, data);
	ss9601_videoram_w(&m_layers[0], VRAM_LO, space, offset, m_ss9601_byte_lo);
}

WRITE8_MEMBER(subsino2_state::ss9601_videoram_0_hi_lo2_w)
{
	ss9601_videoram_w(&m_layers[0], VRAM_HI, space, offset, data);
	ss9601_videoram_w(&m_layers[0], VRAM_LO, space, offset, m_ss9601_byte_lo2);
}

READ8_MEMBER(subsino2_state::ss9601_videoram_0_hi_r)
{
	return m_layers[0].videorams[VRAM_HI][offset];
}

READ8_MEMBER(subsino2_state::ss9601_videoram_0_lo_r)
{
	return m_layers[0].videorams[VRAM_LO][offset];
}

// Layer 1
WRITE8_MEMBER(subsino2_state::ss9601_videoram_1_hi_w)
{
	ss9601_videoram_w(&m_layers[1], VRAM_HI, space, offset, data);
}

WRITE8_MEMBER(subsino2_state::ss9601_videoram_1_lo_w)
{
	ss9601_videoram_w(&m_layers[1], VRAM_LO, space, offset, data);
}

WRITE8_MEMBER(subsino2_state::ss9601_videoram_1_hi_lo_w)
{
	ss9601_videoram_w(&m_layers[1], VRAM_HI, space, offset, data);
	ss9601_videoram_w(&m_layers[1], VRAM_LO, space, offset, m_ss9601_byte_lo);
}

WRITE8_MEMBER(subsino2_state::ss9601_videoram_1_hi_lo2_w)
{
	ss9601_videoram_w(&m_layers[1], VRAM_HI, space, offset, data);
	ss9601_videoram_w(&m_layers[1], VRAM_LO, space, offset, m_ss9601_byte_lo2);
}

READ8_MEMBER(subsino2_state::ss9601_videoram_1_hi_r)
{
	return m_layers[1].videorams[VRAM_HI][offset];
}

READ8_MEMBER(subsino2_state::ss9601_videoram_1_lo_r)
{
	return m_layers[1].videorams[VRAM_LO][offset];
}

// Layer 0 Reels

WRITE8_MEMBER(subsino2_state::ss9601_reelram_hi_lo_w)
{
	m_ss9601_reelrams[VRAM_HI][offset] = data;
	m_ss9601_reelrams[VRAM_LO][offset] = m_ss9601_byte_lo;
}
READ8_MEMBER(subsino2_state::ss9601_reelram_hi_r)
{
	return m_ss9601_reelrams[VRAM_HI][offset];
}
READ8_MEMBER(subsino2_state::ss9601_reelram_lo_r)
{
	return m_ss9601_reelrams[VRAM_LO][offset];
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

WRITE8_MEMBER(subsino2_state::ss9601_scrollctrl_w)
{
	m_ss9601_scrollctrl = data;
}

WRITE8_MEMBER(subsino2_state::ss9601_tilesize_w)
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

WRITE8_MEMBER(subsino2_state::ss9601_scroll_w)
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
WRITE8_MEMBER(subsino2_state::ss9601_scrollram_0_hi_w)
{
	m_layers[0].scrollrams[VRAM_HI][offset] = data;
}

WRITE8_MEMBER(subsino2_state::ss9601_scrollram_0_lo_w)
{
	m_layers[0].scrollrams[VRAM_LO][offset] = data;
}

WRITE8_MEMBER(subsino2_state::ss9601_scrollram_0_hi_lo_w)
{
	m_layers[0].scrollrams[VRAM_HI][offset] = data;
	m_layers[0].scrollrams[VRAM_LO][offset] = m_ss9601_byte_lo;
}

READ8_MEMBER(subsino2_state::ss9601_scrollram_0_hi_r)
{
	return m_layers[0].scrollrams[VRAM_HI][offset];
}

READ8_MEMBER(subsino2_state::ss9601_scrollram_0_lo_r)
{
	return m_layers[0].scrollrams[VRAM_LO][offset];
}

// Layer 1
WRITE8_MEMBER(subsino2_state::ss9601_scrollram_1_hi_w)
{
	m_layers[1].scrollrams[VRAM_HI][offset] = data;
}

WRITE8_MEMBER(subsino2_state::ss9601_scrollram_1_lo_w)
{
	m_layers[1].scrollrams[VRAM_LO][offset] = data;
}

WRITE8_MEMBER(subsino2_state::ss9601_scrollram_1_hi_lo_w)
{
	m_layers[1].scrollrams[VRAM_HI][offset] = data;
	m_layers[1].scrollrams[VRAM_LO][offset] = m_ss9601_byte_lo;
}

READ8_MEMBER(subsino2_state::ss9601_scrollram_1_hi_r)
{
	return m_layers[1].scrollrams[VRAM_HI][offset];
}

READ8_MEMBER(subsino2_state::ss9601_scrollram_1_lo_r)
{
	return m_layers[1].scrollrams[VRAM_LO][offset];
}


/***************************************************************************
                              Tilemaps Disable
***************************************************************************/

WRITE8_MEMBER(subsino2_state::ss9601_disable_w)
{
	m_ss9601_disable = data;
}


/***************************************************************************
                                Video Update
***************************************************************************/

VIDEO_START_MEMBER(subsino2_state,subsino2)
{
	m_hm86171_colorram = auto_alloc_array(machine(), UINT8, 256*3);

	// SS9601 Regs:

	m_ss9601_tilesize       =   TILE_8x8;
	m_ss9601_scrollctrl =   0xfd;   // not written by mtrain, default to reels on
	m_ss9601_disable        =   0x00;

	// SS9601 Layers:

	for (int i = 0; i < 2; i++)
	{
		layer_t *l = &m_layers[i];

		l->tmap = &machine().tilemap().create(m_gfxdecode, i ?
												tilemap_get_info_delegate(FUNC(subsino2_state::ss9601_get_tile_info_1),this) :
												tilemap_get_info_delegate(FUNC(subsino2_state::ss9601_get_tile_info_0),this),
												TILEMAP_SCAN_ROWS, 8,8, 0x80,0x40);

		l->tmap->set_transparent_pen(0);

		// line scroll
		l->tmap->set_scroll_rows(0x200);

		l->videorams[VRAM_HI] = auto_alloc_array(machine(), UINT8, 0x80 * 0x40);
		l->videorams[VRAM_LO] = auto_alloc_array(machine(), UINT8, 0x80 * 0x40);

		l->scrollrams[VRAM_HI] = auto_alloc_array(machine(), UINT8, 0x200);
		l->scrollrams[VRAM_LO] = auto_alloc_array(machine(), UINT8, 0x200);
		memset(l->scrollrams[VRAM_HI], 0, 0x200);
		memset(l->scrollrams[VRAM_LO], 0, 0x200);
	}

	// SS9601 Reels:

	m_ss9601_reelrams[VRAM_HI] = auto_alloc_array(machine(), UINT8, 0x2000);
	m_ss9601_reelrams[VRAM_LO] = auto_alloc_array(machine(), UINT8, 0x2000);
	memset(m_ss9601_reelrams[VRAM_HI], 0, 0x2000);
	memset(m_ss9601_reelrams[VRAM_LO], 0, 0x2000);
	m_ss9601_reelrects[0].set(0, 0, 0x00*8, 0x09*8-1);
	m_ss9601_reelrects[1].set(0, 0, 0x09*8, 0x10*8-1);
	m_ss9601_reelrects[2].set(0, 0, 0x10*8, 256-16-1);

/*
    save_pointer(NAME(m_ss9601_reelrams[VRAM_HI]), 0x2000);
    save_pointer(NAME(m_ss9601_reelrams[VRAM_LO]), 0x2000);

    save_pointer(NAME(m_layers[0].scrollrams[VRAM_HI]), 0x200);
    save_pointer(NAME(m_layers[0].scrollrams[VRAM_LO]), 0x200);

    save_pointer(NAME(m_layers[1].scrollrams[VRAM_HI]), 0x200);
    save_pointer(NAME(m_layers[1].scrollrams[VRAM_LO]), 0x200);
*/
}

VIDEO_START_MEMBER(subsino2_state,mtrain)
{
	VIDEO_START_CALL_MEMBER( subsino2 );
}

VIDEO_START_MEMBER(subsino2_state,xtrain)
{
	VIDEO_START_CALL_MEMBER( subsino2 );
	m_ss9601_reelrects[0].set(0, 0, 0x00*8, 0x08*8-1);
	m_ss9601_reelrects[1].set(0, 0, 0x08*8, 0x18*8-1);
	m_ss9601_reelrects[2].set(0, 0, 0x18*8, 256-16-1);
}

UINT32 subsino2_state::screen_update_subsino2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

		UINT16 scroll_dx = 0;
		for (y = 0; y < 0x200; y++)
		{
			if (mask_y[i])
				scroll_dx = (l->scrollrams[VRAM_HI][y & mask_y[i]] << 8) + l->scrollrams[VRAM_LO][y & mask_y[i]];

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

			for (auto visible : m_ss9601_reelrects)
			{
				

				for (int x = 0; x < 0x40; x++)
				{
					visible.min_x = 8 * x;
					visible.max_x = 8 * (x+1) - 1;

					int reeladdr = (visible.min_y / 0x10) * 0x80 + x;
					UINT16 reelscroll = (m_ss9601_reelrams[VRAM_HI][reeladdr] << 8) + m_ss9601_reelrams[VRAM_LO][reeladdr];

					l->tmap->set_scrollx(0, (reelscroll >> 9) * 8 + l->scroll_x - visible.min_x);

					// wrap around at half tilemap (0x100)
					int reelscroll_y = (reelscroll & 0x100) + ((reelscroll + l->scroll_y - visible.min_y/0x10*0x10 + 1) & 0xff);
					int reelwrap_y = 0x100 - (reelscroll_y & 0xff);

					rectangle tmp = visible;

					// draw above the wrap around y
					if ( reelwrap_y-1 >= visible.min_y )
					{
						if ( reelwrap_y-1 <= visible.max_y )
							tmp.max_y = reelwrap_y-1;
						l->tmap->set_scrolly(0, reelscroll_y);
						l->tmap->draw(screen, bitmap, tmp, 0, 0);
						tmp.max_y = visible.max_y;
					}

					// draw below the wrap around y
					if ( reelwrap_y <= visible.max_y )
					{
						if ( reelwrap_y >= visible.min_y )
							tmp.min_y = reelwrap_y;
						l->tmap->set_scrolly(0, -((reelwrap_y &0xff) | (reelscroll_y & 0x100)));
						l->tmap->draw(screen, bitmap, tmp, 0, 0);
						tmp.min_y = visible.min_y;
					}
				}
			}
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
                Palette: HMC HM86171 VGA 256 colour RAMDAC
***************************************************************************/


WRITE8_MEMBER(subsino2_state::hm86171_colorram_w)
{
	switch (offset)
	{
		case 0:
			m_hm86171_offs = data * 3;
			break;

		case 1:
			m_hm86171_colorram[m_hm86171_offs] = data;
			m_palette->set_pen_color(m_hm86171_offs/3,
				pal6bit(m_hm86171_colorram[(m_hm86171_offs/3)*3+0]),
				pal6bit(m_hm86171_colorram[(m_hm86171_offs/3)*3+1]),
				pal6bit(m_hm86171_colorram[(m_hm86171_offs/3)*3+2])
			);
			m_hm86171_offs = (m_hm86171_offs+1) % (256*3);
			break;

		case 2:
			// ff?
			break;

		case 3:
			break;
	}
}


/***************************************************************************
                                Input / Output
***************************************************************************/

WRITE8_MEMBER(subsino2_state::dsw_mask_w)
{
	m_dsw_mask = data;
}

READ8_MEMBER(subsino2_state::dsw_r)
{
	return  ( (ioport("DSW1")->read() & m_dsw_mask) ? 0x01 : 0 ) |
			( (ioport("DSW2")->read() & m_dsw_mask) ? 0x02 : 0 ) |
			( (ioport("DSW3")->read() & m_dsw_mask) ? 0x04 : 0 ) |
			( (ioport("DSW4")->read() & m_dsw_mask) ? 0x08 : 0 ) ;
}


READ8_MEMBER(subsino2_state::vblank_bit2_r)
{
	return m_screen->vblank() ? 0x04 : 0x00;
}
READ8_MEMBER(subsino2_state::vblank_bit6_r)
{
	return m_screen->vblank() ? 0x40 : 0x00;
}

WRITE8_MEMBER(subsino2_state::oki_bank_bit0_w)
{
	// it writes 0x32 or 0x33
	m_oki->set_bank_base((data & 1) * 0x40000);
}

WRITE8_MEMBER(subsino2_state::oki_bank_bit4_w)
{
	// it writes 0x23 or 0x33
	m_oki->set_bank_base(((data >> 4) & 1) * 0x40000);
}

INTERRUPT_GEN_MEMBER(subsino2_state::am188em_int0_irq)
{
	downcast<i80186_cpu_device *>(m_maincpu.target())->int0_w(1);
}


/***************************************************************************
                                Memory Maps
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/


WRITE16_MEMBER(subsino2_state::bishjan_sel_w)
{
	/*
	    sound writes in service mode:
	    01 88 04 00 (coin in)
	    02 89 04 0v (v = voice = 0..3)
	*/
	if (ACCESSING_BITS_8_15)
		m_bishjan_sel = data >> 8;
}

READ16_MEMBER(subsino2_state::bishjan_serial_r)
{
	return
		(machine().rand() & 0x9800) |   // bit 7 - serial communication
		(((m_bishjan_sel==0x12) ? 0x40:0x00) << 8) |
//      (machine.rand() & 0xff);
//      (((m_screen->frame_number()%60)==0)?0x18:0x00);
		0x18;
}

WRITE16_MEMBER(subsino2_state::bishjan_input_w)
{
	if (ACCESSING_BITS_8_15)
		m_bishjan_input = data >> 8;
}

READ16_MEMBER(subsino2_state::bishjan_input_r)
{
	int i;
	UINT16 res = 0xff;
	static const char *const port[] = { "KEYB_0", "KEYB_1", "KEYB_2", "KEYB_3", "KEYB_4" };

	for (i = 0; i < 5; i++)
		if (m_bishjan_input & (1 << i))
			res = ioport(port[i])->read();

	return  (res << 8) |                                    // high byte
			ioport("SYSTEM")->read() |      // low byte
			(machine().device<ticket_dispenser_device>("hopper")->read(space, 0) ? 0x00 : 0x04) // bit 2: hopper sensor
	;
}

WRITE16_MEMBER(subsino2_state::bishjan_outputs_w)
{
	m_outputs16[offset] = data;

	switch (offset)
	{
		case 0:
			if (ACCESSING_BITS_0_7)
			{
				// coin out         data & 0x01;
				machine().device<ticket_dispenser_device>("hopper")->write(space, 0, (data & 0x0002) ? 0x80 : 0);   // hopper
				coin_counter_w(machine(), 0,    data & 0x0010 );
			}
			break;
	}

//  popmessage("0: %04x", m_outputs16[0]);
}


static ADDRESS_MAP_START( bishjan_map, AS_PROGRAM, 16, subsino2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE( 0x080000, 0x0fffff ) AM_ROM AM_REGION("maincpu", 0)

	AM_RANGE( 0x200000, 0x207fff ) AM_RAM AM_SHARE("nvram") // battery

	// read lo (L1)   (only half tilemap?)
	AM_RANGE( 0x412000, 0x412fff ) AM_READ8(ss9601_videoram_1_lo_r, 0xffff )
	AM_RANGE( 0x413000, 0x4131ff ) AM_READWRITE8(ss9601_scrollram_1_lo_r, ss9601_scrollram_1_lo_w, 0xffff )
	// read lo (REEL)
	AM_RANGE( 0x416000, 0x416fff ) AM_READ8(ss9601_reelram_lo_r, 0xffff )
	AM_RANGE( 0x417000, 0x4171ff ) AM_READWRITE8(ss9601_scrollram_0_lo_r, ss9601_scrollram_0_lo_w, 0xffff )

	// read hi (L1)
	AM_RANGE( 0x422000, 0x422fff ) AM_READ8(ss9601_videoram_1_hi_r, 0xffff )
	AM_RANGE( 0x423000, 0x4231ff ) AM_READWRITE8(ss9601_scrollram_1_hi_r, ss9601_scrollram_1_hi_w, 0xffff )
	// read hi (REEL)
	AM_RANGE( 0x426000, 0x426fff ) AM_READ8(ss9601_reelram_hi_r, 0xffff )
	AM_RANGE( 0x427000, 0x4271ff ) AM_READWRITE8(ss9601_scrollram_0_hi_r, ss9601_scrollram_0_hi_w, 0xffff )

	// write both (L1)
	AM_RANGE( 0x430000, 0x431fff ) AM_WRITE8(ss9601_videoram_1_hi_lo_w, 0xffff )
	AM_RANGE( 0x432000, 0x432fff ) AM_WRITE8(ss9601_videoram_1_hi_lo_w, 0xffff )
	AM_RANGE( 0x433000, 0x4331ff ) AM_WRITE8(ss9601_scrollram_1_hi_lo_w, 0xffff )
	// write both (L0 & REEL)
	AM_RANGE( 0x434000, 0x435fff ) AM_WRITE8(ss9601_videoram_0_hi_lo_w, 0xffff )
	AM_RANGE( 0x436000, 0x436fff ) AM_WRITE8(ss9601_reelram_hi_lo_w, 0xffff )
	AM_RANGE( 0x437000, 0x4371ff ) AM_WRITE8(ss9601_scrollram_0_hi_lo_w, 0xffff )

	AM_RANGE( 0x600000, 0x600001 ) AM_READNOP AM_WRITE(bishjan_sel_w )
	AM_RANGE( 0x600040, 0x600041 ) AM_WRITE8(ss9601_scrollctrl_w, 0xff00 )
	AM_RANGE( 0x600060, 0x600063 ) AM_WRITE8(hm86171_colorram_w, 0xffff )
	AM_RANGE( 0x600080, 0x600081 ) AM_WRITE8(ss9601_tilesize_w, 0xff00 )
	AM_RANGE( 0x6000a0, 0x6000a1 ) AM_WRITE8(ss9601_byte_lo_w, 0xff00 )

	AM_RANGE( 0xa0001e, 0xa0001f ) AM_WRITE8(ss9601_disable_w, 0x00ff )

	AM_RANGE( 0xa00020, 0xa00025 ) AM_WRITE8(ss9601_scroll_w, 0xffff )

	AM_RANGE( 0xc00000, 0xc00001 ) AM_READ_PORT("DSW")                              // SW1
	AM_RANGE( 0xc00002, 0xc00003 ) AM_READ_PORT("JOY") AM_WRITE(bishjan_input_w )   // IN C
	AM_RANGE( 0xc00004, 0xc00005 ) AM_READ(bishjan_input_r )                        // IN A & B
	AM_RANGE( 0xc00006, 0xc00007 ) AM_READ(bishjan_serial_r )                       // IN D
	AM_RANGE( 0xc00008, 0xc00009 ) AM_READ_PORT("RESET") AM_WRITE(bishjan_outputs_w ) AM_SHARE("outputs16")
ADDRESS_MAP_END

/***************************************************************************
                       Express Card / Top Card
***************************************************************************/

WRITE8_MEMBER(subsino2_state::expcard_outputs_w)
{
	m_outputs[offset] = data;

	switch (offset)
	{
		case 0: // D
			// 0x40 = serial out ? (at boot)
			break;

		case 1: // C
			set_led_status(machine(), 0,    data & 0x02);   // raise
			break;

		case 2: // B
			set_led_status(machine(), 1,    data & 0x04);   // hold 4 / small & hold 5 / big ?
			set_led_status(machine(), 2,    data & 0x08);   // hold 1 / bet
			set_led_status(machine(), 3,    data & 0x10);   // hold 2 / take ?
			set_led_status(machine(), 4,    data & 0x20);   // hold 3 / double up ?
			break;

		case 3: // A
			coin_counter_w(machine(), 0,    data & 0x01 );  // coin in
			coin_counter_w(machine(), 1,    data & 0x02 );  // key in

			set_led_status(machine(), 5,    data & 0x10);   // start
			break;
	}

//  popmessage("0: %02x - 1: %02x - 2: %02x - 3: %02x", m_outputs[0], m_outputs[1], m_outputs[2], m_outputs[3]);
}

/***************************************************************************
                                Magic Train
***************************************************************************/

WRITE8_MEMBER(subsino2_state::mtrain_outputs_w)
{
	m_outputs[offset] = data;

	switch (offset)
	{
		case 0:
			coin_counter_w(machine(), 0,    data & 0x01 );  // key in
			coin_counter_w(machine(), 1,    data & 0x02 );  // coin in
			coin_counter_w(machine(), 2,    data & 0x10 );  // pay out
//          coin_counter_w(machine(), 3,   data & 0x20 );  // hopper motor
			break;

		case 1:
			set_led_status(machine(), 0,    data & 0x01);   // stop reel?
			set_led_status(machine(), 1,    data & 0x02);   // stop reel? (double or take)
			set_led_status(machine(), 2,    data & 0x04);   // start all
			set_led_status(machine(), 3,    data & 0x08);   // bet / stop all
			set_led_status(machine(), 4,    data & 0x20);   // stop reel? (double or take)
			break;

		case 2:
			break;

		case 3:
			break;
	}

//  popmessage("0: %02x - 1: %02x - 2: %02x - 3: %02x", m_outputs[0], m_outputs[1], m_outputs[2], m_outputs[3]);
}

WRITE8_MEMBER(subsino2_state::mtrain_videoram_w)
{
	vram_t vram = (m_ss9601_byte_lo & 0x08) ? VRAM_HI : VRAM_LO;
	switch (m_ss9601_byte_lo & (~0x08))
	{
		case 0x00:  ss9601_videoram_w(&m_layers[1], vram, space, offset,        data);
					ss9601_videoram_w(&m_layers[1], vram, space, offset+0x1000, data);  break;

		case 0x04:  ss9601_videoram_w(&m_layers[0], vram, space, offset,        data);
					ss9601_videoram_w(&m_layers[0], vram, space, offset+0x1000, data);  break;

		case 0x06:  m_ss9601_reelrams[vram][offset] = data; break;
	}
}

WRITE8_MEMBER(subsino2_state::mtrain_tilesize_w)
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

READ8_MEMBER(subsino2_state::mtrain_prot_r)
{
	return "SUBSION"[offset];
}

static ADDRESS_MAP_START( mtrain_map, AS_PROGRAM, 8, subsino2_state )
	AM_RANGE( 0x00000, 0x06fff ) AM_ROM

	AM_RANGE( 0x07800, 0x07fff ) AM_RAM AM_SHARE("nvram")   // battery

	AM_RANGE( 0x08000, 0x08fff ) AM_WRITE(mtrain_videoram_w )

	AM_RANGE( 0x0911f, 0x0911f ) AM_WRITE(ss9601_disable_w )
	AM_RANGE( 0x09120, 0x09125 ) AM_WRITE(ss9601_scroll_w )

	AM_RANGE( 0x0912f, 0x0912f ) AM_WRITE(ss9601_byte_lo_w )

	AM_RANGE( 0x09140, 0x09142 ) AM_WRITE(mtrain_outputs_w ) AM_SHARE("outputs")
	AM_RANGE( 0x09143, 0x09143 ) AM_READ_PORT( "IN D" ) // (not shown in system test) 0x40 serial out, 0x80 serial in
	AM_RANGE( 0x09144, 0x09144 ) AM_READ_PORT( "IN A" ) // A
	AM_RANGE( 0x09145, 0x09145 ) AM_READ_PORT( "IN B" ) // B
	AM_RANGE( 0x09146, 0x09146 ) AM_READ_PORT( "IN C" ) // C
	AM_RANGE( 0x09147, 0x09147 ) AM_READ(dsw_r )
	AM_RANGE( 0x09148, 0x09148 ) AM_WRITE(dsw_mask_w )

	AM_RANGE( 0x09152, 0x09152 ) AM_READ(vblank_bit2_r ) AM_WRITE(oki_bank_bit0_w )

	AM_RANGE( 0x09158, 0x0915e ) AM_READ(mtrain_prot_r )

	AM_RANGE( 0x09160, 0x09163 ) AM_WRITE(hm86171_colorram_w )
	AM_RANGE( 0x09164, 0x09164 ) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE( 0x09168, 0x09168 ) AM_WRITE(mtrain_tilesize_w )

	AM_RANGE( 0x09800, 0x09fff ) AM_RAM

	AM_RANGE( 0x0a000, 0x0ffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mtrain_io, AS_IO, 8, subsino2_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs
ADDRESS_MAP_END

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

WRITE8_MEMBER(subsino2_state::saklove_outputs_w)
{
	m_outputs[offset] = data;

	switch (offset)
	{
		case 0:
			coin_counter_w(machine(), 0,    data & 0x01 );  // coin in
			coin_counter_w(machine(), 1,    data & 0x02 );  // key in
			break;

		case 1:
			break;

		case 2:
			break;

		case 3:
			// 1, 2, 4
			break;
	}

//  popmessage("0: %02x - 1: %02x - 2: %02x - 3: %02x", m_outputs[0], m_outputs[1], m_outputs[2], m_outputs[3]);
}

static ADDRESS_MAP_START( saklove_map, AS_PROGRAM, 8, subsino2_state )
	AM_RANGE(0x00000, 0x07fff) AM_RAM AM_SHARE("nvram") // battery

	// read lo (L1)   (only half tilemap?)
	AM_RANGE(0x12000, 0x12fff) AM_READWRITE(ss9601_videoram_1_lo_r,  ss9601_videoram_1_lo_w )
	AM_RANGE(0x13000, 0x131ff) AM_READWRITE(ss9601_scrollram_1_lo_r, ss9601_scrollram_1_lo_w )
	// read lo (L0)
	AM_RANGE(0x16000, 0x16fff) AM_READWRITE(ss9601_videoram_0_lo_r,  ss9601_videoram_0_lo_w )
	AM_RANGE(0x17000, 0x171ff) AM_READWRITE(ss9601_scrollram_0_lo_r, ss9601_scrollram_0_lo_w )

	// read hi (L1)
	AM_RANGE(0x22000, 0x22fff) AM_READWRITE(ss9601_videoram_1_hi_r,  ss9601_videoram_1_hi_w )
	AM_RANGE(0x23000, 0x231ff) AM_READWRITE(ss9601_scrollram_1_hi_r, ss9601_scrollram_1_hi_w )
	// read hi (L0)
	AM_RANGE(0x26000, 0x26fff) AM_READWRITE(ss9601_videoram_0_hi_r,  ss9601_videoram_0_hi_w )
	AM_RANGE(0x27000, 0x271ff) AM_READWRITE(ss9601_scrollram_0_hi_r, ss9601_scrollram_0_hi_w )

	// write both (L1)
	AM_RANGE(0x30000, 0x31fff) AM_READWRITE(ss9601_videoram_1_hi_r, ss9601_videoram_1_hi_lo_w )
	// write both (L0)
	AM_RANGE(0x34000, 0x35fff) AM_READWRITE(ss9601_videoram_0_hi_r, ss9601_videoram_0_hi_lo_w )

	AM_RANGE(0xe0000, 0xfffff) AM_ROM AM_REGION("maincpu",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( saklove_io, AS_IO, 8, subsino2_state )
	AM_RANGE(0x0000, 0x0000) AM_WRITE(ss9601_scrollctrl_w )

	AM_RANGE(0x0020, 0x0020) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x0040, 0x0041) AM_DEVWRITE("ymsnd", ym3812_device, write)

	AM_RANGE(0x0060, 0x0063) AM_WRITE(hm86171_colorram_w )

	AM_RANGE(0x0080, 0x0080) AM_WRITE(ss9601_tilesize_w )
	AM_RANGE(0x00a0, 0x00a0) AM_WRITE(ss9601_byte_lo_w )
	AM_RANGE(0x021f, 0x021f) AM_WRITE(ss9601_disable_w )
	AM_RANGE(0x0220, 0x0225) AM_WRITE(ss9601_scroll_w )

	AM_RANGE(0x0300, 0x0303) AM_WRITE(saklove_outputs_w ) AM_SHARE("outputs")
	AM_RANGE(0x0303, 0x0303) AM_READ_PORT( "IN D" ) // 0x40 serial out, 0x80 serial in
	AM_RANGE(0x0304, 0x0304) AM_READ_PORT( "IN A" )
	AM_RANGE(0x0305, 0x0305) AM_READ_PORT( "IN B" )
	AM_RANGE(0x0306, 0x0306) AM_READ_PORT( "IN C" )

	AM_RANGE(0x0307, 0x0307) AM_READ(dsw_r )
	AM_RANGE(0x0308, 0x0308) AM_WRITE(dsw_mask_w )

	AM_RANGE(0x0312, 0x0312) AM_READ(vblank_bit2_r ) AM_WRITE(oki_bank_bit0_w )

ADDRESS_MAP_END

/***************************************************************************
                                X-Plan
***************************************************************************/

WRITE8_MEMBER(subsino2_state::xplan_outputs_w)
{
	m_outputs[offset] = data;

	switch (offset)
	{
		case 0:
			// 0x40 = serial out ? (at boot)
			break;

		case 1:
			set_led_status(machine(), 0,    data & 0x02);   // raise
			break;

		case 2: // B
			set_led_status(machine(), 1,    data & 0x04);   // hold 1 / big ?
			set_led_status(machine(), 2,    data & 0x08);   // hold 5 / bet
			set_led_status(machine(), 3,    data & 0x10);   // hold 4 ?
			set_led_status(machine(), 4,    data & 0x20);   // hold 2 / double up
			set_led_status(machine(), 5,    data & 0x40);   // hold 3 / small ?
			break;

		case 3: // A
			coin_counter_w(machine(), 0,    data & 0x01 );
			coin_counter_w(machine(), 1,    data & 0x02 );

			set_led_status(machine(), 6,    data & 0x10);   // start / take
			break;
	}

//  popmessage("0: %02x - 1: %02x - 2: %02x - 3: %02x", m_outputs[0], m_outputs[1], m_outputs[2], m_outputs[3]);
}

static ADDRESS_MAP_START( xplan_map, AS_PROGRAM, 8, subsino2_state )
	AM_RANGE(0x00000, 0x07fff) AM_RAM AM_SHARE("nvram") // battery

	// write both (L1, byte_lo2)
	AM_RANGE( 0x10000, 0x11fff ) AM_WRITE(ss9601_videoram_1_hi_lo2_w )
	// read lo (L1)   (only half tilemap?)
	AM_RANGE( 0x12000, 0x12fff ) AM_READ(ss9601_videoram_1_lo_r )
	AM_RANGE( 0x13000, 0x131ff ) AM_READWRITE(ss9601_scrollram_1_lo_r, ss9601_scrollram_1_lo_w )

	// write both (L0, byte_lo2)
	AM_RANGE( 0x14000, 0x15fff ) AM_WRITE(ss9601_videoram_0_hi_lo2_w )
	// read lo (REEL)
	AM_RANGE( 0x16000, 0x16fff ) AM_READ(ss9601_reelram_lo_r )
	AM_RANGE( 0x17000, 0x171ff ) AM_READWRITE(ss9601_scrollram_0_lo_r, ss9601_scrollram_0_lo_w )

	// read hi (L1)
	AM_RANGE( 0x22000, 0x22fff ) AM_READ(ss9601_videoram_1_hi_r )
	AM_RANGE( 0x23000, 0x231ff ) AM_READWRITE(ss9601_scrollram_1_hi_r, ss9601_scrollram_1_hi_w )
	// read hi (REEL)
	AM_RANGE( 0x26000, 0x26fff ) AM_READ(ss9601_reelram_hi_r )
	AM_RANGE( 0x27000, 0x271ff ) AM_READWRITE(ss9601_scrollram_0_hi_r, ss9601_scrollram_0_hi_w )

	// write both (L1, byte_lo)
	AM_RANGE( 0x30000, 0x31fff ) AM_WRITE(ss9601_videoram_1_hi_lo_w )
	AM_RANGE( 0x32000, 0x32fff ) AM_WRITE(ss9601_videoram_1_hi_lo_w )
	AM_RANGE( 0x33000, 0x331ff ) AM_WRITE(ss9601_scrollram_1_hi_lo_w )
	// write both (L0 & REEL, byte_lo)
	AM_RANGE( 0x34000, 0x35fff ) AM_WRITE(ss9601_videoram_0_hi_lo_w )
	AM_RANGE( 0x36000, 0x36fff ) AM_WRITE(ss9601_reelram_hi_lo_w )
	AM_RANGE( 0x37000, 0x371ff ) AM_WRITE(ss9601_scrollram_0_hi_lo_w )

	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("maincpu",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xplan_io, AS_IO, 8, subsino2_state )
	AM_RANGE(0x0000, 0x0000) AM_DEVREADWRITE("oki", okim6295_device, read, write)

	AM_RANGE(0x0020, 0x0020) AM_WRITE(ss9601_byte_lo2_w )

	AM_RANGE(0x0040, 0x0040) AM_WRITE(ss9601_scrollctrl_w )

	AM_RANGE(0x0060, 0x0063) AM_WRITE(hm86171_colorram_w )
	AM_RANGE(0x0080, 0x0080) AM_WRITE(ss9601_tilesize_w )
	AM_RANGE(0x00a0, 0x00a0) AM_WRITE(ss9601_byte_lo_w )

	AM_RANGE(0x021f, 0x021f) AM_WRITE(ss9601_disable_w )
	AM_RANGE(0x0220, 0x0225) AM_WRITE(ss9601_scroll_w )

	AM_RANGE(0x0235, 0x0235) AM_NOP // INT0 Ack.?

	AM_RANGE(0x0300, 0x0300) AM_READ(vblank_bit6_r ) AM_WRITE(oki_bank_bit4_w )
	AM_RANGE(0x0301, 0x0301) AM_WRITE(dsw_mask_w )
	AM_RANGE(0x0302, 0x0302) AM_READ(dsw_r )
	AM_RANGE(0x0303, 0x0303) AM_READ_PORT( "IN C" )
	AM_RANGE(0x0304, 0x0304) AM_READ_PORT( "IN B" )
	AM_RANGE(0x0305, 0x0305) AM_READ_PORT( "IN A" )
	AM_RANGE(0x0306, 0x0306) AM_READ_PORT( "IN D" ) // 0x40 serial out, 0x80 serial in

	// 306 = d, 307 = c, 308 = b, 309 = a
	AM_RANGE(0x0306, 0x0309) AM_WRITE(xplan_outputs_w ) AM_SHARE("outputs")
ADDRESS_MAP_END

/***************************************************************************
                                X-Train
***************************************************************************/

WRITE8_MEMBER(subsino2_state::xtrain_outputs_w)
{
	m_outputs[offset] = data;

	switch (offset)
	{
		case 0: // D
			// 0x40 = serial out ? (at boot)
			break;

		case 1: // C
			set_led_status(machine(), 0,    data & 0x02);   // re-double
			set_led_status(machine(), 1,    data & 0x04);   // half double
			break;

		case 2: // B
			set_led_status(machine(), 2,    data & 0x02);   // hold 3 / small
			set_led_status(machine(), 3,    data & 0x04);   // hold 2 / big
			set_led_status(machine(), 4,    data & 0x08);   // bet
			set_led_status(machine(), 5,    data & 0x10);   // hold1 / take
			set_led_status(machine(), 6,    data & 0x20);   // double up
			break;

		case 3: // A
			coin_counter_w(machine(), 0,    data & 0x01 );  // coin in
			coin_counter_w(machine(), 1,    data & 0x02 );  // key in

			set_led_status(machine(), 7,    data & 0x10);   // start
			break;
	}

//  popmessage("0: %02x - 1: %02x - 2: %02x - 3: %02x", m_outputs[0], m_outputs[1], m_outputs[2], m_outputs[3]);
}

static ADDRESS_MAP_START( expcard_io, AS_IO, 8, subsino2_state )
	// 306 = d, 307 = c, 308 = b, 309 = a
	AM_RANGE(0x0306, 0x0309) AM_WRITE(expcard_outputs_w ) AM_SHARE("outputs")

	AM_IMPORT_FROM( xplan_io )
ADDRESS_MAP_END

static ADDRESS_MAP_START( xtrain_io, AS_IO, 8, subsino2_state )
	// 306 = d, 307 = c, 308 = b, 309 = a
	AM_RANGE(0x0306, 0x0309) AM_WRITE(xtrain_outputs_w ) AM_SHARE("outputs")

	AM_IMPORT_FROM( xplan_io )
ADDRESS_MAP_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout ss9601_8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

static GFXDECODE_START( ss9601 )
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
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)

	PORT_START("DSW")   // SW1
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Controls ) )
	PORT_DIPSETTING(      0x0001, "Keyboard" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("JOY")   // IN C
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1         ) PORT_NAME("1 Player Start (Joy Mode)")    // start (joy)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )   // down (joy)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )   // left (joy)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )   // right (joy)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        )   // n (joy)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_BET    ) PORT_NAME("P1 Mahjong Bet (Joy Mode)")    // bet (joy)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2        )   // select (joy)

	PORT_START("SYSTEM") // IN A
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE        )   PORT_IMPULSE(1) // service mode (press twice for inputs)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH,IPT_SPECIAL        )   // hopper sensor
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1       )   // stats
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2       )   // pay out? "hopper empty"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1          )   PORT_IMPULSE(2) // coin
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE3       )   // pay out? "hopper empty"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2          )   PORT_IMPULSE(2) // coin

	PORT_START("KEYB_0")    // IN B(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A      )   // a
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E      )   // e
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I      )   // i
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M      )   // m
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // i2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1         )   // b2 (start)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEYB_1")    // IN B(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B      )   // b
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F      )   // f
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J      )   // j
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N      )   // n
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // l2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET    )   // c2 (bet)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEYB_2")    // IN B(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C      )   // c
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G      )   // g
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K      )   // k
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // k2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // m2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEYB_3")    // IN B(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D      )   // d
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H      )   // h
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L      )   // l
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // j2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEYB_4")    // IN B(4)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // g2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // e2
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // d2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN        )   // f2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN        )
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

	PORT_START("IN A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN     ) PORT_NAME("Raise") PORT_CODE(KEYCODE_M)    // raise
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1      ) PORT_NAME("Start")                 // start
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Small")        // hold 4 / small / decrease sample in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Bet")          // hold 1 / bet

	PORT_START("IN B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Take" )        // hold 2 / take
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Double Up" )   // hold 3 / double up
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Big")          // hold 5 / big / increase sample in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1       )                                    // coin in
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN     )

	PORT_START("IN C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK  )                                   // stats (keep pressed during boot for service mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                                   // cancel?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )                                   // key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE      ) PORT_IMPULSE(1)                   // service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN      )

	PORT_START("IN D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER        ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)  // reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN      )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL      )                                   // serial in?
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

	PORT_START("IN A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START      ) PORT_CODE(KEYCODE_N)    PORT_NAME("Start All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_CODE(KEYCODE_Z)    PORT_NAME("Info / Double?")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  ) PORT_IMPULSE(5)          // key in
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1         )                          // coin in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )  // stats
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE       )  // service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout (hopper error)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("IN C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN D")  // not shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL  )   // serial in?
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

	PORT_START("IN A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_NAME("Bet 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_NAME("Bet 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_NAME("Bet 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1   ) PORT_NAME("Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5  ) PORT_NAME("Big or Small 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4  ) PORT_NAME("Bet Amount")   // 1-5-10

	PORT_START("IN B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2   )           // selects music in system test / exit
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )  // top 10? / double up?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6  ) PORT_NAME("Big or Small 2")   // plays sample or advances music in system test / big or small?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?

	PORT_START("IN C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Statistics")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2    )   // key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_IMPULSE(2)   // service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )   // used?

	PORT_START("IN D")  // bits 3 and 4 shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )
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
	PORT_START("IN A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1       ) PORT_NAME("A / Play Gambling 1")         // A \__ play gambling game
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2       ) PORT_NAME("C / Play Gambling 2")         // C /
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3       ) PORT_NAME("B / Play Shoot'Em Up")        // B ___ play shoot'em up game
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1        ) PORT_NAME("Start / Take")                // start / take
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) PORT_NAME("Hold 3 / Small")              // hold 3 / small / decrease sample in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5   ) PORT_NAME("Hold 5 / Bet")                // hold 5 / bet

	PORT_START("IN B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4   ) PORT_NAME("Hold 4 / Re-Double" )         // hold 4 / re-double?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) PORT_NAME("Hold 2 / Double Up / Right")  // hold 2 / double up? / right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Raise") PORT_CODE(KEYCODE_N)  // raise
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1   ) PORT_NAME("Hold 1 / Big / Left")         // hold 1 / big / increase sample in test mode / left
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1         )                                          // coin in
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       )

	PORT_START("IN C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )                      // stats (keep pressed during boot for service mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )                      // key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE       )  PORT_IMPULSE(1)     // service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                      // pay-out

	PORT_START("IN D")  // bits 3 and 4 shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )                      // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL       )                      // serial in?
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
	PORT_START("IN A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Re-Double") PORT_CODE(KEYCODE_N)  // re-double
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF   ) PORT_NAME("Half Double")             // half double
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1        ) PORT_NAME("Start")                   // start
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) PORT_NAME("Hold 3 / Small")          // hold 3 / small / decrease sample in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BET    ) PORT_NAME("Bet")                     // bet

	PORT_START("IN B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1   ) PORT_NAME("Hold 1 / Take" )          // hold 1 / take
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   ) PORT_NAME("Double Up / Help")        // double up / help
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) PORT_NAME("Hold 2 / Big")            // hold 2 / big / increase sample in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1         )                                      // coin in
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       )

	PORT_START("IN C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )                                      // stats (keep pressed during boot for service mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )                                      // key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE       )  PORT_IMPULSE(1)                     // service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                                      // pay-out

	PORT_START("IN D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1) // reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL       )                                      // serial in?
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

	PORT_START("IN A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START      ) PORT_CODE(KEYCODE_N)    PORT_NAME("Start All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_CODE(KEYCODE_Z)    PORT_NAME("Info / Double?") // down
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("IN B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  ) PORT_IMPULSE(5)          // key in
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1         )                          // coin in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )  // stats
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE       )  // service mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout (hopper error)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("IN C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Left / Play Gambling 1")        // C \__ play gambling game
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_NAME("Play Gambling 2") PORT_CODE(KEYCODE_D)   // D /
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN    )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take / Rotate")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER      ) PORT_NAME("Play Tetris")     PORT_CODE(KEYCODE_T)   // T |__ play Tetris game

	PORT_START("IN D")  // not shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL  )   // serial in?
INPUT_PORTS_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/

static MACHINE_CONFIG_START( bishjan, subsino2_state )
	MCFG_CPU_ADD("maincpu", H83044, XTAL_44_1MHz / 3)
	MCFG_CPU_PROGRAM_MAP( bishjan_map )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", subsino2_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE( 512, 256 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_UPDATE_DRIVER(subsino2_state, screen_update_subsino2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ss9601 )
	MCFG_PALETTE_ADD( "palette", 256 )

	MCFG_VIDEO_START_OVERRIDE(subsino2_state, subsino2 )

	// sound hardware
	// SS9904?
MACHINE_CONFIG_END

/***************************************************************************
                                Magic Train
***************************************************************************/

static MACHINE_CONFIG_START( mtrain, subsino2_state )
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 8)   /* Unknown clock */
	MCFG_CPU_PROGRAM_MAP( mtrain_map )
	MCFG_CPU_IO_MAP( mtrain_io )

	MCFG_NVRAM_ADD_0FILL("nvram")

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE( 512, 256 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-32-1 )
	MCFG_SCREEN_REFRESH_RATE( 58.7270 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)   // game reads vblank state
	MCFG_SCREEN_UPDATE_DRIVER(subsino2_state, screen_update_subsino2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ss9601 )
	MCFG_PALETTE_ADD( "palette", 256 )

	MCFG_VIDEO_START_OVERRIDE(subsino2_state, mtrain )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_8_4672MHz / 8, OKIM6295_PIN7_HIGH)    // probably
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

static MACHINE_CONFIG_START( saklove, subsino2_state )
	MCFG_CPU_ADD("maincpu", I80188, XTAL_20MHz*2 )    // !! AMD AM188-EM !!
	MCFG_CPU_PROGRAM_MAP( saklove_map )
	MCFG_CPU_IO_MAP( saklove_io )

	MCFG_NVRAM_ADD_0FILL("nvram")

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE( 512, 256 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MCFG_SCREEN_REFRESH_RATE( 58.7270 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)   // game reads vblank state
	MCFG_SCREEN_UPDATE_DRIVER(subsino2_state, screen_update_subsino2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ss9601 )
	MCFG_PALETTE_ADD( "palette", 256 )

	MCFG_VIDEO_START_OVERRIDE(subsino2_state, subsino2 )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_8_4672MHz / 8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_12MHz / 4) // ? chip and clock unknown
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

/***************************************************************************
                                X-Plan
***************************************************************************/

static MACHINE_CONFIG_START( xplan, subsino2_state )
	MCFG_CPU_ADD("maincpu", I80188, XTAL_20MHz*2 )    // !! AMD AM188-EM !!
	MCFG_CPU_PROGRAM_MAP( xplan_map )
	MCFG_CPU_IO_MAP( xplan_io )
	MCFG_CPU_VBLANK_INT_DRIVER("screen", subsino2_state,  am188em_int0_irq)

	MCFG_NVRAM_ADD_0FILL("nvram")

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE( 512, 256 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MCFG_SCREEN_REFRESH_RATE( 58.7270 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)   // game reads vblank state
	MCFG_SCREEN_UPDATE_DRIVER(subsino2_state, screen_update_subsino2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ss9601 )
	MCFG_PALETTE_ADD( "palette", 256 )

	MCFG_VIDEO_START_OVERRIDE(subsino2_state, subsino2 )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_8_4672MHz / 8, OKIM6295_PIN7_HIGH)    // probably
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( xtrain, xplan )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(xtrain_io)

	MCFG_VIDEO_START_OVERRIDE(subsino2_state, xtrain )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( expcard, xplan )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(expcard_io)
MACHINE_CONFIG_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

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
                The H8/3044 is a H8/3002 with 24bit address bus and has 32k MASKROM and 2k RAM, clock input is 14.7MHz [44.1/3]
                MD0,MD1 & MD2 are configured to MODE 6 16MByte Expanded Mode with the on-chip 32k MASKROM enabled.
     CXK58257 - Sony CXK58257 32k x8 SRAM (SOP28)
      HM86171 - Hualon Microelectronics HMC HM86171 VGA 256 colour RAMDAC (DIP28)
          S-1 - ?? Probably some kind of audio OP AMP or DAC? (DIP8)
          SW1 - Push Button Test Switch
        HSync - 15.75kHz
        VSync - 60Hz

***************************************************************************/

ROM_START( bishjan )
	ROM_REGION( 0x100000, "maincpu", 0 )    // H8/3044
	ROM_LOAD( "1-v203.u21", 0x000000, 0x080000, CRC(1f891d48) SHA1(0b6a5aa8b781ba8fc133289790419aa8ea21c400) )

	ROM_REGION( 0x400000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "3-v201.u25", 0x000000, 0x100000, CRC(e013e647) SHA1(a5b0f82f3454393c1ea5e635b0d37735a25e2ea5) )
	ROM_LOAD32_BYTE( "4-v201.u26", 0x000001, 0x100000, CRC(e0d40ef1) SHA1(95f80889103a7b93080b46387274cb1ffe0c8768) )
	ROM_LOAD32_BYTE( "5-v201.u27", 0x000002, 0x100000, CRC(85067d40) SHA1(3ecf7851311a77a0dfca90775fcbf6faabe9c2ab) )
	ROM_LOAD32_BYTE( "6-v201.u28", 0x000003, 0x100000, CRC(430bd9d7) SHA1(dadf5a7eb90cf2dc20f97dbf20a4b6c8e7734fb1) )

	ROM_REGION( 0x100000, "samples", 0 )    // SS9904?
	ROM_LOAD( "2-v201.u9", 0x000000, 0x100000, CRC(ea42764d) SHA1(13fe1cd30e474f4b092949c440068e9ddca79976) )
ROM_END

DRIVER_INIT_MEMBER(subsino2_state,bishjan)
{
	UINT16 *rom = (UINT16*)memregion("maincpu")->base();

	// patch serial protection test (it always enters test mode on boot otherwise)
	rom[0x042EA/2] = 0x4008;

	// rts -> rte
	rom[0x33386/2] = 0x5670;
	rom[0x0CC5C/2] = 0x5670;
}

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
	ROM_LOAD32_BYTE( "top_card_alpha_4_v1.4.u19", 0x00001, 0x80000, CRC(1382fd45) SHA1(1d81b7e72e702f5a254e1ec5ec6adb5d8af5d467) )
	ROM_LOAD32_BYTE( "top_card_alpha_5_v1.4.u18", 0x00002, 0x80000, CRC(bbe465ac) SHA1(7a5ee6f7696e5f768ac56ccfaf0914dd56a83339) )
	ROM_LOAD32_BYTE( "top_card_alpha_6_v1.4.u17", 0x00003, 0x80000, CRC(315d7a81) SHA1(8dafa1d422d8fe306765413084e35f16e4c17d27) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "top_card-ve1.u7", 0x00000, 0x80000, CRC(0ca9bd18) SHA1(af791c78ae321104afa738564bc23f520f37e7d5) )
ROM_END

DRIVER_INIT_MEMBER(subsino2_state,expcard)
{
	UINT8 *rom = memregion("maincpu")->base();

	// patch protection test (it always enters test mode on boot otherwise)
	rom[0xed4dc-0xc0000] = 0xeb;
}

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
	ROM_REGION( 0x10000, "maincpu", 0 )
	// code starts at 0x8100!
	ROM_LOAD( "out_1v131.u17", 0x0000, 0x8100, CRC(6761be7f) SHA1(a492f8179d461a454516dde33ff04473d4cfbb27) )
	ROM_CONTINUE(              0x0000, 0x7f00 )
	ROM_RELOAD(                0xa000, 0x6000 )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "rom_4.u02", 0x00000, 0x40000, CRC(b7e65d04) SHA1(5eea1b8c1129963b3b83a59410cd0e1de70621e4) )
	ROM_LOAD32_BYTE( "rom_3.u03", 0x00001, 0x40000, CRC(cef2c079) SHA1(9ee54a08ef8db90a80a4b3568bb82ce09ee41e65) )
	ROM_LOAD32_BYTE( "rom_2.u04", 0x00002, 0x40000, CRC(a794f287) SHA1(7b9c0d57224a700f49e55ba5aeb7ed9d35a71e02) )
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

DRIVER_INIT_MEMBER(subsino2_state,mtrain)
{
	subsino_decrypt(machine(), crsbingo_bitswaps, crsbingo_xors, 0x8000);

	// patch serial protection test (it always enters test mode on boot otherwise)
	UINT8 *rom = memregion("maincpu")->base();
	rom[0x0cec] = 0x18;
	rom[0xb037] = 0x18;

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
	ROM_LOAD32_BYTE( "4.u28", 0x000001, 0x80000, CRC(f8db7ab6) SHA1(3af4e92ab27edc980eccecdbbbb431e1d2101059) )
	ROM_LOAD32_BYTE( "5.u29", 0x000002, 0x80000, CRC(c6ca1764) SHA1(92bfa19e116d358b03164f2448a28e7524e3cc62) )
	ROM_LOAD32_BYTE( "6.u30", 0x000003, 0x80000, CRC(5823c39e) SHA1(257e862ac736ff403ce9c70bbdeed340dfe168af) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "2.u10", 0x00000, 0x80000, CRC(4f70125c) SHA1(edd5e6bd47b9a4fa3c4057cb4a85544241fe483d) )
ROM_END

DRIVER_INIT_MEMBER(subsino2_state,saklove)
{
	UINT8 *rom = memregion("maincpu")->base();

	// patch serial protection test (it always enters test mode on boot otherwise)
	rom[0x0e029] = 0xeb;
}

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
	ROM_LOAD32_BYTE( "x-plan_rom_4_v102b.u19", 0x00001, 0x80000, CRC(744be318) SHA1(1c1f2a9e1da77d9bc1bf897072df44a681a53079) )
	ROM_LOAD32_BYTE( "x-plan_rom_5_v102b.u18", 0x00002, 0x80000, CRC(7e89c9b3) SHA1(9e3fea0d74cac48c068a15595f2342a2b0b3f747) )
	ROM_LOAD32_BYTE( "x-plan_rom_6_v102b.u17", 0x00003, 0x80000, CRC(a86ca3b9) SHA1(46aa86b9c62aa0a4e519eb06c72c2d540489afee) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "x-plan_rom_2_v100.u7", 0x00000, 0x80000, CRC(c742b5c8) SHA1(646960508be738824bfc578c1b21355c17e05010) )
ROM_END

DRIVER_INIT_MEMBER(subsino2_state,xplan)
{
	UINT8 *rom = memregion("maincpu")->base();

	// patch protection test (it always enters test mode on boot otherwise)
	rom[0xeded9-0xc0000] = 0xeb;
}

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
	ROM_LOAD( "x-train(top)_out_1_v1.3.u14", 0x00000, 0x40000, CRC(019812b4) SHA1(33c73c53f8cf730c35fa310868f5b8360dfaad9e) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "x-train_rom_3_v1.2.u20", 0x00000, 0x80000, CRC(0e18ca82) SHA1(8fbc62a16ab109994086f58c9b9915a92bda0448) )
	ROM_LOAD32_BYTE( "x-train_rom_4_v1.2.u19", 0x00001, 0x80000, CRC(959fa749) SHA1(d39fcedd1d13d9f86c1915d7dcff7d024739a6fa) )
	ROM_LOAD32_BYTE( "x-train_rom_5_v1.2.u18", 0x00002, 0x80000, CRC(d0e8279f) SHA1(174483871c9e98936b37cc6cede71b64e19cae90) )
	ROM_LOAD32_BYTE( "x-train_rom_6_v1.2.u17", 0x00003, 0x80000, CRC(289ae881) SHA1(b3f8db43d86078688ad56a04d1e7d7a825df60d7) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "x-train_rom_2_v1.2.u7", 0x00000, 0x80000, CRC(aae563ff) SHA1(97db845d7e3d343bd70352371cb27b16faacca7f) )
ROM_END

DRIVER_INIT_MEMBER(subsino2_state,xtrain)
{
	UINT8 *rom = memregion("maincpu")->base();

	// patch protection test (it always enters test mode on boot otherwise)
	rom[0xe190f-0xc0000] = 0xeb;
}

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
	ROM_LOAD( "panda(top)-novam_1-v1.4.u14", 0x00000, 0x40000, CRC(75b12734) SHA1(d05d0cba2de9d7021736bbd7c67d9b3c552374ee) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "panda-novam_3-v1.4.0.u20", 0x00000, 0x80000, CRC(2d5ab471) SHA1(3df42b7f762d738a4409498984e90c80625fae1f) )
	ROM_LOAD32_BYTE( "panda-novam_4-v1.4.1.u19", 0x00001, 0x80000, CRC(a4b6985c) SHA1(1d3d23f7c9e775439a2d1a4c68b703bf51b0350f) )
	ROM_LOAD32_BYTE( "panda-novam_5-v1.4.2.u18", 0x00002, 0x80000, CRC(716f7500) SHA1(971589a2530a0d4152bb68dbc7794985525a837d) )
	ROM_LOAD32_BYTE( "panda-novam_6-v1.4.3.u17", 0x00003, 0x80000, CRC(10f0c21a) SHA1(400e53bf3dd6fe6f2dd679ed5151fb4400a6ec9f) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "panda-novam_2-v1.4.u7", 0x00000, 0x80000, CRC(d1debec8) SHA1(9086975e5bef2066a688ab3c1df3b384f59e507d) )
ROM_END

DRIVER_INIT_MEMBER(subsino2_state,ptrain)
{
	UINT8 *rom = memregion("maincpu")->base();

	// patch protection test (it always enters test mode on boot otherwise)
	rom[0xe1b08-0xc0000] = 0xeb;
}

/***************************************************************************

Water-Nymph (Ver. 1.4)
(c) 1996 Subsino

Same PCB as Magic Train

***************************************************************************/

ROM_START( wtrnymph )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// code starts at 0x8100!
	ROM_LOAD( "ocean-n tetris_1 v1.4.u17", 0x0000, 0x8100, CRC(c7499123) SHA1(39a9ea6d927ee839cfb127747e5e3df3535af098) )
	ROM_CONTINUE(                          0x0000, 0x7f00 )
	ROM_RELOAD(                            0xa000, 0x6000 )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "ocean-n tetris_2 v1.21.u2", 0x00000, 0x40000, CRC(813aac90) SHA1(4555adf8dc363359b10f1d5cfae2dcebed411679) )
	ROM_LOAD32_BYTE( "ocean-n tetris_3 v1.21.u3", 0x00001, 0x40000, CRC(83c39379) SHA1(e7f9315d19370c18b664b759e433052a88f8c146) )
	ROM_LOAD32_BYTE( "ocean-n tetris_4 v1.21.u4", 0x00002, 0x40000, CRC(6fc64b42) SHA1(80110d7dae28cca5e39c8a7c2ceebf589116ae23) )
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
ROM_END

DRIVER_INIT_MEMBER(subsino2_state,wtrnymph)
{
	subsino_decrypt(machine(), victor5_bitswaps, victor5_xors, 0x8000);

	// patch serial protection test (it always enters test mode on boot otherwise)
	UINT8 *rom = memregion("maincpu")->base();
	rom[0x0d79] = 0x18;
	rom[0xc1cf] = 0x18;
	rom[0xc2a9] = 0x18;
	rom[0xc2d7] = 0x18;
}

GAME( 1996, mtrain,   0,        mtrain,   mtrain,   subsino2_state, mtrain,   ROT0, "Subsino",        "Magic Train (Ver. 1.31)",              0 )
GAME( 1996, wtrnymph, 0,        mtrain,   wtrnymph, subsino2_state, wtrnymph, ROT0, "Subsino",        "Water-Nymph (Ver. 1.4)",               0 )
GAME( 1998, expcard,  0,        expcard,  expcard,  subsino2_state, expcard,  ROT0, "American Alpha", "Express Card / Top Card (Ver. 1.5)",   0 )
GAME( 1998, saklove,  0,        saklove,  saklove,  subsino2_state, saklove,  ROT0, "Subsino",        "Ying Hua Lian 2.0 (China, Ver. 1.02)", 0 )
GAME( 1999, xtrain,   0,        xtrain,   xtrain,   subsino2_state, xtrain,   ROT0, "Subsino",        "X-Train (Ver. 1.3)",                   0 )
GAME( 1999, ptrain,   0,        xtrain,   xtrain,   subsino2_state, ptrain,   ROT0, "Subsino",        "Panda Train (Novamatic 1.7)",          0 )
GAME( 1999, bishjan,  0,        bishjan,  bishjan,  subsino2_state, bishjan,  ROT0, "Subsino",        "Bishou Jan (Japan, Ver. 2.03)",        MACHINE_NO_SOUND )
GAME( 2006, xplan,    0,        xplan,    xplan,    subsino2_state, xplan,    ROT0, "Subsino",        "X-Plan (Ver. 1.01)",                   0 )
