// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy, hd and cdrom images

#ifndef MAME_FORMATS_FSMGR_H
#define MAME_FORMATS_FSMGR_H

#pragma once

#include "flopimg.h"
#include "fsmeta.h"

namespace fs {

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

enum err_t {
	ERR_OK = 0,
	ERR_UNSUPPORTED,
	ERR_INVALID,
	ERR_NOT_FOUND,
	ERR_NOT_EMPTY,
	ERR_NO_SPACE,
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
		void wstr(u32 offset, const std::string &str);
		void w8(  u32 offset, u8 data);
		void w16b(u32 offset, u16 data);
		void w24b(u32 offset, u32 data);
		void w32b(u32 offset, u32 data);
		void w16l(u32 offset, u16 data);
		void w24l(u32 offset, u32 data);
		void w32l(u32 offset, u32 data);

		std::string rstr(u32 offset, u32 size) const;
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
	virtual err_t volume_metadata_change(const meta_data &meta);

	// Get the metadata for a file or a directory.  Empty path targets the root directory
	virtual std::pair<err_t, meta_data> metadata(const std::vector<std::string> &path);

	// Change the metadata for a file or a directory.  Empty path targets the root directory
	virtual err_t metadata_change(const std::vector<std::string> &path, const meta_data &meta);

	// Get the contents of a directory, empty path targets the root directory
	virtual std::pair<err_t, std::vector<dir_entry>> directory_contents(const std::vector<std::string> &path);

	// Rename a file or a directory.  In contrast to metadata_change, this can move the object
	// between directories
	virtual err_t rename(const std::vector<std::string> &opath, const std::vector<std::string> &npath);

	// Remove a file or a directory.  Directories must be empty (e.g. it's not recursive)
	virtual err_t remove(const std::vector<std::string> &path);

	// Create a directory, path designates where the directory must be, directory name is in meta
	virtual err_t dir_create(const std::vector<std::string> &path, const meta_data &meta);

	// Create an empty file, path designates where the file must be, file name is in meta
	virtual err_t file_create(const std::vector<std::string> &path, const meta_data &meta);

	// Read the contents of a file
	virtual std::pair<err_t, std::vector<u8>> file_read(const std::vector<std::string> &path);

	// Replace the contents of a file, the file must already exist
	virtual err_t file_write(const std::vector<std::string> &path, const std::vector<u8> &data);

	// Read the resource fork of a file on systems that handle those
	virtual std::pair<err_t, std::vector<u8>> file_rsrc_read(const std::vector<std::string> &path);

	// Replace the resource fork of a file, the file must already exist
	virtual err_t file_rsrc_write(const std::vector<std::string> &path, const std::vector<u8> &data);

	// Format an image, provide the volume metadata
	virtual err_t format(const meta_data &meta);

	static void copy(u8 *p, const u8 *src, u32 size);
	static void fill(u8 *p, u8 data, u32 size);
	static void wstr(u8 *p, const std::string &str);
	static void w8(  u8 *p, u8 data);
	static void w16b(u8 *p, u16 data);
	static void w24b(u8 *p, u32 data);
	static void w32b(u8 *p, u32 data);
	static void w16l(u8 *p, u16 data);
	static void w24l(u8 *p, u32 data);
	static void w32l(u8 *p, u32 data);

	static std::string rstr(const u8 *p, u32 size);
	static u8  r8(  const u8 *p);
	static u16 r16b(const u8 *p);
	static u32 r24b(const u8 *p);
	static u32 r32b(const u8 *p);
	static u16 r16l(const u8 *p);
	static u32 r24l(const u8 *p);
	static u32 r32l(const u8 *p);

protected:
	filesystem_t(fsblk_t &blockdev, u32 size) : m_blockdev(blockdev) {
		m_blockdev.set_block_size(size);
	}

	static std::string trim_end_spaces(const std::string &str);

	fsblk_t &m_blockdev;
};

class unformatted_floppy_creator;

class manager_t {
public:
	struct floppy_enumerator {
		virtual ~floppy_enumerator() = default;

		virtual void add(const floppy_image_format_t &type, u32 image_size, const char *name, const char *description) = 0;
		virtual void add_raw(const char *name, u32 key, const char *description) = 0;
	};

	struct hd_enumerator {
		virtual ~hd_enumerator() = default;

		virtual void add(const manager_t *manager, const char *name, const char *description) = 0;
	};

	struct cdrom_enumerator {
		virtual ~cdrom_enumerator() = default;

		virtual void add(const manager_t *manager, const char *name, const char *description) = 0;
	};


	virtual ~manager_t() = default;

	virtual const char *name() const = 0;
	virtual const char *description() const = 0;

	virtual void enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const;
	virtual void enumerate_h(hd_enumerator &he) const;
	virtual void enumerate_c(cdrom_enumerator &ce) const;

	virtual bool can_format() const = 0;
	virtual bool can_read() const = 0;
	virtual bool can_write() const = 0;
	virtual bool has_rsrc() const = 0;
	virtual char directory_separator() const;

	bool has_subdirectories() const { return directory_separator() != 0; }

	virtual std::vector<meta_description> volume_meta_description() const;
	virtual std::vector<meta_description> file_meta_description() const;
	virtual std::vector<meta_description> directory_meta_description() const;

	// Create a filesystem object from a block device
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const = 0;

protected:
	manager_t() = default;

	static bool has(u32 form_factor, const std::vector<u32> &variants, u32 ff, u32 variant);
	static bool has_variant(const std::vector<u32> &variants, u32 variant);
};

} // namespace fs

#endif
