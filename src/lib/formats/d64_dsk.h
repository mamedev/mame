/*********************************************************************

    formats/d64_dsk.h

    Commodore 2040/8050/1541 sector disk image format

*********************************************************************/

#ifndef D64_DSK_H_
#define D64_DSK_H_

#include "flopimg.h"

class d64_format : public floppy_image_format_t {
public:
	struct format {
		UINT32 form_factor;      // See floppy_image for possible values
		UINT32 variant;          // See floppy_image for possible values

		int dos;
		int sector_count;
		int track_count;
		int head_count;
		int gap_1;
		int gap_2;
	};

	d64_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);
	virtual bool supports_save() const;

protected:
	enum
	{
		DOS_1,
		DOS_2,
		DOS_25
	};

	enum
	{
		ERROR_00 = 1,
		ERROR_20,       /* header block not found */
		ERROR_21,       /* no sync character */
		ERROR_22,       /* data block not present */
		ERROR_23,       /* checksum error in data block */
		ERROR_24,       /* write verify (on format) UNIMPLEMENTED */
		ERROR_25,       /* write verify error UNIMPLEMENTED */
		ERROR_26,       /* write protect on UNIMPLEMENTED */
		ERROR_27,       /* checksum error in header block */
		ERROR_28,       /* write error UNIMPLEMENTED */
		ERROR_29,       /* disk ID mismatch */
		ERROR_74,       /* disk not ready (no device 1) UNIMPLEMENTED */
	};

	static const int SECTOR_SIZE = 256;

	static const int DOS1_DISK_ID_OFFSET = 101144;
	static const int DOS25_DISK_ID_OFFSET = 282136;

	floppy_image_format_t::desc_e* get_sector_desc(const format &f, int &current_size, int track, int sector_count, UINT8 id1, UINT8 id2, int gap_2);

	int find_size(io_generic *io, UINT32 form_factor);
	int get_physical_track(const format &f, int track);
	UINT32 get_cell_size(const format &f, int track);
	int get_sectors_per_track(const format &f, int track);
	void get_disk_id(const format &f, io_generic *io, UINT8 &id1, UINT8 &id2);
	void build_sector_description(const format &f, UINT8 *sectdata, desc_s *sectors, int sector_count, UINT8 *errordata) const;

	static const format formats[];

	static const UINT32 dos1_cell_size[];
	static const UINT32 dos25_cell_size[];
	
	static const int dos1_speed_zone[];
	static const int dos25_speed_zone[];

	static const int dos1_sectors_per_track[];
	static const int dos2_sectors_per_track[];
	static const int dos25_sectors_per_track[];
};

extern const floppy_format_type FLOPPY_D64_FORMAT;


FLOPPY_IDENTIFY( d64_dsk_identify );
FLOPPY_IDENTIFY( d67_dsk_identify );
FLOPPY_IDENTIFY( d71_dsk_identify );
FLOPPY_IDENTIFY( d80_dsk_identify );
FLOPPY_IDENTIFY( d82_dsk_identify );

FLOPPY_CONSTRUCT( d64_dsk_construct );

#endif
