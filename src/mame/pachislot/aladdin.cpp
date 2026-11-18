// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Niigata Denshi Seiki Pachi-Slot Series Driver
    Z80 + 3-reel + YM2149 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2025-06-01.

    Supported Games:
    - Aladdin (c) 1989 Niigata Denshi Seiki (2nd Generation, 2nd model, Type A-C)

    Hardware:
    - Main CPU: Z80 @ provisional 5.825 MHz
      (2.9125 MHz XTAL x 2, provisionally doubled from 2.9125 MHz
      to prevent reel opto detection loss pending precise hardware measurement)
    - Sound: YM2149 @ 1.092 MHz (2.184375 MHz XTAL / 2)
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 512 bytes battery-backed RAM (0x2000-0x21ff)

    TODO:
    - Identify the exact hardware startup / mode-exit conditions required to
      avoid the RAM-clear path (0x00dc -> 0x00e1) and complete normal operation.
      This likely requires verification against physical hardware to determine
      whether a non-standard switch input or unique key-switch transition is involved.
    - Confirm the physical segment-decoder IC / circuit for the 7-segment LED
      (currently rendered via a provisional BCD decoding table).
    - Measure precise crystal clock frequencies and periodic IRQ rate from
      real hardware.
    - Verify precise crystal clock frequency and actual hardware timing;
      currently, the main CPU clock is provisionally doubled from 2.9125 MHz
      to prevent occasional reel index sensor detection misses.

    NOTE:
    [Lineage and Setting Mode / Power-Fail Restoration]
    - Although Aladdin was released by Niigata Denshi Seiki and Aladdin 2 by Sammy,
      the software codebase was directly carried over due to Niigata Denshi Seiki
      joining the Sammy group.
    - Consequently, both games share an identical, non-standard setting-change
      and power-fail recovery mechanism (validating the 0x5a / 0xa5 pattern and
      restoring SP) that does not match standard 2nd/3rd generation pachislot flow.
    - The sequence accepts setting value changes, but cleanly leaving the setting
      loop without falling into the NVRAM clear path remains unresolved.
    - Standard physical cabinets of this era only feature standard key
      switches and reset buttons; however, the program logic distinctly requires
      an additional input condition to exit the routine cleanly.
      Possibilities include an undocumented cabinet interlock, a combined
      key-off physical contact behavior, or specific hardware line logic.
    - Full hardware-accurate handling is kept on hold pending further inspection
      of original hardware behavior.

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
      active-low PFI signal on port 0x4000 bit 3 and preserves the physical
      reel absolute positions via stepper_device helpers.
    - Firmware writes non-zero to 0xa000 during power-fail shutdown to latch
      off the periodic IRQ line until the next reset cycle.
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

    [Electronic Flash Simulation]
    - On real hardware, a dedicated strobe/flash tube on the front panel
      flashes rapidly during bonus rounds. Because the hardware latch bit
      simply remains asserted during this state, the flashing pulse effect
      is driven by an internal timer helper.

    [Bookkeeping / Mechanical Counters]
    - Total IN and Total OUT pulse outputs are connected to coin_counter_w(0/1).
      On physical machines, these lines directly pulsed internal electromechanical
      6-digit meters and parlor management terminals.

    [Game Rules & Options]
    - Automatic game-stop (UCHIDOME) is enforced by program logic upon
      BIG BONUS completion.
    - No configurable SEISAN / UCHIDOME options are available.

***************************************************************************/

#include "emu.h"

#include "pachislotdev.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/steppers.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "speaker.h"


namespace {

// internal artwork
#include "aladdin.lh"

class aladdin_state : public driver_device
{
public:
	aladdin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pachislot(*this, "pachislot")
		, m_ym2149(*this, "ym2149")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_lamp_bigchance(*this, "lamp_bigchance")
		, m_lay_lamp_insert(*this, "lamp_insert")
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_lamp_bcount(*this, "lamp_bcount%u", 0U)
		, m_lay_eflash_bonus(*this, "eflash_bonus")
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void aladdin(machine_config &config);

	void init_aladdin();

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

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_nvram;
	required_device<pachislot_device> m_pachislot;
	required_device<ym2149_device> m_ym2149;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<2> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<> m_lay_lamp_bigchance;
	output_finder<> m_lay_lamp_insert;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<3> m_lay_lamp_bcount;
	output_finder<> m_lay_eflash_bonus;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	void watchdog_kick_w(offs_t offset, u8 data);
	void irq0_control_w(offs_t offset, u8 data);
	void lamp_latch_a_w(offs_t offset, u8 data);
	void lamp_latch_b_w(offs_t offset, u8 data);
	void seg_control_w(offs_t offset, u8 data);
	void drive_control_w(offs_t offset, u8 data);

	TIMER_CALLBACK_MEMBER(irq0_timer_cb);
	TIMER_CALLBACK_MEMBER(eflash_timer_cb);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	emu_timer *m_eflash_timer;
	emu_timer *m_irq0_timer;
	bool m_irq0_enabled;
	u8 m_eflash_flag;
	u8 m_eflash_task;
	int m_eflash_count;

	u8 m_opto[3];
	u8 m_segdata[8];
	u8 m_segdot;
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;
};


void aladdin_state::init_aladdin()
{
	// Electronic flash initialization
	m_eflash_flag = 0;
	m_eflash_task = 0;

	m_eflash_timer = timer_alloc(FUNC(aladdin_state::eflash_timer_cb), this);
	m_eflash_timer->adjust(attotime::zero);

	// NOTE:
	// The periodic IRQ rate is currently an estimated value based on observed
	// game behavior such as reel speed, lamp timing, and overall pacing.
	// It is not yet measured from real hardware and may need adjustment.
	m_irq0_timer = timer_alloc(FUNC(aladdin_state::irq0_timer_cb), this);
	m_irq0_timer->adjust(attotime::from_hz(520), 0, attotime::from_hz(520));
}

void aladdin_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_segdot));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_irq0_enabled));
	save_item(NAME(m_eflash_flag));
	save_item(NAME(m_eflash_task));
	save_item(NAME(m_eflash_count));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void aladdin_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segdot = 0;

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;

	m_eflash_flag = 0;
	m_eflash_task = 0;
	m_eflash_count = 0;
	m_irq0_enabled = true;

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
void aladdin_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void aladdin_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void aladdin_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

INPUT_CHANGED_MEMBER(aladdin_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
	}
}

// ---------------------------------------------------------------------------
//  Electronic Flash (Strobe) Simulation
// ---------------------------------------------------------------------------
// On genuine hardware, the top panel features a high-voltage xenon flash tube
// that strobes intensely during bonus rounds. Because the CPU simply asserts
// a single latch line (port 0x04 bit 4) continuously during bonus mode, the
// flashing pulse rate (~1 Hz pulse with ~30 ms flash width) is governed by an
// autonomous RC relaxation / trigger circuit on the cabinet lamp board.
//
// This state machine models the physical discharge and recharge cycle:
//   Task 0: Wait for CPU bonus flash trigger bit to be asserted.
//   Task 1: Fire strobe tube (held active for ~30 ms for visual persistence).
//   Task 2: Extinguish strobe and simulate reservoir capacitor recharge
//           delay (~750 ms).
// ---------------------------------------------------------------------------
TIMER_CALLBACK_MEMBER(aladdin_state::eflash_timer_cb)
{
	m_eflash_timer->adjust(attotime::from_msec(10));

	switch (m_eflash_task)
	{
		case 0:
			// Task 0: Idle / waiting for trigger flag from CPU output latch
			if (m_eflash_flag)
			{
				m_eflash_count = 0;
				m_lay_eflash_bonus = 1;		// Fire strobe pulse
				m_eflash_task++;
			}
			break;

		case 1:
			// Task 1: Strobe pulse hold time (~30 ms)
			if (m_eflash_count > 2)
			{
				m_lay_eflash_bonus = 0;		// Discharge complete
				m_eflash_task++;
			}
			break;

		case 2:
			// Task 2: High-voltage recharge interval (~750 ms)
			if (m_eflash_count > 75)
				m_eflash_task = 0;		// Ready for next strobe cycle
			break;
	}

	m_eflash_count++;
}

TIMER_CALLBACK_MEMBER(aladdin_state::irq0_timer_cb)
{
	if (m_irq0_enabled)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

u8 aladdin_state::switch_sense_a_r(offs_t offset)
{
	// bit0: reel 1 sensor (left)
	// bit1: reel 2 sensor (center)
	// bit2: reel 3 sensor (right)
	// bit3: PFI signal in (ready to return from power-fail)
	// bit4: start lever
	// bit5: reel 3 stop button (right)
	// bit6: reel 2 stop button (center)
	// bit7: reel 1 stop button (left)

	u8 port = m_in_port[0]->read();

	if (m_powerfail_status_ready)
		port &= ~0x08;

	return port;
}

u8 aladdin_state::switch_sense_b_r(offs_t offset)
{
	// bit0: medal selector sensor 0
	// bit1: medal hopper sensor 0
	// bit2: setting key (setting-loop control input)
	// bit3: change setting value Button / setting-mode exit button
	//       (tentative development input; physical cabinet circuit unresolved)
	// bit4: bet button
	// bit5: payout / credit game button
	// bit6: door key / reset button
	// bit7: unknown (possibly unused; appears to be normally high)

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[1]->read();
}

void aladdin_state::watchdog_kick_w(offs_t offset, u8 data)
{
	// 0x8000 is write-only and hit heavily during early startup delay loops.
	// The data pattern appears to come from the loop counter itself.
	// It may be a watchdog kick or other side-effect write, but no behavior
	// is modelled yet because the game currently runs without it.
}

void aladdin_state::irq0_control_w(offs_t offset, u8 data)
{
	// Analysis note:
	// Writing non-zero (observed during power-fail path) halts periodic INT0
	// generation until the next hard reset cycle. Writing 0x00 keeps INT0 enabled.
	if (data != 0x00)
	{
		powerfail_prepare_state();
		m_irq0_enabled = false;
	}
	else
		m_irq0_enabled = true;
}

void aladdin_state::lamp_latch_a_w(offs_t offset, u8 data)
{
	// bit0: bet medal(s) line lamp 1
	// bit1: bet medal(s) line lamp 2 (upper)
	// bit2: bet medal(s) line lamp 2 (lower)
	// bit3: bet medal(s) line lamp 3 (upper)
	// bit4: bet medal(s) line lamp 3 (lower)
	// bit5: reel 3 stop button lamp (right)
	// bit6: reel 2 stop button lamp (center)
	// bit7: reel 1 stop button lamp (left)

	// bit0-4: bet line lamp (physical upper/lower pairs)
	for (u8 i = 0; i < 5; i++)
		m_lay_lamp_betline[i] = BIT(data, i);

	// bit5-7: stop lamp
	m_lay_lamp_stop[0] = BIT(data, 7);
	m_lay_lamp_stop[1] = BIT(data, 6);
	m_lay_lamp_stop[2] = BIT(data, 5);
}

void aladdin_state::lamp_latch_b_w(offs_t offset, u8 data)
{
	// bit0:   bonus count 3 lamp (lower)
	// bit1:   bonus count 2 lamp (middle)
	// bit2:   bonus count 1 lamp (upper)
	// bit3:   insert medal lamp
	// bit4:   bonus lamp (electronic flash trigger)
	// bit5:   BIG chance lamp
	// bit6-7: unused

	// bit0-2: bonus count lamp
	m_lay_lamp_bcount[0] = BIT(data, 2);
	m_lay_lamp_bcount[1] = BIT(data, 1);
	m_lay_lamp_bcount[2] = BIT(data, 0);

	// bit3: insert medal lamp
	m_lay_lamp_insert = BIT(data, 3);

	// bit4: bonus lamp (electronic flash trigger)
	m_eflash_flag = BIT(data, 4);
	// bit5: BIG chance lamp
	m_lay_lamp_bigchance = BIT(data, 5);
}

void aladdin_state::seg_control_w(offs_t offset, u8 data)
{
	// bit0: BCD data bit 1
	// bit1: BCD data bit 2
	// bit2: BCD data bit 3
	// bit3: BCD data bit 4
	// bit4: credit SEG low common bit
	// bit5: credit SEG high common bit
	// bit6: payout SEG low common bit
	// bit7: payout SEG high common bit

	static const u8 segdigit[16] = 
	{
		//.gfedcba
		0b00111111,					// 0x00
		0b00000110,					// 0x01
		0b01011011,					// 0x02
		0b01001111,					// 0x03
		0b01100110,					// 0x04
		0b01101101,					// 0x05
		0b01111101,					// 0x06
		0b00000111,					// 0x07
		0b01111111,					// 0x08
		0b01101111,					// 0x09
		0b01110111,					// 0x0a (A) debug
		0b01111100,					// 0x0b (b) debug
		0b00111001,					// 0x0c (C) debug
		0b01011110,					// 0x0d (d) debug
		0b01111001,					// 0x0e (E) debug
		0b00000000					// 0x0f blank
	};

	for (u8 i = 0; i < 4; i++)
		if (BIT(data, 7 - i))
			m_segdata[i] = data & 0x0f;

	// SEG credit (SEG bit value -> SEG digit data)
	m_lay_seg_credit[0] = segdigit[m_segdata[0]];		// high
	m_lay_seg_credit[1] = segdigit[m_segdata[1]];		// low
	// SEG payout (SEG bit value -> SEG digit data)
	m_lay_seg_payout[0] = segdigit[m_segdata[2]];		// high
	m_lay_seg_payout[1] = segdigit[m_segdata[3]] | ((m_segdot << 7) & 0x80);	// low + dot flag
}

void aladdin_state::drive_control_w(offs_t offset, u8 data)
{
	// bit0: total IN pulse out
	// bit1: total OUT pulse out
	// bit2: JAC game signal out
	// bit3: BIG bonus signal out
	// bit4: medal lockout solenoid
	// bit5: hopper motor
	// bit6: unused
	// bit7: payout SEG dot flag

	// bit0: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	// bit1: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	// bit2: JAC game signal out
	m_lay_signal_jac = !BIT(data, 2);
	// bit3: BIG bonus signal out
	m_lay_signal_big = !BIT(data, 3);

	// bit4: medal lockout solenoid
	m_solenoid = BIT(data, 4);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	// bit5: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	m_hopper = !BIT(data, 5);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;

	// bit7: payout SEG dot flag
	m_segdot = !BIT(data, 7);
}

void aladdin_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram().share("nvram");
	map(0x4000, 0x4000).r(FUNC(aladdin_state::switch_sense_a_r));
	map(0x6000, 0x6000).r(FUNC(aladdin_state::switch_sense_b_r));
	map(0x8000, 0x8000).w(FUNC(aladdin_state::watchdog_kick_w));	// Startup loop write / candidate watchdog kick
	map(0xa000, 0xa000).w(FUNC(aladdin_state::irq0_control_w));	// IRQ0 enable/disable latch (asserted on power-fail)
}

void aladdin_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x02).lw8(NAME([this] (offs_t offset, u8 data) { m_reel[offset]->write(data & 0x0f); }));
	map(0x03, 0x03).w(FUNC(aladdin_state::lamp_latch_a_w));
	map(0x04, 0x04).w(FUNC(aladdin_state::lamp_latch_b_w));
	map(0x05, 0x05).w(m_ym2149, FUNC(ym2149_device::address_w));
	map(0x06, 0x06).nopw();
	map(0x0d, 0x0d).w(m_ym2149, FUNC(ym2149_device::data_w));
}


static INPUT_PORTS_START( aladdin )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdin_state::opto_r<0>))	// reel 1 sensor (left)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdin_state::opto_r<1>))	// reel 2 sensor (center)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdin_state::opto_r<2>))	// reel 3 sensor (right)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM )	// PFI signal in (ready to return from power-fail)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(aladdin_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(aladdin_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button/ Exit Setting Mode Button (tentative)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_NAME("Door Key / Reset Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aladdin_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void aladdin_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void aladdin_state::aladdin(machine_config &config)
{
	// basic machine hardware
	// 
	// NOTE:
	// 2.9125 MHz is cited from available hardware references, but the clock
	// is provisionally doubled (*2) here because the original frequency causes
	// occasional reel opto sensor detection misses during high-speed rotation.
	// This should be verified against physical hardware measurements in the future.
	Z80(config, m_maincpu, 2912500*2);	// XTAL(2'912'500) x 2 (Provisional)
	m_maincpu->set_addrmap(AS_PROGRAM, &aladdin_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &aladdin_state::io_map);

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(70*2);
	m_pachislot->hopper_cb().set(FUNC(aladdin_state::playing_hopper_samples));

	// Configure sensor emulation modes: single-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SINGLE);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], SAMMY_200STEP_REEL, 11, 14, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(aladdin_state::opto_cb<0>));
	REEL(config, m_reel[1], SAMMY_200STEP_REEL, 11, 14, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(aladdin_state::opto_cb<1>));
	REEL(config, m_reel[2], SAMMY_200STEP_REEL, 11, 14, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(aladdin_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2149(config, m_ym2149, 2184375 / 2).add_route(ALL_OUTPUTS, "speaker", 0.25);	// XTAL(2'184'375)
	m_ym2149->port_a_write_callback().set(FUNC(aladdin_state::seg_control_w));
	m_ym2149->port_b_write_callback().set(FUNC(aladdin_state::drive_control_w));

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psaladdin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "aladdin.bin", 0x0000, 0x2000, CRC(f996d94f) SHA1(0886f572753cc266f7aba9c1ca4db6431ee5be32) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,      PARENT, MACHINE, INPUT,   CLASS,         INIT,         MONITOR, COMPANY,                FULLNAME,                                        FLAGS,                                                            LAYOUT
GAMEL( 1989, psaladdin, 0,      aladdin, aladdin, aladdin_state, init_aladdin, 0,       "Niigata Denshi Seiki", "Aladdin (2nd Generation, 2nd model, Type A-C)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL|MACHINE_IS_INCOMPLETE), layout_aladdin )
