// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*********************************************************************
 *
 * TI-99 family disk images
 *
 * used by TI-99/4, TI-99/4A, TI-99/8, SGCPU ("TI-99/4P"), and Geneve
 *
 * Sector Dump Format, aka "v9t9" format
 *    customized wd177x_format
 *    no track data
 *
 * Track Dump Format, aka "pc99" format
 *    contains all track information, but no clock patterns
 *
 * Both formats allow for a broad range of medium sizes. All sectors are 256
 * bytes long. The most common formats are 9 sectors per track, single-sided,
 * 40 tracks, which yields 90 KiB of sector data (known as SSSD), and 18
 * sectors per track, double-sided, and 40 tracks, which is 360 KiB (known as
 * DSDD). There are rare occurances of 8/16 sectors/track
 * (prototypical TI double-density controller) and 35 track media. Newer
 * controllers and ROMs allow for up to 36 sectors per track and 80 tracks on
 * both sides, which is 1,44 MiB (DSHD80).
 *
 * Double stepping
 * --------------
 * When using a 40-track disk in an 80-track drive, it seems as if each
 * track appears twice in sequence (0,0,1,1,2,2,...,39,39).
 * The system will write to each second track and leave the others untouched.
 * When we write back that image, there is no simple way to know that
 * the 80 tracks are in fact doubled 40 tracks. We will actually write
 * all 80 tracks, doubling the size of the image file. This disk image will
 * now become unusable in a 40-track drive - which is exactly what happens in reality.
 *
 * Michael Zapf, July 2015
 *
 ********************************************************************/

#include <string.h>
#include <time.h>
#include <assert.h>

#include "emu.h" // logerror
#include "imageutl.h"
#include "ti99_dsk.h"

#define SECTOR_SIZE 256

// Debugging
#define TRACE 0

// ====================================================
//  Common methods for both formats.
// ====================================================

/*
    Find out whether there are IDAMs and DAMs (without having clock bits).
    As said above, let's not spend too much effort allowing format deviations.
    If the image does not exactly adhere to the format, give up.
*/
bool ti99_floppy_format::check_for_address_marks(UINT8* trackdata, int encoding)
{
	int i=0;

	if (encoding==floppy_image::FM)
	{
		// Check 5 sectors of track 0
		while (i < 5)
		{
			if (trackdata[16 + 6 + i*334] != 0xfe) break;
			if (trackdata[16 + 30 + i*334] != 0xfb && trackdata[16 + 30 + i*334] != 0xf8) break;
			i++;
		}
	}
	else
	{
		// Try MFM
		i = 0;
		while (i < 5)
		{
			if (trackdata[40 + 13 + i*340] != 0xfe) break;
			if (trackdata[40 + 57 + i*340] != 0xfb && trackdata[40 + 57 + i*334] != 0xf8) break;
			i++;
		}
	}
	return (i==5);
}

int ti99_floppy_format::get_encoding(int cell_size)
{
	return (cell_size==4000)? floppy_image::FM : floppy_image::MFM;
}

/*
    Load the image from disk and convert it into a sequence of flux levels.
*/
bool ti99_floppy_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int cell_size = 0;
	int sector_count = 0;
	int heads = 0;
	determine_sizes(io, cell_size, sector_count, heads);

	if (cell_size == 0) return false;

	// Be ready to hold a track of up to 36 sectors (with gaps and marks)
	UINT8 trackdata[13000];

	int maxtrack, maxheads;
	image->get_maximal_geometry(maxtrack, maxheads);

	int file_size = io_generic_size(io);
	int track_size = get_track_size(cell_size, sector_count);
	int track_count = file_size / (track_size*heads);

	if (TRACE) logerror("ti99_dsk: track count = %d\n", track_count);

	if (track_count > maxtrack)
	{
		logerror("ti99_dsk: Floppy disk has too many tracks for this drive.\n");
		return false;
	}

	bool doubletracks = (track_count * 2 <= maxtrack);

	if (doubletracks) logerror("ti99_dsk: 40-track image in an 80-track drive. On save, image size will double.\n");

	// Read the image
	for(int head=0; head < heads; head++)
	{
		for(int track=0; track < track_count; track++)
		{
			load_track(io, trackdata, head, track, sector_count, track_count, cell_size);

			if (doubletracks)
			{
				// Create two tracks from each medium track. This reflects the
				// fact that the drive head will find the same data after
				// a single step
				if (get_encoding(cell_size)==floppy_image::FM)
				{
					generate_track_fm(track*2, head, cell_size, trackdata, image);
					generate_track_fm(track*2+1, head, cell_size, trackdata, image);
				}
				else
				{
					generate_track_mfm(track*2, head, cell_size, trackdata, image);
					generate_track_mfm(track*2+1, head, cell_size, trackdata, image);
				}
			}
			else
			{
				// Normal tracks
				if (get_encoding(cell_size)==floppy_image::FM)
					generate_track_fm(track, head, cell_size, trackdata, image);
				else
					generate_track_mfm(track, head, cell_size, trackdata, image);
			}
		}
	}
	return true;
}

bool ti99_floppy_format::save(io_generic *io, floppy_image *image)
{
	int act_track_size = 0;

	UINT8 bitstream[500000/8];
	UINT8 trackdata[9216];   // max size

	int cellsizes[] = { 2000, 4000, 1000, 2000 };

	// Do we use double-stepping?
	// If our image was loaded into a 80-track drive, we will always write 80 tracks.
	int maxtrack, maxheads;
	image->get_maximal_geometry(maxtrack, maxheads);

	if (maxtrack > 80) maxtrack = 80;
	else
	{
		if (maxtrack > 35 && maxtrack < 80) maxtrack = 40;
		else maxtrack = 35;
	}

	int attempt = 0;
	int sector[36];
	int maxsect = 18;
	bool write = true;

	// We expect a bitstream of length 50000 for FM and 100000 for MFM
	for(int head=0; head < 2; head++)
	{
		int track = 0;
		while (track < maxtrack)
		{
			int cell_size = cellsizes[attempt];
			int encoding = get_encoding(cell_size);
			int track_size = get_track_size(cell_size, 36); // max number of sectors

			// Retrieve the cells from the flux sequence
			generate_bitstream_from_track(track, head, cell_size, bitstream, act_track_size, image);

			// Maybe the track has become longer due to moving splices
			if (act_track_size > 200000000/cell_size) act_track_size = 200000000/cell_size;

			int marks = decode_bitstream(bitstream, trackdata, sector, act_track_size, encoding, (encoding==floppy_image::FM)? 0xff:0x4e, track_size);

			if (track==0)
			{
				if (head==0)
				{
					// Find the highest sector in the track
					// This is only needed for the SDF format
					int i=35;
					while (i>=0 && sector[i]==-1) i--;

					if (i>18) maxsect = 36;
					else
					{
						if (i>16) maxsect = 18;
						else
						{
							if (i>9) maxsect = 16;
							else maxsect = 9;
						}
					}
					if (TRACE) logerror("ti99_dsk: Sectors/track: %d\n", maxsect);

					// We try different cell sizes until we find a fitting size.
					// If this fails, we fall back to a size of 2000 ns
					// The criterion for a successful decoding is that we find at least
					// 6 ID or data address marks on the track. It is highly unlikely
					// to find so many marks with a wrong cell size.
					if (marks < 6 && attempt < 4)
					{
						if (TRACE) logerror("ti99_dsk: Decoding with cell size %d failed.\n", cell_size);
						attempt++;
						write = false;
					}
					else write = true;
				}
				else
				{
					if (marks < 6)
					{
						if (min_heads()==1)
						{
							if (TRACE) logerror("ti99_dsk: We don't have a second side and the format allows for single-sided recording.\n");
							return true;
						}
						else
						{
							logerror("ti99_dsk: No second side, but this format requires two-sided recording. Saving empty tracks.\n");
						}
					}
				}
			}

			if (write)
			{
				if (TRACE)
				{
					if (head == 0 && track == 0)
					{
						if (marks >=6) { if (TRACE) logerror("ti99_dsk: Decoding with cell size %d successful.\n", cell_size); }
						else logerror("ti99_dsk: No address marks found on track 0. Assuming MFM format.\n");
					}
				}
				// Save to the file
				write_track(io, trackdata, sector, track, head, maxsect, maxtrack, track_size);
				track++;
			}
		}
	}

	return true;
}

void ti99_floppy_format::generate_track_fm(int track, int head, int cell_size, UINT8* trackdata, floppy_image *image)
{
	int track_size_cells = 200000000/cell_size;
	std::vector<UINT32> buffer;

	// The TDF has a long track lead-out that makes the track length sum up
	// to 3253; this is too long for the number of cells in the real track.
	// This was either an error when that format was defined,
	// or it is due to the fact that when reading a track via
	// the controller, after the track has been read, the controller still
	// delivers some FF values until it times out.

	// Accordingly, we limit the track size to cell_number / 16,
	// which means 3125 for FM
	// This also means that Gap 4 (lead-out) is not 231 bytes long but only 103 bytes

	int track_size = track_size_cells / 16;

	short crc1, crc2, found_crc;
	int start = 16;

	if (check_for_address_marks(trackdata, floppy_image::FM)==false)
	{
		if (head==0 && track==0) logerror("ti99_dsk: Cannot find FM address marks on track %d, head %d; likely broken or unformatted.\n", track, head);
		return;
	}

	// Found a track; we now know where the address marks are:
	// (start is positioned at the pre-id gap)
	// IDAM: start + 6 + n*334
	// DAM:  start + 30 + n*334
	// and the CRCs are at
	// CRC1: start + 11 + n*334
	// CRC2: start + 287 + n*334
	// If the CRCs are F7F7, we recalculate them.
	for (int i=0; i < track_size; i++)
	{
		if (((i-start-6)%334==0) && (i < start + 9*334))
		{
			// IDAM
			raw_w(buffer, 16, 0xf57e);
		}
		else
		{
			if (((i-start-30)%334==0) && (i < start + 9*334))
			{
				// DAM
				raw_w(buffer, 16, 0xf56f);
			}
			else
			{
				if (((i-start-11)%334==0) && (i < start + 9*334))
				{
					// CRC1
					crc1 = ccitt_crc16(0xffff, &trackdata[i-5], 5);
					found_crc = (trackdata[i]<<8 | trackdata[i+1]);
					if ((found_crc & 0xffff) == 0xf7f7)
					{
						// PC99 format: no real CRC; replace it
						// Also, when converting from SDF we let this method create the proper CRC.
						// logerror("Warning: PC99 format using pseudo CRC1; replace by %04x\n", crc1);
						trackdata[i] = (crc1 >> 8) & 0xff;
						trackdata[i+1] = crc1 & 0xff;
						found_crc = crc1;
					}
					if (crc1 != found_crc)
					{
						logerror("ti99_dsk: Warning: CRC1 does not match (track=%d, head=%d). Found = %04x, calc = %04x\n", track, head, found_crc& 0xffff, crc1& 0xffff);
					}
				}
				else
				{
					if (((i-start-287)%334==0) && (i < start + 9*334))
					{
						// CRC2
						crc2 = ccitt_crc16(0xffff, &trackdata[i-SECTOR_SIZE-1], SECTOR_SIZE+1);
						found_crc = (trackdata[i]<<8 | trackdata[i+1]);
						if ((found_crc & 0xffff) == 0xf7f7)
						{
							// PC99 format: no real CRC; replace it
							// logerror("Warning: PC99 format using pseudo CRC2; replace by %04x\n", crc2);
							trackdata[i] = (crc2 >> 8) & 0xff;
							trackdata[i+1] = crc2 & 0xff;
							found_crc = crc2;
						}
						if (crc2 != found_crc)
						{
							logerror("ti99_dsk: Warning: CRC2 does not match (track=%d, head=%d). Found = %04x, calc = %04x\n", track, head, found_crc& 0xffff, crc2& 0xffff);
						}
					}
				}
				fm_w(buffer, 8, trackdata[i]);
			}
		}
	}

	generate_track_from_levels(track, head, buffer, 0, image);
}

void ti99_floppy_format::generate_track_mfm(int track, int head, int cell_size, UINT8* trackdata, floppy_image *image)
{
	int track_size_cells = 200000000/cell_size;
	std::vector<UINT32> buffer;

	// See above
	// We limit the track size to cell_number / 16, which means 6250 for MFM
	// Here, Gap 4 is actually only 90 bytes long
	// (not 712 as specified in the TDF format)
	int track_size = track_size_cells / 16;

	short crc1, crc2, found_crc;
	int start = 40;

	if (check_for_address_marks(trackdata, floppy_image::MFM)==false)
	{
		if (track==0 && head==0) logerror("ti99_dsk: Cannot find MFM address marks on track %d, head %d; likely broken or unformatted.\n", track, head);
		return;
	}

	// Found a track; we now know where the address marks are:
	// (start is positioned at the pre-id gap)
	// IDAM: start + 10 + n*340 (starting at first a1)
	// DAM:  start + 54 + n*340 (starting at first a1)
	// and the CRCs are at
	// CRC1: start + 18 + n*340
	// CRC2: start + 314 + n*334

	for (int i=0; i < track_size; i++)
	{
		if (((i-start-10)%340==0) && (i < start + 18*340))
		{
			// IDAM
			for (int j=0; j < 3; j++)
				raw_w(buffer, 16, 0x4489);  // 3 times A1
			mfm_w(buffer, 8, 0xfe);
			i += 3;
		}
		else
		{
			if (((i-start-54)%340==0) && (i < start + 18*340))
			{
				// DAM
				for (int j=0; j < 3; j++)
					raw_w(buffer, 16, 0x4489);  // 3 times A1
				mfm_w(buffer, 8, 0xfb);
				i += 3;
			}
			else
			{
				if (((i-start-18)%340==0) && (i < start + 18*340))
				{
					// CRC1
					// The CRC also covers the three A1 bytes!
					crc1 = ccitt_crc16(0xffff, &trackdata[i-8], 8);
					found_crc = (trackdata[i]<<8 | trackdata[i+1]);
					if ((found_crc & 0xffff) == 0xf7f7)
					{
						// PC99 format: pseudo CRC; replace it
						// logerror("Warning: PC99 format using pseudo CRC1; replace by %04x\n", crc1);
						trackdata[i] = (crc1 >> 8) & 0xff;
						trackdata[i+1] = crc1 & 0xff;
						found_crc = crc1;
					}
					if (crc1 != found_crc)
					{
						logerror("ti99_dsk: Warning: CRC1 does not match (track=%d, head=%d). Found = %04x, calc = %04x\n", track, head, found_crc & 0xffff, crc1& 0xffff);
					}
				}
				else
				{
					if ((i > 340) && ((i-start-314)%340==0) && (i < start + 18*340))
					{
						// CRC2
						crc2 = ccitt_crc16(0xffff, &trackdata[i-SECTOR_SIZE-4], SECTOR_SIZE+4);
						found_crc = (trackdata[i]<<8 | trackdata[i+1]);
						if ((found_crc & 0xffff) == 0xf7f7)
						{
							// PC99 format: pseudo CRC; replace it
							// logerror("Warning: PC99 format using pseudo CRC2; replace by %04x\n", crc2);
							trackdata[i] = (crc2 >> 8) & 0xff;
							trackdata[i+1] = crc2 & 0xff;
							found_crc = crc2;
						}
						if (crc2 != found_crc)
						{
							logerror("ti99_dsk: Warning: CRC2 does not match (track=%d, head=%d). Found = %04x, calc = %04x\n", track, head, found_crc& 0xffff,  crc2& 0xffff);
						}
					}
				}
				mfm_w(buffer, 8, trackdata[i]);
			}
		}
	}

	generate_track_from_levels(track, head, buffer, 0, image);
}

// States for decoding the bitstream
enum
{
	FMIDAM,
	MFMIDAM,
	HEADER,
	FMDAM,
	MFMDAM,
	DATA
};

/*
    Decodes the bitstream into a TDF track image.
    Returns the number of detected marks.
*/
int ti99_floppy_format::decode_bitstream(const UINT8 *bitstream, UINT8 *trackdata, int* sector, int cell_count, int encoding, UINT8 gapbytes, int track_size)
{
	int databytes = 0;
	int a1count = 0;
	int lastpos = 0;
	int headerbytes = 0;
	int curpos = 0;
	UINT8 curbyte = 0;
	UINT16 shift_reg = 0;
	int tpos = 0;
	int pos = 0;
	int state;
	int marks = 0;
	int current_sector = 0;

	// Init track
	memset(trackdata, 0x00, track_size);

	for (int i=0; i < 36; i++) sector[i] = -1;

	state = (encoding==floppy_image::MFM)? MFMIDAM : FMIDAM;

	while (pos < cell_count && tpos < track_size)
	{
		shift_reg = (shift_reg << 1) & 0xffff;
		if ((pos & 0x07)==0) curbyte = bitstream[curpos++];
		if ((curbyte & 0x80) != 0) shift_reg |= 1;
		curbyte <<= 1;

		switch (state)
		{
		case FMIDAM:
			if (shift_reg == 0xf57e)
			{
				marks++;
				// Found a header
				headerbytes = 5;
				// Create GAP0 at the beginning or GAP3 later
				if (tpos==0)
					tpos += 16;
				else
					for (int i=0; i < 45; i++) trackdata[tpos++] = gapbytes;

				// IDAM sync bytes
				tpos += 6;
				trackdata[tpos++] = 0xfe;
				state = HEADER;
				lastpos = pos;
			}
			break;

		case MFMIDAM:
			// Count three subsequent A1
			if (shift_reg == 0x4489)
			{
				if (lastpos > 0)
				{
					if (pos - lastpos == 16) a1count++;
					else a1count = 1;
				}
				else a1count = 1;

				lastpos = pos;
			}
			if (a1count == 3)
			{
				marks++;
				// Found a header
				headerbytes = 6;
				// Create GAP0 at the beginning or GAP3 later
				if (tpos==0)
					for (int i=0; i < 40; i++) trackdata[tpos++] = gapbytes;
				else
					for (int i=0; i < 24; i++) trackdata[tpos++] = gapbytes;

				// IDAM sync bytes
				tpos += 10;
				state = HEADER;
				trackdata[tpos++] = 0xa1;
				trackdata[tpos++] = 0xa1;
				trackdata[tpos++] = 0xa1;
			}
			break;

		case HEADER:
			if (pos - lastpos == 16)
			{
				// Transfer header bytes
				trackdata[tpos] = get_data_from_encoding(shift_reg);

				if (headerbytes == 3) current_sector = trackdata[tpos];
				tpos++;

				if (headerbytes == 0)
				{
					state = (encoding==floppy_image::MFM)? MFMDAM : FMDAM;
					a1count = 0;
				}
				else headerbytes--;
				lastpos = pos;
			}
			break;

		case FMDAM:
			if (shift_reg == 0xf56a || shift_reg == 0xf56f)
			{
				if (pos - lastpos > 400)
				{
					// Too far apart
					state = FMIDAM;
					// Abort this sector, skip to the next
					tpos += 321;
					a1count = 1;
					break;
				}
				marks++;
				// Add GAP2 and DAM sync
				for (int i=0; i < 11; i++) trackdata[tpos++] = 0xff;
				tpos += 6;
				state = DATA;
				databytes = 257;
				trackdata[tpos++] = (shift_reg==0xf56a)? 0xf8 : 0xfb;
				lastpos = pos;
				sector[current_sector] = tpos;
			}
			break;

		case MFMDAM:
			// Count three subsequent A1
			if (shift_reg == 0x4489)
			{
				if (pos - lastpos > 560)
				{
					// Too far apart
					state = MFMIDAM;
					// Abort this sector, skip to the next
					tpos += 320;
					a1count = 1;
					break;
				}

				if (lastpos > 0)
				{
					if (pos - lastpos == 16) a1count++;
					else a1count = 1;
				}
				else a1count = 1;

				lastpos = pos;
			}
			if (a1count == 3)
			{
				marks++;
				// Add GAP2 and DAM sync
				for (int i=0; i < 22; i++) trackdata[tpos++] = gapbytes;
				tpos += 12;
				trackdata[tpos++] = 0xa1;
				trackdata[tpos++] = 0xa1;
				trackdata[tpos++] = 0xa1;
				state = DATA;
				databytes = 258;
				sector[current_sector] = tpos+1;
			}
			break;

		case DATA:
			if (pos - lastpos == 16)
			{
				// Ident byte
				trackdata[tpos++] = get_data_from_encoding(shift_reg);
				if (databytes > 0) databytes--;
				else
				{
					state = (encoding==floppy_image::MFM)? MFMIDAM : FMIDAM;
					a1count = 0;
				}
				lastpos = pos;
			}
			break;
		}
		pos++;
	}
	return marks;
}

UINT8 ti99_floppy_format::get_data_from_encoding(UINT16 raw)
{
	return (raw & 0x4000 ? 0x80 : 0x00) |
			(raw & 0x1000 ? 0x40 : 0x00) |
			(raw & 0x0400 ? 0x20 : 0x00) |
			(raw & 0x0100 ? 0x10 : 0x00) |
			(raw & 0x0040 ? 0x08 : 0x00) |
			(raw & 0x0010 ? 0x04 : 0x00) |
			(raw & 0x0004 ? 0x02 : 0x00) |
			(raw & 0x0001 ? 0x01 : 0x00);
}

/*
    Sector Dump Format
    ------------------
    The Sector Dump Format is also known as v9t9 format (named after the first
    TI emulator to use this format). It is a contiguous sequence of sector
    contents without track data. The first sector of the disk is located at
    the start of the image, while the last sector is at its end. The sectors
    are ordered by their logical number as used in the TI file system.

    In this implementation, the sector dump format is just a kind of
    wd177x_format with minor customizations, which allows us to keep the code
    small. The difference is the logical ordering of tracks and sectors.

    The TI file system orders all tracks on side 0 as going inwards,
    and then all tracks on side 1 going outwards.

        00 01 02 03 ... 38 39     side 0
        79 78 77 76 ... 41 40     side 1

    The SDF format stores the tracks and their sectors in logical order
        00 01 02 03 ... 38 39 [40 41 ... 79]

    There is also a variant of the SDF which adds three sectors at the end
    containing a map of bad sectors. This was introduced by a tool to read
    real TI floppy disks on a PC. As other emulators tolerate this additional
    bad sector map, we just check whether there are 3 more sectors and ignore
    them.
*/
const char *ti99_sdf_format::name() const
{
	return "sdf";
}

const char *ti99_sdf_format::description() const
{
	return "TI99 sector dump floppy disk image";
}

const char *ti99_sdf_format::extensions() const
{
	return "dsk";
}

int ti99_sdf_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT64 file_size = io_generic_size(io);
	int vote = 0;

	// Adding support for another sector image format which adds 768 bytes
	// as a bad sector map
	if ((file_size / SECTOR_SIZE) % 10 == 3)
	{
		if (TRACE) logerror("ti99_dsk: Stripping map of bad sectors at image end\n");
		file_size -= SECTOR_SIZE*3;
	}

	// Formats with 9 or 18 sectors per track (multiples of SSSD)
	// 40-track formats: SSSD/9/40 (1), DSSD/9/40 (2), SSDD/18/40 (2), DSDD/18/40 (4)
	// 80-track formats: SSSD/9/80 (2), DSSD/9/80 (4), SSDD/18/80 (4), DSDD/18/80,(8)
	// High density: DSQD/36/80 (16) (experimental)

	// All formats apply for 5.25" and 3.5" form factors

	// Default formats (when the geometry cannot be determined from the VIB)
	// (1)  -> SSSD
	// (2)  -> DSSD
	// (4)  -> DSDD
	// (8)  -> DSDD80
	// (16) -> DSQD

	if ((file_size % 92160)==0)
	{
		int multiple = file_size/92160;
		if ((multiple == 1) || (multiple == 2) || (multiple == 4) || (multiple == 8) || (multiple == 16)) vote = 50;
	}

	// Formats with 16 sectors per track (rare, SSDD/16/40, DSDD/16/40).
	if (file_size == 163840) vote = 50;
	if (file_size == 327680) vote = 50;

	if (vote > 0)
	{
		// Read first sector (Volume Information Block)
		ti99vib vib;
		io_generic_read(io, &vib, 0, sizeof(ti99vib));

		// Check from contents
		if ((vib.id[0]=='D')&&(vib.id[1]=='S')&&(vib.id[2]=='K'))
		{
			if (TRACE) logerror("ti99_dsk: Found formatted SDF disk medium\n");
			vote = 100;
		}
		else
		{
			if (TRACE) logerror("ti99_dsk: No valid VIB found; disk may be unformatted\n");
		}
	}
	else if (TRACE) logerror("ti99_dsk: Disk image is not a SDF image\n");
	return vote;
}

void ti99_sdf_format::determine_sizes(io_generic *io, int& cell_size, int& sector_count, int& heads)
{
	UINT64 file_size = io_generic_size(io);
	ti99vib vib;

	cell_size = 0;
	sector_count = 0;
	heads = 2;

	bool have_vib = false;

	// See above
	if ((file_size / SECTOR_SIZE) % 10 == 3) file_size -= SECTOR_SIZE*3;

	// Read first sector
	io_generic_read(io, &vib, 0, sizeof(ti99vib));

	// Check from contents
	if ((vib.id[0]=='D')&&(vib.id[1]=='S')&&(vib.id[2]=='K'))
	{
		sector_count = vib.secspertrack;
		heads = vib.sides;

		// Find out more about the density. SSDD happens to be the same size
		// as DSSD in the sector dump format, so we need to ask the
		// VIB if available. Otherwise, we assume that we have a DSSD medium.
		if (vib.density < 2) cell_size = 4000;
		else
		{
			if (vib.density < 4) cell_size = 2000;
			else cell_size = 1000;
		}
		if (TRACE) logerror("ti99_dsk: VIB says that this disk is %s density with %d sectors per track, %d tracks, and %d heads\n", (cell_size==4000)? "single": ((cell_size==2000)? "double" : "high"), sector_count, vib.tracksperside, heads);
		have_vib = true;
	}

	// Do we have a broken VIB? The Pascal disks are known to have such incomplete VIBs
	if (heads == 0 || sector_count == 0) have_vib = false;

	// We're also checking the size of the image
	int cell_size1 = 0;
	int sector_count1 = 0;

	// 90 KiB -> SSSD, 9 sect, FM
	// 160 KiB -> SSDD, 16 sect, MFM
	// 180 KiB -> DSSD, 9 sect, FM
	// 320 KiB -> DSDD, 16 sect, MFM
	// 360 KiB -> DSDD, 18 sect, MFM
	// 720 KiB -> DSDD, 18 sect, MFM, 80 tracks
	// 1440 KiB-> DSQD, 36 sect, MFM, 80 tracks

	if ((file_size == 163840) || (file_size == 327680))
	{
		cell_size1 = 2000;
		sector_count1 = 16;
	}
	else
	{
		if (file_size < 300000) cell_size1 = 4000;
		else
		{
			if (file_size < 1000000) cell_size1 = 2000;
			else cell_size1 = 1000;
		}
		sector_count1 = 36000 / cell_size1;
	}

	if (have_vib)
	{
		if (sector_count == 16 && sector_count1 == 18)
		{
			logerror("ti99_dsk: Warning: Invalid 16-sector format. Assuming 18 sectors.\n");
			sector_count = 18;
		}
		else
		{
			if (heads == 2 && ((cell_size1 != cell_size) || (sector_count1 != sector_count)))
				logerror("ti99_dsk: Warning: Disk image size does not correspond with format information in VIB.\n");
		}
	}
	else
	{
		heads = (file_size < 100000)? 1 : 2;    // for SSSD
		cell_size = cell_size1;
		sector_count = sector_count1;
	}
}

int ti99_sdf_format::get_track_size(int cell_size, int sector_count)
{
	return sector_count * SECTOR_SIZE;
}

/*
    Load a SDF image track. Essentially, we want to end up in the same
    kind of track image as with the TDF, so the easiest thing is to produce
    a TDF image track from the sectors and process them as if it came from TDF.
*/
void ti99_sdf_format::load_track(io_generic *io, UINT8 *trackdata, int head, int track, int sectorcount, int trackcount, int cellsize)
{
	bool fm = (cellsize==4000);
	int tracksize = sectorcount * SECTOR_SIZE;

	// Calculate the track offset from the beginning of the image file
	int logicaltrack = head * trackcount;
	logicaltrack += ((head&1)==0)?  track : (trackcount - 1 - track);

	// Interleave and skew
	int interleave = fm? 4 : 5;
	int skew = fm? 6 : 0;

	int secsize = fm? 334 : 340;
	int position = 0;
	int count = 0;

	memset(trackdata, 0x00, 9216);

	int secno = 0;
	secno = (track * skew) % sectorcount;

	// Gap 1
	int gap1 = fm? 16 : 40;
	for (int i=0; i < gap1; i++) trackdata[position+i] = fm? 0x00 : 0x4e;

	for (int i=0; i < sectorcount; i++)
	{
		position = secno * secsize + gap1;
		// Sync
		count = fm? 6 : 10;
		while (count-- > 0) trackdata[position++] = 0x00;

		if (!fm)
		{
			trackdata[position++] = 0xa1;
			trackdata[position++] = 0xa1;
			trackdata[position++] = 0xa1;
		}
		trackdata[position++] = 0xfe;   // IDAM / ident

		// Header
		trackdata[position++] = track;
		trackdata[position++] = head;
		trackdata[position++] = i;
		trackdata[position++] = 1;

		trackdata[position++] = 0xf7;
		trackdata[position++] = 0xf7;

		// Gap 2
		count = fm? 11 : 22;
		while (count-- > 0) trackdata[position++] = fm? 0xff : 0x4e;

		// Sync
		count = fm? 6 : 12;
		while (count-- > 0) trackdata[position++] = 0x00;

		if (!fm)
		{
			trackdata[position++] = 0xa1;
			trackdata[position++] = 0xa1;
			trackdata[position++] = 0xa1;
		}
		trackdata[position++] = 0xfb;

		io_generic_read(io, trackdata + position, logicaltrack * tracksize + i*SECTOR_SIZE, SECTOR_SIZE);

		position += SECTOR_SIZE;
		trackdata[position++] = 0xf7;
		trackdata[position++] = 0xf7;

		// Gap 3
		count = fm? 45 : 24;
		while (count-- > 0) trackdata[position++] = fm? 0xff : 0x4e;
		secno = (secno + interleave) % sectorcount;
	}

	// Gap 4
	count = fm? 231 : 712;
	position = sectorcount * secsize + gap1;
	while (count-- > 0) trackdata[position++] = fm? 0xff : 0x4e;

	// if (head==0 && track==0) showtrack(trackdata, 9216);
}

/*
    For debugging. Outputs the byte array in a xxd-like way.
*/
void ti99_floppy_format::showtrack(UINT8* trackdata, int length)
{
	for (int i=0; i < length; i+=16)
	{
		logerror("%07x: ", i);
		for (int j=0; j < 16; j++)
		{
			logerror("%02x", trackdata[i+j]);
			if ((j&1)==1) logerror(" ");
		}
		logerror(" ");
		for (int j=0; j < 16; j++)
		{
			if (trackdata[i+j] >= 32 && trackdata[i+j]<128) logerror("%c", trackdata[i+j]);
			else logerror(".");
		}
		logerror("\n");
	}
}

/*
    Write the data to the disk. We have a list of sector positions, so we
    just need to go through that list and save each sector in the track data.
*/
void ti99_sdf_format::write_track(io_generic *io, UINT8 *trackdata, int *sector, int track, int head, int maxsect, int maxtrack, int numbytes)
{
	int logicaltrack = head * maxtrack;
	logicaltrack += ((head&1)==0)?  track : (maxtrack - 1 - track);
	int trackoffset = logicaltrack * maxsect * SECTOR_SIZE;

//  if (head==0 && track==0) showtrack(trackdata, 9216);

	for (int i=0; i < maxsect; i++)
		io_generic_write(io, trackdata + sector[i], trackoffset + i * SECTOR_SIZE, SECTOR_SIZE);
}

const floppy_format_type FLOPPY_TI99_SDF_FORMAT = &floppy_image_format_creator<ti99_sdf_format>;

/*
    Track Dump Format
    -----------------
    The Track Dump Format is also known as pc99 (again, named after the first
    TI emulator to use this format). It is a contiguous sequence of track
    contents, containing all information including the data values of the
    address marks and CRC, but it does not contain clock signals.
    Therefore, the format requires that the address marks be at the same
    positions within each track.

    Different to earlier implementations of the TDF format, we stay much
    closer to the specification. Deviations (like different gap sizes) are
    automatically rectified. It does not make sense to have a format that
    pretends to be an almost precise track image and then fail to load in
    other emulations because of small deviations.
    For precise track images there are better suited formats.

    For FM recording, each track is exactly 3253 bytes long in this format.
    For MFM recording, track length is 6872 bytes.

    Accordingly, we get a multiple of these values as the image length.
    There are no single-sided images (when single-sided, the upper half of
    the image is empty).

    DSSD: 260240 bytes
    DSDD: 549760 bytes

    We do not support other geometries in this format. One exception: We accept
    images of double size which may be generated when a 40-track disk is
    modified in an 80-track drive. This will automatically cause the creation
    of an 80-track image.

    Tracks are stored from outside to inside of head 0, then outside to inside of head 1:

    (Head,Track): (0,0) (0,1) (0,2) ... (0,38) (0,39) (1,0) (1,1) ... (1,39)
*/
const char *ti99_tdf_format::name() const
{
	return "tdf";
}

const char *ti99_tdf_format::description() const
{
	return "TI99 track dump floppy disk image";
}

const char *ti99_tdf_format::extensions() const
{
	return "dsk,dtk";
}

/*
    Determine whether the image file can be interpreted as a track dump
*/
int ti99_tdf_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT64 file_size = io_generic_size(io);
	int vote = 0;
	UINT8 trackdata[2000];

	// Do we have a plausible image size? From the size alone we only give a 50 percent vote.
	if ((file_size == 260240) || (file_size == 549760) || (file_size == 1099520))
		vote = 50;

	if (vote > 0)
	{
		if (TRACE) logerror("ti99_dsk: Image looks like a TDF image\n");

		// Get some bytes from track 0
		io_generic_read(io, trackdata, 0, 2000);

		if (check_for_address_marks(trackdata, floppy_image::FM)==true || check_for_address_marks(trackdata, floppy_image::MFM)==true) vote=100;
		else
		{
			if (TRACE) logerror("ti99_dsk: Image does not comply with TDF; may be broken or unformatted\n");
		}
	}
	else if (TRACE) logerror("ti99_dsk: Disk image is not a TDF image\n");
	return vote;
}

/*
    Find the proper format for a given image file. We determine the cell size,
    but we do not care about the sector size (only needed by the SDF converter).
*/
void ti99_tdf_format::determine_sizes(io_generic *io, int& cell_size, int& sector_count, int& heads)
{
	UINT64 file_size = io_generic_size(io);
	heads = 2;  // TDF only supports two-sided recordings

	if (file_size % get_track_size(2000, 0)==0) cell_size = 2000;
	else if (file_size % get_track_size(4000, 0)==0) cell_size = 4000;

	// We could check for the content, but if we find that the content is
	// FM and the size implies MFM, the calculated track count will be wrong.
}

/*
    For TDF this just amounts to loading the track from the image file.
*/
void ti99_tdf_format::load_track(io_generic *io, UINT8 *trackdata, int head, int track, int sectorcount, int trackcount, int cellsize)
{
	int offset = ((trackcount * head) + track) * get_track_size(cellsize, 0);
	io_generic_read(io, trackdata, offset, get_track_size(cellsize, 0));
}

/*
    Also here, we just need to write the complete track on the image file.
*/
void ti99_tdf_format::write_track(io_generic *io, UINT8 *trackdata, int *sector, int track, int head, int maxsect, int maxtrack, int numbytes)
{
	int offset = ((maxtrack * head) + track) * numbytes;
	io_generic_write(io, trackdata, offset, numbytes);
}

int ti99_tdf_format::get_track_size(int cell_size, int sector_count)
{
	if (cell_size == 4000) return 3253;
	if (cell_size == 2000) return 6872;
	return 0;
}

const floppy_format_type FLOPPY_TI99_TDF_FORMAT = &floppy_image_format_creator<ti99_tdf_format>;
