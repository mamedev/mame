// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Stella
German Fruit Machines / Gambling Machines

CPU Board:
----------
 ____________________________________________________________
 |           ______________  ______________     ___________ |
 | 74HC245N  | t1 i       |  |KM681000ALP7|     |+        | |
 | 74HC573   |____________|  |____________|     |  3V Bat | |
 |                                              |         | |
 |           ______________  ______________     |        -| |
 |           | t1 ii      |  |KM681000ALP7|     |_________| |
 |     |||   |____________|  |____________| |||             |
 |     |||   ___________                    |||  M62X42B    |
 | X   |||   |         |                    |||             |
 |     |||   |68EC000 8|  74HC32   74HC245  |||  MAX691CPE  |
 |     |||   |         |  74AC138  74HC573  |||    74HC32   |
 |           |         |                                    |
 | 74HC573   |_________|  74HC08   74HC10  74HC32  74HC21   |
 |__________________________________________________________|

Parts:

 68EC000FN8         - Motorola 68k CPU
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM
 OKIM62X42B         - Real-time Clock ic With Built-in Crystal
 MAX691CPE          - P Reset ic With Watchdog And Battery Switchover
 X                    - 8MHz xtal
 3V Bat             - Lithium 3V power module

Sound  and I/O board:
---------------------
"Steuereinheit 68000"
 _________________________________________________________________________________
 |                        TS271CN    74HC02                        ****  ****    |
 |*         74HC573      ________________                          P1    P2     *|
 |*                      | YM2149F      |                                       *|
 |*         74HC574  ||| |______________|   74HC393  74HC4015 ||| MX7224KN      *|
 |P3                 |||                                      |||              P6|
 |*         74HC245  ||| ________________   3.6864M  74HC125  ||| TL7705ACP     *|
 |*   L4974A         ||| |SCN68681C1N40 |                     |||               *|
 |*                  ||| |______________|   74HC32   74AC138  |||               *|
 |P7                 |||                                      |||              P8|
 |*                        TC428CPA                                             *|
 |*                                                                             *|
 |*    P11  P12    P13    P14       P15   P16   P17      P18   P19   P20  P21   *|
 |P9   **** *****  *****  ****  OO  ****  ****  *******  ****  ****  ***  *** P10|
 |_______________________________________________________________________________|

Parts:

 YM2149F         - Yamaha PSG
 SCN68681C1N40   - Dual Asynchronous Receiver/transmitter (DUART);
 TS271CN         - Programmable Low Power CMOS Single Op-amp
 MX7224KN        - Maxim CMOS 8-bit DAC with Output Amplifier
 TL7705ACP       - Supply Voltage Supervisor
 TC428CPA        - Dual CMOS High-speed Driver
 L4974A          - ST 3.5A Switching Regulator
 OO              - LEDs (red); "Fehlerdiagnose siehe Fehlertable"

Connectors:

 Two connectors to link with Video Board
 P1  - Türöffnungen [1-6]
 P2  - PSG In/Out [1-6]
 P3  - Lautsprecher [1-6]
 P6  - Service - Test Gerät [1-6]
 P7  - Maschine [1-8]
 P8  - Münzeinheit [1-8]
 P9  - Akzeptor [1-4]
 P10 - Fadenfoul [1-4]
 P11 - Netzteil [1-5]
 P12 - Serienplan [1-8]
 P13 - Serienplan 2 [1-8]
 P14 - Münzeinheit 2 [1-8]
 P15 - I2C-Bus [1-4]
 P16 - Kodierg. [1-4]
 P17 - TTL-Ein-/Ausgänge (PSG-Port) [1-10]
 P18 - RS485 Aus [1-2]
 P19 - RS485 Ein [1-2]
 P20 - Serielle-S. [1-5]
 P21 - Türschalter [1-4]


*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "stellafr.lh"

namespace {

class stellafr_state : public driver_device
{
public:
	stellafr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart"),
		m_nvram(*this, "nvram"),
		m_digits(*this, "digit%u", 0U)
	{ }

	void stellafr(machine_config &config);

private:
	void write_8000c1(uint8_t data);
	uint8_t read_800101();
	void write_800101(uint8_t data);
	void duart_output_w(uint8_t data);
	void ay8910_portb_w(uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<nvram_device> m_nvram;
	output_finder<8> m_digits;
};


void stellafr_state::write_8000c1(uint8_t data)
{
}

uint8_t stellafr_state::read_800101()
{
	return 0xff;
}

void stellafr_state::write_800101(uint8_t data)
{
}

void stellafr_state::duart_output_w(uint8_t data)
{
}

void stellafr_state::ay8910_portb_w(uint8_t data)
{
}



void stellafr_state::mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x8000c1, 0x8000c1).w(FUNC(stellafr_state::write_8000c1));
	map(0x800101, 0x800101).rw(FUNC(stellafr_state::read_800101), FUNC(stellafr_state::write_800101));
	map(0x800141, 0x800141).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x800143, 0x800143).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xff0000, 0xffffff).ram().share("nvram");
}

void stellafr_state::fc7_map(address_map &map)
{
	map(0xfffff5, 0xfffff5).r(m_duart, FUNC(mc68681_device::get_irq_vector));
}


static INPUT_PORTS_START( stellafr )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) // Left
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW ) // Right
INPUT_PORTS_END


void stellafr_state::stellafr(machine_config &config)
{
	M68000(config, m_maincpu, 10000000 ); //?
	m_maincpu->set_addrmap(AS_PROGRAM, &stellafr_state::mem_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &stellafr_state::fc7_map);

	MC68681(config, m_duart, 3686400);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2); // ?
	m_duart->outport_cb().set(FUNC(stellafr_state::duart_output_w));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", 1000000));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.85);
	aysnd.port_a_read_callback().set_ioport("IN0");
	aysnd.port_b_write_callback().set(FUNC(stellafr_state::ay8910_portb_w));
}

ROM_START( action )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "action_f2_i.u2", 0x00000, 0x10000, CRC(5ebc8fab) SHA1(3a1e9cfab91af6c1096e464777d12b60d2ab7fb8) )
	ROM_LOAD16_BYTE( "action_f2_ii.u6", 0x00001, 0x10000, CRC(6f1634cc) SHA1(ad0f3d5d43705c5c3e8bc01a87e8ac328862e277) )
ROM_END

ROM_START( allfred )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "allfred_w3_i.u2", 0x00000, 0x80000, CRC(f03bdbef) SHA1(8cd32d80d03842d72b096b469a0ec1be5958a6e4) )
	ROM_LOAD16_BYTE( "allfred_w3_ii.u6", 0x00001, 0x80000, CRC(2f216373) SHA1(71d713b267c21dc0a4e955f422e7102553d16d30) )
ROM_END

ROM_START( grandhand )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grandhand_f2.u2", 0x00000, 0x10000, CRC(367c86f0) SHA1(c4a42887887614f0d4927b5a36a12b7d88a28e32) )
	ROM_LOAD16_BYTE( "grandhand_f2.u6", 0x00001, 0x10000, CRC(b0f14dd4) SHA1(f6a713334ed85ecf52e0671aa15c6c43d32db4d2) )
ROM_END

ROM_START( kleopatra )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kleopatra_w4_i.u2", 0x00000, 0x80000, CRC(2035d182) SHA1(683cab310445a6d31f080830a12c07d711119874) )
	ROM_LOAD16_BYTE( "kleopatra_w4_ii.u6", 0x00001, 0x80000, CRC(fdf02576) SHA1(7750ff6f3611b5c6903cdd3c138e34248ba378be) )
ROM_END

ROM_START( multimulti )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "multi_multi_f1_i.u2", 0x00000, 0x20000, CRC(6aa663af) SHA1(cfcdf930fa26c06e49b241dbcb520c0c64cc8af0) )
	ROM_LOAD16_BYTE( "multi_multi_f1_ii.u6", 0x00001, 0x20000, CRC(a7a5ac70) SHA1(38fd3ad4306aa46a1a9414b3ae3d0691c67f0357) )
ROM_END

ROM_START( st_ohla )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "oh_la_la_f1_1.bin", 0x00000, 0x10000, CRC(94583885) SHA1(5083d65da0347a37ffbb537f94d3b247241f1e8c) )
	ROM_LOAD16_BYTE( "oh_la_la_f1_2.bin", 0x00001, 0x10000, CRC(8ac647cd) SHA1(858f67d6121dde28477a5df8569e7ae92db6299e) )
ROM_END

ROM_START( st_vulkn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vulkan_f1_1.bin", 0x00000, 0x10000, CRC(06109bd5) SHA1(78f6b0cb3ae5873350fd50af8990fa38454c1183) )
	ROM_LOAD16_BYTE( "vulkan_f1_2.bin", 0x00001, 0x10000, CRC(951baf42) SHA1(1346043155ba85926b2bf9eef8136b377953abe1) )
ROM_END

ROM_START( taipan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "taipan_w1_i.u2", 0x00000, 0x20000, CRC(feaf45f9) SHA1(ded06e9536aa69d17a1f6dcd2b84f7ecaed7ad18) )
	ROM_LOAD16_BYTE( "taipan_w1_ii.u6", 0x00001, 0x20000, CRC(b2c5418a) SHA1(23c542b983325e677cdd9728bb2fce9263793098) )
ROM_END

ROM_START( turbosun )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "turbo_sunny_f1_i.u2", 0x00000, 0x20000, CRC(763c00e7) SHA1(8bae5206a3ebad6ec552a9714242cebc78819251) )
	ROM_LOAD16_BYTE( "turbo_sunny_f1_ii.u6", 0x00001, 0x20000, CRC(4d431ae3) SHA1(bb5ff763b9bbaf4eb15ec3fde643b601421fbde1) )
ROM_END

ROM_START( sunny )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sunny_f2_i.u2", 0x00000, 0x20000, CRC(49776821) SHA1(d68a9e86ea336c46cc07d7bf6ecc3632930f18b9) )
	ROM_LOAD16_BYTE( "sunny_f2_ii.u6", 0x00001, 0x20000, CRC(86b3b81d) SHA1(e12a511bbc53e4614bed561c9544f9ac8faa9fd2) )
ROM_END

} // anonymous namespace

GAMEL(1993, action,    0,      stellafr, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Action",                MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, grandhand, action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Grand Hand",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, st_vulkn,  action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Vulkan",                MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, multimulti,action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Multi Multi",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, sunny,     action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Sunny",                 MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, st_ohla,   action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Oh La La",              MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, allfred,   action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Allfred",               MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, taipan,    action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Nova",   "Tai Pan Money",         MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(199?, kleopatra, action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Asterix und Kleopatra", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(2001, turbosun,  action, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Turbo Sunny",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
