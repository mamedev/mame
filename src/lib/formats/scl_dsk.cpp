// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*********************************************************************

    formats/scl_dsk.cpp

    SCL disk images

    SCL is a compact representation of TR-DOS filesystem content.
    It stores only file headers and used sectors, not full disk geometry.
    On load, an empty TRD disk is built in memory and files are placed
    sequentially.

    SCL file structure:
      Offset      Length    Description
      0x0000      8         Signature "SINCLAIR"
      0x0008      1         Number of files (N)
      0x0009      N*14      File header entries (TR-DOS directory entry
                            without the last 2 bytes: sector, track)
      0x0009+N*14 DATA_LEN  File data blocks (each file's sectors concatenated,
                            sector count x 256 bytes per file)

    File header entry (14 bytes):
      Offset      Length    Description
      0x00        8         Filename (space-padded)
      0x08        1         File type ('B'=Basic, 'C'=Code, 'D'=Data, '#'=Seq)
      0x09        2         Parameter 1 (start address for B/C, array info for D)
      0x0B        2         Parameter 2 (length for B/C/D)
      0x0D        1         File length in sectors

    The generated TRD disk uses DS80 geometry (80 tracks, 2 sides, 16
    sectors/track, 256 bytes/sector). Track numbering is side-interleaved:
    logical track 0 = side 0 track 0, 1 = side 1 track 0, 2 = side 0 track 1,
    etc. Sector numbers in directory entries and sector 9 are 0-indexed.

    Reference: https://sinclair.wiki.zxnet.co.uk/wiki/SCL_format

*********************************************************************/

#include "formats/scl_dsk.h"

#include "ioprocs.h"

#include <cstring>
#include <tuple>

namespace {

#pragma pack(push, 1)
struct scl_hdr {
	uint8_t sig[8];        // "SINCLAIR"
	uint8_t file_cnt;
};

struct trd_sector9 {
	uint8_t zero;          // 0x00
	uint8_t reserved1[224];// 0x01-0xE0
	uint8_t first_free_sec;// 0xE1
	uint8_t first_free_trk;// 0xE2
	uint8_t disk_type;     // 0xE3 - 16-DS80, 17-DS40, 18-SS80, 19-SS40
	uint8_t file_cnt;      // 0xE4
	uint16_t free_sec_cnt; // 0xE5-E6
	uint8_t trdos_sig;     // 0xE7
	uint8_t reserved2[2];  // 0xE8-E9
	uint8_t reserved3[9];  // 0xEA-F2
	uint8_t reserved4;     // 0xF3
	uint8_t deleted_cnt;   // 0xF4
	uint8_t label[8];      // 0xF5-FC
	uint8_t reserved5[3];  // 0xFD-FF
};

struct trd_dir_entry {
	char name[8];
	uint8_t type;
	uint16_t start;
	uint16_t length;
	uint8_t sec_cnt;       // file size in sectors
	uint8_t sec;           // 0-indexed sector within 16-sector group
	uint8_t trk;           // combined logical track (side-interleaved)
};
#pragma pack(pop)

static constexpr unsigned SPT = 16;             // sectors per track (per side)
static constexpr unsigned SEC_SIZE = 256;       // sector size
static constexpr unsigned TRK0_RESERVED = 16;   // sectors reserved on track 0 (directory + info)

} // anonymous namespace


scl_format::scl_format() : wd177x_format(formats)
{
}


const char *scl_format::name() const noexcept
{
	return "scl";
}

const char *scl_format::description() const noexcept
{
	return "SCL floppy disk image";
}

const char *scl_format::extensions() const noexcept
{
	return "scl";
}

int scl_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t sig[8];
	std::error_condition err;
	size_t actual;
	std::tie(err, actual) = read_at(io, 0, sig, 8);
	if (err || actual != 8)
		return 0;

	if (memcmp(sig, "SINCLAIR", 8) == 0)
		return FIFID_SIGN;

	return 0;
}

bool scl_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t image_size;
	if (io.length(image_size))
		return false;

	std::unique_ptr<uint8_t[]> scl_data = std::make_unique<uint8_t[]>(image_size);
	{
		std::error_condition err;
		size_t actual;
		std::tie(err, actual) = read_at(io, 0, scl_data.get(), image_size);
		if (err || actual != image_size)
			return false;
	}

	const scl_hdr *hdr = reinterpret_cast<const scl_hdr *>(scl_data.get());
	unsigned file_cnt = hdr->file_cnt;
	if (file_cnt > 128)
		return false;

	const scl_entry *entries = reinterpret_cast<const scl_entry *>(scl_data.get() + sizeof(scl_hdr));

	// Always DS80, geometry determined by sector 9 disk_type
	constexpr unsigned disk_type = 0x16;
	constexpr unsigned tracks = 80;
	constexpr unsigned heads = 2;

	unsigned max_sectors = tracks * heads * SPT;
	unsigned user_sectors = max_sectors - TRK0_RESERVED;

	// Build raw TRD image in memory
	std::unique_ptr<uint8_t[]> trd = std::make_unique<uint8_t[]>(max_sectors * SEC_SIZE);
	memset(trd.get(), 0, max_sectors * SEC_SIZE);

	// Initialise sector 9 (disk info) at byte offset 0x800
	{
		auto *s9 = reinterpret_cast<trd_sector9 *>(trd.get() + 8 * SEC_SIZE);
		s9->first_free_sec = 0;
		s9->first_free_trk = 1;
		s9->disk_type = disk_type;
		s9->file_cnt = 0;
		s9->free_sec_cnt = user_sectors;
		s9->trdos_sig = 0x10;
		memset(s9->label, ' ', 8);
	}

	// Add files
	const uint8_t *file_data = scl_data.get() + sizeof(scl_hdr) + file_cnt * sizeof(scl_entry);
	unsigned free_pos = TRK0_RESERVED; // data starts at logical track 1
	unsigned dir_count = 0;

	for (unsigned i = 0; i < file_cnt; i++) {
		unsigned sec_cnt = entries[i].sec_count;
		if (sec_cnt == 0)
			continue;

		// Write directory entry (entries are packed at start of track 0)
		auto *d = reinterpret_cast<trd_dir_entry *>(trd.get() + dir_count * sizeof(trd_dir_entry));
		memcpy(d->name, entries[i].name, 8);
		d->type = entries[i].type;
		d->start = entries[i].start[0] | (entries[i].start[1] << 8);
		d->length = entries[i].length[0] | (entries[i].length[1] << 8);
		d->sec_cnt = sec_cnt;
		d->sec = free_pos % SPT;   // 0-indexed sector within 16-sector group
		d->trk = free_pos / SPT;   // combined logical track (side-interleaved)

		// Write file data sectors
		for (unsigned s = 0; s < sec_cnt; s++)
			memcpy(trd.get() + (free_pos + s) * SEC_SIZE, file_data + s * SEC_SIZE, SEC_SIZE);

		file_data += sec_cnt * SEC_SIZE;
		free_pos += sec_cnt;
		dir_count++;

		// Update sector 9
		auto *s9 = reinterpret_cast<trd_sector9 *>(trd.get() + 8 * SEC_SIZE);
		s9->file_cnt = dir_count;
		s9->free_sec_cnt = user_sectors - (free_pos - TRK0_RESERVED);
		s9->first_free_sec = free_pos % SPT;
		s9->first_free_trk = free_pos / SPT;
	}


	// Wrap as in-memory random_read and delegate to parent
	util::random_read::ptr trd_io = util::ram_read(trd.get(), max_sectors * SEC_SIZE);
	return wd177x_format::load(*trd_io, form_factor, variants, image);
}

const scl_format::format scl_format::formats[] = {
	{   // 5"25 640K 80 track double sided double density
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 16, 80, 2, 256, {}, -1, { 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 }, 10, 22, 60
	},
	{}
};

const scl_format FLOPPY_SCL_FORMAT;
