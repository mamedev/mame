/*********************************************************************

    formats/ti99_dsk.c

    TI99 and Geneve disk images

    Michael Zapf, Feb 2014

*********************************************************************/

#ifndef TI99_DSK_H
#define TI99_DSK_H

#include "flopimg.h"
#include "wd177x_dsk.h"

/*
    Modern implementation of the sector dump format.
*/
class ti99_sdf_format : public wd177x_format
{
public:
	ti99_sdf_format(): wd177x_format(formats) { }

	int identify(io_generic *io, UINT32 form_factor);

	const char *name() const;
	const char *description() const;
	const char *extensions() const;
	bool supports_save() const;

private:
	static const format formats[];
	floppy_image_format_t::desc_e* get_desc_fm(const format &f, int &current_size, int &end_gap_index);
	floppy_image_format_t::desc_e* get_desc_mfm(const format &f, int &current_size, int &end_gap_index);

	int get_format_param(io_generic *io);
	int get_image_offset(const format &f, int head, int track);
	int find_size(io_generic *io, UINT32 form_factor);

	typedef struct ti99_vib
	{
		char    name[10];       // volume name (10 characters, pad with spaces)
		UINT8   totsecsMSB;     // disk length in sectors (big-endian) (usually 360, 720 or 1440)
		UINT8   totsecsLSB;
		UINT8   secspertrack;   // sectors per track (usually 9 (FM) or 18 (MFM))
		UINT8   id[3];          // String "DSK"
		UINT8   protection;     // 'P' if disk is protected, ' ' otherwise.
		UINT8   tracksperside;  // tracks per side (usually 40)
		UINT8   sides;          // sides (1 or 2)
		UINT8   density;        // 0,1 (FM) or 2,3,4 (MFM)
		UINT8   res[36];        // Empty for traditional disks, or up to 3 directory pointers
		UINT8   abm[200];       // allocation bitmap: a 1 for each sector in use (sector 0 is LSBit of byte 0,
								// sector 7 is MSBit of byte 0, sector 8 is LSBit of byte 1, etc.)
	} vib_t;
};

extern const floppy_format_type FLOPPY_TI99_SDF_FORMAT;

/*
    Modern implementation of the track dump format.
*/
class ti99_tdf_format : public floppy_image_format_t
{
public:

	int identify(io_generic *io, UINT32 form_factor);

	bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	bool save(io_generic *io, floppy_image *image);

	const char *name() const;
	const char *description() const;
	const char *extensions() const;
	bool supports_save() const;

private:

	struct format {
		UINT32 form_factor;      // See floppy_image for possible values
		UINT32 variant;          // See floppy_image for possible values
		UINT32 encoding;         // See floppy_image for possible values
		int cell_size;           // See floppy_image_format_t for details
		int sector_count;
		int track_count;
		int head_count;
		int track_size;
		UINT8 gapbytes;
	};

	static const format formats[];

	int get_format_param(io_generic *io);
	int get_image_offset(const format &f, int head, int track);
	int find_size(io_generic *io, UINT32 form_factor);

	void generate_track_fm(int track, int head, int cell_size, UINT8* trackdata, floppy_image *image);
	void generate_track_mfm(int track, int head, int cell_size, UINT8* trackdata, floppy_image *image);

	int find_start(UINT8* trackdata, int track_size, int encoding);
	bool has_byteseq(UINT8* trackdata, int track_size, UINT8 byte, int pos, int count);

	void extract_track_from_bitstream(const UINT8 *bitstream, int track_size, UINT8 *trackdata);
};

extern const floppy_format_type FLOPPY_TI99_TDF_FORMAT;

// ========================================================================
/*
    Legacy implementation.
*/

LEGACY_FLOPPY_OPTIONS_EXTERN(ti99);
void ti99_set_80_track_drives(int use80);

#endif /* TI99_DSK_H */
