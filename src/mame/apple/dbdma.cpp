// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Descriptor-Based DMA" channel device
    Emulation by R. Belmont

    A channel can currently be driven two ways:
    - The device pulls/pushes data itself with dma_read()/dma_write()
    - The classic DRQ model
*/

#include "emu.h"
#include "dbdma.h"

#define VERBOSE (0)
#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DBDMA_CHANNEL, dbdma_device, "appledbdma", "Apple descriptor-based DMA channel")

static constexpr u16 STATUS_RUN     = 0x8000;
static constexpr u16 STATUS_PAUSE   = 0x4000;
static constexpr u16 STATUS_FLUSH   = 0x2000;
static constexpr u16 STATUS_WAKE    = 0x1000;
[[maybe_unused]] static constexpr u16 STATUS_DEAD = 0x0800;
static constexpr u16 STATUS_ACTIVE  = 0x0400;
static constexpr u16 STATUS_BT      = 0x0100;

static constexpr u32 WAIT_MASK      = 0x0003'0000;

enum
{
	OP_OUTPUT_MORE = 0,
	OP_OUTPUT_LAST,
	OP_INPUT_MORE,
	OP_INPUT_LAST,
	OP_STORE_QUAD,
	OP_LOAD_QUAD,
	OP_NOP,
	OP_STOP
};

//-------------------------------------------------
//  dbdma_device - constructor
//-------------------------------------------------

dbdma_device::dbdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DBDMA_CHANNEL, tag, owner, clock),
	write_irq(*this),
	m_status(0),
	m_command_pointer(0),
	m_intselect(0),
	m_branchselect(0),
	m_waitselect(0),
	m_xfer_word(0),
	m_opcode(0),
	m_address(0),
	m_cmdDep(0),
	m_statusCount(0),
	m_currentXfer(0),
	m_xferLimit(0),
	m_bytesLeft(0),
	m_width(4),
	m_drq_state(0),
	m_in_pump(false),
	m_drq_status_bit(-1),
	m_hw_status_mask(0),
	m_hw_status(0),
	m_waiting(false),
	m_wake_timer(nullptr),
	m_read_dma(*this, 0),
	m_write_dma(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dbdma_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_command_pointer));
	save_item(NAME(m_intselect));
	save_item(NAME(m_branchselect));
	save_item(NAME(m_waitselect));
	save_item(NAME(m_xfer_word));
	save_item(NAME(m_opcode));
	save_item(NAME(m_address));
	save_item(NAME(m_cmdDep));
	save_item(NAME(m_statusCount));
	save_item(NAME(m_currentXfer));
	save_item(NAME(m_xferLimit));
	save_item(NAME(m_bytesLeft));
	save_item(NAME(m_drq_state));
	save_item(NAME(m_hw_status_mask));
	save_item(NAME(m_hw_status));
	save_item(NAME(m_waiting));

	m_wake_timer = timer_alloc(FUNC(dbdma_device::wake_tick), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dbdma_device::device_reset()
{
	// the hardware-driven status bits keep whatever level the device is presenting
	m_status = m_hw_status;
	m_command_pointer = 0;
	m_intselect = m_branchselect = m_waitselect = 0;
	m_in_pump = false;
	m_waiting = false;
	m_wake_timer->reset();
}

// 00 = control (write only)
// 04 = status (read only)
// 0c = command pointer (read/write)
// 10 = interrupt select
// 14 = branch select
// 18 = wait select

void dbdma_device::map(address_map &map)
{
	map(0x00, 0x03).w(FUNC(dbdma_device::control_w));
	map(0x04, 0x07).r(FUNC(dbdma_device::status_r));
	map(0x0c, 0x0f).rw(FUNC(dbdma_device::cmdpointer_r), FUNC(dbdma_device::cmdpointer_w));
	map(0x10, 0x13).rw(FUNC(dbdma_device::intselect_r), FUNC(dbdma_device::intselect_w));
	map(0x14, 0x17).rw(FUNC(dbdma_device::branchselect_r), FUNC(dbdma_device::branchselect_w));
	map(0x18, 0x1b).rw(FUNC(dbdma_device::waitselect_r), FUNC(dbdma_device::waitselect_w));
}

void dbdma_device::control_w(u32 data)
{
	// the top 16 bits of data are a mask selecting which bits of
	// the status are changed by the write.  Status bits driven by the
	// device can't be changed by software.
	const u16 mask = (data >> 16) & ~m_hw_status_mask;
	const u16 old_status = m_status;

	m_status &= (mask ^ 0xffff);
	m_status |= (data & mask);
	LOG("%s: channel status/control to %04x (raw %08x)\n", tag(), m_status, data);

	if (m_status & STATUS_RUN)
	{
		LOG("%s: channel set to RUN, also setting ACTIVE\n", tag());
		m_status |= STATUS_ACTIVE;
	}
	else
	{
		m_status &= ~STATUS_ACTIVE;
	}

	if (m_status & STATUS_PAUSE)
	{
		m_status &= ~STATUS_ACTIVE;
	}

	if (m_status & STATUS_FLUSH)
	{
		// INPUT commands must return a partial transfer status
		const u32 op = m_opcode >> 28;
		if ((m_status & STATUS_ACTIVE) && ((op == OP_INPUT_MORE) || (op == OP_INPUT_LAST)))
		{
			m_pci_memory->write_word(m_command_pointer + 12, m_bytesLeft);
			m_pci_memory->write_word(m_command_pointer + 14, m_status);
		}
		m_status &= ~STATUS_FLUSH;
	}

	if (m_status & STATUS_WAKE)
	{
		// WAKE releases a waiting channel and is self-clearing
		m_status &= ~STATUS_WAKE;
		if (m_waiting)
		{
			m_waiting = false;
			m_wake_timer->adjust(attotime::zero);
		}
	}

	// a program waiting on a software-controlled status bit may be able to continue
	check_wait();

	// on a RUN rising edge, fetch the channel program from the command pointer
	if ((m_status & STATUS_RUN) && !(old_status & STATUS_RUN))
	{
		new_command();
	}

	// service any DRQ the device raised before the channel was configured
	pump();
}

u32 dbdma_device::status_r()
{	return m_status;
}

u32 dbdma_device::cmdpointer_r()
{
	return m_command_pointer;
}

void dbdma_device::cmdpointer_w(u32 data)
{
	if (!(m_status & STATUS_ACTIVE))
	{
		LOG("%s: %08x to command pointer\n", tag(), swapendian_int32(data));
		m_command_pointer = data;
		new_command();
	}
}

u32 dbdma_device::intselect_r()
{
	return m_intselect;
}

void dbdma_device::intselect_w(u32 data)
{
	m_intselect = data;
}

u32 dbdma_device::branchselect_r()
{
	return m_branchselect;
}

void dbdma_device::branchselect_w(u32 data)
{
	m_branchselect = data;
}

u32 dbdma_device::waitselect_r()
{
	return m_waitselect;
}

void dbdma_device::waitselect_w(u32 data)
{
	m_waitselect = data;
}

u32 dbdma_device::dma_read(offs_t offset)
{
	// A device clocked by its own rate (the audio codec) keeps pulling even
	// while the channel is between DMA programs (stopped or paused).  Feed it
	// silence rather than the stale last word, which would hold a DC level and
	// click when playback resumes.  SCSI never gets here while inactive because
	// pump() only runs the callbacks while a transfer command is active.
	if (((m_status & (STATUS_ACTIVE | STATUS_PAUSE)) != STATUS_ACTIVE) || m_waiting)
	{
		return 0;
	}
	m_xfer_word = 0;
	step_program();
	return m_xfer_word;
}

void dbdma_device::dma_write(offs_t offset, u32 data)
{
	m_xfer_word = data;
	step_program();
}

bool dbdma_device::is_to_memory()
{
	switch (m_opcode >> 28)
	{
		case OP_OUTPUT_MORE:
		case OP_OUTPUT_LAST:
		case OP_LOAD_QUAD:
			return false;

		case OP_INPUT_MORE:
		case OP_INPUT_LAST:
		case OP_STORE_QUAD:
			return true;

		default:
			logerror("%s: getting DMA direction for non-transfer opcode %d\n", tag(), m_opcode >> 28);
			break;
	}

	return false;
}

void dbdma_device::drq_w(int state)
{
	m_drq_state = state;

	// optionally mirror the request line into a general-purpose status bit
	if (m_drq_status_bit >= 0)
	{
		if (state)
		{
			m_status |= (1 << m_drq_status_bit);
		}
		else
		{
			m_status &= ~(1 << m_drq_status_bit);
		}
	}

	if (state)
	{
		pump();
	}
}

void dbdma_device::status_bit_w(int bit, int state)
{	const u8 mask = 1 << bit;

	m_hw_status_mask |= mask;
	if (state)
	{
		m_hw_status |= mask;
		m_status |= mask;
	}
	else
	{
		m_hw_status &= ~mask;
		m_status &= ~mask;
	}

	check_wait();
}

// Move data between the device (via the dma_r/dma_w callbacks) and memory
// as long as DRQ is asserted and a transfer command is active.  If a command
// is not yet active but DRQ is asserted, nothing happens.
void dbdma_device::pump()
{
	// check for unwanted recursion
	if (m_in_pump)
	{
		return;
	}

	m_in_pump = true;
	while (m_drq_state && !m_waiting && ((m_status & (STATUS_ACTIVE | STATUS_PAUSE)) == STATUS_ACTIVE))
	{
		switch (m_opcode >> 28)
		{
			case OP_OUTPUT_MORE:
			case OP_OUTPUT_LAST:
				m_write_dma(0, dma_read(0));
				break;

			case OP_INPUT_MORE:
			case OP_INPUT_LAST:
				dma_write(0, m_read_dma(0));
				break;

			default:    // current command doesn't move device data
				m_in_pump = false;
				return;
		}
	}
	m_in_pump = false;
}

void dbdma_device::step_program()
{
	if (((m_status & (STATUS_ACTIVE | STATUS_PAUSE)) != STATUS_ACTIVE) || m_waiting)
	{
		return;
	}

	const u32 initial_op = m_opcode >> 28;
	const bool input = (initial_op == OP_INPUT_MORE) || (initial_op == OP_INPUT_LAST);
	if (!input && (initial_op != OP_OUTPUT_MORE) && (initial_op != OP_OUTPUT_LAST))
	{
		logerror("%s: transfer step on non-transfer opcode %d\n", tag(), initial_op);
		return;
	}

	// a descriptor can end with residual bytes, so we need to do some special handling so the transfer
	// makes it through intact.
	u32 byte_offset = 0;
	while ((byte_offset < m_width) && ((m_status & (STATUS_ACTIVE | STATUS_PAUSE)) == STATUS_ACTIVE))
	{
		const u32 op = m_opcode >> 28;
		const bool current_input = (op == OP_INPUT_MORE) || (op == OP_INPUT_LAST);
		const bool current_output = (op == OP_OUTPUT_MORE) || (op == OP_OUTPUT_LAST);
		const bool more = (op == OP_INPUT_MORE) || (op == OP_OUTPUT_MORE);
		if ((input && !current_input) || (!input && !current_output))
		{
			return;
		}

		const u32 transfer_size = std::min<u32>(m_width - byte_offset, m_bytesLeft);
		if (transfer_size == 0)
		{
			finish_command();
			process_commands();
			if (!more || m_waiting)
			{
				return;
			}
			continue;
		}

		u32 transfer_word = 0;
		if (!input)
		{
			switch (transfer_size)
			{
				case 1:
					transfer_word = m_pci_memory->read_byte(m_address);
					break;

				case 2:
					transfer_word = m_pci_memory->read_word_unaligned(m_address);
					break;

				case 3:
					transfer_word = m_pci_memory->read_byte(m_address)
						| (m_pci_memory->read_byte(m_address + 1) << 8)
						| (m_pci_memory->read_byte(m_address + 2) << 16);
					break;

				case 4:
					transfer_word = m_pci_memory->read_dword_unaligned(m_address);
					break;
			}
			m_xfer_word |= transfer_word << (byte_offset * 8);
		}
		else
		{
			transfer_word = m_xfer_word >> (byte_offset * 8);
			switch (transfer_size)
			{
				case 1:
					m_pci_memory->write_byte(m_address, transfer_word);
					break;

				case 2:
					m_pci_memory->write_word_unaligned(m_address, transfer_word);
					break;

				case 3:
					m_pci_memory->write_byte(m_address, transfer_word);
					m_pci_memory->write_byte(m_address + 1, transfer_word >> 8);
					m_pci_memory->write_byte(m_address + 2, transfer_word >> 16);
					break;

				case 4:
					m_pci_memory->write_dword_unaligned(m_address, transfer_word);
					break;
			}
		}

		m_address += transfer_size;
		m_currentXfer += transfer_size;
		m_bytesLeft -= transfer_size;
		byte_offset += transfer_size;

		if (m_bytesLeft == 0)
		{
			finish_command();
			process_commands();
			if (!more || m_waiting)
			{
				return;
			}
		}
	}
}

// Execute commands that don't require the device to move data until
// we land on a transfer command or the channel stops.
void dbdma_device::process_commands()
{
	while (!m_waiting && ((m_status & (STATUS_ACTIVE | STATUS_PAUSE)) == STATUS_ACTIVE))
	{
		switch (m_opcode >> 28)
		{
			case OP_OUTPUT_MORE:
			case OP_OUTPUT_LAST:
			case OP_INPUT_MORE:
			case OP_INPUT_LAST:
				if (m_bytesLeft != 0)
				{
					return;
				}
				finish_command();
				break;

			case OP_STORE_QUAD:
				switch (quad_size())
				{
					case 1:
						m_pci_memory->write_byte(m_address, m_cmdDep);
						break;

					case 2:
						m_pci_memory->write_word(m_address, m_cmdDep);
						break;

					case 4:
						m_pci_memory->write_dword(m_address, m_cmdDep);
						break;
				}
				m_bytesLeft = m_xferLimit;  // quads report reqCount as the residual
				finish_command();
				break;

			case OP_LOAD_QUAD:
				switch (quad_size())
				{
					case 1:
						m_xfer_word = m_pci_memory->read_byte(m_address);
						m_pci_memory->write_byte(m_cmdDep, m_xfer_word);
						break;

					case 2:
						m_xfer_word = m_pci_memory->read_word(m_address);
						m_pci_memory->write_word(m_cmdDep, m_xfer_word);
						break;

					case 4:
						m_xfer_word = m_pci_memory->read_dword(m_address);
						m_pci_memory->write_dword(m_cmdDep, m_xfer_word);
						break;
				}
				m_bytesLeft = m_xferLimit;
				finish_command();
				break;

			case OP_NOP:
				finish_command();
				break;

			case OP_STOP:
				LOG("%s: STOP, clearing ACTIVE\n", tag());
				m_status &= ~STATUS_ACTIVE;
				return;
		}
	}
}

//  Decode the transfer size of a STORE/LOAD QUAD from the low bits of
//  reqCount: 4 wins over 2 wins over 1.
int dbdma_device::quad_size()
{
	const u32 size = m_xferLimit & 7;
	if (size & 4)
	{
		return 4;
	}
	return (size & 2) ? 2 : 1;
}

bool dbdma_device::test_condition(u32 field, u32 select)
{
	const u32 mask = (select >> 16) & 0xff;
	const bool cond = (u32(m_status) & mask) == (select & mask);

	switch (field)
	{
		case 1: // condition set
			return cond;

		case 2: // condition clear
			return !cond;

		case 3: // unconditional
			return true;
	}

	return false;   // never
}

// The "w" field stalls the channel after the command's transfer (if any) until
// the wait condition is no longer met.  The condition is re-checked whenever a
// ChannelStatus bit changes.
bool dbdma_device::wait_condition()
{	return (m_opcode & WAIT_MASK) && test_condition((m_opcode >> 16) & 3, m_waitselect);
}

void dbdma_device::check_wait()
{
	if (m_waiting && !wait_condition())
	{
		m_waiting = false;
		// Continue from a timer so a device that changed the bit from inside one
		// of its own register handlers isn't re-entered by the next command.
		m_wake_timer->adjust(attotime::zero);
	}
}

TIMER_CALLBACK_MEMBER(dbdma_device::wake_tick)
{
	if (m_waiting || ((m_status & (STATUS_ACTIVE | STATUS_PAUSE)) != STATUS_ACTIVE))
	{
		return;
	}

	complete_command();
	process_commands();
	if (m_drq_state)
	{
		pump();
	}
}

void dbdma_device::finish_command()
{
	if (wait_condition())
	{
		LOG("%s: waiting on %08x (status %04x)\n", tag(), m_waitselect, m_status);
		m_waiting = true;
		return;
	}

	complete_command();
}

// Everything that happens once a command (and any wait) is over: the status
// write-back, the branch, the interrupt, and moving on to the next command.
void dbdma_device::complete_command()
{
	const u32 op = m_opcode >> 28;

	// evaluate the branch condition ("b" bits)
	const bool branch_taken = test_condition((m_opcode >> 18) & 3, m_branchselect);
	if (branch_taken)
	{
		m_status |= STATUS_BT;
	}

	// all commands except STOP write back the transfer status
	m_pci_memory->write_word(m_command_pointer + 14, m_status | STATUS_ACTIVE);
	m_status &= ~STATUS_BT;

	// INPUT_xxx, OUTPUT_xxx, and QUAD_xxx commands also write back the residual count
	if (op < OP_NOP)
	{
		m_pci_memory->write_word(m_command_pointer + 12, m_bytesLeft);
	}

	if (branch_taken)
	{
		m_command_pointer = m_cmdDep;
	}
	else
	{
		m_command_pointer += 16;
	}

	if (test_condition((m_opcode >> 20) & 3, m_intselect))
	{
		LOG("%s: raising completion interrupt\n", tag());		// Latch the completion event and hold it.  The macio IRQ controller
		// keeps the event pending until Mac OS acknowledges it, at which point
		// it also drops this channel's held level so the next completion is
		// seen as a fresh edge.
		write_irq(ASSERT_LINE);
	}

	LOG("%s: Advancing command pointer to %08x\n", tag(), m_command_pointer);
	fetch_command();
}

void dbdma_device::new_command()
{
	fetch_command();
	process_commands();
}

void dbdma_device::fetch_command()
{
	m_opcode = m_pci_memory->read_dword(m_command_pointer);
	m_address = m_pci_memory->read_dword(m_command_pointer + 4);
	m_cmdDep = m_pci_memory->read_dword(m_command_pointer + 8);
	m_statusCount = m_pci_memory->read_dword(m_command_pointer + 12);
	m_currentXfer = m_statusCount & 0xffff;
	m_xferLimit = m_opcode & 0xffff;
	m_bytesLeft = m_xferLimit;

	LOG("%s: new command %08x addr %08x %08x %08x\n", tag(), m_opcode, m_address, m_cmdDep, m_statusCount);
	LOG("%s: opcode %d\n", tag(), m_opcode >> 28);
	LOG("%s: transfer count %x\n", tag(), m_xferLimit);
}
