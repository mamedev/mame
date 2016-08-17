// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/oric_dsk.c

    Oric disk images

*********************************************************************/

#include "emu.h" // logerror
#include "formats/oric_dsk.h"

oric_dsk_format::oric_dsk_format()
{
}

const char *oric_dsk_format::name() const
{
	return "oric_dsk";
}

const char *oric_dsk_format::description() const
{
	return "Oric disk image";
}

const char *oric_dsk_format::extensions() const
{
	return "dsk";
}

bool oric_dsk_format::supports_save() const
{
	return true;
}

int oric_dsk_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 h[256];
	io_generic_read(io, h, 0, 256);

	if(memcmp(h, "MFM_DISK", 8))
		return 0;

	int sides  = (h[11] << 24) | (h[10] << 16) | (h[ 9] << 8) | h[ 8];
	int tracks = (h[15] << 24) | (h[14] << 16) | (h[13] << 8) | h[12];
	int geom   = (h[19] << 24) | (h[18] << 16) | (h[17] << 8) | h[16];

	int size = io_generic_size(io);
	if(sides < 0 || sides > 2 || geom != 1 || size != 256+6400*sides*tracks)
		return 0;

	return 100;
}

bool oric_dsk_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 h[256];
	UINT8 t[6250+3];

	t[6250] = t[6251] = t[6252] = 0;
	io_generic_read(io, h, 0, 256);

	int sides  = (h[11] << 24) | (h[10] << 16) | (h[ 9] << 8) | h[ 8];
	int tracks = (h[15] << 24) | (h[14] << 16) | (h[13] << 8) | h[12];

	for(int side=0; side<sides; side++)
		for(int track=0; track<tracks; track++) {
			io_generic_read(io, t, 256+6400*(tracks*side + track), 6250);
			std::vector<UINT32> stream;
			int sector_size = 128;
			for(int i=0; i<6250; i++) {
				if(t[i] == 0xc2 && t[i+1] == 0xc2 && t[i+2] == 0xc2) {
					raw_w(stream, 16, 0x5224);
					raw_w(stream, 16, 0x5224);
					raw_w(stream, 16, 0x5224);
					i += 2;
					continue;
				}
				if(t[i] == 0xa1 && t[i+1] == 0xa1 && t[i+2] == 0xa1) {
					raw_w(stream, 16, 0x4489);
					raw_w(stream, 16, 0x4489);
					raw_w(stream, 16, 0x4489);
					int copy;
					if(t[i+3] == 0xfe) {
						copy = 7;
						sector_size = 128 << (t[i+7] & 3);
					} else if(t[i+3] == 0xfb)
						copy = sector_size+3;
					else
						copy = 0;
					for(int j=0; j<copy; j++)
						mfm_w(stream, 8, t[i+3+j]);
					i += 2+copy;
					continue;
				}
				mfm_w(stream, 8, t[i]);
			}
			generate_track_from_levels(track, side, stream, 0, image);
		}

	return true;
}

bool oric_dsk_format::save(io_generic *io, floppy_image *image)
{
	return true;
}

const floppy_format_type FLOPPY_ORIC_DSK_FORMAT = &floppy_image_format_creator<oric_dsk_format>;
