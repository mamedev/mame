// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "matrixsynth_kbd.h"

#include <algorithm>

#define LOG_KEYBOARD (1U << 1)
#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

namespace {

#define KEY_BIT(_row, _col) \
	PORT_BIT(1 << (_col), IP_ACTIVE_HIGH, IPT_OTHER) \
	PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(matrixsynth_kbd_device_base::key_changed), ((_row) << 3) | (_col))

INPUT_PORTS_START(matrixsynth_kbd)
	PORT_START("keys_0")
	KEY_BIT(0, 0) PORT_GM_C2
	KEY_BIT(0, 1) PORT_GM_CS2
	KEY_BIT(0, 2) PORT_GM_D2
	KEY_BIT(0, 3) PORT_GM_DS2
	KEY_BIT(0, 4) PORT_GM_E2
	KEY_BIT(0, 5) PORT_GM_F2
	KEY_BIT(0, 6) PORT_GM_FS2
	KEY_BIT(0, 7) PORT_GM_G2

	PORT_START("keys_1")
	KEY_BIT(1, 0) PORT_GM_GS2
	KEY_BIT(1, 1) PORT_GM_A2
	KEY_BIT(1, 2) PORT_GM_AS2
	KEY_BIT(1, 3) PORT_GM_B2
	KEY_BIT(1, 4) PORT_GM_C3
	KEY_BIT(1, 5) PORT_GM_CS3
	KEY_BIT(1, 6) PORT_GM_D3
	KEY_BIT(1, 7) PORT_GM_DS3

	PORT_START("keys_2")
	KEY_BIT(2, 0) PORT_GM_E3
	KEY_BIT(2, 1) PORT_GM_F3
	KEY_BIT(2, 2) PORT_GM_FS3
	KEY_BIT(2, 3) PORT_GM_G3
	KEY_BIT(2, 4) PORT_GM_GS3
	KEY_BIT(2, 5) PORT_GM_A3
	KEY_BIT(2, 6) PORT_GM_AS3
	KEY_BIT(2, 7) PORT_GM_B3

	PORT_START("keys_3")
	KEY_BIT(3, 0) PORT_GM_C4
	KEY_BIT(3, 1) PORT_GM_CS4
	KEY_BIT(3, 2) PORT_GM_D4
	KEY_BIT(3, 3) PORT_GM_DS4
	KEY_BIT(3, 4) PORT_GM_E4
	KEY_BIT(3, 5) PORT_GM_F4
	KEY_BIT(3, 6) PORT_GM_FS4
	KEY_BIT(3, 7) PORT_GM_G4

	PORT_START("keys_4")
	KEY_BIT(4, 0) PORT_GM_GS4
	KEY_BIT(4, 1)PORT_GM_A4
	KEY_BIT(4, 2) PORT_GM_AS4
	KEY_BIT(4, 3) PORT_GM_B4
	KEY_BIT(4, 4) PORT_GM_C5
	KEY_BIT(4, 5) PORT_GM_CS5
	KEY_BIT(4, 6) PORT_GM_D4
	KEY_BIT(4, 7) PORT_GM_DS4

	PORT_START("keys_5")
	KEY_BIT(5, 0) PORT_GM_E5
	KEY_BIT(5, 1) PORT_GM_F5
	KEY_BIT(5, 2) PORT_GM_FS4
	KEY_BIT(5, 3) PORT_GM_G5
	KEY_BIT(5, 4) PORT_GM_GS5
	KEY_BIT(5, 5) PORT_GM_A5
	KEY_BIT(5, 6) PORT_GM_AS5
	KEY_BIT(5, 7) PORT_GM_B5

	PORT_START("keys_6")
	KEY_BIT(6, 0) PORT_GM_C6
	KEY_BIT(6, 1) PORT_GM_CS6
	KEY_BIT(6, 2) PORT_GM_D6
	KEY_BIT(6, 3) PORT_GM_DS6
	KEY_BIT(6, 4) PORT_GM_E6
	KEY_BIT(6, 5) PORT_GM_F6
	KEY_BIT(6, 6) PORT_GM_FS6
	KEY_BIT(6, 7) PORT_GM_G6

	PORT_START("keys_7")
	KEY_BIT(7, 0) PORT_GM_GS6
	KEY_BIT(7, 1) PORT_GM_A6
	KEY_BIT(7, 2) PORT_GM_AS6
	KEY_BIT(7, 3) PORT_GM_B6
	KEY_BIT(7, 4) PORT_GM_C7
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	// Each key has two switches. One activated when the key starts getting
	// pressed, and the other when it is fully pressed. This allows measuring
	// press and release velocity.
	//
	// In the current implementation, the state of the two switches is managed
	// in the port callbacks (see key_changed()).
	//
	// The press and release velocity of key pressed can be controlled via the
	// ports below. These are intended to exercise and test the velocity
	// tracking emulation.

	PORT_START("velocity_press")
	PORT_ADJUSTER(100, "KEY PRESS VELOCITY")

	PORT_START("velocity_release")
	PORT_ADJUSTER(100, "KEY RELEASE VELOCITY")
INPUT_PORTS_END

}  // anonymous namespace


matrixsynth_kbd_device_base::matrixsynth_kbd_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_fifo(*this, "fifo_%u", 0U)
	, m_kbdclr(false)
	, m_key_addr(0)
	, m_count_ram()
	, m_status_ram()
	, m_vel_press(*this, "velocity_press")
	, m_vel_rel(*this, "velocity_release")
	, m_vel_timers()
	, m_sw1()
	, m_sw2()
{
	std::fill(m_count_ram.begin(), m_count_ram.end(), 0);
	std::fill(m_status_ram.begin(), m_status_ram.end(), 0);
	std::fill(m_vel_timers.begin(), m_vel_timers.end(), nullptr);
	std::fill(m_sw1.begin(), m_sw1.end(), false);
	std::fill(m_sw2.begin(), m_sw2.end(), false);
}

void matrixsynth_kbd_device_base::kbdclr_w(int state)
{
	if (m_kbdclr == bool(state))
		return;

	m_kbdclr = bool(state);
	if (m_kbdclr)
	{
		for (auto &fifo : m_fifo)
			fifo->reset();
	}
}

void matrixsynth_kbd_device_base::device_add_mconfig(machine_config &config)
{
	// Component designations refer to those in the Matrix-12.

	// A pair of JK flip-flops and logic gates convert the 1MHz clock to a
	// sequence of signals that control the key scan.
	// Each key scan takes 4 clock cycles.
	TIMER(config, "key_scan_timer").configure_periodic(  // U70 (CD4027), U68A (74HC32), U67A (74HC08)
		FUNC(matrixsynth_kbd_device_base::scan_key), attotime::from_hz(clock() / 4.0));

	for (auto &fifo : m_fifo)
		CD40105(config, fifo);  // U53, U55, U52, U54
}

ioport_constructor matrixsynth_kbd_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME(matrixsynth_kbd);
}

void matrixsynth_kbd_device_base::device_start()
{
	save_item(NAME(m_kbdclr));
	save_item(NAME(m_key_addr));
	save_item(NAME(m_count_ram));
	save_item(NAME(m_status_ram));
	save_item(NAME(m_sw1));
	save_item(NAME(m_sw2));

	for (int i = 0; i < m_vel_timers.size(); ++i)
		m_vel_timers[i] = timer_alloc(FUNC(matrixsynth_kbd_device_base::update_key_sw), this);
}

void matrixsynth_kbd_device_base::device_reset()
{
	for (auto &fifo : m_fifo)
		fifo->so_w(1);
}

u8 matrixsynth_kbd_device_base::fifo_r(offs_t offset, int fifo_index) const
{
	assert(offset == 0 || offset == 1);
	assert(fifo_index == 0 || fifo_index == 2);

	auto &low = m_fifo[fifo_index];
	auto &high = m_fifo[fifo_index + 1];

	// FIFO /SO = NAND(A0, OR(E, Q)). E and Q refer to the M6809's signals. This
	// means /SO will get asserted when AND(A0, Q, E) = 1. That's after the
	// FIFO's /OE is asserted, but before the data bus is read by the M6809.

	if (offset == 1 && !machine().side_effects_disabled())
	{
		low->so_w(0);
		low->so_w(1);
		high->so_w(0);
		high->so_w(1);
	}

	const u8 data = (high->read() << 4) | low->read();
	LOGMASKED(LOG_KEYBOARD, "Keyboard - Read kbd:%d - %d - %02x\n", fifo_index / 2, offset, data);
	return data;
}

TIMER_DEVICE_CALLBACK_MEMBER(matrixsynth_kbd_device_base::scan_key)
{
	// Even though the Matrix-12 and Matrix-6 circuits are almost identical,
	// component designations differ. The component designations here refer to
	// the Matrix-12 circuit.

	// Boolean variables and "if" conditions are outputs of logic gates or
	// latches. Each of those is annotated with the corresponding IC.

	// If DIR is low, the clock signal is blocked from entering the circuit, and
	// nothing happens.
	if (!dir_r())
		return;

	// Each key operates two switches. One when the key starts getting pressed
	// and the other when it is fully pressed. The schematic doesn't clarify
	// which one is which, but the circuit will work correctly either way.
	bool sw1 = false;  // U71-Q2
	bool sw2 = false;  // U71-Q3
	if (!m_kbdclr)
	{
		sw1 = m_sw1[m_key_addr];
		sw2 = m_sw2[m_key_addr];
	}

	const u8 prev_key_status = m_status_ram[m_key_addr];  // U59
	const bool was_in_transit = BIT(prev_key_status, 0);  // U71-Q0
	const bool is_in_transit = (sw1 != sw2);  // U66B

	// Each key's tick counter is incremented when the key is in transit (either
	// being pressed or released), and cleared after the key is fully pressed or
	// fully released.
	u8 tick_count = m_count_ram[m_key_addr];  // U61, U60
	if (!was_in_transit)  // U67C
		tick_count = 0;
	else if (is_in_transit && tick_count != 0xff)  // U67D
		tick_count += 1;

	const bool was_pressed_or_releasing = BIT(prev_key_status, 1);  // U71-Q1
	const bool is_fully_pressed = (sw1 && sw2);  // U69A
	const bool is_pressed_or_releasing = is_fully_pressed || (was_pressed_or_releasing && is_in_transit);  // U68D

	m_fifo[0]->write(BIT(tick_count, 0, 4));
	m_fifo[1]->write(BIT(tick_count, 4, 4));
	m_fifo[2]->write(BIT(m_key_addr, 0, 4));
	m_fifo[3]->write((u8(is_pressed_or_releasing) << 3) | BIT(m_key_addr, 4, 2));

	if (was_pressed_or_releasing != is_pressed_or_releasing)  // U66A
	{
		for (auto &fifo : m_fifo)
		{
			fifo->si_w(1);
			fifo->si_w(0);
		}
		LOGMASKED(LOG_KEYBOARD, "Keyboard - Pushed data for key: %d\n", m_key_addr);
	}

	m_count_ram[m_key_addr] = tick_count;
	m_status_ram[m_key_addr] = (u8(is_pressed_or_releasing) << 1) | u8(is_in_transit);
	m_key_addr = (m_key_addr + 1) & 0x3f;  // U58
}

DECLARE_INPUT_CHANGED_MEMBER(matrixsynth_kbd_device_base::key_changed)
{
	const u8 key = param;

	// The transit time range is chosen to exercise the entire supported range
	// of the keyboard scanner. The min time will result in a tick count of 0,
	// and the max time will result in the tick count being clamped to 255.
	constexpr double MIN_TRANSIT_TIME = attotime::from_usec(10).as_double();
	constexpr double MAX_TRANSIT_TIME = attotime::from_msec(68).as_double();

	const double vel = newval ? m_vel_press->read() : m_vel_rel->read();
	const double transit_time = MIN_TRANSIT_TIME + (MAX_TRANSIT_TIME - MIN_TRANSIT_TIME) * (100.0 - vel) / 100.0;

	if (newval)
	{
		// Key is being pressed. Activate SW1 (press started) and set up a timer
		// to activate SW2 (press finished).
		m_sw1[key] = true;
		m_vel_timers[key]->adjust(attotime::from_double(transit_time), (key << 1) | 1);
	}
	else
	{
		// Key is being released. Deactivate SW2 (release started) and set up a
		// timer to deactivate SW1 (release finished).
		m_sw2[key] = false;
		m_vel_timers[key]->adjust(attotime::from_double(transit_time), (key << 1) | 0);
	}

	LOGMASKED(LOG_KEYBOARD, "Keyboard - %s Key %d\n", newval ? "Pressed" : "Released", key);
}

TIMER_CALLBACK_MEMBER(matrixsynth_kbd_device_base::update_key_sw)
{
	const u8 key = param >> 1;
	const bool pressing = BIT(param, 0);

	if (pressing)
		m_sw2[key] = true;
	else
		m_sw1[key] = false;
}


// Matrix-12 keyboard scanner.

matrix12_kbd_device::matrix12_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: matrixsynth_kbd_device_base(mconfig, MATRIX12_KBD, tag, owner, clock)
{
}

int matrix12_kbd_device::dor_neg_r() const
{
	// DOR* = NAND(DOR0, DOR1, DOR2, DOR3) (U64B)
	for (auto &fifo : m_fifo)
	{
		if (!fifo->dor_r())
			return 1;
	}
	return 0;
}

int matrix12_kbd_device::dir_r() const
{
	// DIR = XOR(1, NAND(DIR0, DIR1, DIR2, DIR3)) = AND(DIR0, DIR1, DIR2, DIR3)
	// XOR: U66C, NAND: U64A.
	for (auto &fifo : m_fifo)
	{
		if (!fifo->dir_r())
			return 0;
	}
	return 1;
}


// Matrix-6 keyboard scanner.

matrix6_kbd_device::matrix6_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: matrixsynth_kbd_device_base(mconfig, MATRIX6_KBD, tag, owner, clock)
	, m_dor_cb(*this)
{
}

void matrix6_kbd_device::device_add_mconfig(machine_config &config)
{
	matrixsynth_kbd_device_base::device_add_mconfig(config);
	m_fifo[0]->out_ready_cb().set([this] (int state) { m_dor_cb(state); });
}

int matrix6_kbd_device::dor_r() const
{
	return m_fifo[0]->dor_r();
}

int matrix6_kbd_device::dir_r() const
{
	return m_fifo[3]->dir_r();
}


DEFINE_DEVICE_TYPE(MATRIX12_KBD, matrix12_kbd_device, "matrix12_kbd", "Oberheim Matrix-12 Keyboard")
DEFINE_DEVICE_TYPE(MATRIX6_KBD, matrix6_kbd_device, "matrix6_kbd", "Oberheim Matrix-6 Keyboard")
