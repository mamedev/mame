// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Diver Boy
 (c)1992 Device Electronics


 ----

 Here's the info about this dump:

 Name:            DiverBoy
 Manufacturer:    Unknown
 Year:            Unknown
 Date Dumped:     17-07-2002 (DD-MM-YYYY)

 CPU:             68000, Z80
 SOUND:           OKIM6295
 GFX:             Unknown

 About the game:

 The worst game I have :) Enjoy it so much as me :D

 ----

 Stephh's notes :

  - COIN3 gives ("Coinage" * 2) coins/credits :

     COIN1/2    COIN3
      4C_1C     2C_1C
      3C_1C    special   (see below)
      2C_1C     1C_1C
      1C_1C     1C_2C
      1C_2C     1C_4C
      1C_3C     1C_6C
      1C_4C     1C_8C
      1C_6C     1C_12C

    when "Coinage" set to 3C_1C, pressing COIN3 has this effect :

      * 1st coin : nothing
      * 2nd coin : adds 1 credit
      * 3rd coin : adds 1 credit
      * 4th coin : nothing
      * 5th coin : adds 1 credit
      * 6th coin : adds 1 credit ...

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class diverboy_state : public driver_device
{
public:
	diverboy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void diverboy(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void soundcmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void okibank_w(uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_diverboy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect );
	void diverboy_map(address_map &map) ATTR_COLD;
	void snd_map(address_map &map) ATTR_COLD;
};


void diverboy_state::video_start()
{
}

void diverboy_state::draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint16_t *source = m_spriteram;
	uint16_t *finish = source + (m_spriteram.bytes() / 2);

	while (source < finish)
	{
		int16_t xpos, ypos, number, colr, bank, flash;

		ypos = source[4];
		xpos = source[0];
		colr = (source[1] & 0x00f0) >> 4;
		number = source[3];
		flash = source[1] & 0x1000;

		colr |= ((source[1] & 0x000c) << 2);

		ypos = 0x100 - ypos;

		bank = (source[1] & 0x0002) >> 1;

		if (!flash || (m_screen->frame_number() & 1))
		{
			m_gfxdecode->gfx(bank)->transpen(bitmap,cliprect,
					number,
					colr,
					0,0,
					xpos,ypos,
					(source[1] & 0x0008) ? -1 : 0);
		}

		source += 8;
	}
}

uint32_t diverboy_state::screen_update_diverboy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  bitmap.fill(m_palette->black_pen(), cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void diverboy_state::soundcmd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_soundlatch->write(data & 0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

void diverboy_state::okibank_w(uint8_t data)
{
	/* bit 2 might be reset */
//  popmessage("%02x",data);
	m_oki->set_rom_bank(data & 3);
}



void diverboy_state::diverboy_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x04ffff).ram();
	map(0x080000, 0x083fff).ram().share("spriteram");
	map(0x100000, 0x100001).w(FUNC(diverboy_state::soundcmd_w));
	map(0x140000, 0x1407ff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x180001).portr("P1_P2");
	map(0x180002, 0x180003).portr("DSW");
	map(0x180008, 0x180009).portr("COINS");
//  map(0x18000a, 0x18000b).nopr();
//  map(0x18000c, 0x18000d).nopw();
	map(0x320000, 0x3207ff).nopw(); /* ?? */
	map(0x322000, 0x3227ff).nopw(); /* ?? */
//  map(0x340000, 0x340001).nopw();
//  map(0x340002, 0x340003).nopw();
}

void diverboy_state::snd_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(diverboy_state::okibank_w));
	map(0x9800, 0x9800).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}



static INPUT_PORTS_START( diverboy )
	PORT_START("P1_P2") // 0x180000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)   // unused ?
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) // unused ?
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)       // "Dive"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)       // unknown effect
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)   // unused ?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) // unused ?
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)       // "Dive"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)       // unknown effect
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")   // 0x180002.w
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Display Copyright" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("COINS") // 0x180008.w
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )      // read notes
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // must be 00 - check code at 0x001680
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static GFXDECODE_START( gfx_diverboy )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_packed_lsb, 0, 4*16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_lsb, 0, 4*16 )
GFXDECODE_END


void diverboy_state::machine_start()
{
}

void diverboy_state::diverboy(machine_config &config)
{
	M68000(config, m_maincpu, 12000000); /* guess */
	m_maincpu->set_addrmap(AS_PROGRAM, &diverboy_state::diverboy_map);
	m_maincpu->set_vblank_int("screen", FUNC(diverboy_state::irq6_line_hold));

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &diverboy_state::snd_map);


	GFXDECODE(config, m_gfxdecode, m_palette, gfx_diverboy);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8+4, 40*8+1, 2*8, 32*8-1);
	m_screen->set_screen_update(FUNC(diverboy_state::screen_update_diverboy));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x400);


	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	OKIM6295(config, m_oki, 1320000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // clock frequency & pin 7 not verified
}

/*

both program roms contain the following string (at the same location)

This Game is programmed by the freelance group GAMART.
ADRESS: C\SAnt Ramon,11 08130-STA PERPETUA DE MOGODA - BARCELONA (SPAIN)
Telephone (93) 560 27 32
Fax (93) 574 18 34

*/

ROM_START( diverboy )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "db_01.bin", 0x00000, 0x20000, CRC(6aa11366) SHA1(714c8a4a64c18632825a734a76a2d1b031106d76) )
	ROM_LOAD16_BYTE( "db_02.bin", 0x00001, 0x20000, CRC(45f8a673) SHA1(4eea1374cafacb4a2e0b623fcb802deb5fca1b3a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* z80 */
	ROM_LOAD( "db_05.bin", 0x00000, 0x8000, CRC(ffeb49ec) SHA1(911b13897ff4ace3940bfff4ab88584a93796c24) ) /* this part is empty */
	ROM_CONTINUE( 0x0000, 0x8000 ) /* this part contains the code */

	ROM_REGION( 0x100000, "gfx1", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "db_08.bin", 0x000000, 0x80000, CRC(7bb96220) SHA1(671b3f218106e594b13ae5f2e680cf2e2cfc5501) )
	ROM_LOAD16_BYTE( "db_09.bin", 0x000001, 0x80000, CRC(12b15476) SHA1(400a5b846f70567de137e0b95586dd9cfc27becb) )

	ROM_REGION( 0x180000, "gfx2", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "db_07.bin", 0x000000, 0x20000, CRC(18485741) SHA1(a8edceaf34a98f2aa2bfada9d6e06fb82639a4e0) )
	ROM_LOAD16_BYTE( "db_10.bin", 0x000001, 0x20000, CRC(c381d1cc) SHA1(88b97d8893c500951cfe8e7e7f0b547b36bbe2c0) )
	ROM_LOAD16_BYTE( "db_06.bin", 0x040000, 0x20000, CRC(21b4e352) SHA1(a553de67e5dc751ea81ec4739724e0e46e8c5fab) )
	ROM_LOAD16_BYTE( "db_11.bin", 0x040001, 0x20000, CRC(41d29c81) SHA1(448fd5c1b16159d03436b8bd71ffe871c8daf7fa) )

	ROM_REGION( 0x100000, "oki", 0 ) /* Sound */
	ROM_LOAD( "db_03.bin", 0x00000, 0x20000, CRC(50457505) SHA1(faf1c055ec56d2ed7f5e6993cc04d3317bf1c3cc) )
	ROM_CONTINUE(          0x40000, 0x20000 )
	ROM_CONTINUE(          0x80000, 0x20000 )
	ROM_CONTINUE(          0xc0000, 0x20000 )
	ROM_LOAD( "db_04.bin", 0x20000, 0x20000, CRC(01b81da0) SHA1(914802f3206dc59a720af9d57eb2285bc8ba822b) ) /* same as tumble pop?, is this used? */
	ROM_RELOAD(            0x60000, 0x20000 )
	ROM_RELOAD(            0xa0000, 0x20000 )
	ROM_RELOAD(            0xe0000, 0x20000 )
ROM_END

} // anonymous namespace


GAME( 1992, diverboy, 0, diverboy, diverboy, diverboy_state, empty_init, ORIENTATION_FLIP_X, "Gamart (Electronic Devices Italy license)", "Diver Boy", MACHINE_SUPPORTS_SAVE )
