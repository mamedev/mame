// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Universal / Aruze Group Pachi-Slot Series Driver
    LE1080A(V1) + 3-reel + Z80 + YM2413 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2025-03-03.

    Supported Games:
    - CC Angel        (c) 1996 Macy       (4th Generation, Type A)
    - Thunder V       (c) 1997 Macy       (4th Generation, Type A)
    - Cranky Contest  (c) 1998 Universal  (4th Generation, Type A)
    - Hanabi          (c) 1998 Aruze      (4th Generation, Type A)
    - B-Max           (c) 1998 Aruze      (4th Generation, Type A-600)

    Hardware:
    - Main CPU: LE1080A-PA @ provisional 6.0 MHz (12.0 MHz XTAL / 2)
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
#include "ccangel.lh"
#include "thunderv.lh"
#include "crankycontest.lh"
#include "hanabi.lh"
#include "bmax.lh"

class thunderv_state : public driver_device
{
public:
	thunderv_state(const machine_config &mconfig, device_type type, const char *tag)
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
		, m_lay_lamp_win(*this, "lamp_win")
		, m_lay_lamp_replay(*this, "lamp_replay")
		, m_lay_lamp_wait(*this, "lamp_wait")
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
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_seg_count(*this, "seg_count%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void common(machine_config &config) ATTR_COLD;
	void thunderv(machine_config &config) ATTR_COLD;

	void init_common();

	template <pachislot_device::DEVSENSOR_NUM N> int selector_r() { return m_pachislot->selector_r(N); }
	template <pachislot_device::DEVSENSOR_NUM N> int hopper_r() { return m_pachislot->hopper_r(N); }
	template <unsigned N> int in_fakebtn_r() { return BIT(m_in_fakebtn->read(), N); }
	template <unsigned N> int opto_r() { return m_opto[N]; }

	struct powerfail_snapshot
	{
		bool valid = false;
		int reel_abs_pos[3]{};
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
	void audio_mem_map(address_map &map) ATTR_COLD;

	void common_io_map(address_map &map) ATTR_COLD;
	void thunderv_io_map(address_map &map) ATTR_COLD;

	virtual void commit_lamp_buffers();

	TIMER_CALLBACK_MEMBER(reel_backlight_commit_tick);

	template <unsigned N> u8 rng_r(offs_t offset);
	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	u8 switch_sense_c_r(offs_t offset);

	void drive_latch_a_w(offs_t offset, u8 data);
	void drive_latch_b_w(offs_t offset, u8 data);
	void drive_latch_c_w(offs_t offset, u8 data);
	void lamp_latch_stop_bet_w(offs_t offset, u8 data);
	void lamp_latch_reel_start_w(offs_t offset, u8 data);
	void lamp_latch_misc_w(offs_t offset, u8 data);
	void lamp_latch_frame_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void seg_common_w(offs_t offset, u8 data);
	void soundcode_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;
	bool m_powerfail_status_ready;

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
	output_finder<> m_lay_lamp_win;
	output_finder<> m_lay_lamp_replay;
	output_finder<> m_lay_lamp_wait;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_start;
	output_finder<> m_lay_lamp_maxbet;
	output_finder<> m_lay_lamp_credit;
	output_finder<> m_lay_lamp_winmeter;
	output_finder<> m_lay_lamp_gameover;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<3, 3> m_lay_lamp_reel;
	output_finder<2> m_lay_led_chance;
	output_finder<2> m_lay_seg_credit;
	output_finder<3> m_lay_seg_payout;
	output_finder<3> m_lay_seg_count;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	emu_timer *m_reel_backlight_commit_timer = nullptr;

	u8 m_segcommon;
	u8 m_segdata[7];
	u8 m_opto[3];
	u8 m_leddata_red;
	u8 m_leddata_green;
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;

	u8 m_segdata_pending;
	u8 m_segdata_setting_pending;
	u8 m_reel_lamp_pending[3][3];
	u8 m_betline_lamp_pending[5];
	u8 m_seg_credit_pending[2];
	u8 m_seg_count_pending[3];
	u8 m_seg_payout_pending[2];
};

class bmax_state : public thunderv_state
{
public:
	bmax_state(const machine_config &mconfig, device_type type, const char *tag)
		: thunderv_state(mconfig, type, tag)
	{ }

	void bmax(machine_config &config) ATTR_COLD;

protected:
	void bmax_io_map(address_map &map) ATTR_COLD;

	u8 switch_sense_b_r(offs_t offset);
	u8 switch_sense_c_r(offs_t offset);
};

class ccangel_state : public thunderv_state
{
public:
	ccangel_state(const machine_config &mconfig, device_type type, const char *tag)
		: thunderv_state(mconfig, type, tag)
		, m_lay_seg_gcount(*this, "seg_gcount%u", 0U)
		, m_lay_seg_bcount(*this, "seg_bcount%u", 0U)
	{ }

	void ccangel(machine_config &config) ATTR_COLD;

protected:
	void ccangel_io_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void commit_lamp_buffers() override;

	void seg_data_a_w(offs_t offset, u8 data);
	void seg_data_b_w(offs_t offset, u8 data);
	void seg_common_w(offs_t offset, u8 data);
	void soundcode_w(offs_t offset, u8 data);

private:
	output_finder<2> m_lay_seg_gcount;
	output_finder<2> m_lay_seg_bcount;

	u8 m_seg_common_old;
	u8 m_segdata[2][8];
	u8 m_segdata_pending[2];
	u8 m_seg_gcount_pending[2];
	u8 m_seg_bcount_pending[2];
};


void thunderv_state::init_common()
{
	// LE1080A Interrupt address set
	m_maincpu->set_irq0_address(0x0084);
	m_maincpu->set_irq1_address(0x0088);
	m_maincpu->set_irq2_address(0x009c);

	// Provisional frequency calibrated against physical reel revolution period.
	// Internal timer configuration register behavior is not yet fully documented.
	m_maincpu->set_irq0_rate_hz(448);
	m_maincpu->set_irq1_rate_hz(512);
}

void thunderv_state::machine_start()
{
	m_reel_backlight_commit_timer = timer_alloc(FUNC(thunderv_state::reel_backlight_commit_tick), this);

	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segcommon));
	save_item(NAME(m_segdata));
	save_item(NAME(m_leddata_red));
	save_item(NAME(m_leddata_green));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_segdata_pending));
	save_item(NAME(m_segdata_setting_pending));
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

void ccangel_state::machine_start()
{
	thunderv_state::machine_start();

	save_item(NAME(m_seg_gcount_pending));
	save_item(NAME(m_seg_bcount_pending));
}

void thunderv_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segcommon = 0;
	m_leddata_red = 0;
	m_leddata_green = 0;
	m_segdata_pending = 0;
	m_segdata_setting_pending = 0;

	m_reel_backlight_commit_timer->adjust(attotime::from_hz(512), 0, attotime::from_hz(512));

	memset(m_reel_lamp_pending, 0, sizeof(m_reel_lamp_pending));
	memset(m_betline_lamp_pending, 0, sizeof(m_betline_lamp_pending));
	memset(m_seg_credit_pending, 0, sizeof(m_seg_credit_pending));
	memset(m_seg_count_pending, 0, sizeof(m_seg_count_pending));
	memset(m_seg_payout_pending, 0, sizeof(m_seg_payout_pending));

	commit_lamp_buffers();

	powerfail_restore_state();
	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;

	m_pachislot->reset_selector();
	m_pachislot->reset_hopper();
}

void ccangel_state::machine_reset()
{
	memset(m_seg_gcount_pending, 0, sizeof(m_seg_gcount_pending));
	memset(m_seg_bcount_pending, 0, sizeof(m_seg_bcount_pending));
	memset(m_segdata, 0, sizeof(m_segdata));
	memset(m_segdata_pending, 0, sizeof(m_segdata_pending));

	m_seg_common_old = 0;

	thunderv_state::machine_reset();
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
void thunderv_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
	m_powerfail_status_ready = true;
}

void thunderv_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void thunderv_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();

	// Flush pending atomic display/lamp buffers to prevent visual tearing
	commit_lamp_buffers();
}

void thunderv_state::commit_lamp_buffers()
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

	for (int i = 0; i < 3; i++)
		m_lay_seg_count[i] = m_seg_count_pending[i];
}

void ccangel_state::commit_lamp_buffers()
{
	thunderv_state::commit_lamp_buffers();

	for (int i = 0; i < 2; i++)
	{
		m_lay_seg_gcount[i] = m_seg_gcount_pending[i];
		m_lay_seg_bcount[i] = m_seg_bcount_pending[i];
	}
}

TIMER_CALLBACK_MEMBER(thunderv_state::reel_backlight_commit_tick)
{
	commit_lamp_buffers();
}

INPUT_CHANGED_MEMBER(thunderv_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_maincpu->trigger_power_fail();
	}
}

template <unsigned N>
u8 thunderv_state::rng_r(offs_t offset)
{
	// bit0-7: Read once after lever ON and before reel rotation. Possibly LE1080A internal random source.
	return m_maincpu->internal_rng_r(N);
}

u8 thunderv_state::switch_sense_a_r(offs_t offset)
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

u8 thunderv_state::switch_sense_b_r(offs_t offset)
{
	// bit0: medal selector sensor 1 (CE)
	// bit1: medal selector sensor 2 (Cr)
	// bit2: reset / change setting value button
	// bit3: setting key
	// bit4: medal selector sensor 3 (CJ)
	// bit5: door key
	// bit6: "SEISAN" ("清算") switch (automatic payout)
	// bit7: "UCHIDOME" ("打ち止め") switch (automatic game-stop)

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[1]->read();
}

u8 thunderv_state::switch_sense_c_r(offs_t offset)
{
	// bit0: reel 1 sensor (left)
	// bit1: reel 2 sensor (center)
	// bit2: reel 3 sensor (right)
	// bit3: Always reads high
	// bit4: subtank medal overflow sensor
	// bit5: Hopper medal out test (for maintenance?)
	// bit6: hopper sensor 1 (Ho)
	// bit7: hopper sensor 2 (Ho/HJ)
	return m_in_port[2]->read();
}

u8 bmax_state::switch_sense_b_r(offs_t offset)
{
	// bit0: medal selector sensor 1 (CE)
	// bit1: medal selector sensor 2 (Cr)
	// bit2: reset / change setting value button
	// bit3: setting key
	// bit4: medal selector sensor 3 (CJ)
	// bit5: door key
	// bit6: subtank medal overflow sensor
	// bit7: hopper sensor (Ho/HJ)

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[1]->read();
}

u8 bmax_state::switch_sense_c_r(offs_t offset)
{
	// bit0:   reel 1 sensor (left)
	// bit1:   reel 2 sensor (center)
	// bit2:   reel 3 sensor (right)
	// bit3:   Always reads high
	// bit4-7: unknown
	return m_in_port[2]->read();
}

void thunderv_state::drive_latch_a_w(offs_t offset, u8 data)
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

void thunderv_state::drive_latch_b_w(offs_t offset, u8 data)
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

void thunderv_state::drive_latch_c_w(offs_t offset, u8 data)
{
	// bit0-3: reel 3 pulse bit (right)
	// bit4:   wait lamp
	// bit5-7: unknown

	// bit0-3: reel 3 pulse bit (right)
	m_reel[2]->write(data & 0x0f);

	// bit4: wait lamp
	m_lay_lamp_wait = BIT(data, 4);
}

void thunderv_state::lamp_latch_stop_bet_w(offs_t offset, u8 data)
{
	// bit0:   reel 1 stop button lamp (left)
	// bit1:   reel 2 stop button lamp (center)
	// bit2:   reel 3 stop button lamp (right)
	// bit3-7: bet medal(s) line lamp (physical upper/lower pairs)

	m_lay_lamp_stop[0] = BIT(data, 0);
	m_lay_lamp_stop[1] = BIT(data, 1);
	m_lay_lamp_stop[2] = BIT(data, 2);

	for (int i = 0; i < 5; i++)
		m_betline_lamp_pending[i] = BIT(data, i + 3);
}

void thunderv_state::lamp_latch_reel_start_w(offs_t offset, u8 data)
{
	// bit0-5: reel lamp
	// bit6:   start lamp
	// bit7:   insert medal lamp

	m_reel_lamp_pending[0][0] = BIT(data, 0);
	m_reel_lamp_pending[1][0] = BIT(data, 1);
	m_reel_lamp_pending[2][0] = BIT(data, 2);
	m_reel_lamp_pending[0][1] = BIT(data, 3);
	m_reel_lamp_pending[1][1] = BIT(data, 4);
	m_reel_lamp_pending[2][1] = BIT(data, 5);

	m_lay_lamp_start = BIT(data, 6);
	m_lay_lamp_insert = BIT(data, 7);
}

void thunderv_state::lamp_latch_misc_w(offs_t offset, u8 data)
{
	// bit0:   unknown
	// bit1:   chance LED
	// bit2:   maxbet button lamp
	// bit3:   win lamp (payout lamp)
	// bit4:   replay lamp
	// bit5-7: reel lamp

	m_lay_led_chance[0] = BIT(data, 1);
	m_lay_led_chance[1] = BIT(data, 1);

	m_lay_lamp_maxbet = BIT(data, 2);
	m_lay_lamp_win = BIT(data, 3);
	m_lay_lamp_replay = BIT(data, 4);

	m_reel_lamp_pending[0][2] = BIT(data, 5);
	m_reel_lamp_pending[1][2] = BIT(data, 6);
	m_reel_lamp_pending[2][2] = BIT(data, 7);
}

void thunderv_state::lamp_latch_frame_w(offs_t offset, u8 data)
{
	// bit0-7: frame lamps 1-8
	for (int i = 0; i < 8; i++)
		m_lay_lamp_frame[i] = BIT(data, i);
}

void thunderv_state::seg_data_w(offs_t offset, u8 data)
{
	// bit0-7: SEG data pattern
	m_segdata_pending = data;
}

void thunderv_state::seg_common_w(offs_t offset, u8 data)
{
	// bit0-7: SEG common bit
	for (u8 i = 0; i < 7; i++)
		if (BIT(data, i))
			m_segdata[i] = bitswap<8>(m_segdata_pending, 0,1,2,3,4,5,6,7);

	// SEG credit
	m_seg_credit_pending[0] = m_segdata[3];
	m_seg_credit_pending[1] = m_segdata[2];
	// SEG game count
	m_seg_count_pending[0]  = m_segdata[6];
	m_seg_count_pending[1]  = m_segdata[5];
	m_seg_count_pending[2]  = m_segdata[4];
	// SEG payout
	m_seg_payout_pending[0] = m_segdata[1];
	m_seg_payout_pending[1] = m_segdata[0];
}

void thunderv_state::soundcode_w(offs_t offset, u8 data)
{
	// bit0-7: soundcode write
	m_soundlatch->write(data);
}

void ccangel_state::seg_data_a_w(offs_t offset, u8 data)
{
	// bit0-7: SEG data (PAYOUT, SETTING)
	m_segdata_pending[0] = data;
}

void ccangel_state::seg_data_b_w(offs_t offset, u8 data)
{
	// bit0-7: SEG data (CREDIT)
	m_segdata_pending[1] = data;
}

void ccangel_state::seg_common_w(offs_t offset, u8 data)
{
	if ((!m_seg_common_old) && (!data))
	{
		m_segdata[1][7] = 0;
		m_segdata[1][5] = 0;
		m_segdata[0][7] = 0;
		m_segdata[0][5] = 0;
	}

	for (u8 i = 0; i < 8; i++)
	{
		if ((data != 0) && BIT(data, i))
		{
			m_segdata[0][i] = bitswap<8>(m_segdata_pending[0], 0,1,2,3,4,5,6,7);
			m_segdata[1][i] = bitswap<8>(m_segdata_pending[1], 0,1,2,3,4,5,6,7);
		}
	}

	m_seg_credit_pending[0] = m_segdata[1][3];
	m_seg_credit_pending[1] = m_segdata[1][2];
	m_seg_payout_pending[0] = m_segdata[0][1];
	m_seg_payout_pending[1] = m_segdata[0][0];
	m_seg_gcount_pending[0]  = m_segdata[1][7];
	m_seg_gcount_pending[1]  = m_segdata[1][5];
	m_seg_bcount_pending[0]  = m_segdata[0][7];
	m_seg_bcount_pending[1]  = m_segdata[0][5];

	m_seg_common_old = data;
}

void ccangel_state::soundcode_w(offs_t offset, u8 data)
{
	// bit0-7: soundcode write
	m_soundlatch->write(data);
}

void thunderv_state::le1080a_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}

void thunderv_state::common_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(thunderv_state::drive_latch_a_w));
	map(0x01, 0x01).w(FUNC(thunderv_state::drive_latch_b_w));
	map(0x02, 0x02).w(FUNC(thunderv_state::drive_latch_c_w));
	map(0x03, 0x03).w(FUNC(thunderv_state::lamp_latch_stop_bet_w));
	map(0x04, 0x04).w(FUNC(thunderv_state::lamp_latch_reel_start_w));
	map(0x05, 0x05).w(FUNC(thunderv_state::lamp_latch_misc_w));
	map(0x06, 0x06).w(FUNC(thunderv_state::lamp_latch_frame_w));
	map(0x10, 0x10).r(FUNC(thunderv_state::rng_r<0>));
	map(0x11, 0x11).r(FUNC(thunderv_state::rng_r<1>));
	map(0x12, 0x12).r(FUNC(thunderv_state::switch_sense_a_r));
}

void thunderv_state::thunderv_io_map(address_map &map)
{
	common_io_map(map);

	map(0x07, 0x07).w(FUNC(thunderv_state::seg_data_w));
	map(0x08, 0x08).w(FUNC(thunderv_state::seg_common_w));
	map(0x09, 0x09).w(FUNC(thunderv_state::soundcode_w));
	map(0x13, 0x13).r(FUNC(thunderv_state::switch_sense_b_r));
	map(0x14, 0x14).r(FUNC(thunderv_state::switch_sense_c_r));
}

void bmax_state::bmax_io_map(address_map &map)
{
	common_io_map(map);

	map(0x07, 0x07).w(FUNC(bmax_state::seg_data_w));
	map(0x08, 0x08).w(FUNC(bmax_state::seg_common_w));
	map(0x09, 0x09).w(FUNC(bmax_state::soundcode_w));
	map(0x13, 0x13).r(FUNC(bmax_state::switch_sense_b_r));
	map(0x14, 0x14).r(FUNC(bmax_state::switch_sense_c_r));
}

void ccangel_state::ccangel_io_map(address_map &map)
{
	common_io_map(map);

	map(0x07, 0x07).w(FUNC(ccangel_state::seg_data_a_w));
	map(0x08, 0x08).w(FUNC(ccangel_state::seg_data_b_w));
	map(0x09, 0x09).w(FUNC(ccangel_state::seg_common_w));
	map(0x0a, 0x0a).w(FUNC(ccangel_state::soundcode_w));
	map(0x13, 0x13).r(FUNC(ccangel_state::switch_sense_b_r));
	map(0x14, 0x14).r(FUNC(ccangel_state::switch_sense_c_r));
}

void thunderv_state::audio_mem_map(address_map &map)
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


static INPUT_PORTS_START( common )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("Bet 1 Button")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Bet 2 Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Max Bet Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")

	// Cabinet option switches and other inputs
	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::selector_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal selector sensor 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Door Key / Reset Button / Change Setting Value Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::selector_r<pachislot_device::PACHISLOT_SENSOR2>))	// medal selector sensor 3
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Door Key")
	PORT_CONFNAME( 0x40, 0x00, "UCHIDOME" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x40, DEF_STR( On ) )
	PORT_CONFNAME( 0x80, 0x00, "SEISAN" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(thunderv_state::opto_r<0>))	// reel 1 sensor
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(thunderv_state::opto_r<1>))	// reel 2 sensor
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(thunderv_state::opto_r<2>))	// reel 3 sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )	// Always set to High level.
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("Subtank Overflow")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )	// Hopper medal out test (for maintenance?)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::hopper_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal hopper sensor 1
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(thunderv_state::powerfail_trigger), 0)

INPUT_PORTS_END

static INPUT_PORTS_START( thunderv )

	PORT_INCLUDE( common )

INPUT_PORTS_END

static INPUT_PORTS_START( bmax )

	PORT_INCLUDE( common )

	PORT_MODIFY("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::selector_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal selector sensor 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Door Key / Reset Button / Change Setting Value Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::selector_r<pachislot_device::PACHISLOT_SENSOR2>))	// medal selector sensor 3
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Door Key")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("Subtank Overflow")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(thunderv_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0

	// Cabinet option switches and other inputs
	PORT_MODIFY("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(thunderv_state::opto_r<0>))	// reel 1 sensor
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(thunderv_state::opto_r<1>))	// reel 2 sensor
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(thunderv_state::opto_r<2>))	// reel 3 sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )	// Always set to High level.
	PORT_CONFNAME( 0x10, 0x00, "UCHIDOME" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x10, DEF_STR( On ) )
	PORT_CONFNAME( 0x20, 0x00, "SEISAN" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNKNOWN )

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void thunderv_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void thunderv_state::common(machine_config &config)
{
	// basic machine hardware
	LE1080A(config, m_maincpu, XTAL(12'000'000) / 2);	// LE1080A-PA 12 MHz / 2 = 6 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &thunderv_state::le1080a_mem_map);

	Z80(config, m_audiocpu, XTAL(12'000'000) / 2);		// 12 MHz / 2 = 6 MHz ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &thunderv_state::audio_mem_map);
	// Audio CPU interrupt frequency is currently estimated from observed
	// sound playback timing and overall behavior, not from measured hardware.
	// It may require adjustment when more accurate information becomes available.
	m_audiocpu->set_periodic_int(FUNC(thunderv_state::irq0_line_hold), attotime::from_hz(750));

	GENERIC_LATCH_8(config, m_soundlatch);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// Configure sensor emulation modes: Aruze 3-sensor coin selector and 2-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_UNIVERSAL1);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_UNIVERSAL1);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(50*2);
	m_pachislot->hopper_cb().set(FUNC(thunderv_state::playing_hopper_samples));

	REEL(config, m_reel[0], UNIVERSAL_200STEP_REEL, 319, 321, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(thunderv_state::opto_cb<0>));
	REEL(config, m_reel[1], UNIVERSAL_200STEP_REEL, 319, 321, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(thunderv_state::opto_cb<1>));
	REEL(config, m_reel[2], UNIVERSAL_200STEP_REEL, 319, 321, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(thunderv_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 3.57_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.2);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}

void thunderv_state::thunderv(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &thunderv_state::thunderv_io_map);
}

void bmax_state::bmax(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &bmax_state::bmax_io_map);
}

void ccangel_state::ccangel(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_IO, &ccangel_state::ccangel_io_map);
}


ROM_START( psccangel )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "ccangel_main.bin", 0x0000, 0x2000, CRC(839605e4) SHA1(e02170cac1f7cd2a303db20cc01eb5d5beb31985) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "ccangel_music.bin", 0x0000, 0x2000, CRC(3c6a031c) SHA1(2b22c6d7953988906f02e4374132d141892d5791) )
ROM_END

ROM_START( psthunderv )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "thunderv_main.bin", 0x0000, 0x2000, CRC(2242c9d6) SHA1(8b5238e9ff57c47998f873dce37281a9dc0512ec) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "thunderv_music.bin", 0x0000, 0x2000, CRC(00b9f00c) SHA1(472aca09113efc11934ce94c68373fe1c15c9a5b) )
ROM_END

ROM_START( pscraconte )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "crankycontest_main.bin", 0x0000, 0x2000, CRC(e336a140) SHA1(132c6c36c548944ab497516f7d9e3488a3a26e43) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "crankycontest_music.bin", 0x0000, 0x2000, CRC(af7af60c) SHA1(40840030826089299de0ad1eb29ba1dfb7bb6a1c) )
ROM_END

ROM_START( pshanabi )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "hanabi_main.bin", 0x0000, 0x2000, CRC(306e0ae3) SHA1(e4b1fe853dd17de4d56d331cc63a51951d49bc1a) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hanabi_music.bin", 0x0000, 0x2000, CRC(498b7e8e) SHA1(2584d6aeacf796d1717bd25276a11eb29f7d5751) )
ROM_END

ROM_START( psbmax )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "bmax_main.bin", 0x0000, 0x2000, CRC(ce38750f) SHA1(ef56129d4e06ae13a392fa5c5745eb1372890be7) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bmax_music.bin", 0x0000, 0x2000, CRC(4efe1b79) SHA1(1aef42cb8a08c93ec5ba9827de7c6a9216c6b454) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,  INPUT,    CLASS,          INIT,        MONITOR, COMPANY,     FULLNAME,                                  FLAGS,                                      LAYOUT
GAMEL( 1996, psccangel,  0,      ccangel,  thunderv, ccangel_state,  init_common, 0,       "Macy",      "CC Angel (4th Generation, Type A)",       (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_ccangel )
GAMEL( 1997, psthunderv, 0,      thunderv, thunderv, thunderv_state, init_common, 0,       "Macy",      "Thunder V (4th Generation, Type A)",      (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_thunderv )
GAMEL( 1998, pscraconte, 0,      thunderv, thunderv, thunderv_state, init_common, 0,       "Universal", "Cranky Contest (4th Generation, Type A)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_crankycontest )
GAMEL( 1998, pshanabi,   0,      thunderv, thunderv, thunderv_state, init_common, 0,       "Aruze",     "Hanabi (4th Generation, Type A)",         (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_hanabi )
GAMEL( 1998, psbmax,     0,      bmax,     bmax,     bmax_state,     init_common, 0,       "Aruze",     "B-Max (4th Generation, Type A-600)",      (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_bmax )
