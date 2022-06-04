// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Apple Macintosh 5380 SCSI helper

    The Macintosh Toolbox's SCSI Manager defines two methods each for
    reading and writing SCSI data (including data comparisons). All
    of these methods are implemented using the 5380's pseudo-DMA mode.
    For SCSIRead and SCSIWrite, the CPU polls DRQ before it reads or
    writes each byte, but SCSIRBlind and SCSIWBlind, in order to
    transfer data as quickly as possible, do not perform this check in
    software except before the very first byte (to give SCSI devices
    time to react to commands). Rather, the hardware enables DRQ onto
    DTACK (MC68000) or DSACK0 (MC68020/030) so wait states can be
    inserted to correspond to SCSI delays. Too long a delay will
    result in a BERR timeout, and the SCSI Manager anticipates bus
    errors by inserting its own exception handler in the vector table.
    The exact timeout is system-dependent. Later Macs will attempt a
    limited number of recoveries from bus errors during SCSI transfers
    before exiting with the scBusTOErr code; the MC68000's rudimentary
    handling of bus errors does not permit this.

    This causes some conflicts between MAME's line-level emulation of
    the SCSI bus and its current M68000 family core, which has no
    working concept of wait states and tries to execute each read or
    write instruction instantaneously. The problems are least
    pronounced on the Macintosh Plus, which never reads or writes more
    than one SCSI byte at a time in any case. However, the SE and
    later models implement the "blind" transfer modes using unrolled
    loops with MOVEP.L (MC68000) or MOVE.L (MC68020/030) to transfer
    four bytes at a time (after first transfering a single byte if its
    buffer pointer is not word-aligned).

    The workaround this device provides is to mediate pseudo-DMA
    transfers through four-byte FIFOs (implementing these using
    32-bit integers is not a coincidence) and halt the CPU to keep
    the FIFO filled during read operations and emptied during write
    operations. In the case of read operations, the buffer must of
    course be filled before CPU attempts its first data read, which
    could be a MOVEP.L or MOVE.L already. To make read operations a
    little more realistic, the device waits until the CPU polls DRQ as
    active before it begins halting the CPU to fill the FIFO. The CPU
    halt will be released if DRQ is not asserted within a certain
    time.

    It is possible for the current implementation to lose data if
    there are still bytes in the FIFO when the pseudo-DMA operation
    ends. However, this is unlikely to be relevant due to a defect in
    the SCSI Manager which Apple's documentation warns about.

**********************************************************************/

#include "emu.h"
#include "macscsi.h"

#define VERBOSE 0
#include "logmacro.h"

static constexpr u8 BAD_BYTE = 0xbb;
static constexpr u8 READ_ERROR = 1;
static constexpr u8 WRITE_ERROR = 0;

// device type definition
DEFINE_DEVICE_TYPE(MAC_SCSI_HELPER, mac_scsi_helper_device, "scsipdma", "Mac 5380 SCSI helper")

ALLOW_SAVE_TYPE(mac_scsi_helper_device::mode);

mac_scsi_helper_device::mac_scsi_helper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MAC_SCSI_HELPER, tag, owner, clock)
	, m_scsi_read_callback(*this)
	, m_scsi_write_callback(*this)
	, m_scsi_dma_read_callback(*this)
	, m_scsi_dma_write_callback(*this)
	, m_cpu_halt_callback(*this)
	, m_timeout_error_callback(*this)
	, m_timeout(attotime::from_usec(16))
	, m_pseudo_dma_timer(nullptr)
	, m_mode(mode::NON_DMA)
	, m_read_fifo_bytes(0)
	, m_write_fifo_bytes(0)
	, m_read_fifo_data(0)
	, m_write_fifo_data(0)
{
}

void mac_scsi_helper_device::device_resolve_objects()
{
	m_scsi_read_callback.resolve_safe(BAD_BYTE);
	m_scsi_write_callback.resolve_safe();
	m_scsi_dma_read_callback.resolve_safe(BAD_BYTE);
	m_scsi_dma_write_callback.resolve_safe();
	m_cpu_halt_callback.resolve_safe();
	m_timeout_error_callback.resolve_safe();
}

void mac_scsi_helper_device::device_start()
{
	m_pseudo_dma_timer = timer_alloc(FUNC(mac_scsi_helper_device::timer_callback), this);

	save_item(NAME(m_mode));
	save_item(NAME(m_read_fifo_bytes));
	save_item(NAME(m_write_fifo_bytes));
	save_item(NAME(m_read_fifo_data));
	save_item(NAME(m_write_fifo_data));
}

void mac_scsi_helper_device::device_reset()
{
	dma_stop();

	m_cpu_halt_callback(CLEAR_LINE);
	m_pseudo_dma_timer->enable(false);
}

void mac_scsi_helper_device::read_fifo_process()
{
	assert(m_read_fifo_bytes < 4);

	u8 data = m_scsi_dma_read_callback(6);
	++m_read_fifo_bytes;
	LOG("Read byte %02X into FIFO (%d/4 filled)\n", data, m_read_fifo_bytes);
	m_read_fifo_data |= u32(data) << (32 - m_read_fifo_bytes * 8);
	if (m_read_fifo_bytes != 4)
		m_pseudo_dma_timer->adjust(m_timeout);
	else
	{
		m_cpu_halt_callback(CLEAR_LINE);
		m_pseudo_dma_timer->enable(false);
	}
}

void mac_scsi_helper_device::write_fifo_process()
{
	assert(m_write_fifo_bytes != 0);
	--m_write_fifo_bytes;
	u8 data = BIT(m_write_fifo_data, m_write_fifo_bytes * 8, 8);
	LOG("Write byte %02X from FIFO (%d left)\n", data, m_write_fifo_bytes);
	m_scsi_dma_write_callback(0, data);

	if (m_write_fifo_bytes != 0)
		m_pseudo_dma_timer->adjust(m_timeout);
	else
	{
		m_cpu_halt_callback(CLEAR_LINE);
		m_pseudo_dma_timer->enable(false);
	}
}

WRITE_LINE_MEMBER(mac_scsi_helper_device::drq_w)
{
	if (state)
	{
		if (m_mode == mode::READ_DMA && m_read_fifo_bytes < 4)
			read_fifo_process();
		else if ((m_mode == mode::WRITE_DMA || m_mode == mode::BAD_DMA) && m_write_fifo_bytes != 0)
			write_fifo_process();
	}
}

TIMER_CALLBACK_MEMBER(mac_scsi_helper_device::timer_callback)
{
	if (m_mode == mode::WRITE_DMA && m_write_fifo_bytes != 0 && BIT(m_scsi_read_callback(5), 6))
		write_fifo_process();
	else if (m_mode == mode::READ_DMA || m_mode == mode::WRITE_DMA)
	{
		LOG("DMA timed out\n");
		m_cpu_halt_callback(CLEAR_LINE);
	}
	else
		dma_stop();
}

void mac_scsi_helper_device::dma_stop()
{
	if (m_read_fifo_bytes != 0)
		logerror("%s: %d unread byte(s) lost (%08X)\n", machine().describe_context(), m_read_fifo_bytes, m_read_fifo_data);
	if (m_write_fifo_bytes != 0)
		logerror("%s: %d unwritten byte(s) lost (%08X)\n", machine().describe_context(), m_write_fifo_bytes, m_write_fifo_data);

	m_mode = mode::NON_DMA;
	m_read_fifo_bytes = 0;
	m_write_fifo_bytes = 0;
	m_read_fifo_data = 0;
	m_write_fifo_data = 0;
}

u8 mac_scsi_helper_device::read_wrapper(bool pseudo_dma, offs_t offset)
{
	u8 data = BAD_BYTE;
	switch (offset & 7)
	{
	case 2:
		data = m_scsi_read_callback(2);
		if (!machine().side_effects_disabled() && !BIT(data, 1))
			dma_stop();
		break;

	case 5:
		data = m_scsi_read_callback(5);
		if (!machine().side_effects_disabled())
		{
			if (!BIT(m_scsi_read_callback(2), 1))
				dma_stop();
			if (m_mode == mode::READ_WAIT_DRQ && BIT(data, 6))
			{
				m_mode = mode::READ_DMA;
				m_read_fifo_data = u32(m_scsi_dma_read_callback(6)) << 24;
				LOG("%s: Pseudo-DMA read started: first byte = %02X\n", machine().describe_context(), m_read_fifo_data >> 24);
				m_read_fifo_bytes = 1;
				m_pseudo_dma_timer->adjust(m_timeout);
				m_cpu_halt_callback(ASSERT_LINE);
			}
		}
		if ((m_mode == mode::READ_DMA && m_read_fifo_bytes != 0) || (m_mode == mode::WRITE_DMA && m_write_fifo_bytes < 4))
			data |= 0x40;
		break;

	case 6:
		if (!machine().side_effects_disabled() && !BIT(m_scsi_read_callback(2), 1))
			dma_stop();
		if (m_read_fifo_bytes != 0)
		{
			data = BIT(m_read_fifo_data, 24, 8);
			if (!machine().side_effects_disabled())
			{
				--m_read_fifo_bytes;
				LOG("%s: CPU read byte %02X from FIFO (%d left)\n", machine().describe_context(), data, m_read_fifo_bytes);
				m_read_fifo_data <<= 8;
				if (BIT(m_scsi_read_callback(5), 6))
					read_fifo_process();
				else if (!m_pseudo_dma_timer->enabled())
				{
					m_pseudo_dma_timer->adjust(m_timeout);
					m_cpu_halt_callback(ASSERT_LINE);
				}
			}
		}
		else if (m_mode != mode::NON_DMA && pseudo_dma)
		{
			if (m_mode != mode::BAD_DMA)
			{
				logerror("%s: Read underflow on SCSI pseudo-DMA\n", machine().describe_context());
				m_mode = mode::BAD_DMA;
			}
			m_timeout_error_callback(READ_ERROR);
			m_pseudo_dma_timer->enable(false);
		}
		else
			data = pseudo_dma ? m_scsi_dma_read_callback(6) : m_scsi_read_callback(6);
		break;

	default:
		if (!machine().side_effects_disabled() && !BIT(m_scsi_read_callback(2), 1))
			dma_stop();
		data = m_scsi_read_callback(offset & 7);
		break;
	}

	return data;
}

void mac_scsi_helper_device::write_wrapper(bool pseudo_dma, offs_t offset, u8 data)
{
	switch (offset & 7)
	{
	case 0:
		if (m_mode == mode::BAD_DMA && pseudo_dma)
			m_timeout_error_callback(WRITE_ERROR);
		else if (m_mode == mode::WRITE_DMA && (m_write_fifo_bytes != 0 || !BIT(m_scsi_read_callback(5), 6)))
		{
			if (m_write_fifo_bytes < 4)
			{
				m_write_fifo_data = (m_write_fifo_data << 8) | data;
				++m_write_fifo_bytes;
				logerror("%s: CPU writing byte %02X into FIFO (%d/4 filled)\n", machine().describe_context(), data, m_write_fifo_bytes);
				if (!m_pseudo_dma_timer->enabled())
				{
					m_pseudo_dma_timer->adjust(m_timeout);
					m_cpu_halt_callback(ASSERT_LINE);
				}
			}
			else if (pseudo_dma)
			{
				logerror("%s: Write overflow on SCSI pseudo-DMA\n", machine().describe_context());
				m_timeout_error_callback(WRITE_ERROR);
				m_mode = mode::BAD_DMA;
			}
		}
		else if (pseudo_dma)
			m_scsi_dma_write_callback(0, data);
		else
			m_scsi_write_callback(0, data);
		break;

	case 2:
		if (!BIT(data, 1))
			dma_stop();
		m_scsi_write_callback(2, data);
		break;

	case 5:
		if (m_mode == mode::NON_DMA && BIT(m_scsi_read_callback(2), 1))
			m_mode = mode::WRITE_DMA;
		m_scsi_write_callback(5, data);
		break;

	case 6: case 7:
		if (m_mode == mode::NON_DMA && BIT(m_scsi_read_callback(2), 1))
			m_mode = mode::READ_WAIT_DRQ;
		m_scsi_write_callback(offset & 7, data);
		break;

	default:
		m_scsi_write_callback(offset & 7, data);
		break;
	}
}
