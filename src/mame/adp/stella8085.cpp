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

#include "cpu/i8085/i8085.h"
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

	void dicemstr(machine_config &config);
	void doppelpot(machine_config &config);
	void excellent(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;

private:
	uint8_t m_digit = 0U;
	uint8_t m_kbd_sl = 0x00;
	bool m_kbd_bd = false;

	required_device<cpu_device> m_maincpu;
	required_device<i8256_device> m_uart;
	required_device<i8279_device> m_kdc;
	required_ioport_array<8> m_tz;
	required_ioport m_dsw;
	output_finder<16> m_digits;
	output_finder<8, 8> m_lamps;
	required_device<beep_device> m_beep;
	emu_timer *m_sound_timer;

	void program_map(address_map &map) ATTR_COLD;
	void large_program_map(address_map &map) ATTR_COLD;
	void small_program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	// I8256 ports
	uint8_t lw_r(); //P1.0-P1.3
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
	void io70(uint8_t data) ATTR_COLD;
	void io71(uint8_t data) ATTR_COLD;
	void sounddev(uint8_t data) ATTR_COLD;
	void io73(uint8_t data) ATTR_COLD;
	uint8_t io9r() ATTR_COLD;
	void io9w(uint8_t data) ATTR_COLD;

	void makesound(uint8_t tone, uint8_t octave, uint8_t length);
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

void stella8085_state::program_map(address_map &map)
{
	map(0x0000, 0x8fff).rom(); // ICE6, ICD6, ICC5
	map(0x9000, 0x933f).rw("rtc", FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0xa000, 0xafff).ram(); // ??
	map(0xc000, 0xc7ff).ram(); // ICC6
}

void stella8085_state::large_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); // ICE6
	map(0x8000, 0x9fff).ram(); // ICC6
	map(0xa000, 0xffff).rom(); // ICD6
}

void stella8085_state::small_program_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0x5000, 0x5fff).ram();
	map(0x6000, 0x633f).rw("rtc", FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0x7000, 0x7fff).rom();
}

void stella8085_state::io_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(stella8085_state::io00));
	map(0x50, 0x51).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0x60, 0x6f).rw("muart", FUNC(i8256_device::read), FUNC(i8256_device::write));
	map(0x70, 0x70).w(FUNC(stella8085_state::io70));
	map(0x71, 0x71).w(FUNC(stella8085_state::io71));
	map(0x72, 0x72).w(FUNC(stella8085_state::sounddev));
	map(0x73, 0x73).w(FUNC(stella8085_state::io73)); // probably extra lamps
	// map(0x80, 0x8f) //Y8 ICC5 empty socket
	map(0x90, 0x9f).rw(FUNC(stella8085_state::io9r),FUNC(stella8085_state::io9w)); //Y9 wired to rtc circuits but somehow memory mapped in hardware
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

	return 0xbf;
}

void stella8085_state::machine1_w(uint8_t data)
{
	popmessage("M1 A %d B %d\nM2 A %d B %d\nM3 A %d B %d\nM4 A %d B %d",BIT(0,data),BIT(1,data),BIT(2,data),BIT(3,data),BIT(4,data),BIT(5,data),BIT(6,data),BIT(7,data));
}

void stella8085_state::machine2_w(uint8_t data)
{
	popmessage("M5 A %d B %d",BIT(4,data),BIT(5,data));
}

/*********************************************
*      I8279 Keyboard-Disply Interface       *
*                                            *
*********************************************/

void stella8085_state::kbd_sl_w(uint8_t data)
{
	m_kbd_sl = data;

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
	if (m_kbd_sl < 8)
		ret = m_tz[m_kbd_sl]->read();
	else
		LOG("read unmapped line %02x\n", m_kbd_sl);
	return ret;
}

void stella8085_state::disp_w(uint8_t data)
{
	if (m_kbd_sl < 8)
	{
		for (int i = 0; i < 8; i++)
		{
			bool lamp_value = BIT(data, i);
			m_lamps[m_kbd_sl][i] = lamp_value;
		}
	}
	else
	{
		output_digit(m_kbd_sl, data);
	}
}

void stella8085_state::output_digit(uint8_t i, uint8_t data)
{
	// Seven-segment encoding for digits 0-9 (abcdefg, no decimal point)
	static const uint8_t bcd_to_7seg[16] =
	{
		0x3f, // 0: 0b00111111
		0x06, // 1: 0b00000110
		0x5b, // 2: 0b01011011
		0x4f, // 3: 0b01001111
		0x66, // 4: 0b01100110
		0x6d, // 5: 0b01101101
		0x7d, // 6: 0b01111101
		0x07, // 7: 0b00000111
		0x7f, // 8: 0b01111111
		0x6f, // 9: 0b01101111

		0x77, // A: 0b01110111
		0x7c, // B: 0b01111100
		0x39, // C: 0b00111001
		0x5e, // D: 0b01011110
		0x79, // E: 0b01111001
		0x71  // F: 0b01110001
	};

	if (i > 7)
	{
		uint8_t debug = data & 0x0f;
		uint8_t cash = data >> 4;

		m_digits[i - 8] = bcd_to_7seg[debug];
		m_digits[i] = bcd_to_7seg[cash];
	}
}

TIMER_CALLBACK_MEMBER(stella8085_state::sound_stop)
{
	m_beep->set_state(0);
}

void stella8085_state::rst65_w(uint8_t state)
{
	m_maincpu->set_input_line(I8085_RST55_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void stella8085_state::io00(uint8_t data)
{
	//old boards
}

uint8_t stella8085_state::io9r()
{
	return 0xff; //old boards
}

void stella8085_state::io9w(uint8_t data)
{
	//old boards
}

void stella8085_state::io70(uint8_t data)
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
	{
		//LOG("Short test\n");
	}

	if (PA7)
		LOG("PA7 high\n");
}

void stella8085_state::io71(uint8_t data)
{
	const bool RS = BIT(data,0);
	const bool GONG = BIT(data,1);
	const bool DG = BIT(data,2);
	const bool UG = BIT(data,3);
	const bool DS = BIT(data,4);
	const bool US = BIT(data,5);
	const bool DM = BIT(data,6);
	const bool UM = BIT(data,7);

	// IO 71 D0 connected through CD4013 flipflop to DIP switch connected to RST55
	if (BIT(m_dsw->read(), 3))
		m_maincpu->set_input_line(I8085_RST55_LINE, RS ? CLEAR_LINE : ASSERT_LINE);
	else
		m_maincpu->set_input_line(I8085_RST55_LINE, CLEAR_LINE);

	if (GONG)
		popmessage("GONG");
	if (US)
		LOG("activating US\n");
	m_beep->set_output_gain(ALL_OUTPUTS,!DG);
	if (UG || DS || DM || UM)
		LOG("UG %d DS %d DM %d UM %d\n", UG,DS,DM,UM);
}

void stella8085_state::sounddev(uint8_t data)
{
	uint8_t tone = data & 0x0f;
	uint8_t length = data & 0x30;
	uint8_t octave = data & 0xc0;
	makesound(tone, octave, 60*(length+1)); // 60 is not correct
}

void stella8085_state::io73(uint8_t data)
{
	//old boards
}

void stella8085_state::makesound(uint8_t tone, uint8_t octave, uint8_t length)
{
	int sfrq = soundfreq(tone, octave);
	LOG("sound freq %02x for %02x ms\n", sfrq, length);
	m_beep->set_clock(sfrq);
	if (length > 0)
	{
		m_beep->set_state(1);
		m_sound_timer->adjust(attotime::from_msec(length), 0);
	}
	else
	{
		m_beep->set_state(0);
	}
}

int stella8085_state::soundfreq(uint8_t channel, uint8_t clockdiv)
{
	const int SOUND_CLOCK = (m_maincpu->clock() / 3);
	const int INT_CLOCK = SOUND_CLOCK >> (4-clockdiv);
	const int C8SHARP = INT_CLOCK / 451;
	const int D8 = INT_CLOCK / 426;
	const int D8SHARP = INT_CLOCK / 402;
	const int E8 = INT_CLOCK / 379;
	const int F8 = INT_CLOCK / 358;
	const int F8SHARP = INT_CLOCK / 338;
	const int G8 = INT_CLOCK / 319;
	const int G8SHARP = INT_CLOCK / 301;
	const int A8 = INT_CLOCK / 284;
	const int A8SHARP = INT_CLOCK / 268;
	const int B8 = INT_CLOCK / 253;
	const int C9 = INT_CLOCK / 239;
	const int C8 = INT_CLOCK / 478; //unused?
	switch (channel)
	{
		case 1:
			return C9;
		case 2:
			return B8;
		case 3:
			return A8SHARP;
		case 4:
			return A8;
		case 5:
			return G8SHARP;
		case 6:
			return G8;
		case 7:
			return F8SHARP;
		case 8:
			return F8;
		case 9:
			return E8;
		case 10:
			return D8SHARP;
		case 11:
			return D8;
		case 12:
			return C8SHARP;
		default:
			return C8;
	}
}

static INPUT_PORTS_START( dicemstr )
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

static INPUT_PORTS_START( disc )
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

	PORT_START("TZ0") //TASTEN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) //TS Door switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) //SK Read data button
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) //ZE2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // Return
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) //STR
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START ) //NF

	PORT_START("TZ1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("DM 0.10")  //LIM1 COIN II
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("DM 1.00")  //LIM2 COIN II
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("DM 2.00")  //LIM3 COIN II
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("DM 5.00")  //LIM4 COIN II
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) //MK
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //LIG

	PORT_START("TZ2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("DM 0.10")  //LIA1 COIN I
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("DM 1.00")  //LIA2 COIN I
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("DM 2.00")  //LIA3 COIN I
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("DM 5.00")  //LIA4 COIN I

	PORT_START("TZ3") //ZUSATZ-EINGAENGE
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) // Risiko Leiter 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) // Risiko Leiter 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // ZE2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Serienuebernahme
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Rueckfuehrung

	PORT_START("TZ4") //MATRIX-EINGAENGE
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TZ5") //MATRIX-EINGAENGE
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

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

void stella8085_state::dicemstr(machine_config &config)
{
	I8085A(config, m_maincpu, 10.240_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::large_program_map);
	m_maincpu->set_addrmap(AS_IO, &stella8085_state::io_map);

	I8256(config, m_uart, 10.240_MHz_XTAL / 2); // divider not verified
	m_uart->int_callback().set_inputline(m_maincpu, I8085_INTR_LINE);

	I8279(config, m_kdc, 10.240_MHz_XTAL / 4); // divider not verified
	m_kdc->out_sl_callback().set(FUNC(stella8085_state::kbd_sl_w));
	m_kdc->out_disp_callback().set(FUNC(stella8085_state::disp_w));
	m_kdc->in_rl_callback().set(FUNC(stella8085_state::kbd_rl_r));
	m_kdc->out_irq_callback().set(FUNC(stella8085_state::rst65_w));

	RTC62421(config, "rtc", 32.768_kHz_XTAL);

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 0)
		.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void stella8085_state::doppelpot(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &stella8085_state::io_map);

	I8256(config, m_uart, 6.144_MHz_XTAL / 2);
	m_uart->int_callback().set_inputline(m_maincpu, I8085_INTR_LINE);
	m_uart->out_p2_callback().set(FUNC(stella8085_state::machine1_w)); //M1-4
	m_uart->in_p1_callback().set(FUNC(stella8085_state::lw_r));
	m_uart->out_p1_callback().set(FUNC(stella8085_state::machine2_w));

	I8279(config, m_kdc, 6.144_MHz_XTAL / 2);
	m_kdc->out_sl_callback().set(FUNC(stella8085_state::kbd_sl_w));
	m_kdc->out_bd_callback().set(FUNC(stella8085_state::kbd_bd_w));
	m_kdc->out_disp_callback().set(FUNC(stella8085_state::disp_w));
	m_kdc->in_rl_callback().set(FUNC(stella8085_state::kbd_rl_r));
	m_kdc->out_irq_callback().set(FUNC(stella8085_state::rst65_w));

	config.set_default_layout(layout_adpservice);

	MC146818(config, "rtc", 32.768_kHz_XTAL);

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 0)
		.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void stella8085_state::excellent(machine_config &config)
{
	doppelpot(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::small_program_map);
}

ROM_START( bahia )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bahia_pr1", 0x0000, 0x1000, CRC(41e7f89c) SHA1(933334e2f78a91e24ec0132b8e7757da5a9d2e02) )
	ROM_LOAD( "bahia_pr2", 0x1000, 0x1000, CRC(ab06262a) SHA1(435f16002054f010e1349f2dbc998a2e5eb50c70) )
	ROM_LOAD( "bahia_pr3", 0x2000, 0x1000, CRC(6aecba71) SHA1(5a522329b0aeb7707014f3879adfbaf963aed27d) )
	ROM_LOAD( "bahia_pr4", 0x3000, 0x1000, CRC(bf6a989f) SHA1(bdd24b82f6f60ac42f83e5c4b68607c14929028f) )
	ROM_LOAD( "bahia_pr5", 0x4000, 0x1000, CRC(70622047) SHA1(a4e33bfd56c862ca2b130a96828d9d15eae7a5b2) )
	ROM_LOAD( "bahia_pr6", 0x7000, 0x1000, CRC(0373bee7) SHA1(4c0a31feab21872fee7ecd6a04933c0df050b99f) )
ROM_END

ROM_START( dicemstr ) // curiously hand-written stickers say F3 but strings in ROM are F2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stella_dice_master_f3_i.ice6",  0x0000, 0x8000, CRC(9897fb87) SHA1(bfb18c1370d9bd12ec61622c0ebbad5c0138e1d8) )
	ROM_LOAD( "stella_dice_master_f3_ii.icd6", 0x8000, 0x8000, CRC(9484cf3b) SHA1(e1104882eaba860ab984c1a37e2f97d4bed08829) ) // 0x0000 - 0x1fff is 0xff filled
ROM_END

ROM_START( doppelpot )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "doppelpot.ice6", 0x0000, 0x4000, CRC(b01d3307) SHA1(8364506e8169432ddec275ef5b53660c01dc209e) )
	ROM_LOAD( "doppelpot.icd6", 0x4000, 0x4000, CRC(153708cb) SHA1(3d15b115ec39c1df42d4437226e83413f495c4d9) )
	ROM_LOAD( "doppelpot.icc5", 0x8000, 0x1000, CRC(135dac6b) SHA1(10873ee64579245eac7069bf84d61550684e67de) )
ROM_END

ROM_START( doppelstart )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "doppel_start_eprom1_2732.bin", 0x0000, 0x1000, CRC(0a90cc49) SHA1(87a2aaa85ecf0525473d02a2121d13c6615b7188) )
	ROM_LOAD( "doppel_start_eprom2_2732.bin", 0x1000, 0x1000, CRC(720c4262) SHA1(da7f6a399093e4596d84798423201e1445ac38a1) )
	ROM_LOAD( "doppel_start_eprom3_2732.bin", 0x2000, 0x1000, CRC(1d26e43a) SHA1(389a5398536097c3dc3e084f4635908aad17c62d) )
	ROM_LOAD( "doppel_start_eprom4_2732.bin", 0x3000, 0x1000, CRC(d0fa1fdd) SHA1(c56df831c0c112762636675a60a76a0f00732ab2) )
	ROM_LOAD( "doppel_start_eprom5_2732.bin", 0x4000, 0x1000, CRC(40079325) SHA1(9e9f5e3853b3b75c89cd9086814f8a762cc3643b) )
	ROM_LOAD( "doppel_start_eprom6_2732.bin", 0x7000, 0x1000, CRC(1b988121) SHA1(7886ad67d62db61588640f95efc679bc26220691) )
ROM_END

ROM_START( disc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "disc_eprom1_2732.bin", 0x0000, 0x1000, CRC(b9d1f518) SHA1(3a49b248eeb77767e8274a8be523678d7b4aa7d0) )
	ROM_LOAD( "disc_eprom2_2732.bin", 0x1000, 0x1000, CRC(f55fba7c) SHA1(941f2653cb48836bb46f0903f64c3e9d32e67f46) )
	ROM_LOAD( "disc_eprom3_2732.bin", 0x2000, 0x1000, CRC(bd05e77a) SHA1(9e2b5ad6de3eb36cb1f588906a2a10e512b79ce8) )
	ROM_LOAD( "disc_eprom4_2732.bin", 0x3000, 0x1000, CRC(fa8dfac6) SHA1(7e8ba772218f4344070c4fa7e7bc5606b004ddc7) )
	ROM_LOAD( "disc_eprom5_2732.bin", 0x4000, 0x1000, CRC(d036733c) SHA1(f2912f9090b3737ddd1c0702f30a6817fd36ec2c) )
	ROM_LOAD( "disc_eprom6_2732.bin", 0x7000, 0x1000, CRC(d94d5f6e) SHA1(a27df116478b776c549c392297ffa4fdbb073514) )
ROM_END

ROM_START( disc2000 )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "disc2000.ice6", 0x0000, 0x4000, CRC(53a66005) SHA1(a5bb63abe8eb631a0fb09496ef6e0ee6c713985c) )
	ROM_LOAD( "disc2000.icd6", 0x4000, 0x4000, CRC(787b6708) SHA1(be990f95b6d04cbe0b9832603204f2a81b0ace3f) )
ROM_END

ROM_START( disc2001 )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "disc2001.ice6", 0x0000, 0x4000, CRC(4d128fe1) SHA1(2b9b0a1296ff77b281173fb0fcf667ed3e3ece2b) )
	ROM_LOAD( "disc2001.icd6", 0x4000, 0x4000, CRC(72f6560a) SHA1(3fdc3aaafcc2c185a19a27ccd511d8522fbe0c2e) )
ROM_END

ROM_START( disc3000 )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "disc3000.ice6", 0x0000, 0x4000, CRC(6e024e72) SHA1(7198c0cd844d4bc080b2d8654d32d53a04ce8bb4) )
	ROM_LOAD( "disc3000.icd6", 0x4000, 0x4000, CRC(ad88715a) SHA1(660f4044e8f24ad59767ce025966475f9fd56885) )
ROM_END

ROM_START( elitedisc )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "elitedisc.ice6", 0x0000, 0x4000, CRC(7f7a2f30) SHA1(01e3ce5fce2c9d51d3f4b8aab7dd67ed4b26d8f4) )
	ROM_LOAD( "elitedisc.icd6", 0x4000, 0x4000, CRC(e56f2360) SHA1(691a6762578daca6ce4581418761dcc07c291fab) )
ROM_END

ROM_START( excellent )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "excellent.ice5", 0x0800, 0x0800, CRC(b4c573b5) SHA1(5b01b68b8abd48bd293bc9aa507c3285a6e7550f) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.ice6", 0x1800, 0x0800, CRC(f1d53581) SHA1(7aef66149f3427b287d3e9d86cc198dc1ed40d7c) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icd5", 0x2800, 0x0800, CRC(912a5f59) SHA1(3df3ca7eaef8de8e13e93f6a1e6975f8da7ed7a1) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icd6", 0x3800, 0x0800, CRC(5a2b95b4) SHA1(b0d17b327664e8680b163c872109769c4ae42039) BAD_DUMP ) // underdumped
	ROM_LOAD( "excellent.icc5", 0x4800, 0x0800, CRC(ae424805) SHA1(14e12ceebd9fbf6eba96c168e8e7b797b34f7ca5) BAD_DUMP ) // underdumped
ROM_END

ROM_START( extrablatt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "extrablatt.ice6", 0x0000, 0x8000, CRC(6885cf89) SHA1(30acd5511fb73cb22ae4230fedcf40f385c0d261) )
	ROM_LOAD( "extrablatt.icd6", 0x8000, 0x8000, CRC(5c0cb9bd) SHA1(673d5f8dec7ccce1c4f39dce6be1e9d1ed699047) )
ROM_END

ROM_START( glucksstern )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "glucksstern.ice6", 0x00000, 0x20000, CRC(8e969bae) SHA1(bf66d491932b77dab4c6b15ec7fbf470223636ac) )
	ROM_LOAD( "glucksstern.icd6", 0x20000, 0x20000, CRC(f31b860a) SHA1(7b016bb7d0699cfe7165c0abb2c1bbcb944cdc86) )
ROM_END

ROM_START( juwel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "juwel.ice6", 0x0000, 0x8000, CRC(6fd9fd6a) SHA1(2ff982750d87be1bc7757bde706d9e329ac29785) )
	ROM_LOAD( "juwel.icd6", 0x8000, 0x8000, CRC(a9ec9e36) SHA1(f7a2b5866988116e0bbeb8a120cae9083d651c5b) )
ROM_END

ROM_START( karoas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "karoas.ice6", 0x0000, 0x8000, CRC(71c4c39d) SHA1(b188896838a788d5bfc7b18f1bb423a06fe5fcc6) )
	ROM_LOAD( "karoas.icd6", 0x8000, 0x8000, CRC(e1b131bd) SHA1(dc2fbfaf86fa5b161d17a563eae2bc8fc4d19395) )
ROM_END

ROM_START( kniffi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kniffi.ice6", 0x0000, 0x8000, CRC(57df5d69) SHA1(78bc9cabf0b4bec5f8c2578d55011f0adc034798) )
	ROM_LOAD( "kniffi.icd6", 0x8000, 0x8000, CRC(1c129cec) SHA1(bad22f18b94c16dba36995ff8daf4d48f4d082a2) )
ROM_END

ROM_START( macao )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mega_macao_f1_1.ice6", 0x0000, 0x8000, CRC(b16c9349) SHA1(f07c3dd215bccab088741f95972489284d6a4db9) )
	ROM_LOAD( "mega_macao_f1_2.icd6", 0x8000, 0x8000, CRC(4df216e6) SHA1(28b3ad213f3af9a472c5e7de1c139399677dd825) )
ROM_END

ROM_START( rasant )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rasant_pr_1.ice6", 0x0000, 0x8000, CRC(6abef716) SHA1(8ef2999f6c72f7fb134bfa4ad72ab7be7d12af27) )
	ROM_LOAD( "rasant_pr_2.icd6", 0x8000, 0x8000, CRC(c3a95f74) SHA1(87805ca63a93cc9012e7f2ab4d808c48ba93c919) )
ROM_END

ROM_START( supermultib )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super_multi_dob_pr1.ice6", 0x0000, 0x8000, CRC(2eb21e6b) SHA1(214f0f26f03551ecda88a64d4e1d75a49f376aae) )
	ROM_LOAD( "super_multi_dob_pr2.icd6", 0x8000, 0x8000, CRC(e3f14918) SHA1(8ba7fc80044b5d27005a53ddbf9e928c74c25d48) )
ROM_END

} // anonymous namespace

GAMEL( 1982, excellent,   0, excellent, dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Excellent",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1983, bahia,       0, excellent, dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Bahia",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1984, disc,        0, excellent, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc",              MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1985, doppelstart, 0, excellent, dicemstr, stella8085_state, empty_init, ROT0, "Nova",   "Doppelstart",       MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1986, doppelpot,   0, doppelpot, dicemstr, stella8085_state, empty_init, ROT0, "Nova",   "Doppelpot",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1986, elitedisc,   0, doppelpot, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Elite Disc",        MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1986, rasant,      0, doppelpot, disc,     stella8085_state, empty_init, ROT0, "Venus",  "Rasant",            MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1987, disc2000,    0, doppelpot, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc 2000",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1987, disc2001,    0, doppelpot, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc 2001",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1987, kniffi,      0, dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "Nova",   "Kniffi",            MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1987, supermultib, 0, doppelpot, dicemstr, stella8085_state, empty_init, ROT0, "Venus",  "Super Multi (DOB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1988, extrablatt,  0, dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Extrablatt",        MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1988, juwel,       0, dicemstr,  disc,     stella8085_state, empty_init, ROT0, "ADP",    "Juwel",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1989, disc3000,    0, doppelpot, disc,     stella8085_state, empty_init, ROT0, "ADP",    "Disc 3000",         MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_disc2000 )
GAMEL( 1991, macao,       0, dicemstr,  disc,     stella8085_state, empty_init, ROT0, "MEGA",   "Macao",             MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1992, karoas,      0, dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "ADP",    "Karo As",           MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
// 'STELLA DICE MASTER F2' and 'COPYRIGHT BY ADP LUEBBECKE GERMANY 1993' in ROM
GAMEL( 1993, dicemstr,    0, dicemstr,  dicemstr, stella8085_state, empty_init, ROT0, "Stella", "Dice Master",       MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
GAMEL( 1998, glucksstern, 0, dicemstr,  disc,     stella8085_state, empty_init, ROT0, "ADP",    u8"Glücks-Stern",    MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_adpservice )
