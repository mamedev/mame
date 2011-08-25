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
	adf_format(const char *name,const char *extensions,const char *description,const char *param_guidelines);

	virtual int identify(floppy_image *image);
	virtual bool load(floppy_image *image);

	static const desc_e desc[];
};

extern const floppy_format_type FLOPPY_ADF_FORMAT;

#endif /*AMI_DSK_H_*/
