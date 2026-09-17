// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD1015 Buffer Manager Control Processor

**********************************************************************/

#include "emu.h"
#include "wd1015.h"

#include <algorithm>
#include <cstring>


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WD1015, wd1015_device, "wd1015", "WD1015 Buffer Manager")
DEFINE_DEVICE_TYPE(WD1015PL5402, wd1015pl5402_device, "wd1015pl5402", "WD1015-PL-54-02 Buffer Manager")


//-------------------------------------------------
//  wd1015_device - constructor
//-------------------------------------------------

wd1015_device::wd1015_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_host(*this, finder_base::DUMMY_TAG)
	, m_hdc(*this, finder_base::DUMMY_TAG)
	, m_hdd{ nullptr, nullptr }
	, m_cdb{ 0 }
	, m_cdb_ptr(0)
	, m_completion(0)
	, m_sense{ 0 }
	, m_secbuf{ 0 }
	, m_initbuf{ 0 }
	, m_secptr(0)
	, m_chunk_sectors(1)
	, m_din_sense(false)
	, m_din_len(0)
	, m_phase(PHASE_CDB)
	, m_pending_command(0)
	, m_remaining(0)
	, m_sector_number(0)
	, m_error(false)
	, m_cur_drive(0)
	, m_cur_head(0)
	, m_cur_cylinder(0)
	, m_cur_sector(0)
	, m_geometry{ { false, 0, 0 }, { false, 0, 0 } }
{
}

wd1015_device::wd1015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd1015_device(mconfig, WD1015, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd1015_device::device_start()
{
	m_hdd[0] = m_hdc->subdevice<harddisk_image_device>("0");
	m_hdd[1] = m_hdc->subdevice<harddisk_image_device>("1");

	save_item(NAME(m_cdb));
	save_item(NAME(m_cdb_ptr));
	save_item(NAME(m_completion));
	save_item(NAME(m_sense));
	save_item(NAME(m_secbuf));
	save_item(NAME(m_initbuf));
	save_item(NAME(m_secptr));
	save_item(NAME(m_chunk_sectors));
	save_item(NAME(m_din_sense));
	save_item(NAME(m_din_len));
	save_item(NAME(m_phase));
	save_item(NAME(m_pending_command));
	save_item(NAME(m_remaining));
	save_item(NAME(m_sector_number));
	save_item(NAME(m_error));
	save_item(NAME(m_cur_drive));
	save_item(NAME(m_cur_head));
	save_item(NAME(m_cur_cylinder));
	save_item(NAME(m_cur_sector));
	save_item(STRUCT_MEMBER(m_geometry, programmed));
	save_item(STRUCT_MEMBER(m_geometry, cylinders));
	save_item(STRUCT_MEMBER(m_geometry, heads));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wd1015_device::device_reset()
{
	m_hdc->drdy_w((m_hdd[0]->exists() || m_hdd[1]->exists()) ? 1 : 0);
	m_hdc->sc_w(1);
	// the WD1015 buffer manager isn't modeled; its BRDY handshake is tied
	// permanently ready since the sector buffer here is plain host RAM
	m_hdc->brdy_w(1);

	m_phase = PHASE_CDB;
	m_cdb_ptr = 0;
	m_secptr = 0;
	m_completion = 0;
	m_error = false;
}


//-------------------------------------------------
//  WD11C00-17 host interface glue
//-------------------------------------------------

void wd1015_device::mr_w(int state)
{
	// board-level software reset: abort whatever command phase we were in
	if (state)
	{
		m_phase = PHASE_CDB;
		m_cdb_ptr = 0;
	}
}

void wd1015_device::busy_w(int state)
{
	// out_busy_callback is active low; state == 0 means the host has just
	// written the Select register (BUSY newly asserted) -- a new command begins
	if (state == 0)
	{
		m_phase = PHASE_CDB;
		m_cdb_ptr = 0;
	}
}


//-------------------------------------------------
//  shared RAM window (WD11C00-17 Random Address counter) -- this is where the
//  WD1015's own command-block parsing and buffer management is high-level
//  emulated.  ram_r/ram_w see every byte the host (PIO or DMA) or our own
//  WD1010/WD2010 command sequencing moves through the Data register; which
//  buffer they land in is tracked here by protocol phase rather than by the
//  WD11C00-17's internal RA value (see wd1002a_wx1.h for why).
//-------------------------------------------------

uint8_t wd1015_device::ram_r(offs_t offset)
{
	if (m_phase == PHASE_DATA_IN && m_secptr < m_din_len)
	{
		uint8_t const data = m_din_sense ? m_sense[m_secptr] : m_secbuf[m_secptr];
		m_secptr++;
		keep_host_address_in_window(m_din_len);
		if (m_secptr == m_din_len)
			advance_or_finish();
		return data;
	}

	// PHASE_STATUS (or a stray read once the data phase is drained): serve the
	// completion byte and release BUSY so the host can select() the next command
	m_host->ireq_w(0);
	return m_completion;
}

void wd1015_device::ram_w(offs_t offset, uint8_t data)
{
	switch (m_phase)
	{
	case PHASE_CDB:
		m_cdb[m_cdb_ptr++] = data;
		if (m_cdb_ptr == 6)
			begin_command();
		break;

	case PHASE_INIT_DATA:
		// INITIALIZE DRIVE CHARACTERISTICS parameter block: bytes 0-1 are the
		// max cylinder count (big-endian), byte 2 is the head count; the rest
		// (write precomp/RWC cylinders, ECC burst length) aren't used for the
		// bound checks this exists for, so they're only counted, not stored
		if (m_secptr < 8)
			m_initbuf[m_secptr] = data;
		if (++m_secptr >= 8)
		{
			int const drive = BIT(m_cdb[1], 5);
			m_geometry[drive].cylinders = (uint16_t(m_initbuf[0]) << 8) | m_initbuf[1];
			m_geometry[drive].heads = m_initbuf[2];
			m_geometry[drive].programmed = true;
			complete(0);
		}
		break;

	case PHASE_DATA_OUT:
	{
		uint32_t const length = uint32_t(512) * m_chunk_sectors;
		if (m_secptr < sizeof(m_secbuf))
			m_secbuf[m_secptr++] = data;
		keep_host_address_in_window(length);
		if (m_secptr >= length)
		{
			if (m_pending_command == CDB_WRITE_BUFFER)
				complete(0); // no disk access -- the buffer is now what the host sent
			else
				issue_hdc_command(m_chunk_sectors > 1 ? 0x34 : 0x30); // WRITE SECTOR, +multi
		}
		break;
	}

	default:
		break;
	}
}


//-------------------------------------------------
//  WD1010/WD2010 sector-data callbacks -- during cmd_read_sector()/cmd_write_sector()
//  the task file core moves a whole 512-byte sector through these synchronously
//-------------------------------------------------

uint8_t wd1015_device::hdc_data_r()
{
	return (m_secptr < sizeof(m_secbuf)) ? m_secbuf[m_secptr++] : 0;
}

void wd1015_device::hdc_data_w(uint8_t data)
{
	if (m_secptr < sizeof(m_secbuf))
		m_secbuf[m_secptr++] = data;
}

void wd1015_device::hdc_intrq_w(int state)
{
	if (!state || m_phase != PHASE_WAIT_HDC)
		return;

	uint8_t const status = m_hdc->read(7);

	// wd1010_device fires an early completion interrupt as soon as a sector lands in
	// its internal buffer (status bit 1, CIP, is still set at that point) -- real
	// hardware would only see this once the buffer manager (unimplemented here, see
	// device_reset()) asserts BRDY, so wd1010 raises the *real* completion later, on
	// its own timer, once CIP finally clears. Reading status here (which clears wd1010's
	// own intrq) sets up a fresh edge for that later interrupt, so just ignore this one.
	if (BIT(status, 1))
		return;

	m_error = BIT(status, 0);
	if (m_error)
	{
		// report where the transfer actually got to (m_cur_*), not the CDB's original
		// request -- for a multi-sector transfer these have long since diverged
		m_sense[0] = 0x80 | sense_code_from_status(status);
		m_sense[1] = (m_cdb[1] & 0x20) | m_cur_head;
		m_sense[2] = ((m_cur_cylinder >> 8) << 6) | m_cur_sector;
		m_sense[3] = m_cur_cylinder & 0xff;

		complete(1);
		return;
	}

	if (m_pending_command == CDB_READ)
		start_data_in(false, 512 * m_chunk_sectors); // the whole burst is already in m_secbuf via hdc_data_w
	else
		advance_or_finish();
}


//-------------------------------------------------
//  WD1002/XT command descriptor block protocol (WD1015 high-level emulation)
//-------------------------------------------------

void wd1015_device::setup_task_file(int drive, int head, int cylinder, int sector)
{
	m_cur_drive = drive & 1;
	m_cur_head = head;
	m_cur_cylinder = cylinder;
	m_cur_sector = sector;

	m_hdc->drdy_w(m_hdd[drive & 1]->exists() ? 1 : 0);
	m_hdc->head_w(head);
	m_hdc->write(2, 1); // sector count: we always run the task file one sector at a time
	m_sector_number = sector;
	m_hdc->write(3, m_sector_number);
	m_hdc->write(4, cylinder & 0xff);
	m_hdc->write(5, (cylinder >> 8) & 0xff);
	m_hdc->write(6, (drive & 1) << 3); // SDH drive-select bits (head is applied via head_w above)
}

bool wd1015_device::format_track(int drive, int head, int cylinder)
{
	harddisk_image_device *const hdd = m_hdd[drive & 1];
	if (!hdd->exists())
		return false;

	// the real drive writes physical sync/gap/ID structure that a CHD has no notion of;
	// this only (re)initializes each sector's data area. The fill pattern the firmware
	// actually uses was never traced (see the notes on $6C3/$6E2 in wd1015.asm), so this
	// uses zero as a safe placeholder rather than guessing
	uint8_t fill[512];
	std::memset(fill, 0, sizeof(fill));

	int const sectors_per_track = hdd->get_info().sectors;
	uint32_t const lba = (uint32_t(cylinder) * max_heads(drive & 1) + head) * sectors_per_track;
	for (int sector = 0; sector < sectors_per_track; sector++)
		if (!hdd->write(lba + sector, fill))
			return false;

	return true;
}

int wd1015_device::max_heads(int drive) const
{
	if (m_geometry[drive].programmed)
		return m_geometry[drive].heads;
	// get_info() on a drive with no media dereferences a null CHD handle; callers are
	// expected to check exists() first (an absent drive has no real geometry anyway),
	// but fall back to 0 rather than crash if one doesn't
	return m_hdd[drive]->exists() ? m_hdd[drive]->get_info().heads : 0;
}

int wd1015_device::max_cylinders(int drive) const
{
	if (m_geometry[drive].programmed)
		return m_geometry[drive].cylinders;
	return m_hdd[drive]->exists() ? m_hdd[drive]->get_info().cylinders : 0;
}

bool wd1015_device::multi_sector_out_of_range(int drive, int head, int cylinder, int sector, int count) const
{
	int const sectors_per_track = m_hdd[drive]->get_info().sectors;
	int const heads_count = max_heads(drive);
	int const last_linear = sector + count - 1;
	int const final_head_linear = head + last_linear / sectors_per_track;
	int const final_cylinder = cylinder + final_head_linear / heads_count;
	return final_cylinder >= max_cylinders(drive);
}

uint8_t wd1015_device::sense_code_from_status(uint8_t status) const
{
	// modeled on the firmware's HDC status/error decode at $279-$2A7 (wd1015.asm); the
	// WF/AC/BB/DM cases below are confirmed there, the remaining documented sense codes
	// (no seek complete, track 0 not found, ECC-related codes, ...) are not reachable
	// from this generic path and are not modeled
	if (BIT(status, 5))
		return 0x03; // write fault

	uint8_t const err = m_hdc->read(1);
	if (BIT(err, 2))
		return 0x04; // aborted command
	if (BIT(err, 7))
		return 0x19; // bad block -> bad track
	if (BIT(err, 0))
		return 0x12; // data address mark not found
	return 0x15; // seek error (fallback)
}

void wd1015_device::issue_hdc_command(uint8_t command)
{
	m_secptr = 0;
	// set before the write, not after: a drive that's already not-ready completes
	// synchronously from inside write() (wd1010_device aborts immediately rather than
	// seeking), so hdc_intrq_w must see PHASE_WAIT_HDC by the time that reentrant call
	// happens or it silently drops the completion and this waits forever
	m_phase = PHASE_WAIT_HDC;
	m_hdc->write(7, command);
}

void wd1015_device::begin_command()
{
	// wd1015_device never uses the WD11C00-17's own RA counter for addressing (see
	// ram_r/ram_w above), so nothing else ever resets it; left alone it climbs by every
	// byte moved in every phase and eventually crosses bit 10, which force-clears the
	// chip's DRQ status mid-transfer. Resetting it at the start of every command keeps
	// it well clear of that boundary. (Must not be done from busy_w(): that runs nested
	// inside the WD11C00-17's own check_interrupt()/select(), and calling back into the
	// same device from there recurses forever -- m_out_busy_cb fires before m_busy is
	// updated, so a reentrant call sees the same stale mismatch and loops.)
	m_host->write(0x60, 0);

	int const drive = BIT(m_cdb[1], 5);
	int const head = m_cdb[1] & 0x1f;
	int const cylinder = ((m_cdb[2] & 0xc0) << 2) | m_cdb[3];
	int const sector = m_cdb[2] & 0x3f;
	int const count = m_cdb[4] ? m_cdb[4] : 256; // a block count of 0 means 256 sectors, not 1 ($434)

	m_remaining = 0;
	m_pending_command = m_cdb[0];

	switch (m_cdb[0])
	{
	case CDB_TEST_READY:
	case CDB_DRIVE_DIAG: // the real diagnostic recalibrates and walks the drive with SCAN ID;
	                      // simplified here to the same readiness check as TEST DRIVE READY
	{
		bool const ready = m_hdd[drive]->exists();
		if (!ready)
		{
			m_sense[0] = 0x04; // drive not ready
			m_sense[1] = m_cdb[1];
			m_sense[2] = m_cdb[2];
			m_sense[3] = m_cdb[3];
		}
		complete(ready ? 0 : 1);
		break;
	}

	case CDB_SENSE:
		start_data_in(true, 4);
		break;

	case CDB_ECC_BURST:
		// no ECC correction is modeled, so there is never a burst length to report
		m_secbuf[0] = 0;
		start_data_in(false, 1);
		break;

	case CDB_READ_BUFFER:
		start_data_in(false, 512); // whatever is currently sitting in m_secbuf
		break;

	case CDB_WRITE_BUFFER:
		m_chunk_sectors = 1;
		m_phase = PHASE_DATA_OUT;
		m_secptr = 0;
		m_host->cd_w(0);
		m_host->io_w(0);
		break;

	case CDB_RAM_DIAG:
		// walking-pattern test of the sector buffer; there's nothing here that can
		// fail, so always succeed
		complete(0);
		break;

	case CDB_CTRL_DIAG:
		// firmware ROM checksum / sector buffer / RAM / ECC / task-file sub-tests: none
		// of them are modeled (there is no 8049 ROM or ECC logic to check), so always
		// succeed
		controller_diagnostic_extra();
		complete(0);
		break;

	case CDB_FORMAT_TRK:
	case CDB_FORMAT_BAD: // bad-track marking isn't modeled; formats the track exactly as FORMAT TRACK
	{
		bool const ok = format_track(drive, head, cylinder);
		if (!ok)
		{
			m_sense[0] = 0x04; // drive not ready
			m_sense[1] = m_cdb[1];
			m_sense[2] = m_cdb[2];
			m_sense[3] = m_cdb[3];
		}
		complete(ok ? 0 : 1);
		break;
	}

	case CDB_FORMAT_DRV:
	{
		bool ok = m_hdd[drive]->exists();
		int const heads = max_heads(drive);
		int const cylinders = max_cylinders(drive);
		for (int cyl = 0; ok && cyl < cylinders; cyl++)
			for (int h = 0; ok && h < heads; h++)
				ok = format_track(drive, h, cyl);
		if (!ok)
		{
			m_sense[0] = 0x04; // drive not ready
			m_sense[1] = m_cdb[1];
			m_sense[2] = m_cdb[2];
			m_sense[3] = m_cdb[3];
		}
		complete(ok ? 0 : 1);
		break;
	}

	case CDB_READ_LONG:
	case CDB_WRITE_LONG:
		// 516-byte transfers (512 data bytes + 4 ECC bytes) aren't modeled; reject
		// rather than mis-transfer, since nothing is known to issue these
		m_sense[0] = 0x20; // invalid command
		m_sense[1] = m_cdb[1];
		m_sense[2] = m_cdb[2];
		m_sense[3] = m_cdb[3];
		complete(1);
		break;

	case CDB_INIT_DRIVE:
		m_phase = PHASE_INIT_DATA;
		m_secptr = 0;
		m_host->cd_w(0);
		m_host->io_w(0);
		break;

	case CDB_RECALIBRATE:
		// cdb[5]'s step-rate nibble and disable-retry bit are both discarded by the real
		// firmware ($24C issues a bare 0x11); there is no retry logic to disable here either
		setup_task_file(drive, 0, 0, 0);
		issue_hdc_command(0x11); // RESTORE
		break;

	case CDB_SEEK:
		// ditto re: cdb[5]; the exact HDC command byte the firmware uses for SEEK isn't
		// among the ones confirmed from the disassembly (see wd1015.asm notes), so this
		// keeps wd1010's SEEK opcode with a step rate of 0 rather than one from the CDB
		setup_task_file(drive, head, cylinder, sector);
		issue_hdc_command(0x70); // SEEK
		break;

	case CDB_READ:
	case CDB_VERIFY:
	{
		// a drive-less request (e.g. the BIOS probing for a second drive that isn't
		// fitted) must be rejected before any geometry lookup: max_heads()/max_cylinders()
		// have nothing to report for it
		if (!m_hdd[drive]->exists())
		{
			m_sense[0] = 0x04; // drive not ready
			m_sense[1] = m_cdb[1];
			m_sense[2] = m_cdb[2];
			m_sense[3] = m_cdb[3];
			complete(1);
			break;
		}

		// bound-check the *whole* multi-sector run up front: wd1010's native multi-sector
		// mode (below) sequences it internally with no per-sector checkpoint of our own to
		// catch an out-of-range tail the way a single-sector reissue loop could
		if (multi_sector_out_of_range(drive, head, cylinder, sector, count))
		{
			m_sense[0] = 0x80 | 0x21; // illegal disk address
			m_sense[1] = m_cdb[1];
			m_sense[2] = m_cdb[2];
			m_sense[3] = m_cdb[3];
			complete(1);
			break;
		}

		m_remaining = count;
		setup_task_file(drive, head, cylinder, sector);
		// wd1010's sector-count register is 8 bits, so a 256-sector request (cdb[4]==0)
		// is capped to a 255-sector burst here; advance_or_finish() issues the 1-sector
		// remainder as a second command once this one completes
		m_chunk_sectors = (count > 255) ? 255 : count;
		m_hdc->write(2, m_chunk_sectors);
		// bit 2 tells wd1010 to chain through the sector-count register itself instead of
		// stopping after one sector, so the whole burst arrives as a single uninterrupted
		// DRQ-asserted DMA transfer rather than us re-triggering it sector by sector (which
		// left DRQ's status bits stale between sectors and stalled the host's DMA mid-burst)
		issue_hdc_command(m_chunk_sectors > 1 ? 0x24 : 0x20);
		break;
	}

	case CDB_WRITE:
		if (!m_hdd[drive]->exists())
		{
			m_sense[0] = 0x04; // drive not ready
			m_sense[1] = m_cdb[1];
			m_sense[2] = m_cdb[2];
			m_sense[3] = m_cdb[3];
			complete(1);
			break;
		}

		if (multi_sector_out_of_range(drive, head, cylinder, sector, count))
		{
			m_sense[0] = 0x80 | 0x21; // illegal disk address
			m_sense[1] = m_cdb[1];
			m_sense[2] = m_cdb[2];
			m_sense[3] = m_cdb[3];
			complete(1);
			break;
		}

		m_remaining = count;
		setup_task_file(drive, head, cylinder, sector);
		// same native chaining as READ/VERIFY above: collect the whole chunk from the host
		// first (PHASE_DATA_OUT below), then let wd1010 write it as one multi-sector burst
		m_chunk_sectors = (count > 255) ? 255 : count;
		m_hdc->write(2, m_chunk_sectors);
		m_phase = PHASE_DATA_OUT;
		m_secptr = 0;
		m_host->cd_w(0);
		m_host->io_w(0);
		break;

	default:
		m_sense[0] = 0x20; // illegal command
		m_sense[1] = m_cdb[1];
		m_sense[2] = m_cdb[2];
		m_sense[3] = m_cdb[3];
		complete(1);
		break;
	}
}

// The WD11C00-17's own RA counter is never used for addressing here (see the class
// comment on ram_r/ram_w), but it still increments on every byte the host moves through
// the data register, and the chip force-clears its DRQ status as soon as RA crosses bit
// 10. The real firmware re-points RA at the sector buffer between sectors, which keeps
// it clear of that boundary; without doing the same, any transfer of 1024 bytes or more
// -- i.e. every multi-sector READ or WRITE -- loses DRQ partway through and stalls the
// host's DMA until it times out the command.
void wd1015_device::keep_host_address_in_window(uint32_t length)
{
	if ((m_secptr & 0x1ff) == 0 && m_secptr < length)
		m_host->write(0x60, 0);
}

void wd1015_device::advance_or_finish()
{
	// only gets here again if the original request was a >255-sector burst (cdb[4]==0,
	// i.e. 256 sectors -- the CDB's count field is a single byte), once the first
	// 255-sector chunk completes; the 1-sector remainder is issued as a second command
	int const done = std::min<int>(m_remaining, m_chunk_sectors);
	m_remaining -= done;

	if (m_remaining > 0 && !m_error)
	{
		int const sectors_per_track = m_hdd[m_cur_drive]->get_info().sectors;
		int const heads_count = max_heads(m_cur_drive);

		int const linear = m_cur_sector + done;
		m_cur_sector = linear % sectors_per_track;
		int const total_head = m_cur_head + linear / sectors_per_track;
		m_cur_head = total_head % heads_count;
		m_cur_cylinder += total_head / heads_count;

		if (m_cur_cylinder >= max_cylinders(m_cur_drive))
		{
			// shouldn't happen -- begin_command() already bound-checked the whole run --
			// but guard anyway rather than silently reading/writing an out-of-range address
			m_sense[0] = 0x80 | 0x21; // illegal disk address
			m_sense[1] = (m_cdb[1] & 0x20) | m_cur_head;
			m_sense[2] = ((m_cur_cylinder >> 8) << 6) | m_cur_sector;
			m_sense[3] = m_cur_cylinder & 0xff;
			complete(1);
			return;
		}

		m_sector_number = m_cur_sector;
		m_hdc->head_w(m_cur_head);
		m_hdc->write(3, m_sector_number);
		m_hdc->write(4, m_cur_cylinder & 0xff);
		m_hdc->write(5, (m_cur_cylinder >> 8) & 0xff);

		if (m_pending_command == CDB_WRITE)
		{
			// the remainder of a >255-sector burst is at most 1 sector (see above)
			m_chunk_sectors = 1;
			m_phase = PHASE_DATA_OUT;
			m_secptr = 0;
			m_host->cd_w(0);
			m_host->io_w(0);
		}
		else // READ or VERIFY: issue the remainder of a >255-sector burst
		{
			m_chunk_sectors = (m_remaining > 255) ? 255 : m_remaining;
			m_hdc->write(2, m_chunk_sectors);
			// mark this as a command-pending gap, not a data phase, before reissuing:
			// otherwise C/D=0 and I/O=1 are still left over from the previous burst's
			// start_data_in(), which keeps DRQ3 asserted (see check_interrupt()) for the
			// entire WAIT_HDC gap and lets a stray DMA cycle land in ram_r() before this
			// burst's own start_data_in() re-arms it
			m_host->cd_w(1);
			issue_hdc_command(m_chunk_sectors > 1 ? 0x24 : 0x20);
		}
	}
	else
	{
		complete(m_error ? 1 : 0);
	}
}

void wd1015_device::complete(uint8_t error)
{
	m_error = error != 0;
	m_completion = (m_error ? 0x02 : 0x00) | (m_cdb[1] & 0x20); // bit1 = error, bit5 = echoed drive select
	enter_status_phase();
}

void wd1015_device::start_data_in(bool sense, int length)
{
	m_din_sense = sense;
	m_din_len = length;
	m_secptr = 0;
	m_phase = PHASE_DATA_IN;
	m_host->cd_w(0);
	m_host->io_w(1);
}

void wd1015_device::enter_status_phase()
{
	m_phase = PHASE_STATUS;
	m_host->cd_w(1);
	m_host->ireq_w(1); // also forces I_O=1 and fires IRQ5: the completion interrupt
}


//**************************************************************************
//  wd1015pl5402_device
//**************************************************************************

wd1015pl5402_device::wd1015pl5402_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd1015_device(mconfig, WD1015PL5402, tag, owner, clock)
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( wd1015pl5402 )
	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "wd1015-pl-54-02.u6", 0x000, 0x800, CRC(116e0608) SHA1(bcbd6b39c5a7e16e3bae9372b53d54d6761ba6bc) )
ROM_END

const tiny_rom_entry *wd1015pl5402_device::device_rom_region() const
{
	return ROM_NAME( wd1015pl5402 );
}

void wd1015pl5402_device::begin_command()
{
	m_cdb[0] &= 0xef; // this revision's decode ignores bit 4: 0x1x aliases 0x0x, 0xFx aliases 0xE0-0xE6
	wd1015_device::begin_command();
}

void wd1015pl5402_device::controller_diagnostic_extra()
{
	// $5F2: on success the firmware points RA at 0x200 (the sector data buffer) and
	// writes these two bytes there for the host to collect with READ DATA FROM SECTOR
	// BUFFER; reproduced verbatim even though nothing is known to read them back
	m_secbuf[0] = 0x2d;
	m_secbuf[1] = 0x54;
}
