// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    fs_hp98x5.cpp

    HP 9825, 9831 & 9845 filesystem handling

    HP9825,HP9831 and HP9845 have very similar filesystems for disk
    storage (storage on tape is an entirely different story, each
    system has its own format).
    The main reference for 9845 FS comes from A. Kueckes page at
    https://www.hp9845.net/9845/projects/hpdir
    For what concerns the 9825 FS, I mainly reverse engineered it
    from assembly sources of mass storage ROMs.
    File names in 98x5 filesystems have no extension. In this driver
    I'm using a virtual extensions to provide a more user-friendly
    encoding of the numeric file type,
    I've added the "TXT" extension to handle the very common string-only
    DATA files. This kind of DATA files emulates text files on
    9825/45 systems. When reading or writing TXT files, ASCII text is
    automatically translated to/from DATA files (see also
    src/tools/imgtool/modules/hp9845_tape.cpp for the same function
    on DC100 tapes).
    The general form of file name is:
    name[[.&bpr].ext]
    Where "name" is the filename itself (1 to 6 characters), "bpr" is
    the optional record size and "ext" is the virtual extension.
    BPR applies to 9845 only. It encodes the record size attribute
    of files (in bytes, 4 <= bpr <= 32766, even values only).
    BPR and ext are optional when reading a file. If specified they
    have to match file metadata, though.
    Extension is required when file is being created. It's also
    required for DATA<->TXT translation (see above).
    BPR is optional when creating a file, it defaults to 256 bytes
    when missing.
    This form of file names comes from hpdir tool (see URL above).
    Examples:
    - Create a text file having 1024 bytes-per-record:
      "FILE.&1024.TXT"
    - Read a data file as text:
      "FILE.TXT"
    - Read/write an existing program file:
      "FILE[.PROG]"

    This driver generates the spare track when formatting and updates
    it when writing. If the main directory or availability table
    are corrupted, though, no attempt is made to use their 2nd copy
    from spare track.

    NB: This driver doesn't add the so-called "bootstraps" when
    formatting a HP9825 disk. For this reason the created image
    cannot be used with HP98217 mass storage ROM which relies on
    bootstraps. Pre-formatted images (such as the one available at
    http://www.hpmuseum.net/software/9825_discs.zip) should be
    used with this ROM. Bootstraps, when present, are completely
    ignored by this driver.

*********************************************************************/

#include "fs_hp98x5.h"
#include "fsblk.h"
#include "hpi_dsk.h"

#include "strformat.h"

#include <bitset>
#include <map>

using namespace fs;
using namespace std::literals;

// Constants
constexpr unsigned SECTORS = hpi_format::HPI_SECTORS;
constexpr unsigned SECTOR_SIZE = hpi_format::HPI_SECTOR_SIZE;
constexpr unsigned DS_SECTORS = hpi_format::HPI_TRACKS * hpi_format::HPI_HEADS * SECTORS;
constexpr unsigned DS_IMAGE_SIZE = DS_SECTORS * SECTOR_SIZE;
constexpr unsigned SS_SECTORS = hpi_format::HPI_9885_TRACKS * SECTORS;
constexpr unsigned SS_IMAGE_SIZE = SS_SECTORS * SECTOR_SIZE;

// +--------+
// | HP9825 |
// | HP9831 |
// | HP9845 |
// +--------+
namespace fs {
	const hp9825_image HP9825;
	const hp9831_image HP9831;
	const hp9845_image HP9845;
}

// +-------------+
// | hp98x5_impl |
// +-------------+
namespace {
	class hp98x5_impl : public filesystem_t {
	public:
		enum fs_type {
			FS_TYPE_9825,
			FS_TYPE_9831,
			FS_TYPE_9845,
			FS_COUNT
		};

		hp98x5_impl(fsblk_t& blockdev, fs_type type);
		virtual ~hp98x5_impl() = default;

		virtual meta_data volume_metadata() override;

		virtual std::pair<std::error_condition, meta_data> metadata(const std::vector<std::string> &path) override;

		virtual std::pair<std::error_condition, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;

		virtual std::error_condition file_create(const std::vector<std::string> &path, const meta_data &meta) override;

		virtual std::pair<std::error_condition, std::vector<u8>> file_read(const std::vector<std::string> &path) override;

		virtual std::error_condition file_write(const std::vector<std::string> &path, const std::vector<u8> &data) override;

		virtual std::error_condition format(const meta_data &meta) override;

	private:
		// Map of used/free sectors
		using sect_map = std::bitset<DS_SECTORS>;
		// 0-based LBAs
		using lba_t = int;

		// Position of system record
		static constexpr lba_t SYS_REC_LBA = 0;
		// Size of dir. entries
		static constexpr unsigned DIR_ENTRY_SIZE = 16;
		// Unspecified file type
		static constexpr u8 NO_FILE_TYPE = 0xff;
		// Special file type: DATA file as a text file
		static constexpr u8 TEXT_TYPE = 0xfe;
		// DATA file type on 9825/9831
		static constexpr u8 DATA_TYPE_9825 = 1;
		// DATA file type on 9845
		static constexpr u8 DATA_TYPE_9845 = 8;
		// Record size of DATA files on 9825
		static constexpr unsigned BPR_9825 = 256;
		// Default record size
		static constexpr unsigned DEF_BPR = 256;
		// Special extension for text files
		inline static const std::string_view TEXT_EXT = "TXT"sv;
		// DATA record types
		static constexpr u16 REC_TYPE_EOR     = 0x1e;   // End-of-record
		static constexpr u16 REC_TYPE_EOF     = 0x3e;   // End-of-file
		static constexpr u16 REC_TYPE_FULLSTR = 0x3c;   // A whole (un-split) string
		static constexpr u16 REC_TYPE_1STSTR  = 0x1c;   // First part of a string
		static constexpr u16 REC_TYPE_MIDSTR  = 0x0c;   // Middle part(s) of a string
		static constexpr u16 REC_TYPE_ENDSTR  = 0x2c;   // Last part of a string
		// Sys flag value for DATA files (HP9845)
		static constexpr u8 SYS_FLAG_DATA = 1;
		// Sys flag value for non-DATA files (HP9845)
		static constexpr u8 SYS_FLAG_NON_DATA = 6;
		// Spare track when formatting
		static constexpr unsigned FMT_SPARE_TRACK = 1;
		// Directory size when formatting (sectors)
		static constexpr unsigned FMT_DIR_SIZE = 22;
		// Av. table size when formatting (sectors)
		static constexpr unsigned FMT_MAP_SIZE = 6;
		// # of system tracks when formatting
		static constexpr unsigned FMT_SYS_TRACKS = 2;
		// # of data tracks when formatting
		static constexpr unsigned FMT_DATA_TRACKS_SS = 65;
		static constexpr unsigned FMT_DATA_TRACKS_DS = 148;

		// Directory entries
		struct entry {
			entry()
				: m_in_use(false)
				, m_name{}
				, m_type_code{0}
				, m_size{0}
				, m_sectors{0}
				, m_bpr{0}
				, m_1st_sect{0}
				, m_extra_w{0}
				, m_attrs{0}
			{ }

			// In use?
			bool m_in_use;
			// Filename
			std::string m_name;
			// Type code
			u8 m_type_code;
			// Size (bytes)
			unsigned m_size;
			// Size in sectors
			unsigned m_sectors;
			// Record size (bytes-per-record)
			unsigned m_bpr;
			// First sector of data
			lba_t m_1st_sect;
			// Extra word
			u16 m_extra_w;
			// Various attributes
			u8 m_attrs;
		};

		using dir_t = std::vector<entry>;

		std::error_condition parse_filename(const std::string& name, std::string& basename, int& bpr, u8& file_type) const;
		bool validate_filename(const std::string& s) const;
		void ensure_dir_loaded();
		std::pair<dir_t, sect_map> decode_dir() const;
		std::vector<u8> encode_dir() const;
		bool is_data_file(u8 type_code) const;
		void store_dir_map();
		static dir_t::iterator scan_dir(dir_t& dir, const std::string& name);
		dir_t::iterator find_file(const std::string& basename, int bpr, u8 file_type);
		meta_data get_metadata(const entry& e);
		sect_map no_data_map() const;
		sect_map decode_map() const;
		std::vector<u8> encode_map() const;
		void set_not_in_use(lba_t first, unsigned sectors);
		bool allocate(unsigned sectors, lba_t& first);
		bool is_valid_char(u8 c) const;
		std::string dec_name(std::vector<u8>::const_iterator begin, std::vector<u8>::const_iterator end) const;
		static u16 r16b(std::vector<u8>::const_iterator it);
		template<class T> static void w16b(T it, u16 w);
		std::vector<u8> get_sector_range(lba_t first, unsigned size) const;
		void store_sector_range(lba_t first, const std::vector<u8>& data);
		static void check_av_data(const std::vector<u8>& in, const std::vector<u8>::const_iterator& it, int min_av);
		static u16 get_u16(const std::vector<u8>& in, std::vector<u8>::const_iterator& it);
		static std::vector<u8> convert_data_2_txt(const std::vector<u8>& in, unsigned bpr);
		static void split_string_n_dump(const std::vector<u8>& in, std::vector<u8>& out, unsigned bpr);
		static std::vector<u8> convert_txt_2_data(const std::vector<u8>& in, unsigned bpr);

		// FS type
		const fs_type m_fs_type;
		// Number of tracks in image
		unsigned m_img_tracks;
		// Total tracks
		unsigned m_tot_tracks;
		// System tracks
		unsigned m_sys_tracks;
		// Data tracks
		unsigned m_data_tracks;
		// Directory start
		lba_t m_dir_start;
		// Size of directory
		unsigned m_dir_size;
		// Start of av. table
		lba_t m_av_start;
		// Size of av. table
		unsigned m_av_size;
		// Start of data part
		lba_t m_data_start;
		// End of data part
		lba_t m_data_end;
		// Start of spare part
		lba_t m_spare_start;
		// Has dir. been loaded?
		bool m_dir_loaded;
		// Directory
		dir_t m_dir;
		// Sectors in use
		sect_map m_in_use;

		// Interleave factor when formatting
		// Index into array is fs_type
		static const std::array<u16, FS_COUNT> m_fmt_interleave;

		// Mapping of file types in each FS type
		// Index into array is fs_type
		// Key is type code
		// Value is virtual file extension
		const std::array<std::map<u8, std::string_view>, FS_COUNT> m_fs_type_maps;
	};
}

hp98x5_impl::hp98x5_impl(fsblk_t& blockdev, fs_type type)
	: filesystem_t(blockdev , SECTOR_SIZE)
	, m_fs_type(type)
	, m_dir_loaded(false)
	, m_fs_type_maps({
			// ********
			// * 9825 *
			// ********
			std::map<u8, std::string_view>{
				{ 1, "DATA"sv },
				{ 2, "PROG"sv },
				{ 3, "KEYS"sv },
				{ 5, "ALL"sv },
				{ 6, "BPRG"sv }
			},
			// ********
			// * 9831 *
			// ********
			std::map<u8, std::string_view>{
				{ 1, "DATA"sv },
				{ 9, "DATA"sv },
				{ 18, "PROG"sv },
				{ 19, "KEYS"sv },
				{ 20, "NULL"sv },
				{ 21, "MEMORY"sv },
				{ 22, "BINARY"sv },
				{ 23, "86DATA"sv }
			},
			// ********
			// * 9845 *
			// ********
			std::map<u8, std::string_view>{
				{ 0, "BKUP"sv },
				{ 8, "DATA"sv },
				{ 16,"PROG"sv },
				{ 24,"KEYS"sv },
				{ 32,"BDAT"sv },
				{ 40,"ALL"sv  },
				{ 48,"BPRG"sv },
				{ 56,"OPRM"sv },
				{ 60,"ASMB"sv }
			}})
{
}

meta_data hp98x5_impl::volume_metadata()
{
	meta_data res;
	return res;
}

std::pair<std::error_condition, meta_data> hp98x5_impl::metadata(const std::vector<std::string> &path)
{
	if (path.size() != 1) {
		return std::make_pair(error::not_found, meta_data{});
	}

	std::string basename;
	int bpr;
	u8 file_type;

	auto err = parse_filename(path.front(), basename, bpr, file_type);
	if (err) {
		return std::make_pair(error::not_found, meta_data{});
	}

	ensure_dir_loaded();
	auto it = scan_dir(m_dir, basename);
	if (it == m_dir.end()) {
		return std::make_pair(error::not_found, meta_data{});
	}

	auto meta = get_metadata(*it);
	return std::make_pair(std::error_condition(), std::move(meta));
}

std::pair<std::error_condition, std::vector<dir_entry>> hp98x5_impl::directory_contents(const std::vector<std::string> &path)
{
	if (path.empty()) {
		ensure_dir_loaded();
		// Copy all dir_entry(s) out of m_dir
		std::vector<dir_entry> dir_entries;
		for (const auto& e : m_dir) {
			if (e.m_in_use) {
				auto meta = get_metadata(e);
				dir_entries.emplace_back(dir_entry_type::file, std::move(meta));
			}
		}
		return std::make_pair(std::error_condition(), dir_entries);
	} else {
		return std::make_pair(error::not_found, std::vector<dir_entry>{});
	}
}

std::error_condition hp98x5_impl::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	if (!path.empty()) {
		return error::invalid_name;
	}

	auto name = meta.get_string(meta_name::name);

	std::string basename;
	int bpr;
	u8 file_type;

	// Parse & validate filename
	// Extension must be specified
	auto err = parse_filename(name, basename, bpr, file_type);
	if (err) {
		return err;
	}
	if (file_type == NO_FILE_TYPE) {
		return error::invalid_name;
	}

	if (bpr < 0) {
		// Use default bpr when not specified
		bpr = DEF_BPR;
	}

	// Check that file doesn't exist
	ensure_dir_loaded();
	auto it = scan_dir(m_dir, basename);
	if (it != m_dir.end()) {
		return error::already_exists;
	}

	// Find a free entry in directory
	auto it2 = std::find_if(m_dir.begin(), m_dir.end(), [](const entry& e) { return !e.m_in_use; });
	if (it2 == m_dir.end()) {
		// Add a new entry at the end of directory
		if (m_dir.size() < m_dir_size * SECTOR_SIZE / DIR_ENTRY_SIZE) {
			m_dir.emplace_back(entry());
			it2 = m_dir.end() - 1;
		} else {
			return error::no_space;
		}
	}

	// Fill dir entry
	it2->m_in_use = true;
	it2->m_name = basename;
	it2->m_type_code = (file_type != TEXT_TYPE) ? file_type : (m_fs_type != FS_TYPE_9845 ? DATA_TYPE_9825 : DATA_TYPE_9845);
	it2->m_size = 0;
	it2->m_sectors = 0;
	it2->m_bpr = bpr > 0 ? unsigned(bpr) : DEF_BPR;
	it2->m_1st_sect = 0;
	it2->m_extra_w = 0;
	it2->m_attrs = m_fs_type != FS_TYPE_9845 ? 0 : (it2->m_type_code == DATA_TYPE_9845 ? SYS_FLAG_DATA : SYS_FLAG_NON_DATA);

	store_dir_map();

	return std::error_condition();
}

std::pair<std::error_condition, std::vector<u8>> hp98x5_impl::file_read(const std::vector<std::string> &path)
{
	if (path.size() != 1) {
		return std::make_pair(error::not_found, std::vector<u8>{});
	}
	std::string basename;
	int bpr;
	u8 file_type;

	auto err = parse_filename(path.front(), basename, bpr, file_type);

	if (!err) {
		auto it = find_file(basename, bpr, file_type);
		if (it == m_dir.end()) {
			err = error::not_found;
		} else {
			auto file_data = get_sector_range(it->m_1st_sect, it->m_sectors);
			if (file_type == TEXT_TYPE) {
				file_data = convert_data_2_txt(file_data, it->m_bpr);
			} else {
				file_data.resize(it->m_size);
			}
			return std::make_pair(std::error_condition(), std::move(file_data));
		}
	}
	return std::make_pair(err, std::vector<u8>{});
}

std::error_condition hp98x5_impl::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	if (path.size() != 1) {
		return error::not_found;
	}

	std::string basename;
	int bpr;
	u8 file_type;

	auto err = parse_filename(path.front(), basename, bpr, file_type);
	if (err) {
		return err;
	}
	// Check that file already exists
	auto it = find_file(basename, bpr, file_type);
	if (it == m_dir.end()) {
		return error::not_found;
	}

	// De-allocate current blocks
	if (it->m_sectors) {
		set_not_in_use(it->m_1st_sect, it->m_sectors);
	}

	const std::vector<u8> *data_ptr = &data;
	std::vector<u8> new_data;

	if (file_type == TEXT_TYPE) {
		new_data = convert_txt_2_data(data, it->m_bpr);
		data_ptr = &new_data;
	}

	// Allocate space
	it->m_sectors = (data_ptr->size() + SECTOR_SIZE - 1) / SECTOR_SIZE;
	if (!allocate(it->m_sectors, it->m_1st_sect)) {
		return error::no_space;
	}

	// Store file content
	store_sector_range(it->m_1st_sect, *data_ptr);
	it->m_size = data_ptr->size();

	// Update directory & map
	store_dir_map();

	return std::error_condition();
}

std::error_condition hp98x5_impl::format(const meta_data &meta)
{
	bool is_ds = m_blockdev.block_count() == DS_SECTORS;

	// Compose system record
	std::vector<u8> sys_rec;
	auto out_ins = std::back_inserter(sys_rec);

	// System ID
	w16b(out_ins, 0);
	// Sectors per track
	w16b(out_ins, SECTORS);
	// Total tracks
	m_data_tracks = is_ds ? FMT_DATA_TRACKS_DS : FMT_DATA_TRACKS_SS;
	m_sys_tracks = FMT_SYS_TRACKS;
	m_tot_tracks = m_data_tracks + m_sys_tracks;
	m_data_start = m_sys_tracks * SECTORS;
	m_data_end = m_tot_tracks * SECTORS;
	w16b(out_ins, m_tot_tracks);
	// Spare track
	m_spare_start = FMT_SPARE_TRACK * SECTORS;
	w16b(out_ins, FMT_SPARE_TRACK);
	// First sector of directory
	m_dir_start = 1;
	m_dir_size = FMT_DIR_SIZE;
	w16b(out_ins, m_dir_start);
	// First sector of availability table
	m_av_start = m_dir_start + m_dir_size;
	m_av_size = FMT_MAP_SIZE;
	w16b(out_ins, m_av_start);
	// First sector after availability table
	w16b(out_ins, m_av_start + m_av_size);
	// Count of system tracks
	w16b(out_ins, m_sys_tracks);
	// Count of data tracks
	w16b(out_ins, m_data_tracks);
	// Interleave factor
	w16b(out_ins, m_fmt_interleave[ m_fs_type ]);

	m_blockdev.get(0)->write(0, sys_rec.data(), sys_rec.size());

	// 9825 & 9831 have a backup copy of system record in spare track but 9845 hasn't, go figure
	if (m_fs_type != FS_TYPE_9845) {
		m_blockdev.get(m_spare_start)->write(0, sys_rec.data(), sys_rec.size());
	}

	m_dir.clear();
	m_dir_loaded = true;
	m_in_use = no_data_map();

	store_dir_map();

	return std::error_condition();
}

std::error_condition hp98x5_impl::parse_filename(const std::string& name, std::string& basename, int& bpr, u8& file_type) const
{
	// General form of filenames:
	// basename[[.&bpr].ext]
	// where "bpr" is the optional byte-per-record attribute of file (this part can only be
	// specified for 9845 filesystems)
	// and "ext" is the virtual file extension

	file_type = NO_FILE_TYPE;
	bpr = -1;

	auto p1 = name.find_first_of('.');

	if (p1 != std::string::npos) {
		std::string::size_type p2 = std::string::npos;

		if (m_fs_type == FS_TYPE_9845) {
			p2 = name.find_first_of('.', p1 + 1);
		}

		if (p2 != std::string::npos) {
			// [p1+1 p2) should have "&bpr"
			if (name[ p1 + 1 ] != '&') {
				return error::invalid_name;
			}
			if (!std::all_of(name.cbegin() + p1 + 2, name.cbegin() + p2, [](char c) { return c >= '0' && c <= '9'; })) {
				return error::invalid_name;
			}
			try {
				bpr = std::stoul(std::string{name, p1 + 2, p2 - p1 - 2});
			} catch (...) {
				return error::invalid_name;
			}
			if (bpr < 4 || bpr > 32767 || (bpr & 1) != 0) {
				return error::invalid_name;
			}
		} else {
			p2 = p1;
		}

		std::string ext{ name, p2 + 1, std::string::npos };
		if (ext == TEXT_EXT) {
			file_type = TEXT_TYPE;
		} else {
			for (const auto& e : m_fs_type_maps[ m_fs_type ]) {
				if (e.second == ext) {
					file_type = e.first;
					break;
				}
			}
			if (file_type == NO_FILE_TYPE) {
				return error::invalid_name;
			}
		}
	}

	// [0 p1) has "basename"
	basename = std::string(name, 0, p1);
	if (validate_filename(basename)) {
		return std::error_condition();
	} else {
		return error::invalid_name;
	}
}

bool hp98x5_impl::validate_filename(const std::string& s) const
{
	if (s.size() < 1 || s.size() > 6) {
		return false;
	}

	// 9831 files must not have '*' at beginning
	if (m_fs_type == FS_TYPE_9831 && s.front() == '*') {
		return false;
	}

	auto it = std::find_if_not(s.cbegin(), s.cend(), [this](char c) { return is_valid_char(u8(c)); });
	return it == s.cend();
}

void hp98x5_impl::ensure_dir_loaded()
{
	if (m_dir_loaded) {
		return;
	}

	// Get system record
	auto sys_rec = m_blockdev.get(SYS_REC_LBA);

	// Layout of system record
	// Word #   Content
	// ================
	// 0        System ID
	// 1        Sectors per track
	// 2        Total tracks
	// 3        Track of spare system info
	// 4        First sector of directory
	// 5        First sector of availability table
	// 6        First sector after availability table
	// 7        Count of system tracks
	// 8        Count of data tracks
	// 9        Interleave factor
	// 10..127  N/U
	u16 sects_per_track = sys_rec->r16b(2);
	u16 tot_tracks      = sys_rec->r16b(4);
	u16 spare_track     = sys_rec->r16b(6);
	u16 dir_start       = sys_rec->r16b(8);
	u16 av_start        = sys_rec->r16b(10);
	u16 av_end          = sys_rec->r16b(12);
	u16 sys_tracks      = sys_rec->r16b(14);
	u16 data_tracks     = sys_rec->r16b(16);

	m_img_tracks = m_blockdev.block_count() / SECTORS;

	m_tot_tracks = tot_tracks;
	m_sys_tracks = sys_tracks;
	m_data_tracks = data_tracks;

	m_dir_start = lba_t(dir_start);
	m_dir_size = av_start - dir_start;
	m_av_start = lba_t(av_start);
	m_av_size = av_end - av_start;

	m_data_start = sys_tracks * SECTORS;
	m_data_end = (sys_tracks + data_tracks) * SECTORS;

	m_spare_start = spare_track * SECTORS;

	// Consistency checks
	if (sects_per_track != SECTORS) {
		throw std::runtime_error(util::string_format("Invalid sectors per track (%u)", sects_per_track));
	}
	if (dir_start == 0 ||
		av_start <= dir_start ||
		av_end <= av_start) {
		throw std::runtime_error(util::string_format("Invalid dir/av. table pointers (%u,%u,%u)", dir_start, av_start, av_end));
	}
	lba_t spare_end = m_spare_start + av_end;
	if (m_spare_start < av_end ||
		spare_end > m_data_start) {
		throw std::runtime_error(util::string_format("Invalid spare info position (%d,%d)", m_spare_start, spare_end));
	}
	if (tot_tracks > m_img_tracks) {
		throw std::runtime_error(util::string_format("Invalid total track number (%u,%u)", tot_tracks, m_img_tracks));
	}
	if (tot_tracks < (data_tracks + sys_tracks) ||
		data_tracks < 1 ||
		sys_tracks < 2) {
		throw std::runtime_error(util::string_format("Invalid sys/data tracks (%u,%u,%u)", sys_tracks, data_tracks, tot_tracks));
	}

	auto&& [decoded_dir, decoded_map] = decode_dir();
	m_in_use = decode_map();

	if (m_in_use != decoded_map) {
		fprintf(stderr, "WARNING: Incorrect map\n");
	}

	m_dir = std::move(decoded_dir);

	m_dir_loaded = true;
}

std::pair<hp98x5_impl::dir_t, hp98x5_impl::sect_map> hp98x5_impl::decode_dir() const
{
	dir_t dir;
	sect_map in_use = no_data_map();

	auto enc_dir = get_sector_range(m_dir_start, m_dir_size);

	for (auto it = enc_dir.cbegin(); it != enc_dir.cend(); it += DIR_ENTRY_SIZE) {
		auto w1 = r16b(it + 2);
		if (w1 == 0xffff) {
			// Directory ends
			break;
		}

		entry e;

		auto w0 = r16b(it);
		if (w0 != 0) {
			e.m_in_use = true;
			// +0..+5: File name
			e.m_name = dec_name(it, it + 6);
			if (scan_dir(dir, e.m_name) != dir.end()) {
				throw std::runtime_error(util::string_format("Duplicated file name (%s)", e.m_name));
			}
			// +6..+7: 1st sector
			lba_t first_sect = r16b(it + 6);
			// +8..+9: Number of sectors
			unsigned sectors = r16b(it + 8);
			if (first_sect < m_data_start || (first_sect + sectors) >= m_data_end || sectors == 0) {
				throw std::runtime_error(util::string_format("Invalid file allocation (%d,%u)", first_sect, sectors));
			}
			e.m_1st_sect = first_sect;
			e.m_sectors = sectors;
			// Mark used sectors
			for (lba_t i = first_sect; i < (first_sect + sectors); i++) {
				in_use.set(i);
			}
			if (m_fs_type == FS_TYPE_9825 || m_fs_type == FS_TYPE_9831) {
				// ******** ********
				// * 9825 * * 9831 *
				// ******** ********

				// +12..+13: File type & secure flag
				auto w6 = r16b(it + 12);
				// b15..b10 = File type
				e.m_type_code = u8(w6 >> 10);
				// b9 = secured file (9825 only)
				if (m_fs_type == FS_TYPE_9825) {
					e.m_attrs = u8((w6 >> 9) & 1);
				} else {
					e.m_attrs = 0;
				}
				// +14..+15: Extra word
				e.m_extra_w = r16b(it + 14);

				// DATA files have a fixed record size
				e.m_bpr = BPR_9825;

				if (!is_data_file(e.m_type_code)) {
					// File type != DATA
					// +10..+11: Size of file in words - 1
					auto w5 = r16b(it + 10);
					e.m_size = (w5 + 1) * 2;
					unsigned exp_sectors = (e.m_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
					if (exp_sectors != sectors) {
						throw std::runtime_error(util::string_format("Mismatching number of sectors (%u,%u)", exp_sectors, sectors));
					}
				} else {
					// File type == DATA
					e.m_size = e.m_sectors * SECTOR_SIZE;
				}
			} else {
				// ********
				// * 9845 *
				// ********

				// +10..+11: Words per record
				auto w5 = r16b(it + 10);
				if (w5 < 2 || w5 > 16383) {
					throw std::runtime_error(util::string_format("Invalid size of records (%u)", w5));
				}
				e.m_bpr = w5 * 2;

				// +12..+13: File type & sys. flags
				auto w6 = r16b(it + 12);
				// b12..b7 = File type
				e.m_type_code = u8((w6 >> 7) & 0x3f);
				// b15..b13 = System flags
				e.m_attrs = u8(w6 >> 13);

				// +14..+15: Protection
				e.m_extra_w = r16b(it + 14);

				e.m_size = e.m_sectors * SECTOR_SIZE;
			}
		}

		dir.emplace_back(std::move(e));
	}

	return std::make_pair(std::move(dir), std::move(in_use));
}

std::vector<u8> hp98x5_impl::encode_dir() const
{
	std::vector<u8> res;
	auto out_ins = std::back_inserter(res);

	for (const auto& e : m_dir) {
		if (e.m_in_use) {
			// Filename
			for (auto c : e.m_name) {
				*out_ins = u8(c);
			}
			if (e.m_name.size() < 6) {
				std::fill_n(out_ins, 6 - e.m_name.size(), 0);
			}
			// 1st sector
			w16b(out_ins, e.m_1st_sect);
			// Number of sectors
			w16b(out_ins, e.m_sectors);
			if (m_fs_type == FS_TYPE_9825 || m_fs_type == FS_TYPE_9831) {
				// ******** ********
				// * 9825 * * 9831 *
				// ******** ********

				// Size of file in words - 1 (any file type but DATA)
				if (!is_data_file(e.m_type_code)) {
					w16b(out_ins, e.m_size / 2 - 1);
				} else {
					w16b(out_ins, 0);
				}

				// File type & secure flag
				u16 w6 = u16(e.m_type_code) << 10;
				if (e.m_attrs) {
					w6 |= 1U << 9;
				}
				w16b(out_ins, w6);

				// Extra word
				w16b(out_ins, e.m_extra_w);
			} else {
				// ********
				// * 9845 *
				// ********

				// Words per record
				w16b(out_ins, e.m_bpr / 2);

				// File type & sys. flags
				u16 w6 = (u16(e.m_attrs) << 13) | (u16(e.m_type_code) << 7);
				w16b(out_ins, w6);

				// Protection
				w16b(out_ins, e.m_extra_w);
			}
		} else {
			// Empty entry
			std::fill_n(out_ins, DIR_ENTRY_SIZE, 0);
		}
	}

	// Insert dir. terminator (9845 only)
	unsigned dir_len = m_dir_size * SECTOR_SIZE;
	if (m_fs_type == FS_TYPE_9845 && dir_len - res.size() >= DIR_ENTRY_SIZE) {
		w16b(out_ins, 0);
		w16b(out_ins, 0xffff);
		w16b(out_ins, 0);
		w16b(out_ins, 0);
		w16b(out_ins, 0);
		w16b(out_ins, 0);
		w16b(out_ins, 0);
		w16b(out_ins, 0);
	}

	// Pad to end
	if (res.size() < dir_len) {
		std::fill_n(out_ins, dir_len - res.size(), 0);
	}

	return res;
}

bool hp98x5_impl::is_data_file(u8 type_code) const
{
	auto mp = m_fs_type_maps[ m_fs_type ];
	auto it = mp.find(type_code);
	return it != mp.end() && it->second == "DATA"sv;
}

void hp98x5_impl::store_dir_map()
{
	// Store directory (2 copies)
	auto dir_content = encode_dir();
	store_sector_range(m_dir_start, dir_content);
	store_sector_range(m_dir_start + m_spare_start, dir_content);
	// Store map (2 copies)
	auto map_content = encode_map();
	store_sector_range(m_av_start, map_content);
	store_sector_range(m_av_start + m_spare_start, map_content);
}

hp98x5_impl::dir_t::iterator hp98x5_impl::scan_dir(dir_t& dir, const std::string& name)
{
	return std::find_if(dir.begin(), dir.end(), [&name](const entry& e){ return e.m_in_use && name == e.m_name; });
}

hp98x5_impl::dir_t::iterator hp98x5_impl::find_file(const std::string& basename, int bpr, u8 file_type)
{
	ensure_dir_loaded();
	auto it = scan_dir(m_dir, basename);
	if (it != m_dir.end() &&
		(file_type == NO_FILE_TYPE ||
		 (file_type == TEXT_TYPE && is_data_file(it->m_type_code)) ||
		 file_type == it->m_type_code) &&
		(bpr < 0 || bpr == it->m_bpr)) {
		return it;
	} else {
		return m_dir.end();
	}
}

meta_data hp98x5_impl::get_metadata(const entry& e)
{
	meta_data meta;
	meta.set(meta_name::name, e.m_name);

	auto mp = m_fs_type_maps[ m_fs_type ];
	auto it = mp.find(e.m_type_code);
	if (it != mp.end()) {
		meta.set(meta_name::file_type , it->second);
	} else {
		meta.set(meta_name::file_type , util::string_format("?%02x", e.m_type_code));
	}

	if (m_fs_type == FS_TYPE_9825) {
		// ********
		// * 9825 *
		// ********
		if (e.m_attrs) {
			// "Secured" program
			meta.set(meta_name::attributes, "S");
		}
	} else if (m_fs_type == FS_TYPE_9845) {
		// ********
		// * 9845 *
		// ********
		if (e.m_bpr != DEF_BPR) {
			// System flags & record size
			meta.set(meta_name::attributes, util::string_format("S=%u R=%u", e.m_attrs, e.m_bpr));
		} else {
			// System flags
			meta.set(meta_name::attributes, util::string_format("S=%u", e.m_attrs));
		}
	}

	meta.set(meta_name::length, e.m_size);

	return meta;
}

hp98x5_impl::sect_map hp98x5_impl::no_data_map() const
{
	sect_map res;

	// First mark every sector as used...
	res.set();

	// ...then carve out the data part of disk
	for (auto i = m_data_start; i < m_data_end; i++) {
		res.reset(i);
	}

	return res;
}

hp98x5_impl::sect_map hp98x5_impl::decode_map() const
{
	sect_map res;

	// First mark every sector as used...
	res.set();

	// ..then scan availability table and mark free sectors
	auto enc_av = get_sector_range(m_av_start, m_av_size);
	for (auto it = enc_av.cbegin(); it != enc_av.cend(); it += 4) {
		lba_t start = r16b(it);
		unsigned size = r16b(it + 2);
		if (start == 0xffff) {
			// table ends
			break;
		} else if (size == 0) {
			// Null entry, skip
			continue;
		}
		if (start < m_data_start ||
			start + size > m_data_end) {
			throw std::runtime_error(util::string_format("Invalid entry in av. table (%d,%u)", start, size));
		}
		for (lba_t k = start; k < start + size; k++) {
			if (!res[ k ]) {
				throw std::runtime_error(util::string_format("Sector %d marked free twice", k));
			} else {
				res.reset(k);
			}
		}
	}

	return res;
}

std::vector<u8> hp98x5_impl::encode_map() const
{
	std::vector<u8> res(m_av_size * SECTOR_SIZE);

	auto it = res.begin();

	bool in_free_block = false;

	for (auto i = m_data_start; i < m_data_end; i++) {
		if (m_in_use[ i ]) {
			in_free_block = false;
		} else if (in_free_block) {
			it -= 2;
			w16b(it, r16b(it) + 1);
			it += 2;
		} else {
			in_free_block = true;
			if (res.end() - it < 4) {
				throw std::runtime_error("Availability table too fragmented");
			}
			w16b(it, u16(i));
			it += 2;
			w16b(it, 1);
			it += 2;
		}
	}

	if (m_fs_type == FS_TYPE_9845 && res.end() - it >= 4) {
		w16b(it, 0xffffU);
	}

	return res;
}

void hp98x5_impl::set_not_in_use(lba_t first, unsigned sectors)
{
	for (lba_t i = first; i < (first + sectors); i++) {
		m_in_use.reset(i);
	}
}

bool hp98x5_impl::allocate(unsigned sectors, lba_t& first)
{
	if (sectors > (m_data_end - m_data_start)) {
		return false;
	}

	sect_map scanner;
	for (lba_t i = m_data_start; i < (m_data_start + sectors); i++) {
		scanner.set(i);
	}

	for (lba_t i = m_data_start; i <= (m_data_end - sectors); i++) {
		sect_map tmp = m_in_use & scanner;
		if (tmp.none()) {
			m_in_use |= scanner;
			first = i;
			return true;
		}
		scanner <<= 1;
	}

	return false;
}

bool hp98x5_impl::is_valid_char(u8 c) const
{
	if (c <= 0x20 || c >= 0x7f || c == '"' || c == ':') {
		return false;
	} else if (m_fs_type == FS_TYPE_9825) {
		return c != ',' && c != ';';
	} else if (m_fs_type == FS_TYPE_9831) {
		return c != ',';
	} else {
		return true;
	}
}

std::string hp98x5_impl::dec_name(std::vector<u8>::const_iterator begin, std::vector<u8>::const_iterator end) const
{
	std::string res;
	bool ended = false;
	for (auto i = begin; i < end; i++) {
		auto c = *i;
		if (ended) {
			if (c != 0) {
				throw std::runtime_error("Invalid name in dir. entry");
			}
		} else {
			if (c == 0) {
				ended = true;
			} else if (is_valid_char(c)) {
				res.push_back(char(c));
			} else {
				throw std::runtime_error("Invalid name in dir. entry");
			}
		}
	}

	return res;
}

u16 hp98x5_impl::r16b(std::vector<u8>::const_iterator it)
{
	u16 w = (u16(it[ 0 ]) << 8) | it[ 1 ];
	return w;
}

template<class T> void hp98x5_impl::w16b(T it, u16 w)
{
	*it++ = u8(w >> 8);
	*it = u8(w & 0xff);
}

std::vector<u8> hp98x5_impl::get_sector_range(lba_t first, unsigned size) const
{
	std::vector<u8> res;

	res.resize(size * SECTOR_SIZE);

	u8 *ptr = res.data();

	for (lba_t idx = first; idx < first + size; idx++) {
		auto data_sect = m_blockdev.get(u32(idx));
		data_sect->read(0, ptr, SECTOR_SIZE);
		ptr += SECTOR_SIZE;
	}
	return res;
}

void hp98x5_impl::store_sector_range(lba_t first, const std::vector<u8>& data)
{
	const u8 *ptr = data.data();
	u32 to_go = u32(data.size());
	unsigned sects = (data.size() + SECTOR_SIZE - 1) / SECTOR_SIZE;
	for (lba_t idx = first; idx < first + sects; idx++) {
		u32 count = std::min<u32>(to_go, SECTOR_SIZE);
		auto blk = m_blockdev.get(u32(idx));
		blk->write(0, ptr, count);
		ptr += count;
		to_go -= count;
	}
}

void hp98x5_impl::check_av_data(const std::vector<u8>& in, const std::vector<u8>::const_iterator& it, int min_av)
{
	if (in.cend() - it < min_av) {
		throw std::runtime_error("DATA file too short");
	}
}

u16 hp98x5_impl::get_u16(const std::vector<u8>& in, std::vector<u8>::const_iterator& it)
{
	check_av_data(in, it, 2);
	auto tmp = r16b(it);
	it += 2;
	return tmp;
}

std::vector<u8> hp98x5_impl::convert_data_2_txt(const std::vector<u8>& in, unsigned bpr)
{
	auto it = in.cbegin();
	std::vector<u8> accum;

	u16 rec_type;
	unsigned accum_len = 0;

	do {
		// Get record type
		rec_type = get_u16(in, it);
		switch (rec_type) {
		case REC_TYPE_EOR:
			// End of record: just skip it
			break;

		case REC_TYPE_FULLSTR:
			// A string in a single piece
		case REC_TYPE_1STSTR:
			// First piece of a split string
		case REC_TYPE_MIDSTR:
			// Mid piece(s) of a split string
		case REC_TYPE_ENDSTR: {
			// Closing piece of a split string
			if (((rec_type == REC_TYPE_FULLSTR || rec_type == REC_TYPE_1STSTR) && accum_len > 0) ||
				((rec_type == REC_TYPE_MIDSTR || rec_type == REC_TYPE_ENDSTR) && accum_len == 0)) {
				throw std::runtime_error("Wrong sequence of string pieces");
			}

			unsigned tmp_len = (unsigned)get_u16(in, it);

			if (rec_type == REC_TYPE_FULLSTR || rec_type == REC_TYPE_1STSTR) {
				accum_len = tmp_len;
			} else if (tmp_len != accum_len) {
				throw std::runtime_error("Wrong length of string piece");
			}

			unsigned len_to_eor = bpr - (it - in.cbegin()) % bpr;
			unsigned rec_len;
			if (rec_type == REC_TYPE_FULLSTR || rec_type == REC_TYPE_ENDSTR) {
				if (accum_len > len_to_eor) {
					throw std::runtime_error("Wrong length of string piece");
				}
				rec_len = accum_len;
			} else {
				if (accum_len <= len_to_eor) {
					throw std::runtime_error("Wrong length of string piece");
				}
				rec_len = len_to_eor;
			}
			check_av_data(in, it, rec_len);
			// Sanitize and copy string
			for (unsigned i = 0; i < rec_len; i++) {
				u8 c = *it++;
				if (c < 0x20 || c >= 0x7f) {
					c = ' ';
				}
				accum.push_back(c);
			}
			if (rec_type == REC_TYPE_FULLSTR || rec_type == REC_TYPE_ENDSTR) {
				// Record ended, add a \n to output
				accum.push_back('\n');
			}
			if (rec_len & 1) {
				// Keep length of string pieces even
				it++;
			}
			accum_len -= rec_len;
			break;
		}

		case REC_TYPE_EOF:
			// End of file
			break;

		default:
			throw std::runtime_error(util::string_format("Unknown record type (%04x)", rec_type));
		}
	} while (rec_type != REC_TYPE_EOF);

	return accum;
}

void hp98x5_impl::split_string_n_dump(const std::vector<u8>& in, std::vector<u8>& out, unsigned bpr)
{
	auto it = in.cbegin();
	auto out_ins = std::back_inserter(out);
	u16 rec_type = REC_TYPE_1STSTR;

	while (1) {
		unsigned free_len = bpr - (out.size() % bpr);
		if (free_len <= 4) {
			// Not enough free space at end of current record: fill with EORs
			while (free_len) {
				w16b(out_ins, REC_TYPE_EOR);
				free_len -= 2;
			}
		} else {
			unsigned s_len = in.cend() - it;
			unsigned s_part_len = std::min(free_len - 4 , s_len);
			if (s_part_len == s_len) {
				// Free space to EOR enough for what's left of string
				break;
			}
			w16b(out_ins, rec_type);
			w16b(out_ins, u16(s_len));
			std::copy(it, it + s_part_len, out_ins);
			it += s_part_len;
			rec_type = REC_TYPE_MIDSTR;
		}
	}

	w16b(out_ins, rec_type == REC_TYPE_MIDSTR ? REC_TYPE_ENDSTR : REC_TYPE_FULLSTR);
	w16b(out_ins, u16(in.cend() - it));
	std::copy(it, in.cend(), out_ins);
	if (in.size() & 1) {
		*out_ins = 0;
	}
}

std::vector<u8> hp98x5_impl::convert_txt_2_data(const std::vector<u8>& in, unsigned bpr)
{
	std::vector<u8> res;

	std::vector<u8> accum;
	std::vector<u8> trailer;

	// Split "in" into lines of text discarding trailing whitespace on each one
	for (auto c : in) {
		if (c == '\n') {
			if (!accum.empty()) {
				split_string_n_dump(accum, res, bpr);
			}
			accum.clear();
			trailer.clear();
		} else if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' || c == '\v') {
			trailer.push_back(c);
		} else {
			if (!trailer.empty()) {
				accum.insert(accum.end(), trailer.cbegin(), trailer.cend());
				trailer.clear();
			}
			accum.push_back(c);
		}
	}

	// Fill free space of last record with EOFs
	auto out_ins = std::back_inserter(res);
	for (unsigned pad_len = bpr - (res.size() % bpr); pad_len; pad_len -= 2) {
		w16b(out_ins, REC_TYPE_EOF);
	}

	return res;
}

const std::array<u16, hp98x5_impl::FS_COUNT> hp98x5_impl::m_fmt_interleave = {
	// ********
	// * 9825 *
	// ********
	2,
	// ********
	// * 9831 *
	// ********
	0,
	// ********
	// * 9845 *
	// ********
	7
};

// +--------------+
// | hp9825_image |
// +--------------+
const char *hp9825_image::name() const
{
	return "hp9825";
}

const char *hp9825_image::description() const
{
	return "HP9825 FS";
}

void hp9825_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_HPI_FORMAT, floppy_image::FF_8, floppy_image::SSDD, SS_IMAGE_SIZE, "hp9825_ss", "HP9825 SS");
	fe.add(FLOPPY_HPI_FORMAT, floppy_image::FF_8, floppy_image::DSDD, DS_IMAGE_SIZE, "hp9825_ds", "HP9825 DS");
}

bool hp9825_image::can_format() const
{
	return true;
}

bool hp9825_image::can_read() const
{
	return true;
}

bool hp9825_image::can_write() const
{
	return true;
}

bool hp9825_image::has_rsrc() const
{
	return false;
}

std::vector<meta_description> hp9825_image::volume_meta_description() const
{
	std::vector<meta_description> res;

	return res;
}

std::vector<meta_description> hp9825_image::file_meta_description() const
{
	std::vector<meta_description> res;

	res.emplace_back(meta_description(meta_name::name, "Empty", false, [](const meta_value &m) { return m.as_string().size() <= 6; }, "File name, up to 6 characters"));
	res.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "File size"));
	res.emplace_back(meta_description(meta_name::file_type, "", true, nullptr, "File type"));
	res.emplace_back(meta_description(meta_name::attributes, "", true, nullptr, "File attributes"));

	return res;
}

std::unique_ptr<filesystem_t> hp9825_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<hp98x5_impl>(blockdev, hp98x5_impl::FS_TYPE_9825);
}

// +--------------+
// | hp9831_image |
// +--------------+
const char *hp9831_image::name() const
{
	return "hp9831";
}

const char *hp9831_image::description() const
{
	return "HP9831 FS";
}

void hp9831_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_HPI_FORMAT, floppy_image::FF_8, floppy_image::SSDD, SS_IMAGE_SIZE, "hp9831_ss", "HP9831 SS");
}

bool hp9831_image::can_format() const
{
	return true;
}

bool hp9831_image::can_read() const
{
	return true;
}

bool hp9831_image::can_write() const
{
	return true;
}

bool hp9831_image::has_rsrc() const
{
	return false;
}

std::vector<meta_description> hp9831_image::volume_meta_description() const
{
	std::vector<meta_description> res;

	return res;
}

std::vector<meta_description> hp9831_image::file_meta_description() const
{
	std::vector<meta_description> res;

	res.emplace_back(meta_description(meta_name::name, "Empty", false, [](const meta_value &m) { return m.as_string().size() <= 6; }, "File name, up to 6 characters"));
	res.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "File size"));
	res.emplace_back(meta_description(meta_name::file_type, "", true, nullptr, "File type"));

	return res;
}

std::unique_ptr<filesystem_t> hp9831_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<hp98x5_impl>(blockdev, hp98x5_impl::FS_TYPE_9831);
}

// +--------------+
// | hp9845_image |
// +--------------+
const char *hp9845_image::name() const
{
	return "hp9845";
}

const char *hp9845_image::description() const
{
	return "HP9845 FS";
}

void hp9845_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_HPI_FORMAT, floppy_image::FF_8, floppy_image::SSDD, SS_IMAGE_SIZE, "hp9845_ss", "HP9845 SS");
	fe.add(FLOPPY_HPI_FORMAT, floppy_image::FF_8, floppy_image::DSDD, DS_IMAGE_SIZE, "hp9845_ds", "HP9845 DS");
}

bool hp9845_image::can_format() const
{
	return true;
}

bool hp9845_image::can_read() const
{
	return true;
}

bool hp9845_image::can_write() const
{
	return true;
}

bool hp9845_image::has_rsrc() const
{
	return false;
}

std::vector<meta_description> hp9845_image::volume_meta_description() const
{
	std::vector<meta_description> res;

	return res;
}

std::vector<meta_description> hp9845_image::file_meta_description() const
{
	std::vector<meta_description> res;

	res.emplace_back(meta_description(meta_name::name, "Empty", false, [](const meta_value &m) { return m.as_string().size() <= 6; }, "File name, up to 6 characters"));
	res.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "File size"));
	res.emplace_back(meta_description(meta_name::file_type, "", true, nullptr, "File type"));
	res.emplace_back(meta_description(meta_name::attributes, "", true, nullptr, "File attributes"));

	return res;
}

std::unique_ptr<filesystem_t> hp9845_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<hp98x5_impl>(blockdev, hp98x5_impl::FS_TYPE_9845);
}
