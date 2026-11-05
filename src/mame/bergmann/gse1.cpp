// license:BSD-3-Clause
// copyright-holders:stonedDiscord

/*
Tech used by Bergmann Automaten GmbH from 1983-1994
In later manuals they are called GSE1.3 or GSE1.4
GSE stands for Gerätesteuereinheit (device control unit)

_________________________________
|                               |
|       74C373     74   TA1 TA2 |
|-      74C373     LS           |
|J      74C373  74 123          |
|1              LS              |
|3              132        4MHz |
|-                   47      74 |
|S         CTC1      02   -  04 |
|E                       BAT 74 |
|R         PIO1           +  LS |
|-                           74 |
|J    45   CTC2         4093    |
|1    03                4069    |
|5     P    RAM1        74H032  |
|-     I    CPU1       74HC138  |
|J     O               74LS155  |
|1     2                        |
|6         EPROM1               |
|-                              |
|J                      L7805   |
|1  8251A  EPROM2               |
|7                              |
|-------------------------------|

TA1 Reset
TA2 NMI (unpopulated)
CPU Z0840004PSC
RAM MB8416-20L
PIO 2x Z0842004PSC
CTC 2x Z0843004PSC
SERIAL 8251A + HD-4702 (unpopulated)

TODO:
Coin sequencing (string and security sensors)
Spinning discs (both position detection and layout)
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "sound/dac.h"

#include "speaker.h"

#include "croyal1.lh"

//#define VERBOSE 1
#include "logmacro.h"

namespace {

class gse1_state : public driver_device
{
public:
	gse1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pio1(*this, "pio1")
		, m_pio2(*this, "pio2")
		, m_ctc1(*this, "ctc1")
		, m_ctc2(*this, "ctc2")
		, m_watchdog(*this, "watchdog")
		, m_dac_alarm_l(*this, "dac_alarm_l")
		, m_dac_alarm_r(*this, "dac_alarm_r")
		, m_dac_l(*this, "dac_l")
		, m_dac_r(*this, "dac_r")
		, m_led(*this, "led_error")
		, m_digits(*this, "digit%u", 0U)
		, m_lamps(*this, "lamp%u%u", 0U, 0U)
		, m_money_led(*this, "money_led%u", 0U)
	{
	}

	void gse1(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<nvram_device> m_nvram;

	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;

	required_device<z80ctc_device> m_ctc1;
	required_device<z80ctc_device> m_ctc2;

	required_device<watchdog_timer_device> m_watchdog;

	required_device<dac_bit_interface> m_dac_alarm_l;
	required_device<dac_bit_interface> m_dac_alarm_r;
	required_device<dac_bit_interface> m_dac_l;
	required_device<dac_bit_interface> m_dac_r;

	output_finder<> m_led;
	output_finder<8> m_digits;
	output_finder<8, 8> m_lamps;
	output_finder<8> m_money_led;

	uint8_t m_adresse = 0;
	bool m_battery = false;

	uint8_t m_pio1_pb = 0xff;
	uint8_t m_pio2_pa = 0xff;
	uint8_t m_pio2_pb = 0xff;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void adresse_w(uint8_t data);
	void daten_w(uint8_t data);
	uint8_t daten_r();

	uint8_t pio1_pa_r();
	uint8_t pio1_pb_r();
	void pio1_pb_w(uint8_t data);
	uint8_t pio2_pb_r();
	void pio2_pb_w(uint8_t data);
	uint8_t pio2_pa_r();
	void pio2_pa_w(uint8_t data);
	void ctc1_zc0_w(int state);
	void ctc1_zc1_w(int state);
	void ctc1_zc2_w(int state);
	void ctc2_zc0_w(int state);
};

void gse1_state::machine_start()
{
	save_item(NAME(m_adresse));
	save_item(NAME(m_battery));
	save_item(NAME(m_pio1_pb));
	save_item(NAME(m_pio2_pa));
	save_item(NAME(m_pio2_pb));
}

void gse1_state::mem_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x6000, 0x6000).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
}

void gse1_state::io_map(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x03).rw(m_ctc2, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x04, 0x07).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x08, 0x0b).rw(m_pio2, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0f).rw(m_pio1, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x10, 0x13).w(FUNC(gse1_state::adresse_w)); //74C373/3
	map(0x14, 0x17).w(FUNC(gse1_state::daten_w)); //74C373/2
	map(0x18, 0x1b).r(FUNC(gse1_state::daten_r)); //74C373/1
}

void gse1_state::adresse_w(uint8_t data)
{
	m_adresse = data;
}

void gse1_state::daten_w(uint8_t data)
{
	enum : u8
	{
		_a = 1 << 0,
		_b = 1 << 1,
		_c = 1 << 2,
		_d = 1 << 3,
		_e = 1 << 4,
		_f = 1 << 5,
		_g = 1 << 6,
		_h = 1 << 7
	};

	static constexpr u8 cd4511[16] = {
	_a | _b | _c | _d | _e | _f,
	_b | _c,
	_a | _b | _d | _e | _g,
	_a | _b | _c | _d | _g,
	_b | _c | _f | _g,
	_a | _c | _d | _f | _g,
	_c | _d | _e | _f | _g,
	_a | _b | _c,
	_a | _b | _c | _d | _e | _f | _g,
	_a | _b | _c | _f | _g,
	0,
	0,
	0,
	0,
	0,
	0
	};

	switch (m_adresse & 0x7f)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			for (int i = 0; i < 8; i++)
			{
				bool lamp_value = BIT(data, i);
				m_lamps[m_adresse & 0x07][i] = lamp_value;
			}
			break;
		case 0x10:
			// Münzspeicher Pfennig
			m_digits[0] = cd4511[data & 0x0f];
			m_digits[1] = cd4511[data >> 4];
			break;
		case 0x11:
			// Münzspeicher DM
			m_digits[2] = cd4511[data & 0x0f];
			m_digits[3] = cd4511[data >> 4];
			break;
		case 0x12:
			// Sonderspiele 10 and 1
			m_digits[4] = cd4511[data & 0x0f];
			m_digits[5] = cd4511[data >> 4];
			break;
		case 0x13:
			// Sonderspiele 100
			m_digits[6] = cd4511[data & 0x0f];
			break;
		case 0x14:
			// Münzeinwurf/Sonderspiele
			for (int i = 0; i < 8; i++)
			{
				bool lamp_value = BIT(data, i);
				m_money_led[i] = lamp_value;
			}
			break;
		default:
			LOG("Unhandled write %02x to address %02x\n", data, m_adresse);
			break;
	}
}

uint8_t gse1_state::daten_r()
{
	uint8_t data = 0xff;

	switch (m_adresse & 0x07)
	{
		case 0x06:
			data = ioport("T6")->read();
			break;
		case 0x07:
			data = ioport("T7")->read();
			break;
		default:
			LOG("Unhandled read from address %02x\n", m_adresse);
			break;
	}

	return data;
}

//PIO1
uint8_t gse1_state::pio1_pa_r()
{
	// Steckerleiste 15
	// TODO: The coin sequencing is missing
	// Specifically PA2 and/or PA3 have to be triggered shortly after a coin passes PA5-7
	return ioport("COIN")->read();
}

uint8_t gse1_state::pio1_pb_r()
{
	// Steckerleiste 15
	// PB5-7
	return ioport("RETURN")->read();
}

void gse1_state::pio1_pb_w(uint8_t data)
{
	// Steckerleiste 15
	m_pio1_pb = data;
	//coins out
	machine().bookkeeping().coin_counter_w(3,BIT(m_pio1_pb,0)); // PB0 0.10DM
	machine().bookkeeping().coin_counter_w(2,BIT(m_pio1_pb,1)); // PB1 1DM
	machine().bookkeeping().coin_counter_w(1,BIT(m_pio1_pb,2)); // PB2 2DM
	machine().bookkeeping().coin_counter_w(0,BIT(m_pio1_pb,3)); // PB3 5DM

	machine().bookkeeping().coin_lockout_global_w(!BIT(m_pio1_pb,4)); // PB4 coin magnet
}

//PIO2
uint8_t gse1_state::pio2_pa_r()
{
	// Steckerleiste 17
	uint8_t data = m_pio2_pa;
	// The only input here is the battery status
	data = m_battery ? data | 0x80 : data & ~0x80; //PA7 from IC1
	return data;
}

void gse1_state::pio2_pa_w(uint8_t data)
{
	// Steckerleiste 17
	m_pio2_pa = data;
	m_led = BIT(data, 0);
	// 1 NC
	m_dac_alarm_l->write(BIT(data, 2));
	m_dac_alarm_r->write(BIT(data, 3));
	m_dac_r->write(BIT(data, 4));
	m_dac_l->write(BIT(data, 5));
	m_battery = BIT(data, 6);
}

uint8_t gse1_state::pio2_pb_r()
{
	// Steckerleiste 16
	uint8_t data = m_pio2_pb;
	// Motor sync
	bool motor_right = false;
	bool motor_middle = false;
	bool motor_left = false;

	data = motor_right ? data | 0x08 : data & ~0x08; //PB3
	data = motor_middle ? data | 0x10 : data & ~0x10; //PB4
	data = motor_left ? data | 0x20 : data & ~0x20; //PB5

	return data;
}

void gse1_state::pio2_pb_w(uint8_t data)
{
	m_pio2_pb = data;
	bool motor_reset = BIT(data, 6);
	bool motor_switch = BIT(data, 7);
	LOG("MOTOR reset: %02x, switch: %02x\n", motor_reset, motor_switch);
	// Steckerleiste 16
}

//CTC
void gse1_state::ctc1_zc0_w(int state)
{
	LOG("Left motor: %d\n", state);
}

void gse1_state::ctc1_zc1_w(int state)
{
	LOG("Right motor: %d\n", state);
}

void gse1_state::ctc1_zc2_w(int state)
{
	LOG("Middle motor: %d\n", state);
}

void gse1_state::ctc2_zc0_w(int state)
{
	m_ctc2->trg1(state);
	m_ctc2->trg2(state);
	m_ctc2->trg3(state);
}

static INPUT_PORTS_START( gse1 )
	PORT_START("RETURN")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x20, "Level switch 2DM" ) // Pegelschalter 2,-
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Level switch 5DM" ) // Pegelschalter 5,-
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Return")

	PORT_START("COIN")
	PORT_DIPNAME( 0x01, 0x01, "Level switch 10Pf" ) // Pegelschalter -,10
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Level switch 1DM" ) // Pegelschalter 1,-
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	// TODO: These next 2 light barriers are triggered by the coin.
	PORT_DIPNAME( 0x04, 0x04, "String light barrier" ) // Fadenfalle-Lichtschranke
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Safety light barrier" ) // Sicherheits-Lichtschranke
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) // 5DM
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) // 2DM
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) // 1DM
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) // 0.10 DM

	PORT_START("T6")
	PORT_DIPNAME( 0x01, 0x01, "Program start" ) PORT_DIPLOCATION("SW1:6") // Programmstart
	PORT_DIPSETTING(    0x01, "Normal" ) // Normalfall
	PORT_DIPSETTING(    0x00, "60 seconds after power-up or reset" ) // 60 Sekunden nach Einschalten, bzw. Reset.
	PORT_DIPNAME( 0x02, 0x02, "String light barrier" ) PORT_DIPLOCATION("SW1:7") // Fadenlichtschranke
	PORT_DIPSETTING(    0x02, "Normal" ) // Normalfall
	PORT_DIPSETTING(    0x00, "Unused" ) // außer Betrieb
	PORT_DIPNAME( 0x04, 0x04, "Coin limit" ) PORT_DIPLOCATION("SW1:8") // Einwurfbegrenzung
	PORT_DIPSETTING(    0x04, "Normal" ) // Normalfall
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Risiko links")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Aussp.Wiedh.")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop rechts+mitte")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Risiko rechts")

	PORT_START("T7")
	PORT_DIPNAME( 0x0f, 0x0f, "Service switch" ) // Serviceschalter
	PORT_DIPSETTING(    0x0f, "Regular" ) // 0 Normalstellung
	PORT_DIPSETTING(    0x0e, "Show malfunction counter" ) // 1 Manko-Zähler anzeigen
	PORT_DIPSETTING(    0x0c, "Erase series counters" ) // 3 Vorlage- und Serienzähler löschen
	PORT_DIPSETTING(    0x0a, "Game simulation" ) // 5 Spielsimulation
	PORT_DIPSETTING(    0x08, "Show error counter" ) // 7 Fehlerzähler anzeigen
	PORT_DIPSETTING(    0x07, "Test input/output" ) // 8 Ein- und Ausgänge testen
	PORT_DIPSETTING(    0x06, "Test coin mechanism" ) // 9 Münzeinheit testen
	PORT_DIPNAME( 0x30, 0x30, "Game simulation" ) PORT_DIPLOCATION("SW1:2,3") // Spielsimulation
	PORT_DIPSETTING(    0x30, "Normal" ) // Normalfall
	PORT_DIPSETTING(    0x00, "Game simulation" ) // Spielsimulation
	PORT_DIPNAME( 0x40, 0x40, "Credit" ) PORT_DIPLOCATION("SW1:4") // Kredit
	PORT_DIPSETTING(    0x40, "Normal" ) // Normalfall
	PORT_DIPSETTING(    0x00, "The coin mechanism button is active" ) // Am Münzaggregat ist der Taster aktiv
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("NC")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc1" },
	{ "pio1" },
	{ "pio2" },
	{ "ctc2" },
	{ nullptr }
};

void gse1_state::gse1(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gse1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &gse1_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	Z80PIO(config, m_pio1, 4_MHz_XTAL/2);
	m_pio1->in_pa_callback().set( FUNC(gse1_state::pio1_pa_r));
	m_pio1->out_pb_callback().set(FUNC(gse1_state::pio1_pb_w));
	m_pio1->in_pb_callback().set( FUNC(gse1_state::pio1_pb_r));
	m_pio1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio2, 4_MHz_XTAL/2);
	m_pio2->out_pa_callback().set(FUNC(gse1_state::pio2_pa_w));
	m_pio2->in_pa_callback().set( FUNC(gse1_state::pio2_pa_r));
	m_pio2->out_pb_callback().set(FUNC(gse1_state::pio2_pb_w));
	m_pio2->in_pb_callback().set( FUNC(gse1_state::pio2_pb_r));
	m_pio2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc1, 4_MHz_XTAL/2);
	m_ctc1->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc1->zc_callback<0>().set(FUNC(gse1_state::ctc1_zc0_w));
	m_ctc1->zc_callback<1>().set(FUNC(gse1_state::ctc1_zc1_w));
	m_ctc1->zc_callback<2>().set(FUNC(gse1_state::ctc1_zc2_w));

	Z80CTC(config, m_ctc2, 4_MHz_XTAL/2);
	m_ctc2->zc_callback<0>().set(FUNC(gse1_state::ctc2_zc0_w));
	m_ctc2->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_usec(341000)); // 47uF x 22k x 0,33

	config.set_default_layout(layout_croyal1);

	SPEAKER(config, "speaker", 2).front();
	DAC_1BIT(config, m_dac_alarm_l, 0).add_route(0, "speaker", 1.0, 0);
	DAC_1BIT(config, m_dac_alarm_r, 0).add_route(0, "speaker", 1.0, 1);
	DAC_1BIT(config, m_dac_l, 0).add_route(0, "speaker", 0.8, 0); //pot
	DAC_1BIT(config, m_dac_r, 0).add_route(0, "speaker", 0.8, 1); //pot
}

ROM_START( corsar )
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "crown_corsar.a", 0x0000, 0x2000, CRC(cf907cab) SHA1(66e22b8e1f8e3645dd4b5f1504517b7e734b20f2) )
	ROM_LOAD( "crown_corsar.b", 0x2000, 0x2000, CRC(c1a69ccd) SHA1(97561620110ed991dcd2b17fbd0de5d0bbc282fe) )
ROM_END

ROM_START( croyal1 )
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "crown_royal_no1.a", 0x0000, 0x2000, CRC(eccf354f) SHA1(de023a3403a82314609acac10072a3bef00b4969) )
	ROM_LOAD( "crown_royal_no1.b", 0x2000, 0x1000, CRC(c703a9cd) SHA1(015dd4c2454ff865bb9a88f63d7ad4d785ba5495) )
ROM_END

ROM_START( cwinner )
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "winner.a", 0x0000, 0x2000, CRC(1fa5ef3a) SHA1(4c645b60dba740cfb5da55a9c096ef0669573af3) )
	ROM_LOAD( "winner.b", 0x2000, 0x2000, CRC(0cb99872) SHA1(18e4085b9c407ec7e656a9a733eae39ff00403e8) )
ROM_END

ROM_START( jubilees )
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "jubilee_super.a", 0x0000, 0x2000, CRC(32b8ad57) SHA1(8b95f23b5cb22261f7db62e3879b475722cfe0f2) )
	ROM_LOAD( "jubilee_super.b", 0x2000, 0x1000, CRC(60b7a6d3) SHA1(a98375c970176927ad4f0ca6b62b1a22cc07df69) )
ROM_END

} // anonymous namespace

GAMEL( 1983, croyal1,  0, gse1, gse1, gse1_state, empty_init, ROT0, "Bergmann Automaten", "Crown Royal No.1",    MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_croyal1 )
GAMEL( 1984, corsar,   0, gse1, gse1, gse1_state, empty_init, ROT0, "Bergmann Automaten", "Crown Corsar",        MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_croyal1 )
GAMEL( 1984, jubilees, 0, gse1, gse1, gse1_state, empty_init, ROT0, "Bergmann Automaten", "Crown Jubilee Super", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_croyal1 )
GAMEL( 1984, cwinner,  0, gse1, gse1, gse1_state, empty_init, ROT0, "Bergmann Automaten", "Crown Winner",        MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_croyal1 )
