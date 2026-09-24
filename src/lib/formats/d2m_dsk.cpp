// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d2m_dsk.cpp

    CMD FD-2000/FD-4000 disk image formats (D1M/D2M/D4M)

    D1M - DD container disk (theoretical; no released CMD hardware)
    D2M - CMD FD-2000 HD 1.6 MB disk
    D4M - CMD FD-4000 ED 3.2 MB disk

    Each image is a flat sector dump containing 81 cylinders.  The
    final cylinder is the CMD system partition.  Within each cylinder,
    the file stores the half-track whose ID fields contain H=0 first,
    followed by the half-track whose ID fields contain H=1.

    CMD's side-select wiring is inverted relative to the H value in
    the ID field.  Thus an FDC command with HDS=1 accesses ID head 0,
    and HDS=0 accesses ID head 1.  HEAD_ID_SWAP creates the correct ID
    fields; load() and save() below also swap the two file half-tracks
    so their sector payload follows the corresponding ID fields.

    Filesystem/container details:
    https://ist.uwaterloo.ca/~schepers/formats/D2M-DNP.TXT

*********************************************************************/

#include "formats/d2m_dsk.h"

#include "ioprocs.h"
#include "osdcore.h"

#include <vector>


//**************************************************************************
//  COMMON CMD FD FORMAT IMPLEMENTATION
//**************************************************************************

cmd_fd_format::cmd_fd_format(const format *formats)
	: upd765_format(formats)
	, m_formats(formats)
{
}


floppy_image_format_t::desc_e *cmd_fd_format::get_desc_mfm_cmd(
		const format &f, int &current_size, int &end_gap_index) const
{
	static floppy_image_format_t::desc_e desc[29] = {
		/* 00 */ { MFM, 0x4e, 0 },
		/* 01 */ { MFM, 0x00, 12 },
		/* 02 */ { RAW, 0x5224, 3 },
		/* 03 */ { MFM, 0xfc, 1 },
		/* 04 */ { MFM, 0x4e, 0 },
		/* 05 */ { SECTOR_LOOP_START, 0, 0 },
		/* 06 */ {   MFM, 0x00, 12 },
		/* 07 */ {   CRC_CCITT_START, 1, 0 },
		/* 08 */ {     RAW, 0x4489, 3 },
		/* 09 */ {     MFM, 0xfe, 1 },
		/* 10 */ {     TRACK_ID, 0, 0 },
		/* 11 */ {     HEAD_ID_SWAP, 0, 0 },
		/* 12 */ {     SECTOR_ID, 0, 0 },
		/* 13 */ {     SIZE_ID, 0, 0 },
		/* 14 */ {   CRC_END, 1, 0 },
		/* 15 */ {   CRC, 1, 0 },
		/* 16 */ {   MFM, 0x4e, 0 },
		/* 17 */ {   MFM, 0x00, 12 },
		/* 18 */ {   CRC_CCITT_START, 2, 0 },
		/* 19 */ {     RAW, 0x4489, 3 },
		/* 20 */ {     MFM, 0xfb, 1 },
		/* 21 */ {     SECTOR_DATA, -1, 0 },
		/* 22 */ {   CRC_END, 2, 0 },
		/* 23 */ {   CRC, 2, 0 },
		/* 24 */ {   MFM, 0x4e, 0 },
		/* 25 */ { SECTOR_LOOP_END, 0, 0 },
		/* 26 */ { MFM, 0x4e, 0 },
		/* 27 */ { RAWBITS, 0x9254, 0 },
		/* 28 */ { END, 0, 0 }
	};

	desc[0].p2  = f.gap_4a;
	desc[4].p2  = f.gap_1;
	desc[5].p2  = f.sector_count - 1;
	desc[16].p2 = f.gap_2;
	desc[24].p2 = f.gap_3;

	current_size = (f.gap_4a + 12 + 3 + 1 + f.gap_1) * 16;
	if (f.sector_base_size)
		current_size += f.sector_base_size * f.sector_count * 16;
	else
	{
		for (int sector = 0; sector != f.sector_count; sector++)
			current_size += f.per_sector_size[sector] * 16;
	}

	current_size +=
		(12 + 3 + 1 + 4 + 2 + f.gap_2 + 12 + 3 + 1 + 2 + f.gap_3)
		* f.sector_count * 16;

	end_gap_index = 26;
	return desc;
}


bool cmd_fd_format::load(util::random_read &io, uint32_t form_factor,
		const std::vector<uint32_t> &variants, floppy_image &image) const
{
	// Every concrete CMD format currently has exactly one entry.
	const format &f = m_formats[0];

	if (form_factor != floppy_image::FF_UNKNOWN && form_factor != f.form_factor)
		return false;
	if (!variants.empty() && !has_variant(variants, f.variant))
		return false;

	uint64_t file_size;
	if (io.length(file_size))
		return false;

	const int track_size = compute_track_size(f);
	const uint64_t expected_size =
		uint64_t(track_size) * f.track_count * f.head_count;
	if (file_size != expected_size)
		return false;

	int image_tracks;
	int image_heads;
	image.get_maximal_geometry(image_tracks, image_heads);
	if (f.track_count > image_tracks || f.head_count > image_heads)
		return false;

	int current_size;
	int end_gap_index;
	floppy_image_format_t::desc_e *const desc =
		get_desc_mfm_cmd(f, current_size, end_gap_index);

	const int total_size = 200000000 / f.cell_size;
	const int remaining_size = total_size - current_size;
	if (remaining_size < 0)
	{
		osd_printf_error(
			"cmd_fd_format: incorrect track layout, max_size=%d, current_size=%d\n",
			total_size,
			current_size);
		return false;
	}

	// Fill the unused tail of the revolution with 0x4e, followed by any
	// remaining raw bit cells.
	desc[end_gap_index].p2 = remaining_size / 16;
	desc[end_gap_index + 1].p2 = remaining_size & 15;
	desc[end_gap_index + 1].p1 = 0x9254 >> (16 - (remaining_size & 15));

	std::vector<uint8_t> sector_data(track_size);
	std::vector<desc_s> sectors(f.sector_count);

	for (int track = 0; track < f.track_count; track++)
	{
		for (int image_head = 0; image_head < f.head_count; image_head++)
		{
			// File order follows the stored H field, while the physical
			// side selected by the controller is inverted.
			const int file_head = image_head ^ 1;
			const uint64_t offset =
				uint64_t(track * f.head_count + file_head) * track_size;

			auto const [error, actual] = util::read_at(
				io, offset, sector_data.data(), sector_data.size());
			if (error || actual != sector_data.size())
				return false;

			build_sector_description(
				f, sector_data.data(), sectors.data(), track, image_head);
			generate_track(
				desc,
				track,
				image_head,
				sectors.data(),
				f.sector_count,
				total_size,
				image);
		}
	}

	image.set_form_variant(f.form_factor, f.variant);
	return true;
}


bool cmd_fd_format::save(util::random_read_write &io,
		const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	const format &f = m_formats[0];
	if (!variants.empty() && !has_variant(variants, f.variant))
		return false;

	const int track_size = compute_track_size(f);
	std::vector<uint8_t> sector_data(track_size);
	std::vector<desc_s> sectors(f.sector_count);

	for (int track = 0; track < f.track_count; track++)
	{
		for (int image_head = 0; image_head < f.head_count; image_head++)
		{
			build_sector_description(
				f, sector_data.data(), sectors.data(), track, image_head);
			extract_sectors(image, f, sectors.data(), track, image_head);

			const int file_head = image_head ^ 1;
			const uint64_t offset =
				uint64_t(track * f.head_count + file_head) * track_size;

			auto const [error, actual] = util::write_at(
				io, offset, sector_data.data(), sector_data.size());
			if (error || actual != sector_data.size())
				return false;
		}
	}

	return true;
}


//**************************************************************************
//  D1M: DD, 10 * 512-BYTE SECTORS
//**************************************************************************

d1m_format::d1m_format()
	: cmd_fd_format(formats)
{
}

const char *d1m_format::name() const noexcept
{
	return "d1m";
}

const char *d1m_format::description() const noexcept
{
	return "CMD FD-2000 disk image";
}

const char *d1m_format::extensions() const noexcept
{
	return "d1m";
}

const d1m_format::format d1m_format::formats[] = {
	// 81 cylinders, 2 sides, 10 sectors, 512 bytes, DD (250 kbit/s)
	{
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 81, 2, 512, {}, 1, {}, 80, 50, 22, 35
	},
	{}
};


//**************************************************************************
//  D2M: HD, 10 * 1024-BYTE SECTORS
//**************************************************************************

d2m_format::d2m_format()
	: cmd_fd_format(formats)
{
}

const char *d2m_format::name() const noexcept
{
	return "d2m";
}

const char *d2m_format::description() const noexcept
{
	return "CMD FD-2000 disk image";
}

const char *d2m_format::extensions() const noexcept
{
	return "d2m";
}

const d2m_format::format d2m_format::formats[] = {
	// 81 cylinders, 2 sides, 10 sectors, 1024 bytes, HD (500 kbit/s)
	{
		floppy_image::FF_35, floppy_image::DSHD, floppy_image::MFM,
		1000, 10, 81, 2, 1024, {}, 1, {}, 80, 50, 22, 100
	},
	{}
};


//**************************************************************************
//  D4M: ED, 20 * 1024-BYTE SECTORS
//**************************************************************************

d4m_format::d4m_format()
	: cmd_fd_format(formats)
{
}

const char *d4m_format::name() const noexcept
{
	return "d4m";
}

const char *d4m_format::description() const noexcept
{
	return "CMD FD-4000 disk image";
}

const char *d4m_format::extensions() const noexcept
{
	return "d4m";
}

const d4m_format::format d4m_format::formats[] = {
	// 81 cylinders, 2 sides, 20 sectors, 1024 bytes, ED (1000 kbit/s)
	{
		floppy_image::FF_35, floppy_image::DSED, floppy_image::MFM,
		500, 20, 81, 2, 1024, {}, 1, {}, 80, 50, 41, 100
	},
	{}
};


const d1m_format FLOPPY_D1M_FORMAT;
const d2m_format FLOPPY_D2M_FORMAT;
const d4m_format FLOPPY_D4M_FORMAT;
