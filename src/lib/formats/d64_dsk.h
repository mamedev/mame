// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d64_dsk.h

    Commodore 4040/1541/1551 sector disk image format

*********************************************************************/

#ifndef D64_DSK_H_
#define D64_DSK_H_

#include "flopimg.h"

class d64_format : public floppy_image_format_t {
public:
	struct format {
		UINT32 form_factor;      // See floppy_image for possible values
		UINT32 variant;          // See floppy_image for possible values

		UINT16 sector_count;
		UINT8 track_count;
		UINT8 head_count;
		UINT16 sector_base_size;
		UINT8 gap_1;
		UINT8 gap_2;
	};

	d64_format();
	d64_format(const format *formats);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool supports_save() const;

protected:
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

	const format *formats;

	int find_size(io_generic *io, UINT32 form_factor);
	virtual int get_physical_track(const format &f, int head, int track);
	virtual UINT32 get_cell_size(const format &f, int track);
	virtual int get_sectors_per_track(const format &f, int track);
	virtual int get_disk_id_offset(const format &f);
	void get_disk_id(const format &f, io_generic *io, UINT8 &id1, UINT8 &id2);
	virtual floppy_image_format_t::desc_e* get_sector_desc(const format &f, int &current_size, int sector_count, UINT8 id1, UINT8 id2, int gap_2);
	void build_sector_description(const format &f, UINT8 *sectdata, offs_t sect_offs, offs_t error_offs, desc_s *sectors, int sector_count) const;

	static const format file_formats[];

	static const UINT32 cell_size[];
	static const int sectors_per_track[];
	static const int speed_zone[];
};

extern const floppy_format_type FLOPPY_D64_FORMAT;


FLOPPY_IDENTIFY( d64_dsk_identify );
FLOPPY_IDENTIFY( d67_dsk_identify );
FLOPPY_IDENTIFY( d71_dsk_identify );
FLOPPY_IDENTIFY( d80_dsk_identify );
FLOPPY_IDENTIFY( d82_dsk_identify );

FLOPPY_CONSTRUCT( d64_dsk_construct );

#endif
