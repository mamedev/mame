// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Sammy Pachi-Slot Series Driver
    LE1080A(V1) + 3-reel + SN76496 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2025-06-01.

    Supported Games:
    - Aladdin Master  (c) 1995 Sammy  (4th Generation, Type A-C)

    Hardware:
    - Main CPU: LE1080A-PA @ 2.5 MHz (2.5 MHz XTAL)
    - Sound: SN76496 @ 2.5 MHz (2.5 MHz XTAL)
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

    NOTE:
    [LE-Tech LE1080A CPU & Interrupt Architecture]
    - The LE-Tech LE1080A is a custom Z80-compatible microcontroller integrating
      internal battery-backed SRAM, security features, hardware pseudo-random
      generators, and custom programmable interrupt timers. For complete details
      on system registers and internal peripheral maps, refer to 'cpu/z80/le1080a.cpp'.
    - Because the exact internal timer reload and register programming sequences
      are not yet fully reverse-engineered from firmware code, the main periodic
      interrupt rate (LE1080A IRQ1 = 512 Hz at vector 0x00a8) is provisionally
      calibrated from physical reel revolution pacing and lamp refresh cycles.

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, triggering the LE1080A PFI interrupt (IRQ2)
      allowing the CPU to execute its state-saving routine into battery-backed
      internal SRAM during the reservoir capacitance hold-up time.
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
    - Real machines route coin-in and coin-out signals through 'pachislot_device'.
    - This unified architecture provides several key advantages:
      1. Realistic, hardware-accurate hopper ejection intervals and sensor pulse widths
         without relying on the deprecated 'ticket.cpp' hopper implementation.
      2. Direct callback dispatching for synchronized mechanical payout sound playback
         via the MAME 'samples' device.
      3. A fully standardized driver interface, ensuring architectural consistency
         across all supported platforms.

    [Stepper Reel Drive & Physical Start Kick Calibration]
    - Aladdin Master restarts from a 0x00 hold phase into an opposite-phase pattern.
      On real hardware, this generates an initial small downward mechanical kick rather
      than a full 4 half-step jump.
      This driver enables extended stepper_device flags:
      (STEPPER_IGNORE_ZERO | STEPPER_ALLOW_MULTISTEP | STEPPER_CLAMP_OPPOSITE_PHASE_START | STEPPER_KICK_DIR_POSITIVE)
      to clamp the initial step edge and authentically recreate the real start motion.

    [Display & Lamp Output Synchronization]
    - Due to physical dynamic multiplexing, the ROM firmware clears common lines
      and rewrites segment/lamp states across multiple interrupt phases.
    - Directly passing these intermediate writes to MAME output finders causes
      visible display flickering and transient glitch frames in the layout view.
    - To eliminate flickering, output states are staged in pending buffers and
      atomically committed to layout outputs in sync with the periodic interrupt cycle.

    [NVRAM Cold-Start Behavior and Error E1]
    - On a fresh start with no existing NVRAM file, the game will report
      a RAM check error (E1) and cannot proceed to normal gameplay.
    - This error cannot be cleared by a simple reset or door key operation.
    - To initialise the backup RAM, the following procedure is required:
      1. Turn Setting Key (F2) on and reset (F3 or restart).
      2. Change setting value (F1) to any value.
      3. Operate start lever to confirm setting.
      4. Turn Setting Key (F2) off and reset again.
    - After this procedure, the game will boot normally.
    - This behaviour reflects the original hardware, where the backup RAM
      must be initialised through the setting procedure.

    [Bookkeeping / Mechanical Counters]
    - Total IN (bit 4) and Total OUT (bit 5) pulse outputs driven by 0x80
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

    [Setting Change Procedure]
    - Standard 4th generation sequence:
      1. Press the 'P' key to enter the power-fail state.
      2. Turn Setting Key (F2) on and restart/reset machine.
      3. Select desired setting (1-6) on 7-segment display with Change Setting Value button (F1).
      4. Confirm setting value by operating Start Lever.
      5. Turn Setting Key off to enter normal game state.

    [Game Rules & Options]
    - Configurable SEISAN (automatic payout) and UCHIDOME (automatic game-stop)
      behaviors are supported via cabinet option switches ("SEISAN": Off / On,
      "UCHIDOME": Off / On). Note that option switch changes require cycling through
      setting mode to reinitialize state.

***************************************************************************/

#include "emu.h"

#include "pachislotdev.h"

#include "cpu/z80/le1080a.h"
#include "machine/steppers.h"
#include "sound/sn76496.h"
#include "sound/samples.h"
#include "speaker.h"


namespace {

// internal artwork
#include "aladdinmaster.lh"

class aladdinmaster_state : public driver_device
{
public:
	aladdinmaster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "le1080a")
		, m_pachislot(*this, "pachislot")
		, m_sn76496(*this, "sn76496")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_lamp_frame(*this, "lamp_frame%u", 0U)
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_lamp_bcount(*this, "lamp_bcount%u", 0U)
		, m_lay_lamp_replay(*this, "lamp_replay")
		, m_lay_lamp_insert(*this, "lamp_insert")
		, m_lay_lamp_wait(*this, "lamp_wait")
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void aladdinmaster(machine_config &config);

	void init_aladdinmaster();

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

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

private:
	required_device<le1080a_device> m_maincpu;
	required_device<pachislot_device> m_pachislot;
	required_device<sn76496_device> m_sn76496;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<3> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<8> m_lay_lamp_frame;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<4> m_lay_lamp_bcount;
	output_finder<> m_lay_lamp_replay;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_wait;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	u8 reel_status_r(offs_t offset);
	template <unsigned N> u8 rng_r(offs_t offset);

	void reel1_and_signal_w(offs_t offset, u8 data);
	void reel2_and_drive_w(offs_t offset, u8 data);
	void reel3_and_seg_comm_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void lamp_latch_bcount_w(offs_t offset, u8 data);
	void lamp_latch_bet_stop_w(offs_t offset, u8 data);
	void lamp_latch_frame_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	u8 m_opto[3];
	u8 m_segdata[4];
	u8 m_segdata_pending;
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;
};


void aladdinmaster_state::init_aladdinmaster()
{
	// LE1080A Interrupt vector setup
	m_maincpu->set_irq1_address(0x00a8);
	m_maincpu->set_irq2_address(0x0066);

	// Interrupt frequencies are provisionally calibrated based on observed gameplay
	// behavior such as physical reel revolution rate and overall game pacing.
	m_maincpu->set_irq1_rate_hz(512);
}

void aladdinmaster_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_segdata_pending));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void aladdinmaster_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segdata_pending = 0;

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
void aladdinmaster_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void aladdinmaster_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void aladdinmaster_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

// Provisional trigger handler simulating the physical power failure supervisor circuit.
// Triggers the LE1080A PFI interrupt (IRQ2 at vector 0x0066).
INPUT_CHANGED_MEMBER(aladdinmaster_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
		m_maincpu->trigger_power_fail();
	}
}

u8 aladdinmaster_state::switch_sense_a_r(offs_t offset)
{
	// bit0: medal selector sensor 0
	// bit1: medal selector sensor 1
	// bit2: bet button
	// bit3: hopper medal out signal
	// bit4: payout / credit game button
	// bit5: start lever
	// bit6: setting key
	// bit7: unused

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[0]->read();
}

u8 aladdinmaster_state::switch_sense_b_r(offs_t offset)
{
	// bit0:   "SEISAN" ("清算") switch (automatic payout)
	// bit1:   unknown
	// bit2:   unknown
	// bit3:   door key / reset / change setting value button
	// bit4:   change setting value button
	// bit5:   unknown
	// bit6-7: unknown

	return m_in_port[1]->read();
}

u8 aladdinmaster_state::reel_status_r(offs_t offset)
{
	// bit0:   reel 1 sensor (left)
	// bit1:   reel 2 sensor (center)
	// bit2:   reel 3 sensor (right)
	// bit3-4: unknown
	// bit5:   reel 1 stop button (left)
	// bit6:   reel 2 stop button (center)
	// bit7:   reel 3 stop button (right)

	return m_in_port[2]->read();
}

// Kept as helper handlers for now so bit usage notes and temporary
// debug/test input remapping can remain local while program analysis
// is still in progress.
template <unsigned N>
u8 aladdinmaster_state::rng_r(offs_t offset)
{
	// bit0-7: Read once after lever ON and before reel rotation. Possibly LE1080A internal random source.
	return m_maincpu->internal_rng_r(N);
}

void aladdinmaster_state::reel1_and_signal_w(offs_t offset, u8 data)
{
	// bit0-3: reel 1 pulse bit
	// bit4:   total IN pulse out
	// bit5:   total OUT pulse out
	// bit6:   JAC game signal out
	// bit7:   BIG bonus signal out

	// bit0-3: reel 1 pulse bit
	m_reel[0]->write(data & 0x0f);

	// bit4: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	// bit5: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));

	// bit6: JAC game signal out
	m_lay_signal_jac = BIT(data, 6);
	// bit7: BIG bonus signal out
	m_lay_signal_big = BIT(data, 7);
}

void aladdinmaster_state::reel2_and_drive_w(offs_t offset, u8 data)
{
	// bit0-3: reel 2 pulse bit
	// bit4:   medal lockout solenoid
	// bit5:   wait lamp
	// bit6:   unknown
	// bit7:   hopper motor drive

	// bit0-3: reel 2 pulse bit
	m_reel[1]->write(data & 0x0f);

	// bit4: medal lockout solenoid
	m_solenoid = !BIT(data, 4);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	// bit5: wait lamp
	m_lay_lamp_wait = BIT(data, 5);

	// bit7: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	m_hopper = BIT(data, 7);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;
}

void aladdinmaster_state::reel3_and_seg_comm_w(offs_t offset, u8 data)
{
	// bit0-3: reel 3 pulse bit
	// bit4-7: SEG common bit

	// bit0-3: reel 3 pulse bit
	m_reel[2]->write(data & 0x0f);

	for (u8 i = 0; i < 4; i++)
		if (BIT(data, i + 4))
			m_segdata[i] = m_segdata_pending;

	// SEG credit
	m_lay_seg_credit[0]	= m_segdata[3];			// high
	m_lay_seg_credit[1]	= m_segdata[2];			// low
	// SEG payout
	m_lay_seg_payout[0]	= m_segdata[1];			// high
	m_lay_seg_payout[1]	= m_segdata[0];			// low
}

void aladdinmaster_state::seg_data_w(offs_t offset, u8 data)
{
	// bit0-7: SEG data

	m_segdata_pending = data;
}

void aladdinmaster_state::lamp_latch_bcount_w(offs_t offset, u8 data)
{
	// bit0:   insert medal lamp
	// bit1:   replay lamp
	// bit2:   SINGLE BONUS lamp
	// bit3:   BONUS CHANCE lamp
	// bit4:   BIG CHANCE lamp
	// bit5:   ALADDIN CHANCE lamp
	// bit6-7: unknown

	// bit0: insert medal lamp
	m_lay_lamp_insert = BIT(data, 0);

	// bit1: replay lamp
	m_lay_lamp_replay = BIT(data, 1);

	// bit2-5: bonus chance lamp
	m_lay_lamp_bcount[0] = BIT(data, 5);			// ALADDIN CHANCE
	m_lay_lamp_bcount[1] = BIT(data, 4);			// BIG CHANCE
	m_lay_lamp_bcount[2] = BIT(data, 3);			// BONUS CHANCE
	m_lay_lamp_bcount[3] = BIT(data, 2);			// SINGLE BONUS
}

void aladdinmaster_state::lamp_latch_bet_stop_w(offs_t offset, u8 data)
{
	// bit0-4: bet medal(s) line lamp (physical upper/lower pairs)
	// bit5-7: stop lamp

	// bit0-4: bet line lamp (physical upper/lower pairs)
	m_lay_lamp_betline[4] = BIT(data, 0);			// line lamp 3
	m_lay_lamp_betline[2] = BIT(data, 1);			// line lamp 2
	m_lay_lamp_betline[0] = BIT(data, 2);			// line lamp 1
	m_lay_lamp_betline[1] = BIT(data, 3);			// line lamp 2
	m_lay_lamp_betline[3] = BIT(data, 4);			// line lamp 3

	// bit5-7: stop lamp
	for (u8 i = 0; i < 3; i++)
		m_lay_lamp_stop[i] = BIT(data, i + 5);
}

void aladdinmaster_state::lamp_latch_frame_w(offs_t offset, u8 data)
{
	// bit0-7: frame lamp
	for (u8 i = 0; i < 8; i++)
		m_lay_lamp_frame[i] = BIT(data, i);
}

void aladdinmaster_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}

void aladdinmaster_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x80).r(FUNC(aladdinmaster_state::switch_sense_a_r)).w(FUNC(aladdinmaster_state::reel1_and_signal_w));
	map(0x81, 0x81).r(FUNC(aladdinmaster_state::switch_sense_b_r)).w(FUNC(aladdinmaster_state::reel2_and_drive_w));
	map(0x82, 0x82).r(FUNC(aladdinmaster_state::reel_status_r)).w(FUNC(aladdinmaster_state::reel3_and_seg_comm_w));
	map(0x83, 0x83).r(FUNC(aladdinmaster_state::rng_r<0>)).w(FUNC(aladdinmaster_state::seg_data_w));
	map(0x84, 0x84).r(FUNC(aladdinmaster_state::rng_r<1>)).w(FUNC(aladdinmaster_state::lamp_latch_bcount_w));
	map(0x85, 0x85).r(FUNC(aladdinmaster_state::rng_r<2>)).w(FUNC(aladdinmaster_state::lamp_latch_bet_stop_w));
	map(0x86, 0x86).r(FUNC(aladdinmaster_state::rng_r<3>)).w(FUNC(aladdinmaster_state::lamp_latch_frame_w));
	map(0x87, 0x87).w(m_sn76496, FUNC(sn76496_device::write));
}


static INPUT_PORTS_START( aladdinmaster )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(aladdinmaster_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(aladdinmaster_state::selector_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal selector sensor 1
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(aladdinmaster_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	// Cabinet option switches and other inputs
	PORT_START("INPUT1")
	PORT_CONFNAME( 0x01, 0x00, "SEISAN" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x00, "UCHIDOME" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdinmaster_state::opto_r<0>))	// reel 1 sensor (left)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdinmaster_state::opto_r<1>))	// reel 2 sensor (center)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdinmaster_state::opto_r<2>))	// reel 3 sensor (right)
	PORT_BIT( 0x18, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aladdinmaster_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void aladdinmaster_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void aladdinmaster_state::aladdinmaster(machine_config &config)
{
	// basic machine hardware
	LE1080A(config, m_maincpu, XTAL(2'500'000)); // LE1080A-PA 2.5 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &aladdinmaster_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &aladdinmaster_state::io_map);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(50*2);
	m_pachislot->hopper_cb().set(FUNC(aladdinmaster_state::playing_hopper_samples));

	// Configure sensor emulation modes: Sammy 2-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SAMMY1);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], SAMMY_200STEP_REEL, 14, 16, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(aladdinmaster_state::opto_cb<0>));
	REEL(config, m_reel[1], SAMMY_200STEP_REEL, 14, 16, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(aladdinmaster_state::opto_cb<1>));
	REEL(config, m_reel[2], SAMMY_200STEP_REEL, 14, 16, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(aladdinmaster_state::opto_cb<2>));

	// Aladdin Master restarts from a 0x00 hold into an opposite-phase pattern, but the
	// visible motion at reel start is only a small downward kick rather than a full 4 half-step jump.
	// Clamp that first restart edge to a normal single half-step move and force the
	// kick direction to match this machine until other Sammy reels are confirmed.
	for (int i = 0; i < 3; i++)
		m_reel[i]->set_flags(STEPPER_IGNORE_ZERO | STEPPER_ALLOW_MULTISTEP | STEPPER_CLAMP_OPPOSITE_PHASE_START | STEPPER_KICK_DIR_POSITIVE);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SN76496(config, m_sn76496, XTAL(2'500'000)).add_route(ALL_OUTPUTS, "speaker", 0.8);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psaladdinm )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "aladdinmaster.bin", 0x0000, 0x2000, CRC(f3e02f67) SHA1(cfe631556a6136b5ff1296394c68eba04934fc68) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,       INPUT,         CLASS,               INIT,               MONITOR, COMPANY, FULLNAME,                                       FLAGS,                                      LAYOUT
GAMEL( 1995, psaladdinm, 0,      aladdinmaster, aladdinmaster, aladdinmaster_state, init_aladdinmaster, 0,       "Sammy", "Aladdin Master (4th Generation, Type A-C)",    (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_aladdinmaster )
