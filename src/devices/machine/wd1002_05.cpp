// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Western Digital WD1002-05 Winchester/Floppy Disk Controller board

    See wd1002_05.h.  Extends the WD1002-HD0 (WD1010 rigid + sector buffer)
    with the floppy section: a WD2797 reached through the same host task file,
    selected by the SDH register (drive-select bits 3-4 == 11 -> floppy).  The
    floppy data path shares the board sector buffer with the rigid path.

***************************************************************************/

#include "emu.h"
#include "wd1002_05.h"


DEFINE_DEVICE_TYPE(WD1002_05, wd1002_05_device, "wd1002_05", "Western Digital WD1002-05 Winchester/Floppy Disk Controller")


namespace {

// SDH register: the WD1015 selects the floppy section when drive-select bits 3-4
// are both set; bit 1 then picks floppy drive 0/1 and bit 0 the side.
enum : uint8_t {
	SDH_FLOPPY   = 0x18, // bits 3-4 == 11 -> route to the WD2797
	SDH_DRIVE    = 0x02, // floppy drive select (0x18 = drive 0, 0x1a = drive 1)
	SDH_SIDE     = 0x01  // head/side select
};

// host task-file commands (WD1010 opcodes) the front-end routes to the WD2797
enum : uint8_t {
	HOST_READ    = 0x20,
	HOST_WRITE   = 0x30
};

// host status byte the WD1015 front-end presents at register 7
enum : uint8_t {
	HSTAT_ERR    = 0x01,
	HSTAT_SC     = 0x10, // seek complete
	HSTAT_RDY    = 0x40, // drive ready
	HSTAT_BSY    = 0x80,
	HSTAT_FLOPPY_READY = HSTAT_RDY | HSTAT_SC // idle floppy: ready + seek complete
};

constexpr uint8_t HERR_NOT_FOUND = 0x10; // host error register: sector not found

// WD2797 commands the front-end issues (read/write carry the E head-load settle)
enum : uint8_t {
	FDC_RESTORE  = 0x00,
	FDC_SEEK     = 0x10,
	FDC_READ     = 0x84,
	FDC_WRITE    = 0xa4
};

constexpr uint8_t WDFDC_NOT_FOUND = 0x10; // WD2797 status: record not found

} // anonymous namespace


static void wd1002_05_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD); // 5.25" DS DD (the EM-II 16/8 media)
}


wd1002_05_device::wd1002_05_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd1002_hd0_device(mconfig, WD1002_05, tag, owner, clock)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fd%u", 0U)
	, m_secno(0), m_cyllo(0), m_cylhi(0), m_sdh(0)
	, m_fop(FOP_IDLE)
	, m_f_write(false)
	, m_f_wr_pending(false)
	, m_f_cyl(0), m_f_head(0), m_f_sec(0), m_f_drive(0)
	, m_floppy_cyl{ 0xff, 0xff }
	, m_f_err(0)
{
}


void wd1002_05_device::device_add_mconfig(machine_config &config)
{
	wd1002_hd0_device::device_add_mconfig(config); // the WD1010 rigid + the board sector buffer

	WD2797(config, m_fdc, 1'000'000); // board oscillator: 1 MHz = 250 kbps MFM for 5.25" DD
	m_fdc->intrq_wr_callback().set(FUNC(wd1002_05_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(wd1002_05_device::fdc_drq_w));
	m_fdc->set_force_ready(true); // 5.25" drives supply no READY line; the board ties it active
	// two 5.25" drives (A: and B:), as fitted in the EM-II case
	FLOPPY_CONNECTOR(config, m_floppy[0], wd1002_05_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], wd1002_05_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
}

void wd1002_05_device::device_start()
{
	wd1002_hd0_device::device_start();

	save_item(NAME(m_secno));
	save_item(NAME(m_cyllo));
	save_item(NAME(m_cylhi));
	save_item(NAME(m_sdh));
	save_item(NAME(m_fop));
	save_item(NAME(m_f_write));
	save_item(NAME(m_f_wr_pending));
	save_item(NAME(m_f_cyl));
	save_item(NAME(m_f_head));
	save_item(NAME(m_f_sec));
	save_item(NAME(m_f_drive));
	save_item(NAME(m_floppy_cyl));
	save_item(NAME(m_f_err));
}

void wd1002_05_device::device_reset()
{
	wd1002_hd0_device::device_reset();
	m_fop = FOP_IDLE;
	m_f_wr_pending = false;
	m_floppy_cyl[0] = m_floppy_cyl[1] = 0xff;
}


uint8_t wd1002_05_device::read(offs_t offset)
{
	bool const floppy = (m_sdh & SDH_FLOPPY) == SDH_FLOPPY;
	if ((offset & 7) == 7) // status register
	{
		if (m_fop != FOP_IDLE)
			return HSTAT_BSY; // BSY while a floppy op runs
		if (floppy)
			return HSTAT_FLOPPY_READY | (m_f_err ? HSTAT_ERR : 0); // WD1015 presents the floppy status
		// rigid: fall through to the real WD1010 status
	}
	if ((offset & 7) == 1 && floppy)
		return m_f_err; // floppy error register (WD1015 front-end, not the WD1010's)
	return wd1002_hd0_device::read(offset); // buffer (0) + rigid task file / status
}

void wd1002_05_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
	case 0: // data port
		if (m_f_wr_pending) // a floppy write is buffering host data into the shared buffer
		{
			if (m_ptr < BUFFER_SIZE)
				m_buf[m_ptr++] = data;
			if (m_ptr >= BUFFER_SIZE) // a full sector has arrived -> hand it to the WD2797
			{
				m_f_wr_pending = false;
				f_start(true);
			}
			return;
		}
		break; // rigid: the base buffers it
	case 3: m_secno = data; break; // shadow the task file for the floppy path,
	case 4: m_cyllo = data; break; // and fall through so the WD1010 gets it too
	case 5: m_cylhi = data; break;
	case 6: m_sdh   = data; break;
	case 7: // command
	{
		uint8_t const cmd = data & 0xf0;
		bool const floppy = (m_sdh & SDH_FLOPPY) == SDH_FLOPPY;
		if (floppy)
		{
			m_f_cyl   = m_cyllo | (uint16_t(m_cylhi) << 8);
			m_f_head  = m_sdh & SDH_SIDE;          // bit0 = side
			m_f_drive = (m_sdh & SDH_DRIVE) ? 1 : 0; // bit1 = drive 0/1
			m_f_sec   = m_secno;
			m_ptr     = 0;
			if (cmd == HOST_READ)            // READ: start the WD2797 now; host polls then reads buf
				f_start(false);
			else if (cmd == HOST_WRITE)      // WRITE: buffer the host data first, then start the FDC
				m_f_wr_pending = true;
			// restore/seek/diag: nothing to do (status already not-BSY)
			return;
		}
		break; // rigid: the base drives the WD1010
	}
	}
	wd1002_hd0_device::write(offset, data);
}


//-------------------------------------------------
//  floppy section: drive the WD2797 through seek + read/write, completing via
//  its interrupt/DRQ callbacks; BSY (status read) holds the host off meanwhile
//-------------------------------------------------

void wd1002_05_device::f_select(int drive, int head)
{
	floppy_image_device *const fd = m_floppy[drive]->get_device();
	m_fdc->set_floppy(fd);
	m_fdc->dden_w(0); // DDEN low = double density (MFM); the EM-II boot disk is MFM throughout
	if (fd)
	{
		fd->mon_w(0);
		fd->ss_w(head);
	}
}

void wd1002_05_device::f_start(bool write)
{
	m_f_write = write;
	f_select(m_f_drive, m_f_head);

	if (m_floppy_cyl[m_f_drive] == 0xff) // this drive's head position unknown: recalibrate first
	{
		m_fop = FOP_RESTORE;
		m_fdc->cmd_w(FDC_RESTORE);
	}
	else
	{
		m_fdc->track_w(m_floppy_cyl[m_f_drive]); // prime the WD2797 with the selected drive's position
		f_issue_seek();
	}
}

void wd1002_05_device::f_issue_seek()
{
	if (m_fdc->track_r() == m_f_cyl)
	{
		f_issue_rw();
		return;
	}
	m_fop = FOP_SEEK;
	m_fdc->data_w(m_f_cyl);
	m_fdc->cmd_w(FDC_SEEK);
}

void wd1002_05_device::f_issue_rw()
{
	m_fop = FOP_RW;
	m_ptr = 0; // transfer starts at the head of the shared buffer
	m_fdc->sector_w(m_f_sec);
	m_fdc->cmd_w(m_f_write ? FDC_WRITE : FDC_READ); // single sector, E=1 (head-load settle)
}

void wd1002_05_device::fdc_drq_w(int state)
{
	if (!state || m_fop != FOP_RW)
		return;

	if (!m_f_write) // read: WD2797 -> shared buffer
	{
		if (m_ptr < BUFFER_SIZE)
			m_buf[m_ptr++] = m_fdc->data_r();
		else
			m_fdc->data_r();
	}
	else            // write: shared buffer -> WD2797
	{
		if (m_ptr < BUFFER_SIZE)
			m_fdc->data_w(m_buf[m_ptr++]);
		else
			m_fdc->data_w(0x00);
	}
}

void wd1002_05_device::fdc_intrq_w(int state)
{
	if (!state || m_fop == FOP_IDLE)
		return;

	switch (m_fop)
	{
	case FOP_RESTORE:
		m_floppy_cyl[m_f_drive] = 0;
		f_issue_seek();
		break;

	case FOP_SEEK:
		m_floppy_cyl[m_f_drive] = m_fdc->track_r();
		f_issue_rw();
		break;

	case FOP_RW:
		// reflect a WD2797 record-not-found failure to the host as an error
		m_f_err = (m_fdc->status_r() & WDFDC_NOT_FOUND) ? HERR_NOT_FOUND : 0;
		m_ptr = 0;            // rewind so the host reads the buffer from the start
		m_fop = FOP_IDLE;    // done: BSY clears
		break;

	default:
		m_fop = FOP_IDLE;
		break;
	}
}
