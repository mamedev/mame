// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Citizen 120D 9-pin dot matrix print mechanism (skeleton)

**********************************************************************/

#include "emu.h"
#include "citizen120d.h"


namespace {

constexpr int PAPER_HDPI   = 240;
constexpr int PAPER_VDPI   = 72;
constexpr int PAPER_WIDTH  = 8 * PAPER_HDPI;
constexpr int PAPER_HEIGHT = 12 * PAPER_VDPI;

// 1/240" columns the carriage rotor trails the phase the firmware has latched
constexpr int CR_STEP_LAG = 2;

// a wire's dot is roughly 1/60" across, so it covers four 1/240" columns --
// without that the half-dot offsets the fonts use to round off corners read as
// one-pixel jogs instead of smooth strokes
constexpr int DOT_WIDTH = 4;


// the M40994 drives the coils active low, and pairs bits 3/1 and 2/0 where
// stepper_device's table pairs 3/2 and 1/0
constexpr u8 drive_pattern(u8 nibble)
{
	return bitswap<4>(u8(~nibble), 3, 1, 2, 0);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE(CITIZEN_120D, citizen_120d_device, "citizen_120d", "Citizen 120D Print Mechanism")


citizen_120d_device::citizen_120d_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CITIZEN_120D, tag, owner, clock),
	m_bitmap_printer(*this, "bitmap"),
	m_cr_stepper(*this, "bitmap:cr_stepper"),
	m_pf_stepper(*this, "bitmap:pf_stepper"),
	m_phase_data(0),
	m_solenoid_data(0),
	m_cr_delta(0),
	m_fire_slot(0),
	m_head_en(false)
{
}


void citizen_120d_device::device_add_mconfig(machine_config &config)
{
	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, PAPER_HDPI, PAPER_VDPI);
	m_bitmap_printer->set_continuous_feed(true);

	m_bitmap_printer->set_cr_stepper_ratio(2, 1);
	m_bitmap_printer->set_pf_stepper_ratio(1, 6);
}


void citizen_120d_device::device_start()
{
	save_item(NAME(m_phase_data));
	save_item(NAME(m_solenoid_data));
	save_item(NAME(m_cr_delta));
	save_item(NAME(m_fire_slot));
	save_item(NAME(m_head_en));
}


void citizen_120d_device::device_reset()
{
	m_phase_data = 0;
	m_solenoid_data = 0;
	m_cr_delta = 0;
	m_fire_slot = 0;
	m_head_en = false;
}


void citizen_120d_device::phase_w(u8 data)
{
	u8 const last = m_phase_data;
	m_phase_data = data;

	if ((data & 0xf0) != (last & 0xf0))
	{
		int const before = m_cr_stepper->get_absolute_position();
		m_bitmap_printer->update_cr_stepper(bitswap<4>(drive_pattern(data >> 4), 0, 1, 2, 3));
		m_cr_delta = m_cr_stepper->get_absolute_position() - before;
		m_fire_slot = 0;
	}

	if ((data & 0x0f) != (last & 0x0f))
		m_bitmap_printer->update_pf_stepper(drive_pattern(data));
}


void citizen_120d_device::solenoid_w(u16 data)
{
	m_solenoid_data = data & 0x1ff;
}


void citizen_120d_device::head_en_w(int state)
{
	bool const en = !state;
	if (en == m_head_en)
		return;
	m_head_en = en;

	if (!en)
		return;

	// the head fires twice per carriage phase, the second one at the phase's own
	// position and the first half a phase short of it -- so the column pitch
	// follows the drive: draft half steps the carriage twice per column and
	// prints 1/120" apart, NLQ steps once and prints 1/240" apart. The rotor
	// also trails the latched phase, and without that term the two print
	// directions land two columns apart (measured against the self-test's
	// one-character-per-line shift, which only comes out at exactly 24 columns
	// of 1/240" with it)
	int const x = m_bitmap_printer->m_xpos - m_bitmap_printer->m_cr_direction * CR_STEP_LAG
			- (m_fire_slot ? 0 : m_cr_delta);
	if (m_fire_slot < 1)
		m_fire_slot++;

	int const y = m_bitmap_printer->m_ypos;

	m_bitmap_printer->check_new_page();

	if (x < 0)
		return;

	// TODO: pin pitch not traced, one dot row per pin
	for (int pin = 0; pin < 9; pin++)
		if (BIT(m_solenoid_data, pin))
			for (int dot = 0; dot < DOT_WIDTH; dot++)
				m_bitmap_printer->draw_pixel(x + dot, y + pin, 0x000000);
}


bool citizen_120d_device::at_home() const
{
	return m_bitmap_printer->m_xpos >= 0;
}


void citizen_120d_device::fault_led_w(int state)
{
	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ERROR, state);
}


void citizen_120d_device::ready_led_w(int state)
{
	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_READY, state);
}
