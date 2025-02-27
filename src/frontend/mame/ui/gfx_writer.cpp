// license:BSD-3-Clause
// copyright-holders:Fabrice Lambert

#include <filesystem>

#include "fileio.h"
#include "main.h"
#include "png.h"
#include "video.h"
#include "ui.h"
#include "emu.h"
#include "gfx_writer.h"

bitmap_argb32 gfx_writer::getBitmap(int xCells, int yCells, gfx_viewer::gfxset::setinfo& set, gfx_element& gfx) const
{
	auto cellXpix{ (set.m_rotate & ORIENTATION_SWAP_XY) ? gfx.height() : gfx.width() };
	auto cellYpix{ (set.m_rotate & ORIENTATION_SWAP_XY) ? gfx.width() : gfx.height() };

	//bitmap_rgb32 bitmap;
	bitmap_argb32 bitmap;
	bitmap.reset();
	bitmap.allocate(cellXpix * xCells, cellYpix * yCells);

	// loop over rows
	for (int y{ 0 }; y < yCells; ++y)
	{
		// make a rect that covers this row
		rectangle cellBounds{ 0, bitmap.width() - 1, y * cellYpix, (y + 1) * cellYpix - 1 };

		// only display if there is data to show
		if (y * xCells < gfx.elements())
		{
			// draw the individual cells
			for (int x{ 0 }; x < xCells; ++x)
			{
				int index{ y * xCells + x };

				// update the bounds for this cell
				cellBounds.min_x = x * cellXpix;
				cellBounds.max_x = (x + 1) * cellXpix - 1;

				// only render if there is data
				if (index < gfx.elements())
					drawCell(gfx, index, bitmap, cellBounds.min_x, cellBounds.min_y, set.m_color, set.m_rotate, set.m_palette);
				// otherwise, fill with transparency
				else
					bitmap.fill(0, cellBounds);
			}
		}
		// otherwise, fill with transparency
		else
			bitmap.fill(0, cellBounds);
	}
	return bitmap;
}

void gfx_writer::drawCell(gfx_element& gfx, int index, bitmap_argb32& bitmap, int dstx, int dsty, int color, int rotate, device_palette_interface* dpalette) const
{
	auto width{ (rotate & ORIENTATION_SWAP_XY) ? gfx.height() : gfx.width() };
	auto height{ (rotate & ORIENTATION_SWAP_XY) ? gfx.width() : gfx.height() };
	rgb_t const* const palette{ dpalette->palette()->entry_list_raw() + gfx.colorbase() + color * gfx.granularity() };

	// loop over rows in the cell
	for (u16 y{ 0 }; y < height; ++y)
	{
		uint32_t* dest{ &bitmap.pix(dsty + y, dstx) };
		const uint8_t* src{ gfx.get_data(index) };

		// loop over columns in the cell
		for (u16 x{ 0 }; x < width; ++x)
		{
			int effx{ x };
			int effy{ y };
			const uint8_t* s;
			// compute effective x,y values after rotation
			if (!(rotate & ORIENTATION_SWAP_XY))
			{
				if (rotate & ORIENTATION_FLIP_X)
					effx = gfx.width() - 1 - effx;
				if (rotate & ORIENTATION_FLIP_Y)
					effy = gfx.height() - 1 - effy;
			}
			else
			{
				int temp;
				if (rotate & ORIENTATION_FLIP_X)
					effx = gfx.height() - 1 - effx;
				if (rotate & ORIENTATION_FLIP_Y)
					effy = gfx.width() - 1 - effy;
				temp = effx; effx = effy; effy = temp;
			}

			// get a pointer to the start of this source row
			s = src + effy * gfx.rowbytes();

			// extract the pixel
			*dest++ = palette[s[effx]];
		}
	}
}

gfx_writer::gfx_writer(running_machine& machine, gfx_viewer::gfxset& gfxSet) :
	mMachine{ machine },
	mGfxSet{ gfxSet }
{
}

void gfx_writer::writePng()
{
	// get graphics info
	auto& info{ mGfxSet.m_devices[mGfxSet.m_device] };
	auto& set{ info.set(mGfxSet.m_set) };
	device_gfx_interface& interface{ info.interface() };
	gfx_element& gfx{ *interface.gfx(mGfxSet.m_set) };

	// Compute the number of cells in the x and y directions
	u32 xCells{ 32 };
	u32 yCells{ (gfx.elements() + xCells - 1) / xCells };
	bitmap_argb32 bitmap{ getBitmap(xCells, yCells, set, gfx) };

	// add two text entries describing the image
	std::string text1 = std::string(emulator_info::get_appname()).append(" ").append(emulator_info::get_build_version());
	std::string text2 = std::string(mMachine.system().manufacturer).append(" ").append(mMachine.system().type.fullname());
	util::png_info pnginfo;
	pnginfo.add_text("Software", text1);
	pnginfo.add_text("System", text2);

	emu_file file{ "gfxsave", OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS };
	std::error_condition const filerr = mMachine.video().open_next(file, "png");
	if (!filerr)
	{
		// now do the actual work
		const rgb_t* palette{ (set.m_palette != nullptr) ? set.m_palette->palette()->entry_list_adjusted() : nullptr };
		unsigned entries{ (set.m_palette != nullptr) ? gfx.palette().entries() : 0 };
		std::error_condition const error = util::png_write_bitmap(file, &pnginfo, bitmap, entries, palette);
		if (error)
			osd_printf_error("Error generating PNG for snapshot (%s:%d %s)\n", error.category().name(), error.value(), error.message());
	}
}
