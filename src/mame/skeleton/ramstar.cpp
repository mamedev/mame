// license:BSD-3-Clause
// copyright-holders:

/*

Unknown multi-game by Ram Star (1991)
It is currently unknown if the system itself has a name.
It comprises at least 2 PCBs, which don't plug in one another.
So very probably missing at least a backplane, if not more PCBs.

RAM STAR VIDEO GAME PCB4201C
W65C816PL CPU
MAX691CPE supervisor
X24C04 serial E2PROM
P5C032 CPLD
10 MHz XTAL
M62X42B RTC (32.768KHz built-in XTAL)
P5164SL SRAM
8-DIP bank
AY-3-8912 sound chip
2x 27C512 ROM
R65C52P2 DACIA
R65C22P3 VIA
3.6864 MHz XTAL
NE5152BN TRIPLE 4-Bit RGB D/A CONVERTER
HM62256LP-10 video RAM
W65C29P IOD
W65C90P EBC

RamStar GAME MODULE CONTROL PCB4204A
SC80C31BCCN40 ROMless MCU
27C640 ROM
6 MHz XTAL
6x card slots

The carts' PCB is RamStar GAME MODULE PCB4203A.
It only contains a 27C512 or 27C1024 and a X24C04.
*/


#include "emu.h"

#include "cpu/g65816/g65816.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/6522via.h"
#include "machine/i2cmem.h"
#include "machine/msm6242.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ramstar_state : public driver_device
{
public:
	ramstar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cartcpu(*this, "cartcpu")
	{ }

	void ramstar(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cartcpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void main_data_map(address_map &map) ATTR_COLD;
	void cart_program_map(address_map &map) ATTR_COLD;
	void cart_io_map(address_map &map) ATTR_COLD;
};


uint32_t ramstar_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void ramstar_state::main_program_map(address_map &map)
{
	map.global_mask(0x0ffff);
	map(0x00000, 0x0ffff).rom();
}

void ramstar_state::main_data_map(address_map &map)
{
	map(0x00000, 0x01fff).ram(); // P5164SL
	map(0x08200, 0x0820f).m("via", FUNC(via6522_device::map)); // TODO: maybe
	//map(0x08400, 0x08407).rw() // DACIA?
	//map(0x08600, 0x08601).w()  // ??
	//map(0x08800, 0x08801).w()  // ??
	//map(0x08a00, 0x08a01).w()  // ??
	//map(0x08c00, 0x08c01).rw() // AY8912?
	//map(0x08e00, 0x08e0f).rw() // MSM6242?
	map(0x10000, 0x17fff).ram(); // HM62256, video RAM according to PCB silkscreen
	//map(0x18000, 0x1802f).w()  // ??
}

void ramstar_state::cart_program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void ramstar_state::cart_io_map(address_map &map)
{
	//map(0x00, 0x00).r()
}


static INPUT_PORTS_START( ramstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


void ramstar_state::ramstar(machine_config &config)
{
	// basic machine hardware
	G65816(config, m_maincpu, 10_MHz_XTAL); // (absence of) divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &ramstar_state::main_program_map);
	m_maincpu->set_addrmap(AS_DATA, &ramstar_state::main_data_map);

	I2C_X2404P(config, "i2cmem"); // actually X24C04

	MSM6242(config, "rtc", 32.768_kHz_XTAL); // actually M62X42B

	R65C22(config, "via", 3.6864_MHz_XTAL / 2); // divider not verified

	// TODO: R65C52 DACIA

	I80C31(config, m_cartcpu, 6_MHz_XTAL); // (absence of) divider not verified
	m_cartcpu->set_addrmap(AS_PROGRAM, &ramstar_state::cart_program_map);
	m_cartcpu->set_addrmap(AS_IO, &ramstar_state::cart_io_map);

	SOFTWARE_LIST(config, "cart_list").set_original("ramstar");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify everything once emulation works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(ramstar_state::screen_update));

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8912(config, "ay", 10_MHz_XTAL / 10).add_route(ALL_OUTPUTS, "mono", 1.0); // divider not verified
}


ROM_START( ramstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.u46", 0x00000, 0x10000, CRC(8079f1f1) SHA1(82070e1b350e8a91723f892fb21e1fe00c2e5bb2) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "27c512.u45", 0x00000, 0x10000, CRC(9a5d02fd) SHA1(f33baa7cac6f334dd167afb36a50bc0c040d0645) )

	ROM_REGION( 0x2000, "cartcpu", 0 )
	ROM_LOAD( "27c640.u5", 0x0000, 0x2000, CRC(68529c08) SHA1(47aa8693f8c1028e4807e3d94301ef51fab98909) )
ROM_END

} // anonymous namespace


GAME( 1991, ramstar, 0, ramstar, ramstar, ramstar_state, empty_init, ROT0, "Ram Star", "unknown Ram Star multi-game (version 80.1)", MACHINE_IS_SKELETON )
