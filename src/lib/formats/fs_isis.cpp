// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    fs_isis.cpp

    Intel ISIS-II filesystem handling

*********************************************************************/

#include "fs_isis.h"
#include "fsblk.h"
#include "img_dsk.h"

#include "multibyte.h"
#include "strformat.h"

#include <bitset>

using namespace fs;
using namespace std::literals;

// Constants
constexpr unsigned SECTOR_SIZE = img_format::SECTOR_SIZE;
constexpr unsigned DD_SECTORS = img_format::TRACKS * img_format::HEADS * img_format::SECTORS;
constexpr unsigned DD_IMAGE_SIZE = DD_SECTORS * SECTOR_SIZE;
constexpr unsigned SD_SECTORS = DD_SECTORS / 2;
constexpr unsigned SD_IMAGE_SIZE = DD_IMAGE_SIZE / 2;

// +------+
// | ISIS |
// +------+
namespace fs {
	const isis_image ISIS;
}

// +-----------+
// | isis_impl |
// +-----------+
namespace {
	class isis_impl : public filesystem_t {
	public:
		isis_impl(fsblk_t& blockdev);
		virtual ~isis_impl() = default;

		virtual meta_data volume_metadata() override;

		virtual std::pair<std::error_condition, meta_data> metadata(const std::vector<std::string> &path) override;

		virtual std::pair<std::error_condition, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;

		virtual std::error_condition file_create(const std::vector<std::string> &path, const meta_data &meta) override;

		virtual std::pair<std::error_condition, std::vector<u8>> file_read(const std::vector<std::string> &path) override;

		virtual std::error_condition file_write(const std::vector<std::string> &path, const std::vector<u8> &data) override;

		virtual std::error_condition format(const meta_data &meta) override;

	private:
		using sect_map = std::bitset<DD_SECTORS>;
		// first: Track [0..76]
		// second: Sector [1..52]
		// (0,0) is a special case: a null pointer in linkage blocks
		using track_sect = std::pair<u8 , u8>;
		// 0-based LBAs
		using lba_t = int;
		// List of LBAs
		using lba_list = std::vector<lba_t>;

		// Position of ISIS.DIR (directory): (1,1)
		static constexpr track_sect DIR_TS{ 1 , 1 };
		// Size of ISIS.DIR
		static constexpr unsigned DIR_SECTS = 25;
		static constexpr unsigned DIR_SIZE = DIR_SECTS * SECTOR_SIZE;
		// Size of dir. entries
		static constexpr unsigned DIR_ENTRY_SIZE = 16;
		// Count of directory entries
		static constexpr unsigned DIR_ENTRIES = DIR_SIZE / DIR_ENTRY_SIZE;
		// Dir entry allocation states
		static constexpr u8 DIR_IN_USE = 0x00;
		static constexpr u8 DIR_NEVER_USED = 0x7f;
		static constexpr u8 DIR_DELETED = 0xff;
		// Default attributes for system files (format & invisible)
		static constexpr u8 DEF_SYS_ATTRS = 0x81;
		// Position of ISIS.MAP (map): (2,1)
		static constexpr track_sect MAP_TS{ 2 , 1 };
		// ISIS.MAP size in SD & DD formats
		static constexpr unsigned ISIS_MAP_SD_SIZE = 256;
		static constexpr unsigned ISIS_MAP_DD_SIZE = 512;
		// Position of ISIS.LAB (disk label): (0,25)
		static constexpr track_sect LAB_TS{ 0 , 25 };
		// ISIS.LAB size in SD & DD formats
		static constexpr unsigned ISIS_LAB_SD_SIZE = 128;
		static constexpr unsigned ISIS_LAB_DD_SIZE = 6784;
		// Position and number of block sets in ISIS.LAB (SD format)
		static constexpr unsigned ISIS_LAB_SD_BLOCKS = 1;
		// Position and number of block sets in ISIS.LAB (DD format)
		static constexpr unsigned ISIS_LAB_DD_1ST_BLOCKS = 27;
		static constexpr track_sect LAB_DD_2ND_TS{ 1 , 27 };
		static constexpr unsigned ISIS_LAB_DD_2ND_BLOCKS = 26;
		// Position of ISIS.T0 (boot): (0,1)
		static constexpr track_sect T0_TS{ 0 , 1 };
		// Position of ISIS.T0 1st linkage block: (0,24)
		static constexpr track_sect T0_1ST_LINK_TS{ 0 , 24 };
		// ISIS.T0 size
		static constexpr unsigned T0_SECTS = 23;
		static constexpr unsigned ISIS_T0_SIZE = T0_SECTS * SECTOR_SIZE;
		// First position that can be allocated for user files
		static constexpr track_sect ALLOC_START_SD{ 2 , 4 };
		static constexpr track_sect ALLOC_START_DD{ 2 , 6 };
		// Pointers per linkage block (excluding backward/forward pointers)
		static constexpr unsigned PTRS_PER_BLOCK = 62;

		// ISIS.DIR file (directory)
		inline static const std::string_view ISIS_DIR = "ISIS.DIR"sv;
		// ISIS.MAP file (map of free/used sectors)
		inline static const std::string_view ISIS_MAP = "ISIS.MAP"sv;
		// ISIS.T0 file (boot code)
		inline static const std::string_view ISIS_T0 = "ISIS.T0"sv;
		// ISIS.LAB file (it holds volume label & OS version)
		inline static const std::string_view ISIS_LAB = "ISIS.LAB"sv;

		// Valid characters in file names (digits & uppercase letters)
		inline static const std::string_view VALID_CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"sv;

		// Directory entries
		struct entry {
			entry()
				: m_alloc_state(DIR_NEVER_USED)
				, m_name{}
				, m_attrs{0}
				, m_size{0}
				, m_1st_link{-1}
				, m_lbas()
			{ }

			// Allocation state, one of 00, 7f, ff
			u8 m_alloc_state;
			// Filename
			std::string m_name;
			// Attributes
			u8 m_attrs;
			// Size
			unsigned m_size;
			// First linkage block
			lba_t m_1st_link;
			// List of file blocks
			// Blocks @ idx = 63 * N are linkage blocks, all others are data blocks
			lba_list m_lbas;
		};

		using dir_t = std::array<entry, DIR_ENTRIES>;

		// Convert between TS pair & 0-based LBA
		// TS=(0,0) is converted to -1
		lba_t ts_2_lba(const track_sect& ts) const;
		track_sect lba_2_ts(lba_t lba) const;

		lba_t lba_from_2b(const fsblk_t::block_t& blk, u32 offset) const;
		void lba_to_2b(lba_t lba, fsblk_t::block_t& blk, u32 offset) const;

		bool is_dd() const;
		unsigned sectors_per_track() const;
		unsigned tot_sectors() const { return is_dd() ? DD_SECTORS : SD_SECTORS; }

		static std::string dec_name(std::vector<u8>::const_iterator begin, std::vector<u8>::const_iterator end);
		static bool validate_filename(const std::string& s);
		static bool user_can_create(const std::string& name);
		static bool user_can_write(const std::string& name);
		void ensure_dir_loaded();
		void check_map();
		std::vector<u8> encode_dir() const;
		void store_dir_map();
		dir_t::iterator scan_dir(const std::string& name);
		static meta_data get_metadata(const entry& e);
		std::pair<const lba_list*, unsigned> find_file(const std::string& name);
		std::pair<const lba_list*, unsigned> find_file(const std::vector<std::string> &path);
		lba_list get_file_allocation(lba_t first_link, unsigned size, sect_map& in_use);
		std::vector<u8> get_file_content(const lba_list& sects, unsigned size);
		void store_file_content(const lba_list& sects, const std::vector<u8>& data);
		unsigned map_size() const;
		sect_map decode_map(const std::vector<u8>& data) const;
		std::vector<u8> encode_map(const sect_map& map) const;
		void set_not_in_use(const lba_list& lbas);
		bool allocate(unsigned file_size, lba_list& list);
		void allocate_seq(lba_t first_lba, unsigned blocks, lba_list& list);
		void allocate_t0(lba_list& list);

		// Has dir. been loaded?
		bool m_dir_loaded;
		// Directory
		dir_t m_dir;
		// Sectors in use
		sect_map m_in_use;
	};
}

isis_impl::isis_impl(fsblk_t& blockdev)
	: filesystem_t(blockdev , SECTOR_SIZE)
	, m_dir_loaded{false}
	, m_dir()
	, m_in_use{}
{
}

meta_data isis_impl::volume_metadata()
{
	meta_data res;

	const auto& [ lbas, size ] = find_file(std::string(ISIS_LAB));
	if (lbas != nullptr && size >= 11) {
		auto file_data = get_file_content(*lbas, size);
		auto vol_name = dec_name(file_data.cbegin(), file_data.cbegin() + 9);
		if (vol_name.empty()) {
			vol_name = "UNTITLED";
		}
		res.set(meta_name::name, vol_name);
		try {
			res.set(meta_name::os_version, u64(std::stoul(std::string(file_data.cbegin() + 9, file_data.cbegin() + 11))));
		} catch (...) {
			res.set(meta_name::os_version, 0);
		}
	}
	return res;
}

std::pair<std::error_condition, meta_data> isis_impl::metadata(const std::vector<std::string> &path)
{
	if (path.size() != 1) {
		return std::make_pair(error::not_found, meta_data{});
	}

	ensure_dir_loaded();

	auto it = scan_dir(path.front());
	if (it == m_dir.end()) {
		return std::make_pair(error::not_found, meta_data{});
	}

	auto meta = get_metadata(*it);
	return std::make_pair(std::error_condition(), std::move(meta));
}

std::pair<std::error_condition, std::vector<dir_entry>> isis_impl::directory_contents(const std::vector<std::string> &path)
{
	if (path.empty()) {
		ensure_dir_loaded();
		// Copy all dir_entry(s) out of m_dir
		std::vector<dir_entry> dir_entries;
		for (const auto& e : m_dir) {
			if (e.m_alloc_state == DIR_IN_USE) {
				auto meta = get_metadata(e);
				dir_entries.emplace_back(dir_entry_type::file, std::move(meta));
			}
		}
		return std::make_pair(std::error_condition(), dir_entries);
	} else {
		return std::make_pair(error::not_found, std::vector<dir_entry>{});
	}
}

std::error_condition isis_impl::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	if (!path.empty()) {
		return error::invalid_name;
	}

	auto name = meta.get_string(meta_name::name);

	// Validate filename
	auto pt_pos = name.find('.');

	std::string filename = name.substr(0, pt_pos);

	if (filename.size() < 1 || filename.size() > 6 || !validate_filename(filename)) {
		return error::invalid_name;
	}

	if (pt_pos != std::string::npos) {
		auto ext = name.substr(pt_pos + 1);
		if (ext.size() < 1 || ext.size() > 3 || !validate_filename(ext)) {
			return error::invalid_name;
		}
		filename.push_back('.');
		filename += ext;
	}

	// Check that file can be created by user
	if (!user_can_create(filename)) {
		return error::unsupported;
	}

	// Check that file doesn't exist
	ensure_dir_loaded();
	auto it = scan_dir(filename);
	if (it != m_dir.end()) {
		return error::already_exists;
	}

	// Find a free entry in directory
	for (it = m_dir.begin(); it != m_dir.end(); it++) {
		if (it->m_alloc_state != DIR_IN_USE) {
			break;
		}
	}
	if (it == m_dir.end()) {
		return error::no_space;
	}

	// Allocate space
	lba_list lbas;
	if (!allocate(0, lbas)) {
		return error::no_space;
	}

	// Fill dir entry
	// TODO: get attributes from metadata
	it->m_alloc_state = DIR_IN_USE;
	it->m_name = filename;
	it->m_attrs = 0;
	it->m_size = 0;
	it->m_1st_link = lbas.front();
	it->m_lbas = std::move(lbas);

	// Update directory & map
	store_dir_map();

	return std::error_condition();
}

std::pair<std::error_condition, std::vector<u8>> isis_impl::file_read(const std::vector<std::string> &path)
{
	const auto& [ lbas, size ] = find_file(path);
	if (lbas != nullptr) {
		auto file_data = get_file_content(*lbas, size);
		return std::make_pair(std::error_condition(), std::move(file_data));
	}
	return std::make_pair(error::not_found, std::vector<u8>{});
}

std::error_condition isis_impl::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	if (path.size() != 1) {
		return error::not_found;
	}

	// Check that file is user-writeable
	const auto& filename = path.front();
	if (!user_can_write(filename)) {
		return error::unsupported;
	}

	// ISIS.T0 can only be 2944 bytes long
	bool is_t0 = filename == ISIS_T0;
	if (is_t0 && data.size() != ISIS_T0_SIZE) {
		return error::incorrect_size;
	}

	// Check that file already exists
	ensure_dir_loaded();
	auto it = scan_dir(filename);
	if (it == m_dir.end()) {
		return error::not_found;
	}

	// De-allocate current blocks
	set_not_in_use(it->m_lbas);

	// Allocate blocks for new file size
	lba_list lbas;
	if (is_t0) {
		allocate_t0(lbas);
	} else if (!allocate(data.size(), lbas)) {
		return error::no_space;
	}

	// Update dir entry
	it->m_size = data.size();
	it->m_1st_link = lbas.front();
	it->m_lbas = lbas;

	// Store file content
	store_file_content(lbas, data);

	// Update directory & map
	store_dir_map();

	return std::error_condition();
}

std::error_condition isis_impl::format(const meta_data &meta)
{
	entry dir_e;
	dir_e.m_alloc_state = DIR_IN_USE;
	dir_e.m_name = ISIS_DIR;
	dir_e.m_attrs = DEF_SYS_ATTRS;
	dir_e.m_size = DIR_SIZE;
	dir_e.m_1st_link = ts_2_lba(DIR_TS);
	m_dir[ 0 ] = dir_e;
	entry map_e;
	map_e.m_alloc_state = DIR_IN_USE;
	map_e.m_name = ISIS_MAP;
	map_e.m_attrs = DEF_SYS_ATTRS;
	map_e.m_size = map_size();
	map_e.m_1st_link = ts_2_lba(MAP_TS);
	m_dir[ 1 ] = map_e;

	// Empty T0
	entry t0_e;
	t0_e.m_alloc_state = DIR_IN_USE;
	t0_e.m_name = ISIS_T0;
	t0_e.m_attrs = DEF_SYS_ATTRS;
	t0_e.m_size = ISIS_T0_SIZE;
	t0_e.m_1st_link = ts_2_lba(T0_1ST_LINK_TS);
	m_dir[ 2 ] = t0_e;
	std::vector<u8> t0_content(ISIS_T0_SIZE);
	lba_list t0_lbas;
	allocate_t0(t0_lbas);
	store_file_content(t0_lbas, t0_content);

	// Empty ISIS.LAB
	// TODO: fill with volume metadata
	entry lab_e;
	lab_e.m_alloc_state = DIR_IN_USE;
	lab_e.m_name = ISIS_LAB;
	lab_e.m_attrs = DEF_SYS_ATTRS;
	lab_e.m_size = is_dd() ? ISIS_LAB_DD_SIZE : ISIS_LAB_SD_SIZE;
	lab_e.m_1st_link = ts_2_lba(LAB_TS);
	m_dir[ 3 ] = lab_e;
	if (is_dd()) {
		std::vector<u8> lab_content(ISIS_LAB_DD_SIZE);
		lba_list lab_lbas;
		allocate_seq(ts_2_lba(LAB_TS), ISIS_LAB_DD_1ST_BLOCKS + 1, lab_lbas);
		allocate_seq(ts_2_lba(LAB_DD_2ND_TS), ISIS_LAB_DD_2ND_BLOCKS, lab_lbas);
		store_file_content(lab_lbas, lab_content);
	} else {
		std::vector<u8> lab_content(ISIS_LAB_SD_SIZE);
		lba_list lab_lbas;
		allocate_seq(ts_2_lba(LAB_TS), ISIS_LAB_SD_BLOCKS + 1, lab_lbas);
		store_file_content(lab_lbas, lab_content);
	}

	store_dir_map();
	return std::error_condition();
}

isis_impl::lba_t isis_impl::ts_2_lba(const track_sect &ts) const
{
	unsigned secs_per_track = sectors_per_track();

	if (ts.first == 0 && ts.second == 0) {
		return -1;
	} else if (ts.first >= img_format::TRACKS || ts.second < 1 ||
			   ts.second > secs_per_track) {
		throw std::out_of_range(util::string_format("Invalid TS=(%u,%u)", ts.first, ts.second));
	} else {
		return ts.first * secs_per_track + ts.second - 1;
	}
}

isis_impl::track_sect isis_impl::lba_2_ts(lba_t lba) const
{
	if (lba < 0) {
		return track_sect{ 0 , 0 };
	} else {
		unsigned secs_per_track = sectors_per_track();

		unsigned track = unsigned(lba) / secs_per_track;
		unsigned sect = unsigned(lba) % secs_per_track;
		return track_sect{ u8(track), u8(sect) + 1 };
	}
}

isis_impl::lba_t isis_impl::lba_from_2b(const fsblk_t::block_t& blk, u32 offset) const
{
	return ts_2_lba(track_sect{ blk.r8(offset + 1), blk.r8(offset) });
}

void isis_impl::lba_to_2b(lba_t lba, fsblk_t::block_t& blk, u32 offset) const
{
	auto ts = lba_2_ts(lba);

	blk.w8(offset, ts.second);
	blk.w8(offset + 1, ts.first);
}

bool isis_impl::is_dd() const
{
	return m_blockdev.block_count() == DD_SECTORS;
}

unsigned isis_impl::sectors_per_track() const
{
	return is_dd() ? img_format::SECTORS : img_format::SECTORS / 2;
}

std::string isis_impl::dec_name(std::vector<u8>::const_iterator begin, std::vector<u8>::const_iterator end)
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
			} else if (VALID_CHARS.find(char(c)) != std::string::npos) {
				res.push_back(char(c));
			} else {
				throw std::runtime_error("Invalid name in dir. entry");
			}
		}
	}

	return res;
}

bool isis_impl::validate_filename(const std::string& s)
{
	auto pos = s.find_first_not_of(VALID_CHARS);
	return pos == std::string::npos;
}

bool isis_impl::user_can_create(const std::string& name)
{
	return name != ISIS_DIR &&
		name != ISIS_MAP &&
		name != ISIS_LAB &&
		name != ISIS_T0;
}

bool isis_impl::user_can_write(const std::string& name)
{
	return name != ISIS_DIR &&
		name != ISIS_MAP &&
		name != ISIS_LAB;
}

void isis_impl::ensure_dir_loaded()
{
	if (m_dir_loaded) {
		return;
	}

	m_in_use.reset();

	// Fetch dir
	auto dir_lba_list = get_file_allocation(ts_2_lba(DIR_TS), DIR_SIZE, m_in_use);
	auto enc_dir = get_file_content(dir_lba_list, DIR_SIZE);

	m_in_use.reset();

	// Scan the whole dir.
	auto dir_it = m_dir.begin();
	for (auto i = enc_dir.cbegin(); i < enc_dir.cend(); i += DIR_ENTRY_SIZE, dir_it++) {
		entry new_entry;
		// +0: Entry state
		// 00   Entry in use
		// 7f   Entry empty/never used
		// ff   Entry deleted
		u8 entry_state = *i;
		new_entry.m_alloc_state = entry_state;
		if (entry_state == DIR_IN_USE) {
			new_entry.m_alloc_state = entry_state;
			// +1..+6: File name
			// +7..+9: File extension
			auto file_name = dec_name(i + 1, i + 7);
			if (file_name.empty()) {
				throw std::runtime_error("Null filename");
			}
			auto file_ext  = dec_name(i + 7, i + 10);
			if (!file_ext.empty()) {
				file_name.push_back('.');
				file_name += file_ext;
			}
			auto other_entry = scan_dir(file_name);
			if (other_entry != m_dir.end()) {
				throw std::runtime_error(util::string_format("Duplicated file name (%s)", file_name));
			}
			new_entry.m_name = file_name;
			// +10: Attributes
			new_entry.m_attrs = i[ 10 ];
			// +11: bytes in last sector
			auto remainder = i[ 11 ];
			// +12..+13: size in sectors
			unsigned sectors = get_u16le(&i[ 12 ]);
			if (remainder > SECTOR_SIZE) {
				throw std::runtime_error("Invalid dir. entry");
			}
			unsigned size;
			if (sectors == 0) {
				size = 0;
			} else {
				size = (sectors - 1) * SECTOR_SIZE + remainder;
			}
			new_entry.m_size = size;
			// +14..+15: Sector/Track of first linkage block
			new_entry.m_1st_link = ts_2_lba(track_sect{ i[ 15 ], i[ 14 ] });
			auto file_lba_list = get_file_allocation(new_entry.m_1st_link, size, m_in_use);
			new_entry.m_lbas = std::move(file_lba_list);
		} else if (entry_state != DIR_NEVER_USED && entry_state != DIR_DELETED) {
			throw std::runtime_error(util::string_format("Invalid dir. entry type (%02x)", entry_state));
		}
		*dir_it = std::move(new_entry);
	}

	m_dir_loaded = true;

	// Consistency checks
	// ISIS.DIR position and size
	if (m_dir[ 0 ].m_alloc_state != DIR_IN_USE ||
		m_dir[ 0 ].m_name != ISIS_DIR ||
		m_dir[ 0 ].m_size != DIR_SIZE ||
		m_dir[ 0 ].m_1st_link != ts_2_lba(DIR_TS)) {
		throw std::runtime_error(util::string_format("Invalid %s", ISIS_DIR));
	}
	// ISIS.MAP position
	if (m_dir[ 1 ].m_alloc_state != DIR_IN_USE ||
		m_dir[ 1 ].m_name != ISIS_MAP ||
		m_dir[ 1 ].m_1st_link != ts_2_lba(MAP_TS)) {
		throw std::runtime_error(util::string_format("Invalid %s", ISIS_MAP));
	}
	// ISIS.T0 position and size
	if (m_dir[ 2 ].m_alloc_state != DIR_IN_USE ||
		m_dir[ 2 ].m_name != ISIS_T0 ||
		m_dir[ 2 ].m_size != ISIS_T0_SIZE ||
		m_dir[ 2 ].m_1st_link != ts_2_lba(T0_1ST_LINK_TS)) {
		throw std::runtime_error(util::string_format("Invalid %s", ISIS_T0));
	}
	// ISIS.LAB position and size
	if (m_dir[ 3 ].m_alloc_state != DIR_IN_USE ||
		m_dir[ 3 ].m_name != ISIS_LAB ||
		m_dir[ 3 ].m_size != (is_dd() ? ISIS_LAB_DD_SIZE : ISIS_LAB_SD_SIZE) ||
		m_dir[ 3 ].m_1st_link != ts_2_lba(LAB_TS)) {
		throw std::runtime_error(util::string_format("Invalid %s", ISIS_LAB));
	}

	check_map();
}

void isis_impl::check_map()
{
	const auto& [ lbas, size ] = find_file(std::string(ISIS_MAP));
	if (lbas == nullptr) {
		fprintf(stderr, "WARNING: map file missing\n");
		return;
	}
	auto exp_size = map_size();
	if (size != exp_size) {
		fprintf(stderr, "WARNING: map file has wrong size (%u vs %u)\n", size, exp_size);
		return;
	}
	auto map_file_data = get_file_content(*lbas, size);
	auto dec_map = decode_map(map_file_data);
	if (dec_map != m_in_use) {
		fprintf(stderr, "WARNING: Incorrect map file\n");
	}
}

std::vector<u8> isis_impl::encode_dir() const
{
	std::vector<u8> res(DIR_SIZE);

	auto i = res.begin();
	for (const auto& e : m_dir) {
		// +0: Entry state
		i[ 0 ] = e.m_alloc_state;
		if (e.m_alloc_state == DIR_IN_USE) {
			// +1..+6: File name
			// +7..+9: File extension
			auto pos = e.m_name.find('.');
			if (pos != std::string::npos) {
				for (std::string::size_type j = 0; j < pos; j++) {
					i[ 1 + j ] = u8(e.m_name[ j ]);
				}
				for (std::string::size_type j = pos + 1; j < e.m_name.size(); j++) {
					i[ 6 + j - pos ] = u8(e.m_name[ j ]);
				}
			} else {
				for (std::string::size_type j = 0; j < e.m_name.size(); j++) {
					i[ 1 + j ] = u8(e.m_name[ j ]);
				}
			}
			// +10: Attributes
			i[ 10 ] = e.m_attrs;
			// +11: bytes in last sector
			// +12..+13: size in sectors
			unsigned sects = e.m_size / SECTOR_SIZE;
			unsigned rem = e.m_size % SECTOR_SIZE;
			i[ 11 ] = rem != 0 ? rem : SECTOR_SIZE;
			if (rem) {
				sects++;
			}
			i[ 12 ] = u8(sects & 0xff);
			i[ 13 ] = u8(sects >> 8);
			// +14..+15: Sector/Track of first linkage block
			auto ts = lba_2_ts(e.m_1st_link);
			i[ 14 ] = ts.second;
			i[ 15 ] = ts.first;
		}
		i += DIR_ENTRY_SIZE;
	}

	return res;
}

void isis_impl::store_dir_map()
{
	// Store directory
	auto dir_content = encode_dir();
	lba_list dir_lbas;
	allocate_seq(ts_2_lba(DIR_TS), DIR_SECTS + 1, dir_lbas);
	store_file_content(dir_lbas, dir_content);
	// Store map
	unsigned map_sects = map_size() / SECTOR_SIZE;
	lba_list map_lbas;
	allocate_seq(ts_2_lba(MAP_TS), map_sects + 1, map_lbas);
	auto map_content = encode_map(m_in_use);
	store_file_content(map_lbas, map_content);
}

isis_impl::dir_t::iterator isis_impl::scan_dir(const std::string& name)
{
	for (auto it = m_dir.begin(); it != m_dir.end(); it++) {
		if (it->m_alloc_state == DIR_IN_USE && it->m_name == name) {
			return it;
		}
	}
	return m_dir.end();
}

meta_data isis_impl::get_metadata(const entry& e)
{
	meta_data meta;
	meta.set(meta_name::name, e.m_name);
	std::string attributes;
	// Format
	attributes.push_back(e.m_attrs & 0x80 ? 'F' : ' ');
	// Write protected
	attributes.push_back(e.m_attrs & 0x04 ? 'W' : ' ');
	// System
	attributes.push_back(e.m_attrs & 0x02 ? 'S' : ' ');
	// Invisible
	attributes.push_back(e.m_attrs & 0x01 ? 'I' : ' ');
	meta.set(meta_name::attributes, std::move(attributes));
	meta.set(meta_name::length, e.m_size);

	return meta;
}

std::pair<const isis_impl::lba_list*, unsigned> isis_impl::find_file(const std::string& name)
{
	ensure_dir_loaded();

	auto it = scan_dir(name);

	if (it != m_dir.end()) {
		return std::make_pair(&it->m_lbas, it->m_size);
	} else {
		return std::make_pair(nullptr, 0);
	}
}

std::pair<const isis_impl::lba_list*, unsigned> isis_impl::find_file(const std::vector<std::string> &path)
{
	if (path.size() == 1) {
		return find_file(path[ 0 ]);
	}
	return std::make_pair(nullptr, 0);
}

isis_impl::lba_list isis_impl::get_file_allocation(lba_t first_link, unsigned size, sect_map& in_use)
{
	lba_t prev_lba = -1;
	lba_t curr_map = first_link;
	lba_list res;
	while (true) {
		if (in_use[ curr_map ]) {
			throw std::runtime_error(util::string_format("Sector %d used more than once", curr_map));
		}
		in_use.set(curr_map);
		res.push_back(curr_map);
		// Get a linkage block
		auto map_sect = m_blockdev.get(curr_map);
		auto prev_ptr = lba_from_2b(*map_sect , 0);
		auto next_ptr = lba_from_2b(*map_sect , 2);
		if (prev_lba != prev_ptr) {
			throw std::runtime_error(util::string_format("Incorrect backward linking in sector %d", curr_map));
		}
		// Scan all pointers in linkage block
		for (u32 i = 4; i < SECTOR_SIZE; i += 2) {
			auto ptr = lba_from_2b(*map_sect , i);
			if (size > 0) {
				if (ptr < 0) {
					throw std::runtime_error(util::string_format("Unexpected end of pointer list in sector %d", curr_map));
				} else if (in_use[ ptr ]) {
					throw std::runtime_error(util::string_format("Sector %d used more than once", ptr));
				} else {
					in_use.set(ptr);
					res.push_back(ptr);
					unsigned to_copy = std::min(size , SECTOR_SIZE);
					size -= to_copy;
				}
			} else if (ptr >= 0) {
				throw std::runtime_error(util::string_format("Unexpected continuation of pointer list in sector %d", curr_map));
			}
		}
		// More linkage blocks?
		if (size > 0) {
			if (next_ptr < 0) {
				throw std::runtime_error("Pointer list too short");
			} else {
				prev_lba = curr_map;
				curr_map = next_ptr;
			}
		} else if (next_ptr >= 0) {
			throw std::runtime_error("Pointer list too long");
		} else {
			break;
		}
	}
	return res;
}

std::vector<u8> isis_impl::get_file_content(const lba_list& sects, unsigned size)
{
	std::vector<u8> res;

	res.resize(size);

	unsigned pos = 0;

	for (unsigned idx = 0; idx < sects.size(); idx++) {
		// Skip over linkage blocks
		if ((idx % (PTRS_PER_BLOCK + 1)) != 0) {
			unsigned to_copy = std::min(size, SECTOR_SIZE);
			auto data_sect = m_blockdev.get(sects[ idx ]);
			data_sect->read(0, res.data() + pos, to_copy);
			size -= to_copy;
			pos += to_copy;
		}
	}
	return res;
}

void isis_impl::store_file_content(const lba_list& sects, const std::vector<u8>& data)
{
	lba_t prev_ptr = -1;
	// Write linkage blocks
	// A linkage block has room for 62 pointers to data blocks
	for (unsigned idx = 0; idx < sects.size(); idx += (PTRS_PER_BLOCK + 1)) {
		lba_t link_lba = sects[ idx ];
		auto blk = m_blockdev.get(link_lba);
		// Pointer to previous block
		lba_to_2b(prev_ptr, *blk, 0);
		prev_ptr = link_lba;
		lba_t next_ptr;
		if (sects.size() - idx > (PTRS_PER_BLOCK + 1)) {
			// Points to next linkage block
			next_ptr = sects[ idx + PTRS_PER_BLOCK + 1 ];
		} else {
			// No more linkage blocks
			next_ptr = -1;
		}
		// Pointer to next block
		lba_to_2b(next_ptr, *blk, 2);
		// Pointers to data blocks
		for (unsigned j = 0; j < PTRS_PER_BLOCK; j++) {
			lba_to_2b((j + idx + 1 < sects.size()) ? sects[ j + idx + 1 ] : -1, *blk, 4 + 2 * j);
		}
	}
	// Write data blocks
	auto ptr = data.data();
	u32 to_go = u32(data.size());
	for (unsigned idx = 0; idx < sects.size(); idx++) {
		if ((idx % (PTRS_PER_BLOCK + 1)) == 0) {
			// Skip over linkage blocks
			continue;
		}
		u32 count = std::min<u32>(to_go, SECTOR_SIZE);
		auto blk = m_blockdev.get(sects[ idx ]);
		blk->write(0, ptr, count);
		ptr += count;
		to_go -= count;
	}
}

unsigned isis_impl::map_size() const
{
	return is_dd() ? ISIS_MAP_DD_SIZE : ISIS_MAP_SD_SIZE;
}

isis_impl::sect_map isis_impl::decode_map(const std::vector<u8>& data) const
{
	sect_map res;
	auto sects = tot_sectors();
	for (unsigned i = 0; i < sects; i++) {
		if ((data[ i / 8 ] & (0x80 >> (i % 8))) != 0) {
			res.set(i);
		}
	}
	return res;
}

std::vector<u8> isis_impl::encode_map(const sect_map& map) const
{
	size_t sz = map_size();
	std::vector<u8> res(sz);
	auto sects = tot_sectors();
	for (unsigned i = 0; i < sects; i++) {
		if (map[ i ]) {
			res[ i / 8 ] |= (0x80 >> (i % 8));
		}
	}
	return res;
}

void isis_impl::set_not_in_use(const lba_list& lbas)
{
	for (const auto lba : lbas) {
		m_in_use.reset(lba);
	}
}

bool isis_impl::allocate(unsigned file_size, lba_list& list)
{
	unsigned data_blocks = (file_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	unsigned linkage_blocks = (data_blocks + PTRS_PER_BLOCK - 1) / PTRS_PER_BLOCK;
	if (data_blocks == 0) {
		// At least one linkage block is always allocated, even for null files
		linkage_blocks = 1;
	}
	unsigned to_do = data_blocks + linkage_blocks;
	lba_t first_blk = is_dd() ? ts_2_lba(ALLOC_START_DD) : ts_2_lba(ALLOC_START_SD);
	auto sects = tot_sectors();
	for (lba_t i = first_blk; i < sects; i++) {
		if (!m_in_use[ i ]) {
			list.push_back(i);
			m_in_use.set(i);
			to_do--;
			if (to_do == 0) {
				break;
			}
		}
	}
	return to_do == 0;
}

void isis_impl::allocate_seq(lba_t first_lba, unsigned blocks, lba_list& list)
{
	for (lba_t i = first_lba; i < first_lba + blocks; i++) {
		list.push_back(i);
		m_in_use.set(i);
	}
}

void isis_impl::allocate_t0(lba_list& list)
{
	// T0 keeps the linkage block at the end of data blocks
	allocate_seq(ts_2_lba(T0_1ST_LINK_TS), 1, list);
	allocate_seq(ts_2_lba(T0_TS), T0_SECTS, list);
}

// +------------+
// | isis_image |
// +------------+
const char *isis_image::name() const
{
	return "isis";
}

const char *isis_image::description() const
{
	return "Intel ISIS-II";
}

void isis_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_IMG_FORMAT, floppy_image::FF_8, floppy_image::SSSD, SD_IMAGE_SIZE, "isis_sd", "Intel ISIS-II SD");
	fe.add(FLOPPY_IMG_FORMAT, floppy_image::FF_8, floppy_image::SSDD, DD_IMAGE_SIZE, "isis_dd", "Intel ISIS-II DD");
}

bool isis_image::can_format() const
{
	return true;
}

bool isis_image::can_read() const
{
	return true;
}

bool isis_image::can_write() const
{
	return true;
}

bool isis_image::has_rsrc() const
{
	return false;
}

std::vector<meta_description> isis_image::volume_meta_description() const
{
	std::vector<meta_description> res;

	res.emplace_back(meta_description(meta_name::name, "UNTITLED", false, [](const meta_value &m) { return m.as_string().size() <= 9; }, "Volume name, up to 9 characters"));
	res.emplace_back(meta_description(meta_name::os_version, 41, false, [](const meta_value &m) { return m.as_number() <= 99; }, "O/S version, 0..99"));

	return res;
}

std::vector<meta_description> isis_image::file_meta_description() const
{
	std::vector<meta_description> res;

	res.emplace_back(meta_description(meta_name::name, "Empty", false, [](const meta_value &m) { return m.as_string().size() <= 9; }, "File name, up to 9 characters"));
	res.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "File size"));
	res.emplace_back(meta_description(meta_name::attributes, "", true, nullptr, "File attributes"));

	return res;
}

std::unique_ptr<filesystem_t> isis_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<isis_impl>(blockdev);
}
