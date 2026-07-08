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
	HOST_WRITE   = 0x30,
	HOST_FORMAT  = 0x50  // format a track (INIT's c.format); WD1015 builds the track from the skew
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
	FDC_RESTORE     = 0x00,
	FDC_SEEK        = 0x10,
	FDC_READ        = 0x8c, // read sector: E settle + bit3 (L) selects 128<<N sector length, as real 16/8 media uses
	FDC_WRITE       = 0xac, // write sector: E settle + bit3 (L)
	FDC_WRITE_TRACK = 0xf4  // format the whole track (E head-load settle)
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
	, m_secno(0), m_cyllo(0), m_cylhi(0), m_sdh(0), m_secnt(0)
	, m_fop(FOP_IDLE)
	, m_f_write(false)
	, m_f_format(false)
	, m_f_wr_pending(false)
	, m_f_fmt_pending(false)
	, m_f_cyl(0), m_f_head(0), m_f_sec(0), m_f_drive(0)
	, m_floppy_cyl{ 0xff, 0xff }
	, m_f_err(0)
	, m_fmt_buf{ }
	, m_fmt_len(0), m_fmt_ptr(0)
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
	save_item(NAME(m_secnt));
	save_item(NAME(m_fop));
	save_item(NAME(m_f_write));
	save_item(NAME(m_f_format));
	save_item(NAME(m_f_wr_pending));
	save_item(NAME(m_f_fmt_pending));
	save_item(NAME(m_f_cyl));
	save_item(NAME(m_f_head));
	save_item(NAME(m_f_sec));
	save_item(NAME(m_f_drive));
	save_item(NAME(m_floppy_cyl));
	save_item(NAME(m_f_err));
	save_item(NAME(m_fmt_buf));
	save_item(NAME(m_fmt_len));
	save_item(NAME(m_fmt_ptr));
}

void wd1002_05_device::device_reset()
{
	wd1002_hd0_device::device_reset();
	m_fop = FOP_IDLE;
	m_f_write = false;
	m_f_format = false;
	m_f_wr_pending = false;
	m_f_fmt_pending = false;
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
	if ((offset & 7) == 6 && floppy)
		return m_sdh; // present the shadowed SDH: the WD1010 base masks the drive-select bits,
		              // but INIT reads the SDH back to add side/size before issuing FORMAT
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
			if (m_ptr >= floppy_seclen()) // a full selected-size sector has arrived -> hand it to the WD2797
			{
				m_f_wr_pending = false;
				f_start(true, false);
			}
			return;
		}
		if (m_f_fmt_pending) // a floppy format is streaming its skew table (2 bytes per sector)
		{
			if (m_ptr < BUFFER_SIZE)
				m_buf[m_ptr++] = data;
			if (m_ptr >= uint32_t(m_secnt) * 2) // whole skew table in -> build + write the track
			{
				m_f_fmt_pending = false;
				f_start(true, true);
			}
			return;
		}
		break; // rigid: the base buffers it
	case 2: m_secnt = data; break; // shadow sector count (skew-table length for the floppy format)
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
			m_f_err   = 0; // a fresh command clears the prior error -- the WD1015 status is per-command
			                // (the EM ROM's SEEK-then-READ probe reads the error via a shared WAIT/RSTAT,
			                //  so a stale RNF from a failed probe must not carry into the next command)
			if (cmd == HOST_READ)            // READ: start the WD2797 now; host polls then reads buf
				f_start(false, false);
			else if (cmd == HOST_WRITE)      // WRITE: buffer the host data first, then start the FDC
				m_f_wr_pending = true;
			else if (cmd == HOST_FORMAT)     // FORMAT: collect the skew table, then WRITE TRACK
				m_f_fmt_pending = true;
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

uint32_t wd1002_05_device::floppy_seclen() const
{
	static constexpr uint16_t k[4] = { 256, 512, 1024, 128 }; // SDH bits 6-5 select bytes/sector
	return k[(m_sdh >> 5) & 3];
}

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

void wd1002_05_device::f_start(bool write, bool format)
{
	m_f_write = write;
	m_f_format = format;
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
		f_issue_transfer();
		return;
	}
	m_fop = FOP_SEEK;
	m_fdc->data_w(m_f_cyl);
	m_fdc->cmd_w(FDC_SEEK);
}

void wd1002_05_device::f_issue_transfer()
{
	if (m_f_format)
		f_issue_format();
	else
		f_issue_rw();
}

void wd1002_05_device::f_issue_rw()
{
	m_fop = FOP_RW;
	m_ptr = 0; // transfer starts at the head of the shared buffer
	m_fdc->sector_w(m_f_sec);
	// the WD2797 drives its side-select output (and thus the drive's active side)
	// from command bit 1, so the head must be encoded there -- a preceding ss_w()
	// is overridden when the read/write command latches.
	m_fdc->cmd_w((m_f_write ? FDC_WRITE : FDC_READ) | (m_f_head ? 0x02 : 0)); // single sector, E=1 (head-load settle)
}

void wd1002_05_device::f_issue_format()
{
	m_fop = FOP_FORMAT;
	build_format_track();  // lay out the whole track image; the WD2797 pulls it byte-by-byte via DRQ
	m_fmt_ptr = 0;
	m_fdc->cmd_w(FDC_WRITE_TRACK | (m_f_head ? 0x02 : 0)); // SSO from bit 1, as for read/write
}

//-------------------------------------------------
//  build a standard IBM System-34 MFM track image for the WD2797 WRITE TRACK command.
//  the physical sector order is the skew table INIT streamed (entry i = [flag, sector],
//  2 bytes each); cylinder/head/size/count come from the shadowed task file.  the WD2797
//  interprets 0xf5->A1, 0xf6->C2, 0xf7->two CRC bytes and writes everything else verbatim.
//-------------------------------------------------

void wd1002_05_device::build_format_track()
{
	int const spt = m_secnt;
	int const szval = (m_sdh >> 5) & 3;              // SDH bits 5-6 select bytes per sector
	int const szcode = (szval == 3) ? 0 : szval + 1; // 00=256, 01=512, 10=1024, 11=128 -> N field
	int const seclen = 128 << szcode;

	m_fmt_len = 0;
	auto const put = [this] (uint8_t b, int n) { while (n-- > 0 && m_fmt_len < m_fmt_buf.size()) m_fmt_buf[m_fmt_len++] = b; };

	put(0x4e, 80);                                   // gap 4a
	put(0x00, 12); put(0xf6, 3); put(0xfc, 1);       // index address mark
	put(0x4e, 50);                                   // gap 1
	for (int i = 0; i < spt; i++)
	{
		uint8_t const sec = m_buf[(i * 2 + 1) & (BUFFER_SIZE - 1)]; // sector number from the skew entry
		put(0x00, 12); put(0xf5, 3); put(0xfe, 1);   // ID address mark
		put(uint8_t(m_f_cyl), 1); put(uint8_t(m_f_head), 1); put(sec, 1); put(uint8_t(szcode), 1); // C,H,R,N (standard IBM N = 128<<N, matching real 16/8 media; read/write use bit3 to decode it)
		put(0xf7, 1);                                // ID CRC
		put(0x4e, 22);                               // gap 2
		put(0x00, 12); put(0xf5, 3); put(0xfb, 1);   // data address mark
		put(0xe5, seclen);                           // data field (formatted fill)
		put(0xf7, 1);                                // data CRC
		put(0x4e, 54);                               // gap 3
	}
	// gap 4b: fdc_drq_w streams 0x4e past m_fmt_len until the WD2797 hits the index and stops
}

void wd1002_05_device::fdc_drq_w(int state)
{
	if (!state)
		return;

	if (m_fop == FOP_FORMAT) // format: stream the built track image, then 0x4e gap to the index
	{
		m_fdc->data_w(m_fmt_ptr < m_fmt_len ? m_fmt_buf[m_fmt_ptr++] : 0x4e);
		return;
	}
	if (m_fop != FOP_RW)
		return;

	uint32_t const seclen = floppy_seclen();
	if (!m_f_write) // read: WD2797 -> shared buffer
	{
		if (m_ptr < seclen)
			m_buf[m_ptr++] = m_fdc->data_r();
		else
			m_fdc->data_r();
	}
	else            // write: shared buffer -> WD2797
	{
		if (m_ptr < seclen)
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
		f_issue_transfer();
		break;

	case FOP_RW:
		// Reflect a WD2797 record-not-found to the host.  A size mismatch must NOT fault: the EM
		// ROM's geometry probe (DETSPT) reads a 512-byte sector at 256, escalating on the sector-
		// NUMBER not-found (sector 17 absent on a 9-sector disk), so a short read has to succeed
		// and simply hand back the selected size.
		m_f_err = (m_fdc->status_r() & WDFDC_NOT_FOUND) ? HERR_NOT_FOUND : 0;
		m_ptr = 0;            // rewind so the host reads the buffer from the start
		m_fop = FOP_IDLE;    // done: BSY clears
		break;

	case FOP_FORMAT:
		// the only WRITE TRACK failure the host tests for is write protection (status bit 6)
		m_f_err = (m_fdc->status_r() & 0x40) ? HERR_NOT_FOUND : 0;
		m_fop = FOP_IDLE;    // done: BSY clears
		break;

	default:
		m_fop = FOP_IDLE;
		break;
	}
}
