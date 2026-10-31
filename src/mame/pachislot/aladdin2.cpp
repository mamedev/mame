// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Sammy Pachi-Slot Series Driver
    Z80 + 3-reel + SN76496 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2025-04-17.

    Supported Games:
    - Aladdin 2 (c) 1991 Sammy (3rd Generation, 1st model, Type A-C)
    - Mr.Magic  (c) 1992 Sammy (3rd Generation, 2nd model, Type A-C)

    Hardware:
    - Main CPU: Z80 @ provisional 5.0 MHz (XTAL unknown, 3.579545 MHz suspected)
    - Sound: SN76496 @ provisional 3.0 MHz
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 512 bytes battery-backed RAM (0x2000-0x21ff)

    TODO:
    - Identify the exact hardware startup / mode-exit conditions required to
      avoid the RAM-clear path and complete normal operation. This likely requires
      verification against physical hardware to determine whether a non-standard
      switch input or unique key-switch transition is involved.
    - Verify main CPU crystal and true clock divider; at 3.579545 MHz, reel
      sensor polling can miss optical edges, and Mr. Magic slows down during
      BIG BONUS start lever polling.
    - Identify the real source and trigger condition of the unknown NMI line.
    - Determine the exact role of 0x6006 beyond IRQ0 writeback / display commit.

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
      active-low PFI signal on port 0x4002 bit 7 and preserves the physical
      reel absolute positions via stepper_device helpers.
    - Firmware writes to 0x6007 to control periodic IRQ0 enabling and latches
      the interrupt-stop state during the power-fail save sequence.
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

    [Bookkeeping / Mechanical Counters]
    - Output port 0x6002 drives Total IN (bit 0) and Total OUT (bit 1) pulses.
      These lines are mapped to coin_counter_w(0/1) representing the physical
      mechanical meter wiring and external centralized terminal signals.

    [Game Rules & Options]
    - Automatic game-stop (UCHIDOME) is enforced by program logic upon
      BIG BONUS completion.
    - Configurable SEISAN (automatic payout) behavior is supported via
      cabinet option switch ("SEISAN": Off / On). Note that changes
      require cycling through setting mode to reinitialize state.

***************************************************************************/

#include "emu.h"

#include "pachislotdev.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/steppers.h"
#include "sound/sn76496.h"
#include "sound/samples.h"
#include "speaker.h"


namespace {

// internal artwork
#include "aladdin2.lh"
#include "mrmagic.lh"

class aladdin2_state : public driver_device
{
public:
	aladdin2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pachislot(*this, "pachislot")
		, m_sn76496(*this, "sn76496")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_lamp_frame(*this, "lamp_frame%u", 0U)
		, m_lay_lamp_bigchance(*this, "lamp_bigchance")
		, m_lay_lamp_insert(*this, "lamp_insert")
		, m_lay_lamp_wait(*this, "lamp_wait")
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_lamp_bcount(*this, "lamp_bcount%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void aladdin2(machine_config &config);

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

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_nvram;
	required_device<pachislot_device> m_pachislot;
	required_device<sn76496_device> m_sn76496;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<3> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<3> m_lay_lamp_frame;
	output_finder<> m_lay_lamp_bigchance;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_wait;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<3> m_lay_lamp_bcount;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 cabinet_switch_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	void reel_phase_ab_w(offs_t offset, u8 data);
	void drive_latch_w(offs_t offset, u8 data);
	void signal_and_segcomm_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void lamp_latch_a_w(offs_t offset, u8 data);
	void lamp_latch_b_w(offs_t offset, u8 data);
	void display_commit_w(offs_t offset, u8 data);
	void powerfail_control_w(offs_t offset, u8 data);

	TIMER_CALLBACK_MEMBER(int_on);
	TIMER_CALLBACK_MEMBER(int_off);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	emu_timer *m_int_on_timer;
	emu_timer *m_int_off_timer;

	bool m_irq0_enabled;
	u8 m_irq0_disable_latch;

	u8 m_opto[3];
	u8 m_segdata[4];
	u8 m_segcommon;
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;
};


void aladdin2_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_segcommon));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_irq0_enabled));
	save_item(NAME(m_irq0_disable_latch));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	// Timer setup
	m_int_on_timer = timer_alloc(FUNC(aladdin2_state::int_on), this);
	m_int_off_timer = timer_alloc(FUNC(aladdin2_state::int_off), this);

	// NOTE:
	// The current IRQ timing values are estimated from observed game behavior
	// such as reel speed, lamp timing, and overall pacing.
	// They are not yet measured from real hardware and may need adjustment.

	m_int_on_timer->adjust(attotime::zero, 0, attotime::from_hz(500));
	m_int_off_timer->adjust(attotime::never);

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void aladdin2_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segcommon = 0;

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;

	m_irq0_enabled = true;
	m_irq0_disable_latch = 0;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

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
void aladdin2_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void aladdin2_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void aladdin2_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

INPUT_CHANGED_MEMBER(aladdin2_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
	}
}

TIMER_CALLBACK_MEMBER(aladdin2_state::int_on)
{
	if (!m_irq0_enabled)
		return;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	m_int_off_timer->adjust(m_maincpu->clocks_to_attotime(32));
}

TIMER_CALLBACK_MEMBER(aladdin2_state::int_off)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

u8 aladdin2_state::switch_sense_a_r(offs_t offset)
{
	// bit0: medal selector sensor 0
	// bit1: medal selector sensor 1
	// bit2: bet button
	// bit3: medal hopper sensor 0
	// bit4: payout / credit game button
	// bit5: start lever / setting confirm
	// bit6: setting key
	// bit7: change setting value button

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[0]->read();
}

u8 aladdin2_state::cabinet_switch_r(offs_t offset)
{
	// bit0:   "SEISAN" ("清算") switch (automatic payout)
	// bit1:   door key / reset button
	// bit2-7: unknown / unused

	return m_in_port[1]->read();
}

u8 aladdin2_state::switch_sense_b_r(offs_t offset)
{
	// bit0: reel 1 sensor (left)
	// bit1: reel 2 sensor (center)
	// bit2: reel 3 sensor (right)
	// bit3: unknown
	// bit4: reel 1 stop button (left)
	// bit5: reel 2 stop button (center)
	// bit6: reel 3 stop button (right)
	// bit7: PFI signal in (ready to return from power-fail)

	u8 port = m_in_port[2]->read();

	if (m_powerfail_status_ready)
		port &= ~0x80;

	return port;
}

void aladdin2_state::reel_phase_ab_w(offs_t offset, u8 data)
{
	// bit0-3: reel 1 pulse bit (left)
	// bit4-7: reel 2 pulse bit (center)

	m_reel[0]->write(data & 0x0f);
	m_reel[1]->write((data & 0xf0) >> 4);
}

void aladdin2_state::drive_latch_w(offs_t offset, u8 data)
{
	// bit0-3: reel 3 pulse bit (right)
	// bit4:   medal lockout solenoid
	// bit5:   wait lamp
	// bit6:   hopper motor
	// bit7:   unknown

	// bit0-3: reel 3 pulse bit (right)
	m_reel[2]->write(data & 0x0f);

	// bit4: medal lockout solenoid
	m_solenoid = !BIT(data, 4);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	// bit5: wait lamp
	m_lay_lamp_wait = BIT(data, 5);

	// bit6: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	m_hopper = BIT(data, 6);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;
}

void aladdin2_state::signal_and_segcomm_w(offs_t offset, u8 data)
{
	// bit0:   total IN pulse out
	// bit1:   total OUT pulse out
	// bit2:   JAC game signal out
	// bit3:   BIG bonus signal out
	// bit4-7: SEG common bit

	// bit0: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	// bit1: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	// bit2: JAC game signal out
	m_lay_signal_jac = BIT(data, 2);
	// bit3: BIG bonus signal out
	m_lay_signal_big = BIT(data, 3);

	// bit4-7: SEG common bit
	if (data & 0xf0)
		m_segcommon = (data & 0xf0) >> 4;
}

void aladdin2_state::seg_data_w(offs_t offset, u8 data)
{
	// bit0-7: SEG common bit -> SEG select

	for (u8 i = 0; i < 4; i++)
		if (BIT(m_segcommon, i))
			m_segdata[i] = data;
}

void aladdin2_state::lamp_latch_a_w(offs_t offset, u8 data)
{
	// bit0: insert medal lamp
	// bit1: bonus count 3 lamp (lower)
	// bit2: bonus count 2 lamp (middle)
	// bit3: bonus count 1 lamp (upper)
	// bit4: big chance lamp 2 (lower)
	// bit5: big chance lamp 1 (upper)
	// bit6: frame lamp 2
	// bit7: frame lamp 1

	// bit0: insert medal lamp
	m_lay_lamp_insert = BIT(data, 0);

	// bit1-3: bonus count lamp
	m_lay_lamp_bcount[0] = BIT(data, 3);
	m_lay_lamp_bcount[1] = BIT(data, 2);
	m_lay_lamp_bcount[2] = BIT(data, 1);

	// bit4-5: big chance lamp
	m_lay_lamp_bigchance = BIT(data, 4) | BIT(data, 5);

	// bit6-7: frame lamp
	m_lay_lamp_frame[0] = BIT(data, 7);
	m_lay_lamp_frame[1] = BIT(data, 6);
	m_lay_lamp_frame[2] = BIT(data, 7);
}

void aladdin2_state::lamp_latch_b_w(offs_t offset, u8 data)
{
	// bit0: bet line 3 lamp
	// bit1: bet line 2 lamp
	// bit2: bet line 1 lamp
	// bit3: bet line 2 lamp
	// bit4: bet line 3 lamp
	// bit5: stop lamp 1
	// bit6: stop lamp 2
	// bit7: stop lamp 3

	// bit0-4: bet line lamp
	m_lay_lamp_betline[4] = BIT(data, 0);
	m_lay_lamp_betline[2] = BIT(data, 1);
	m_lay_lamp_betline[0] = BIT(data, 2);
	m_lay_lamp_betline[1] = BIT(data, 3);
	m_lay_lamp_betline[3] = BIT(data, 4);

	// bit5-7: stop lamp
	for (u8 i = 0; i < 3; i++)
		m_lay_lamp_stop[i] = BIT(data, i + 5);
}

// Writeback / strobe-like output to commit 7-segment display digits.
// Writing to 0x6006 occurs at the end of the periodic interrupt cycle; applying
// the latched segment data here synchronizes display multiplexing and eliminates flicker.
void aladdin2_state::display_commit_w(offs_t offset, u8 data)
{
	// bit0-7: provisional IRQ0-end writeback / ack-like output

	// SEG credit
	m_lay_seg_credit[0] = m_segdata[2];		// high
	m_lay_seg_credit[1] = m_segdata[1];		// low
	// SEG payout
	m_lay_seg_payout[0] = m_segdata[0];		// high
	m_lay_seg_payout[1] = m_segdata[3];		// low
}

void aladdin2_state::powerfail_control_w(offs_t offset, u8 data)
{
	// Provisional power-fail-side IRQ0 stop handling:
	// treat 0x6007 as the IRQ-related / power-fail control write that
	// latches the IRQ0 stop after the preparation pattern and saved SP are
	// already present in NVRAM.
	m_irq0_disable_latch = data;

	if (data == 0x00)
	{
		powerfail_prepare_state();
		m_irq0_enabled = false;
	}
	else if (data == 0x01)
	{
		// Keep IRQ0 stopped once the power-fail-side stop has latched.
		// Re-enable happens on reset / restart, not by a 0x6007=01 write.
		m_irq0_enabled = true;
	}
}

void aladdin2_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram().share("nvram");
	map(0x4000, 0x4000).r(FUNC(aladdin2_state::switch_sense_a_r));
	map(0x4001, 0x4001).r(FUNC(aladdin2_state::cabinet_switch_r));
	map(0x4002, 0x4002).r(FUNC(aladdin2_state::switch_sense_b_r));
	map(0x4007, 0x4007).w(m_sn76496, FUNC(sn76496_device::write));
	map(0x6000, 0x6000).w(FUNC(aladdin2_state::reel_phase_ab_w));
	map(0x6001, 0x6001).w(FUNC(aladdin2_state::drive_latch_w));
	map(0x6002, 0x6002).w(FUNC(aladdin2_state::signal_and_segcomm_w));
	map(0x6003, 0x6003).w(FUNC(aladdin2_state::seg_data_w));
	map(0x6004, 0x6004).w(FUNC(aladdin2_state::lamp_latch_a_w));
	map(0x6005, 0x6005).w(FUNC(aladdin2_state::lamp_latch_b_w));
	map(0x6006, 0x6006).w(FUNC(aladdin2_state::display_commit_w));
	map(0x6007, 0x6007).w(FUNC(aladdin2_state::powerfail_control_w));
}


static INPUT_PORTS_START( aladdin2 )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(aladdin2_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(aladdin2_state::selector_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal selector sensor 1
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(aladdin2_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button")

	// Cabinet option switches and other inputs
	PORT_START("INPUT1")
	PORT_CONFNAME( 0x01, 0x00, "SEISAN" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_NAME("Door Key / Reset Button")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdin2_state::opto_r<0>))	// reel 1 sensor (left)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdin2_state::opto_r<1>))	// reel 2 sensor (center)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(aladdin2_state::opto_r<2>))	// reel 3 sensor (right)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )	// PFI signal in (ready to return from power-fail)

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(aladdin2_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void aladdin2_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void aladdin2_state::aladdin2(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(5'000'000)); //  5.00 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &aladdin2_state::mem_map);

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(50*2);
	m_pachislot->hopper_cb().set(FUNC(aladdin2_state::playing_hopper_samples));

	// Configure sensor emulation modes: Sammy 2-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SAMMY1);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], SAMMY_200STEP_REEL, 13, 16, 0x00, 1, 200*2);
	m_reel[0]->optic_handler().set(FUNC(aladdin2_state::opto_cb<0>));
	REEL(config, m_reel[1], SAMMY_200STEP_REEL, 13, 16, 0x00, 1, 200*2);
	m_reel[1]->optic_handler().set(FUNC(aladdin2_state::opto_cb<1>));
	REEL(config, m_reel[2], SAMMY_200STEP_REEL, 13, 16, 0x00, 1, 200*2);
	m_reel[2]->optic_handler().set(FUNC(aladdin2_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SN76496(config, m_sn76496, 3.00_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 0.5);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psaladdin2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "aladdin2.bin", 0x0000, 0x2000, CRC(15a259bd) SHA1(c7c1c1e6a373933059053f93b9991e15de961e0a) )
ROM_END

ROM_START( psmrmagic )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mrmagic.bin", 0x0000, 0x2000, CRC(2db79ba0) SHA1(cfb9affb549cb53f949c986617ad64a328a1a300) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,  INPUT,    CLASS,          INIT,       MONITOR, COMPANY, FULLNAME,                                          FLAGS,                                                            LAYOUT
GAMEL( 1991, psaladdin2, 0,      aladdin2, aladdin2, aladdin2_state, empty_init, 0,       "Sammy", "Aladdin 2 (3rd Generation, 1st model, Type A-C)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL|MACHINE_IS_INCOMPLETE), layout_aladdin2 )
GAMEL( 1992, psmrmagic,  0,      aladdin2, aladdin2, aladdin2_state, empty_init, 0,       "Sammy", "Mr.Magic (3rd Generation, 2nd model, Type A-C)",  (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL|MACHINE_IS_INCOMPLETE), layout_mrmagic )
