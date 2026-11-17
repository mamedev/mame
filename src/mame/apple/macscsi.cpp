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

    The SE and later models implement the "blind" transfer modes using
    unrolled loops with MOVEP.L (MC68000) or MOVE.L (MC68020/030) to
    transfer four bytes at a time. On the MC68020/030 a long access to
    the 8-bit port is four bus cycles, each waiting for DRQ; MAME
    performs it as a single access. A pseudo-DMA access that finds no
    DRQ for one of its bytes therefore halts the CPU and has the access
    (MC68000) or instruction (MC68020/030) retried when DRQ comes,
    keeping the bytes already transferred for the retry. No byte is
    transferred ahead of the access that wants it.

**********************************************************************/

#include "emu.h"
#include "macscsi.h"

#include "cpu/m68000/m68kmusashi.h"

#define VERBOSE 0
#include "logmacro.h"

static constexpr u8 READ_ERROR = 1;
static constexpr u8 WRITE_ERROR = 0;

// device type definition
DEFINE_DEVICE_TYPE(MAC_SCSI_HELPER, mac_scsi_helper_device, "scsipdma", "Mac 5380 SCSI helper")

mac_scsi_helper_device::mac_scsi_helper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MAC_SCSI_HELPER, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_scsi_read_callback(*this, 0xff)
	, m_scsi_write_callback(*this)
	, m_scsi_dma_read_callback(*this, 0xff)
	, m_scsi_dma_write_callback(*this)
	, m_timeout_error_callback(*this)
	// the SE's BBU times out any access after 265 ms (Guide to the Macintosh
	// Family Hardware, 2nd ed., p. 126); no figure found for other models
	, m_timeout(attotime::from_msec(265))
	, m_timeout_timer(nullptr)
	, m_drq(false)
	, m_waiting(false)
	, m_timed_out(false)
	, m_count(0)
	, m_data(0)
{
}

void mac_scsi_helper_device::device_start()
{
	m_timeout_timer = timer_alloc(FUNC(mac_scsi_helper_device::timeout), this);

	// wait() restarts the access on a 68020/030, which Musashi only honours
	// with instruction restart on; that is otherwise on only while the PMMU is
	// enabled, so not on a II with the HMMU (as scsidma.cpp does)
	if (!m_maincpu->cpu_is_interruptible())
		downcast<m68000_musashi_device &>(*m_maincpu).set_emmu_enable(true);

	save_item(NAME(m_drq));
	save_item(NAME(m_waiting));
	save_item(NAME(m_timed_out));
	save_item(NAME(m_count));
	save_item(NAME(m_data));
}

void mac_scsi_helper_device::device_reset()
{
	m_timeout_timer->enable(false);
	if (m_waiting)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_waiting = false;
	m_timed_out = false;
	m_count = 0;
	m_data = 0;
}

void mac_scsi_helper_device::drq_w(int state)
{
	m_drq = state;
	if (state && m_waiting)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(mac_scsi_helper_device::timeout)
{
	LOG("pseudo-DMA access timed out\n");
	m_timed_out = true;
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}

// the retried access after a timeout ends in a bus error; bytes already
// transferred are gone, as they would be on hardware
bool mac_scsi_helper_device::timed_out(u8 error)
{
	if (!m_timed_out)
		return false;

	logerror("%s: SCSI pseudo-DMA %s timed out after %d byte(s)\n", machine().describe_context(), error == READ_ERROR ? "read" : "write", m_count);
	m_timed_out = false;
	m_waiting = false;
	m_count = 0;
	m_data = 0;
	m_timeout_error_callback(error);
	return true;
}

// hold the CPU until DRQ, then have it retry the access; the timeout runs
// from the last byte transferred
void mac_scsi_helper_device::wait(bool progress)
{
	if (!m_waiting || progress)
		m_timeout_timer->adjust(m_timeout);
	m_waiting = true;

	if (m_maincpu->cpu_is_interruptible())
		m_maincpu->retry_access();
	else
	{
		downcast<m68000_musashi_device &>(*m_maincpu).restart_this_instruction();
		m_maincpu->abort_timeslice();
	}
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void mac_scsi_helper_device::done()
{
	if (m_waiting)
		m_timeout_timer->enable(false);
	m_waiting = false;
	m_count = 0;
	m_data = 0;
}

// read the given number of bytes, returned in the low bits
u32 mac_scsi_helper_device::dma_read(unsigned bytes)
{
	if (machine().side_effects_disabled())
		return ~u32(0);
	if (timed_out(READ_ERROR))
		return ~u32(0);

	bool const progress = m_drq;
	while (m_count < bytes && m_drq)
	{
		// reading the data clears DRQ until the next byte is latched
		m_data = (m_data << 8) | m_scsi_dma_read_callback(6);
		m_count++;
	}
	if (m_count < bytes)
	{
		wait(progress);
		return ~u32(0);
	}

	u32 const data = m_data;
	done();
	return data;
}

// write the given number of bytes from the low bits
void mac_scsi_helper_device::dma_write(u32 data, unsigned bytes)
{
	if (timed_out(WRITE_ERROR))
		return;

	bool const progress = m_drq;
	while (m_count < bytes && m_drq)
	{
		// writing the data clears DRQ until the byte has gone out
		m_scsi_dma_write_callback(0, BIT(data, (bytes - 1 - m_count) * 8, 8));
		m_count++;
	}
	if (m_count < bytes)
		wait(progress);
	else
		done();
}

u8 mac_scsi_helper_device::read_wrapper(bool pseudo_dma, offs_t offset)
{
	if (pseudo_dma && (offset & 7) == 6)
		return dma_read(1);
	return m_scsi_read_callback(offset & 7);
}

void mac_scsi_helper_device::write_wrapper(bool pseudo_dma, offs_t offset, u8 data)
{
	if (pseudo_dma && (offset & 7) == 0)
		dma_write(data, 1);
	else
		m_scsi_write_callback(offset & 7, data);
}

u32 mac_scsi_helper_device::pseudo_dma_r(offs_t offset, u32 mem_mask)
{
	switch (mem_mask)
	{
	case 0xff000000:
		return dma_read(1) << 24;

	case 0xffff0000:
		return dma_read(2) << 16;

	case 0xffffffff:
		return dma_read(4);

	default:
		logerror("pseudo_dma_r: unknown mem_mask %08x\n", mem_mask);
		return 0;
	}
}

void mac_scsi_helper_device::pseudo_dma_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (mem_mask)
	{
	case 0xff000000:
		dma_write(data >> 24, 1);
		break;

	case 0xffff0000:
		dma_write(data >> 16, 2);
		break;

	case 0xffffffff:
		dma_write(data, 4);
		break;

	default:
		logerror("pseudo_dma_w: unknown mem_mask %08x\n", mem_mask);
		break;
	}
}
