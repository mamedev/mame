// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*********************************************************************************************************************

Mahjong Dunhuang (C)1995 Spirit

driver by Luca Elia

PCB Layout
----------

|---------------------------------------|
|uPD1242H  VOL     UM3567  AR17961  ROM6|
|          3.579545MHz                  |
|                  DSW1(8)          ROM5|
|   VOL            DSW2(8)              |
|                  DSW3(8)          ROM4|
|                  DSW4(8)              |
|                  DSW5(8)          ROM3|
|1   WF19054                            |
|8                                  ROM2|
|W                      |-------|       |
|A                      |       |       |
|Y                      |  *    |       |
|                       |       |       |
|                       |-------|       |
|                    12MHz         6264 |
|                                       |
|                                       |
|1         PAL                     6264 |
|0                                      |
|W                     Z80              |
|A  HM86171-80   BATTERY           6264 |
|Y                     PAL              |
|          PAL         PAL          ROM1|
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      Z80     - clock 6.000MHz [12/2]
      WF19054 - == AY3-8910. Clock = 1.500MHz [12/8]
      UM3567  - == YM2413. Clock 3.579545MHz
      HM86171 - Hualon Microelectronics HMC HM86171 VGA 256 colour RAMDAC (DIP28)
      6264    - UT6264PC-70LL 8k x8 SRAM (DIP28)
      *       - QFP120 IC marked 55102602-12. Logo on the chip is of two 5 1/4 inch floppy
                discs standing upright and next to each other, with the corner facing up,
                like 2 pyramids. The middle hole and head access slot are visible too.
                Chip was manufactured 50th week of 1993
      AR17961 - == Oki M6295 (QFP44). Clock = 1.500MHz [12/8]. pin 7 = high
      VSync   - 60Hz
      HSync   - 15.28kHz

*********************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"

#define DUNHUANG_DEBUG  0


class dunhuang_state : public driver_device
{
public:
	dunhuang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* video-related */
	tilemap_t         *m_tmap;
	tilemap_t       *m_tmap2;
	int             m_written;
	int             m_written2;
	UINT8           m_pos_x;
	UINT8           m_pos_y;
	UINT8           m_clear_y;
	UINT8           m_block_x;
	UINT8           m_block_y;
	UINT8           m_block_w;
	UINT8           m_block_h;
	UINT8           m_block_addr_hi;
	UINT8           m_block_addr_lo;
	UINT8           m_block_dest;
	UINT8           m_block_c;
	UINT8           m_layers;
	int             m_paloffs;

	/* input-related */
	UINT8           m_input;
	UINT8           m_hopper;

	/* memory */
	UINT16         m_videoram[0x40 * 0x20];
	UINT16         m_videoram2[0x40 * 0x8];
	UINT8          m_colorram[0x40 * 0x20];
	UINT8          m_colorram2[0x40 * 0x8];
	UINT8          m_paldata[3 * 256];
	DECLARE_WRITE8_MEMBER(dunhuang_pos_x_w);
	DECLARE_WRITE8_MEMBER(dunhuang_pos_y_w);
	DECLARE_WRITE8_MEMBER(dunhuang_tile_w);
	DECLARE_WRITE8_MEMBER(dunhuang_tile2_w);
	DECLARE_WRITE8_MEMBER(dunhuang_clear_y_w);
	DECLARE_WRITE8_MEMBER(dunhuang_horiz_clear_w);
	DECLARE_WRITE8_MEMBER(dunhuang_vert_clear_w);
	DECLARE_WRITE8_MEMBER(dunhuang_block_dest_w);
	DECLARE_WRITE8_MEMBER(dunhuang_block_x_w);
	DECLARE_WRITE8_MEMBER(dunhuang_block_y_w);
	DECLARE_WRITE8_MEMBER(dunhuang_block_w_w);
	DECLARE_WRITE8_MEMBER(dunhuang_block_c_w);
	DECLARE_WRITE8_MEMBER(dunhuang_block_addr_lo_w);
	DECLARE_WRITE8_MEMBER(dunhuang_block_addr_hi_w);
	DECLARE_WRITE8_MEMBER(dunhuang_block_h_w);
	DECLARE_WRITE8_MEMBER(dunhuang_paloffs_w);
	DECLARE_WRITE8_MEMBER(dunhuang_paldata_w);
	DECLARE_WRITE8_MEMBER(dunhuang_layers_w);
	DECLARE_WRITE8_MEMBER(dunhuang_input_w);
	DECLARE_READ8_MEMBER(dunhuang_service_r);
	DECLARE_READ8_MEMBER(dunhuang_input_r);
	DECLARE_WRITE8_MEMBER(dunhuang_rombank_w);
	DECLARE_READ8_MEMBER(dunhuang_dsw_r);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_dunhuang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};


/***************************************************************************
                                Video Hardware
***************************************************************************/


TILE_GET_INFO_MEMBER(dunhuang_state::get_tile_info)
{
	UINT16 code = m_videoram[tile_index];
	UINT8 color = m_colorram[tile_index] & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
}
TILE_GET_INFO_MEMBER(dunhuang_state::get_tile_info2)
{
	UINT16 code = m_videoram2[tile_index];
	UINT8 color = m_colorram2[tile_index] & 0x0f;
	SET_TILE_INFO_MEMBER(1, code, color, 0);
}

void dunhuang_state::video_start()
{
	m_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dunhuang_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8,8, 0x40,0x20);
	m_tmap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dunhuang_state::get_tile_info2),this), TILEMAP_SCAN_ROWS, 8,32, 0x40,0x8);

	m_tmap->set_transparent_pen(0);
	m_tmap2->set_transparent_pen(0);

	save_item(NAME(m_videoram));
	save_item(NAME(m_colorram));
	save_item(NAME(m_videoram2));
	save_item(NAME(m_colorram2));
	save_item(NAME(m_paldata));
}

UINT32 dunhuang_state::screen_update_dunhuang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#if DUNHUANG_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	bitmap.fill(m_palette->black_pen(), cliprect);

	switch (m_layers)
	{
		case 0x04:  // girl select: bg over fg
			if (layers_ctrl & 2)    m_tmap2->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 1)    m_tmap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x05:  // dips: must hide fg
			if (layers_ctrl & 1)    m_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			break;
		case 0x07:  // game,demo: fg over bg
		default:
			if (layers_ctrl & 1)    m_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 2)    m_tmap2->draw(screen, bitmap, cliprect, 0, 0);
			break;
	}

	return 0;
}

// Tilemaps access

WRITE8_MEMBER(dunhuang_state::dunhuang_pos_x_w)
{
	m_pos_x = data & 0x3f;
	m_written = 0;
	m_written2 = 0;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_pos_y_w)
{
	m_pos_y = data;
	m_written = 0;
	m_written2 = 0;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_tile_w)
{
	int addr;

	if (m_written & (1 << offset))
	{
		m_written = 0;
		m_pos_x++;
		if (m_pos_x == 0x40)
		{
			m_pos_x = 0;
			m_pos_y++;
		}
	}
	m_written |= 1 << offset;

	addr = (m_pos_x & 0x3f) + (m_pos_y & 0x1f) * 0x40;
	switch (offset)
	{
		case 0: m_videoram[addr] = (m_videoram[addr] & 0xff00) | data;      break;
		case 1: m_videoram[addr] = (m_videoram[addr] & 0x00ff) | (data<<8); break;
		case 2: m_colorram[addr] = data;                                                break;
	}
	m_tmap->mark_tile_dirty(addr);
}

WRITE8_MEMBER(dunhuang_state::dunhuang_tile2_w)
{
	int addr;

	if (m_written2 & (1 << offset))
	{
		m_written2 = 0;
		m_pos_x++;
		if (m_pos_x == 0x40)
		{
			m_pos_x = 0;
			m_pos_y++;
		}
	}
	m_written2 |= 1 << offset;

	addr = (m_pos_x & 0x3f) + (m_pos_y & 0x07) * 0x40;
	switch (offset)
	{
		case 0: m_videoram2[addr] = (m_videoram2[addr] & 0xff00) | data;        break;
		case 1: m_videoram2[addr] = (m_videoram2[addr] & 0x00ff) | (data<<8);   break;
		case 2: m_colorram2[addr] = data;                                           break;
	}
	m_tmap2->mark_tile_dirty(addr);
}

// Clear a row of tiles (videoram)

WRITE8_MEMBER(dunhuang_state::dunhuang_clear_y_w)
{
	m_clear_y = data;
}
WRITE8_MEMBER(dunhuang_state::dunhuang_horiz_clear_w)
{
	int i;
//  logerror("%06x: horiz clear, y = %02x, data = %02d\n", space.device().safe_pc(), m_clear_y,data);
	for (i = 0; i < 0x40; i++)
	{
		int addr = m_clear_y * 0x40 + i;

		m_videoram[addr] = 0;
		m_colorram[addr] = 0;
		m_tmap->mark_tile_dirty(addr);
	}
}

// Clear a column of tiles (videoram2)

WRITE8_MEMBER(dunhuang_state::dunhuang_vert_clear_w)
{
	int i;
//  logerror("%06x: vert clear, x = %02x, y = %02x, data = %02x\n", space.device().safe_pc(), m_pos_x,m_pos_y,data);
	for (i = 0; i < 0x08; i++)
	{
		int addr = (m_pos_x & 0x3f) + (i & 0x07) * 0x40;

		m_videoram2[addr] = 1;
		m_colorram2[addr] = 0;
		m_tmap2->mark_tile_dirty(addr);
	}
}


// Draw a block of tiles.
//
// The tiles codes are read from the graphics roms too!
//

WRITE8_MEMBER(dunhuang_state::dunhuang_block_dest_w)
{
	m_block_dest = data;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_block_x_w)
{
	m_block_x = data;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_block_y_w)
{
	m_block_y = data;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_block_w_w)
{
	m_block_w = data;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_block_c_w)
{
	m_block_c = data;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_block_addr_lo_w)
{
	m_block_addr_lo = data;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_block_addr_hi_w)
{
	m_block_addr_hi = data;
}


WRITE8_MEMBER(dunhuang_state::dunhuang_block_h_w)
{
	int i,j, addr;
	UINT8 *tile_addr;

//  logerror("%06x: block dst %x, src %x, xy %x %x, wh %x %x, clr %x\n", space.device().safe_pc(), m_block_dest, (m_block_addr_hi << 8) + m_block_addr_lo, m_block_x,m_block_y,m_block_w+1,m_block_h+1,m_block_c);

	m_block_h = data;

	tile_addr = memregion("gfx2")->base() + ((m_block_addr_hi << 8) + m_block_addr_lo) * 4;

	switch (m_block_dest)
	{
		case 0x04:  // write to videoram
			for (j = 0; j <= m_block_h; j++)
			{
				for (i = 0; i <= m_block_w; i++)
				{
					addr = ((m_block_x + i)& 0x3f) + ((m_block_y + j) & 0x1f) * 0x40;

					m_videoram[addr] = (tile_addr[1] << 8) | tile_addr[0];
					m_colorram[addr] = m_block_c;
					m_tmap->mark_tile_dirty(addr);
					tile_addr += 4;
				}
			}
			break;

		case 0x08:  // write to videoram2
			for (j = 0; j <= m_block_h; j++)
			{
				for (i = 0; i <= m_block_w; i++)
				{
					addr = ((m_block_x + i)& 0x3f) + ((m_block_y + j) & 0x7) * 0x40;

					m_videoram2[addr] = (tile_addr[1] << 8) | tile_addr[0];
					m_colorram2[addr] = m_block_c;
					m_tmap2->mark_tile_dirty(addr);
					tile_addr += 4;
				}
			}
			break;

		default:
			popmessage("%06x: block dst=%x", space.device().safe_pc(), m_block_dest);
	}
}

// Palette: HMC HM86171 VGA 256 colour RAMDAC

WRITE8_MEMBER(dunhuang_state::dunhuang_paloffs_w)
{
	m_paloffs = data * 3;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_paldata_w)
{
	m_paldata[m_paloffs] = data;

	m_palette->set_pen_color( m_paloffs/3,
		pal6bit(m_paldata[(m_paloffs/3)*3+0]),
		pal6bit(m_paldata[(m_paloffs/3)*3+1]),
		pal6bit(m_paldata[(m_paloffs/3)*3+2])
	);

	m_paloffs = (m_paloffs + 1) % (3*256);
}

// Layers control (not understood)

WRITE8_MEMBER(dunhuang_state::dunhuang_layers_w)
{
//  popmessage("layers %02x",data);
	m_layers = data;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static ADDRESS_MAP_START( dunhuang_map, AS_PROGRAM, 8, dunhuang_state )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK( "bank1" )
ADDRESS_MAP_END

// Inputs

WRITE8_MEMBER(dunhuang_state::dunhuang_input_w)
{
	m_input = data;
}

READ8_MEMBER(dunhuang_state::dunhuang_service_r)
{
	return ioport("SERVICE")->read()
		| ((m_hopper && !(m_screen->frame_number() % 10)) ? 0x00 : 0x08)    // bit 3: hopper sensor
		| 0x80                                                              // bit 7 low -> tiles block transferrer busy
	;
}

READ8_MEMBER(dunhuang_state::dunhuang_dsw_r)
{
	if (!(m_input & 0x01))  return ioport("DSW1")->read();
	if (!(m_input & 0x02))  return ioport("DSW2")->read();
	if (!(m_input & 0x04))  return ioport("DSW3")->read();
	if (!(m_input & 0x08))  return ioport("DSW4")->read();
	if (!(m_input & 0x10))  return ioport("DSW5")->read();
	logerror("%s: warning, unknown dsw bits read, input = %02x\n", machine().describe_context(), m_input);
	return 0xff;
}
READ8_MEMBER(dunhuang_state::dunhuang_input_r)
{
	if (!(m_input & 0x01))  return ioport("IN0")->read();
	if (!(m_input & 0x02))  return ioport("IN1")->read();
	if (!(m_input & 0x04))  return ioport("IN2")->read();
	if (!(m_input & 0x08))  return ioport("IN3")->read();
	if (!(m_input & 0x10))  return ioport("IN4")->read();
	logerror("%s: warning, unknown input bits read, input = %02x\n", machine().describe_context(), m_input);
	return 0xff;
}

WRITE8_MEMBER(dunhuang_state::dunhuang_rombank_w)
{
	// ?                data & 0x01
	// ?                data & 0x02

	membank("bank1")->set_entry(((data >> 2) & 0x7));

	// COIN OUT:        data & 0x20
	coin_counter_w(machine(), 0,    data & 0x40);
	m_hopper = data & 0x80;
}


#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(dunhuang_state::dunhuang_82_w)
{
//  popmessage("82 = %02x",dunhuang_82);
}
#endif

static ADDRESS_MAP_START( dunhuang_io_map, AS_IO, 8, dunhuang_state )
	AM_RANGE( 0x0000, 0x0000 ) AM_WRITE(dunhuang_pos_x_w )
	AM_RANGE( 0x0001, 0x0001 ) AM_WRITE(dunhuang_pos_y_w )
	AM_RANGE( 0x0002, 0x0004 ) AM_WRITE(dunhuang_tile_w )
	AM_RANGE( 0x0005, 0x0007 ) AM_WRITE(dunhuang_tile2_w )

	AM_RANGE( 0x0008, 0x0008 ) AM_WRITE(dunhuang_vert_clear_w )

	AM_RANGE( 0x000c, 0x000c ) AM_READ(watchdog_reset_r )

	AM_RANGE( 0x000f, 0x000f ) AM_WRITE(dunhuang_block_addr_lo_w )
	AM_RANGE( 0x0010, 0x0010 ) AM_WRITE(dunhuang_block_addr_hi_w )
//  AM_RANGE( 0x0011, 0x0011 ) ?
	AM_RANGE( 0x0012, 0x0012 ) AM_WRITE(dunhuang_block_c_w )
	AM_RANGE( 0x0015, 0x0015 ) AM_WRITE(dunhuang_block_x_w )
	AM_RANGE( 0x0016, 0x0016 ) AM_WRITE(dunhuang_block_y_w )
	AM_RANGE( 0x0017, 0x0017 ) AM_WRITE(dunhuang_block_w_w )
	AM_RANGE( 0x0018, 0x0018 ) AM_WRITE(dunhuang_block_h_w )

	AM_RANGE( 0x0019, 0x0019 ) AM_WRITE(dunhuang_clear_y_w )
	AM_RANGE( 0x001a, 0x001a ) AM_WRITE(dunhuang_horiz_clear_w )

	AM_RANGE( 0x001b, 0x001b ) AM_WRITE(dunhuang_block_dest_w )

	AM_RANGE( 0x0081, 0x0081 ) AM_DEVWRITE("ymsnd", ym2413_device, register_port_w)
	AM_RANGE( 0x0089, 0x0089 ) AM_DEVWRITE("ymsnd", ym2413_device, data_port_w)

//  AM_RANGE( 0x0082, 0x0082 ) AM_WRITE(dunhuang_82_w )

	AM_RANGE( 0x0083, 0x0083 ) AM_WRITE(dunhuang_paloffs_w )
	AM_RANGE( 0x008b, 0x008b ) AM_WRITE(dunhuang_paldata_w )

	AM_RANGE( 0x0084, 0x0084 ) AM_READ(dunhuang_service_r )
	AM_RANGE( 0x0085, 0x0085 ) AM_READ(dunhuang_input_r )

	AM_RANGE( 0x0086, 0x0086 ) AM_WRITE(dunhuang_rombank_w )
	AM_RANGE( 0x0087, 0x0087 ) AM_WRITE(dunhuang_layers_w )

	AM_RANGE( 0x0088, 0x0088 ) AM_DEVREAD("ay8910", ay8910_device, data_r )
	AM_RANGE( 0x0090, 0x0090 ) AM_DEVWRITE("ay8910", ay8910_device, data_w )
	AM_RANGE( 0x0098, 0x0098 ) AM_DEVWRITE("ay8910", ay8910_device, address_w )
ADDRESS_MAP_END



/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( dunhuang )
	PORT_START("DSW1")      /* IN0 - DSW1 */
	PORT_DIPNAME( 0x0f, 0x0f, "Main Game Chance (%)" )  PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, "78" )
	PORT_DIPSETTING(    0x01, "80" )
	PORT_DIPSETTING(    0x02, "81" )
	PORT_DIPSETTING(    0x03, "83" )
	PORT_DIPSETTING(    0x04, "84" )
	PORT_DIPSETTING(    0x05, "86" )
	PORT_DIPSETTING(    0x06, "87" )
	PORT_DIPSETTING(    0x07, "89" )
	PORT_DIPSETTING(    0x08, "90" )
	PORT_DIPSETTING(    0x09, "92" )
	PORT_DIPSETTING(    0x0a, "93" )
	PORT_DIPSETTING(    0x0b, "94" )
	PORT_DIPSETTING(    0x0c, "95" )
	PORT_DIPSETTING(    0x0d, "96" )
	PORT_DIPSETTING(    0x0e, "97" )
	PORT_DIPSETTING(    0x0f, "98" )
	PORT_DIPNAME( 0x30, 0x30, "Main Game Rate" )        PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "A" )
	PORT_DIPSETTING(    0x20, "B" )
	PORT_DIPSETTING(    0x10, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x40, 0x40, "Input Tokens" )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Keys" )
	PORT_DIPSETTING(    0x00, "Coins" )
	PORT_DIPNAME( 0x80, 0x80, "Output Tokens" )         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Keys" )
	PORT_DIPSETTING(    0x00, "Payout" )

	PORT_START("DSW2")      /* IN1 - DSW2 */
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPNAME( 0x06, 0x06, "Credits Per Coin" )      PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x18, 0x18, "Credits Per Key-In" )    PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x20, 0x20, "Credits Per Key-Out" )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Credits" )           PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x80, "1000" )
	PORT_DIPSETTING(    0xc0, "3000" )

	PORT_START("DSW3")      /* IN2 - DSW3 */
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )               PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )               PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x30, 0x30, "Renso Bonus Time" )      PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0xc0, 0xc0, "DonDen Times" )          PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_START("DSW4")      /* IN3 - DSW4 */
	PORT_DIPNAME( 0x07, 0x07, "Credits Limit" )         PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x07, "2k" )
	PORT_DIPSETTING(    0x06, "3k" )
	PORT_DIPSETTING(    0x05, "5k" )
	PORT_DIPSETTING(    0x04, "10k" )
	PORT_DIPSETTING(    0x03, "20k" )
	PORT_DIPSETTING(    0x02, "30k" )
	PORT_DIPSETTING(    0x01, "50k" )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPNAME( 0x38, 0x38, "Service-In Limit" )      PORT_DIPLOCATION("SW4:4,5,6")
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x08, "30k" )
	PORT_DIPSETTING(    0x10, "40k" )
	PORT_DIPSETTING(    0x18, "50k" )
	PORT_DIPSETTING(    0x20, "70k" )
	PORT_DIPSETTING(    0x28, "100k" )
	PORT_DIPSETTING(    0x30, "200k" )
	PORT_DIPSETTING(    0x38, "990k" )
	PORT_DIPNAME( 0x40, 0x40, "Hu Type" )               PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, "Topple" )
	PORT_DIPSETTING(    0x00, "Non-Topple" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up" )             PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW5")      /* IN4 - DSW5 */
	PORT_DIPNAME( 0x03, 0x03, "Douple Up Chance (%)" )  PORT_DIPLOCATION("SW5:1,2")
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "60" )
	PORT_DIPSETTING(    0x02, "70" )
	PORT_DIPSETTING(    0x03, "80" )
	PORT_DIPNAME( 0x04, 0x04, "Chinese Word" )          PORT_DIPLOCATION("SW5:3")
	PORT_DIPSETTING(    0x04, "3..6" )
	PORT_DIPSETTING(    0x00, "5..10" )
	PORT_DIPNAME( 0x08, 0x08, "Big Odds Times" )        PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Hu Test Times" )         PORT_DIPLOCATION("SW5:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x20, "8" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW5:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Computer Strength" )     PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(    0x80, "Strong" )
	PORT_DIPSETTING(    0x00, "Weak" )

	PORT_START("SERVICE")       /* IN5 - SERVICE */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE3 )      // clear (during boot)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE2 )      // book
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE  )      // test (in game: dips, during boot: service mode)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL  )      // hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE4 )      // payout
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_IMPULSE(2) // "coin jam" otherwise
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )       // 0 = tiles block transferrer busy

	PORT_START("IN0")       /* IN6 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )    // gun
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")       /* IN7 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )  // tin
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")       /* IN8 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )    // eat
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )    // hu
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")       /* IN9 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )    // pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")       /* IN10 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 7*4, 6*4, 1*4, 0*4, 3*4, 2*4, 5*4, 4*4 },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout layout_8x32 =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 7*4, 6*4, 1*4, 0*4, 3*4, 2*4, 5*4, 4*4 },
	{ STEP16(0,32), STEP16(16*32,32) },
	32*32
};

static GFXDECODE_START( dunhuang )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x32, 0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

void dunhuang_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x8000);

	save_item(NAME(m_written));
	save_item(NAME(m_written2));
	save_item(NAME(m_pos_x));
	save_item(NAME(m_pos_y));
	save_item(NAME(m_clear_y));
	save_item(NAME(m_block_x));
	save_item(NAME(m_block_y));
	save_item(NAME(m_block_w));
	save_item(NAME(m_block_h));
	save_item(NAME(m_block_addr_hi));
	save_item(NAME(m_block_addr_lo));
	save_item(NAME(m_block_dest));
	save_item(NAME(m_block_c));
	save_item(NAME(m_layers));
	save_item(NAME(m_paloffs));
	save_item(NAME(m_input));
	save_item(NAME(m_hopper));
}

void dunhuang_state::machine_reset()
{
	m_written = 0;
	m_written2 = 0;
	m_pos_x = 0;
	m_pos_y = 0;
	m_clear_y = 0;
	m_block_x = 0;
	m_block_y = 0;
	m_block_w = 0;
	m_block_h = 0;
	m_block_addr_hi = 0;
	m_block_addr_lo = 0;
	m_block_dest = 0;
	m_block_c = 0;
	m_layers = 0;
	m_paloffs = 0;
	m_input = 0;
	m_hopper = 0;
}


static MACHINE_CONFIG_START( dunhuang, dunhuang_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)
	MCFG_CPU_PROGRAM_MAP(dunhuang_map)
	MCFG_CPU_IO_MAP(dunhuang_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dunhuang_state,  irq0_line_hold)


	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(5))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0+8, 512-8-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(dunhuang_state, screen_update_dunhuang)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dunhuang)
	MCFG_PALETTE_ADD("palette", 0x100)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("ay8910", AY8910, 12000000/8)
	MCFG_AY8910_PORT_B_READ_CB(READ8(dunhuang_state, dunhuang_dsw_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(dunhuang_state, dunhuang_input_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_OKIM6295_ADD("oki", 12000000/8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( dunhuang )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "rom1.u9", 0x00000, 0x40000, CRC(843a0117) SHA1(26a838cb3552ea6a9ec55940fcbf83b06c068743) )
	ROM_RELOAD(          0x10000, 0x40000 )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "rom3.u4", 0x00000, 0x80000, CRC(1ff5d35e) SHA1(b808eb4f81be8fc77a58dadd661a9cc2b376a509) )
	ROM_LOAD( "rom2.u5", 0x80000, 0x40000, CRC(384fa1d3) SHA1(f329db17aacacf1768ebd6ca2cc612503db93fac) )

	ROM_REGION( 0xc0000, "gfx2", 0 )    // do not dispose
	ROM_LOAD( "rom4.u3", 0x00000, 0x40000, CRC(7db45227) SHA1(2a12a2b8a1e58946ce3e7c770b3ca4803c3c3ccd) )
	ROM_LOAD( "rom5.u2", 0x40000, 0x80000, CRC(d609880e) SHA1(3d69800e959e8f24ef950fea4312610c4407f6ba) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rom6.u1", 0x00000, 0x20000, CRC(31cfdc29) SHA1(725249eae9227eadf05418b799e0da0254bb2f51) )
ROM_END

GAME( 1995, dunhuang, 0, dunhuang, dunhuang, driver_device, 0, ROT0, "Spirit", "Mahjong Dunhuang", MACHINE_SUPPORTS_SAVE )
