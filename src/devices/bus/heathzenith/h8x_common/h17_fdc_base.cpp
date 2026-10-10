// license:BSD-3-Clause
// copyright-holders:Mark Garlanger, Paul Galbraith
/***************************************************************************

  Heathkit H-17 Floppy controller

    This was an option for both the Heathkit H8 and H89 computer systems.
    The bus attachments are h89/h17_fdc.cpp (H-88-1) and h8/h_8_17.cpp
    (H-8-17).

  TODO
    - writing to disk images

****************************************************************************/

#include "emu.h"
#include "h17_fdc_base.h"

#include "formats/h17disk.h"


#define LOG_REG   (1U << 1) // Register setup
#define LOG_LINES (1U << 2) // Control lines
#define LOG_DRIVE (1U << 3) // Drive select
#define LOG_FUNC  (1U << 4) // Function calls
#define LOG_SETUP (1U << 5)

//#define VERBOSE (LOG_GENERAL | LOG_REG | LOG_LINES | LOG_DRIVE | LOG_FUNC)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGDRIVE(...)      LOGMASKED(LOG_DRIVE, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)
#define LOGSETUP(...)      LOGMASKED(LOG_SETUP, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


heath_h17_fdc_base_device::heath_h17_fdc_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_s2350(*this, "s2350")
	, m_floppies(*this, "floppy%u", 0U)
	, m_tx_timer(*this, "tx_timer")
	, m_rx_timer(nullptr)
	, m_floppy(nullptr)
{
}

void heath_h17_fdc_base_device::write(offs_t offset, u8 data)
{
	LOGFUNC("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, data);

	switch (offset)
	{
		case 0: // data port
			m_s2350->transmitter_holding_reg_w(data);
			break;
		case 1: // fill character
			m_s2350->transmit_fill_reg_w(data);
			break;
		case 2: // sync port
			m_s2350->receiver_sync_reg_w(data);
			break;
		case 3: // control port
			ctrl_w(data);
			break;
	}
}

void heath_h17_fdc_base_device::set_floppy(floppy_image_device *floppy)
{
	if (m_floppy == floppy)
	{
		return;
	}

	LOGDRIVE("%s: selecting new drive\n", FUNCNAME);

	m_floppy = floppy;

	// set any latched signals
	if (m_floppy)
	{
		m_floppy->ss_w(m_side);
	}

	reset_rx_separator();
}

void heath_h17_fdc_base_device::side_select_w(int state)
{
	m_side = BIT(state, 0);

	if (m_floppy)
	{
		m_floppy->ss_w(m_side);
		reset_rx_separator();
	}
}

void heath_h17_fdc_base_device::dir_w(int state)
{
	if (m_floppy)
	{
		LOGFUNC("%s: step dir: 0x%02x\n", FUNCNAME, state);

		m_floppy->dir_w(state);
	}
}

void heath_h17_fdc_base_device::step_w(int state)
{
	if (m_floppy)
	{
		LOGFUNC("%s: step: 0x%02x\n", FUNCNAME, state);

		m_floppy->stp_w(state);
		reset_rx_separator();
	}
}

void heath_h17_fdc_base_device::set_motor(bool motor_on)
{
	if (m_motor_on == motor_on)
	{
		return;
	}

	m_motor_on = motor_on;

	for (auto &elem : m_floppies)
	{
		floppy_image_device *floppy = elem->get_device();
		if (floppy)
		{
			LOGFUNC("%s: motor: %d\n", FUNCNAME, motor_on);

			floppy->mon_w(!motor_on);
		}
	}

	reset_rx_separator();
}

// Receive data separator.  The board has no PLL, so the cell rate is fixed and
// each flux transition only moves the phase.  The first pulse in a cell is its
// clock; after that a pulse near the middle is a data 1, a later one is the
// next cell's clock, and an earlier one is dropped.

void heath_h17_fdc_base_device::schedule_rx_cell()
{
	if (!m_motor_on || !m_floppy)
	{
		m_rx_timer->adjust(attotime::never);
		return;
	}

	// Wake for whichever comes first, the next flux transition or the end of
	// the cell being assembled.
	attotime const cell_end = m_rx_cell_start + fm_bit_time();
	attotime const edge     = m_floppy->get_next_transition(m_rx_scan);
	attotime const next     = (!edge.is_never() && edge < cell_end) ? edge : cell_end;
	attotime const now      = machine().time();

	m_rx_timer->adjust(next > now ? next - now : attotime::zero);
}

void heath_h17_fdc_base_device::reset_rx_separator()
{
	m_rx_cell_start = machine().time();
	m_rx_scan       = m_rx_cell_start;
	m_rx_have_clock = false;
	m_rx_data       = false;

	schedule_rx_cell();
}

void heath_h17_fdc_base_device::rx_emit_cell()
{
	m_s2350->rx_w(m_rx_data ? 1 : 0);
	m_s2350->rcp_w();

	m_rx_have_clock = false;
	m_rx_data       = false;
}

TIMER_CALLBACK_MEMBER(heath_h17_fdc_base_device::rx_timer_cb)
{
	if (!m_motor_on || !m_floppy)
	{
		m_rx_timer->adjust(attotime::never);
		return;
	}

	attotime const now       = machine().time();
	attotime const half      = fm_cell_time();
	attotime const win_open  = (half * 3) / 4;
	attotime const win_close = (half * 5) / 4;

	while (true)
	{
		attotime const cell_end = m_rx_cell_start + fm_bit_time();
		attotime const edge     = m_floppy->get_next_transition(m_rx_scan);

		if (!edge.is_never() && edge < cell_end && edge <= now)
		{
			m_rx_scan = edge;

			attotime const in_cell = edge - m_rx_cell_start;
			if (!m_rx_have_clock)
			{
				// Before the data window: a phase half a cell out would
				// otherwise eat each clock as data and never recover.
				m_rx_cell_start = edge;
				m_rx_have_clock = true;
			}
			else if (in_cell >= win_open && in_cell < win_close)
			{
				m_rx_data = true;
			}
			else if (in_cell >= win_close)
			{
				// The next cell's clock, ending this cell early.
				rx_emit_cell();

				m_rx_cell_start = edge;
				m_rx_have_clock = true;
			}
			// Anything else is too early to be data, and must not move the
			// phase.
		}
		else if (cell_end <= now)
		{
			// No clock ended the cell; roll on to the next.
			rx_emit_cell();
			m_rx_cell_start = cell_end;
		}
		else
		{
			break;
		}
	}

	schedule_rx_cell();
}

void heath_h17_fdc_base_device::ctrl_w(u8 val)
{
	m_write_gate = bool(BIT(val, CTRL_WRITE_GATE));

	set_motor(bool(BIT(val, CTRL_MOTOR_ON)));

	if (BIT(val, CTRL_DRIVE_SELECT_0))
	{
		LOGFUNC("%s: set drive 0\n", FUNCNAME);

		set_floppy(m_floppies[0]->get_device());
	}
	else if (BIT(val, CTRL_DRIVE_SELECT_1))
	{
		LOGFUNC("%s: set drive 1\n", FUNCNAME);

		set_floppy(m_floppies[1]->get_device());
	}
	else if (BIT(val, CTRL_DRIVE_SELECT_2))
	{
		LOGFUNC("%s: set drive 2\n", FUNCNAME);

		set_floppy(m_floppies[2]->get_device());
	}
	else
	{
		LOGFUNC("%s: set drive none\n", FUNCNAME);

		set_floppy(nullptr);
	}

	dir_w(!BIT(val, CTRL_DIRECTION));

	step_w(!BIT(val, CTRL_STEP_COMMAND));

	set_ram_write_enable(BIT(val, CTRL_WRITE_ENABLE_RAM));
}

u8 heath_h17_fdc_base_device::read(offs_t offset)
{
	u8 val = 0;

	switch (offset)
	{
		case 0: // data port
			val = m_s2350->receiver_output_reg_r();
			break;
		case 1: // status port
			val = m_s2350->status_word_r();
			break;
		case 2: // sync port
			val = m_s2350->receiver_sync_search();
			break;
		case 3: // floppy status port
			val = floppy_status_r();
			break;
	}

	LOGREG("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, val);

	return val;
}

u8 heath_h17_fdc_base_device::floppy_status_r()
{
	u8 val = 0;

	// statuses from the floppy drive
	if (m_floppy)
	{
		// index/sector hole
		val |= m_floppy->idx_r() ? 0x01 : 0x00;

		// track 0
		val |= m_floppy->trk00_r() ? 0x00 : 0x02;

		// disk is write-protected
		val |= m_floppy->wpt_r() ? 0x04 : 0x00;
	}
	else
	{
		LOGREG("%s: no drive selected\n", FUNCNAME);
	}

	// status from USRT
	val |= m_sync_char_received ? 0x08 : 0x00;

	LOGFUNC("%s: val: 0x%02x\n", FUNCNAME, val);

	return val;
}

void heath_h17_fdc_base_device::device_start()
{
	m_rx_timer = timer_alloc(FUNC(heath_h17_fdc_base_device::rx_timer_cb), this);

	save_item(NAME(m_motor_on));
	save_item(NAME(m_write_gate));
	save_item(NAME(m_sync_char_received));
	save_item(NAME(m_rx_cell_start));
	save_item(NAME(m_rx_scan));
	save_item(NAME(m_rx_have_clock));
	save_item(NAME(m_rx_data));
	save_item(NAME(m_step_direction));
	save_item(NAME(m_side));
}

void heath_h17_fdc_base_device::device_reset()
{
	m_motor_on           = false;
	m_write_gate         = false;
	m_sync_char_received = false;

	m_tx_timer->adjust(attotime::from_hz(USRT_TX_CLOCK), 0, attotime::from_hz(USRT_TX_CLOCK));
	reset_rx_separator();
}

static void h17_floppies(device_slot_interface &device)
{
	// H-17-1
	device.option_add("ssdd", FLOPPY_525_SSDD);

	// Future plans - test and verify higher capacity drives with LLC's BIOS-80 for CP/M and an HUG's enhanced HDOS driver
	//  - FLOPPY_525_SSQD
	//  - FLOPPY_525_DD
	//  - FLOPPY_525_QD (H-17-4)
}

TIMER_DEVICE_CALLBACK_MEMBER(heath_h17_fdc_base_device::tx_timer_cb)
{
	m_s2350->tcp_w();
}

void heath_h17_fdc_base_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_H17D_FORMAT);
}

void heath_h17_fdc_base_device::device_add_mconfig(machine_config &config)
{
	S2350(config, m_s2350);
	m_s2350->sync_character_received_cb().set(FUNC(heath_h17_fdc_base_device::sync_character_received));

	for (int i = 0; i < MAX_FLOPPY_DRIVES; i++)
	{
		FLOPPY_CONNECTOR(config, m_floppies[i], h17_floppies, "ssdd", heath_h17_fdc_base_device::floppy_formats);
		m_floppies[i]->enable_sound(true);
	}

	TIMER(config, m_tx_timer).configure_generic(FUNC(heath_h17_fdc_base_device::tx_timer_cb));
}

void heath_h17_fdc_base_device::sync_character_received(int state)
{
	LOGFUNC("%s: state: %d\n", FUNCNAME, state);

	m_sync_char_received = bool(BIT(state, 0));
}
