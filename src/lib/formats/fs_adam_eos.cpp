// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    fs_adam_eos.cpp

    Adam EOS (Elementary Operating System) filesystem implementation

***************************************************************************/

#include "fs_adam_eos.h"
#include "adam_dsk.h"
#include "fsblk.h"
#include "fsblk_multi.h"

#include "corestr.h"

#include <algorithm>
#include <memory>
#include <numeric>

namespace fs { const adam_eos_image ADAM_EOS; }

using namespace fs;

namespace {

// Each 1024-byte block is actually stored in a pair of nonconsecutive sectors.
// This helper class manages the (1,6,3,8,5,2,7,4) interleave, which is transparent to EOS.
class fsblk_adam_interleaved : public fsblk_t
{
public:
	fsblk_adam_interleaved(fsblk_t &base) : m_base(base) { }

	virtual u32 block_count() const override { return m_base.block_count() / 2; }
	virtual block_t::ptr get(u32 id) override
	{
		const u32 l = id * 2;
		const u32 h = l ^ 5;
		return std::make_shared<multi_block<2>>(m_base.get(l), m_base.get(h));
	}
	virtual void fill_all(u8 data) override { m_base.fill_all(data); }

private:
	fsblk_t &m_base;
};

class adam_eos_impl : public filesystem_t
{
	friend class fs::adam_eos_image;

	static constexpr u8 ETX = 0x03;
	static constexpr unsigned NAME_LENGTH = 12; // this includes the file type
	static constexpr u32 DIRECTORY_CHECK = 0xff00aa55; // magic

	static constexpr unsigned VOL_NAME = 0;
	static constexpr unsigned VOL_ATTR = 12;
	static constexpr unsigned VOL_DIRSIZE = 12;
	static constexpr unsigned VOL_DIR_CHECK = 13;
	static constexpr unsigned VOL_SIZE = 17;
	static constexpr unsigned VOL_YEAR = 23;
	static constexpr unsigned VOL_MONTH = 24;
	static constexpr unsigned VOL_DAY = 25;
	static constexpr unsigned VOL_DES_LENGTH = 26;

	static constexpr unsigned DIR_NAME = 0;
	static constexpr unsigned DIR_ATTR = 12;
	static constexpr unsigned DIR_START_BLOCK = 13;
	static constexpr unsigned DIR_MAX_LENGTH = 17;
	static constexpr unsigned DIR_USED_LENGTH = 19;
	static constexpr unsigned DIR_LAST_COUNT = 21;
	static constexpr unsigned DIR_YEAR = 23;
	static constexpr unsigned DIR_MONTH = 24;
	static constexpr unsigned DIR_DAY = 25;
	static constexpr unsigned DIR_ENT_LENGTH = 26;

	static constexpr unsigned ATTR_PERMANENT = 0x80; // protects against MODE_UPDATE (= 3)
	static constexpr unsigned ATTR_WRITE_PROT = 0x40; // protects against MODE_WRITE (= 2)
	static constexpr unsigned ATTR_READ_PROT = 0x20; // protects against MODE_READ (= 1)
	static constexpr unsigned ATTR_USER = 0x10;
	static constexpr unsigned ATTR_SYSTEM = 0x08;
	static constexpr unsigned ATTR_DELETED = 0x04;
	static constexpr unsigned ATTR_EXECUTE = 0x02; // protects against MODE_EXECUTE (= 4)
	static constexpr unsigned ATTR_HOLE = 0x01;

public:
	adam_eos_impl(fsblk_t &blockdev);
	virtual ~adam_eos_impl() = default;

	virtual meta_data volume_metadata() override;
	virtual std::error_condition volume_metadata_change(const meta_data &meta) override;
	virtual std::error_condition format(const meta_data &meta) override;
	virtual std::pair<std::error_condition, meta_data> metadata(const std::vector<std::string> &path) override;
	virtual std::error_condition metadata_change(const std::vector<std::string> &path, const meta_data &meta) override;
	virtual std::error_condition rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath) override;
	virtual std::error_condition remove(const std::vector<std::string> &path) override;
	virtual std::pair<std::error_condition, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path) override;
	virtual std::pair<std::error_condition, std::vector<u8>> file_read(const std::vector<std::string> &path) override;
	virtual std::tuple<std::error_condition, std::vector<u32>, std::vector<u32>> enum_blocks(const std::vector<std::string> &path) override;
	virtual std::error_condition file_create(const std::vector<std::string> &path, const meta_data &meta) override;
	virtual std::error_condition file_write(const std::vector<std::string> &path, const std::vector<u8> &data) override;

private:
	struct dir_handle
	{
		dir_handle(fsblk_t::block_t::ptr &&blkp, unsigned offs)
			: blockp(std::move(blkp)), offset(offs)
		{
		}

		fsblk_t::block_t::ptr blockp;
		unsigned offset;
	};

	static inline u32 blocks_to_bytes(u16 block_count, u16 last_count) { return block_count ? (block_count - 1) * u32(1024) + std::min<u16>(last_count, 1024) : 0; }
	static bool validate_filename(std::string_view str, bool with_type) noexcept;
	static bool validate_filetype(std::string_view str) noexcept;
	static bool validate_permissions(std::string_view str) noexcept;
	static bool is_valid_filetype(u8 filetype) noexcept;
	static util::arbitrary_datetime make_date(u8 y, u8 m, u8 d);
	static std::string read_name(const fsblk_t::block_t &dir_block, unsigned offset);
	static bool filename_matches(const fsblk_t::block_t &dir_block, unsigned offset, std::string_view name);
	static void write_name(fsblk_t::block_t &dir_block, unsigned offset, std::string_view name);
	static void write_filename(fsblk_t::block_t &dir_block, unsigned offset, std::string_view name);
	static void write_special_dir_entry(fsblk_t::block_t &dir_block, unsigned offset, std::string_view name, u8 attr, u32 start_block, u16 max_length, u16 used_blocks, u16 last_count);

	dir_handle find_dir_entry(std::string_view name);

	fsblk_adam_interleaved m_interleaved;
};

// Not clear what this type is supposed to be called (SmartBASIC hides it), but many disks execute a file with this type upon boot
constexpr std::string_view filetype_stx = "STX";


adam_eos_impl::adam_eos_impl(fsblk_t &blockdev)
	: filesystem_t(blockdev, 512)
	, m_interleaved(blockdev)
{
}


bool adam_eos_impl::validate_filename(std::string_view str, bool with_type) noexcept
{
	return str.length() <= NAME_LENGTH - (with_type ? 1 : 0) && str.find_first_of(ETX) == std::string_view::npos;
}

bool adam_eos_impl::validate_filetype(std::string_view str) noexcept
{
	using namespace std::literals;
	return (str.length() == 1 && str.find_first_not_of("AaHhC"sv) == std::string_view::npos) || util::strequpper(str, filetype_stx);
}

bool adam_eos_impl::validate_permissions(std::string_view str) noexcept
{
	using namespace std::literals;
	if (!str.empty() && (str.front() == '+' || str.front() == '-'))
		str.remove_prefix(1);
	return str.find_first_not_of("rwx."sv) == std::string_view::npos;
}

bool adam_eos_impl::is_valid_filetype(u8 filetype) noexcept
{
	// Lowercase types are used for SmartBASIC backups; C type is used by CopyCart
	return filetype == 'A' || filetype == 'a' || filetype == 'H' || filetype == 'h' || filetype == 'C' || filetype == 0x02;
}

util::arbitrary_datetime adam_eos_impl::make_date(u8 y, u8 m, u8 d)
{
	// TODO: what to do about dates that are not proper BCD?
	util::arbitrary_datetime dt;
	dt.second = 0;
	dt.minute = 0;
	dt.hour = 0;
	dt.day_of_month = bcd_2_dec(d);
	dt.month = bcd_2_dec(m);
	if (y == 0 && m == 0 && d == 0)
		dt.year = 0;
	else
	{
		dt.year = 1900 + bcd_2_dec(y);
		if (dt.year < 1950)
			dt.year += 100;
	}
	return dt;
}

std::string adam_eos_impl::read_name(const fsblk_t::block_t &dir_block, unsigned offset)
{
	std::string name(dir_block.rstr(offset + DIR_NAME, NAME_LENGTH));
	const auto etx_pos = name.find_first_of(ETX);
	if (etx_pos != std::string::npos)
		name.resize(etx_pos); // truncate
	return name;
}

bool adam_eos_impl::filename_matches(const fsblk_t::block_t &dir_block, unsigned offset, std::string_view name)
{
	// sanity check
	if (name.empty() || name.length() >= NAME_LENGTH)
		return false;

	return dir_block.eqstr(offset + DIR_NAME, name)
		&& (name.length() == NAME_LENGTH - 1 || dir_block.r8(offset + DIR_NAME + name.length() + 1) == ETX)
		&& is_valid_filetype(dir_block.r8(offset + DIR_NAME + name.length()));
}

void adam_eos_impl::write_name(fsblk_t::block_t &dir_block, unsigned offset, std::string_view name)
{
	dir_block.wstr(offset, name);

	if (name.length() < NAME_LENGTH)
	{
		dir_block.w8(offset + name.length(), ETX);

		// Fill any remaining bytes with spaces as per convention
		dir_block.fill(offset + name.length() + 1, NAME_LENGTH - name.length() - 1, 0x20);
	}
}

void adam_eos_impl::write_filename(fsblk_t::block_t &dir_block, unsigned offset, std::string_view name)
{
	// Rename the file, but preserve its old type
	const u8 filetype = dir_block.r8(offset + DIR_NAME + read_name(dir_block, offset).length() - 1);
	dir_block.wstr(offset + DIR_NAME, name);
	dir_block.w8(offset + DIR_NAME + name.length(), filetype);
	if (name.length() < NAME_LENGTH - 1)
		dir_block.w8(offset + DIR_NAME + name.length() + 1, ETX);
}

void adam_eos_impl::write_special_dir_entry(fsblk_t::block_t &dir_block, unsigned offset, std::string_view name, u8 attr, u32 start_block, u16 max_length, u16 used_blocks, u16 last_count)
{
	write_name(dir_block, offset + DIR_NAME, name);
	dir_block.w8(offset + DIR_ATTR, attr);
	dir_block.w32l(offset + DIR_START_BLOCK, start_block);
	dir_block.w16l(offset + DIR_MAX_LENGTH, max_length);
	dir_block.w16l(offset + DIR_USED_LENGTH, used_blocks);
	dir_block.w16l(offset + DIR_LAST_COUNT, last_count);
}


adam_eos_impl::dir_handle adam_eos_impl::find_dir_entry(std::string_view name)
{
	unsigned blk = 1;
	unsigned offset = VOL_DES_LENGTH;

	auto block = m_interleaved.get(blk);

	const unsigned lastblk = 1 + (block->r8(VOL_DIRSIZE) & 0x7f);
	while (blk < lastblk)
	{
		const u8 attr = block->r8(offset + DIR_ATTR);

		// Game over if we fall into the hole
		if (attr & ATTR_HOLE)
			break;

		// Skip deleted files
		if (!(attr & ATTR_DELETED) && filename_matches(*block, offset, name))
			return dir_handle(std::move(block), offset);

		offset += DIR_ENT_LENGTH;
		if (offset > 1024 - DIR_ENT_LENGTH)
		{
			if (++blk < lastblk)
				block = m_interleaved.get(blk);
			offset = 0;
		}
	}

	return dir_handle(nullptr, 0);
}


meta_data adam_eos_impl::volume_metadata()
{
	auto block = m_interleaved.get(1);

	const u8 y = block->r8(VOL_YEAR);
	const u8 m = block->r8(VOL_MONTH);
	const u8 d = block->r8(VOL_DAY);

	meta_data result;
	result.set(meta_name::name, read_name(*block, VOL_NAME));
	result.set(meta_name::protect, bool(block->r8(VOL_ATTR) & 0x80));
	result.set(meta_name::size_in_blocks, block->r32l(VOL_SIZE)); // only lower 16 bits used in practice?
	result.set(meta_name::creation_date, make_date(y, m, d));
	return result;
}

std::error_condition adam_eos_impl::volume_metadata_change(const meta_data &meta)
{
	auto dir = m_interleaved.get(1);

	if (meta.has(meta_name::name))
		write_name(*dir, VOL_NAME, meta.get_string(meta_name::name));
	if (meta.has(meta_name::protect))
	{
		u8 attr = dir->r8(VOL_ATTR);
		if (meta.get_flag(meta_name::protect))
			attr |= ATTR_PERMANENT;
		else
			attr &= ~ATTR_PERMANENT;
		dir->w8(VOL_ATTR, attr);
	}
	if (meta.has(meta_name::creation_date))
	{
		util::arbitrary_datetime creation_date = meta.get_date(meta_name::creation_date);
		dir->w8(VOL_YEAR, dec_2_bcd(creation_date.year % 100));
		dir->w8(VOL_MONTH, dec_2_bcd(creation_date.month));
		dir->w8(VOL_DAY, dec_2_bcd(creation_date.day_of_month));
	}

	return std::error_condition();
}

// A degenerate boot routine (provided by some copier programs) that stores RETN at the NMI vector, then jumps to _GOTO_WP
const u8 boot_code[] = { 0x01, 0xed, 0x45, 0xed, 0x43, 0x66, 0x00, 0xc3, 0xe7, 0xfc };

std::error_condition adam_eos_impl::format(const meta_data &meta)
{
	auto boot = m_blockdev.get(0);
	boot->write(0, std::data(boot_code), std::size(boot_code));

	{
		std::string volume_name = meta.get_string(meta_name::name, "UNTITLED");
		util::arbitrary_datetime creation_date = meta.get_date(meta_name::creation_date);
		u32 block_count = m_blockdev.block_count() / 2;

		auto dir_block = m_interleaved.get(1);
		dir_block->fill(0, VOL_DES_LENGTH + DIR_ENT_LENGTH * 3, 0x00); // includes reserved fields

		write_name(*dir_block, VOL_NAME, volume_name);
		dir_block->w8(VOL_DIRSIZE, 0x01);
		dir_block->w32l(VOL_DIR_CHECK, DIRECTORY_CHECK);
		dir_block->w32l(VOL_SIZE, block_count);
		dir_block->w8(VOL_YEAR, dec_2_bcd(creation_date.year % 100));
		dir_block->w8(VOL_MONTH, dec_2_bcd(creation_date.month));
		dir_block->w8(VOL_DAY, dec_2_bcd(creation_date.day_of_month));

		// Add user-inaccessible bookkeeping files
		// BOOT and DIRECTORY merely reserve space (their user and system attributes may differ), and BLOCKS LEFT is a sentinel
		using namespace std::literals;
		write_special_dir_entry(*dir_block, VOL_DES_LENGTH, "BOOT"sv, ATTR_PERMANENT | ATTR_USER, 0, 1, 1, 1024);
		write_special_dir_entry(*dir_block, VOL_DES_LENGTH + DIR_ENT_LENGTH, "DIRECTORY"sv, ATTR_PERMANENT | ATTR_WRITE_PROT | ATTR_USER | ATTR_SYSTEM, 1, 1, 1, 1024);
		write_special_dir_entry(*dir_block, VOL_DES_LENGTH + DIR_ENT_LENGTH * 2, "BLOCKS LEFT"sv, ATTR_HOLE, 2, block_count - 2, 0, 0);
	}

	return std::error_condition();
}

std::pair<std::error_condition, meta_data> adam_eos_impl::metadata(const std::vector<std::string> &path)
{
	if (path.size() != 1)
		return std::make_pair(error::unsupported, meta_data());

	const dir_handle dir = find_dir_entry(path.front());
	if (!dir.blockp)
		return std::make_pair(error::not_found, meta_data());

	const std::string name = read_name(*dir.blockp, dir.offset);
	const u8 attr = dir.blockp->r8(dir.offset + DIR_ATTR);
	const u16 used_blocks = dir.blockp->r16l(dir.offset + DIR_USED_LENGTH);
	const u8 y = dir.blockp->r8(dir.offset + DIR_YEAR);
	const u8 m = dir.blockp->r8(dir.offset + DIR_MONTH);
	const u8 d = dir.blockp->r8(dir.offset + DIR_DAY);

	meta_data meta;
	meta.set(meta_name::name, name.substr(0, name.length() - 1));
	meta.set(meta_name::file_type, name.back() == 0x02 ? filetype_stx : std::string(1, name.back()));
	meta.set(meta_name::protect, bool(attr & ATTR_PERMANENT));
	meta.set(meta_name::user, bool(attr & ATTR_USER));
	meta.set(meta_name::system, bool(attr & ATTR_SYSTEM));
	meta.set(meta_name::permissions, std::string{
		(attr & ATTR_READ_PROT) ? '.' : 'r',
		(attr & ATTR_WRITE_PROT) ? '.' : 'w',
		(attr & ATTR_EXECUTE) ? '.' : 'x'
	});
	meta.set(meta_name::max_blocks, used_blocks);
	meta.set(meta_name::length, blocks_to_bytes(used_blocks, dir.blockp->r16l(dir.offset + DIR_LAST_COUNT)));
	meta.set(meta_name::creation_date, make_date(y, m, d));

	return std::make_pair(std::error_condition(), meta);
}

std::error_condition adam_eos_impl::metadata_change(const std::vector<std::string> &path, const meta_data &meta)
{
	if (path.size() != 1)
		return error::unsupported;

	dir_handle dir = find_dir_entry(path.front());
	if (!dir.blockp)
		return error::not_found;
	u8 attr = dir.blockp->r8(dir.offset + DIR_ATTR);

	unsigned name_length;
	if (meta.has(meta_name::name))
	{
		const std::string name = meta.get_string(meta_name::name);
		write_filename(*dir.blockp, dir.offset, name);
		name_length = name.length() + 1;
	}
	else
		name_length = read_name(*dir.blockp, dir.offset).length();
	if (meta.has(meta_name::file_type))
	{
		const std::string filetype = meta.get_string(meta_name::file_type);
		dir.blockp->w8(dir.offset + name_length - 1, util::streqlower(filetype, filetype_stx) ? 0x02 : u8(filetype.front()));
	}
	if (meta.has(meta_name::protect))
	{
		if (meta.get_flag(meta_name::protect))
			attr |= ATTR_PERMANENT;
		else
			attr &= ~ATTR_PERMANENT;
	}
	if (meta.has(meta_name::permissions))
	{
		const std::string str = meta.get_string(meta_name::permissions);
		const bool modify = !str.empty() && (str.front() == '+' || str.front() == '-');
		u8 p = 0;
		if (str.find('r') != std::string::npos)
			p |= ATTR_READ_PROT;
		if (str.find('w') != std::string::npos)
			p |= ATTR_WRITE_PROT;
		if (str.find('x') != std::string::npos)
			p |= ATTR_EXECUTE;
		if (!modify)
			attr |= ATTR_READ_PROT | ATTR_WRITE_PROT | ATTR_EXECUTE;
		if (!modify || str.front() == '+')
			attr &= ~p;
		else
			attr |= p;
	}
	if (meta.has(meta_name::user))
	{
		if (meta.get_flag(meta_name::user))
			attr |= ATTR_USER;
		else
			attr &= ~ATTR_USER;
	}
	if (meta.has(meta_name::system))
	{
		if (meta.get_flag(meta_name::system))
			attr |= ATTR_SYSTEM;
		else
			attr &= ~ATTR_SYSTEM;
	}
	if (meta.has(meta_name::creation_date))
	{
		util::arbitrary_datetime creation_date = meta.get_date(meta_name::creation_date);
		dir.blockp->w8(dir.offset + DIR_YEAR, dec_2_bcd(creation_date.year % 100));
		dir.blockp->w8(dir.offset + DIR_MONTH, dec_2_bcd(creation_date.month));
		dir.blockp->w8(dir.offset + DIR_DAY, dec_2_bcd(creation_date.day_of_month));
	}

	dir.blockp->w8(dir.offset + DIR_ATTR, attr);
	return std::error_condition();
}

std::error_condition adam_eos_impl::rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath)
{
	if (opath.size() != 1 || npath.size() != 1)
		return error::unsupported;

	dir_handle dir = find_dir_entry(opath.front());
	if (!dir.blockp)
		return error::not_found;

	write_filename(*dir.blockp, dir.offset, npath.front());
	return std::error_condition();
}

std::error_condition adam_eos_impl::remove(const std::vector<std::string> &path)
{
	// TODO: verify that this algorithm works properly
	unsigned blk = 1;
	unsigned offset = VOL_DES_LENGTH;

	auto dir_block = m_interleaved.get(blk);

	const unsigned lastblk = 1 + (dir_block->r8(VOL_DIRSIZE) & 0x7f);
	unsigned foundblk = 0;
	u32 endblk = 0;
	while (blk < lastblk)
	{
		u8 attr = dir_block->r8(offset + DIR_ATTR);

		// If we fall into the hole, we have more to do
		if (attr & 0x01)
			break;

		// Skip (already) deleted files
		if (!(attr & ATTR_DELETED))
		{
			if (foundblk == 0 && filename_matches(*dir_block, offset, path.front()))
			{
				// Flag the deletion
				attr |= ATTR_DELETED;
				dir_block->w8(offset + DIR_ATTR, attr);

				// Remember the directory block number
				foundblk = blk;
			}
			else
			{
				// Find the first block past the last used block
				const u32 startblk = dir_block->r32l(offset + DIR_START_BLOCK);
				if (startblk >= endblk)
					endblk = startblk + dir_block->r16l(offset + DIR_MAX_LENGTH);
			}
		}

		offset += DIR_ENT_LENGTH;
		if (offset > 1024 - DIR_ENT_LENGTH)
		{
			if (++blk < lastblk)
				dir_block = m_interleaved.get(blk);
			offset = 0;
		}
	}

	if (foundblk == 0)
		return error::not_found;

	// Add more blocks to the hole if the end has opened up
	const u32 holeblk = blk < lastblk ? 0 : dir_block->r32l(offset + DIR_START_BLOCK);
	if (holeblk > endblk)
	{
		dir_block->w32l(offset + DIR_START_BLOCK, endblk);
		dir_block->w16l(offset + DIR_MAX_LENGTH, dir_block->r16l(offset + DIR_MAX_LENGTH) + holeblk - lastblk);
	}

	return std::error_condition();
}

std::pair<std::error_condition, std::vector<dir_entry>> adam_eos_impl::directory_contents(const std::vector<std::string> &path)
{
	std::vector<dir_entry> results;

	if (!path.empty())
		return std::make_pair(error::unsupported, results);

	unsigned blk = 1;
	unsigned offset = VOL_DES_LENGTH;

	auto block = m_interleaved.get(blk);

	const unsigned lastblk = 1 + (block->r8(VOL_DIRSIZE) & 0x7f);
	while (blk < lastblk)
	{
		const u8 attr = block->r8(offset + DIR_ATTR);

		// Game over if we fall into the hole
		if (attr & ATTR_HOLE)
			break;

		// Skip deleted files
		if (!(attr & ATTR_DELETED))
		{
			const std::string name = read_name(*block, offset);
			if (!name.empty() && is_valid_filetype(name.back()))
			{
				const u16 max_blocks = block->r16l(offset + DIR_MAX_LENGTH);
				const u16 used_blocks = block->r16l(offset + DIR_USED_LENGTH);
				const u8 y = block->r8(offset + DIR_YEAR);
				const u8 m = block->r8(offset + DIR_MONTH);
				const u8 d = block->r8(offset + DIR_DAY);

				meta_data meta;
				meta.set(meta_name::name, name.substr(0, name.length() - 1));
				meta.set(meta_name::file_type, name.back() == 0x02 ? filetype_stx : std::string(1, name.back()));
				meta.set(meta_name::protect, bool(attr & ATTR_PERMANENT));
				meta.set(meta_name::user, bool(attr & ATTR_USER));
				meta.set(meta_name::system, bool(attr & ATTR_SYSTEM));
				meta.set(meta_name::permissions, std::string{
					(attr & ATTR_READ_PROT) ? '.' : 'r',
					(attr & ATTR_WRITE_PROT) ? '.' : 'w',
					(attr & ATTR_EXECUTE) ? '.' : 'x'
				});
				meta.set(meta_name::max_blocks, max_blocks);
				meta.set(meta_name::length, blocks_to_bytes(used_blocks, block->r16l(offset + DIR_LAST_COUNT)));
				meta.set(meta_name::creation_date, make_date(y, m, d));

				results.emplace_back(dir_entry_type::file, meta);
			}
		}

		offset += DIR_ENT_LENGTH;
		if (offset > 1024 - DIR_ENT_LENGTH)
		{
			if (++blk < lastblk)
				block = m_interleaved.get(blk);
			offset = 0;
		}
	}

	return std::make_pair(std::error_condition(), results);
}

std::pair<std::error_condition, std::vector<u8>> adam_eos_impl::file_read(const std::vector<std::string> &path)
{
	if (path.size() != 1)
		return std::make_pair(error::unsupported, std::vector<u8>());

	const dir_handle dir = find_dir_entry(path.front());
	if (!dir.blockp)
		return std::make_pair(error::not_found, std::vector<u8>());

	u16 blocks_left = dir.blockp->r16l(dir.offset + DIR_USED_LENGTH);
	const u16 last_count = std::min<u16>(dir.blockp->r16l(dir.offset + DIR_LAST_COUNT), 1024);

	const u32 lastblk = dir.blockp->r32l(dir.offset + DIR_START_BLOCK) + blocks_left - 1;
	if (lastblk >= m_blockdev.block_count() * 2)
		return std::make_pair(error::invalid_block, std::vector<u8>());

	std::vector<u8> data;
	while (blocks_left-- != 0)
	{
		const u16 size = blocks_left ? 1024 : last_count;
		if (size)
		{
			data.resize(data.size() + size);
			m_interleaved.get(lastblk - blocks_left)->read(0, &*(data.end() - size), size);
		}
	}
	return std::make_pair(std::error_condition(), data);
}

std::tuple<std::error_condition, std::vector<u32>, std::vector<u32>> adam_eos_impl::enum_blocks(const std::vector<std::string> &path)
{
	if (path.empty())
	{
		unsigned blk = 1;
		unsigned offset = VOL_DES_LENGTH;

		auto dir_block = m_interleaved.get(blk);

		const unsigned lastblk = 1 + (dir_block->r8(VOL_DIRSIZE) & 0x7f);
		while (blk < lastblk)
		{
			const u8 attr = dir_block->r8(offset + DIR_ATTR);

			if (attr & ATTR_HOLE)
				break;

			offset += DIR_ENT_LENGTH;
			if (offset > 1024 - DIR_ENT_LENGTH)
			{
				if (++blk < lastblk)
					dir_block = m_interleaved.get(blk);
				offset = 0;
			}
		}

		std::vector<u32> blocks(blk);
		std::iota(blocks.begin(), blocks.end(), 1);
		return std::make_tuple(std::error_condition(), std::vector<u32>(), std::move(blocks));
	}

	if (path.size() != 1)
		return std::make_tuple(error::unsupported, std::vector<u32>(), std::vector<u32>());

	const dir_handle dir = find_dir_entry(path.front());
	if (!dir.blockp)
		return std::make_tuple(error::not_found, std::vector<u32>(), std::vector<u32>());

	const u16 numblocks = dir.blockp->r16l(dir.offset + DIR_USED_LENGTH);
	const u32 firstblk = dir.blockp->r32l(dir.offset + DIR_START_BLOCK);

	std::vector<u32> blocks(numblocks);
	std::iota(blocks.begin(), blocks.end(), firstblk);
	if (numblocks > m_blockdev.block_count() * 2 - firstblk)
		return std::make_tuple(error::invalid_block, std::vector<u32>(), std::move(blocks));
	else
		return std::make_tuple(std::error_condition(), std::vector<u32>(), std::move(blocks));
}

std::error_condition adam_eos_impl::file_create(const std::vector<std::string> &path, const meta_data &meta)
{
	// TODO
	return error::unsupported;
}

std::error_condition adam_eos_impl::file_write(const std::vector<std::string> &path, const std::vector<u8> &data)
{
	// TODO
	return error::unsupported;
}

} // anonymous namespace


const char *fs::adam_eos_image::name() const
{
	return "adam_eos";
}

const char *fs::adam_eos_image::description() const
{
	return "Adam/EOS";
}

void fs::adam_eos_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add(FLOPPY_ADAM_FORMAT, floppy_image::FF_525, floppy_image::SSDD, 163840, "adam_eos_ssdd", "Adam/EOS single-sided double density (5.25\" 160K)");
	fe.add(FLOPPY_ADAM_FORMAT, floppy_image::FF_525, floppy_image::DSDD, 327680, "adam_eos_dsdd", "Adam/EOS double-sided double density (5.25\" 320K)");
	fe.add(FLOPPY_ADAM_FORMAT, floppy_image::FF_35, floppy_image::DSDD, 737280, "adam_eos_35dd", "Adam/EOS double-sided double density (3.5\" 720K)");
	fe.add(FLOPPY_ADAM_FORMAT, floppy_image::FF_35, floppy_image::DSHD, 1474560, "adam_eos_35hd", "Adam/EOS double-sided high density (3.5\" 1.44M)");
}

bool fs::adam_eos_image::can_format() const
{
	return true;
}

bool fs::adam_eos_image::can_read() const
{
	return true;
}

bool fs::adam_eos_image::can_write() const
{
	return false; // TODO
}

bool fs::adam_eos_image::has_rsrc() const
{
	return false;
}

std::vector<fs::meta_description> fs::adam_eos_image::volume_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "", false, [](const meta_value &m) { return adam_eos_impl::validate_filename(m.as_string(), false); }, "Volume name, up to 12 characters");
	results.emplace_back(meta_name::protect, false, false, nullptr, "Delete protection");
	results.emplace_back(meta_name::size_in_blocks, 0, true, nullptr, "Total number of blocks");

	const auto now = util::arbitrary_datetime::now();
	results.emplace_back(meta_name::creation_date, now, false, nullptr, "Creation date");

	return results;
}

std::vector<fs::meta_description> fs::adam_eos_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_name::name, "", false, [](const meta_value &m) { return adam_eos_impl::validate_filename(m.as_string(), true); }, "File name, up to 11 characters");
	results.emplace_back(meta_name::file_type, "A", false, [](const meta_value &m) { return adam_eos_impl::validate_filetype(m.as_string()); }, "Type of the file");
	results.emplace_back(meta_name::protect, false, false, nullptr, "Permanently protected against update");
	results.emplace_back(meta_name::user, true, false, nullptr, "User file");
	results.emplace_back(meta_name::system, false, false, nullptr, "System file");
	results.emplace_back(meta_name::permissions, "rwx", false, [](const meta_value &m) { return adam_eos_impl::validate_permissions(m.as_string()); }, "Permitted modes (r/w/x)");
	results.emplace_back(meta_name::max_blocks, 0, true, nullptr, "Maximum size of the file in blocks");
	results.emplace_back(meta_name::length, 0, true, nullptr, "Size of the file in bytes");

	const auto now = util::arbitrary_datetime::now();
	results.emplace_back(meta_name::creation_date, now, false, nullptr, "Creation date");

	return results;
}

std::unique_ptr<fs::filesystem_t> fs::adam_eos_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<adam_eos_impl>(blockdev);
}
