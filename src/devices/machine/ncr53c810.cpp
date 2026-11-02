// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    NCR 53C8xx PCI SCSI controller family
    Emulation by R. Belmont
    Portions based on ncr53c90.cpp by Olivier Galibert

    This is the base SCRIPTS processor and SCSI implementation that appears in
    the NCR53C720, 53C770, and a variety of 53C8xx devices.  Note that there
	are many small register differences in the 720 and 770 that will need
	addressing, including DMODE, DCNTL, SCNTL2, CTEST3, and MACNTL.

    TODO: target mode, synchronous transfers, low-level arbitration, 53C810A
    extensions, the DMA FIFO, PCI retry/parity/target-abort and the DMA
    watchdog. Also, timings are approximate.
 */

#include "emu.h"
#include "ncr53c810.h"

#include <algorithm>
#include <iterator>

DEFINE_DEVICE_TYPE(NCR53C810, ncr53c810_device, "ncr53c810", "NCR 53C810 SCRIPTS Processor")

ncr53c8xx_device_base::ncr53c8xx_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, nscsi_device_interface(mconfig, *this)
	, m_memory_read(*this, 0xffff'ffff)
	, m_memory_write(*this)
	, m_io_read(*this, 0xffff'ffff)
	, m_io_write(*this)
	, m_irq_handler(*this)
	, m_timer(nullptr)
	, m_interrupt_timer(nullptr)
	, m_scsi_timer(nullptr)
	, m_regs{}
	, m_move_source(0)
	, m_move_destination(0)
	, m_stacked_dstat(0)
	, m_stacked_sist0(0)
	, m_stacked_sist1(0)
	, m_scsi_state(IDLE)
	, m_scsi_irq(0)
	, m_bus_control(0)
	, m_waiting(false)
	, m_first_byte(false)
	, m_bus_master(false)
	, m_cio(false)
	, m_cm(false)
	, m_running(false)
	, m_moving(false)
	, m_carry(false)
	, m_dma_irq(false)
	, m_irq(false)
{
}

void ncr53c8xx_device_base::device_start()
{
	m_timer = timer_alloc(FUNC(ncr53c8xx_device_base::execute), this);
	m_interrupt_timer = timer_alloc(FUNC(ncr53c8xx_device_base::promote_interrupt), this);
	m_scsi_timer = timer_alloc(FUNC(ncr53c8xx_device_base::scsi_tick), this);
	save_item(NAME(m_stacked_sist0));
	save_item(NAME(m_stacked_sist1));
	save_item(NAME(m_scsi_state));
	save_item(NAME(m_scsi_irq));
	save_item(NAME(m_bus_control));
	save_item(NAME(m_waiting));
	save_item(NAME(m_first_byte));
	save_item(NAME(m_regs));
	save_item(NAME(m_move_source));
	save_item(NAME(m_move_destination));
	save_item(NAME(m_stacked_dstat));
	save_item(NAME(m_bus_master));
	save_item(NAME(m_cio));
	save_item(NAME(m_cm));
	save_item(NAME(m_running));
	save_item(NAME(m_moving));
	save_item(NAME(m_carry));
	save_item(NAME(m_dma_irq));
	save_item(NAME(m_irq));
}

void ncr53c8xx_device_base::reset_registers()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_regs[GPCNTL] = 0x03; // GPIO pins are inputs
	m_move_source = 0;
	m_move_destination = 0;
	m_stacked_dstat = m_stacked_sist0 = m_stacked_sist1 = 0;
	m_scsi_irq = 0;
	m_scsi_state = IDLE;
	m_bus_control = 0;
	m_waiting = m_first_byte = false;
	m_scsi_timer->enable(false);

	if (m_scsi_bus)
	{
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_scsi_bus->ctrl_wait(m_scsi_refid, S_ALL, S_ALL);
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
	}

	m_running = false;
	m_moving = false;
	m_carry = false;
	m_dma_irq = false;
	m_timer->enable(false);
	m_interrupt_timer->enable(false);
	update_irq();
}

void ncr53c8xx_device_base::device_reset()
{
	m_bus_master = false;
	reset_registers();
}

u8 ncr53c8xx_device_base::high_reg_r(u8 offset)
{
	return 0;
}

void ncr53c8xx_device_base::high_reg_w(u8 offset, u8 data)
{
}

bool ncr53c8xx_device_base::load_store(u32 instruction)
{
	return false;
}

u32 ncr53c8xx_device_base::scsi_width() const
{
	return 1;
}

u32 ncr53c8xx_device_base::word_r(u8 offset) const
{
	return u32(m_regs[offset]) | (u32(m_regs[offset + 1]) << 8) |
		(u32(m_regs[offset + 2]) << 16) | (u32(m_regs[offset + 3]) << 24);
}

void ncr53c8xx_device_base::word_w(u8 offset, u32 data)
{
	for (u32 i = 0; i != 4; ++i)
	{
		m_regs[offset + i] = data >> (i * 8);
	}
}

u8 ncr53c8xx_device_base::reg_r(offs_t offset)
{
	offset &= 0x7f;
	if (offset >= HIGH_REGS)
	{
		return high_reg_r(offset);
	}

	switch (offset)
	{
		case DSTAT:
		{
			const u8 result = m_regs[DSTAT] | 0x80; // DMA FIFO empty
			if (!machine().side_effects_disabled())
			{
				m_regs[DSTAT] = 0;
				m_dma_irq = false;
				update_irq();
				if (BIT(m_regs[ISTAT], 7))
				{
					m_stacked_dstat |= DSTAT_ABRT;
				}
				promote_stacked_interrupts();
			}
			return result;
		}
		case ISTAT:
			return m_regs[ISTAT] | (m_regs[DSTAT] ? 1 : 0) | (scsi_pending() ? 2 : 0) |
				(BIT(m_regs[SCNTL1], 4) ? 8 : 0);
		case SIST0:
		case SIST1:
		{
			const u8 result = m_regs[offset];
			if (!machine().side_effects_disabled())
			{
				m_regs[offset] = 0;
				m_scsi_irq &= ~(1 << (offset - SIST0));
				update_irq();
				promote_stacked_interrupts();
			}
			return result;
		}
		case SBCL:
		{
			const u32 ctrl = m_scsi_bus ? m_scsi_bus->ctrl_r() : 0;
			return (ctrl & 0x57) | ((ctrl & S_BSY) << 2) | ((ctrl & S_REQ) << 2) | ((ctrl & S_ATN) >> 4);
		}
		case SSTAT0:
			return m_regs[SSTAT0] | ((m_bus_control & S_RST) ? 2 : 0);
		case SBDL:
			return m_scsi_bus ? m_scsi_bus->data_r() : 0;
		case SSTAT2: // LDSC is not emulated
		case CTEST6: // the DMA FIFO is not emulated
			return 0;
		case STEST0: // no synchronous offset is ever outstanding
			return 0x02 | ((m_regs[SXFER] & 0x0f) ? 0 : 0x01);
		case CTEST1:
			return 0xf0; // all FIFO byte lanes empty
		case CTEST2:
		{
			// DACK/ reads inactive
			const u8 result = (BIT(m_regs[ISTAT], 5) ? 0x40 : 0) | (m_cio ? 0x20 : 0) | (m_cm ? 0x10 : 0) | 0x01;
			if (!machine().side_effects_disabled())
			{
				m_regs[ISTAT] &= ~0x20;
			}
			return result;
		}
		case CTEST3:
			return (revision() << 4) | m_regs[CTEST3];
		case MACNTL:
			return (chip_type() << 4) | m_regs[MACNTL];
		default:
			return m_regs[offset];
	}
}

void ncr53c8xx_device_base::reg_w(offs_t offset, u8 data)
{
	offset &= 0x7f;
	if (offset == ISTAT)
	{
		if (BIT(data, 6))
		{
			reset_registers();
			m_regs[ISTAT] = 0x40;
			return;
		}
		m_regs[ISTAT] = (data & 0xb0) | (m_regs[ISTAT] & ~data & 0x04);
		update_irq();
		if (BIT(data, 7))
		{
			dma_interrupt(DSTAT_ABRT);
		}
		else if (BIT(data, 5) && m_scsi_state == RESELECT)
		{
			m_waiting = false;
			schedule();
		}
		return;
	}
	if (BIT(m_regs[ISTAT], 6))
	{
		return;
	}

	if (offset >= HIGH_REGS)
	{
		high_reg_w(offset, data);
		return;
	}

	if ((offset >= DSA && offset < DSA + 4) ||
		(offset >= TEMP && offset < TEMP + 4) ||
		(offset >= DBC && offset < SCRATCHA + 4) ||
		(offset >= SCRATCHB && offset < SCRATCHB + 4) ||
		(offset == CTEST0) || (offset == DWT))
	{
		m_regs[offset] = data;
		if ((offset == DSP + 3) && !BIT(m_regs[DMODE], 0))
		{
			start();
		}
		return;
	}

	switch (offset)
	{
	case SCNTL0:
	case SCNTL3:
	case SCID:
	case SXFER:
	case SDID:
	case GPREG:
	case STIME0:
	case RESPID:
	case CTEST4:
		m_regs[offset] = data;
		break;

	case DFIFO:
		m_regs[offset] = data & 0x7f;
		break;

	case CTEST5:
		m_regs[offset] = data & 0x18;
		break;

	case SLPAR:
		m_regs[offset] = 0;
		break;

	case MACNTL:
	case STIME1:
		m_regs[offset] = data & 0x0f;
		break;

	case GPCNTL:
		m_regs[offset] = data & 0xc3;
		break;

	case STEST2:
		m_regs[offset] = data & 0x9b;
		break;

	case STEST3:
		m_regs[offset] = data & 0xf5;
		break;

	case SCNTL1:
	{
		const bool reset = !BIT(m_regs[SCNTL1], 3) && BIT(data, 3);
		const bool adb = BIT(m_regs[SCNTL1] ^ data, 6);
		m_regs[offset] = data;
		if (adb)
		{
			drive_sodl();
		}
		scsi_lines(BIT(data, 3) ? S_RST : 0, S_RST);
		if (reset)
		{
			m_scsi_state = IDLE;
			m_scsi_timer->enable(false);
			scsi_lines(0, S_ALL & ~S_RST);
			if (m_scsi_bus)
			{
				m_scsi_bus->data_w(m_scsi_refid, 0);
			}
			scsi_interrupt(SIST0_RST);
		}
		break;
	}

	case SCNTL2:
		m_regs[offset] = data & 0x80;
		break;

	case SOCL:
		scsi_lines((data & 0x57) | ((data & 0x20) >> 2) | ((data & 0x80) >> 2) | ((data & 8) << 4), S_ALL & ~S_RST);
		break;

	case SODL:
		m_regs[offset] = data;
		if (BIT(m_regs[SCNTL1], 6))
		{
			drive_sodl();
		}
		break;

	case SIEN0:
	case SIEN1:
		m_regs[offset] = data & (offset == SIEN0 ? 0xff : 7);
		if (m_regs[offset] & m_regs[offset + 2])
		{
			m_scsi_irq |= 1 << (offset - SIEN0);
		}
		update_irq();
		break;

	case CTEST3:
		m_regs[offset] = data & 0x0a;
		break;

	case DMODE:
		m_regs[offset] = data & 0xf9;
		break;

	case DIEN:
		m_regs[offset] = data & 0x7d;
		m_dma_irq |= bool(m_regs[DSTAT] & m_regs[DIEN]);
		update_irq();
		break;

	case DCNTL:
		m_regs[offset] = data & 0x19;
		if (BIT(data, 2))
		{
			start();
		}
		break;

	default:
		break;
	}
}

void ncr53c8xx_device_base::bus_master_w(int state)
{
	m_bus_master = bool(state);
	if (m_bus_master)
	{
		schedule();
	}
	else
	{
		m_timer->enable(false);
	}
}

void ncr53c8xx_device_base::cio_w(int state)
{
	m_cio = bool(state);
}

void ncr53c8xx_device_base::cm_w(int state)
{
	m_cm = bool(state);
}

void ncr53c8xx_device_base::start()
{
	if ((m_regs[ISTAT] & 0xc0) || m_regs[DSTAT] || scsi_pending() ||
		m_stacked_dstat || m_stacked_sist0 || m_stacked_sist1)
	{
		return;
	}

	m_moving = false;
	m_scsi_state = IDLE;
	m_waiting = false;
	m_running = true;
	schedule();
}

void ncr53c8xx_device_base::schedule()
{
	if (m_running && m_bus_master && !m_waiting && !m_timer->enabled())
	{
		m_timer->adjust(clocks_to_attotime(32));
	}
}

void ncr53c8xx_device_base::update_irq()
{
	const bool state = m_dma_irq || m_scsi_irq || BIT(m_regs[ISTAT], 2);
	if (m_irq != state)
	{
		m_irq = state;
		m_irq_handler(state);
	}
}

void ncr53c8xx_device_base::dma_interrupt(u8 cause)
{
	m_running = false;
	m_timer->enable(false);

	if (m_regs[DSTAT] || scsi_pending() || m_interrupt_timer->enabled())
	{
		m_stacked_dstat |= cause;
	}
	else
	{
		m_regs[DSTAT] = cause;
		m_dma_irq |= bool(cause & m_regs[DIEN]);
		update_irq();
	}
}

TIMER_CALLBACK_MEMBER(ncr53c8xx_device_base::promote_interrupt)
{
	m_regs[DSTAT] = m_stacked_dstat;
	m_regs[SIST0] |= m_stacked_sist0;
	m_regs[SIST1] |= m_stacked_sist1;
	m_stacked_dstat = m_stacked_sist0 = m_stacked_sist1 = 0;

	if (m_regs[SIST0] & m_regs[SIEN0])
	{
		m_scsi_irq |= 1;
	}

	if (m_regs[SIST1] & m_regs[SIEN1])
	{
		m_scsi_irq |= 2;
	}

	m_dma_irq |= bool(m_regs[DSTAT] & m_regs[DIEN]);
	update_irq();
}

void ncr53c8xx_device_base::instruction_done()
{
	m_moving = false;
	if (m_running && BIT(m_regs[DCNTL], 4))
	{
		dma_interrupt(DSTAT_SSI);
	}
}

TIMER_CALLBACK_MEMBER(ncr53c8xx_device_base::execute)
{
	for (u32 budget = 0; budget != 32 && m_running && m_bus_master; ++budget)
	{
		if (m_scsi_state != IDLE)
		{
			if (!scsi_step())
			{
				m_waiting = true;
				break;
			}
		}
		else if (m_moving)
		{
			memory_move();
		}
		else
		{
			fetch();
		}
	}
	schedule();
}

void ncr53c8xx_device_base::fetch()
{
	const u32 pc = word_r(DSP);
	if (pc & 3)
	{
		dma_interrupt(DSTAT_IID);
		return;
	}
	const u32 instruction = m_memory_read(pc, 0xffff'ffff);
	word_w(DBC, instruction);
	word_w(DSPS, m_memory_read(pc + 4, 0xffff'ffff));
	word_w(DSP, pc + 8);

	switch (instruction >> 30)
	{
		case 3:
			if (BIT(instruction, 29) && load_store(instruction))
			{
				break;
			}
			m_move_source = word_r(DSPS);
			m_move_destination = m_memory_read(pc + 8, 0xffff'ffff);
			word_w(DSP, pc + 12);
			if ((instruction & 0x3f00'0000) || ((m_move_source ^ m_move_destination) & 3))
			{
				dma_interrupt(DSTAT_IID);
			}
			else if (instruction & 0xffffff)
			{
				m_moving = true;
			}
			else
			{
				instruction_done();
			}
			break;

		case 2:
			transfer_control(instruction);
			break;

		case 1:
			if (((instruction >> 27) & 7) >= 5)
			{
				register_operation(instruction);
			}
			else
			{
				scsi_io(instruction);
			}
			break;

		default:
			block_move(instruction);
			break;
	}
}

void ncr53c8xx_device_base::memory_move()
{
	const u32 remaining = word_r(DBC) & 0xffffff;
	if (!remaining)
	{
		instruction_done();
		return;
	}
	const u32 shift = (m_move_source & 3) * 8;
	const u32 count = std::min(remaining, 4 - (m_move_source & 3));
	const u32 mask = (0xffff'ffffU >> ((4 - count) * 8)) << shift;
	const u32 data = BIT(m_regs[DMODE], 5)
		? m_io_read(m_move_source & ~3U, mask)
		: m_memory_read(m_move_source & ~3U, mask);
	const u32 destination = m_move_destination & ~3U;
	m_move_source += count;
	m_move_destination += count;
	word_w(DBC, (u32(m_regs[DCMD]) << 24) | (remaining - count));

	// Commit progress before the write: a move to our own registers can reset
	// the device or start another script.  Do not overwrite that resulting state.
	if (BIT(m_regs[DMODE], 4))
	{
		m_io_write(destination, data, mask);
	}
	else
	{
		m_memory_write(destination, data, mask);
	}

	if (m_running && m_moving && !(word_r(DBC) & 0xffffff))
	{
		instruction_done();
	}
}

void ncr53c8xx_device_base::transfer_control(u32 instruction)
{
	const u32 opcode = (instruction >> 27) & 7;
	if ((opcode > 3) || (instruction & 0x0040'0000) ||
		(BIT(instruction, 21) && (instruction & 0x0006'0000)))
	{
		dma_interrupt(DSTAT_IID);
		return;
	}

	if (BIT(instruction, 16) && (!m_scsi_bus || ((m_scsi_bus->ctrl_r() & (S_REQ | S_ACK)) != S_REQ)))
	{
		m_scsi_state = PHASE;
		return;
	}

	m_scsi_state = IDLE;
	const bool wanted = BIT(instruction, 19);
	bool take = true;
	if (BIT(instruction, 21))
	{
		take = m_carry == wanted;
	}
	else
	{
		if (BIT(instruction, 17))
		{
			take &= ((m_regs[SSTAT1] & 7) == ((instruction >> 24) & 7)) == wanted;
		}
		if (BIT(instruction, 18))
		{
			take &= (((m_regs[SFBR] ^ instruction) & ~(instruction >> 8) & 0xff) == 0) == wanted;
		}
		if (!(instruction & 0x0006'0000))
		{
			take = wanted;
		}
	}

	if (take)
	{
		const u32 next = word_r(DSP);
		const u32 target = BIT(instruction, 23) ? next + util::sext(word_r(DSPS), 24) : word_r(DSPS);
		switch (opcode)
		{
			case 0:
				word_w(DSP, target);
				break;

			case 1:
				word_w(TEMP, next);
				word_w(DSP, target);
				break;

			case 2:
				word_w(DSP, word_r(TEMP));
				break;

			case 3:
				if (BIT(instruction, 20))
				{
					m_regs[ISTAT] |= 4;
					update_irq();
				}
				else
				{
					dma_interrupt(DSTAT_SIR);
				}
				break;
		}
	}
	instruction_done();
}

void ncr53c8xx_device_base::register_operation(u32 instruction)
{
	const u8 address = (instruction >> 16) & 0x7f;
	const u8 immediate = instruction >> 8;
	const u8 opcode = (instruction >> 27) & 7;
	if ((instruction & 0x0080'00ff) || word_r(DSPS))
	{
		dma_interrupt(DSTAT_IID);
		return;
	}
	const u8 operand = (opcode == 5) ? m_regs[SFBR] : (((instruction >> 24) & 7) ? reg_r(address) : 0);
	u8 value = immediate;
	switch ((instruction >> 24) & 7)
	{
		case 1:
			value = (operand << 1) | m_carry;
			m_carry = BIT(operand, 7);
			break;

		case 2:
			value = operand | immediate;
			break;

		case 3:
			value = operand ^ immediate;
			break;

		case 4:
			value = operand & immediate;
			break;

		case 5:
			value = (operand >> 1) | (m_carry ? 0x80 : 0);
			m_carry = BIT(operand, 0);
			break;

		case 6:
		case 7:
		{
			const u16 sum = u16(operand) + immediate + (BIT(instruction, 24) && m_carry ? 1 : 0);
			value = sum;
			m_carry = BIT(sum, 8);
			break;
		}
	}

	if ((opcode == 6) || (address == SFBR))
	{
		m_regs[SFBR] = value;
	}
	else
	{
		reg_w(address, value);
	}
	instruction_done();
}

bool ncr53c8xx_device_base::scsi_pending() const
{
	// Masked non-fatal causes are readable but do not set SIP.
	return m_scsi_irq || (m_regs[SIST0] & 0x8f) || (m_regs[SIST1] & 4);
}

void ncr53c8xx_device_base::promote_stacked_interrupts()
{
	if (!m_regs[DSTAT] && !scsi_pending() &&
		(m_stacked_dstat || m_stacked_sist0 || m_stacked_sist1) && !m_interrupt_timer->enabled())
	{
		m_interrupt_timer->adjust(clocks_to_attotime(3));
	}
}

void ncr53c8xx_device_base::scsi_interrupt(u8 sist0, u8 sist1)
{
	const bool halt = (sist0 & (m_regs[SIEN0] | 0x8f)) || (sist1 & (m_regs[SIEN1] | 4));
	if (halt)
	{
		m_running = false;
		m_timer->enable(false);
	}

	if (m_regs[DSTAT] || scsi_pending() || m_interrupt_timer->enabled())
	{
		m_stacked_sist0 |= sist0;
		m_stacked_sist1 |= sist1;
	}
	else
	{
		m_regs[SIST0] |= sist0;
		m_regs[SIST1] |= sist1;
		if (sist0 & m_regs[SIEN0])
		{
			m_scsi_irq |= 1;
		}
		if (sist1 & m_regs[SIEN1])
		{
			m_scsi_irq |= 2;
		}
		update_irq();
	}
}

u8 ncr53c8xx_device_base::dma_byte_r(u32 address)
{
	const u32 shift = (address & 3) * 8;
	const u32 mask = 0xffU << shift;
	return (BIT(m_regs[DMODE], 5) ? m_io_read(address & ~3U, mask) : m_memory_read(address & ~3U, mask)) >> shift;
}

void ncr53c8xx_device_base::dma_byte_w(u32 address, u8 data)
{
	const u32 shift = (address & 3) * 8;
	const u32 mask = 0xffU << shift;
	if (BIT(m_regs[DMODE], 4))
	{
		m_io_write(address & ~3U, u32(data) << shift, mask);
	}
	else
	{
		m_memory_write(address & ~3U, u32(data) << shift, mask);
	}
}

void ncr53c8xx_device_base::scsi_lines(u32 data, u32 mask)
{
	// SOCL uses a different pin order from NSCSI.
	u32 ctrl = (m_regs[SOCL] & 0x57) | ((m_regs[SOCL] & 0x20) >> 2) |
		((m_regs[SOCL] & 0x80) >> 2) | ((m_regs[SOCL] & 8) << 4);
	ctrl = (ctrl & ~mask) | (data & mask);
	m_regs[SOCL] = (ctrl & 0x57) | ((ctrl & S_BSY) << 2) |
		((ctrl & S_REQ) << 2) | ((ctrl & S_ATN) >> 4);

	if (m_scsi_bus)
	{
		m_scsi_bus->ctrl_w(m_scsi_refid, data, mask);
		m_bus_control = m_scsi_bus->ctrl_r();
	}
}

void ncr53c8xx_device_base::drive_sodl()
{
	// as an initiator, ADB drives SODL only while I/O is inactive
	if (m_scsi_bus)
	{
		const bool drive = BIT(m_regs[SCNTL1], 6) && !(m_scsi_bus->ctrl_r() & S_INP);
		m_scsi_bus->data_w(m_scsi_refid, drive ? (m_regs[SODL] | (u32(m_regs[SODL + 1]) << 8)) : 0);
	}
}

void ncr53c8xx_device_base::scsi_ctrl_changed()
{
	const u32 ctrl = m_scsi_bus->ctrl_r();
	const u32 changed = ctrl ^ m_bus_control;
	m_bus_control = ctrl;
	if (ctrl & S_REQ)
	{
		m_regs[SSTAT1] = ctrl & S_PHASE_MASK;
	}
	if ((changed & S_INP) && BIT(m_regs[SCNTL1], 6))
	{
		drive_sodl();
	}
	if ((changed & ctrl & S_RST) != 0)
	{
		m_scsi_state = IDLE;
		m_scsi_timer->enable(false);
		m_regs[SCNTL1] &= ~0x10;
		m_scsi_bus->data_w(m_scsi_refid, 0);
		scsi_lines(0, S_ALL & ~S_RST);
		scsi_interrupt(SIST0_RST);
	}
	else if ((changed & S_BSY) && !(ctrl & S_BSY) && BIT(m_regs[SCNTL1], 4))
	{
		m_regs[SCNTL1] &= ~0x10;
		if (BIT(m_regs[SCNTL2], 7) && m_scsi_state != DISCONNECT)
		{
			scsi_interrupt(SIST0_UDC);
		}
	}
	m_waiting = false;
	schedule();
}

void ncr53c8xx_device_base::block_move(u32 instruction)
{
	if (!m_scsi_bus || !BIT(instruction, 27) || BIT(m_regs[SCNTL0], 0) ||
		((instruction & 0x3000'0000) == 0x3000'0000))
	{
		dma_interrupt(DSTAT_IID);
		return;
	}
	u32 address = word_r(DSPS);
	if (BIT(instruction, 28))
	{
		address = word_r(DSA) + util::sext(address, 24);
		word_w(DBC, (instruction & 0xff00'0000) | (m_memory_read(address, 0xffff'ffff) & 0xffffff));
		address = m_memory_read(address + 4, 0xffff'ffff);
	}
	else if (BIT(instruction, 29))
	{
		address = m_memory_read(address, 0xffff'ffff);
	}
	word_w(DNAD, address);
	if (!(word_r(DBC) & 0xffffff))
	{
		dma_interrupt(DSTAT_IID);
		return;
	}

	// The DMA FIFO is always empty, so DFIFO follows DBC.
	m_regs[DFIFO] = m_regs[DBC] & 0x7f;
	m_first_byte = true;
	m_scsi_state = TRANSFER;
}

void ncr53c8xx_device_base::scsi_io(u32 instruction)
{
	const u32 opcode = (instruction >> 27) & 7;
	if (opcode == 3 || opcode == 4)
	{
		const bool set = opcode == 3;
		if (instruction & 0x400)
		{
			m_carry = set;
		}
		if (instruction & 0x200)
		{
			m_regs[SCNTL0] = (m_regs[SCNTL0] & ~1) | (set ? 1 : 0);
		}
		const u32 mask = (BIT(instruction, 3) ? S_ATN : 0) | (BIT(instruction, 6) ? S_ACK : 0);
		scsi_lines(set ? mask : 0, mask);
		instruction_done();
		return;
	}
	if (!m_scsi_bus || BIT(m_regs[SCNTL0], 0))
	{
		dma_interrupt(DSTAT_IID);
		return;
	}
	word_w(DNAD, BIT(instruction, 26) ? word_r(DSP) + util::sext(word_r(DSPS), 24) : word_r(DSPS));
	switch (opcode)
	{
		case 0:
			if (BIT(instruction, 25))
			{
				const u32 table = m_memory_read(word_r(DSA) + util::sext(instruction, 24), 0xffff'ffff);
				m_regs[SXFER] = table >> 8;
				m_regs[SDID] = (table >> 16) & 7;
				m_regs[SCNTL3] = table >> 24;
			}
			else
			{
				m_regs[SDID] = (instruction >> 16) & 7;
			}
			m_scsi_state = BUS_FREE;
			break;

		case 1:
			m_regs[SCNTL2] &= ~0x80;
			m_scsi_state = DISCONNECT;
			break;

		case 2:
			m_scsi_state = RESELECT;
			break;
	}
}

bool ncr53c8xx_device_base::scsi_step()
{
	const u32 ctrl = m_scsi_bus ? m_scsi_bus->ctrl_r() : 0;
	switch (m_scsi_state)
	{
	case BUS_FREE:
		if (ctrl & (S_BSY | S_SEL))
		{
			return false;
		}
		m_scsi_state = ARBITRATE;
		m_regs[SSTAT0] = 0x10;
		m_scsi_bus->data_w(m_scsi_refid, 1 << (m_regs[SCID] & 7));
		scsi_lines(S_BSY, S_BSY);
		m_scsi_timer->adjust(attotime::from_nsec(2400));
		return false;
	case ARBITRATE:
		return false;
	case SELECT:
		if (!(ctrl & S_BSY))
		{
			return false;
		}
		m_scsi_timer->enable(false);
		m_scsi_state = IDLE;
		m_regs[SCNTL1] |= 0x10;
		m_regs[SCNTL2] |= 0x80;
		m_scsi_bus->data_w(m_scsi_refid, 0);
		scsi_lines(0, S_SEL);
		scsi_interrupt(SIST0_CMP);
		instruction_done();
		return true;
	case TRANSFER:
		if (!(ctrl & S_REQ) || (ctrl & S_ACK))
		{
			return false;
		}
		if ((ctrl & S_PHASE_MASK) != (m_regs[DCMD] & 7))
		{
			m_scsi_state = IDLE;
			scsi_interrupt(SIST0_MA);
			return true;
		}
		{
			const u32 address = word_r(DNAD);
			const u32 count = word_r(DBC) & 0xffffff;
			const u32 bytes = std::min(scsi_width(), count);
			const u32 remaining = count - bytes;
			if (ctrl & S_INP)
			{
				const u32 data = m_scsi_bus->data_r();
				for (u32 i = 0; i != bytes; ++i)
				{
					const u8 byte = data >> (i * 8);
					m_regs[SIDL + i] = byte;
					m_regs[SLPAR] ^= byte;
					if (m_first_byte && !i)
					{
						m_regs[SFBR] = byte;
					}
					dma_byte_w(address + i, byte);
				}
			}
			else
			{
				u32 data = 0;
				for (u32 i = 0; i != bytes; ++i)
				{
					const u8 byte = dma_byte_r(address + i);
					m_regs[SODL + i] = byte;
					m_regs[SLPAR] ^= byte;
					data |= u32(byte) << (i * 8);
				}
				m_scsi_bus->data_w(m_scsi_refid, data);
				// Release attention before acknowledging the final outgoing message byte.
				if (!remaining && (ctrl & S_PHASE_MASK) == S_PHASE_MSG_OUT)
				{
					scsi_lines(0, S_ATN);
				}
			}
			m_first_byte = false;
			word_w(DNAD, address + bytes);
			word_w(DBC, (u32(m_regs[DCMD]) << 24) | remaining);
			m_regs[DFIFO] = remaining & 0x7f;
			m_scsi_state = ACKNOWLEDGE;
			scsi_lines(S_ACK, S_ACK);
		}
		return true;
	case ACKNOWLEDGE:
		if (ctrl & S_REQ)
		{
			return false;
		}
		m_scsi_bus->data_w(m_scsi_refid, 0);
		if (!(word_r(DBC) & 0xffffff))
		{
			m_scsi_state = IDLE;
			// SCRIPTS must explicitly clear ACK after receiving a message.
			if ((m_regs[DCMD] & 7) != S_PHASE_MSG_IN)
			{
				scsi_lines(0, S_ACK);
			}
			instruction_done();
		}
		else
		{
			m_scsi_state = TRANSFER;
			scsi_lines(0, S_ACK);
		}
		return true;
	case DISCONNECT:
		if (ctrl & S_REQ)
		{
			dma_interrupt(DSTAT_IID);
			return true;
		}
		if (ctrl & (S_BSY | S_SEL))
		{
			return false;
		}
		m_scsi_state = IDLE;
		instruction_done();
		return true;
	case RESELECT:
	{
		const u8 data = m_scsi_bus->data_r();
		if ((ctrl & (S_SEL | S_INP)) == (S_SEL | S_INP) && BIT(m_regs[SCID], 6) && (data & m_regs[RESPID]))
		{
			const u8 ids = data & ~m_regs[RESPID];
			m_regs[SSID] = 0;
			for (u8 id = 0; id != 8; ++id)
			{
				if (ids == (1 << id))
				{
					m_regs[SSID] = 0x80 | id;
				}
			}
			if (!BIT(m_regs[DCNTL], 0))
			{
				m_regs[SFBR] = data; // 53C700 compatibility
			}
			m_regs[SCNTL1] |= 0x10;
			m_regs[SCNTL2] |= 0x80;
			m_scsi_state = RESELECTED;
			scsi_lines(S_BSY, S_BSY);
			return true;
		}
		if (!BIT(m_regs[ISTAT], 5))
		{
			return false;
		}
		word_w(DSP, word_r(DNAD));
		m_scsi_state = IDLE;
		instruction_done();
		return true;
	}
	case RESELECTED:
		if (ctrl & S_SEL)
		{
			return false;
		}
		m_scsi_state = IDLE;
		scsi_lines(0, S_BSY);
		scsi_interrupt(SIST0_RSL);
		instruction_done();
		return true;
	case PHASE:
		if (!(ctrl & S_REQ) || (ctrl & S_ACK))
		{
			return false;
		}
		transfer_control(word_r(DBC));
		return true;
	default:
		return true;
	}
}

TIMER_CALLBACK_MEMBER(ncr53c8xx_device_base::scsi_tick)
{
	if (m_scsi_state == ARBITRATE)
	{
		const u8 id = m_regs[SCID] & 7;
		if (m_scsi_bus->data_r() & (0xffU << (id + 1)))
		{
			m_regs[SSTAT0] = 8;
			m_scsi_state = BUS_FREE;
			m_scsi_bus->data_w(m_scsi_refid, 0);
			scsi_lines(0, S_BSY);
		}
		else
		{
			m_regs[SSTAT0] = 4;
			m_scsi_state = SELECT;
			scsi_lines(S_SEL | (BIT(m_regs[DCMD], 0) ? S_ATN : 0), S_SEL | S_ATN);
			m_scsi_bus->data_w(m_scsi_refid, (1 << id) | (1 << (m_regs[SDID] & 7)));
			scsi_lines(0, S_BSY);
			const u8 timeout = m_regs[STIME0] & 15;
			if (timeout)
			{
				m_scsi_timer->adjust(attotime::from_usec((100 << (timeout - 1)) + 200));
			}
		}
	}
	else if (m_scsi_state == SELECT)
	{
		m_scsi_state = IDLE;
		m_scsi_bus->data_w(m_scsi_refid, 0);
		scsi_lines(0, S_ALL);
		// FIXME: the manual says UDC is also set; stacking it makes the BeBox ROM's bus scan time out
		scsi_interrupt(0, SIST1_STO);
	}
	m_waiting = false;
	schedule();
}


ncr53c810_device::ncr53c810_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ncr53c8xx_device_base(mconfig, NCR53C810, tag, owner, clock)
{
}

u8 ncr53c810_device::revision() const
{
	return 1;
}

u8 ncr53c810_device::chip_type() const
{
	return 0;
}
