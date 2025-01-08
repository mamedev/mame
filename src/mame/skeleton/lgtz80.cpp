// license:BSD-3-Clause
// copyright-holders:AJR

/*
Video slots by Logic Game Tech Int. (LGT).

The main components are:
scratched off rectangular 100-pin chip, stickered ASIC 1
scratched off square 100-pin chip, stickered ASIC 2
scratched off square 100-pin chip, stickered ASIC 3
scratched off square 44-pin chip, stickered ASIC 4
12 MHz XTAL (near ASIC 2)
7.3728 MHz XTAL (near ASIC 4)
U6295 sound chip
HM86171-80 RAMDAC (near CPU ROM)
6x M5M5256DVP (1 near CPU ROM, 5 near GFX ROMs)

The two dumped games use PCBs with different layout, however the components appear
to be the same or at least same from different manufacturers.

TODO: everything. "ASIC 1" is probably a KL5C80A12 CPU, though its on-chip peripherals are mostly unused.
fruitcat currently runs off the rails due to seemingly missing code, but fortuitously recovers.
*/


#include "emu.h"

#include "cpu/z80/kl5c80a12.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class lgtz80_state : public driver_device
{
public:
	lgtz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_control(0)
	{ }

	void fruitcat(machine_config &config) ATTR_COLD;
	void arthurkn(machine_config &config) ATTR_COLD;

	void init_arthurkn() ATTR_COLD;
	void init_fruitcat() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<kl5c80a12_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vblank_nmi_w(int state);

	void p0_w(u8 data);
	u8 control_r();
	void control_w(u8 data);
	u8 e0_r();

	void program_map(address_map &map) ATTR_COLD;
	void fruitcat_io_map(address_map &map) ATTR_COLD;
	void arthurkn_io_map(address_map &map) ATTR_COLD;

	u8 m_control;
};


void lgtz80_state::machine_start()
{
	save_item(NAME(m_control));
}

void lgtz80_state::machine_reset()
{
	control_w(0);
}

uint32_t lgtz80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void lgtz80_state::video_start()
{
}


void lgtz80_state::p0_w(u8 data)
{
	logerror("%s: p0_w(%02X)\n", machine().describe_context(), data);
}

u8 lgtz80_state::control_r()
{
	return m_control;
}

void lgtz80_state::control_w(u8 data)
{
	// Bit 7 = NMI mask
	m_control = data;
	if (!BIT(data, 7))
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

u8 lgtz80_state::e0_r()
{
	// arthurkn: protection?
	return 0x6e;
}

void lgtz80_state::vblank_nmi_w(int state)
{
	if (state && BIT(m_control, 7))
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void lgtz80_state::program_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x28000, 0x29fff).ram(); // NVRAM?
	map(0x2a000, 0x2bfff).ram(); // arthurkn needs to copy code to RAM here, but fruitcat doesn't initialize it!
	map(0x4c000, 0x4efff).ram(); // video RAM?
}

void lgtz80_state::fruitcat_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x82).nopw(); // RAMDAC?
	map(0x88, 0x88).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc0, 0xc0).rw(FUNC(lgtz80_state::control_r), FUNC(lgtz80_state::control_w));
}

void lgtz80_state::arthurkn_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x88, 0x88).nopw();
	map(0xa0, 0xa0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xb0, 0xb2).nopw(); // RAMDAC?
	map(0xc0, 0xc0).rw(FUNC(lgtz80_state::control_r), FUNC(lgtz80_state::control_w));
	map(0xe0, 0xe0).r(FUNC(lgtz80_state::e0_r));
}


static INPUT_PORTS_START( fruitcat )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

// no DSW on PCB
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx_lgtz80 )
GFXDECODE_END


void lgtz80_state::fruitcat(machine_config &config)
{
	KL5C80A12(config, m_maincpu, 12_MHz_XTAL); // exact CPU model and divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &lgtz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &lgtz80_state::fruitcat_io_map);
	m_maincpu->out_p0_callback().set(FUNC(lgtz80_state::p0_w));
	m_maincpu->in_p1_callback().set_ioport("IN1");
	m_maincpu->in_p2_callback().set_ioport("IN2");
	m_maincpu->in_p3_callback().set_ioport("IN3");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(lgtz80_state::screen_update));
	screen.screen_vblank().set(FUNC(lgtz80_state::vblank_nmi_w));

	GFXDECODE(config, "gfxdecode", "palette", gfx_lgtz80);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}

void lgtz80_state::arthurkn(machine_config &config)
{
	fruitcat(config);
	m_maincpu->set_addrmap(AS_IO, &lgtz80_state::arthurkn_io_map);
}


ROM_START( fruitcat )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fruit_cat_v2.00.u8", 0x00000, 0x20000, CRC(83d71147) SHA1(4253f5d3273bce22262d34a08f492fa72f776aa5) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruit_cat_rom2.u22", 0x000000, 0x200000, CRC(c78150e8) SHA1(eb276b9b2c4e45b8caf81f17831f6201a6d7392c) ) // actual label has ROM2 between brackets
	ROM_LOAD( "fruit_cat_rom3.u29", 0x200000, 0x200000, CRC(71afea49) SHA1(89c814302fb58705a479310edb433594d151dfb5) ) // actual label has ROM3 between brackets

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "am29f040.u2", 0x00000, 0x80000, CRC(efd1209e) SHA1(5cd76c9d3073b2e689aa7903e2d65b8ce5b091ca) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u21", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( arthurkn ) // no stickers on ROMs
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "w29ee011.u21", 0x00000, 0x20000, CRC(d8e2b9f4) SHA1(e8c55c42d7b57fde3168e07fa51f307b83803967) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "m27c160.u42", 0x000000, 0x200000, CRC(f03a9b0d) SHA1(1e8d9efe7d50871ffc6a0c4c7f08047dd5aac294) )
	ROM_LOAD( "m27c160.u43", 0x200000, 0x200000, CRC(31d2caab) SHA1(0ee7f35dadb1d5159a487701d059bfd2f54f8c02) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "m29f040b.u18", 0x00000, 0x80000, CRC(2b9ab706) SHA1(92154126c7db227acaa4966f71d28475c622e1e6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u3", 0x000, 0x117, NO_DUMP )
ROM_END


void lgtz80_state::init_fruitcat()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
		case 0x000:
			rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0);
			break;

		case 0x040:
		case 0x440:
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0);
			break;

		case 0x080:
		case 0x480:
			rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0);
			break;

		case 0x0c0:
			rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0);
			break;

		case 0x100:
		case 0x400:
			rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0);
			break;

		case 0x140:
			rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0);
			break;

		case 0x180:
		case 0x2c0:
			rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0);
			break;

		case 0x1c0:
			rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0);
			break;

		case 0x200:
			rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0);
			break;

		case 0x240:
		case 0x5c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0);
			break;

		case 0x280:
			rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0);
			break;

		case 0x300:
			//rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 2, 1, 0);
			break;

		case 0x340:
			rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0);
			break;

		case 0x380:
			rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0);
			break;

		case 0x3c0:
		case 0x540:
			rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0);
			break;

		case 0x4c0:
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0);
			break;

		case 0x500:
		case 0x780:
			rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0);
			break;

		case 0x580:
			rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0);
			break;

		case 0x600:
		case 0x680:
			rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0);
			break;

		case 0x640:
			rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0);
			break;

		case 0x6c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0);
			break;

		case 0x700:
			rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0);
			break;

		case 0x740:
			rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0);
			break;

		case 0x7c0:
			rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 5, 2, 7, 0);
			break;
		}
	}
}

void lgtz80_state::init_arthurkn()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
		case 0x000:
		case 0x1c0:
			rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0);
			break;

		case 0x040:
		case 0x0c0:
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0);
			break;

		case 0x080:
		case 0x2c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0);
			break;

		case 0x100:
			rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0);
			break;

		case 0x140:
		case 0x340:
			rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0);
			break;

		case 0x180:
			//rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 2, 1, 0);
			break;

		case 0x200:
		case 0x380:
			rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0);
			break;

		case 0x240:
			rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0);
			break;

		case 0x280:
		case 0x300:
			rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0);
			break;

		case 0x3c0:
		case 0x440:
			rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0);
			break;

		case 0x400:
		case 0x480:
			rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0);
			break;

		case 0x4c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0);
			break;

		case 0x500:
			rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0);
			break;

		case 0x540:
			rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0);
			break;

		case 0x580:
			rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0);
			break;

		case 0x5c0:
			rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0);
			break;

		case 0x600:
			rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0);
			break;

		case 0x640:
			rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0);
			break;

		case 0x680:
			rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0);
			break;

		case 0x6c0:
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0);
			break;

		case 0x700:
			rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 5, 2, 7, 0);
			break;

		case 0x740:
			rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0);
			break;

		case 0x780:
			rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0);
			break;

		case 0x7c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0);
			break;
		}
	}
}

} // anonymous namespace


GAME( 2003?, fruitcat, 0, fruitcat, fruitcat, lgtz80_state, init_fruitcat, ROT0, "LGT", "Fruit Cat (v2.00)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?,  arthurkn, 0, arthurkn, fruitcat, lgtz80_state, init_arthurkn, ROT0, "LGT", "Arthur's Knights",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
