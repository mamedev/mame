// license:BSD-3-Clause
// copyright-holders:gregre365
/*************************************************************************

    wildcats.cpp

    ARK TECHNICO "Wild Cats", a Japanese pachislot (slot machine).

    Pachislot is the Japanese form of a medal-operated slot machine.  The
    player starts all three reels with a lever and stops each reel with a
    separate button.  Winning symbol combinations are paid out as medals
    through a hopper.

    Z80 CPU @ 4MHz, YM2413 for sound, three physical stepper-motor reels
    (wildcats_reel_device) published through stepper_device::draw().  All I/O
    is memory-mapped in the 0x4000-0x400b window; see wildcats_map().

    The program ROM was dumped from an original PCB marked "P3AA-001-I" on
    the silkscreen.

    The firmware behaviour and I/O observations below were derived from
    analysis of the Wild Cats ROM and tests on Wild Cats hardware.

    The Wild Cats output bit assignments were determined by comparing emulated
    behaviour with the real machine and tracing the PCB tracks and connections
    to the cabinet.  Anything not confirmed on real hardware is marked as such.

    In Wild Cats, the RAM at 0x2000 is sustained across a power cut by a
    supercapacitor.  When the main power-off signal is asserted, the firmware
    saves the interrupted CPU context there, writes 0x20 to 0x4008 to signal
    backup completion, and halts.  On reset it restores a valid saved context
    and resumes the interrupted game.  This preserves credits, pending winning
    flags and bonus state across an end-of-day shutdown or power failure.  The
    Main Power Off input allows the sequence to be triggered manually.  MAME
    does not assert it automatically on exit; unless it is triggered first,
    the NVRAM file preserves the RAM bytes but contains no resumable CPU
    context, so the next reset takes the cold-start path and clears that game
    state.  MAME's -autosave option can instead preserve the complete emulated
    state across an ordinary exit, but it bypasses the firmware's own
    power-fail save and restore sequence.

    If power fails while the setting-change screen is active, the firmware
    skips the context save but still writes 0x20 and halts.  On reset it
    validates only the 0x55aa signature at 0x200b-0x200c before restoring a
    saved context.  Restoration overwrites the signature, so each saved
    context can be resumed only once.

    If the SRAM is all zero, the firmware's RAM-loss stop displays "nE" and
    stores default setting 3; resetting or restarting then boots normally.
    This matches real hardware after the supercapacitor has discharged or the
    RAM has been cleared.

    The default NVRAM is not factory content or a hardware dump.  It is a
    convenience image written by the Wild Cats firmware itself running in
    MAME: after the "nE" stop, the machine was restarted, left idle at
    setting 3 and shut down with Main Power Off, so a first run resumes
    that saved context.

    Not every bit of these ports is a lamp. Some are solenoids, a motor,
    sensors, or terminals wired to the hall's management computer, and
    they are only exposed as outputs so they can be watched while
    debugging; those are named for what they really are rather than
    "lampNN", so a layout author can tell at a glance which outputs
    correspond to something the player can actually see.

    Register map, 0x4000-0x400b (see wildcats_map()):

    4000  R  IN1 port (bit 7: main power-off signal)
          W  reel0_w()      - reel 0 (left) stepper drive data
    4001  R  IN2 port, with the hopper payout sensor synthesised onto
                             bit 0 via hopper_sensor_r() (PORT_CUSTOM_MEMBER)
                             while the hopper runs; a debug-stretched copy
                             (DEBUG_LED_STRETCH, not the real timing) is
                             also published as the hopper_sensor output
          W  reel1_w()      - reel 1 (centre) stepper drive data
    4002  R  unknown input; Wild Cats reads it at startup and every interrupt,
              then discards the value. Bits 0 and 1 change on Wild Cats
              hardware, but their functions have not been identified
          W  reel2_w()      - reel 2 (right) stepper drive data
    4003  W  panel_lamps_w() - panel lamps - table below
    4004  W  strobe_w()      - digit select; writing one of 1/2/4/8 latches
                               the byte last written to 4005 into that digit.
                               The Wild Cats firmware writes 0 first to blank
                               the display between digits; the driver ignores
                               this because each digit latches independently
    4005  W  digit_data_w()  - segment data, written before the 4004 strobe
                               that latches it
    4006  W  payline_lamps_w() - payline lamps, then the medal IN/OUT
                                  terminals - table below
    4007  W  output_ctrl_w() - more panel lamps, the hopper motor, the
              coin lockout solenoid, and the BIG/JAC terminals - table below
    4008  W  status_w()      - status/error-code latch. Wild Cats writes 0x00
                               every frame; 0x20 signals backup completion
                               during power failure. Other possible error codes
                               have not been observed
    4009  W  unknown_4009_w() - unknown, not yet reverse engineered.
              Only ever written once, right after reset, and never again
              afterwards - consistent with it being NC, but that is not
              confirmed either. Logged for debugging
    400a  W  ymsnd address_w() - YM2413 register select
    400b  W  ymsnd data_w()    - YM2413 register data

    panel_lamps_w() - panel lamps:

    7654 3210
    ---- ---x  * STOP lamp, left reel                (stop_lamp0)
    ---- --x-  * STOP lamp, centre reel              (stop_lamp1)
    ---- -x--  * STOP lamp, right reel               (stop_lamp2)
    ---- x---  * CREDIT display-mode lamp            (credit_lamp)
    ---x ----  * top-panel cat lamp                  (cat_lamp)
    --x- ----  * top-panel red 7 lamp                (seven_lamp0)
    -x-- ----  * top-panel yellow 7 lamp             (seven_lamp1)
    x--- ----  * top-panel blue 7 lamp               (seven_lamp2)

    payline_lamps_w() - payline lamps, then the medal count terminals:

    7654 3210
    ---- ---x  * payline lamp, centre row            (payline_lamp0)
    ---- --x-  * payline lamp, top or bottom row     (payline_lamp1)
    ---- -x--  * payline lamp, the other of those    (payline_lamp2)
    ---- x---  * payline lamp, one diagonal          (payline_lamp3)
    ---x ----  * payline lamp, the other diagonal    (payline_lamp4)
    --x- ----  * INSERT MEDAL blink                  (insert_medal_lamp)
                 Confirmed on real hardware; 0x4007 bit 4 controls the
                 coin-blocking solenoid, not this lamp.
    -x-- ----  * medal OUT count terminal            (medal_out_terminal)
    x--- ----  * medal IN count terminal             (medal_in_terminal)

    The five payline lamps light cumulatively, lowest bit first: one medal lights
    bit 0 alone, two medals bits 0-2, three medals all five - which lines
    up with an A-type machine enabling the centre row, then the top and
    bottom rows, then the two diagonals. Checked by feeding coins in one
    at a time and watching the outputs. Which of bit 1 / bit 2 is the top
    row (and likewise which diagonal is bit 3) cannot be told that way,
    since each pair always lights together.

    The last two are not lamps either: they are the terminals the hall's
    management computer counts, one pulse per medal paid out and one per
    medal taken in (a bet counts as taken in). They also drive MAME's own
    coin counters.

    A medal is counted in when the game starts, not when the coin drops:
    playing a game through under script shows the IN terminal pulsing for
    about 50 ms just after the reels begin to turn.

    output_ctrl_w() - two lamps, the hopper, the coin lockout, and
    the two terminals that report to the hall computer:

    7654 3210
    ---- ---x  * ?                                   (unknown_4007_bit0)
    ---- --x-  * ?                                   (unknown_4007_bit1)
    ---- -x--  * TIME UP lamp                        (time_up_lamp)
                 Lights while the machine enforces a 4.1025-second interval
                 between game starts.
    ---- x---  * hopper motor drive                  (hopper_motor)
    ---x ----  * coin-blocking solenoid              (coin_block_solenoid)
                 Fed into coin_lockout_w(0, !bit) below with inverted
                 polarity.
    --x- ----  * top lamp strip                      (top_lamp)
    -x-- ----  * JAC game in progress                (jac_terminal)
    x--- ----  * BIG bonus in progress               (big_bonus_terminal)

    bit 7 and bit 6 are not lamps either. They are the terminals the
    machine uses to tell the hall's central management computer
    (集中制御基板 / ホルコン) that a BIG bonus or a JAC game is running,
    so nothing on the cabinet lights up when they assert.

    Reel index sensors arrive through opto_cb() rather than a port, and
    are published as reel_opto0..2 - again sensors, not lamps.

**************************************************************************/

#include "emu.h"

#include "wildcats_reel.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ymopl.h"

#include "speaker.h"

#include "wildcats.lh"

#define LOG_UNKNOWN (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class wildcats_state : public driver_device
{
public:
	wildcats_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_reel(*this, "reel%u", 0U)
		, m_payline_lamps(*this, "payline_lamp%u", 0U)
		, m_stop_lamps(*this, "stop_lamp%u", 0U)
		, m_seven_lamps(*this, "seven_lamp%u", 0U)
		, m_insert_medal_lamp(*this, "insert_medal_lamp")
		, m_credit_lamp(*this, "credit_lamp")
		, m_cat_lamp(*this, "cat_lamp")
		, m_top_lamp(*this, "top_lamp")
		, m_time_up_lamp(*this, "time_up_lamp")
		, m_reel_opto(*this, "reel_opto%u", 0U)
		, m_coin_block_solenoid(*this, "coin_block_solenoid")
		, m_hopper_motor(*this, "hopper_motor")
		, m_medal_in_terminal(*this, "medal_in_terminal")
		, m_medal_out_terminal(*this, "medal_out_terminal")
		, m_big_bonus_terminal(*this, "big_bonus_terminal")
		, m_jac_terminal(*this, "jac_terminal")
		, m_unknown_4007(*this, "unknown_4007_bit%u", 0U)
		, m_hopper_sensor_output(*this, "hopper_sensor")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void wildcats(machine_config &config) ATTR_COLD;

	ioport_value opto_r0() { return m_opto[0]; }
	ioport_value opto_r1() { return m_opto[1]; }
	ioport_value opto_r2() { return m_opto[2]; }
	ioport_value hopper_sensor_r() { return m_hopper_active && m_hopper_sensor; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	template <unsigned N> void opto_cb(int state);

	bool m_opto[3]{};

private:
	uint8_t m_digit_data = 0;

	bool m_hopper_active = false;
	emu_timer *m_hopper_timer = nullptr;
	emu_timer *m_hopper_pulse_timer = nullptr;
	bool m_hopper_sensor = false;

	// Stretch reel_opto0..2 and hopper_sensor on the debug overlay so a
	// human eye can catch them; see opto_cb() and hopper_timer_callback().
	// Below the hopper's own 100ms pulse period so back-to-back pulses
	// still show a visible gap instead of looking like one solid light
	static constexpr attotime DEBUG_LED_STRETCH = attotime::from_msec(60);
	emu_timer *m_opto_stretch_timer[3]{};
	emu_timer *m_hopper_sensor_stretch_timer = nullptr;

	void reel0_w(uint8_t data);
	void reel1_w(uint8_t data);
	void reel2_w(uint8_t data);
	void panel_lamps_w(uint8_t data);
	void strobe_w(uint8_t data);
	void digit_data_w(uint8_t data);
	void payline_lamps_w(uint8_t data);
	void output_ctrl_w(uint8_t data);
	void status_w(uint8_t data);
	void unknown_4009_w(uint8_t data);
	void reset_irq_timer();

	emu_timer *m_irq_timer = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device_array<wildcats_reel_device, 3> m_reel;
	// panel lamps - the ones that actually light up behind the glass
	output_finder<5> m_payline_lamps;   // payline_lamp0..4 (see header comment for the bit assignments)
	output_finder<3> m_stop_lamps;      // stop_lamp0..2 (left/centre/right)
	output_finder<3> m_seven_lamps;     // seven_lamp0..2 (red/yellow/blue)
	output_finder<> m_insert_medal_lamp;
	output_finder<> m_credit_lamp;
	output_finder<> m_cat_lamp;
	output_finder<> m_top_lamp;
	output_finder<> m_time_up_lamp;

	// not lamps: solenoid, motor, sensor and hall-computer terminals exposed for debugging
	output_finder<3> m_reel_opto;       // reel_opto0..2
	output_finder<> m_coin_block_solenoid;
	output_finder<> m_hopper_motor;
	output_finder<> m_medal_in_terminal;    // external terminal to the hall's management computer
	output_finder<> m_medal_out_terminal;
	output_finder<> m_big_bonus_terminal;
	output_finder<> m_jac_terminal;
	output_finder<2> m_unknown_4007;    // unknown_4007_bit0..1

	// internal state exposed only so it can be watched while debugging
	output_finder<> m_hopper_sensor_output; // hopper_sensor: raw payout-sensor pulse train

	output_finder<4> m_digits;

	TIMER_CALLBACK_MEMBER(irq_timer_callback);
	TIMER_CALLBACK_MEMBER(hopper_timer_callback);
	TIMER_CALLBACK_MEMBER(clear_hopper_pulse);
	TIMER_CALLBACK_MEMBER(opto_stretch_off);
	TIMER_CALLBACK_MEMBER(hopper_sensor_stretch_off);
	void wildcats_map(address_map &map) ATTR_COLD;
};

template <unsigned N>
void wildcats_state::opto_cb(int state)
{
	m_opto[N] = state;

	// Reel index sensor; not a lamp, exposed for debugging only.  The real
	// pulse is only a few hundred microseconds wide (too short to see), so
	// stretch it for 60 ms on the debug layout.
	if (state)
	{
		m_reel_opto[N] = 1;
		m_opto_stretch_timer[N]->adjust(DEBUG_LED_STRETCH, N);
	}
}

void wildcats_state::reel0_w(uint8_t data)
{
	m_reel[0]->update(data);
	m_reel[0]->draw();
}

void wildcats_state::reel1_w(uint8_t data)
{
	m_reel[1]->update(data);
	m_reel[1]->draw();
}

void wildcats_state::reel2_w(uint8_t data)
{
	m_reel[2]->update(data);
	m_reel[2]->draw();
}

void wildcats_state::panel_lamps_w(uint8_t data)
{
	m_stop_lamps[0]  = BIT(data, 0);    // STOP lamp, left
	m_stop_lamps[1]  = BIT(data, 1);    // STOP lamp, centre
	m_stop_lamps[2]  = BIT(data, 2);    // STOP lamp, right
	m_credit_lamp    = BIT(data, 3);    // credit-mode lamp
	m_cat_lamp       = BIT(data, 4);    // cat
	m_seven_lamps[0] = BIT(data, 5);    // red 7
	m_seven_lamps[1] = BIT(data, 6);    // yellow 7
	m_seven_lamps[2] = BIT(data, 7);    // blue 7
}

void wildcats_state::strobe_w(uint8_t data)
{
	switch (data)
	{
	case 1:
		m_digits[1] = m_digit_data;
		break;
	case 2:
		m_digits[0] = m_digit_data;
		break;
	case 4:
		m_digits[3] = m_digit_data;
		break;
	case 8:
		m_digits[2] = m_digit_data;
		break;
	}
}

void wildcats_state::digit_data_w(uint8_t data)
{
	m_digit_data = data;
}

void wildcats_state::payline_lamps_w(uint8_t data)
{
	for (int i = 0; i < 5; i++)
		m_payline_lamps[i] = BIT(data, i);  // payline plate

	m_insert_medal_lamp = BIT(data, 5); // INSERT MEDAL blink (see header comment)

	// bit7 / bit6 are not lamps; they are the medal IN/OUT terminals
	// the hall's management computer counts. Fed straight into
	// MAME's own coin counters too
	m_medal_in_terminal  = BIT(data, 7);    // counts one medal taken in (a bet counts as taken in)
	m_medal_out_terminal = BIT(data, 6);    // counts one medal paid out
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));
}

void wildcats_state::output_ctrl_w(uint8_t data)
{
	// See the header comment's table for the bit assignments.
	m_top_lamp     = BIT(data, 5);  // top lamp strip
	m_time_up_lamp = BIT(data, 2);  // TIME UP lamp

	// Everything below is not a panel lamp. These are the external
	// terminals to the hall's management computer, the solenoid, and
	// the motor, exposed only so they can be watched while debugging
	m_big_bonus_terminal  = BIT(data, 7);   // BIG bonus in progress terminal
	m_jac_terminal        = BIT(data, 6);   // JAC game in progress terminal
	m_coin_block_solenoid = !BIT(data, 4);  // lit while coin insertion is actually blocked
	m_hopper_motor        = BIT(data, 3);   // hopper motor drive
	m_unknown_4007[1]     = BIT(data, 1);   // ?
	m_unknown_4007[0]     = BIT(data, 0);   // ?

	if (!m_hopper_active)
	{
		if (data & 0x08)
		{
			m_hopper_active = true;
			m_hopper_timer->adjust(attotime::zero, 0, attotime::from_msec(100));
		}
	}
	else if ((data & 0x08) == 0)
	{
		m_hopper_active = false;
		m_hopper_timer->adjust(attotime::never);
	}

	// coin lockout (m_coin_block_solenoid mirrors the same signal, inverted)
	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 4));
}

void wildcats_state::status_w(uint8_t data)
{
	// Written with 0 every frame in normal operation. 0x20 tells the
	// hardware that backup is complete and power can be removed.
	// Other values may be error codes, but none have been observed.
	if (data & ~0x20)
		logerror("status_w: unhandled status/error code: %02x\n", data);
}

void wildcats_state::unknown_4009_w(uint8_t data)
{
	LOGMASKED(LOG_UNKNOWN, "%s: unknown_4009_w: %02x\n", machine().describe_context(), data);
}

void wildcats_state::reset_irq_timer()
{
	// 7500 cycles at 4 MHz is 1.875 ms.  The firmware counts 2188 interrupts
	// per game interval (4.1025 seconds), one interrupt more than the minimum
	// integer count needed to exceed 4.1 seconds.
	m_irq_timer->adjust(m_maincpu->cycles_to_attotime(7500));
}

TIMER_CALLBACK_MEMBER(wildcats_state::irq_timer_callback)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
	reset_irq_timer();
}

TIMER_CALLBACK_MEMBER(wildcats_state::hopper_timer_callback)
{
	if (m_hopper_active)
	{
		// simulate a medal passing the hopper sensor
		m_hopper_sensor = true;
		// debug-overlay copy: the real pulse above is only 10ms (too short
		// to see), so stretch it; hopper_sensor_stretch_off turns it back
		// off 60ms later regardless of m_hopper_sensor's state
		m_hopper_sensor_output = 1;
		m_hopper_sensor_stretch_timer->adjust(DEBUG_LED_STRETCH);
		m_hopper_pulse_timer->adjust(attotime::from_msec(10));
	}
}

TIMER_CALLBACK_MEMBER(wildcats_state::clear_hopper_pulse)
{
	m_hopper_sensor = false;
}

TIMER_CALLBACK_MEMBER(wildcats_state::opto_stretch_off)
{
	m_reel_opto[param] = 0;
}

TIMER_CALLBACK_MEMBER(wildcats_state::hopper_sensor_stretch_off)
{
	m_hopper_sensor_output = 0;
}

void wildcats_state::machine_start()
{
	m_hopper_pulse_timer = timer_alloc(FUNC(wildcats_state::clear_hopper_pulse), this);
	m_irq_timer = timer_alloc(FUNC(wildcats_state::irq_timer_callback), this);
	m_hopper_timer = timer_alloc(FUNC(wildcats_state::hopper_timer_callback), this);
	for (int i = 0; i < 3; i++)
	{
		m_opto_stretch_timer[i] = timer_alloc(FUNC(wildcats_state::opto_stretch_off), this);
	}
	m_hopper_sensor_stretch_timer = timer_alloc(FUNC(wildcats_state::hopper_sensor_stretch_off), this);

	// m_reel is a pointer to a device (required_device_array), not state
	// itself. Reel state is already registered with save_item by the
	// stepper_device in its own device_start().
	save_item(NAME(m_opto));
	save_item(NAME(m_digit_data));
	save_item(NAME(m_hopper_active));
	save_item(NAME(m_hopper_sensor));
}

void wildcats_state::machine_reset()
{
	reset_irq_timer();

	// stop an in-flight hopper payout so a soft reset doesn't leave IN2's
	// hopper_sensor_r() still pulsing from the previous session
	m_hopper_active = false;
	m_hopper_sensor = false;
	m_hopper_pulse_timer->adjust(attotime::never);
	m_hopper_timer->adjust(attotime::never);
	m_hopper_sensor_stretch_timer->adjust(attotime::never);
	m_hopper_sensor_output = 0;
	m_hopper_motor = 0; // raw motor-drive mirror from output_ctrl_w; keep it in sync with m_hopper_active above
}

void wildcats_state::wildcats_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	// uPD449 (2Kbyte SRAM). A11/A12 are not wired to the chip, so its
	// 2Kbyte mirrors 4 times across this 8Kbyte window.
	map(0x2000, 0x27ff).mirror(0x1800).ram().share("nvram");
	map(0x4000, 0x4000).portr("IN1").w(FUNC(wildcats_state::reel0_w));
	map(0x4001, 0x4001).portr("IN2").w(FUNC(wildcats_state::reel1_w));
	map(0x4002, 0x4002).nopr().w(FUNC(wildcats_state::reel2_w)); // unknown input; bits 0 and 1 change on real hardware
	map(0x4003, 0x4003).w(FUNC(wildcats_state::panel_lamps_w));
	map(0x4004, 0x4004).w(FUNC(wildcats_state::strobe_w));
	map(0x4005, 0x4005).w(FUNC(wildcats_state::digit_data_w));
	map(0x4006, 0x4006).w(FUNC(wildcats_state::payline_lamps_w));
	map(0x4007, 0x4007).w(FUNC(wildcats_state::output_ctrl_w));
	map(0x4008, 0x4008).w(FUNC(wildcats_state::status_w));
	map(0x4009, 0x4009).w(FUNC(wildcats_state::unknown_4009_w));
	map(0x400a, 0x400a).w("ymsnd", FUNC(ym2413_device::address_w));
	map(0x400b, 0x400b).w("ymsnd", FUNC(ym2413_device::data_w));
}

void wildcats_state::wildcats(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &wildcats_state::wildcats_map);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	// The 8MHz crystal is divided by two for both the Z80 and YM2413.
	// The YM2413 therefore runs at 4MHz,
	// not the usual 3.579545MHz TV colour-subcarrier-derived clock seen
	// on many YM2413 boards
	YM2413(config, "ymsnd", 8_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "mono", 1.0);

	SPEAKER(config, "mono").front_center();

	WILDCATS_REEL(config, m_reel[0], 340, 344, 0x00, 5, 400);
	m_reel[0]->optic_handler().set(FUNC(wildcats_state::opto_cb<0>));
	WILDCATS_REEL(config, m_reel[1], 340, 344, 0x00, 5, 400);
	m_reel[1]->optic_handler().set(FUNC(wildcats_state::opto_cb<1>));
	WILDCATS_REEL(config, m_reel[2], 340, 344, 0x00, 5, 400);
	m_reel[2]->optic_handler().set(FUNC(wildcats_state::opto_cb<2>));

	config.set_default_layout(layout_wildcats);
}

static INPUT_PORTS_START( wildcats )
	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_NAME("Main Power Off") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wildcats_state::opto_r2))  // right reel start position sensor
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wildcats_state::opto_r1))  // centre reel start position sensor
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wildcats_state::opto_r0))  // left reel start position sensor
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_NAME("Right Reel Stop") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_NAME("Centre Reel Stop") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_NAME("Left Reel Stop") PORT_CODE(KEYCODE_A)
	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start Lever")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Reset Key")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("Bet")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Setting Change Switch")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wildcats_state::hopper_sensor_r))  // hopper payout sensor
INPUT_PORTS_END

ROM_START( wildcats )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wildcats-ndk.bin", 0x00000, 0x2000, CRC(362b3e92) SHA1(40aa96dded5a55865892868fd09cf5af4c909c85) )

	ROM_REGION( 0x0800, "nvram", 0 ) // uPD449C-1 at IC9, default contents generated in MAME (see notes above)
	ROM_LOAD( "upd449c-1.ic9", 0x0000, 0x0800, CRC(fe35d05d) SHA1(4e3f065a3c014ef8f46d224081b4c46d93e0a8db) )
ROM_END

} // anonymous namespace

GAME( 1991, wildcats, 0, wildcats, wildcats, wildcats_state, empty_init, ROT0, "Ark Technico", "Wild Cats (Ark Technico Pachislot)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
