// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote (ClawGrip)
/***************************************************************************

   M89 hardware for kiddie rides from Falgàs.

   Base Falgàs M89 PCB:

   _|_|_|_|___|_|_|_|___|_|_|_|_|_|_|____
  |          _______   _______ _______  |
  | _______  TIC206M   TIC206M TIC206M  |
  |                                     |
  |                   __________________|
  |                  | AY8910A         ||
  |                  |_________________||
  |                                     |
  | 7805CV            :::::::::         |
  |                   __________________|
  |                  | 82C55           ||
  |                  |_________________||
  | TDA7241B             _______________|
  |                     | GM76C28A     ||
  |                     |______________||
  |                    _________________|
  |                   | EPROM          ||
  |                   |________________||
  |    __________           __________  |
  |   |_PAL16V8_|          |SN74LS373N  |
  |  __________       __________________|
  | |_PAL16V8_|      |OKI M80C85A-2    ||
  |                  |_________________||
  |  6.000 MHz Xtal  _________ ________ |
  |                MC14020BCP MC14020BCP|
  | ____________RISER_PCB_______________|
  |_____________________________________|

  The riser PCB contains:
   -4 LEDs (motor on, coin input, timer-sound, light).
   -Bank of 4 DIP switches for timer configuration.
   -Bank of 4 DIP switches for coinage configuration.
   -Volume knob.

   Coche Bully PCB, silkscreened M89-4 and M89-4 N/E:

     U1   OKI M80C85A-2, 6.000 MHz crystal
     U2   27C256 EPROM (27C512 on other games)
     U3   GoldStar GM76C28A-10 2 KiB SRAM
     U4   74LS373
     U5   MC14020BCP
     U6   MC14020BCP
     U8   Microchip AY-3-8910A
     U9   82C55 socket, empty
     U10  PALCE16V8 labelled M894-A
     U11  PALCE16V8 labelled M894-BT
     U12  TDA7241 audio amplifier
     CEX  16 pin header to the video PCB (Rally Montecarlo)

   Connectors on the back of the box:

     CIO  1-3 player switches (common on 4), 5-6 speaker, 7-10 two lamps and
          their common
     CSM  START, COIN 1 and COIN 2 switches, a lamp and two coin counters
     CAL  1-2 12 V lights, 3-4 12 V timer and sound, 5-6 motor control

   RST 7.5 is a 5 kHz time base, the only rate that gives the ride times of
   the operator sheet (50 interrupts per main loop pass, 100 passes per
   second).  Its source is unknown.

   AY port A bit 4 is the START lamp, bits 5-6 the car lights, and bit 7
   toggles once per main loop pass (watchdog?).

   The AY clock is unknown: the note table is tuned for 2 MHz, but CLK OUT
   (3 MHz) is too fast for the AY-3-8910A, so 1.5 MHz is assumed.


   Optional video PCB (25291):
   ______________________________________
  |   Power conn -> ::::::  :::::::::: <- Conn to M89 (timer, sound)
  |                   __________________|
  |                  | NEC D8155HC     ||
  |                  |_________________||
  |                                     |
  |                     ________________|
  |           __       | GM76C28A-10   ||
  |          | |       |_______________||
SN74LS14N -> | |       _________________|
  |          |_|      | EPROM          ||
  |                   |________________||
  |      __________        __________   |
  |     |_PAL16V8_|       |SN74LS373N|  |
  |                   __________________|
  |             Xtal | OKI M80C85A-2   ||
  |        6.000 MHz |_________________||
  |                   __________________|
  |             Xtal | TMS9129NL       ||
  |    10.738635 MHz |_________________||
  |                           __________|
  |                          |UD61464DC||
  |                           __________|
  |                          |UD61464DC||
  |  ___   ___   ___     _________  :   |
  |LM318P LM318P LM318P |CD4016BCN  : <- Conn to monitor
  |                                 :   |
  |_____________________________________|

   The video board runs the race game.  Its program sets the 8155 timer to
   3000 counts of CLK OUT, a 1 ms tick, assumed to reach TRAP (RST 7.5 shares
   the handler; the VDP interrupt stays disabled).

   The main board keeps coins, motor, sound and the engine model, and talks to
   the video board through the CEX ribbon, which reverses the bit order both
   ways.  As seen by the video board:

     received  bit 7 set:   start a race; bits 1-3 TIMER switches 1-3,
                            bits 4-6 credits waiting
               bit 7 clear: speed (0-31) in bits 1-5
     sent      bit 0  no race running
               bit 1  extended play reached, original time over
               bit 2  car going past
               bit 4  pushed out by a curve
               bit 5  off the road
               bit 6  crash
               bit 7  heartbeat, toggled on every exchange


  TODO:
  * measure the RST 7.5 rate and the AY clock on a real board
  * resistor values of the DAC and how it is mixed with the AY
  * what drives the COIN INPUT, TIMER-SOUND and LIGHT LEDs
  * the watchdog, if there is one
  * fantcar: which bodywork lamps pattern0-3 and ride_lamp1 drive

***************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/tms9928a.h"

#include "screen.h"
#include "speaker.h"

#include "falgas_fantcar.lh"
#include "falgas_fantcar_m87.lh"
#include "falgas_m89.lh"
#include "falgas_rmontecarlo.lh"

namespace
{

class falgasm89_state : public driver_device
{
public:
	falgasm89_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_psg(*this, "psg")
		, m_dac(*this, "dac")
		, m_timer_sw(*this, "TIMER")
		, m_cost_sw(*this, "COST")
		, m_lamps(*this, "lamp%u", 0U)
	{
	}

	void falgasm89_simple(machine_config &config) ATTR_COLD;

	ioport_value dsw_rows_r();

protected:
	virtual void machine_start() override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<ay8910_device> m_psg;
	required_device<dac_4bit_r2r_device> m_dac;

	u8 m_psg_pa = 0xff;

private:
	required_ioport m_timer_sw;
	required_ioport m_cost_sw;
	output_finder<3> m_lamps;

	void mem_map(address_map &map) ATTR_COLD;

	void psg_pa_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(timebase);
};

class fantcar_state : public falgasm89_state
{
public:
	fantcar_state(const machine_config &mconfig, device_type type, const char *tag)
		: falgasm89_state(mconfig, type, tag)
		, m_psg2(*this, "psg2")
		, m_controls(*this, "CONTROLS")
		, m_scanner(*this, "scanner%u", 0U)
		, m_speed(*this, "speed%u", 0U)
		, m_voice(*this, "voice%u", 0U)
		, m_pattern(*this, "pattern%u", 0U)
		, m_ride_lamp(*this, "ride_lamp%u", 0U)
		, m_turbo_lamp(*this, "turbo_lamp")
		, m_siren_lamp(*this, "siren_lamp")
		, m_radar_lamp(*this, "radar_lamp")
		, m_blink_lamp(*this, "blink_lamp")
		, m_start_lamp(*this, "start_lamp")
	{
	}

	void fantcar(machine_config &config) ATTR_COLD;

	ioport_value pb5_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<ay8910_device> m_psg2;
	required_ioport m_controls;
	output_finder<8> m_scanner;
	output_finder<4> m_speed;
	output_finder<4> m_voice;
	output_finder<4> m_pattern;
	output_finder<2> m_ride_lamp;
	output_finder<> m_turbo_lamp;
	output_finder<> m_siren_lamp;
	output_finder<> m_radar_lamp;
	output_finder<> m_blink_lamp;
	output_finder<> m_start_lamp;

	u8 m_psg2_pa = 0;
	u16 m_shift[2] = { 0, 0 };

	void fantcar_pa_w(u8 data);
	void psg2_pa_w(u8 data);
	void chain_w(unsigned chain, u16 data);

	void fantcar_io_map(address_map &map) ATTR_COLD;
};

class falgasm89_video_state : public falgasm89_state
{
public:
	falgasm89_video_state(const machine_config &mconfig, device_type type, const char *tag)
		: falgasm89_state(mconfig, type, tag)
		, m_videocpu(*this, "videocpu")
		, m_ppi(*this, "ppi")
		, m_i8155(*this, "i8155")
		, m_vdp(*this, "vdp")
	{
	}

	void falgasm89_video(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i8085a_cpu_device> m_videocpu;
	required_device<i8255_device> m_ppi;
	required_device<i8155_device> m_i8155;
	required_device<tms9129_device> m_vdp;

	u8 m_to_video = 0;
	u8 m_to_main = 0;

	void main_io_map(address_map &map) ATTR_COLD;
	void video_mem_map(address_map &map) ATTR_COLD;
	void video_io_map(address_map &map) ATTR_COLD;

	void to_video_w(u8 data);
	void to_main_w(u8 data);
	TIMER_CALLBACK_MEMBER(set_to_video);
	TIMER_CALLBACK_MEMBER(set_to_main);
};


/***************************************************************************

   Falgàs Micro-87 hardware.

   Single board silkscreened 9187B, with the LEDs, knobs and selector on a
   metal front panel and the output latches on daughter boards.  Parts on the
   fantcar87 board (EPROM labelled "DATA 15.2.88 / SOFT: MIKEL ES / HARD:
   M.87.E"):

     NEC D8085AHC        CPU, 4 MHz crystal
     SN74LS373 PC        address latch
     T74LS138B1          I/O decoder
     74LS32 PC           /
     KS74HCTLS00N        \ read and write strobes
     HCF4093BE (SGS)     interrupt astables and watchdog monostable
     NM27C256Q           EPROM
     KM6816AL-15         2 KiB SRAM
     AY-3-8910 (GI)      sound and the only general purpose I/O
     3021 optotriac and triac for the mains loads

   Daughter boards:

     driver board    14187 FANTASTIC CAR: 74LS245, four 74LS373 latches, four
                     ULN2003A, TIC216D triacs for the 12 V lamps, two 74LS02
                     and the player control inputs
     display board   20187: three 7-segment digits and two HCF4511BE; the
                     third digit is wired as a fixed 0
     lamp board      261286: the 40 bulbs of the dashboard centre panel

   Front panel: breaker, 220 V neon, FAIL LED, SOUND-TIMER and LIGHT fuses,
   four SELECTOR DIP switches, SOUND knob, TIMER knob (60 to 150 seconds) and
   the MOTOR ON, COIN INPUT, SOUND, TIMER and LIGHT LEDs.

   TRAP comes from an astable at about 4.8 kHz, measured on recordings of a
   real machine; it feeds the DAC and every 50 interrupts pace the main loop.
   RST 7.5 comes from a second astable, set by the TIMER knob, and times the
   ride.

   TODO:
   * what drives the SOUND, TIMER, LIGHT and FAIL LEDs
   * what the d8h strobe does (written once at boot)
   * SOD is high for exactly as long as the ride: motor enable, or the TIMER
     LED?
   * measure the RC values of the watchdog and of the two astables
   * which bodywork lamps pattern0-3 and ride_lamp1 drive

***************************************************************************/

class falgasm87_state : public driver_device
{
public:
	falgasm87_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_psg(*this, "psg")
		, m_dac(*this, "dac")
		, m_traptimer(*this, "traptimer")
		, m_ridetimer(*this, "ridetimer")
		, m_watchdog(*this, "watchdog")
		, m_controls(*this, "CONTROLS")
		, m_dial(*this, "DIAL")
		, m_leds(*this, "led%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_scanner(*this, "scanner%u", 0U)
		, m_speed(*this, "speed%u", 0U)
		, m_voice(*this, "voice%u", 0U)
		, m_pattern(*this, "pattern%u", 0U)
		, m_ride_lamp(*this, "ride_lamp%u", 0U)
		, m_turbo_lamp(*this, "turbo_lamp")
		, m_siren_lamp(*this, "siren_lamp")
		, m_radar_lamp(*this, "radar_lamp")
		, m_blink_lamp(*this, "blink_lamp")
		, m_start_lamp(*this, "start_lamp")
	{
	}

	void falgasm87(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<ay8910_device> m_psg;
	required_device<dac_4bit_r2r_device> m_dac;
	required_device<timer_device> m_traptimer;
	required_device<timer_device> m_ridetimer;
	required_device<watchdog_timer_device> m_watchdog;
	required_ioport m_controls;
	required_ioport m_dial;
	output_finder<2> m_leds;
	output_finder<3> m_digits;
	output_finder<8> m_scanner;
	output_finder<4> m_speed;
	output_finder<4> m_voice;
	output_finder<4> m_pattern;
	output_finder<2> m_ride_lamp;
	output_finder<> m_turbo_lamp;
	output_finder<> m_siren_lamp;
	output_finder<> m_radar_lamp;
	output_finder<> m_blink_lamp;
	output_finder<> m_start_lamp;

	u8 m_display;
	u8 m_latched[2];
	u8 m_display_on;
	u8 m_sod;
	u8 m_trap;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void psg_pa_w(u8 data);
	u8 controls_r();
	void d8_w(u8 data);
	void lamps_e0_w(u8 data);
	void display_w(u8 data);
	void update_display();
	void scanner_w(u8 data);
	void lamps_f8_w(u8 data);
	void sod_w(int state);

	attotime dial_period() const;

	TIMER_DEVICE_CALLBACK_MEMBER(trap_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(ride_tick);
};


void falgasm89_state::machine_start()
{
	save_item(NAME(m_psg_pa));
}

void falgasm89_state::psg_pa_w(u8 data)
{
	m_psg_pa = data;
	m_dac->write(data & 0x0f);

	for (int i = 0; i < 3; i++)
		m_lamps[i] = BIT(data, 4 + i);
}

ioport_value falgasm89_state::dsw_rows_r()
{
	u8 const timer = m_timer_sw->read();
	u8 const cost = m_cost_sw->read();
	u8 const rows[4] = { u8(timer >> 2), timer, u8(cost >> 2), cost };

	ioport_value result = 0x03;
	for (int i = 0; i < 4; i++)
		if (!BIT(m_psg_pa, i))
			result &= rows[i];

	return result;
}

TIMER_DEVICE_CALLBACK_MEMBER(falgasm89_state::timebase)
{
	m_maincpu->pulse_input_line(I8085_RST75_LINE, attotime::zero);
}

void falgasm89_state::mem_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("maincpu", 0);
	map(0xf800, 0xffff).ram(); // GM76C28A-10
}

void falgasm89_state::io_map(address_map &map)
{
	map(0x00, 0x00).rw(m_psg, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x04, 0x04).w(m_psg, FUNC(ay8910_device::address_w));
}

void falgasm89_video_state::machine_start()
{
	falgasm89_state::machine_start();

	save_item(NAME(m_to_video));
	save_item(NAME(m_to_main));
}

void falgasm89_video_state::main_io_map(address_map &map)
{
	io_map(map);
	map(0x18, 0x1b).mirror(0x80).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void falgasm89_video_state::video_mem_map(address_map &map)
{
	map(0x0000, 0xf7ff).rom().region("videocpu", 0);
	map(0xf800, 0xffff).ram(); // GM76C28A-10; the 8155 RAM is unused
}

void falgasm89_video_state::video_io_map(address_map &map)
{
	map(0x00, 0x07).rw(m_i8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x08, 0x08).rw(m_vdp, FUNC(tms9928a_device::vram_read), FUNC(tms9928a_device::vram_write));
	map(0x09, 0x09).rw(m_vdp, FUNC(tms9928a_device::register_read), FUNC(tms9928a_device::register_write));
}

void falgasm89_video_state::to_video_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(falgasm89_video_state::set_to_video), this), data);
}

void falgasm89_video_state::to_main_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(falgasm89_video_state::set_to_main), this), data);
}

TIMER_CALLBACK_MEMBER(falgasm89_video_state::set_to_video)
{
	m_to_video = bitswap<8>(u8(param), 0, 1, 2, 3, 4, 5, 6, 7);
}

TIMER_CALLBACK_MEMBER(falgasm89_video_state::set_to_main)
{
	m_to_main = bitswap<8>(u8(param), 0, 1, 2, 3, 4, 5, 6, 7);
}


/***************************************************************************

   Fantastic Car on M89 hardware: the Micro-87 game with a second AY that
   shifts out the dashboard lamps (see the Micro-87 dashboard notes).  This
   program has no R.P.M display code.

***************************************************************************/

void fantcar_state::machine_start()
{
	falgasm89_state::machine_start();

	save_item(NAME(m_psg2_pa));
	save_item(NAME(m_shift));
}

ioport_value fantcar_state::pb5_r()
{
	u8 const controls = m_controls->read();

	if (BIT(m_psg_pa, 5) && !BIT(controls, 1))
		return 0;
	if (BIT(m_psg_pa, 6) && !BIT(controls, 0))
		return 0;

	return 1;
}

void fantcar_state::fantcar_pa_w(u8 data)
{
	m_psg_pa = data;
	m_dac->write(data & 0x0f);
	m_start_lamp = BIT(data, 4);
}

void fantcar_state::psg2_pa_w(u8 data)
{
	if (BIT(m_psg2_pa, 0) && !BIT(data, 0))
	{
		for (int i = 0; i < 2; i++)
			if (BIT(data, 1 + i))
				m_shift[i] = (m_shift[i] >> 1) | (BIT(data, 7) << 15);
	}

	for (int i = 0; i < 2; i++)
		if (BIT(m_psg2_pa, 1 + i) && !BIT(data, 1 + i))
			chain_w(i, m_shift[i]);

	m_psg2_pa = data;
}

void fantcar_state::chain_w(unsigned chain, u16 data)
{
	if (!chain)
	{
		m_blink_lamp = BIT(data, 0);
		m_turbo_lamp = BIT(data, 1);
		m_siren_lamp = BIT(data, 2);
		m_radar_lamp = BIT(data, 3);
		m_ride_lamp[0] = BIT(data, 4);
		m_speed[3] = BIT(data, 5);
		m_speed[2] = BIT(data, 6);
		m_speed[1] = BIT(data, 7);
		m_speed[0] = BIT(data, 8);
		m_scanner[0] = BIT(data, 9);
		m_scanner[1] = BIT(data, 10);
		m_scanner[2] = BIT(data, 11);
		m_scanner[3] = BIT(data, 12);
		m_scanner[6] = BIT(data, 13);
		m_scanner[5] = BIT(data, 14);
		m_scanner[4] = BIT(data, 15);
	}
	else
	{
		// bits 6 and 7 carry the same values as bits 14 and 15
		m_voice[3] = BIT(data, 0);
		m_ride_lamp[1] = BIT(data, 1);
		m_scanner[7] = BIT(data, 2);
		m_voice[2] = BIT(data, 3);
		m_voice[1] = BIT(data, 4);
		m_voice[0] = BIT(data, 5);
		m_pattern[3] = BIT(data, 8);
		m_pattern[2] = BIT(data, 9);
		m_pattern[1] = BIT(data, 14);
		m_pattern[0] = BIT(data, 15);
	}
}

void fantcar_state::fantcar_io_map(address_map &map)
{
	io_map(map);
	map(0x08, 0x08).rw(m_psg2, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x0c, 0x0c).w(m_psg2, FUNC(ay8910_device::address_w));
}


/***************************************************************************
   Falgàs Micro-87 implementation
***************************************************************************/

void falgasm87_state::machine_start()
{
	m_display = 0;
	m_latched[0] = m_latched[1] = 0;
	m_display_on = 0;
	m_sod = 0;
	m_trap = 0;

	save_item(NAME(m_display));
	save_item(NAME(m_latched));
	save_item(NAME(m_display_on));
	save_item(NAME(m_sod));
	save_item(NAME(m_trap));
}

void falgasm87_state::machine_reset()
{
	m_ridetimer->adjust(dial_period());
}

void falgasm87_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).mirror(0x7800).ram(); // KM6816AL-15; mirroring guessed
}

void falgasm87_state::io_map(address_map &map)
{
	map(0xc0, 0xc7).rw(m_psg, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xc8, 0xcf).w(m_psg, FUNC(ay8910_device::address_w));
	map(0xd0, 0xd7).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0xd8, 0xdf).w(FUNC(falgasm87_state::d8_w));
	map(0xe0, 0xe7).w(FUNC(falgasm87_state::lamps_e0_w));
	map(0xe8, 0xef).w(FUNC(falgasm87_state::display_w));
	map(0xf0, 0xf7).w(FUNC(falgasm87_state::scanner_w));
	map(0xf8, 0xff).rw(FUNC(falgasm87_state::controls_r), FUNC(falgasm87_state::lamps_f8_w));
}

void falgasm87_state::psg_pa_w(u8 data)
{
	m_dac->write(data >> 4);

	// set while credits wait for START: taken to light the START button and
	// the COIN INPUT LED
	m_start_lamp = BIT(data, 1);
	m_leds[1] = BIT(data, 1);
}

u8 falgasm87_state::controls_r()
{
	return m_controls->read();
}

void falgasm87_state::d8_w(u8 data)
{
	logerror("%s: D8 strobe = %02x\n", machine().describe_context(), data);
}

/*
   Fantastic Car dashboard (KITT, from Knight Rider):

     scanner0-7   window of three sweeping the two columns of squares of the
                  centre panel and the bar in the nose of the car
     speed0-3     100/200/250/300 speedometer
     voice0-3     KITT's voice: level n of the middle column of bars (voice0
                  innermost) and the opposite level of both side columns
     pattern0-3   not on the dashboard; probably bodywork lamps
     blink_lamp   OIL lamp
     ride_lamp0   lit for the whole ride; taken to switch the supply of the
                  R.P.M display, which is dark outside the ride
     ride_lamp1   not on the dashboard
*/
void falgasm87_state::lamps_e0_w(u8 data)
{
	m_speed[0] = BIT(data, 0);
	m_ride_lamp[0] = BIT(data, 1);
	m_turbo_lamp = BIT(data, 2);
	m_speed[2] = BIT(data, 3);
	m_blink_lamp = BIT(data, 4);
	m_siren_lamp = BIT(data, 5);
	m_radar_lamp = BIT(data, 6);
	m_speed[3] = BIT(data, 7);

	m_display_on = BIT(data, 1);
	update_display();
}

// bits 3-6 are the BCD digit, in the only order that decodes the R.P.M table;
// bits 7 and 2 latch it into the tens and units HCF4511
void falgasm87_state::display_w(u8 data)
{
	// CD4511 patterns: tail-less 6 and 9, 10-15 blank
	static constexpr u8 PATTERNS[16] =
	{
		0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07,
		0x7f, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	u8 const bcd = bitswap<4>(data, 4, 5, 6, 3);
	if (BIT(data, 7) && !BIT(m_display, 7))
		m_latched[0] = PATTERNS[bcd];
	if (BIT(data, 2) && !BIT(m_display, 2))
		m_latched[1] = PATTERNS[bcd];
	m_display = data;
	update_display();

	m_pattern[0] = BIT(data, 0);
	m_speed[1] = BIT(data, 1);
}

void falgasm87_state::update_display()
{
	u8 const segs[3] = { m_latched[0], m_latched[1], 0x3f };
	for (int i = 0; i < 3; i++)
		m_digits[i] = m_display_on ? segs[i] : 0;
}

void falgasm87_state::scanner_w(u8 data)
{
	m_scanner[0] = BIT(data, 7);
	m_scanner[1] = BIT(data, 5);
	m_scanner[2] = BIT(data, 3);
	m_scanner[3] = BIT(data, 1);
	m_scanner[4] = BIT(data, 6);
	m_scanner[5] = BIT(data, 4);
	m_scanner[6] = BIT(data, 2);
	m_scanner[7] = BIT(data, 0);
}

void falgasm87_state::lamps_f8_w(u8 data)
{
	m_ride_lamp[1] = BIT(data, 0);
	m_voice[1] = BIT(data, 1);
	m_pattern[1] = BIT(data, 2);
	m_voice[0] = BIT(data, 3);
	m_pattern[2] = BIT(data, 4);
	m_voice[2] = BIT(data, 5);
	m_pattern[3] = BIT(data, 6);
	m_voice[3] = BIT(data, 7);
}

void falgasm87_state::sod_w(int state)
{
	if (m_sod != state)
		logerror("%s: SOD = %d\n", machine().describe_context(), state);
	m_sod = state;
	m_leds[0] = state; // motor on
}

/*
   The TIMER knob sets the frequency of the RST 7.5 astable.  A COIN1 ride
   lasts 2040 ticks, so the 60 to 150 second scale of the knob spans 34.0 to
   13.6 Hz; COIN2 credits use a shorter prescaler.
*/
attotime falgasm87_state::dial_period() const
{
	const double seconds = 60.0 + (m_dial->read() & 0xff) * 0.9;
	return attotime::from_hz(2040.0 / seconds);
}

// TRAP needs the line held high until it is taken: square wave from the astable
TIMER_DEVICE_CALLBACK_MEMBER(falgasm87_state::trap_tick)
{
	m_trap = !m_trap;
	m_maincpu->set_input_line(I8085_TRAP_LINE, m_trap ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(falgasm87_state::ride_tick)
{
	m_maincpu->pulse_input_line(I8085_RST75_LINE, attotime::zero);
	m_ridetimer->adjust(dial_period());
}


// Settings from the M89 operator sheet; defaults as found on a Coche Bully board
INPUT_PORTS_START(falgasm89)
	PORT_START("IN0")
	PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(falgasm89_state::dsw_rows_r))
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_COIN1)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_COIN2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_BUTTON1)
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_BUTTON2)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_BUTTON3)

	PORT_START("TIMER")
	PORT_DIPNAME(0x07, 0x03, "Ride Time") PORT_DIPLOCATION("TIMER:1,2,3")
	PORT_DIPSETTING(   0x07, "8 seconds")
	PORT_DIPSETTING(   0x06, "15 seconds")
	PORT_DIPSETTING(   0x05, "30 seconds")
	PORT_DIPSETTING(   0x04, "45 seconds")
	PORT_DIPSETTING(   0x03, "60 seconds")
	PORT_DIPSETTING(   0x02, "75 seconds")
	PORT_DIPSETTING(   0x01, "90 seconds")
	PORT_DIPSETTING(   0x00, "105 seconds")
	PORT_DIPNAME(0x08, 0x00, DEF_STR(Demo_Sounds)) PORT_DIPLOCATION("TIMER:4") // "Reclamo" in the operator sheet
	PORT_DIPSETTING(   0x08, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))

	PORT_START("COST") // the switches are lettered A-D on the panel
	PORT_DIPNAME(0x03, 0x00, DEF_STR(Coin_B)) PORT_DIPLOCATION("COST:1,2") // COIN 2
	PORT_DIPSETTING(   0x03, DEF_STR(1C_2C))
	PORT_DIPSETTING(   0x02, DEF_STR(1C_3C))
	PORT_DIPSETTING(   0x01, DEF_STR(1C_4C))
	PORT_DIPSETTING(   0x00, DEF_STR(1C_5C))
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR(Coin_A)) PORT_DIPLOCATION("COST:3,4") // COIN 1
	PORT_DIPSETTING(   0x00, DEF_STR(4C_1C))
	PORT_DIPSETTING(   0x04, DEF_STR(3C_1C))
	PORT_DIPSETTING(   0x08, DEF_STR(2C_1C))
	PORT_DIPSETTING(   0x0c, DEF_STR(1C_1C))
INPUT_PORTS_END

INPUT_PORTS_START(cbully)
	PORT_INCLUDE(falgasm89)

	PORT_MODIFY("IN0")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_BUTTON1) PORT_NAME("Horn")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_BUTTON2) PORT_NAME("Accelerator")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_UNUSED)
INPUT_PORTS_END

INPUT_PORTS_START(rmontecarlo)
	PORT_INCLUDE(falgasm89)

	PORT_MODIFY("IN0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // the race starts on the coin
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_BUTTON1) PORT_NAME("Accelerator")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_BUTTON2) PORT_NAME("Horn")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_BUTTON3) PORT_NAME("Gear Lever (Low/High)") PORT_TOGGLE // closed is HIGH

	// interpreted by the video board; the extended play score adapts after each race
	PORT_MODIFY("TIMER")
	PORT_DIPNAME(0x03, 0x03, "Race Time") PORT_DIPLOCATION("TIMER:1,2")
	PORT_DIPSETTING(   0x03, "60 seconds")
	PORT_DIPSETTING(   0x02, "90 seconds")
	PORT_DIPSETTING(   0x01, "120 seconds")
	PORT_DIPSETTING(   0x00, "150 seconds")
	PORT_DIPNAME(0x04, 0x00, "Extended Play") PORT_DIPLOCATION("TIMER:3") // +50% time
	PORT_DIPSETTING(   0x04, "1000 to 4000 points (with Race Time)")
	PORT_DIPSETTING(   0x00, "500 points")

	PORT_START("STEERING")
	PORT_BIT(0x07, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT) PORT_2WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT) PORT_2WAY
	PORT_BIT(0xe0, IP_ACTIVE_LOW,  IPT_UNUSED)
INPUT_PORTS_END

INPUT_PORTS_START(fantcar)
	PORT_INCLUDE(falgasm89)

	PORT_MODIFY("IN0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(fantcar_state::pb5_r))
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_BUTTON2) PORT_NAME("Siren")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_BUTTON1) PORT_NAME("Turbo")

	PORT_START("CONTROLS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_BUTTON3) PORT_NAME("Radar")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_BUTTON4) PORT_NAME("Accelerator")
INPUT_PORTS_END

// SELECTOR sets at 10h-1bh in the EPROM; times relative to the longest ride
INPUT_PORTS_START(fantcar87)
	PORT_START("IN0")
	PORT_DIPNAME(0x03, 0x03, "Coinage / Ride Length") PORT_DIPLOCATION("SELECTOR:1,2")
	PORT_DIPSETTING(   0x00, "1 pulse/credit, 3 credits per COIN2, 100% time")
	PORT_DIPSETTING(   0x01, "2 pulses/credit, 3 credits per COIN2, 83% time")
	PORT_DIPSETTING(   0x02, "1 pulse/credit, 2 credits per COIN2, 73% time")
	PORT_DIPSETTING(   0x03, "2 pulses/credit, 2 credits per COIN2, 63% time")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1) // edge triggered: starts the ride
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unused)) PORT_DIPLOCATION("SELECTOR:3")
	PORT_DIPSETTING(   0x20, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unused)) PORT_DIPLOCATION("SELECTOR:4")
	PORT_DIPSETTING(   0x40, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CONTROLS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Siren")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Radar")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Turbo")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Accelerator")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DIAL")
	PORT_ADJUSTER(50, "Timer Knob (60-150 s ride)")
INPUT_PORTS_END

INPUT_PORTS_START(fantcar87a)
	PORT_INCLUDE(fantcar87)

	PORT_MODIFY("IN0")
	PORT_DIPNAME(0x03, 0x03, "Coinage / Ride Length") PORT_DIPLOCATION("SELECTOR:1,2")
	PORT_DIPSETTING(   0x00, "1 pulse/credit, 5 credits per COIN2, 100% time")
	PORT_DIPSETTING(   0x01, "1 pulse/credit, 6 credits per COIN2, 83% time")
	PORT_DIPSETTING(   0x02, "2 pulses/credit, 3 credits per COIN2, 73% time")
	PORT_DIPSETTING(   0x03, "2 pulses/credit, 4 credits per COIN2, 63% time")
INPUT_PORTS_END


// The "simple" PCB has the i8255 socket empty
void falgasm89_state::falgasm89_simple(machine_config &config)
{
	I8085A(config, m_maincpu, 6_MHz_XTAL); // OKI M80C85A-2
	m_maincpu->set_addrmap(AS_PROGRAM, &falgasm89_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falgasm89_state::io_map);
	m_maincpu->out_sod_func().set_output("motor");

	TIMER(config, "timebase").configure_periodic(FUNC(falgasm89_state::timebase), attotime::from_hz(5'000));

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_psg, 6_MHz_XTAL / 4); // divider unknown
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.30);
	m_psg->port_a_write_callback().set(FUNC(falgasm89_state::psg_pa_w));
	m_psg->port_b_read_callback().set_ioport("IN0");

	DAC_4BIT_R2R(config, m_dac).add_route(ALL_OUTPUTS, "mono", 0.30); // resistor network unknown
}

void fantcar_state::fantcar(machine_config &config)
{
	falgasm89_simple(config);

	m_maincpu->set_addrmap(AS_IO, &fantcar_state::fantcar_io_map);

	m_psg->port_a_write_callback().set(FUNC(fantcar_state::fantcar_pa_w));

	AY8910(config, m_psg2, 6_MHz_XTAL / 4); // divider unknown
	m_psg2->add_route(ALL_OUTPUTS, "mono", 0.30);
	m_psg2->port_a_write_callback().set(FUNC(fantcar_state::psg2_pa_w));
}

void falgasm87_state::falgasm87(machine_config &config)
{
	I8085A(config, m_maincpu, 4_MHz_XTAL); // NEC D8085AHC
	m_maincpu->set_addrmap(AS_PROGRAM, &falgasm87_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falgasm87_state::io_map);
	m_maincpu->out_sod_func().set(FUNC(falgasm87_state::sod_w));
	TIMER(config, m_traptimer).configure_periodic(FUNC(falgasm87_state::trap_tick), attotime::from_hz(4800 * 2));
	TIMER(config, m_ridetimer).configure_generic(FUNC(falgasm87_state::ride_tick));

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(500)); // period guessed

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_psg, 4_MHz_XTAL / 2); // CLK OUT, confirmed by recordings of a real machine
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.30);
	m_psg->port_a_write_callback().set(FUNC(falgasm87_state::psg_pa_w));
	m_psg->port_b_read_callback().set_ioport("IN0");

	DAC_4BIT_R2R(config, m_dac).add_route(ALL_OUTPUTS, "mono", 0.30); // resistor network unknown
}

void falgasm89_video_state::falgasm89_video(machine_config &config)
{
	falgasm89_simple(config);

	m_maincpu->set_addrmap(AS_IO, &falgasm89_video_state::main_io_map);

	I8255(config, m_ppi); // NEC D71055C
	m_ppi->out_pa_callback().set(FUNC(falgasm89_video_state::to_video_w));
	m_ppi->in_pb_callback().set([this] () -> u8 { return m_to_main; });

	I8085A(config, m_videocpu, 6_MHz_XTAL); // OKI M80C85A-2
	m_videocpu->set_addrmap(AS_PROGRAM, &falgasm89_video_state::video_mem_map);
	m_videocpu->set_addrmap(AS_IO, &falgasm89_video_state::video_io_map);

	I8155(config, m_i8155, 6_MHz_XTAL / 2); // NEC D8155HC
	m_i8155->in_pa_callback().set([this] () -> u8 { return m_to_video; });
	m_i8155->out_pb_callback().set(FUNC(falgasm89_video_state::to_main_w));
	m_i8155->in_pc_callback().set_ioport("STEERING");
	m_i8155->out_to_callback().set_inputline(m_videocpu, I8085_TRAP_LINE);

	TMS9129(config, m_vdp, 10.738635_MHz_XTAL);
	m_vdp->set_screen("screen");
	m_vdp->set_vram_size(0x4000); // 2 x UD61464DC
	SCREEN(config, "screen");
}

// M89-4 PCB, in a box labelled M89N
ROM_START(cbully)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bully-gs_m89-iv_16-1-91.u2", 0x0000, 0x8000, CRC(4cc85230) SHA1(c3851e6610bcb3427f81ecfcd4575603a9edca6e)) // 27C256

	ROM_REGION(0x22e, "plds", 0)
	ROM_LOAD("palce16v8_m894-bt.u11", 0x000, 0x117, NO_DUMP) // Protected
	ROM_LOAD("palce16v8_m894-a.u10",  0x117, 0x117, NO_DUMP) // Protected
ROM_END

// M89-E5 PCB
ROM_START(fantcar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cochefantastico89.p25", 0x0000, 0x8000, CRC(884a9768) SHA1(6f36a63312ae1f6899d26ca6953f942ddd860742))

	ROM_REGION(0x22e, "plds", 0)
	ROM_LOAD("pal.u11", 0x000, 0x117, NO_DUMP)
	ROM_LOAD("pal.u10", 0x117, 0x117, NO_DUMP)
ROM_END

// Developed by Gaelco, sold by Falgàs
ROM_START(fantcar87)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("mikel_es_m87e_15_2_88.u2",  0x0000, 0x8000, CRC(e1db4836) SHA1(7020bde14ee9afae41691beb0708ed50ca3506d3)) // NM27C256Q
ROM_END

ROM_START(fantcar87a)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("mikel_micro_87_25-3-87.u2", 0x0000, 0x8000, CRC(83a16ff4) SHA1(52a1fcd89882fd00c1f46328d75c2623f6f2f83e))
ROM_END


// M89-E5 PCB and 25291 video PCB; the race game adapts Konami's Hyper Rally (MSX)
ROM_START(rmontecarlo)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("uj_504_m-89e5_17-7-91.u2", 0x00000, 0x10000, CRC(ff1be338) SHA1(9a3f4760bd7e4d9328d44e546bb588561fc53016)) // 27C512

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD("uj_v10_22-5-91.bin",       0x00000, 0x10000, CRC(8ac21706) SHA1(bd399136d4793c1eaa49c2d5a35022864e771833)) // 27C512

	ROM_REGION(0x345, "plds", 0)
	ROM_LOAD("palce16v8_m894-bt.u11", 0x000, 0x117, NO_DUMP) // Protected, on M89 PCB
	ROM_LOAD("palce16v8_m894-a.u10",  0x117, 0x117, NO_DUMP) // Protected, on M89 PCB
	ROM_LOAD("palce16v8_video91.bin", 0x22e, 0x117, NO_DUMP) // Protected, on video PCB
ROM_END

} // anonymous namespace

//    YEAR  NAME         PARENT   MACHINE           INPUT        CLASS                  INIT        ROT   COMPANY     FULLNAME                                    FLAGS                                                                 LAYOUT
GAMEL(1991, cbully,      0,       falgasm89_simple, cbully,      falgasm89_state,       empty_init, ROT0, u8"Falgàs", "Coche Bully",                              MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_falgas_m89 )
GAMEL(19??, fantcar,     0,       fantcar,          fantcar,     fantcar_state,         empty_init, ROT0, u8"Falgàs", "Fantastic Car (M89 hardware)",             MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_falgas_fantcar )
GAMEL(1988, fantcar87,   fantcar, falgasm87,        fantcar87,   falgasm87_state,       empty_init, ROT0, u8"Falgàs", "Fantastic Car (Micro-87 hardware, newer)", MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_falgas_fantcar_m87 )
GAMEL(1987, fantcar87a,  fantcar, falgasm87,        fantcar87a,  falgasm87_state,       empty_init, ROT0, u8"Falgàs", "Fantastic Car (Micro-87 hardware, older)", MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_falgas_fantcar_m87 )
GAMEL(1991, rmontecarlo, 0,       falgasm89_video,  rmontecarlo, falgasm89_video_state, empty_init, ROT0, u8"Falgàs", "Rally Montecarlo / Grand-Prix",            MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_falgas_rmontecarlo )
