// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

t10sbc.h

***************************************************************************/

#ifndef MAME_MACHINE_T10SBC_H
#define MAME_MACHINE_T10SBC_H

#pragma once

#include "t10spc.h"
#include "imagedev/harddriv.h"

class t10sbc : public virtual t10spc
{
public:
	t10sbc()
		: t10spc(), m_image(nullptr), m_lba(0), m_blocks(0), m_disk(nullptr), m_device(nullptr)
	{
	}

	virtual void SetDevice( void *device ) override;
	virtual void GetDevice( void **device ) override;
	virtual void ExecCommand() override;
	virtual void WriteData( uint8_t *data, int dataLength ) override;
	virtual void ReadData( uint8_t *data, int dataLength ) override;

protected:
	virtual void t10_start(device_t &device) override;
	virtual void t10_reset() override;

	struct format_page_t
	{
		uint8_t m_page_code;
		uint8_t m_page_length;
		uint8_t m_tracks_per_zone_msb;
		uint8_t m_tracks_per_zone_lsb;
		uint8_t m_alt_sectors_per_zone_msb;
		uint8_t m_alt_sectors_per_zone_lsb;
		uint8_t m_alt_tracks_per_zone_msb;
		uint8_t m_alt_tracks_per_zone_lsb;
		uint8_t m_alt_tracks_per_volume_msb;
		uint8_t m_alt_tracks_per_volume_lsb;
		uint8_t m_sectors_per_track_msb;
		uint8_t m_sectors_per_track_lsb;
		uint8_t m_bytes_per_sector_msb;
		uint8_t m_bytes_per_sector_lsb;
		uint8_t m_interleave_msb;
		uint8_t m_interleave_lsb;
		uint8_t m_track_skew_msb;
		uint8_t m_track_skew_lsb;
		uint8_t m_cylinder_skew_msb;
		uint8_t m_cylinder_skew_lsb;
		uint8_t m_format;
		uint8_t m_reserved[3];
	};

	struct geometry_page_t
	{
		uint8_t m_page_code;
		uint8_t m_page_length;
		uint8_t m_num_cylinders_msb;
		uint8_t m_num_cylinders_2nd;
		uint8_t m_num_cylinders_lsb;
		uint8_t m_num_heads;
		uint8_t m_start_cylinder_msb;
		uint8_t m_start_cylinder_2nd;
		uint8_t m_start_cylinder_lsb;
		uint8_t m_start_cylinder_rwc_msb;
		uint8_t m_start_cylinder_rwc_2nd;
		uint8_t m_start_cylinder_rwc_lsb;
		uint8_t m_step_rate_msb;
		uint8_t m_step_rate_lsb;
		uint8_t m_lz_cylinder_msb;
		uint8_t m_lz_cylinder_2nd;
		uint8_t m_lz_cylinder_lsb;
		uint8_t m_rot_pos_locking;
		uint8_t m_rot_offset;
		uint8_t m_reserved0;
		uint8_t m_rot_rate_msb;
		uint8_t m_rot_rate_lsb;
		uint8_t m_reserved1[2];
	};

	virtual void GetFormatPage( format_page_t *page );
	virtual void GetGeometryPage( geometry_page_t *page );
	virtual void ReadCapacity( uint8_t *data );

	harddisk_image_device *m_image;

	uint32_t m_lba;
	uint32_t m_blocks;

	hard_disk_file *m_disk;
	device_t *m_device;
};

#endif // MAME_MACHINE_T10SBC_H
