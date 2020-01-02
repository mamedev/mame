// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    img_dsk.cpp

    "IMG" disk format

    This format is just a raw image of every sector on SSDD 8" floppy
    disks as used in Intel MDS-II systems.
    Files with this format have no header/trailer and are exactly
	512512 bytes in size (52 sectors, 1 head, 77 tracks,
    128 bytes per sector).

*********************************************************************/

#include "emu.h"
#include "img_dsk.h"

// Debugging
#define VERBOSE 0
#define LOG(...)  do { if (VERBOSE) printf(__VA_ARGS__); } while (false)

constexpr unsigned CELL_SIZE    = 1200; // Bit cell size (1 µs)
constexpr uint8_t  INDEX_AM		= 0x0c;	// Index address mark
constexpr uint8_t  ID_AM        = 0x0e; // ID address mark
constexpr uint8_t  DATA_AM      = 0x0b; // Data address mark
constexpr uint8_t  AM_CLOCK     = 0x70; // Clock pattern of AM
constexpr unsigned PREIDX_GAP	= 45;	// Size of pre-index gap
//constexpr unsigned GAP1_SIZE    = 28;   // Size of gap 1
//constexpr unsigned GAP2_SIZE    = 28;   // Size of gap 2
//constexpr int ID_DATA_OFFSET = 35 * 16; // Nominal distance (in cells) between ID & DATA AM
// Size of image file
constexpr unsigned IMG_IMAGE_SIZE = IMG_TRACKS * IMG_HEADS * IMG_SECTORS * IMG_SECTOR_SIZE;
constexpr uint16_t CRC_POLY		= 0x1021;	// CRC-CCITT

img_format::img_format()
{
}

int img_format::identify(io_generic *io, uint32_t form_factor)
{
	uint64_t size = io_generic_size(io);

	if (((form_factor == floppy_image::FF_8) || (form_factor == floppy_image::FF_UNKNOWN)) &&
		size == IMG_IMAGE_SIZE) {
		return 50;
	} else {
		return 0;
	}
}

bool img_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
{
	uint64_t size = io_generic_size(io);
	if (size != IMG_IMAGE_SIZE) {
		return false;
	}
	image->set_variant(floppy_image::SSDD);

	// Suck in the whole image
	std::vector<uint8_t> image_data(size);
	io_generic_read(io, (void *)image_data.data(), 0, size);

	for (unsigned cyl = 0; cyl < IMG_TRACKS; cyl++) {
		std::vector<uint32_t> track_data;

		write_gap(track_data, 0 , PREIDX_GAP);
		write_mmfm_byte(track_data , INDEX_AM , AM_CLOCK);
		
		for (unsigned sector = 0; sector < IMG_SECTORS; sector++) {
			unsigned offset_in_image = (sector + cyl * IMG_SECTORS) * IMG_SECTOR_SIZE;
			write_sector(track_data , cyl , sector + 1 , &image_data[ offset_in_image ]);
		}
		fill_with_gap4(track_data);
		generate_track_from_levels(cyl , 0 , track_data , 0 , image);
	}
	return true;
}

bool img_format::save(io_generic *io, floppy_image *image)
{
	// TODO:
	return false;
}

const char *img_format::name() const
{
	return "img";
}

const char *img_format::description() const
{
	return "MDS-II floppy disk image";
}

const char *img_format::extensions() const
{
	return "img";
}

bool img_format::supports_save() const
{
	// TODO: 
	return false;
}

void img_format::write_mmfm_bit(std::vector<uint32_t> &buffer , bool data_bit , bool clock_bit)
{
	bool had_transition = buffer.size() < 2 ? false : bit_r(buffer, buffer.size() - 1) || bit_r(buffer , buffer.size() - 2);
	clock_bit = !data_bit && (clock_bit || !had_transition);
	bit_w(buffer , clock_bit , CELL_SIZE);
	bit_w(buffer , data_bit , CELL_SIZE);

	if (BIT(m_crc , 15) ^ data_bit) {
		m_crc = (m_crc << 1) ^ CRC_POLY;
	} else {
		m_crc <<= 1;
	}
}

void img_format::write_mmfm_byte(std::vector<uint32_t> &buffer , uint8_t data , uint8_t clock)
{
	for (int i = 7; i >= 0; i--) {
		write_mmfm_bit(buffer , BIT(data , i) , BIT(clock , i));
	}
}

void img_format::write_sync(std::vector<uint32_t> &buffer)
{
	write_gap(buffer , 18 , 10);
}

void img_format::write_crc(std::vector<uint32_t> &buffer , uint16_t crc)
{
	// Note that CRC is stored with MSB (x^15) first
	for (unsigned i = 0; i < 16; i++) {
		write_mmfm_bit(buffer , BIT(crc , 15 - i) , 0);
	}
}

void img_format::write_gap(std::vector<uint32_t> &buffer , unsigned size_00 , unsigned size_ff)
{
	for (unsigned i = 0; i < size_00; ++i) {
		write_mmfm_byte(buffer, 0);
	}
	for (unsigned i = 0; i < size_ff; ++i) {
		write_mmfm_byte(buffer, 0xff);
	}
}

void img_format::write_sector(std::vector<uint32_t> &buffer , uint8_t track_no , uint8_t sect_no , const uint8_t *sect_data)
{
	// **** On-disk format of a sector ****
	//
	// | Offset | Size | Value | Content              |
	// |--------+------+-------+----------------------|
	// |      0 |   18 |    00 | Gap 1/3              |
	// |     18 |   10 |    ff | Gap 1/3              |
	// |     28 |    1 |    0e | ID AM (clock = 70)   |
	// |     29 |    1 |    xx | Track no.            |
	// |     30 |    1 |    00 | N/U                  |
	// |     31 |    1 |    xx | Sector no.           |
	// |     32 |    1 |    00 | N/U                  |
	// |     33 |    2 |    xx | ID CRC               |
	// |     35 |   18 |    00 | Gap 2                |
	// |     53 |   10 |    ff | Gap 2                |
	// |     63 |    1 |    0b | Data AM (clock = 70) |
	// |     64 |  128 |    xx | Sector data          |
	// |    192 |    2 |    xx | Data CRC             |
	// |    194 |      |       |                      |

	// Gap1
	write_sync(buffer);
	// ID AM
	m_crc = 0;
	write_mmfm_byte(buffer , ID_AM , AM_CLOCK);
	// Track #
	write_mmfm_byte(buffer , track_no);
	write_mmfm_byte(buffer , 0);
	// Sector #
	write_mmfm_byte(buffer , sect_no);
	write_mmfm_byte(buffer , 0);
	// ID CRC
	write_crc(buffer , m_crc);
	// Gap 2
	write_sync(buffer);
	// Data AM
	m_crc = 0;
	write_mmfm_byte(buffer , DATA_AM , AM_CLOCK);
	for (unsigned i = 0; i < IMG_SECTOR_SIZE; i++) {
		// Data
		write_mmfm_byte(buffer , sect_data[ i ]);
	}
	// Data CRC
	write_crc(buffer , m_crc);
}

void img_format::fill_with_gap4(std::vector<uint32_t> &buffer)
{
	// Cell count in a track (1 µs cells in a 1/6 s track)
	unsigned cell_count = (500000 * 120) / 360;
	unsigned cells_in_buffer = buffer.size();
	// Size of gap 4
	unsigned gap_4 = (cell_count - cells_in_buffer) / 16;
	// Gap 4
	write_gap(buffer , gap_4 , 0);
	// Last cell to round everything up to 2E+8
	if (buffer.size() * CELL_SIZE < 200000000) {
		bit_w(buffer , false , 200000000 - buffer.size() * CELL_SIZE);
	}
}

const floppy_format_type FLOPPY_IMG_FORMAT = &floppy_image_format_creator<img_format>;
