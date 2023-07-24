// license:BSD-3-Clause
// copyright-holders:

/*
ドラネコバンバン - Dora Neco BanBan (as transliterated on the cab)
Medal game by Kato's, whack-a-mole style

PCB is unmarked

Main components:
Sharp LH0080B Z80B-CPU
12.000 MHz XTAL
HM6116LP-3 Static RAM
2x NEC D71055C
OKI M6295GS
4-DIP bank
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/okim6295.h"

#include "speaker.h"


namespace {

class katosmedz80_state : public driver_device
{
public:
	katosmedz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu (*this, "maincpu")
	{}

	void dnbanban(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map);
	void io_map(address_map &map);

	void ppi0_b_w(uint8_t data);
	void ppi0_c_w(uint8_t data);
	void ppi1_b_w(uint8_t data);
	void ppi1_c_w(uint8_t data);
};


void katosmedz80_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
}

void katosmedz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	// map(0x08, 0x08) // ??
	map(0x0c, 0x0c).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}
/*
  .-----.-----------.---------.---------.---------.-------.-------.-------.
  | PPI | Map Range | control | PA mode | PB mode | PortA | PortB | PortC |
  '-----+-----------'---------'---------'---------'-------'-------'-------'
   PPI0    00h-03h     0x90       M0        M0       IN      OUT     OUT
   PPI1    04h-07h     0x90       M0        M0       IN      OUT     OUT

*/

void katosmedz80_state::ppi0_b_w(uint8_t data)
{
	logerror("PPI0 Port B OUT: %02X\n", data);
}

void katosmedz80_state::ppi0_c_w(uint8_t data)
{
	logerror("PPI0 Port C OUT: %02X\n", data);
}

void katosmedz80_state::ppi1_b_w(uint8_t data)
{
	logerror("PPI1 Port B OUT: %02X\n", data);
}

void katosmedz80_state::ppi1_c_w(uint8_t data)
{
	logerror("PPI1 Port C OUT: %02X\n", data);
}


static INPUT_PORTS_START( dnbanban )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("DSW") // 4-DIP bank
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void katosmedz80_state::dnbanban(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &katosmedz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &katosmedz80_state::io_map);
	m_maincpu->set_periodic_int(FUNC(katosmedz80_state::irq0_line_hold), attotime::from_hz(4*60)); // wrong

	i8255_device &ppi0(I8255(config, "ppi0")); // D71055C
	// (00-03) Mode 0 - Ports A set as input, Ports B, high C & low C as output.
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.out_pb_callback().set(FUNC(katosmedz80_state::ppi0_b_w));
	ppi0.out_pc_callback().set(FUNC(katosmedz80_state::ppi0_c_w));

	i8255_device &ppi1(I8255(config, "ppi1")); // D71055C
	// (04-07) Mode 0 - Ports A set as input, Ports B, high C & low C as output.
	ppi1.in_pa_callback().set_ioport("IN1");
	ppi1.out_pb_callback().set(FUNC(katosmedz80_state::ppi1_b_w));
	ppi1.out_pc_callback().set(FUNC(katosmedz80_state::ppi1_c_w));

	// 2x LEDs

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.65); // resonator value and pin 7 verified
}


ROM_START( dnbanban )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "g25_a.ic17", 0x0000, 0x8000, CRC(ef441127) SHA1(69fea4992abb2c4905d3831b6f18e464088f0ec7) ) // MBM27C256A, 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "g25_v.ic7", 0x00000, 0x20000, CRC(87c7d45d) SHA1(3f035d5e62fe62111cee978ed1708e902c98526a) ) // MBM27C1000
ROM_END

} // anonymous namespace


GAME( 1993, dnbanban, 0, dnbanban, dnbanban, katosmedz80_state, empty_init, ROT0, "Kato's", "Dora Neco BanBan", MACHINE_IS_SKELETON_MECHANICAL )
