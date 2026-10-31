// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Aruze / Universal Group Pachi-Slot Series Driver
    LE1080A(V1)  + 3-reel + 4th-reel + Z80 + YM2413 titles
//  LE2080(V2) or LE2280(V2) + 3-reel + 4th-reel + Z80 + RPP-16 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2025-04-02.

    Supported Games:
    - Oohanabi  (c) 1999 Aruze  (4th Generation, Type A-700)

    Hardware:
    - Main CPU: LE1080A-PA @ provisional 6.0 MHz (12.0 MHz XTAL / 2)
    - Sound CPU: Z80 @ 6.0 MHz (12.0 MHz XTAL / 2)
    - Sound: YM2413 @ 3.579545 MHz
    - Stepper Motors: 3-reel (200-step) + 1-reel (4th-reel/hachimaki, 200-step, 2-phase unipolar)
    - NVRAM: Internal LE1080A battery-backed RAM

    [Driver scope note]

    This source is intentionally kept as the family driver for
    Aruze / Universal-style 4th-reel pachislot hardware.

    At present, only confirmed LE1080A-based titles are included.
    Related V2-era titles using LE2080-class CPUs and
    Roland RPP-16 stereo sound hardware are known from external references,
    but are not included here until ROM data and exact board-level
    differences are confirmed.

    Representative examples associated with this broader hardware line
    include titles such as Tarot Master R, Delsol 2, and Conti 4X.

    TODO:
    - LE1080A interrupt addresses and internal random-number behavior are
      still not fully understood, so some model-specific behavior remains
      implemented in the driver rather than in le1080a.cpp.

    - IRQ0 / IRQ1 / power-fail preparation interrupt addresses differ by
      machine and manufacturer.
      These are currently assigned from analysis results found in each
      program rather than being derived automatically by the LE1080A core.

    - 4th reel timing and stop-position behavior are not yet fully
      understood.
      Full-cycle movement back to the same symbol appears to stay aligned
      in both left and right rotation, but movement from the current symbol
      to a different target symbol does not currently match observed
      behavior. The program appears to drive the 4th reel using a step
      distance from the current symbol to the requested symbol, but the
      reference position, phase, and/or opto-based origin used for that
      calculation have not been identified yet. The current 400-step reel
      model and 448 Hz IRQ0 timing are therefore provisional.

    NOTE:
    [LE-Tech LE1080A CPU & Interrupt Architecture]
    - For hardware specifications, system register details (0xFF00-0xFF3F),
      memory maps, and power-fail interrupt logic of the LE-Tech LE1080A
      Z80-compatible core, see the CPU core driver: cpu/z80/le1080a.cpp.

    [Periodic IRQ and Clock Pacing]
    - The main gameplay loop is serviced by LE1080A IRQ0 (448 Hz) and
      IRQ1 (512 Hz).
      (Provisional frequency calibrated against physical reel revolution period).
    - Pacing and reel motor timing align with verified gameplay behavior.

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, allowing the CPU to execute its
      state-saving routine into battery-backed NVRAM during the reservoir
      capacitance hold-up time.
    - Because MAME currently lacks a pre-reset / pre-shutdown callback
      mechanism to allow the CPU to gracefully complete its power-down
      sequence before machine_reset() or exit (e.g. F3 / ESC keys), this
      driver provisionally provides a manual 'P' key.
    - Pressing the 'P' key asserts the PFI signal and preserves the physical
      reel absolute positions via stepper_device helpers.
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
    - 4th reel control behavior is not yet fully understood.
      The current implementation is based on partial program analysis and
      remains incomplete pending further verification.
      Reel stop positions and symbol alignment may not match real hardware.

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
#include "oohanabi.lh"

class oohanabi_state : public driver_device
{
public:
	oohanabi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "le1080a")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_pachislot(*this, "pachislot")
		, m_ym2413(*this, "ym2413")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_lamp_frame_t(*this, "lamp_frame_t%u", 0U)
		, m_lay_lamp_frame_l_s(*this, "lamp_frame_sl%u", 0U)
		, m_lay_lamp_frame_r_s(*this, "lamp_frame_sr%u", 0U)
		, m_lay_lamp_win(*this, "lamp_win")
		, m_lay_lamp_replay(*this, "lamp_replay")
		, m_lay_lamp_wait(*this, "lamp_wait")
		, m_lay_lamp_chtime(*this, "lamp_chtime")
		, m_lay_lamp_insert(*this, "lamp_insert")
		, m_lay_lamp_start(*this, "lamp_start")
		, m_lay_lamp_maxbet(*this, "lamp_maxbet")
		, m_lay_lamp_credit(*this, "lamp_credit")
		, m_lay_lamp_winmeter(*this, "lamp_winmeter")
		, m_lay_lamp_gameover(*this, "lamp_gameover")
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_lamp_reel(*this, "lamp_reel%u_%u", 0U, 0U)
		, m_lay_led_chance(*this, "led_chance%u", 0U)
		, m_lay_led_counter(*this, "led_counter%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_seg_count(*this, "seg_count%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void oohanabi(machine_config &config) ATTR_COLD;

	void init_oohanabi();

	template <pachislot_device::DEVSENSOR_NUM N> int selector_r() { return m_pachislot->selector_r(N); }
	template <pachislot_device::DEVSENSOR_NUM N> int hopper_r() { return m_pachislot->hopper_r(N); }
	template <unsigned N> int in_fakebtn_r() { return BIT(m_in_fakebtn->read(), N); }
	template <unsigned N> int opto_r() { return m_opto[N]; }

	struct powerfail_snapshot
	{
		bool valid = false;
		int reel_abs_pos[4]{};
	};

	void powerfail_prepare_state();
	void powerfail_restore_state();
	INPUT_CHANGED_MEMBER(powerfail_trigger);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	template <unsigned N> void opto_cb(int state) { m_opto[N] = state; }

	void le1080a_mem_map(address_map &map) ATTR_COLD;
	void oohanabi_io_map(address_map &map) ATTR_COLD;
	void audio_mem_map(address_map &map) ATTR_COLD;

	void commit_lamp_buffers();

	TIMER_CALLBACK_MEMBER(lamp_buffer_commit_tick);

private:
	required_device<le1080a_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<pachislot_device> m_pachislot;
	required_device<ym2413_device> m_ym2413;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 4> m_reel;
	required_ioport_array<3> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<6> m_lay_lamp_frame_t;
	output_finder<3> m_lay_lamp_frame_l_s;
	output_finder<3> m_lay_lamp_frame_r_s;
	output_finder<> m_lay_lamp_win;
	output_finder<> m_lay_lamp_replay;
	output_finder<> m_lay_lamp_wait;
	output_finder<> m_lay_lamp_chtime;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_start;
	output_finder<> m_lay_lamp_maxbet;
	output_finder<> m_lay_lamp_credit;
	output_finder<> m_lay_lamp_winmeter;
	output_finder<> m_lay_lamp_gameover;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<4, 3> m_lay_lamp_reel;
	output_finder<2> m_lay_led_chance;
	output_finder<8> m_lay_led_counter;
	output_finder<2> m_lay_seg_credit;
	output_finder<3> m_lay_seg_payout;
	output_finder<3> m_lay_seg_count;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	template <unsigned N> u8 rng_r(offs_t offset);
	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	u8 switch_sense_c_r(offs_t offset);
	void drive_latch_a_w(offs_t offset, u8 data);
	void drive_latch_b_w(offs_t offset, u8 data);
	void drive_latch_c_w(offs_t offset, u8 data);
	void drive_latch_d_w(offs_t offset, u8 data);
	void reel_lamp_w(offs_t offset, u8 data);
	void lamp_latch_misc_w(offs_t offset, u8 data);
	void lamp_latch_stop_bet_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void seg_common_w(offs_t offset, u8 data);
	void lamp_latch_frame_w(offs_t offset, u8 data);
	void soundcode_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;
	bool m_powerfail_status_ready;

	emu_timer *m_lamp_buffer_commit_timer = nullptr;

	u8 m_opto[4];
	u8 m_segcommon;
	u8 m_segdata[7];
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;

	u8 m_segdata_pending;
	u8 m_reel_lamp_pending[4][3];
	u8 m_betline_lamp_pending[5];
	u8 m_seg_credit_pending[2];
	u8 m_seg_count_pending[3];
	u8 m_seg_payout_pending[2];
};


void oohanabi_state::init_oohanabi()
{
	// LE1080A Interrupt address set
	m_maincpu->set_irq0_address(0x0084);
	m_maincpu->set_irq1_address(0x0088);
	m_maincpu->set_irq2_address(0x009c);

	// Provisional frequency calibrated against physical reel revolution period.
	// Internal timer configuration register behavior is not yet fully documented.
	m_maincpu->set_irq0_rate_hz(448);	// 4th reel
	m_maincpu->set_irq1_rate_hz(512);	// main reel
}

void oohanabi_state::machine_start()
{
	m_lamp_buffer_commit_timer = timer_alloc(FUNC(oohanabi_state::lamp_buffer_commit_tick), this);

	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segcommon));
	save_item(NAME(m_segdata));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_segdata_pending));
	save_item(NAME(m_reel_lamp_pending));
	save_item(NAME(m_betline_lamp_pending));
	save_item(NAME(m_seg_credit_pending));
	save_item(NAME(m_seg_count_pending));
	save_item(NAME(m_seg_payout_pending));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void oohanabi_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segcommon = 0;
	m_segdata_pending = 0;

	m_lamp_buffer_commit_timer->adjust(attotime::from_hz(512), 0, attotime::from_hz(512));
	memset(m_reel_lamp_pending, 0, sizeof(m_reel_lamp_pending));
	memset(m_betline_lamp_pending, 0, sizeof(m_betline_lamp_pending));
	memset(m_seg_credit_pending, 0, sizeof(m_seg_credit_pending));
	memset(m_seg_count_pending, 0, sizeof(m_seg_count_pending));
	memset(m_seg_payout_pending, 0, sizeof(m_seg_payout_pending));
	commit_lamp_buffers();

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
void oohanabi_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 4; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
	m_powerfail_status_ready = true;
}

void oohanabi_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 4; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void oohanabi_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();

	// Flush pending atomic display/lamp buffers to prevent visual tearing
	commit_lamp_buffers();
}

void oohanabi_state::commit_lamp_buffers()
{
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 3; y++)
			m_lay_lamp_reel[x][y] = m_reel_lamp_pending[x][y];

	for (int i = 0; i < 5; i++)
		m_lay_lamp_betline[i] = m_betline_lamp_pending[i];

	for (int i = 0; i < 2; i++)
	{
		m_lay_seg_credit[i] = m_seg_credit_pending[i];
		m_lay_seg_payout[i] = m_seg_payout_pending[i];
	}

	for (int i = 0; i < 3; i++)
		m_lay_seg_count[i] = m_seg_count_pending[i];
}

TIMER_CALLBACK_MEMBER(oohanabi_state::lamp_buffer_commit_tick)
{
	commit_lamp_buffers();
}

INPUT_CHANGED_MEMBER(oohanabi_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_maincpu->trigger_power_fail();
	}
}

template <unsigned N>
u8 oohanabi_state::rng_r(offs_t offset)
{
	// bit0-7: Read once after lever ON and before reel rotation. Possibly LE1080A internal random source.
	return m_maincpu->internal_rng_r(N);
}

u8 oohanabi_state::switch_sense_a_r(offs_t offset)
{
	// bit0: reel 1 stop button (left)
	// bit1: reel 2 stop button (center)
	// bit2: reel 3 stop button (right)
	// bit3: bet 1 button
	// bit4: bet 2 button
	// bit5: maxbet button
	// bit6: payout / credit game button
	// bit7: start lever

	return m_in_port[0]->read();
}

u8 oohanabi_state::switch_sense_b_r(offs_t offset)
{
	// bit0: medal selector sensor 1 (CE)
	// bit1: medal selector sensor 2 (Cr)
	// bit2: reset / change setting value button
	// bit3: setting key
	// bit4: medal selector sensor 3 (CJ)
	// bit5: door key
	// bit6: subtank medal overflow sensor
	// bit7: hopper medal out sensor

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[1]->read();
}

u8 oohanabi_state::switch_sense_c_r(offs_t offset)
{
	// bit0: main reel 1 sensor (left)
	// bit1: main reel 2 sensor (center)
	// bit2: main reel 3 sensor (right)
	// bit3: always set to 1
	// bit4: "SEISAN" ("清算") switch (automatic payout)
	// bit5: "UCHIDOME" ("打ち止め") switch (automatic game-stop)
	// bit6: 4th reel sensor
	// bit7: unknown

	return m_in_port[2]->read();
}

void oohanabi_state::drive_latch_a_w(offs_t offset, u8 data)
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

void oohanabi_state::drive_latch_b_w(offs_t offset, u8 data)
{
	// bit0-3: reel 2 pulse bit (center)
	// bit4:   medal lockout solenoid
	// bit5:   unknown (possibly interrupt-related; written from the IRQ0 path?)
	// bit6-7: hopper motor drive

	// bit0-3: reel 2 pulse bit (center)
	m_reel[1]->write(data & 0x0f);

	// bit4: medal lockout solenoid
	m_solenoid = !BIT(data, 4);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	// bit6-7: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	if (BIT(data, 6) && !BIT(data, 7))
	{
		// hopper motor on (forward)
		m_hopper = 1;
		m_lay_signal_hopper = 1;
	}
	else if (BIT(data, 6) && BIT(data, 7))
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
}

void oohanabi_state::drive_latch_c_w(offs_t offset, u8 data)
{
	// bit0-3: reel 3 pulse bit (right)
	// bit4:   wait lamp
	// bit5-7: unknown

	// bit0-3: reel 3 pulse bit (right)
	m_reel[2]->write(data & 0x0f);

	// bit4: wait lamp
	m_lay_lamp_wait = BIT(data, 4);
}

void oohanabi_state::drive_latch_d_w(offs_t offset, u8 data)
{
	// bit0-3: 4th reel pulse bits
	// bit4:   reel lamp
	// bit5-7: 4th reel lamp

	// bit0-3: 4th reel pulse bits
	m_reel[3]->write(data & 0x0f);

	// bit4: reel lamp
	m_reel_lamp_pending[2][2] = BIT(data, 4);	// x:2, y:2

	// bit5-7: 4th reel lamp
	m_reel_lamp_pending[3][0] = BIT(data, 5);
	m_reel_lamp_pending[3][1] = BIT(data, 6);
	m_reel_lamp_pending[3][2] = BIT(data, 7);
}

void oohanabi_state::reel_lamp_w(offs_t offset, u8 data)
{
	// bit0-7: reel lamp
	m_reel_lamp_pending[0][0] = BIT(data, 0);	// x:0, y:0
	m_reel_lamp_pending[1][0] = BIT(data, 1);	// x:1, y:0
	m_reel_lamp_pending[2][0] = BIT(data, 2);	// x:2, y:0
	m_reel_lamp_pending[0][1] = BIT(data, 3);	// x:0, y:1
	m_reel_lamp_pending[1][1] = BIT(data, 4);	// x:1, y:1
	m_reel_lamp_pending[2][1] = BIT(data, 5);	// x:2, y:1
	m_reel_lamp_pending[0][2] = BIT(data, 6);	// x:0, y:2
	m_reel_lamp_pending[1][2] = BIT(data, 7);	// x:1, y:2
}

void oohanabi_state::lamp_latch_misc_w(offs_t offset, u8 data)
{
	// bit0:   unknown
	// bit1:   chance LED
	// bit2:   maxbet button lamp
	// bit3:   replay lamp
	// bit4:   win lamp
	// bit5:   unknown
	// bit6:   start lamp
	// bit7:   insert medal lamp

	// bit1: chance LED
	m_lay_led_chance[0] = BIT(data, 1);			// chance LED 1 (left)
	m_lay_led_chance[1] = BIT(data, 1);			// chance LED 2 (right)

	// bit2: maxbet button lamp
	m_lay_lamp_maxbet = BIT(data, 2);

	// bit3: replay lamp
	m_lay_lamp_replay = BIT(data, 3);

	// bit4: win lamp
	m_lay_lamp_win = BIT(data, 4);

	// bit6: start lamp
	m_lay_lamp_start = BIT(data, 6);

	// bit7: insert medal lamp
	m_lay_lamp_insert = BIT(data, 7);
}

void oohanabi_state::lamp_latch_stop_bet_w(offs_t offset, u8 data)
{
	// bit0:   reel 1 stop button lamp (left)
	// bit1:   reel 2 stop button lamp (center)
	// bit2:   reel 3 stop button lamp (right)
	// bit3-7: bet medal(s) line lamp (physical upper/lower pairs)

	// bit0-2: reel stop button lamp
	m_lay_lamp_stop[0] = BIT(data, 0);			// (left)
	m_lay_lamp_stop[1] = BIT(data, 1);			// (center)
	m_lay_lamp_stop[2] = BIT(data, 2);			// (right)

	// bit3-7: bet medal(s) line lamp (physical upper/lower pairs)
	for (int i = 0; i < 5; i++)
		m_betline_lamp_pending[i] = BIT(data, i + 3);
}

void oohanabi_state::seg_data_w(offs_t offset, u8 data)
{
	// bit0-7: SEG data pattern
	m_segdata_pending = data;
}

void oohanabi_state::seg_common_w(offs_t offset, u8 data)
{
	// bit0-7: SEG common bit
	for (u8 i = 0; i < 7; i++)
		if (BIT(data, i))
			m_segdata[i] = bitswap<8>(m_segdata_pending, 0,1,2,3,4,5,6,7);

	// SEG credit
	m_seg_credit_pending[0] = m_segdata[3];		// SEG left
	m_seg_credit_pending[1] = m_segdata[2];		// SEG right

	// SEG game count
	m_seg_count_pending[0]  = m_segdata[6];		// SEG left
	m_seg_count_pending[1]  = m_segdata[5];		// SEG center
	m_seg_count_pending[2]  = m_segdata[4];		// SEG right

	// SEG payout
	m_seg_payout_pending[0] = m_segdata[1];		// SEG left
	m_seg_payout_pending[1] = m_segdata[0];		// SEG right
}

void oohanabi_state::lamp_latch_frame_w(offs_t offset, u8 data)
{
	// bit0-4: bonus 4th reel top lamp
	// bit5-7: bonus 4th reel side lamp

	// bit0-4: bonus 4th reel top lamp
	m_lay_lamp_frame_t[0] = BIT(data, 0);		// left
	m_lay_lamp_frame_t[1] = BIT(data, 1);
	m_lay_lamp_frame_t[2] = BIT(data, 2);		// center
	m_lay_lamp_frame_t[3] = BIT(data, 3);
	m_lay_lamp_frame_t[4] = BIT(data, 4);		// right

	// bit5-7: bonus 4th reel side lamp
	m_lay_lamp_frame_l_s[0] = m_lay_lamp_frame_r_s[0] = BIT(data, 7);	// green
	m_lay_lamp_frame_l_s[1] = m_lay_lamp_frame_r_s[1] = BIT(data, 6);	// yellow
	m_lay_lamp_frame_l_s[2] = m_lay_lamp_frame_r_s[2] = BIT(data, 5);	// red
}

void oohanabi_state::soundcode_w(offs_t offset, u8 data)
{
	// bit0-7: soundcode write
	m_soundlatch->write(data);
}

void oohanabi_state::le1080a_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}

void oohanabi_state::oohanabi_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(oohanabi_state::drive_latch_a_w));
	map(0x01, 0x01).w(FUNC(oohanabi_state::drive_latch_b_w));
	map(0x02, 0x02).w(FUNC(oohanabi_state::drive_latch_c_w));
	map(0x03, 0x03).w(FUNC(oohanabi_state::drive_latch_d_w));
	map(0x04, 0x04).w(FUNC(oohanabi_state::reel_lamp_w));
	map(0x05, 0x05).w(FUNC(oohanabi_state::lamp_latch_misc_w));
	map(0x06, 0x06).w(FUNC(oohanabi_state::lamp_latch_stop_bet_w));
	map(0x07, 0x07).w(FUNC(oohanabi_state::seg_data_w));
	map(0x08, 0x08).w(FUNC(oohanabi_state::seg_common_w));
	map(0x09, 0x09).w(FUNC(oohanabi_state::lamp_latch_frame_w));
	map(0x0a, 0x0a).w(FUNC(oohanabi_state::soundcode_w));
	map(0x10, 0x10).r(FUNC(oohanabi_state::rng_r<0>));
	map(0x11, 0x11).r(FUNC(oohanabi_state::rng_r<1>));
	map(0x12, 0x12).r(FUNC(oohanabi_state::rng_r<2>));
	map(0x13, 0x13).r(FUNC(oohanabi_state::switch_sense_a_r));
	map(0x14, 0x14).r(FUNC(oohanabi_state::switch_sense_b_r));
	map(0x15, 0x15).r(FUNC(oohanabi_state::switch_sense_c_r));
}

void oohanabi_state::audio_mem_map(address_map &map)
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


INPUT_PORTS_START( oohanabi )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_NAME("Bet 1 Button")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Bet 2 Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Max Bet Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(oohanabi_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(oohanabi_state::selector_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal selector sensor 1
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Door Key / Reset Button / Change Setting Value Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(oohanabi_state::selector_r<pachislot_device::PACHISLOT_SENSOR2>))	// medal selector sensor 2
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Door Key")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("Subtank Overflow")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(oohanabi_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0

	// Cabinet option switches and other inputs
	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(oohanabi_state::opto_r<0>))	// main reel 0 sensor
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(oohanabi_state::opto_r<1>))	// main reel 1 sensor
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(oohanabi_state::opto_r<2>))	// main reel 2 sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )	// Always set to High level.
	PORT_CONFNAME( 0x10, 0x00, "UCHIDOME" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x10, DEF_STR( On ) )
	PORT_CONFNAME( 0x20, 0x00, "SEISAN" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(oohanabi_state::opto_r<3>))	// 4th reel sensor
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(oohanabi_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void oohanabi_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void oohanabi_state::oohanabi(machine_config &config)
{
	// basic machine hardware
	LE1080A(config, m_maincpu, XTAL(12'000'000) / 2);	// LE1080A-PA 12 MHz / 2 = 6 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &oohanabi_state::le1080a_mem_map);
	m_maincpu->set_addrmap(AS_IO, &oohanabi_state::oohanabi_io_map);

	Z80(config, m_audiocpu, XTAL(12'000'000) / 2);		// 12 MHz / 2 = 6 MHz ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &oohanabi_state::audio_mem_map);
	// Audio CPU interrupt frequency is currently estimated from observed
	// sound playback timing and overall behavior, not from measured hardware.
	// It may require adjustment when more accurate information becomes available.
	m_audiocpu->set_periodic_int(FUNC(oohanabi_state::irq0_line_hold), attotime::from_hz(750));

	GENERIC_LATCH_8(config, m_soundlatch);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(50 * 2);
	m_pachislot->hopper_cb().set(FUNC(oohanabi_state::playing_hopper_samples));

	// Configure sensor emulation modes: Aruze 3-sensor coin selector and 2-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_UNIVERSAL1);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_UNIVERSAL1);

	// reel device
	REEL(config, m_reel[0], UNIVERSAL_200STEP_REEL, 318, 320, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(oohanabi_state::opto_cb<0>));
	REEL(config, m_reel[1], UNIVERSAL_200STEP_REEL, 318, 320, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(oohanabi_state::opto_cb<1>));
	REEL(config, m_reel[2], UNIVERSAL_200STEP_REEL, 318, 320, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(oohanabi_state::opto_cb<2>));

	// reel device(4th)
	static constexpr int fourth_reel_index = ((400 / 20) * 11) + 15;
	REEL(config, m_reel[3], UNIVERSAL_200STEP_REEL, fourth_reel_index, fourth_reel_index + 200, 0x00, 0, 200*2);
	m_reel[3]->optic_handler().set(FUNC(oohanabi_state::opto_cb<3>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 3.57_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.2);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psoohanabi )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "oohanabi_main.bin", 0x0000, 0x2000, CRC(4afdba43) SHA1(42c7143c3ff6feaf220adede2ae2dc9fe8d799c6) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "oohanabi_music.bin", 0x0000, 0x2000, CRC(2447cc2c) SHA1(6adc12aa51bc33630060b224b19a4e56dced54d7) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,  INPUT,     CLASS,           INIT,           MONITOR, COMPANY, FULLNAME,                                 FLAGS,                                                              LAYOUT
GAMEL( 1999, psoohanabi, 0,      oohanabi, oohanabi,  oohanabi_state,  init_oohanabi,  0,       "Aruze", "Oohanabi (4th Generation, Type A-700)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL|MACHINE_IS_INCOMPLETE), layout_oohanabi )
