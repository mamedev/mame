// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*********************************************************************

  TI-99 family disk images

  used by TI-99/4, TI-99/4A, TI-99/8, SGCPU ("TI-99/4P"), and Geneve

  Sector Dump Format, aka "v9t9" format
     customized wd177x_format
     no track data

  Track Dump Format, aka "pc99" format
     contains all track information, but no clock patterns

  Both formats allow for a broad range of medium sizes. All sectors are 256
  bytes long. The most common formats are 9 sectors per track, single-sided,
  40 tracks, which yields 90 KiB of sector data (known as SSSD), and 18
  sectors per track, double-sided, and 40 tracks, which is 360 KiB (known as
  DSDD). There are rare occurances of 16 sectors/track
  (prototypical TI double-density controller) and 35 track media. Newer
  controllers and ROMs allow for up to 36 sectors per track and 80 tracks on
  both sides, which is 1,44 MiB (DSHD80).

  Double stepping
  --------------
  When using a 40-track disk in an 80-track drive, it seems as if each
  track appears twice in sequence (0,0,1,1,2,2,...,39,39).
  The system will write to each second track and leave the others untouched.
  When we write back that image, there is no simple way to know that
  the 80 tracks are in fact doubled 40 tracks. We will actually write
  all 80 tracks, doubling the size of the image file. This disk image will
  now become unusable in a 40-track drive - which is exactly what happens in reality.

  Michael Zapf, July 2015
  Updated August 2018

   TODO:
   - Support for 77 track images for HX5102

********************************************************************/

#include "ti99_dsk.h"
#include "imageutl.h"

#include "ioprocs.h"

#include "osdcore.h" // osd_printf_* (in osdcore.h)

#include <cstring>
#include <ctime>
#include <iomanip>

#define SECTOR_SIZE 256

#define LOG_OUTPUT_FUNC osd_printf_info

#define LOG_WARN       (1U<<1)   // Warnings
#define LOG_HEADER     (1U<<2)   // Header
#define LOG_SHIFT      (1U<<3)   // Shift register
#define LOG_INTERLEAVE (1U<<4)   // Interleave information
#define LOG_INFO       (1U<<5)   // Disk info
#define LOG_DETAIL     (1U<<6)   // Details
#define LOG_TRACK      (1U<<7)   // Track output

#define VERBOSE ( LOG_WARN )

#define __EMU_H__ // logmacro wasn't really intended to be used outside stuff that uses libemu
#include "logmacro.h"


// ====================================================
//  Common methods for both formats.
// ====================================================

int ti99_floppy_format::get_encoding(int cell_size)
{
	return (cell_size==4000)? floppy_image::FM : floppy_image::MFM;
}

/*
    Load the image from disk and convert it into a sequence of flux levels.
*/
bool ti99_floppy_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	uint64_t file_size;
	if (io.length(file_size))
		return false;

	int cell_size = 0;
	int sector_count = 0;
	int heads = 0;
	int log_track_count = 0;

	bool img_high_tpi = false;
	bool drive_high_tpi = false;
	bool skip_track = false;
	int track_count, head_count;

	// Get information about cell size, sector count, and logical track count
	// (the track count as defined by the file system)
	determine_sizes(io, cell_size, sector_count, heads, log_track_count);

	image->get_maximal_geometry(track_count, head_count);
	drive_high_tpi = (track_count > 44);

	// Depends on the image format (SDF or TDF)
	int track_size = get_track_size(sector_count);

	// Adding support for a variant of the sector image format which adds 768 bytes
	// as a bad sector map. Only applies for SDF; TDF will not yield that result
	if (((file_size - 768) % (SECTOR_SIZE*360)) == 0)
	{
		LOGMASKED(LOG_INFO, "[ti99_dsk] Stripping map of bad sectors at image end\n");
		file_size -= 768;
	}

	// Problem: If the disk is improperly formatted, the track count will be
	// wrong. For instance, a disk could be reformatted to single-side.
	// We assume there is no disk with single side format beyond 40 tracks.
	if ((heads==1) && (file_size > track_size*40))
		heads = 2;

	// TI formats are known as having a constant track size, so the physical
	// track count on the medium can easily be calculated.
	int img_track_count = file_size / (track_size*heads);
	img_high_tpi = (img_track_count > 44);

	// Some disks are known to have an incomplete header.
	// PASCAL disks have a track count of 0.
	if (log_track_count == 0) log_track_count = img_track_count;

	LOGMASKED(LOG_INFO, "[ti99_dsk] logical tracks = %d, image tracks = %d, drive track_count = %d\n", log_track_count, img_track_count, track_count);

	// If the floppy disk has 80 tracks but the drive has 40 tracks, force
	// the floppy to 40 tracks (will make it unreadable)
	if (img_high_tpi && !drive_high_tpi)
	{
		LOGMASKED(LOG_WARN, "[ti99_dsk] Floppy disk has too many tracks for this drive, will skip every second track.\n");
		img_high_tpi = false;
		skip_track = true; // only read the even numbered tracks
	}

	// Is this the first time that this disk is read in an 80-track drive?
	bool double_step = (drive_high_tpi && log_track_count <= 44);
	bool first_time_double = (drive_high_tpi && !img_high_tpi);

	if (double_step)
		LOGMASKED(LOG_INFO, "[ti99_dsk] Applying double stepping\n");

	if (first_time_double)
		LOGMASKED(LOG_WARN, "[ti99_dsk] 40-track image in an 80-track drive. On save, image size will double.\n");

	int trackid;
	// Read the image
	uint8_t sectordata[36*SECTOR_SIZE];
	int sectornmb[36];
	int sectoroff[36];

	for (int head=0; head < heads; head++)
	{
		for (int track=0; track < img_track_count; track++)
		{
			LOGMASKED(LOG_TRACK, "[ti99_dsk] Reading track %d, head %d\n", track, head);
			if (double_step && !first_time_double) trackid = track/2;
			else trackid = track;

			load_track(io, sectordata, sectornmb, sectoroff, head, track, sector_count, img_track_count);

			if (double_step)
			{
				if (first_time_double)
				{
					// Create two tracks from each medium track. This reflects the
					// fact that the drive head will find the same data after
					// a single step
					if (sector_count < 10)
					{
						generate_fm_track_from_sectors(image, sectordata, sector_count, sectornmb, sectoroff, track*2, trackid, head);
						generate_fm_track_from_sectors(image, sectordata, sector_count, sectornmb, sectoroff, track*2+1, trackid, head);
					}
					else
					{
						generate_mfm_track_from_sectors(image, sectordata, sector_count, sectornmb, sectoroff, track*2, trackid, head);
						generate_mfm_track_from_sectors(image, sectordata, sector_count, sectornmb, sectoroff, track*2+1, trackid, head);
					}
				}
				else
				{
					// Reading 80 track image for double stepping
					if (sector_count < 10)
						generate_fm_track_from_sectors(image, sectordata, sector_count, sectornmb, sectoroff, track, trackid, head);
					else
						generate_mfm_track_from_sectors(image, sectordata, sector_count, sectornmb, sectoroff, track, trackid, head);
				}
			}
			else
			{
				// No double stepping now, but maybe the image was created under double stepping
				int drivetrack = (skip_track)? track / 2 : track;

				// Normal tracks
				if (sector_count < 10)
					generate_fm_track_from_sectors(image, sectordata, sector_count, sectornmb, sectoroff, drivetrack, drivetrack, head);
				else
					generate_mfm_track_from_sectors(image, sectordata, sector_count, sectornmb, sectoroff, drivetrack, drivetrack, head);
			}
			if (skip_track) track++;
		}
	}

	return true;
}

/*
    Save all tracks to the image file.
*/
bool ti99_floppy_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	uint8_t sectordata[9216];   // max size (36*256)

	int cellsizes[] = { 2000, 4000, 1000, 2000 };

	// Do we use double-stepping?
	// If our image was loaded into a 80-track drive, we will always write 80 tracks.
	int track_count, head_count;
	image->get_maximal_geometry(track_count, head_count);

	if (track_count > 80) track_count = 80;
	else
	{
		if (track_count > 35 && track_count < 80) track_count = 40;
		else track_count = 35;
	}

	int attempt = 0;
	int sector[36];
	int seccount = 0;

	int head = 0;
	int cell_size = cellsizes[0];
	int encoding = get_encoding(cell_size);
	int expected_sectors = (encoding==floppy_image::MFM)? 16:9;

	while (head < 2)
	{
		for (int track = 0; track < track_count; track++)
		{
			// Retrieve the cells from the flux sequence. This also delivers the actual track size.
			auto bitstream = generate_bitstream_from_track(track, head, cell_size, image);

			LOGMASKED(LOG_DETAIL, "[ti99_dsk] Getting sectors from track %d, head %d\n", track, head);
			seccount = get_sectors(bitstream, encoding, track, head, expected_sectors, sectordata, sector);
			LOGMASKED(LOG_DETAIL, "[ti99_dsk] Seccount = %d\n", seccount);

			// We may have more sectors in MFM (18). This is OK in track 0; otherwise we need to restart the process.
			if (track==0 && head==0)
			{
				if (seccount==18 && expected_sectors==16)
				{
					LOGMASKED(LOG_INFO, "[ti99_dsk] Using full 18 sectors/track format for MFM\n");
					expected_sectors=18;
				}
			}
			else
			{
				if (seccount==18 && expected_sectors==16)
				{
					// This happens when we assumed 16 sectors/track up to here
					// and now the track uses 18 tracks. We need to start over,
					// assuming 18 sectors/track, and adding empty sectors
					// where necessary
					head = -1;
					LOGMASKED(LOG_WARN, "[ti99_dsk] Tracks with variable sector count; assuming 18 sectors/track\n");
					break;
				}
			}

			if (seccount < 3 && track == 0)
			{
				if (head == 0)
				{
					// Bad cell size.
					LOGMASKED(LOG_DETAIL, "[ti99_dsk] Decoding with cell size %d failed.\n", cell_size);
					attempt++;
					if (attempt == 4)
					{
						LOGMASKED(LOG_INFO, "[ti99_dsk] Unsupported cell size or unformatted medium.\n");
						return false;
					}
					else
					{
						// Pick a new cell size
						cell_size = cellsizes[attempt];
						encoding = get_encoding(cell_size);
						expected_sectors = (encoding==floppy_image::MFM)? 16:9;
					}
					head = -1;
					break;
				}
				else
				{
					// head 0 already done, but head 1 failing
					if (min_heads()==1)
					{
						LOGMASKED(LOG_INFO, "[ti99_dsk] Saving as single-sided\n");
						break;
					}
					else
					{
						LOGMASKED(LOG_WARN, "[ti99_dsk] Saving empty tracks on second side\n");
					}
				}
			}

			// Save to the file
			write_track(io, sectordata, sector, track, head, seccount, track_count);
		}
		head++;
	}
	return true;
}

enum
{
	GAP0 = 0, GAP1, GAP2, GAP3, GAPBYTE, SYNC
};

/*
    Build a new track from sector contents.
*/
void ti99_floppy_format::generate_fm_track_from_sectors(floppy_image *image, uint8_t *sectordata, int sector_count, int *sectornmb, int *secoffset, int track, int trackid, int head)
{
	std::vector<uint32_t> buffer;

	const int param[6] = { 10, 26, 11, 28, 0xff, 6 };

	// cell size in ns
	const int cell_size = 4000;
	const int cell_count = 200000000/cell_size;

	int sectorpartlen = 2*param[SYNC] + 10 + param[GAP2] + SECTOR_SIZE + param[GAP3];
	int tracklen = param[GAP1] + sector_count * sectorpartlen;
	if (param[GAP0]!=0) tracklen += param[GAP0] + param[SYNC] + 1;
	// The remaining bytes are filled with the gap byte as gap 4.
	int gap4 = cell_count/16 - tracklen;

	uint16_t crc = 0;

	// Create Gap 0 and Index AM
	for (int i=0; i < param[GAP0]; i++)
		fm_w(buffer, 8, param[GAPBYTE], cell_size);

	for (int i=0; i < param[SYNC]; i++)
		fm_w(buffer, 8, 0x00, cell_size);

	raw_w(buffer, 16, 0xf77a, cell_size);  // IXAM

	// and Gap 1
	for (int i=0; i < param[GAP1]; i++)
		fm_w(buffer, 8, param[GAPBYTE], cell_size);

	// For each sector
	for (int j=0; j < sector_count; j++)
	{
		// If the sector is not available, create an empty space of zeros
		if (sectornmb[j]==-1)
		{
			for (int i=0; i < sectorpartlen; i++)
				raw_w(buffer, 16, 0x0000, cell_size);
		}
		else
		{
			// Sync
			for (int i=0; i < param[SYNC]; i++)
				fm_w(buffer, 8, 0x00, cell_size);
			raw_w(buffer, 16, 0xf57e, cell_size);  // IDAM

			crc = ccitt_crc16_one(0xffff, 0xfe);

			// Header
			fm_w(buffer, 8, trackid, cell_size);
			crc = ccitt_crc16_one(crc, trackid);
			fm_w(buffer, 8, head, cell_size);
			crc = ccitt_crc16_one(crc, head);
			fm_w(buffer, 8, sectornmb[j], cell_size);
			crc = ccitt_crc16_one(crc, sectornmb[j]);
			fm_w(buffer, 8, 0x01, cell_size);
			crc = ccitt_crc16_one(crc, 0x01);
			// CRC
			fm_w(buffer, 8, (crc>>8)&0xff, cell_size);
			fm_w(buffer, 8, crc&0xff, cell_size);

			// Gap 2
			for (int i=0; i < param[GAP2]; i++)
				fm_w(buffer, 8, param[GAPBYTE], cell_size);
			// Sync
			for (int i=0; i < param[SYNC]; i++)
				fm_w(buffer, 8, 0x00, cell_size);
			raw_w(buffer, 16, 0xf56f, cell_size);  // DAM
			crc = ccitt_crc16_one(0xffff, 0xfb);

			// Sector contents
			for (int k=0; k < SECTOR_SIZE; k++)
			{
				fm_w(buffer, 8, sectordata[secoffset[j]+k], cell_size);
				crc = ccitt_crc16_one(crc, sectordata[secoffset[j]+k]);
			}
			// CRC
			fm_w(buffer, 8, (crc>>8)&0xff, cell_size);
			fm_w(buffer, 8, crc&0xff, cell_size);

			// Gap 3
			for (int i=0; i < param[GAP3]; i++)
				fm_w(buffer, 8, param[GAPBYTE], cell_size);
		}
	}

	// and fill the remaining bytes with gap bytes
	for (int i=0; i < gap4; i++)
		fm_w(buffer, 8, param[GAPBYTE], cell_size);

	generate_track_from_levels(track, head, buffer, 0, image);
}

void ti99_floppy_format::generate_mfm_track_from_sectors(floppy_image *image, uint8_t *sectordata, int sector_count, int *sectornmb, int *secoffset, int track, int trackid, int head)
{
	// MFM16:  (80,50,22,50,4e,12)
	// MFM18/36:  (10,30,22,21,4e,12)
	// A1: 0x4489   01 00 01 00 10 *00 10 01
	// C2: 0x5224   01 01 00 10 *00 10 01 00
	std::vector<uint32_t> buffer;

	const int mfm16param[7] = { 80, 50, 22, 50, 0x4e, 12 };
	const int mfm18param[7] = { 10, 30, 22, 21, 0x4e, 12 };
	const int *param = mfm18param;

	// cell size in ns
	int cell_size = 2000;

	if (sector_count < 30)
	{
		cell_size = 2000;
		if (sector_count == 16) param = mfm16param;
	}
	else cell_size = 1000;

	int cell_count = 200000000/cell_size;

	int sectorpartlen = 2*param[SYNC] + 16 + param[GAP2] + SECTOR_SIZE + param[GAP3];
	int tracklen = param[GAP1] + sector_count * sectorpartlen;
	if (param[GAP0]!=0) tracklen += param[GAP0] + param[SYNC] + 4;
	// The remaining bytes are filled with the gap byte as gap 4.
	int gap4 = cell_count/16 - tracklen;

	uint16_t crc = 0;

	// Create Gap 0 and Index AM
	for (int i=0; i < param[GAP0]; i++)
		mfm_w(buffer, 8, param[GAPBYTE], cell_size);

	for (int i=0; i < param[SYNC]; i++)
		mfm_w(buffer, 8, 0x00, cell_size);

	for (int i=0; i < 3; i++)
		raw_w(buffer, 16, 0x5224, cell_size);
	mfm_w(buffer, 8, 0xfc, cell_size);

	// and Gap 1
	for (int i=0; i < param[GAP1]; i++)
		mfm_w(buffer, 8, param[GAPBYTE], cell_size);

	// For each sector
	for (int j=0; j < sector_count; j++)
	{
		// If the sector is not available, create an empty space of zeros
		if (sectornmb[j]==-1)
		{
			for (int i=0; i < sectorpartlen; i++)
				raw_w(buffer, 16, 0x0000, cell_size);
		}
		else
		{
			// Sync
			for (int i=0; i < param[SYNC]; i++)
				mfm_w(buffer, 8, 0x00, cell_size);
			// IDAM
			for (int i=0; i < 3; i++)
				raw_w(buffer, 16, 0x4489, cell_size);
			mfm_w(buffer, 8, 0xfe, cell_size);

			crc = 0xb230; // pre-init including IDAM

			// Header
			mfm_w(buffer, 8, trackid, cell_size);
			crc = ccitt_crc16_one(crc, trackid);
			mfm_w(buffer, 8, head, cell_size);
			crc = ccitt_crc16_one(crc, head);
			mfm_w(buffer, 8, sectornmb[j], cell_size);
			crc = ccitt_crc16_one(crc, sectornmb[j]);
			mfm_w(buffer, 8, 0x01, cell_size);
			crc = ccitt_crc16_one(crc, 0x01);
			// CRC
			mfm_w(buffer, 8, (crc>>8)&0xff, cell_size);
			mfm_w(buffer, 8, crc&0xff, cell_size);

			// Gap 2
			for (int i=0; i < param[GAP2]; i++)
				mfm_w(buffer, 8, param[GAPBYTE], cell_size);
			// Sync
			for (int i=0; i < param[SYNC]; i++)
				mfm_w(buffer, 8, 0x00, cell_size);

			// DAM
			for (int i=0; i < 3; i++)
				raw_w(buffer, 16, 0x4489, cell_size);
			mfm_w(buffer, 8, 0xfb, cell_size);
			crc = 0xe295; // pre-init including DAM

			// Sector contents
			for (int k=0; k < SECTOR_SIZE; k++)
			{
				mfm_w(buffer, 8, sectordata[secoffset[j]+k], cell_size);
				crc = ccitt_crc16_one(crc, sectordata[secoffset[j]+k]);
			}
			// CRC
			mfm_w(buffer, 8, (crc>>8)&0xff, cell_size);
			mfm_w(buffer, 8, crc&0xff, cell_size);

			// Gap 3
			for (int i=0; i < param[GAP3]; i++)
				mfm_w(buffer, 8, param[GAPBYTE], cell_size);
		}
	}
	// and fill the remaining bytes with gap bytes
	for (int i=0; i < gap4; i++)
		mfm_w(buffer, 8, param[GAPBYTE], cell_size);

	generate_track_from_levels(track, head, buffer, 0, image);
}

// States for decoding the bitstream
enum
{
	FMIDAM,
	A1IDAM1,
	A1IDAM2,
	A1IDAM3,
	MFMIDAM,
	MFMIDENT,
	HEADER,
	FMDAM,
	A1DAM1,
	A1DAM2,
	A1DAM3,
	MFMDAM,
	DATA
};

/*
    Retrieve all sectors from the track, given as a bitstream.
    The sectors are assumed to have a length of 256 byte, and their sequence
    is stored in the secnumber array.
*/
int ti99_floppy_format::get_sectors(const std::vector<bool> &bitstream, int encoding, int track, int head, int sectors, uint8_t *sectordata, int *secnumber)
{
	int bitpos = 0;
	int lastpos = 0;
	int spos = 0;
	int seccount = 0;
	uint16_t shift_reg = 0;
	uint8_t databyte = 0;
	uint8_t curbyte = 0;
	int rep = 0;
	int cursect = 0;

	// An array of flags (max 32), set for each found sector
	uint32_t foundsectors = 0;

	int maxsect = (encoding==floppy_image::MFM)? 18 : 9;
	int first = (encoding==floppy_image::MFM)? A1IDAM1 : FMIDAM;
	int state = first;

	while (bitpos < bitstream.size())
	{
		LOGMASKED(LOG_SHIFT, "[ti99_dsk] shift = %04x\n", shift_reg);
		shift_reg = (shift_reg << 1) | bitstream[bitpos];

		if (((bitpos - lastpos) & 1)==0)
		{
			databyte = (databyte << 1) & 0xff;
			databyte |= shift_reg & 1;
		}
		curbyte <<= 1;

		switch (state)
		{
		case FMIDAM:
			if (shift_reg == 0xf57e)
			{
				// IDAM found
				LOGMASKED(LOG_DETAIL, "[ti99_dsk] IDAM found at %05x\n", bitpos);
				state = HEADER;
				lastpos = bitpos;
				rep = 0;
			}
			break;
		case A1IDAM1:
			if (shift_reg == 0x4489)
			{
				state = A1IDAM2;
				lastpos = bitpos;
			}
			break;
		case A1IDAM2:
		case A1IDAM3:
			if (bitpos - lastpos == 16)
			{
				if (shift_reg == 0x4489)
				{
					state = (state==A1IDAM2)? A1IDAM3 : MFMIDENT;
				}
				else
				{
					LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Missing A1, not a header at %05x\n", bitpos);
					state = A1IDAM1;
				}
				lastpos = bitpos;
			}
			break;

		case MFMIDENT:
			if (bitpos - lastpos == 16)
			{
				if (databyte != 0xfe)
				{
					LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Ident (found) = %02x, expected = fe; skipping header\n", databyte);
					state = A1IDAM1;
				}
				else
				{
					rep = 0;
					state = HEADER;
				}
				lastpos = bitpos;
			}
			break;
		case HEADER:
			// Get the header, store the sector number, and report any
			// unexpected contents in the other fields. When invalid, skip
			// that header.
			if (bitpos - lastpos == 16)
			{
				LOGMASKED(LOG_HEADER, "[ti99_dsk] Header byte: %02x\n", databyte);

				switch (rep)
				{
				case 0:
					if (databyte > 79)
					{
						// Illegal cylinder, ignore this header.
						LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Track (found) = %d, must be less than 80; skipping header.\n", databyte);
						state = (encoding==floppy_image::MFM)? A1IDAM1 : FMIDAM;
						break;
					}
					// Consider double stepping
					if (databyte != track && databyte != track/2)
						LOGMASKED(LOG_WARN, "[ti99_dsk] Warning: Track (found) = %d, assuming %d\n", databyte, track);
					break;
				case 1:
					if (databyte > 2)
					{
						// Illegal head, ignore this header.
						LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Head (found) = %d, must be less than 2; skipping header.\n", databyte);
						state = (encoding==floppy_image::MFM)? A1IDAM1 : FMIDAM;
						break;
					}
					if (databyte != head)
						LOGMASKED(LOG_WARN, "[ti99_dsk] Warning: Head (found) = %d, assuming = %d\n", databyte, head);
					break;
				case 2:
					if (databyte >= maxsect)
					{
						// Illegal sector, ignore this header.
						LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Sector number (found) = %d, must be less than %d; skipping header\n", databyte, maxsect);
						state = (encoding==floppy_image::MFM)? A1IDAM1 : FMIDAM;
					}
					else cursect = databyte;

					if ((foundsectors & (1 << cursect)) != 0)
					{
						// Sector already appeared; discard it
						LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Sector number (found) = %d already appeared; skipping header\n", databyte);
						state = (encoding==floppy_image::MFM)? A1IDAM1 : FMIDAM;
					}
					break;
				case 3:
					if (databyte != 1)
					{
						// Illegal sector, ignore this header.
						LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Sector size (found) = %d, must be 1; skipping header\n", databyte);
						state = (encoding==floppy_image::MFM)? A1IDAM1 : FMIDAM;
					}
					else
					{
						state = (encoding==floppy_image::MFM)? A1DAM1 : FMDAM;
					}
					rep = -1;
					break;
				}
				rep++;
				lastpos = bitpos;
			}
			break;
		case FMDAM:
			if (bitpos - lastpos > 500)
			{
				LOGMASKED(LOG_WARN, "[ti99_dsk] Error: DAM too far away, skipping header\n");
				state = FMIDAM;
				break;
			}
			if (shift_reg == 0xf57e)
			{
				LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Expected DAM, found next IDAM; skipping header\n");
				state = HEADER;  // continue with reading the header
				lastpos = bitpos;
				break;
			}
			if (shift_reg == 0xf56a || shift_reg == 0xf56f)
			{
				// DAM found
				LOGMASKED(LOG_DETAIL, "[ti99_dsk] DAM found: %04x\n", bitpos);
				state = DATA;
				rep = 0;
				lastpos = bitpos;
				secnumber[seccount++] = cursect;
				// Keep track of the found sectors
				foundsectors |= (1 << cursect);
			}
			break;
		case A1DAM1:
			if (bitpos - lastpos > 1000)
			{
				LOGMASKED(LOG_WARN, "[ti99_dsk] Error: A1/DAM too far away, skipping header\n");
				state = A1IDAM1;
				break;
			}
			if (shift_reg == 0x4489)
			{
				state = A1DAM2;
				lastpos = bitpos;
			}
			break;
		case A1DAM2:
		case A1DAM3:
			if (bitpos - lastpos == 16)
			{
				if (shift_reg == 0x4489)
				{
					state = (state==A1DAM2)? A1DAM3 : MFMDAM;
				}
				else
				{
					LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Missing A1, not a data field at %05x\n", bitpos);
					state = A1IDAM1;
				}
				lastpos = bitpos;
			}
			break;

		case MFMDAM:
			if (bitpos - lastpos == 16)
			{
				if (databyte == 0xfe)
				{
					LOGMASKED(LOG_WARN, "[ti99_dsk] Error: Expected DAM, found next IDAM; skipping header\n");
					state = HEADER;  // continue with reading the header
					lastpos = bitpos;
					break;
				}

				if (databyte != 0xf8 && databyte != 0xfb)
				{
					LOGMASKED(LOG_WARN, "[ti99_dsk] Error: DAM (found) = %02x, expected = f8 or fb; skipping header\n", databyte);
					state = HEADER;  // continue with reading the header
				}
				else
				{
					state = DATA;
					secnumber[seccount++] = cursect;
					// Keep track of the found sectors
					foundsectors |= (1 << cursect);
				}
				rep = 0;
				lastpos = bitpos;
			}
			break;

		case DATA:
			if (bitpos - lastpos == 16)
			{
				if (rep < 256)
				{
					sectordata[spos++] = databyte;
				}
				else
				{
					state = first;
				}
				rep++;
				lastpos = bitpos;
			}
			break;
		}
		bitpos++;
	}

	// We start with 9 sectors (FM) and 16 sectors (MFM)
	// If we have 18 sectors in MFM, and we have track 0/head 0, then accept
	// it, and switch to expected = 18.
	// If we have expected=16 and find sectors past sector 15,
	// and track!=0 or head!=0, then stop
	// the creation and restart with expected=18.

	// We now add all remaining, unfound sectors to complete the track
	// If all sectors were found, this loop is skipped
	int j = 0;
	int actual_sectors = seccount;
	LOGMASKED(LOG_DETAIL, "[ti99_dsk] Sector map: %08x\n", foundsectors);

	// Don't try to fill the first track
	if (track==0 && head==0 && seccount<3)
		return seccount;

	if (seccount < sectors)
	{
		LOGMASKED(LOG_INFO, "[ti99_dsk] %d missing sectors in track %d, head %d, adding as empty\n", sectors-seccount, track, head);
	}

	for (int i = seccount; i < sectors; i++)
	{
		// Search the next missing sector number. All sectors that were
		// found have a 1 at their bit position in foundsectors.
		while ((foundsectors & 1)!=0 && j < sectors)
		{
			foundsectors >>= 1;
			j++;
		}
		LOGMASKED(LOG_INFO, "[ti99_dsk] add empty sector %d\n", j);
		// Add the sector
		secnumber[seccount++] = j;
		foundsectors >>= 1;
		j++;
		// Create an empty sector
		for (int k=0; k < 256; k++)
			sectordata[spos++] = 0xe5;
	}

	LOGMASKED(LOG_INTERLEAVE, "[ti99_dsk] End of track reached, sectors = %d\n", seccount);
	LOGMASKED(LOG_INTERLEAVE, "[ti99_dsk] Sector sequence: ");
	for (int i=0; i < seccount; i++)
		LOGMASKED(LOG_INTERLEAVE, "%d ", secnumber[i]);
	LOGMASKED(LOG_INTERLEAVE, "\n");
	return actual_sectors;
}


uint8_t ti99_floppy_format::get_data_from_encoding(uint16_t raw)
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
	return "ti99";
}

const char *ti99_sdf_format::description() const
{
	return "TI99 sector dump floppy disk image";
}

const char *ti99_sdf_format::extensions() const
{
	return "dsk";
}

int ti99_sdf_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint64_t file_size;
	if (io.length(file_size))
		return 0;

	int vote = 0;

	// Adding support for another sector image format which adds 768 bytes
	// as a bad sector map
	if (((file_size - 768) % (SECTOR_SIZE*360)) == 0)
		file_size -= 768;

	// All formats apply for 5.25" and 3.5" form factors
	switch (file_size)
	{
	case 92160:     // SSSD
	case 163840:    // SSDD16
	case 184320:    // DSSD
	case 327680:    // DSDD16
	case 368640:    // DSDD
	case 737280:    // DSDD80
	case 1474560:   // DSQD
		vote = 50;
		break;
	default:
		vote = 0;
		break;
	}

	if (vote > 0)
	{
		// Read first sector (Volume Information Block)
		ti99vib vib;
		size_t actual;
		io.read_at(0, &vib, sizeof(ti99vib), actual);

		// Check from contents
		if ((vib.id[0]=='D')&&(vib.id[1]=='S')&&(vib.id[2]=='K'))
		{
			LOGMASKED(LOG_INFO, "[ti99_dsk] Found formatted SDF disk medium\n");
			vote = 100;
		}
		else
		{
			LOGMASKED(LOG_INFO, "[ti99_dsk] No valid VIB found; disk may be unformatted\n");
		}
	}
	else LOGMASKED(LOG_INFO, "[ti99_dsk] Disk image is not a SDF image\n");
	return vote;
}

void ti99_sdf_format::determine_sizes(util::random_read &io, int& cell_size, int& sector_count, int& heads, int& tracks)
{
	uint64_t file_size;
	if (io.length(file_size))
	{
		cell_size = sector_count = heads = tracks = 0;
		return;
	}

	cell_size = 0;
	sector_count = 0;
	heads = 2;

	bool have_vib = false;

	// Remove the bad sector map
	if (((file_size - 768) % (SECTOR_SIZE*360)) == 0)
		file_size -= 768;

	// Read first sector
	ti99vib vib;
	size_t actual;
	io.read_at(0, &vib, sizeof(ti99vib), actual);

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
		LOGMASKED(LOG_INFO, "[ti99_dsk] VIB says that this disk is %s density with %d sectors per track, %d tracks, and %d heads\n", (cell_size==4000)? "single": ((cell_size==2000)? "double" : "high"), sector_count, vib.tracksperside, heads);
		have_vib = true;
		tracks = vib.tracksperside;
	}

	// Do we have a broken VIB? The Pascal disks are known to have such incomplete VIBs
	if (tracks == 0 || heads == 0 || sector_count == 0) have_vib = false;

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
			LOGMASKED(LOG_WARN, "[ti99_dsk] Warning: Invalid 16-sector format. Assuming 18 sectors.\n");
			sector_count = 18;
		}
		else
		{
			if (heads == 2 && ((cell_size1 != cell_size) || (sector_count1 != sector_count)))
				LOGMASKED(LOG_WARN, "[ti99_dsk] Warning: Disk image size does not correspond with format information in VIB.\n");
		}
	}
	else
	{
		heads = (file_size < 180000)? 1 : 2;    // for SSSD, SSDD16
		cell_size = cell_size1;
		sector_count = sector_count1;
	}
}

int ti99_sdf_format::get_track_size(int sector_count)
{
	return sector_count * SECTOR_SIZE;
}

void ti99_sdf_format::load_track(util::random_read &io, uint8_t *sectordata, int *sector, int *secoffset, int head, int track, int sectorcount, int trackcount)
{
	// Calculate the track offset from the beginning of the image file
	int logicaltrack = (head==0)? track : (2*trackcount - track - 1);
	int position = logicaltrack * get_track_size(sectorcount);

	size_t actual;
	io.read_at(position, sectordata, sectorcount*SECTOR_SIZE, actual);

	// Interleave and skew
	int interleave = 7;
	if (sectorcount == 18) interleave = 11;
	else if (sectorcount == 16) interleave = 9;

	int skew = (sectorcount<10)? 6 : 0;

	int secno = (track * skew) % sectorcount;

	// Construct the sector sequence
	for (int i=0; i < sectorcount; i++)
	{
		sector[i] = secno;
		secoffset[i] = secno*SECTOR_SIZE;
		secno = (secno + interleave) % sectorcount;
	}
	if (track==0 && head==0)
	{
		dumpbytes(sectordata, sectorcount*SECTOR_SIZE);
		LOGMASKED(LOG_TRACK, "[ti99_dsk] Sectors = ");
		for (int i=0; i < sectorcount; i++)
		{
			LOGMASKED(LOG_TRACK, "%02d ", sector[i]);
		}
		LOGMASKED(LOG_TRACK, "\n[ti99_dsk] Offset = ");
		for (int i=0; i < sectorcount; i++)
		{
			LOGMASKED(LOG_TRACK, "%04x ", secoffset[i]);
		}
		LOGMASKED(LOG_TRACK, "\n");
	}
}

/*
    For debugging. Outputs the byte array in a xxd-like way.
*/
void ti99_floppy_format::dumpbytes(uint8_t* trackdata, int length)
{
	for (int i=0; i < length; i+=16)
	{
		LOGMASKED(LOG_TRACK, "[ti99_dsk] %s\n", dumpline(trackdata+i, i).c_str());
	}
}

std::string ti99_floppy_format::dumpline(uint8_t* line, int address) const
{
	std::ostringstream stream;
	stream << std::hex << std::setfill('0') << std::setw(6) << unsigned(address);

	for (int i=0; i < 16; i++)
	{
		stream << " "  << std::setw(2) << (int)line[i];
	}
	stream << "   " ;
	for (int i=0; i < 16; i++)
	{
		char ch = (line[i] >= 32 && line[i] < 127)? (char)line[i] : '.';
		stream << std::setw(1) << ch;
	}
	return stream.str();
}

/*
    Write the data to the disk. We have a list of sector positions, so we
    just need to go through that list and save each sector in the sector data.
*/
void ti99_sdf_format::write_track(util::random_read_write &io, uint8_t *sectordata, int *sector, int track, int head, int sector_count, int track_count)
{
	int logicaltrack = head * track_count;
	logicaltrack += ((head&1)==0)?  track : (track_count - 1 - track);
	int trackoffset = logicaltrack * sector_count * SECTOR_SIZE;

	for (int i=0; i < sector_count; i++)
	{
		uint8_t const *const buf = sectordata + i * SECTOR_SIZE;
		LOGMASKED(LOG_DETAIL, "[ti99_dsk] Writing sector %d (offset %06x)\n", sector[i], sector[i] * SECTOR_SIZE);
		size_t actual;
		io.write_at(trackoffset + sector[i] * SECTOR_SIZE, buf, SECTOR_SIZE, actual);
	}
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

    DSSD80: 520480 bytes
    DSDD80: 1099520 bytes

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
int ti99_tdf_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	int vote = 0;
	uint8_t fulltrack[6872];

	int cell_size = 0;
	int sector_count = 0;
	int heads = 0;
	int tracks = 0;
	int sectorinterval = 0;
	int headsectoff = 0;
	int pos = 0;
	determine_sizes(io, cell_size, sector_count, heads, tracks);

	// Do we have a plausible image size? From the size alone we only give a 50 percent vote.
	if (sector_count != 0)
		vote = 50;

	if (vote > 0)
	{
		LOGMASKED(LOG_INFO, "[ti99_dsk] Image file length matches TDF\n");

		// Fetch track 0
		size_t actual;
		io.read_at(0, fulltrack, get_track_size(sector_count), actual);

		if (sector_count == 9)
		{
			headsectoff = 24;
			sectorinterval = 334;
		}
		else
		{
			headsectoff = 44;
			sectorinterval = (sector_count==16)? 368 : 340;
		}

		while (fulltrack[pos] != 0xfe && pos < 70) pos++;

		bool failed = false;
		// Find the IDAMs and DAMs on track 0
		for (int i=0; i < sector_count; i++)
		{
			int idam = fulltrack[pos+i*sectorinterval];
			int dam = fulltrack[pos+headsectoff+i*sectorinterval];
			if ((idam != 0xfe) || ((dam != 0xfb) && (dam != 0xf8)))
			{
				LOGMASKED(LOG_WARN, "[ti99_dsk] Failed at %04x or %04x\n", pos+i*sectorinterval, pos+headsectoff+i*sectorinterval);
				failed = true;
				break;
			}
		}

		if (failed)
		{
			LOGMASKED(LOG_WARN, "[ti99_dsk] Image does not comply with TDF; may be broken or unformatted\n");
		}
		else
		{
			LOGMASKED(LOG_INFO, "[ti99_dsk] Image format complies with TDF\n");
			vote = 100;
		}
	}
	else LOGMASKED(LOG_INFO, "[ti99_dsk] Disk image is not a TDF image\n");
	return vote;
}

/*
    Find the proper format for a given image file. Tracks are counted per side.
    Note that only two formats are actually compatible with the PC99 emulator.
*/
void ti99_tdf_format::determine_sizes(util::random_read &io, int& cell_size, int& sector_count, int& heads, int& tracks)
{
	uint64_t file_size;
	if (io.length(file_size))
	{
		cell_size = sector_count = heads = tracks = 0;
		return;
	}

	// LOGMASKED(LOG_INFO, "[ti99_dsk] Image size = %ld\n", file_size);   // doesn't compile
	switch (file_size)
	{
	case 260240:            // used in PC99
		sector_count = 9;
		tracks = 40;
		break;
	case 491520:            // 320K HX5102
		sector_count = 16;
		tracks = 40;
		break;
	case 549760:            // used in PC99
		sector_count = 18;
		tracks = 40;
		break;
	case 1003520:
		sector_count = 36;
		tracks = 40;
		break;

	case 520480:
		sector_count = 9;
		tracks = 80;
		break;
	case 983040:
		sector_count = 16;
		tracks = 80;
		break;
	case 1099520:
		sector_count = 18;
		tracks = 80;
		break;
	case 2007040:
		sector_count = 36;
		tracks = 80;
		break;

	default:
		cell_size = 0;
		sector_count = 0;
		return;
	}

	heads = 2;  // TDF only supports two-sided recordings
	cell_size = (sector_count <= 9)? 4000 : ((sector_count <= 18)? 2000 : 1000);
}

/*
    Load a track. Although TDF provides us with a near-precise image of the
    track, we just copy the sectors, store the sequence, and recreate the
    track from scratch. TDF is not as flexible as it suggests, it does not
    allow different gap lengths and so on.
*/
void ti99_tdf_format::load_track(util::random_read &io, uint8_t *sectordata, int *sector, int *secoffset, int head, int track, int sectorcount, int trackcount)
{
	size_t actual;
	uint8_t fulltrack[12544]; // space for a full TDF track

	// Read beginning of track 0. We need this to get the first gap, according
	// to the format
	io.read_at(0, fulltrack, 100, actual);

	int offset = 0;
	int tracksize = get_track_size(sectorcount);
	int sectorinterval = 0;
	int headsectoff = 0;
	int pos = 0;
	int img_sect = 0;

	if (sectorcount == 9)
	{
		headsectoff = 25;
		sectorinterval = 334;
	}
	else
	{
		headsectoff = 45;
		sectorinterval = (sectorcount==16)? 368 : 340;
	}

	// Initialize the sequence lists
	for (int i=0; i < sectorcount; i++)
	{
		sector[i] = -1;
		secoffset[0] = 0;
	}

	// We already verified that this is TDF, but the disk may be unformatted
	while (fulltrack[pos] != 0xfe && pos < tracksize) pos++;

	if (pos==tracksize)
	{
		// No sectors found
		LOGMASKED(LOG_WARN, "[ti99_dsk] No sectors on track %d, head %d\n", track, head);
		return;
	}

	offset = pos;

	int base = (head * trackcount + track) * tracksize;
	int position = 0;
	io.read_at(base, fulltrack, tracksize, actual);

	for (int i=0; i < sectorcount; i++)
	{
		position = i*sectorinterval + offset;
		if (position >= 6872)
		{
			LOGMASKED(LOG_WARN, "[ti99_dsk] No more sectors on track %d, head %d\n", track, head);
			break;                // no more sectors
		}
		if (fulltrack[position] != 0xfe)
		{
			LOGMASKED(LOG_WARN, "[ti99_dsk] Invalid header (no FE) on track %d, head %d\n", track, head);
			continue;  // no valid header
		}
		img_sect = fulltrack[position+3];
		if (img_sect >= 18)
		{
			LOGMASKED(LOG_WARN, "[ti99_dsk] Invalid sector number (%d) on track %d, head %d\n", img_sect, track, head);
			continue;               // invalid sector number
		}
		sector[i] = img_sect;
		secoffset[i] = i*SECTOR_SIZE;
		position += headsectoff;
		memcpy(sectordata + i*SECTOR_SIZE, fulltrack+position, SECTOR_SIZE);
	}

	// dumpbytes(sectordata, sectorcount*SECTOR_SIZE);
	LOGMASKED(LOG_TRACK, "[ti99_dsk] Sectors = ");
	for (int i=0; i < sectorcount; i++)
	{
		LOGMASKED(LOG_TRACK, "%02d ", sector[i]);
	}
	LOGMASKED(LOG_TRACK, "\n[ti99_dsk] Offset = ");
	for (int i=0; i < sectorcount; i++)
	{
		LOGMASKED(LOG_TRACK, "%04x ", secoffset[i]);
	}
	LOGMASKED(LOG_TRACK, "\n");
}

/*
    Rebuild TDF track image
    The TDF format is a bit more restrictive than it could be. The gaps need
    to be of the exact size and contain the expected bytes. So we only
    need the sector contents and the sector sequence, which are passed via
    sectordata, sector, and sector_count.
*/
void ti99_tdf_format::write_track(util::random_read_write &io, uint8_t *sectordata, int *sector, int track, int head, int sector_count, int track_count)
{
	uint8_t trackdata[12544];
	int offset = ((track_count * head) + track) * get_track_size(sector_count);
	int a1 = 0;

	enum
	{
		WGAP1 = 0, WGAP2, WGAP3, WGAP4, WGAP1BYTE, WGAPBYTE, WSYNC1, WSYNC2
	};

	const int fm9param[8]   = { 16, 11, 45, 231, 0x00, 0xff,  6,  6 };
	const int mfm16param[8] = { 50, 22, 50, 206, 0x4e, 0x4e, 12, 12 };
	const int mfm18param[8] = { 40, 22, 24, 712, 0x4e, 0x4e, 10, 12 };
	const int mfm36param[8] = { 40, 22, 24, 264, 0x4e, 0x4e, 10, 12 };

	const int *param = fm9param;

	if (sector_count == 16)
		param = mfm16param;
	else if (sector_count == 18)
		param = mfm18param;
	else if (sector_count == 36)
		param = mfm36param;

	LOGMASKED(LOG_TRACK, "[ti99_dsk] Writing track %d, head %d\n", track, head);

	if (sector_count >= 16) a1 = 3;

	// Create track
	int pos = 0;
	for (int i=0; i < param[WGAP1]; i++) trackdata[pos++] = param[WGAP1BYTE];
	for (int j=0; j < sector_count; j++)
	{
		for (int i=0; i < param[WSYNC1]; i++) trackdata[pos++] = 0x00;
		for (int i=0; i < a1; i++) trackdata[pos++] = 0xa1;
		trackdata[pos++] = 0xfe;
		trackdata[pos++] = track;
		trackdata[pos++] = head;
		trackdata[pos++] = sector[j];
		trackdata[pos++] = 0x01;
		trackdata[pos++] = 0xf7;    // PC99 does not accept real CRC values
		trackdata[pos++] = 0xf7;
		for (int i=0; i < param[WGAP2]; i++) trackdata[pos++] = param[WGAPBYTE];
		for (int i=0; i < param[WSYNC2]; i++) trackdata[pos++] = 0x00;
		for (int i=0; i < a1; i++) trackdata[pos++] = 0xa1;
		trackdata[pos++] = 0xfb;
		memcpy(trackdata + pos, sectordata + j * SECTOR_SIZE, SECTOR_SIZE);
		pos +=256;
		trackdata[pos++] = 0xf7;
		trackdata[pos++] = 0xf7;
		for (int i=0; i < param[WGAP3]; i++) trackdata[pos++] = param[WGAPBYTE];
	}
	for (int i=0; i < param[WGAP4]; i++) trackdata[pos++] = param[WGAPBYTE];
	size_t actual;
	io.write_at(offset, trackdata, get_track_size(sector_count), actual);
}

int ti99_tdf_format::get_track_size(int sector_count)
{
	switch (sector_count)
	{
	case 9:
		return 3253;
	case 16:
		return 6144;
	case 18:
		return 6872;
	case 36:
		return 12544;
	}
	return 0;
}

const floppy_format_type FLOPPY_TI99_TDF_FORMAT = &floppy_image_format_creator<ti99_tdf_format>;
