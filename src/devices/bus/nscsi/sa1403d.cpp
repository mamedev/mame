// license:BSD-3-Clause
// copyright-holders:Dave Rand
/*********************************************************************

    Shugart SA1403D Winchester/Floppy Disk Controller

    Behavioral model of the microprogrammed SA1400-series controller
    (bit-slice engine + serdes/data-separator + single-sector buffer)
    at the SASI boundary, per the December 1980 OEM manual (39022-0):

    - up to four drives, any combination of SA1000-family rigid and
      floppy drives, addressed as LUN 0-3 (logical, not port, order);
    - class 0 commands: Test Drive Ready, Recalibrate, Request
      Syndrome, Request Sense, Format Drive, Check Track Format,
      Format Track, Format Bad Track, Read, Write, Seek;
    - class 1: Copy Blocks; class 6: Define Floppy Disk Track Format;
    - completion status byte = LUN<<5 | error<<1, then a 00 message;
    - 4-byte sense: [valid|type|code], LUN|adr2, adr1, adr0;
    - logical address = (cyl * heads + head) * spt + (sector - 1).

    Rigid section: served from per-LUN CHD images.  The host always
    addresses flat logical sectors, so only the bounds check and the
    32 x 256-byte track shape are geometry-visible.

    Floppy section: drive types SA800/SA850 (8") and SA400/SA450
    (5.25", later OEM firmware), served through an internal wd_fdc
    standing in for the board's microcode-driven serdes and data
    separator.  All transfers take the flux-level media path, so
    writes and formats are as real as reads; wrong-density accesses
    fail with the documented ID-address-mark errors, which is what
    the Xerox 820-II boot ROM's density probing relies on.  Track
    formats are the manual's figure 9: FM IBM 3740 (26 x 128) and
    MFM IBM System 34 (26 x 256), with cylinder 0 side 0 always FM
    in the double-density modes.

    Deviations from the real board, both invisible at the SASI
    boundary to known consumers: transfers are staged through a
    whole-transfer buffer rather than the single-sector buffer (the
    inter-sector REQ cadence differs), and READ/WRITE against
    unformatted media returns record-not-found instead of hanging
    until RST as the manual warns.

*********************************************************************/

#include "emu.h"
#include "bus/nscsi/sa1403d.h"

#include "formats/flopimg.h"

#define LOG_COMMAND (1U << 1)
#define LOG_DATA    (1U << 2)
#define LOG_FLOPPY  (1U << 3)

#define VERBOSE 0

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_SA1403D, nscsi_sa1403d_device, "nscsi_sa1403d", "Shugart SA1403D Winchester/Floppy Disk Controller")

// drive-type geometry, indexed by drive_type.  Rigid types use the fixed
// 32 x 256 track format; floppy spt varies with the per-LUN format code
// (26 sectors on 8" media; 5.25" values are the later-firmware extension).
// 8" floppies keep 26 sectors in both densities (manual figure 9).  The 5.25"
// types follow the same scheme scaled to the smaller track: FM 18 x 128,
// MFM 17 x 256 (the native Xerox 5.25" layout; provisional for SASI use --
// no later-firmware document has surfaced).
const nscsi_sa1403d_device::drive_geom nscsi_sa1403d_device::s_geom[7] = {
	{ "SA1002", 2, 256, 32, 32, false },
	{ "SA1004", 4, 256, 32, 32, false },
	{ "SA800",  1,  77, 26, 26, true  },
	{ "SA850",  2,  77, 26, 26, true  },
	{ "SA400",  1,  35, 18, 17, true  },
	{ "SA450",  2,  40, 18, 17, true  },
	{ "RIGID5", 4, 256, 32, 32, false },
};

nscsi_sa1403d_device::nscsi_sa1403d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_sa1403d_device(mconfig, NSCSI_SA1403D, tag, owner, clock)
{
}

nscsi_sa1403d_device::nscsi_sa1403d_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_full_device(mconfig, type, tag, owner, clock),
	m_hd(*this, "hd%u", 0U),
	m_fd(*this, "fd%u", 0U),
	m_fdc(*this, "fdc"),
	m_dip(*this, "DRIVETYPE")
{
	// default drive complement = the Xerox 820-II rigid-disk unit (also the
	// canonical Shugart demo config): three SA850 floppies + one SA1004, so
	// the machine exposes a single -hard image like the real single-drive
	// cabinet.  Machines with other complements use set_drive_type() (from a
	// subclass constructor: the option machine-config hook runs too late to
	// affect subdevice instantiation).
	m_cfg_type[0] = SA850;
	m_cfg_type[1] = SA850;
	m_cfg_type[2] = SA850;      // LUN 2 = CP/M drive D:; the connector defaults
	                            // empty below (Xerox units shipped two floppies)
	m_cfg_type[3] = SA1004;
}

static void sa1403d_floppies(device_slot_interface &device)
{
	device.option_add("sa400", FLOPPY_525_SSSD_35T);
	device.option_add("sa450", FLOPPY_525_DD);
	device.option_add("sa800", FLOPPY_8_SSDD);
	device.option_add("sa850", FLOPPY_8_DSDD);
}

void nscsi_sa1403d_device::device_add_mconfig(machine_config &config)
{
	// the internal FDC stands in for the board's serdes/data-separator
	// microcode; 2 MHz for 8" media, re-clocked to 1 MHz for 5.25"
	FD1793(config, m_fdc, 2'000'000);
	m_fdc->intrq_wr_callback().set(FUNC(nscsi_sa1403d_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(nscsi_sa1403d_device::fdc_drq_w));

	// per-LUN media, following the configured drive types (set_drive_type);
	// a LUN left to the DIP gets both a rigid image and a floppy connector,
	// with the Xerox 820-II rigid-disk unit complement as the floppy default
	static const char *const floppy_option[7] = { nullptr, nullptr, "sa800", "sa850", "sa400", "sa450", nullptr };
	for (int lun = 0; lun < 4; lun++) {
		const uint8_t t = m_cfg_type[lun];
		const bool rigid  = (t == TYPE_FROM_DIP) || !s_geom[t].floppy;
		const bool floppy = (t == TYPE_FROM_DIP) || s_geom[t].floppy;
		if (rigid)
			HARDDISK(config, m_hd[lun]).set_interface("scsi_hdd,hdd");
		if (floppy)
			FLOPPY_CONNECTOR(config, m_fd[lun], sa1403d_floppies,
					(lun < 2) ? ((t == TYPE_FROM_DIP) ? "sa850" : floppy_option[t]) : nullptr,
					floppy_image_device::default_mfm_floppy_formats);
	}
}

static INPUT_PORTS_START( sa1403d )
	// Drive-type selection DIP switch at board location 2H ("customer
	// firmware" PROM set AS30 semantics).  Switch pairs 8-7/6-5/4-3/2-1
	// select the drive type for LUN 0/1/2/3.  Defaults follow the Xerox
	// 820-II rigid-disk unit wiring: floppies on LUN 0-2, SA1004 on LUN 3.
	PORT_START("DRIVETYPE")
	PORT_DIPNAME( 0xc0, 0xc0, "LUN 0 Drive Type" ) PORT_DIPLOCATION("2H:8,7")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x40, "SA1004" )
	PORT_DIPSETTING(    0x80, "SA800" )
	PORT_DIPSETTING(    0xc0, "SA850" )
	PORT_DIPNAME( 0x30, 0x30, "LUN 1 Drive Type" ) PORT_DIPLOCATION("2H:6,5")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x10, "SA1004" )
	PORT_DIPSETTING(    0x20, "SA800" )
	PORT_DIPSETTING(    0x30, "SA850" )
	PORT_DIPNAME( 0x0c, 0x0c, "LUN 2 Drive Type" ) PORT_DIPLOCATION("2H:4,3")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x04, "SA1004" )
	PORT_DIPSETTING(    0x08, "SA800" )
	PORT_DIPSETTING(    0x0c, "SA850" )
	PORT_DIPNAME( 0x03, 0x01, "LUN 3 Drive Type" ) PORT_DIPLOCATION("2H:2,1")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x01, "SA1004" )
	PORT_DIPSETTING(    0x02, "SA800" )
	PORT_DIPSETTING(    0x03, "SA850" )
INPUT_PORTS_END

ioport_constructor nscsi_sa1403d_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sa1403d );
}

void nscsi_sa1403d_device::device_start()
{
	nscsi_full_device::device_start();

	save_item(NAME(m_lun_type));
	save_item(NAME(m_format_code));
	save_item(NAME(m_lun_cyl));
	save_item(NAME(m_lba));
	save_item(NAME(m_blocks));
	save_item(NAME(m_xfer_lun));
	save_item(NAME(m_cur_lba));
	save_item(NAME(m_block));
	save_item(NAME(m_f_write));
	save_item(NAME(m_f_lun));
	save_item(NAME(m_f_lba));
	save_item(NAME(m_f_left));
	save_item(NAME(m_f_pos));
	save_item(NAME(m_f_retry));
	save_item(NAME(m_f_noretry));
	save_item(NAME(m_f_cmd));
	save_item(NAME(m_f_interleave));
}

void nscsi_sa1403d_device::device_reset()
{
	nscsi_full_device::device_reset();

	// resolve per-LUN drive types: machine-config override, else the DIP
	const uint8_t dip = m_dip->read();
	for (int lun = 0; lun < 4; lun++) {
		if (m_cfg_type[lun] != TYPE_FROM_DIP)
			m_lun_type[lun] = m_cfg_type[lun];
		else
			m_lun_type[lun] = (dip >> (6 - 2 * lun)) & 3;

		// default floppy track format from the drive type (manual 5.1.3
		// NOTE): single density, single- or double-sided per the drive
		m_format_code[lun] = (s_geom[m_lun_type[lun]].heads > 1) ? 0x01 : 0x00;
		m_lun_cyl[lun] = 0xff;
	}

	m_lba = 0;
	m_blocks = 0;
	m_xfer_lun = 0;
	m_cur_lba = 0xffffffff;

	m_fop = fop::IDLE;
	m_f_write = false;
	m_f_lun = 0;
	m_f_lba = 0;
	m_f_left = 0;
	m_f_pos = 0;
	m_f_secstart = 0;
	m_f_retry = 0;
	m_f_noretry = false;
	m_f_cmd = 0;
	m_f_interleave = 1;

	m_fdc->set_floppy(nullptr);
}

void nscsi_sa1403d_device::device_reset_after_children()
{
	// The drive-select line doubles as motor-on; 8" spindles run off the AC
	// line anyway, so mounted media is up to speed from the start.  This must
	// run after the floppy devices reset (their reset turns the motor off
	// again), or 5.25" drives sit not-ready through their spin-up window.
	for (auto &con : m_fd) {
		floppy_image_device *fd = con ? con->get_device() : nullptr;
		if (fd)
			fd->mon_w(0);
	}
}

// a LUN is ready when its medium is present
bool nscsi_sa1403d_device::lun_ready(int lun)
{
	if (lun_is_floppy(lun)) {
		floppy_image_device *fd = m_fd[lun] ? m_fd[lun]->get_device() : nullptr;
		return fd && fd->exists();
	}
	return m_hd[lun] && m_hd[lun]->exists();
}

uint32_t nscsi_sa1403d_device::lun_max_lba(int lun) const
{
	const drive_geom &g = geom(lun);
	if (g.floppy) {
		// floppy capacity depends on the selected track format: sides and
		// density from the format code, sectors per track from the media size
		const int heads = (m_format_code[lun] & 0x01) ? 2 : 1;
		const int spt = (m_format_code[lun] & 0x06) ? g.spt_mfm : g.spt_fm;
		return uint32_t(g.cyls) * heads * spt - 1;
	}
	return uint32_t(g.cyls) * g.heads * g.spt_mfm - 1;
}

// completion status byte: 0 | LUN(2) | spare(3) | error | parity
void nscsi_sa1403d_device::sasi_status(int lun, bool error)
{
	scsi_status_complete(((lun & 3) << 5) | (error ? 0x02 : 0x00));
}

void nscsi_sa1403d_device::sasi_good(int lun)
{
	m_scsi_sense_buffer[0] = ERR_NONE;
	m_scsi_sense_buffer[1] = (lun & 3) << 5;
	m_scsi_sense_buffer[2] = 0;
	m_scsi_sense_buffer[3] = 0;
	sasi_status(lun, false);
}

// record a 4-byte SASI sense block and complete with the error bit set
void nscsi_sa1403d_device::sasi_error(int lun, uint8_t code, bool addr_valid, uint32_t lba)
{
	m_scsi_sense_buffer[0] = (addr_valid ? 0x80 : 0x00) | code;
	m_scsi_sense_buffer[1] = ((lun & 3) << 5) | ((lba >> 16) & 0x1f);
	m_scsi_sense_buffer[2] = (lba >> 8) & 0xff;
	m_scsi_sense_buffer[3] = lba & 0xff;
	sasi_status(lun, true);
}

// CDB lengths: class 0 and class 6 are 6 bytes, class 1 is 10 bytes
bool nscsi_sa1403d_device::scsi_command_done(uint8_t command, uint8_t length)
{
	if (!length)
		return false;
	switch (command >> 5) {
	case 0: return length == 6;
	case 1: return length == 10;
	case 6: return length == 6;
	default: return length == 6;    // rejected as invalid in dispatch
	}
}

//**************************************************************************
//  floppy section
//**************************************************************************

// decode a logical address into floppy CHS + the track's recording mode.
// In the double-density formats cylinder 0 side 0 stays FM 128-byte (the
// IBM-compatible system/boot track); everything else follows the format
// code.  Sector IDs are 1-based.
void nscsi_sa1403d_device::f_decode(int lun, uint32_t lba, int &cyl, int &head, int &sec, int &size, bool &fm) const
{
	const drive_geom &g = geom(lun);
	const uint8_t f = m_format_code[lun];
	const int heads = (f & 0x01) ? 2 : 1;
	const bool dd = (f & 0x06) != 0;    // 02/03 (manual) or 06/07 (Xerox firmware)
	const int spt = dd ? g.spt_mfm : g.spt_fm;
	cyl = lba / (spt * heads);
	head = (lba / spt) % heads;
	sec = lba % spt + 1;
	fm = !dd || (cyl == 0 && head == 0);
	size = fm ? 128 : 256;
}

// total bytes a transfer of [lba, lba+blocks) moves, accounting for the
// mixed 128/256-byte sector sizes of the double-density formats
uint32_t nscsi_sa1403d_device::f_xfer_bytes(int lun, uint32_t lba, uint32_t blocks) const
{
	uint32_t total = 0;
	for (uint32_t i = 0; i < blocks; i++) {
		int cyl, head, sec, size;
		bool fm;
		f_decode(lun, lba + i, cyl, head, sec, size, fm);
		total += size;
	}
	return total;
}

// route the selected drive to the FDC with the track's density and side
void nscsi_sa1403d_device::f_select(int cyl, int head, bool fm)
{
	floppy_image_device *fd = cur_fd();
	m_fdc->set_floppy(fd);
	m_fdc->set_unscaled_clock(fd->get_form_factor() == floppy_image::FF_8 ? 2'000'000 : 1'000'000);
	m_fdc->dden_w(fm ? 1 : 0);
	fd->mon_w(0);
	fd->ss_w(head);
}

// begin a deferred floppy operation; completion is reported from the FDC
// callback chain (the SASI phase queue stays empty until then)
void nscsi_sa1403d_device::f_start(uint8_t cmd, int lun, uint32_t lba, uint32_t blocks, bool write)
{
	m_f_cmd = cmd;
	m_f_lun = lun;
	m_f_lba = lba;
	m_f_left = blocks;
	m_f_write = write;
	m_f_retry = 0;

	int cyl, head, sec, size;
	bool fm;
	f_decode(lun, lba, cyl, head, sec, size, fm);

	LOGMASKED(LOG_FLOPPY, "f_start cmd %02x lun %d lba %u blocks %u chs %d/%d/%d %s\n",
			cmd, lun, lba, blocks, cyl, head, sec, fm ? "FM" : "MFM");

	f_select(cyl, head, fm);

	// teach the FDC where this drive's head actually is, then position
	if (m_lun_cyl[lun] == 0xff) {
		m_fop = fop::RESTORE;
		m_fdc->cmd_w(0x00);     // RESTORE, unload head, no verify, slowest rate
	} else {
		m_fdc->track_w(m_lun_cyl[lun]);
		f_issue_seek(cyl);
	}
}

void nscsi_sa1403d_device::f_issue_seek(int cyl)
{
	int tcyl, head, sec, size;
	bool fm;
	f_decode(m_f_lun, m_f_lba, tcyl, head, sec, size, fm);

	if (m_fdc->track_r() == cyl) {
		// already there: go straight to the data phase of the operation
		m_fop = fop::RW;
		if (m_f_cmd == 0x06 || m_f_cmd == 0x07 || m_f_cmd == 0x04)
			f_issue_wtrack();
		else if (m_f_cmd == 0x01)
			f_finish(ERR_NONE);     // recalibrate completes on positioning
		else
			f_issue_rw();
		return;
	}

	m_fop = fop::SEEK;
	m_fdc->data_w(cyl);
	m_fdc->cmd_w(0x10);             // SEEK, no verify
}

// issue the next sector read/write at the current position
void nscsi_sa1403d_device::f_issue_rw()
{
	int cyl, head, sec, size;
	bool fm;
	f_decode(m_f_lun, m_f_lba, cyl, head, sec, size, fm);

	f_select(cyl, head, fm);
	m_fop = fop::RW;
	m_f_secstart = m_f_pos;
	m_fdc->sector_w(sec);
	m_fdc->cmd_w(m_f_write ? 0xa0 : 0x80);  // WRITE/READ SECTOR, single
}

// compose and write one formatted track (manual figure 9 layouts, 6C data
// fill, sector interleave per the CDB code, bad-track flag in the head byte)
void nscsi_sa1403d_device::f_issue_wtrack()
{
	int cyl, head, sec, size;
	bool fm;
	f_decode(m_f_lun, m_f_lba, cyl, head, sec, size, fm);

	f_select(cyl, head, fm);
	f_compose_track(cyl, head, fm, m_f_cmd == 0x07);
	m_f_pos = 0;
	m_fop = fop::WTRACK;
	m_fdc->cmd_w(0xf0);             // WRITE TRACK
}

void nscsi_sa1403d_device::f_compose_track(int cyl, int head, bool fm, bool bad)
{
	const drive_geom &g = geom(m_f_lun);
	const int spt = (m_format_code[m_f_lun] & 0x06) ? g.spt_mfm : g.spt_fm;
	const uint8_t hd = uint8_t(head) | (bad ? 0x80 : 0x00);

	// interleave: logical sectors placed every <code> physical slots,
	// advancing on collision (code 1 = sequential, IBM compatible)
	const int il = std::clamp<int>(m_f_interleave, 1, 16);
	std::vector<uint8_t> ids(spt, 0);
	int slot = 0;
	for (int l = 1; l <= spt; l++) {
		while (ids[slot])
			slot = (slot + 1) % spt;
		ids[slot] = l;
		slot = (slot + il) % spt;
	}

	std::vector<uint8_t> &t = m_xferbuf;
	t.clear();
	auto fill = [&t](int n, uint8_t v) { t.insert(t.end(), n, v); };

	if (fm) {
		// FM, IBM 3740: 26 x 128
		fill(40, 0xff); fill(6, 0x00); t.push_back(0xfc);   // index mark
		fill(26, 0xff);                                     // gap 1
		for (int s = 0; s < spt; s++) {
			fill(6, 0x00);
			t.push_back(0xfe);                              // ID address mark
			t.push_back(cyl); t.push_back(hd); t.push_back(ids[s]); t.push_back(0x00);
			t.push_back(0xf7);                              // CRC
			fill(11, 0xff); fill(6, 0x00);                  // gap 2
			t.push_back(0xfb);                              // data address mark
			fill(128, 0x6c);
			t.push_back(0xf7);                              // CRC
			fill(27, 0xff);                                 // gap 3
		}
		fill(247, 0xff);                                    // gap 4
	} else {
		// MFM, IBM System 34: 26 x 256
		fill(80, 0x4e); fill(12, 0x00);
		fill(3, 0xf6); t.push_back(0xfc);                   // index mark (C2 C2 C2 FC)
		fill(50, 0x4e);                                     // gap 1
		for (int s = 0; s < spt; s++) {
			fill(12, 0x00);
			fill(3, 0xf5); t.push_back(0xfe);               // A1 A1 A1 FE
			t.push_back(cyl); t.push_back(hd); t.push_back(ids[s]); t.push_back(0x01);
			t.push_back(0xf7);
			fill(22, 0x4e); fill(12, 0x00);                 // gap 2
			fill(3, 0xf5); t.push_back(0xfb);               // A1 A1 A1 FB
			fill(256, 0x6c);
			t.push_back(0xf7);
			fill(54, 0x4e);                                 // gap 3
		}
		fill(598, 0x4e);                                    // gap 4
	}
}

// complete the deferred floppy operation: push the queued-up SASI phases
void nscsi_sa1403d_device::f_finish(uint8_t err)
{
	LOGMASKED(LOG_FLOPPY, "f_finish err %02x\n", err);
	m_fop = fop::IDLE;
	if (err != ERR_NONE) {
		sasi_error(m_f_lun, err, true, m_f_lba);
		return;
	}
	if (m_f_cmd == 0x08)            // READ: data follows, then status
		scsi_data_in(3, m_xferbuf.size());
	sasi_good(m_f_lun);
}

void nscsi_sa1403d_device::fdc_drq_w(int state)
{
	if (!state)
		return;

	if (m_fop == fop::RW && !m_f_write) {
		if (m_f_pos < m_xferbuf.size())
			m_xferbuf[m_f_pos++] = m_fdc->data_r();
		else
			m_fdc->data_r();        // overrun guard: sink the byte
	} else if (m_fop == fop::RW || m_fop == fop::WTRACK) {
		if (m_f_pos < m_xferbuf.size())
			m_fdc->data_w(m_xferbuf[m_f_pos++]);
		else
			m_fdc->data_w(0x4e);    // underrun guard: pad
	}
}

void nscsi_sa1403d_device::fdc_intrq_w(int state)
{
	if (!state || m_fop == fop::IDLE)
		return;

	const uint8_t st = m_fdc->status_r();

	switch (m_fop) {
	case fop::RESTORE:
		if (st & 0x04) {            // track 0 reached
			m_lun_cyl[m_f_lun] = 0;
			int cyl, head, sec, size;
			bool fm;
			f_decode(m_f_lun, m_f_lba, cyl, head, sec, size, fm);
			f_issue_seek(cyl);
		} else
			f_finish(ERR_NO_TRACK00);
		break;

	case fop::SEEK: {
		int cyl, head, sec, size;
		bool fm;
		f_decode(m_f_lun, m_f_lba, cyl, head, sec, size, fm);
		m_lun_cyl[m_f_lun] = m_fdc->track_r();
		if (m_f_cmd == 0x01) {      // Recalibrate/seek-only commands end here
			f_finish(ERR_NONE);
			break;
		}
		m_fop = fop::RW;
		if (m_f_cmd == 0x06 || m_f_cmd == 0x07 || m_f_cmd == 0x04)
			f_issue_wtrack();
		else
			f_issue_rw();
		break;
	}

	case fop::RW: {
		// sector completion: check the type II status bits
		uint8_t err = ERR_NONE;
		if (st & 0x80)
			err = ERR_NOT_READY;
		else if (st & 0x40 && m_f_write)
			err = ERR_WRITE_PROTECTED;
		else if (st & 0x10)
			err = ERR_RECORD_NOT_FOUND;
		else if (st & 0x08)
			err = m_f_write ? ERR_WRITE_FAULT : ERR_DATA_UNCORR;
		else if (st & 0x04)
			err = ERR_DMA_TIMEOUT;  // lost data

		if (err != ERR_NONE) {
			// the controller retries three times unless disabled (5.2.1)
			if (!m_f_noretry && ++m_f_retry < 3) {
				m_f_pos = m_f_secstart;     // rewind the partial sector
				f_issue_rw();
				break;
			}
			f_finish(err);
			break;
		}

		m_f_retry = 0;
		m_f_lba++;
		if (--m_f_left) {
			// automatic head and cylinder switching (manual 1.1)
			int cyl, head, sec, size;
			bool fm;
			f_decode(m_f_lun, m_f_lba, cyl, head, sec, size, fm);
			if (cyl != m_lun_cyl[m_f_lun])
				f_issue_seek(cyl);
			else
				f_issue_rw();
		} else
			f_finish(ERR_NONE);
		break;
	}

	case fop::WTRACK: {
		if (st & 0x40) {
			f_finish(ERR_WRITE_PROTECTED);
			break;
		}
		if (st & 0x04) {
			f_finish(ERR_FORMAT_ERROR); // lost data while formatting
			break;
		}
		// Format Drive walks every remaining track; Format Track/Bad Track
		// do just the one
		if (m_f_cmd == 0x04 && --m_f_left) {
			const drive_geom &g = geom(m_f_lun);
			m_f_lba += (m_format_code[m_f_lun] & 0x06) ? g.spt_mfm : g.spt_fm;  // next track
			int cyl, head, sec, size;
			bool fm;
			f_decode(m_f_lun, m_f_lba, cyl, head, sec, size, fm);
			if (cyl != m_lun_cyl[m_f_lun])
				f_issue_seek(cyl);
			else
				f_issue_wtrack();
		} else
			f_finish(ERR_NONE);
		break;
	}

	default:
		break;
	}
}

//**************************************************************************
//  SASI command dispatch
//**************************************************************************

void nscsi_sa1403d_device::scsi_command()
{
	const int lun = m_scsi_cmdbuf[1] >> 5;
	m_lba = ((m_scsi_cmdbuf[1] & 0x1f) << 16) | (m_scsi_cmdbuf[2] << 8) | m_scsi_cmdbuf[3];
	m_blocks = m_scsi_cmdbuf[4] ? m_scsi_cmdbuf[4] : 256;
	m_f_noretry = bool(m_scsi_cmdbuf[5] & 0x80);
	m_f_interleave = m_scsi_cmdbuf[4] ? m_scsi_cmdbuf[4] : 1;

	// the CDB LUN field is three bits but the controller serves four drives
	if (lun > 3) {
		sasi_error(lun & 3, ERR_NOT_SELECTED);
		return;
	}
	m_xfer_lun = lun;

	LOGMASKED(LOG_COMMAND, "command %02x lun %d lba %u blocks %u (%s)\n",
			m_scsi_cmdbuf[0], lun, m_lba, m_blocks, geom(lun).name);

	switch (m_scsi_cmdbuf[0]) {
	case 0x00:  // Test Drive Ready
		if (lun_ready(lun))
			sasi_good(lun);
		else
			sasi_error(lun, ERR_NOT_READY);
		break;

	case 0x01:  // Recalibrate
		if (!lun_ready(lun)) {
			sasi_error(lun, ERR_NOT_READY);
			break;
		}
		if (lun_is_floppy(lun)) {
			// physically restore to track 0 (deferred)
			m_lun_cyl[lun] = 0xff;
			f_start(0x01, lun, 0, 0, false);
			break;
		}
		sasi_good(lun);
		break;

	case 0x02:  // Request Syndrome (rigid ECC; no media errors from CHD)
		if (lun_is_floppy(lun)) {
			sasi_error(lun, ERR_ILLEGAL_FUNCTION);
			break;
		}
		m_block[0] = 0;
		m_block[1] = 0;
		scsi_data_in(2, 2);
		sasi_status(lun, false);
		break;

	case 0x03:  // Request Sense: 4-byte drive/controller sense block
		scsi_data_in(SBUF_SENSE, 4);
		sasi_status(lun, false);
		break;

	case 0x04:  // Format Drive: all tracks from the given address
	case 0x06:  // Format Track
	case 0x07:  // Format Bad Track (bad-block flag set in the ID head bytes)
	{
		if (!lun_ready(lun)) {
			sasi_error(lun, ERR_NOT_READY);
			break;
		}
		if (m_lba > lun_max_lba(lun)) {
			sasi_error(lun, ERR_ILLEGAL_ADDRESS, true, m_lba);
			break;
		}
		const drive_geom &g = geom(lun);
		if (g.floppy) {
			floppy_image_device *fd = m_fd[lun]->get_device();
			if (fd->wpt_r()) {
				sasi_error(lun, ERR_WRITE_PROTECTED);
				break;
			}
			// deferred: count is the number of tracks to lay down
			const uint32_t fspt = (m_format_code[lun] & 0x06) ? g.spt_mfm : g.spt_fm;
			const uint32_t tracks = (m_scsi_cmdbuf[0] == 0x04)
					? (lun_max_lba(lun) + 1 - (m_lba / fspt) * fspt) / fspt : 1;
			f_start(m_scsi_cmdbuf[0], lun, (m_lba / fspt) * fspt, tracks, true);
			break;
		}
		// rigid: 6C-fill the addressed track(s); bad-track flags have no
		// CHD representation
		uint32_t first = m_lba;
		uint32_t last = (m_scsi_cmdbuf[0] == 0x04) ? lun_max_lba(lun)
				: (first / g.spt_mfm) * g.spt_mfm + g.spt_mfm - 1;
		memset(m_block, 0x6c, sizeof(m_block));
		bool fail = false;
		for (uint32_t lba = first; lba <= last && !fail; lba++)
			fail = !m_hd[lun]->write(lba, m_block);
		if (fail)
			sasi_error(lun, ERR_FORMAT_ERROR, true, m_lba);
		else
			sasi_good(lun);
		break;
	}

	case 0x05:  // Check Track Format
		// floppy: a full ID/interleave verification needs a read-track pass;
		// presence of the track is what consumers check (TODO: interleave)
		if (!lun_ready(lun))
			sasi_error(lun, ERR_NOT_READY);
		else if (m_lba > lun_max_lba(lun))
			sasi_error(lun, ERR_ILLEGAL_ADDRESS, true, m_lba);
		else
			sasi_good(lun);
		break;

	case 0x08:  // Read
		if (!lun_ready(lun)) {
			sasi_error(lun, ERR_NOT_READY);
			break;
		}
		if (m_lba + m_blocks - 1 > lun_max_lba(lun)) {
			sasi_error(lun, ERR_ILLEGAL_ADDRESS, true, m_lba);
			break;
		}
		if (lun_is_floppy(lun)) {
			// deferred: data-in + status are pushed when the media pass ends
			m_xferbuf.assign(f_xfer_bytes(lun, m_lba, m_blocks), 0);
			m_f_pos = 0;
			f_start(0x08, lun, m_lba, m_blocks, false);
			break;
		}
		m_cur_lba = 0xffffffff;
		scsi_data_in(2, m_blocks * 256);
		sasi_status(lun, false);
		break;

	case 0x0a:  // Write
		if (!lun_ready(lun)) {
			sasi_error(lun, ERR_NOT_READY);
			break;
		}
		if (m_lba + m_blocks - 1 > lun_max_lba(lun)) {
			sasi_error(lun, ERR_ILLEGAL_ADDRESS, true, m_lba);
			break;
		}
		if (lun_is_floppy(lun)) {
			floppy_image_device *fd = m_fd[lun]->get_device();
			if (fd->wpt_r()) {
				sasi_error(lun, ERR_WRITE_PROTECTED);
				break;
			}
			// queue only the data phase; the media pass runs when the last
			// byte lands (scsi_put_data) and pushes the status itself, so
			// the completion reflects the real write outcome
			m_xferbuf.assign(f_xfer_bytes(lun, m_lba, m_blocks), 0);
			m_f_pos = 0;
			scsi_data_out(3, m_xferbuf.size());
			break;
		}
		if (m_hd[lun]->is_readonly()) {
			sasi_error(lun, ERR_WRITE_FAULT);
			break;
		}
		scsi_data_out(2, m_blocks * 256);
		sasi_status(lun, false);
		break;

	case 0x0b:  // Seek (overlapped: completion returns immediately)
		if (!lun_ready(lun))
			sasi_error(lun, ERR_NOT_READY);
		else if (m_lba > lun_max_lba(lun))
			sasi_error(lun, ERR_ILLEGAL_ADDRESS, true, m_lba);
		else
			sasi_good(lun);
		break;

	case 0x20:  // class 1: Copy Blocks (source LUN/LBA -> destination LUN/LBA)
	{
		const int slun = (m_scsi_cmdbuf[1] >> 5) & 3;
		const int dlun = (m_scsi_cmdbuf[5] >> 5) & 3;
		const uint32_t slba = ((m_scsi_cmdbuf[1] & 0x1f) << 16) | (m_scsi_cmdbuf[2] << 8) | m_scsi_cmdbuf[3];
		const uint32_t dlba = ((m_scsi_cmdbuf[5] & 0x1f) << 16) | (m_scsi_cmdbuf[6] << 8) | m_scsi_cmdbuf[7];
		const uint32_t n = m_scsi_cmdbuf[4] ? m_scsi_cmdbuf[4] : 256;

		if (lun_is_floppy(slun) || lun_is_floppy(dlun)) {
			// TODO: floppy copy needs a chained deferred read+write pass
			sasi_error(lun_is_floppy(slun) ? slun : dlun, ERR_ILLEGAL_FUNCTION);
			break;
		}
		if (!lun_ready(slun) || !lun_ready(dlun)) {
			sasi_error(lun_ready(slun) ? dlun : slun, ERR_NOT_READY);
			break;
		}
		if (slba + n - 1 > lun_max_lba(slun)) {
			sasi_error(slun, ERR_ILLEGAL_ADDRESS, true, slba);
			break;
		}
		if (dlba + n - 1 > lun_max_lba(dlun)) {
			sasi_error(dlun, ERR_ILLEGAL_ADDRESS, true, dlba);
			break;
		}
		bool fail = false;
		for (uint32_t i = 0; i < n && !fail; i++) {
			fail = !m_hd[slun]->read(slba + i, m_block);
			if (!fail)
				fail = !m_hd[dlun]->write(dlba + i, m_block);
		}
		if (fail)
			sasi_error(slun, ERR_DATA_UNCORR, true, slba);
		else
			sasi_good(slun);
		break;
	}

	case 0xc0:  // class 6: Define Floppy Disk Track Format
		LOGMASKED(LOG_COMMAND, "define-format lun %d code %02x\n", lun, m_scsi_cmdbuf[5]);
		if (!lun_is_floppy(lun)) {
			sasi_error(lun, ERR_ILLEGAL_FUNCTION);
			break;
		}
		// codes 00-03 per the Dec-80 manual; later firmware (the unit the
		// Xerox 820-II v5.0 driver targets, fm.* in sa1403.mac) adds a
		// sector-size bit: 06/07 = double density, 256-byte, SS/DS
		if (m_scsi_cmdbuf[5] > 0x03 && m_scsi_cmdbuf[5] != 0x06 && m_scsi_cmdbuf[5] != 0x07) {
			sasi_error(lun, ERR_INVALID_COMMAND);
			break;
		}
		m_format_code[lun] = m_scsi_cmdbuf[5];
		sasi_good(lun);
		break;

	default:
		LOGMASKED(LOG_COMMAND, "unknown command %02x\n", m_scsi_cmdbuf[0]);
		sasi_error(lun, ERR_INVALID_COMMAND);
		break;
	}
}

uint8_t nscsi_sa1403d_device::scsi_get_data(int id, int pos)
{
	if (id == 3)    // floppy transfer buffer
		return (uint32_t(pos) < m_xferbuf.size()) ? m_xferbuf[pos] : 0;

	if (id != 2)
		return nscsi_full_device::scsi_get_data(id, pos);

	if (m_scsi_cmdbuf[0] == 0x02)   // Request Syndrome streams from m_block
		return m_block[pos];

	const uint32_t clba = m_lba + pos / 256;
	if (clba != m_cur_lba) {
		m_cur_lba = clba;
		if (!m_hd[m_xfer_lun]->read(clba, m_block)) {
			LOG("read error lun %d lba %u\n", m_xfer_lun, clba);
			memset(m_block, 0, sizeof(m_block));
		}
	}
	return m_block[pos & 255];
}

void nscsi_sa1403d_device::scsi_put_data(int id, int pos, uint8_t data)
{
	if (id == 3) {  // floppy transfer buffer
		if (uint32_t(pos) < m_xferbuf.size())
			m_xferbuf[pos] = data;
		// last byte of the data-out phase: run the deferred media write
		if (uint32_t(pos) == m_xferbuf.size() - 1) {
			m_f_pos = 0;
			f_start(0x0a, m_xfer_lun, m_lba, m_blocks, true);
		}
		return;
	}

	if (id != 2) {
		nscsi_full_device::scsi_put_data(id, pos, data);
		return;
	}

	m_block[pos & 255] = data;
	if ((pos & 255) == 255) {
		const uint32_t clba = m_lba + pos / 256;
		if (!m_hd[m_xfer_lun]->write(clba, m_block))
			LOG("write error lun %d lba %u\n", m_xfer_lun, clba);
	}
}

// Byte transfer pacing: the microcoded engine moves roughly a byte per
// microsecond through the sector buffer (the 5 Mb/s serdes is not the
// bottleneck; the REQ/ACK handshake and microcode loop are).
attotime nscsi_sa1403d_device::scsi_data_byte_period()
{
	return attotime::from_nsec(1000);
}
