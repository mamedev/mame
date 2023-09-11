// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo

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

#include "dnbanban.lh"


namespace {

class katosmedz80_state : public driver_device
{
public:
	katosmedz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu (*this, "maincpu"),
		m_digits(*this, "digit%u", 0U),
		m_ledsp2(*this, "p2led%u", 0U),
		m_ledsp3(*this, "p3led%u", 0U),
		m_ledsp5(*this, "p5led%u", 0U),
		m_ledsp6(*this, "p6led%u", 0U),
		m_ledsp8(*this, "p8led%u", 0U)
	{ }

	void dnbanban(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	output_finder<4> m_digits;
	output_finder<8> m_ledsp2;
	output_finder<8> m_ledsp3;
	output_finder<8> m_ledsp5;
	output_finder<8> m_ledsp6;
	output_finder<8> m_ledsp8;

	void program_map(address_map &map);
	void io_map(address_map &map);

	void port8_w(uint8_t data);
	void output_digit(int i, u8 data);

	void ppi0_b_w(uint8_t data);
	void ppi0_c_w(uint8_t data);
	void ppi1_b_w(uint8_t data);
	void ppi1_c_w(uint8_t data);
};

void katosmedz80_state::machine_start()
{
	m_digits.resolve();
	m_ledsp2.resolve();
	m_ledsp3.resolve();
	m_ledsp5.resolve();
	m_ledsp6.resolve();
	m_ledsp8.resolve();
}

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
	map(0x08, 0x08).w(FUNC(katosmedz80_state::port8_w));
	map(0x0c, 0x0c).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void katosmedz80_state::port8_w(u8 data)
{
	// show layout (debug)
	for (u8 i = 0; i < 8; i++)
		m_ledsp8[i] = (data >> i) & 0x01;
}

void katosmedz80_state::output_digit(int i, u8 data)
{
	// assuming it's using a 7448
	static const u8 led_map[16] =
		//{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x5f,0x7c,0x58,0x5e,0x7b,0x71 }; // hex format

	m_digits[i] = led_map[data & 0xf];
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
	for(u8 i = 0; i < 8; i++)
		m_ledsp2[i] = (data >> i) & 0x01;

	logerror("PPI0 Port B OUT: %02X\n", data);
}

void katosmedz80_state::ppi0_c_w(uint8_t data)
{
	for(u8 i = 0; i < 8; i++)
		m_ledsp3[i] = (data >> i) & 0x01;

	//logerror("PPI0 Port C OUT: %02X\n", data);
}

void katosmedz80_state::ppi1_b_w(uint8_t data)
{
	for(u8 i = 0; i < 8; i++)
		m_ledsp5[i] = (data >> i) & 0x01;

	//logerror("PPI1 Port B OUT: %02X\n", data);
}

void katosmedz80_state::ppi1_c_w(uint8_t data)
{
	u8 dn = 0;
	u8 dig = data & 0x0f;
	if (dig == 1) dn = 0;
	if (dig == 2) dn = 1;
	if (dig == 4) dn = 2;
	if (dig == 8) dn = 3;

	output_digit(dn, (data >> 4) & 0x0f);

	for(u8 i = 0; i < 8; i++)
		m_ledsp6[i] = (data >> i) & 0x01;

	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));  // coin counter

	//logerror("PPI1 Port C OUT: %02X\n", data);
}


static INPUT_PORTS_START( dnbanban )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")	// Arm 1? (related error E1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")	// Arm 2? (related error E2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")	// Arm 3? (related error E3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")	// Arm 4? (related error E4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	// attract/demo sounds are triggered after 40d0-40d1 16bit counter decrements to 0.
	// reg at 40d4 stores the triggered sample number.
	// turning off, the counter and sample enumerator dissappear.
	PORT_DIPNAME( 0x01, 0x01, "Attract / Demo Sounds" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Test Sounds" )	    // LOW = Sound Test
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  // LOW = 2 Coins - HIGH = 1 Coin
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Coin In")       // COIN IN (related error E5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("Service Coin")  // Service COIN (related error E6)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Door")         // DOOR (related error E7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

INPUT_PORTS_END


void katosmedz80_state::dnbanban(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &katosmedz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &katosmedz80_state::io_map);
	m_maincpu->set_periodic_int(FUNC(katosmedz80_state::irq0_line_hold), attotime::from_hz(4*60));  // wrong

	i8255_device &ppi0(I8255(config, "ppi0"));  // D71055C
	// (00-03) Mode 0 - Ports A set as input, Ports B, high C & low C as output.
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.out_pb_callback().set(FUNC(katosmedz80_state::ppi0_b_w));
	ppi0.out_pc_callback().set(FUNC(katosmedz80_state::ppi0_c_w));

	i8255_device &ppi1(I8255(config, "ppi1"));  // D71055C
	// (04-07) Mode 0 - Ports A set as input, Ports B, high C & low C as output.
	ppi1.in_pa_callback().set_ioport("IN1");
	ppi1.out_pb_callback().set(FUNC(katosmedz80_state::ppi1_b_w));
	ppi1.out_pc_callback().set(FUNC(katosmedz80_state::ppi1_c_w));

	// 2x LEDs

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.65);
}


ROM_START( dnbanban )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "g25_a.ic17", 0x0000, 0x8000, CRC(ef441127) SHA1(69fea4992abb2c4905d3831b6f18e464088f0ec7) ) // MBM27C256A, 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "g25_v.ic7", 0x00000, 0x20000, CRC(87c7d45d) SHA1(3f035d5e62fe62111cee978ed1708e902c98526a) ) // MBM27C1000
ROM_END

} // anonymous namespace


GAMEL( 1993, dnbanban, 0, dnbanban, dnbanban, katosmedz80_state, empty_init, ROT0, "Kato's", "Dora Neco BanBan", MACHINE_IS_SKELETON_MECHANICAL, layout_dnbanban )
