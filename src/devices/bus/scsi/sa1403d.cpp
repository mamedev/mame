// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Shugart SA1403D Winchester Disk Controller emulation

**********************************************************************/

#include "emu.h"
#include "sa1403d.h"

#include "formats/flopimg.h"

namespace {
// The bitstream/sector decode helpers are protected statics of floppy_image_format_t;
// expose them via a trivial subclass so the SA1403D can decode sectors at HLE level.
class flopdec : public floppy_image_format_t
{
public:
	static std::vector<bool> bitstream(int t, int h, int cs, const floppy_image &img)
	{ return generate_bitstream_from_track(t, h, cs, img); }
	static std::vector<std::vector<uint8_t>> mfm(const std::vector<bool> &b)
	{ return extract_sectors_from_bitstream_mfm_pc(b); }
	static std::vector<std::vector<uint8_t>> fm(const std::vector<bool> &b)
	{ return extract_sectors_from_bitstream_fm_pc(b); }
	// pure-virtual stubs (never used -- this format is never registered)
	int identify(util::random_read &, uint32_t, const std::vector<uint32_t> &) const override { return 0; }
	bool load(util::random_read &, uint32_t, const std::vector<uint32_t> &, floppy_image &) const override { return false; }
	const char *name() const noexcept override { return "sa1403d_dec"; }
	const char *description() const noexcept override { return "SA1403D decode helper"; }
	const char *extensions() const noexcept override { return ""; }
};
} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SA1403D, sa1403d_device, "sa1403d", "Shugart SA1403D")


// The real SA1403D carries its controller firmware in four 4K PROMs (labelled
// I-IV, with AS30/AS31 production revisions and a 12668 diagnostic set).  This
// device is a high-level emulation -- the firmware is reimplemented in C++ and
// the PROMs are never executed -- so no ROM region is declared; none of those
// PROMs has been dumped, and declaring them only as NO_DUMP would report them
// perpetually missing for a controller that is in fact fully functional here.


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sa1403d_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, "image");
}


//-------------------------------------------------
//  INPUT_PORTS( sa1403d )
//-------------------------------------------------

INPUT_PORTS_START( sa1403d )
	PORT_INCLUDE(scsihle)

	// Drive-type select DIP, "Location 2H" (SA1403D OEM manual sec 9.2): two bits per
	// LUN select the drive served there -- 00=SA1002, 01=SA1004 (rigid Winchester),
	// 10=SA800, 11=SA850 (8" floppy).  The controller's AS30 firmware reads this to
	// configure per-LUN behaviour.  The Xerox 820-II wiring (9R80758 manual) puts the
	// floppies on LUN 0-2 and the rigid disk on LUN 3, so default to floppy/floppy/
	// floppy/rigid.  (NOTE: the HLE does not yet consume this -- feeding it back to the
	// firmware/host is WIP.)
	PORT_START("2H")
	PORT_DIPNAME( 0xc0, 0xc0, "LUN 0 Drive Type" ) PORT_DIPLOCATION("2H:7,8")
	PORT_DIPSETTING(    0x00, "SA1002" ) // 2 heads, 256 cylinders
	PORT_DIPSETTING(    0x40, "SA1004" ) // 4 heads, 256 cylinders
	PORT_DIPSETTING(    0x80, "SA800" ) // 1 head, 77 cylinders
	PORT_DIPSETTING(    0xc0, "SA850" ) // 2 heads, 77 cylinders
	PORT_DIPNAME( 0x30, 0x30, "LUN 1 Drive Type" ) PORT_DIPLOCATION("2H:5,6")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x10, "SA1004" )
	PORT_DIPSETTING(    0x20, "SA800" )
	PORT_DIPSETTING(    0x30, "SA850" )
	PORT_DIPNAME( 0x0c, 0x0c, "LUN 2 Drive Type" ) PORT_DIPLOCATION("2H:3,4")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x04, "SA1004" )
	PORT_DIPSETTING(    0x08, "SA800" )
	PORT_DIPSETTING(    0x0c, "SA850" )
	PORT_DIPNAME( 0x03, 0x01, "LUN 3 Drive Type" ) PORT_DIPLOCATION("2H:1,2")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x01, "SA1004" )
	PORT_DIPSETTING(    0x02, "SA800" )
	PORT_DIPSETTING(    0x03, "SA850" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sa1403d_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sa1403d );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sa1403d_device - constructor
//-------------------------------------------------

sa1403d_device::sa1403d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: scsihd_device(mconfig, SA1403D, tag, owner, clock)
{
}

// The controller answers SELECT if any drive is present: a mounted floppy on any LUN,
// or the scsihd hard-disk image (e.g. an SA1004 CHD on LUN 0).
bool sa1403d_device::exists() const
{
	for (auto *f : m_floppy)
		if (f != nullptr && f->exists())
			return true;
	return scsihd_device::exists();
}

// Decode one physical track of the mounted floppy into sectors indexed by sector ID.
// 820-II 5.25" media is 250 kbps: track 0 is FM (cell 4000 ns), data tracks are MFM
// (cell 2000 ns). Probe MFM first, then FM, and keep whichever yields sectors.
bool sa1403d_device::decode_track(int cyl, int head, track_cache &tc)
{
	if (tc.valid) return true;
	floppy_image_device *fd = cur_floppy();
	if (!fd || !fd->exists()) return false;
	floppy_image *img = fd->get_image();
	if (img == nullptr) return false;

	int img_tracks = 0, img_heads = 0;
	img->get_actual_geometry(img_tracks, img_heads);
	if (cyl < 0 || cyl >= img_tracks || head < 0 || head >= img_heads)
		return false;

	// Probe both 8" (500 kbps: MFM~1200ns, FM~2400ns) and 5.25" (250 kbps: MFM~2000ns,
	// FM~4000ns) cell sizes; keep whichever yields sectors. 8" is the faithful media.
	struct probe { bool mfm; int cell; } const probes[] = {
		{ false, 2400 }, { true, 1200 }, { false, 4000 }, { true, 2000 } };
	for (auto const &p : probes)
	{
		auto bs = flopdec::bitstream(cyl, head, p.cell, *img);
		auto secs = p.mfm ? flopdec::mfm(bs) : flopdec::fm(bs);
		int count = 0, size = 0;
		for (auto const &s : secs) if (!s.empty()) { count++; size = s.size(); }
		if (count > 0)
		{
			tc.valid = true; tc.is_mfm = p.mfm; tc.sector_size = size; tc.by_id = std::move(secs);
			m_spt = count; // detected sectors/track (17 for 5.25 MFM, 26 for 8" FM, ...)
			return true;
		}
	}
	return false;
}

// Map a SASI logical block to a physical (cyl,sector) and return its bytes.
// The SA1403D presents the floppy as 32 logical sectors/track (Dave-confirmed; the
// source of the boot's stride-0x20 CDB addresses), so lba/32 is a contiguous sector
// index.  The CP/M system image is much larger than one sector, so the boot reads
// FULL TRACKS: consecutive index -> consecutive physical sectors of a track.  The
// bootable system lives on the reserved tracks starting at cyl1 (cyl0 holds only the
// loader stub at sec1; cyl0 sec1 == cyl1 sec1, both the EB0105 loader), 17 sectors/track.
bool sa1403d_device::read_logical_block(uint32_t lba, std::vector<uint8_t> &out)
{
	// Determine the selected drive's geometry from a reference data track (sets m_spt:
	// 17 for 5.25" MFM, 26 for 8" FM/MFM, ...).
	{ track_cache geo; decode_track(1, 0, geo); }
	// Two regions in the SA1403D logical address:
	//  - byte2=0 (LBA 0-255): the reserved/system tracks -> cyl1.. (cold boot loads the
	//    system here; e.g. LBA 22 -> cyl2 sec6 = the BIOS jump table).
	//  - byte2>=1 (LBA 256+): the CP/M data area -> cyl3.. (off=3 reserved tracks), so the
	//    directory (CP/M block 0) at LBA 256 maps to cyl3 sec1.
	// SA1403D logical address (manual 5.5): lba = (cyl*HDCYL + head)*SETRK + sec, all
	// 0-based.  Invert to the physical (cyl, head, sector) and serve that sector directly
	// -- no off/region fudge, no synthetic directory.  SETRK = the drive's physical
	// sectors/track (m_spt); HDCYL = the media's recorded head count.
	const int spt = m_spt > 0 ? m_spt : 26;
	const int heads = m_sel_ds ? 2 : 1;   // selected format's heads (see ExecCommand note)
	const int cyl  = lba / (heads * spt);
	const int head = (lba / spt) % heads;
	const int sec  = (lba % spt) + 1;
	track_cache tc;
	if (!decode_track(cyl, head, tc)) return false;
	if (sec >= int(tc.by_id.size()) || tc.by_id[sec].empty())
		return false;
	out = tc.by_id[sec];
	return true;
}

void sa1403d_device::ExecCommand()
{
	// Select the drive addressed by the CDB LUN (the floppies sit on LUN 0-2 and the
	// rigid disk on LUN 3; a LUN with no floppy falls through to the scsihd image).
	m_lun = command[1] >> 5;
	m_sector_bytes = 256;
	int read_cyl = -1, read_mfm = -1;   // for the density check below
	floppy_image_device *fd = cur_floppy();
	if (command[0] == 0x08 && fd) // floppy READ: transfer size = the addressed track's sector size
	{
		uint32_t lba = ((command[1] & 0x1f) << 16) | (command[2] << 8) | command[3];
		{ track_cache geo; decode_track(1, 0, geo); }   // m_spt for this drive
		const int spt = m_spt > 0 ? m_spt : 26;
		// Heads come from the SELECTED format (C0 sides bit), not the media geometry: a
		// double-sided probe of a single-sided disk must address head 1 (which decode_track
		// then fails) so the detector rejects DS and lands on the right single-sided format.
		const int heads = m_sel_ds ? 2 : 1;
		read_cyl = lba / (heads * spt);
		track_cache tc;
		if (decode_track(read_cyl, (lba / spt) % heads, tc) && tc.sector_size)
		{
			m_sector_bytes = tc.sector_size;
			read_mfm = tc.is_mfm ? 1 : 0;
		}
	}
	switch( command[ 0 ] )
	{
	case 0xC0: // Class 6: Define Floppy Disk Track Format -- record the selected density.
		// byte5 = format code (00/01=SD, 06/07=DD; bit 2 = double density).  The ROM's
		// media-format detector (smfs/cdd) sets a density then READs track 2 and accepts
		// the format only if the read succeeds -- so a read at the wrong density must FAIL
		// (the real controller RNFs on density mismatch).  Record it; the READ checks it.
		m_sel_dd = (command[5] & 0x04) != 0;
		m_sel_ds = (command[5] & 0x01) != 0;
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;
	case 0x08: // floppy/rigid READ
		// Density gate (floppy only): track 0 is always single density ("track zero
		// single density" in the DPB) so it reads in any mode; on tracks >= 1 the selected
		// density must match the media or the controller returns Record-Not-Found.  This
		// is what lets the detector distinguish SD from DD (FM read of an MFM track fails).
		if (fd && read_cyl >= 1 && (read_mfm < 0 || (read_mfm != 0) != m_sel_dd))
		{
			// read_mfm < 0 = the addressed track/head isn't present (e.g. side 1 of a
			// single-sided disk -> a double-sided probe must fail there); otherwise the
			// selected density doesn't match the media.  Either way the real controller
			// returns Record-Not-Found, which is how the detector rejects wrong formats.
			m_phase = SCSI_PHASE_STATUS;
			m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
			m_transfer_length = 0;
			break;
		}
		scsihd_device::ExecCommand();
		break;
	case 0x06: // Format Track (SASI class-0): the Rigid Disk Initialization utility (FMT)
	{
		// formats the rigid one track at a time (1024 tracks = 256 cyl x 4 heads, the CDB
		// logical address steps by the 32 sectors/track).  On the CHD there are no physical
		// marks to write -- just fill the track's sectors with the CP/M empty pattern (0xE5)
		// so the freshly-formatted volume reads back as an empty directory.  Floppy LUNs are
		// served from their read-only image and are never formatted here.
		uint32_t lba = ((command[1] & 0x1f) << 16) | (command[2] << 8) | command[3];
		if (cur_floppy() == nullptr && m_image && m_image->exists())
		{
			std::vector<uint8_t> blank(m_sector_bytes > 0 ? m_sector_bytes : 256, 0xe5);
			for (int s = 0; s < 32; s++)
				m_image->write(lba + s, blank.data());
		}
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;
	}
	default:
		scsihd_device::ExecCommand();
		break;
	}
}

void sa1403d_device::ReadData( uint8_t *data, int dataLength )
{
	// LUNs with no floppy (e.g. the SA1004 hard disk on LUN0) read from the scsihd image.
	if (cur_floppy() == nullptr)
	{
		scsihd_device::ReadData(data, dataLength);
		return;
	}

	// Serve real floppy sector data for the current logical block (advances per call).
	std::vector<uint8_t> blk;
	if (read_logical_block(m_lba, blk))
	{
		int n = std::min<int>(dataLength, int(blk.size()));
		memset(data, 0, dataLength);
		memcpy(data, blk.data(), n);
	}
	else
	{
		memset(data, 0, dataLength);
	}
	m_lba++;
}

void sa1403d_device::WriteData( uint8_t *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	default:
		scsihd_device::WriteData( data, dataLength );
		break;
	}
}
