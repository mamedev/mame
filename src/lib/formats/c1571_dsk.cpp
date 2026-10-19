// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/c1571_dsk.cpp

    Commodore 1571 native MFM disk image formats (read via the WD1770)

*********************************************************************/

#include "formats/c1571_dsk.h"

c1571_format::c1571_format() : wd177x_format(formats)
{
}

const char *c1571_format::name() const noexcept
{
	return "c1571";
}

const char *c1571_format::description() const noexcept
{
	return "Commodore 1571 MFM disk image";
}

const char *c1571_format::extensions() const noexcept
{
	return "dsk,img";
}

bool c1571_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	if (!wd177x_format::load(io, form_factor, variants, image))
		return false;

	int tracks, heads;
	image.get_maximal_geometry(tracks, heads);

	for (int track = (tracks - 1) / 2; track > 0; track--)
	{
		for (int head = 0; head < heads; head++)
		{
			image.get_buffer(track * 2, head) = std::move(image.get_buffer(track, head));
			image.get_buffer(track, head).clear();
			image.set_write_splice_position(track * 2, head, image.get_write_splice_position(track, head));
		}
	}

	return true;
}

bool c1571_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int tracks, heads;
	image.get_maximal_geometry(tracks, heads);

	floppy_image single(tracks, heads, image.get_form_factor());
	single.set_variant(image.get_variant());

	for (int track = 0; track * 2 < tracks; track++)
	{
		for (int head = 0; head < heads; head++)
		{
			single.get_buffer(track, head) = image.get_buffer(track * 2, head);
			single.set_write_splice_position(track, head, image.get_write_splice_position(track * 2, head));
		}
	}

	return wd177x_format::save(io, variants, single);
}

const c1571_format::format c1571_format::formats[] = {
	// MS-DOS 360KB
	// 80x4e 12x00 3xf6 fc 
	// 50x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 09 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 755x4e
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 9, 40, 2, 512, {}, 1, { 1,6,2,7,3,8,4,9,5 }, 50, 22, 23
	},

	// MS-DOS 320KB
	// 80x4e 12x00 3xf6 fc
	// 50x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 1352x4e
	{
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 8, 40, 2, 512, {}, 1, { 1,4,7,2,5,8,3,6 }, 50, 22, 23
	},

	// MS-DOS 180KB
	// 80x4e 12x00 3xf6 fc
	// 50x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 09 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 755x4e
	{
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 9, 40, 1, 512, {}, 1, { 1,6,2,7,3,8,4,9,5 }, 50, 22, 23
	},

	// MS-DOS 160KB
	// 80x4e 12x00 3xf6 fc
	// 50x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 23x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 1352x4e
	{
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 8, 40, 1, 512, {}, 1, { 1,4,7,2,5,8,3,6 }, 50, 22, 23
	},

    // Osborne 1
    {
        floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
        4000, 10, 40, 1, 256, {}, 1, {}, 14, 11, 12
    },

    // Osborne Executive
    {
        floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
        2000, 5, 40, 1, 1024, {}, 1, {}, 80, 22, 80
    },

    // Kaypro II
    {
        floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
        2000, 10, 40, 1, 512, {}, 0, {}, 80, 22, 26
    },

    // Kaypro IV / 2X
    {
        floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
        2000, 10, 40, 2, 512, {}, 0, {}, 80, 22, 26
    },

    // Epson QX-10
    {
        floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
        2000, 16, 40, 2, 256, {}, 1, {}, 32, 22, 54
    },

  	{}
};

const c1571_format FLOPPY_C1571_FORMAT;
