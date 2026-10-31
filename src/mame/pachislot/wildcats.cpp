// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana, gregre365
/***************************************************************************

    Arktechnico / Baltec Pachi-Slot Series Driver
    Z80 + 3-reel + YM2413 titles

    Driver by Chiaki Tachibana
    Hardware research, PCB tracing and notes by gregre365

    Initial driver development started on 2024-09-11.

    Supported Games:
    - Wild Cats     (c) 1990 Arktechnico  (3rd Generation, 1st model, Type A)
    - Seven Bomber  (c) 1990 Baltec       (3rd Generation, 1st model, Type A)

    Hardware:
    - Main CPU: Z80 @ 4.0 MHz (8.0 MHz XTAL / 2)
    - Sound: YM2413 @ 4.0 MHz (8.0 MHz XTAL / 2)
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 2048 bytes battery-backed SRAM (uPD449C-1 at IC9)
    - PCB Silkscreen: "P3AA-001-I" (Wild Cats)

    TODO:
    - The reel drive outputs an 8-bit pattern; currently, even-numbered
      bits (0/2/4/6) are sampled and mapped to standard 4-bit stepper phases.
      Confirm physical coil wiring and transistor arrays on the PCB to verify
      full 8-step excitation timing.

    NOTE:
    [Periodic IRQ Rate and Pacing]
    - Periodic IRQ rate is 4 MHz / 7500 cycles = 533.333 Hz (1.875 ms period).
      This exactly matches the internal 4.1025-second wait timer (2188
      interrupts per game interval) as well as verified reel pacing and
      audio tempo matched against real gameplay recordings.

    [NVRAM Cold-Start Behavior and "nE" Display]
    - On genuine hardware, an uninitialized or discharged battery-backed
      SRAM (uPD449C-1) contains cleared or invalid data, causing the firmware
      to trigger a RAM-loss stop displaying "nE" on the 7-segment LEDs and
      resetting internal settings to default (setting 3).
    - This is the intended and correct factory cold-boot / power-loss sequence
      designed into the official ROM. Once this state is reached, a standard
      software reset (e.g. F3 in MAME) or power cycle initiates normal operation.
    - Consequently, this driver does not include an artificial, pre-generated
      NVRAM binary dump (such as the provisional upd449c-1.ic9 image used in
      earlier submissions). Relying on MAME's standard DEFAULT_ALL_0 NVRAM
      device faithfully mirrors the physical hardware cold-start lifecycle.

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, allowing the CPU to execute its
      state-saving routine into battery-backed NVRAM during the reservoir
      capacitance hold-up time.
    - Because MAME currently lacks a pre-reset / pre-shutdown callback
      mechanism to allow the CPU to gracefully complete its power-down
      sequence before machine_reset() or exit (e.g. F3 / ESC keys), this
      driver provisionally provides a manual 'P' key (powerfail_trigger).
    - Pressing the 'P' key sets m_powerfail_status_ready to simulate the
      active-high PFI signal on port 0x4000 bit 7 and preserves the physical
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

    [Historical Background: SRAM Injection and Normal State]
    - During the active 3rd-generation era in Japanese parlors, many units of
      Wild Cats and Seven Bomber were modified by distributors prior to parlor
      installation using a technique known as "SRAM injection" (注射).
    - In these modified machines, an external payload written into the battery-
      backed SRAM altered the bonus payout logic to create a "payout reserve /
      burst accumulation" (貯金) behavior, postponing BIG BONUS hits and releasing
      them in rapid succession under specific triggers.
    - Standard setting change sequences typically erased the SRAM payload to
      restore the machine to its compliant, factory-normal behavior during
      regulatory inspections, while bypassing the start-lever confirmation
      preserved the modified SRAM routines.
    - Because the battery backup (supercapacitor) in vintage original boards has
      long discharged, preserved raw SRAM payloads are extremely rare today.
      Later underground revisions incorporated equivalent logic directly into
      modified program EPROMs (bootleg/ura-ROM sets).
    - This driver currently emulates the official factory-stock program ROM;
      support for dumped modified EPROMs or authentic SRAM payloads may be
      added as preservation dumps become available.

    [Bookkeeping / Mechanical Counters]
    - Total IN (bit 7) and Total OUT (bit 6) pulse outputs driven by 0x4006
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

    [Setting Change Procedure]
    - Standard 3rd generation sequence:
      1. Turn power on while asserting Setting Key (F2).
      2. Select desired setting (1-6) using Setting Value button (F1 / Door Reset).
      3. Confirm setting value by operating Start Lever.
      4. Turn Setting Key off to enter normal game state.

    [Game Rules & Options]
    - Automatic game-stop (UCHIDOME) is enforced by program logic upon
      BIG BONUS completion. Cancelled via the Door Key / Reset control.
    - No configurable SEISAN / UCHIDOME options are available.

***************************************************************************/

#include "emu.h"

#include "pachislotdev.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/steppers.h"
#include "sound/ymopl.h"
#include "sound/samples.h"
#include "speaker.h"


namespace {

// internal artwork
#include "wildcats.lh"
#include "sevenbomber.lh"

class wildcats_state : public driver_device
{
public:
	wildcats_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pachislot(*this, "pachislot")
		, m_ym2413(*this, "ym2413")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_lamp_frame(*this, "lamp_frame%u", 0U)
		, m_lay_lamp_777(*this, "lamp_777%u", 0U)
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_lamp_timeup(*this, "lamp_timeup")
		, m_lay_lamp_pause(*this, "lamp_pause")
		, m_lay_lamp_insert(*this, "lamp_insert")
		, m_lay_lamp_credit(*this, "lamp_credit")
		, m_lay_light_reel(*this, "light_reel%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void common(machine_config &config);
	void wildcats(machine_config &config);
	void sevenbomber(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(powerfail_trigger);

	template <pachislot_device::DEVSENSOR_NUM N> int selector_r() { return m_pachislot->selector_r(N); }
	template <pachislot_device::DEVSENSOR_NUM N> int hopper_r() { return m_pachislot->hopper_r(N); }
	template <unsigned N> int in_fakebtn_r() { return BIT(m_in_fakebtn->read(), N); }
	template <unsigned N> int opto_r() { return m_opto[N]; }

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

	void wildcats_mem_map(address_map &map) ATTR_COLD;
	void sevenbomber_mem_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_nvram;
	required_device<pachislot_device> m_pachislot;
	required_device<ym2413_device> m_ym2413;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<2> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<6> m_lay_lamp_frame;
	output_finder<5> m_lay_lamp_777;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<> m_lay_lamp_timeup;
	output_finder<> m_lay_lamp_pause;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_credit;
	output_finder<3> m_lay_light_reel;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	void reel_phase_w(offs_t offset, u8 data);
	void lamp_latch_a_w(offs_t offset, u8 data);
	void seg_digit_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void wildcats_lamp_and_counter_w(offs_t offset, u8 data);
	void wildcats_drive_latch_w(offs_t offset, u8 data);
	void sevenbomber_lamp_and_counter_w(offs_t offset, u8 data);
	void sevenbomber_drive_latch_w(offs_t offset, u8 data);
	void sevenbomber_lamp_latch_b_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	u8 m_opto[3];
	u8 m_segcommon;
	u8 m_segdata[4];
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;
};


void wildcats_state::machine_start()
{
	save_item(NAME(m_opto));
	save_item(NAME(m_segcommon));
	save_item(NAME(m_segdata));
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

void wildcats_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segcommon = 0;

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
void wildcats_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void wildcats_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void wildcats_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

INPUT_CHANGED_MEMBER(wildcats_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
	}
}

u8 wildcats_state::switch_sense_a_r(offs_t offset)
{
	// bit0: reel 1 stop button (left)
	// bit1: reel 2 stop button (center)
	// bit2: reel 3 stop button (right)
	// bit3: medal selector sensor 0
	// bit4: reel 1 sensor (left)
	// bit5: reel 2 sensor (center)
	// bit6: reel 3 sensor (right)
	// bit7: PFI signal in (ready to return from power-fail)

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	u8 port = m_in_port[0]->read();

	if (m_powerfail_status_ready)
		port |= 0x80;

	return port;
}

u8 wildcats_state::switch_sense_b_r(offs_t offset)
{
	// bit0: medal hopper sensor 0
	// bit1: setting key
	// bit2: [wildcats] change setting value button / [sevenbomber] door key / reset button
	// bit3: bet button
	// bit4: unknown
	// bit5: [wildcats] door key / reset button / [sevenbomber] change setting value button
	// bit6: payout / credit game button
	// bit7: start lever

	return m_in_port[1]->read();
}

void wildcats_state::reel_phase_w(offs_t offset, u8 data)
{
	// bit0-7: reel pulse bit (8-bit data)

	// The hardware outputs an 8-bit reel-drive pattern. Even-numbered bits
	// (0, 2, 4, 6) are sampled and packed into standard 4-bit stepper phases.
	m_reel[offset]->write((BIT(data, 0) << 0) | (BIT(data, 2) << 1) | (BIT(data, 4) << 2) | (BIT(data, 6) << 3));
}

void wildcats_state::lamp_latch_a_w(offs_t offset, u8 data)
{
	// bit0-2: stop lamp
	// bit3:   credit lamp
	// bit4-7: bonus game count lamp "777" / "SEVEN" lamp 1-4

	// bit0-2: stop lamp
	m_lay_lamp_stop[0] = BIT(data, 0);		// left
	m_lay_lamp_stop[1] = BIT(data, 1);		// center
	m_lay_lamp_stop[2] = BIT(data, 2);		// right

	// bit3: credit lamp
	m_lay_lamp_credit = BIT(data, 3);

	// bit4-7: bonus game count lamp "777" / "SEVEN" lamp 1-4
	m_lay_lamp_777[0] = BIT(data, 4);		// 1 ("cat")      / 1 ("S")
	m_lay_lamp_777[1] = BIT(data, 5);		// 2 ("7" red)    / 2 ("E")
	m_lay_lamp_777[2] = BIT(data, 6);		// 3 ("7" yellow) / 3 ("V")
	m_lay_lamp_777[3] = BIT(data, 7);		// 4 ("7" blue")
}

void wildcats_state::seg_digit_w(offs_t offset, u8 data)
{
	// bit0-3: SEG common select bit
	// bit4-7: unknown

	if (data & 0x0f)
		m_segcommon = data;
}

void wildcats_state::seg_data_w(offs_t offset, u8 data)
{
	// bit0-7: SEG common data pattern

	// SEG common bit -> SEG select
	for (u8 i = 0; i < 4; i++)
		if (BIT(m_segcommon, i))
			m_segdata[i] = data;

	// SEG credit
	m_lay_seg_credit[0]	= m_segdata[0];		// credit SEG high
	m_lay_seg_credit[1]	= m_segdata[3];		// credit SEG low
	// SEG payout
	m_lay_seg_payout[0]	= m_segdata[2];		// payout SEG high
	m_lay_seg_payout[1]	= m_segdata[1];		// payout SEG low
}

void wildcats_state::wildcats_lamp_and_counter_w(offs_t offset, u8 data)
{
	// bit0-4: bet medal(s) line lamp (physical upper/lower pairs)
	// bit5:   insert medal lamp
	// bit6:   total OUT pulse out
	// bit7:   total IN pulse out

	// bit0-4: bet line lamp (physical upper/lower pairs)
	m_lay_lamp_betline[0] = BIT(data, 0);		// 1 medal
	m_lay_lamp_betline[1] = BIT(data, 1);		// 2 medals (upper)
	m_lay_lamp_betline[2] = BIT(data, 2);		// 2 medals (lower)
	m_lay_lamp_betline[3] = BIT(data, 3);		// 3 medals (upper)
	m_lay_lamp_betline[4] = BIT(data, 4);		// 3 medals (lower)

	// bit5: insert medal lamp
	m_lay_lamp_insert = BIT(data, 5);

	// bit6: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));
	// bit7: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));
}

void wildcats_state::wildcats_drive_latch_w(offs_t offset, u8 data)
{
	// bit0-1: unknown
	// bit2:   time up lamp
	// bit3:   hopper motor drive
	// bit4:   medal lockout solenoid
	// bit5:   frame lamp
	// bit6:   JAC game signal out
	// bit7:   BIG bonus signal out

	// bit2: time up lamp
	m_lay_lamp_timeup = BIT(data, 2);

	// bit3: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	m_hopper = BIT(data, 3);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;

	// bit4: medal lockout solenoid
	m_solenoid = !BIT(data, 4);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	// bit5: frame lamp
	m_lay_lamp_frame[0] = BIT(data, 5);

	// bit6: JAC game signal out
	m_lay_signal_jac = BIT(data, 6);
	// bit7: BIG bonus signal out
	m_lay_signal_big = BIT(data, 7);
}

void wildcats_state::sevenbomber_lamp_and_counter_w(offs_t offset, u8 data)
{
	// bit0-4: bet medal(s) line lamp (physical upper/lower pairs)
	// bit5:   reel light
	// bit6:   total OUT pulse out
	// bit7:   total IN pulse out

	// bit0-4: bet line lamp (physical upper/lower pairs)
	m_lay_lamp_betline[0] = BIT(data, 0);		// 1 medal
	m_lay_lamp_betline[1] = BIT(data, 1);		// 2 medals (upper)
	m_lay_lamp_betline[2] = BIT(data, 2);		// 2 medals (lower)
	m_lay_lamp_betline[3] = BIT(data, 3);		// 3 medals (upper)
	m_lay_lamp_betline[4] = BIT(data, 4);		// 3 medals (lower)

	// bit5: reel light
	for (u8 i = 0; i < 3; i++)
		m_lay_light_reel[i] = BIT(data, 5);

	// bit6: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));
	// bit7: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));
}

void wildcats_state::sevenbomber_drive_latch_w(offs_t offset, u8 data)
{
	// bit0-1: unknown
	// bit2:   pause lamp
	// bit3:   hopper motor drive
	// bit4:   medal lockout solenoid
	// bit5:   unknown
	// bit6:   JAC game signal out
	// bit7:   BIG bonus signal out

	// bit2: pause lamp
	m_lay_lamp_pause = BIT(data, 2);

	// bit3: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	m_hopper = BIT(data, 3);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;

	// bit4: medal lockout solenoid
	m_solenoid = !BIT(data, 4);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	// bit6: JAC game signal out
	m_lay_signal_jac = BIT(data, 6);
	// bit7: BIG bonus signal out
	m_lay_signal_big = BIT(data, 7);
}

void wildcats_state::sevenbomber_lamp_latch_b_w(offs_t offset, u8 data)
{
	// bit0-5: frame top lamp (1-6)
	// bit6-7: bonus game count "SEVEN" lamp 4-5

	for (u8 i = 0; i < 6; i++)
		m_lay_lamp_frame[i] = BIT(data, i);

	m_lay_lamp_777[3] = BIT(data, 6);		// 4 ("E")
	m_lay_lamp_777[4] = BIT(data, 7);		// 5 ("N")
}

void wildcats_state::wildcats_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	// uPD449C-1 (2K SRAM) mirrored across 8K window
	map(0x2000, 0x27ff).mirror(0x1800).ram().share("nvram");
	map(0x4000, 0x4000).r(FUNC(wildcats_state::switch_sense_a_r));
	map(0x4001, 0x4001).r(FUNC(wildcats_state::switch_sense_b_r));
	map(0x4002, 0x4002).nopr();			// unknown (bits 0-1 vary on real hardware)
	map(0x4000, 0x4002).w(FUNC(wildcats_state::reel_phase_w));
	map(0x4003, 0x4003).w(FUNC(wildcats_state::lamp_latch_a_w));
	map(0x4004, 0x4004).w(FUNC(wildcats_state::seg_digit_w));
	map(0x4005, 0x4005).w(FUNC(wildcats_state::seg_data_w));
	map(0x4006, 0x4006).w(FUNC(wildcats_state::wildcats_lamp_and_counter_w));
	map(0x4007, 0x4007).w(FUNC(wildcats_state::wildcats_drive_latch_w));
	map(0x4008, 0x4008).nopw();			// status/backup-done latch (0x20 signals backup completion)
	map(0x4009, 0x4009).nopw();			// one-time init write after reset
	map(0x400a, 0x400b).w(m_ym2413, FUNC(ym2413_device::write));
}

void wildcats_state::sevenbomber_mem_map(address_map &map)
{
	wildcats_mem_map(map);

	map(0x4006, 0x4006).w(FUNC(wildcats_state::sevenbomber_lamp_and_counter_w));
	map(0x4007, 0x4007).w(FUNC(wildcats_state::sevenbomber_drive_latch_w));
	map(0x4008, 0x4008).w(FUNC(wildcats_state::sevenbomber_lamp_latch_b_w));
}


static INPUT_PORTS_START( common )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(wildcats_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(wildcats_state::opto_r<0>))	// reel 1 sensor (left)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(wildcats_state::opto_r<1>))	// reel 2 sensor (center)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(wildcats_state::opto_r<2>))	// reel 3 sensor (right)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )	// PFI signal in (ready to return from power-fail)

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(wildcats_state::powerfail_trigger), 0)

INPUT_PORTS_END

static INPUT_PORTS_START( wildcats )

	PORT_INCLUDE( common )

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(wildcats_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_NAME("Door Key / Reset Button")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")

INPUT_PORTS_END

static INPUT_PORTS_START( sevenbomber )

	PORT_INCLUDE( common )

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(wildcats_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_NAME("Door Key / Reset Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void wildcats_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void wildcats_state::common(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_periodic_int(FUNC(wildcats_state::irq0_line_hold), attotime::from_hz(4'000'000 / 7500));

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(60*2);
	m_pachislot->hopper_cb().set(FUNC(wildcats_state::playing_hopper_samples));

	// Configure sensor emulation modes: single-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SINGLE);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], ARKTECHNICO_200STEP_REEL, 38, 40, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(wildcats_state::opto_cb<0>));
	REEL(config, m_reel[1], ARKTECHNICO_200STEP_REEL, 38, 40, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(wildcats_state::opto_cb<1>));
	REEL(config, m_reel[2], ARKTECHNICO_200STEP_REEL, 38, 40, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(wildcats_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 8_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "speaker", 2.2);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(4);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}

void wildcats_state::wildcats(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &wildcats_state::wildcats_mem_map);
}

void wildcats_state::sevenbomber(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &wildcats_state::sevenbomber_mem_map);
}


ROM_START( pswildcats )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "wildcats.bin", 0x0000, 0x2000, CRC(362b3e92) SHA1(40aa96dded5a55865892868fd09cf5af4c909c85) )
ROM_END

ROM_START( pssbomber )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sevenbomber.bin", 0x0000, 0x2000, CRC(c691c509) SHA1(e074f77390e88178aa8e235e0d311d49f8f355af) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,     INPUT,       CLASS,          INIT,       MONITOR, COMPANY,       FULLNAME,                                           FLAGS,                                      LAYOUT
GAMEL( 1990, pswildcats, 0,      wildcats,    wildcats,    wildcats_state, empty_init, 0,       "Arktechnico", "Wild Cats (3rd Generation, 1st model, Type A)",    (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_wildcats )
GAMEL( 1990, pssbomber,  0,      sevenbomber, sevenbomber, wildcats_state, empty_init, 0,       "Baltec",      "Seven Bomber (3rd Generation, 1st model, Type A)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_sevenbomber )
