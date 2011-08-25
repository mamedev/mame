#include "hxcmfm_dsk.h"

#define MFM_FORMAT_HEADER	"HXCMFM"

#pragma pack(1)

struct MFMIMG
{
	UINT8 headername[7];

	UINT16 number_of_track;
	UINT8 number_of_side;

	UINT16 floppyRPM;
	UINT16 floppyBitRate;
	UINT8 floppyiftype;

	UINT32 mfmtracklistoffset;
};

struct MFMTRACKIMG
{
	UINT16 track_number;
	UINT8 side_number;
	UINT32 mfmtracksize;
	UINT32 mfmtrackoffset;
};

#pragma pack()

mfm_format::mfm_format(const char *name,const char *extensions,const char *description,const char *param_guidelines) :
	floppy_image_format_t(name,extensions,description,param_guidelines)
{
}

int mfm_format::identify(floppy_image *image)
{
	UINT8 header[7];

	image->image_read(&header, 0, sizeof(header));
	if ( memcmp( header, MFM_FORMAT_HEADER, 6 ) ==0) {
		return 100;
	}
	return 0;
}

bool mfm_format::load(floppy_image *image)
{
	MFMIMG header;
	MFMTRACKIMG trackdesc;

	// read header
	image->image_read(&header,0, sizeof(header));

	image->set_meta_data(header.number_of_track,header.number_of_side,header.floppyRPM,header.floppyBitRate);

	for(int track=0; track < header.number_of_track; track++) {
		for(int side=0; side < header.number_of_side; side++) {
			// read location of
			image->image_read(&trackdesc,(header.mfmtracklistoffset)+((( track << 1 ) + side)*sizeof(trackdesc)),sizeof(trackdesc));

			image->set_track_size(track, side, trackdesc.mfmtracksize);
			// actual data read
			image->image_read(image->get_buffer(track,side), trackdesc.mfmtrackoffset, trackdesc.mfmtracksize);
		}
	}
	return FALSE;
}

const floppy_format_type FLOPPY_MFM_FORMAT = &floppy_image_format_creator<mfm_format>;
