/*********************************************************************

    formats/pc_dsk.h

    PC disk images

*********************************************************************/

#ifndef PC_DSK_H
#define PC_DSK_H

#include "flopimg.h"


/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(pc);


class pc_format : public floppy_image_format_t
{
public:
	pc_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

private:
	struct format {
		int size;
		UINT32 form_factor;
		UINT32 variant;
		int track_count;
		int head_count;
		int sector_count;
		const desc_e *desc;
		int cell_count;
	};

	static const format formats[];
	static const desc_e pc_9_desc[], pc_18_desc[], pc_36_desc[];

	int find_size(io_generic *io, UINT32 form_factor);
};

extern const floppy_format_type FLOPPY_PC_FORMAT;

#endif /* PC_DSK_H */
