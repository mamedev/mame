// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Niigata Denshi Seiki Pachi-Slot Series Driver
    Z80 + 3-reel + YM2149 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2024-06-10.

    Supported Games:
    - Reno (c) 1990 Niigata Denshi Seiki  (3rd Generation, 1st model, Type A)
    - Reno (bootleg / modified ROM)

    Hardware:
    - Main CPU: Z80 @ provisional 4.368750 MHz (8.7375 MHz XTAL / 2)
    - Sound: YM2149 @ provisional 1.0921875 MHz (8.7375 MHz XTAL / 8)
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 512 bytes battery-backed SRAM (0x2000-0x21ff)

    TODO:
    - Turning the setting key on does not display the current setting value.
      Real hardware behavior is still unknown, but the current result looks
      suspicious and should be re-checked.
    - Confirm the physical crystal frequency from genuine hardware.

    NOTE:
    [ROM Lineage and Modifications]
    - This driver currently includes both the original program ROM and one
      unauthorized modified ROM image ("ura ROM") found in the field.
      The modified image is treated as a bootleg/modification rather than
      an official manufacturer release.

    [Periodic IRQ and Clock Pacing]
    - The main periodic interrupt is driven at 500 Hz (irq0_line_hold).
      Pacing and reel motor timing align with verified gameplay behavior.

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
    - Total IN (bit 0) and Total OUT (bit 1) pulse outputs driven by port 0x06 (signal_latch_w)
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

    [Setting Change Procedure]
    - Standard 2nd/3rd generation sequence:
      1. Press the 'P' key to assert power-fail preparation.
      2. Turn Setting Key (F2) on and restart/reset machine.
      3. Select desired setting (1-6) on 7-segment display with Door Reset / CSV button (F1).
      4. Confirm setting value by operating Start Lever.
      5. Turn Setting Key off to enter normal game state.

    [Game Rules & Options]
    - Automatic game-stop (UCHIDOME) is enforced by program logic upon
      BIG BONUS completion. Cancelled via the Door Reset / CSV button.
    - No configurable SEISAN / UCHIDOME options are available.

***************************************************************************/

#include "emu.h"

#include "pachislotdev.h"

#include "cpu/z80/z80.h"
#include "machine/steppers.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "speaker.h"


namespace {

// internal artwork
#include "reno.lh"

class reno_state : public driver_device
{
public:
	reno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pachislot(*this, "pachislot")
		, m_ym2149(*this, "ym2149")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_lamp_frame(*this, "lamp_frame%u", 0U)
		, m_lay_lamp_777(*this, "lamp_777%u", 0U)
		, m_lay_lamp_jac(*this, "lamp_jac%u", 0U)
		, m_lay_lamp_medals(*this, "lamp_medals")
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void reno(machine_config &config);

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
	output_finder<6> m_lay_lamp_frame;
	output_finder<3> m_lay_lamp_777;
	output_finder<6> m_lay_lamp_jac;
	output_finder<> m_lay_lamp_medals;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	void reel_phase_w(offs_t offset, u8 data);
	void lamp_latch_bet_w(offs_t offset, u8 data);
	void lamp_latch_frame_w(offs_t offset, u8 data);
	void lamp_latch_jac_w(offs_t offset, u8 data);
	void seg_control_w(offs_t offset, u8 data);
	void signal_latch_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	u8 m_opto[3];
	u8 m_segdata[4];
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;
};


void reno_state::machine_start()
{
	save_item(NAME(m_opto));
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

void reno_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);

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
void reno_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void reno_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void reno_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

INPUT_CHANGED_MEMBER(reno_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
	}
}

u8 reno_state::switch_sense_a_r(offs_t offset)
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

u8 reno_state::switch_sense_b_r(offs_t offset)
{
	// bit0: medal selector sensor 0
	// bit1: medal hopper sensor 0
	// bit2: reset / door key / change setting value button
	// bit3: setting key
	// bit4: unknown
	// bit5: bet button
	// bit6: payout / credit game button
	// bit7: unknown

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[1]->read();
}

void reno_state::reel_phase_w(offs_t offset, u8 data)
{
	// bit0-3: reel 1-3 pulse bit
	// bit4-7: unused
	m_reel[offset]->write(data & 0x0f);
}

void reno_state::lamp_latch_bet_w(offs_t offset, u8 data)
{
	// bit0: bet medal(s) line lamp 1
	// bit1: bet medal(s) line lamp 2 (lower)
	// bit2: bet medal(s) line lamp 2 (upper)
	// bit3: bet medal(s) line lamp 3 (lower)
	// bit4: bet medal(s) line lamp 3 (upper)
	// bit5: reel 3 stop button lamp (right)
	// bit6: reel 2 stop button lamp (center)
	// bit7: reel 1 stop button lamp (left)

	// bit0-4: bet line lamp (physical upper/lower pairs)
	m_lay_lamp_betline[0] = BIT(data, 0);				// 1 medal
	m_lay_lamp_betline[1] = BIT(data, 2);				// 2 medals (lower)
	m_lay_lamp_betline[2] = BIT(data, 1);				// 2 medals (upper)
	m_lay_lamp_betline[3] = BIT(data, 4);				// 3 medals (lower)
	m_lay_lamp_betline[4] = BIT(data, 3);				// 3 medals (upper)

	// bit5-7: stop lamp
	m_lay_lamp_stop[0] = BIT(data, 7);				// left
	m_lay_lamp_stop[1] = BIT(data, 6);				// center
	m_lay_lamp_stop[2] = BIT(data, 5);				// right
}

void reno_state::lamp_latch_frame_w(offs_t offset, u8 data)
{
	// bit0: "MEDALS" lamp
	// bit1: frame lamp 3,4 (--oo--)
	// bit2: frame lamp 2,5 (-o--o-)
	// bit3: frame lamp 1,6 (o----o)
	// bit4: bonus count "777" lamp (right)
	// bit5: bonus count "777" lamp (center)
	// bit6: bonus count "777" lamp (left)
	// bit7: unknown

	m_lay_lamp_medals = BIT(data, 0);

	// frame lamp
	m_lay_lamp_frame[0] = m_lay_lamp_frame[5] = BIT(data, 3);	// frame lamp 1,6 (o----o)
	m_lay_lamp_frame[1] = m_lay_lamp_frame[4] = BIT(data, 2);	// frame lamp 2,5 (-o--o-)
	m_lay_lamp_frame[2] = m_lay_lamp_frame[3] = BIT(data, 1);	// frame lamp 3,4 (--oo--)

	// bonus count "777" lamp
	m_lay_lamp_777[0] = BIT(data, 6);				// left
	m_lay_lamp_777[1] = BIT(data, 5);				// center
	m_lay_lamp_777[2] = BIT(data, 4);				// right
}

void reno_state::lamp_latch_jac_w(offs_t offset, u8 data)
{
	// bit0-1: unknown
	// bit2-7: JAC game count lamps (1-6)
	m_lay_lamp_jac[0] = BIT(data, 7);				// JAC game count lamp 1
	m_lay_lamp_jac[1] = BIT(data, 6);				// JAC game count lamp 2
	m_lay_lamp_jac[2] = BIT(data, 5);				// JAC game count lamp 3
	m_lay_lamp_jac[3] = BIT(data, 4);				// JAC game count lamp 4
	m_lay_lamp_jac[4] = BIT(data, 3);				// JAC game count lamp 5
	m_lay_lamp_jac[5] = BIT(data, 2);				// JAC game count lamp 6
}

void reno_state::seg_control_w(offs_t offset, u8 data)
{
	// bit0-3: SEG BCD data bit
	// bit4-7: SEG common select bit

	static const u8 segdigit[16] = 
	{
		//.gfedcba
		0b00111111,						// 0x00
		0b00000110,						// 0x01
		0b01011011,						// 0x02
		0b01001111,						// 0x03
		0b01100110,						// 0x04
		0b01101101,						// 0x05
		0b01111101,						// 0x06
		0b00000111,						// 0x07
		0b01111111,						// 0x08
		0b01101111,						// 0x09
		0,							// 0x0a
		0,							// 0x0b
		0,							// 0x0c
		0,							// 0x0d
		0,							// 0x0e
		0							// 0x0f
	};

	// SEG common bit -> SEG select
	for (u8 i = 0; i < 4; i++)
		if (BIT(data, i + 4))
			m_segdata[i] = data & 0x0f;

	// SEG credit (SEG bit value -> SEG digit data)
	m_lay_seg_credit[0]	= segdigit[m_segdata[0]];		// credit SEG high
	m_lay_seg_credit[1]	= segdigit[m_segdata[1]];		// credit SEG low
	// SEG payout (SEG bit value -> SEG digit data)
	m_lay_seg_payout[0]	= segdigit[m_segdata[2]];		// payout SEG high
	m_lay_seg_payout[1]	= segdigit[m_segdata[3]];		// payout SEG low
}

void reno_state::signal_latch_w(offs_t offset, u8 data)
{
	// bit0:   total IN pulse out
	// bit1:   total OUT pulse out
	// bit2:   JAC game signal out
	// bit3:   BIG bonus signal out
	// bit4:   medal lockout solenoid
	// bit5:   hopper motor drive
	// bit6-7: unknown

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
}

void reno_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram().share("nvram");
	map(0x4000, 0x4000).r(FUNC(reno_state::switch_sense_a_r));
	map(0x6000, 0x6000).r(FUNC(reno_state::switch_sense_b_r));
	map(0x8000, 0x8000).nopw();
}

void reno_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x02).w(FUNC(reno_state::reel_phase_w));
	map(0x03, 0x03).w(FUNC(reno_state::lamp_latch_bet_w));
	map(0x04, 0x04).w(FUNC(reno_state::lamp_latch_frame_w));
	map(0x05, 0x05).w(FUNC(reno_state::lamp_latch_jac_w));
	map(0x06, 0x06).w(m_ym2149, FUNC(ym2149_device::address_w));
	map(0x07, 0x07).nopw();
	map(0x0e, 0x0e).w(m_ym2149, FUNC(ym2149_device::data_w));
}


static INPUT_PORTS_START( reno )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(reno_state::opto_r<0>))	// reel 1 sensor (left)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(reno_state::opto_r<1>))	// reel 2 sensor (center)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(reno_state::opto_r<2>))	// reel 3 sensor (right)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM )	// PFI signal in (ready to return from power-fail)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(reno_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(reno_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Door Key / Reset Button / Change Setting Value Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(reno_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void reno_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void reno_state::reno(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8'737'500 / 2); // XTAL(8'737'500) / 2 = 4.368750 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &reno_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &reno_state::io_map);

	// IRQ frequency is currently estimated from observed gameplay behavior
	// such as reel speed, lamp timing, and overall game pacing.
	// It is not measured from real hardware and may require adjustment.
	m_maincpu->set_periodic_int(FUNC(reno_state::irq0_line_hold), attotime::from_hz(500));

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(60*2);
	m_pachislot->hopper_cb().set(FUNC(reno_state::playing_hopper_samples));

	// Configure sensor emulation modes: single-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SINGLE);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], PIONEER_200STEP_REEL, 51, 53, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(reno_state::opto_cb<0>));
	REEL(config, m_reel[1], PIONEER_200STEP_REEL, 51, 53, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(reno_state::opto_cb<1>));
	REEL(config, m_reel[2], PIONEER_200STEP_REEL, 51, 53, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(reno_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2149(config, m_ym2149, 8'737'500 / 8).add_route(ALL_OUTPUTS, "speaker", 0.4); // XTAL(8'737'500) / 8 = 1.0921875 MHz
	m_ym2149->port_a_write_callback().set(FUNC(reno_state::seg_control_w));
	m_ym2149->port_b_write_callback().set(FUNC(reno_state::signal_latch_w));

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psreno )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "reno.bin", 0x0000, 0x2000, CRC(7452bf51) SHA1(ac400b79fc04ab8e1885291f97c391648f8e902c) )
ROM_END

ROM_START( psrenoa )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "reno_b01.bin", 0x0000, 0x2000, CRC(b6a26c5d) SHA1(1a900d9d5d200b85401e53e529199f3dff5d5e87) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,    PARENT, MACHINE, INPUT, CLASS,      INIT,       MONITOR, COMPANY,                FULLNAME,                                             FLAGS,                                      LAYOUT
GAMEL( 1990, psreno,  0,      reno,    reno,  reno_state, empty_init, 0,       "Niigata Denshi Seiki", "Reno (3rd Generation, 1st model, Type A)",           (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_reno )
GAMEL( 1990, psrenoa, psreno, reno,    reno,  reno_state, empty_init, 0,       "unknown",              "Reno (3rd Generation, 1st model, Type A, bootleg)",  (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_reno )
