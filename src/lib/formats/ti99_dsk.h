/*********************************************************************

    formats/ti99_dsk.c

    TI99 and Geneve disk images

    Header file to be included by drivers which use these floppy options
*********************************************************************/

#ifndef TI99_DSK_H
#define TI99_DSK_H

#include "flopimg.h"
#include "wd177x_dsk.h"

/*
    New implementation.
    TDF format still to be done;
    also, this implementation just considers FM for now.
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
};

/*
    Legacy implementation.
*/

LEGACY_FLOPPY_OPTIONS_EXTERN(ti99);

void ti99_set_80_track_drives(int use80);
extern const floppy_format_type FLOPPY_TI99_SDF_FORMAT;

#endif /* TI99_DSK_H */
