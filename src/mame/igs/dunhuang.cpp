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

#include "mahjong.h"

#include "cpu/z80/z80.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_GFX     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_GFX)

#include "logmacro.h"

#define LOGGFX(...)     LOGMASKED(LOG_GFX,     __VA_ARGS__)


namespace {

#define DUNHUANG_DEBUG  0


class dunhuang_state : public driver_device
{
public:
	dunhuang_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_videoram(*this, "videoram", 2* 0x40 * 0x20, ENDIANNESS_LITTLE),
		m_videoram2(*this, "videoram2", 2* 0x40 * 0x08, ENDIANNESS_LITTLE),
		m_colorram(*this, "colorram", 0x40 * 0x20, ENDIANNESS_LITTLE),
		m_colorram2(*this, "colorram2", 0x40 * 0x08, ENDIANNESS_LITTLE),
		m_mainbank(*this, "mainbank"),
		m_gfx2(*this, "gfx2"),
		m_inputs(*this, "KEY%u", 0U),
		m_dsw(*this, "DSW%u", 1U),
		m_service(*this, "SERVICE")
	{ }

	void dunhuang(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<hopper_device> m_hopper;

	memory_share_creator<uint16_t> m_videoram;
	memory_share_creator<uint16_t> m_videoram2;
	memory_share_creator<uint8_t> m_colorram;
	memory_share_creator<uint8_t> m_colorram2;
	required_memory_bank m_mainbank;
	required_region_ptr<uint8_t> m_gfx2;

	required_ioport_array<5> m_inputs;
	required_ioport_array<5> m_dsw;
	required_ioport m_service;

	// video-related
	tilemap_t *m_tmap[2] {};
	uint8_t m_written[2] {};
	uint8_t m_pos_x = 0;
	uint8_t m_pos_y = 0;
	uint8_t m_clear_y = 0;
	uint8_t m_block_x = 0;
	uint8_t m_block_y = 0;
	uint8_t m_block_w = 0;
	uint8_t m_block_h = 0;
	uint8_t m_block_addr_hi = 0;
	uint8_t m_block_addr_lo = 0;
	uint8_t m_block_dest = 0;
	uint8_t m_block_c = 0;
	uint8_t m_layers = 0;

	// input-related
	uint8_t m_input = 0;

	void pos_x_w(uint8_t data);
	void pos_y_w(uint8_t data);
	void tile_w(offs_t offset, uint8_t data);
	void tile2_w(offs_t offset, uint8_t data);
	void clear_y_w(uint8_t data);
	void horiz_clear_w(uint8_t data);
	void vert_clear_w(uint8_t data);
	void block_dest_w(uint8_t data);
	void block_x_w(uint8_t data);
	void block_y_w(uint8_t data);
	void block_w_w(uint8_t data);
	void block_c_w(uint8_t data);
	void block_addr_lo_w(uint8_t data);
	void block_addr_hi_w(uint8_t data);
	void block_h_w(uint8_t data);
	void layers_w(uint8_t data);
	void input_w(uint8_t data);
	uint8_t service_r();
	uint8_t input_r();
	void rombank_w(uint8_t data);
	uint8_t dsw_r();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


/***************************************************************************
                                Video Hardware
***************************************************************************/


TILE_GET_INFO_MEMBER(dunhuang_state::get_tile_info)
{
	uint16_t const code = m_videoram[tile_index];
	uint8_t const color = m_colorram[tile_index] & 0x0f;
	tileinfo.set(0, code, color, 0);
}
TILE_GET_INFO_MEMBER(dunhuang_state::get_tile_info2)
{
	uint16_t const code = m_videoram2[tile_index];
	uint8_t const color = m_colorram2[tile_index] & 0x0f;
	tileinfo.set(1, code, color, 0);
}

void dunhuang_state::video_start()
{
	m_tmap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dunhuang_state::get_tile_info)), TILEMAP_SCAN_ROWS, 0x08, 0x08, 0x40, 0x20);
	m_tmap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dunhuang_state::get_tile_info2)), TILEMAP_SCAN_ROWS, 0x08, 0x20, 0x40, 0x08);

	m_tmap[0]->set_transparent_pen(0);
	m_tmap[1]->set_transparent_pen(0);
}

uint32_t dunhuang_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
			if (layers_ctrl & 2)    m_tmap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 1)    m_tmap[0]->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x05:  // DIP switches: must hide fg
			if (layers_ctrl & 1)    m_tmap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			break;
		case 0x07:  // game,demo: fg over bg
		default:
			if (layers_ctrl & 1)    m_tmap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 2)    m_tmap[1]->draw(screen, bitmap, cliprect, 0, 0);
			break;
	}

	return 0;
}

// Tilemaps access

void dunhuang_state::pos_x_w(uint8_t data)
{
	m_pos_x = data & 0x3f;
	m_written[0] = 0;
	m_written[1] = 0;
}

void dunhuang_state::pos_y_w(uint8_t data)
{
	m_pos_y = data;
	m_written[0] = 0;
	m_written[1] = 0;
}

void dunhuang_state::tile_w(offs_t offset, uint8_t data)
{
	if (m_written[0]  & (1 << offset))
	{
		m_written[0]  = 0;
		m_pos_x++;
		if (m_pos_x == 0x40)
		{
			m_pos_x = 0;
			m_pos_y++;
		}
	}
	m_written[0]  |= 1 << offset;

	int const addr = (m_pos_x & 0x3f) + (m_pos_y & 0x1f) * 0x40;
	switch (offset)
	{
		case 0: m_videoram[addr] = (m_videoram[addr] & 0xff00) | data; break;
		case 1: m_videoram[addr] = (m_videoram[addr] & 0x00ff) | (data << 8); break;
		case 2: m_colorram[addr] = data; break;
	}
	m_tmap[0]->mark_tile_dirty(addr);
}

void dunhuang_state::tile2_w(offs_t offset, uint8_t data)
{
	if (BIT(m_written[1], offset))
	{
		m_written[1] = 0;
		m_pos_x++;
		if (m_pos_x == 0x40)
		{
			m_pos_x = 0;
			m_pos_y++;
		}
	}
	m_written[1] |= 1 << offset;

	int const addr = (m_pos_x & 0x3f) + (m_pos_y & 0x07) * 0x40;
	switch (offset)
	{
		case 0: m_videoram2[addr] = (m_videoram2[addr] & 0xff00) | data; break;
		case 1: m_videoram2[addr] = (m_videoram2[addr] & 0x00ff) | (data << 8); break;
		case 2: m_colorram2[addr] = data; break;
	}
	m_tmap[1]->mark_tile_dirty(addr);
}

// Clear a row of tiles (videoram)

void dunhuang_state::clear_y_w(uint8_t data)
{
	m_clear_y = data;
}
void dunhuang_state::horiz_clear_w(uint8_t data)
{
	LOGGFX("%s: horiz clear, y = %02x, data = %02d\n", machine().describe_context(), m_clear_y, data);
	for (int i = 0; i < 0x40; i++)
	{
		int const addr = m_clear_y * 0x40 + i;

		m_videoram[addr] = 0;
		m_colorram[addr] = 0;
		m_tmap[0]->mark_tile_dirty(addr);
	}
}

// Clear a column of tiles (videoram2)

void dunhuang_state::vert_clear_w(uint8_t data)
{
	LOGGFX("%s: vert clear, x = %02x, y = %02x, data = %02x\n", machine().describe_context(), m_pos_x, m_pos_y, data);
	for (int i = 0; i < 0x08; i++)
	{
		int const addr = (m_pos_x & 0x3f) + (i & 0x07) * 0x40;

		m_videoram2[addr] = 1;
		m_colorram2[addr] = 0;
		m_tmap[1]->mark_tile_dirty(addr);
	}
}


// Draw a block of tiles.
//
// The tiles codes are read from the graphics ROMs too!
//

void dunhuang_state::block_dest_w(uint8_t data)
{
	m_block_dest = data;
}

void dunhuang_state::block_x_w(uint8_t data)
{
	m_block_x = data;
}

void dunhuang_state::block_y_w(uint8_t data)
{
	m_block_y = data;
}

void dunhuang_state::block_w_w(uint8_t data)
{
	m_block_w = data;
}

void dunhuang_state::block_c_w(uint8_t data)
{
	m_block_c = data;
}

void dunhuang_state::block_addr_lo_w(uint8_t data)
{
	m_block_addr_lo = data;
}

void dunhuang_state::block_addr_hi_w(uint8_t data)
{
	m_block_addr_hi = data;
}


void dunhuang_state::block_h_w(uint8_t data)
{
	LOGGFX("%s: block dst %x, src %x, xy %x %x, wh %x %x, clr %x\n",
			machine().describe_context(), m_block_dest, (m_block_addr_hi << 8) + m_block_addr_lo, m_block_x, m_block_y, m_block_w + 1, m_block_h + 1, m_block_c);

	m_block_h = data;

	uint8_t *tile_addr = &m_gfx2[((m_block_addr_hi << 8) + m_block_addr_lo) * 4];

	switch (m_block_dest)
	{
		case 0x04:  // write to videoram
			for (int j = 0; j <= m_block_h; j++)
			{
				for (int i = 0; i <= m_block_w; i++)
				{
					int const addr = ((m_block_x + i)& 0x3f) + ((m_block_y + j) & 0x1f) * 0x40;

					m_videoram[addr] = (tile_addr[1] << 8) | tile_addr[0];
					m_colorram[addr] = m_block_c;
					m_tmap[0]->mark_tile_dirty(addr);
					tile_addr += 4;
				}
			}
			break;

		case 0x08:  // write to videoram2
			for (int j = 0; j <= m_block_h; j++)
			{
				for (int i = 0; i <= m_block_w; i++)
				{
					int const addr = ((m_block_x + i)& 0x3f) + ((m_block_y + j) & 0x7) * 0x40;

					m_videoram2[addr] = (tile_addr[1] << 8) | tile_addr[0];
					m_colorram2[addr] = m_block_c;
					m_tmap[1]->mark_tile_dirty(addr);
					tile_addr += 4;
				}
			}
			break;

		default:
			popmessage("%s: block dst=%x", machine().describe_context(), m_block_dest);
	}
}

// Layers control (not understood)

void dunhuang_state::layers_w(uint8_t data)
{
	//popmessage("layers %02x", data);
	m_layers = data;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

void dunhuang_state::program_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_mainbank);
}

// Inputs

void dunhuang_state::input_w(uint8_t data)
{
	m_input = data;
}

uint8_t dunhuang_state::service_r()
{
	return m_service->read() | 0x80; // bit 7 low -> tiles block transferrer busy
}

uint8_t dunhuang_state::dsw_r()
{
	if ((m_input & 0xe0) != 0x00)
		logerror("%s: warning, unknown dsw bits read, input = %02x\n", machine().describe_context(), m_input);

	uint8_t res = 0xff;

	for (int i = 0; i < 5; i++)
	{
		if (!BIT(m_input, i))
			res &= m_dsw[i]->read();
	}

	return res;
}

uint8_t dunhuang_state::input_r()
{
	if ((m_input & 0xe0) != 0x00)
		logerror("%s: warning, unknown input bits read, input = %02x\n", machine().describe_context(), m_input);

	uint8_t res = 0x3f;

	for (int i = 0; i < 5; i++)
	{
		if (!BIT(m_input, i))
			res &= m_inputs[i]->read();
	}

	return 0xc0 | res;
}

void dunhuang_state::rombank_w(uint8_t data)
{
	// ?                BIT(data, 0)
	// ?                BIT(data, 1)

	m_mainbank->set_entry(((data >> 2) & 0x07));

	// COIN OUT:        BIT(data, 5)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	m_hopper->motor_w(BIT(data, 7));
}


void dunhuang_state::io_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(dunhuang_state::pos_x_w));
	map(0x0001, 0x0001).w(FUNC(dunhuang_state::pos_y_w));
	map(0x0002, 0x0004).w(FUNC(dunhuang_state::tile_w));
	map(0x0005, 0x0007).w(FUNC(dunhuang_state::tile2_w));

	map(0x0008, 0x0008).w(FUNC(dunhuang_state::vert_clear_w));

	map(0x000c, 0x000c).r("watchdog", FUNC(watchdog_timer_device::reset_r));

	map(0x000f, 0x000f).w(FUNC(dunhuang_state::block_addr_lo_w));
	map(0x0010, 0x0010).w(FUNC(dunhuang_state::block_addr_hi_w));
//  map(0x0011, 0x0011) ?
	map(0x0012, 0x0012).w(FUNC(dunhuang_state::block_c_w));
	map(0x0015, 0x0015).w(FUNC(dunhuang_state::block_x_w));
	map(0x0016, 0x0016).w(FUNC(dunhuang_state::block_y_w));
	map(0x0017, 0x0017).w(FUNC(dunhuang_state::block_w_w));
	map(0x0018, 0x0018).w(FUNC(dunhuang_state::block_h_w));

	map(0x0019, 0x0019).w(FUNC(dunhuang_state::clear_y_w));
	map(0x001a, 0x001a).w(FUNC(dunhuang_state::horiz_clear_w));

	map(0x001b, 0x001b).w(FUNC(dunhuang_state::block_dest_w));

	map(0x0081, 0x0081).w("ymsnd", FUNC(ym2413_device::address_w));
	map(0x0089, 0x0089).w("ymsnd", FUNC(ym2413_device::data_w));

	map(0x0082, 0x0082).w("oki", FUNC(okim6295_device::write));

	map(0x0083, 0x0083).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x008b, 0x008b).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0093, 0x0093).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x0084, 0x0084).r(FUNC(dunhuang_state::service_r));
	map(0x0085, 0x0085).r(FUNC(dunhuang_state::input_r));

	map(0x0086, 0x0086).w(FUNC(dunhuang_state::rombank_w));
	map(0x0087, 0x0087).w(FUNC(dunhuang_state::layers_w));

	map(0x0088, 0x0088).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x0090, 0x0090).w("ay8910", FUNC(ay8910_device::data_w));
	map(0x0098, 0x0098).w("ay8910", FUNC(ay8910_device::address_w));
}

void dunhuang_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( dunhuang )
	PORT_START("DSW1") // IN0 - DSW1
	PORT_DIPNAME( 0x0f, 0x0f, "Main Game Payout Rate" )       PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPSETTING(    0x01, "80%" )
	PORT_DIPSETTING(    0x02, "81%" )
	PORT_DIPSETTING(    0x03, "83%" )
	PORT_DIPSETTING(    0x04, "84%" )
	PORT_DIPSETTING(    0x05, "86%" )
	PORT_DIPSETTING(    0x06, "87%" )
	PORT_DIPSETTING(    0x07, "89%" )
	PORT_DIPSETTING(    0x08, "90%" )
	PORT_DIPSETTING(    0x09, "92%" )
	PORT_DIPSETTING(    0x0a, "93%" )
	PORT_DIPSETTING(    0x0b, "94%" )
	PORT_DIPSETTING(    0x0c, "95%" )
	PORT_DIPSETTING(    0x0d, "96%" )
	PORT_DIPSETTING(    0x0e, "97%" )
	PORT_DIPSETTING(    0x0f, "98%" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )                   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )         // A
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )          // D
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )         // B
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )        // C
	PORT_DIPNAME( 0x40, 0x40, "Credit Mode" )                 PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Key-In" )
	PORT_DIPSETTING(    0x00, "Coin Acceptor" )
	PORT_DIPNAME( 0x80, 0x80, "Payout Mode" )                 PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Key-Out" )
	PORT_DIPSETTING(    0x00, "Return Coins" )

	PORT_START("DSW2") // IN1 - DSW2
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR(Coinage) )              PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR(1C_1C) )
	PORT_DIPSETTING(    0x04, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x02, DEF_STR(1C_5C) )
	PORT_DIPSETTING(    0x00, DEF_STR(1C_20C) )
	PORT_DIPNAME( 0x18, 0x18, "Key-In Rate" )                 PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x20, 0x20, "Key-Out Rate" )                PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Credits" )                 PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x80, "1000" )
	PORT_DIPSETTING(    0xc0, "3000" )

	PORT_START("DSW3") // IN2 - DSW3
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" )                 PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Maximum Bet" )                 PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x30, 0x30, "Renso Bonus Time" )            PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0xc0, 0xc0, "DonDen Times" )                PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_START("DSW4") // IN3 - DSW4
	PORT_DIPNAME( 0x07, 0x07, "Credits Limit" )               PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x07, "2k" )
	PORT_DIPSETTING(    0x06, "3k" )
	PORT_DIPSETTING(    0x05, "5k" )
	PORT_DIPSETTING(    0x04, "10k" )
	PORT_DIPSETTING(    0x03, "20k" )
	PORT_DIPSETTING(    0x02, "30k" )
	PORT_DIPSETTING(    0x01, "50k" )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPNAME( 0x38, 0x38, "Service-In Limit" )            PORT_DIPLOCATION("SW4:4,5,6")
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x08, "30k" )
	PORT_DIPSETTING(    0x10, "40k" )
	PORT_DIPSETTING(    0x18, "50k" )
	PORT_DIPSETTING(    0x20, "70k" )
	PORT_DIPSETTING(    0x28, "100k" )
	PORT_DIPSETTING(    0x30, "200k" )
	PORT_DIPSETTING(    0x38, "990k" )
	PORT_DIPNAME( 0x40, 0x40, "Hu Type" )                     PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, "Topple" )
	PORT_DIPSETTING(    0x00, "Non-Topple" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up Game" )              PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x80, DEF_STR(On) )

	PORT_START("DSW5") // IN4 - DSW5
	PORT_DIPNAME( 0x03, 0x03, "Double Up Game Payout Rate" )  PORT_DIPLOCATION("SW5:1,2")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x03, "80%" )
	PORT_DIPNAME( 0x04, 0x04, "Chinese Word" )                PORT_DIPLOCATION("SW5:3")
	PORT_DIPSETTING(    0x04, "3..6" )
	PORT_DIPSETTING(    0x00, "5..10" )
	PORT_DIPNAME( 0x08, 0x08, "Big Odds Times" )              PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Hu Test Times" )               PORT_DIPLOCATION("SW5:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x20, "8" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR(Demo_Sounds) )          PORT_DIPLOCATION("SW5:7")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x40, DEF_STR(On) )
	PORT_DIPNAME( 0x80, 0x80, "Computer Strength" )           PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(    0x80, "Strong" )
	PORT_DIPSETTING(    0x00, "Weak" )

	PORT_START("SERVICE") // IN5 - SERVICE
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_MEMORY_RESET ) // during boot
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE  ) // test (in game: dips, during boot: service mode)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT )                PORT_CONDITION("DSW1", 0x80, EQUALS, 0x80)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )                PORT_CONDITION("DSW1", 0x80, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN )                 PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )        PORT_IMPULSE(2) PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00) // "coin jam" otherwise
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) // 0 = tiles block transferrer busy

	PORT_INCLUDE(mahjong_matrix_1p_bet_wup)
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

static GFXDECODE_START( gfx_dunhuang )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x32, 0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

void dunhuang_state::machine_start()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base(), 0x8000);

	save_item(NAME(m_written));
	save_item(NAME(m_written[1]));
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
	save_item(NAME(m_input));
}

void dunhuang_state::machine_reset()
{
	m_written[0] = 0;
	m_written[1] = 0;
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
	m_input = 0;
}


void dunhuang_state::dunhuang(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dunhuang_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &dunhuang_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(dunhuang_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(5));

	HOPPER(config, m_hopper, attotime::from_msec(50)); // period not measured

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0+8, 512-8-1, 0+16, 256-16-1);
	m_screen->set_screen_update(FUNC(dunhuang_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dunhuang);
	PALETTE(config, m_palette).set_entries(0x100);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", m_palette)); // HMC HM86171 VGA 256 colour RAMDAC
	ramdac.set_addrmap(0, &dunhuang_state::ramdac_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.80);

	ay8910_device &ay8910(AY8910(config, "ay8910", 12_MHz_XTAL / 8));
	ay8910.port_b_read_callback().set(FUNC(dunhuang_state::dsw_r));
	ay8910.port_a_write_callback().set(FUNC(dunhuang_state::input_w));
	ay8910.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, "oki", 12_MHz_XTAL / 8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.80);
}


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( dunhuang )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "rom1.u9", 0x00000, 0x40000, CRC(843a0117) SHA1(26a838cb3552ea6a9ec55940fcbf83b06c068743) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "rom3.u4", 0x00000, 0x80000, CRC(1ff5d35e) SHA1(b808eb4f81be8fc77a58dadd661a9cc2b376a509) )
	ROM_LOAD( "rom2.u5", 0x80000, 0x40000, CRC(384fa1d3) SHA1(f329db17aacacf1768ebd6ca2cc612503db93fac) )

	ROM_REGION( 0xc0000, "gfx2", 0 )
	ROM_LOAD( "rom4.u3", 0x00000, 0x40000, CRC(7db45227) SHA1(2a12a2b8a1e58946ce3e7c770b3ca4803c3c3ccd) )
	ROM_LOAD( "rom5.u2", 0x40000, 0x80000, CRC(d609880e) SHA1(3d69800e959e8f24ef950fea4312610c4407f6ba) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rom6.u1", 0x00000, 0x20000, CRC(31cfdc29) SHA1(725249eae9227eadf05418b799e0da0254bb2f51) )
ROM_END

} // anonymous namespace


GAME( 1995, dunhuang, 0, dunhuang, dunhuang, dunhuang_state, empty_init, ROT0, "Spirit", "Mahjong Dunhuang", MACHINE_SUPPORTS_SAVE )
