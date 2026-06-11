// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Stella
German Fruit Machines / Gambling Machines

CPU Board:
----------
 ____________________________________________________________
 |           ______________  ______________     ___________ |
 | 74HC245N  | t1 i       |  |NVRAM       |     |+        | |
 | 74HC573   |____________|  |____________|     |  3V Bat | |
 |                                              |         | |
 |           ______________  ______________     |        -| |
 |           | t1 ii      |  |NVRAM       |     |_________| |
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
 NVRAM              - Either KM681000ALP7 128K X 8 Bit Low Power CMOS Static RAM or ST M48T08 timekeeper
 OKIM62X42B         - Real-time Clock ic With Built-in Crystal
 MAX691CPE          - P Reset ic With Watchdog And Battery Switchover
 X                  - 12/8MHz xtal
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
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/timekpr.h"
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
		m_printer(*this, "printer"),
		m_ym2149(*this, "ym2149"),
		m_dac(*this, "dac"),
		m_speaker(*this, "speaker"),
		m_digits(*this, "digit%02u", 0U),
		m_coinled(*this, "anzled%u", 0U),
		m_magnet(*this, "magnet%u", 0U),
		m_lamps(*this, "lamp%u", 0U),
		m_leds(*this, "led%u", 0U),
		m_in0(*this, "IN0"),
		m_door(*this, "DOOR")
	{ }

	void sus_tk(machine_config &config) ATTR_COLD;
	void sus_rtc(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<rs232_port_device> m_printer;
	required_device<ym2149_device> m_ym2149;
	required_device<ad7224_device> m_dac;
	required_device<speaker_device> m_speaker;
	output_finder<74> m_digits;
	output_finder<5> m_coinled;
	output_finder<2> m_magnet;
	output_finder<128> m_lamps;
	output_finder<2> m_leds;
	required_ioport m_in0;
	required_ioport m_door;

	uint8_t m_ma1;
	uint8_t m_ma2;
	uint8_t m_me;
	uint8_t m_data3;
	uint8_t m_anz1;
	uint16_t m_mux1;
	uint8_t m_anz2;
	uint8_t m_mux2;
	uint8_t m_strobe;

	uint16_t m_anz1_bank[8];
	uint8_t m_anz_bank;      // which of the two ANZ banks per scan step (0/1)
	uint8_t m_anz_cycle;     // module pair: 0 = modules 0/1, 1 = modules 2/3
	uint8_t m_anz_prevpos;   // previous step (to detect wrap)

	uint8_t mux_r();
	uint8_t duart_input_r();
	void enable_w(uint8_t data);
	void mux_w(uint8_t data);
	void duart_output_w(uint8_t data);
	void ym2149_portb_w(uint8_t data);
	void lamps_w(bool second);
	void anzeigen_w();
	void anzout_digit_w(int anzout, int even_field);
	void anzout_aux(int even_field);
	void service_w();
	static uint8_t digit_map(int field, uint8_t s);

	void mem_map_steuereinheit(address_map &map) ATTR_COLD;
	void mem_map_tk(address_map &map) ATTR_COLD;
	void mem_map_rtc(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;

	void steuereinheit(machine_config &config) ATTR_COLD;
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

void stellafr_state::lamps_w(bool second)
{
	uint8_t row = (m_mux1 >> 12) & 0x07;
	uint16_t data = m_mux1 & 0x0fff;

	LOG("Row %d\n",row);
	for (int i = 0; i < 8; i++)
	{
		uint8_t lamp_index = (row * 10) + i;
		bool lamp_value = BIT(data, i);
		m_lamps[lamp_index] = lamp_value;
	}
}

uint8_t stellafr_state::digit_map(int field, uint8_t s)
{
	// they switched up the segment wiring between the shift register and the display
	if (field & 1)
		return bitswap<8>(s, 0, 4, 1, 6, 5, 7, 3, 2); // digits 1 & 3
	else
		return bitswap<8>(s, 7, 3, 4, 2, 1, 0, 6, 5); // digits 0 & 2
}

// Each Anzout chain is one or two 74HC4094s holding two modules: the even field of the pair
// just latched lives in the low 4094 (segment s in bit s), the odd field in the
// high 4094.  Only that pair is updated; the other two fields belong to the
// other module pair and are written on its strobe.
void stellafr_state::anzout_digit_w(int anzout, int even_field)
{
	int const odd_field = even_field | 1;
	uint16_t const v = m_anz1_bank[anzout];
	m_digits[anzout * 10 + even_field] = digit_map(even_field, v & 0xff) & 0x7f;
	m_digits[anzout * 10 + odd_field]  = digit_map(odd_field, v >> 8) & 0x7f;
}

// Anzout4 chain, only one 4094: coin-accept LEDs + magnets.
void stellafr_state::anzout_aux(int even_field)
{
	if (even_field != 2)
		return;
	uint8_t const aux = m_anz1_bank[4] >> 8;
	m_coinled[0] = BIT(aux, 0); // 0,10 DM
	m_coinled[1] = BIT(aux, 1); // 1 DM
	m_coinled[2] = BIT(aux, 2); // 2 DM
	m_coinled[3] = BIT(aux, 3); // 5 DM
	m_magnet[0] = BIT(aux, 4); // Magnet L
	m_magnet[1] = BIT(aux, 5); // Magnet R
	// bit 6 = NC
	m_coinled[4] = BIT(aux, 7); // Freispiele
}

void stellafr_state::anzeigen_w()
{
	// clock one bit downstream
	for (uint8_t i = 0; i < 8; i++)
		m_anz1_bank[i] = (m_anz1_bank[i] >> 1) | (BIT(m_anz1, i) << 15);

	// which module/segment this strobe belongs to, derived from the MUX1 "lz"
	// select alone: after bank 0 it sits in bits 4-7, after bank 1 in bits 12-15.
	int const sel = m_anz_bank ? ((m_mux1 >> 12) & 0x0f) : ((m_mux1 >> 4) & 0x0f);
	int const pos = 7 - (sel & 0x07);
	if (m_anz_bank == 0)
	{
		if (pos == 0 && m_anz_prevpos == 7) // select wrap 7->0: next module pair
			m_anz_cycle ^= 1;
		m_anz_prevpos = pos;
	}

	// either U6A or U9 QP7 triggering the enable
	if (pos == 7 && m_anz_bank == 1)
	{
		int const even_field = m_anz_cycle << 1;

		anzout_digit_w(0, even_field); // 150er
		anzout_digit_w(1, even_field); // usually NC
		anzout_digit_w(2, even_field); // usually NC
		anzout_digit_w(3, even_field); // Serienspeicher
		anzout_aux(even_field); // coin-accept LEDs + magnets (Anzout4 high 4094)
		anzout_digit_w(5, even_field); // Munzspeicher
		anzout_digit_w(6, even_field); // usually NC
		anzout_digit_w(7, even_field); // usually NC
	}

	m_anz_bank ^= 1; // the two banks alternate, reset by ENMUX down
}

void stellafr_state::service_w()
{
	; // LOG("Service %d\n", m_ma1);
}

void stellafr_state::enable_w(uint8_t data)
{
	// ENMUX falls just before the first ANZ bank of each scan step, so its
	// falling edge re-aligns the ANZ bank counter
	if (!BIT(data, U5_ENMUX1) && BIT(m_strobe, U5_ENMUX1))
		m_anz_bank = 0;

	if (BIT(data, U5_ENMUX1) && !BIT(m_strobe, U5_ENMUX1))
		lamps_w(false); // P12

	if (BIT(data, U5_EN1MA) && !BIT(m_strobe, U5_EN1MA))
		; // LOG("1MA %d / ME %d\n", m_ma1, m_me); // P7

	if (BIT(data, U5_EN2MA) && !BIT(m_strobe, U5_EN2MA))
		; // LOG("2MA %d\n", m_ma2); // P7

	if (BIT(data, U5_ENANZ1) && !BIT(m_strobe, U5_ENANZ1))
	{
		anzeigen_w(); // P12
		service_w();  // P6
	}

	if (BIT(data, U5_ENANZ2) && !BIT(m_strobe, U5_ENANZ2))
		; // anzeigen_w(true); // P13

	if (BIT(data, U5_ENMUX2) && !BIT(m_strobe, U5_ENMUX2))
		; // lamps_w(true); // P13

	if (BIT(data, U5_AW1) && !BIT(m_strobe, U5_AW1))
		; // P8

	if (BIT(data, U5_AW2) && !BIT(m_strobe, U5_AW2))
		; // P14

	m_strobe = data;
}

void stellafr_state::mux_w(uint8_t data)
{
	// anz goes into one 74hc4094
	// mux has 2 chained 74hc4094 for lamp cols 0 - 11, 3 bits for lz encoded and EnSDAp
	m_ma1   = (m_ma1   << 1) | BIT(data,U1_1MA);
	m_ma2   = (m_ma2   << 1) | BIT(data,U1_2MA);
	m_me    = (m_me    << 1) | BIT(data,U1_ME);
	m_data3 = (m_data3 << 1) | BIT(data,U1_D3OUT);
	m_anz1  = (m_anz1  << 1) | BIT(data,U1_ANZ1);
	m_mux1  = (m_mux1  << 1) | BIT(data,U1_MUX1);
	m_anz2  = (m_anz2  << 1) | BIT(data,U1_ANZ2);
	m_mux2  = (m_mux2  << 1) | BIT(data,U1_MUX2);
}

uint8_t stellafr_state::duart_input_r()
{
	uint8_t data = 0x00;

	if (BIT(m_door->read(), 0))
		data |= (1 << PORT_I_DOOR);

	return data;
}

void stellafr_state::duart_output_w(uint8_t data)
{
	m_speaker->set_input_gain(0, !BIT(data, PORT_O_EN_SPK) ? 0.9 : 0.0);
	m_leds[0] = !BIT(data, PORT_O_LED0);
	m_leds[1] = !BIT(data, PORT_O_SDA);
}

void stellafr_state::ym2149_portb_w(uint8_t data)
{
	//TODO

	//PORT_B
}

void stellafr_state::mem_map_steuereinheit(address_map &map)
{
	// controlled by U17 74HC138
	map(0x800001, 0x800001).w(m_dac, FUNC(dac_byte_interface::data_w)); // Y0
	// Y1 device on cpu board
	// Y2 graphics board
	map(0x8000c1, 0x8000c1).w(FUNC(stellafr_state::enable_w)); // Y3 En out
	map(0x800100, 0x800101).rw(FUNC(stellafr_state::mux_r), FUNC(stellafr_state::mux_w)); // Y4 SP/ME out / Inputs
	map(0x800141, 0x800141).rw(m_ym2149, FUNC(ym2149_device::data_r), FUNC(ym2149_device::address_w)); // Y5
	map(0x800143, 0x800143).w(m_ym2149, FUNC(ym2149_device::data_w)); // Y5
	map(0x800180, 0x80019f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff); // Y6
	// Y7 NC
}

void stellafr_state::mem_map_tk(address_map &map)
{
	mem_map_steuereinheit(map);
	map(0x000000, 0x0fffff).rom();
	map(0xff0000, 0xffffff).ram().share("timekeeper").umask16(0x00ff);
	map(0xff0000, 0xffffff).ram().share("zeropower").umask16(0xff00);
}

void stellafr_state::mem_map_rtc(address_map &map)
{
	mem_map_steuereinheit(map);
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40001f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff);
	map(0xfc0000, 0xffffff).ram().share("nvram");
}

void stellafr_state::fc7_map(address_map &map)
{
	map(0xfffff5, 0xfffff5).r(m_duart, FUNC(mc68681_device::get_irq_vector));
}

void stellafr_state::machine_start()
{
	save_item(NAME(m_mux1));
	save_item(NAME(m_strobe));
	save_item(NAME(m_anz_bank));
	save_item(NAME(m_anz_cycle));
	save_item(NAME(m_anz_prevpos));
}

void stellafr_state::machine_reset()
{
	m_mux1 = 0;
	m_strobe = 0;
	m_anz_bank = 0;
	m_anz_cycle = 0;
	m_anz_prevpos = 0;
}

static INPUT_PORTS_START( stellafr )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) // Left
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW ) // Right

	PORT_START("DOOR")
	PORT_CONFNAME( 0x01, 0x00, "Door" ) // P21 - Türschalter
	PORT_CONFSETTING(    0x00, "Closed" )
	PORT_CONFSETTING(    0x01, "Open" )
INPUT_PORTS_END

void stellafr_state::steuereinheit(machine_config &config)
{
	MC68681(config, m_duart, 3'686'400);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2); // ?
	m_duart->inport_cb().set(FUNC(stellafr_state::duart_input_r));
	m_duart->outport_cb().set(FUNC(stellafr_state::duart_output_w));

	// P20 - Serielle-Schnittstelle for printer
	m_duart->a_tx_cb().set(m_printer, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_printer, default_rs232_devices, "printer");
	m_printer->rxd_handler().set(m_duart, FUNC(mc68681_device::rx_a_w));

	// RS485 (P18 - RS485 Aus, P19 - RS485 Ein)
	// m_duart->b_tx_cb().set(m_rs485, ???);
	// m_rs485->rxd_handler().set(m_duart, FUNC(mc68681_device::rx_b_w));

	AD7224(config, m_dac, 0);

	SPEAKER(config, m_speaker).front_center();
	YM2149(config, m_ym2149, 3'686'400 / 2);
	m_ym2149->add_route(ALL_OUTPUTS, m_speaker, 0.9);
	m_ym2149->port_a_read_callback().set_ioport("IN0");
	m_ym2149->port_b_write_callback().set(FUNC(stellafr_state::ym2149_portb_w));
}

void stellafr_state::sus_tk(machine_config &config)
{
	steuereinheit(config);

	M68000(config, m_maincpu, 8'000'000 );
	m_maincpu->set_addrmap(AS_PROGRAM, &stellafr_state::mem_map_tk);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &stellafr_state::fc7_map);

	MK48T08(config, "timekeeper");
	MK48T08(config, "zeropower");
}

void stellafr_state::sus_rtc(machine_config &config)
{
	steuereinheit(config);

	M68000(config, m_maincpu, 12'000'000 );
	m_maincpu->set_addrmap(AS_PROGRAM, &stellafr_state::mem_map_rtc);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &stellafr_state::fc7_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	MSM6242(config, "rtc", XTAL(32'768));
}

ROM_START( actionf2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "action_f2_i.u2", 0x00000, 0x10000, CRC(5ebc8fab) SHA1(3a1e9cfab91af6c1096e464777d12b60d2ab7fb8) )
	ROM_LOAD16_BYTE( "action_f2_ii.u6", 0x00001, 0x10000, CRC(6f1634cc) SHA1(ad0f3d5d43705c5c3e8bc01a87e8ac328862e277) )
ROM_END

ROM_START( allfred )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "allfred_w3_i.u2", 0x00000, 0x80000, CRC(f03bdbef) SHA1(8cd32d80d03842d72b096b469a0ec1be5958a6e4) )
	ROM_LOAD16_BYTE( "allfred_w3_ii.u6", 0x00001, 0x80000, CRC(2f216373) SHA1(71d713b267c21dc0a4e955f422e7102553d16d30) )
ROM_END

ROM_START( bigjkpot )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "big_jackpot_f1_pr_1.u2", 0x00000, 0x8000, CRC(94a14d8e) SHA1(3c4abdad8e38102278920b0f35a8ab3f7a4f2142) )
	ROM_LOAD16_BYTE( "big_jackpot_f1_pr_2.u6", 0x00001, 0x8000, CRC(51f8ab0b) SHA1(1cb2aa40922956d93605c77862f0fd6f38595eb8) )
ROM_END

ROM_START( bigwinnr )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE("big_winner_f1_1_m27c1001.u2", 0x00000, 0x20000, CRC(3abc347b) SHA1(7f6c570cecdab8e7db070c744b9222f725c7af66))
	ROM_LOAD16_BYTE("big_winner_f1_2_m27c1001.u6", 0x00001, 0x20000, CRC(7c4f8a70) SHA1(da797544f897ce8ebbc4c3c5277a6fe83c274a9a))
ROM_END

ROM_START( cstliiw1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE("castello_ii_w1_i.u2", 0x00000, 0x20000, CRC(0e6602aa) SHA1(618e1cb17a0e238d04b7c9cf099f926744d5206b))
	ROM_LOAD16_BYTE("castello_ii_w1_ii.u6", 0x00001, 0x20000, CRC(ea75c597) SHA1(57348b0f39376c4dcfcc7822adf105e862fadf9c))
ROM_END

ROM_START( disc4000 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "disc_4000_f2_1.u2", 0x00000, 0x10000, CRC(ca766804) SHA1(53d338084fe4d3a0d9dd6bd5f6a5541aafc50037) )
	ROM_LOAD16_BYTE( "disc_4000_f2_2.u6", 0x00001, 0x10000, CRC(ae772983) SHA1(2c9cebbe7a2d1503742126112b685ba5826cc6df) )
ROM_END

ROM_START( discfun )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "disc_fun_f1_1.u2", 0x00000, 0x80000, CRC(7c2f9eff) SHA1(82d4511b039277784426923574407a0ab0155047) )
	ROM_LOAD16_BYTE( "disc_fun_f1_2.u6", 0x00001, 0x80000, CRC(873cb431) SHA1(dac69324c0a60a790d47d19af3288329656c3afe) )
ROM_END

ROM_START( dpplson )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "doppelsonne_f2_1.u2", 0x00000, 0x20000, CRC(4dcdc269) SHA1(05182ba671890a102184e3dadc53d4aca267db9d) )
	ROM_LOAD16_BYTE( "doppelsonne_f2_2.u6", 0x00001, 0x20000, CRC(9d024a01) SHA1(fd674ad65d5a202504e619ae37dd1c2a1792b9f2) )
ROM_END

ROM_START( dscbonus )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "disc_bonus_f3_1.u2", 0x00000, 0x10000, CRC(6599babf) SHA1(4ba8844ecee15d299e00fff1c5f51d53ce2ccfde) )
	ROM_LOAD16_BYTE( "disc_bonus_f3_2.u6", 0x00001, 0x10000, CRC(6e7fa161) SHA1(7f0e695ede3ba198cc94f80e72c6cbe41468a970) )
ROM_END

ROM_START( dscjkpot )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "disc_jackpot_f2_pr.1.u2", 0x00000, 0x8000, CRC(5af04926) SHA1(7e10ddd1f068565854c245e39f73faf0685e4bf3) )
	ROM_LOAD16_BYTE( "disc_jackpot_f2_pr.2.u6", 0x00001, 0x8000, CRC(95a7f938) SHA1(2d14da419d89fd26ea3245fbe24cafa346fecdca) )
ROM_END

ROM_START( gjubil )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "m27c1001_goldenjubilar_f1_1.u2", 0x00000, 0x20000, CRC(5babe5f7) SHA1(75e99203e86c8977fe8363c17dab8324133fe0a8) )
	ROM_LOAD16_BYTE( "m27c1001_goldenjubilar_f1_2.u6", 0x00001, 0x20000, CRC(25b8691f) SHA1(f28cd4523b387880175b66ee71276f977293b06d) )
ROM_END

ROM_START( glksstrn )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "glucksstern.u2", 0x00000, 0x20000, CRC(8e969bae) SHA1(bf66d491932b77dab4c6b15ec7fbf470223636ac) )
	ROM_LOAD16_BYTE( "glucksstern.u6", 0x00001, 0x20000, CRC(f31b860a) SHA1(7b016bb7d0699cfe7165c0abb2c1bbcb944cdc86) )
ROM_END

ROM_START( grandhnd )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "grandhand_f2.u2", 0x00000, 0x10000, CRC(367c86f0) SHA1(c4a42887887614f0d4927b5a36a12b7d88a28e32) )
	ROM_LOAD16_BYTE( "grandhand_f2.u6", 0x00001, 0x10000, CRC(b0f14dd4) SHA1(f6a713334ed85ecf52e0671aa15c6c43d32db4d2) )
ROM_END

ROM_START( grnadaf1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "granada_i_f1.u2", 0x00000, 0x20000, CRC(45d080b8) SHA1(1543ee5bb3f0d490744d0d0df852914797902198) )
	ROM_LOAD16_BYTE( "granada_ii_f1.u6", 0x00001, 0x20000, CRC(8016fc9b) SHA1(3c438701bdb221ab373f3356d95911d8a6568d9a) )
ROM_END

ROM_START( grnadaw1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "granada_i_w1.u2", 0x00000, 0x20000, CRC(d87abd91) SHA1(153df930b9ef70a64bdf34b6f53016a52d355844) )
	ROM_LOAD16_BYTE( "granada_ii_w1.u6", 0x00001, 0x20000, CRC(61194912) SHA1(d6aca706a5b69401dfad66130361751b5db95c89) )
ROM_END

ROM_START( jack4000 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "m27c512_jackpot4000_f2_1.u2", 0x00000, 0x10000, CRC(5abc4d8c) SHA1(87ca16fd23724629b7882068475fe0d479f885bd) )
	ROM_LOAD16_BYTE( "m27c512_jackpot4000_f2_2.u6", 0x00001, 0x10000, CRC(efcb85e4) SHA1(b6de892cf097fd027a3d7c4bf2d00912fe3e97aa) )
ROM_END

ROM_START( jkrpoker )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "joker_poker_f1.u2", 0x00000, 0x18000, CRC(9f8ef927) SHA1(9a894e7a9326c9846eabb7b22916244b51c16fd3) )
	ROM_LOAD16_BYTE( "joker_poker_f1.u6", 0x00001, 0x18000, CRC(f53973a1) SHA1(27dabe5e6df6ec03080635da5b68b5a8125e71d1) )
ROM_END

ROM_START( jmbojmbo )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jumbo_jumbo_f2_pr1.u2", 0x00000, 0x10000, CRC(97a04942) SHA1(f512451376697e5d3fd18bfadbe6711b9bfeb74b) )
	ROM_LOAD16_BYTE( "jumbo_jumbo_f2_pr2.u6", 0x00001, 0x10000, CRC(35acb575) SHA1(88a7cb6397fe031bda0b7dddd1049fb04eba8b40) )
ROM_END

ROM_START( jmbojmbf )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jumbo_jumbo_fun_f1_pr1.u2", 0x00000, 0x20000, CRC(93c19377) SHA1(72a2455dc968b605c408cf0d5ed36e25ded55085) )
	ROM_LOAD16_BYTE( "jumbo_jumbo_fun_f1_pr2.u6", 0x00001, 0x20000, CRC(be428893) SHA1(273a5339201997b6043992e278f262db28fb3bf9) )
ROM_END

ROM_START( jumbo400 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jumbo_400_f3_i.u2", 0x00000, 0x10000, CRC(c2e57b9c) SHA1(7f9b39f2c07dcbe0fbef6c23c57a55c77041d2ee) )
	ROM_LOAD16_BYTE( "jumbo_400_f3_ii.u6", 0x00001, 0x10000, CRC(6a9849fb) SHA1(c8464a8c7c4342e218ca6e9be1c6523f231c60c2) )
ROM_END

ROM_START( jumboa )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jumbo_action_f2_1.u2", 0x00000, 0x20000, CRC(d775b564) SHA1(8f6e2a0cf55114689e4fca28fadc3536e8d7967b) )
	ROM_LOAD16_BYTE( "jumbo_action_f2_2.u6", 0x00001, 0x20000, CRC(d8f0f7ee) SHA1(5bd4d603c84e7b5ffa5b4a8c9f9271b7ee28f52b) )
ROM_END

ROM_START( kleoptra )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "kleopatra_w4_i.u2", 0x00000, 0x80000, CRC(2035d182) SHA1(683cab310445a6d31f080830a12c07d711119874) )
	ROM_LOAD16_BYTE( "kleopatra_w4_ii.u6", 0x00001, 0x80000, CRC(fdf02576) SHA1(7750ff6f3611b5c6903cdd3c138e34248ba378be) )
ROM_END

ROM_START( kometf1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "komet_f1_pr1.u2", 0x00000, 0x20000, CRC(c0328d5d) SHA1(ebb934f72e2bf275717d47b72aeffe4b38136622) )
	ROM_LOAD16_BYTE( "komet_f1_pr2.u6", 0x00001, 0x20000, CRC(86715c37) SHA1(cb986eb50d215294375450c651043e617f66fe21) )
ROM_END

ROM_START( m55f1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mega_fifty_five_i_f1.u2", 0x00000, 0x10000, CRC(03d66f2a) SHA1(e1ad377c47be1c1969a6ac7d0e9cb515bb51496b) )
	ROM_LOAD16_BYTE( "mega_fifty_five_ii_f1.u6", 0x00001, 0x10000, CRC(2e63e382) SHA1(b6e666b289a4b2ae0bc90a14dbf55a50bf02c12d) )
ROM_END

ROM_START( mdouble )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "double_f4_1.u2", 0x00000, 0x10000, CRC(0b84c92f) SHA1(818f21d43571a214862e84efd3ee083515ba5860) )
	ROM_LOAD16_BYTE( "double_f4_2.u6", 0x00001, 0x10000, CRC(b1f6c974) SHA1(db8622f2109ebb2ca0797e8228519cf0f80a41be) )
ROM_END

ROM_START( moneyf1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "m27c1001_money_f1_i.u2", 0x00000, 0x20000, CRC(5ca79bfa) SHA1(c52b7d2ecd649ccde3457bc922ad05e734fba862) )
	ROM_LOAD16_BYTE( "m27c1001_money_f1_ii.u6", 0x00001, 0x20000, CRC(ad95ffc4) SHA1(1062be41a3822efd1d4c848c8ec50488dde42a78) )
ROM_END

ROM_START( mptschf2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "monopoly_bistrotisch_f2_i.u2", 0x00000, 0x10000, CRC(e481529f) SHA1(101a596222b1458481024fdc579b7909d22e8234) )
	ROM_LOAD16_BYTE( "monopoly_bistrotisch_f2_ii.u6", 0x00001, 0x10000, CRC(1a3d51c1) SHA1(e3cf1079210bbcb1363b55e08ac9352bfefcbee1) )
ROM_END

ROM_START( multif2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "multi_f2_i.u2", 0x00000, 0x10000, CRC(26908481) SHA1(4efedc5b78e877197a5a8c5b358d1918ba031dbc) )
	ROM_LOAD16_BYTE( "multi_f2_ii.u6", 0x00001, 0x10000, CRC(16fdaadd) SHA1(868a22d5717b287a8d30b340770b5535bc707221) )
ROM_END

ROM_START( multmult )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "multi_multi_f1_i.u2", 0x00000, 0x20000, CRC(6aa663af) SHA1(cfcdf930fa26c06e49b241dbcb520c0c64cc8af0) )
	ROM_LOAD16_BYTE( "multi_multi_f1_ii.u6", 0x00001, 0x20000, CRC(a7a5ac70) SHA1(38fd3ad4306aa46a1a9414b3ae3d0691c67f0357) )

	ROM_REGION( 0x800000, "dsp", 0 )
	ROM_LOAD( "js28f640.bin", 0x00000, 0x800000, CRC(b71f87da) SHA1(b8adb307655fb0a79ee0ad4521351c237c3ff161) )
ROM_END

ROM_START( multprim )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "m27c1001_multi_prima_f1_i.u2", 0x00000, 0x20000, CRC(b873f300) SHA1(4c605ea6337ab3c8eed818b23a9d9979a5047750) )
	ROM_LOAD16_BYTE( "m27c1001_multi_prima_f1_ii.u6", 0x00001, 0x20000, CRC(2a4e371c) SHA1(02494cab0ff7e4c169383a335169edf76501ed1a) )
ROM_END

ROM_START( multstar )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "multi_star_f2_pr_1.u2", 0x00000, 0x20000, CRC(8f63fd16) SHA1(d9b8ef2e1f7616bc0569768af26794fa6166cc0d) )
	ROM_LOAD16_BYTE( "multi_star_f2_pr_2.u6", 0x00001, 0x20000, CRC(26952bd7) SHA1(20de0d890a91e728776278fb9c8d6de3b65f8703) )
ROM_END

ROM_START( mystjack )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mystery_jackpot_f4_i.u2", 0x00000, 0x80000, CRC(4ff3ddd7) SHA1(6765628c4858ba78898bad5d3aed6e2e8e651264) )
	ROM_LOAD16_BYTE( "mystery_jackpot_f4_ii.u6", 0x00001, 0x80000, CRC(96523e1a) SHA1(4f53b454790dab4ce45b1db78bd033c40ce47179) )
ROM_END

ROM_START( ndiscow1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nova_disco_w1_i.u2", 0x00000, 0x20000, CRC(790d164b) SHA1(65cf6e6e712818ff12985833b49c1dd9a3e5149f) )
	ROM_LOAD16_BYTE( "nova_disco_w1_ii.u6", 0x00001, 0x20000, CRC(0204a2b3) SHA1(a434c61237d73a83e11f1a51e7fccc9e3cf3c4e0) )
ROM_END

ROM_START( nunran )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nun_ran_f1_nr1.u2", 0x00000, 0x20000, CRC(d8eed6de) SHA1(937cf33694a3d3baf1cc0577cef6974e1d99a6c4) )
	ROM_LOAD16_BYTE( "nun_ran_f1_nr2.u6", 0x00001, 0x20000, CRC(2a7b9cf2) SHA1(bf30d41dd75b4d316f2f24d11886126484b10415) )
ROM_END

ROM_START( optmsw1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "optimus_w1_i.u2", 0x00000, 0x20000, CRC(fd8c9e7a) SHA1(48848b1958ced9dd2c51679961d883eabbcfd558) )
	ROM_LOAD16_BYTE( "optimus_w1_ii.u6", 0x00001, 0x20000, CRC(7017ac5d) SHA1(bfabccefc283e40fba53ed500e34d547fce9cf0b) )
ROM_END

ROM_START( suprdisc )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "super_disc_f2_pr1.bin", 0x00000, 0x10000, CRC(0e3d67f9) SHA1(19e081ac8e3f0d8d16f67be032ebc788dcb53b26) )
	ROM_LOAD16_BYTE( "super_disc_f2_pr2.bin", 0x00001, 0x10000, CRC(f5c5f6a4) SHA1(9a19dc12027e1b26160fa20cbd53f96ecd679576) )
ROM_END

ROM_START( st_ohla )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "oh_la_la_f1_1.bin", 0x00000, 0x10000, CRC(94583885) SHA1(5083d65da0347a37ffbb537f94d3b247241f1e8c) )
	ROM_LOAD16_BYTE( "oh_la_la_f1_2.bin", 0x00001, 0x10000, CRC(8ac647cd) SHA1(858f67d6121dde28477a5df8569e7ae92db6299e) )
ROM_END

ROM_START( st_vulkn )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "vulkan_f1_1.bin", 0x00000, 0x10000, CRC(06109bd5) SHA1(78f6b0cb3ae5873350fd50af8990fa38454c1183) )
	ROM_LOAD16_BYTE( "vulkan_f1_2.bin", 0x00001, 0x10000, CRC(951baf42) SHA1(1346043155ba85926b2bf9eef8136b377953abe1) )
ROM_END

ROM_START( sunny )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sunny_f2_i.u2", 0x00000, 0x20000, CRC(49776821) SHA1(d68a9e86ea336c46cc07d7bf6ecc3632930f18b9) )
	ROM_LOAD16_BYTE( "sunny_f2_ii.u6", 0x00001, 0x20000, CRC(86b3b81d) SHA1(e12a511bbc53e4614bed561c9544f9ac8faa9fd2) )
ROM_END

ROM_START( suntopw1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sunny_top_w1_i.u2", 0x00000, 0x20000, CRC(c82a3333) SHA1(1e4f28df6e0841f91ef54f453cb1186d1e532d4c) )
	ROM_LOAD16_BYTE( "sunny_top_w1_ii.u6", 0x00001, 0x20000, CRC(d503b558) SHA1(882ff19275c6a5a947bf64bdcc595a7449fdeafb) )
ROM_END

ROM_START( swing )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "swing_f1_i.u2", 0x00000, 0x20000, CRC(b7456f2a) SHA1(91866eb8e7124e479388e9f1ef7144752c77b461) )
	ROM_LOAD16_BYTE( "swing_f1_ii.u6", 0x00001, 0x20000, CRC(a8205180) SHA1(c9b8aeb511d6b181707ecd7ce1606ae658f173d7) )
ROM_END

ROM_START( taipan )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "taipan_w1_i.u2", 0x00000, 0x20000, CRC(feaf45f9) SHA1(ded06e9536aa69d17a1f6dcd2b84f7ecaed7ad18) )
	ROM_LOAD16_BYTE( "taipan_w1_ii.u6", 0x00001, 0x20000, CRC(b2c5418a) SHA1(23c542b983325e677cdd9728bb2fce9263793098) )
ROM_END

ROM_START( trbsunf1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "turbo_sunny_f1_i.u2", 0x00000, 0x20000, CRC(763c00e7) SHA1(8bae5206a3ebad6ec552a9714242cebc78819251) )
	ROM_LOAD16_BYTE( "turbo_sunny_f1_ii.u6", 0x00001, 0x20000, CRC(4d431ae3) SHA1(bb5ff763b9bbaf4eb15ec3fde643b601421fbde1) )
ROM_END

ROM_START( trbsunw1 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "turbo_sunny_w1_i.u2", 0x00000, 0x20000, CRC(10ff87ec) SHA1(2475966b8ef5e030bb85640dce4bc8d584b59275) )
	ROM_LOAD16_BYTE( "turbo_sunny_w1_ii.u6", 0x00001, 0x20000, CRC(59b4fdf8) SHA1(0afa7908945e3036820dd9dc3d76a3d4db36766b) )
ROM_END

ROM_START( viva )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "viva_f1_i.u2", 0x00000, 0x20000, CRC(3fa24273) SHA1(01990504f558c0ad0d6e5e2b7761144fbe314765) )
	ROM_LOAD16_BYTE( "viva_f1_ii.u6", 0x00001, 0x20000, CRC(da0fab3b) SHA1(ef13ea163c60253e88d200c68a25fe7e4a56a657) )
ROM_END

ROM_START( vlogo )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "logo_i.u2", 0x00000, 0x20000, CRC(01ee7a33) SHA1(1976faa9ed7dfbfe46914fa656ba61182bb0b6c1) )
	ROM_LOAD16_BYTE( "logo_ii.u6", 0x00001, 0x20000, CRC(14f939e5) SHA1(9161c49ebfa9005f6b710c91f1e66eab52c58ed2) )
ROM_END

ROM_START( vulkanf2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "vulkan_f2_i.u2", 0x00000, 0x10000, CRC(9b20a948) SHA1(221ae3604752a1db7e73a00d11a90d8b2b242264) )
	ROM_LOAD16_BYTE( "vulkan_f2_ii.u6", 0x00001, 0x10000, CRC(b7613165) SHA1(05321f5ddd0118eb52df033a9ddcc0a4a092a7b9) )
ROM_END

ROM_START( wurfelf2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wuerfel_fix_f2_i.u2", 0x00000, 0x80000, CRC(ae2ce7e8) SHA1(05ed1ece7a35f4883b61d30e4076f513d009aa9c) )
	ROM_LOAD16_BYTE( "wuerfel_fix_f2_ii.u6", 0x00001, 0x80000, CRC(4fad055f) SHA1(68eca493488e7bd17f893e11800b60f2c3a1869d) )
ROM_END

ROM_START( wurfelf3 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wuerfel_fix_f3_i.u2", 0x00000, 0x80000, CRC(9608c498) SHA1(6e3a4ac92177d3c8d2c4773e389c13a404961978) )
	ROM_LOAD16_BYTE( "wuerfel_fix_f3_ii.u6", 0x00001, 0x80000, CRC(3652e163) SHA1(8e4ed9badee4e14d89ce66eaecd218e34539a68b) )
ROM_END

} // anonymous namespace

GAMEL(1993, actionf2, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Action (F2)",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1993, disc4000, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Disc 4000",             MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1993, jack4000, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Jackpot 4000",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1993, jumbo400, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Jumbo 400",             MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1993, multif2,  0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Multi (F2)",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1993, vulkanf2, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Vulkan (F2)",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, dscbonus, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Disc Bonus",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, grandhnd, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Stella", "Grand Hand",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, jmbojmbo, jmbojmbf, sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Venus",  "Jumbo Jumbo",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, m55f1,    0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Fifty Five (F1)",       MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, mdouble,  0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Double",                MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, st_vulkn, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Stella", "Vulkan",                MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, suprdisc, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Super Disc",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, multmult, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Multi Multi",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, dscjkpot, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Disc Jackpot",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, sunny,    0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Sunny",                 MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, swing,    0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Swing",                 MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, bigjkpot, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Big Jackpot",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, jmbojmbf, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Venus",  "Jumbo Jumbo Fun",       MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, wurfelf2, wurfelf3, sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    u8"Würfel Fix (F2)",     MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1994, wurfelf3, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    u8"Würfel Fix (F3)",     MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1995, mptschf2, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Monopoly Tisch (F2)",   MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, discfun,  0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Disc Fun",              MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, multstar, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Multi Star",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, st_ohla,  0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Stella", "Oh La La",              MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, suntopw1, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Sunny Top (W1)",        MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, trbsunf1, trbsunw1, sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Turbo Sunny (F1)",      MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, trbsunw1, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Turbo Sunny (W1)",      MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1996, viva,     0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Mega",   "Viva",                  MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, dpplson,  0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Doppel-Sonne",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, gjubil,   0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Golden Jubilar",        MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, jkrpoker, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Joker Poker (Merkur)",  MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, jumboa,   0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Jumbo Action",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, kometf1,  0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Komet (F1)",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, moneyf1,  0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Money (F1)",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, multprim, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Venus",  "Multi Prima",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, mystjack, 0,        sus_rtc, stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Mystery Jackpot",       MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, ndiscow1, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Nova",   "Disco (W1)",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, nunran,   0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Nun Ran",               MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, optmsw1,  0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Venus",  "Optimus (W1)",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1997, vlogo   , 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Venus",  "Logo (W1)",             MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, allfred,  0,        sus_rtc, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Allfred",               MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, cstliiw1, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Venus",  "Castello II (W1)",      MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, glksstrn, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    u8"Glücks-Stern",        MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, grnadaf1, grnadaw1, sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Granada (F1)",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, grnadaw1, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "ADP",    "Granada (W1)",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, taipan,   0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Nova",   "Tai Pan Money",         MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(1998, bigwinnr, 0,        sus_tk,  stellafr, stellafr_state, empty_init, ROT0, "Nova",   "Big Winner",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
GAMEL(2001, kleoptra, 0,        sus_rtc, stellafr, stellafr_state, empty_init, ROT0, "Stella", "Asterix und Kleopatra", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_stellafr )
