// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Flak Attack / MX5000 (Konami GX669)

    Driver by:
        Manuel Abadia <emumanu+mame@gmail.com>

    TO DO:
        -What does 0x900X do? (Z80)

NOTE: There is known to exist a USA version of Flak Attack - currently not dumped

24MHz & 3.579545MHz OSCs

***************************************************************************/

#include "emu.h"
#include "includes/flkatck.h"
#include "includes/konamipt.h"

#include "cpu/z80/z80.h"
#include "cpu/m6809/hd6309.h"
#include "sound/ym2151.h"
#include "screen.h"
#include "speaker.h"


INTERRUPT_GEN_MEMBER(flkatck_state::flkatck_interrupt)
{
	if (m_irq_enabled)
		device.execute().set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(flkatck_state::flkatck_bankswitch_w)
{
	/* bits 3-4: coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);

	/* bits 0-1: bank # */
	if ((data & 0x03) != 0x03)  /* for safety */
		membank("bank1")->set_entry(data & 0x03);
}

READ8_MEMBER(flkatck_state::flkatck_ls138_r)
{
	int data = 0;

	switch ((offset & 0x1c) >> 2)
	{
		case 0x00:
			if (offset & 0x02)
				data = ioport((offset & 0x01) ? "COIN" : "DSW3")->read();
			else
				data = ioport((offset & 0x01) ? "P2" : "P1")->read();
			break;
		case 0x01:
			if (offset & 0x02)
				data = ioport((offset & 0x01) ? "DSW1" : "DSW2")->read();
			break;
	}

	return data;
}

WRITE8_MEMBER(flkatck_state::flkatck_ls138_w)
{
	switch ((offset & 0x1c) >> 2)
	{
		case 0x04:  /* bankswitch */
			flkatck_bankswitch_w(space, 0, data);
			break;
		case 0x05:  /* sound code number */
			m_soundlatch->write(data);
			break;
		case 0x06:  /* Cause interrupt on audio CPU */
			m_audiocpu->set_input_line(0, HOLD_LINE);
			break;
		case 0x07:  /* watchdog reset */
			m_watchdog->watchdog_reset();
			break;
	}
}

/* Protection - an external multiplyer connected to the sound CPU */
READ8_MEMBER(flkatck_state::multiply_r)
{
	return (m_multiply_reg[0] * m_multiply_reg[1]) & 0xff;
}

WRITE8_MEMBER(flkatck_state::multiply_w)
{
	m_multiply_reg[offset] = data;
}


void flkatck_state::flkatck_map(address_map &map)
{
	map(0x0000, 0x0007).ram().w(FUNC(flkatck_state::flkatck_k007121_regs_w));                                   /* 007121 registers */
	map(0x0008, 0x03ff).ram();                                                                 /* RAM */
	map(0x0400, 0x041f).rw(FUNC(flkatck_state::flkatck_ls138_r), FUNC(flkatck_state::flkatck_ls138_w));                         /* inputs, DIPS, bankswitch, counters, sound command */
	map(0x0800, 0x0bff).ram().w("palette", FUNC(palette_device::write8)).share("palette"); /* palette */
	map(0x1000, 0x1fff).ram().share("spriteram");                                            /* RAM */
	map(0x2000, 0x2fff).ram().w(FUNC(flkatck_state::vram_w)).share("vram");                    /* Video RAM (007121) */
	map(0x3000, 0x3fff).ram();
	map(0x4000, 0x5fff).bankr("bank1");                                                            /* banked ROM */
	map(0x6000, 0xffff).rom();                                                                 /* ROM */
}

void flkatck_state::flkatck_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();                                             /* ROM */
	map(0x8000, 0x87ff).ram();                                             /* RAM */
	map(0x9000, 0x9000).r(FUNC(flkatck_state::multiply_r));                                // 007452: Protection (see wecleman, but unused here?)
	map(0x9001, 0x9001).nopr();                                         // 007452: ?
	map(0x9000, 0x9001).w(FUNC(flkatck_state::multiply_w));                               // 007452: Protection (see wecleman, but unused here?)
	map(0x9004, 0x9004).nopr();                                         // 007452: ?
	map(0x9006, 0x9006).nopw();                                        // 007452: ?
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write)); /* 007232 registers */
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));           /* YM2151 */
}


static INPUT_PORTS_START( flkatck )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30K, Every 70K" )
	PORT_DIPSETTING(    0x10, "40K, Every 80K" )
	PORT_DIPSETTING(    0x08, "30K Only" )
	PORT_DIPSETTING(    0x00, "40K Only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC(   0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )        /* Listed as "Unused" */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static const gfx_layout gfxlayout =
{
	8,8,
	0x80000/32,
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( gfx_flkatck )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout, 0, 32 )
GFXDECODE_END

WRITE8_MEMBER(flkatck_state::volume_callback)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void flkatck_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 3, &ROM[0x10000], 0x2000);

	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_multiply_reg));
	save_item(NAME(m_flipscreen));
}

void flkatck_state::machine_reset()
{
	m_k007232->set_bank(0, 1);

	m_irq_enabled = 0;
	m_multiply_reg[0] = 0;
	m_multiply_reg[1] = 0;
	m_flipscreen = 0;
}

void flkatck_state::flkatck(machine_config &config)
{
	/* basic machine hardware */
	HD6309E(config, m_maincpu, 24_MHz_XTAL / 8); /* HD63C09EP, 3MHz (24MHz/8) */
	m_maincpu->set_addrmap(AS_PROGRAM, &flkatck_state::flkatck_map);
	m_maincpu->set_vblank_int("screen", FUNC(flkatck_state::flkatck_interrupt));

	Z80(config, m_audiocpu, 3.579545_MHz_XTAL);   /* NEC D780C-1, 3.579545MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &flkatck_state::flkatck_sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(37*8, 32*8);
	screen.set_visarea(0*8, 35*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(flkatck_state::screen_update_flkatck));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_flkatck);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 512).set_endianness(ENDIANNESS_LITTLE);

	K007121(config, m_k007121, 0);
	m_k007121->set_palette_tag("palette");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "lspeaker", 1.0).add_route(0, "rspeaker", 1.0);

	K007232(config, m_k007232, 3.579545_MHz_XTAL);
	m_k007232->port_write().set(FUNC(flkatck_state::volume_callback));
	m_k007232->add_route(0, "lspeaker", 0.50);
	m_k007232->add_route(0, "rspeaker", 0.50);
	m_k007232->add_route(1, "lspeaker", 0.50);
	m_k007232->add_route(1, "rspeaker", 0.50);
}



ROM_START( mx5000 )
	ROM_REGION( 0x18000, "maincpu", 0 )  /* 6309 code */
	ROM_LOAD( "669_r01.16c", 0x010000, 0x006000, CRC(79b226fc) SHA1(3bc4d93717230fecd54bd08a0c3eeedc1c8f571d) ) /* banked ROM */
	ROM_CONTINUE(            0x006000, 0x00a000 )           /* fixed ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )  /* 64k for the SOUND CPU */
	ROM_LOAD( "669_m02.16b", 0x000000, 0x008000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x080000, "gfx1", 0 )  /* tiles + sprites */
	ROM_LOAD( "gx669f03.5e",   0x000000, 0x080000, CRC(ff1d718b) SHA1(d44fe3ed5a3ba1b3036264e37f9cd3500b706635) ) /* MASK4M */

	ROM_REGION( 0x040000, "k007232", 0 )  /* 007232 data (chip 1) */
	ROM_LOAD( "gx669f04.11a",  0x000000, 0x040000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) ) /* MASK2M */
ROM_END

ROM_START( flkatck )
	ROM_REGION( 0x18000, "maincpu", 0 )  /* 6309 code */
	ROM_LOAD( "669_p01.16c", 0x010000, 0x006000, CRC(c5cd2807) SHA1(22ddd911a23954ff2d52552e07323f5f0ddaeead) ) /* banked ROM */
	ROM_CONTINUE(            0x006000, 0x00a000 )           /* fixed ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )  /* 64k for the SOUND CPU */
	ROM_LOAD( "669_m02.16b", 0x000000, 0x008000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x080000, "gfx1", 0 )  /* tiles + sprites */
	ROM_LOAD( "gx669f03.5e",   0x000000, 0x080000, CRC(ff1d718b) SHA1(d44fe3ed5a3ba1b3036264e37f9cd3500b706635) ) /* MASK4M */

	ROM_REGION( 0x040000, "k007232", 0 )  /* 007232 data (chip 1) */
	ROM_LOAD( "gx669f04.11a",  0x000000, 0x040000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) ) /* MASK2M */
ROM_END

// identical to flkatck except for the board / ROM type configuration
ROM_START( flkatcka )
	ROM_REGION( 0x18000, "maincpu", 0 )  /* 6309 code */
	ROM_LOAD( "669_p01.16c", 0x010000, 0x006000, CRC(c5cd2807) SHA1(22ddd911a23954ff2d52552e07323f5f0ddaeead) ) /* banked ROM */
	ROM_CONTINUE(            0x006000, 0x00a000 )           /* fixed ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )  /* 64k for the SOUND CPU */
	ROM_LOAD( "669_m02.16b", 0x000000, 0x008000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x080000, "gfx1", 0 )  /* tiles + sprites */ // same data as above set, on PWB 450593 sub-board instead.
	ROM_LOAD16_BYTE( "669_f03a.4b",   0x000001, 0x010000, CRC(f0ed4c1e) SHA1(58efe3cd81054d22de54a7d195aa3b865bde4a01) )
	ROM_LOAD16_BYTE( "669_f03e.4d",   0x000000, 0x010000, CRC(95a57a26) SHA1(c8aa30c2c734c0740630b1b04ae43c69931cc7c1) )
	ROM_LOAD16_BYTE( "669_f03b.5b",   0x020001, 0x010000, CRC(e2593f3c) SHA1(aa0f6d04015650eaef17c4a39f228eaccf9a2948) )
	ROM_LOAD16_BYTE( "669_f03f.5d",   0x020000, 0x010000, CRC(c6c9903e) SHA1(432ad6d03992499cc533273226944a666b40fa58) )
	ROM_LOAD16_BYTE( "669_f03c.6b",   0x040001, 0x010000, CRC(47be92dd) SHA1(9ccc62d7d42fccbd5ad60e35e3a0478a04405cf1) )
	ROM_LOAD16_BYTE( "669_f03g.6d",   0x040000, 0x010000, CRC(70d35fbd) SHA1(21384f738684c5da4a7a84a1c9aa173fffddf47a) )
	ROM_LOAD16_BYTE( "669_f03d.7b",   0x060001, 0x010000, CRC(18d48f9e) SHA1(b95e38aa813e0f3a0dc6bd45fdb4bf71f7e2066c) )
	ROM_LOAD16_BYTE( "669_f03h.7d",   0x060000, 0x010000, CRC(abfe76e7) SHA1(f8661f189308e83056ec442fa6c936efff67ba0a) )

	ROM_REGION( 0x040000, "k007232", 0 )  /* 007232 data (chip 1) */
	ROM_LOAD( "gx669f04.11a",  0x000000, 0x040000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) ) /* MASK2M */
ROM_END

GAME( 1987, mx5000,   0,      flkatck, flkatck, flkatck_state, empty_init, ROT90, "Konami", "MX5000", MACHINE_SUPPORTS_SAVE )
GAME( 1987, flkatck,  mx5000, flkatck, flkatck, flkatck_state, empty_init, ROT90, "Konami", "Flak Attack (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, flkatcka, mx5000, flkatck, flkatck, flkatck_state, empty_init, ROT90, "Konami", "Flak Attack (Japan, PWB 450593 sub-board)", MACHINE_SUPPORTS_SAVE )
