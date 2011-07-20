#include "hxcmfm_dsk.h"

#define MFM_FORMAT_HEADER	"HXCMFM"

mfm_format::mfm_format(const char *name,const char *extensions,const char *description,const char *param_guidelines) :
	floppy_image_format_t(name,extensions,description,param_guidelines)
{
}

int mfm_format::identify(floppy_image *image)
{
	UINT8 header[7];

	image->image_read(header, 0, 7);
	if ( memcmp( header, MFM_FORMAT_HEADER, 6 ) ==0) {
		return 100;
	}
	return 0;
}


const floppy_format_type FLOPPY_MFM_FORMAT = &floppy_image_format_creator<mfm_format>;