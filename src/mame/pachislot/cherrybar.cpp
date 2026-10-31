// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Universal Group Pachi-Slot Series Driver
    Z80 + 3-reel + YM2413 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2026-05-12.

    Supported Games:
    - Cherry Bar  (c) 1992 ElectroCoin Japan  (4th Generation, 1st model, Type A-C)

    Hardware:
    - Main CPU: Z80 @ provisional 5.408 MHz (10.816 MHz XTAL / 2)
    - Sound: YM2413 @ 3.579545 MHz
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 512 bytes battery-backed SRAM (0x2000-0x21ff)
    - I/O: 5x NEC uPD8255AC-2 (or equivalent i8255 PPI) devices mapped across
      4-byte windows (Port A, Port B, Port C, Control Register):
        * PPI 0 (0x4000-0x4003): Input sensing (Switches, cabinet options, opto)
        * PPI 1 (0x6000-0x6003): Stepper reels 1-3, hopper drive, meter pulses
        * PPI 2 (0x8000-0x8003): Status lamps and decorative bonus LEDs
        * PPI 3 (0xa000-0xa003): Stop button lamps, frame lamps, reel backlight lamps
        * PPI 4 (0xc000-0xc003): 7-segment credit / win meter display multiplexing

    TODO:
    - Confirm the main CPU crystal frequency from real hardware.

    NOTE:
    [Lineage and IM 2 Interrupt Architecture]
    - ElectroCoin Japan (ECJ) was formed after the UK-based slot machine
      manufacturer ElectroCoin entered the Universal group. While Cherry Bar
      shares Universal cabinet parts, hopper, and V0-generation board layout,
      its firmware is unique in utilizing Z80 Interrupt Mode 2 (IM 2).
    - Other Universal titles of this era avoid the I register and rely on
      IM 1; Cherry Bar drives its main periodic task (0x00c2) via vector 0xfa
      at 512 Hz, and secondary counter/timing updates (0x0faf) via vector 0xf8
      at provisional 768 Hz.

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, allowing the CPU to execute its
      state-saving routine into battery-backed NVRAM during the reservoir
      capacitance hold-up time.
    - Genuine hardware asserts NMI to transfer control immediately to the
      power-down serialization vector.
    - Because MAME currently lacks a pre-reset / pre-shutdown callback
      mechanism to allow the CPU to gracefully complete its power-down
      sequence before machine_reset() or exit (e.g. F3 / ESC keys), this
      driver provisionally provides a manual 'P' key (powerfail_trigger).
    - Pressing the 'P' key triggers the supervisor NMI and preserves the
      physical reel absolute positions via stepper_device helpers.
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
    - Total IN (bit 6) and Total OUT (bit 7) pulse outputs from port 0x6000
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

    [Setting Change Procedure]
    - Standard 4th generation sequence:
      1. Turn Setting Key (F2) on and restart/reset machine.
      2. Select desired setting (1-6) with Setting Value button (F1).
      3. Confirm setting value by operating Start Lever.
      4. Turn Setting Key off to enter normal game state.

    [Game Rules & Options]
    - Configurable SEISAN (automatic payout) and UCHIDOME (automatic game-stop)
      behaviors are supported via cabinet option switches ("SEISAN": Off / On,
      "UCHIDOME": Off / On). Note that option switch changes require cycling through
      setting mode to reinitialize state.

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
#include "cherrybar.lh"

class cherrybar_state : public driver_device
{
public:
	cherrybar_state(const machine_config &mconfig, device_type type, const char *tag)
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
		, m_lay_lamp_replay(*this, "lamp_replay")
		, m_lay_lamp_credit(*this, "lamp_credit")
		, m_lay_lamp_payout(*this, "lamp_payout")
		, m_lay_lamp_betline(*this, "lamp_betline%u", 0U)
		, m_lay_lamp_bcount(*this, "lamp_bcount%u", 0U)
		, m_lay_lamp_stop(*this, "lamp_stop%u", 0U)
		, m_lay_lamp_reel(*this, "lamp_reel%u_%u", 0U, 0U)
		, m_lay_led_button(*this, "led_button%u", 0U)
		, m_lay_led_bonus(*this, "led_bonus%u", 0U)
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void cherrybar(machine_config &config);

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

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	template <unsigned N> void opto_cb(int state) { m_opto[N] = state; }

	void commit_lamp_buffers();

	TIMER_CALLBACK_MEMBER(lamp_buffer_commit_tick);

private:
	required_device<z80_device> m_maincpu;
	required_shared_ptr<u8> m_nvram;
	required_device<pachislot_device> m_pachislot;
	required_device<ym2413_device> m_ym2413;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<3> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<3> m_lay_lamp_frame;
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
	output_finder<8> m_lay_led_bonus;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	u8 switch_sense_a_r(offs_t offset);
	u8 cabinet_switch_r(offs_t offset);
	u8 switch_sense_b_r(offs_t offset);
	u8 rng_seed_r(offs_t offset);
	void drive_latch_a_w(offs_t offset, u8 data);
	void drive_latch_b_w(offs_t offset, u8 data);
	void reel_phase_c_w(offs_t offset, u8 data);
	void lamp_control_w(offs_t offset, u8 data);
	void led_bonus_a_w(offs_t offset, u8 data);
	void led_bonus_b_w(offs_t offset, u8 data);
	void lamp_latch_a_w(offs_t offset, u8 data);
	void lamp_latch_b_w(offs_t offset, u8 data);
	void lamp_latch_c_w(offs_t offset, u8 data);
	void seg_data_w(offs_t offset, u8 data);
	void seg_digit_w(offs_t offset, u8 data);

	TIMER_CALLBACK_MEMBER(main_irq_tick);
	TIMER_CALLBACK_MEMBER(sub_irq_tick);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	emu_timer *m_main_irq_timer = nullptr;
	emu_timer *m_sub_irq_timer = nullptr;
	emu_timer *m_lamp_buffer_commit_timer = nullptr;

	u8 m_irq_vector = 0;
	int irq_ack(device_t &device, int irqline) { return m_irq_vector; }

	u8 m_segdata[5];
	u8 m_segdata_pending;
	u8 m_opto[3];
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;

	u8 m_reel_lamp_pending[3][3];
	u8 m_betline_lamp_pending[5];
	u8 m_seg_credit_pending[2];
	u8 m_seg_payout_pending[2];
	u8 m_led_bonus_pending[2];
};


void cherrybar_state::machine_start()
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
	save_item(NAME(m_led_bonus_pending));
	save_item(NAME(m_segdata_pending));
	save_item(NAME(m_irq_vector));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_main_irq_timer = timer_alloc(FUNC(cherrybar_state::main_irq_tick), this);
	m_sub_irq_timer = timer_alloc(FUNC(cherrybar_state::sub_irq_tick), this);
	m_lamp_buffer_commit_timer = timer_alloc(FUNC(cherrybar_state::lamp_buffer_commit_tick), this);

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void cherrybar_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(std::begin(m_segdata), std::end(m_segdata), 0);

	m_lamp_buffer_commit_timer->adjust(attotime::from_hz(512), 0, attotime::from_hz(512));

	memset(m_reel_lamp_pending, 0, sizeof(m_reel_lamp_pending));
	memset(m_betline_lamp_pending, 0, sizeof(m_betline_lamp_pending));
	memset(m_seg_credit_pending, 0, sizeof(m_seg_credit_pending));
	memset(m_seg_payout_pending, 0, sizeof(m_seg_payout_pending));
	memset(m_led_bonus_pending, 0, sizeof(m_led_bonus_pending));
	m_segdata_pending = 0;

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;
	m_irq_vector = 0;

	commit_lamp_buffers();

	powerfail_restore_state();
	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;

	// Vector 0xfa is the main periodic handler at 0x00c2 (512 Hz).
	// Vector 0xf8 is the secondary counter update at 0x0faf (provisional 768 Hz).
	m_main_irq_timer->adjust(attotime::from_hz(7680), 0, attotime::from_hz(512));
	m_sub_irq_timer->adjust(attotime::zero, 0, attotime::from_hz(768));

	m_pachislot->reset_selector();
	m_pachislot->reset_hopper();
}

// ---------------------------------------------------------------------------
//  Power-fail (PFI) Simulation Helpers
// ---------------------------------------------------------------------------
void cherrybar_state::powerfail_prepare_state()
{
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void cherrybar_state::powerfail_restore_state()
{
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void cherrybar_state::device_post_load()
{
	powerfail_restore_state();
	commit_lamp_buffers();
}

// Provisional trigger handler simulating the physical power failure supervisor circuit.
INPUT_CHANGED_MEMBER(cherrybar_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

TIMER_CALLBACK_MEMBER(cherrybar_state::main_irq_tick)
{
	m_irq_vector = 0xfa;
	m_maincpu->set_input_line(0, HOLD_LINE);
}

TIMER_CALLBACK_MEMBER(cherrybar_state::sub_irq_tick)
{
	m_irq_vector = 0xf8;
	m_maincpu->set_input_line(0, HOLD_LINE);
}

// Staging output buffers committed atomically at 512 Hz to eliminate visual multiplex flicker
void cherrybar_state::commit_lamp_buffers()
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

	for (int i = 0; i < 8; i++)
		m_lay_led_bonus[i] = BIT(m_led_bonus_pending[0], i) | (BIT(m_led_bonus_pending[1], i) << 1);
}

TIMER_CALLBACK_MEMBER(cherrybar_state::lamp_buffer_commit_tick)
{
	commit_lamp_buffers();
}

u8 cherrybar_state::switch_sense_a_r(offs_t offset)
{
	return m_in_port[0]->read();
}

u8 cherrybar_state::cabinet_switch_r(offs_t offset)
{
	return m_in_port[1]->read();
}

u8 cherrybar_state::switch_sense_b_r(offs_t offset)
{
	m_pachislot->selector_w(in_fakebtn_r<0>());
	return m_in_port[2]->read();
}

u8 cherrybar_state::rng_seed_r(offs_t offset)
{
	return machine().rand() & 0xff;
}

void cherrybar_state::drive_latch_a_w(offs_t offs, u8 data)
{
	m_reel[0]->write(data & 0x0f);

	m_lay_signal_jac = BIT(data, 4);
	m_lay_signal_big = BIT(data, 5);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 7));
}

void cherrybar_state::drive_latch_b_w(offs_t offs, u8 data)
{
	m_reel[1]->write(data & 0x0f);

	if (BIT(data, 5) && !BIT(data, 4))
	{
		m_hopper = 1;
		m_lay_signal_hopper = 1;
	}
	else if (!BIT(data, 5) && BIT(data, 4))
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

	m_solenoid = !BIT(data, 7);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;
}

void cherrybar_state::reel_phase_c_w(offs_t offs, u8 data)
{
	m_reel[2]->write(data & 0x0f);
}

void cherrybar_state::lamp_control_w(offs_t offs, u8 data)
{
	m_lay_lamp_payout = BIT(data, 3);
	m_lay_lamp_start = BIT(data, 4);
	m_lay_lamp_insert = BIT(data, 5);
	m_lay_lamp_gameover = BIT(data, 6);
	m_lay_lamp_replay = BIT(data, 7);

	m_lay_lamp_credit = 0;
}

void cherrybar_state::led_bonus_a_w(offs_t offs, u8 data)
{
	m_led_bonus_pending[0] = data;
}

void cherrybar_state::led_bonus_b_w(offs_t offs, u8 data)
{
	m_led_bonus_pending[1] = data;
}

void cherrybar_state::lamp_latch_a_w(offs_t offs, u8 data)
{
	m_lay_lamp_stop[0] = BIT(data, 0);
	m_lay_lamp_stop[1] = BIT(data, 1);
	m_lay_lamp_stop[2] = BIT(data, 2);

	m_lay_lamp_frame[0] = BIT(data, 3);
	m_lay_lamp_frame[1] = BIT(data, 4);
	m_lay_lamp_frame[2] = BIT(data, 5);
}

void cherrybar_state::lamp_latch_b_w(offs_t offs, u8 data)
{
	m_reel_lamp_pending[2][2] = BIT(data, 0);

	for (int i = 0; i < 5; i++)
		m_betline_lamp_pending[i] = BIT(data, i + 1);
}

void cherrybar_state::lamp_latch_c_w(offs_t offs, u8 data)
{
	m_reel_lamp_pending[0][0] = BIT(data, 0);
	m_reel_lamp_pending[1][0] = BIT(data, 1);
	m_reel_lamp_pending[2][0] = BIT(data, 2);
	m_reel_lamp_pending[0][1] = BIT(data, 3);
	m_reel_lamp_pending[1][1] = BIT(data, 4);
	m_reel_lamp_pending[2][1] = BIT(data, 5);
	m_reel_lamp_pending[0][2] = BIT(data, 6);
	m_reel_lamp_pending[1][2] = BIT(data, 7);
}

void cherrybar_state::seg_data_w(offs_t offs, u8 data)
{
	m_segdata_pending = data;
}

void cherrybar_state::seg_digit_w(offs_t offs, u8 data)
{
	for (u8 i = 0; i < 5; i++)
		if (BIT((data & 0x1f), i))
			m_segdata[i] = bitswap<8>(m_segdata_pending, 7,0,1,2,3,4,5,6);

	m_seg_credit_pending[0] = m_segdata[3];
	m_seg_credit_pending[1] = m_segdata[2];

	m_seg_payout_pending[0] = m_segdata[1];
	m_seg_payout_pending[1] = m_segdata[0];
}

void cherrybar_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram().share("nvram");

	// --- PPI 0: Input buffer registers (0x4000-0x4003) ---
	map(0x4000, 0x4000).r(FUNC(cherrybar_state::switch_sense_a_r));
	map(0x4001, 0x4001).r(FUNC(cherrybar_state::cabinet_switch_r));
	map(0x4002, 0x4002).r(FUNC(cherrybar_state::switch_sense_b_r));

	map(0x4005, 0x4006).r(FUNC(cherrybar_state::rng_seed_r));

	// --- PPI 1: Stepper motor and mechanical drive outputs (0x6000-0x6003) ---
	map(0x6000, 0x6000).w(FUNC(cherrybar_state::drive_latch_a_w));
	map(0x6001, 0x6001).w(FUNC(cherrybar_state::drive_latch_b_w));
	map(0x6002, 0x6002).w(FUNC(cherrybar_state::reel_phase_c_w));
	map(0x6003, 0x6003).nopw();

	// --- PPI 2: Display & bonus LED outputs (0x8000-0x8003) ---
	map(0x8000, 0x8000).w(FUNC(cherrybar_state::lamp_control_w));
	map(0x8001, 0x8001).w(FUNC(cherrybar_state::led_bonus_a_w));
	map(0x8002, 0x8002).w(FUNC(cherrybar_state::led_bonus_b_w));
	map(0x8003, 0x8003).nopw();

	// --- PPI 3: Cabinet and reel illumination outputs (0xa000-0xa003) ---
	map(0xa000, 0xa000).w(FUNC(cherrybar_state::lamp_latch_a_w));
	map(0xa001, 0xa001).w(FUNC(cherrybar_state::lamp_latch_b_w));
	map(0xa002, 0xa002).w(FUNC(cherrybar_state::lamp_latch_c_w));
	map(0xa003, 0xa003).nopw();

	// --- PPI 4: 7-segment display multiplex outputs (0xc000-0xc003) ---
	map(0xc000, 0xc000).w(FUNC(cherrybar_state::seg_data_w));
	map(0xc001, 0xc001).w(FUNC(cherrybar_state::seg_digit_w));
	map(0xc002, 0xc003).nopw();

	// Sound generator
	map(0xe000, 0xe001).w(m_ym2413, FUNC(ym2413_device::write));
}

void cherrybar_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).nopw();
}


static INPUT_PORTS_START( cherrybar )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Door Key / Reset Button / Change Setting Value Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("UCHIDOME Reset Button")
	PORT_CONFNAME( 0x04, 0x00, "SEISAN" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_CONFNAME( 0x08, 0x00, "UCHIDOME" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(cherrybar_state::hopper_r<pachislot_device::PACHISLOT_SENSOR1>))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(cherrybar_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(cherrybar_state::opto_r<0>))
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(cherrybar_state::opto_r<1>))
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(cherrybar_state::opto_r<2>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(cherrybar_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cherrybar_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void cherrybar_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void cherrybar_state::cherrybar(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10.816_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cherrybar_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &cherrybar_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(cherrybar_state::irq_ack));

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(30*2);
	m_pachislot->hopper_cb().set(FUNC(cherrybar_state::playing_hopper_samples));

	// Configure sensor emulation modes: single-sensor coin selector and Eleco 2-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SINGLE);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_ELECO1);

	// reel device
	REEL(config, m_reel[0], UNIVERSAL_200STEP_REEL, 118, 320, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(cherrybar_state::opto_cb<0>));
	REEL(config, m_reel[1], UNIVERSAL_200STEP_REEL, 118, 320, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(cherrybar_state::opto_cb<1>));
	REEL(config, m_reel[2], UNIVERSAL_200STEP_REEL, 118, 320, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(cherrybar_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.2);

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( pscherybar )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "cherrybar.bin", 0x0000, 0x2000, CRC(3244ebba) SHA1(9f4bbaba34f014feeff71b8e25cce82bd28c2eb3) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,   INPUT,     CLASS,           INIT,       MONITOR, COMPANY,             FULLNAME,                                           FLAGS,                                      LAYOUT
GAMEL( 1992, pscherybar, 0,      cherrybar, cherrybar, cherrybar_state, empty_init, ROT0,    "ElectroCoin Japan", "Cherry Bar (4th Generation, 1st model, Type A-C)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_cherrybar )
