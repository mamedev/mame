// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo
/**************************************************************************************

  Megaturbo II?
  TOPDRAW 2OOO?

  Copyright 1992 Premier Technology
  Unknown poker game.

  Preliminary driver by Roberto Fresca & Grull Osgo.


  SPECS:

  1x Intel 80188 PLCC CPU (8 MHz)
  1x Intel P8031AH CPU
  1x National NS16450 UART
  1x Maxim MAX232C (RS-232)

  1x 27C256 (U109, near P8031 CPU)
  3x 27C256 (U17, U18, and U21)

  1x 11.0592 MHz Xtal (near P8031 CPU)
  1x 16.0000 MHz Xtal (near 80188 CPU)
  1x 1.8432 MHz Xtal (near 16450 UART)

  1x 8 DIP switches bank (SW1).


  Sikscreened in PCB:
  Copyright 1992 Premier Technology
  Megaturbo II rev 1

  String inside ROM:
  TOPDRAW 2OOO


  Status:
  - Preliminary

  TODO:
  - A lot...


****************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/ins8250.h"
#include "speaker.h"

namespace {

#define CPU_CLOCK      XTAL(16'000'000)
#define MCU_CLOCK      XTAL(11'059'200)


class megaturbo2_state : public driver_device
{
public:
	megaturbo2_state(const machine_config &mconfig, device_type type, const char *tag):
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu")
	{ }

	void megaturbo2(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;

	void mem_io(address_map &map) ATTR_COLD;
	void mem_prg(address_map &map) ATTR_COLD;

	// i80188
	uint8_t main_io_r(offs_t offset);
	void main_io_w(offs_t offset, uint8_t data);
	uint8_t unk1_r();
	uint8_t m_unk1 = 0;

	// i8031
	uint8_t unk_port_r();
	void unk_port_w(uint8_t data);
};


/*************************************************
*              Read Write Handlers               *
*************************************************/

// i80188
uint8_t megaturbo2_state::unk1_r()
{
	m_unk1 = m_unk1 + 0x40;
	return m_unk1;
}

// i80188
uint8_t megaturbo2_state::main_io_r(offs_t offset)
{
	logerror("main io read %04x\n", offset);
	return 0xff;
}

void megaturbo2_state::main_io_w(offs_t offset, uint8_t data)
{
	logerror("main io write %04x : %02x\n", offset, data);
}


// i8031
uint8_t megaturbo2_state::unk_port_r()
{
	return 0xff;
}

void megaturbo2_state::unk_port_w(uint8_t data)
{
}


/*************************************************
*             Memory map information             *
*************************************************/
// I80188
void megaturbo2_state::main_map(address_map &map)
{
	map(0x00000, 0x03fff).ram();
	map(0x20000, 0x207ff).ram();

	map(0x2c000, 0x2cfff).ram();

	map(0x30400, 0x30fff).ram();  // some device
	map(0x38000, 0x38003).ram();  // some device

	map(0x38300, 0x38300).r(FUNC(megaturbo2_state::unk1_r));

	map(0x48000, 0x48001).ram();  // some device
	map(0xd0000, 0xdffff).rom();  // u17 - u18 ???

	map(0xf0000, 0xf0003).ram();  // some device
	map(0xf8000, 0xfffff).rom();  // u21
}

void megaturbo2_state::main_io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(megaturbo2_state::main_io_r), FUNC(megaturbo2_state::main_io_w));
}

// I8031
void megaturbo2_state::mem_prg(address_map &map)
{
	map(0x0000, 0xffff).rom().region("mcu", 0);
}

void megaturbo2_state::mem_io(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0xa000, 0xa007).rw("uart", FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w));
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe000).rw(FUNC(megaturbo2_state::unk_port_r), FUNC(megaturbo2_state::unk_port_w));
}


/*************************************************
*                  Input ports                   *
*************************************************/

static INPUT_PORTS_START( megaturbo2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP01")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP02")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP03")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP04")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP05")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP06")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP07")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP10")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP12")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP13")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP15")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP16")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP17")

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
INPUT_PORTS_END


/*************************************************
*             Machine Start & Reset              *
*************************************************/

void megaturbo2_state::machine_start()
{
}

void megaturbo2_state::machine_reset()
{
}


/*************************************************
*                Machine Drivers                 *
*************************************************/

void megaturbo2_state::megaturbo2(machine_config &config)
{
	// basic machine hardware
	I80188(config, m_maincpu, CPU_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &megaturbo2_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &megaturbo2_state::main_io_map);

	I8031(config, m_mcu, MCU_CLOCK);
	m_mcu->set_addrmap(AS_PROGRAM, &megaturbo2_state::mem_prg);
	m_mcu->set_addrmap(AS_IO, &megaturbo2_state::mem_io);

	ns16450_device &uart(NS16450(config, "uart", 10_MHz_XTAL / 8));
	uart.out_tx_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.out_rts_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	uart.out_dtr_callback().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("uart", FUNC(ns16450_device::rx_w));
	rs232.cts_handler().set("uart", FUNC(ns16450_device::cts_w));
	rs232.dsr_handler().set("uart", FUNC(ns16450_device::dsr_w));
	rs232.dcd_handler().set("uart", FUNC(ns16450_device::dcd_w));

	SPEAKER(config, "mono").front_center();
}


/*************************************************
*                    ROM Load                    *
*************************************************/

/*-------------------------------------------------------------------
/ Mega Turbo II (1991)
/-------------------------------------------------------------------*/

ROM_START(megaturbo)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("nosticker_27c256.u17", 0xd0000, 0x08000, CRC(2607df4e) SHA1(9fb33c87607085cd51a9c7b43713313356b0042e))
	ROM_LOAD("nosticker_27c256.u18", 0xd8000, 0x08000, CRC(3db346bd) SHA1(1159128f6f1d33547d0cf99c6089f094f711f8b8))
	ROM_LOAD("nosticker_27c256.u21", 0xf8000, 0x08000, CRC(cfdad6e9) SHA1(39a0569ce484af0474cd627387d9d79d6afefe1b))

	ROM_REGION(0x100000, "mcu", 0)
	ROM_LOAD("nosticker_27c256.u109", 0x0000, 0x08000, CRC(2a6c9f63) SHA1(833380d86f6b5267489321cc7d4fb5f1ebb72c1b))
ROM_END

} // Anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME        PARENT   MACHINE      INPUT       CLASS             INIT        ROT   COMPANY               FULLNAME         FLAGS
GAME( 1991, megaturbo,  0,       megaturbo2,  megaturbo2, megaturbo2_state, empty_init, ROT0, "Premier Technology", "Mega Turbo 2",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
