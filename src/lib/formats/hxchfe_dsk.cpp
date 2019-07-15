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

    (first possible occurance at 0x0400)

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

#define HFE_FORMAT_HEADER   "HXCPICFE"

#define HEADER_LENGTH 512
#define TRACK_TABLE_LENGTH 1024

hfe_format::hfe_format() : floppy_image_format_t(),
   m_cylinders(0),
   m_heads(0),
   m_track_encoding(UNKNOWN_ENCODING),
   m_bit_rate(0),
   m_floppy_rpm(0),
   m_interface_mode(DISABLE_FLOPPYMODE),
   m_write_allowed(true),
   m_single_step(true),
   m_track0s0_has_altencoding(false),
   m_track0s0_encoding(UNKNOWN_ENCODING),
   m_track0s1_has_altencoding(false),
   m_track0s1_encoding(UNKNOWN_ENCODING),
   m_selected_mode(DISABLE_FLOPPYMODE),
   m_selected_encoding(UNKNOWN_ENCODING)
{
}

const char *hfe_format::name() const
{
	return "hfe";
}

const char *hfe_format::description() const
{
	return "SDCard HxC Floppy Emulator HFE File format";
}

const char *hfe_format::extensions() const
{
	return "hfe";
}

bool hfe_format::supports_save() const
{
	return true;
}

int hfe_format::identify(io_generic *io, uint32_t form_factor)
{
	uint8_t header[8];

	io_generic_read(io, &header, 0, sizeof(header));
	if ( memcmp( header, HFE_FORMAT_HEADER, 8 ) ==0) {
		return 100;
	}
	return 0;
}

bool hfe_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
{
	uint8_t header[HEADER_LENGTH];
	uint8_t track_table[TRACK_TABLE_LENGTH];

	int drivecyl, driveheads;
	image->get_maximal_geometry(drivecyl, driveheads);

	// read header
	io_generic_read(io, header, 0, HEADER_LENGTH);

	// get values
	// Format revision must be 0
	if (header[8] != 0)
	{
		osd_printf_error("hxchfe: Invalid format revision. Expected 0, got %d.\n", header[8]);
		return false;
	}

	m_cylinders = header[9] & 0xff;
	m_heads = header[10] & 0xff;

	if (drivecyl < m_cylinders)
	{
		if (m_cylinders - drivecyl > DUMP_THRESHOLD)
		{
			osd_printf_error("hxchfe: Floppy disk has too many tracks for this drive (floppy tracks=%d, drive tracks=%d).\n", m_cylinders, drivecyl);
			return false;
		}
		else
		{
			// Some dumps has a few excess tracks to be safe,
			// lets be nice and just skip those tracks
			osd_printf_warning("hxchfe: Floppy disk has a slight excess of tracks for this drive that will be discarded (floppy tracks=%d, drive tracks=%d).\n", m_cylinders, drivecyl);
			m_cylinders = drivecyl;
		}
	}

	if (m_cylinders <= drivecyl/2)
	{
		osd_printf_error("hxchfe: Double stepping not yet supported (floppy tracks=%d, drive tracks=%d).\n", m_cylinders, drivecyl);
		return false;
	}

	m_track_encoding = (encoding_t)(header[11] & 0xff);

	if (m_track_encoding > EMU_FM_ENCODING)
	{
		osd_printf_error("hxchfe: Unknown track encoding %d.\n", m_track_encoding);
		return false;
	}

	m_bit_rate = (header[12] & 0xff) | ((header[13] & 0xff)<<8);

	if (m_bit_rate > 500)
	{
		osd_printf_error("hxchfe: Unsupported bit rate %d.\n", m_bit_rate);
		return false;
	}
	int samplelength = 500000 / m_bit_rate;

	// Not used in the HxC emulator
	m_floppy_rpm = (header[14] & 0xff) | ((header[15] & 0xff)<<8);

	m_interface_mode = (floppymode_t)(header[16] & 0xff);
	if (m_interface_mode > S950_HD_FLOPPYMODE)
	{
		osd_printf_error("hxchfe: Unknown interface mode %d.\n", m_interface_mode);
		return false;
	}

	m_write_allowed = (header[20] != 0);
	m_single_step = (header[21] != 0);
	m_track0s0_has_altencoding = (header[22] == 0x00);
	m_track0s0_encoding = (encoding_t)(header[23] & 0xff);
	m_track0s1_has_altencoding = (header[24] == 0x00);
	m_track0s1_encoding = (encoding_t)(header[25] & 0xff);

	// read track lookup table (multiple of 512)
	int table_offset = (header[18] & 0xff) | ((header[19] & 0xff)<<8);

	io_generic_read(io, track_table, table_offset<<9, TRACK_TABLE_LENGTH);

	for (int i=0; i < m_cylinders; i++)
	{
		m_cyl_offset[i] = (track_table[4*i] & 0xff) | ((track_table[4*i+1] & 0xff)<<8);
		m_cyl_length[i] = (track_table[4*i+2] & 0xff) | ((track_table[4*i+3] & 0xff)<<8);
	}

	// Load the tracks
	std::vector<uint8_t> cylinder_buffer;
	for(int cyl=0; cyl < m_cylinders; cyl++)
	{
		// actual data read
		// The HFE format defines an interleave of the two sides per cylinder
		// at every 256 bytes
		cylinder_buffer.resize(m_cyl_length[cyl]);
		io_generic_read(io, &cylinder_buffer[0], m_cyl_offset[cyl]<<9, m_cyl_length[cyl]);

		generate_track_from_hfe_bitstream(cyl, 0, samplelength, &cylinder_buffer[0], m_cyl_length[cyl], image);
		if (m_heads == 2)
			generate_track_from_hfe_bitstream(cyl, 1, samplelength, &cylinder_buffer[0], m_cyl_length[cyl], image);
	}

	bool success = true;

	// Find variant
	if (m_track_encoding == ISOIBM_FM_ENCODING || m_track_encoding == EMU_FM_ENCODING)
		// FM is for single density
		image->set_variant((m_heads==1)? floppy_image::SSSD : floppy_image::DSSD);
	else
	{
		// MFM encoding is for everything else
		if (m_track_encoding == ISOIBM_MFM_ENCODING || m_track_encoding == AMIGA_MFM_ENCODING)
		{
			// Each cylinder contains the samples of both sides, 8 samples per
			// byte; the bitRate determines how many samples constitute a cell

			// DSDD: 360 KiB (5.25")= 2*40*18*256; 100000 cells/track, 2 us, bit rate = 250 kbit/s
			// DSDD: 720 KiB (3.5") = 2*80*18*256; 100000 cells/track, 2 us, 250 kbit/s
			// DSHD: 1.4 MiB = 2*80*18*512 bytes; 200000 cells/track, 1 us, 500 kbit/s
			// DSED: 2.8 MiB = 2*80*36*512 bytes; 400000 cells/track, 500 ns, 1 Mbit/s

			// Use cylinder 1 (cyl 0 may have special encodings)
			int cellcount = (m_cyl_length[1] * 8 / 2) * 250 / m_bit_rate;
			if (cellcount > 300000)
				image->set_variant(floppy_image::DSED);
			else
			{
				if (cellcount > 150000)
					image->set_variant(floppy_image::DSHD);
				else
				{
					if (cellcount > 90000)
						// We cannot distinguish DSDD from DSQD without knowing the size of the floppy disk
						image->set_variant((m_heads==1)? floppy_image::SSDD : floppy_image::DSDD);
				}
			}
		}
		else
			success = false;
	}
	return success;
}

void hfe_format::generate_track_from_hfe_bitstream(int cyl, int head, int samplelength, const uint8_t *trackbuf, int track_end, floppy_image *image)
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
	// MG_A / MG_B are physical flux directions
	//
	// Cell: | AAAABBBB | = MG_1 = | BBBBAAAA |
	//       | AAAAAAAA | = MG_0 = | BBBBBBBB |

	std::vector<uint32_t> &dest = image->get_buffer(cyl, head, 0);
	dest.clear();

	// Start with MG_A
	uint32_t cbit = floppy_image::MG_A;

	int offset = 0x100;

	if (head==0)
	{
		offset = 0;
		track_end -= 0x0100;
	}

	uint8_t current = 0;
	int time  = 0;

	dest.push_back(cbit | time);

	cbit = floppy_image::MG_B;

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
			{
				// Append another transition to the vector
				dest.push_back(cbit | time);

				// Toggle the cell level
				cbit = (cbit == floppy_image::MG_A)? floppy_image::MG_B : floppy_image::MG_A;
			}

			// HFE uses little-endian bit order
			current >>= 1;
		}
		offset++;
		if ((offset & 0xff)==0) offset += 0x100;

		// When we have not reached the track end (after 0.2 sec) but run
		// out of samples, repeat the last value
		if (offset >= track_end) offset = track_end - 1;
	}

	image->set_write_splice_position(cyl, head, 0, 0);
}

bool hfe_format::save(io_generic *io, floppy_image *image)
{
	std::vector<uint8_t> cylbuf;

	// Create a buffer that is big enough to handle HD formats. We don't
	// know the track length until we generate the HFE bitstream.
	cylbuf.resize(0x10000);

	uint8_t header[HEADER_LENGTH];
	uint8_t track_table[TRACK_TABLE_LENGTH];

	int track_end = 0x61c0;
	int samplelength = 2000;

	// Set up header
	const char* sig = "HXCPICFE";
	memcpy(header, sig, 8);

	header[8] = 0;
	// Can we change the number of tracks or heads?
	image->get_actual_geometry(m_cylinders, m_heads);

	header[9] = m_cylinders;
	header[10] = m_heads;
	// Floppy RPM is not used
	header[14] = 0;
	header[15] = 0;

	// Bit rate and encoding will be set later, they may have changed by
	// reformatting. The selected encoding is UNKNOWN_ENCODING unless
	// explicitly set
	m_track_encoding = m_selected_encoding;

	// Take the old mode, unless we have specified a mode
	header[16] = (m_selected_mode != DISABLE_FLOPPYMODE)? m_selected_mode : m_interface_mode;
	header[17] = 0;

	// The track lookup table is located at offset 0x200 (as 512 multiple)
	header[18] = 1;
	header[19] = 0;

	header[20] = m_write_allowed? 0xff : 0x00;
	header[21] = m_single_step? 0xff : 0x00;

	// TODO: Allow for divergent track 0 format
	header[22] = m_track0s0_has_altencoding? 0x00 : 0xff;
	header[23] = m_track0s0_encoding;
	header[24] = m_track0s1_has_altencoding? 0x00 : 0xff;
	header[25] = m_track0s1_encoding;

	// Fill the remaining bytes with 0xff
	for (int i=26; i < HEADER_LENGTH; i++) header[i] = 0xff;

	// Don't write yet; we still have to find out the bit rate.

	// We won't have more than 200000 cells on the track
	for (int cyl=0; cyl < m_cylinders; cyl++)
	{
		// After the call, the encoding will be set to FM or MFM
		generate_hfe_bitstream_from_track(cyl, 0, samplelength, m_track_encoding, &cylbuf[0], track_end, image);
		if (m_heads == 2)
			generate_hfe_bitstream_from_track(cyl, 1, samplelength, m_track_encoding, &cylbuf[0], track_end, image);

		if (cyl==0)
		{
			// Complete the header and write it
			header[11] = m_track_encoding;
			m_bit_rate = 500000/samplelength;
			header[12] = m_bit_rate & 0xff;
			header[13] = (m_bit_rate >> 8) & 0xff;

			// Now write the header
			io_generic_write(io, header, 0, HEADER_LENGTH);

			// Set up the track lookup table
			// We need the encoding value to be sure about the track length
			int len = (m_track_encoding==ISOIBM_FM_ENCODING)? 0x61b0 : 0x61c0;
			int pos = 0x400;

			for (int i=0; i < m_cylinders; i++)
			{
				m_cyl_offset[i] = (pos >> 9);
				m_cyl_length[i] = len;
				pos += (len + 0x1ff) & 0xfe00;
				track_table[i*4] = m_cyl_offset[i] & 0xff;
				track_table[i*4+1] = (m_cyl_offset[i]>>8) & 0xff;
				track_table[i*4+2] = len & 0xff;
				track_table[i*4+3] = (len>>8) & 0xff;
			}
			// Set the remainder to 0xff
			for (int i=m_cylinders*4; i < TRACK_TABLE_LENGTH; i++)
				track_table[i] = 0xff;

			io_generic_write(io, track_table, 0x200, TRACK_TABLE_LENGTH);
		}
		// Write the current cylinder
		io_generic_write(io, &cylbuf[0], m_cyl_offset[cyl]<<9, (m_cyl_length[cyl] + 0x1ff) & 0xfe00);
	}
	return true;
}

void hfe_format::generate_hfe_bitstream_from_track(int cyl, int head, int& samplelength, encoding_t& encoding, uint8_t *cylinder_buffer, int track_end, floppy_image *image)
{
	// We are using an own implementation here because the result of the
	// parent class method would require some post-processing that we
	// can easily avoid.

	// See floppy_image_format_t::generate_bitstream_from_track
	// as the original code

	// No subtracks definded
	std::vector<uint32_t> &tbuf = image->get_buffer(cyl, head, 0);
	if (tbuf.size() <= 1)
	{
		// Unformatted track
		// TODO must handle that according to HFE
		int track_size = 200000000/samplelength;
		memset(cylinder_buffer, 0, (track_size+7)/8);
		return;
	}

	// Find out whether we have FM or MFM recording, and determine the bit rate.
	// This is needed for the format header.
	//
	// The encoding may have changed by reformatting; we cannot rely on the
	// header when loading.
	//
	// FM:   encoding 1    -> flux length = 4 us (min)          ambivalent
	//       encoding 10   -> flux length = 8 us (max)          ambivalent
	// MFM:  encoding 10   -> flux length = 4 us (min, DD)      ambivalent
	//       encoding 100  -> flux length = 6 us (DD)            significant
	//       encoding 1000 -> flux length = 8 us (max, DD)      ambivalent
	//       encoding 10   -> flux length = 2 us (min, HD)       significant
	//       encoding 100  -> flux length = 3 us (max, HD)       significant

	// If we have MFM, we should very soon detect a flux length of 6 us.
	// But if we have FM, how long should we search to be sure?
	// We assume that after 2000 us we should have reached the first IDAM,
	// which contains a sequence 1001, implying a flux length of 6 us.
	// If there was no such flux in that area, this can safely be assumed to be FM.

	// Do it only for the first track; the format only supports one encoding.
	if (encoding == UNKNOWN_ENCODING)
	{
		bool mfm_recording = false;
		int time0 = 0;
		int minflux = 4000;
		int fluxlen = 0;
		// Skip the beginning (may have a short cell)
		for (int i=2; (i < tbuf.size()-1) && (time0 < 2000000) && !mfm_recording; i++)
		{
			time0 = tbuf[i] & floppy_image::TIME_MASK;
			fluxlen = (tbuf[i+1] & floppy_image::TIME_MASK) - time0;
			if ((fluxlen < 3500) || (fluxlen > 5500 && fluxlen < 6500))
				mfm_recording = true;
			if (fluxlen < minflux) minflux = fluxlen;
		}
		encoding = mfm_recording? ISOIBM_MFM_ENCODING : ISOIBM_FM_ENCODING;

		// samplelength = 1000ns => 10^6 cells/sec => 500 kbit/s
		// samplelength = 2000ns => 250 kbit/s
		// We stay with double sampling at 250 kbit/s for FM
		if (minflux < 3500) samplelength = 1000;
		else samplelength = 2000;
	}

	// Start at the write splice
	uint32_t splice = image->get_write_splice_position(cyl, head, 0);

	int cur_pos = splice;
	int cur_entry = 0;

	// Fast-forward to the write splice position (always 0 in this format)
	while (cur_entry < int(tbuf.size())-1 && (tbuf[cur_entry+1] & floppy_image::TIME_MASK) < cur_pos)
		cur_entry++;

	int period = samplelength;
	int period_adjust_base = period * 0.05;

	int min_period = int(samplelength*0.75);
	int max_period = int(samplelength*1.25);
	int phase_adjust = 0;
	int freq_hist = 0;
	uint32_t next = 0;

	int offset = 0x100;

	// Prepare offset for the format storage
	if (head==0)
	{
		offset = 0;
		track_end -= 0x0100;
	}

	uint8_t bit = 0x01;
	uint8_t current = 0;

	while (next < 200000000) {
		int edge = tbuf[cur_entry] & floppy_image::TIME_MASK;

		// Start of track? Use next entry.
		if (edge==0)
		{
			cur_pos = 0;
			edge = tbuf[++cur_entry] & floppy_image::TIME_MASK;
		}

		// Wrapped over end?
		if (edge < cur_pos) edge += 200000000;

		// End of cell
		next = cur_pos + period + phase_adjust;

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
					if(freq_hist > 0) freq_hist++;
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

		cur_pos = next;
		if(cur_pos >= 200000000) {
			cur_pos -= 200000000;
			cur_entry = 0;
		}

		bit = (bit << 1) & 0xff;
		if (bit == 0)
		{
			bit = 0x01;
			cylinder_buffer[offset++] = current;
			if ((offset & 0xff)==0) offset += 0x100;
			current = 0;
		}

		// Fast-forward to next cell
		while (cur_entry < int(tbuf.size())-1 && (tbuf[cur_entry] & floppy_image::TIME_MASK) < cur_pos)
			cur_entry++;

		// Reaching the end of the track
		if (cur_entry == int(tbuf.size())-1 &&  (tbuf[cur_entry] & floppy_image::TIME_MASK) < cur_pos)
		{
			// Wrap to index 0 or 1 depending on whether there is a transition exactly at the index hole
			cur_entry = (tbuf[int(tbuf.size())-1] & floppy_image::MG_MASK) != (tbuf[0] & floppy_image::MG_MASK) ?
			0 : 1;
		}
	}
	// Write the current byte when not done
	if (bit != 0x01)
		cylinder_buffer[offset] = current;
}

const floppy_format_type FLOPPY_HFE_FORMAT = &floppy_image_format_creator<hfe_format>;


