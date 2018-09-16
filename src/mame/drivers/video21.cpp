// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************

2018-09-15

Video 21 blackjack game. Thanks to hap who figured out the inputs and the name of the game.

VIDEO-GAMES - LICH/GERMANY 1017a

NEC D8080AFC (i8080), unknown xtal, bank of 7 dips. 10x 4-bit proms, type F93453 (=82S137)
Video Ram = 7 x 2102 (bit 7 omitted). Main Ram = 2x MCM145101 (=M5101L).
The game has sound (there's a LM380N visible), looks like there's a bunch of TTL chips
involved, and a 555.

To Do:
- Sound
- CPU clock
- unknown status bits? eg. hopper
- color overlay as seen on flyer upright cabinet

When booted, press W to get it going.


*******************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "screen.h"
#include "emupal.h"


class video21_state : public driver_device
{
public:
	video21_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this,"maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void video21(machine_config &config);

private:

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mem_map(address_map &map);
	void io_map(address_map &map);
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};


uint32_t video21_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;

	for (y = 0; y < 28; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = 0; x < 32; x++)
			{
				chr = m_p_videoram[x+ma] & 0x7f;
				gfx = m_p_chargen[(chr<<3) | ra ];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=32;
	}
	return 0;
}

void video21_state::mem_map(address_map &map) {
	map(0x0000,0x0fff).rom().mirror(0x3000);
	map(0xe000,0xe3ff).ram().share("videoram");
	map(0xff00,0xffff).ram();
}

void video21_state::io_map(address_map &map) {
	map(0x02,0x02).nopw();  // lots of unknown writes, might be some kind of dac
	map(0x04,0x04);  //.w unknown write
	map(0x08,0x08);  //.w unknown write
	map(0x41,0x41).portr("IN41");
	map(0x42,0x42).portr("IN42");
	map(0x44,0x44).portr("IN44");
}


static INPUT_PORTS_START( video21 )
	PORT_START("IN41") // dips and tilt
	PORT_DIPNAME( 0x01, 0x01, "41b0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "41b1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "41b2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "41b3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "41b4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "41b5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "41b6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("IN42")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Stay")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Payout")
	PORT_DIPNAME( 0x40, 0x40, "42b6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )

	PORT_START("IN44")
	PORT_DIPNAME( 0x01, 0x01, "44b0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "44b1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "44b2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_DIPNAME( 0x20, 0x20, "44b5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "44b6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "44b7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout video21_charlayout =
{
	8, 8,                   // 8 x 8 characters
	128,                    // 128 characters, but only the first 76 look useful
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_video21 )
	GFXDECODE_ENTRY( "chargen", 0x0000, video21_charlayout, 0, 1 )
GFXDECODE_END

MACHINE_CONFIG_START( video21_state::video21 )
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8080A, 20.79_MHz_XTAL / 16) // crystal confirmed but divisor unknown
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::white())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(video21_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_video21)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
MACHINE_CONFIG_END



ROM_START( video21 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "lich_prg.02", 0x0000, 0x0400, CRC(05585e39) SHA1(7eefb5d63b4499a303ecdcad6af5df9fe9c89205) )
	ROM_LOAD_NIB_LOW ( "lich_prg.03", 0x0000, 0x0400, CRC(b9134e96) SHA1(e7a8ff71f735add608d3c9dcc287ca37414debcb) )
	ROM_LOAD_NIB_HIGH( "lich_prg.04", 0x0400, 0x0400, CRC(8a6aa143) SHA1(16973106a95b17d8c4712db8aa7ef564751ae6d4) )
	ROM_LOAD_NIB_LOW ( "lich_prg.05", 0x0400, 0x0400, CRC(98c07d4d) SHA1(d3126b5484c67ecc7c44ddc25b48ff72ed8a734f) )
	ROM_LOAD_NIB_HIGH( "lich_prg.76", 0x0800, 0x0400, CRC(737c27f5) SHA1(55c7eb29b979d35633e5fb2c1c1ac3117901a0f0) )
	ROM_LOAD_NIB_LOW ( "lich_prg.78", 0x0800, 0x0400, CRC(c1081a2f) SHA1(24dc1d9afa4635c3114369c338903343a8b81d1a) )
	ROM_LOAD_NIB_HIGH( "lich_prg.77", 0x0c00, 0x0400, CRC(3a725b98) SHA1(efa3802025f1f99b45e56abd85dc3d1860d4734d) )
	ROM_LOAD_NIB_LOW ( "lich_prg.79", 0x0c00, 0x0400, CRC(044d1bfd) SHA1(5a57c1ab7eb7dd7ed05852e128b704c3b37a87b8) )

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD_NIB_HIGH( "lich_gfx.29", 0x0000, 0x0400, CRC(2b70870d) SHA1(1f50a6976e1634020c78f10c1259e38f5e010a86) )
	ROM_LOAD_NIB_LOW ( "lich_gfx.43", 0x0000, 0x0400, CRC(0ecb0aab) SHA1(7f3f1b93a5d38828ae3e97e5f8ef1a6a96dc798b) )
ROM_END

GAME(1980, video21, 0, video21, video21, video21_state, empty_init, ROT0, "Video Games GmbH", "Video 21", MACHINE_NO_SOUND)

