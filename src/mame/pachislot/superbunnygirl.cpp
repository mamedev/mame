// license:BSD-3-Clause
// copyright-holders:Chiaki Tachibana
/***************************************************************************

    Olympia Pachi-Slot Series Driver
    Z80 + 3-reel + YM2413 titles

    Driver by Chiaki Tachibana

    Initial driver development started on 2026-04-29.

    Supported Games:
    - Super Bunny Girl  (c) 1990 Olympia  (2nd Generation, 2nd model, Type A)

    Hardware:
    - Main CPU: Z80 @ provisional 5.0 MHz (10.0 MHz XTAL / 2)
    - Sound: YM2413 @ 3.579545 MHz
    - Stepper Motors: 3-reel (200-step, 2-phase unipolar)
    - NVRAM: 512 bytes battery-backed SRAM (0x4000-0x41ff)

    TODO:
    - Confirm the physical CPU clock divider from genuine hardware.
    - Top panel outer lamp layout consists of 12 physical lamp segments,
      but only 6 control bits are mapped in firmware; currently mirrored
      as an approximation.

    NOTE:
    [Periodic IM 2 Scanning]
    - The CPU runs in IM 2 (I = 0x11). A 2048 Hz scan timer sequentially
      services four separate interrupt vectors (0x06: main periodic,
      0x00: reel 1, 0x02: reel 2, 0x04: reel 3) yielding a 512 Hz service
      rate per vector.

    [Power-fail (PFI) Simulation and Core Architecture Note]
    - Real hardware relies on a dedicated power-monitoring circuit (PFI) to
      signal an impending power loss, allowing the CPU to execute its
      state-saving routine into battery-backed NVRAM during the reservoir
      capacitance hold-up time.
    - On genuine hardware, the power supervisor drops the NMI line and asserts
      a low-active status flag on port 0x800a bit 7 once the backup sequence is
      acknowledged.
    - Because MAME currently lacks a pre-reset / pre-shutdown callback
      mechanism to allow the CPU to gracefully complete its power-down
      sequence before machine_reset() or exit (e.g. F3 / ESC keys), this
      driver provisionally provides a manual 'P' key (powerfail_trigger).
    - Pressing the 'P' key pulses the NMI line, sets m_powerfail_status_ready
      to simulate the active-low completion status on port 0x800a bit 7, and
      preserves the physical reel absolute positions via stepper_device helpers.
    - For seamless out-of-the-box power restoration in the future, MAME core
      framework support for graceful shutdown execution and non-volatile
      stepper position persistence will be required.

    [Bookkeeping / Mechanical Counters]
    - Total IN (bit 4) and Total OUT (bit 5) pulse outputs from port 0x8007
      are mapped to coin_counter_w(0/1). These lines physically pulsed the
      internal 6-digit electromechanical counters and connected to the
      centralized parlor management terminal.

    [Setting Change Procedure]
    - Standard 2nd generation sequence:
      1. Turn Setting Key (F2) on and restart/reset machine.
      2. Select desired setting (1-6) on 7-segment display with Door Reset / CSV button (F1).
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
#include "machine/steppers.h"
#include "machine/nvram.h"
#include "sound/ymopl.h"
#include "sound/samples.h"
#include "speaker.h"


namespace {

// internal artwork
#include "spbunnygirl.lh"

class spbunnygirl_state : public driver_device
{
public:
	spbunnygirl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pachislot(*this, "pachislot")
		, m_ym2413(*this, "ym2413")
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

	void spbunnygirl(machine_config &config) ATTR_COLD;

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

	template <unsigned N> void opto_cb(int state) { m_opto[N] = state; }

	void spbunnygirl_mem_map(address_map &map) ATTR_COLD;
	void spbunnygirl_io_map(address_map &map) ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_shared_ptr<u8> m_nvram;
	required_device<pachislot_device> m_pachislot;
	required_device<ym2413_device> m_ym2413;
	required_device<samples_device> m_samples;
	required_device_array<stepper_device, 3> m_reel;
	required_ioport_array<2> m_in_port;
	required_ioport m_in_fakebtn;
	output_finder<2> m_lay_lamp_toppanelc;
	output_finder<12> m_lay_lamp_toppanelw;
	output_finder<> m_lay_lamp_insert;
	output_finder<> m_lay_lamp_gameover;
	output_finder<5> m_lay_lamp_betline;
	output_finder<3> m_lay_lamp_stop;
	output_finder<40> m_lay_led_waitlampl;
	output_finder<2> m_lay_seg_jacbb;
	output_finder<2> m_lay_seg_credit;
	output_finder<2> m_lay_seg_payout;
	output_finder<> m_lay_signal_big;
	output_finder<> m_lay_signal_jac;
	output_finder<> m_lay_signal_hopper;
	output_finder<> m_lay_signal_solenoid;

	TIMER_CALLBACK_MEMBER(irq_scan_tick);

	emu_timer *m_irq_scan_timer = nullptr;
	u8 m_irq_scan_phase = 0;

	u8 m_irq_vector = 0;
	int irq_ack(device_t &device, int irqline) { return m_irq_vector; }

	u8 spbunnygirl_mem8008_r(offs_t offset);
	u8 spbunnygirl_mem800A_r(offs_t offset);
	void spbunnygirl_mem8000_w(offs_t offset, u8 data);
	void spbunnygirl_mem8001_w(offs_t offset, u8 data);
	void spbunnygirl_mem8002_w(offs_t offset, u8 data);
	void spbunnygirl_mem8003_w(offs_t offset, u8 data);
	void spbunnygirl_mem8004_w(offs_t offset, u8 data);
	void spbunnygirl_mem8005_w(offs_t offset, u8 data);
	void spbunnygirl_mem8006_w(offs_t offset, u8 data);
	void spbunnygirl_mem8007_w(offs_t offset, u8 data);

	void playing_hopper_samples(int state);

	powerfail_snapshot m_powerfail_snapshot;

	// Latch flag simulating the external PFI status line asserted by the power supervisor
	bool m_powerfail_status_ready = false;

	u8 m_opto[3];
	u8 m_segdata[6];
	u8 m_common;
	u8 m_hopper;
	u8 m_solenoid;
	u8 m_samples_ch;

	u8 m_wait_led_latch[5];
};


void spbunnygirl_state::machine_start()
{
	m_irq_scan_timer = timer_alloc(FUNC(spbunnygirl_state::irq_scan_tick), this);

	// register for savestates
	save_item(NAME(m_opto));
	save_item(NAME(m_segdata));
	save_item(NAME(m_common));
	save_item(NAME(m_hopper));
	save_item(NAME(m_solenoid));
	save_item(NAME(m_samples_ch));

	save_item(NAME(m_wait_led_latch));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_powerfail_snapshot.reel_abs_pos));
	save_item(NAME(m_powerfail_snapshot.valid));
	save_item(NAME(m_powerfail_status_ready));

	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;
	std::fill(std::begin(m_powerfail_snapshot.reel_abs_pos), std::end(m_powerfail_snapshot.reel_abs_pos), 0);
}

void spbunnygirl_state::machine_reset()
{
	m_irq_scan_phase = 0;
	m_irq_scan_timer->adjust(attotime::zero, 0, attotime::from_hz(2048));

	for (u8 &segdata : m_segdata)
		segdata = 0;
	m_common = 0;
	for (u8 &wait_led : m_wait_led_latch)
		wait_led = 0;

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_hopper = 0;
	m_solenoid = 0;
	m_samples_ch = 0;
	m_irq_vector = 0;

	powerfail_restore_state();
	m_powerfail_snapshot.valid = false;
	m_powerfail_status_ready = false;

	m_pachislot->reset_selector();
	m_pachislot->reset_hopper();
}

// ---------------------------------------------------------------------------
//  Power-fail (PFI) Simulation Helpers
// ---------------------------------------------------------------------------
void spbunnygirl_state::powerfail_prepare_state()
{
	for (int i = 0; i < 3; i++)
		m_powerfail_snapshot.reel_abs_pos[i] = m_reel[i]->get_absolute_position();

	m_powerfail_snapshot.valid = true;
}

void spbunnygirl_state::powerfail_restore_state()
{
	if (!m_powerfail_snapshot.valid)
		return;

	for (int i = 0; i < 3; i++)
		m_reel[i]->restore_position(m_powerfail_snapshot.reel_abs_pos[i]);
}

// Provisional trigger handler simulating the physical power failure supervisor circuit.
INPUT_CHANGED_MEMBER(spbunnygirl_state::powerfail_trigger)
{
	if (newval && !oldval)
	{
		powerfail_prepare_state();
		m_powerfail_status_ready = true;
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}


TIMER_CALLBACK_MEMBER(spbunnygirl_state::irq_scan_tick)
{
	static constexpr u8 vectors[4] = { 0x06, 0x00, 0x02, 0x04 };

	m_irq_vector = vectors[m_irq_scan_phase & 3];
	m_maincpu->set_input_line(0, HOLD_LINE);
	m_irq_scan_phase++;
}

u8 spbunnygirl_state::spbunnygirl_mem8008_r(offs_t offset)
{
	m_pachislot->selector_w(in_fakebtn_r<0>());
	return m_in_port[0]->read();
}

u8 spbunnygirl_state::spbunnygirl_mem800A_r(offs_t offset)
{
	u8 port = m_in_port[1]->read();

	if (m_powerfail_status_ready)
		port &= 0b01111111;

	return port;
}

void spbunnygirl_state::spbunnygirl_mem8000_w(offs_t offset, u8 data)
{
	m_common = data & 0x3f;
}

void spbunnygirl_state::spbunnygirl_mem8001_w(offs_t offset, u8 data)
{
	for (u8 common = 0; common < 6; common++)
		if (BIT(m_common, common))
			m_segdata[common] = data;

	m_lay_seg_jacbb[0] = m_segdata[5];
	m_lay_seg_jacbb[1] = m_segdata[4];

	m_lay_seg_credit[0] = m_segdata[3];
	m_lay_seg_credit[1] = m_segdata[2];

	m_lay_seg_payout[0] = m_segdata[1];
	m_lay_seg_payout[1] = m_segdata[0];
}

void spbunnygirl_state::spbunnygirl_mem8002_w(offs_t offset, u8 data)
{
	for (u8 common = 0; common < 5; common++)
		if (BIT(m_common, common))
			m_wait_led_latch[common] = data;

	for (int common = 0; common < 5; common++)
		for (int bit = 0; bit < 8; bit++)
			m_lay_led_waitlampl[common * 8 + bit] = BIT(m_wait_led_latch[common], bit);
}

void spbunnygirl_state::spbunnygirl_mem8003_w(offs_t offset, u8 data)
{
	m_lay_lamp_toppanelw[0] = m_lay_lamp_toppanelw[6]  = BIT(data, 0);
	m_lay_lamp_toppanelw[1] = m_lay_lamp_toppanelw[7]  = BIT(data, 1);
	m_lay_lamp_toppanelw[2] = m_lay_lamp_toppanelw[8]  = BIT(data, 2);
	m_lay_lamp_toppanelw[3] = m_lay_lamp_toppanelw[9]  = BIT(data, 3);
	m_lay_lamp_toppanelw[4] = m_lay_lamp_toppanelw[10] = BIT(data, 4);
	m_lay_lamp_toppanelw[5] = m_lay_lamp_toppanelw[11] = BIT(data, 5);
}

void spbunnygirl_state::spbunnygirl_mem8004_w(offs_t offset, u8 data)
{
	m_lay_lamp_stop[0] = BIT(data, 0);
	m_lay_lamp_stop[1] = BIT(data, 1);
	m_lay_lamp_stop[2] = BIT(data, 2);

	for (int i = 0; i < 5; i++)
		m_lay_lamp_betline[i] = BIT(data, i + 3);
}

void spbunnygirl_state::spbunnygirl_mem8005_w(offs_t offset, u8 data)
{
	m_reel[0]->write(data & 0x0f);

	m_lay_lamp_insert = BIT(data, 4);
	m_lay_lamp_gameover = BIT(data, 6);

	m_lay_lamp_toppanelc[0] = BIT(data, 5);
	m_lay_lamp_toppanelc[1] = BIT(data, 7);
}

void spbunnygirl_state::spbunnygirl_mem8006_w(offs_t offset, u8 data)
{
	m_reel[1]->write(data & 0x0f);

	m_solenoid = !BIT(data, 4);
	m_pachislot->selector_lock_w(m_solenoid);
	m_lay_signal_solenoid = m_solenoid;

	m_hopper = BIT(data, 5);
	m_pachislot->hopper_motor_w(m_hopper);
	m_lay_signal_hopper = m_hopper;

	m_lay_signal_jac = BIT(data, 6);
	m_lay_signal_big = BIT(data, 7);
}

void spbunnygirl_state::spbunnygirl_mem8007_w(offs_t offset, u8 data)
{
	m_reel[2]->write(data & 0x0f);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
}


void spbunnygirl_state::spbunnygirl_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x41ff).ram().share("nvram");
	map(0x8000, 0x8000).w(FUNC(spbunnygirl_state::spbunnygirl_mem8000_w));
	map(0x8001, 0x8001).w(FUNC(spbunnygirl_state::spbunnygirl_mem8001_w));
	map(0x8002, 0x8002).w(FUNC(spbunnygirl_state::spbunnygirl_mem8002_w));
	map(0x8003, 0x8003).w(FUNC(spbunnygirl_state::spbunnygirl_mem8003_w));
	map(0x8004, 0x8004).w(FUNC(spbunnygirl_state::spbunnygirl_mem8004_w));
	map(0x8005, 0x8005).w(FUNC(spbunnygirl_state::spbunnygirl_mem8005_w));
	map(0x8006, 0x8006).w(FUNC(spbunnygirl_state::spbunnygirl_mem8006_w));
	map(0x8007, 0x8007).w(FUNC(spbunnygirl_state::spbunnygirl_mem8007_w));
	map(0x8008, 0x8008).r(FUNC(spbunnygirl_state::spbunnygirl_mem8008_r));
	map(0x800a, 0x800a).r(FUNC(spbunnygirl_state::spbunnygirl_mem800A_r));
	map(0xc000, 0xc001).w(m_ym2413, FUNC(ym2413_device::write));
}

void spbunnygirl_state::spbunnygirl_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x04).nopw();
	map(0x10, 0x13).nopw();
}


static INPUT_PORTS_START( spbunnygirl )

	PORT_START("INPUT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Reel Stop 1 (Left)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Reel Stop 2 (Center)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Reel Stop 3 (Right)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(spbunnygirl_state::selector_r<pachislot_device::PACHISLOT_SENSOR0>))
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Lever")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(spbunnygirl_state::hopper_r<pachislot_device::PACHISLOT_SENSOR0>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Bet Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out / Credit Game Button")

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(spbunnygirl_state::opto_r<0>))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(spbunnygirl_state::opto_r<1>))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(spbunnygirl_state::opto_r<2>))
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_R) PORT_NAME("Door Key / Reset Button")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE PORT_NAME("Setting Key")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Change Setting Value Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )

	PORT_START("PWRFAIL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_P) PORT_NAME("Power Fail Preparation") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spbunnygirl_state::powerfail_trigger), 0)

	PORT_START("FAKEBTN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Medal In")

INPUT_PORTS_END


static const char *const pachislot_samples_names[] =
{
	"*pachislot",
	"medalout",
	nullptr
};

void spbunnygirl_state::playing_hopper_samples(int state)
{
	if (state)
	{
		m_samples->start(m_samples_ch & 1, 0);
		m_samples_ch++;
	}
}


void spbunnygirl_state::spbunnygirl(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10_MHz_XTAL / 2); // 5.0 MHz provisional
	m_maincpu->set_addrmap(AS_PROGRAM, &spbunnygirl_state::spbunnygirl_mem_map);
	m_maincpu->set_addrmap(AS_IO, &spbunnygirl_state::spbunnygirl_io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(spbunnygirl_state::irq_ack));

	// battery backup device
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Pachi-Slot medal simulator device
	PACHISLOT(config, m_pachislot, 0);

	// hopper device
	m_pachislot->set_hopper_period_msec(55*2);
	m_pachislot->hopper_cb().set(FUNC(spbunnygirl_state::playing_hopper_samples));

	// medal selector device type & hopper sensor device type setting
	m_pachislot->init_selector(pachislot_device::DEVSELECTOR_SINGLE);
	m_pachislot->init_hopper(pachislot_device::DEVHOPPER_SINGLE);

	// reel device
	REEL(config, m_reel[0], OLYMPIA_200STEP_REEL, 362, 364, 0x00, 0, 200*2);
	m_reel[0]->optic_handler().set(FUNC(spbunnygirl_state::opto_cb<0>));
	REEL(config, m_reel[1], OLYMPIA_200STEP_REEL, 362, 364, 0x00, 0, 200*2);
	m_reel[1]->optic_handler().set(FUNC(spbunnygirl_state::opto_cb<1>));
	REEL(config, m_reel[2], OLYMPIA_200STEP_REEL, 362, 364, 0x00, 0, 200*2);
	m_reel[2]->optic_handler().set(FUNC(spbunnygirl_state::opto_cb<2>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.6);

	// samples hardware
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(pachislot_samples_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.2);
}


ROM_START( psspbgirl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superbunnygirl.bin", 0x0000, 0x2000, CRC(74aecad6) SHA1(934930dfcbe24f55bff270e390d4c9aa04829908) )
ROM_END

} // anonymous namespace


//     YEAR, NAME,      PARENT, MACHINE,     INPUT,       CLASS,             INIT,       MONITOR, COMPANY,   FULLNAME,                                    FLAGS,                                      LAYOUT
GAMEL( 1990, psspbgirl, 0,      spbunnygirl, spbunnygirl, spbunnygirl_state, empty_init, 0,       "Olympia", "Super Bunny Girl (2nd Generation, Type A)", (MACHINE_SUPPORTS_SAVE|MACHINE_MECHANICAL), layout_spbunnygirl )
