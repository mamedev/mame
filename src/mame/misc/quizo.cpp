// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*********************************************
 Quiz Olympic (c)1985 Seoul Coin Corp.

 driver by Tomasz Slanina

 ROMs contains strings:

    QUIZ OLYMPIC Ver 1.0
    PROGRAMMED BY  K.ISHIDA
    AT 1984.10.26
    TAITO CORP.
    KUMAGAYA-TSC


--

Z80 @ 4.0MHz [8/2]
AY-3-8910 @ 1.3423MHz [21.47727/16]
1x 2016 RAM
4x 4416 RAM
Xtals 8MHz, 21.47727MHz
1x 82S123 PROM

**********************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class quizo_state : public driver_device
{
public:
	quizo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_rombank(*this, "rom")
		, m_vrambank(*this, "vram")
	{ }

	void quizo(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_memory_bank m_rombank;
	required_memory_bank m_vrambank;

	std::unique_ptr<uint8_t[]> m_vram;

	void rombank_w(uint8_t data);
	void vrambank_w(uint8_t data);

	void quizo_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void memmap(address_map &map) ATTR_COLD;
	void portmap(address_map &map) ATTR_COLD;
};

void quizo_state::quizo_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 16; i++)
	{
		int bit0, bit1, bit2;

		bit0 = 0;
		bit1 = BIT(color_prom[i], 0);
		bit2 = BIT(color_prom[i], 1);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 3);
		bit2 = BIT(color_prom[i], 4);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 5);
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

uint32_t quizo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 200; y++)
	{
		for (int x = 0; x < 80; x++)
		{
			uint8_t data1 = m_vram[y * 80 + x];
			uint8_t data2 = m_vram[y * 80 + x + 0x4000];

			// draw 4 pixels
			bitmap.pix(y, x * 4 + 0) = m_palette->pen((BIT(data2, 7) << 3) | (BIT(data2, 3) << 2) | (BIT(data1, 7) << 1) | BIT(data1, 3));
			bitmap.pix(y, x * 4 + 1) = m_palette->pen((BIT(data2, 6) << 3) | (BIT(data2, 2) << 2) | (BIT(data1, 6) << 1) | BIT(data1, 2));
			bitmap.pix(y, x * 4 + 2) = m_palette->pen((BIT(data2, 5) << 3) | (BIT(data2, 1) << 2) | (BIT(data1, 5) << 1) | BIT(data1, 1));
			bitmap.pix(y, x * 4 + 3) = m_palette->pen((BIT(data2, 4) << 3) | (BIT(data2, 0) << 2) | (BIT(data1, 4) << 1) | BIT(data1, 0));
		}
	}

	return 0;
}

void quizo_state::rombank_w(uint8_t data)
{
	m_rombank->set_entry(data & 0x0f);
}

void quizo_state::vrambank_w(uint8_t data)
{
	m_vrambank->set_entry(BIT(data, 3));
}

void quizo_state::machine_start()
{
	m_rombank->configure_entries(0, 16, memregion("user1")->base(), 0x4000);

	m_vram = std::make_unique<uint8_t[]>(0x8000);
	m_vrambank->configure_entries(0, 2, &m_vram[0], 0x4000);

	save_pointer(NAME(m_vram), 0x8000);
}

void quizo_state::memmap(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0xbfff).bankr("rom");
	map(0xc000, 0xffff).bankw("vram");
}

void quizo_state::portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x10, 0x10).portr("IN1");
	map(0x40, 0x40).portr("IN2");
	map(0x50, 0x51).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x60, 0x60).w(FUNC(quizo_state::rombank_w));
	map(0x70, 0x70).w(FUNC(quizo_state::vrambank_w));
}

static INPUT_PORTS_START( quizo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown) ) // enable this and IN2 bit 5 to get a message
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Timer" )
	PORT_DIPSETTING(    0x00, "Slow" ) // 7 seconds
	PORT_DIPSETTING(    0x02, "Fast" ) // 5 seconds
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Test Mode" ) // test mode + timer freeze during game
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown) ) // connected to IN0 bit 6
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 2 Coin/1 Credit, B 1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x40, "A 1 Coin/1 Credit, B 1 Coin/2 Credit" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
INPUT_PORTS_END

void quizo_state::quizo(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &quizo_state::memmap);
	m_maincpu->set_addrmap(AS_IO, &quizo_state::portmap);
	m_maincpu->set_vblank_int("screen", FUNC(quizo_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 200);
	screen.set_visarea(0*8, 320-1, 0*8, 200-1);
	screen.set_screen_update(FUNC(quizo_state::screen_update));

	PALETTE(config, m_palette, FUNC(quizo_state::quizo_palette), 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, "aysnd", 21.477272_MHz_XTAL / 16).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( quizo )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("rom1", 0x0000, 0x4000, CRC(6731735f) SHA1(7dbf48f833c7b7cde77df2a10781e5a8b6ae0533))
	ROM_CONTINUE(    0x0000, 0x4000)

	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("rom3", 0x00000, 0x8000, CRC(d344f97e) SHA1(3d669a56f084f2a7a50d7d211b84a50d35de66ac))
	ROM_LOAD("rom4", 0x18000, 0x8000, CRC(ab1eb174) SHA1(7d7a935aa7196a814c15f13444b88e770678b672))
	ROM_LOAD("rom2", 0x20000, 0x8000, CRC(a700eb30) SHA1(7800b3d2b7992c67c91cfb7e02c7cfc313b0ed5d))

	ROM_REGION(0x20, "proms", 0)
	ROM_LOAD("82s123", 0x00, 0x20, CRC(c3f15914) SHA1(19fd8e6f2a1256ae51c500a3bf1d7358810ef97e))
ROM_END

ROM_START( quizoa )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("7.bin", 0x0000, 0x4000, CRC(1579ae31) SHA1(b23539413d108223001a9fe827ea151d20682b7b))

	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("4.bin", 0x00000, 0x4000, CRC(a977bd3a) SHA1(22f1158253a31cf5513eed3537a6096b993b0919))
	ROM_LOAD("3.bin", 0x04000, 0x4000, CRC(4411bcff) SHA1(2f6692e082b335c3af8b92108f757d333599dd29))
	ROM_LOAD("2.bin", 0x08000, 0x4000, CRC(4a0df776) SHA1(4a7dc2347b33843c0a6bb497be56ccae1af1dae0))
	ROM_LOAD("1.bin", 0x1c000, 0x4000, CRC(d9566c1a) SHA1(2495c071d077e5a359c2d7541d8b7c175b398b56))
	ROM_LOAD("6.bin", 0x20000, 0x4000, CRC(f00f6356) SHA1(f306ec26ddbb503214e266cc9b74304af86bdbc6))
	ROM_LOAD("5.bin", 0x24000, 0x4000, CRC(39e577e3) SHA1(430d9fe916dfeecdb94c23be89f79a6408ff444e))

	ROM_REGION(0x20, "proms", 0)
	ROM_LOAD("82s123", 0x00, 0x20, CRC(c3f15914) SHA1(19fd8e6f2a1256ae51c500a3bf1d7358810ef97e))
ROM_END

} // anonymous namespace


GAME( 1985, quizo,  0,       quizo,  quizo, quizo_state, empty_init, ROT0, "Seoul Coin Corp.", "Quiz Olympic (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, quizoa, quizo,   quizo,  quizo, quizo_state, empty_init, ROT0, "Seoul Coin Corp.", "Quiz Olympic (set 2)", MACHINE_SUPPORTS_SAVE )
