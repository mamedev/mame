// license:BSD-3-Clause
// copyright-holders:Michael Zapf

/*
    The hfe_format class implements the HFE format that is used for the Lotharek
    floppy emulator.

    The overall structure of the format is:

    +-------------------------+     0000
    |         Header          |
    +-------------------------+     0200
    |        Track LUT        |
    +-------------------------+     0400
    |     Track 0 side 0/1    |
    +-------------------------+
    |     Track 1 side 0/1    |
    +-------------------------+
    ...                     ...
    |     Track n-1 side 0/1  |
    +-------------------------+

    Header (0x0000 - 0x01FF, 512 bytes)
    -------------------------------------------------------------------
    00   HEADERSIGNATURE[8]      fixed             "HXCPICFE"
    08   formatrevision          fixed             00
    09   number_of_track         drive             28/50 (typ)
    0a   number_of_side          fixed             02
    0b   track_encoding          preset/detected   00/02 (typ)
    0c   sampleRate              preset            00FA (250), little endian
    0e   floppyRPM               fixed             0000 (not used)
    10   floppyinterfacemode     preset            07 (typically, default)
    11   dnu                     fixed             00
    12   track_list_offset       fixed             0001, little endian
    14   write_allowed           implied           ff (true)
    15   single_step             preset            ff (true)
    16   track0s0_altencoding    preset            ff (no alternative enc.)
    17   track0s0_encoding       preset            ff (no alternative enc.)
    18   track0s1_altencoding    preset            ff (no alternative enc.)
    19   track0s1_encoding       preset            ff (no alternative enc.)
    1a - ff                      fixed             ff

    The fixed entries are used for all systems.

    The number of tracks can be derived from the disk drive.

    The track encoding must be preset; when ISOIBM_MFM is set, the actual
    encoding (FM or MFM) is determined from track 1. The values are defined
    in the header file as encoding_t.

    The sample rate can be preset and is then fixed; when 0, the rate is
    automatically set according to the density.

    The floppy interface mode is meaningless for MAME but probably needed for
    the actual HxC floppy emulator. It must be preset and is currently set
    to "generic shugart". PC drives with HD or ED need special handling. The
    values are defined in the header file as floppymode_t.

    Image files may be set to read-only. In MAME this can be achieved by
    mounting the image as read-only. Hence, this flag is ignored and set to
    true on write.

    Single stepping determines whether the drive has to move the R/W head by
    one (single) or two (double) steps to switch from track n to track n+1.
    This is currently ignored and set to single stepping.

    Copy protections may apply different encodings for track 0 on either or
    both sides. Currently, this is only supported for ISOIBM_(M)FM and only
    as switching between both. The flags on position 0x16 and 0x18 determine
    whether the fields (0x17 and 0x19) contain alternative encodings (0xff=false,
    0x00=true).

    Track offset lookup table (at 0x0200)
    -------------------------------------

    typedef struct pictrack_
    {
        uint16_t offset;     // Offset of the track data in blocks of 512 bytes (Ex: 2=0x400)
        uint16_t track_len;  // Length of the track data in byte.
    } pictrack;

    This table has a size of number_of_track*4 bytes.

    Track data
    ----------

    (first possible occurrence at 0x0400)

    Each track is encoded in a sequence of cell levels which are represented
    by bits in the data.

     +--------+--------+--------+--------+---- ........ ---+--------+--------+
     | Head 0 | Head 1 | Head 0 | Head 1 | Hea ........  1 | Head 0 | Head 1 |
     +--------+--------+--------+--------+---- ........ ---+--------+--------+
     |     Block 0     |     Block 1     |                 |    Block n-1    |

    Each block (Head 0 + Head 1) is 0x200 bytes long, with 0x100 bytes for
    each head. Block n-1 may be partially filled, e.g. with 64 bytes for
    head 0 and 64 bytes for head 1. The contents for head 1 in block n-1
    start at offxet 0x100 nevertheless:

     +--------+--------+
     |]]]]  0 |]]]]  1 |
     +--------+--------+
     |    Block n-1    |


    Each byte in the track data is a sequence of cell sample levels
    according to the sample rate. Bit order is little endian:

     Bits
     7 6 5 4 3 2 1 0   15 14 13 12 11 10 9 8   23 22 21 20 19 18 17 16

     0-bits indicate no change, 1-bits indicate flux level change.

    To encode the byte 0x4e in MFM at 250 kbit/s, the following data bytes
    are used:

     Byte:          0  1  0  0  1  1  1  0
     MFM encoding: 10 01 00 10 01 01 01 00
     Reversed order: 0010 1010 0100 1001     = 2a 49

    Interestingly, FM-encoded disks are usually sampled at 250 kbit/s,
    like MFM, although FM only delivers 125 kbit/s. This oversampling leads
    to zero bits (no change) every two positions. See below for details.

    ------------------

    HFE currently does not support weakly magnetized or defect zones.

    ------------------

    Note: Many of the format header fields seem to serve documentary purpose
    only, which makes it difficult to preserve them in MAME. Since the image is
    completely rewritten and there is no proper way to preserve a state from
    loading (which may have changed by reformatting anyway), the user of the
    format must provide these entries by a specialized subclass of this format.

    Michael Zapf, 2025
*/

#include "hxchfe_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include "osdcore.h" // osd_printf_*

#include <tuple>

#define HFE_FORMAT_HEADER   "HXCPICFE"

#define HEADER_LENGTH 512

// The track table should not have more than no_of_tracks * 4 bytes length,
// and floppy disks typically have less than 128 tracks.
#define MAX_LUT_LENGTH 512

/*
    Constructor of the format with presets.

    These values are used to populate the header of the format with data that are
    relevant for the HxC floppy emulator itself, but cannot easily (or at all)
    be derived during runtime.

    Subclasses of hfe_format may override these presets (e.g. using
    AMIGA_MFM_ENCODING or other sample rates).

    The ISOIBM_(M)FM_ENCODINGs are the only ones that may change during runtime
    by reformatting. For that purpose, track 1 is analyzed for cell sizes.
    Unless the disk image is specially prepared, this should quickly find good
    evidence for either FM or MFM.  Track 0 may have a different encoding in
    the HFE format (e.g. for copy protection).

    If the sample rate is preset to 0, a sample rate of 250 K/s (FM, MFM with
    SD/DD) or 500 (MFM with HD) is assumed.
*/
hfe_format::hfe_format() : floppy_image_format_t(),
	m_standard_track_count(true),
	m_samplerate(0),
	m_rpm(300),
	m_write_allowed(true),
	m_single_step(true),
	m_floppymode(GENERIC_SHUGART_DD_FLOPPYMODE),
	m_encoding(ISOIBM_MFM_ENCODING)
{
}

const char *hfe_format::name() const noexcept
{
	return "hfe";
}

const char *hfe_format::description() const noexcept
{
	return "HxC Floppy Emulator HFE File format";
}

const char *hfe_format::extensions() const noexcept
{
	return "hfe";
}

bool hfe_format::supports_save() const noexcept
{
	return true;
}

int hfe_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[8];
	auto const [err, actual] = read_at(io, 0, &header, sizeof(header));
	if (err || (sizeof(header) != actual)) {
		return 0;
	}
	if (!memcmp(header, HFE_FORMAT_HEADER, 8)) {
		return FIFID_SIGN;
	}
	return 0;
}

bool hfe_format::eval_header(header_info& header, uint8_t* headerbytes, int drive_cylinders) const
{
	// Format revision must be 0
	if (headerbytes[8] != 0)
	{
		osd_printf_error("hxchfe: Invalid format revision. Expected 0, got %d.\n", headerbytes[8]);
		return false;
	}

	header.cylinders = headerbytes[9] & 0xff;
	header.heads = headerbytes[10] & 0xff;

	if (header.cylinders == 0)
	{
		osd_printf_warning("hxchfe: No header found; image not loadable.\n");
		return false;
	}

	if (drive_cylinders < header.cylinders)
	{
		if (header.cylinders > drive_cylinders + DUMP_THRESHOLD)
		{
			osd_printf_error("hxchfe: Floppy disk has too many tracks for this drive (floppy tracks=%d, drive tracks=%d).\n", header.cylinders, drive_cylinders);
			return false;
		}
		else
		{
			// Some dumps has a few excess tracks to be safe,
			// lets be nice and just skip those tracks
			osd_printf_warning("hxchfe: Floppy disk has a slight excess of tracks for this drive that will be discarded (floppy tracks=%d, drive tracks=%d).\n", header.cylinders, drive_cylinders);
			header.cylinders = drive_cylinders;
		}
	}

	// Do we have double stepping (two drive steps for a single track step)?
	if (header.cylinders <= drive_cylinders/2)
	{
		osd_printf_error("hxchfe: Double stepping not supported (floppy tracks=%d, drive tracks=%d).\n", header.cylinders, drive_cylinders);
		return false;
	}

	header.track_encoding = encoding_t(headerbytes[11] & 0xff);

	if (header.track_encoding > EMU_FM_ENCODING)
	{
		osd_printf_error("hxchfe: Unsupported track encoding 0x%02x.\n", header.track_encoding);
		// Don't exit on a property that currently has only documentary value
		// return false;
	}

	header.sample_rate = get_u16le(&headerbytes[12]);

	if ((header.sample_rate > 500) || (header.sample_rate < 125))
	{
		osd_printf_error("hxchfe: Unsupported sample rate %d.\n", header.sample_rate);
		return false;
	}

	header.interface_mode = (floppymode_t)(headerbytes[16] & 0xff);

	if (header.interface_mode != DISABLE_FLOPPYMODE)
	{
		if ((header.interface_mode < IBMPC_DD_FLOPPYMODE) ||
			(header.interface_mode > S950_HD_FLOPPYMODE))
		{
			osd_printf_error("hxchfe: Unsupported interface mode 0x%02x.\n", header.interface_mode);
			return false;
		}
	}

	header.track_list_offset = get_u16le(&headerbytes[18]) * 512;

	// These settings are (currently) meaningless inside MAME, so we
	// could also skip them
	header.write_allowed = (headerbytes[20] != 0);
	header.single_step = (headerbytes[21] != 0);
	header.track0s0_has_altencoding = (headerbytes[22] == 0x00);
	header.track0s0_encoding = (encoding_t)(headerbytes[23] & 0xff);
	header.track0s1_has_altencoding = (headerbytes[24] == 0x00);
	header.track0s1_encoding = (encoding_t)(headerbytes[25] & 0xff);

	return true;
}

bool hfe_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	std::error_condition err;
	size_t actual;

	int drive_cylinders = 0;
	int drive_heads = 0;

	header_info header;
	std::vector<lut_entry> tracklut;

	// Get the geometry of the drive (for 5.25" this is usually 42 tracks,
	// for 3.5" it is 84 tracks)
	image.get_maximal_geometry(drive_cylinders, drive_heads);

	// Single-sided images are OK, but the drives must support two heads
	if (drive_heads < 2)
	{
		osd_printf_error("hxchfe: Single-sided drives are not supported.\n");
		return false;
	}

	uint8_t headerbytes[HEADER_LENGTH];   // reserve some space for the header
	std::tie(err, actual) = read_at(io, 0, headerbytes, HEADER_LENGTH);  // FIXME: check for errors and premature EOF

	// Fill the header entries. The header.cylinders field may be lowered to
	// the value of drive_cylinders
	if (eval_header(header, headerbytes, drive_cylinders)==false)
		return false;

	// Load the track table
	uint8_t track_list[MAX_LUT_LENGTH];   // Check constant
	std::tie(err, actual) = read_at(io, header.track_list_offset, track_list, MAX_LUT_LENGTH); // FIXME: check for errors and premature EOF

	for (int i=0; i < header.cylinders; i++)
	{
		// Offset in units of 512 bytes
		lut_entry track(get_u16le(&track_list[4*i]) * 512, get_u16le(&track_list[4*i+2]));
		tracklut.push_back(track);
	}

	// Now load the tracks
	std::vector<uint8_t> cylinder_buffer;
	int samplelength = 500000 / header.sample_rate;

	for (int cyl=0; cyl < header.cylinders; cyl++)
	{
		// actual data read
		// The HFE format defines an interleave of the two sides per cylinder at every 256 bytes
		cylinder_buffer.resize(tracklut[cyl].length);
		std::tie(err, actual) = read_at(io, tracklut[cyl].offset, &cylinder_buffer[0], tracklut[cyl].length); // FIXME: check for errors and premature EOF

		// Even when the image is defined as single-sided, it allocates space
		// for the second head, so we may as well load the cells for it. If
		// it remains unchanged in the drive, it will simply be written back.
		generate_track_from_hfe_bitstream(cyl, 0, samplelength, &cylinder_buffer[0], tracklut[cyl].length, image);
		generate_track_from_hfe_bitstream(cyl, 1, samplelength, &cylinder_buffer[0], tracklut[cyl].length, image);
	}

	// Find variant
	// See below: HFE uses 250 kBit for FM also, creating oversampling
	// We have to rely on the encoding field.
	if (header.track_encoding == ISOIBM_FM_ENCODING || header.track_encoding == EMU_FM_ENCODING)
	{
		// FM is for single density
		image.set_variant((header.heads==1)? floppy_image::SSSD : floppy_image::DSSD);
	}
	else
	{
		if (header.sample_rate < 300)
			image.set_variant((header.heads==1)? floppy_image::SSDD : floppy_image::DSDD);
		// no single-sided variants from here on
		else if (header.sample_rate < 600)
			image.set_variant(floppy_image::DSHD);
		else
			image.set_variant(floppy_image::DSED);
	}
	return true;
}

void hfe_format::generate_track_from_hfe_bitstream(int cyl, int head, int samplelength, const uint8_t *trackbuf, int track_end, floppy_image &image) const
{
	// HFE has a few peculiarities:
	//
	// - The track images do not always sum up to 200 ms but may be slightly shorter.
	//   We may assume that the last byte (last 8 samples) are part of the end
	//   gap, so it should not harm to repeat it until there are enough samples.
	//
	// - The track image switches between both heads every 0x100 bytes.
	//   The last block of the first side is padded to a 0x100 multiple, but
	//   the last block of the second side is not.
	//   Even when the image is set as single-sided, space is reserved
	//   for the second head. Thus, we will read both sides in any case, so the
	//   unused side will be correctly written back later.
	//
	// - Tracks are sampled at 250 K/s for both FM and MFM, which yields
	//   50000 data bits (1 sample per cell) for MFM, while FM is twice
	//   oversampled (2 samples per cell).
	//   Accordingly, for both FM and MFM, we have 100000 samples, and the
	//   images are equally long for both recordings.
	//
	// - The oversampled FM images of HFE start with a cell 0 (no change),
	//   where a 1 would be expected for 125 K/s.
	//   In order to make an oversampled image look like a normally sampled one,
	//   we position the transition at 500 ns before the cell end.
	//   The HFE format has a 1 us minimum cell size; this means that a normally
	//   sampled FM image with 11111... at the begining means
	//
	//   125 kbit/s:    1   1   1   1   1...
	//   250 kbit/s:   01  01  01  01  01...
	//   500 kbit/s: 00010001000100010001...
	//
	//   -500             3500            7500            11500
	//     +-|---:---|---:-+ |   :   |   : +-|---:---|---:-+ |
	//     | |   :   |   : | |   :   |   : | |   :   |   : | |
	//     | |   :   |   : +-|---:---|---:-+ |   :   |   : +-|
	//  -500 0      2000    4000    6000    8000   10000   12000
	//
	//  3500 (1)     samplelength - 500
	//  7500 (1)     +samplelength
	// 11500 (1)     +samplelength
	// 15500 (1)     +samplelength
	//
	//  Double samples
	//
	//  1500 (0)    samplelength - 500
	//  3500 (1)    +samplelength
	//  5500 (0)    +samplelength
	//  7500 (1)    +samplelength
	//  9500 (0)    +samplelength
	// 11500 (1)    +samplelength
	//
	// - Subtracks are not supported

	std::vector<uint32_t> &dest = image.get_buffer(cyl, head);
	dest.clear();

	int offset = (head==0)? 0 : 0x100;

	// Tracks are assumed to be equally long for both sides, and to switch
	// sides every 0x100 bytes. So when the final block is c bytes long, and
	// it is padded to the next 0x100 multiple on the first side,
	// we skip back by c + (0x100-c) bytes, i.e. by 0x100
	if (head == 0) track_end -= 0x100;

	uint8_t curcells = 0;
	int timepos = -500;

	// We are creating a sequence of timestamps with flux information
	// As explained above, we arrange for the flux change to occur in the last
	// quarter of a cell

	long cyltime = (long)(1000000000LL * 60 / m_rpm);  // one rotation in nanosec

	while (timepos < cyltime)
	{
		curcells = trackbuf[offset];
		for (int i=0; i < 8; i++)
		{
			timepos += samplelength;
			if ((curcells & 1)!=0)
				// Append another transition to the vector
				dest.push_back(floppy_image::MG_F | timepos);

			// HFE uses little-endian bit order
			curcells >>= 1;
		}
		offset++;
		// We have alternating blocks of 0x100 bytes for each head
		// If we are at the block end, jump forward to the next block for this head
		if ((offset & 0xff)==0) offset += 0x100;

		// If we have not reached the track end (after cyltime) but run
		// out of samples, repeat the last byte (cell group)
		if (offset >= track_end) offset = track_end - 1;
	}

	// Write splice is always at the start
	image.set_write_splice_position(cyl, head, 0);
}

/*
    Save the tracks to the file. Since we cannot preserve existing
    settings in the header (which may have become outdated anyway), it must
    be rebuilt.
*/
bool hfe_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	std::error_condition err;

	int drive_cylinders = 0;
	int drive_heads = 0;

	// Get the geometry of the drive
	image.get_maximal_geometry(drive_cylinders, drive_heads);

	// Single-sided images are OK, but the drives must support two heads
	if (drive_heads < 2)
	{
		osd_printf_error("hxchfe: Single-sided drives are not supported.\n");
		return false;
	}

	// Create a new header
	int tracks = drive_cylinders;

	if (m_standard_track_count)
	{
		// Keep 35 tracks as is
		// 41 and 42 tracks are usually overdumped
		if ((tracks > 35) && (tracks < 45)) tracks = 40;
		// 81 to 84 tracks are usually overdumped
		if ((tracks >= 70) && (tracks < 85)) tracks = 80;
	}

	encoding_t enc = m_encoding;
	encoding_t enct0s0 = m_encoding;
	encoding_t enct0s1 = m_encoding;

	// ISOIBM may change between FM and MFM
	// The other possible encodings (AMIGA_MFM and EMU_FM) cannot change during use
	int cell_size = 0;
	int samplerate = m_samplerate;

	if (samplerate == 0) samplerate = 250;

	if (enc == ISOIBM_MFM_ENCODING)
	{
		const std::vector<uint32_t> &tbuf = image.get_buffer(1, 0);  // use track 1; may have to use more or others
		cell_size = determine_cell_size(tbuf);
		if (cell_size == 4000) enc = ISOIBM_FM_ENCODING;

		// Check for alternative encodings for track 0 side 0 or side 1. We only
		// support this for ISOIBM_FM/MFM
		enct0s0 = ISOIBM_MFM_ENCODING;
		const std::vector<uint32_t> &tbuf0 = image.get_buffer(0, 0);
		int cell_size0 = determine_cell_size(tbuf0);
		if (cell_size0 == 4000) enct0s0 = ISOIBM_FM_ENCODING;

		enct0s1 = ISOIBM_MFM_ENCODING;
		const std::vector<uint32_t> &tbuf1 = image.get_buffer(0, 1);
		cell_size0 = determine_cell_size(tbuf1);
		if (cell_size0 == 4000) enct0s1 = ISOIBM_FM_ENCODING;
	}

	if (cell_size < 2000) samplerate = 500;

	uint8_t headerbytes[HEADER_LENGTH];   // reserve some space for the header
	std::fill(std::begin(headerbytes), std::end(headerbytes), 0xff);

	memcpy(headerbytes, HFE_FORMAT_HEADER, 8);
	headerbytes[0x08] = 0x00;
	headerbytes[0x09] = tracks;
	headerbytes[0x0a] = 0x02;
	headerbytes[0x0b] = enc;
	headerbytes[0x0c] = samplerate & 0xff;
	headerbytes[0x0d] = (samplerate >> 8) & 0xff;
	headerbytes[0x0e] = 0x00;
	headerbytes[0x0f] = 0x00;
	headerbytes[0x10] = m_floppymode;
	headerbytes[0x11] = 0x00;
	headerbytes[0x12] = 0x01;
	headerbytes[0x13] = 0x00;
	headerbytes[0x14] = m_write_allowed? 0xff : 0x00;
	headerbytes[0x15] = m_single_step? 0xff : 0x00;

	// If no difference, keep the filled 0xff
	if (enct0s0 != enc)
	{
		headerbytes[0x16] = 0x00;
		headerbytes[0x17] = enct0s0;
	}
	if (enct0s1 != enc)
	{
		headerbytes[0x18] = 0x00;
		headerbytes[0x19] = enct0s1;
	}

	// Write back the header
	write_at(io, 0, headerbytes, HEADER_LENGTH);

	// Set up the track list
	std::vector<lut_entry> tracklut;
	int samplelength = 500000 / samplerate;

	// Do we save all tracks that were loaded, or all tracks of the drive?
	// The drive may have more tracks (42, 84), so
	// [x] Option 1: we write as many tracks as specified by the image
	// Does not work when reformatting in an 80-track drive
	// This is currently not supported anyway, see check for double-stepping in eval_header

	// Calculate the buffer length for the cylinder

	long cyltime = (long)(1000000000LL * 60 / m_rpm);
	int size1track = (cyltime / samplelength) / 8;

	// Round up the length of one side to a 0x100 multiple (padding)
	int cylsize = ((size1track + 0xff) & ~0xff) + size1track;

	// Create the lookup table
	// Each entry contains two 16-bit values
	std::vector<lut_entry> cyltable;
	int trackpos = 0x0002 << 9;
	uint8_t byteentry[4];
	int entrypos = 0x0200;

	for (int cyl = 0; cyl < tracks; cyl++)
	{
		lut_entry track(trackpos, cylsize);
		cyltable.push_back(track);

		// Write the new lookup table to the image
		put_u16le(byteentry, trackpos >> 9);
		put_u16le(byteentry+2, cylsize);
		// osd_printf_info("write at %04x: trackpos = %04x, cylsize=%04x\n", entrypos, trackpos, cylsize);
		write_at(io, entrypos, byteentry, 4);
		entrypos += 4;
		trackpos = (trackpos + cylsize + 0x1ff) & ~0x1ff;
	}

	// Allocate and clear the buffer
	std::unique_ptr<uint8_t[]> cylbuf = std::make_unique<uint8_t[]>(cylsize);

	for (int cyl = 0; cyl < tracks; cyl++)
	{
		// Even when the image is set as single-sided, we write both sides
		generate_hfe_bitstream_from_track(cyl, 0, cyltime, samplelength, &cylbuf[0], cyltable[cyl].length, image);
		generate_hfe_bitstream_from_track(cyl, 1, cyltime, samplelength, &cylbuf[0], cyltable[cyl].length, image);

		// Save each track; get the position and length from the lookup table
		// osd_printf_info("write track at %06x\n", cyltable[cyl].offset);
		write_at(io, cyltable[cyl].offset, &cylbuf[0], cyltable[cyl].length);
	}

	return true;
}

int hfe_format::determine_cell_size(const std::vector<uint32_t> &tbuf) const
{
	// Find out the cell size so we can tell whether this is FM or MFM recording.
	//
	// Some systems may have a fixed recording; the size should then be set
	// on instantiation.
	// The encoding may have changed by reformatting; we cannot rely on the
	// header that we loaded.
	//
	// The HFE format needs this information for its format header, which is
	// a bit tricky, because we have to assume a correctly formatted track.
	// Some flux lengths may appear in different recordings:
	//
	//                        Encodings by time in us
	//  Flux lengths   Dens   2     3     4     5     6      7      8
	//  Cell size 4us  SD     -     -     1     -     -      -      10
	//            2us  DD     -     -     10    -     100    -      1000
	//            1us  HD     10    100   1000  -     -      -      -
	//
	// To be sure, we have to find a flux length of 6us (MFM/DD) or 3 us or
	// 2 us (MFM/HD). A length of 4 us may appear for all densities.
	// If there is at least one MFM-IDAM on the track, this will deliver
	// a 6 us length for DD or 3 us for HD (01000[100]1000[100]1).
	// Otherwise we assume FM (4 us).

	// MAME track format:
	// xlllllll xlllllll xlllllll xlllllll xlllllll xlllllll ...
	// |\--+--/
	// |  Area Length in ns (TIME_MASK=0x0fffffff), max: 199999999 ns
	// +- Area code (0=Flux change, 1=non-magnetized, 2=damaged, 3=end of zone)
	//
	// Empty track: zero-length array, equiv to [(non_mag,0ns),(end, 199999999ns)]
	//
	// The HFE format only supports flux changes, no demagnetized or defect zones.
	//
	int cell_start = -1;
	int cell_size = 4000;

	// Skip the beginning (may have a short cell)
	for (int i=2; i < tbuf.size(); i++)
	{
		if (cell_start >= 0)
		{
			int fluxlen = (tbuf[i] & floppy_image::TIME_MASK) - cell_start;
			// Is this a flux length of less than 3.5us (HD) or of 6 us (DD)?
			if (fluxlen < 3500)
			{
				cell_size = 1000;
				break;
			}
			if ((fluxlen > 5500 && fluxlen < 6500))
			{
				cell_size = 2000;
				break;
			}
		}
		// We only measure from the last flux change
		if ((tbuf[i] & floppy_image::MG_MASK)==floppy_image::MG_F)
			cell_start = tbuf[i] & floppy_image::TIME_MASK;
		else
			cell_start = -1;
	}
	return cell_size;
}

void hfe_format::generate_hfe_bitstream_from_track(int cyl, int head, long cyltime, int samplelength, uint8_t* cylinder_buffer, int track_end, const floppy_image &image) const
{

	// See floppy_image_format_t::generate_bitstream_from_track
	// as the original code

	// Get the emulator track image
	const std::vector<uint32_t> &tbuf = image.get_buffer(cyl, head);
	uint32_t next = 0;

	// Prepare offset for the format storage
	int offset = 0x100;
	if (head==0)
	{
		offset = 0;
		track_end -= 0x0100;
	}

	if (tbuf.size() <= 1)
	{
		// Unformatted track
		// HFE does not support unformatted tracks. Return without changes,
		// we assume that the track image was initialized with zeros.
		return;
	}

	// We start directly at position 0, as this format does not preserve a
	// write splice position
	int cur_time = 0;
	int buf_pos = 0;

	// The remaining part of this method is very similar to the implementation
	// of the PLL in floppy_image_format_t, except that it directly creates the
	// bytes for the format. Bits are stored from right to left in each byte.
	int period = samplelength;
	int period_adjust_base = period * 0.05;

	int min_period = int(samplelength*0.75);
	int max_period = int(samplelength*1.25);
	int phase_adjust = 0;
	int freq_hist = 0;

	uint8_t bit = 0x01;
	uint8_t current = 0;

	while (next < cyltime)
	{
		int edge = tbuf[buf_pos] & floppy_image::TIME_MASK;

		// Edge on start of track? Use next entry.
		if (edge==0)
		{
			cur_time = 0;
			edge = tbuf[++buf_pos] & floppy_image::TIME_MASK;
		}

		// Wrapped over end?
		if (edge < cur_time) edge += cyltime;

		// End of cell
		next = cur_time + period + phase_adjust;

		// End of the window is at next; edge is the actual transition
		if (edge >= next)
		{
			// No transition in the window -> 0
			phase_adjust = 0;
		}
		else
		{
			// Transition in the window -> 1
			current |= bit;
			int delta = edge - (next - period/2);

			phase_adjust = 0.65*delta;

			if (delta < 0)
			{
				if (freq_hist < 0) freq_hist--;
				else freq_hist = -1;
			}
			else
			{
				if (delta > 0)
				{
					if (freq_hist > 0) freq_hist++;
					else freq_hist = 1;
				}
				else freq_hist = 0;
			}

			if (freq_hist)
			{
				int afh = freq_hist < 0 ? -freq_hist : freq_hist;
				if (afh > 1)
				{
					int aper = period_adjust_base*delta/period;
					if (!aper)
						aper = freq_hist < 0 ? -1 : 1;
					period += aper;

					if (period < min_period) period = min_period;
					else if (period > max_period) period = max_period;
				}
			}
		}

		cur_time = next;

		// Wrap over the start of the track
		if (cur_time >= cyltime)
		{
			cur_time -= cyltime;
			buf_pos = 0;
		}

		bit = (bit << 1) & 0xff;
		if (bit == 0)
		{
			// All 8 cells done, write result byte to track image and start
			// over with the next one
			bit = 0x01;
			cylinder_buffer[offset++] = current;

			// Do we have a limit for the track end?
			if ((track_end > 0) && (offset > track_end))
				break;

			// Skip to next block for this head
			if ((offset & 0xff)==0) offset += 0x100;
			current = 0;
		}

		// We may have more entries before the edge that indicates the end of
		// this cell. But this cell is done, so skip them all.
		while (buf_pos < tbuf.size()-1 && (tbuf[buf_pos] & floppy_image::TIME_MASK) < cur_time)
			buf_pos++;

		// Wrap around
		if ((buf_pos == tbuf.size()-1) && (tbuf[buf_pos] & floppy_image::TIME_MASK) < cur_time)
			buf_pos = 0;
	}

	// Write the current byte when not done
	if (bit != 0x01)
		cylinder_buffer[offset] = current;
}

const hfe_format FLOPPY_HFE_FORMAT;

/*
    To get specialized HFE formats, create a subclass and set the
    respective format parameters.

    Example:
    hfe_format_amiga::hfe_format_amiga() : hfe_format()
    {
        m_encoding = AMIGA_MFM_ENCODING;
        m_floppymode = AMIGA_DD_FLOPPYMODE;
    }

    const char *hfe_format_amiga::description() const noexcept
    {
        return "HxC Floppy Emulator HFE File format (Amiga)";
    }

    const hfe_format_amiga FLOPPY_HFE_FORMAT_AMIGA;    // add this name to the format_registration
*/

