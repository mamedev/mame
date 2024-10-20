// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************************

PINBALL
Sleic Pinball

The only known schematic is for Bike Race and has most crystal frequencies unmarked.
 Some ICs aren't identified. No schematic for the display. It can be seen from videos
 that a DMD is used.

There's no standard inputs for coin, start etc.

Games:
- Pinball
- Bike Race
- Dona Elvira 2
- Super Pang
- Io Moon

Principal components:
    80C188-10
    80C39-11
    27C64
    27C040
    27C010
    28C64A
    6376 (Voice Synthesiser by OKI)
    YM3812 (Sound Generator by Yamaha)
    YM3014 (DAC Sounds by Yamaha)
    X9103 NVRAM
    Z80A
    27C256
    7764 (unable to find out what this is)
    Undumped PALs: 20L40, 16L8
    Optional PIC16CR54-HS

Status:
- Skeletons

ToDo:
- Everything!


****************************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/ripple_counter.h"
#include "sound/okim6376.h"
#include "speaker.h"

namespace {

class sleic_state : public genpin_class
{
public:
	sleic_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cpu2(*this, "cpu2")
		, m_cpu3(*this, "cpu3")
		, m_oki(*this, "oki")
		, m_4020(*this, "4020")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void sleic(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
	void cpu2_io(address_map &map) ATTR_COLD;
	void cpu3_map(address_map &map) ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	u8 cpu2_in00() { return 0; }   // communications + handshake
	u8 cpu2_in01() { return 0; }   // communications + handshake
	u8 cpu2_in02() { u8 data = 0U; for (u8 i = 0; i < 6; i++) if (BIT(m_row, i)) data |= m_io_keyboard[i]->read(); return data; }   // contactors
	u8 cpu2_in03() { return 0; }   // contactors
	void cpu2_out80(u8 data) {}   // communications + handshake
	void cpu2_out81(u8 data) {}   // communications + handshake
	void cpu2_out82(u8 data) { m_row = data; }   // contactors
	void cpu2_out83(u8 data) {}   // lamps + solenoids
	void cpu2_out84(u8 data) {}   // lamps + solenoids
	void cpu2_out85(u8 data) {}   // lamps + solenoids
	void cpu2_out86(u8 data) {}   // additional outputs
	void cpu2_out87(u8 data) {}   // additional outputs
	void clockcnt_w(u16 data);
	u8 m_row = 0U;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu2;
	required_device<cpu_device> m_cpu3;
	required_device<okim6376_device> m_oki;
	required_device<ripple_counter_device> m_4020;
	required_ioport_array<6> m_io_keyboard;
	output_finder<64> m_io_outputs;  // ?? solenoids + ?? lamps
};


void sleic_state::main_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0x20000, 0x2ffff).nopr();
	map(0x40000, 0x70fff).noprw();
	map(0x80000, 0xfffff).rom();
	map(0xa0000, 0xa0001).nopw();
	map(0xa0200, 0xa0201).nopw();
}

void sleic_state::cpu2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
}

void sleic_state::cpu2_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(sleic_state::cpu2_in00));
	map(0x01, 0x01).r(FUNC(sleic_state::cpu2_in01));
	map(0x02, 0x02).r(FUNC(sleic_state::cpu2_in02));
	map(0x03, 0x03).r(FUNC(sleic_state::cpu2_in03));
	map(0x04, 0x04).portr("DSW");
	map(0x80, 0x80).w(FUNC(sleic_state::cpu2_out80));
	map(0x81, 0x81).w(FUNC(sleic_state::cpu2_out81));
	map(0x82, 0x82).w(FUNC(sleic_state::cpu2_out82));
	map(0x83, 0x83).w(FUNC(sleic_state::cpu2_out83));
	map(0x84, 0x84).w(FUNC(sleic_state::cpu2_out84));
	map(0x85, 0x85).w(FUNC(sleic_state::cpu2_out85));
	map(0x86, 0x86).w(FUNC(sleic_state::cpu2_out86));
	map(0x87, 0x87).w(FUNC(sleic_state::cpu2_out87));
}

void sleic_state::cpu3_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

static INPUT_PORTS_START( sleic )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "SW1")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "SW2")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "SW3")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x08, "SW4")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "SW5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ))
	PORT_DIPNAME( 0x20, 0x20, "SW6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ))
	PORT_DIPNAME( 0x40, 0x40, "SW7")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "SW8")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))

	// Temporary - need to find out the start button, coins etc
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP01")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP02")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP03")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP04")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP05")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP06")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP07")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP10")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP12")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP13")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP15")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP16")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP17")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP20")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP21")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP22")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP23")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP24")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP25")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP26")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP27")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP30")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP31")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP32")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP35")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP36")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP37")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP40")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP41")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP42")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP43")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP45")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP46")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP47")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP50")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP51")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP52")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP53")
INPUT_PORTS_END

void sleic_state::clockcnt_w(u16 data)
{
	if (data == 0x7ff)
		m_cpu2->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	else
	if (data == 0)
		m_cpu2->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

void sleic_state::machine_start()
{
	genpin_class::machine_start();

	m_io_outputs.resolve();

	save_item(NAME(m_row));
}

void sleic_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}

void sleic_state::sleic(machine_config &config)
{
	/* basic machine hardware */
	I80188(config, m_maincpu, 12_MHz_XTAL);   // 80188, crystal unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &sleic_state::main_map);

	Z80(config, m_cpu2, XTAL(8'000'000)/2);
	m_cpu2->set_addrmap(AS_PROGRAM, &sleic_state::cpu2_map);
	m_cpu2->set_addrmap(AS_IO, &sleic_state::cpu2_io);

	I8039(config, m_cpu3, 11_MHz_XTAL);   // crystal unknown
	m_cpu3->set_addrmap(AS_PROGRAM, &sleic_state::cpu3_map);
	//m_cpu3->p2_out_cb().set(FUNC(sleic_state::p2_w));
	//m_cpu3->p1_in_cb().set(FUNC(sleic_state::p1_r));
	//m_cpu3->p1_out_cb().set(FUNC(sleic_state::p1_w));
	//m_cpu3->t1_in_cb().set(FUNC(sleic_state::t1_r));

	RIPPLE_COUNTER(config, m_4020);
	m_4020->set_stages(11); // Using Q1-11
	m_4020->count_out_cb().set(FUNC(sleic_state::clockcnt_w));
	CLOCK(config, "rclock", 8'000'000/4).signal_handler().set(m_4020, FUNC(ripple_counter_device::clock_w));

	/* video hardware */
	//dmd;

	/* sound hardware */
	genpin_audio(config);

	SPEAKER(config, "mono").front_center();
	OKIM6376(config, m_oki, 128'000); // xtal value not shown, guessing
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);

	//ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(4'000'000))); // no idea.. ic type not shown, source of clock not evident
	//ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	//ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

/*-------------------------------------------------------------------
/ Bike Race (1992)
/-------------------------------------------------------------------*/
ROM_START(bikerace)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("bkcpu04.bin", 0xe0000, 0x20000, CRC(ce745e89) SHA1(04ba97a9ef1e60a7609c87cf6d8fcae2d0e32621))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("bkio07.bin", 0x0000, 0x8000, CRC(b52a9d4f) SHA1(726a4d9b354729d7390d2a4f877dc480701ec795))

	ROM_REGION(0x2000, "cpu3", 0)
	ROM_LOAD("bkdsp01.bin", 0x0000, 0x2000, CRC(9b220fcb) SHA1(54e82705d8ce8a26d9e1b5f0fe382ded1f2070c3))

	ROM_REGION(0x100000, "oki", 0)
	ROM_LOAD("bksnd02.bin", 0x00000, 0x80000, CRC(d67b3883) SHA1(712022b9b24c6ab559d020ab8e2106f68b4d7896))
	ROM_LOAD("bksnd03.bin", 0x80000, 0x80000, CRC(b6d00245) SHA1(f7da6f2ca681fbe62ea9cab7f92d3e501b7e867d))

	ROM_REGION(0x100000, "user2", 0)
	ROM_LOAD("bkcpu05.bin", 0x00000, 0x20000, CRC(072ce879) SHA1(4f6fb044592feb4c72bbdcbe5f19e063c0e49d0d))
	ROM_LOAD("bkcpu06.bin", 0x20000, 0x20000, CRC(9db436d4) SHA1(3869524c0490e0a019d2f8ab46546ff42727665e))
ROM_END

ROM_START(bikerace2)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("04.bin", 0xe0000, 0x20000, CRC(aaaa4a8a) SHA1(ff579041575da4060615da2ff634f3aa91537751))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("07.bin", 0x0000, 0x8000, CRC(0b763a89) SHA1(8952d7b13674e1599e53cce96e57c2783899a90a))

	ROM_REGION(0x2000, "cpu3", 0)
	ROM_LOAD("bkdsp01.bin", 0x0000, 0x2000, CRC(9b220fcb) SHA1(54e82705d8ce8a26d9e1b5f0fe382ded1f2070c3))

	ROM_REGION(0x100000, "oki", 0)
	ROM_LOAD("bksnd02.bin", 0x00000, 0x80000, CRC(d67b3883) SHA1(712022b9b24c6ab559d020ab8e2106f68b4d7896))
	ROM_LOAD("bksnd03.bin", 0x80000, 0x80000, CRC(b6d00245) SHA1(f7da6f2ca681fbe62ea9cab7f92d3e501b7e867d))

	ROM_REGION(0x100000, "user2", 0)
	ROM_LOAD("bkcpu05.bin", 0x00000, 0x20000, CRC(072ce879) SHA1(4f6fb044592feb4c72bbdcbe5f19e063c0e49d0d))
	ROM_LOAD("bkcpu06.bin", 0x20000, 0x20000, CRC(9db436d4) SHA1(3869524c0490e0a019d2f8ab46546ff42727665e))
ROM_END
/*-------------------------------------------------------------------
/ Dona Elvira 2 (1996)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Io Moon (1994)
/-------------------------------------------------------------------*/
ROM_START(iomoon)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("v1_3_01.bin", 0x80000, 0x80000, CRC(df80bf4f) SHA1(29547b444cad116c9dc925d6b3112f584df37250))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("v1_3_05.bin", 0x0000, 0x8000, CRC(6bb5e101) SHA1(125412953bbee7ee171c0bd34f7848fde37ace67))

	ROM_REGION(0x2000, "cpu3", ROMREGION_ERASEFF)

	ROM_REGION(0x100000, "oki", 0)
	ROM_LOAD("v1_3_03.bin", 0x00000, 0x80000, CRC(334d0e20) SHA1(06b38cc7fcee633c45a9000187fcde8d7e03a51f))
	ROM_LOAD("v1_3_04.bin", 0x80000, 0x80000, CRC(f3a950bf) SHA1(e0410f8fe9b4efe7d21052c0a19894a563f90a27))

	ROM_REGION(0x100000, "user2", 0)
	ROM_LOAD("v1_3_02.bin", 0x00000, 0x80000, CRC(2bd589cd) SHA1(87354c76cbef8185d563266230c72a618ce6fcd7))
ROM_END
/*-------------------------------------------------------------------
/ Sleic Pin Ball (1993)
/-------------------------------------------------------------------*/
ROM_START(sleicpin)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("sp03-1_1.rom", 0xe0000, 0x20000, CRC(261b0ae4) SHA1(e7d9d1c2cab7776afb732701b0b8697b62a8d990))

	ROM_REGION(0x8000, "cpu2", 0)
	ROM_LOAD("sp04-1_1.rom", 0x0000, 0x8000, CRC(84514cfa) SHA1(6aa87b86892afa534cf963821f08286c126b4245))

	ROM_REGION(0x2000, "cpu3", 0)
	ROM_LOAD("sp01-1_1.rom", 0x0000, 0x2000, CRC(240015bb) SHA1(0e647718173ad59dafbf3b5bc84bef3c33886e23))

	ROM_REGION(0x100000, "oki", 0)
	ROM_LOAD("sp02-1_1.rom", 0x00000, 0x80000, CRC(0e4851a0) SHA1(0692ee2df0b560e2013db9c03fd27c6eb12e618d))
ROM_END

} // Anonymous namespace

GAME(1992,  bikerace,  0,         sleic,  sleic, sleic_state, empty_init, ROT0, "Sleic", "Bike Race",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  bikerace2, bikerace,  sleic,  sleic, sleic_state, empty_init, ROT0, "Sleic", "Bike Race (2-ball play)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1993,  sleicpin,  0,         sleic,  sleic, sleic_state, empty_init, ROT0, "Sleic", "Sleic Pin Ball",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1994,  iomoon,    0,         sleic,  sleic, sleic_state, empty_init, ROT0, "Sleic", "Io Moon",                 MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
