// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VDK

    Disk image format

    Used by Paul Burgin's PC-Dragon emulator

***************************************************************************/

#include "emu.h"
#include "vdk_dsk.h"

vdk_format::vdk_format() : wd177x_format(NULL)
{
	m_format.form_factor = floppy_image::FF_525;
	m_format.encoding = floppy_image::MFM;
	m_format.cell_size = 2000;
	m_format.sector_count = 18;
//	m_format.track_count = 40/80
//	m_format.head_count = 1/2
	m_format.sector_base_size = 256;
	m_format.sector_base_id = 1;
	m_format.gap_1 = 32;
	m_format.gap_2 = 24;
	m_format.gap_3 = 22;
}

const char *vdk_format::name() const
{
	return "vdk";
}

const char *vdk_format::description() const
{
	return "VDK disk image";
}

const char *vdk_format::extensions() const
{
	return "vdk";
}

int vdk_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 id[2];
	io_generic_read(io, id, 0, 2);

	if (id[0] == 'd' && id[1] == 'k')
		return 50;
	else
		return 0;
}

bool vdk_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 header[0x100];
	io_generic_read(io, header, 0, 100);
	int header_size = header[3] * 0x100 + header[2];

	m_format.track_count = header[8];
	m_format.head_count = header[9];

	switch (m_format.head_count)
	{
	case 1: m_format.variant = floppy_image::SSDD; break;
	case 2: m_format.variant = floppy_image::DSDD; break;
	default: return false;
	}

	floppy_image_format_t::desc_e *desc;
	int current_size;
	int end_gap_index;

	desc = get_desc_mfm(m_format, current_size, end_gap_index);

	int total_size = 200000000 / m_format.cell_size;
	int remaining_size = total_size - current_size;
	if (remaining_size < 0)
		throw emu_fatalerror("vdk_format: Incorrect track layout, max_size=%d, current_size=%d", total_size, current_size);

	// fixup the end gap
	desc[end_gap_index].p2 = remaining_size / 16;
	desc[end_gap_index + 1].p2 = remaining_size & 15;
	desc[end_gap_index + 1].p1 >>= 16 - (remaining_size & 15);

	int track_size = compute_track_size(m_format);

	UINT8 sectdata[40*512];
	desc_s sectors[40];
	build_sector_description(m_format, sectdata, sectors);

	for(int track=0; track < m_format.track_count; track++)
		for(int head=0; head < m_format.head_count; head++) {
			io_generic_read(io, sectdata, header_size + get_image_offset(m_format, head, track), track_size);
			generate_track(desc, track, head, sectors, m_format.sector_count, total_size, image);
		}

	image->set_variant(m_format.variant);

	return true;
}

bool vdk_format::save(io_generic *io, floppy_image *image)
{
	int tracks, heads;
	image->get_actual_geometry(tracks, heads);

	m_format.track_count = tracks;
	m_format.head_count = heads;

	switch (m_format.head_count)
	{
	case 1: m_format.variant = floppy_image::SSDD; break;
	case 2: m_format.variant = floppy_image::DSDD; break;
	default: return false;
	}

	// write header
	UINT8 header[12];

	header[0] = 'd';
	header[1] = 'k';
	header[2] = sizeof(header) % 0x100;
    header[3] = sizeof(header) / 0x100;
	header[4] = 0x10;
	header[5] = 0x10;
	header[6] = 'M';
	header[7] = 0x01;
	header[8] = tracks;
	header[9] = heads;
	header[10] = 0;
	header[11] = 0;

	io_generic_write(io, header, 0, sizeof(header));

	// write disk data
	int track_size = compute_track_size(m_format);

	UINT8 sectdata[40*512];
	desc_s sectors[40];
	build_sector_description(m_format, sectdata, sectors);

	for (int track = 0; track < m_format.track_count; track++)
	{
		for (int head = 0; head < m_format.head_count; head++)
		{
			extract_sectors(image, m_format, sectors, track, head);
			io_generic_write(io, sectdata, sizeof(header) + get_image_offset(m_format, head, track), track_size);
		}
	}

	return true;
}

const floppy_format_type FLOPPY_VDK_FORMAT = &floppy_image_format_creator<vdk_format>;
