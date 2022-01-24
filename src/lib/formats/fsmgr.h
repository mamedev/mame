// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy, hd and cdrom images

#ifndef MAME_FORMATS_FSMGR_H
#define MAME_FORMATS_FSMGR_H

#pragma once

#include "flopimg.h"
#include "fsmeta.h"

template<typename T> class fs_refcounted_outer {
public:
	fs_refcounted_outer(bool weak) :  m_object(nullptr), m_is_weak_ref(weak) {}
	fs_refcounted_outer(T *object, bool weak) : m_object(object), m_is_weak_ref(weak) {
		ref();
	}

	fs_refcounted_outer(const fs_refcounted_outer &cref) {
		m_object = cref.m_object;
		m_is_weak_ref = cref.m_is_weak_ref;
		ref();
	}

	fs_refcounted_outer(fs_refcounted_outer &&cref) {
		m_object = cref.m_object;
		m_is_weak_ref = cref.m_is_weak_ref;
		cref.m_object = nullptr;
	}

	~fs_refcounted_outer() {
		unref();
	}

	fs_refcounted_outer<T> &operator =(T *dir) {
		if(m_object != dir) {
			unref();
			m_object = dir;
			ref();
		}
		return *this;
	}

	fs_refcounted_outer<T> &operator =(const fs_refcounted_outer<T> &cref) {
		if(m_object != cref.m_object) {
			unref();
			m_object = cref.m_object;
			ref();
		}
		return *this;
	}

	fs_refcounted_outer<T> &operator =(fs_refcounted_outer<T> &&cref) {
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


class fs_refcounted_inner {
public:
	fs_refcounted_inner() : m_ref(0), m_weak_ref(0) {}
	virtual ~fs_refcounted_inner() = default;

	void ref();
	void ref_weak();
	bool unref();
	bool unref_weak();

	virtual void drop_weak_references() = 0;

public:
	uint32_t m_ref, m_weak_ref;
};

enum class fs_dir_entry_type {
	dir,
	file,
	system_file,
};

struct fs_dir_entry {
	std::string m_name;
	fs_dir_entry_type m_type;
	uint64_t m_key;

	fs_dir_entry(const std::string &name, fs_dir_entry_type type, uint64_t key) : m_name(name), m_type(type), m_key(key) {}
};

class fsblk_t {
protected:
	class iblock_t : public fs_refcounted_inner {
	public:
		iblock_t(uint32_t size) : fs_refcounted_inner(), m_size(size) {}
		virtual ~iblock_t() = default;

		uint32_t size() const { return m_size; }

		virtual const uint8_t *rodata() = 0;
		virtual uint8_t *data() = 0;
		uint8_t *offset(const char *function, uint32_t off, uint32_t size);
		const uint8_t *rooffset(const char *function, uint32_t off, uint32_t size);

	protected:
		uint32_t m_size;
	};


public:
	class block_t : public fs_refcounted_outer<iblock_t> {
	public:
		block_t(bool weak = false) :  fs_refcounted_outer<iblock_t>(weak) {}
		block_t(iblock_t *block, bool weak = true) : fs_refcounted_outer(block, weak) {}
		virtual ~block_t() = default;

		block_t strong() { return block_t(m_object, false); }
		block_t weak() { return block_t(m_object, true); }

		const uint8_t *rodata() { return m_object->rodata(); }
		uint8_t *data() { return m_object->data(); }

		void copy(uint32_t offset, const uint8_t *src, uint32_t size);
		void fill(                 uint8_t data);
		void fill(uint32_t offset, uint8_t data, uint32_t size);
		void wstr(uint32_t offset, const std::string &str);
		void w8(  uint32_t offset, uint8_t data);
		void w16b(uint32_t offset, uint16_t data);
		void w24b(uint32_t offset, uint32_t data);
		void w32b(uint32_t offset, uint32_t data);
		void w16l(uint32_t offset, uint16_t data);
		void w24l(uint32_t offset, uint32_t data);
		void w32l(uint32_t offset, uint32_t data);

		std::string rstr(uint32_t offset, uint32_t size);
		uint8_t  r8(  uint32_t offset);
		uint16_t r16b(uint32_t offset);
		uint32_t r24b(uint32_t offset);
		uint32_t r32b(uint32_t offset);
		uint16_t r16l(uint32_t offset);
		uint32_t r24l(uint32_t offset);
		uint32_t r32l(uint32_t offset);
	};

	fsblk_t() : m_block_size(0) {}
	virtual ~fsblk_t() = default;

	virtual void set_block_size(uint32_t block_size);
	virtual uint32_t block_count() const = 0;
	virtual block_t get(uint32_t id) = 0;
	virtual void fill(uint8_t data) = 0;

protected:
	uint32_t m_block_size;
};


class filesystem_t {
public:
	class dir_t;
	class file_t;

protected:
	class idir_t : public fs_refcounted_inner {
	public:
		idir_t() : fs_refcounted_inner() {}
		virtual ~idir_t() = default;

		virtual fs_meta_data metadata() = 0;
		virtual void metadata_change(const fs_meta_data &info);
		virtual std::vector<fs_dir_entry> contents() = 0;
		virtual file_t file_get(uint64_t key) = 0;
		virtual dir_t dir_get(uint64_t key) = 0;
		virtual file_t file_create(const fs_meta_data &info);
		virtual void file_delete(uint64_t key);
	};

	class ifile_t : public fs_refcounted_inner {
	public:
		ifile_t() : fs_refcounted_inner() {}
		virtual ~ifile_t() = default;

		virtual fs_meta_data metadata() = 0;
		virtual void metadata_change(const fs_meta_data &info);
		virtual std::vector<uint8_t> read_all() = 0;
		virtual void replace(const std::vector<uint8_t> &data);
		virtual std::vector<uint8_t> rsrc_read_all();
		virtual void rsrc_replace(const std::vector<uint8_t> &data);
	};

public:
	class dir_t : public fs_refcounted_outer<idir_t> {
	public:
		dir_t(bool weak = false) :  fs_refcounted_outer<idir_t>(weak) {}
		dir_t(idir_t *dir, bool weak = true) : fs_refcounted_outer(dir, weak) {}
		virtual ~dir_t() = default;

		dir_t strong() { return dir_t(m_object, false); }
		dir_t weak() { return dir_t(m_object, true); }

		fs_meta_data metadata() { return m_object->metadata(); }
		void metadata_change(const fs_meta_data &info) { m_object->metadata_change(info); }
		std::vector<fs_dir_entry> contents() { return m_object->contents(); }
		file_t file_get(uint64_t key) { return m_object->file_get(key); }
		dir_t dir_get(uint64_t key)  { return m_object->dir_get(key); }
		file_t file_create(const fs_meta_data &info) { return m_object->file_create(info); }
		void file_delete(uint64_t key) { m_object->file_delete(key); }
	};

	class file_t : public fs_refcounted_outer<ifile_t> {
	public:
		file_t(bool weak = false) : fs_refcounted_outer<ifile_t>(weak) {}
		file_t(ifile_t *file, bool weak = true) : fs_refcounted_outer(file, weak) {}
		virtual ~file_t() = default;

		file_t strong() { return file_t(m_object, false); }
		file_t weak() { return file_t(m_object, true); }

		fs_meta_data metadata() { return m_object->metadata(); }
		void metadata_change(const fs_meta_data &info) { m_object->metadata_change(info); }
		std::vector<uint8_t> read_all() { return m_object->read_all(); }
		void replace(const std::vector<uint8_t> &data) { m_object->replace(data); }
		std::vector<uint8_t> rsrc_read_all() { return m_object->rsrc_read_all(); }
		void rsrc_replace(const std::vector<uint8_t> &data) { m_object->rsrc_replace(data); }
	};

	filesystem_t(fsblk_t &blockdev, uint32_t size) : m_blockdev(blockdev) {
		m_blockdev.set_block_size(size);
	}

	virtual ~filesystem_t() = default;

	virtual dir_t root();
	virtual void format(const fs_meta_data &meta);
	virtual fs_meta_data metadata();
	virtual void  metadata_change(const fs_meta_data &info);

	static void copy(uint8_t *p, const uint8_t *src, uint32_t size);
	static void fill(uint8_t *p, uint8_t data, uint32_t size);
	static void wstr(uint8_t *p, const std::string &str);
	static void w8(  uint8_t *p, uint8_t data);
	static void w16b(uint8_t *p, uint16_t data);
	static void w24b(uint8_t *p, uint32_t data);
	static void w32b(uint8_t *p, uint32_t data);
	static void w16l(uint8_t *p, uint16_t data);
	static void w24l(uint8_t *p, uint32_t data);
	static void w32l(uint8_t *p, uint32_t data);

	static std::string rstr(const uint8_t *p, uint32_t size);
	static uint8_t  r8(  const uint8_t *p);
	static uint16_t r16b(const uint8_t *p);
	static uint32_t r24b(const uint8_t *p);
	static uint32_t r32b(const uint8_t *p);
	static uint16_t r16l(const uint8_t *p);
	static uint32_t r24l(const uint8_t *p);
	static uint32_t r32l(const uint8_t *p);

	static std::string trim_end_spaces(const std::string &str);

protected:
	fsblk_t &m_blockdev;
};

class unformatted_floppy_creator;

class filesystem_manager_t {
public:
	struct floppy_enumerator {
		virtual ~floppy_enumerator() = default;

		virtual void add(floppy_format_type type, uint32_t image_size, const char *name, const char *description) = 0;
		virtual void add_raw(const char *name, uint32_t key, const char *description) = 0;
	};

	struct hd_enumerator {
		virtual ~hd_enumerator() = default;

		virtual void add(const filesystem_manager_t *manager, const char *name, const char *description) = 0;
	};

	struct cdrom_enumerator {
		virtual ~cdrom_enumerator() = default;

		virtual void add(const filesystem_manager_t *manager, const char *name, const char *description) = 0;
	};


	virtual ~filesystem_manager_t() = default;

	virtual const char *name() const = 0;
	virtual const char *description() const = 0;

	virtual void enumerate_f(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const;
	virtual void enumerate_h(hd_enumerator &he) const;
	virtual void enumerate_c(cdrom_enumerator &ce) const;

	virtual bool can_format() const = 0;
	virtual bool can_read() const = 0;
	virtual bool can_write() const = 0;
	virtual bool has_rsrc() const = 0;
	virtual char directory_separator() const;

	bool has_subdirectories() const { return directory_separator() != 0; }

	virtual std::vector<fs_meta_description> volume_meta_description() const;
	virtual std::vector<fs_meta_description> file_meta_description() const;
	virtual std::vector<fs_meta_description> directory_meta_description() const;

	// Create a filesystem object from a block device
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const = 0;

protected:
	filesystem_manager_t() = default;

	static bool has(uint32_t form_factor, const std::vector<uint32_t> &variants, uint32_t ff, uint32_t variant);
	static bool has_variant(const std::vector<uint32_t> &variants, uint32_t variant);
};

#endif
