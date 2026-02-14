// license:BSD-3-Clause
// copyright-holders:stonedDiscord

/*

Elektronische Steuereinheit
8085 based hardware
Lots of lamps and 8 7-segment LEDs

Main components:
Siemens SAB 8085AH-2-P (CPU)
Sharp LH5164D-10L or Sony CXK5816PN-12L (SRAM)
Siemens SAB 8256 A 2 P (MUART)
NEC D8279C-2 (keyboard & display interface)
AMI or Micrel S50240 (sound)

At least 4 different boards exist:
4040-000-101 (6 ROM slots, TC5514 RAM) used in excellent
4087-000-101 (3 ROM slots, RTC HD146818) used in doppelpot
4109-000-101 (2 ROM slots, RTC 62421A) used in kniffi
4382-000-101 (2 ROM slots, RTC 62421A) used in dicemstr

Dice Master reference: https://www.youtube.com/watch?v=NlB06dMxjME
Merkur Disc reference: https://www.youtube.com/watch?v=1NjJPkzg9Mk
Nova Kniffi reference: https://www.youtube.com/watch?v=YBq2Z1irXek
*/


#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/i8256.h"
#include "machine/i8279.h"
#include "machine/mc146818.h"
#include "machine/msm6242.h"
#include "sound/beep.h"

#include "speaker.h"

#include "adpservice.lh"
#include "disc2000.lh"

//#define VERBOSE 1
#include "logmacro.h"


namespace {

class stella8085_state : public driver_device
{
public:
	stella8085_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_uart(*this, "muart"),
		m_kdc(*this, "kdc"),
		m_tz(*this, "TZ%u", 0U),
		m_dsw(*this, "DSW"),
		m_digits(*this, "digit%u", 0U),
		m_lamps(*this, "lamp%u%u", 0U, 0U),
		m_beep(*this, "beeper")
	{ }

	void board4040(machine_config &config);
	void board4087(machine_config &config);
	void board4109(machine_config &config);
	void board4382(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;

private:
	uint8_t m_digit = 0U;
	uint8_t m_kbd_sl = 0x00;
	uint8_t m_machine[5] = {0,0,0,0,0};
	bool m_kbd_bd = false;
	uint8_t m_sounddata = 0U;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8256_device> m_uart;
	required_device<i8279_device> m_kdc;
	required_ioport_array<8> m_tz;
	required_ioport m_dsw;
	output_finder<16> m_digits;
	output_finder<8, 8> m_lamps;
	required_device<beep_device> m_beep;
	emu_timer *m_sound_timer;

	void boards_common(machine_config &config, XTAL clock);

	void program_4040_map(address_map &map) ATTR_COLD;
	void program_4087_map(address_map &map) ATTR_COLD;
	void program_4109_map(address_map &map) ATTR_COLD;
	void io_4040_map(address_map &map) ATTR_COLD;
	void io_4087_map(address_map &map) ATTR_COLD;

	// I8256 ports
	uint8_t lw_r(); //P1.0-P1.3
	uint8_t machine_r();
	void machine1_w(uint8_t data);
	void machine2_w(uint8_t data);

	// I8279 Interface
	uint8_t kbd_rl_r();
	void kbd_sl_w(uint8_t data);
	void kbd_bd_w(uint8_t data);
	void disp_w(uint8_t data);
	void rst65_w(uint8_t state);
	void output_digit(uint8_t i, uint8_t data);

	void io00(uint8_t data) ATTR_COLD;
	uint8_t io70r() ATTR_COLD;
	void io70w(uint8_t data) ATTR_COLD;
	uint8_t io71r() ATTR_COLD;
	void io71w(uint8_t data) ATTR_COLD;
	void sounddev(uint8_t data) ATTR_COLD;

	void setsound(uint8_t tone, uint8_t octave);
	int soundfreq(uint8_t channel, uint8_t clockdiv);
	TIMER_CALLBACK_MEMBER(sound_stop);
};

void stella8085_state::machine_start()
{
	m_digits.resolve();
	m_lamps.resolve();

	m_sound_timer = timer_alloc(FUNC(stella8085_state::sound_stop), this);

	save_item(NAME(m_digit));
}

void stella8085_state::program_4040_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0x5000, 0x5fff).ram();
	map(0x6000, 0x6fff).ram();
	//map(0x6000, 0x603f).rw("rtc", FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0x7000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xffff).ram();
}

void stella8085_state::program_4087_map(address_map &map)
{
	map(0x0000, 0x3fff).rom(); // ICE6
	map(0x4000, 0x7fff).rom(); // ICD6
	map(0x8000, 0x8fff).rom(); // ICC5
	map(0x9000, 0x903f).rw("rtc", FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0xa000, 0xafff).ram(); // ??
	map(0xc000, 0xc7ff).ram(); // ICC6
}

void stella8085_state::program_4109_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); // ICE6
	map(0x8000, 0x9fff).ram(); // ICC6
	map(0xa000, 0xffff).rom(); // ICD6
}

void stella8085_state::io_4040_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(stella8085_state::io00));
	map(0x70, 0x73).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)).mirror(0x0c);
	map(0x80, 0x81).rw(m_kdc, FUNC(i8279_device::read), FUNC(i8279_device::write)).mirror(0x0e);
	map(0x90, 0x9f).rw(m_uart, FUNC(i8256_device::read), FUNC(i8256_device::write));
}

void stella8085_state::io_4087_map(address_map &map)
{
	// ICF5 74LS42
	map(0x00, 0x4f).noprw();
	map(0x50, 0x51).rw(m_kdc, FUNC(i8279_device::read), FUNC(i8279_device::write)).mirror(0x0e);
	map(0x60, 0x6f).rw(m_uart, FUNC(i8256_device::read), FUNC(i8256_device::write));
	// 7x handled by ICH5 74LS138
	map(0x70, 0x70).rw(FUNC(stella8085_state::io70r),FUNC(stella8085_state::io70w)).mirror(0x0c);
	map(0x71, 0x71).rw(FUNC(stella8085_state::io71r),FUNC(stella8085_state::io71w)).mirror(0x0c);
	map(0x72, 0x72).w(FUNC(stella8085_state::sounddev)).mirror(0x0c);
	map(0x73, 0x73).nopw();
	map(0x80, 0x8f).noprw(); //Y8 ICC5 empty socket
}

/*********************************************
*      I8256 Ports controlling the wheels    *
*                                            *
*********************************************/

uint8_t stella8085_state::lw_r()
{
	// wheel light sensors

	// LIW1
	// LIW2
	// LIW3
	// LIW4
	//M5A out
	//M5B out
	//P1.6 is always low
	// LIW5

	return 0x00;
}

uint8_t stella8085_state::machine_r()
{
	uint8_t machine_state = 0x00;

	machine_state |= m_machine[0];       // M1 A,B
	machine_state |= m_machine[1]  << 2; // M2 A,B
	machine_state |= m_machine[2]  << 4; // M3 A,B
	machine_state |= m_machine[3]  << 6; // M4 A,B

	return machine_state;
}

void stella8085_state::machine1_w(uint8_t data)
{
	m_machine[0] = data & 0x03;
	m_machine[1] = (data >> 2) & 0x03;
	m_machine[2] = (data >> 4) & 0x03;
	m_machine[3] = (data >> 6) & 0x03;

	popmessage("M1 A %d B %d\nM2 A %d B %d\nM3 A %d B %d\nM4 A %d B %d",BIT(0,data),BIT(1,data),BIT(2,data),BIT(3,data),BIT(4,data),BIT(5,data),BIT(6,data),BIT(7,data));
}

void stella8085_state::machine2_w(uint8_t data)
{
	m_machine[4] = (data >> 4) & 0x03;
	popmessage("M5 A %d B %d",BIT(4,data),BIT(5,data));
}

/*********************************************
*      I8279 Keyboard-Display Interface      *
*                                            *
*********************************************/

void stella8085_state::kbd_sl_w(uint8_t data)
{
	m_kbd_sl = data & 0x0f; //SL0-SL3

	// SL3 connected through CD4093 NAND to DIP switch connected to RST75
	if (BIT(m_dsw->read(), 0))
		m_maincpu->set_input_line(I8085_RST75_LINE, BIT(data,3) ? CLEAR_LINE : ASSERT_LINE);
	else
		m_maincpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);
}

void stella8085_state::kbd_bd_w(uint8_t data)
{
	m_kbd_bd = data;
}

uint8_t stella8085_state::kbd_rl_r()
{
	uint8_t ret = 0xff;
	const uint8_t RL = m_kbd_sl & 0x07; // only SL0-2 connected
	ret = m_tz[RL]->read();
	return ret;
}

void stella8085_state::disp_w(uint8_t data)
{
	if (m_kbd_sl < 8) // SL3 is inverted for the display outs
	{
		for (int i = 0; i < 8; i++)
		{
			bool lamp_value = BIT(data, i);
			m_lamps[m_kbd_sl][i] = lamp_value;
		}
	}
	else
	{
		// service kb A0-A3
		output_digit(m_kbd_sl, data >> 4);
		// money counter B0-B3
		output_digit(m_kbd_sl-8, data & 0x0f);
	}
}

void stella8085_state::output_digit(uint8_t digit, uint8_t data)
{
	uint8_t i = digit & 0x0F;

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

	static constexpr u8 ttl7448[16] = {
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
	_d | _e | _g,
	_c | _d | _g,
	_b | _f | _g,
	_a | _d | _f | _g,
	_d | _e | _f | _g,
	0
	};

	static constexpr u8 cd4543[16] = {
	_a | _b | _c | _d | _e | _f,
	_b | _c,
	_a | _b | _d | _e | _g,
	_a | _b | _c | _d | _g,
	_b | _c | _f | _g,
	_a | _c | _d | _f | _g,
	_a | _c | _d | _e | _f | _g,
	_a | _b | _c,
	_a | _b | _c | _d | _e | _f | _g,
	_a | _b | _c | _d | _f | _g,
	0,
	0,
	0,
	0,
	0,
	0
	};

	if (i<8)
		m_digits[i] = cd4543[data & 0x0F];
	else
		m_digits[i] = ttl7448[data & 0x0F];
}

TIMER_CALLBACK_MEMBER(stella8085_state::sound_stop)
{
	m_beep->set_state(0);

	if (BIT(m_dsw->read(), 3))
		m_maincpu->set_input_line(I8085_RST55_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(I8085_RST55_LINE, CLEAR_LINE);
}

void stella8085_state::rst65_w(uint8_t state)
{
	m_maincpu->set_input_line(I8085_RST65_LINE, state ? CLEAR_LINE : ASSERT_LINE);
}

void stella8085_state::io00(uint8_t data)
{
	//old boards
}

uint8_t stella8085_state::io70r()
{
	return 0xff;
}

void stella8085_state::io70w(uint8_t data)
{
	const bool AW1 = BIT(data,0);
	const bool AW2 = BIT(data,1);
	const bool AW3 = BIT(data,2);
	const bool AW4 = BIT(data,3);
	const bool MP = BIT(data,4);
	const bool SZ = BIT(data,5);
	const bool D6 = BIT(data,6); // high on startup
	const bool PA7 = BIT(data,7);

	machine().bookkeeping().coin_lockout_global_w(MP); // coin magnet
	machine().bookkeeping().coin_counter_w(0,AW1); // coin eject
	machine().bookkeeping().coin_counter_w(1,AW2);
	machine().bookkeeping().coin_counter_w(2,AW3);
	machine().bookkeeping().coin_counter_w(3,AW4);
	machine().bookkeeping().coin_counter_w(5,SZ); // game counter

	if (D6)
		;//LOG("Short test\n");

	if (PA7)
		LOG("PA7 high\n");
}

uint8_t stella8085_state::io71r()
{
	return 0xff;
}

void stella8085_state::io71w(uint8_t data)
{
	const bool SS = BIT(data,0);   // start sound
	const bool GONG = BIT(data,1); // bell
	const bool DG = BIT(data,2);   // counter groschen down
	const bool UG = BIT(data,3);   // counter groschen up
	const bool DS = BIT(data,4);   // counter serie down
	const bool US = BIT(data,5);   // counter serie up
	const bool DM = BIT(data,6);   // counter mark down
	const bool UM = BIT(data,7);   // counter mark up

	if (SS)
	{
		m_beep->set_state(1);
		m_maincpu->set_input_line(I8085_RST55_LINE, CLEAR_LINE);
		m_sound_timer->adjust(attotime::from_msec((60*(m_sounddata & 0x30) >> 4)+1), 0); // TODO: measure this
	}

	if (GONG)
		popmessage("GONG");
	if (US)
		LOG("activating US\n");

	m_beep->set_output_gain(ALL_OUTPUTS,!DG); //repurposed as unmute on newer boards

	if (UG || DS || DM || UM)
		LOG("UG %d DS %d DM %d UM %d\n", UG,DS,DM,UM);
}

void stella8085_state::sounddev(uint8_t data)
{
	m_sounddata = data;
	const uint8_t TONE = (data & 0x0f);
	const uint8_t OCTAVE = (data & 0xc0) >> 6;
	setsound(TONE, OCTAVE);
}

void stella8085_state::setsound(uint8_t tone, uint8_t octave)
{
	int sfrq = soundfreq(tone, octave);
	m_beep->set_clock(sfrq);
}

int stella8085_state::soundfreq(uint8_t tone, uint8_t octave)
{
	const int BUS_CLOCK = (m_maincpu->clock() / 2);
	const int SOUND_CLOCK = BUS_CLOCK >> 2; //74LS290
	const int INT_CLOCK = SOUND_CLOCK >> (1 + octave); //CD4040

	//                             C9   B8   A8#  A8   G8#  G8   F8#  F8   E8   D8#  D8   C8#
	const int FREQOUT[16] = { 478, 239, 253, 268, 284, 301, 319, 338, 358, 379, 402, 426, 451, 478, 478, 478 };

	return INT_CLOCK / FREQOUT[tone & 0x0f];
}

static INPUT_PORTS_START( stella8085_tatatur )
	PORT_START("TZ6") // TASTATUR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") // TS7
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dauerlauf")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Spielzähler")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Münzspeicher")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hardware-Test")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Auszahlquote")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Foul")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Gewinn") // TS0

	PORT_START("TZ7") // TASTATUR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hoch 1,-")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Runter 1,-")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hoch Serie")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Runter Serie")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hoch 0,10")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Runter 0,10")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Münzung")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Initialisieren")
INPUT_PORTS_END

static INPUT_PORTS_START( stella8085_dip )
	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, "8085 RST75")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "8085 HOLD")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "8085 Reset")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "8085 RST55")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START( servicem )
	PORT_INCLUDE(stella8085_dip)

	PORT_START("TZ0") //TASTEN
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TZ1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("DM 0.10")  //LIM1 COIN II
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("DM 1.00")  //LIM2 COIN II
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("DM 2.00")  //LIM3 COIN II
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("DM 5.00")  //LIM4 COIN II

	PORT_START("TZ2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("DM 0.10")  //LIA1 COIN I
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("DM 1.00")  //LIA2 COIN I
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("DM 2.00")  //LIA3 COIN I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("DM 5.00")  //LIA4 COIN I

	PORT_START("TZ3") //ZUSATZ-EINGAENGE
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TZ4") //MATRIX-EINGAENGE
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TZ5") //MATRIX-EINGAENGE
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE(stella8085_tatatur)
INPUT_PORTS_END

static INPUT_PORTS_START( disc )
	PORT_INCLUDE(stella8085_dip)

	PORT_START("TZ0") //TASTEN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_DOOR ) //TS Door switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) //Schlagkontakt / Read data button
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //ZE2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) // Return
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) //STR
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START ) //NF

	PORT_START("TZ1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("DM 0.10")  //LIM1 COIN II
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("DM 1.00")  //LIM2 COIN II
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("DM 2.00")  //LIM3 COIN II
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("DM 5.00")  //LIM4 COIN II
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) //MK
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //LIG

	PORT_START("TZ2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("DM 0.10")  //LIA1 COIN I
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("DM 1.00")  //LIA2 COIN I
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("DM 2.00")  //LIA3 COIN I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("DM 5.00")  //LIA4 COIN I
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TZ3") //ZUSATZ-EINGAENGE
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW ) // Risiko Leiter 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) // Risiko Leiter 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // ZE2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) // Serienuebernahme
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Rueckfuehrung
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TZ4") //MATRIX-EINGAENGE
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TZ5") //MATRIX-EINGAENGE
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE(stella8085_tatatur)
INPUT_PORTS_END

void stella8085_state::boards_common(machine_config &config, XTAL clock)
{
	I8085A(config, m_maincpu, clock);
	m_maincpu->in_inta_func().set(m_uart, FUNC(i8256_device::acknowledge));
	m_maincpu->in_sid_func().set_constant(0);	// connected through opamp to battery voltage

	I8256(config, m_uart, clock / 2);
	m_uart->int_callback().set_inputline(m_maincpu, I8085_INTR_LINE);
	m_uart->in_p1_callback().set(FUNC(stella8085_state::lw_r));
	m_uart->out_p1_callback().set(FUNC(stella8085_state::machine2_w));
	m_uart->in_p2_callback().set(FUNC(stella8085_state::machine_r));
	m_uart->out_p2_callback().set(FUNC(stella8085_state::machine1_w)); //M1-4
	m_uart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(i8256_device::write_rxd));

	I8279(config, m_kdc, clock / 2);
	m_kdc->out_sl_callback().set(FUNC(stella8085_state::kbd_sl_w));
	m_kdc->out_bd_callback().set(FUNC(stella8085_state::kbd_bd_w));
	m_kdc->out_disp_callback().set(FUNC(stella8085_state::disp_w));
	m_kdc->in_rl_callback().set(FUNC(stella8085_state::kbd_rl_r));
	m_kdc->out_irq_callback().set(FUNC(stella8085_state::rst65_w));

	config.set_default_layout(layout_adpservice);

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 0)
		.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void stella8085_state::board4040(machine_config &config)
{
	boards_common(config, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::program_4040_map);
	m_maincpu->set_addrmap(AS_IO, &stella8085_state::io_4040_map);

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(stella8085_state::io70w));
	ppi.in_pb_callback().set(FUNC(stella8085_state::io70r));
	ppi.out_pc_callback().set(FUNC(stella8085_state::sounddev));
	ppi.tri_pc_callback().set_constant(0);
}

void stella8085_state::board4087(machine_config &config)
{
	boards_common(config, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::program_4087_map);
	m_maincpu->set_addrmap(AS_IO, &stella8085_state::io_4087_map);

	MC146818(config, "rtc", 32.768_kHz_XTAL);
}

void stella8085_state::board4109(machine_config &config)
{
	boards_common(config, 10.240_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::program_4109_map);
	m_maincpu->set_addrmap(AS_IO, &stella8085_state::io_4087_map);

	MC146818(config, "rtc", 32.768_kHz_XTAL);
}

void stella8085_state::board4382(machine_config &config)
{
	board4109(config);
}

ROM_START( bahia )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bahia_pr1", 0x0000, 0x1000, CRC(41e7f89c) SHA1(933334e2f78a91e24ec0132b8e7757da5a9d2e02) )
	ROM_LOAD( "bahia_pr2", 0x1000, 0x1000, CRC(ab06262a) SHA1(435f16002054f010e1349f2dbc998a2e5eb50c70) )
	ROM_LOAD( "bahia_pr3", 0x2000, 0x1000, CRC(6aecba71) SHA1(5a522329b0aeb7707014f3879adfbaf963aed27d) )
	ROM_LOAD( "bahia_pr4", 0x3000, 0x1000, CRC(bf6a989f) SHA1(bdd24b82f6f60ac42f83e5c4b68607c14929028f) )
	ROM_LOAD( "bahia_pr5", 0x4000, 0x1000, CRC(70622047) SHA1(a4e33bfd56c862ca2b130a96828d9d15eae7a5b2) )
	ROM_LOAD( "bahia_pr6", 0x7000, 0x1000, CRC(0373bee7) SHA1(4c0a31feab21872fee7ecd6a04933c0df050b99f) )
ROM_END

ROM_START( dicemstr ) // curiously hand-written stickers say F3 but strings in ROM are F2
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "stella_dice_master_f3_i.ice6",  0x0000, 0x8000, CRC(9897fb87) SHA1(bfb18c1370d9bd12ec61622c0ebbad5c0138e1d8) )
	ROM_LOAD( "stella_dice_master_f3_ii.icd6", 0x8000, 0x8000, CRC(9484cf3b) SHA1(e1104882eaba860ab984c1a37e2f97d4bed08829) ) // 0x0000 - 0x1fff is 0xff filled
ROM_END

ROM_START( dpplpot )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "doppelpot.ice6", 0x0000, 0x4000, CRC(b01d3307) SHA1(8364506e8169432ddec275ef5b53660c01dc209e) )
	ROM_LOAD( "doppelpot.icd6", 0x4000, 0x4000, CRC(153708cb) SHA1(3d15b115ec39c1df42d4437226e83413f495c4d9) )
	ROM_LOAD( "doppelpot.icc5", 0x8000, 0x1000, CRC(135dac6b) SHA1(10873ee64579245eac7069bf84d61550684e67de) )
ROM_END

ROM_START( dpplstrt )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "doppel_start_1_2732.bin", 0x0000, 0x1000, CRC(0a90cc49) SHA1(87a2aaa85ecf0525473d02a2121d13c6615b7188) )
	ROM_LOAD( "doppel_start_2_2732.bin", 0x1000, 0x1000, CRC(720c4262) SHA1(da7f6a399093e4596d84798423201e1445ac38a1) )
	ROM_LOAD( "doppel_start_3_2732.bin", 0x2000, 0x1000, CRC(1d26e43a) SHA1(389a5398536097c3dc3e084f4635908aad17c62d) )
	ROM_LOAD( "doppel_start_4_2732.bin", 0x3000, 0x1000, CRC(d0fa1fdd) SHA1(c56df831c0c112762636675a60a76a0f00732ab2) )
	ROM_LOAD( "doppel_start_5_2732.bin", 0x4000, 0x1000, CRC(40079325) SHA1(9e9f5e3853b3b75c89cd9086814f8a762cc3643b) )
	ROM_LOAD( "doppel_start_6_2732.bin", 0x7000, 0x1000, CRC(1b988121) SHA1(7886ad67d62db61588640f95efc679bc26220691) )
ROM_END

ROM_START( disc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "disc_1_2732.bin", 0x0000, 0x1000, CRC(b9d1f518) SHA1(3a49b248eeb77767e8274a8be523678d7b4aa7d0) )
	ROM_LOAD( "disc_2_2732.bin", 0x1000, 0x1000, CRC(f55fba7c) SHA1(941f2653cb48836bb46f0903f64c3e9d32e67f46) )
	ROM_LOAD( "disc_3_2732.bin", 0x2000, 0x1000, CRC(bd05e77a) SHA1(9e2b5ad6de3eb36cb1f588906a2a10e512b79ce8) )
	ROM_LOAD( "disc_4_2732.bin", 0x3000, 0x1000, CRC(fa8dfac6) SHA1(7e8ba772218f4344070c4fa7e7bc5606b004ddc7) )
	ROM_LOAD( "disc_5_2732.bin", 0x4000, 0x1000, CRC(d036733c) SHA1(f2912f9090b3737ddd1c0702f30a6817fd36ec2c) )
	ROM_LOAD( "disc_6_2732.bin", 0x7000, 0x1000, CRC(d94d5f6e) SHA1(a27df116478b776c549c392297ffa4fdbb073514) )
ROM_END

ROM_START( disc2000 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "disc2000.ice6", 0x0000, 0x4000, CRC(53a66005) SHA1(a5bb63abe8eb631a0fb09496ef6e0ee6c713985c) )
	ROM_LOAD( "disc2000.icd6", 0x4000, 0x4000, CRC(787b6708) SHA1(be990f95b6d04cbe0b9832603204f2a81b0ace3f) )
ROM_END

ROM_START( disc2001 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "disc2001.ice6", 0x0000, 0x4000, CRC(4d128fe1) SHA1(2b9b0a1296ff77b281173fb0fcf667ed3e3ece2b) )
	ROM_LOAD( "disc2001.icd6", 0x4000, 0x4000, CRC(72f6560a) SHA1(3fdc3aaafcc2c185a19a27ccd511d8522fbe0c2e) )
ROM_END

ROM_START( disc3000 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "disc3000.ice6", 0x0000, 0x4000, CRC(6e024e72) SHA1(7198c0cd844d4bc080b2d8654d32d53a04ce8bb4) )
	ROM_LOAD( "disc3000.icd6", 0x4000, 0x4000, CRC(ad88715a) SHA1(660f4044e8f24ad59767ce025966475f9fd56885) )
ROM_END

ROM_START( disciip )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "disc_ii_plus_1-f7m27256f1.ice6", 0x0000, 0x8000, CRC(b2d999f2) SHA1(cb961dfa7d6eec84e742261d6cf66a3e95715101) )
	ROM_LOAD( "disc_ii_plus_2-f7m27256f1.icd6", 0x8000, 0x8000, CRC(c87dd5ce) SHA1(721293fd9ba19bb58b657d6eabd3cd1c7dd74aac) )
ROM_END

ROM_START( discoly )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "olympia_pr1", 0x0000, 0x1000, CRC(531deb63) SHA1(6fddfc5791465c3bcdb554f207de391905085df9) )
	ROM_LOAD( "olympia_pr2", 0x1000, 0x1000, CRC(81e119b2) SHA1(a28eca9394e88b862b15e7bc117b0c4d01d4cf38) )
	ROM_LOAD( "olympia_pr3", 0x2000, 0x1000, CRC(2e156cef) SHA1(e5f145f3e4b7515b949fa7b570ca312c5dd12311) )
	ROM_LOAD( "olympia_pr4", 0x3000, 0x1000, CRC(8f51c072) SHA1(f23fd7d683a5d0765469de8910bb24d0db1b42e2) )
	ROM_LOAD( "olympia_pr5", 0x4000, 0x1000, CRC(f486c0da) SHA1(ebec6f66bffa1057f5fa9b4ab53cedd533036f0a) )
	ROM_LOAD( "olympia_pr6", 0x7000, 0x1000, CRC(e6830d26) SHA1(dbe7a39f24a1dcee298dae3ec1b2d6249a914262) )
ROM_END

ROM_START( discryl )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("disc_royal_1_m27256.ice6", 0x0000, 0x8000, CRC(b286c166) SHA1(08fecc3bf21013f8dbcc08fef3755757c7ff8053))
	ROM_LOAD("disc_royal_2_m27256.icd6", 0x8000, 0x8000, CRC(be2a96c2) SHA1(07efc914832fe549b69a2ec0de5fd5725502ee86))
ROM_END

ROM_START( discrylb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("disc_royal_dob._i.ice6", 0x0000, 0x8000, CRC(eafe92ca) SHA1(5dc172d7cd4efca7a49ac5884ff30fea7be02a30))
	ROM_LOAD("disc_royal_dob.ii.icd6", 0x8000, 0x8000, CRC(ad58476d) SHA1(4565156cac372f45058bce20006692e9afa53ebe))
ROM_END

ROM_START( elitdisc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "elitedisc.ice6", 0x0000, 0x4000, CRC(7f7a2f30) SHA1(01e3ce5fce2c9d51d3f4b8aab7dd67ed4b26d8f4) )
	ROM_LOAD( "elitedisc.icd6", 0x4000, 0x4000, CRC(e56f2360) SHA1(691a6762578daca6ce4581418761dcc07c291fab) )
ROM_END

ROM_START( excellnt )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "excellent.ice5", 0x0800, 0x0800, CRC(b4c573b5) SHA1(5b01b68b8abd48bd293bc9aa507c3285a6e7550f) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.ice6", 0x1800, 0x0800, CRC(f1d53581) SHA1(7aef66149f3427b287d3e9d86cc198dc1ed40d7c) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icd5", 0x2800, 0x0800, CRC(912a5f59) SHA1(3df3ca7eaef8de8e13e93f6a1e6975f8da7ed7a1) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icd6", 0x3800, 0x0800, CRC(5a2b95b4) SHA1(b0d17b327664e8680b163c872109769c4ae42039) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icc5", 0x4800, 0x0800, CRC(ae424805) SHA1(14e12ceebd9fbf6eba96c168e8e7b797b34f7ca5) BAD_DUMP ) // underdumped
ROM_END

ROM_START( extrbltt )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "extrablatt.ice6", 0x0000, 0x8000, CRC(6885cf89) SHA1(30acd5511fb73cb22ae4230fedcf40f385c0d261) )
	ROM_LOAD( "extrablatt.icd6", 0x8000, 0x8000, CRC(5c0cb9bd) SHA1(673d5f8dec7ccce1c4f39dce6be1e9d1ed699047) )
ROM_END

ROM_START( fullhous )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "merkur_full_house_ic1.ice6", 0x0000, 0x8000, CRC(4f984add) SHA1(5a31c96475fe12c4f19658133d97e6bf0536b776) )
	ROM_LOAD( "merkur_full_house_ic2.icd6", 0x8000, 0x8000, CRC(c0f393a0) SHA1(fa16db49d44e813e68701eb77284d04903cf3ec7) )
ROM_END

ROM_START( herzas )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "herz_as_nr1.ice6", 0x0000, 0x4000, CRC(dd4dbaac) SHA1(7fb3c8ea495d5bf989c4aa807ecbe5601c451a73) )
	ROM_LOAD( "herz_as_nr2.icd6", 0x4000, 0x4000, CRC(f2c6a0c4) SHA1(2dad5f79cb5b21905cbefd56b00db1cce1d0b920) )
	ROM_LOAD( "herz_as_nr3.icc5", 0x8000, 0x1000, CRC(1c8657e8) SHA1(836319901c77037c7f414cf0fddf5ab1bdf90ee5) )
ROM_END

ROM_START( herzasf8 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "herz_as_f8_1.ice6", 0x0000, 0x4000, CRC(830bada0) SHA1(8c1fc7e8433c986687b68f6de7610a624e9ac707) )
	ROM_LOAD( "herz_as_f8_2.icd6", 0x4000, 0x4000, CRC(77f88503) SHA1(671a7e819a0361101a30327179132f2661388b72) )
	ROM_LOAD( "herz_as_f8_3.icc5", 0x8000, 0x1000, CRC(b343bfac) SHA1(3772045fcaeb9a87459e481149f27873fc713ca7) )
ROM_END

ROM_START( herzasf1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "herz_as_f10_1.ice6", 0x0000, 0x4000, CRC(e8342e8b) SHA1(e32ad013cdd1480d350ba2e6db18a4489f152301) )
	ROM_LOAD( "herz_as_f10_2.icd6", 0x4000, 0x4000, CRC(03ba2d03) SHA1(12434a3c862b30e40b7c3187066b25b2b7c4eaa6) )
	ROM_LOAD( "herz_as_f10_3.icc5", 0x8000, 0x1000, CRC(f67d2492) SHA1(a2daad380376d19cd9ca37f530a23c01b8d3ce5c) )
ROM_END

ROM_START( juwel )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "juwel.ice6", 0x0000, 0x8000, CRC(6fd9fd6a) SHA1(2ff982750d87be1bc7757bde706d9e329ac29785) )
	ROM_LOAD( "juwel.icd6", 0x8000, 0x8000, CRC(a9ec9e36) SHA1(f7a2b5866988116e0bbeb8a120cae9083d651c5b) )
ROM_END

ROM_START( karoas )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "karoas.ice6", 0x0000, 0x8000, CRC(71c4c39d) SHA1(b188896838a788d5bfc7b18f1bb423a06fe5fcc6) )
	ROM_LOAD( "karoas.icd6", 0x8000, 0x8000, CRC(e1b131bd) SHA1(dc2fbfaf86fa5b161d17a563eae2bc8fc4d19395) )
ROM_END

ROM_START( kniffi )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "kniffi.ice6", 0x0000, 0x8000, CRC(57df5d69) SHA1(78bc9cabf0b4bec5f8c2578d55011f0adc034798) )
	ROM_LOAD( "kniffi.icd6", 0x8000, 0x8000, CRC(1c129cec) SHA1(bad22f18b94c16dba36995ff8daf4d48f4d082a2) )
ROM_END

ROM_START( m21point )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("21_point_27c256_f3_i.ice6",  0x0000, 0x8000, CRC(c2b7b030) SHA1(affb317da2f892213556937fa8857186dccac58a))
	ROM_LOAD("21_point_27c256_f3_ii.icd6", 0x8000, 0x8000, CRC(a466591c) SHA1(c481fc91055b41c9976ff86785f7ee0ce631bd69))
ROM_END

ROM_START( macao )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mega_macao_f1_1.ice6", 0x0000, 0x8000, CRC(b16c9349) SHA1(f07c3dd215bccab088741f95972489284d6a4db9) )
	ROM_LOAD( "mega_macao_f1_2.icd6", 0x8000, 0x8000, CRC(4df216e6) SHA1(28b3ad213f3af9a472c5e7de1c139399677dd825) )
ROM_END

ROM_START(mas)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("mega_as_f5_eprom1_27256.ice6", 0x0000, 0x8000, CRC(17e22e95) SHA1(6fbc11c41c99ee4aac3dcad6647cede25b73f3da))
	ROM_LOAD("mega_as_f5_eprom2_27256.icd6", 0x8000, 0x8000, CRC(12453d57) SHA1(c6c9fa39bdfc7801471bed57e365e37bb02f50b0))
ROM_END

ROM_START( mastro )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "merkur_astro_pr1.ice6", 0x0000, 0x8000, CRC(b2d61886) SHA1(12d2aed9315fc311929edeacd23a38bceadb69f8) )
	ROM_LOAD( "merkur_astro_pr2.icd6", 0x8000, 0x8000, CRC(1e0e42d0) SHA1(46b1eec99331f6656f7cb1542207a79091bce9d9) )
ROM_END

ROM_START( mbistro )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "merkur_bistro_f1.ice6", 0x0000, 0x8000, CRC(e497eeef) SHA1(a5f621627ee80c11697ee5aa9fcd99023e7b6479) )
	ROM_LOAD( "merkur_bistro_f1.icd6", 0x8000, 0x8000, NO_DUMP )
ROM_END

ROM_START( mclub )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("merkur_club_f1_i_st_m27256f1_original.bin",  0x0000, 0x8000, CRC(c78b19b2) SHA1(79aeeee6e82bf987e2aa936575e1e1b251b1a425))
	ROM_LOAD("merkur_club_f1_ii_st_m27256f1_original.bin", 0x8000, 0x8000, CRC(ad3b7d5f) SHA1(317909be8a7853bf83f4f3a2497b1f38a0d954c9))
ROM_END

ROM_START( mmax )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("mega_max_f4_i_st_m27256f1_ice6",  0x0000, 0x8000, CRC(91aa91ba) SHA1(f6c3a6e2e2edeaa79cf0bcdb6af01ddd50eb5488))
	ROM_LOAD("mega_max_f4_ii_st_m27256f1_icd6", 0x8000, 0x8000, CRC(6120080b) SHA1(16209bfe8e75a165ec1e8a5bf2ec7fa078725380))
ROM_END

ROM_START( mtrio )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("mega_trio_f1_ic1.ice6", 0x00000, 0x08000, CRC(9d97fd8c) SHA1(c398610e14c33985a186ae816b759cfdd2b0c6fa))
	ROM_LOAD("mega_trio_f1_ic2.icd6", 0x00000, 0x08000, CRC(b8c2fc4c) SHA1(ddecd608286eb1f3efc6fccce8806a74ad7ce4b8))
ROM_END

ROM_START( rasant )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "rasant_pr_1.ice6", 0x0000, 0x8000, CRC(6abef716) SHA1(8ef2999f6c72f7fb134bfa4ad72ab7be7d12af27) )
	ROM_LOAD( "rasant_pr_2.icd6", 0x8000, 0x8000, CRC(c3a95f74) SHA1(87805ca63a93cc9012e7f2ab4d808c48ba93c919) )
ROM_END

ROM_START( sesam )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("sesam_1_27128.ice6", 0x0000, 0x4000, CRC(29a07575) SHA1(d7e3355e32fcb7a064d8d0fd9b4904be860f7eed))
	ROM_LOAD("sesam_2_27128.icd6", 0x4000, 0x4000, CRC(dac087d0) SHA1(0776e3db14c9b88140887237b5b3d71396b2e6e9))
	ROM_LOAD("sesam_3_2732.icc5",  0x8000, 0x1000, CRC(ec6a2eac) SHA1(6608dd6f477db0df7de3e4262c5b7bcdf1af7ef4))
ROM_END

ROM_START( sherzas )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "super_herz_as_1.ice6", 0x0000, 0x8000, CRC(4212cfaa) SHA1(c428a2a59ae73a92abd08e2b9b2f4feb8ae4dc31) )
	ROM_LOAD( "super_herz_as_2.icd6", 0x8000, 0x8000, CRC(c5cab1a1) SHA1(d3425c94d898369ad22e969a00697e2f0a1305f9) )
ROM_END

ROM_START( sjackpot )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "super_jackpot_i.ice6",  0x0000, 0x4000, CRC(3f14364a) SHA1(4711e2d1aa76a08478177ad7b1f5509b11649f9d) )
	ROM_LOAD( "super_jackpot_ii.icd6", 0x4000, 0x4000, CRC(984d4ca1) SHA1(1da5533f06fb7a1ab8f221c5a58c1afafdd5f862) )
ROM_END

ROM_START( sprmlti )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "super_multi_1.ice6", 0x0000, 0x8000, CRC(fcf126ba) SHA1(89dfd10b6529a92b55d2585c0aa3d0c6b1751550) )
	ROM_LOAD( "super_multi_2.icd6", 0x8000, 0x8000, CRC(0b7a8352) SHA1(ac03b226296085f43634ba96e3e390d3e44c1760) )
ROM_END

ROM_START( sprmltib )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "super_multi_dob_pr1.ice6", 0x0000, 0x8000, CRC(2eb21e6b) SHA1(214f0f26f03551ecda88a64d4e1d75a49f376aae) )
	ROM_LOAD( "super_multi_dob_pr2.icd6", 0x8000, 0x8000, CRC(e3f14918) SHA1(8ba7fc80044b5d27005a53ddbf9e928c74c25d48) )
ROM_END

ROM_START( superpro )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "super_pro_f2_1.ice6", 0x0000, 0x8000, CRC(3294f651) SHA1(3c2dcecda4cbebf29246bbc7705430e96dcafbae) )
	ROM_LOAD( "super_pro_f2_2.icd6", 0x8000, 0x8000, CRC(82802b74) SHA1(8e6ebc429d4e1ccfc5ed6a3bb6fb1747a6a7187a) )
ROM_END

ROM_START( treffasm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "treff_as_medaille_1.ice6", 0x0000, 0x8000, CRC(053831b7) SHA1(d534a9a37d1556c366af523c397d1d5cf97b2a12) )
	ROM_LOAD( "treff_as_medaille_2.icd6", 0x8000, 0x8000, CRC(c2576b97) SHA1(d4f3ca7d7565500b66366b04ef6395c20037b380) )
ROM_END

ROM_START( v4assef1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "venus_4_asse_f1_i.ice6", 0x0000, 0x8000, CRC(29fd7f6a) SHA1(84a8f744e189f0645410c4b7ac36b65f30aa1cc9) )
	ROM_LOAD( "venus_4_asse_f1_ii.icd6", 0x8000, 0x8000, CRC(314dc36c) SHA1(d076651910c713326fe5f0c617ae6e74b6c15334) )
ROM_END

ROM_START( v4assef2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "4asse_f2_1_27256.ice6", 0x0000, 0x8000, CRC(577a1a55) SHA1(b13ac1b761fba6b7e25c18ad3c1edeef8c892089) )
	ROM_LOAD( "4asse_f2_2_27256.icd6", 0x8000, 0x8000, CRC(4f921c1a) SHA1(a6cbca333e29e490306820e2df6c9579a67941c8) )
ROM_END

ROM_START( vmulti )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("venus_multi_1_2732.bin", 0x0000, 0x1000, CRC(3b269798) SHA1(511fb8a86008c124de37d5359681d8379d25891d))
	ROM_LOAD("venus_multi_2_2732.bin", 0x1000, 0x1000, CRC(67e22cec) SHA1(8639cb4496012d9f20f2ece89f15290d017ece2e))
	ROM_LOAD("venus_multi_3_2732.bin", 0x2000, 0x1000, CRC(64bd9bd8) SHA1(c878bdd147e011f4191b5613455648852d395bf1))
	ROM_LOAD("venus_multi_4_2732.bin", 0x3000, 0x1000, CRC(b47e70c1) SHA1(a52cd6568dee16f917c92a41693abd91c4dc2d8c))
	ROM_LOAD("venus_multi_5_2732.bin", 0x4000, 0x1000, CRC(c2905422) SHA1(5c8e3f0440671dc16df32b599239b0435f120778))
	ROM_LOAD("venus_multi_6_2732.bin", 0x7000, 0x1000, CRC(09dd81e7) SHA1(35e9a96d913678a75851a9bf7e7349f93e337805))
ROM_END

} // anonymous namespace

GAMEL( 1982, excellnt,          0, board4040, servicem, stella8085_state, empty_init, ROT0, "ADP",    "Excellent",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1983, bahia,             0, board4040, servicem, stella8085_state, empty_init, ROT0, "ADP",    "Bahia",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1984, disc,              0, board4040, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc",              MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1985, dpplstrt,          0, board4040, servicem, stella8085_state, empty_init, ROT0, "Nova",   "Doppelstart",       MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1986, discoly,           0, board4040, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc Olympia",      MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1986, dpplpot,           0, board4087, servicem, stella8085_state, empty_init, ROT0, "Nova",   "Doppelpot",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1986, elitdisc,          0, board4087, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Elite Disc",        MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1986, sjackpot,          0, board4087, disc,     stella8085_state, empty_init, ROT0, "Nova",   "Super Jackpot",     MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1986, vmulti,            0, board4040, disc,     stella8085_state, empty_init, ROT0, "Venus",  "Multi",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, disc2000,          0, board4087, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc 2000",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1987, disc2001,          0, board4087, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc 2001",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1987, fullhous,          0, board4087, disc,     stella8085_state, empty_init, ROT0, "Merkur", "Full House",        MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, herzas,     herzasf1, board4087, servicem, stella8085_state, empty_init, ROT0, "ADP",    "Herz As",           MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, herzasf8,   herzasf1, board4087, servicem, stella8085_state, empty_init, ROT0, "ADP",    "Herz As (F8)",      MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, herzasf1,          0, board4087, servicem, stella8085_state, empty_init, ROT0, "ADP",    "Herz As (F10)",     MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, kniffi,            0, board4109, servicem, stella8085_state, empty_init, ROT0, "Nova",   "Kniffi",            MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, rasant,            0, board4109, disc,     stella8085_state, empty_init, ROT0, "Venus",  "Rasant",            MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, sesam,             0, board4109, disc,     stella8085_state, empty_init, ROT0, "Merkur", "Sesam",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1987, sprmlti,    sprmltib, board4109, servicem, stella8085_state, empty_init, ROT0, "Venus",  "Super Multi",       MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, sprmltib,          0, board4109, servicem, stella8085_state, empty_init, ROT0, "Venus",  "Super Multi (DOB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1988, extrbltt,          0, board4109, servicem, stella8085_state, empty_init, ROT0, "ADP",    "Extrablatt",        MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1988, juwel,             0, board4109, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Juwel",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1988, mastro,            0, board4109, servicem, stella8085_state, empty_init, ROT0, "ADP",    "Merkur Astro",      MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1988, sherzas,           0, board4109, servicem, stella8085_state, empty_init, ROT0, "Merkur", "Super Herz AS",     MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1989, disc3000,          0, board4087, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc 3000",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1989, discryl,    discrylb, board4087, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc Royal",        MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1989, discrylb,          0, board4087, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc Royal (DOB)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1989, mas,               0, board4382, disc,     stella8085_state, empty_init, ROT0, "MEGA",   "As",                MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1990, v4assef1,   v4assef2, board4382, disc,     stella8085_state, empty_init, ROT0, "Venus",  "4 Asse (F1)",       MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1990, v4assef2,          0, board4382, disc,     stella8085_state, empty_init, ROT0, "Venus",  "4 Asse (F2)",       MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1991, macao,             0, board4382, disc,     stella8085_state, empty_init, ROT0, "MEGA",   "Macao",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1991, mbistro,           0, board4382, disc,     stella8085_state, empty_init, ROT0, "MEGA",   "Bistro",            MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1991, mclub,             0, board4382, disc,     stella8085_state, empty_init, ROT0, "Merkur", "Club",              MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1991, superpro,          0, board4382, servicem, stella8085_state, empty_init, ROT0, "Merkur", "Super Pro",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1991, treffasm,          0, board4382, servicem, stella8085_state, empty_init, ROT0, "Merkur", "Treff As",          MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1992, m21point,          0, board4382, servicem, stella8085_state, empty_init, ROT0, "MEGA",   "21 Point",          MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1992, disciip,           0, board4382, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc II Plus",      MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1992, karoas,            0, board4382, servicem, stella8085_state, empty_init, ROT0, "ADP",    "Karo As",           MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1992, mmax,              0, board4382, servicem, stella8085_state, empty_init, ROT0, "MEGA",   "Max",               MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1992, mtrio,             0, board4382, disc,     stella8085_state, empty_init, ROT0, "MEGA",   "Trio",              MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1993, dicemstr,          0, board4382, servicem, stella8085_state, empty_init, ROT0, "Stella", "Dice Master",       MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
