// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Universal Group Pachi-Slot Series Driver
    Z80 + 3-reel + YM2413 titles (Universal V0 Hardware Architecture)

    Driver by Chiaki Tachibana

    Initial driver development started on 2024-04-12.

    Supported Games:
    - Liberty Bell 3    (c) 1989 Macy                (2nd Generation, 2nd model, Type A)
    - Century 21        (c) 1989 Mizuho              (2nd Generation, 2nd model, Type A)
    - Liberty Bell 4    (c) 1990 Universal           (2nd Generation, 2nd model, Type A)
    - Rock'n Roll       (c) 1990 AI (Universal OEM)  (2nd Generation, 2nd model, Type A)
    - Continental       (c) 1990 Mizuho              (3rd Generation, 1st model, Type A)
    - Continental 3     (c) 1990 Macy                (3rd Generation, 1st model, Type A-C)
    - Americana Magnum  (c) 1990 Universal           (3rd Generation, 1st model, Type A)
    - Continental 2     (c) 1990 Universal           (3rd Generation, 2nd model, Type A)

    Hardware:
    - Main CPU: TMPZ84C00AP-6 @ provisional 5.408 MHz (10.816 MHz XTAL / 2)
    - Sound: YM2413 @ 3.579545 MHz
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 512 bytes battery-backed SRAM (0x3000-0x31ff)

    TODO:
    - Liberty Bell 4 cabinet decorative LEDs:
      Not all physical Liberty Bell 4 cabinets are equipped with these decorative LEDs;
      two distinct cabinet variations exist (units without decorative LEDs and units
      equipped with them, likely attributable to production lot differences).
      There are NO differences or dedicated control routines in the firmware / main PCB
      between these variants; the feature is implemented purely through internal cabinet
      lamp wiring modifications.
      Because the LEDs cannot be addressed directly by the CPU, the driver simulates
      the cabinet-side chase sequencer by tapping into the active state of the existing
      frame lamp line.
    - Confirm whether the 10.816 MHz clock is divided by 2 across all PCB variants.

    NOTE:
    [Periodic IRQ Rate and Pacing]
    - The main interrupt is driven at 500 Hz (irq0_line_hold). Pacing, reel
      motor timing, and YM2413 music tempos closely align with verified
      real gameplay recordings.

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
      active-low PFI signal on port 0x6001 bit 7 and preserves the physical
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

    [NVRAM Cold-Start & Setting Initialization]
    - Due to firmware check routines on uninitialized NVRAM, starting with a
      blank or corrupted NVRAM may cause hardware error states or locked controls.
      Executing the standard setting sequence clears the backup memory and
      initializes the machine to normal operation.

    [Bookkeeping / Mechanical Counters]
    - Total IN (bit 6) and Total OUT (bit 7) pulse outputs from port 0x5000
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

    [Setting Change Procedures]
    - Procedure differs between early V0 models and later standard revisions:

      [Continental / Continental 2 / Continental 3 / Liberty Bell 4 / Rock'n Roll / Americana Magnum]
      1. Turn Setting Key (F2) on and reset (F3).
      2. Select desired setting (1-6) with Door Reset / CSV button (F1).
      3. Confirm setting value by operating Start Lever.
      4. Turn Setting Key (F2) off to enter normal game state.

      [Century 21 / Liberty Bell 3 (Early V0 Revision)]
      1. Turn Setting Key (F2) on and reset (F3).
      2. Select desired setting (1-6) with Door Reset / CSV button (F1).
      3. Turn Setting Key (F2) off first.
      4. Confirm setting value by operating Start Lever to enter normal game state.

    [Game Rules & Options]
    - Automatic game-stop (UCHIDOME) is enforced by program logic upon
      BIG BONUS completion. Cancelled via the Door Key / Reset control.
    - Software-configured SEISAN behavior is supported: on original machines,
      entering setting mode while holding the center stop button and
      confirming the setting disables SEISAN (standard default is enabled).
    - No physical cabinet option switches for SEISAN / UCHIDOME are present.

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
#include "libertybell3.lh"
#include "century21.lh"
#include "libertybell4.lh"
#include "rocknroll.lh"
#include "continental.lh"
#include "continental3.lh"
#include "amemag.lh"
#include "continental2.lh"

class continental_state : public driver_device
{
public:
	continental_state(const machine_config &mconfig, device_type type, const char *tag)
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
		, m_lay_lamp_gameover(*this, "lamp_gameover")
		, m_lay_lamp_insert(*this, "lamp_insert")
		, m_lay_lamp_start(*this, "lamp_start")
		, m_lay_lamp_credit(*this, "lamp_credit")
		, m_lay_lamp_payout(*this, "lamp_payout")
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_bcount(*this, "lamp_bcount%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_lamp_reel(*this, "lamp_reel%u_%u", 0U, 0U)
		, m_lay_led_button(*this, "led_button%u", 0U)
		, m_lay_led_decorate(*this, "led_decorate%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_seg_bonus(*this, "seg_bonus")
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void common(machine_config &config);
	void continental(machine_config &config);
	void rocknroll(machine_config &config);

	void init_continental();
	void init_libbell4();

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

	void common_mem_map(address_map &map) ATTR_COLD;
	void continental_mem_map(address_map &map) ATTR_COLD;
	void rocknroll_mem_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_nvram;
	required_device<pachislot_device> m_pachislot;
	required_device<ym2413_device> m_ym2413;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<2> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<8> m_lay_lamp_frame;
	output_finder<> m_lay_lamp_gameover;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_start;
	output_finder<> m_lay_lamp_credit;
	output_finder<> m_lay_lamp_payout;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_bcount;
	output_finder<3> m_lay_lamp_stop;
	output_finder<3, 3> m_lay_lamp_reel;
	output_finder<2> m_lay_led_button;
	output_finder<8> m_lay_led_decorate;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_seg_bonus;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	void drive_latch_w(offs_t offset, u8 data);
	void lamp_latch_bet_w(offs_t offset, u8 data);
	void lamp_latch_frame_w(offs_t offset, u8 data);
	void lamp_latch_misc_w(offs_t offset, u8 data);
	void bonus_seg_and_lamp_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void seg_digit_w(offs_t offset, u8 data);
	void reel_lamp_a_w(offs_t offset, u8 data);
	void reel_lamp_b_w(offs_t offset, u8 data);
	void status_lamp_w(offs_t offset, u8 data);
	u8 rocknroll_switch_sense_a_r(offs_t offset);
	u8 rocknroll_switch_sense_b_r(offs_t offset);
	void rocknroll_lamp_frame_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	TIMER_CALLBACK_MEMBER(libertybell4_led_timer_cb);
	emu_timer *m_libertybell4_led_timer = nullptr;
	u8 m_led_task = 0;
	u8 m_led_count = 0;

	int m_libertybell4_flag = 0;
	int m_libertybell4_decoled_flag = 0;

	u8 m_latch_segdata = 0;
	u8 m_segdata[4];
	u8 m_opto[3];
	u8 m_hopper = 0;
	u8 m_solenoid = 0;
	u8 m_samples_ch = 0;
};


void continental_state::init_continental()
{
	m_libertybell4_flag = 0;
}

void continental_state::init_libbell4()
{
	m_libertybell4_flag = 1;
	m_led_task = 0;

	m_libertybell4_led_timer = timer_alloc(FUNC(continental_state::libertybell4_led_timer_cb), this);
	m_libertybell4_led_timer->adjust(attotime::zero);
}


void continental_state::machine_start()
{
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_latch_segdata));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_libertybell4_flag));
	save_item(NAME(m_libertybell4_decoled_flag));
	save_item(NAME(m_led_task));
	save_item(NAME(m_led_count));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void continental_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);
	m_latch_segdata = 0;

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;

	for (int i = 0; i < 8; i++)
		m_lay_led_decorate[i] = 0;

	m_led_task = 0;
	m_led_count = 0;
	m_libertybell4_decoled_flag = 0;

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
void continental_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void continental_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void continental_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

INPUT_CHANGED_MEMBER(continental_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
	}
}

// ---------------------------------------------------------------------------
//  Liberty Bell 4 Decorative LEDs Simulation
// ---------------------------------------------------------------------------
// Note on physical cabinet variations and hardware wiring:
// Genuine Liberty Bell 4 machines exist in two distinct cabinet variations:
// units without decorative chasing LEDs and units equipped with them.
// Crucially, the firmware (ROM) and main CPU board are completely identical
// across both variants; the decorative LEDs were realized purely via internal
// cabinet lamp wiring modifications without any CPU-level I/O lines.
// Therefore, this driver provisionally models the cabinet's autonomous chase
// behavior by monitoring the activation timing of the shared frame lamp signal.
// ---------------------------------------------------------------------------
TIMER_CALLBACK_MEMBER(continental_state::libertybell4_led_timer_cb)
{
	m_libertybell4_led_timer->adjust(attotime::from_msec(20));

	switch (m_led_task)
	{
		case 0:
			if (m_libertybell4_flag && m_libertybell4_decoled_flag)
			{
				m_led_count = 0;
				m_led_task++;
			}
			break;

		case 1:
			for (int i = 0; i < 8; i++)
				m_lay_led_decorate[i] = (((m_led_count % 8) + 1 > i) ^ ((m_led_count / 8) & 1)) & 1;

			if (m_led_count++ > 15 - 1)
			{
				m_led_count = 0;

				if (!m_libertybell4_decoled_flag)
					m_led_task = 0;
			}
			break;
	}
}

u8 continental_state::switch_sense_a_r(offs_t offset)
{
	// bit0: reel 1 stop button (left)
	// bit1: reel 2 stop button (center)
	// bit2: reel 3 stop button (right)
	// bit3: medal selector sensor 0
	// bit4: reel 1 sensor (left)
	// bit5: reel 2 sensor (center)
	// bit6: reel 3 sensor (right)
	// bit7: medal hopper sensor 0

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[0]->read();
}

u8 continental_state::switch_sense_b_r(offs_t offset)
{
	// bit0: start lever
	// bit1: reset / door key / change setting value button
	// bit2: subtank medal overflow sensor
	// bit3: unknown
	// bit4: setting key
	// bit5: bet button
	// bit6: payout / credit game button
	// bit7: PFI signal in (ready to return from power-fail)

	u8 port = m_in_port[1]->read();

	if (m_powerfail_status_ready)
		port &= ~0x80;

	// *** Don't accept start lever while [Pay Out / Credit Game] button is pressed ***
	// 
	// NOTE:
	// Prevent simultaneous Start Lever actuation while the Payout/Credit button
	// is held down. On early factory production units, this hardware loophole
	// allowed players to trigger reel spin and credit clearance in the same frame
	// (a famous real-world exploit known as the "21st Century World Annihilation Slot Strategy").
	// This interlock simulates the subsequent field-retrofit fix installed by operators.
	if (BIT(port, 6))
		port |= (1U << 0);

	return port;
}

void continental_state::drive_latch_w(offs_t offset, u8 data)
{
	// 0x5000-0x5002 bit0-3: reel pulse bit
	m_reel[offset]->write(data & 0x0f);

	// 0x5000-0x5002 bit4-7: auxiliary drive signals
	switch (offset)
	{
		case 0:
			// 0x5000 bit6: total IN pulse out (drives electromechanical counter)
			machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
			// 0x5000 bit7: total OUT pulse out (drives electromechanical counter)
			machine().bookkeeping().coin_counter_w(1, BIT(data, 7));
			break;

		case 1:
			// 0x5001 bit4-5: hopper motor drive (bit5: forward payout, bit4: reverse anti-jamming)
			if (BIT(data, 5))
			{
				m_hopper = 1;
				m_lay_signal_hopper = 1;
			}
			else if (BIT(data, 4))
			{
				m_hopper = 1;
				m_lay_signal_hopper = 2;
			}
			else
			{
				m_hopper = 0;
				m_lay_signal_hopper = 0;
			}
			m_pachislot->hopper_motor_w(m_hopper);

			// 0x5001 bit7: medal lockout solenoid
			m_solenoid = !BIT(data, 7);
			m_pachislot->selector_lock_w(m_solenoid);
			m_lay_signal_solenoid = m_solenoid;
			break;

		case 2:
			// 0x5002 bit6: JAC game signal out
			m_lay_signal_jac = BIT(data, 6);
			// 0x5002 bit7: BIG bonus signal out
			m_lay_signal_big = BIT(data, 7);
			break;
	}
}

void continental_state::lamp_latch_bet_w(offs_t offset, u8 data)
{
	// bit0-4: bet line lamps (physical upper/lower pairs)
	m_lay_lamp_betline[0] = BIT(data, 0);			// 1 medal
	m_lay_lamp_betline[1] = BIT(data, 1);			// 2 medals (upper)
	m_lay_lamp_betline[2] = BIT(data, 2);			// 2 medals (lower)
	m_lay_lamp_betline[3] = BIT(data, 3);			// 3 medals (upper)
	m_lay_lamp_betline[4] = BIT(data, 4);			// 3 medals (lower)
}

void continental_state::lamp_latch_frame_w(offs_t offset, u8 data)
{
	// bit0-7: frame top lamp (Liberty Bell 4 uses bits 4 and 5 differently)
	m_lay_lamp_frame[0] = BIT(data, 0);
	m_lay_lamp_frame[1] = BIT(data, 1);
	m_lay_lamp_frame[2] = BIT(data, 2);
	m_lay_lamp_frame[3] = BIT(data, 3);

	if (!m_libertybell4_flag)
	{
		m_lay_lamp_frame[4] = BIT(data, 4);
		m_lay_lamp_frame[5] = BIT(data, 5);
	}
	else
	{
		m_lay_lamp_frame[4] = BIT(data, 2);
		m_lay_lamp_frame[5] = BIT(data, 3);
		m_libertybell4_decoled_flag = BIT(data, 5);
	}

	m_lay_lamp_frame[6] = BIT(data, 6);
	m_lay_lamp_frame[7] = BIT(data, 7);
}

void continental_state::lamp_latch_misc_w(offs_t offset, u8 data)
{
	// bit0:   reel stop button lamp (left)
	// bit1:   reel stop button lamp (center)
	// bit2:   reel stop button lamp (right)
	// bit3-5: bonus count lamps (1-3)
	// bit6:   bet button LED
	// bit7:   payout / credit game button LED
	m_lay_lamp_stop[0] = BIT(data, 0);
	m_lay_lamp_stop[1] = BIT(data, 1);
	m_lay_lamp_stop[2] = BIT(data, 2);

	m_lay_lamp_bcount[0] = BIT(data, 3);
	m_lay_lamp_bcount[1] = BIT(data, 4);
	m_lay_lamp_bcount[2] = BIT(data, 5);

	m_lay_led_button[1] = BIT(data, 6);
	m_lay_led_button[0] = BIT(data, 7);
}

void continental_state::bonus_seg_and_lamp_w(offs_t offset, u8 data)
{
	// bit0-6: bonus count 7-segment display pattern
	// bit7:   reel lamp x:2, y:2
	m_lay_seg_bonus = bitswap<8>(data, 7,0,1,2,3,4,5,6) & 0x7f;
	m_lay_lamp_reel[2][2] = BIT(data, 7);
}

void continental_state::seg_data_w(offs_t offset, u8 data)
{
	// latch segment data
	m_latch_segdata = data;
}

void continental_state::seg_digit_w(offs_t offset, u8 data)
{
	// bit0-3: SEG common select bit
	for (u8 i = 0; i < 4; i++)
		if (BIT(data, i))
			m_segdata[i] = bitswap<8>(m_latch_segdata, 7,0,1,2,3,4,5,6);

	// SEG credit
	m_lay_seg_credit[0] = m_segdata[3];			// credit SEG high
	m_lay_seg_credit[1] = m_segdata[2];			// credit SEG low
	// SEG win meter
	m_lay_seg_payout[0] = m_segdata[1];			// payout SEG high
	m_lay_seg_payout[1] = m_segdata[0];			// payout SEG low
}

void continental_state::reel_lamp_a_w(offs_t offset, u8 data)
{
	// bit0-3: reel lamps
	m_lay_lamp_reel[0][0] = BIT(data, 0);			// x:0, y:0
	m_lay_lamp_reel[1][0] = BIT(data, 1);			// x:1, y:0
	m_lay_lamp_reel[2][0] = BIT(data, 2);			// x:2, y:0
	m_lay_lamp_reel[0][1] = BIT(data, 3);			// x:0, y:1
}

void continental_state::reel_lamp_b_w(offs_t offset, u8 data)
{
	// bit0-3: reel lamps
	m_lay_lamp_reel[1][1] = BIT(data, 0);			// x:1, y:1
	m_lay_lamp_reel[2][1] = BIT(data, 1);			// x:2, y:1
	m_lay_lamp_reel[0][2] = BIT(data, 2);			// x:0, y:2
	m_lay_lamp_reel[1][2] = BIT(data, 3);			// x:1, y:2
}

void continental_state::status_lamp_w(offs_t offset, u8 data)
{
	// bit0: win meter (payout) lamp
	// bit1: start lamp
	// bit2: insert medal lamp
	// bit3: game over / wait lamp
	m_lay_lamp_payout = BIT(data, 0);
	m_lay_lamp_start = BIT(data, 1);
	m_lay_lamp_insert = BIT(data, 2);
	m_lay_lamp_gameover = BIT(data, 3);

	// Credit lamp is provisionally kept lit
	m_lay_lamp_credit = 1;
}

u8 continental_state::rocknroll_switch_sense_a_r(offs_t offset)
{
	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[0]->read();
}

u8 continental_state::rocknroll_switch_sense_b_r(offs_t offset)
{
	u8 port = m_in_port[1]->read();

	if (m_powerfail_status_ready)
		port &= ~0x80;

	// *** Don't accept start lever while [Pay Out / Credit Game] button is pressed ***
	// 
	// NOTE:
	// Prevent simultaneous Start Lever actuation while the Payout/Credit button
	// is held down. On early factory production units, this hardware loophole
	// allowed players to trigger reel spin and credit clearance in the same frame
	// (a famous real-world exploit known as the "21st Century World Annihilation Slot Strategy").
	// This interlock simulates the subsequent field-retrofit fix installed by operators.
	if (!BIT(port, 6))
		port &= ~(1U << 0);

	return port;
}

void continental_state::rocknroll_lamp_frame_w(offs_t offset, u8 data)
{
	// bit0-7: frame top lamp
	for (int i = 0; i < 8; i++)
		m_lay_lamp_frame[i] = BIT(data, i);
}

void continental_state::common_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x3000, 0x31ff).ram().share("nvram");
	map(0x5000, 0x5002).w(FUNC(continental_state::drive_latch_w));
	map(0x5003, 0x5003).w(FUNC(continental_state::lamp_latch_bet_w));
	map(0x5005, 0x5005).w(FUNC(continental_state::lamp_latch_misc_w));
	map(0x5006, 0x5006).w(FUNC(continental_state::bonus_seg_and_lamp_w));
	map(0x5007, 0x5007).w(FUNC(continental_state::seg_data_w));
	map(0x5008, 0x5008).w(FUNC(continental_state::seg_digit_w));
	map(0x5009, 0x5009).w(FUNC(continental_state::reel_lamp_a_w));
	map(0x500a, 0x500a).w(FUNC(continental_state::reel_lamp_b_w));
	map(0x500b, 0x500b).w(FUNC(continental_state::status_lamp_w));
	map(0x7000, 0x7001).w(m_ym2413, FUNC(ym2413_device::write));
}

void continental_state::continental_mem_map(address_map &map)
{
	common_mem_map(map);

	map(0x5004, 0x5004).w(FUNC(continental_state::lamp_latch_frame_w));
	map(0x6000, 0x6000).r(FUNC(continental_state::switch_sense_a_r));
	map(0x6001, 0x6001).r(FUNC(continental_state::switch_sense_b_r));
}

void continental_state::rocknroll_mem_map(address_map &map)
{
	common_mem_map(map);

	map(0x5004, 0x5004).w(FUNC(continental_state::rocknroll_lamp_frame_w));
	map(0x6000, 0x6000).r(FUNC(continental_state::rocknroll_switch_sense_a_r));
	map(0x6001, 0x6001).r(FUNC(continental_state::rocknroll_switch_sense_b_r));
}


static INPUT_PORTS_START( continental )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(continental_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(continental_state::opto_r<0>))	// reel 1 sensor (left)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(continental_state::opto_r<1>))	// reel 2 sensor (center)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(continental_state::opto_r<2>))	// reel 3 sensor (right)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(continental_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Door Key / Reset Button / Change Setting Value Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("Subtank Overflow")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )	// PFI signal in (ready to return from power-fail)

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(continental_state::powerfail_trigger), 0)

INPUT_PORTS_END

static INPUT_PORTS_START( rocknroll )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(continental_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(continental_state::opto_r<0>))	// reel 1 sensor (left)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(continental_state::opto_r<1>))	// reel 2 sensor (center)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(continental_state::opto_r<2>))	// reel 3 sensor (right)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(continental_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Door Key / Reset Button / Change Setting Value Button")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )	// PFI signal in (ready to return from power-fail)

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(continental_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void continental_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void continental_state::common(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10.816_MHz_XTAL / 2);
	m_maincpu->set_periodic_int(FUNC(continental_state::irq0_line_hold), attotime::from_hz(500));

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->hopper_cb().set(FUNC(continental_state::playing_hopper_samples));

	// Configure sensor emulation modes: single-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SINGLE);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], UNIVERSAL_200STEP_REEL, 118, 318, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(continental_state::opto_cb<0>));
	REEL(config, m_reel[1], UNIVERSAL_200STEP_REEL, 118, 318, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(continental_state::opto_cb<1>));
	REEL(config, m_reel[2], UNIVERSAL_200STEP_REEL, 118, 318, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(continental_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.2);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}

void continental_state::continental(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &continental_state::continental_mem_map);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(30*2);
}

void continental_state::rocknroll(machine_config &config)
{
	common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &continental_state::rocknroll_mem_map);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(70*2);
}


ROM_START( pslibbell3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "libertybell3.bin", 0x0000, 0x2000, CRC(014438a6) SHA1(730700e93434cf9d84b4907d46ae524c405b7554) )
ROM_END

ROM_START( pscentur21 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "century21.bin", 0x0000, 0x2000, CRC(5b1098ad) SHA1(a111fa2eab4a32d9186be49452c8e2b67e2e102a) )
ROM_END

ROM_START( pslibbell4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "libertybell4.bin", 0x0000, 0x2000, CRC(cac0afee) SHA1(cfce620c94a3bb93b495666741e9ef1e6bf509ec) )
ROM_END

ROM_START( psconti )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "continental.bin", 0x0000, 0x2000, CRC(a1c191ad) SHA1(725bd4c200f8f4b6c0f330e0c639cda6863f6abf) )
ROM_END

ROM_START( psconti3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "continental3.bin", 0x0000, 0x2000, CRC(06fea396) SHA1(89649484b8afd60edf50d873109814f0107b0901) )
ROM_END

ROM_START( psamemag )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "amemag.bin", 0x0000, 0x2000, CRC(2db79ba0) SHA1(cfb9affb549cb53f949c986617ad64a328a1a300) )
ROM_END

ROM_START( psconti2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "continental2.bin", 0x0000, 0x2000, CRC(5b776955) SHA1(f9f5ba3b0745eb10c7d8896015c7fc34c3bd0d26) )
ROM_END

ROM_START( psrocknrol )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "rocknroll.bin", 0x0000, 0x2000, CRC(131d9952) SHA1(8e0caaed37a644c5b118ef594738461c3c188a3f) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,     INPUT,       CLASS,             INIT,             MONITOR, COMPANY,              FULLNAME,                                               FLAGS,                                      LAYOUT
GAMEL( 1989, pslibbell3, 0,      continental, continental, continental_state, init_continental, 0,       "Macy",               "Liberty Bell 3 (2nd Generation, 2nd model, Type A)",   (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_libertybell3 )
GAMEL( 1989, pscentur21, 0,      continental, continental, continental_state, init_continental, 0,       "Mizuho",             "Century 21 (2nd Generation, 2nd model, Type A)",       (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_century21 )
GAMEL( 1990, pslibbell4, 0,      continental, continental, continental_state, init_libbell4,    0,       "Universal",          "Liberty Bell 4 (2nd Generation, 2nd model, Type A)",   (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_libertybell4 )
GAMEL( 1990, psrocknrol, 0,      rocknroll,   rocknroll,   continental_state, init_continental, 0,       "AI (Universal OEM)", "Rock'n Roll (2nd Generation, 2nd model, Type A)",      (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_rocknroll )
GAMEL( 1990, psconti,    0,      continental, continental, continental_state, init_continental, 0,       "Mizuho",             "Continental (3rd Generation, 1st model, Type A)",      (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_continental )
GAMEL( 1990, psconti3,   0,      continental, continental, continental_state, init_continental, 0,       "Macy",               "Continental 3 (3rd Generation, 1st model, Type A-C)",  (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_continental3 )
GAMEL( 1990, psamemag,   0,      continental, continental, continental_state, init_continental, 0,       "Universal",          "Americana Magnum (3rd Generation, 1st model, Type A)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_amemag )
GAMEL( 1990, psconti2,   0,      continental, continental, continental_state, init_continental, 0,       "Universal",          "Continental 2 (3rd Generation, 2nd model, Type A)",    (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_continental2 )
