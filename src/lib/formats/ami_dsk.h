/*********************************************************************

    formats/ami_dsk.h

    Amiga disk images

*********************************************************************/

#ifndef AMI_DSK_H_
#define AMI_DSK_H_

#include "flopimg.h"

class adf_format : public floppy_image_format_t
{
public:
	adf_format();

	virtual int identify(io_generic *io);
	virtual bool load(io_generic *io, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

private:
	static UINT32 g32(const UINT8 *trackbuf, int track_size, int pos);
	static UINT32 checksum(const UINT8 *trackbuf, int track_size, int pos, int long_count);
};

extern const floppy_format_type FLOPPY_ADF_FORMAT;

#endif /*AMI_DSK_H_*/
