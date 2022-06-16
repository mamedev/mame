// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    img_dsk.h

    "IMG" disk format for SSDD Intel MDS-II 8" disks

*********************************************************************/
#ifndef MAME_FORMATS_IMG_DSK_H
#define MAME_FORMATS_IMG_DSK_H

#pragma once

#include "flopimg.h"

#include <vector>

class img_format : public floppy_image_format_t
{
public:
	// Geometry constants
	static constexpr unsigned TRACKS = 77;
	static constexpr unsigned HEADS = 1;
	static constexpr unsigned SECTORS = 52;
	static constexpr unsigned SECTOR_SIZE = 128;

	img_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	static std::vector<uint8_t> interleaved_sectors(unsigned il_factor);
	static void write_mmfm_bit(std::vector<uint32_t> &buffer , bool data_bit , bool clock_bit , uint16_t &crc);
	static void write_mmfm_byte(std::vector<uint32_t> &buffer , uint8_t data , uint16_t &crc , uint8_t clock = 0);
	static void write_sync(std::vector<uint32_t> &buffer);
	static void write_crc(std::vector<uint32_t> &buffer , uint16_t crc);
	static void write_gap(std::vector<uint32_t> &buffer , unsigned size_00 , unsigned size_ff);
	static void write_sector(std::vector<uint32_t> &buffer , uint8_t track_no , uint8_t sect_no , const uint8_t *sect_data);
	static void fill_with_gap4(std::vector<uint32_t> &buffer);
	static std::vector<uint8_t> get_next_id_n_block(const std::vector<bool> &bitstream , int& pos , int& start_pos);
	static bool get_next_sector(const std::vector<bool> &bitstream , int& pos , unsigned& track , unsigned& sector , uint8_t *sector_data);
};

extern const img_format FLOPPY_IMG_FORMAT;

#endif // MAME_FORMATS_IMG_DSK_H
