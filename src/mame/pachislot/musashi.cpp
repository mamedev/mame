// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Pioneer Pachi-Slot Series Driver
    Z80 + 3-reel + YM2149 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2024-07-03.

    Supported Games:
    - Musashi    (c) 1990 Pioneer  (2nd Generation, 2nd model, Type A)
    - Musashi 2  (c) 1992 Pioneer  (3rd Generation, 1st model, Type A)

    Hardware:
    - Main CPU: Z80 @ 4.0 MHz (4.0 MHz XTAL)
    - Sound: YM2149 @ 2.0 MHz (4.0 MHz XTAL / 2)
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 512 bytes battery-backed SRAM (0x2000-0x21ff)

    TODO:
    - Confirm physical crystal frequencies and verify the precise periodic
      interrupt divider from genuine hardware.

    NOTE:
    [Periodic IRQ and Clock Pacing]
    - The main periodic interrupt is driven at 500 Hz (irq0_line_hold).
      Port 0x6000 controls IRQ0 generation (enabled during boot, disabled
      upon NMI power-fail handling).

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, allowing the CPU to execute its
      state-saving routine into battery-backed NVRAM during the reservoir
      capacitance hold-up time.
    - On physical hardware, the CPU naturally dies after saving state due to
      voltage decay. Because MAME lacks an automatic power-down framework,
      the driver-side helper (init_musashi) patches the end of the PFI routine
      to HALT (0x76) to prevent CPU loops from corrupting the serialized state.
    - Because MAME currently lacks a pre-reset / pre-shutdown callback
      mechanism to allow the CPU to gracefully complete its power-down
      sequence before machine_reset() or exit (e.g. F3 / ESC keys), this
      driver provisionally provides a manual 'P' key (powerfail_trigger).
    - Pressing the 'P' key asserts the supervisor NMI and preserves the physical
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
    - Total IN (bit 0) and Total OUT (bit 1) pulse outputs driven by port 0x0b
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

    [Setting Change Procedure]
    - Standard 2nd/3rd generation sequence:
      1. Turn Setting Key (F2) on and restart/reset machine.
      2. Select desired setting (1-6) on 7-segment display with Door Reset / CSV button (F1).
      3. Confirm setting value by operating Start Lever.
      4. Turn Setting Key off to enter normal game state.

    [Game Rules & Options]
    - Automatic game-stop (UCHIDOME) is enforced by program logic upon
      BIG BONUS completion.
    - Configurable SEISAN (automatic payout) behavior is supported via
      cabinet option switch ("SEISAN": Off / On). Changes take effect
      after re-entering setting mode and reinitializing NVRAM.

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
#include "musashi.lh"
#include "musashi2.lh"

class musashi_state : public driver_device
{
public:
	musashi_state(const machine_config &mconfig, device_type type, const char *tag)
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
		, m_lay_lamp_timeup(*this, "lamp_timeup")
		, m_lay_seg_credit(*this, "seg_credit%u", 0U)
		, m_lay_seg_payout(*this, "seg_payout%u", 0U)
		, m_lay_signal_big(*this, "signal_big")
		, m_lay_signal_jac(*this, "signal_jac")
		, m_lay_signal_hopper(*this, "signal_hopper")
		, m_lay_signal_solenoid(*this, "signal_solenoid")
	{ }

	void musashi_common(machine_config &config);
	void musashi(machine_config &config);
	void musashi2(machine_config &config);

	void init_musashi();

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
	output_finder<> m_lay_lamp_timeup;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	INTERRUPT_GEN_MEMBER(interrupt);
	void irq0_control_w(offs_t offset, u8 data);
	bool m_irq0_enabled = true;

	u8 switch_sense_a_r(offs_t offset);
	u8 system_status_r(offs_t offset);
	void reel_phase_w(offs_t offset, u8 data);
	void seg_control_w(offs_t offset, u8 data);
	void lamp_latch_bet_w(u8 data);
	void lamp_latch_jac_w(u8 data);
	void drive_latch_w(offs_t offset, u8 data);
	void signal_latch_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	u8 m_opto[3];
	u8 m_segdata[2][4];
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;
};


void musashi_state::init_musashi()
{
	u8 *ROM = memregion("maincpu")->base();

	// NOTE:
	// On genuine hardware, after the PFI interrupt routine finishes serializing
	// critical CPU registers into battery-backed SRAM, the physical CPU simply
	// dies due to power supply decay; therefore, the original ROM does not end
	// with a HALT instruction.
	// In MAME, because the CPU continues executing normal main/interrupt cycles
	// after the simulated PFI routine without an actual power loss, subsequent
	// loop iterations overwrite the freshly saved state in NVRAM with corrupted data.
	// To prevent this NVRAM corruption until MAME gains proper framework-level
	// power-off handling, we patch the exit point of the backup sequence to HALT (0x76).
	// Both Musashi and Musashi 2 share the identical PFI routine layout at 0x009c.
	ROM[0x009c] = 0x76;	// -> HALT
}

void musashi_state::machine_start()
{
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_irq0_enabled));

	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void musashi_state::machine_reset()
{
	std::fill(std::begin(m_opto), std::end(m_opto), 0);
	std::fill(&m_segdata[0][0], &m_segdata[0][0] + (sizeof(m_segdata) / sizeof(m_segdata[0][0])), 0);

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
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
void musashi_state::powerfail_prepare_state()
{
	// Snapshot physical reel absolute positions prior to CPU shutdown/reset
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void musashi_state::powerfail_restore_state()
{
	// Restore physical reel mechanical positions upon machine reset / startup
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

void musashi_state::device_post_load()
{
	// Restore physical stepper positions from savestate snapshot
	powerfail_restore_state();
}

// Provisional trigger handler simulating the physical power failure supervisor circuit.
// Asserts NMI to execute the CPU's state-saving routine.
INPUT_CHANGED_MEMBER(musashi_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

INTERRUPT_GEN_MEMBER(musashi_state::interrupt)
{
	if (m_irq0_enabled)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}

void musashi_state::irq0_control_w(offs_t offset, u8 data)
{
	// 0x00 = disable periodic IRQ0, non-zero = enable periodic IRQ0
	m_irq0_enabled = (data != 0x00);
}

u8 musashi_state::switch_sense_a_r(offs_t offset)
{
	// bit0: payout / credit game button
	// bit1: bet button
	// bit2: medal hopper sensor 0
	// bit3: start lever
	// bit4: medal selector sensor 0
	// bit5: reel 3 stop button (right)
	// bit6: reel 2 stop button (center)
	// bit7: reel 1 stop button (left)

	// Feed raw medal insertion input into the pachislot device to generate
	// timing-chart-compliant sensor waveforms (read via selector_r in PORT_BIT)
	m_pachislot->selector_w(in_fakebtn_r<0>());

	return m_in_port[0]->read();
}

u8 musashi_state::system_status_r(offs_t offset)
{
	// bit0:   reel 1 sensor (left)
	// bit1:   reel 2 sensor (center)
	// bit2:   reel 3 sensor (right)
	// bit3:   reset / door key / change setting value button
	// bit4:   setting key
	// bit5:   SEISAN ("清算") switch (automatic payout)
	// bit6-7: unused

	return m_in_port[1]->read();
}

void musashi_state::reel_phase_w(offs_t offset, u8 data)
{
	// bit0-3: reel 1-3 pulse bit
	// bit4-7: unused
	m_reel[offset]->write(data & 0x0f);
}

void musashi_state::seg_control_w(offs_t offset, u8 data)
{
	// bit0-3: SEG BCD data bit
	// bit4-7: SEG common select bit

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
		0,						// 0x0a
		0,						// 0x0b
		0,						// 0x0c
		0,						// 0x0d
		0,						// 0x0e
		0						// 0x0f
	};

	// SEG common bit -> SEG select
	for (u8 i = 0; i < 4; i++)
		if (BIT(data, i + 4))
			m_segdata[offset][i] = data & 0x0f;

	// SEG credit (SEG bit value -> SEG digit data)
	m_lay_seg_credit[0] = segdigit[m_segdata[0][1]];	// credit SEG high
	m_lay_seg_credit[1] = segdigit[m_segdata[0][0]];	// credit SEG low
	// SEG payout (SEG bit value -> SEG digit data)
	m_lay_seg_payout[0] = segdigit[m_segdata[0][3]];	// payout SEG high
	m_lay_seg_payout[1] = segdigit[m_segdata[0][2]];	// payout SEG low
}

void musashi_state::lamp_latch_bet_w(u8 data)
{
	// bit0-4: bet medal(s) line lamp (physical upper/lower pairs)
	m_lay_lamp_betline[0] = BIT(data, 0);			// 1 medal
	m_lay_lamp_betline[1] = BIT(data, 2);			// 2 medals (lower)
	m_lay_lamp_betline[2] = BIT(data, 1);			// 2 medals (upper)
	m_lay_lamp_betline[3] = BIT(data, 4);			// 3 medals (lower)
	m_lay_lamp_betline[4] = BIT(data, 3);			// 3 medals (upper)

	// bit5-7: stop lamp
	m_lay_lamp_stop[0] = BIT(data, 7);			// left
	m_lay_lamp_stop[1] = BIT(data, 6);			// center
	m_lay_lamp_stop[2] = BIT(data, 5);			// right
}

void musashi_state::lamp_latch_jac_w(u8 data)
{
	// bit0:   "MEDALS" lamp
	// bit1:   "TIME UP" lamp
	// bit2-7: JAC game count lamps (1-6)

	m_lay_lamp_medals = BIT(data, 0);
	m_lay_lamp_timeup = BIT(data, 1);

	m_lay_lamp_jac[0] = BIT(data, 7);
	m_lay_lamp_jac[1] = BIT(data, 6);
	m_lay_lamp_jac[2] = BIT(data, 5);
	m_lay_lamp_jac[3] = BIT(data, 4);
	m_lay_lamp_jac[4] = BIT(data, 3);
	m_lay_lamp_jac[5] = BIT(data, 2);
}

void musashi_state::drive_latch_w(offs_t offset, u8 data)
{
	// bit0-2: frame lamp (left, center, right)
	// bit3-5: bonus count "777" lamps (left, center, right)
	// bit6:   hopper motor drive
	// bit7:   medal lockout solenoid

	// bit0-2: frame lamp
	m_lay_lamp_frame[0] = BIT(data, 2);			// right
	m_lay_lamp_frame[1] = BIT(data, 1);			// center
	m_lay_lamp_frame[2] = BIT(data, 0);			// left

	// bit3-5: bonus count "777" lamp
	m_lay_lamp_777[0] = BIT(data, 5);			// left
	m_lay_lamp_777[1] = BIT(data, 4);			// center
	m_lay_lamp_777[2] = BIT(data, 3);			// right

	// bit6: hopper motor drive (notifies pachislot device to start/stop coin payout cycle)
	m_hopper = BIT(data, 6);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;

	// bit7: medal lockout solenoid
	m_solenoid = !BIT(data, 7);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;
}

void musashi_state::signal_latch_w(offs_t offset, u8 data)
{
	// bit0:   total IN pulse out
	// bit1:   total OUT pulse out
	// bit2:   JAC game signal out
	// bit3:   BIG bonus signal out
	// bit4-7: unused

	// bit0: total IN pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	// bit1: total OUT pulse (drives internal mechanical counter / parlor system)
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	// bit2: JAC game signal out
	m_lay_signal_jac = BIT(data, 2);
	// bit3: BIG bonus signal out
	m_lay_signal_big = BIT(data, 3);
}

void musashi_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x21ff).ram().share("nvram");
	map(0x6000, 0x6000).w(FUNC(musashi_state::irq0_control_w));	// IRQ0 enable/disable latch (asserted on power-fail)
}

void musashi_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(m_ym2149, FUNC(ym2149_device::address_w));
	map(0x01, 0x01).w(m_ym2149, FUNC(ym2149_device::data_w));
	map(0x04, 0x06).w(FUNC(musashi_state::reel_phase_w));
	map(0x08, 0x09).w(FUNC(musashi_state::seg_control_w));
	map(0x0a, 0x0a).w(FUNC(musashi_state::drive_latch_w));
	map(0x0b, 0x0b).w(FUNC(musashi_state::signal_latch_w));
	map(0x0c, 0x0c).r(FUNC(musashi_state::switch_sense_a_r));
	map(0x0d, 0x0d).r(FUNC(musashi_state::system_status_r));
}


static INPUT_PORTS_START( musashi )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(musashi_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal hopper sensor 0
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(musashi_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))	// medal selector sensor 0
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")

	// Cabinet option switches and other inputs
	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(musashi_state::opto_r<0>))	// reel 1 sensor (left)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(musashi_state::opto_r<1>))	// reel 2 sensor (center)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(musashi_state::opto_r<2>))	// reel 3 sensor (right)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Door Key / Reset Button / Change Setting Value Button")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_CONFNAME( 0x20, 0x20, "SEISAN" )
	PORT_CONFSETTING(    0x20, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(musashi_state::powerfail_trigger), 0)

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void musashi_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void musashi_state::musashi_common(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &musashi_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &musashi_state::io_map);
	m_maincpu->set_periodic_int(FUNC(musashi_state::interrupt), attotime::from_hz(500));

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device configuration
	m_pachislot->set_hopper_period_msec(60*2);
	m_pachislot->hopper_cb().set(FUNC(musashi_state::playing_hopper_samples));

	// Configure sensor emulation modes: single-sensor coin selector and single-sensor hopper
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SINGLE);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2149(config, m_ym2149, 4_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "speaker", 0.4);
	m_ym2149->port_a_write_callback().set(FUNC(musashi_state::lamp_latch_bet_w));
	m_ym2149->port_b_write_callback().set(FUNC(musashi_state::lamp_latch_jac_w));

	// samples hardware (plays physical mechanical sounds like hopper medal ejection)
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}

void musashi_state::musashi(machine_config &config)
{
	musashi_common(config);

	// reel device
	REEL(config, m_reel[0], PIONEER_200STEP_REEL, 52, 54, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(musashi_state::opto_cb<0>));
	REEL(config, m_reel[1], PIONEER_200STEP_REEL, 52, 54, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(musashi_state::opto_cb<1>));
	REEL(config, m_reel[2], PIONEER_200STEP_REEL, 52, 54, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(musashi_state::opto_cb<2>));
}

void musashi_state::musashi2(machine_config &config)
{
	musashi_common(config);

	// reel device
	REEL(config, m_reel[0], PIONEER_200STEP_REEL, 50, 52, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(musashi_state::opto_cb<0>));
	REEL(config, m_reel[1], PIONEER_200STEP_REEL, 50, 52, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(musashi_state::opto_cb<1>));
	REEL(config, m_reel[2], PIONEER_200STEP_REEL, 50, 52, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(musashi_state::opto_cb<2>));
}


ROM_START( psmusashi )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "musashi.bin", 0x0000, 0x2000, CRC(a01fb5b7) SHA1(d304d9ada072a6f9394928e351c25cd41ff1277f) )
ROM_END

ROM_START( psmusashi2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "musashi2.bin", 0x0000, 0x2000, CRC(3a80732e) SHA1(2ac689f1a1c009d7e2f2b54862ead969767780f2) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,       PARENT, MACHINE,  INPUT,   CLASS,         INIT,         MONITOR, COMPANY,   FULLNAME,                                        FLAGS,                                      LAYOUT
GAMEL( 1990, psmusashi,  0,      musashi,  musashi, musashi_state, init_musashi, 0,       "Pioneer", "Musashi (2nd Generation, 2nd model, Type A)",   (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_musashi )
GAMEL( 1992, psmusashi2, 0,      musashi2, musashi, musashi_state, init_musashi, 0,       "Pioneer", "Musashi 2 (3rd Generation, 1st model, Type A)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_musashi2 )
