/***************************************************************************************************

  Sky Lancer / Mad Zoo / Super Star '97
  Bordun International.

  Original preliminary driver by Luca Elia.
  Additional Work: Roberto Fresca & David Haywood.

****************************************************************************************************

  Notes:

  - Some of the tiles look badly scaled down, and others appear to have columns swapped.
    This might actually be correct.

  - Skylncr and madzoo can run with the same program roms, they're basically graphics swaps.

  - To enter the Service Mode, press F2. All the game settings are there.
    Change regular values using DIP switches, and jackpot value using STOP2 and STOP3.
    To exit the mode, press START. You must reset the machine (F3) to update the changes.

  - Press key 0 to navigate between statistics pages. Press START to exit the mode.


  TODO:

  - Proper M5M82C255 device emulation.

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

***************************************************************************************************/


#define MASTER_CLOCK        XTAL_12MHz  /* confirmed */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "machine/nvram.h"


class skylncr_state : public driver_device
{
public:
	skylncr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_palette(*this, "palette")  { }

	tilemap_t *m_tmap;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_reeltiles_1_ram;
	required_shared_ptr<UINT8> m_reeltiles_2_ram;
	required_shared_ptr<UINT8> m_reeltiles_3_ram;
	required_shared_ptr<UINT8> m_reeltiles_4_ram;
	required_shared_ptr<UINT8> m_reeltileshigh_1_ram;
	required_shared_ptr<UINT8> m_reeltileshigh_2_ram;
	required_shared_ptr<UINT8> m_reeltileshigh_3_ram;
	required_shared_ptr<UINT8> m_reeltileshigh_4_ram;
	tilemap_t *m_reel_1_tilemap;
	tilemap_t *m_reel_2_tilemap;
	tilemap_t *m_reel_3_tilemap;
	tilemap_t *m_reel_4_tilemap;
	required_shared_ptr<UINT8> m_reelscroll1;
	required_shared_ptr<UINT8> m_reelscroll2;
	required_shared_ptr<UINT8> m_reelscroll3;
	required_shared_ptr<UINT8> m_reelscroll4;
	UINT8 m_nmi_enable;
	int m_color;
	int m_color2;
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
	DECLARE_WRITE8_MEMBER(skylncr_paletteram_w);
	DECLARE_WRITE8_MEMBER(skylncr_paletteram2_w);
	DECLARE_WRITE8_MEMBER(reelscroll1_w);
	DECLARE_WRITE8_MEMBER(reelscroll2_w);
	DECLARE_WRITE8_MEMBER(reelscroll3_w);
	DECLARE_WRITE8_MEMBER(reelscroll4_w);
	DECLARE_WRITE8_MEMBER(skylncr_coin_w);
	DECLARE_READ8_MEMBER(ret_ff);
	DECLARE_READ8_MEMBER(ret_00);
	DECLARE_WRITE8_MEMBER(skylncr_nmi_enable_w);
	DECLARE_DRIVER_INIT(skylncr);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_1_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_2_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_3_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_4_tile_info);
	virtual void video_start();
	UINT32 screen_update_skylncr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(skylncr_vblank_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
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
	UINT16 code = m_videoram[ tile_index ] + (m_colorram[ tile_index ] << 8);
	SET_TILE_INFO_MEMBER(0, code, 0, TILE_FLIPYX( 0 ));
}

TILE_GET_INFO_MEMBER(skylncr_state::get_reel_1_tile_info)
{
	UINT8 bank = 1;
	UINT16 code = m_reeltiles_1_ram[ tile_index ] + (m_reeltileshigh_1_ram[ tile_index ] << 8);
//  if (code > 0x200)
//      bank = 2;
	SET_TILE_INFO_MEMBER(bank, code, 0, TILE_FLIPYX( 0 ));
}

TILE_GET_INFO_MEMBER(skylncr_state::get_reel_2_tile_info)
{
	UINT16 code = m_reeltiles_2_ram[ tile_index ] + (m_reeltileshigh_2_ram[ tile_index ] << 8);
	SET_TILE_INFO_MEMBER(1, code, 0, TILE_FLIPYX( 0 ));
}

TILE_GET_INFO_MEMBER(skylncr_state::get_reel_3_tile_info)
{
	UINT16 code = m_reeltiles_3_ram[ tile_index ] + (m_reeltileshigh_3_ram[ tile_index ] << 8);
	SET_TILE_INFO_MEMBER(1, code, 0, TILE_FLIPYX( 0 ));
}

TILE_GET_INFO_MEMBER(skylncr_state::get_reel_4_tile_info)
{
	UINT16 code = m_reeltiles_4_ram[ tile_index ] + (m_reeltileshigh_4_ram[ tile_index ] << 8);
	SET_TILE_INFO_MEMBER(1, code, 0, TILE_FLIPYX( 0 ));
}


void skylncr_state::video_start()
{
	m_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 0x40, 0x20    );

	m_reel_1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_reel_1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 32, 64, 8 );
	m_reel_2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_reel_2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 32, 64, 8 );
	m_reel_3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_reel_3_tile_info),this), TILEMAP_SCAN_ROWS, 8, 32, 64, 8 );
	m_reel_4_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skylncr_state::get_reel_4_tile_info),this), TILEMAP_SCAN_ROWS, 8, 32, 64, 8 );

	m_reel_2_tilemap->set_scroll_cols(0x40);
	m_reel_3_tilemap->set_scroll_cols(0x40);
	m_reel_4_tilemap->set_scroll_cols(0x40);

	m_reel_2_tilemap->set_transparent_pen(0);
	m_reel_3_tilemap->set_transparent_pen(0);
	m_reel_4_tilemap->set_transparent_pen(0);


	m_tmap->set_transparent_pen(0);
}


UINT32 skylncr_state::screen_update_skylncr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	bitmap.fill(0, cliprect);
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


WRITE8_MEMBER(skylncr_state::skylncr_paletteram_w)
{
	if (offset == 0)
	{
		m_color = data;
	}
	else
	{
		int r,g,b;
		m_generic_paletteram_8[m_color] = data;

		r = m_generic_paletteram_8[(m_color/3 * 3) + 0];
		g = m_generic_paletteram_8[(m_color/3 * 3) + 1];
		b = m_generic_paletteram_8[(m_color/3 * 3) + 2];
		r = (r << 2) | (r >> 4);
		g = (g << 2) | (g >> 4);
		b = (b << 2) | (b >> 4);

		m_palette->set_pen_color(m_color / 3, rgb_t(r, g, b));
		m_color = (m_color + 1) % (0x100 * 3);
	}
}

WRITE8_MEMBER(skylncr_state::skylncr_paletteram2_w)
{
	if (offset == 0)
	{
		m_color2 = data;
	}
	else
	{
		int r,g,b;
		m_generic_paletteram2_8[m_color2] = data;

		r = m_generic_paletteram2_8[(m_color2/3 * 3) + 0];
		g = m_generic_paletteram2_8[(m_color2/3 * 3) + 1];
		b = m_generic_paletteram2_8[(m_color2/3 * 3) + 2];
		r = (r << 2) | (r >> 4);
		g = (g << 2) | (g >> 4);
		b = (b << 2) | (b >> 4);

		m_palette->set_pen_color(0x100 + m_color2 / 3, rgb_t(r, g, b));
		m_color2 = (m_color2 + 1) % (0x100 * 3);
	}
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
	coin_counter_w( machine(), 0, data & 0x04 );
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


/**************************************
*             Memory Map              *
**************************************/

static ADDRESS_MAP_START( mem_map_skylncr, AS_PROGRAM, 8, skylncr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(skylncr_videoram_w ) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(skylncr_colorram_w ) AM_SHARE("colorram")

	AM_RANGE(0x9800, 0x99ff) AM_RAM_WRITE(reeltiles_1_w ) AM_SHARE("reeltiles_1_ram")
	AM_RANGE(0x9a00, 0x9bff) AM_RAM_WRITE(reeltiles_2_w ) AM_SHARE("reeltiles_2_ram")
	AM_RANGE(0x9c00, 0x9dff) AM_RAM_WRITE(reeltiles_3_w ) AM_SHARE("reeltiles_3_ram")
	AM_RANGE(0x9e00, 0x9fff) AM_RAM_WRITE(reeltiles_4_w ) AM_SHARE("reeltiles_4_ram")
	AM_RANGE(0xa000, 0xa1ff) AM_RAM_WRITE(reeltileshigh_1_w ) AM_SHARE("rthigh_1_ram")
	AM_RANGE(0xa200, 0xa3ff) AM_RAM_WRITE(reeltileshigh_2_w ) AM_SHARE("rthigh_2_ram")
	AM_RANGE(0xa400, 0xa5ff) AM_RAM_WRITE(reeltileshigh_3_w ) AM_SHARE("rthigh_3_ram")
	AM_RANGE(0xa600, 0xa7ff) AM_RAM_WRITE(reeltileshigh_4_w ) AM_SHARE("rthigh_4_ram")

	AM_RANGE(0xaa55, 0xaa55) AM_READ(ret_ff )

	AM_RANGE(0xb000, 0xb03f) AM_RAM_WRITE(reelscroll1_w) AM_SHARE("reelscroll1")
	AM_RANGE(0xb040, 0xb07f) AM_RAM_WRITE(reelscroll1_w)
	AM_RANGE(0xb080, 0xb0bf) AM_RAM_WRITE(reelscroll1_w)
	AM_RANGE(0xb0c0, 0xb0ff) AM_RAM_WRITE(reelscroll1_w)
	AM_RANGE(0xb100, 0xb13f) AM_RAM_WRITE(reelscroll1_w)
	AM_RANGE(0xb140, 0xb17f) AM_RAM_WRITE(reelscroll1_w)
	AM_RANGE(0xb180, 0xb1bf) AM_RAM_WRITE(reelscroll1_w)
	AM_RANGE(0xb1c0, 0xb1ff) AM_RAM_WRITE(reelscroll1_w)

	AM_RANGE(0xb200, 0xb23f) AM_RAM_WRITE(reelscroll2_w) AM_SHARE("reelscroll2")
	AM_RANGE(0xb240, 0xb27f) AM_RAM_WRITE(reelscroll2_w)
	AM_RANGE(0xb280, 0xb2bf) AM_RAM_WRITE(reelscroll2_w)
	AM_RANGE(0xb2c0, 0xb2ff) AM_RAM_WRITE(reelscroll2_w)
	AM_RANGE(0xb300, 0xb33f) AM_RAM_WRITE(reelscroll2_w)
	AM_RANGE(0xb340, 0xb37f) AM_RAM_WRITE(reelscroll2_w)
	AM_RANGE(0xb380, 0xb3bf) AM_RAM_WRITE(reelscroll2_w)
	AM_RANGE(0xb3c0, 0xb3ff) AM_RAM_WRITE(reelscroll2_w)

	AM_RANGE(0xb400, 0xb43f) AM_RAM_WRITE(reelscroll3_w) AM_SHARE("reelscroll3")
	AM_RANGE(0xb440, 0xb47f) AM_RAM_WRITE(reelscroll3_w)
	AM_RANGE(0xb480, 0xb4bf) AM_RAM_WRITE(reelscroll3_w)
	AM_RANGE(0xb4c0, 0xb4ff) AM_RAM_WRITE(reelscroll3_w)
	AM_RANGE(0xb500, 0xb53f) AM_RAM_WRITE(reelscroll3_w)
	AM_RANGE(0xb540, 0xb57f) AM_RAM_WRITE(reelscroll3_w)
	AM_RANGE(0xb580, 0xb5bf) AM_RAM_WRITE(reelscroll3_w)
	AM_RANGE(0xb5c0, 0xb5ff) AM_RAM_WRITE(reelscroll3_w)

	AM_RANGE(0xb600, 0xb63f) AM_RAM_WRITE(reelscroll4_w) AM_SHARE("reelscroll4")
	AM_RANGE(0xb640, 0xb67f) AM_RAM_WRITE(reelscroll4_w)
	AM_RANGE(0xb680, 0xb6bf) AM_RAM_WRITE(reelscroll4_w)
	AM_RANGE(0xb6c0, 0xb6ff) AM_RAM_WRITE(reelscroll4_w)
	AM_RANGE(0xb700, 0xb73f) AM_RAM_WRITE(reelscroll4_w)
	AM_RANGE(0xb740, 0xb77f) AM_RAM_WRITE(reelscroll4_w)
	AM_RANGE(0xb780, 0xb7bf) AM_RAM_WRITE(reelscroll4_w)
	AM_RANGE(0xb7c0, 0xb7ff) AM_RAM_WRITE(reelscroll4_w)

	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( io_map_skylncr, AS_IO, 8, skylncr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)    /* Input Ports */
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)    /* Input Ports */

	AM_RANGE(0x20, 0x20) AM_WRITE(skylncr_coin_w )

	AM_RANGE(0x30, 0x31) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x31, 0x31) AM_DEVREAD("aysnd", ay8910_device, data_r)

	AM_RANGE(0x40, 0x41) AM_WRITE(skylncr_paletteram_w )
	AM_RANGE(0x50, 0x51) AM_WRITE(skylncr_paletteram2_w )

	AM_RANGE(0x70, 0x70) AM_WRITE(skylncr_nmi_enable_w )
ADDRESS_MAP_END


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


/**************************************
*           Graphics Decode           *
**************************************/

static GFXDECODE_START( skylncr )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x8,        0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8,       0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_rot,   0, 2 )
GFXDECODE_END

static GFXDECODE_START( sstar97 )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x8_alt,    0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_alt,   0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout8x32x8_alt,   0x100, 2 )
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW) PORT_NAME("Down/Low") PORT_CODE(KEYCODE_S)
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH) PORT_NAME("Up/High") PORT_CODE(KEYCODE_A)

	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")

	PORT_START("IN4")   /* $12 (PPI1 port C) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* Settings */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW) PORT_NAME("Low") PORT_CODE(KEYCODE_S)
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("High") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")

	PORT_START("IN4")   /* $12 (PPI1 port C) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* Settings */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
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


/**************************************
*       PPI 8255 (x2) Interface       *
**************************************/

static I8255A_INTERFACE( ppi8255_0_intf )
{
	DEVCB_INPUT_PORT("IN1"),            /* Port A read */
	DEVCB_NULL,                         /* Port A write */
	DEVCB_INPUT_PORT("IN2"),            /* Port B read */
	DEVCB_NULL,                         /* Port B write */
	DEVCB_INPUT_PORT("DSW1"),           /* Port C read */
	DEVCB_NULL                          /* Port C write */
};

static I8255A_INTERFACE( ppi8255_1_intf )
{
	DEVCB_INPUT_PORT("DSW2"),           /* Port A read */
	DEVCB_NULL,                         /* Port A write */
	DEVCB_INPUT_PORT("IN3"),            /* Port B read */
	DEVCB_NULL,                         /* Port B write */
	DEVCB_INPUT_PORT("IN4"),            /* Port C read */
	DEVCB_NULL                          /* Port C write */
};


/**********************************
*       AY-3-8910 Interface       *
**********************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW3"),
	DEVCB_INPUT_PORT("DSW4"),
	DEVCB_NULL,
	DEVCB_NULL
};


// It runs in IM 0, thus needs an opcode on the data bus
INTERRUPT_GEN_MEMBER(skylncr_state::skylncr_vblank_interrupt)
{
	if (m_nmi_enable) device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


/*************************************
*           Machine Driver           *
*************************************/

static MACHINE_CONFIG_START( skylncr, skylncr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(mem_map_skylncr)
	MCFG_CPU_IO_MAP(io_map_skylncr)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", skylncr_state,  skylncr_vblank_interrupt)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* 1x M5M82C255, or 2x PPI8255 */
	MCFG_I8255A_ADD( "ppi8255_0", ppi8255_0_intf )
	MCFG_I8255A_ADD( "ppi8255_1", ppi8255_1_intf )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(skylncr_state, screen_update_skylncr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", skylncr)
	MCFG_PALETTE_ADD("palette", 0x200)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/8)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sstar97, skylncr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_GFXDECODE_MODIFY("gfxdecode", sstar97)
MACHINE_CONFIG_END


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
  Super Star '97
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

  One extra ROM (u48) is blank.
  Sure is the one that store the palette at offset $C000,
  among the missing graphics.

  Also graphics ROMs are half the standard size and
  they lack of at least 4 extra big girl pictures.

  BP 170 to see the palette registers...

*/
ROM_START( sstar97 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27256.u15",    0x0000, 0x8000, CRC(a5da4f92) SHA1(82ac70bd379649f130db017aa226d0247db0f3cd) )
	ROM_LOAD( "unknown.u48",  0x8000, 0x8000, BAD_DUMP CRC(9f4c02e3) SHA1(05975184130ea7dd3bb5d32eff77b585bd53e6b5) )   // palette borrowed from other game

	ROM_REGION( 0x40000, "gfx1", 0 )    // All ROMs are 28-pins mask ROMs dumped as 27512. Should be dumped as Fujitsu MB831000 or TC531000 (mask ROM).
	ROM_LOAD16_BYTE( "bor_dun_4.u23", 0x00000, 0x10000, BAD_DUMP CRC(82c9db19) SHA1(3611fb59bb7c962c7fabe7a29fa72b632fa69bed) )
	ROM_LOAD16_BYTE( "bor_dun_2.u25", 0x00001, 0x10000, BAD_DUMP CRC(42ee9b7a) SHA1(b39f677f58072ea7dcd7f49208be1a7b70bdc5e5) )
	ROM_LOAD16_BYTE( "bor_dun_3.u24", 0x20000, 0x10000, BAD_DUMP CRC(6d70879b) SHA1(83cbe67cda95e5f3d95065015f6b1b2044b88989) )
	ROM_LOAD16_BYTE( "bor_dun_1.u26", 0x20001, 0x10000, BAD_DUMP CRC(1b8b84ac) SHA1(b914bad0b1fb58cf581d1227e8127c6afb906fb7) )

	ROM_REGION( 0x40000, "gfx2", 0 )    // All ROMs are 28-pins mask ROMs dumped as 27512. Should be dumped as Fujitsu MB831000 or TC531000 (mask ROM).
	ROM_LOAD16_BYTE( "bor_dun_8.u19", 0x00000, 0x10000, BAD_DUMP CRC(daf651a7) SHA1(d4e472aa90aa2b52c997b2f2272007b139e3cbc2) )
	ROM_LOAD16_BYTE( "bor_dun_6.u21", 0x00001, 0x10000, BAD_DUMP CRC(1d88bc70) SHA1(49246d96a4ce2b8e9b10e928d7dd13973feac883) )
	ROM_LOAD16_BYTE( "bor_dun_7.u20", 0x20000, 0x10000, BAD_DUMP CRC(7e28ba2f) SHA1(ac8d4e95efce87456f569a71650bd7afcb59095e) )
	ROM_LOAD16_BYTE( "bor_dun_5.u22", 0x20001, 0x10000, BAD_DUMP CRC(52f98575) SHA1(b786c441d5ef47ff4cb50835c6ac6889cb169c6e) )
ROM_END


/**********************************
*           Driver Init           *
**********************************/

DRIVER_INIT_MEMBER(skylncr_state,skylncr)
{
	m_generic_paletteram_8.allocate(0x100 * 3);
	m_generic_paletteram2_8.allocate(0x100 * 3);
}


/****************************************************
*                  Game Drivers                     *
****************************************************/

/*    YEAR  NAME      PARENT   MACHINE   INPUT    STATE           INIT     ROT    COMPANY                 FULLNAME                               FLAGS  */
GAME( 1995, skylncr,  0,       skylncr,  skylncr, skylncr_state,  skylncr, ROT0, "Bordun International", "Sky Lancer (Bordun, version U450C)",   0 )
GAME( 1995, butrfly,  0,       skylncr,  skylncr, skylncr_state,  skylncr, ROT0, "Bordun International", "Butterfly Video Game (version U350C)", 0 )
GAME( 1995, madzoo,   0,       skylncr,  skylncr, skylncr_state,  skylncr, ROT0, "Bordun International", "Mad Zoo (version U450C)",              0 )
GAME( 1995, leader,   0,       skylncr,  skylncr, skylncr_state,  skylncr, ROT0, "bootleg",              "Leader",                               GAME_NOT_WORKING )
GAME( 199?, sstar97,  0,       sstar97,  sstar97, skylncr_state,  skylncr, ROT0, "Bordun International", "Super Star '97 (version V153B)",       GAME_WRONG_COLORS | GAME_IMPERFECT_GRAPHICS )
