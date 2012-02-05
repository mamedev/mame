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
	static const desc_e pc_desc[];

	void find_size(io_generic *io, UINT32 form_factor, int &track_count, int &head_count, int &sector_count);
};

extern const floppy_format_type FLOPPY_PC_FORMAT;

#endif /* PC_DSK_H */
