// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ALPS DPG23 4-pen X/Y plotter mechanism

**********************************************************************/

#include "emu.h"
#include "alpsdpg23.h"


namespace {

constexpr int PAPER_WIDTH  = 560; // pixels, 127 dpi -- 480 drawable steps plus right margin
// only the most recent PAPER_HEIGHT rows of roll stay live; older paper is
// filed and comes back blank. "d" is absolute and clamps at about +/-999, so
// sizing the ring over that 1999-row span plus margin keeps any redraw a job
// can address on live paper
constexpr int PAPER_HEIGHT = 2100;
constexpr int PAPER_DPI    = 127; // 1 motor step = 0.2 mm = 1/127"

constexpr u32 PEN_COLOUR[4] = { 0x000000, 0x0000cc, 0x008800, 0xcc0000 };

} // anonymous namespace


DEFINE_DEVICE_TYPE(ALPS_DPG23, alps_dpg23_device, "alps_dpg23", "ALPS DPG23 Plotter Mechanism")


alps_dpg23_device::alps_dpg23_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, ALPS_DPG23, tag, owner, clock),
	m_bitmap_printer(*this, "bitmap"),
	m_cr_stepper(*this, "bitmap:cr_stepper"),
	m_pf_stepper(*this, "bitmap:pf_stepper"),
	m_pd_data(0),
	m_pc_data(0xff),
	m_pen_down(false),
	m_x_dir(0),
	m_y_homed(false),
	m_ratchet(0),
	m_sensor(1)
{
}


void alps_dpg23_device::device_add_mconfig(machine_config &config)
{
	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, PAPER_DPI, PAPER_DPI);
	m_bitmap_printer->set_continuous_feed(true);
	m_bitmap_printer->set_cr_stepper_ratio(1, 2);
	m_bitmap_printer->set_pf_stepper_ratio(-1, 2);
}


void alps_dpg23_device::device_start()
{
	save_item(NAME(m_pd_data));
	save_item(NAME(m_pc_data));
	save_item(NAME(m_pen_down));
	save_item(NAME(m_x_dir));
	save_item(NAME(m_y_homed));
	save_item(NAME(m_ratchet));
	save_item(NAME(m_sensor));
}


void alps_dpg23_device::device_reset()
{
	m_pd_data = 0;
	m_pc_data = 0xff;
	m_pen_down = false;
	m_x_dir = 0;
	m_y_homed = false;

	m_ratchet = 6;
	set_sensor(m_ratchet == 0);
}


//-------------------------------------------------
//  motor_w - PD0-3 X phase, PD4-7 Y phase, both
//  matching stepper_device's standard drive table
//-------------------------------------------------

void alps_dpg23_device::motor_w(u8 data)
{
	u8 const last = m_pd_data;
	m_pd_data = data;

	if ((data & 0x0f) != (last & 0x0f))
	{
		int const before = m_cr_stepper->get_absolute_position();
		m_bitmap_printer->update_cr_stepper(data & 0x0f);
		int const delta = m_cr_stepper->get_absolute_position() - before;
		if (machine().time().seconds() > 680) logerror("DBG t=%.2f motor_w X data=%02x pos=%d delta=%d ratchet=%d pen_down=%d\n", machine().time().as_double(), data, m_cr_stepper->get_absolute_position(), delta, m_ratchet, m_pen_down);
		step_x(delta);
		draw();
	}

	if ((data & 0xf0) != (last & 0xf0))
	{
		m_bitmap_printer->update_pf_stepper((data >> 4) & 0x0f);
		if (machine().time().seconds() > 680) logerror("DBG t=%.2f motor_w Y data=%02x ypos=%d\n", machine().time().as_double(), data, m_bitmap_printer->m_ypos);
		draw();
	}
}


//-------------------------------------------------
//  pen_w - PC0 pen-up solenoid, PC1 pen-down
//  solenoid, both active low and self-holding
//-------------------------------------------------

void alps_dpg23_device::pen_w(u8 data)
{
	u8 const last = m_pc_data;
	m_pc_data = data;

	if (!BIT(data, 1) && BIT(last, 1))
		m_pen_down = true;
	else if (!BIT(data, 0) && BIT(last, 0))
		m_pen_down = false;
}


//-------------------------------------------------
//  step_x - track the X carriage direction and
//  advance the colour-change ratchet
//-------------------------------------------------

void alps_dpg23_device::step_x(int delta)
{
	if (delta == 0)
		return;

	int const dir = (delta > 0) ? 1 : -1;

	// motor_w() applies delta before calling here, so undo it for the
	// pre-step position: without that, a reversal landing exactly on 0 reads
	// as "not in the bay" and the ratchet never advances again
	int const pos_before = m_cr_stepper->get_absolute_position() - delta;

	// the bay is the left end stop; each outward jog rotates the holder one
	// tooth. The tooth-0 re-zero below lands a step or two right of the
	// physical stop, hence the tolerance. Pen-up is what separates a colour
	// change from a plot line that merely reaches the left margin
	if (dir > 0 && m_x_dir < 0 && pos_before <= BAY_SLOP && !m_pen_down)
	{
		m_ratchet = (m_ratchet + 1) % TEETH_PER_REV;
		set_sensor(m_ratchet == 0);

		// per the technical manual a spring, not the motor, snaps the
		// holder back to the origin once the magnet trips the sensor, so
		// counted steps can't find it - re-zero here. Y has no sensor of
		// its own and is left similarly offset, so reference it here too
		if (m_ratchet == 0)
		{
			m_cr_stepper->set_absolute_position(0);
			m_bitmap_printer->m_xpos = 0;

			// jobs request Y on both sides of their origin, so park it
			// mid-buffer rather than clipping every negative row. Only
			// on the power-on pass: the carousel passes tooth 0 on every
			// later colour change, and a pen change doesn't feed paper
			if (!m_y_homed)
			{
				m_y_homed = true;
				m_pf_stepper->set_absolute_position(-PAPER_HEIGHT);
				m_bitmap_printer->m_ypos = PAPER_HEIGHT / 2;
			}
		}
	}

	m_x_dir = dir;
}


void alps_dpg23_device::set_sensor(bool found)
{
	m_sensor = found ? 0 : 1;
}


//-------------------------------------------------
//  draw - the pen is dragged along, not stamped,
//  so mark the paper on every step while it's down
//-------------------------------------------------

void alps_dpg23_device::draw()
{
	if (!m_pen_down)
		return;

	int const x = m_bitmap_printer->m_xpos;
	int const y = m_bitmap_printer->m_ypos;

	// X below 0 is the pen-change bay, off the paper entirely
	if (x < 0)
		return;

	m_bitmap_printer->check_new_page();

	int const pen = (m_ratchet / TEETH_PER_PEN) % 4;
	m_bitmap_printer->draw_pixel(x, y, PEN_COLOUR[pen]);
}
