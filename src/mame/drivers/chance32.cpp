// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese, Roberto Fresca
/*********************************************************

  Chance 32

  PAL System Co, Ltd.
  Osaka, Japan.

  Driver by David Haywood, Angelo Salese & Roberto Fresca.


  1x HD46505SP / HD6845SP
  1x Z84C0008PEC

  XTAL: 12.000 Mhz


*********************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "chance32.lh"


class chance32_state : public driver_device
{
public:
	chance32_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void chance32(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	DECLARE_WRITE8_MEMBER(chance32_fgram_w)
	{
		m_fgram[offset] = data;
		m_fg_tilemap->mark_tile_dirty(offset / 2);
	}

	DECLARE_WRITE8_MEMBER(chance32_bgram_w)
	{
		m_bgram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset / 2);
	}

	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(muxout_w);
	DECLARE_READ8_MEMBER(mux_r);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update_chance32(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void chance32_map(address_map &map);
	void chance32_portmap(address_map &map);

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_bgram;

	uint8_t mux_data;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<13> m_lamps;
};


TILE_GET_INFO_MEMBER(chance32_state::get_fg_tile_info)
{
	int code = (m_fgram[tile_index * 2 + 1] << 8) | m_fgram[tile_index * 2];
	int flip = (~code >> 12)&1;
	SET_TILE_INFO_MEMBER(1,
			code & 0x0fff,
			code >> 13,
			TILE_FLIPYX(flip<<1)|flip);
}

TILE_GET_INFO_MEMBER(chance32_state::get_bg_tile_info)
{
	int code = (m_bgram[tile_index * 2 +1] << 8) | m_bgram[tile_index * 2];
	int flip = (~code >> 12)&1;
	SET_TILE_INFO_MEMBER(0,
			code & 0x0fff,
			code >> 13,
			TILE_FLIPYX(flip<<1|flip));
}


void chance32_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(chance32_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 35, 29);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(chance32_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 35, 29);

	m_fg_tilemap->set_flip(TILE_FLIPX|TILE_FLIPY);
	m_bg_tilemap->set_flip(TILE_FLIPX|TILE_FLIPY);
}


uint32_t chance32_state::screen_update_chance32(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


WRITE8_MEMBER(chance32_state::mux_w)
{
	mux_data = data;
}

READ8_MEMBER(chance32_state::mux_r)
{
	uint8_t res,i;
	const char *const muxnames[4] = { "IN0", "IN1", "IN2", "IN3" };
	res = 0;

	for(i=0;i<4;i++)
	{
		if(mux_data & 1 << i)
			res |= ioport(muxnames[i])->read();
	}

	return res;
}


WRITE8_MEMBER(chance32_state::muxout_w)
{
/* Muxed Lamps

  There are 2 groups of 7 output lines muxed in port 60h
  The first bit is the group/mux selector.

  - bits -
  7654 3210
  ---- ---x   Mux selector.
  ---- --x-   Small / Big lamps.
  ---- -x--   Big / Small lamps.
  ---- x---   Hold 5 lamp.
  ---x ----   Hold 4 lamp.
  --x- ----   Hold 3 lamp.
  -x-- ----   Hold 2 lamp.
  x--- ----   Hold 1 lamp.

  (alt state)

  - bits -
  7654 3210
  ---- ---x   Mux selector.
  ---- --x-   unknown...
  ---- -x--   Fever lamp
  ---- x---   Cancel lamp.
  ---x ----   D-Up / Take lamps.
  --x- ----   Take / D-Up lamps.
  -x-- ----   Deal lamp.
  x--- ----   Bet lamp.

*/
	if (data & 1)   // bit 0 is the mux selector.
	{
		m_lamps[0] = BIT(data, 1);  /* Lamp 0 - Small / Big */
		m_lamps[1] = BIT(data, 2);  /* Lamp 1 - Big / Small */
		m_lamps[2] = BIT(data, 3);  /* Lamp 2 - Hold 5 */
		m_lamps[3] = BIT(data, 4);  /* Lamp 3 - Hold 4 */
		m_lamps[4] = BIT(data, 5);  /* Lamp 4 - Hold 3 */
		m_lamps[5] = BIT(data, 6);  /* Lamp 5 - Hold 2 */
		m_lamps[6] = BIT(data, 7);  /* Lamp 6 - Hold 1 */

		logerror("Lamps A: %02x\n", data);
	}

	else
	{
		// bit 1 is unknown...
		m_lamps[7] = BIT(data, 2);  /* Lamp 7 - Fever! */
		m_lamps[8] = BIT(data, 3);  /* Lamp 8 - Cancel */
		m_lamps[9] = BIT(data, 4);  /* Lamp 9 - D-Up / Take */
		m_lamps[10] = BIT(data, 5); /* Lamp 10 - Take / D-Up */
		m_lamps[11] = BIT(data, 6); /* Lamp 11 - Deal */
		m_lamps[12] = BIT(data, 7); /* Lamp 12 - Bet */

		logerror("Lamps B: %02x\n", data);
	}
}


void chance32_state::chance32_map(address_map &map)
{
	map(0x0000, 0xcfff).rom();
	map(0xd800, 0xdfff).ram();
	map(0xe000, 0xefff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf7ff).ram().w(FUNC(chance32_state::chance32_fgram_w)).share("fgram");
	map(0xf800, 0xffff).ram().w(FUNC(chance32_state::chance32_bgram_w)).share("bgram");
}

void chance32_state::chance32_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).nopw();        // writing bit3 constantly... watchdog?
	map(0x13, 0x13).w(FUNC(chance32_state::mux_w));
	map(0x20, 0x20).portr("DSW0");
	map(0x21, 0x21).portr("DSW1");
	map(0x22, 0x22).portr("DSW2");
	map(0x23, 0x23).portr("DSW3");
	map(0x24, 0x24).portr("DSW4");
	map(0x25, 0x25).r(FUNC(chance32_state::mux_r));
	map(0x26, 0x26).portr("UNK"); // vblank (other bits are checked for different reasons)
	map(0x30, 0x30).w("crtc", FUNC(mc6845_device::address_w));
	map(0x31, 0x31).w("crtc", FUNC(mc6845_device::register_w));
	map(0x50, 0x50).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x60, 0x60).w(FUNC(chance32_state::muxout_w));
}


static INPUT_PORTS_START( chance32 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "DSW0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Auto Max Bet" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x80, "30" )
	PORT_DIPSETTING(    0xc0, "50" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Hold" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Double-Up Type" )
	PORT_DIPSETTING(    0x00, "Holds" )
	PORT_DIPSETTING(    0x80, "Big/Small" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Remote" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x03, "25" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x07, "100" )
	PORT_DIPNAME( 0x38, 0x00, "A-B Coinage Multiplier" )
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x08, "x2" )
	PORT_DIPSETTING(    0x10, "x4" )
	PORT_DIPSETTING(    0x18, "x5" )
	PORT_DIPSETTING(    0x20, "x6" )
	PORT_DIPSETTING(    0x28, "x10" )
	PORT_DIPSETTING(    0x30, "x25" )
	PORT_DIPSETTING(    0x38, "x50" )
	PORT_DIPNAME( 0x40, 0x00, "DSW2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DSW3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, "DSW4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Bet Limit" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x06, "30000" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("UNK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")  /* Otherwise is a 'Freeze' DIP switch */
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_BET )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )   PORT_NAME("Small / DIP Test (In Book Mode)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) // payout (hopper jam)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Coin A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Coin B")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CODE(KEYCODE_8) PORT_NAME("Flip Screen 1")  /* unknown purpose */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CODE(KEYCODE_9) PORT_NAME("Flip Screen 2")  /* unknown purpose */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN)
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	16,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3,4,5,6,7 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8, 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 7*128, 6*128, 5*128, 4*128, 3*128, 2*128, 1*128, 0*128 },
	128*8
};

static GFXDECODE_START( gfx_chance32 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 8 )
GFXDECODE_END


void chance32_state::machine_start()
{
	m_lamps.resolve();
}

void chance32_state::machine_reset()
{
}


void chance32_state::chance32(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &chance32_state::chance32_map);
	m_maincpu->set_addrmap(AS_IO, &chance32_state::chance32_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(chance32_state::irq0_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(52.786);
//  screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*16, 32*8);
	screen.set_visarea(0, 35*16-1, 0, 29*8-1);
	screen.set_screen_update(FUNC(chance32_state::screen_update_chance32));

	hd6845s_device &crtc(HD6845S(config, "crtc", 12000000/16));   /* 52.786 Hz (similar to Major Poker) */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(16);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_chance32);
	PALETTE(config, "palette").set_format(palette_device::xGRB_555, 0x800);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* clock at 1050 kHz match the 8000 Hz samples stored inside the ROM */
	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}


ROM_START( chance32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.u52", 0x00000, 0x10000, CRC(331048b2) SHA1(deb4da570b3efe6e15deefb6351f925b642b4614)  )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD16_BYTE( "4.u64", 0x00001, 0x20000, CRC(da80d9bd) SHA1(3b5235ab59fd55f0ec5584b3cf1aa5c8f36c76f6) )
	ROM_LOAD16_BYTE( "5.u65", 0x00000, 0x20000, CRC(7528773b) SHA1(95c8e55cdec2c5c1dcdcc5a7edc6e590e3829f92) )
	ROM_LOAD16_BYTE( "6.u66", 0x40001, 0x20000, CRC(cee2ffb0) SHA1(527c2072d39484317b0320afd975df1bbe244a01) )
	ROM_LOAD16_BYTE( "7.u67", 0x40000, 0x20000, CRC(42dc4b69) SHA1(44c8f902db4c7ac235d5ea15d1b509f98663690a) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD16_BYTE( "1.u71", 0x00001, 0x20000, CRC(f8e85873) SHA1(6ad24f7fcbc62a03180e168d70239df1ce662f0d) )
	ROM_LOAD16_BYTE( "2.u72", 0x00000, 0x20000, CRC(860b534d) SHA1(44649ea93acdf173356bfcd7e81916253b52c378) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "8.u21", 0x00000, 0x40000, CRC(161b35dd) SHA1(d20a75a4c4ed9cd9cfc12faee921122274840f06) )

	ROM_REGION( 0x40000, "gals", 0 ) // no idea if these are any good
	ROM_LOAD( "gal20v8a.u53.jed.bin", 0x0000, 0x157, CRC(9f680800) SHA1(2fa41ead85136e851d465432a7b9d3ec848c7a22) )
	ROM_LOAD( "gal20v8a.u54.jed.bin", 0x0000, 0x157, CRC(9f680800) SHA1(2fa41ead85136e851d465432a7b9d3ec848c7a22) )
	ROM_LOAD( "gal20v8a.u55.jed.bin", 0x0000, 0x157, CRC(9f680800) SHA1(2fa41ead85136e851d465432a7b9d3ec848c7a22) )
	ROM_LOAD( "gal20v8a.u56.jed.bin", 0x0000, 0x157, CRC(6bab01ad) SHA1(c69e4be41a989a52788af8062f48bbe26bc3dab8) )
	ROM_LOAD( "gal20v8a.u57.jed.bin", 0x0000, 0x157, CRC(787c4159) SHA1(f4a869b317c6be1024f1ca21bcc4af478c8227c8) )
	ROM_LOAD( "gal20v8a.u58.jed.bin", 0x0000, 0x157, CRC(7b16053b) SHA1(cdb289d4f27c7a1a918393943bb8db9712e2f52e) )

	ROM_LOAD( "gal16v8a.u47.jed.bin", 0x0000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8a.u48.jed.bin", 0x0000, 0x117, CRC(5f1360ef) SHA1(56e4ee0dbae5602d810b2f7c744a71eb1a1e08a8) )

	ROM_LOAD( "gal16v8a.u32.jed.bin", 0x0000, 0x117, CRC(c0784cd1) SHA1(0ae2ce482d379e29c2a9f130fc0d9ed928faef98) )

	ROM_LOAD( "gal16v8a.u24.jed.bin", 0x0000, 0x117, NO_DUMP )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROT   COMPANY                FULLNAME             FLAGS  LAYOUT */
GAMEL( 19??, chance32, 0,      chance32, chance32, chance32_state, empty_init, ROT0, "PAL System Co, Ltd.", "Chance Thirty Two", 0,     layout_chance32 )
