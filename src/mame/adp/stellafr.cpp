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
#include "sound/dac.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"

#include "stellafr.lh"

namespace {

enum
{
	PORT_I_SDA,
	PORT_I_COIN,
	PORT_I_IP2, //bridged to IP5
	PORT_I_MISO,
	PORT_I_DOOR,
	PORT_I_IP5
};

enum
{
	PORT_O_ALARM,
	PORT_O_EN_COIN,
	PORT_O_EN_SPK,
	PORT_O_PROT_OD,
	PORT_O_SCL,
	PORT_O_LED0,
	PORT_O_SDA,
	PORT_O_SPZ
};

enum
{
	PORT_A_IO0,
	PORT_A_IO1,
	PORT_A_IO2,
	PORT_A_IO3,
	PORT_A_IO4,
	PORT_A_IO5,
	PORT_A_RESET,
	PORT_A_DOOR_SIN
};

enum
{
	PORT_B_IO0,
	PORT_B_IO1,
	PORT_B_DAC,
	PORT_B_COIN_AW,
	PORT_B_RS485_OUT_EN,
	PORT_B_RS485_IN_EN,
	PORT_B_DOOR_SCK,
	PORT_B_DOOR_SOUT
};

// outputs
enum
{
	U1_1MA, //shared with output to service keyboard
	U1_2MA,
	U1_ME,
	U1_D3OUT,
	U1_ANZ1,
	U1_MUX1,
	U1_ANZ2,
	U1_MUX2
};

enum
{
	U5_EN1MA, //shared with output to coin unit 1
	U5_EN2MA,
	U5_AW1,
	U5_AW2,
	U5_ENANZ1, //shared with output to service keyboard
	U5_ENMUX1,
	U5_ENANZ2, //shared with output to coin unit 2
	U5_ENMUX2
};

// inputs
enum
{
	U10_OUTLI,
	U10_OUTEMP,
	U10_OUTMA,
	U10_OUTST,
	U10_OUTT,
	U10_OUTT2,
	U10_EMP2,
	U10_LI2
};

class stellafr_state : public driver_device
{
public:
	stellafr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart"),
		m_nvram(*this, "nvram"),
		m_dac(*this, "dac"),
		m_digits(*this, "digit%u", 0U),
		m_lamps(*this, "lamp%u", 0U),
		m_leds(*this, "led%u", 0U),
		m_in0(*this, "IN0")
	{ }

	void stellafr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<nvram_device> m_nvram;
	required_device<ad7224_device> m_dac;
	output_finder<8> m_digits;
	output_finder<128> m_lamps;
	output_finder<2> m_leds;
	required_ioport m_in0;

	uint8_t m_ma1;
	uint8_t m_ma2;
	uint8_t m_me;
	uint8_t m_data3;
	uint8_t m_anz1;
	uint16_t m_mux1;
	uint8_t m_anz2;
	uint8_t m_mux2;

	uint8_t mux_r();
	void mux_w(uint8_t data);
	void mux2_w(uint8_t data);
	void duart_output_w(uint8_t data);
	void ay8910_portb_w(uint8_t data);
	void lamps_w(uint8_t row, uint16_t data);

	void mem_map(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;

};


uint8_t stellafr_state::mux_r()
{
	bool li = false;
	bool emp = false;
	bool ma = false;
	bool st = false;
	bool t = false; // main buttons in
	bool t2 = false;
	bool emp2 = false;
	bool li2 = false;

	uint8_t data = 0x00;

	if (li)   data |= (1 << U10_OUTLI);
	if (emp)  data |= (1 << U10_OUTEMP);
	if (ma)   data |= (1 << U10_OUTMA);
	if (st)   data |= (1 << U10_OUTST);
	if (t)    data |= (1 << U10_OUTT);
	if (t2)   data |= (1 << U10_OUTT2);
	if (emp2) data |= (1 << U10_EMP2);
	if (li2)  data |= (1 << U10_LI2);

	return data;
}

void stellafr_state::lamps_w(uint8_t row, uint16_t data)
{
	LOG("Row %d\n",row);
	for (int i = 0; i < 8; i++)
	{
		uint8_t lamp_index = (row * 10) + i;
		bool lamp_value = BIT(data, i);
		m_lamps[lamp_index] = lamp_value;
	}
}

void stellafr_state::mux_w(uint8_t data)
{
	bool enma1  = BIT(data,U5_EN1MA);
	bool enma2  = BIT(data,U5_EN2MA);
	bool aw1    = BIT(data,U5_AW1);
	bool aw2    = BIT(data,U5_AW2);
	bool enanz1 = BIT(data,U5_ENANZ1); //enable 7seg
	bool enmux1 = BIT(data,U5_ENMUX1); //enable lamps/buttons
	bool enanz2 = BIT(data,U5_ENANZ2);
	bool enmux2 = BIT(data,U5_ENMUX2);

	if (enma1)
		; // LOG("1MA %d\n",m_ma1);
	if (enma1)
		; // LOG("ME %d\n",m_me);
	if (enma2)
		; // LOG("2MA %d\n",m_ma2);
	if (enanz1)
		; // LOG("ANZ1 %d\n",m_anz1); //main 7seg led out
	if (enanz1)
		; // LOG("ST %d\n",m_ma1);
	if (enmux1)
		lamps_w((m_mux1 >> 12) & 0x07, m_mux1 & 0x0FFF); //main lamps out
	if (enanz2)
		; // LOG("ANZ2 %d\n",m_anz2);
	if (enmux2)
		; // LOG("MUX2 %d\n",m_mux2);
	if (aw1)
		;
	if (aw2)
		;
}

void stellafr_state::mux2_w(uint8_t data)
{
	// anz goes into one 74hc4094
	// mux has 2 chained for lamp cols 0 - 11, 3 bits for lz encoded and EnSDAp
	m_ma1   = (m_ma1   << 1) | BIT(data,U1_1MA);
	m_ma2   = (m_ma2   << 1) | BIT(data,U1_2MA);
	m_me    = (m_me    << 1) | BIT(data,U1_ME);
	m_data3 = (m_data3 << 1) | BIT(data,U1_D3OUT);
	m_anz1  = (m_anz1  << 1) | BIT(data,U1_ANZ1);
	m_mux1  = (m_mux1  << 1) | BIT(data,U1_MUX1);
	m_anz2  = (m_anz2  << 1) | BIT(data,U1_ANZ2);
	m_mux2  = (m_mux2  << 1) | BIT(data,U1_MUX2);
}

void stellafr_state::duart_output_w(uint8_t data)
{
	m_leds[0] = !BIT(data, PORT_O_LED0);
	m_leds[1] = !BIT(data, PORT_O_SDA);
}

void stellafr_state::ay8910_portb_w(uint8_t data)
{
}

void stellafr_state::mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	// controlled by U17 74HC138
	map(0x800001, 0x800001).w(m_dac, FUNC(dac_byte_interface::data_w)); // Y0
	// Y1 device on cpu board
	// Y2 device on cpu board
	map(0x8000c1, 0x8000c1).w(FUNC(stellafr_state::mux2_w)); // Y3 SP/ME II out
	map(0x800100, 0x800101).rw(FUNC(stellafr_state::mux_r), FUNC(stellafr_state::mux_w)); // Y4 SP/ME I out / Inputs
	map(0x800141, 0x800141).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w)); // Y5
	map(0x800143, 0x800143).w("aysnd", FUNC(ay8910_device::data_w)); // Y5
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff); // Y6
	// Y7 NC
	map(0xff0000, 0xffffff).ram().share("nvram");
}

void stellafr_state::fc7_map(address_map &map)
{
	map(0xfffff5, 0xfffff5).r(m_duart, FUNC(mc68681_device::get_irq_vector));
}

void stellafr_state::machine_start()
{
	m_digits.resolve();
	m_lamps.resolve();
	m_leds.resolve();
	save_item(NAME(m_mux1));
}

void stellafr_state::machine_reset()
{
	m_mux1 = 0;
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
	M68000(config, m_maincpu, 10'000'000 ); //?
	m_maincpu->set_addrmap(AS_PROGRAM, &stellafr_state::mem_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &stellafr_state::fc7_map);

	MC68681(config, m_duart, 3'686'400);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2); // ?
	m_duart->outport_cb().set(FUNC(stellafr_state::duart_output_w));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	AD7224(config, m_dac, 0);

	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", 1'000'000));
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

ROM_START( bigjkpot )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "big_jackpot_f1_pr_1.u2", 0x00000, 0x8000, CRC(94a14d8e) SHA1(3c4abdad8e38102278920b0f35a8ab3f7a4f2142) )
	ROM_LOAD16_BYTE( "big_jackpot_f1_pr_2.u6", 0x00001, 0x8000, CRC(51f8ab0b) SHA1(1cb2aa40922956d93605c77862f0fd6f38595eb8) )
ROM_END

ROM_START( dscbonus )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "disc_bonus_f3_1.u2", 0x00000, 0x10000, CRC(6599babf) SHA1(4ba8844ecee15d299e00fff1c5f51d53ce2ccfde) )
	ROM_LOAD16_BYTE( "disc_bonus_f3_2.u6", 0x00001, 0x10000, CRC(6e7fa161) SHA1(7f0e695ede3ba198cc94f80e72c6cbe41468a970) )
ROM_END

ROM_START( dscjkpot )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "disc_jackpot_f2_pr.1.u2", 0x00000, 0x8000, CRC(5af04926) SHA1(7e10ddd1f068565854c245e39f73faf0685e4bf3) )
	ROM_LOAD16_BYTE( "disc_jackpot_f2_pr.2.u6", 0x00001, 0x8000, CRC(95a7f938) SHA1(2d14da419d89fd26ea3245fbe24cafa346fecdca) )
ROM_END

ROM_START( grandhnd )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "grandhand_f2.u2", 0x00000, 0x10000, CRC(367c86f0) SHA1(c4a42887887614f0d4927b5a36a12b7d88a28e32) )
	ROM_LOAD16_BYTE( "grandhand_f2.u6", 0x00001, 0x10000, CRC(b0f14dd4) SHA1(f6a713334ed85ecf52e0671aa15c6c43d32db4d2) )
ROM_END

ROM_START( jkrpoker )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "joker_poker_f1.u2", 0x00000, 0x18000, CRC(9f8ef927) SHA1(9a894e7a9326c9846eabb7b22916244b51c16fd3) )
	ROM_LOAD16_BYTE( "joker_poker_f1.u6", 0x00001, 0x18000, CRC(f53973a1) SHA1(27dabe5e6df6ec03080635da5b68b5a8125e71d1) )
ROM_END

ROM_START( jmbojmbo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jumbo_jumbo_f2_pr1.u2", 0x00000, 0x10000, CRC(97a04942) SHA1(f512451376697e5d3fd18bfadbe6711b9bfeb74b) )
	ROM_LOAD16_BYTE( "jumbo_jumbo_f2_pr2.u6", 0x00001, 0x10000, CRC(35acb575) SHA1(88a7cb6397fe031bda0b7dddd1049fb04eba8b40) )
ROM_END

ROM_START( jmbojmbf )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jumbo_jumbo_fun_f1_pr1.u2", 0x00000, 0x20000, CRC(93c19377) SHA1(72a2455dc968b605c408cf0d5ed36e25ded55085) )
	ROM_LOAD16_BYTE( "jumbo_jumbo_fun_f1_pr2.u6", 0x00001, 0x20000, CRC(be428893) SHA1(273a5339201997b6043992e278f262db28fb3bf9) )
ROM_END

ROM_START( kleoptra )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kleopatra_w4_i.u2", 0x00000, 0x80000, CRC(2035d182) SHA1(683cab310445a6d31f080830a12c07d711119874) )
	ROM_LOAD16_BYTE( "kleopatra_w4_ii.u6", 0x00001, 0x80000, CRC(fdf02576) SHA1(7750ff6f3611b5c6903cdd3c138e34248ba378be) )
ROM_END

ROM_START( multmult )
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

GAMEL(1993, action,   0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Action",                MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, dscbonus, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Disc Bonus",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, grandhnd, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Grand Hand",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, jmbojmbo, jmbojmbf, stellafr, stellafr, stellafr_state, empty_init, ROT0, "Venus",  "Jumbo Jumbo",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, st_vulkn, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Vulkan",                MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, multmult, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Multi Multi",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, dscjkpot, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Disc Jackpot",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, sunny,    0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Sunny",                 MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, bigjkpot, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Big Jackpot",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, jmbojmbf, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Venus",  "Jumbo Jumbo Fun",       MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, st_ohla,  0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Oh La La",              MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, jkrpoker, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Joker Poker (Merkur)",  MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, allfred,  0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Allfred",               MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, taipan,   0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Nova",   "Tai Pan Money",         MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(199?, kleoptra, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Asterix und Kleopatra", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(2001, turbosun, 0,        stellafr, stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Turbo Sunny",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
