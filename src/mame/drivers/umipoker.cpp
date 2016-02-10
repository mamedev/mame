// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
/***************************************************************************

    Umi de Poker (c) 1997 World Station Co.,LTD
    Slot Poker Saiyuki (c) 1998 World Station Co.,LTD

    Driver by Angelo Salese.
    Additional work by Roberto Fresca.

    TODO:
    - Fix clocks;

    TMP68HC000-16 + z80 + YM3812 + OKI6295

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "saiyukip.lh"


class umipoker_state : public driver_device
{
public:
	umipoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram_0(*this, "vra0"),
		m_vram_1(*this, "vra1"),
		m_vram_2(*this, "vra2"),
		m_vram_3(*this, "vra3"),
		m_z80_wram(*this, "z80_wram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_vram_2;
	required_shared_ptr<UINT16> m_vram_3;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	tilemap_t *m_tilemap_3;
	required_shared_ptr<UINT8> m_z80_wram;
	int m_umipoker_scrolly[4];
	DECLARE_READ8_MEMBER(z80_rom_readback_r);
	DECLARE_READ8_MEMBER(z80_shared_ram_r);
	DECLARE_WRITE8_MEMBER(z80_shared_ram_w);
	DECLARE_WRITE16_MEMBER(umipoker_irq_ack_w);
	DECLARE_WRITE16_MEMBER(umipoker_scrolly_0_w);
	DECLARE_WRITE16_MEMBER(umipoker_scrolly_1_w);
	DECLARE_WRITE16_MEMBER(umipoker_scrolly_2_w);
	DECLARE_WRITE16_MEMBER(umipoker_scrolly_3_w);
	DECLARE_WRITE16_MEMBER(umipoker_vram_0_w);
	DECLARE_WRITE16_MEMBER(umipoker_vram_1_w);
	DECLARE_WRITE16_MEMBER(umipoker_vram_2_w);
	DECLARE_WRITE16_MEMBER(umipoker_vram_3_w);
	DECLARE_WRITE16_MEMBER(lamps_w);
	DECLARE_WRITE16_MEMBER(umi_counters_w);
	DECLARE_WRITE16_MEMBER(saiyu_counters_w);
	DECLARE_DRIVER_INIT(umipoker);
	DECLARE_DRIVER_INIT(saiyukip);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_3);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_umipoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

TILE_GET_INFO_MEMBER(umipoker_state::get_tile_info_0)
{
	int tile = m_vram_0[tile_index*2+0];
	int color = m_vram_0[tile_index*2+1] & 0x3f;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(umipoker_state::get_tile_info_1)
{
	int tile = m_vram_1[tile_index*2+0];
	int color = m_vram_1[tile_index*2+1] & 0x3f;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(umipoker_state::get_tile_info_2)
{
	int tile = m_vram_2[tile_index*2+0];
	int color = m_vram_2[tile_index*2+1] & 0x3f;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(umipoker_state::get_tile_info_3)
{
	int tile = m_vram_3[tile_index*2+0];
	int color = m_vram_3[tile_index*2+1] & 0x3f;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

void umipoker_state::video_start()
{
	m_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(umipoker_state::get_tile_info_0),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(umipoker_state::get_tile_info_1),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_tilemap_2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(umipoker_state::get_tile_info_2),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_tilemap_3 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(umipoker_state::get_tile_info_3),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	m_tilemap_0->set_transparent_pen(0);
	m_tilemap_1->set_transparent_pen(0);
	m_tilemap_2->set_transparent_pen(0);
	m_tilemap_3->set_transparent_pen(0);

}

UINT32 umipoker_state::screen_update_umipoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap_0->set_scrolly(0, m_umipoker_scrolly[0]);
	m_tilemap_1->set_scrolly(0, m_umipoker_scrolly[1]);
	m_tilemap_2->set_scrolly(0, m_umipoker_scrolly[2]);
	m_tilemap_3->set_scrolly(0, m_umipoker_scrolly[3]);

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_tilemap_0->draw(screen, bitmap, cliprect, 0,0);
	m_tilemap_1->draw(screen, bitmap, cliprect, 0,0);
	m_tilemap_2->draw(screen, bitmap, cliprect, 0,0);
	m_tilemap_3->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}

READ8_MEMBER(umipoker_state::z80_rom_readback_r)
{
	UINT8 *ROM = memregion("audiocpu")->base();

	return ROM[offset];
}

READ8_MEMBER(umipoker_state::z80_shared_ram_r)
{
	machine().scheduler().synchronize(); // force resync

	return m_z80_wram[offset];
}

WRITE8_MEMBER(umipoker_state::z80_shared_ram_w)
{
	machine().scheduler().synchronize(); // force resync

	m_z80_wram[offset] = data;
}

WRITE16_MEMBER(umipoker_state::umipoker_irq_ack_w)
{
	m_maincpu->set_input_line(6, CLEAR_LINE);

	/* shouldn't happen */
	if(data)
		popmessage("%04x IRQ ACK, contact MAMEdev",data);
}

WRITE16_MEMBER(umipoker_state::umipoker_scrolly_0_w){ COMBINE_DATA(&m_umipoker_scrolly[0]); }
WRITE16_MEMBER(umipoker_state::umipoker_scrolly_1_w){ COMBINE_DATA(&m_umipoker_scrolly[1]); }
WRITE16_MEMBER(umipoker_state::umipoker_scrolly_2_w){ COMBINE_DATA(&m_umipoker_scrolly[2]); }
WRITE16_MEMBER(umipoker_state::umipoker_scrolly_3_w){ COMBINE_DATA(&m_umipoker_scrolly[3]); }

WRITE16_MEMBER(umipoker_state::umipoker_vram_0_w)
{
	COMBINE_DATA(&m_vram_0[offset]);
	m_tilemap_0->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(umipoker_state::umipoker_vram_1_w)
{
	COMBINE_DATA(&m_vram_1[offset]);
	m_tilemap_1->mark_tile_dirty(offset >> 1);
}


WRITE16_MEMBER(umipoker_state::umipoker_vram_2_w)
{
	COMBINE_DATA(&m_vram_2[offset]);
	m_tilemap_2->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(umipoker_state::umipoker_vram_3_w)
{
	COMBINE_DATA(&m_vram_3[offset]);
	m_tilemap_3->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(umipoker_state::lamps_w)
{
/*
  Umipoker buttons layout:
  .--------.  .--------.  .--------.  .--------.  .--------.  .--------.
  |        |  |        |  |        |  |        |  | START  |  |  BET   |
  | DOUBLE |  |  TAKE  |  |  BIG   |  | SMALL  |  |        |  |        |
  |        |  |        |  |        |  |        |  |  SKIP  |  | RAISE  |
  '--------'  '--------'  '--------'  '--------'  '--------'  '--------'
   (yellow)    (green)      (red)       (red)      (green)     (yellow)

  Saiyukip buttons layout:
  .--------.  .--------.  .--------.  .--------.  .--------.  .--------.
  |        |  |        |  |  LEFT  |  |        |  |        |  |        |
  | DOUBLE |  |  TAKE  |  |        |  | RIGHT  |  | START  |  |  BET   |
  |        |  |        |  |  STOP  |  |        |  |        |  |        |
  '--------'  '--------'  '--------'  '--------'  '--------'  '--------'
   (yellow)    (green)      (red)       (red)      (green)     (yellow)

  Seems that only saiyukip has programmed lamps.

  0x0000 - Normal State (lamps off).
  0x0020 - RIGHT lamp.
  0x0200 - TAKE lamp.
  0x0400 - D-UP lamp.
  0x0800 - BET lamp.
  0x1000 - LEFT/STOP lamp.
  0x2000 - START lamp.

  - Hbits -/- Lbits -
  7654 3210 7654 3210
  ===================
  xx-- ---x xx-x xxxx  Unknown / Not used.
  ---- ---- --x- ----  RIGHT lamp.
  ---- --x- ---- ----  TAKE lamp.
  ---- -x-- ---- ----  D-UP lamp.
  ---- x--- ---- ----  BET lamp.
  ---x ---- ---- ----  LEFT/STOP lamp.
  --x- ---- ---- ----  START lamp.

*/
	output().set_lamp_value(0, (data >> 5) & 1);      /* Lamp 0 - RIGHT */
	output().set_lamp_value(1, (data >> 9) & 1);      /* Lamp 1 - TAKE */
	output().set_lamp_value(2, (data >> 10) & 1);     /* Lamp 2 - D-UP */
	output().set_lamp_value(3, (data >> 11) & 1);     /* Lamp 3 - BET */
	output().set_lamp_value(4, (data >> 12) & 1);     /* Lamp 4 - LEFT/STOP */
	output().set_lamp_value(5, (data >> 13) & 1);     /* Lamp 5 - START */
}

WRITE16_MEMBER(umipoker_state::umi_counters_w)
{/*
  0x0000 - Normal State (lamps off).
  0x0001 - Payout pulse.
  0x0020 - Coin 1.
  0x0040 - Coin 2, 3 and Remote.

  - Hbits -/- Lbits -
  7654 3210 7654 3210
  ===================
  ---- ---- ---- ---x  Payout pulse.
  ---- ---- --x- ----  Coin 1.
  ---- ---- -x-- ----  Coin 2, 3 and Remote.
  xxxx xxxx x--x xxx-  Unknown / Not used.

*/
	machine().bookkeeping().coin_counter_w(0, data & 0x20);  /* COIN 1 */
	machine().bookkeeping().coin_counter_w(1, data & 0x40);  /* COIN 2 */
	machine().bookkeeping().coin_counter_w(2, data & 0x01);  /* PAYOUT */
}

WRITE16_MEMBER(umipoker_state::saiyu_counters_w)
{
/*
  0x0000 - Normal State (lamps off).
  0x0100 - Payout pulse.
  0x2000 - Coin 1.
  0x4000 - Coin 2, 3 and Remote.

  - Hbits -/- Lbits -
  7654 3210 7654 3210
  ===================
  ---- ---x ---- ----  Payout pulse.
  --x- ---- ---- ----  Coin 1.
  -x-- ---- ---- ----  Coin 2, 3 and Remote.
  x--x xxx- xxxx xxxx  Unknown / Not used.

*/
	machine().bookkeeping().coin_counter_w(0, data & 0x2000);    /* COIN 1 */
	machine().bookkeeping().coin_counter_w(1, data & 0x4000);    /* COIN 2 */
	machine().bookkeeping().coin_counter_w(2, data & 0x0100);    /* PAYOUT */
}


static ADDRESS_MAP_START( umipoker_map, AS_PROGRAM, 16, umipoker_state )
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x400000, 0x403fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x600000, 0x6007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x800000, 0x801fff) AM_RAM_WRITE(umipoker_vram_0_w) AM_SHARE("vra0")
	AM_RANGE(0x802000, 0x803fff) AM_RAM_WRITE(umipoker_vram_1_w) AM_SHARE("vra1")
	AM_RANGE(0x804000, 0x805fff) AM_RAM_WRITE(umipoker_vram_2_w) AM_SHARE("vra2")
	AM_RANGE(0x806000, 0x807fff) AM_RAM_WRITE(umipoker_vram_3_w) AM_SHARE("vra3")
	AM_RANGE(0xc00000, 0xc0ffff) AM_READ8(z80_rom_readback_r,0x00ff)
	AM_RANGE(0xc1f000, 0xc1ffff) AM_READWRITE8(z80_shared_ram_r,z80_shared_ram_w,0x00ff)
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("IN0")
	AM_RANGE(0xe00004, 0xe00005) AM_READ_PORT("IN1") // unused?
	AM_RANGE(0xe00008, 0xe00009) AM_READ_PORT("IN2")
//  AM_RANGE(0xe0000c, 0xe0000d) AM_WRITE(lamps_w) -----> lamps only for saiyukip.
//  AM_RANGE(0xe00010, 0xe00011) AM_WRITE(counters_w) --> coin counters for both games.
	AM_RANGE(0xe00014, 0xe00015) AM_READ_PORT("DSW1-2")
	AM_RANGE(0xe00018, 0xe00019) AM_READ_PORT("DSW3-4")
	AM_RANGE(0xe00020, 0xe00021) AM_WRITE(umipoker_scrolly_0_w)
	AM_RANGE(0xe00022, 0xe00023) AM_WRITE(umipoker_irq_ack_w)
	AM_RANGE(0xe00026, 0xe00027) AM_WRITE(umipoker_scrolly_2_w)
	AM_RANGE(0xe0002a, 0xe0002b) AM_WRITE(umipoker_scrolly_1_w)
	AM_RANGE(0xe0002c, 0xe0002d) AM_WRITENOP // unknown meaning, bit 0 goes from 0 -> 1 on IRQ service routine
	AM_RANGE(0xe0002e, 0xe0002f) AM_WRITE(umipoker_scrolly_3_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( umipoker_audio_map, AS_PROGRAM, 8, umipoker_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(z80_shared_ram_r,z80_shared_ram_w) AM_SHARE("z80_wram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( umipoker_audio_io_map, AS_IO, 8, umipoker_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("ym", ym3812_device, read, write)
ADDRESS_MAP_END


static INPUT_PORTS_START( common )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_HIGH )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( umipoker )
	PORT_INCLUDE( common )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )  PORT_NAME("Small")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

/*  Seems that the default switches are all ON.
    This needs confirmation, since the game title
    by default should be the second one.
*/
	PORT_START("DSW1-2")
	PORT_DIPNAME( 0x0003, 0x0003, "Main Level" )        PORT_DIPLOCATION("DSW1:!1,!2")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0001, "2" )
	PORT_DIPSETTING(    0x0002, "3" )
	PORT_DIPSETTING(    0x0003, "4" )
	PORT_DIPNAME( 0x001c, 0x001c, "Double-Up Level" )   PORT_DIPLOCATION("DSW1:!3,!4,!5")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0004, "2" )
	PORT_DIPSETTING(    0x0008, "3" )
	PORT_DIPSETTING(    0x000c, "4" )
	PORT_DIPSETTING(    0x0010, "5" )
	PORT_DIPSETTING(    0x0014, "6" )
	PORT_DIPSETTING(    0x0018, "7" )
	PORT_DIPSETTING(    0x001c, "8" )
	PORT_DIPNAME( 0x0060, 0x0060, "Bet Max" )           PORT_DIPLOCATION("DSW1:!6,!7")
	PORT_DIPSETTING(    0x0000, "10" )
	PORT_DIPSETTING(    0x0020, "20" )
	PORT_DIPSETTING(    0x0040, "30" )
	PORT_DIPSETTING(    0x0060, "50" )
	PORT_DIPNAME( 0x0080, 0x0080, "Magnification" )     PORT_DIPLOCATION("DSW1:!8")
	PORT_DIPSETTING(    0x0000, "A Type" )
	PORT_DIPSETTING(    0x0080, "B Type" )

	PORT_DIPNAME( 0x0700, 0x0700, "Key In - Hopper Out" )   PORT_DIPLOCATION("DSW2:!1,!2,!3")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0100, "10" )
	PORT_DIPSETTING(    0x0200, "50" )
	PORT_DIPSETTING(    0x0300, "100" )
	PORT_DIPSETTING(    0x0400, "200" )
	PORT_DIPSETTING(    0x0500, "300" )
	PORT_DIPSETTING(    0x0600, "500" )
	PORT_DIPSETTING(    0x0700, "1000" )
	PORT_DIPNAME( 0x3800, 0x3800, "Coin-In" )           PORT_DIPLOCATION("DSW2:!4,!5,!6")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0800, "5" )
	PORT_DIPSETTING(    0x1000, "10" )
	PORT_DIPSETTING(    0x1800, "20" )
	PORT_DIPSETTING(    0x2000, "25" )
	PORT_DIPSETTING(    0x2800, "50" )
	PORT_DIPSETTING(    0x3000, "100" )
	PORT_DIPSETTING(    0x3800, "250" )
	PORT_DIPNAME( 0xc000, 0xc000, "Service-In" )        PORT_DIPLOCATION("DSW2:!7,!8")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x4000, "5" )
	PORT_DIPSETTING(    0x8000, "10" )
	PORT_DIPSETTING(    0xc000, "100" )

	PORT_START("DSW3-4")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin In Limit" )     PORT_DIPLOCATION("DSW3:!1,!2,!3")
	PORT_DIPSETTING(    0x0001, "1000" )
	PORT_DIPSETTING(    0x0002, "2000" )
	PORT_DIPSETTING(    0x0003, "3000" )
	PORT_DIPSETTING(    0x0004, "5000" )
	PORT_DIPSETTING(    0x0005, "10000" )
	PORT_DIPSETTING(    0x0006, "20000" )
	PORT_DIPSETTING(    0x0007, "30000" )
	PORT_DIPSETTING(    0x0000, "999999" )
	PORT_DIPNAME( 0x0038, 0x0038, "Credit Limit" )      PORT_DIPLOCATION("DSW3:!4,!5,!6")
	PORT_DIPSETTING(    0x0008, "1000" )
	PORT_DIPSETTING(    0x0010, "3000" )
	PORT_DIPSETTING(    0x0018, "5000" )
	PORT_DIPSETTING(    0x0020, "10000" )
	PORT_DIPSETTING(    0x0028, "20000" )
	PORT_DIPSETTING(    0x0030, "30000" )
	PORT_DIPSETTING(    0x0038, "50000" )
	PORT_DIPSETTING(    0x0000, "999999" )
	PORT_DIPNAME( 0x0040, 0x0040, "Out Coin Counter" )  PORT_DIPLOCATION("DSW3:!7")     /* Conditional to 'Hopper Sub-Board' (DSW4-3) */
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Credit Cut" )        PORT_DIPLOCATION("DSW3:!8")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, "4 Card Fever" )      PORT_DIPLOCATION("DSW4:!1,!2")
	PORT_DIPSETTING(    0x0000, "A Type" )
	PORT_DIPSETTING(    0x0100, "B Type" )
	PORT_DIPSETTING(    0x0200, "C Type" )
	PORT_DIPSETTING(    0x0300, "D Type" )
	PORT_DIPNAME( 0x0400, 0x0400, "Hopper Sub-Board" )  PORT_DIPLOCATION("DSW4:!3")     /* When off, allow set the 'Out Coin Counter' (DSW3-7) */
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, "Use" )
	PORT_DIPNAME( 0x0800, 0x0000, "Title Type" )        PORT_DIPLOCATION("DSW4:!4")
	PORT_DIPSETTING(    0x0000, "A Type: Umi de Poker" )
	PORT_DIPSETTING(    0x0800, "B Type: Marine Paradise" )
	PORT_DIPNAME( 0x3000, 0x3000, "First Bet" )         PORT_DIPLOCATION("DSW4:!5,!6")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x1000, "5" )
	PORT_DIPSETTING(    0x2000, "10" )
	PORT_DIPSETTING(    0x3000, "20" )
	PORT_DIPNAME( 0x4000, 0x4000, "Fever Initialize" )  PORT_DIPLOCATION("DSW4:!7")
	PORT_DIPSETTING(    0x0000, "A Type" )
	PORT_DIPSETTING(    0x4000, "B Type" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:!8")     /* Unmapped?... */
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( saiyukip )
	PORT_INCLUDE( common )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) PORT_NAME("Left / Stop")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )  PORT_NAME("Right")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1-2")
	PORT_DIPNAME( 0x0007, 0x0007, "Main Level" )        PORT_DIPLOCATION("DSW1:!1,!2,!3")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0001, "2" )
	PORT_DIPSETTING(    0x0002, "3" )
	PORT_DIPSETTING(    0x0003, "4" )
	PORT_DIPSETTING(    0x0004, "5" )
	PORT_DIPSETTING(    0x0005, "6" )
	PORT_DIPSETTING(    0x0006, "7" )
	PORT_DIPSETTING(    0x0007, "8" )
	PORT_DIPNAME( 0x0038, 0x0038, "Double-Up Level" )   PORT_DIPLOCATION("DSW1:!4,!5,!6")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0008, "2" )
	PORT_DIPSETTING(    0x0010, "3" )
	PORT_DIPSETTING(    0x0018, "4" )
	PORT_DIPSETTING(    0x0020, "5" )
	PORT_DIPSETTING(    0x0028, "6" )
	PORT_DIPSETTING(    0x0030, "7" )
	PORT_DIPSETTING(    0x0038, "8" )
	PORT_DIPNAME( 0x0040, 0x0040, "Percentage Wide" )   PORT_DIPLOCATION("DSW1:!7")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "One Game Timer" )    PORT_DIPLOCATION("DSW1:!8")
	PORT_DIPSETTING(    0x0000, "20 Sec." )
	PORT_DIPSETTING(    0x0080, "50 Sec." )

	PORT_DIPNAME( 0x0700, 0x0700, "Key In - Hopper Out" )   PORT_DIPLOCATION("DSW2:!1,!2,!3")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0100, "5" )
	PORT_DIPSETTING(    0x0200, "10" )
	PORT_DIPSETTING(    0x0300, "20" )
	PORT_DIPSETTING(    0x0400, "50" )
	PORT_DIPSETTING(    0x0500, "100" )
	PORT_DIPSETTING(    0x0600, "500" )
	PORT_DIPSETTING(    0x0700, "1000" )
	PORT_DIPNAME( 0x3800, 0x3800, "Coin-In" )           PORT_DIPLOCATION("DSW2:!4,!5,!6")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0800, "5" )
	PORT_DIPSETTING(    0x1000, "10" )
	PORT_DIPSETTING(    0x1800, "20" )
	PORT_DIPSETTING(    0x2000, "25" )
	PORT_DIPSETTING(    0x2800, "50" )
	PORT_DIPSETTING(    0x3000, "100" )
	PORT_DIPSETTING(    0x3800, "250" )
	PORT_DIPNAME( 0xc000, 0xc000, "Service-In" )        PORT_DIPLOCATION("DSW2:!7,!8")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x4000, "5" )
	PORT_DIPSETTING(    0x8000, "10" )
	PORT_DIPSETTING(    0xc000, "100" )

	PORT_START("DSW3-4")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin In Max" )       PORT_DIPLOCATION("DSW3:!1,!2,!3")
	PORT_DIPSETTING(    0x0001, "1000" )
	PORT_DIPSETTING(    0x0002, "2000" )
	PORT_DIPSETTING(    0x0003, "3000" )
	PORT_DIPSETTING(    0x0004, "5000" )
	PORT_DIPSETTING(    0x0005, "10000" )
	PORT_DIPSETTING(    0x0006, "20000" )
	PORT_DIPSETTING(    0x0007, "30000" )
	PORT_DIPSETTING(    0x0000, "50000" )
	PORT_DIPNAME( 0x0038, 0x0038, "Limit Over" )        PORT_DIPLOCATION("DSW3:!4,!5,!6")
	PORT_DIPSETTING(    0x0008, "1000" )
	PORT_DIPSETTING(    0x0010, "3000" )
	PORT_DIPSETTING(    0x0018, "5000" )
	PORT_DIPSETTING(    0x0020, "10000" )
	PORT_DIPSETTING(    0x0028, "20000" )
	PORT_DIPSETTING(    0x0030, "30000" )
	PORT_DIPSETTING(    0x0038, "50000" )
	PORT_DIPSETTING(    0x0000, "Limit Over & Coin In Max OFF" )
	PORT_DIPNAME( 0x0040, 0x0040, "Demo Sound" )        PORT_DIPLOCATION("DSW3:!7")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "BGM" )               PORT_DIPLOCATION("DSW3:!8")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, "First Bet" )         PORT_DIPLOCATION("DSW4:!1,!2")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0100, "5" )
	PORT_DIPSETTING(    0x0200, "10" )
	PORT_DIPSETTING(    0x0300, "15" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Bet Max" )           PORT_DIPLOCATION("DSW4:!3,!4")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0400, "10" )
	PORT_DIPSETTING(    0x0800, "30" )
	PORT_DIPSETTING(    0x0c00, "50" )
	PORT_DIPNAME( 0x1000, 0x1000, "Hopper" )            PORT_DIPLOCATION("DSW4:!5")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Medal Sub-Board" )   PORT_DIPLOCATION("DSW4:!6")     /* When off, allow 'Out Counter' to be set */
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Out Counter" )       PORT_DIPLOCATION("DSW4:!7")     /* Conditional to 'Medal Sub-Board' */
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Credit Over Cut" )   PORT_DIPLOCATION("DSW4:!8")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4)  },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( umipoker )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,     0, 0x40)
GFXDECODE_END

void umipoker_state::machine_start()
{
}

void umipoker_state::machine_reset()
{
}

// TODO: clocks
static MACHINE_CONFIG_START( umipoker, umipoker_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,16000000) // TMP68HC000-16
	MCFG_CPU_PROGRAM_MAP(umipoker_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", umipoker_state,  irq6_line_assert)

	MCFG_CPU_ADD("audiocpu",Z80,4000000)
	MCFG_CPU_PROGRAM_MAP(umipoker_audio_map)
	MCFG_CPU_IO_MAP(umipoker_audio_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(umipoker_state, irq0_line_hold,  120)  // ? controls ym3812 music tempo

	MCFG_NVRAM_ADD_1FILL("nvram")


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 48*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(umipoker_state, screen_update_umipoker)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", umipoker)

	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym", YM3812, 4000000 / 2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", 4000000 / 2, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  ROM Loads

***************************************************************************/

ROM_START( umipoker )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sp0.u61",      0x000000, 0x020000, CRC(866eaa02) SHA1(445afdfe010aad1102219a0dbd3a363a22294b4c) )
	ROM_LOAD16_BYTE( "sp1.u60",      0x000001, 0x020000, CRC(8db08696) SHA1(2854d511a8fd30b023e2a2a00b25413f88205d82) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sz.u8",        0x000000, 0x010000, CRC(d874ba1a) SHA1(13c06f3b67694d5d5194023c4f7b75aea8b57129) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sg0.u42",      0x000000, 0x020000, CRC(876f1f4f) SHA1(eca4c397be57812f2c34791736fee7c43925d927) )
	ROM_LOAD( "sg1.u41",      0x020000, 0x020000, CRC(7fcbfb17) SHA1(be2f308a8e8f0941c54125950702ddfbd8538733) )
	ROM_LOAD( "sg2.u40",      0x040000, 0x020000, CRC(eb31649b) SHA1(c0741d85537827e2396e81a1aa3005871dffad78) )
	ROM_LOAD( "sg3.u39",      0x060000, 0x020000, CRC(ebd5f96d) SHA1(968c107ee17f1e92ffc2835e13803347881862f1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sm.u17",       0x000000, 0x040000, CRC(99503aed) SHA1(011404fad01b3ced708a94143908be3e1d0194d3) ) // first half 1-filled
	ROM_CONTINUE(             0x000000, 0x040000 )
ROM_END

ROM_START( saiyukip )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "slp0-spq.u61", 0x000000, 0x020000, CRC(7fc0f201) SHA1(969170d68278e212dd459744373ed9e704976e45) )
	ROM_LOAD16_BYTE( "slp1-spq.u60", 0x000001, 0x020000, CRC(c8e3547c) SHA1(18bb380a64ed36f45a377b86cbbac892efe879bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "slz.u8",       0x000000, 0x010000, CRC(4f32ba1c) SHA1(8f1f8c0995bcd05d19120dd3b64b135908caf759) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "slg0.u42",     0x000000, 0x020000, CRC(49ba7ffd) SHA1(3bbb7656eafbd8c91c9054fca056c8fc3002ed13) )
	ROM_LOAD( "slg1.u41",     0x020000, 0x020000, CRC(59b5f399) SHA1(2b999cebcc53b3b8fd38e3034a12434d82b6fad3) )
	ROM_LOAD( "slg2.u40",     0x040000, 0x020000, CRC(fe6cd717) SHA1(65e59d88a30efd0cec642cda54e2bc38196f0231) )
	ROM_LOAD( "slg3.u39",     0x060000, 0x020000, CRC(e99b2906) SHA1(77884d2dae2e7f7cf27103aa8bbd0eaa39628993) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "slm.u17",      0x000000, 0x040000, CRC(b50eb70b) SHA1(342fcb307844f4d0a02a85b2c61e73b5e8bacd44) ) // first half 1-filled
	ROM_CONTINUE(             0x000000, 0x040000 )
ROM_END


/******************************************
*              Driver Init                *
******************************************/

DRIVER_INIT_MEMBER(umipoker_state,umipoker)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe00010, 0xe00011, write16_delegate(FUNC(umipoker_state::umi_counters_w), this));
}

DRIVER_INIT_MEMBER(umipoker_state,saiyukip)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe00010, 0xe00011, write16_delegate(FUNC(umipoker_state::saiyu_counters_w), this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xe0000c, 0xe0000d, write16_delegate(FUNC(umipoker_state::lamps_w), this));
}


/******************************************
*              Game Drivers               *
******************************************/

/*     YEAR  NAME       PARENT    MACHINE    INPUT      INIT      ROT    COMPANY                  FULLNAME                                 FLAGS   LAYOUT      */
GAME(  1997, umipoker,  0,        umipoker,  umipoker, umipoker_state,  umipoker, ROT0, "World Station Co.,LTD", "Umi de Poker / Marine Paradise (Japan)", 0 )                      // title screen is toggleable thru a dsw
GAMEL( 1998, saiyukip,  0,        umipoker,  saiyukip, umipoker_state,  saiyukip, ROT0, "World Station Co.,LTD", "Slot Poker Saiyuki (Japan)",             0,      layout_saiyukip )
