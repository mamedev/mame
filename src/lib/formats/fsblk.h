// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem operations on mounted image blocks

#ifndef MAME_FORMATS_FSBLK_H
#define MAME_FORMATS_FSBLK_H

#pragma once

#include "fsmeta.h"

#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace fs {

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

enum class error : int {
	unsupported = 1,
	not_found,
	no_space,
	invalid_block,
	invalid_name,
	incorrect_size,
	already_exists,
};

template<typename T> class refcounted_outer {
public:
	refcounted_outer(bool weak) :  m_object(nullptr), m_is_weak_ref(weak) {}
	refcounted_outer(T *object, bool weak) : m_object(object), m_is_weak_ref(weak) {
		ref();
	}

	refcounted_outer(const refcounted_outer &cref) {
		m_object = cref.m_object;
		m_is_weak_ref = cref.m_is_weak_ref;
		ref();
	}

	refcounted_outer(refcounted_outer &&cref) {
		m_object = cref.m_object;
		m_is_weak_ref = cref.m_is_weak_ref;
		cref.m_object = nullptr;
	}

	~refcounted_outer() {
		unref();
	}

	refcounted_outer<T> &operator =(T *dir) {
		if(m_object != dir) {
			unref();
			m_object = dir;
			ref();
		}
		return *this;
	}

	refcounted_outer<T> &operator =(const refcounted_outer<T> &cref) {
		if(m_object != cref.m_object) {
			unref();
			m_object = cref.m_object;
			ref();
		}
		return *this;
	}

	refcounted_outer<T> &operator =(refcounted_outer<T> &&cref) {
		if(m_object != cref.m_object) {
			unref();
			m_object = cref.m_object;
			ref();
		} else if(m_is_weak_ref != cref.m_is_weak_ref) {
			ref();
			cref.unref();
			m_object = cref.m_object; // In case the object got deleted (when going from strong ref to weak on the last strong)
		}
		cref.m_object = nullptr;
		return *this;
	}

	operator bool() const { return m_object != nullptr; }

protected:
	T *m_object;
	bool m_is_weak_ref;

private:
	void ref() {
		if(m_object) {
			if(m_is_weak_ref)
				m_object->ref_weak();
			else
				m_object->ref();
		}
	}

	void unref() {
		if(m_object) {
			bool del = m_is_weak_ref ? m_object->unref_weak() : m_object->unref();
			if(del)
				m_object = nullptr;
		}
	}
};


class refcounted_inner {
public:
	refcounted_inner() : m_ref(0), m_weak_ref(0) {}
	virtual ~refcounted_inner() = default;

	void ref();
	void ref_weak();
	bool unref();
	bool unref_weak();

	virtual void drop_weak_references() = 0;

public:
	u32 m_ref, m_weak_ref;
};

enum class dir_entry_type {
	dir,
	file,
};

struct dir_entry {
	std::string m_name;
	dir_entry_type m_type;
	meta_data m_meta;

	dir_entry(dir_entry_type type, const meta_data &meta) : m_name(meta.get_string(meta_name::name)), m_type(type), m_meta(std::move(meta)) {}
};

class fsblk_t {
protected:
	class iblock_t : public refcounted_inner {
	public:
		iblock_t(u32 size) : refcounted_inner(), m_size(size) {}
		virtual ~iblock_t() = default;

		u32 size() const { return m_size; }

		virtual const u8 *rodata() = 0;
		virtual u8 *data() = 0;
		u8 *offset(const char *function, u32 off, u32 size);
		const u8 *rooffset(const char *function, u32 off, u32 size);

	protected:
		u32 m_size;
	};


public:
	class block_t : public refcounted_outer<iblock_t> {
	public:
		block_t(bool weak = false) :  refcounted_outer<iblock_t>(weak) {}
		block_t(iblock_t *block, bool weak = true) : refcounted_outer(block, weak) {}
		virtual ~block_t() = default;

		block_t strong() { return block_t(m_object, false); }
		block_t weak() { return block_t(m_object, true); }

		u32 size() const { return m_object->size(); }

		const u8 *rodata() const { return m_object->rodata(); }
		u8 *data() { return m_object->data(); }

		void copy(u32 offset, const u8 *src, u32 size);
		void fill(            u8 data);
		void fill(u32 offset, u8 data, u32 size);
		void wstr(u32 offset, std::string_view str);
		void w8(  u32 offset, u8 data);
		void w16b(u32 offset, u16 data);
		void w24b(u32 offset, u32 data);
		void w32b(u32 offset, u32 data);
		void w16l(u32 offset, u16 data);
		void w24l(u32 offset, u32 data);
		void w32l(u32 offset, u32 data);

		std::string_view rstr(u32 offset, u32 size) const;
		u8  r8(  u32 offset) const;
		u16 r16b(u32 offset) const;
		u32 r24b(u32 offset) const;
		u32 r32b(u32 offset) const;
		u16 r16l(u32 offset) const;
		u32 r24l(u32 offset) const;
		u32 r32l(u32 offset) const;
	};

	fsblk_t() : m_block_size(0) {}
	virtual ~fsblk_t() = default;

	virtual void set_block_size(u32 block_size);
	virtual u32 block_count() const = 0;
	virtual block_t get(u32 id) = 0;
	virtual void fill(u8 data) = 0;

protected:
	u32 m_block_size;
};


class filesystem_t {
public:
	virtual ~filesystem_t() = default;

	// Get the metadata for the volume
	virtual meta_data volume_metadata();

	// Change the metadata for the volume
	virtual std::error_condition volume_metadata_change(const meta_data &meta);

	// Get the metadata for a file or a directory.  Empty path targets the root directory
	virtual std::pair<std::error_condition, meta_data> metadata(const std::vector<std::string> &path);

	// Change the metadata for a file or a directory.  Empty path targets the root directory
	virtual std::error_condition metadata_change(const std::vector<std::string> &path, const meta_data &meta);

	// Get the contents of a directory, empty path targets the root directory
	virtual std::pair<std::error_condition, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path);

	// Rename a file or a directory.  In contrast to metadata_change, this can move the object
	// between directories
	virtual std::error_condition rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath);

	// Remove a file or a directory.  Directories must be empty (e.g. it's not recursive)
	virtual std::error_condition remove(const std::vector<std::string> &path);

	// Create a directory, path designates where the directory must be, directory name is in meta
	virtual std::error_condition dir_create(const std::vector<std::string> &path, const meta_data &meta);

	// Create an empty file, path designates where the file must be, file name is in meta
	virtual std::error_condition file_create(const std::vector<std::string> &path, const meta_data &meta);

	// Read the contents of a file
	virtual std::pair<std::error_condition, std::vector<u8>> file_read(const std::vector<std::string> &path);

	// Replace the contents of a file, the file must already exist
	virtual std::error_condition file_write(const std::vector<std::string> &path, const std::vector<u8> &data);

	// Read the resource fork of a file on systems that handle those
	virtual std::pair<std::error_condition, std::vector<u8>> file_rsrc_read(const std::vector<std::string> &path);

	// Replace the resource fork of a file, the file must already exist
	virtual std::error_condition file_rsrc_write(const std::vector<std::string> &path, const std::vector<u8> &data);

	// Format an image, provide the volume metadata
	virtual std::error_condition format(const meta_data &meta);

	static void wstr(u8 *p, std::string_view str);

	static std::string_view rstr(const u8 *p, u32 size);

	static std::string_view trim_end_spaces(std::string_view str);

protected:
	filesystem_t(fsblk_t &blockdev, u32 size) : m_blockdev(blockdev) {
		m_blockdev.set_block_size(size);
	}

	fsblk_t &m_blockdev;
};

// error category for filesystem errors
std::error_category const &fs_category() noexcept;
inline std::error_condition make_error_condition(error err) noexcept { return std::error_condition(int(err), fs_category()); }

} // namespace fs


namespace std {

template <> struct is_error_condition_enum<fs::error> : public std::true_type { };

} // namespace std

#endif
