// license:BSD-3-Clause
// copyright-holders: Ansgar Kückes, F. Ulivi
/*********************************************************************

	hpi_dsk.cpp

	HP9895A "HPI" disk images

	CHS = 77/2/30
	Sector size 256 bytes
	Cell size 2 µs
	Gap1 = 16 x 0x00
	Gap2 = 34 x 0x00
	Gap3 = ~490 x 0x00 (depends on actual rotation speed)
	Sync = 4 x 0x00 + 4 x 0xff
	ID AM = 0x0e clock + 0x70 data
	Data AM = 0x0e clock + 0x50 data
	DefectTrack AM = 0x0e clock + 0xf0 data
	CRC-16 excludes address markers
	MMFM/M2FM encoding (LS bit first)

	The order of sectors in a track depends on the interleave factor
	which is the distance (in number of sectors) between two consecutively
	numbered sectors. Interleave factor is specified at formatting time.
	The default factor is 7. The order of sectors for this factor is:
	0, 13, 26, 9, 22, 5, 18, 1, 14, 27, 10, 23, 6, 19, 2,
	15, 28, 11, 24, 7, 20, 3, 16, 29, 12, 25, 8, 21, 4, 17

	<Track> := [Index hole]|Sector0|Gap2|Sector1|Gap2|...|Sector29|Gap3|

	<Sector> := ID field|Gap1|Data field

	<ID field> := Sync|ID AM|Track no.|Sector no.|CRC-16|0x00

	<Data field> := Sync|Data AM|Data|CRC-16|0x00

	This format is just a raw image of every sector on a HP-formatted
	8" floppy disk. Files with this format have no header/trailer and
	are exactly 1182720 bytes in size (30 sectors, 2 heads, 77 tracks,
	256 bytes per sector). There's also a "reduced" version holding
	just 75 cylinders.
	When loading, the disk image is translated to MMFM encoding so
	that it can be loaded into HP9895 emulator.

*********************************************************************/

#include "emu.h"
#include "hpi_dsk.h"

// Debugging
#define VERBOSE 1
#define LOG(...)  do { if (VERBOSE) printf(__VA_ARGS__); } while (false)

constexpr unsigned IL_OFFSET	= 0x12;	// Position of interleave factor in HPI image (2 bytes, big-endian)
constexpr unsigned DEFAULT_IL	= 7;	// Default interleaving factor
constexpr unsigned CELL_SIZE	= 1200;	// Bit cell size (1 µs)
constexpr uint8_t  ID_AM        = 0x70;	// ID address mark
constexpr uint8_t  DATA_AM		= 0x50;	// Data address mark
constexpr uint8_t  AM_CLOCK		= 0x0e;	// Clock pattern of AM
constexpr unsigned GAP1_SIZE	= 17;	// Size of gap 1 (+1)
constexpr unsigned GAP2_SIZE	= 35;	// Size of gap 2 (+1)
// Size of image file (holding 77 cylinders)
constexpr unsigned HPI_IMAGE_SIZE = HPI_TRACKS * HPI_HEADS * HPI_SECTORS * HPI_SECTOR_SIZE;
constexpr unsigned HPI_RED_TRACKS = 75;	// Reduced number of tracks
// Size of reduced image file (holding 75 cylinders)
constexpr unsigned HPI_RED_IMAGE_SIZE = HPI_RED_TRACKS * HPI_HEADS * HPI_SECTORS * HPI_SECTOR_SIZE;

hpi_format::hpi_format()
{
}

int hpi_format::identify(io_generic *io, uint32_t form_factor)
{
	uint64_t size = io_generic_size(io);

	// we try to stay back and give only 50 points, since another image
	// format may also have images of the same size (there is no header and no
	// magic number for HPI format...
	if (((form_factor == floppy_image::FF_8) || (form_factor == floppy_image::FF_UNKNOWN)) &&
		((size == HPI_RED_IMAGE_SIZE) || (size == HPI_IMAGE_SIZE))) {
		return 50;
	} else {
		return 0;
	}
}

bool hpi_format::load(io_generic *io, uint32_t form_factor, floppy_image *image)
{
	image->set_variant(floppy_image::DSDD);	// We actually need to derive the variant from the image size depending on the form factor

	uint64_t size = io_generic_size(io);
	unsigned cylinders;
	if (size == HPI_RED_IMAGE_SIZE) {
		cylinders = HPI_RED_TRACKS;
	} else if (size == HPI_IMAGE_SIZE) {
		cylinders = HPI_TRACKS;
	} else {
		return false;
	}

	// Suck in the whole image
	std::vector<uint8_t> image_data(size);
	io_generic_read(io, (void *)image_data.data(), 0, size);

	// Get interleave factor from image
	unsigned il = (unsigned)image_data[ IL_OFFSET ] * 256 + image_data[ IL_OFFSET + 1 ];
	LOG("I/L from image: %u\n" , il);
	if (il < 1 || il >= HPI_SECTORS) {
		il = DEFAULT_IL;
	}
	LOG("Actual I/L: %u\n" , il);

	sector_list_t sector_list;
	interleaved_sectors(il, sector_list);

	unsigned list_offset = 0;
	for (unsigned cyl = 0; cyl < cylinders; cyl++) {
		for (unsigned head = 0; head < HPI_HEADS; head++) {
			std::vector<uint32_t> track_data;
			for (unsigned sector = 0; sector < HPI_SECTORS; sector++) {
				unsigned real_sector = sector_list[ (sector + list_offset) % HPI_SECTORS ];
				unsigned offset_in_image = chs_to_lba(cyl, head, real_sector) * HPI_SECTOR_SIZE;
				write_sector(track_data , cyl , real_sector + (head << 7) , &image_data[ offset_in_image ]);
			}
			fill_with_gap3(track_data);
			generate_track_from_levels(cyl , head , track_data , 0 , image);
			list_offset = (list_offset + m_track_skew[ il - 1 ][ head ]) % HPI_SECTORS;
		}
	}
	return true;
}

bool hpi_format::save(io_generic *io, floppy_image *image)
{
	// TODO: 
	return false;
}

const char *hpi_format::name() const
{
	return "hpi";
}

const char *hpi_format::description() const
{
	return "HP9895A floppy disk image";
}

const char *hpi_format::extensions() const
{
	return "hpi";
}

bool hpi_format::supports_save() const
{
	// TODO: 
	return false;
}

void hpi_format::interleaved_sectors(unsigned il_factor , sector_list_t& sector_list)
{
	sector_list.fill(0xff);

	unsigned idx = HPI_SECTORS - il_factor;
	for (unsigned sect = 0; sect < HPI_SECTORS; sect++) {
		idx = (idx + il_factor) % HPI_SECTORS;
		while (sector_list[ idx ] != 0xff) {
			idx = (idx + 1) % HPI_SECTORS;
		}
		LOG("[%u]=%u\n" , idx , sect);
		sector_list[ idx ] = sect;
	}
}

void hpi_format::write_mmfm_bit(std::vector<uint32_t> &buffer , bool data_bit , bool clock_bit)
{
	bool had_transition = buffer.size() < 2 ? false : bit_r(buffer, buffer.size() - 1) || bit_r(buffer , buffer.size() - 2);
	clock_bit = !data_bit && (clock_bit || !had_transition);
	bit_w(buffer , clock_bit , CELL_SIZE);
	bit_w(buffer , data_bit , CELL_SIZE);
}

void hpi_format::write_mmfm_byte(std::vector<uint32_t> &buffer , uint8_t data , uint8_t clock)
{
	for (unsigned i = 0; i < 8; i++) {
		write_mmfm_bit(buffer , BIT(data , i) , BIT(clock , i));
	}
}

void hpi_format::write_sync(std::vector<uint32_t> &buffer)
{
	// Sync
	// 4x 00
	for (unsigned i = 0; i < 4; i++) {
		write_mmfm_byte(buffer , 0);
	}
	// 4x ff
	for (unsigned i = 0; i < 4; i++) {
		write_mmfm_byte(buffer , 0xff);
	}
}

void hpi_format::write_crc(std::vector<uint32_t> &buffer , uint16_t crc)
{
	// Note that CRC is stored with MSB (x^15) first
	for (unsigned i = 0; i < 16; i++) {
		write_mmfm_bit(buffer , BIT(crc , 15 - i) , 0);
	}
}

void hpi_format::write_sector(std::vector<uint32_t> &buffer , uint8_t track_no , uint8_t sect_head_no , const uint8_t *sect_data)
{
	// **** On-disk format of a sector ****
	//
	// | Offset | Size | Value | Content              |
	// |--------+------+-------+----------------------|
	// |      0 |    4 |    00 | Sync                 |
	// |      4 |    4 |    ff | Sync                 |
	// |      8 |    1 |    70 | ID AM (clock = 0e)   |
	// |      9 |    1 |    xx | Track no.            |
	// |     10 |    1 |    xx | Sector and head no.  |
	// |     11 |    2 |    xx | ID CRC               |
	// |     13 |    1 |    00 | ID tail              |
	// |     14 |   16 |    00 | Gap 1                |
	// |     30 |    4 |    00 | Sync                 |
	// |     34 |    4 |    ff | Sync                 |
	// |     38 |    1 |    50 | Data AM (clock = 0e) |
	// |     39 |  256 |    xx | Sector data          |
	// |    295 |    2 |    xx | Data CRC             |
	// |    297 |    1 |    00 | Data tail            |
	// |    298 |   34 |    00 | Gap 2                |
	// |    332 |      |       |                      |

	// Sync
	write_sync(buffer);
	// ID AM
	write_mmfm_byte(buffer , ID_AM , AM_CLOCK);
	auto crc_start = buffer.size();
	// Track #
	write_mmfm_byte(buffer , track_no);
	// Sector/head #
	write_mmfm_byte(buffer , sect_head_no);
	uint16_t crc = calc_crc_ccitt(buffer , crc_start , buffer.size());
	// ID CRC
	write_crc(buffer , crc);
	// Gap 1
	for (unsigned i = 0; i < GAP1_SIZE; i++) {
		write_mmfm_byte(buffer , 0);
	}
	// Sync
	write_sync(buffer);
	// Data AM
	write_mmfm_byte(buffer , DATA_AM , AM_CLOCK);
	crc_start = buffer.size();
	for (unsigned i = 0; i < HPI_SECTOR_SIZE; i += 2) {
		// Data: bytes are swapped in pairs
		write_mmfm_byte(buffer , sect_data[ i + 1 ]);
		write_mmfm_byte(buffer , sect_data[ i ]);
	}
	crc = calc_crc_ccitt(buffer , crc_start , buffer.size());
	// Data CRC
	write_crc(buffer , crc);
	// Gap 2
	for (unsigned i = 0; i < GAP2_SIZE; i++) {
		write_mmfm_byte(buffer , 0);
	}
}

void hpi_format::fill_with_gap3(std::vector<uint32_t> &buffer)
{
	// Cell count in a track (1 µs cells in a 1/6 s track)
	unsigned cell_count = (500000 * 120) / 360;
	unsigned cells_in_buffer = buffer.size();
	// Size of gap 3
	unsigned gap_3 = (cell_count - cells_in_buffer) / 16;
	// Gap 3
	for (unsigned i = 0; i < gap_3; i++) {
		write_mmfm_byte(buffer , 0);
	}
	// Last cell to round everything up to 2E+8
	if (buffer.size() * CELL_SIZE < 200000000) {
		bit_w(buffer , false , 200000000 - buffer.size() * CELL_SIZE);
	}
}

unsigned hpi_format::chs_to_lba(unsigned cylinder , unsigned head , unsigned sector)
{
	return sector + (head + cylinder * HPI_HEADS) * HPI_SECTORS;
}

// This table comes straight from hp9895 firmware (it's @ 0x0f90)
// For each interleave factor in [1..29] it stores the number of positions
// to move forward in the interleaved sector list when beginning a new track.
// There are different offsets for tracks on head 0 and tracks on head 1.
const uint8_t hpi_format::m_track_skew[ HPI_SECTORS - 1 ][ HPI_HEADS ] = {
	{ 0x1c , 0x18 },	// Interleave = 1
	{ 0x1c , 0x18 },	// Interleave = 2
	{ 0x1c , 0x18 },	// Interleave = 3 
	{ 0x1d , 0x1a },	// Interleave = 4
	{ 0x1a , 0x18 },	// Interleave = 5
	{ 0x19 , 0x18 },	// Interleave = 6
	{ 0x00 , 0x00 },	// Interleave = 7
	{ 0x1d , 0x1d },	// Interleave = 8
	{ 0x1c , 0x1c },	// Interleave = 9
	{ 0x15 , 0x15 },	// Interleave = 10
	{ 0x00 , 0x00 },	// Interleave = 11
	{ 0x19 , 0x19 },	// Interleave = 12
	{ 0x00 , 0x00 },	// Interleave = 13
	{ 0x1d , 0x1d },	// Interleave = 14
	{ 0x10 , 0x10 },	// Interleave = 15
	{ 0x1d , 0x1d },	// Interleave = 16
	{ 0x00 , 0x00 },	// Interleave = 17
	{ 0x19 , 0x19 },	// Interleave = 18
	{ 0x00 , 0x00 },	// Interleave = 19
	{ 0x15 , 0x15 },	// Interleave = 20
	{ 0x1c , 0x1c },	// Interleave = 21
	{ 0x1d , 0x1d },	// Interleave = 22
	{ 0x00 , 0x00 },	// Interleave = 23
	{ 0x19 , 0x19 },	// Interleave = 24
	{ 0x1a , 0x1a },	// Interleave = 25
	{ 0x1d , 0x1d },	// Interleave = 26
	{ 0x1c , 0x1c },	// Interleave = 27
	{ 0x1d , 0x1d },	// Interleave = 28
	{ 0x00 , 0x00 }		// Interleave = 29
};

const floppy_format_type FLOPPY_HPI_FORMAT = &floppy_image_format_creator<hpi_format>;
