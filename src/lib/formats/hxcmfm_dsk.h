/*********************************************************************

    formats/hxcmfm_dsk.h

    HxC Floppy Emulator disk images

*********************************************************************/

#ifndef HXCMFM_DSK_H
#define HXCMFM_DSK_H

#include "flopimg.h"

class mfm_format : public floppy_image_format_t 
{
public:
	mfm_format(const char *name,const char *extensions,const char *description,const char *param_guidelines);
	
	virtual int identify(floppy_image *image);
};

extern const floppy_format_type FLOPPY_MFM_FORMAT;

#endif /* HXCMFM_DSK_H */
