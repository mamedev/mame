// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*********************************************************************

    formats/ti99_dsk.c

    TI99 and Geneve disk images

    Michael Zapf, Sept 2014

*********************************************************************/
#ifndef MAME_FORMATS_TI99_DSK_H
#define MAME_FORMATS_TI99_DSK_H

#pragma once

#include "flopimg.h"

#include <string>


class ti99_floppy_format : public floppy_image_format_t
{
public:
	bool supports_save() const override { return true; }
	bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	bool save(io_generic *io, floppy_image *image) override;

protected:
	int decode_bitstream(const uint8_t *bitstream, uint8_t *trackdata, int *sector, int cell_count, int encoding, uint8_t gapbytes, int track_size);
	uint8_t get_data_from_encoding(uint16_t raw);
	int get_sectors(const uint8_t *bitstream, int cell_count, int encoding, int track, int head, int sectors, uint8_t *sectordata, int *secnumber);

	virtual int min_heads() =0;

	virtual void determine_sizes(io_generic *io, int& cell_size, int& sector_count, int& heads, int& tracks) =0;
	virtual int get_track_size(int sector_count) =0;
	virtual void load_track(io_generic *io, uint8_t *sectordata, int *sector, int *secoffset, int head, int track, int sectorcount, int trackcount) =0;
	virtual void write_track(io_generic *io, uint8_t *sectordata, int *sector, int track, int head, int sector_count, int track_count) =0;

	int get_encoding(int cell_size);

	void generate_fm_track_from_sectors(floppy_image *image, uint8_t *sectordata, int sector_count, int *sector, int *secoffset, int track, int trackid, int head);
	void generate_mfm_track_from_sectors(floppy_image *image, uint8_t *sectordata, int sector_count, int *sector, int *secoffset, int track, int trackid, int head);

	// Debugging
	void dumpbytes(uint8_t* trackdata, int length);
	std::string dumpline(uint8_t* line, int address) const;
};

/*
    Implementation of the sector dump format.
*/
class ti99_sdf_format : public ti99_floppy_format
{
public:
	int identify(io_generic *io, uint32_t form_factor) override;
	const char *name() const override;
	const char *description() const override;
	const char *extensions() const override;

private:
	void determine_sizes(io_generic *io, int& cell_size, int& sector_count, int& heads, int& tracks) override;
	int get_track_size(int sector_count) override;
	void write_track(io_generic *io, uint8_t *sectordata, int *sector, int track, int head, int sector_count, int track_count) override;
	void load_track(io_generic *io, uint8_t *sectordata, int *sector, int *secoffset, int head, int track, int sector_count, int track_count) override;

	// This format supports single-sided images
	int min_heads() override { return 1; }

	struct ti99vib
	{
		char    name[10];       // volume name (10 characters, pad with spaces)
		uint8_t   totsecsMSB;     // disk length in sectors (big-endian) (usually 360, 720 or 1440)
		uint8_t   totsecsLSB;
		uint8_t   secspertrack;   // sectors per track (usually 9 (FM) or 18 (MFM))
		uint8_t   id[3];          // String "DSK"
		uint8_t   protection;     // 'P' if disk is protected, ' ' otherwise.
		uint8_t   tracksperside;  // tracks per side (usually 40)
		uint8_t   sides;          // sides (1 or 2)
		uint8_t   density;        // 0,1 (FM) or 2,3,4 (MFM)
		uint8_t   res[36];        // Empty for traditional disks, or up to 3 directory pointers
		uint8_t   abm[200];       // allocation bitmap: a 1 for each sector in use (sector 0 is LSBit of byte 0,
								// sector 7 is MSBit of byte 0, sector 8 is LSBit of byte 1, etc.)
	};
};

extern const floppy_format_type FLOPPY_TI99_SDF_FORMAT;

/*
    Implementation of the track dump format.
*/
class ti99_tdf_format : public ti99_floppy_format
{
public:
	int identify(io_generic *io, uint32_t form_factor) override;
	const char *name() const override;
	const char *description() const override;
	const char *extensions() const override;

private:
	void determine_sizes(io_generic *io, int& cell_size, int& sector_count, int& heads, int& tracks) override;
	void load_track(io_generic *io, uint8_t *sectordata, int *sector, int *secoffset, int head, int track, int sectorcount, int trackcount) override;
	void write_track(io_generic *io, uint8_t *sectordata, int *sector, int track, int head, int sector_count, int track_count) override;
	int get_track_size(int sector_count) override;

	// This format only supports double-sided images
	int min_heads() override { return 2; }
};

extern const floppy_format_type FLOPPY_TI99_TDF_FORMAT;

#endif // MAME_FORMATS_TI99_DSK_H
