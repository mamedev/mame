// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Yamasa Pachi-Slot Series Driver
    LE1080A(V1) + 3-reel + 4th-reel + Z80 + YM2413 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2025-05-02.

    Supported Games:
    - Oicho KABA X  (c) 1999 Yamasa  (4th Generation, Type A-400)

    Hardware:
    - Main CPU: LE1080A-PA @ provisional 4.0 MHz (8.0 MHz XTAL / 2)
    - Sound CPU: Z80 @ provisional 4.0 MHz (8.0 MHz XTAL / 2)
    - Sound: YM2413 @ 3.579545 MHz
    - Stepper Motors: 3-reel (252-step) + 1-reel (tetra/4th, 200-step, 2-phase unipolar)
    - NVRAM: Internal LE1080A battery-backed RAM

    TODO:
    - LE1080A interrupt addresses and internal random-number behavior are
      still not fully understood, so some model-specific behavior remains
      implemented in the driver rather than in le1080a.cpp.

    - IRQ0 / IRQ1 / power-fail preparation interrupt addresses differ by
      machine and manufacturer.
      These are currently assigned from analysis results in each program
      rather than being derived automatically by the LE1080A core.

    - Medal payout operates correctly, but Credit Off mode cannot be toggled.
      Verify against genuine hardware whether this machine actually supports
      disabling credit mode. Because of this, the payout/credit button LED
      is likely lit continuously on physical hardware; the exact output bit
      controlling this LED has not been confirmed yet, leaving it unmapped
      (unlit) in the layout for now.

    NOTE:
    [LE-Tech LE1080A CPU & Interrupt Architecture]
    - For hardware specifications, system register details (0xFF00-0xFF3F),
      memory maps, and power-fail interrupt logic of the LE-Tech LE1080A
      Z80-compatible core, see the CPU core driver: cpu/z80/le1080a.cpp.

    [Periodic IRQ and Clock Pacing]
    - The main gameplay loop is serviced by LE1080A IRQ1 at 640 Hz.
      (Provisional frequency calibrated against physical reel revolution period).
    - Audio CPU NMI pacing is driven at 2500 Hz.

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

    [Bookkeeping / Mechanical Counters]
    - Total IN (bit 7) and Total OUT (bit 6) pulse outputs driven by port 0xeffb
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

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
#include "oichokabax.lh"

class oichokaba_state : public driver_device
{
public:
	oichokaba_state(const machine_config &mconfig, device_type type, const char *tag)
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
		, m_lay_led_frame(*this, "led_frame%u", 0U)
		, m_lay_led_betline(*this, "led_betline%u", 0U)
		, m_lay_led_stop(*this, "led_stop%u", 0U)
		, m_lay_led_payout(*this, "led_payout")
		, m_lay_led_insert(*this, "led_insert")
		, m_lay_led_maxbet(*this, "led_maxbet")
		, m_lay_led_start(*this, "led_start")
		, m_lay_led_counter(*this, "led_counter%u", 0U)
		, m_lay_led_jac(*this, "led_jac%u", 0U)
		, m_lay_led_status(*this, "led_status%u", 0U)
		, m_lay_led_reel3(*this, "led_reel3_%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void oichokaba(machine_config &config) ATTR_COLD;

	void init_oichokaba();

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
	void oichokaba_mem_map(address_map &map) ATTR_COLD;
	void audio_mem_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;

private:
	required_device<le1080a_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<ym2413_device> m_ym2413;
	required_device<pachislot_device> m_pachislot;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 4> m_reel;
	required_ioport_array<3> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<8> m_lay_led_frame;
	output_finder<5> m_lay_led_betline;
	output_finder<3> m_lay_led_stop;
	output_finder<> m_lay_led_payout;
	output_finder<> m_lay_led_insert;
	output_finder<> m_lay_led_maxbet;
	output_finder<> m_lay_led_start;
	output_finder<12> m_lay_led_counter;
	output_finder<3> m_lay_led_jac;
	output_finder<2> m_lay_led_status;
	output_finder<3> m_lay_led_reel3;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 le1080a_r(offs_t offset);
	void le1080a_w(offs_t offset, u8 data);

	template <unsigned N> u8 rng_r(offs_t offset);
	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	u8 switch_sense_c_r(offs_t offset);
	void drive_latch_a_w(offs_t offset, u8 data);
	void drive_latch_b_w(offs_t offset, u8 data);
	void drive_latch_c_w(offs_t offset, u8 data);
	void signal_and_comm_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void led_data_w(offs_t offset, u8 data);
	void lamp_latch_bet_status_w(offs_t offset, u8 data);
	void tetra_backlight_w(offs_t offset, u8 data);

	INTERRUPT_GEN_MEMBER(sound_nmi);
	void soundcode_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;
	bool m_powerfail_status_ready;

	u8 m_opto[4];
	u8 m_segdata[4];
	u8 m_segdata_pending;
	u8 m_segcommon;
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;

	u8 m_led_data_pending;
	u8 m_led_comm_latch[4];
};


void oichokaba_state::init_oichokaba()
{
	// LE1080A Interrupt address set
	m_maincpu->set_irq1_address(0x0084);
	m_maincpu->set_irq2_address(0x0066);

	// Provisional frequency calibrated against physical reel revolution period.
	// Internal timer configuration register behavior is not yet fully documented.
	m_maincpu->set_irq1_rate_hz(640);
}

void oichokaba_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata_pending));
	save_item(NAME(m_segdata));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_led_data_pending));
	save_item(NAME(m_led_comm_latch));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void oichokaba_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_segdata_pending = 0;
	m_led_data_pending = 0;
	std::fill(std::begin(m_led_comm_latch), std::end(m_led_comm_latch), 0);

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
void oichokaba_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 4; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
	m_powerfail_status_ready = true;
}

void oichokaba_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 4; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void oichokaba_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

INTERRUPT_GEN_MEMBER(oichokaba_state::sound_nmi)
{
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER(oichokaba_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_maincpu->trigger_power_fail();
	}
}

template <unsigned N>
u8 oichokaba_state::rng_r(offs_t offset)
{
	// bit0-7: Read once after lever ON and before reel rotation. Possibly LE1080A internal random source.
	return m_maincpu->internal_rng_r(N);
}

u8 oichokaba_state::switch_sense_a_r(offs_t offset)
{
	// bit0: setting key
	// bit1: change setting value button
	// bit2: start lever
	// bit3: hopper medal out sensor
	// bit4: medal in sensor 2
	// bit5: medal in sensor 1
	// bit6: payout button
	// bit7: door key / reset button

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[0]->read();
}

u8 oichokaba_state::switch_sense_b_r(offs_t offset)
{
	// bit0: reel 1 stop button (left)
	// bit1: reel 2 stop button (center)
	// bit2: reel 3 stop button (right)
	// bit3: reel 1 sensor (left)
	// bit4: reel 2 sensor (center)
	// bit5: reel 3 sensor (right)
	// bit6: reel 4 sensor (tetra)
	// bit7: unknown

	return m_in_port[1]->read();
}

u8 oichokaba_state::switch_sense_c_r(offs_t offset)
{
	// bit0:   bet 1 button
	// bit1:   maxbet button
	// bit2:   "UCHIDOME" ("打ち止め") reset switch
	// bit3:   "UCHIDOME" ("打ち止め") switch (automatic game-stop)
	// bit4-7: unknown

	return m_in_port[2]->read();
}

void oichokaba_state::drive_latch_a_w(offs_t offset, u8 data)
{
	// bit0-3: reel 1 pulse bit (center)
	// bit4-7: reel 2 pulse bit (left)

	// bit0-3: reel 1 pulse bit (center)
	m_reel[1]->write((data & 0xff) >> 4);
	// bit4-7: reel 2 pulse bit (left)
	m_reel[0]->write(data & 0x0f);
}

void oichokaba_state::drive_latch_b_w(offs_t offset, u8 data)
{
	// bit0-3: reel 3 pulse bit (right)
	// bit4-7: reel 4 pulse bit (tetra)

	// bit0-3: reel 3 pulse bit (right)
	m_reel[3]->write((data & 0xf0) >> 4);
	// bit4-7: reel 4 pulse bit (tetra)
	m_reel[2]->write(data & 0x0f);
}

void oichokaba_state::drive_latch_c_w(offs_t offset, u8 data)
{
	// bit0-5: frame LED
	// bit6:   medal lockout solenoid
	// bit7:   hopper motor signal

	// frame LED upper left 1
	m_lay_led_frame[0] = BIT(data, 3);
	// frame LED upper left 2
	m_lay_led_frame[1] = BIT(data, 4);
	// frame LED upper center
	m_lay_led_frame[2] = BIT(data, 5);
	// frame LED upper right 2
	m_lay_led_frame[3] = BIT(data, 4);
	// frame LED upper right 1
	m_lay_led_frame[4] = BIT(data, 3);

	// frame LED side upper
	m_lay_led_frame[5] = BIT(data, 2);
	// frame LED side center
	m_lay_led_frame[6] = BIT(data, 1);
	// frame LED side lower
	m_lay_led_frame[7] = BIT(data, 0);

	// bit6: medal lockout solenoid
	m_solenoid = !BIT(data, 6);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	// bit7: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	m_hopper = BIT(data, 7);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;
}

void oichokaba_state::signal_and_comm_w(offs_t offset, u8 data)
{
	// bit0-3: SEG common bit / LED common bit
	// bit4:   JAC game signal out
	// bit5:   BIG bonus signal out
	// bit6:   total OUT pulse out
	// bit7:   total IN pulse out

	for (u8 i = 0; i < 4; i++)
		if (BIT(data, i))
			m_segdata[i] = m_segdata_pending;

	// SEG credit
	m_lay_seg_credit[0] = m_segdata[2];			// SEG left
	m_lay_seg_credit[1] = m_segdata[3];			// SEG right
	// SEG payout
	m_lay_seg_payout[0] = m_segdata[0];			// SEG left
	m_lay_seg_payout[1] = m_segdata[1];			// SEG right

	// bit4: JAC game signal out
	m_lay_signal_jac = BIT(data, 4);
	// bit5: BIG bonus signal out
	m_lay_signal_big = BIT(data, 5);
	// bit6: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));
	// bit7: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));

	// common-0 bit0-3: game count LED 9-10, 11(+10), 12(+20)
	// common-0 bit4-6: JAC count <RED> LED 1-3
	// common-0 bit7:   "GAME OVER" LED
	// common-1 bit0-7: game count LED 1-8
	// common-2 bit0:   reel stop button LED left
	// common-2 bit1:   reel stop button LED center
	// common-2 bit2:   reel stop button LED right
	// common-2 bit3-7: unused ?
	// common-4 bit0:   medal in LED
	// common-4 bit1:   start LED
	// common-4 bit2-3: unused ?
	// common-4 bit4-6: JAC count <BLUE> LED 1-3

	// Capture 0xeffd for each asserted bit of SEG common 0,1,2,4
	for (u8 i = 0; i < 4; i++)
		if (BIT(data, i))
			m_led_comm_latch[i] = m_led_data_pending;

	// common 0 bit0-3: game count LED 9-10, 11(+10), 12(+20)
	for (u8 i = 0; i < 4; i++)
		m_lay_led_counter[i + 8] = BIT(m_led_comm_latch[0], i);
	// common 0 bit4-6: JAC count <RED> 1-3 (+ common 4 4-6: JAC count <BLUE> 1-3)
	for (u8 i = 0; i < 3; i++)
		m_lay_led_jac[i] = BIT(m_led_comm_latch[0], i + 4) | (BIT(m_led_comm_latch[3], i + 4) << 1);
	// common 0 bit7: "GAME OVER" LED
	m_lay_led_status[1] = BIT(m_led_comm_latch[0], 7);

	// common 1 bit0-7: game count LED 1-8
	for (u8 i = 0; i < 8; i++)
		m_lay_led_counter[i] = BIT(m_led_comm_latch[1], i);

	// common 2 bit0-2: reel stop button LED bit0: left, bit1: center, bit2: right
	for (u8 i = 0; i < 3; i++)
		m_lay_led_stop[i] = BIT(m_led_comm_latch[2], i);

	// common 4 bit0: medal in LED
	m_lay_led_insert = BIT(m_led_comm_latch[3], 0);
	// common 4 bit1: start LED
	m_lay_led_start = BIT(m_led_comm_latch[3], 1);
}

void oichokaba_state::seg_data_w(offs_t offset, u8 data)
{
	// bit0-7: SEG common bit
	m_segdata_pending = data;
}

void oichokaba_state::led_data_w(offs_t offset, u8 data)
{
	// bit0-7: SEG / game counter / JAC count / game over / medal in / start LED / etc. data
	m_led_data_pending = data;
}

void oichokaba_state::lamp_latch_bet_status_w(offs_t offset, u8 data)
{
	// bit0-4: bet medal(s) line LED (physical upper/lower pairs)
	// bit5:   replay("TROUBLE") LED
	// bit6:   unknown, continuously held high (candidate payout button LED?)
	// bit7:   maxbet button LED

	// bit0-4: bet medal(s) line LED (physical upper/lower pairs)
	for (int i = 0; i < 5; i++)
		m_lay_led_betline[i] = BIT(data, i);

	// bit5: replay("TROUBLE") LED
	m_lay_led_status[0] = BIT(data, 5);

//	// bit6: payout / credit game button LED (unconfirmed)
//	m_lay_led_payout = BIT(data, 6);

	// bit7: maxbet button LED
	m_lay_led_maxbet = BIT(data, 7);
}

void oichokaba_state::tetra_backlight_w(offs_t offset, u8 data)
{
	// bit0-1: unknown (appears to stay high?)
	// bit2-4: tetra(4th) reel backlight LED (top, middle, bottom)
	// bit5-7: unknown

	// bit2-4: tetra(4th) reel backlight LED
	m_lay_led_reel3[0] = BIT(data, 2);			// top
	m_lay_led_reel3[1] = BIT(data, 3);			// middle
	m_lay_led_reel3[2] = BIT(data, 4);			// bottom
}

void oichokaba_state::soundcode_w(offs_t offset, u8 data)
{
	// soundcode write
	m_soundlatch->write(data ^ 0xff);
}

u8 oichokaba_state::le1080a_r(offs_t offset)
{
	u8 port = 0xff;
	return port;
}

void oichokaba_state::le1080a_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x18:
			soundcode_w(0, data);
			break;
	}
}

void oichokaba_state::le1080a_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}

void oichokaba_state::oichokaba_mem_map(address_map &map)
{
	le1080a_mem_map(map);

	map(0xeff8, 0xeff8).r(FUNC(oichokaba_state::rng_r<0>)).w(FUNC(oichokaba_state::drive_latch_a_w));
	map(0xeff9, 0xeff9).r(FUNC(oichokaba_state::rng_r<1>)).w(FUNC(oichokaba_state::drive_latch_b_w));
	map(0xeffa, 0xeffa).r(FUNC(oichokaba_state::rng_r<2>)).w(FUNC(oichokaba_state::drive_latch_c_w));
	map(0xeffb, 0xeffb).r(FUNC(oichokaba_state::switch_sense_a_r)).w(FUNC(oichokaba_state::signal_and_comm_w));
	map(0xeffc, 0xeffc).r(FUNC(oichokaba_state::switch_sense_b_r)).w(FUNC(oichokaba_state::seg_data_w));
	map(0xeffd, 0xeffd).r(FUNC(oichokaba_state::switch_sense_c_r)).w(FUNC(oichokaba_state::led_data_w));
	map(0xeffe, 0xeffe).w(FUNC(oichokaba_state::lamp_latch_bet_status_w));
	map(0xefff, 0xefff).w(FUNC(oichokaba_state::tetra_backlight_w));
}

void oichokaba_state::audio_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0xfe00, 0xffff).ram();
}

void oichokaba_state::audio_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xfe, 0xfe).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xfe, 0xff).w(m_ym2413, FUNC(ym2413_device::write));
}


static INPUT_PORTS_START( oichokaba )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(oichokaba_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(oichokaba_state::selector_r<pachislot_device::PACHISLOT_SENSOR1>))	// medal selector sensor 2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(oichokaba_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 1
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Door Key")

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(oichokaba_state::opto_r<0>))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(oichokaba_state::opto_r<1>))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(oichokaba_state::opto_r<2>))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(oichokaba_state::opto_r<3>))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// Cabinet option switches and other inputs
	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Max Bet Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("Bet 1 Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("UCHIDOME Reset Button")
	PORT_CONFNAME( 0x08, 0x00, "UCHIDOME" )
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(oichokaba_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void oichokaba_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void oichokaba_state::oichokaba(machine_config &config)
{
	// basic machine hardware
	LE1080A(config, m_maincpu, XTAL(8_MHz_XTAL) / 2); // 8 MHz / 2 = 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &oichokaba_state::oichokaba_mem_map);
	// Install custom handlers for LE1080A internal system registers (0xff18-0xff19):
	// - Offset 0x18 (0xff18): Write-only latch to output soundcode data to the sound CPU
	// - Offset 0x19 (0xff19): Read-only port to sample cabinet switches (UCHIDOME / SEISAN)
	m_maincpu->install_sysreg_handler(0x18, 0x18, read8sm_delegate(*this, FUNC(oichokaba_state::le1080a_r)), write8sm_delegate(*this, FUNC(oichokaba_state::le1080a_w)));

	Z80(config, m_audiocpu, XTAL(8_MHz_XTAL) / 2); // 8 MHz / 2 = 4 MHz ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &oichokaba_state::audio_mem_map);
	m_audiocpu->set_addrmap(AS_IO, &oichokaba_state::audio_io_map);
	// Audio CPU NMI frequency is currently estimated from observed sound
	// playback timing and overall behavior, not from measured hardware.
	// It may require adjustment when more accurate information becomes available.
	m_audiocpu->set_periodic_int(FUNC(oichokaba_state::sound_nmi), attotime::from_hz(2500));

	GENERIC_LATCH_8(config, m_soundlatch);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(55*2);
	m_pachislot->hopper_cb().set(FUNC(oichokaba_state::playing_hopper_samples));

	// medal selector device type & hopper sensor device type setting
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_YAMASA1);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], YAMASA_252STEP_REEL, 456, 458, 0x00, 0, 252*2);
	m_reel[0]->optic_handler().set(FUNC(oichokaba_state::opto_cb<0>));
	REEL(config, m_reel[1], YAMASA_252STEP_REEL, 456, 458, 0x00, 0, 252*2);
	m_reel[1]->optic_handler().set(FUNC(oichokaba_state::opto_cb<1>));
	REEL(config, m_reel[2], YAMASA_252STEP_REEL, 456, 458, 0x00, 0, 252*2);
	m_reel[2]->optic_handler().set(FUNC(oichokaba_state::opto_cb<2>));

	// reel device tetra(4th)
	REEL(config, m_reel[3], YAMASA_200STEP_REEL, 341, 343, 0x00, 0, 200*2);
	m_reel[3]->optic_handler().set(FUNC(oichokaba_state::opto_cb<3>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 3.57_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.0);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psoichokab )
	ROM_REGION( 0x10000, "le1080a", ROMREGION_ERASE00 )
	ROM_LOAD( "oichokabax_main.bin", 0x0000, 0x2000, CRC(f8da5575) SHA1(f4323262836c21f9d10629893247dbc89517ebcd) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "oichokabax_music.bin", 0x0000, 0x4000, CRC(2626157b) SHA1(10ce0cf96ef92168c26fd1b8e381eb4dab5a1977) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,     INPUT,       CLASS,             INIT,           MONITOR, COMPANY,  FULLNAME,                                    FLAGS,                                      LAYOUT
GAMEL( 1999, psoichokab, 0,      oichokaba,   oichokaba,   oichokaba_state,   init_oichokaba, 0,       "Yamasa", "Oicho KABA X (4th Generation, Type A-400)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_oichokabax )
