// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Yamasa Pachi-Slot Series Driver
    LE1080A(V1) + 3-reel + Z80 + YM2413 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2025-09-11.

    Supported Games:
    - New Pulsar    (c) 1993 Yamasa  (4th Generation, Type A)
    - Divers XX     (c) 1994 Yamasa  (4th Generation, Type A)
    - Pink Panther  (c) 1995 Yamasa  (4th Generation, Type A)

    Hardware:
    - Main CPU: LE1080A-PA @ provisional 4.0 MHz (8.0 MHz XTAL / 2)
    - Sound CPU: Z80 @ provisional 4.0 MHz (8.0 MHz XTAL / 2)
    - Sound: YM2413 @ 3.579545 MHz
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: Internal LE1080A battery-backed RAM

    TODO:
    - LE1080A interrupt addresses and internal random-number behavior are
      still not fully understood, so some model-specific behavior remains
      implemented in the driver rather than in le1080a.cpp.
    - IRQ0 / IRQ1 / power-fail preparation interrupt addresses differ by
      machine and manufacturer.
      These are currently assigned from analysis results in each program
      rather than being derived automatically by the LE1080A core.
    - Decide whether the replay symbol for New Pulsar / Divers XX
      should be labeled "JP" (JacPot).
      Did the "JP" replay-symbol label disappear starting with
      Pink Panther?

    NOTE:
    [LE-Tech LE1080A CPU & Interrupt Architecture]
    - The LE-Tech LE1080A is a custom Z80-compatible microcontroller integrating
      internal battery-backed SRAM, security features, hardware pseudo-random
      generators, and custom programmable interrupt timers. For complete details
      on system registers and internal peripheral maps, refer to 'cpu/z80/le1080a.cpp'.
    - On Yamasa hardware, sound latch writes (0x18) and cabinet switch reads (0x19)
      are mapped directly to LE1080A internal system registers via install_sysreg_handler.
    - Because the exact internal timer reload and register programming sequences
      are not yet fully reverse-engineered from firmware code, the main periodic
      interrupt rate (LE1080A IRQ1 = 780 Hz at 0x0084) is provisionally calibrated
      from physical reel revolution pacing.
    - Sound CPU (Z80) periodic NMI pacing is driven at an estimated 2500 Hz.

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, triggering the LE1080A PFI interrupt (IRQ2)
      allowing the CPU to execute its state-saving routine into battery-backed
      internal SRAM during the reservoir capacitance hold-up time.
    - On New Pulsar hardware, the CPU naturally dies after saving state due to
      voltage decay. Because MAME lacks an automatic power-down framework,
      init_newpulsar patches the end of the PFI sequence (ROM 0x079e) to HALT (0x76)
      to prevent execution loops from corrupting the NVRAM state.
    - Because MAME currently lacks a pre-reset / pre-shutdown callback
      mechanism to allow the CPU to gracefully complete its power-down
      sequence before machine_reset() or exit (e.g. F3 / ESC keys), this
      driver provisionally provides a manual 'P' key (powerfail_trigger).
    - Pressing the 'P' key asserts the LE1080A PFI interrupt (IRQ2 at vector 0x0066),
      sets m_powerfail_status_ready, and preserves physical reel absolute positions
      via stepper_device helpers.
    - For seamless out-of-the-box power restoration in the future, MAME core
      framework support for graceful shutdown execution and non-volatile
      stepper position persistence will be required.

    [Pachi-Slot Medal Simulator Architecture (pachislotdev)]
    - Real V0 machines feature simpler mechanical coin-in selectors and single-sensor
      hoppers without the sophisticated multi-sensor anti-cheat timing checks introduced
      in later generations (V1 and later).
    - While direct input port mapping would technically suffice for single-sensor V0 hardware,
      all drivers consistently route coin-in and coin-out signals through 'pachislot_device'.
    - This unified architecture provides several key advantages across all generations:
      1. Realistic, hardware-accurate hopper ejection intervals and sensor pulse widths
         without relying on the deprecated 'ticket.cpp' hopper implementation.
      2. Direct callback dispatching for synchronized mechanical payout sound playback
         via the MAME 'samples' device.
      3. A fully standardized driver interface, ensuring architectural consistency
         from early V0 titles through multi-sensor platforms in V1 and later generations.

    [NVRAM Cold-Start & Error E4 Initialization]
    - On Yamasa LE1080A hardware, starting with missing, uninitialized, or improperly
      backed-up NVRAM contents will consistently trigger a RAM error (E4 on 7-segment LEDs).
    - In this condition, the machine locks normal gameplay until an official setting
      reinitialization sequence is performed:
      1. Turn Setting Key (F2) on and reset machine (F3 or restart).
      2. Change setting value (F1) to any desired level (1-6).
      3. Operate start lever to confirm setting value.
      4. Turn Setting Key (F2) off to enter normal game state.
    - Subsequent boots with preserved NVRAM will resume normally without error E4.

    [Cabinet Knocker Solenoid Simulation]
    - Genuine Yamasa cabinets from this era incorporate an electromechanical knocker
      solenoid inside the housing that generates a loud physical "knock" sound upon reel
      stop and payout events.
      The driver drives layout output "signal_knocker" and triggers mechanical knocker
      sound playback via the MAME samples device.

    [Bookkeeping / Mechanical Counters]
    - Total IN (bit 7) and Total OUT (bit 6) pulse outputs driven by port 0xeffb
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

    [Setting Change Procedure]
    - Standard 4th generation sequence:
      1. Turn Setting Key (F2) on and restart/reset machine.
      2. Change setting value (F1) to any value (1-6).
      3. Operate start lever to confirm setting.
      4. Turn Setting Key off to enter normal game state.

    [Game Rules & Options]
    - Configurable SEISAN (automatic payout) and UCHIDOME (automatic game-stop)
      behaviors are supported via cabinet option switches ("SEISAN": Off / On,
      "UCHIDOME": Off / On). Note that option switch changes require cycling through
      setting mode to reinitialize state.

***************************************************************************/

#include "emu.h"

#include "pachislotdev.h"

#include "cpu/z80/le1080a.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/steppers.h"
#include "sound/ymopl.h"
#include "sound/samples.h"
#include "speaker.h"


namespace {

// internal artwork
#include "newpulsar.lh"
#include "diversxx.lh"
#include "pinkpanther.lh"

class newpulsar_state : public driver_device
{
public:
	newpulsar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "le1080a")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_ym2413(*this, "ym2413")
		, m_pachislot(*this, "pachislot")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_switch(*this, "SWITCH")
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_led_frame(*this, "led_frame%u", 0U)
		, m_lay_led_toppanel(*this, "led_toppanel")
		, m_lay_led_betline(*this, "led_betline%u", 0U)
		, m_lay_led_stop(*this, "led_stop%u", 0U)
		, m_lay_led_payout(*this, "led_payout")
		, m_lay_led_insert(*this, "led_insert")
		, m_lay_led_maxbet(*this, "led_maxbet")
		, m_lay_led_start(*this, "led_start")
		, m_lay_led_counter(*this, "led_counter%u", 0U)
		, m_lay_led_jac(*this, "led_jac%u", 0U)
		, m_lay_led_status(*this, "led_status%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_knocker(*this, "signal_knocker")
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void newpulsar(machine_config &config) ATTR_COLD;

	void init_common();
	void init_newpulsar();

	template <pachislot_device::DEVSENSOR_NUM N> int selector_r() { return m_pachislot->selector_r(N); }
	template <pachislot_device::DEVSENSOR_NUM N> int hopper_r() { return m_pachislot->hopper_r(N); }
	template <unsigned N> int in_fakebtn_r() { return BIT(m_in_fakebtn->read(), N); }
	template <unsigned N> int opto_r() { return m_opto[N]; }

	DECLARE_INPUT_CHANGED_MEMBER(powerfail_trigger);

	// Mechanical reel state snapshot preserved across power-fail simulation
	struct powerfail_snapshot
	{
		bool valid = false;          // True if a valid snapshot was saved prior to reset
		int reel_abs_pos[3]{};       // Preserved absolute step positions for reels 1-3
	};

	void powerfail_prepare_state();
	void powerfail_restore_state();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	template <unsigned N> void opto_cb(int state) { m_opto[N] = state; }

	void le1080a_mem_map(address_map &map) ATTR_COLD;
	void newpulsar_mem_map(address_map &map) ATTR_COLD;
	void audio_mem_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;

private:
	required_device<le1080a_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<ym2413_device> m_ym2413;
	required_device<pachislot_device> m_pachislot;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<2> m_in_port;
	required_ioport m_in_switch;
	required_ioport m_in_fakebtn;
	output_finder<5> m_lay_led_frame;
	output_finder<> m_lay_led_toppanel;
	output_finder<5> m_lay_led_betline;
	output_finder<3> m_lay_led_stop;
	output_finder<> m_lay_led_payout;
	output_finder<> m_lay_led_insert;
	output_finder<> m_lay_led_maxbet;
	output_finder<> m_lay_led_start;
	output_finder<12> m_lay_led_counter;
	output_finder<3> m_lay_led_jac;
	output_finder<2> m_lay_led_status;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_knocker;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 le1080a_r(offs_t offset);
	void le1080a_w(offs_t offset, u8 data);

	template <unsigned N> u8 rng_r(offs_t offset);
	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	void drive_latch_a_w(offs_t offset, u8 data);
	void drive_latch_b_w(offs_t offset, u8 data);
	void drive_latch_c_w(offs_t offset, u8 data);
	void signal_and_comm_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void led_data_w(offs_t offset, u8 data);

	INTERRUPT_GEN_MEMBER(sound_nmi);
	void soundcode_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);
	void playing_knocker_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	u8 m_opto[3];
	u8 m_segdata[4];
	u8 m_segdata_pending;
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;

	u8 m_led_comm_latch[4];
	u8 m_led_data_pending;
};


void newpulsar_state::init_common()
{
	// LE1080A Interrupt vector setup
	m_maincpu->set_irq1_address(0x0084);
	m_maincpu->set_irq2_address(0x0066);

	// Interrupt frequencies are provisionally calibrated based on observed gameplay
	// behavior such as physical reel revolution rate and overall game pacing.
	m_maincpu->set_irq1_rate_hz(780);
}

void newpulsar_state::init_newpulsar()
{
	u8 *ROM = memregion("le1080a")->base();

	// NOTE:
	// On genuine hardware, after the PFI interrupt routine finishes serializing
	// critical CPU registers into battery-backed SRAM, the physical CPU simply
	// dies due to power supply decay; therefore, the original ROM does not end
	// with a HALT instruction.
	// In MAME, because the CPU continues executing normal main/interrupt cycles
	// after the simulated PFI routine without an actual power loss, subsequent
	// loop iterations overwrite the freshly saved state in NVRAM with corrupted data.
	// To prevent this NVRAM corruption until MAME gains proper framework-level
	// power-off handling, we patch the exit point of the backup sequence to HALT (0x76).
	ROM[0x079e] = 0x76;		// -> HALT

	init_common();
}

void newpulsar_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_segdata_pending));
	save_item(NAME(m_led_comm_latch));
	save_item(NAME(m_led_data_pending));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void newpulsar_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segdata_pending = 0;
	std::fill(std::begin(m_led_comm_latch), std::end(m_led_comm_latch), 0);
	m_led_data_pending = 0;

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;

	powerfail_restore_state();
	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;

	m_pachislot->reset_selector();
	m_pachislot->reset_hopper();
}

// ---------------------------------------------------------------------------
//  Power-fail (PFI) Simulation Helpers
// ---------------------------------------------------------------------------
// NOTE for MAME core maintainers / reviewers:
// Real pachislot hardware relies on a dedicated analog power-fail supervisor (PFI)
// circuit to trigger an NMI/INT, allowing the CPU to serialize machine states and
// absolute reel step positions into battery-backed SRAM before power drops.
//
// Because the MAME core framework currently lacks:
//   1. A pre-shutdown / pre-reset driver callback to allow the CPU to gracefully
//      complete state-saving routines before exit or machine_reset() (e.g. ESC/F3).
//   2. Built-in non-volatile persistence for physical stepper_device positions.
//
// These provisional helper functions and the manual 'P' key are implemented to
// preserve physical reel positions across reset/power cycles until graceful
// shutdown support is introduced into the core framework.
// ---------------------------------------------------------------------------
void newpulsar_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void newpulsar_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void newpulsar_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

INTERRUPT_GEN_MEMBER(newpulsar_state::sound_nmi)
{
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

// Provisional trigger handler simulating the physical power failure supervisor circuit.
// Triggers the LE1080A PFI interrupt (IRQ2 at vector 0x0066).
INPUT_CHANGED_MEMBER(newpulsar_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
		m_maincpu->trigger_power_fail();
	}
}

template <unsigned N>
u8 newpulsar_state::rng_r(offs_t offset)
{
	// bit0-7: Read once after lever ON and before reel rotation. Possibly LE1080A internal random source.
	return m_maincpu->internal_rng_r(N);
}

u8 newpulsar_state::switch_sense_a_r(offs_t offset)
{
	// bit0: setting key
	// bit1: change setting value button
	// bit2: start lever
	// bit3: hopper medal out sensor
	// bit4: payout / credit game button
	// bit5: medal in sensor 1
	// bit6: medal in sensor 2
	// bit7: door key / reset button

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[0]->read();
}

u8 newpulsar_state::switch_sense_b_r(offs_t offset)
{
	// bit0: reel 1 stop button (left)
	// bit1: reel 2 stop button (center)
	// bit2: reel 3 stop button (right)
	// bit3: reel 1 sensor (left)
	// bit4: reel 2 sensor (center)
	// bit5: reel 3 sensor (right)
	// bit6: bet 1 button
	// bit7: maxbet button

	return m_in_port[1]->read();
}

void newpulsar_state::drive_latch_a_w(offs_t offset, u8 data)
{
	// bit0-3: reel 1 pulse bit (center)
	// bit4-7: reel 2 pulse bit (left)

	m_reel[1]->write((data & 0xf0) >> 4);
	m_reel[2]->write(data & 0x0f);
}

void newpulsar_state::drive_latch_b_w(offs_t offset, u8 data)
{
	// bit0-3: reel 3 pulse bit (right)
	// bit4:   replay LED (unconfirmed)
	// bit5:   "TROUBLE" LED
	// bit6-7: unused

	m_reel[0]->write(data & 0x0f);
	m_lay_led_status[0] = BIT(data, 5);
}

void newpulsar_state::drive_latch_c_w(offs_t offset, u8 data)
{
	// bit0: frame LED 1
	// bit1: frame LED 2
	// bit2: frame LED 3 (top panel 777 and KELOT / PANTHER BIG area)
	// bit3: frame LED 4
	// bit4: payout / credit game button LED
	// bit5: medal lockout solenoid
	// bit6: knocker solenoid (internal cabinet knock sound)
	// bit7: hopper motor signal out

	// bit0, 1, 3: frame LED
	m_lay_led_frame[0] = BIT(data, 0);
	m_lay_led_frame[1] = BIT(data, 1);
	m_lay_led_frame[2] = BIT(data, 3);
	m_lay_led_frame[3] = BIT(data, 1);
	m_lay_led_frame[4] = BIT(data, 0);

	// bit2: frame LED 3 (top panel 777 and KELOT / PANTHER BIG area)
	m_lay_led_toppanel = BIT(data, 2);

	// bit4: payout / credit game button LED
	m_lay_led_payout = BIT(data, 4);

	// bit5: medal lockout solenoid
	m_solenoid = BIT(data, 5);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	// bit6: knocker solenoid (internal cabinet knock sound)
	playing_knocker_samples(BIT(data, 6));
	m_lay_signal_knocker = BIT(data, 6);

	// bit7: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	m_hopper = BIT(data, 7);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;
}

void newpulsar_state::signal_and_comm_w(offs_t offset, u8 data)
{
	// bit0-3: SEG common bit / LED common bit
	// bit4:   JAC game signal out
	// bit5:   BIG bonus signal out
	// bit6:   total OUT pulse out
	// bit7:   total IN pulse out

	for (u8 i = 0; i < 4; i++)
		if (BIT(data, i))
			m_led_comm_latch[i] = m_led_data_pending;

	for (u8 i = 0; i < 4; i++)
		if (BIT(data, i))
			m_segdata[i] = m_segdata_pending;

	// SEG credit
	m_lay_seg_credit[0] = m_segdata[2];		// SEG left
	m_lay_seg_credit[1] = m_segdata[3];		// SEG right
	// SEG payout
	m_lay_seg_payout[0] = m_segdata[0];		// SEG left
	m_lay_seg_payout[1] = m_segdata[1];		// SEG right

	for (u8 i = 0; i < 4; i++)
		m_lay_led_counter[i + 8] = BIT(m_led_comm_latch[0], i);
	for (u8 i = 0; i < 3; i++)
		m_lay_led_jac[i] = BIT(m_led_comm_latch[0], i + 4);
	m_lay_led_status[1] = BIT(m_led_comm_latch[0], 7);
	for (u8 i = 0; i < 8; i++)
		m_lay_led_counter[i] = BIT(m_led_comm_latch[1], i);

	// bit4: JAC game signal out
	m_lay_signal_jac = BIT(data, 4);
	// bit5: BIG bonus signal out
	m_lay_signal_big = BIT(data, 5);

	// bit6: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));
	// bit7: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));

	for (u8 i = 0; i < 3; i++)
		m_lay_led_stop[i] = BIT(m_led_comm_latch[2], i);

	m_lay_led_insert = BIT(m_led_comm_latch[3], 0);
	m_lay_led_start = BIT(m_led_comm_latch[3], 1);

	m_lay_led_betline[3] = BIT(m_led_comm_latch[3], 4);	// medals 3 (upper)
	m_lay_led_betline[1] = BIT(m_led_comm_latch[3], 3);	// medals 2 (upper)
	m_lay_led_betline[0] = BIT(m_led_comm_latch[3], 2);	// medal  1
	m_lay_led_betline[2] = BIT(m_led_comm_latch[3], 5);	// medals 2 (lower)
	m_lay_led_betline[4] = BIT(m_led_comm_latch[3], 6);	// medals 3 (lower)
}

void newpulsar_state::seg_data_w(offs_t offset, u8 data)
{
	// bit0-7: SEG data pattern
	m_segdata_pending = data;
}

void newpulsar_state::led_data_w(offs_t offset, u8 data)
{
	// bit0-7: LED status pattern
	m_led_data_pending = data;
}

void newpulsar_state::soundcode_w(offs_t offset, u8 data)
{
	// bit0-7: soundcode write
	m_soundlatch->write(data ^ 0xff);
}

u8 newpulsar_state::le1080a_r(offs_t offset)
{
	u8 port = 0xff;

	switch (offset)
	{
		case 0x19:
			port = m_in_switch->read();	// "UCHIDOME" / "UCHIDOME" reset switch
			break;

		default:
			port = 0xff;
			break;
	}

	return port;
}

void newpulsar_state::le1080a_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x18:
			soundcode_w(0, data);
			break;

		default:
			break;
	}
}

void newpulsar_state::le1080a_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}

void newpulsar_state::newpulsar_mem_map(address_map &map)
{
	le1080a_mem_map(map);

	map(0xeff8, 0xeff8).r(FUNC(newpulsar_state::rng_r<0>)).w(FUNC(newpulsar_state::drive_latch_a_w));
	map(0xeff9, 0xeff9).r(FUNC(newpulsar_state::rng_r<1>)).w(FUNC(newpulsar_state::drive_latch_b_w));
	map(0xeffa, 0xeffa).r(FUNC(newpulsar_state::rng_r<2>)).w(FUNC(newpulsar_state::drive_latch_c_w));
	map(0xeffb, 0xeffb).r(FUNC(newpulsar_state::switch_sense_a_r)).w(FUNC(newpulsar_state::signal_and_comm_w));
	map(0xeffc, 0xeffc).r(FUNC(newpulsar_state::switch_sense_b_r)).w(FUNC(newpulsar_state::seg_data_w));
	map(0xeffd, 0xeffd).w(FUNC(newpulsar_state::led_data_w));
	map(0xeffe, 0xeffe).nopw();
}

void newpulsar_state::audio_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0xfe00, 0xffff).ram();
}

void newpulsar_state::audio_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xfe, 0xfe).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xfe, 0xff).w(m_ym2413, FUNC(ym2413_device::write));
}


static INPUT_PORTS_START( newpulsar )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(newpulsar_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("UCHIDOME Reset Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(newpulsar_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(newpulsar_state::selector_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal selector sensor 1
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Door Key")

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(newpulsar_state::opto_r<0>))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(newpulsar_state::opto_r<1>))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(newpulsar_state::opto_r<2>))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")

	// Cabinet option switches and other inputs
	PORT_START("SWITCH")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_CONFNAME( 0x04, 0x04, "SEISAN" )
	PORT_CONFSETTING(    0x04, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_CONFNAME( 0x10, 0x00, "UCHIDOME" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(newpulsar_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	"knocker",
	nullptr
};

void newpulsar_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start((0 + m_samples_ch) & 1, 0);
		m_samples_ch++;
	}
}

void newpulsar_state::playing_knocker_samples(int state)
{
	if (state)
	{
		m_samples->start((2 + m_samples_ch) & 1, 1);
		m_samples_ch++;
	}
}


void newpulsar_state::newpulsar(machine_config &config)
{
	// basic machine hardware
	LE1080A(config, m_maincpu, XTAL(8_MHz_XTAL) / 2); // 8 MHz / 2 = 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &newpulsar_state::newpulsar_mem_map);
	// Install custom handlers for LE1080A internal system registers (0xff18-0xff19):
	// - Offset 0x18 (0xff18): Write-only latch to output soundcode data to the sound CPU
	// - Offset 0x19 (0xff19): Read-only port to sample cabinet switches (UCHIDOME / SEISAN)
	m_maincpu->install_sysreg_handler(0x18, 0x19, read8sm_delegate(*this, FUNC(newpulsar_state::le1080a_r)), write8sm_delegate(*this, FUNC(newpulsar_state::le1080a_w)));

	Z80(config, m_audiocpu, XTAL(8_MHz_XTAL) / 2); // 8 MHz / 2 = 4 MHz ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &newpulsar_state::audio_mem_map);
	m_audiocpu->set_addrmap(AS_IO, &newpulsar_state::audio_io_map);
	// Audio CPU NMI frequency is currently estimated from observed sound
	// playback timing and overall behavior, not from measured hardware.
	m_audiocpu->set_periodic_int(FUNC(newpulsar_state::sound_nmi), attotime::from_hz(2500));

	GENERIC_LATCH_8(config, m_soundlatch);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// Configure sensor emulation modes: Yamasa 2-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_YAMASA1);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_YAMASA1);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(50*2);
	m_pachislot->hopper_cb().set(FUNC(newpulsar_state::playing_hopper_samples));

	// reel device
	REEL(config, m_reel[0], YAMASA_200STEP_REEL, 37, 39, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(newpulsar_state::opto_cb<0>));
	REEL(config, m_reel[1], YAMASA_200STEP_REEL, 37, 39, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(newpulsar_state::opto_cb<1>));
	REEL(config, m_reel[2], YAMASA_200STEP_REEL, 37, 39, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(newpulsar_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 3.57_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.0);

	// samples hardware (plays physical mechanical sounds like hopper payout and cabinet knocker)
	SAMPLES(config, m_samples);
	m_samples->set_channels(4);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psnewpul )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "newpulsar_main.bin", 0x0000, 0x2000, CRC(cb4f0a68) SHA1(b8f84ec9559e6d899d84aa11bd4fd2b4fc19341e) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "newpulsar_music.bin", 0x0000, 0x4000, CRC(5f98a8a7) SHA1(27c3705288fb2dc6f169ebbaea6e3401ca69c957) )
ROM_END

ROM_START( psdiversxx )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "diversxx_main.bin", 0x0000, 0x2000, CRC(42fae6b0) SHA1(bfd950cff78df6902fcebbc3cb7028d75d9d1b9b) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "diversxx_music.bin", 0x0000, 0x4000, CRC(ef217387) SHA1(3ac743c3887e6d87a39964d202b2895c3f769445) )
ROM_END

ROM_START( pspinkpan )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "pinkpanther_main.bin", 0x0000, 0x2000, CRC(76fbaf68) SHA1(75399553777191d32d8a808e582766cf143ca475) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "pinkpanther_music.bin", 0x0000, 0x4000, CRC(bd90fced) SHA1(f4f7072916c109059139dc64df524e92d4e98106) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,   INPUT,     CLASS,           INIT,           MONITOR, COMPANY,  FULLNAME,                                    FLAGS,                                      LAYOUT
GAMEL( 1993, psnewpul,   0,      newpulsar, newpulsar, newpulsar_state, init_newpulsar, 0,       "Yamasa", "New Pulsar (4th Generation, Type A)",       (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_newpulsar )
GAMEL( 1995, psdiversxx, 0,      newpulsar, newpulsar, newpulsar_state, init_common,    0,       "Yamasa", "Divers XX (4th Generation, Type A)",        (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_diversxx )
GAMEL( 1995, pspinkpan,  0,      newpulsar, newpulsar, newpulsar_state, init_common,    0,       "Yamasa", "Pink Panther (4th Generation, Type A)",     (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_pinkpanther )
