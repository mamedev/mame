// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*************************************************************************

    Hard disk emulation: Format implementation
    ------------------------------------------

    This is the format implementation for MFM hard disks, similar to the
    modular format concept of floppy drives in MAME/MESS.

    The base class is mfmhd_image_format_t; it contains some methods for
    encoding and decoding MFM. Although MFM hard disks should also be able to
    manage FM recording, we do not plan for FM recording here.

    The encode/decode methods rely on a parameter "encoding";
    see imagedev/mfmhd.c for a discussion. Essentially, it determines whether
    data are read bitwise or bytewise, and whether clock bits are separated
    or interleaved.

    The base class is abstract; you must create a subclass to use it. This
    file delivers one subclass called mfmhd_generic_format.

    In order to use this format, you must pass the creator identifier to the
    macro MCFG_MFM_HARDDISK_CONN_ADD. See emu/bus/ti99_peb/hfdc.c for an
    example.


    Generic MFM format
    ------------------
    The heart of this class are the methods load and save. They are designed
    to read sector data from a CHD file and reconstruct the track image (load),
    or to take a track image, isolate the sector data, and store them
    into the CHD (save).

    Rebuilding the track image means to create sector headers, allocate gaps,
    add sync areas, and CRC values. Also, the sectors must be arranged
    according to the "interleave" parameter and the "skew" parameters for
    heads and cylinders. While the skews are commonly set to 0, the interleave
    is often used to fine-tune the transfer speed between the drive hardware
    and the host system.

    Also, the format allows for two header setups.
    a) PC-AT-compatible header: four bytes long (ident, cylinder, head, sector);
       the sector size is always 512 bytes.
    b) Custom headers: five bytes long (..., sector size). The custom headers
       are used in non-PC systems.

    ECC: While floppy drives make use of a CRC field to check the data integrity,
    hard disks use an ECC (error correcting code). The ECC length is 4 bytes
    or longer, depending on the desired correction capability. The ECC length
    can also be specified for this format.

    However, for this version, we do not support ECC computation, but instead
    we use CRC. This is indicated by setting the "ECC length" parameter to -1.

    Format autodetect
    -----------------
    While formatting a hard disk, format parameters are likely to change, so
    we have to find out about the new layout and store the metadata into the
    CHD if they were modified.

    This is done in the save method. This method does not only retrieve the
    sector contents but also counts the gap bytes and sync bytes so that
    they can be stored in the CHD.

    - Interleave detection: save counts the number of sectors between sector
      number n and sector number n+1.

    - Skew detection: Skew is determined by three tracks: (cyl,head)=
      (0,0), (1,0), and (0,1). For this purpose we use the m_secnumber list.

    - Header length is detemined by the first sector on (0,0). This is done
      by checking the header against the following two CRC bytes. If they
      match for 4 bytes, we have an AT-style header, else a custom header.

    - Gap and sync lengths are determined by the first track (0,0). They are
      actually not expected to change, unless they are undefined before first
      use, or the controller or its driver changes. We assume that track
      (0,0) is actually rewritten during reformatting.

    Since write precompensation and reduced write current cannot be seen
    on the track image directly, those two values have to be set by the
    hard disk device itseltf.

    Inhibit autodetect
    ------------------
    In case we do not want the format to detect the layout but want to ensure
    an immutable format, the save_param method may be overwritten to return
    false for all or a particular group of parameters. The generic format
    offers a save_param method which always returns true.

    The effect of inhibiting the autodetection is that the layout parameters
    as found on the CHD are used if available; otherwise defaults are used.

    Defaults
    --------
    The generic format defines a method get_default which returns safe values
    for layout parameters. It can be overwritten for specific formats.

    Debugging
    ---------
    There is a set of debug flags (starting with TRACE_) that can be set to 1;
    after recompiling you will get additional output. Since this class is not
    a descendant of device_t we do not have a tag for output; for a better
    overview in the logfile the hard disk device passes its tag to the base
    class.


    TODO
    ----
    - Add ECC computation


    Michael Zapf
    August 2015

**************************************************************************/

#include "emu.h"
#include "mfm_hd.h"
#include "imageutl.h"

#define TRACE_RWTRACK 0
#define TRACE_LAYOUT 0
#define TRACE_IMAGE 0
#define TRACE_DETAIL 0
#define TRACE_FORMAT 0

/*
    Accept the new layout parameters and reset the sector number fields
    used for skew calculation.
*/
void mfmhd_image_format_t::set_layout_params(mfmhd_layout_params param)
{
	m_param = m_param_old = param;
	m_secnumber[0] = m_secnumber[1] = m_secnumber[2] = -1;
}

/*
    Encode some value with data-type clock bits.
*/
void mfmhd_image_format_t::mfm_encode(UINT16* trackimage, int& position, UINT8 byte, int count)
{
	mfm_encode_mask(trackimage, position, byte, count, 0x00);
}

/*
    Encode an A1 value with mark-type clock bits.
*/
void mfmhd_image_format_t::mfm_encode_a1(UINT16* trackimage, int& position)
{
	m_current_crc = 0xffff;
	mfm_encode_mask(trackimage, position, 0xa1, 1, 0x04);
}

/*
    Encode a byte value with a given clock bit mask. Used by both mfm_encode
    and mfm_encode_a1 methods.
*/
void mfmhd_image_format_t::mfm_encode_mask(UINT16* trackimage, int& position, UINT8 byte, int count, int mask)
{
	UINT16 encclock = 0;
	UINT16 encdata = 0;
	UINT8 thisbyte = byte;
	bool mark = (mask != 0x00);

	m_current_crc = ccitt_crc16_one(m_current_crc, byte);

	for (int i=0; i < 8; i++)
	{
		encdata <<= 1;
		encclock <<= 1;

		if (m_param.encoding == MFM_BITS || m_param.encoding == MFM_BYTE)
		{
			// skip one position for later interleaving
			encdata <<= 1;
			encclock <<= 1;
		}

		if (thisbyte & 0x80)
		{
			// Encoding 1 => 01
			encdata |= 1;
			m_lastbit = true;
		}
		else
		{
			// Encoding 0 => x0
			// If the bit in the mask is set, suppress the clock bit
			// Also, if we use the simplified encoding, don't set the clock bits
			if (m_lastbit == false && m_param.encoding != SEPARATED_SIMPLE && (mask & 0x80) == 0) encclock |= 1;
			m_lastbit = false;
		}
		mask <<= 1;
		// For simplified encoding, set all clock bits to indicate a mark
		if (m_param.encoding == SEPARATED_SIMPLE && mark) encclock |= 1;
		thisbyte <<= 1;
	}

	if (m_param.encoding == MFM_BITS || m_param.encoding == MFM_BYTE)
		encclock <<= 1;
	else
		encclock <<= 8;

	trackimage[position++] = (encclock | encdata);

	// When we write the byte multiple times, check whether the next encoding
	// differs from the previous because of the last bit

	if (m_param.encoding == MFM_BITS || m_param.encoding == MFM_BYTE)
	{
		encclock &= 0x7fff;
		if ((byte & 0x80)==0 && m_lastbit==false) encclock |= 0x8000;
	}

	for (int j=1; j < count; j++)
	{
		trackimage[position++] = (encclock | encdata);
		m_current_crc = ccitt_crc16_one(m_current_crc, byte);
	}
}

/*
    Decode an MFM cell pattern into a byte value.
    Clock bits and data bits are assumed to be interleaved (cdcdcdcdcdcdcdcd);
    the 8 data bits are returned.
*/
UINT8 mfmhd_image_format_t::mfm_decode(UINT16 raw)
{
	unsigned int value = 0;

	for (int i=0; i < 8; i++)
	{
		value <<= 1;

		value |= (raw & 0x4000);
		raw <<= 2;
	}
	return (value >> 14) & 0xff;
}

/*
    For debugging. Outputs the byte array in a xxd-like way.
*/
void mfmhd_image_format_t::showtrack(UINT16* enctrack, int length)
{
	for (int i=0; i < length; i+=16)
	{
		osd_printf_verbose("%07x: ", i);
		for (int j=0; j < 16; j++)
		{
			osd_printf_verbose("%04x ", enctrack[i+j]);
		}
		osd_printf_verbose(" ");
		osd_printf_verbose("\n");
	}
}

// ======================================================================
//    Generic MFM HD format
// ======================================================================

const mfmhd_format_type MFMHD_GEN_FORMAT = &mfmhd_image_format_creator<mfmhd_generic_format>;

/*
    Calculate the ident byte from the cylinder. The specification does not
    define idents beyond cylinder 1023, but formatting programs seem to
    continue with 0xfd for cylinders between 1024 and 2047.
*/
UINT8 mfmhd_generic_format::cylinder_to_ident(int cylinder)
{
	if (cylinder < 256) return 0xfe;
	if (cylinder < 512) return 0xff;
	if (cylinder < 768) return 0xfc;
	return 0xfd;
}

/*
    Returns the linear sector number, given the CHS data.

      C,H,S
    | 0,0,0 | 0,0,1 | 0,0,2 | ...
    | 0,1,0 | 0,1,1 | 0,1,2 | ...
    ...
    | 1,0,0 | ...
    ...
*/
int mfmhd_generic_format::chs_to_lba(int cylinder, int head, int sector)
{
	if ((cylinder < m_param.cylinders) && (head < m_param.heads) && (sector < m_param.sectors_per_track))
	{
		return (cylinder * m_param.heads + head) * m_param.sectors_per_track + sector;
	}
	else return -1;
}

chd_error mfmhd_generic_format::load(chd_file* chdfile, UINT16* trackimage, int tracksize, int cylinder, int head)
{
	chd_error state = CHDERR_NONE;
	UINT8 sector_content[16384];

	int sectorcount = m_param.sectors_per_track;
	int size = m_param.sector_size;
	int position = 0; // will be incremented by each encode call
	int sec_number = 0;
	int identfield = 0;
	int cylfield = 0;
	int headfield = 0;
	int sizefield = (size >> 7)-1;

	// If we don't have interleave data in the CHD, take a default
	if (m_param.interleave==0)
	{
		m_param.interleave = get_default(MFMHD_IL);
		m_param.cylskew = get_default(MFMHD_CSKEW);
		m_param.headskew = get_default(MFMHD_HSKEW);
	}

	int sec_il_start = (m_param.cylskew * cylinder + m_param.headskew * head) % sectorcount;
	int delta = (sectorcount + m_param.interleave-1) / m_param.interleave;

	if (TRACE_RWTRACK) osd_printf_verbose("%s: Load track (c=%d,h=%d) from CHD, interleave=%d, cylskew=%d, headskew=%d\n", tag(), cylinder, head, m_param.interleave, m_param.cylskew, m_param.headskew);

	m_lastbit = false;

	if (m_param.sync==0)
	{
		m_param.gap1 = get_default(MFMHD_GAP1);
		m_param.gap2 = get_default(MFMHD_GAP2);
		m_param.gap3 = get_default(MFMHD_GAP3);
		m_param.sync = get_default(MFMHD_SYNC);
		m_param.headerlen = get_default(MFMHD_HLEN);
		m_param.ecctype = get_default(MFMHD_ECC);
	}

	// Gap 1
	mfm_encode(trackimage, position, 0x4e, m_param.gap1);

	if (TRACE_LAYOUT) osd_printf_verbose("%s: cyl=%d head=%d: sector sequence = ", tag(), cylinder, head);

	sec_number = sec_il_start;
	for (int sector = 0; sector < sectorcount; sector++)
	{
		if (TRACE_LAYOUT) osd_printf_verbose("%02d ", sec_number);

		// Sync gap
		mfm_encode(trackimage, position, 0x00, m_param.sync);

		// Write IDAM
		mfm_encode_a1(trackimage, position);

		// Write header
		identfield = cylinder_to_ident(cylinder);
		cylfield = cylinder & 0xff;
		headfield = head & 0x0f;
		if (m_param.headerlen==5)
			headfield |= ((cylinder & 0x700)>>4);

		mfm_encode(trackimage, position, identfield);
		mfm_encode(trackimage, position, cylfield);
		mfm_encode(trackimage, position, headfield);
		mfm_encode(trackimage, position, sec_number);
		if (m_param.headerlen==5)
			mfm_encode(trackimage, position, sizefield);

		// Write CRC for header.
		int crc = m_current_crc;
		mfm_encode(trackimage, position, (crc >> 8) & 0xff);
		mfm_encode(trackimage, position, crc & 0xff);

		// Gap 2
		mfm_encode(trackimage, position, 0x4e, m_param.gap2);

		// Sync
		mfm_encode(trackimage, position, 0x00, m_param.sync);

		// Write DAM
		mfm_encode_a1(trackimage, position);
		mfm_encode(trackimage, position, 0xfb);

		// Get sector content from CHD
		int lbaposition = chs_to_lba(cylinder, head, sec_number);
		if (lbaposition>=0)
		{
			chd_error state = chdfile->read_units(lbaposition, sector_content);
			if (state != CHDERR_NONE) break;
		}
		else
		{
			osd_printf_verbose("%s: Invalid CHS data (%d,%d,%d); not loading from CHD\n", tag(), cylinder, head, sector);
		}

		for (int i=0; i < size; i++)
			mfm_encode(trackimage, position, sector_content[i]);

		// Write CRC for content.
		crc = m_current_crc;
		mfm_encode(trackimage, position, (crc >> 8) & 0xff);
		mfm_encode(trackimage, position, crc & 0xff);

		// Gap 3
		mfm_encode(trackimage, position, 0x00, 3);
		mfm_encode(trackimage, position, 0x4e, m_param.gap3-3);

		// Calculate next sector number
		sec_number += delta;
		if (sec_number >= sectorcount)
		{
			sec_il_start = (sec_il_start+1) % delta;
			sec_number = sec_il_start;
		}
	}
	if (TRACE_LAYOUT) osd_printf_verbose("\n");

	// Gap 4
	if (state == CHDERR_NONE)
	{
		// Fill the rest with 0x4e
		mfm_encode(trackimage, position, 0x4e, tracksize-position);
		if (TRACE_IMAGE) showtrack(trackimage, tracksize);
	}
	return state;
}

/*
    State names for analyzing the track image.
*/
enum
{
	SEARCH_A1=0,
	FOUND_A1,
	DAM_FOUND,
	CHECK_CRC
};

chd_error mfmhd_generic_format::save(chd_file* chdfile, UINT16* trackimage, int tracksize, int current_cylinder, int current_head)
{
	if (TRACE_RWTRACK) osd_printf_verbose("%s: write back (c=%d,h=%d) to CHD\n", tag(), current_cylinder, current_head);

	UINT8 buffer[16384]; // for header or sector content

	int bytepos = 0;
	int state = SEARCH_A1;
	int count = 0;
	int pos = 0;
	UINT16 crc = 0;
	UINT8 byte;
	bool search_header = true;

	int ident = 0;
	int cylinder = 0;
	int head = 0;
	int sector = 0;
	int size = 0;

	int headerpos = 0;

	int interleave = 0;
	int interleave_prec = -1;
	bool check_interleave = true;
	bool check_skew = true;

	int gap1 = 0;
	int ecctype = 0;

	// if (current_cylinder==0 && current_head==0) showtrack(trackimage, tracksize);

	// If we want to detect gaps, we only do it on cylinder 0, head 0
	// This makes it safer to detect the header length
	// (There is indeed some chance that we falsely assume a header length of 4
	// because the two bytes behind happen to be a valid CRC value)
	if (save_param(MFMHD_GAP1) && current_cylinder==0 && current_head==0)
	{
		m_param.gap1 = 0;
		m_param.gap2 = 0;
		m_param.gap3 = 0;
		m_param.sync = 0;
		// 4-byte headers are used for the IBM-AT format
		// 5-byte headers are used in other formats
		m_param.headerlen = 4;
		m_param.ecctype = 0;
	}

	// AT format implies 512 bytes per sector
	int sector_length = 512;

	// Only check once
	bool countgap1 = (m_param.gap1==0);
	bool countgap2 = false;
	bool countgap3 = false;
	bool countsync = false;

	chd_error chdstate = CHDERR_NONE;

	if (TRACE_IMAGE)
	{
		for (int i=0; i < tracksize; i++)
		{
			if ((i % 16)==0) osd_printf_verbose("\n%04x: ", i);
			osd_printf_verbose("%02x ", (m_param.encoding==MFM_BITS || m_param.encoding==MFM_BYTE)? mfm_decode(trackimage[i]) : (trackimage[i]&0xff));
		}
		osd_printf_verbose("\n");
	}

	// We have to go through the bytes of the track and save a sector as soon as one shows up

	while (bytepos < tracksize)
	{
		// Decode the next 16 bits
		if (m_param.encoding==MFM_BITS || m_param.encoding==MFM_BYTE)
		{
			byte = mfm_decode(trackimage[bytepos]);
		}
		else byte = (trackimage[bytepos] & 0xff);

		switch (state)
		{
		case SEARCH_A1:
			// Counting gaps and sync
			if (countgap2)
			{
				if (byte == 0x4e) m_param.gap2++;
				else if (byte == 0) { countsync = true; countgap2 = false; }
			}

			if (countsync)
			{
				if (byte == 0) m_param.sync++;
				else countsync = false;
			}

			if (countgap3)
			{
				if (byte != 0x00 || m_param.gap3 < 4) m_param.gap3++;
				else countgap3 = false;
			}

			if (((m_param.encoding==MFM_BITS || m_param.encoding==MFM_BYTE) && trackimage[bytepos]==0x4489)
				|| (m_param.encoding==SEPARATED && trackimage[bytepos]==0x0aa1)
				|| (m_param.encoding==SEPARATED_SIMPLE && trackimage[bytepos]==0xffa1))
			{
				state = FOUND_A1;
				count = (search_header? m_param.headerlen : (sector_length+1)) + 2;
				crc = 0x443b; // init value with a1
				pos = 0;
			}
			bytepos++;
			break;

		case FOUND_A1:
			crc = ccitt_crc16_one(crc, byte);
			// osd_printf_verbose("%s: MFM HD: Byte = %02x, CRC=%04x\n", tag(), byte, crc);

			// Put byte into buffer
			// but not the data mark and the CRC
			if (search_header || (count > 2 &&  count < sector_length+3)) buffer[pos++] = byte;

			// Stop counting gap1
			if (search_header && countgap1)
			{
				gap1 = bytepos-1;
				countgap1 = false;
			}

			if (--count == 0)
			{
				if (crc==0)
				{
					if (search_header)
					{
						// Found a header
						ident = buffer[0];
						cylinder = buffer[1];
						// For non-PC-AT formats, highest three bits are in the head field
						if (m_param.headerlen == 5) cylinder |= ((buffer[2]&0x70)<<4);
						else
						{
							osd_printf_verbose("%s: Unexpected header size: %d, cylinder=%d, position=%04x\n", tag(), m_param.headerlen, cylinder, bytepos);
							showtrack(trackimage, tracksize);
						}

						head = buffer[2] & 0x0f;
						sector = buffer[3];
						int identexp = cylinder_to_ident(cylinder);

						if (identexp != ident)
						{
							osd_printf_verbose("%s: Field error; ident = %02x (expected %02x) for sector chs=(%d,%d,%d)\n", tag(), ident, identexp, cylinder, head, sector);
						}

						if (cylinder != current_cylinder)
						{
							osd_printf_verbose("%s: Sector header of sector %d defines cylinder = %02x (should be %02x)\n", tag(), sector, cylinder, current_cylinder);
						}

						if (head != current_head)
						{
							osd_printf_verbose("%s: Sector header of sector %d defines head = %02x (should be %02x)\n", tag(), sector, head, current_head);
						}

						// Check skew
						// We compare the beginning of this track with the track on the next head and the track on the next cylinder
						if (check_skew && cylinder < 2 && head < 2)
						{
							m_secnumber[cylinder*2 + head] = sector;
							check_skew=false;
						}

						// Count the sectors for the interleave
						if (check_interleave)
						{
							if (interleave_prec == -1) interleave_prec = sector;
							else
							{
								if (sector == interleave_prec+1) check_interleave = false;
								interleave++;
							}
						}

						if (interleave == 0) interleave = sector - buffer[3];

						// When we have 4-byte headers, the sector length is 512 bytes
						if (m_param.headerlen == 5)
						{
							size = buffer[4];
							sector_length = 128 << (size&0x07);
							ecctype = (size&0xf0)>>4;
						}

						search_header = false;
						if (TRACE_DETAIL) osd_printf_verbose("%s: Found sector chs=(%d,%d,%d)\n", tag(), cylinder, head, sector);
						headerpos = pos;
						// Start the GAP2 counter (if not already determined)
						if (m_param.gap2==0) countgap2 = true;
					}
					else
					{
						// Sector contents
						// Write the sectors to the CHD
						int lbaposition = chs_to_lba(cylinder, head, sector);
						if (lbaposition>=0)
						{
							if (TRACE_DETAIL) osd_printf_verbose("%s: Writing sector chs=(%d,%d,%d) to CHD\n", tag(), current_cylinder, current_head, sector);
							chdstate = chdfile->write_units(chs_to_lba(current_cylinder, current_head, sector), buffer);

							if (chdstate != CHDERR_NONE)
							{
								osd_printf_verbose("%s: Write error while writing sector chs=(%d,%d,%d)\n", tag(), cylinder, head, sector);
							}
						}
						else
						{
							osd_printf_verbose("%s: Invalid CHS data in track image: (%d,%d,%d); not saving to CHD\n", tag(), cylinder, head, sector);
						}
						if (m_param.gap3==0) countgap3 = true;
						search_header = true;
					}
				}
				else
				{
					// Let's test for a 5-byte header
					if (search_header && m_param.headerlen==4 && current_cylinder==0 && current_head==0)
					{
						if (TRACE_DETAIL) osd_printf_verbose("%s: CRC error for 4-byte header; trying 5 bytes\n", tag());
						m_param.headerlen=5;
						count = 1;
						bytepos++;
						break;
					}
					else
					{
						osd_printf_verbose("%s: CRC error in %s of (%d,%d,%d)\n", tag(), search_header? "header" : "data", cylinder, head, sector);
						search_header = true;
					}
				}
				// search next A1
				state = SEARCH_A1;

				if (!search_header && (pos - headerpos) > 30)
				{
					osd_printf_verbose("%s: Error; missing DAM; searching next header\n", tag());
					search_header = true;
				}
			}
			bytepos++;
			break;
		}
	}

	if (check_interleave == false && save_param(MFMHD_IL))
	{
		// Successfully determined the interleave
		m_param.interleave = interleave;
		if (TRACE_FORMAT)
			if (current_cylinder==0 && current_head==0) osd_printf_verbose("%s: Determined interleave = %d\n", tag(), m_param.interleave);
	}

	if (check_skew == false)
	{
		if (m_secnumber[0] != -1)
		{
			if (m_secnumber[1] != -1)
			{
				if (save_param(MFMHD_HSKEW)) m_param.headskew = m_secnumber[1]-m_secnumber[0];
				if (TRACE_FORMAT) osd_printf_verbose("%s: Determined head skew = %d\n", tag(), m_param.headskew);
			}
			if (m_secnumber[2] != -1)
			{
				if (save_param(MFMHD_CSKEW)) m_param.cylskew = m_secnumber[2]-m_secnumber[0];
				if (TRACE_FORMAT) osd_printf_verbose("%s: Determined cylinder skew = %d\n", tag(), m_param.cylskew);
			}
		}
	}

	gap1 -= m_param.sync;
	ecctype = -1;   // lock to CRC until we have a support for ECC

	if (current_cylinder==0 && current_head==0)
	{
		// If we want to detect gaps, store the new value into the param object
		// The other gaps have already been written directly to the param object above,
		// unless save_param returned false (or we were not on cylinder 0, head 0)
		if (save_param(MFMHD_GAP1)) m_param.gap1 = gap1;
		if (save_param(MFMHD_ECC)) m_param.ecctype = ecctype;
	}
	return chdstate;
}

/*
    Deliver default values.
*/
int mfmhd_generic_format::get_default(mfmhd_param_t type)
{
	switch (type)
	{
	case MFMHD_IL: return 4;
	case MFMHD_HSKEW:
	case MFMHD_CSKEW: return 0;
	case MFMHD_WPCOM:               // Write precompensation cylinder (-1 = none)
	case MFMHD_RWC: return -1;      // Reduced write current cylinder (-1 = none)
	case MFMHD_GAP1: return 16;
	case MFMHD_GAP2: return 3;
	case MFMHD_GAP3: return 18;
	case MFMHD_SYNC: return 13;
	case MFMHD_HLEN: return 5;
	case MFMHD_ECC: return -1;      // -1: use CRC instead of ECC
	}
	return -1;
}
