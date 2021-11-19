// license:BSD-3-Clause
// copyright-holders: Ansgar Kückes, F. Ulivi
/*********************************************************************

    hpi_dsk.h

    "HPI" disk format

*********************************************************************/
#ifndef MAME_FORMATS_HPI_DSK_H
#define MAME_FORMATS_HPI_DSK_H

#pragma once

#include "flopimg.h"

#include <array>
#include <vector>

// Geometry constants
constexpr unsigned HPI_TRACKS = 77;
constexpr unsigned HPI_HEADS = 2;
constexpr unsigned HPI_SECTORS = 30;
constexpr unsigned HPI_SECTOR_SIZE = 256;

class hpi_format : public floppy_image_format_t
{
public:
	hpi_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	typedef std::array<uint8_t , HPI_SECTORS> sector_list_t;
	static bool geometry_from_size(uint64_t image_size , unsigned& heads , unsigned& tracks);
	static void interleaved_sectors(unsigned il_factor , sector_list_t& sector_list);
	void write_mmfm_bit(std::vector<uint32_t> &buffer , bool data_bit , bool clock_bit);
	void write_mmfm_byte(std::vector<uint32_t> &buffer , uint8_t data , uint8_t clock = 0);
	void write_sync(std::vector<uint32_t> &buffer);
	void write_crc(std::vector<uint32_t> &buffer , uint16_t crc);
	void write_sector(std::vector<uint32_t> &buffer , uint8_t track_no , uint8_t sect_head_no , const uint8_t *sect_data);
	void fill_with_gap3(std::vector<uint32_t> &buffer);
	static unsigned chs_to_lba(unsigned cylinder , unsigned head , unsigned sector , unsigned heads);
	std::vector<uint8_t> get_next_id_n_block(const std::vector<bool> &bitstream , int& pos , int& start_pos);
	bool get_next_sector(const std::vector<bool> &bitstream , int& pos , unsigned& track , unsigned& head , unsigned& sector , uint8_t *sector_data);

	static const uint8_t m_track_skew[ HPI_SECTORS - 1 ][ HPI_HEADS ];
};

extern const floppy_format_type FLOPPY_HPI_FORMAT;

#endif // MAME_FORMATS_HPI_DSK_H
