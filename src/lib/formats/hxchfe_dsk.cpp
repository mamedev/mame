// license:BSD-3-Clause
// copyright-holders:Michael Zapf

/*
    The hfe_format class implements the HFE format that is used for the Lotharek
    floppy emulator.

    Format definition according to the official document:

    File header (0x0000 - 0x01FF, 512 bytes)
    ----------------------------------------

    typedef struct picfileformatheader_
    {
       uint8_t HEADERSIGNATURE[8];        // 0: "HXCPICFE"
       uint8_t formatrevision;            // 8: Revision 0
       uint8_t number_of_track;           // 9: Number of track in the file
       uint8_t number_of_side;            // 10: Number of valid side (Not used by the emulator)
       uint8_t track_encoding;            // 11: Track Encoding mode
                                        // (Used for the write support - Please see the list above)
       uint16_t bitRate;                  // 12: Bitrate in Kbit/s. Ex : 250=250000bits/s
                                        // Max value : 500
       uint16_t floppyRPM;                // 14: Rotation per minute (Not used by the emulator)
       uint8_t floppyinterfacemode;       // 16: Floppy interface mode. (Please see the list above.)
       uint8_t dnu;                       // 17: Free
       uint16_t track_list_offset;        // 18: Offset of the track list LUT in block of 512 bytes
                                        // (Ex: 1=0x200)
       uint8_t write_allowed;             // 20: The Floppy image is write protected ?
       uint8_t single_step;               // 21: 0xFF : Single Step – 0x00 Double Step mode
       uint8_t track0s0_altencoding;      // 22: 0x00 : Use an alternate track_encoding for track 0 Side 0
       uint8_t track0s0_encoding;         // 23: alternate track_encoding for track 0 Side 0
       uint8_t track0s1_altencoding;      // 24: 0x00 : Use an alternate track_encoding for track 0 Side 1
       uint8_t track0s1_encoding;         // 25: alternate track_encoding for track 0 Side 1
    } picfileformatheader;

    Byte order for uint16_t is little endian.

    floppyintefacemode values are defined in the header file as floppymode_t,
    track_encodings are defined as encoding_t

    track0s0_encoding is only valid when track0s0_altencoding==0xff
    track0s1_encoding is only valid when track0s1_altencoding==0xff

    Track offset lookup table (at 0x0200)
    -------------------------------------

    typedef struct pictrack_
    {
        uint16_t offset;     // Offset of the track data in blocks of 512 bytes (Ex: 2=0x400)
        uint16_t track_len;  // Length of the track data in byte.
    } pictrack;

    This table has a size of  number_of_track*4 bytes.

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
     MDM encoding: 10 01 00 10 01 01 01 00
     Reversed order: 0010 1010 0100 1001     = 2a 49

    Interestingly, FM-encoded disks are usually sampled at 250 kbit/s,
    like MFM, although FM only delivers 125 kbit/s. This oversampling leads
    to zero bits (no change) every two positions. See below for details.

     TODO:
     - Handle double-stepping for medium.tracks=40, drive.tracks=80
*/

#include "hxchfe_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include "osdcore.h" // osd_printf_*

#include <tuple>


#define HFE_FORMAT_HEADER   "HXCPICFE"

#define HEADER_LENGTH 512
#define TRACK_TABLE_LENGTH 1024

hfe_format::hfe_format() : floppy_image_format_t()
{
}

const char *hfe_format::name() const noexcept
{
	return "hfe";
}

const char *hfe_format::description() const noexcept
{
	return "SDCard HxC Floppy Emulator HFE File format";
}

const char *hfe_format::extensions() const noexcept
{
	return "hfe";
}

bool hfe_format::supports_save() const noexcept
{
	return false;
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

bool hfe_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	std::error_condition err;
	size_t actual;

	int drivecyl, driveheads;
	image.get_maximal_geometry(drivecyl, driveheads);

	// read header
	uint8_t header[HEADER_LENGTH];
	std::tie(err, actual) = read_at(io, 0, header, HEADER_LENGTH); // FIXME: check for errors and premature EOF

	// get values
	// Format revision must be 0
	if (header[8] != 0)
	{
		osd_printf_error("hxchfe: Invalid format revision. Expected 0, got %d.\n", header[8]);
		return false;
	}

	header_info info;
	info.m_cylinders = header[9] & 0xff;
	info.m_heads = header[10] & 0xff;

	if (drivecyl < info.m_cylinders)
	{
		if (info.m_cylinders - drivecyl > DUMP_THRESHOLD)
		{
			osd_printf_error("hxchfe: Floppy disk has too many tracks for this drive (floppy tracks=%d, drive tracks=%d).\n", info.m_cylinders, drivecyl);
			return false;
		}
		else
		{
			// Some dumps has a few excess tracks to be safe,
			// lets be nice and just skip those tracks
			osd_printf_warning("hxchfe: Floppy disk has a slight excess of tracks for this drive that will be discarded (floppy tracks=%d, drive tracks=%d).\n", info.m_cylinders, drivecyl);
			info.m_cylinders = drivecyl;
		}
	}

	if (info.m_cylinders <= drivecyl/2)
	{
		osd_printf_error("hxchfe: Double stepping not yet supported (floppy tracks=%d, drive tracks=%d).\n", info.m_cylinders, drivecyl);
		return false;
	}

	info.m_track_encoding = encoding_t(header[11] & 0xff);

	if (info.m_track_encoding > EMU_FM_ENCODING)
	{
		osd_printf_error("hxchfe: Unknown track encoding %d.\n", info.m_track_encoding);
		return false;
	}

	info.m_bit_rate = get_u16le(&header[12]);

	if (info.m_bit_rate > 500)
	{
		osd_printf_error("hxchfe: Unsupported bit rate %d.\n", info.m_bit_rate);
		return false;
	}
	int samplelength = 500000 / info.m_bit_rate;

	// Not used in the HxC emulator
	info.m_floppy_rpm = get_u16le(&header[14]);

	info.m_interface_mode = (floppymode_t)(header[16] & 0xff);
	if (info.m_interface_mode > S950_HD_FLOPPYMODE)
	{
		osd_printf_error("hxchfe: Unknown interface mode %d.\n", info.m_interface_mode);
		return false;
	}

	info.m_write_allowed = (header[20] != 0);
	info.m_single_step = (header[21] != 0);
	info.m_track0s0_has_altencoding = (header[22] == 0x00);
	info.m_track0s0_encoding = (encoding_t)(header[23] & 0xff);
	info.m_track0s1_has_altencoding = (header[24] == 0x00);
	info.m_track0s1_encoding = (encoding_t)(header[25] & 0xff);

	// read track lookup table (multiple of 512)
	int table_offset = get_u16le(&header[18]);

	uint8_t track_table[TRACK_TABLE_LENGTH];
	std::tie(err, actual) = read_at(io, table_offset<<9, track_table, TRACK_TABLE_LENGTH); // FIXME: check for errors and premature EOF

	for (int i=0; i < info.m_cylinders; i++)
	{
		info.m_cyl_offset[i] = get_u16le(&track_table[4*i]);
		info.m_cyl_length[i] = get_u16le(&track_table[4*i+2]);
	}

	// Load the tracks
	std::vector<uint8_t> cylinder_buffer;
	for(int cyl=0; cyl < info.m_cylinders; cyl++)
	{
		// actual data read
		// The HFE format defines an interleave of the two sides per cylinder at every 256 bytes
		cylinder_buffer.resize(info.m_cyl_length[cyl]);
		std::tie(err, actual) = read_at(io, info.m_cyl_offset[cyl]<<9, &cylinder_buffer[0], info.m_cyl_length[cyl]); // FIXME: check for errors and premature EOF

		generate_track_from_hfe_bitstream(cyl, 0, samplelength, &cylinder_buffer[0], info.m_cyl_length[cyl], image);
		if (info.m_heads == 2)
			generate_track_from_hfe_bitstream(cyl, 1, samplelength, &cylinder_buffer[0], info.m_cyl_length[cyl], image);
	}

	bool success = true;

	// Find variant
	if (info.m_track_encoding == ISOIBM_FM_ENCODING || info.m_track_encoding == EMU_FM_ENCODING)
		// FM is for single density
		image.set_variant((info.m_heads==1)? floppy_image::SSSD : floppy_image::DSSD);
	else
	{
		// MFM encoding is for everything else
		if (info.m_track_encoding == ISOIBM_MFM_ENCODING || info.m_track_encoding == AMIGA_MFM_ENCODING)
		{
			// Each cylinder contains the samples of both sides, 8 samples per
			// byte; the bitRate determines how many samples constitute a cell

			// DSDD: 360 KiB (5.25")= 2*40*18*256; 100000 cells/track, 2 us, bit rate = 250 kbit/s
			// DSDD: 720 KiB (3.5") = 2*80*18*256; 100000 cells/track, 2 us, 250 kbit/s
			// DSHD: 1.4 MiB = 2*80*18*512 bytes; 200000 cells/track, 1 us, 500 kbit/s
			// DSED: 2.8 MiB = 2*80*36*512 bytes; 400000 cells/track, 500 ns, 1 Mbit/s

			// Use cylinder 1 (cyl 0 may have special encodings)
			int cellcount = (info.m_cyl_length[1] * 8 / 2) * 250 / info.m_bit_rate;
			if (cellcount > 300000)
				image.set_variant(floppy_image::DSED);
			else
			{
				if (cellcount > 150000)
					image.set_variant(floppy_image::DSHD);
				else
				{
					if (cellcount > 90000)
						// We cannot distinguish DSDD from DSQD without knowing the size of the floppy disk
						image.set_variant((info.m_heads==1)? floppy_image::SSDD : floppy_image::DSDD);
				}
			}
		}
		else
			success = false;
	}
	return success;
}

void hfe_format::generate_track_from_hfe_bitstream(int cyl, int head, int samplelength, const uint8_t *trackbuf, int track_end, floppy_image &image)
{
	// HFE has a minor issue: The track images do not sum up to 200 ms.
	// Tracks are samples at 250 kbit/s for both FM and MFM, which yields
	// 50000 data bits (100000 samples) for MFM, while FM is twice oversampled
	// (4 samples per actual data bit)
	// Hence, for both FM and MFM, we need 100000 samples.

	// Track length 61B0 (both sides, FM)
	// 100 + 100 + ... + 100 + (B0+50)    = 3000 + B0 + 50 (pad)
	//    100 + 100 + .... + 100  +  B0   = 3000 + B0 = 99712 samples   (-288)

	// Track length 61C0 (both sides, MFM)
	// 100 + 100 + ... + 100 + (C0+40)       = 3000 + C0 + 40 (pad)
	//    100 + 100 + .... + 100   +   C0    = 3000 + C0 = 99840 samples   (-160)

	// Solution: Repeat the last byte until we have enough samples
	// Note: We do not call normalize_times here because we're doing the job here

	// HFE does not define subtracks; set to 0

	// MG_1 / MG_0 are (logical) levels that indicate transition / no change
	// MG_F is the position of a flux transition

	std::vector<uint32_t> &dest = image.get_buffer(cyl, head, 0);
	dest.clear();

	int offset = 0x100;

	if (head==0)
	{
		offset = 0;
		track_end -= 0x0100;
	}

	uint8_t current = 0;
	int time  = 0;

	// Oversampled FM images (250 kbit/s) start with a 0, where a 1 is
	// expected for 125 kbit/s.
	// In order to make an oversampled image look like a normally sampled one,
	// we position the transition at 500 ns before the cell end.
	// The HFE format has a 1 us minimum cell size; this means that a normally
	// sampled FM image with 11111... at the begining means
	// 125 kbit/s:    1   1   1   1   1...
	// 250 kbit/s:   01  01  01  01  01...
	// 500 kbit/s: 00010001000100010001...
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

	time = -500;

	// We are creating a sequence of timestamps with flux info
	// Note that the flux change occurs in the last quarter of a cell

	while (time < 200000000)   // one rotation in nanosec
	{
		current = trackbuf[offset];
		for (int j=0; j < 8; j++)
		{
			time += samplelength;
			if ((current & 1)!=0)
				// Append another transition to the vector
				dest.push_back(floppy_image::MG_F | time);

			// HFE uses little-endian bit order
			current >>= 1;
		}
		offset++;
		if ((offset & 0xff)==0) offset += 0x100;

		// When we have not reached the track end (after 0.2 sec) but run
		// out of samples, repeat the last value
		if (offset >= track_end) offset = track_end - 1;
	}

	image.set_write_splice_position(cyl, head, 0, 0);
}

const hfe_format FLOPPY_HFE_FORMAT;


