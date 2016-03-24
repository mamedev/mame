// license:BSD-3-Clause
// copyright-holders:Luca Elia
/**************************************************************************************************************************************


  Jockey Club II driver

  various hardware revisions
  all use a MC68EC020FG16 CPU

    extra customs

  jclub2  : SETA ST-0032 (video + sound? **1),                       SETA ST-0013, SETA ST-0017     ( PCB E79-001 rev 01a )
  jclub2o : SETA ST-0020 (video), SETA ST-0016 (sound + video? **2), SETA ST-0013, SETA ST-0017     ( E06-00409 /  E06-00407 )
  darkhors: (bootleg hardware)

  ------

  ST-0020 is a zooming sprite / blitter chip, also used by gdfs in ssv.c (see st0020.c)
  ST-0016 is a z80 with integrated video + sound capabilities **1
  ST-0017 is also used in srmp6.c, what is it? I/O? RLE?

  ST-0013 doesn't seem to be used anywhere else?
  ST-0032 doesn't seem to be used anywhere else? **2

  ------

  **1 ST-0032 seems to be similar to ST-0020 but the ram / list formats aren't the same, maybe it handles sound as there doesn't
   seem to be any other dedicated sound chip

  **2 ST-0016 video functionality is probably not used



**************************************************************************************************************************************/





/*

                            -= Dark Horse =-

                driver by   Luca Elia (l.elia@tin.it)

A bootleg of 1997 Seta's "Jockey Club II" on inferior hardware:

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

Notes:

- Sprite ram is parsed and converted from the original format (gdfs zooming sprites +
  ssv row sprites?, see ssv.c) to this harware's plain sprites + 2 tilemaps.
- During boot press: Test (F1) for test menu; Service (9) for config menu.
- This game uses 72 input keys!

To do:

- Emulate coin double sensor. At the moment you will need to enter the config menu
  (9 during boot), change to single sensor, press 9 again then 1 to save.
- Remove the patch to pass the eeprom test.
- Extend palette to 0x20000 entries (palette_start prevents more than 0x10000).
- Fix the disalignment between sprites and tilemap (gap in the fence) during play,
  without breaking the other screens, which are fine.
- Implement a few missing inputs / outputs.
- Correct clocks

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "machine/st0016.h"
#include "video/st0020.h"
#include "machine/nvram.h"

class darkhors_state : public driver_device
{
public:
	darkhors_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tmapram(*this, "tmapram"),
		m_tmapscroll(*this, "tmapscroll"),
		m_tmapram2(*this, "tmapram2"),
		m_tmapscroll2(*this, "tmapscroll2"),
		m_spriteram(*this, "spriteram"),
		m_gdfs_st0020(*this, "st0020_spr"),
		m_gamecpu(*this, "gamecpu"),
		m_eeprom(*this, "eeprom"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	tilemap_t *m_tmap;
	tilemap_t *m_tmap2;
	optional_shared_ptr<UINT32> m_tmapram;
	optional_shared_ptr<UINT32> m_tmapscroll;
	optional_shared_ptr<UINT32> m_tmapram2;
	optional_shared_ptr<UINT32> m_tmapscroll2;
	UINT32 m_input_sel;
	UINT32 m_input_sel_jc_1p;
	UINT32 m_input_sel_jc_2p;
	int m_jclub2_gfx_index;
	optional_shared_ptr<UINT32> m_spriteram;
	optional_device<st0020_device> m_gdfs_st0020;
	required_device<cpu_device> m_gamecpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<palette_device> m_palette;
	DECLARE_WRITE32_MEMBER(darkhors_tmapram_w);
	DECLARE_WRITE32_MEMBER(darkhors_tmapram2_w);
	DECLARE_WRITE32_MEMBER(darkhors_input_sel_w);
	DECLARE_READ32_MEMBER(darkhors_input_sel_r);
	DECLARE_READ32_MEMBER(p_4e0000);
	DECLARE_READ32_MEMBER(p_580004);
	DECLARE_WRITE32_MEMBER(jclub2_input_sel_w_p2);
	DECLARE_WRITE32_MEMBER(jclub2_input_sel_w_p1);
	DECLARE_WRITE32_MEMBER(darkhors_unk1_w);
	DECLARE_WRITE32_MEMBER(darkhors_eeprom_w);
	DECLARE_WRITE32_MEMBER(jclub2o_eeprom_w);
	DECLARE_DRIVER_INIT(darkhors);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	DECLARE_VIDEO_START(darkhors);
	DECLARE_VIDEO_START(jclub2);
	DECLARE_VIDEO_START(jclub2o);
	UINT32 screen_update_darkhors(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_jclub2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_jclub2o(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(darkhors_irq);
	void draw_sprites_darkhors(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<gfxdecode_device> m_gfxdecode;

	WRITE8_MEMBER(st0016_rom_bank_w); // temp?
};


#define DARKHORS_DEBUG  0

/***************************************************************************


                                Video Hardware


***************************************************************************/





TILE_GET_INFO_MEMBER(darkhors_state::get_tile_info_0)
{
	UINT16 tile     =   m_tmapram[tile_index] >> 16;
	UINT16 color    =   m_tmapram[tile_index] & 0xffff;
	SET_TILE_INFO_MEMBER(0, tile/2, (color & 0x200) ? (color & 0x1ff) : ((color & 0x0ff) * 4) , 0);
}

TILE_GET_INFO_MEMBER(darkhors_state::get_tile_info_1)
{
	UINT16 tile     =   m_tmapram2[tile_index] >> 16;
	UINT16 color    =   m_tmapram2[tile_index] & 0xffff;
	SET_TILE_INFO_MEMBER(0, tile/2, (color & 0x200) ? (color & 0x1ff) : ((color & 0x0ff) * 4) , 0);
}

WRITE32_MEMBER(darkhors_state::darkhors_tmapram_w)
{
	COMBINE_DATA(&m_tmapram[offset]);
	m_tmap->mark_tile_dirty(offset);
}
WRITE32_MEMBER(darkhors_state::darkhors_tmapram2_w)
{
	COMBINE_DATA(&m_tmapram2[offset]);
	m_tmap2->mark_tile_dirty(offset);
}

void darkhors_state::draw_sprites_darkhors(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT32 *s       =   m_spriteram;
	UINT32 *end     =   m_spriteram + 0x02000/4;

	for ( ; s < end; s += 8/4 )
	{
		int attr, code, color, sx, sy, flipx, flipy;

		sx      =       (s[ 0 ] >> 16);
		sy      =       (s[ 0 ] & 0xffff);
		attr    =       (s[ 1 ] >> 16);
		code    =       (s[ 1 ] & 0xffff);

		// Last sprite
		if (sx & 0x8000) break;

		flipx   =   0;
		flipy   =   0;
		color   =   (attr & 0x0200) ? (attr & 0x1ff) : (attr & 0x1ff) * 4;

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

VIDEO_START_MEMBER(darkhors_state,darkhors)
{
	m_tmap =    &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(darkhors_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS,16,16, 0x40,0x40);
	m_tmap2= &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(darkhors_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS,16,16, 0x40,0x40);
	m_tmap->set_transparent_pen(0);
	m_tmap2->set_transparent_pen(0);

	m_gfxdecode->gfx(0)->set_granularity(64); /* 256 colour sprites with palette selectable on 64 colour boundaries */
}

UINT32 darkhors_state::screen_update_darkhors(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#if DARKHORS_DEBUG
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
	if (layers_ctrl & 4)    draw_sprites_darkhors(bitmap,cliprect);


#if DARKHORS_DEBUG
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


WRITE32_MEMBER(darkhors_state::jclub2o_eeprom_w)        //seiko s-2929 is used on old style pcb
{
	if (data & ~0xff000000)
		logerror("%s: Unknown EEPROM bit written %08X\n",machine().describe_context(),data);

	if ( ACCESSING_BITS_24_31 )
	{
		// latch the bit
		m_eeprom->di_write((data & 0x01000000) >> 24);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x08000000) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x04000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}

WRITE32_MEMBER(darkhors_state::darkhors_eeprom_w)
{
	if (data & ~0xff000000)
		logerror("%s: Unknown EEPROM bit written %08X\n",machine().describe_context(),data);

	if ( ACCESSING_BITS_24_31 )
	{
		// latch the bit
		m_eeprom->di_write((data & 0x04000000) >> 26);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x01000000) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x02000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}


WRITE32_MEMBER(darkhors_state::darkhors_input_sel_w)
{
	COMBINE_DATA(&m_input_sel);
//  if (ACCESSING_BITS_16_31)    popmessage("%04X",data >> 16);
}

static int mask_to_bit( int mask )
{
	switch( mask )
	{
		default:
		case 0x01:  return 0;
		case 0x02:  return 1;
		case 0x04:  return 2;
		case 0x08:  return 3;
		case 0x10:  return 4;
		case 0x20:  return 5;
		case 0x40:  return 6;
		case 0x80:  return 7;
	}
}

READ32_MEMBER(darkhors_state::darkhors_input_sel_r)
{
	// from bit mask to bit number
	int bit_p1 = mask_to_bit((m_input_sel & 0x00ff0000) >> 16);
	int bit_p2 = mask_to_bit((m_input_sel & 0xff000000) >> 24);
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7" };

	return  (ioport(portnames[bit_p1])->read() & 0x00ffffff) |
			(ioport(portnames[bit_p2])->read() & 0xff000000) ;
}



READ32_MEMBER(darkhors_state::p_580004)
{
UINT32 ret = ioport("580004")->read()& 0x00ffffff;
switch (m_input_sel_jc_2p){
	case 0x01: return  ret | (ioport("580004-01")->read()<<24);
	case 0x02: return  ret | (ioport("580004-02")->read()<<24);
	case 0x04: return  ret | (ioport("580004-04")->read()<<24);
	case 0x08: return  ret | (ioport("580004-08")->read()<<24);
	case 0x10: return  ret | (ioport("580004-10")->read()<<24);
	case 0x20: return  ret | (ioport("580004-20")->read()<<24);
	case 0x40: return  ret | (ioport("580004-40")->read()<<24);
	case 0x80: return  ret | (ioport("580004-80")->read()<<24);
	}

	return  ret;
}


READ32_MEMBER(darkhors_state::p_4e0000)
{
UINT32 ret = ioport("4E0000")->read()& 0x00ffffff;
switch (m_input_sel_jc_2p){
	case 0x01: return  ret | (ioport("4E0000-01")->read()<<24);
	case 0x02: return  ret | (ioport("4E0000-02")->read()<<24);
	case 0x04: return  ret | (ioport("4E0000-04")->read()<<24);
	case 0x08: return  ret | (ioport("4E0000-08")->read()<<24);
	case 0x10: return  ret | (ioport("4E0000-10")->read()<<24);
	case 0x20: return  ret | (ioport("4E0000-20")->read()<<24);
	case 0x40: return  ret | (ioport("4E0000-40")->read()<<24);
	case 0x80: return  ret | (ioport("4E0000-80")->read()<<24);
	}

	return  ret;
}

WRITE32_MEMBER(darkhors_state::jclub2_input_sel_w_p1)
{
	//sometimes 0x80000000 bit is set!!!
	COMBINE_DATA(&m_input_sel);
	m_input_sel_jc_1p = (m_input_sel & 0x00ff0000) >> 16;
}


WRITE32_MEMBER(darkhors_state::jclub2_input_sel_w_p2)
{
	COMBINE_DATA(&m_input_sel);
	m_input_sel_jc_2p = (m_input_sel & 0x00ff0000) >> 16;
}


WRITE32_MEMBER(darkhors_state::darkhors_unk1_w)
{
	// 0x1000 lockout
//  if (ACCESSING_BITS_16_31)    popmessage("%04X",data >> 16);
}

static ADDRESS_MAP_START( darkhors_map, AS_PROGRAM, 32, darkhors_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM

	AM_RANGE(0x490040, 0x490043) AM_WRITE(darkhors_eeprom_w)
	AM_RANGE(0x4e0080, 0x4e0083) AM_READ_PORT("4e0080") AM_WRITE(darkhors_unk1_w)

	AM_RANGE(0x580000, 0x580003) AM_READ_PORT("580000")
	AM_RANGE(0x580004, 0x580007) AM_READ_PORT("580004")
	AM_RANGE(0x580008, 0x58000b) AM_READ(darkhors_input_sel_r)
	AM_RANGE(0x58000c, 0x58000f) AM_WRITE(darkhors_input_sel_w)
	AM_RANGE(0x580084, 0x580087) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xff000000)
	AM_RANGE(0x580200, 0x580203) AM_READNOP
	AM_RANGE(0x580400, 0x580403) AM_READ_PORT("580400")
	AM_RANGE(0x580420, 0x580423) AM_READ_PORT("580420")

	AM_RANGE(0x800000, 0x86bfff) AM_RAM
	AM_RANGE(0x86c000, 0x86ffff) AM_RAM_WRITE(darkhors_tmapram_w) AM_SHARE("tmapram")
	AM_RANGE(0x870000, 0x873fff) AM_RAM_WRITE(darkhors_tmapram2_w) AM_SHARE("tmapram2")
	AM_RANGE(0x874000, 0x87dfff) AM_RAM
	AM_RANGE(0x87e000, 0x87ffff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x880000, 0x89ffff) AM_RAM AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x8a0000, 0x8bffff) AM_RAM   // this should still be palette ram!
	AM_RANGE(0x8c0120, 0x8c012f) AM_WRITEONLY AM_SHARE("tmapscroll")
	AM_RANGE(0x8c0130, 0x8c013f) AM_WRITEONLY AM_SHARE("tmapscroll2")
ADDRESS_MAP_END



static ADDRESS_MAP_START( jclub2_map, AS_PROGRAM, 32, darkhors_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM AM_SHARE("nvram")   //all 68k ram is battery backup
	//AM_RANGE(0x400000, 0x41ffff) AM_RAM

	AM_RANGE(0x490000, 0x490003) AM_WRITE(darkhors_eeprom_w)
	AM_RANGE(0x4E0000, 0x4E0003) AM_READ(p_4e0000)      //input 2p
	AM_RANGE(0x4E0000, 0x4E0003) AM_WRITE(jclub2_input_sel_w_p1)
	AM_RANGE(0x580000, 0x580003) AM_READ_PORT("580000")     //eeprom related?
	AM_RANGE(0x580004, 0x580007) AM_READ(p_580004)      //system + some 1p input here
	AM_RANGE(0x580008, 0x58000b) AM_READ_PORT("580008")     //input 1p + coins
	AM_RANGE(0x58000c, 0x58000f) AM_WRITE(jclub2_input_sel_w_p2)
	AM_RANGE(0x580200, 0x580203) AM_READNOP //????

	AM_RANGE(0x800000, 0x87ffff) AM_DEVREADWRITE16( "st0020_spr", st0020_device, st0020_sprram_r, st0020_sprram_w, 0xffffffff );

	AM_RANGE(0x880000, 0x89ffff) AM_RAM AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x8a0000, 0x8bffff) AM_RAM   // this should still be palette ram!

//  AM_RANGE(0x8C0000, 0x8C00ff) AM_DEVREADWRITE16( "st0020_spr", st0020_device, st0020_blitram_r, st0020_blitram_w, 0xffffffff );
	AM_RANGE(0x8E0000, 0x8E01ff) AM_RAM

	AM_RANGE(0x900000, 0x9fffff) AM_DEVREADWRITE16( "st0020_spr", st0020_device, st0020_gfxram_r, st0020_gfxram_w, 0xffffffff );
ADDRESS_MAP_END




static ADDRESS_MAP_START( jclub2o_map, AS_PROGRAM, 32, darkhors_state )

	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM AM_SHARE("nvram")   //all 68k ram is battery backup
	//AM_RANGE(0x400000, 0x41ffff) AM_RAM

	AM_RANGE(0x490000, 0x490003) AM_WRITE(jclub2o_eeprom_w) //eeprom s2929 not 93c46
	AM_RANGE(0x4E0000, 0x4E0003) AM_READ(p_4e0000)          //input 2p
	AM_RANGE(0x4E0000, 0x4E0003) AM_WRITE(jclub2_input_sel_w_p1)
	AM_RANGE(0x580000, 0x580003) AM_READ_PORT("580000")     //eeprom read
	AM_RANGE(0x580004, 0x580007) AM_READ(p_580004)          //system + some 1p keyboard input here
	AM_RANGE(0x580008, 0x58000b) AM_READ_PORT("580008")     //input 1p + coins
	AM_RANGE(0x58000c, 0x58000f) AM_WRITE(jclub2_input_sel_w_p2)

	AM_RANGE(0x580200, 0x580203) AM_READNOP //????

	AM_RANGE(0x600000, 0x67ffff) AM_DEVREADWRITE16( "st0020_spr", st0020_device, st0020_sprram_r, st0020_sprram_w, 0xffffffff );
	AM_RANGE(0x680000, 0x69ffff) AM_RAM AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x6a0000, 0x6bffff) AM_RAM
	AM_RANGE(0x6C0000, 0x6C00ff) AM_DEVREADWRITE16( "st0020_spr", st0020_device, st0020_blitram_r, st0020_blitram_w, 0xffffffff );
	AM_RANGE(0x700000, 0x7fffff) AM_DEVREADWRITE16( "st0020_spr", st0020_device, st0020_gfxram_r, st0020_gfxram_w, 0xffffffff );

ADDRESS_MAP_END



/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( darkhors )
	PORT_START("580000")
	PORT_BIT( 0xff7fffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("?") PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("580004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BILL1 )    // bill in
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN1 )    // coin in s1
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN3 )    // coin in s2
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN1 )    // coin drop
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_SPECIAL )  // hopper full
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1") PORT_CODE(KEYCODE_ENTER) // bill in p2
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )    // coin in s1 p2
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_COIN4 )    // coin in s2 p2
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_COIN2 )    // coin drop p2
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_SPECIAL)   // hopper full p2

	PORT_START("580400")
	PORT_BIT( 0xfffcffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("580420")
	PORT_BIT( 0xfffcffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0*") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("9*") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("4e0080")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW,  IPT_SERVICE1 )    // config
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW,  IPT_SERVICE2 )    // reset
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW,  IPT_SERVICE3 )    // meter
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW,  IPT_SERVICE4 )    // lastgame
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW,  IPT_SERVICE  ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1) // test
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )    // door 1
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )    // door 2
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_SPECIAL  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW,  IPT_START1   )    // start
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P1 Payout") PORT_CODE(KEYCODE_LCONTROL)    // payout
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P1 Cancel") PORT_CODE(KEYCODE_LALT)        // cancel
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW,  IPT_START2   )    // start p2
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P2 Payout") PORT_CODE(KEYCODE_RCONTROL)    // payout p2
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P2 Cancel") PORT_CODE(KEYCODE_RALT)        // cancel p2
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW,  IPT_UNKNOWN  )

	PORT_START("IN0")   /* 580008(0) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 8") //PORT_CODE(KEYCODE_)

	PORT_START("IN1")   /* 580008(1) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-2") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-4") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-6") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-7") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-8") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-2") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* 580008(2) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-3") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-5") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-6") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-7") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-8") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* 580008(3) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-4") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-6") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-7") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-8") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")   /* 580008(4) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-5") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-7") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-8") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")   /* 580008(5) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-6") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-7") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-8") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN6")   /* 580008(6) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6-7") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6-8") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6-7") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN7")   /* 580008(7) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7-8") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 7-8") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END




static INPUT_PORTS_START( jclub2 )


	PORT_START("580000")
	PORT_BIT( 0x7fffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_SPECIAL  ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("580008")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P1 Payout") PORT_CODE(KEYCODE_LCONTROL)    // payout 1p
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW,  IPT_START1   )                                                    // start 1p
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P2 coin out") PORT_CODE(KEYCODE_RSHIFT)                //coin out 2p
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P1 coin out") PORT_CODE(KEYCODE_LSHIFT)                //coin out 1p
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW,  IPT_COIN1 )                                                       //1p coin drop
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW,  IPT_COIN3 )                                                       //2p coin drop
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW,  IPT_COIN1 )                                                       //1p coin in s1
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW,  IPT_COIN2 )                                                       //1p coin in s2
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW,  IPT_COIN3 )                                                       //2p coin in s1
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW,  IPT_COIN4 )                                                       //2p coin in s2

	PORT_START("580004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW,  IPT_SERVICE2 )                                                    //reset
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW,  IPT_SERVICE3 )                                                    //meter
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW,  IPT_SERVICE4 )                                                    //last game
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P1 Cancel") PORT_CODE(KEYCODE_LALT)        // cancel 1p
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW,  IPT_SERVICE  ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)   //test
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P2 hopper full") PORT_CODE(KEYCODE_CLOSEBRACE)                 //hopper full 2p
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P1 hopper full") PORT_CODE(KEYCODE_OPENBRACE)                  //hopper full 1p
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     //keyboard input 1p
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     //keyboard input 1p
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     //keyboard input 1p
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     //keyboard input 1p
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     //keyboard input 1p
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     //keyboard input 1p
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     //keyboard input 1p
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )     //keyboard input 1p

	PORT_START("580004-01") //1P
	PORT_BIT( 0x00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 8") PORT_CODE(KEYCODE_8_PAD)

	PORT_START("580004-02") //1P
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-7") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-8") PORT_CODE(KEYCODE_U)


	PORT_START("580004-04") //1P
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-3") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-4") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-5") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-6") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-7") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-8") PORT_CODE(KEYCODE_F)

	PORT_START("580004-08") //1P
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-6") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-7") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-8") PORT_CODE(KEYCODE_L)

	PORT_START("580004-10") //1P
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-5") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-6") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-7") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-8") PORT_CODE(KEYCODE_V)


	PORT_START("580004-20") //1P
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-6") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-7") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-8") PORT_CODE(KEYCODE_M)


	PORT_START("580004-40") //1P
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6-7") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6-8") PORT_CODE(KEYCODE_STOP)


	PORT_START("580004-80") //1P
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7-8") PORT_CODE(KEYCODE_SLASH)


	PORT_START("4E0000")   // 4E0000
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P2 Payout") PORT_CODE(KEYCODE_RCONTROL)    // payout 2p
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW,  IPT_START2   )                                                    // start 2p
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("P2 Cancel") PORT_CODE(KEYCODE_RALT)        // cancel 2p
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //unused???
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )                     //keyboard input 2p
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )                     //keyboard input 2p
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )                     //keyboard input 2p
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )                     //keyboard input 2p
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )                     //keyboard input 2p
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )                     //keyboard input 2p
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )                     //keyboard input 2p
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW,  IPT_UNKNOWN )                     //keyboard input 2p

	PORT_START("4E0000-01") //2P
	PORT_BIT( 0x00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 8") PORT_CODE(KEYCODE_8_PAD)

	PORT_START("4E0000-02") //2P
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-7") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-8") PORT_CODE(KEYCODE_U)


	PORT_START("4E0000-04") //2P
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-3") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-4") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-5") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-6") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-7") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-8") PORT_CODE(KEYCODE_F)

	PORT_START("4E0000-08") //2P
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-6") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-7") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-8") PORT_CODE(KEYCODE_L)

	PORT_START("4E0000-10") //2P
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-5") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-6") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-7") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-8") PORT_CODE(KEYCODE_V)


	PORT_START("4E0000-20") //2P
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-6") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-7") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-8") PORT_CODE(KEYCODE_M)

	PORT_START("4E0000-40") //2P
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6-7") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6-8") PORT_CODE(KEYCODE_STOP)

	PORT_START("4E0000-80") //2P
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 7-8") PORT_CODE(KEYCODE_SLASH)
INPUT_PORTS_END

/***************************************************************************


                                Gfx Layouts


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

static GFXDECODE_START( darkhors )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8, 0, 0x10000/64 ) // color codes should be doubled
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(darkhors_state::darkhors_irq)
{
	int scanline = param;

	if(scanline == 248)
		m_gamecpu->set_input_line(5, HOLD_LINE);

	if(scanline == 0)
		m_gamecpu->set_input_line(3, HOLD_LINE);

	if(scanline == 128)
		m_gamecpu->set_input_line(4, HOLD_LINE);
}

static MACHINE_CONFIG_START( darkhors, darkhors_state )
	MCFG_CPU_ADD("gamecpu", M68EC020, 12000000) // 36MHz/3 ??
	MCFG_CPU_PROGRAM_MAP(darkhors_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", darkhors_state, darkhors_irq, "screen", 0, 1)

	MCFG_EEPROM_SERIAL_93C46_8BIT_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x190, 0x100+16)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x190-1, 8, 0x100-8-1)
	MCFG_SCREEN_UPDATE_DRIVER(darkhors_state, screen_update_darkhors)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", darkhors)
	MCFG_PALETTE_ADD("palette", 0x10000)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(darkhors_state,darkhors)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 528000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



VIDEO_START_MEMBER(darkhors_state,jclub2)
{
}

UINT32 darkhors_state::screen_update_jclub2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// this isn't an st0020..
	m_gdfs_st0020->st0020_draw_all(bitmap, cliprect);

	return 0;
}

static MACHINE_CONFIG_START( jclub2, darkhors_state )
	MCFG_CPU_ADD("gamecpu", M68EC020, 12000000)
	MCFG_CPU_PROGRAM_MAP(jclub2_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", darkhors_state, darkhors_irq, "screen", 0, 1)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_EEPROM_SERIAL_93C46_8BIT_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x190, 0x100+16)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x190-1, 8, 0x100-8-1)
	MCFG_SCREEN_UPDATE_DRIVER(darkhors_state, screen_update_jclub2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	// NOT an ST0020 but instead ST0032, ram format isn't compatible at least
	MCFG_DEVICE_ADD("st0020_spr", ST0020_SPRITES, 0)
	st0020_device::set_is_st0032(*device, 1);
	st0020_device::set_is_jclub2o(*device, 1); // offsets
	MCFG_ST0020_SPRITES_GFXDECODE("gfxdecode")
	MCFG_ST0020_SPRITES_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x10000)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(darkhors_state,jclub2)
MACHINE_CONFIG_END



static ADDRESS_MAP_START( st0016_mem, AS_PROGRAM, 8, darkhors_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	//AM_RANGE(0xe900, 0xe9ff) // sound - internal
	//AM_RANGE(0xec00, 0xec1f) AM_READ(st0016_character_ram_r) AM_WRITE(st0016_character_ram_w)
	AM_RANGE(0xe82f, 0xe830) AM_READNOP
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

// common rombank? should go in machine/st0016 with larger address space exposed?
WRITE8_MEMBER(darkhors_state::st0016_rom_bank_w)
{
	membank("bank1")->set_base(memregion("maincpu")->base() + (data* 0x4000));
}


static ADDRESS_MAP_START( st0016_io, AS_IO, 8, darkhors_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0xbf) AM_READ(st0016_vregs_r) AM_WRITE(st0016_vregs_w)
	//AM_RANGE(0xc0, 0xc0) AM_READ(cmd1_r)
	//AM_RANGE(0xc1, 0xc1) AM_READ(cmd2_r)
	//AM_RANGE(0xc2, 0xc2) AM_READ(cmd_stat8_r)
	AM_RANGE(0xe1, 0xe1) AM_WRITE(st0016_rom_bank_w)
	//AM_RANGE(0xe7, 0xe7) AM_WRITE(st0016_rom_bank_w)
	//AM_RANGE(0xf0, 0xf0) AM_READ(st0016_dma_r)
ADDRESS_MAP_END


VIDEO_START_MEMBER(darkhors_state,jclub2o)
{
}

UINT32 darkhors_state::screen_update_jclub2o(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_gdfs_st0020->st0020_draw_all(bitmap, cliprect);
	return 0;
}

static MACHINE_CONFIG_START( jclub2o, darkhors_state )
	MCFG_CPU_ADD("gamecpu", M68EC020, 12000000)
	MCFG_CPU_PROGRAM_MAP(jclub2o_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", darkhors_state, darkhors_irq, "screen", 0, 1)

	MCFG_CPU_ADD("maincpu",ST0016_CPU,8000000)
	MCFG_CPU_PROGRAM_MAP(st0016_mem)
	MCFG_CPU_IO_MAP(st0016_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", darkhors_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_EEPROM_SERIAL_93C56_ADD("eeprom")      //not correct

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x190, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x190-1, 8, 0x100-8-1)
	MCFG_SCREEN_UPDATE_DRIVER(darkhors_state, screen_update_jclub2o)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x10000)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	MCFG_DEVICE_ADD("st0020_spr", ST0020_SPRITES, 0)
	st0020_device::set_is_jclub2o(*device, 1);
	MCFG_ST0020_SPRITES_GFXDECODE("gfxdecode")
	MCFG_ST0020_SPRITES_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(darkhors_state,jclub2o)

MACHINE_CONFIG_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/

ROM_START( darkhors )
	ROM_REGION( 0x100000, "gamecpu", 0 )    // 68EC020 code
	ROM_LOAD32_WORD_SWAP( "prg2", 0x00000, 0x80000, CRC(f2ec5818) SHA1(326937a331496880f517f41b0b8ab54e55fd7af7) )
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
	ROM_LOAD( "eeprom", 0x0000, 0x0080, CRC(1f434f66) SHA1(e1bee11d83fb72aed9c312bdc794d8b9a6645534) )
ROM_END

/*

Jockey Club II Original sets

 -- unlike the bootleg these appear to have the graphics compressed in the main program roms.  (CPS3 / SRMP6 -like)  (I think CPS3 is a SETA design)
 -- the older hardware uses an st-0016 for sound (see st0016.c)
 -- the program can be upgraded by placing a ROM in the socket next to the main program ROM.

There is known to be an undumped v4.00 with copyright 1994-97 Seta Corporation on the E84-0001 Rev.B hardware.

 */



	/*
Jockey Club II by SETA 1996

PCB E79-001 rev 01a (Newer)

Main CPU : MC68EC020FG16

Graphics : SETA ST-0032 70C600JF505

Others :
	 SETA ST-0013
	 SETA ST-0017

Xtal : 42.9545 MHz, 60.0000 MHz, 33.3333 MHz


Rams :  Toshiba : TC5118160CJ-60 (x3)
	NKK N341256SJ-15 (x2)
	NEC D43001GU-70LL (x4)

Gals : gal16V8B(x2) ref : M88-03 M88-04

Eeprom : 93c46

Eproms : M88-01.u38,M88-023.u6 (read as 578200) (1st set)
	 M88-01A.u38,M88-02.u6 (2nd set)
	 M88-01B.u38,M88-02.u6 (3rd set)
	 M88-01.u38,M88-02.u6,Z201x.u39 (4th set)
	 M88-01A.u38,M88-02.u6 M88-03D.u39 (5th set)
	 M88-01B.u38,M88-02.u6,M88-03D.u39 (6th set)

Provided to you by Belgium Dump Team Gerald (COY) on 18/01/2007.
*/


// ToDo: check for other similarities with CPS3 hardware, there are more than a few things which look the same.

// this contains mutliple sets, although splitting them as listed above makes no sense.. especially not the 'subcpu' roms
ROM_START( jclub2 )
	ROM_REGION( 0x200000, "gamecpu", 0 )    // 68EC020 code  + compressed GFX
	// main program (similar to main program of bootleg
	ROM_LOAD16_WORD_SWAP( "m88-01b.u38",0x00000, 0x200000, CRC(f1054c69) SHA1(be6d92653f0d3cc0a36a2ff0798043f4a95439bc) )
	ROM_LOAD16_WORD_SWAP( "m88-01a.u38",0x00000, 0x200000, CRC(c1243e1c) SHA1(2a5857738b8950daf77ddaa8304b765f809f8241) ) // alt revision?
	ROM_LOAD16_WORD_SWAP( "m88-01.u38", 0x00000, 0x200000, CRC(84476b68) SHA1(1014d23d3cebbfa9aa3bfb90505529989a8eedfa) ) // alt revision?

	ROM_REGION( 0x200000, "patch", 0 )  // 68EC020 code
	// it appears that the operator could place a ROM in the socket next to the main CPU rom to update the main program rom by
	// overriding the initial 0x80000 bytes of the program rom.
	ROM_LOAD16_WORD_SWAP( "m88-03d.u39",0x00000, 0x080000, CRC(723dd22b) SHA1(0ca622e0dd315f29e72dd9b82fb419d306ec5df8) )
	ROM_LOAD16_WORD_SWAP( "z201x.u39",0x00000, 0x080000, CRC(1fb79c16) SHA1(c8914f7dfc17c412f6ca756f8eb6d6a35e3b6214) )

	ROM_REGION( 0x100000, "oki", 0 )    // Samples? (not oki probably one of the ST-xx customs, no idea if the dump is good)
	// data distribution would indicate this is a sound rom
	ROM_LOAD( "m88-02.u6", 0x00000, 0x100000, CRC(0dd3436a) SHA1(809d3b7a26d36f71da04036fd8ab5d0c5089392a) )

	ROM_REGION( 0x80000, "misc", ROMREGION_ERASEFF )
	ROM_LOAD( "gal16v8b-m88-03.bin", 0x000, 0x117, CRC(6d9c882e) SHA1(84cb95ab540290c2f8b740668360e9c643a67dcf) )
	ROM_LOAD( "gal16v8b-m88-04.bin", 0x000, 0x117, CRC(5e79f292) SHA1(5e44c234e2b15d486a1af71fee986892aa245b4d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // EEPROM
	ROM_LOAD( "eeprom-jclub2.bin", 0x0000, 0x0080, CRC(1513cdc8) SHA1(22ff752f3e0f8f611c234a1dc4327aa360b4d6eb) )
ROM_END

/*
Jockey Club II by SETA 1994-1995

Other hardware version (older):

Main PCB: E06-00409
Sub PCb : E06-00407 (I/O nothing else)

Main CPU :  MC68EC020FG16

Many XTAL : 48.0000 MHz,33.3333 MHz,4.91520 MHz,42.9545 MHz(x2),105.0000 MHz (this 105.0000 Xtal is sometimes replaced by a tiny pcb silscreened 108.0000 MHz(!), with ICS ICS1494N, MB3771 and 14.3181 MHz Xtal)

Graphics: SETA ST-0020

Others :
     SETA ST-0013
     SETA ST-0016  <-- z80 core + simple gfx + sound, see st0016.c
     SETA ST-0017

Rams :  Toshiba TC514800AJ-70
    Toshiba TC514000ASJ-70 (x8)
    Sharp LH5168D

Eproms : SX006A-01.u26 read as 5716200,JC2-110x.u27,SX006-04.u87 (sound)
     SX006B-01.u26,JC2-110x.u27,SX006-04.u87 (sound)
     SX006A-01.u26,JC2-112x.u27,SX006-04.u87 (sound)
     SX006B-01.u26,JC2-112x.u27,SX006-04.u87 (sound)

Provided to you by Belgium Dump Team Gerald (COY) on 18/01/2007.

*/

// this contains mutliple sets
ROM_START( jclub2o )
	ROM_REGION( 0x200000, "gamecpu", 0 )    // 68EC020 code + compressed gfx
	ROM_LOAD16_WORD_SWAP( "sx006a-01.106",0x00000, 0x200000, CRC(55e249bc) SHA1(ed0f066ed17f047760b712cbbfba1a62d4b452ba) )
	ROM_LOAD16_WORD_SWAP( "sx006b-01.u26",0x00000, 0x200000, CRC(f730dded) SHA1(efb966dcb98440a072d4825ef2788c85acdfd103) )  // alt revision?

	ROM_REGION( 0x200000, "patch", 0 )  // 68EC020 code
	// it appears that the operator could place a ROM in the socket next to the main CPU rom to update the main program rom by
	// overriding the initial 0x80000 bytes of the program rom.
	ROM_LOAD16_WORD_SWAP( "jc2-110x.u27",0x00000, 0x080000, CRC(03aa6882) SHA1(e0343bc77a19994ddafa614891663b40e1476332) )
	ROM_LOAD16_WORD_SWAP( "jc2-112x.u27",0x00000, 0x080000, CRC(e1ab93bd) SHA1(78b618b3f7819bd5351ebf949f328fec7795cec9) ) // alt revision?

	ROM_REGION( 0x80000, "maincpu", 0 ) // z80 core (used for sound?)
	ROM_LOAD( "sx006-04.u87", 0x00000, 0x80000, CRC(a87adedd) SHA1(1cd5af2d03738fff2230b46241659179467c828c) )

	ROM_REGION( 0x100, "eeprom", 0 ) // eeprom 16 bit one!!!
	ROM_LOAD( "eeprom-jclub2o.bin", 0x0000, 0x100, CRC(dd1c88ec) SHA1(acb67e41e832f203361e0f93afcd4eaf963fd13e) )   //jclub2ob ones
ROM_END

/*
  Jockey Club II (26-mar-1997)
  8 Horses, old style PCB.
  Maybe upgraded to a release candidate software revision.
*/
ROM_START( jclub2ob )
	ROM_REGION( 0x200000, "gamecpu", 0 )    // 68EC020 code + compressed gfx
	ROM_LOAD16_WORD_SWAP( "sx006a-01.u26",0x00000, 0x200000, CRC(55e249bc) SHA1(ed0f066ed17f047760b712cbbfba1a62d4b452ba) )

	ROM_REGION( 0x200000, "patch", 0 )  // 68EC020 code
	// it appears that the operator could place a ROM in the socket next to the main CPU rom to update the main program rom by
	// overriding the initial 0x80000 bytes of the program rom.
	ROM_LOAD16_WORD_SWAP( "203x-rom1.u27",0x00000, 0x080000, CRC(7446ed3e) SHA1(b0936e42549280e2965159270429c4fdacba114a) )

	ROM_REGION( 0x80000, "maincpu", 0 ) // z80 core (used for sound?)
	ROM_LOAD( "sx006-04.u87", 0x00000, 0x80000, CRC(a87adedd) SHA1(1cd5af2d03738fff2230b46241659179467c828c) )

	ROM_REGION( 0x100, "eeprom", 0 ) // eeprom 16 bit one!!!
	ROM_LOAD( "eeprom-jclub2o.bin", 0x0000, 0x100, CRC(dd1c88ec) SHA1(acb67e41e832f203361e0f93afcd4eaf963fd13e) )
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/

DRIVER_INIT_MEMBER(darkhors_state,darkhors)
{
	// the dumped eeprom bytes are in a different order to how MAME expects them to be
	// (offset 0x00, 0x40, 0x01, 0x41, 0x02, 0x42 ..... )
	// I guess this is the difference between the internal organization on the real
	// device, and how MAME represents it?

	// the eeprom contains the game ID, which must be valid for it to boot
	// is there a way (key sequence) to reprogram it??
	// I bet the original sets need similar get further in their boot sequence
	UINT8  *eeprom = (UINT8 *)  memregion("eeprom")->base();
	if (eeprom != 0x00)
	{
		size_t len = memregion("eeprom")->bytes();
		dynamic_buffer temp(len);
		int i;
		for (i = 0; i < len; i++)
			temp[i] = eeprom[BITSWAP8(i,7,5,4,3,2,1,0,6)];

		memcpy(eeprom, &temp[0], len);

	}
}

/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT      ROT    COMPANY    FULLNAME                                 FLAGS  */
GAME( 199?, jclub2,   0,      jclub2,  jclub2, driver_device,  0,       ROT0, "Seta", "Jockey Club II (newer hardware)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, jclub2o,  jclub2, jclub2o, jclub2, driver_device,  0,       ROT0, "Seta", "Jockey Club II (older hardware)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, jclub2ob, jclub2, jclub2o, jclub2, driver_device,  0,       ROT0, "Seta", "Jockey Club II (older hardware, set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, darkhors, jclub2, darkhors,darkhors,darkhors_state, darkhors,ROT0, "bootleg", "Dark Horse (bootleg of Jockey Club II)", MACHINE_IMPERFECT_GRAPHICS )


//test boot = test mode
//reset +start 1p at boot, when msg on screen press test without release other key = setup
//reset +cancel 1p  = backup all clear
//reset, test, meter and last game are keys so once you turn them they stay "active"
/*
boot with
- F1 (test) to enter the test
- 0 + 1 (reset+start1) to run the setup
- 0 + left alt (reset + p1 cancel) to clear data
- 0 + 1 + f1 (IIRC) to write ID
*/
