/*********************************************************************

    formats/ami_dsk.c

    Amiga disk images

*********************************************************************/

#include "formats/ami_dsk.h"

adf_format::adf_format() : floppy_image_format_t()
{
}

const char *adf_format::name() const
{
	return "adf";
}

const char *adf_format::description() const
{
	return "Amiga ADF floppy disk image";
}

const char *adf_format::extensions() const
{
	return "adf";
}

bool adf_format::supports_save() const
{
	return false;
}

int adf_format::identify(io_generic *io)
{
	UINT64 size = io_generic_size(io);
	if ((size == 901120) || (size == 1802240))
	{
		return 50;
	}
	return 0;
}

bool adf_format::load(io_generic *io, floppy_image *image)
{
	desc_s sectors[11];
	UINT8 sectdata[512*11];
	for(int i=0; i<11; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
	}

	for(int track=0; track < 80; track++) {
		for(int side=0; side < 2; side++) {
			io_generic_read(io, sectdata, (track*2 + side)*512*11, 512*11);
			generate_track(amiga_11, track, side, sectors, 11, 100000, image);
		}
	}

	return TRUE;
}

const floppy_format_type FLOPPY_ADF_FORMAT = &floppy_image_format_creator<adf_format>;
