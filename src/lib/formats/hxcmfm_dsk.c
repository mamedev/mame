#include "emu.h"

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

mfm_format::mfm_format() : floppy_image_format_t()
{
}

const char *mfm_format::name() const
{
	return "mfm";
}

const char *mfm_format::description() const
{
	return "HxCFloppyEmulator floppy disk image";
}

const char *mfm_format::extensions() const
{
	return "mfm";
}

bool mfm_format::supports_save() const
{
	return false;
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
	UINT8 *trackbuf = 0;
	int trackbuf_size = 0;

	// read header
	image->image_read(&header,0, sizeof(header));

	image->set_meta_data(header.number_of_track, header.number_of_side);

	for(int track=0; track < header.number_of_track; track++) {
		for(int side=0; side < header.number_of_side; side++) {
			// read location of
			image->image_read(&trackdesc,(header.mfmtracklistoffset)+((( track << 1 ) + side)*sizeof(trackdesc)),sizeof(trackdesc));

			if(trackdesc.mfmtracksize > trackbuf_size) {
				if(trackbuf)
					global_free(trackbuf);
				trackbuf_size = trackdesc.mfmtracksize;
				trackbuf = global_alloc_array(UINT8, trackbuf_size);
			}

			// actual data read
			image->image_read(trackbuf, trackdesc.mfmtrackoffset, trackdesc.mfmtracksize);

			generate_track_from_bitstream(track, side, trackbuf, trackdesc.mfmtracksize*8, image);
		}
	}
	if(trackbuf)
		global_free(trackbuf);

	return FALSE;
}

const floppy_format_type FLOPPY_MFM_FORMAT = &floppy_image_format_creator<mfm_format>;
