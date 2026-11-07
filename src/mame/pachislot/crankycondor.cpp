// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Universal Group Pachi-Slot Series Driver
    LE1080A(V1) + 3-reel + Z80 + YM2413 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2024-10-18.

    Supported Games:
    - Cranky Condor  (c) 1995 Universal  (4th Generation, Type A-400)

    Hardware:
    - Main CPU: LE1080A-PA @ 6.0 MHz (12.0 MHz XTAL / 2)
    - Sound CPU: Z80 @ 6.0 MHz (12.0 MHz XTAL / 2)
    - Sound: YM2413 @ 3.579545 MHz
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: Internal LE1080A battery-backed RAM

    TODO:
    - LE1080A interrupt addresses and internal random-number behavior are
      still not fully understood, so some model-specific behavior remains
      implemented in the driver rather than in le1080a.cpp.
    - IRQ0 / IRQ1 / power-fail preparation interrupt addresses differ by
      machine and manufacturer.
      These are currently assigned from analysis results found in each
      program rather than being derived automatically by the LE1080A core.
    - The payout SEG can show occasional temporary flicker.
      This behavior has not yet been confirmed against real hardware.

    NOTE:
    [LE-Tech LE1080A CPU & Interrupt Architecture]
    - The LE-Tech LE1080A is a custom Z80-compatible microcontroller integrating
      internal battery-backed SRAM, security features, hardware pseudo-random
      generators, and custom programmable interrupt timers. For complete details
      on system registers and internal peripheral maps, refer to 'cpu/z80/le1080a.cpp'.
    - Because the exact internal timer reload and register programming sequences
      are not yet fully reverse-engineered from firmware code, the main periodic
      interrupt rates (LE1080A IRQ0 = 448 Hz at 0x0084, IRQ1 = 512 Hz at 0x0088)
      are provisionally calibrated from physical reel revolution pacing and display multiplexing.
    - Sound CPU (Z80) periodic interrupt is driven at an estimated 750 Hz.

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, triggering the LE1080A PFI interrupt (IRQ2)
      allowing the CPU to execute its state-saving routine into battery-backed
      internal SRAM during the reservoir capacitance hold-up time.
    - Because MAME currently lacks a pre-reset / pre-shutdown callback
      mechanism to allow the CPU to gracefully complete its power-down
      sequence before machine_reset() or exit (e.g. F3 / ESC keys), this
      driver provisionally provides a manual 'P' key (powerfail_trigger).
    - Pressing the 'P' key asserts the LE1080A PFI interrupt (IRQ2 at vector 0x009c),
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

    [Display & Lamp Output Synchronization]
    - Due to physical dynamic multiplexing, the ROM firmware clears common lines
      and rewrites segment/lamp states across multiple interrupt phases.
    - Directly passing these intermediate writes to MAME output finders causes
      visible display flickering and transient glitch frames in the layout view.
    - To eliminate flickering, output states are staged in pending buffers and
      atomically committed to layout outputs in sync with the periodic interrupt cycle.

    [Bookkeeping / Mechanical Counters]
    - Total IN (bit 6) and Total OUT (bit 7) pulse outputs driven by port 0x00
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
#include "crankycondor.lh"

class crankycondor_state : public driver_device
{
public:
	crankycondor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "le1080a")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_ym2413(*this, "ym2413")
		, m_pachislot(*this, "pachislot")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_lamp_frame(*this, "lamp_frame%u", 0U)
		, m_lay_lamp_gameover(*this, "lamp_gameover")
		, m_lay_lamp_insert(*this, "lamp_insert")
		, m_lay_lamp_start(*this, "lamp_start")
		, m_lay_lamp_replay(*this, "lamp_replay")
		, m_lay_lamp_credit(*this, "lamp_credit")
		, m_lay_lamp_payout(*this, "lamp_payout")
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_bcount(*this, "lamp_bcount%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_lamp_reel(*this, "lamp_reel%u_%u", 0U, 0U)
		, m_lay_led_button(*this, "led_button%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_seg_bonus(*this, "seg_bonus")
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void crankycondor(machine_config &config);

	void init_crankycondor();

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
	void crankycondor_io_map(address_map &map) ATTR_COLD;
	void audio_mem_map(address_map &map) ATTR_COLD;

	void commit_lamp_buffers();

	TIMER_CALLBACK_MEMBER(lamp_buffer_commit_tick);

private:
	required_device<le1080a_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<ym2413_device> m_ym2413;
	required_device<pachislot_device> m_pachislot;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<3> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<8> m_lay_lamp_frame;
	output_finder<> m_lay_lamp_gameover;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_start;
	output_finder<> m_lay_lamp_replay;
	output_finder<> m_lay_lamp_credit;
	output_finder<> m_lay_lamp_payout;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_bcount;
	output_finder<3> m_lay_lamp_stop;
	output_finder<3, 3> m_lay_lamp_reel;
	output_finder<2> m_lay_led_button;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_seg_bonus;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	u8 cabinet_switch_r(offs_t offset);
	template <unsigned N> u8 rng_r(offs_t offset);

	void drive_latch_a_w(offs_t offset, u8 data);
	void drive_latch_b_w(offs_t offset, u8 data);
	void drive_latch_c_w(offs_t offset, u8 data);
	void lamp_latch_stop_bet_w(offs_t offset, u8 data);
	void reel_lamp_w(offs_t offset, u8 data);
	void lamp_latch_frame_misc_w(offs_t offset, u8 data);
	void bonus_seg_replay_w(offs_t offset, u8 data);
	void seg_digit_bcount_w(offs_t offset, u8 data);
	void seg_data_payout_w(offs_t offset, u8 data);
	void soundcode_w(offs_t offset, u8 data);

	emu_timer *m_lamp_buffer_commit_timer = nullptr;

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	u8 m_segdata[4];
	u8 m_segdata_pending;
	u8 m_opto[3];
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;

	u8 m_reel_lamp_pending[3][3];
	u8 m_betline_lamp_pending[5];
	u8 m_seg_credit_pending[2];
	u8 m_seg_payout_pending[2];
	u8 m_seg_bonus_pending;

	void playing_hopper_samples(int state);
};


void crankycondor_state::init_crankycondor()
{
	// LE1080A Interrupt vector setup
	m_maincpu->set_irq0_address(0x0084);
	m_maincpu->set_irq1_address(0x0088);
	m_maincpu->set_irq2_address(0x009c);

	// Interrupt frequencies are provisionally calibrated based on observed gameplay
	// behavior such as physical reel revolution rate and overall game pacing.
	m_maincpu->set_irq0_rate_hz(448);
	m_maincpu->set_irq1_rate_hz(512);
}

void crankycondor_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_reel_lamp_pending));
	save_item(NAME(m_betline_lamp_pending));
	save_item(NAME(m_seg_credit_pending));
	save_item(NAME(m_seg_payout_pending));
	save_item(NAME(m_seg_bonus_pending));
	save_item(NAME(m_segdata_pending));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_lamp_buffer_commit_timer = timer_alloc(FUNC(crankycondor_state::lamp_buffer_commit_tick), this);

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void crankycondor_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segdata_pending = 0;

	memset(m_reel_lamp_pending, 0, sizeof(m_reel_lamp_pending));
	memset(m_betline_lamp_pending, 0, sizeof(m_betline_lamp_pending));
	memset(m_seg_credit_pending, 0, sizeof(m_seg_credit_pending));
	memset(m_seg_payout_pending, 0, sizeof(m_seg_payout_pending));
	m_seg_bonus_pending = 0;

	m_lamp_buffer_commit_timer->adjust(attotime::from_hz(512), 0, attotime::from_hz(512));

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;

	commit_lamp_buffers();

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
void crankycondor_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void crankycondor_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void crankycondor_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();

	// Flush pending atomic display/lamp buffers to prevent visual tearing
	commit_lamp_buffers();
}

void crankycondor_state::commit_lamp_buffers()
{
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 3; y++)
			m_lay_lamp_reel[x][y] = m_reel_lamp_pending[x][y];

	for (int i = 0; i < 5; i++)
		m_lay_lamp_betline[i] = m_betline_lamp_pending[i];

	for (int i = 0; i < 2; i++)
	{
		m_lay_seg_credit[i] = m_seg_credit_pending[i];
		m_lay_seg_payout[i] = m_seg_payout_pending[i];
	}

	m_lay_seg_bonus = m_seg_bonus_pending;
}

TIMER_CALLBACK_MEMBER(crankycondor_state::lamp_buffer_commit_tick)
{
	commit_lamp_buffers();
}

// Provisional trigger handler simulating the physical power failure supervisor circuit.
// Triggers the LE1080A PFI interrupt (IRQ2 at vector 0x009c).
INPUT_CHANGED_MEMBER(crankycondor_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
		m_maincpu->trigger_power_fail();
	}
}

u8 crankycondor_state::switch_sense_a_r(offs_t offset)
{
	// bit0: reel 1 stop button (left)
	// bit1: reel 2 stop button (center)
	// bit2: reel 3 stop button (right)
	// bit3: medal selector sensor 3 (CJ) Active_HIGH
	// bit4: medal selector sensor 1 (CE) Active_LOW
	// bit5: medal selector sensor 2 (Cr) Active_LOW
	// bit6: reset / change setting value button
	// bit7: start lever

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[0]->read();
}

u8 crankycondor_state::switch_sense_b_r(offs_t offset)
{
	// bit0: hopper sensor 1 (Ho/HJ)
	// bit1: hopper sensor 2 (Ho)
	// bit2: hopper medal out test (for maintenance?)
	// bit3: door key
	// bit4: setting key
	// bit5: payout / credit game button
	// bit6: bet button
	// bit7: Always set to 1

	return m_in_port[1]->read();
}

u8 crankycondor_state::cabinet_switch_r(offs_t offset)
{
	// bit0:   "UCHIDOME" ("打ち止め") switch (automatic game-stop)
	// bit1:   "SEISAN" ("清算") switch (automatic payout)
	// bit2:   subtank medal overflow sensor
	// bit3:   reel 1 sensor (left)
	// bit4:   reel 2 sensor (center)
	// bit5:   reel 3 sensor (right)
	// bit6-7: unused

	return m_in_port[2]->read();
}

// Kept as helper handlers for now so bit usage notes and temporary
// debug/test input remapping can remain local while program analysis
// is still in progress.
template <unsigned N>
u8 crankycondor_state::rng_r(offs_t offset)
{
	// bit0-7: Read once after lever ON and before reel rotation. Possibly LE1080A internal random source.
	return m_maincpu->internal_rng_r(N);
}

void crankycondor_state::drive_latch_a_w(offs_t offset, u8 data)
{
	// bit0-3: reel 1 pulse bit (left)
	// bit4:   JAC game signal out
	// bit5:   BIG bonus signal out
	// bit6:   total IN pulse out
	// bit7:   total OUT pulse out

	// bit0-3: reel 1 pulse bit (left)
	m_reel[0]->write(data & 0x0f);

	// bit4: JAC game signal out
	m_lay_signal_jac = BIT(data, 4);
	// bit5: BIG bonus signal out
	m_lay_signal_big = BIT(data, 5);

	// bit6: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	// bit7: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 7));
}

void crankycondor_state::drive_latch_b_w(offs_t offset, u8 data)
{
	// bit0-3: reel 2 pulse bit (center)
	// bit4-5: hopper motor drive
	// bit6:   unknown
	// bit7:   medal lockout solenoid

	// bit0-3: reel 2 pulse bit (center)
	m_reel[1]->write(data & 0x0f);

	// bit4-5: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	if (BIT(data, 5) && !BIT(data, 4)) 			// 0bx10xxxx
	{
		// hopper motor on (forward)
		m_hopper = 1;
		m_lay_signal_hopper = 1;
	}
	else if (!BIT(data, 5) && BIT(data, 4)) 		// 0bx01xxxx
	{
		// hopper motor on (reverse)
		m_hopper = 1;
		m_lay_signal_hopper = 2;
	}
	else
	{
		// hopper motor off
		m_hopper = 0;
		m_lay_signal_hopper = 0;
	}
	m_pachislot->hopper_motor_w(m_hopper);

	// bit7: medal lockout solenoid
	m_solenoid = !BIT(data, 7);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;
}

void crankycondor_state::drive_latch_c_w(offs_t offset, u8 data)
{
	// bit0-3: reel 3 pulse bit (right)
	// bit4:   gameover / wait lamp
	// bit5-7: unknown

	// bit0-3: reel 3 pulse bit (right)
	m_reel[2]->write(data & 0x0f);

	// bit4: gameover / wait lamp
	m_lay_lamp_gameover = BIT(data, 4);
}

void crankycondor_state::lamp_latch_stop_bet_w(offs_t offset, u8 data)
{
	// bit0-2: stop lamp
	// bit3-7: bet medal(s) line lamp (physical upper/lower pairs)

	// bit0-2: stop lamp
	m_lay_lamp_stop[0] = BIT(data, 0);			// left
	m_lay_lamp_stop[1] = BIT(data, 1);			// center
	m_lay_lamp_stop[2] = BIT(data, 2);			// right

	// bit3-7: bet medal(s) line lamp (physical upper/lower pairs)
	m_betline_lamp_pending[0] = BIT(data, 3);		// 1 medal
	m_betline_lamp_pending[1] = BIT(data, 4);		// 2 medals (upper)
	m_betline_lamp_pending[2] = BIT(data, 5);		// 2 medals (lower)
	m_betline_lamp_pending[3] = BIT(data, 6);		// 3 medals (upper)
	m_betline_lamp_pending[4] = BIT(data, 7);		// 3 medals (lower)
}

void crankycondor_state::reel_lamp_w(offs_t offset, u8 data)
{
	// bit0-7: reel lamp
	m_reel_lamp_pending[0][0] = BIT(data, 0);		// reel lamp x:0, y:0
	m_reel_lamp_pending[1][0] = BIT(data, 1);		// reel lamp x:1, y:0
	m_reel_lamp_pending[2][0] = BIT(data, 2);		// reel lamp x:2, y:0
	m_reel_lamp_pending[0][1] = BIT(data, 3);		// reel lamp x:0, y:1
	m_reel_lamp_pending[1][1] = BIT(data, 4);		// reel lamp x:1, y:1
	m_reel_lamp_pending[2][1] = BIT(data, 5);		// reel lamp x:2, y:1
	m_reel_lamp_pending[0][2] = BIT(data, 6);		// reel lamp x:0, y:2
	m_reel_lamp_pending[1][2] = BIT(data, 7);		// reel lamp x:1, y:2
}

void crankycondor_state::lamp_latch_frame_misc_w(offs_t offset, u8 data)
{
	// bit0: reel lamp
	// bit1: start lamp
	// bit2: insert medal lamp
	// bit3: bet button LED
	// bit4: payout / credit game button LED
	// bit5: frame lamp left
	// bit6: frame lamp center
	// bit7: frame lamp right

	// bit0: reel lamp
	m_reel_lamp_pending[2][2] = BIT(data, 0);		// reel lamp x:2, y:2

	// bit1: start lamp
	m_lay_lamp_start = BIT(data, 1);

	// bit2: insert medal lamp
	m_lay_lamp_insert = BIT(data, 2);

	// bit3: bet button LED
	m_lay_led_button[1] = BIT(data, 3);

	// bit4: payout / credit game button LED
	m_lay_led_button[0] = BIT(data, 4);

	// bit5: frame lamp left
	for (int i = 0; i < 2; i++)
		m_lay_lamp_frame[i + 0] = BIT(data, 5);
	// bit6: frame lamp center
	for (int i = 0; i < 4; i++)
		m_lay_lamp_frame[i + 2] = BIT(data, 6);
	// bit7: bonus lamp right
	for (int i = 0; i < 2; i++)
		m_lay_lamp_frame[i + 6] = BIT(data, 7);

	// Credit lamp is provisionally kept off
	m_lay_lamp_credit = 0;
}

void crankycondor_state::bonus_seg_replay_w(offs_t offset, u8 data)
{
	// bit0-6: bonus count SEG
	// bit7:   replay lamp

	// bit0-6: bonus count SEG
	m_seg_bonus_pending = bitswap<8>(data, 7,0,1,2,3,4,5,6) & 0x7f;

	// bit7: replay lamp
	m_lay_lamp_replay = BIT(data, 7);
}

void crankycondor_state::seg_digit_bcount_w(offs_t offset, u8 data)
{
	// bit0-3: SEG common bit
	// bit4-6: bonus count lamp
	// bit7:   unknown (possibly interrupt-related; written from the IRQ0 path?)

	// SEG common bit -> SEG select
	for (u8 i = 0; i < 4; i++)
		if (BIT((data & 0x0f), i))
			m_segdata[i] = bitswap<8>(m_segdata_pending, 7,0,1,2,3,4,5,6);

	// SEG credit
	m_seg_credit_pending[0] = m_segdata[3];			// high
	m_seg_credit_pending[1] = m_segdata[2];			// low
	// SEG payout
	m_seg_payout_pending[0] = m_segdata[1];			// high
	m_seg_payout_pending[1] = m_segdata[0];			// low

	// bit4-6: bonus count lamp
	m_lay_lamp_bcount[0] = BIT(data, 4);			// (1)
	m_lay_lamp_bcount[1] = BIT(data, 5);			// (2)
	m_lay_lamp_bcount[2] = BIT(data, 6);			// (3)
}

void crankycondor_state::seg_data_payout_w(offs_t offset, u8 data)
{
	// bit0-6: SEG data
	// bit7:   payout lamp

	// bit0-6: SEG data
	m_segdata_pending = data & 0x7f;

	// bit7: payout(win) lamp
	m_lay_lamp_payout = BIT(data, 7);
}

void crankycondor_state::soundcode_w(offs_t offset, u8 data)
{
	// bit0-7: soundcode write
	m_soundlatch->write(data);
}

void crankycondor_state::le1080a_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}

void crankycondor_state::crankycondor_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(crankycondor_state::drive_latch_a_w));
	map(0x01, 0x01).w(FUNC(crankycondor_state::drive_latch_b_w));
	map(0x02, 0x02).w(FUNC(crankycondor_state::drive_latch_c_w));
	map(0x03, 0x03).w(FUNC(crankycondor_state::lamp_latch_stop_bet_w));
	map(0x04, 0x04).w(FUNC(crankycondor_state::reel_lamp_w));
	map(0x05, 0x05).w(FUNC(crankycondor_state::lamp_latch_frame_misc_w));
	map(0x06, 0x06).w(FUNC(crankycondor_state::bonus_seg_replay_w));
	map(0x07, 0x07).w(FUNC(crankycondor_state::seg_digit_bcount_w));
	map(0x08, 0x08).w(FUNC(crankycondor_state::seg_data_payout_w));
	map(0x09, 0x09).w(FUNC(crankycondor_state::soundcode_w));
	map(0x10, 0x10).r(FUNC(crankycondor_state::switch_sense_a_r));
	map(0x11, 0x11).r(FUNC(crankycondor_state::switch_sense_b_r));
	map(0x12, 0x12).r(FUNC(crankycondor_state::cabinet_switch_r));
	map(0x13, 0x13).r(FUNC(crankycondor_state::rng_r<0>));
	map(0x14, 0x14).r(FUNC(crankycondor_state::rng_r<1>));
}

void crankycondor_state::audio_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram();
	map(0x4000, 0x4001).w(m_ym2413, FUNC(ym2413_device::write));
	// 0x6000: soundcode latch written by LE1080A and read by the sound CPU
	// 0x6001: unknown sound CPU status writeback; currently unused by the main CPU
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x6001, 0x6001).nopw();
}


static INPUT_PORTS_START( crankycondor )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(crankycondor_state::selector_r<pachislot_device::PACHISLOT_SENSOR2>))	// medal selector sensor 2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(crankycondor_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(crankycondor_state::selector_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal selector sensor 1
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Reset Button / Change Setting Value Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(crankycondor_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(crankycondor_state::hopper_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal hopper sensor 1
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )	// Hopper medal out test (for maintenance?)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Door Key")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )	// Always set to High level.

	// Cabinet option switches and other inputs
	PORT_START("INPUT2")
	PORT_CONFNAME( 0x01, 0x01, "UCHIDOME" )
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x02, "SEISAN" )
	PORT_CONFSETTING(    0x02, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("Subtank Overflow")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(crankycondor_state::opto_r<0>))	// reel sensor 1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(crankycondor_state::opto_r<1>))	// reel sensor 2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(crankycondor_state::opto_r<2>))	// reel sensor 3
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(crankycondor_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void crankycondor_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void crankycondor_state::crankycondor(machine_config &config)
{
	// basic machine hardware
	LE1080A(config, m_maincpu, XTAL(12'000'000) / 2);	// LE1080A-PA 12 MHz / 2 = 6 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &crankycondor_state::le1080a_mem_map);
	m_maincpu->set_addrmap(AS_IO, &crankycondor_state::crankycondor_io_map);

	Z80(config, m_audiocpu, XTAL(12'000'000) / 2);		// 12 MHz / 2 = 6 MHz ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &crankycondor_state::audio_mem_map);
	// Audio CPU interrupt frequency is currently estimated from observed
	// sound playback timing and overall behavior, not from measured hardware.
	// It may require adjustment when more accurate information becomes available.
	m_audiocpu->set_periodic_int(FUNC(crankycondor_state::irq0_line_hold), attotime::from_hz(750));

	GENERIC_LATCH_8(config, m_soundlatch);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// Configure sensor emulation modes: Universal 3-sensor coin selector and 2-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_UNIVERSAL1);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_UNIVERSAL1);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(30*2);
	m_pachislot->hopper_cb().set(FUNC(crankycondor_state::playing_hopper_samples));

	// reel device
	REEL(config, m_reel[0], UNIVERSAL_200STEP_REEL, 318, 320, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(crankycondor_state::opto_cb<0>));
	REEL(config, m_reel[1], UNIVERSAL_200STEP_REEL, 318, 320, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(crankycondor_state::opto_cb<1>));
	REEL(config, m_reel[2], UNIVERSAL_200STEP_REEL, 318, 320, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(crankycondor_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 3.57_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.2);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psccondor )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "crankycondor_main.bin", 0x0000, 0x2000, CRC(525217d8) SHA1(af8b6b00748e42d7fa5f9d20f274f1146430d8a5) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "crankycondor_music.bin", 0x0000, 0x2000, CRC(ceeede0d) SHA1(409daa1d9e85cead2a73ad05bd6b2a7bda50e8b3) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,      PARENT, MACHINE,      INPUT,        CLASS,              INIT,              MONITOR, COMPANY, FULLNAME,                                     FLAGS,                                      LAYOUT
GAMEL( 1995, psccondor, 0,      crankycondor, crankycondor, crankycondor_state, init_crankycondor, ROT0,    "Aruze", "Cranky Condor (4th Generation, Type A-400)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_crankycondor )
