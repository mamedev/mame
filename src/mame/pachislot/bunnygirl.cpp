// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Olympia Pachi-Slot Series Driver

    Driver by Chiaki Tachibana

    Initial driver development started on 2026-04-29.

    Supported Games:
    - Bunny Girl  (c) 1988 Olympia  (2nd Generation, 1st model, Type A)

    Hardware:
    - Main CPU: Z80 @ provisional 5.0 MHz (10.0 MHz OSC / 2)
    - Sound: SN76496 @ provisional 4.0 MHz
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 512 bytes battery-backed SRAM (0x8000-0x81ff)

    TODO:
    - Identify the exact physical crystal frequencies and verify the CPU
      clock divider from genuine hardware.
    - Confirm the full mapping of the top panel outer lamps (currently 8
      control bits mapped across 12 physical lamp positions).

    NOTE:
    [Interrupt Architecture and Output Ports]
    - The firmware utilizes Z80 Interrupt Mode 2 (IM2) with register I = 0x11.
      The main gameplay and timer loop is driven by vector 0x06 at 512 Hz,
      while stepper reel motors are updated through vectors 0x00, 0x02, and
      0x04 via a 2048 Hz 4-phase slot scan (2048 / 4 = 512 Hz per reel).
    - Physical output latches (lamps, 7-segment displays, wait-time LED ladder,
      stepper phases, and solenoids) are mapped into the lower 16 bytes of
      the battery-backed RAM window (0x8000-0x800f).

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, allowing the CPU to execute its
      state-saving routine into battery-backed NVRAM during the reservoir
      capacitance hold-up time.
    - Genuine hardware drops the NMI line upon detecting a power drop, and the
      power supervisor asserts an active-low status line read back via
      port 0xc030 bit 7 once the backup sequence is initiated.
    - Because MAME currently lacks a pre-reset / pre-shutdown callback
      mechanism to allow the CPU to gracefully complete its power-down
      sequence before machine_reset() or exit (e.g. F3 / ESC keys), this
      driver provisionally provides a manual 'P' key (powerfail_trigger).
    - Pressing the 'P' key pulses the NMI line, sets m_powerfail_status_ready
      to simulate the active-low completion status on port 0xc030 bit 7, and
      preserves the physical reel absolute positions via stepper_device helpers.
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
    - Total IN and Total OUT pulse outputs from port 0x8007 are connected to
      coin_counter_w(0/1). On physical machines, these lines directly pulsed
      internal electromechanical 6-digit meters and centralized parlor terminals.

    [Setting Change Procedure]
    - Standard 2nd/3rd generation sequence:
      1. Turn power on while asserting Setting Key (F2).
      2. Select desired setting (1-6) using Setting Value button (F1).
      3. Confirm setting value by operating Start Lever.
      4. Turn Setting Key off to enter normal game state.

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
#include "sound/sn76496.h"
#include "sound/samples.h"
#include "speaker.h"


namespace {

// internal artwork
#include "bunnygirl.lh"

class bunnygirl_state : public driver_device
{
public:
	bunnygirl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram", 0x200, ENDIANNESS_LITTLE)
		, m_pachislot(*this, "pachislot")
		, m_sn76496(*this, "sn76496")
		, m_samples(*this, "samples")
		, m_reel(*this, "reel%u", 0U)
		, m_in_port(*this, "INPUT%u", 0U)
		, m_in_fakebtn(*this, "FAKEBTN")
		, m_lay_lamp_toppanelc(*this, "lamp_toppanelc%u", 0U)
		, m_lay_lamp_toppanelw(*this, "lamp_toppanelw%u", 0U)
		, m_lay_lamp_insert(*this, "lamp_insert")
		, m_lay_lamp_gameover(*this, "lamp_gameover")
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_led_waitlampl(*this, "led_waittime%u", 0U)
		, m_lay_seg_jacbb(*this, "seg_jacbb%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void bunnygirl(machine_config &config) ATTR_COLD;

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
	required_device<z80_device> m_maincpu;
	memory_share_creator<u8> m_nvram;
	required_device<pachislot_device> m_pachislot;
	required_device<sn76496_device> m_sn76496;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<3> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<2> m_lay_lamp_toppanelc;
	output_finder<12> m_lay_lamp_toppanelw;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_gameover;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<30> m_lay_led_waitlampl;
	output_finder<2> m_lay_seg_jacbb;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	u8 system_status_r(offs_t offset);
	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);
	void output_latch_w(offs_t offset, u8 data);
	void commit_display_outputs();

	TIMER_CALLBACK_MEMBER(main_irq_tick);
	TIMER_CALLBACK_MEMBER(reel_irq_tick);
	TIMER_CALLBACK_MEMBER(display_commit_tick);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	emu_timer *m_main_irq_timer = nullptr;
	emu_timer *m_reel_irq_timer = nullptr;
	emu_timer *m_display_commit_timer = nullptr;
	u8 m_reel_irq_phase = 0;

	u8 m_irq_vector = 0;
	int irq_ack(device_t &device, int irqline) { return m_irq_vector; }

	u8 m_opto[3];
	u8 m_segdata[6];
	u8 m_common;
	u8 m_wait_led_latch[4];
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;
};


void bunnygirl_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_common));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_reel_irq_phase));
	save_item(NAME(m_wait_led_latch));
	save_item(NAME(m_irq_vector));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_main_irq_timer = timer_alloc(FUNC(bunnygirl_state::main_irq_tick), this);
	m_reel_irq_timer = timer_alloc(FUNC(bunnygirl_state::reel_irq_tick), this);
	m_display_commit_timer = timer_alloc(FUNC(bunnygirl_state::display_commit_tick), this);

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void bunnygirl_state::machine_reset()
{
	for (u8 &segdata : m_segdata)
		segdata = 0;
	m_common = 0;
	for (u8 &wait_led : m_wait_led_latch)
		wait_led = 0;

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;
	m_irq_vector = 0;

	// The board has a 10 MHz oscillator. The exact CPU divider is still
	// provisional, but the observed IM2 service timing is treated as a
	// 512 Hz family. Drive the reel-control vectors from a 2048 Hz
	// four-slot scan, giving each reel vector 2048 / 4 = 512 Hz. The
	// fourth slot is left idle.
	m_reel_irq_phase = 0;
	m_main_irq_timer->adjust(attotime::from_hz(5120), 0, attotime::from_hz(512));
	m_reel_irq_timer->adjust(attotime::zero, 0, attotime::from_hz(2048));
	m_display_commit_timer->adjust(attotime::never);

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	commit_display_outputs();

	powerfail_restore_state();
	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;

	m_pachislot->reset_selector();
	m_pachislot->reset_hopper();
}

// ---------------------------------------------------------------------------
//  Power-fail (PFI) Simulation Helpers
// ---------------------------------------------------------------------------
void bunnygirl_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void bunnygirl_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void bunnygirl_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

TIMER_CALLBACK_MEMBER(bunnygirl_state::main_irq_tick)
{
	// Program uses IM2 with I = 0x11. Vector 0x06 selects the main periodic
	// task at 0x1106. It is serviced at the same 512 Hz pacing used by
	// the reel-control vectors.
	m_irq_vector = 0x06;
	m_maincpu->set_input_line(0, HOLD_LINE);

	// The 0x06 IRQ handler updates the display-latch RAM. Commit the
	// accumulated latch state 512 CPU cycles later. This avoids exposing
	// transient common-clear / data-write states in the MAME layout.
	m_display_commit_timer->adjust(m_maincpu->clocks_to_attotime(512));
}

TIMER_CALLBACK_MEMBER(bunnygirl_state::reel_irq_tick)
{
	// Reel-control vectors appear to be generated from a four-slot scan. Use
	// three slots for 0x00, 0x02 and 0x04, leaving the fourth slot idle.
	static constexpr u8 vectors[4] = { 0x00, 0x02, 0x04, 0xff };
	const u8 vector = vectors[m_reel_irq_phase & 3];

	if (vector != 0xff)
	{
		m_irq_vector = vector;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}

	m_reel_irq_phase++;
}

TIMER_CALLBACK_MEMBER(bunnygirl_state::display_commit_tick)
{
	// Reflect the accumulated display latches to the MAME layout.
	commit_display_outputs();
}

// Provisional trigger handler simulating the physical power failure supervisor circuit.
INPUT_CHANGED_MEMBER(bunnygirl_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

u8 bunnygirl_state::nvram_r(offs_t offset)
{
	return m_nvram[offset];
}

void bunnygirl_state::nvram_w(offs_t offset, u8 data)
{
	m_nvram[offset] = data;

	if (offset < 0x10)
		output_latch_w(offset, data);
}

void bunnygirl_state::commit_display_outputs()
{
	m_lay_seg_jacbb[0]  = m_segdata[4];			// JAC / BB SEG high
	m_lay_seg_jacbb[1]  = m_segdata[3];			// JAC / BB SEG low
	m_lay_seg_credit[0] = m_segdata[2];			// credit SEG high
	m_lay_seg_credit[1] = m_segdata[1];			// credit SEG low
	m_lay_seg_payout[0] = m_segdata[0];			// payout SEG high
	m_lay_seg_payout[1] = m_segdata[5];			// payout SEG low

	for (int led = 0; led < 30; led++)
	{
		const int common = led / 8;
		const int bit = led & 7;

		m_lay_led_waitlampl[led] = BIT(m_wait_led_latch[common], bit);
	}
}

void bunnygirl_state::output_latch_w(offs_t offset, u8 data)
{
	data = m_nvram[offset];

	switch (offset)
	{
		case 0x00:
			if (data & 0x3f)
				m_common = data & 0x3f;
			break;

		case 0x01:
			for (u8 common = 0; common < 6; common++)
				if (BIT(m_common, common))
					m_segdata[common] = data;
			break;

		case 0x02:
			static constexpr u8 wait_common_order[4] = { 0x20, 0x01, 0x02, 0x04 };

			for (u8 common = 0; common < 4; common++)
				if (m_common == wait_common_order[common])
					m_wait_led_latch[common] = data;
			break;

		case 0x03:
			m_lay_lamp_toppanelw[0] = m_lay_lamp_toppanelw[8]  = BIT(data, 0);
			m_lay_lamp_toppanelw[1] = m_lay_lamp_toppanelw[9]  = BIT(data, 1);
			m_lay_lamp_toppanelw[2] = m_lay_lamp_toppanelw[10] = BIT(data, 2);
			m_lay_lamp_toppanelw[3] = m_lay_lamp_toppanelw[11] = BIT(data, 3);
			m_lay_lamp_toppanelw[4] =                            BIT(data, 4);
			m_lay_lamp_toppanelw[5] =                            BIT(data, 5);
			m_lay_lamp_toppanelw[6] =                            BIT(data, 6);
			m_lay_lamp_toppanelw[7] =                            BIT(data, 7);
			break;

		case 0x04:
			m_lay_lamp_stop[0] = BIT(data, 0);
			m_lay_lamp_stop[1] = BIT(data, 1);
			m_lay_lamp_stop[2] = BIT(data, 2);

			for (int i = 0; i < 5; i++)
				m_lay_lamp_betline[i] = BIT(data, 3 + i);
			break;

		case 0x05:
			m_reel[0]->write(data & 0x0f);
			m_lay_lamp_insert = BIT(data, 4);
			m_lay_lamp_toppanelc[0] = BIT(data, 6);
			m_lay_lamp_toppanelc[1] = BIT(data, 7);
			break;

		case 0x06:
			m_reel[1]->write(data & 0x0f);

			m_solenoid = !BIT(data, 4);
			m_pachislot->selector_lock_w(m_solenoid);
			m_lay_signal_solenoid = m_solenoid;

			m_hopper = BIT(data, 5);
			m_pachislot->hopper_motor_w(m_hopper);
			m_lay_signal_hopper = m_hopper;

			m_lay_signal_jac = BIT(data, 6);
			m_lay_signal_big = BIT(data, 7);
			break;

		case 0x07:
			m_reel[2]->write(data & 0x0f);

			machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
			machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
			break;
	}
}

u8 bunnygirl_state::switch_sense_a_r(offs_t offset)
{
	return m_in_port[0]->read();
}

u8 bunnygirl_state::switch_sense_b_r(offs_t offset)
{
	m_pachislot->selector_w(in_fakebtn_r<0>());
	return m_in_port[1]->read();
}

u8 bunnygirl_state::system_status_r(offs_t offset)
{
	u8 port = m_in_port[2]->read();

	if (m_powerfail_status_ready)
		port &= ~0x80;

	return port;
}

void bunnygirl_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x81ff).rw(FUNC(bunnygirl_state::nvram_r), FUNC(bunnygirl_state::nvram_w));
	map(0xc000, 0xc000).w(m_sn76496, FUNC(sn76496_device::write));
	map(0xc020, 0xc020).r(FUNC(bunnygirl_state::switch_sense_a_r));
	map(0xc02f, 0xc02f).r(FUNC(bunnygirl_state::switch_sense_b_r));
	map(0xc030, 0xc030).r(FUNC(bunnygirl_state::system_status_r));
}

void bunnygirl_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x04).nopw();
}


static INPUT_PORTS_START( bunnygirl )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(bunnygirl_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Door Key")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(bunnygirl_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(bunnygirl_state::opto_r<0>))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(bunnygirl_state::opto_r<1>))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(bunnygirl_state::opto_r<2>))
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset Button")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bunnygirl_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void bunnygirl_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void bunnygirl_state::bunnygirl(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &bunnygirl_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &bunnygirl_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(bunnygirl_state::irq_ack));

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(50*2);
	m_pachislot->hopper_cb().set(FUNC(bunnygirl_state::playing_hopper_samples));

	// Configure sensor emulation modes: single-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SINGLE);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], OLYMPIA_200STEP_REEL, 384, 386, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(bunnygirl_state::opto_cb<0>));
	REEL(config, m_reel[1], OLYMPIA_200STEP_REEL, 384, 386, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(bunnygirl_state::opto_cb<1>));
	REEL(config, m_reel[2], OLYMPIA_200STEP_REEL, 384, 386, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(bunnygirl_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SN76496(config, m_sn76496, 4.00_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 0.6);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psbunygirl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bunnygirl.bin", 0x0000, 0x2000, CRC(dc8f00b5) SHA1(bf68923c4b7adce29efa05d695e51449233bfebe) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,   INPUT,     CLASS,           INIT,       MONITOR, COMPANY,   FULLNAME,                                         FLAGS,                                      LAYOUT
GAMEL( 1988, psbunygirl, 0,      bunnygirl, bunnygirl, bunnygirl_state, empty_init, 0,       "Olympia", "Bunny Girl (2nd Generation, 1st model, Type A)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_bunnygirl )
