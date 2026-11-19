// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    NEC uPD72070 floppy disk controller (aka Apple "NewAge")
    Emulation by R. Belmont, based on upd765.cpp by Olivier Galibert

    This is an offshoot of the ubiquitous uPD765 family which supports
    Apple GCR formats and an IWM/SWIM style drive interface.

***************************************************************************/

#include "emu.h"
#include "upd72070.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iterator>

#define LOG_COMMAND     (1U << 1)
#define LOG_REGISTER    (1U << 2)
#define LOG_IRQ         (1U << 3)
#define LOG_FIFO        (1U << 4)
#define LOG_DRIVE       (1U << 5)
#define LOG_MEDIA       (1U << 6)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(UPD72070, upd72070_device, "upd72070", "NEC uPD72070 floppy disk controller")

ALLOW_SAVE_TYPE(upd72070_device::phase);
ALLOW_SAVE_TYPE(upd72070_device::interrupt_cause);
ALLOW_SAVE_TYPE(upd72070_device::drive_state);
ALLOW_SAVE_TYPE(upd72070_device::media_state);

namespace {

// Apple-mode opcodes and permitted flag bits (NEC chapter 4)
struct command_desc
{
	u8 opcode;
	u8 mask;
	u8 size;
};

constexpr command_desc COMMANDS[] =
{
	{ 0x01, 0x9f, 5 }, // Format/Write: FM, TB
	{ 0x02, 0x9f, 9 }, // Read Track: FM, TB
	{ 0x03, 0xff, 3 }, // Specify
	{ 0x04, 0xff, 2 }, // Sense Drive Status
	{ 0x05, 0x1f, 9 }, // Write Data: MT, FM, TB
	{ 0x06, 0x1f, 9 }, // Read Data: MT, FM, TB
	{ 0x07, 0xff, 2 }, // Recalibrate
	{ 0x08, 0xff, 1 }, // Sense Interrupt Status
	{ 0x0a, 0xbf, 2 }, // Read ID: FM
	{ 0x0b, 0x7f, 2 }, // Disable/Enable DPLL
	{ 0x0d, 0x9f, 6 }, // Format Track: FM, TB
	{ 0x0f, 0xff, 3 }, // Seek (not Relative Seek)
	{ 0x12, 0xff, 2 }, // Perpendicular Mode
	{ 0x13, 0xff, 4 }, // Configure
	{ 0x1a, 0x7f, 2 }, // Set Motor Control
	{ 0x1b, 0x7f, 2 }, // Set Enable Control
	{ 0x1c, 0xbf, 2 }, // Set Drive Mode
	{ 0x1e, 0xbf, 8 }, // Raw Dump: FM
	{ 0x20, 0xff, 1 }, // Revision
	{ 0x32, 0xff, 2 }, // Select Drive Type
	{ 0x52, 0xff, 2 }, // Eject
};

constexpr u8 GCR_ENCODE[64] =
{
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

// apply some C++ magic to generate the denibbleizer table
constexpr auto GCR_DECODE = []
{
	std::array<u8, 256> table;
	table.fill(0xff);
	for (u32 i = 0; i < std::size(GCR_ENCODE); ++i)
	{
		table[GCR_ENCODE[i]] = i;
	}
	return table;
}();

constexpr u8 GCR_SYNC[] = { 0xff, 0x3f, 0xcf, 0xf3, 0xfc, 0xff };

u16 crc16_byte(u16 crc, u8 data)
{
	for (int i = 7; i >= 0; --i)
	{
		crc = (crc << 1) ^ ((BIT(crc, 15) ^ BIT(data, i)) ? 0x1021 : 0);
	}
	return crc;
}

// data bits of an MFM cell word, clock bits discarded
u8 mfm_data(u16 cells)
{
	return bitswap<8>(cells, 14, 12, 10, 8, 6, 4, 2, 0);
}

void gcr_pack(u8 a, u8 b, u8 c, u8 *group)
{
	group[0] = GCR_ENCODE[((a >> 2) & 0x30) | ((b >> 4) & 0x0c) | (c >> 6)];
	group[1] = GCR_ENCODE[a & 0x3f];
	group[2] = GCR_ENCODE[b & 0x3f];
	group[3] = GCR_ENCODE[c & 0x3f];
}

bool gcr_format(u8 format)
{
	return format == 0x02 || format == 0x12 || format == 0x22 || format == 0x24;
}

} // anonymous namespace

upd72070_device::upd72070_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD72070, tag, owner, clock)
	, m_irq_cb(*this)
	, m_drq_cb(*this)
	, m_connectors(*this, "%u", 0U)
	, m_xb_clock(0)
	, m_firmware_revision(0)
	, m_hardware_revision(0)
	, m_command{}
	, m_result{}
	, m_fifo{}
	, m_tc(false)
	, m_reset(false)
	, m_irq(false)
	, m_drq(false)
	, m_drive_timer(nullptr)
	, m_poll_timer(nullptr)
	, m_media_timer(nullptr)
	, m_write_edges{}
	, m_writing(false)
{
}

void upd72070_device::map(address_map &map)
{
	map(4, 4).rw(FUNC(upd72070_device::status_r), FUNC(upd72070_device::rate_w));
	map(5, 5).rw(FUNC(upd72070_device::data_r), FUNC(upd72070_device::data_w));
}

void upd72070_device::device_start()
{
	if (!clock() || !m_xb_clock)
	{
		fatalerror("%s: NewAge requires its XA and XB clocks\n", tag());
	}

	m_drive_timer = timer_alloc(FUNC(upd72070_device::drive_tick), this);
	m_poll_timer = timer_alloc(FUNC(upd72070_device::poll_tick), this);
	m_media_timer = timer_alloc(FUNC(upd72070_device::media_tick), this);

	save_item(NAME(m_phase));
	save_item(NAME(m_command));
	save_item(NAME(m_command_pos));
	save_item(NAME(m_command_size));
	save_item(NAME(m_result));
	save_item(NAME(m_result_pos));
	save_item(NAME(m_result_size));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_transfer_remaining));
	save_item(NAME(m_to_host));
	save_item(NAME(m_transfer_request));
	save_item(NAME(m_tc));
	save_item(NAME(m_tc_seen));
	save_item(NAME(m_reset));
	save_item(NAME(m_drr));
	save_item(NAME(m_config));
	save_item(NAME(m_pretrk));
	save_item(NAME(m_specify));
	save_item(NAME(m_drive_type));
	save_item(NAME(m_perpendicular));
	save_item(NAME(m_drive_busy));
	save_item(NAME(m_pending));
	save_item(NAME(m_pending_status));
	save_item(NAME(m_pending_pcn));
	save_item(NAME(m_sense_drive));
	save_item(NAME(m_result_irq));
	save_item(NAME(m_async_irq));
	save_item(NAME(m_irq));
	save_item(NAME(m_drq));
	save_item(NAME(m_drive_state));
	save_item(NAME(m_active_drive));
	save_item(NAME(m_pcn));
	save_item(NAME(m_steps));
	save_item(NAME(m_outward));
	save_item(NAME(m_seek_command));
	save_item(NAME(m_dpll));
	save_item(NAME(m_status_read));
	save_item(NAME(m_media_out));
	save_item(NAME(m_media_pending));
	save_item(NAME(m_sense_seek));
	save_item(NAME(m_enabled));
	save_item(NAME(m_poll_drive));
	save_item(NAME(m_deadline));
	save_item(NAME(m_drive_delay));
	save_item(NAME(m_poll_delay));
	save_item(NAME(m_timeout_delay));
	save_item(NAME(m_media_state));
	save_item(NAME(m_media_deadline));
	save_item(NAME(m_next_index));
	save_item(NAME(m_write_start));
	save_item(NAME(m_write_edges));
	save_item(NAME(m_write_count));
	save_item(NAME(m_writing));
	save_item(NAME(m_media_drive));
	save_item(NAME(m_head));
	save_item(NAME(m_id));
	save_item(NAME(m_chrn));
	save_item(NAME(m_st0));
	save_item(NAME(m_st1));
	save_item(NAME(m_st2));
	save_item(NAME(m_shift));
	save_item(NAME(m_crc));
	save_item(NAME(m_bits));
	save_item(NAME(m_sync));
	save_item(NAME(m_byte));
	save_item(NAME(m_sector_size));
	save_item(NAME(m_media_bytes));
	save_item(NAME(m_transfer_total));
	save_item(NAME(m_sector));
	save_item(NAME(m_indexes));
	save_item(NAME(m_saw_id));
	save_item(NAME(m_previous_bit));
	save_item(NAME(m_result_advance));
	save_item(NAME(m_raw_clock));
	save_item(NAME(m_raw_delay));
	save_item(NAME(m_raw_c2));
	save_item(NAME(m_gcr_checksum));
	save_item(NAME(m_gcr_group));
	save_item(NAME(m_gcr_data));
	save_item(NAME(m_gcr_count));
	save_item(NAME(m_gcr_hi));
	save_item(NAME(m_gcr_carry));
	save_item(NAME(m_gcr_pos));
	save_item(NAME(m_gcr_header_pos));
	save_item(NAME(m_pll.ctime));
	save_item(NAME(m_pll.period));
	save_item(NAME(m_pll.min_period));
	save_item(NAME(m_pll.max_period));
	save_item(NAME(m_pll.period_adjust_base));
	save_item(NAME(m_pll.phase_adjust));
	save_item(NAME(m_pll.freq_hist));
}

void upd72070_device::device_reset()
{
	soft_reset();
}

void upd72070_device::soft_reset()
{
	flush_write();
	m_media_state = media_state::IDLE;
	m_media_timer->adjust(attotime::never);
	m_writing = false;
	m_write_count = 0;
	m_media_deadline = m_next_index = m_write_start = attotime::never;
	m_media_drive = m_head = m_st0 = m_st1 = m_st2 = m_shift = m_crc = m_bits = m_sync = 0;
	m_byte = m_sector_size = m_media_bytes = m_transfer_total = m_sector = m_indexes = 0;
	m_saw_id = m_previous_bit = m_result_advance = m_raw_c2 = false;
	m_raw_clock = m_raw_delay = 0;
	gcr_data_reset();
	std::fill(std::begin(m_id), std::end(m_id), 0);
	std::fill(std::begin(m_chrn), std::end(m_chrn), 0);
	m_phase = phase::IDLE;
	m_command_pos = m_command_size = 0;
	m_result_pos = m_result_size = 0;
	m_fifo_head = m_fifo_count = 0;
	m_transfer_remaining = 0;
	m_to_host = m_transfer_request = m_tc_seen = false;
	m_drr = 2; // 250 kbit/s, default precompensation, awake
	m_pll.set_clock(cell_period());
	m_pll.reset(machine().time());
	m_config = 0x20; // FIFO disabled
	m_pretrk = 0;
	m_specify = m_drive_type = m_perpendicular = 0;
	m_drive_busy = 0;
	std::fill(std::begin(m_pending), std::end(m_pending), interrupt_cause::NONE);
	std::fill(std::begin(m_pending_status), std::end(m_pending_status), 0);
	std::fill(std::begin(m_pending_pcn), std::end(m_pending_pcn), 0);
	m_sense_drive = -1;
	m_result_irq = m_async_irq = false;
	m_drive_state = drive_state::IDLE;
	m_active_drive = m_steps = m_poll_drive = 0;
	m_outward = m_seek_command = m_sense_seek = false;
	std::fill(std::begin(m_pcn), std::end(m_pcn), 0);
	std::fill(std::begin(m_dpll), std::end(m_dpll), true);
	std::fill(std::begin(m_status_read), std::end(m_status_read), false);
	std::fill(std::begin(m_media_out), std::end(m_media_out), true);
	std::fill(std::begin(m_media_pending), std::end(m_media_pending), false);
	m_deadline = m_drive_delay = m_poll_delay = m_timeout_delay = attotime::never;
	m_drive_timer->adjust(attotime::never);
	m_poll_timer->adjust(attotime::from_msec(100));
	enable_drive(-1);
	update_outputs(true);
}

void upd72070_device::reset_w(int state)
{
	const bool asserted = bool(state);
	if (m_reset != asserted)
	{
		m_reset = asserted;
		if (asserted)
		{
			soft_reset();
		}
		else
		{
			update_outputs();
		}
	}
}

void upd72070_device::wake()
{
	if (BIT(m_drr, 6))
	{
		m_drr &= ~0x40;
		m_deadline = (m_timeout_delay == attotime::never) ? attotime::never : (machine().time() + m_timeout_delay);
		m_drive_timer->adjust(m_drive_delay);
		m_poll_timer->adjust(m_poll_delay);
		update_outputs();
	}
}

u8 upd72070_device::status_r()
{
	if (!machine().side_effects_disabled())
	{
		wake();
	}

	u8 status = m_drive_busy;
	for (u32 drive = 0; drive < 2; ++drive)
	{
		if (m_connectors[drive] && m_connectors[drive]->get_device())
		{
			status |= 4 << drive;
		}
	}

	if (m_reset || BIT(m_drr, 6))
	{
		return status;
	}

	switch (m_phase)
	{
		case phase::IDLE:
			status |= 0x80;
			break;
		case phase::COMMAND:
			status |= 0x90;
			break;
		case phase::RESULT:
			status |= 0xd0;
			break;
		case phase::DRIVE:
			status |= 0x10;
			break;
		case phase::EXECUTION:
			status |= 0x10;
			if (non_dma())
			{
				status |= 0x20 | (m_to_host ? 0x40 : 0) | (m_transfer_request ? 0x80 : 0);
			}
			break;
	}
	return status;
}

void upd72070_device::rate_w(u8 data)
{
	wake();
	if (m_reset)
	{
		return;
	}
	LOGMASKED(LOG_REGISTER, "%s: DRR <- %02x\n", machine().describe_context(), data);
	if (BIT(data, 7))
	{
		soft_reset();
	}
	m_drr = data & 0x5f; // software reset self-clears; bit 5 is reserved
	if (BIT(m_drr, 6))
	{
		standby();
	}
	update_outputs();
}

u8 upd72070_device::data_r()
{
	const bool peek = machine().side_effects_disabled();
	if (!peek)
	{
		wake();
	}
	if (m_reset)
	{
		return 0xff;
	}
	if (m_phase == phase::EXECUTION && non_dma())
	{
		return transfer_r();
	}
	if (m_phase != phase::RESULT)
	{
		return 0xff;
	}

	const u8 data = m_result[m_result_pos];
	if (!peek)
	{
		// The first result byte acknowledges a command result or pending seek.
		m_result_irq = false;
		if (!m_result_pos && m_sense_seek)
		{
			m_drive_busy &= ~(1 << m_sense_drive);
		}
		if (++m_result_pos == m_result_size)
		{
			if (m_sense_drive >= 0)
			{
				m_async_irq = interrupts_pending();
			}
			command_end();
		}
		else
		{
			update_outputs();
		}
	}
	return data;
}

void upd72070_device::data_w(u8 data)
{
	wake();
	if (m_reset)
	{
		return;
	}
	if (m_phase == phase::EXECUTION && non_dma())
	{
		transfer_w(data);
		return;
	}
	if (m_phase != phase::IDLE && m_phase != phase::COMMAND)
	{
		return;
	}

	if (m_phase == phase::IDLE)
	{
		// A new command clears the interrupt pin, not the pending drive status.
		m_async_irq = m_result_irq = false;
		m_command_size = 0;
		for (const auto &command : COMMANDS)
		{
			if ((data & command.mask) == command.opcode)
			{
				m_command_size = command.size;
				break;
			}
		}
		m_command_pos = 0;
		m_phase = phase::COMMAND;
		update_outputs();
	}
	m_command[m_command_pos++] = data;
	if (!m_command_size)
	{
		invalid_command();
	}
	else if (m_command_pos == m_command_size)
	{
		execute_command();
	}
}

void upd72070_device::execute_command()
{
	LOGMASKED(LOG_COMMAND, "command %02x (%u bytes)\n", m_command[0], m_command_size);
	switch (m_command[0])
	{
		case 0x03: // Apple drive steps and head load/unload are handshaked.
			m_specify = m_command[2] & 1;
			command_end();
			break;
		case 0x08:
			sense_interrupt();
			break;
		case 0x04:
		case 0x07:
		case 0x0f:
		case 0x0b: case 0x8b:
		case 0x1a: case 0x9a:
		case 0x1b: case 0x9b:
		case 0x1c: case 0x5c:
		case 0x52:
			execute_drive_command();
			break;
		case 0x12:
			m_perpendicular = m_command[1] & 3;
			command_end();
			break;
		case 0x13:
			// EIS/POL are Standard-mode only; precompensation isn't emulated
			m_config = m_command[2] & 0x2f;
			m_pretrk = m_command[3];
			command_end();
			break;
		case 0x20:
			m_result[0] = m_firmware_revision;
			m_result[1] = m_hardware_revision;
			// Provisionally follow the Apple result-phase IRQ rule (manual p.93).
			begin_result(2, true);
			break;
		case 0x32:
			m_drive_type = m_command[1] & 3;
			command_end();
			break;
		default: // media commands
			execute_media_command();
			break;
	}
}

void upd72070_device::command_end()
{
	m_phase = phase::IDLE;
	m_command_pos = m_command_size = 0;
	m_result_pos = m_result_size = 0;
	m_sense_drive = -1;
	m_sense_seek = false;
	m_result_irq = false;
	update_outputs();
}

void upd72070_device::begin_result(u8 count, bool interrupt)
{
	assert(count && count <= std::size(m_result));
	m_phase = phase::RESULT;
	m_result_pos = 0;
	m_result_size = count;
	m_transfer_request = false;
	m_result_irq = interrupt;
	update_outputs();
}

void upd72070_device::invalid_command()
{
	// Figure 5.1-10 shows INT low throughout (contrary to the prose on p.93).
	m_result[0] = 0x80;
	begin_result(1);
}

void upd72070_device::queue_interrupt(u32 drive, interrupt_cause cause, u8 status, u8 cylinder)
{
	assert(drive < 2 && cause != interrupt_cause::NONE);
	if (cause == interrupt_cause::MEDIA)
	{
		m_media_pending[drive] = true;
	}
	else
	{
		m_pending[drive] = cause;
		m_pending_status[drive] = status;
		m_pending_pcn[drive] = cylinder;
	}
	if (cause == interrupt_cause::SEEK)
	{
		m_drive_busy |= 1 << drive;
	}
	m_async_irq = true;
	update_outputs();
}

void upd72070_device::sense_interrupt()
{
	for (u32 drive = 0; drive < 2; ++drive)
	{
		if (m_pending[drive] != interrupt_cause::NONE)
		{
			const bool seek = m_pending[drive] == interrupt_cause::SEEK;
			// Set Enable checks no drive status, so it works with the motor off (p.96)
			m_result[0] = (m_pending[drive] == interrupt_cause::ENABLE) ? drive : interrupt_status(drive, m_pending_status[drive]);
			m_result[1] = m_pending_pcn[drive];
			m_pending[drive] = interrupt_cause::NONE;
			m_sense_drive = drive;
			m_sense_seek = seek;
			begin_result(seek ? 2 : 1);
			return;
		}
	}
	for (u32 drive = 0; drive < 2; ++drive)
	{
		if (m_media_pending[drive])
		{
			m_result[0] = interrupt_status(drive, 0xc0);
			m_media_pending[drive] = false;
			m_sense_drive = drive;
			m_sense_seek = false;
			begin_result(1);
			return;
		}
	}
	invalid_command();
}

mac_floppy_device *upd72070_device::floppy(u32 drive) const
{
	return m_connectors[drive] ? dynamic_cast<mac_floppy_device *>(m_connectors[drive]->get_device()) : nullptr;
}

bool upd72070_device::drive_sense(u32 drive, u8 address)
{
	auto *const disk = floppy(drive);
	if (!disk)
	{
		return true; // unconnected RDATA is pulled high
	}
	disk->ss_w(BIT(address, 3));
	disk->seek_phase_w(address & 7);
	return disk->wpt_r();
}

void upd72070_device::drive_command(u32 drive, u8 address)
{
	if (auto *const disk = floppy(drive))
	{
		LOGMASKED(LOG_DRIVE, "%s drive %u latch %x\n", machine().time().as_string(9), drive, address);
		disk->ss_w(BIT(address, 3));
		disk->seek_phase_w(address & 7);
		disk->seek_phase_w((address & 7) | 8);
		disk->seek_phase_w(address & 7);
	}
}

void upd72070_device::enable_drive(s8 drive)
{
	m_enabled = drive;
	for (u32 i = 0; i < 2; ++i)
	{
		if (auto *const disk = floppy(i))
		{
			disk->ds_w(drive);
		}
	}
	if (drive >= 0)
	{
		m_status_read[drive] = false;
	}
}

u8 upd72070_device::drive_status(u32 drive)
{
	if (m_enabled != drive)
	{
		return 0xff;
	}

	u8 status = 0;
	constexpr u8 ADDRESSES[] = { 0xf, 0x9, 0xe, 0xa, 0x5, 0x6 };
	for (u8 address : ADDRESSES)
	{
		status = (status << 1) | (drive_sense(drive, address) ? 1 : 0);
	}

	// Mode ID shares the Motor Off address, so a repeat sense stops the motor (pp.92,124)
	if (m_status_read[drive])
	{
		drive_command(drive, 6);
	}
	m_status_read[drive] = true;
	status = (status << 1) | (drive_sense(drive, 6) ? 1 : 0);
	return (status << 1) | (drive_sense(drive, 0xd) ? 1 : 0);
}

u8 upd72070_device::interrupt_status(u32 drive, u8 flags)
{
	// SIS samples /CSTIN and /READY and clears HD (manual pp.42,150).
	return (flags & 0xf0) | (drive_sense(drive, 0xe) ? 8 : 0) | (drive_sense(drive, 8) ? 2 : 0) | drive;
}

bool upd72070_device::interrupts_pending() const
{
	return m_pending[0] != interrupt_cause::NONE || m_pending[1] != interrupt_cause::NONE || m_media_pending[0] || m_media_pending[1];
}

void upd72070_device::standby()
{
	if (m_media_state != media_state::IDLE)
	{
		finish_media(0x10);
	}
	m_drive_delay = m_drive_timer->remaining();
	m_poll_delay = m_poll_timer->remaining();
	m_timeout_delay = (m_deadline == attotime::never) ? attotime::never : std::max(attotime::zero, m_deadline - machine().time());
	m_drive_timer->adjust(attotime::never);
	m_poll_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(upd72070_device::poll_tick)
{
	// The firmware polls one drive, waits 100 ms, then polls the other (p.136).
	if (!m_reset && m_enabled < 0 && m_phase == phase::IDLE && m_drive_state == drive_state::IDLE)
	{
		const bool out = drive_sense(m_poll_drive, 8);
		if (m_media_out[m_poll_drive] != out)
		{
			m_media_out[m_poll_drive] = out;
			queue_interrupt(m_poll_drive, interrupt_cause::MEDIA, 0xc0, 0);
		}
		m_poll_drive ^= 1;
	}
	m_poll_timer->adjust(attotime::from_msec(100));
}

void upd72070_device::execute_drive_command()
{
	const u8 opcode = m_command[0];
	const u32 drive = m_command[1] & 1;
	// Apple mode has one drive sequencer: reject a second operation, keep the first
	if (m_drive_state != drive_state::IDLE || BIT(m_drive_busy, drive))
	{
		invalid_command();
		return;
	}
	if (opcode == 0x1b || opcode == 0x9b)
	{
		enable_drive(BIT(opcode, 7) ? s8(drive) : -1);
		command_end();
		queue_interrupt(drive, interrupt_cause::ENABLE, 0, 0);
		return;
	}
	if (opcode == 0x04)
	{
		m_result[0] = drive_status(drive);
		begin_result(1);
		return;
	}

	m_active_drive = drive;
	m_seek_command = opcode == 0x07 || opcode == 0x0f;
	m_phase = m_seek_command ? phase::IDLE : phase::DRIVE;
	if (m_seek_command)
	{
		m_drive_busy |= 1 << drive;
	}
	if (m_drive_type != 3 || (m_connectors[drive] && m_connectors[drive]->get_device() && !floppy(drive)))
	{
		finish_drive_command(true);
		return;
	}
	if (opcode == 0x0b || opcode == 0x8b)
	{
		// the drive's own DPLL, which mac_floppy_device doesn't model
		m_dpll[drive] = BIT(opcode, 7);
		finish_drive_command(false);
	}
	else if (opcode == 0x52)
	{
		drive_command(drive, 7);
		drive_command(drive, 3);
		finish_drive_command(false); // /CSTIN change is a separate poll event
	}
	else if (opcode == 0x1a)
	{
		drive_command(drive, 6);
		m_drive_state = drive_state::MOTOR_OFF;
		m_drive_timer->adjust(attotime::from_usec(5));
	}
	else if (opcode == 0x9a)
	{
		drive_command(drive, 2);
		wait_ready(drive_state::READY, attotime::from_seconds(1));
	}
	else if (opcode == 0x1c || opcode == 0x5c)
	{
		drive_command(drive, BIT(opcode, 6) ? 9 : 0xd);
		wait_ready(drive_state::READY, attotime::from_msec(800));
		// The drive may take up to 20 ms to assert busy for a mode change.
		m_drive_timer->adjust(attotime::from_msec(20));
	}
	else if (opcode == 0x07)
	{
		m_pcn[drive] = 0;
		m_steps = 80;
		m_outward = true;
		drive_command(drive, 4);
		m_drive_state = drive_state::RECAL_OUT;
		m_drive_timer->adjust(attotime::from_usec(1));
	}
	else // Seek
	{
		const u8 target = m_command[2];
		m_steps = std::abs(int(target) - int(m_pcn[drive]));
		m_outward = target < m_pcn[drive];
		const u32 zones = std::abs(int(target / 16) - int(m_pcn[drive] / 16));
		const u32 timeout = (m_steps > 1) ? 800 : (!drive_sense(drive, 0xd) && zones) ? 250 : 18;
		m_deadline = machine().time() + attotime::from_msec(timeout);
		// Do not step an empty drive; retain PCN and report a bounded failure.
		if (drive_sense(drive, 8) || !m_steps)
		{
			m_drive_state = drive_state::SEEK_WAIT;
			m_drive_timer->adjust(attotime::from_usec(150));
		}
		else
		{
			drive_command(drive, m_outward ? 4 : 0);
			m_drive_state = drive_state::SEEK_STEP;
			m_drive_timer->adjust(attotime::from_usec(1));
		}
	}
}

void upd72070_device::finish_drive_command(bool error)
{
	const u8 flags = (m_seek_command ? 0x20 : 0) | (error ? 0x50 : 0);
	m_drive_state = drive_state::IDLE;
	m_drive_timer->adjust(attotime::never);
	m_deadline = attotime::never;
	// a seek has already freed the command interface for other commands
	if (m_phase == phase::DRIVE)
	{
		command_end();
	}
	queue_interrupt(m_active_drive, m_seek_command ? interrupt_cause::SEEK : interrupt_cause::CONTROL, flags, m_pcn[m_active_drive]);
}

void upd72070_device::wait_ready(drive_state next, attotime timeout)
{
	m_drive_state = next;
	m_deadline = machine().time() + timeout;
	m_drive_timer->adjust(attotime::from_usec(150));
}

TIMER_CALLBACK_MEMBER(upd72070_device::drive_tick)
{
	const u32 drive = m_active_drive;
	LOGMASKED(LOG_DRIVE, "%s drive state %u steps %u\n", machine().time().as_string(9), u32(m_drive_state), m_steps);
	switch (m_drive_state)
	{
		case drive_state::IDLE:
			return;
		case drive_state::MOTOR_OFF:
			finish_drive_command(!floppy(drive));
			return;
		case drive_state::SEEK_STEP:
			if (!drive_sense(drive, 1))
			{
				if (machine().time() >= m_deadline)
				{
					finish_drive_command(true);
				}
				else
				{
					m_drive_timer->adjust(attotime::from_usec(72));
				}
				return;
			}
			drive_command(drive, 1);
			m_pcn[drive] += m_outward ? -1 : 1;
			if (--m_steps)
			{
				m_drive_timer->adjust(attotime::from_usec(72));
			}
			else
			{
				m_drive_state = drive_state::SEEK_WAIT;
				m_drive_timer->adjust(attotime::from_usec(150));
			}
			return;
		case drive_state::RECAL_OUT:
			drive_command(drive, 1);
			if (--m_steps)
			{
				m_drive_timer->adjust(attotime::from_usec(72));
			}
			else
			{
				wait_ready(drive_state::RECAL_OUT_WAIT, attotime::from_msec(800));
			}
			return;
		case drive_state::RECAL_IN_STEP:
			drive_command(drive, 1);
			wait_ready(drive_state::RECAL_IN_WAIT, attotime::from_msec(18));
			return;
		case drive_state::RECAL_LAST_STEP:
			drive_command(drive, 1);
			wait_ready(drive_state::RECAL_LAST_WAIT, attotime::from_msec(18));
			return;
		default:
			break;
	}

	if (drive_sense(drive, 0xe) || drive_sense(drive, 8))
	{
		if (machine().time() >= m_deadline)
		{
			finish_drive_command(true);
		}
		else
		{
			m_drive_timer->adjust(std::min(attotime::from_usec(150), m_deadline - machine().time()));
		}
		return;
	}
	switch (m_drive_state)
	{
		case drive_state::RECAL_OUT_WAIT:
			if (drive_sense(drive, 0xa))
			{
				finish_drive_command(true);
			}
			else
			{
				m_steps = 80;
				drive_command(drive, 0);
				m_drive_state = drive_state::RECAL_IN_STEP;
				m_drive_timer->adjust(attotime::from_usec(1));
			}
			break;
		case drive_state::RECAL_IN_WAIT:
			if (drive_sense(drive, 0xa))
			{
				drive_command(drive, 4);
				m_drive_state = drive_state::RECAL_LAST_STEP;
				m_drive_timer->adjust(attotime::from_usec(1));
			}
			else if (--m_steps)
			{
				drive_command(drive, 1);
				wait_ready(drive_state::RECAL_IN_WAIT, attotime::from_msec(18));
			}
			else
			{
				finish_drive_command(true);
			}
			break;
		case drive_state::RECAL_LAST_WAIT:
			finish_drive_command(drive_sense(drive, 0xa));
			break;
		default:
			finish_drive_command(false);
			break;
	}
}

void upd72070_device::update_outputs(bool force)
{
	const bool active = !m_reset && !BIT(m_drr, 6);
	const bool irq = active && (m_result_irq || m_async_irq || (non_dma() && m_transfer_request));
	if (force || m_irq != irq)
	{
		m_irq = irq;
		LOGMASKED(LOG_IRQ, "IRQ %d\n", m_irq);
		m_irq_cb(m_irq);
	}
	// the IRQ callback may have changed our state, so recompute
	const bool drq = !m_reset && !BIT(m_drr, 6) && !non_dma() && m_transfer_request;
	if (force || m_drq != drq)
	{
		m_drq = drq;
		LOGMASKED(LOG_IRQ, "DRQ %d\n", m_drq);
		m_drq_cb(m_drq);
	}
}

void upd72070_device::start_transfer(u32 size, bool to_host)
{
	m_phase = phase::EXECUTION;
	m_fifo_head = m_fifo_count = 0;
	m_transfer_remaining = m_transfer_total = size;
	m_to_host = to_host;
	m_tc_seen = m_tc;
	m_transfer_request = false;
	update_transfer_request();
}

bool upd72070_device::fifo_push(u8 data)
{
	if (m_fifo_count == fifo_capacity())
	{
		return false;
	}
	m_fifo[(m_fifo_head + m_fifo_count) & 15] = data;
	++m_fifo_count;
	update_transfer_request();
	return true;
}

bool upd72070_device::fifo_pop(u8 &data)
{
	if (!m_fifo_count)
	{
		return false;
	}
	data = m_fifo[m_fifo_head];
	m_fifo_head = (m_fifo_head + 1) & 15;
	--m_fifo_count;
	update_transfer_request();
	return true;
}

void upd72070_device::update_transfer_request()
{
	if (m_phase != phase::EXECUTION || m_tc_seen || !m_transfer_remaining || (m_st1 & 0x10))
	{
		m_transfer_request = false;
	}
	else if (m_to_host)
	{
		if (!m_fifo_count)
		{
			m_transfer_request = false;
		}
		else if (m_fifo_count >= std::max(1, fifo_capacity() - ((m_config & 15) + 1)) ||
				m_fifo_count >= m_transfer_remaining || m_media_state == media_state::DRAIN ||
				(m_sector_size && !(m_media_bytes % m_sector_size)))
		{
			m_transfer_request = true;
		}
	}
	else
	{
		if (m_fifo_count == fifo_capacity())
		{
			m_transfer_request = false;
		}
		else if (m_fifo_count <= (m_config & 15) + 1)
		{
			m_transfer_request = true;
		}
	}
	// A threshold starts a burst; service continues to empty/full, or TC.
	update_outputs();
}

u8 upd72070_device::transfer_r()
{
	if (!m_to_host || !m_transfer_request || !m_fifo_count)
	{
		return 0xff;
	}
	u8 data = m_fifo[m_fifo_head];
	if (!machine().side_effects_disabled())
	{
		--m_transfer_remaining;
		fifo_pop(data);
		LOGMASKED(LOG_FIFO, "host <- %02x (%u remain)\n", data, m_transfer_remaining);
		if (!m_fifo_count && m_media_state == media_state::DRAIN)
		{
			// let a TC on this last acknowledgement land before the result phase
			m_media_timer->adjust(attotime::zero);
		}
	}
	return data;
}

void upd72070_device::transfer_w(u8 data)
{
	if (m_to_host || !m_transfer_request || m_fifo_count == fifo_capacity())
	{
		return;
	}
	--m_transfer_remaining;
	fifo_push(data);
	LOGMASKED(LOG_FIFO, "host -> %02x (%u remain)\n", data, m_transfer_remaining);
}

u8 upd72070_device::dma_r()
{
	return (m_phase == phase::EXECUTION && !non_dma() && !m_reset && !BIT(m_drr, 6)) ? transfer_r() : 0xff;
}

void upd72070_device::dma_w(u8 data)
{
	if (m_phase == phase::EXECUTION && !non_dma() && !m_reset && !BIT(m_drr, 6))
	{
		transfer_w(data);
	}
}

void upd72070_device::tc_w(int state)
{
	const bool rising = state && !m_tc;
	m_tc = bool(state);
	if (rising && m_phase == phase::EXECUTION)
	{
		m_tc_seen = true;
		// writes zero-fill the rest of the final sector; reads still check its CRC
		update_transfer_request();
		if (raw_dump())
		{
			finish_media();
		}
		else if (m_media_state == media_state::DRAIN)
		{
			media_result();
		}
		else if (m_media_bytes && (m_media_state == media_state::SEARCH_ID || m_media_state == media_state::NEXT_GAP) && m_media_bytes >= m_transfer_total - m_transfer_remaining)
		{
			finish_media();
		}
	}
}

// XA supplies MFM rates; XB/32 is the independent 489.6 kbit/s GCR clock.
attotime upd72070_device::cell_period() const
{
	if (gcr())
	{
		return attotime::from_ticks(32, m_xb_clock);
	}
	static constexpr u32 DIVISORS[] = { 24, 40, 48, 12 };
	return clocks_to_attotime(DIVISORS[m_drr & 3]);
}

void upd72070_device::execute_media_command()
{
	if (m_drive_type != 3 || m_drive_state != drive_state::IDLE || m_drive_busy)
	{
		invalid_command();
		return;
	}
	m_media_drive = m_command[1] & 1;
	m_head = BIT(m_command[1], 2);
	m_st0 = m_st1 = m_st2 = 0;
	m_media_bytes = m_byte = m_write_count = 0;
	m_writing = m_result_advance = false;
	m_transfer_request = false;
	m_fifo_count = m_fifo_head = 0;
	m_tc_seen = m_tc;
	const u32 opcode = m_command[0] & 0x1f;
	const bool format = opcode == 0x0d || opcode == 1;
	m_to_host = opcode == 6 || read_track() || raw_dump();
	m_sector = read_track() ? m_command[6] : 0;
	m_raw_clock = m_raw_delay = 0;
	m_raw_c2 = raw_dump() && !(m_command[1] & 0x18);
	if (read_id())
	{
		m_sector_size = 0;
	}
	else if (gcr())
	{
		m_sector_size = BIT(m_command[0], 5) ? 524 : 512;
	}
	else
	{
		m_sector_size = 128U << std::min<u32>(m_command[format ? 2 : 5], 7);
	}
	gcr_data_reset();
	for (u32 i = 0; i < 4; ++i)
	{
		m_chrn[i] = (format || read_id()) ? 0 : m_command[i + 2];
	}
	if (format)
	{
		m_chrn[3] = m_command[2];
	}
	m_phase = phase::EXECUTION;
	m_media_state = media_state::SEARCH_ID;
	if (m_enabled != m_media_drive)
	{
		m_st0 = 0x0a; // unselected drive sense lines are pulled high
	}
	if (m_st0 || !floppy(m_media_drive) || drive_sense(m_media_drive, 8) || drive_sense(m_media_drive, 0xe))
	{
		finish_media();
		return;
	}
	if ((opcode == 5 || format) && !drive_sense(m_media_drive, 9))
	{
		finish_media(0x02);
		return;
	}
	const u8 size = m_command[format ? 2 : 5];
	if ((!read_id() && (gcr() ? !gcr_format(size) : (size < 1 || size > 6))) || (format && !m_command[3]) || (read_track() && !m_sector))
	{
		finish_media(0x04);
		return;
	}
	if (raw_dump() && ((m_command[1] & 0x18) == 0x18 || (gcr() && !(m_command[1] & 0x18))))
	{
		m_media_state = media_state::IDLE;
		invalid_command();
		return;
	}
	floppy(m_media_drive)->ss_w(m_head);
	m_pll.set_clock(cell_period());
	m_pll.reset(machine().time());
	m_bits = m_shift = m_sync = 0;
	m_previous_bit = false;
	search_id();
	if (format)
	{
		m_sector = 0;
		m_media_state = media_state::FORMAT_WAIT;
		start_transfer((4U + ((opcode == 1) ? m_sector_size : (gcr() && BIT(m_command[0], 5)) ? 12 : 0)) * m_command[3], false);
	}
	else if (raw_dump())
	{
		m_sector_size = 0;
		// the count is in disk bytes, and each disk byte is two host transfers
		start_transfer(2U * ((m_command[6] << 8) | m_command[7]), true);
		if (!m_transfer_total)
		{
			finish_media(0x80);
			return;
		}
		if (!(m_command[1] & 0x18))
		{
			m_media_state = media_state::RAW_INDEX;
			m_media_deadline = machine().time() + attotime::from_msec(200);
		}
	}
	else if (read_track())
	{
		start_transfer(m_sector * m_sector_size, true);
	}
	else if (!read_id())
	{
		u32 sectors = (m_chrn[2] <= m_command[6]) ? (m_command[6] - m_chrn[2] + 1) : 1;
		if (BIT(m_command[0], 7) && !m_head)
		{
			sectors += m_command[6] + (gcr() ? 1 : 0);
		}
		start_transfer(sectors * m_sector_size, opcode == 6);
	}
	else
	{
		m_transfer_remaining = m_transfer_total = 0;
	}
	m_media_timer->adjust(m_pll.period * ((gcr() && format) ? 8 : 1));
}

void upd72070_device::search_id()
{
	m_media_state = media_state::SEARCH_ID;
	m_media_deadline = machine().time() + attotime::from_msec(400);
	m_next_index = floppy(m_media_drive)->time_next_index();
	m_indexes = 0;
	m_saw_id = false;
	m_bits = m_sync = m_shift = 0;
}

void upd72070_device::read_bit(int bit)
{
	if (m_raw_delay && !--m_raw_delay)
	{
		raw_byte(m_raw_clock);
		if (m_media_state != media_state::RAW_DATA)
		{
			return;
		}
	}
	if (m_media_state == media_state::RAW_DATA)
	{
		// once aligned, dump every cell, including bad clocks and GCR sync padding
		m_shift = (m_shift << 1) | bit;
		if (++m_bits == (gcr() ? 8 : 16))
		{
			raw_word(m_shift);
		}
		return;
	}
	if (gcr())
	{
		// Self-synchronizing eight-bit words: leading zero cells are skipped.
		m_shift = ((m_shift << 1) | bit) & 0xff;
		if (BIT(m_shift, 7))
		{
			const u8 data = m_shift;
			m_shift = 0;
			read_gcr(data);
		}
		return;
	}
	m_shift = (m_shift << 1) | bit;
	++m_bits;
	if (m_media_state == media_state::RAW_MARK)
	{
		// sync has no byte boundary, so slide until the mark; retry a wrong mark with C2 (p.61)
		if (m_shift == 0xaaaa)
		{
			m_sync = 1;
			m_byte = 0;
		}
		else if (m_sync)
		{
			if (m_shift == (m_raw_c2 ? 0x5224 : 0x4489))
			{
				raw_word(m_shift);
			}
			else if (m_shift != 0x5555 && ++m_byte >= 32)
			{
				raw_timeout();
			}
		}
		return;
	}
	if (m_media_state == media_state::SEARCH_ID || m_media_state == media_state::SEARCH_DATA)
	{
		++m_byte;
		if (!m_sync)
		{
			if (m_shift == 0x4489)
			{
				m_sync = 1;
				m_bits = 0;
			}
		}
		else if (m_bits == 16)
		{
			m_bits = 0;
			if (m_shift == 0x4489)
			{
				m_sync = std::min<u32>(m_sync + 1, 3);
			}
			else
			{
				const u8 data = mfm_data(m_shift);
				if (m_sync == 3 && ((m_media_state == media_state::SEARCH_ID && data == 0xfe) ||
						(m_media_state == media_state::SEARCH_DATA && (data == 0xfb || data == 0xf8))))
				{
					m_media_state = (m_media_state == media_state::SEARCH_ID) ? media_state::ID : media_state::DATA;
					m_saw_id = true;
					m_byte = 0;
					m_crc = crc16_byte(0xcdb4, data); // 0xcdb4 is the CRC of the three missing-clock A1 bytes
					if (data == 0xf8 && !raw_dump())
					{
						m_st2 |= 0x40;
					}
					if (raw_dump() && m_media_state == media_state::DATA)
					{
						m_media_state = media_state::RAW_SKIP;
					}
				}
				m_sync = 0;
			}
		}
		if (m_media_state == media_state::SEARCH_DATA && !raw_dump() && m_byte >= 62 * 16)
		{
			finish_media(0x01, 0x01);
		}
	}
	else if (m_bits == 16)
	{
		m_bits = 0;
		if (m_media_state == media_state::RAW_SKIP)
		{
			if (++m_byte == (128U << m_command[5]) + 2)
			{
				raw_mark();
			}
			return;
		}
		read_byte(mfm_data(m_shift));
	}
}

void upd72070_device::read_byte(u8 data)
{
	m_crc = crc16_byte(m_crc, data);
	if (m_media_state == media_state::ID)
	{
		m_id[m_byte++] = data;
		if (m_byte != 6)
		{
			return;
		}
		LOGMASKED(LOG_MEDIA, "ID CHRN=%02x %02x %02x %02x CRC=%04x\n", m_id[0], m_id[1], m_id[2], m_id[3], m_crc);
		if (read_id())
		{
			std::copy_n(m_id, 4, m_chrn);
			finish_media(m_crc ? 0x21 : 0);
		}
		else if (read_track() && m_sector < m_command[6])
		{
			if (!std::equal(m_id, m_id + 4, m_chrn))
			{
				m_st1 |= 0x04;
			}
			std::copy_n(m_id, 4, m_chrn);
			if (m_crc)
			{
				m_st1 |= 0x20;
			}
			m_bits = m_sync = 0;
			m_media_state = media_state::READ_GAP;
			m_byte = (mfm_gap2() + 2) * 16;
		}
		else if (m_id[1] == m_chrn[1] && m_id[2] == m_chrn[2] && m_id[3] == m_chrn[3])
		{
			if (m_id[0] != m_chrn[0])
			{
				finish_media(0x04, (m_id[0] == 0xff) ? 0x02 : 0x10);
			}
			else if (m_crc && !read_track() && !raw_dump())
			{
				finish_media(0x21);
			}
			else
			{
				m_byte = m_bits = m_sync = 0;
				m_media_state = m_to_host ? (raw_dump() ? media_state::SEARCH_DATA : media_state::READ_GAP) : media_state::WRITE_GAP;
				if (m_media_state == media_state::READ_GAP)
				{
					m_byte = (mfm_gap2() + 2) * 16;
				}
				if (m_crc && read_track())
				{
					m_st1 |= 0x20;
				}
				if (raw_dump() && (m_command[1] & 0x18) == 8)
				{
					raw_mark();
				}
			}
		}
		else
		{
			m_media_state = media_state::SEARCH_ID;
			m_bits = m_sync = 0;
		}
	}
	else // DATA, including both CRC bytes even after TC
	{
		if (m_byte < m_sector_size)
		{
			++m_media_bytes;
			if (!m_tc_seen && !(m_st1 & 0x10) && !fifo_push(data))
			{
				m_st1 |= 0x10;
				m_transfer_request = false;
				update_outputs();
			}
		}
		if (++m_byte == m_sector_size + 2)
		{
			if (m_crc && !read_track())
			{
				finish_media(0x20, 0x20);
			}
			else
			{
				if (m_crc)
				{
					m_st1 |= 0x20;
					m_st2 |= 0x20;
				}
				sector_done();
			}
		}
	}
}

void upd72070_device::raw_mark()
{
	m_media_state = media_state::RAW_MARK;
	m_media_deadline = machine().time() + attotime::from_msec(400);
	m_indexes = m_bits = m_sync = m_shift = 0;
}

void upd72070_device::raw_word(u16 word)
{
	m_media_state = media_state::RAW_DATA;
	m_bits = m_shift = 0;
	u8 data = word;
	m_raw_clock = data;
	if (!gcr())
	{
		data = mfm_data(word);
		// clocks are LSB first (NEC pp.66-67, Apple's Clock0/Clock1), unlike the examples on pp.59/65
		m_raw_clock = bitswap<8>(word, 1, 3, 5, 7, 9, 11, 13, 15);
	}
	m_raw_delay = gcr() ? 4 : 8;
	raw_byte(data);
}

void upd72070_device::raw_byte(u8 data)
{
	++m_media_bytes;
	if (m_tc_seen)
	{
		finish_media();
	}
	else if (!fifo_push(data))
	{
		finish_media(0x10);
	}
	else if (m_media_bytes == m_transfer_total)
	{
		m_result_advance = true; // accept TC with the last host acknowledgement
		finish_media(0x80);
	}
}

void upd72070_device::raw_timeout()
{
	if (!gcr() && !m_raw_c2 && m_media_state == media_state::RAW_MARK)
	{
		m_raw_c2 = true;
		search_id();
		m_media_timer->adjust(cell_period());
	}
	else if (m_media_state == media_state::RAW_MARK || m_media_state == media_state::SEARCH_DATA)
	{
		finish_media(1, ((m_command[1] & 0x18) == 0x10 && m_media_state == media_state::RAW_MARK) ? 0x41 : 1);
	}
	else
	{
		finish_media(m_saw_id ? 4 : 1);
	}
}

void upd72070_device::gcr_data_reset()
{
	std::fill(std::begin(m_gcr_checksum), std::end(m_gcr_checksum), 0);
	std::fill(std::begin(m_gcr_group), std::end(m_gcr_group), 0);
	std::fill(std::begin(m_gcr_data), std::end(m_gcr_data), 0);
	m_gcr_count = m_gcr_hi = m_gcr_carry = m_gcr_pos = m_gcr_header_pos = 0;
}

void upd72070_device::read_gcr(u8 data)
{
	if (m_media_state == media_state::RAW_MARK)
	{
		if (data == 0xd5)
		{
			raw_word(data);
		}
		return;
	}
	if (m_media_state == media_state::SEARCH_ID || m_media_state == media_state::SEARCH_DATA)
	{
		if (m_sync == 2 && (data == 0x96 || data == 0xad))
		{
			if (data == 0x96)
			{
				if (m_media_state == media_state::SEARCH_DATA)
				{
					finish_media(0x01, 0x01);
					return;
				}
				m_media_state = media_state::ID;
				m_saw_id = true;
			}
			else if (m_media_state == media_state::SEARCH_DATA)
			{
				m_media_state = media_state::DATA;
				gcr_data_reset();
				if (raw_dump())
				{
					raw_mark();
				}
			}
			m_byte = m_sync = 0;
		}
		else
		{
			m_sync = (data == 0xd5) ? 1 : ((data == 0xaa) && (m_sync == 1)) ? 2 : 0;
		}
		return;
	}
	if (m_media_state == media_state::ID)
	{
		m_id[m_byte] = (m_byte < 5) ? GCR_DECODE[data] : data;
		if (++m_byte != 7)
		{
			return;
		}
		const bool valid = std::all_of(std::begin(m_id), std::begin(m_id) + 5, [] (u8 code) { return code < 64; }) &&
			((m_id[0] ^ m_id[1] ^ m_id[2] ^ m_id[3]) == m_id[4]) && m_id[5] == 0xde && m_id[6] == 0xaa;
		const u8 cylinder = m_id[0] | ((m_id[2] & 3) << 6);
		const u8 head = BIT(m_id[2], 5);
		LOGMASKED(LOG_MEDIA, "GCR ID CHRN=%02x %02x %02x %02x valid=%d\n", cylinder, head, m_id[1], m_id[3], valid);
		if (read_track() && m_sector < m_command[6])
		{
			if (cylinder != m_chrn[0] || head != m_chrn[1] || m_id[1] != m_chrn[2] || m_id[3] != m_chrn[3])
			{
				m_st1 |= 0x04;
			}
			m_chrn[0] = cylinder;
			m_chrn[1] = head;
			m_chrn[2] = m_id[1];
			m_chrn[3] = m_id[3];
		}
		else if (read_id() && valid)
		{
			m_chrn[0] = cylinder;
			m_chrn[1] = head;
			m_chrn[2] = m_id[1];
			m_chrn[3] = m_id[3];
		}
		else if (read_id() || head != m_chrn[1] || m_id[1] != m_chrn[2] || m_id[3] != m_chrn[3])
		{
			// Read ID skips damaged headers until its 400 ms search expires
			m_media_state = media_state::SEARCH_ID;
			m_sync = 0;
			return;
		}
		else if (cylinder != m_chrn[0])
		{
			finish_media(0x04, (cylinder == 0xff) ? 0x02 : 0x10);
			return;
		}
		if (!valid && !read_track())
		{
			finish_media(raw_dump() ? 0x20 : 0x21);
		}
		else
		{
			if (!valid)
			{
				m_st1 |= 0x20;
			}
			// GCR Read ID also checks the following data mark and sector (p.146)
			m_media_state = (read_id() || m_to_host) ? media_state::SEARCH_DATA : media_state::WRITE_GAP;
			m_media_deadline = machine().time() + attotime::from_msec(400);
			m_byte = m_sync = 0;
			if (raw_dump() && (m_command[1] & 0x18) == 8)
			{
				raw_mark();
			}
		}
		return;
	}

	// DATA: sector byte, 699 encoded tag/data bytes, four checksum bytes, DE AA.
	u8 code = GCR_DECODE[data];
	if (!m_byte)
	{
		if (code != m_chrn[2])
		{
			finish_media(0x01, 0x01);
		}
		else if (read_id())
		{
			finish_media();
		}
	}
	else if (m_byte <= 699)
	{
		if (code == 0xff)
		{
			if (!read_track())
			{
				finish_media(0x20, 0x20);
				return;
			}
			m_st1 |= 0x20;
			m_st2 |= 0x20;
			code = 0;
		}
		const u32 part = (m_byte - 1) & 3;
		if (!part)
		{
			m_gcr_hi = code;
		}
		else
		{
			u8 value = code | ((m_gcr_hi << (2 * part)) & 0xc0);
			if (part == 1)
			{
				m_gcr_checksum[2] = (m_gcr_checksum[2] << 1) | (m_gcr_checksum[2] >> 7);
				value ^= m_gcr_checksum[2];
				const u16 sum = m_gcr_checksum[0] + value + (m_gcr_checksum[2] & 1);
				m_gcr_checksum[0] = sum;
				m_gcr_carry = sum >> 8;
			}
			else
			{
				value ^= m_gcr_checksum[part - 2];
				const u16 sum = m_gcr_checksum[part - 1] + value + m_gcr_carry;
				m_gcr_checksum[part - 1] = sum;
				m_gcr_carry = sum >> 8;
			}
			if (m_gcr_pos++ >= 12 || BIT(m_command[0], 5))
			{
				++m_media_bytes;
				if (!m_tc_seen && !(m_st1 & 0x10) && !fifo_push(value))
				{
					m_st1 |= 0x10;
					update_transfer_request();
				}
			}
		}
	}
	else if (m_byte <= 703)
	{
		m_gcr_group[m_byte - 700] = code;
		if (m_byte == 703)
		{
			for (u32 i = 0; i < 3; ++i)
			{
				if (m_gcr_group[i + 1] == 0xff || m_gcr_group[0] == 0xff ||
					(m_gcr_group[i + 1] | ((m_gcr_group[0] << (2 + 2 * i)) & 0xc0)) != m_gcr_checksum[i])
				{
					m_st1 |= 0x20;
					m_st2 |= 0x20;
				}
			}
		}
	}
	else if (data != ((m_byte == 704) ? 0xde : 0xaa))
	{
		m_st1 |= 0x20;
		m_st2 |= 0x20;
	}
	if (++m_byte == 706)
	{
		sector_done();
	}
}

u8 upd72070_device::write_host_byte()
{
	u8 data = 0;
	if (!fifo_pop(data) && !m_tc_seen)
	{
		m_st1 |= 0x10;
		update_transfer_request();
	}
	++m_media_bytes;
	return data;
}

void upd72070_device::write_gcr()
{
	const bool format = m_media_state == media_state::FORMAT_SECTOR;
	const u32 gap = 6U * m_command[4];
	const u32 start = format ? (gap + 11) : 0;
	u8 data = 0xff;

	// fetch one byte per encoded byte, so this also works with the FIFO disabled
	if (format && m_gcr_header_pos < 4)
	{
		m_chrn[m_gcr_header_pos++] = write_host_byte();
	}
	else if (m_gcr_count < 3 && m_gcr_pos < 524)
	{
		const bool tag_byte = m_gcr_pos < 12;
		m_gcr_data[m_gcr_count++] = (tag_byte && !BIT(m_command[0], 5)) ? 0 :
			(format && !tag_byte && (m_command[0] & 0x1f) == 0x0d) ? m_command[5] : write_host_byte();
		++m_gcr_pos;
	}

	if (format && m_byte < gap)
	{
		data = GCR_SYNC[m_byte % 6];
	}
	else if (format && m_byte < start)
	{
		const u32 pos = m_byte - gap;
		static constexpr u8 MARK[] = { 0xd5, 0xaa, 0x96 };
		if (pos < 3)
		{
			data = MARK[pos];
		}
		else if (pos < 8)
		{
			const u8 side = ((m_chrn[0] >> 6) & 3) | ((m_chrn[1] & 1) << 5);
			const u8 header[] = { u8(m_chrn[0] & 0x3f), m_chrn[2], side, m_chrn[3], u8(m_chrn[0] ^ m_chrn[2] ^ side ^ m_chrn[3]) };
			data = GCR_ENCODE[header[pos - 3] & 0x3f];
		}
		else if (pos < 10)
		{
			data = (pos == 8) ? 0xde : 0xaa;
		}
	}
	else
	{
		const u32 pos = m_byte - start;
		if (pos < 6)
		{
			data = GCR_SYNC[pos];
		}
		else if (pos < 9)
		{
			static constexpr u8 MARK[] = { 0xd5, 0xaa, 0xad };
			data = MARK[pos - 6];
		}
		else if (pos == 9)
		{
			data = GCR_ENCODE[m_chrn[2] & 0x3f];
		}
		else if (pos < 709)
		{
			const u32 nibble = pos - 10;
			if (!(nibble & 3))
			{
				const bool last = nibble == 696;
				u8 a = m_gcr_data[0], b = m_gcr_data[1], c = last ? 0 : m_gcr_data[2];
				m_gcr_checksum[2] = (m_gcr_checksum[2] << 1) | (m_gcr_checksum[2] >> 7);
				const u16 suma = m_gcr_checksum[0] + a + (m_gcr_checksum[2] & 1);
				m_gcr_checksum[0] = suma;
				a ^= m_gcr_checksum[2];
				const u16 sumb = m_gcr_checksum[1] + b + (suma >> 8);
				m_gcr_checksum[1] = sumb;
				b ^= m_gcr_checksum[0];
				if (!last)
				{
					m_gcr_checksum[2] += c + (sumb >> 8);
				}
				c ^= m_gcr_checksum[1];
				gcr_pack(a, b, c, m_gcr_group);
				m_gcr_count = 0;
			}
			data = m_gcr_group[nibble & 3];
		}
		else if (pos < 713)
		{
			if (pos == 709)
			{
				gcr_pack(m_gcr_checksum[0], m_gcr_checksum[1], m_gcr_checksum[2], m_gcr_group);
			}
			data = m_gcr_group[pos - 709];
		}
		else if (pos < 715)
		{
			data = (pos == 713) ? 0xde : 0xaa;
		}
	}
	m_shift = u16(data) << 8;
	m_bits = 8;
}

void upd72070_device::advance_id()
{
	if (m_chrn[2] == m_command[6])
	{
		m_chrn[2] = gcr() ? 0 : 1;
		if (BIT(m_command[0], 7))
		{
			m_chrn[1] ^= 1;
			if (!m_chrn[1])
			{
				++m_chrn[0];
			}
		}
		else
		{
			++m_chrn[0];
		}
	}
	else
	{
		++m_chrn[2];
	}
}

void upd72070_device::sector_done()
{
	flush_write();
	m_writing = false;
	if (read_track())
	{
		// Checksum/ID errors are sticky but do not stop the physical scan.
		if (m_st1 & 0x10)
		{
			finish_media();
			return;
		}
		if (!m_tc_seen || m_transfer_total - m_transfer_remaining >= (m_command[6] - m_sector + 1U) * m_sector_size)
		{
			--m_sector;
		}
		if (m_tc_seen || !m_sector)
		{
			m_result_advance = !m_tc_seen;
			finish_media(m_tc_seen ? 0 : 0x80);
			return;
		}
		++m_chrn[2];
		search_id();
		if (!gcr() && m_command[7])
		{
			m_media_state = media_state::NEXT_GAP;
			m_byte = m_command[7] * 16;
		}
		return;
	}
	if (m_st1 || m_st2)
	{
		finish_media();
		return;
	}
	if (m_tc_seen && m_media_bytes >= m_transfer_total - m_transfer_remaining)
	{
		advance_id();
		finish_media();
		return;
	}
	if (m_chrn[2] == m_command[6] && (!BIT(m_command[0], 7) || m_head))
	{
		// A delayed final FIFO drain can still deliver TC before result phase.
		m_result_advance = true;
		finish_media(0x80);
		return;
	}
	advance_id();
	if (m_head != m_chrn[1])
	{
		m_head = m_chrn[1] & 1;
		floppy(m_media_drive)->ss_w(m_head);
		m_pll.set_clock(cell_period());
		m_pll.reset(machine().time());
	}
	search_id();
	if (!gcr() && m_to_host && m_command[7])
	{
		m_media_state = media_state::NEXT_GAP;
		m_byte = m_command[7] * 16;
	}
}

void upd72070_device::encode_word(u8 data)
{
	m_shift = 0;
	for (int i = 7; i >= 0; --i)
	{
		const u16 bit = BIT(data, i);
		m_shift = (m_shift << 2) | ((!m_previous_bit && !bit) ? 2 : 0) | bit;
		m_previous_bit = bit;
	}
	m_crc = crc16_byte(m_crc, data);
}

void upd72070_device::write_word()
{
	if (gcr())
	{
		write_gcr();
		return;
	}
	u8 data = 0x4e;
	u16 raw = 0;
	if (m_media_state == media_state::FORMAT_PREFIX)
	{
		if (m_byte >= 80 && m_byte < 92)
		{
			data = 0;
		}
		else if (m_byte >= 92 && m_byte < 95)
		{
			raw = 0x5224;
		}
		else if (m_byte == 95)
		{
			data = 0xfc;
		}
	}
	else if (m_media_state == media_state::WRITE_DATA)
	{
		const u32 gap = mfm_write_gap2();
		const u32 pos = m_byte - gap;
		if (m_byte < gap)
		{
			data = 0x4e;
		}
		else if (pos < 12)
		{
			data = 0;
		}
		else if (pos < 15)
		{
			raw = 0x4489;
		}
		else if (pos == 15)
		{
			m_crc = 0xcdb4;
			data = 0xfb;
		}
		else if (pos < 16 + m_sector_size)
		{
			data = write_host_byte();
		}
		else if (pos < 18 + m_sector_size)
		{
			data = m_crc >> 8;
		}
	}
	else if (m_media_state == media_state::FORMAT_SECTOR)
	{
		const u32 sync = 22 + mfm_gap2();
		const u32 payload = sync + 16;
		if (m_byte < 12 || (m_byte >= sync && m_byte < sync + 12))
		{
			data = 0;
		}
		else if (m_byte < 15 || (m_byte >= sync + 12 && m_byte < sync + 15))
		{
			raw = 0x4489;
		}
		else if (m_byte == 15 || m_byte == payload - 1)
		{
			m_crc = 0xcdb4;
			data = (m_byte == 15) ? 0xfe : 0xfb;
		}
		else if (m_byte < 20)
		{
			data = write_host_byte();
			m_chrn[m_byte - 16] = data;
		}
		else if (m_byte < 22 || (m_byte >= payload + m_sector_size && m_byte < payload + 2 + m_sector_size))
		{
			data = m_crc >> 8;
		}
		else if (m_byte >= payload && m_byte < payload + m_sector_size)
		{
			data = ((m_command[0] & 0x1f) == 1) ? write_host_byte() : m_command[5];
		}
	}
	if (raw)
	{
		m_shift = raw;
		m_previous_bit = BIT(raw, 0);
	}
	else
	{
		encode_word(data);
	}
	m_bits = 16;
}

void upd72070_device::flush_write()
{
	if (!m_writing)
	{
		return;
	}
	// only past transitions reach the disk; pending ones stay in saved state
	auto *const disk = floppy(m_media_drive);
	if (disk)
	{
		disk->ss_w(m_head);
		disk->write_start(m_write_start);
		for (u32 i = 0; i < m_write_count; ++i)
		{
			disk->write_flux_change(m_write_edges[i]);
		}
		disk->write_end(m_pll.ctime);
	}
	m_write_start = m_pll.ctime;
	m_write_count = 0;
}

void upd72070_device::finish_media(u8 st1, u8 st2)
{
	flush_write();
	m_writing = false;
	m_st1 |= st1;
	m_st2 |= st2;
	m_raw_delay = 0;
	m_media_timer->adjust(attotime::never);
	if (m_to_host && m_fifo_count && !m_tc_seen && !(m_st1 & 0x10))
	{
		m_media_state = media_state::DRAIN;
		m_media_timer->adjust(cell_period() * (16 * fifo_capacity()));
		update_transfer_request();
	}
	else
	{
		media_result();
	}
}

void upd72070_device::media_result()
{
	if (m_result_advance && m_tc_seen)
	{
		m_st1 &= ~0x80;
		if (!raw_dump() && !read_track())
		{
			advance_id();
		}
	}
	m_result_advance = false;
	m_media_state = media_state::IDLE;
	m_media_timer->adjust(attotime::never);
	m_transfer_request = false;
	m_fifo_count = 0;
	const u8 sensed = interrupt_status(m_media_drive, 0) | (m_head << 2);
	m_result[0] = sensed | m_st0 | ((m_st0 || m_st1 || (m_st2 & ~0x40) || (sensed & 0x0a)) ? 0x40 : 0);
	m_result[1] = m_st1;
	m_result[2] = m_st2;
	std::copy_n(m_chrn, 4, &m_result[3]);
	if (read_track())
	{
		m_result[5] = m_sector;
	}
	LOGMASKED(LOG_MEDIA, "media result ST=%02x %02x %02x CHRN=%02x %02x %02x %02x\n", m_result[0], m_st1, m_st2, m_chrn[0], m_chrn[1], m_chrn[2], m_chrn[3]);
	begin_result(7, true);
}

TIMER_CALLBACK_MEMBER(upd72070_device::media_tick)
{
	if (m_media_state == media_state::DRAIN)
	{
		if (m_fifo_count)
		{
			m_st1 |= 0x10;
		}
		media_result();
		return;
	}
	auto *const disk = floppy(m_media_drive);
	const attotime now = machine().time();
	const attotime index_time = m_next_index;
	const bool index = !gcr() && now > index_time;
	if (index)
	{
		++m_indexes;
		m_next_index = disk->time_next_index();
	}
	if (!disk->exists() || disk->mon_r())
	{
		finish_media();
		return;
	}
	if (m_media_state == media_state::RAW_INDEX)
	{
		m_pll.ctime = now;
		if (index)
		{
			raw_mark();
		}
		else if (now >= m_media_deadline)
		{
			m_st0 |= 0x10;
			finish_media();
		}
	}
	else if (m_media_state == media_state::FORMAT_WAIT)
	{
		if (index || gcr())
		{
			m_media_state = gcr() ? media_state::FORMAT_SECTOR : media_state::FORMAT_PREFIX;
			m_byte = 0;
			m_writing = true;
			m_write_start = gcr() ? now : index_time;
			m_pll.reset(m_write_start);
			write_word();
		}
		else if (now >= m_media_deadline)
		{
			finish_media(0x01);
		}
	}
	else if (m_writing)
	{
		if (index && ((m_command[0] & 0x1f) == 0x0d || (m_command[0] & 0x1f) == 1))
		{
			finish_media((m_media_state == media_state::FORMAT_GAP) ? 0 : 0x10);
			return;
		}
		if (disk->writing_disabled())
		{
			finish_media(0x02);
			return;
		}
		if (BIT(m_shift, 15))
		{
			m_write_edges[m_write_count++] = m_pll.ctime + m_pll.period / 2;
		}
		m_pll.ctime = now;
		m_shift <<= 1;
		if (m_write_count == std::size(m_write_edges))
		{
			flush_write();
		}
		if (!--m_bits)
		{
			++m_byte;
			if (gcr() && m_media_state == media_state::FORMAT_SECTOR && m_byte == 6U * m_command[4] + 728)
			{
				// GCR doesn't stop at the index; Apple formats an oversized track first to erase it
				if (++m_sector == m_command[3] || m_st1 || (m_tc_seen && !m_fifo_count))
				{
					finish_media();
				}
				else
				{
					m_byte = 0;
					gcr_data_reset();
					write_word();
				}
			}
			else if (m_media_state == media_state::WRITE_DATA && m_byte == (gcr() ? 717U : (18 + mfm_write_gap2() + m_sector_size + m_command[7])))
			{
				sector_done();
			}
			else
			{
				if (!gcr() && m_media_state == media_state::FORMAT_PREFIX && m_byte == 146)
				{
					m_media_state = media_state::FORMAT_SECTOR;
					m_byte = 0;
				}
				else if (!gcr() && m_media_state == media_state::FORMAT_SECTOR && m_byte == 40 + mfm_gap2() + m_sector_size + m_command[4])
				{
					++m_sector;
					m_byte = 0;
					if (m_sector == m_command[3] || m_st1 || (m_tc_seen && !m_fifo_count))
					{
						m_media_state = media_state::FORMAT_GAP;
					}
				}
				write_word();
			}
		}
	}
	else if (m_media_state == media_state::WRITE_GAP || m_media_state == media_state::READ_GAP || m_media_state == media_state::NEXT_GAP)
	{
		m_pll.ctime = now;
		if (m_media_state == media_state::WRITE_GAP)
		{
			if (++m_byte == (gcr() ? 8U : ((mfm_gap2() - mfm_write_gap2()) * 16)))
			{
				m_media_state = media_state::WRITE_DATA;
				m_writing = true;
				m_byte = 0;
				gcr_data_reset();
				m_write_start = now;
				m_pll.set_clock(cell_period());
				m_pll.reset(now);
				write_word();
			}
		}
		else if (!--m_byte)
		{
			m_media_state = (m_media_state == media_state::READ_GAP) ? media_state::SEARCH_DATA : media_state::SEARCH_ID;
		}
	}
	else
	{
		if (raw_dump() && m_media_state != media_state::RAW_DATA && (now >= m_media_deadline || (!gcr() && m_indexes >= 2)))
		{
			raw_timeout();
			return;
		}
		if ((m_media_state == media_state::SEARCH_ID || m_media_state == media_state::ID || (gcr() && (m_media_state == media_state::SEARCH_DATA || m_media_state == media_state::DATA))) &&
				(now >= m_media_deadline || (!gcr() && (m_command[0] & 0x1f) != 6 && m_indexes >= 2)))
		{
			if (m_media_state == media_state::DATA)
			{
				finish_media(0x20, 0x20);
			}
			else if (m_media_state == media_state::SEARCH_DATA)
			{
				finish_media(0x01, 0x01);
			}
			else
			{
				finish_media(m_saw_id ? 0x04 : 0x01);
			}
			return;
		}
		attotime tm = m_pll.ctime;
		const int bit = m_pll.get_next_bit(tm, disk, now);
		if (bit >= 0)
		{
			read_bit(bit);
		}
	}
	if (m_media_state != media_state::IDLE && m_media_state != media_state::DRAIN)
	{
		const attotime next = m_pll.ctime + m_pll.period + (m_writing ? attotime::zero : m_pll.phase_adjust);
		m_media_timer->adjust((m_media_state == media_state::FORMAT_WAIT) ? cell_period() : std::max(attotime::zero, next - now));
	}
}
