// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy, hd and cdrom images

#ifndef MAME_FORMATS_FSMGR_H
#define MAME_FORMATS_FSMGR_H

#pragma once

#include "flopimg.h"

class fsblk_t {
protected:
	class iblock_t {
	public:
		iblock_t(uint32_t size) : m_ref(0), m_weak_ref(0), m_size(size) {}
		virtual ~iblock_t() = default;

		void ref();
		void ref_weak();
		void unref();
		void unref_weak();

		uint32_t size() const { return m_size; }

		virtual void drop_weak_references() = 0;

		virtual const uint8_t *rodata() = 0;
		virtual uint8_t *data() = 0;
		uint8_t *offset(const char *function, uint32_t off, uint32_t size);

	protected:
		uint32_t m_ref, m_weak_ref;
		uint32_t m_size;
	};


public:
	class block_t {
	public:
		block_t(iblock_t *block, bool weak = false) : m_block(block), m_is_weak_ref(weak) {
			ref();
		}

		block_t(const block_t &cref) {
			m_block = cref.m_block;
			m_is_weak_ref = cref.m_is_weak_ref;
			ref();
		}

		block_t(block_t &&cref) {
			m_block = cref.m_block;
			m_is_weak_ref = cref.m_is_weak_ref;
			cref.m_block = nullptr;
		}

		~block_t() {
			unref();
		}

		block_t &operator =(const block_t &cref) {
			m_block = cref.m_block;
			m_is_weak_ref = cref.m_is_weak_ref;
			ref();
			return *this;
		}

		block_t &operator =(block_t &&cref) {
			m_block = cref.m_block;
			m_is_weak_ref = cref.m_is_weak_ref;
			cref.m_block = nullptr;
			return *this;
		}

		const uint8_t *rodata() {
			return m_block ? m_block->rodata() : nullptr;
		}

		uint8_t *data() {
			return m_block ? m_block->data() : nullptr;
		}

		void copy(uint32_t offset, const uint8_t *src, uint32_t size);
		void fill(            uint8_t data);
		void fill(uint32_t offset, uint8_t data, uint32_t size);
		void wstr(uint32_t offset, const std::string &str);
		void w8(  uint32_t offset, uint8_t data);
		void w16b(uint32_t offset, u16 data);
		void w32b(uint32_t offset, uint32_t data);
		void w16l(uint32_t offset, u16 data);
		void w32l(uint32_t offset, uint32_t data);

	private:
		iblock_t *m_block;
		bool m_is_weak_ref;

		void ref() {
			if(m_block) {
				if(m_is_weak_ref)
					m_block->ref_weak();
				else
					m_block->ref();
			}
		}

		void unref() {
			if(m_block) {
				if(m_is_weak_ref)
					m_block->unref_weak();
				else
					m_block->unref();
			}
		}
	};

	fsblk_t() = default;
	virtual ~fsblk_t() = default;
	
	virtual void set_block_size(uint32_t block_size);
	virtual uint32_t block_count() const = 0;
	virtual block_t get(uint32_t id) = 0;
	virtual void fill(uint8_t data) = 0;

protected:
	uint32_t m_block_size;
};


class filesystem_t {
protected:
	class idir_t {
	public:
		virtual ~idir_t();

		void ref();
		void ref_weak();
		void unref();
		void unref_weak();

		virtual void drop_weak_references();
	};

	class ifile_t {
	public:
		virtual ~ifile_t();

		void ref();
		void ref_weak();
		void unref();
		void unref_weak();

		virtual void drop_weak_references();
	};

public:
	class dir_t {
	public:
		dir_t(idir_t *dir, bool weak = false) : m_dir(dir), m_is_weak_ref(weak) {
			ref();
		}

		dir_t(const dir_t &cref) {
			m_dir = cref.m_dir;
			m_is_weak_ref = cref.m_is_weak_ref;
			ref();
		}

		dir_t(dir_t &&cref) {
			m_dir = cref.m_dir;
			m_is_weak_ref = cref.m_is_weak_ref;
			cref.m_dir = nullptr;
		}

		~dir_t() {
			unref();
		}

		dir_t &operator =(const dir_t &cref) {
			m_dir = cref.m_dir;
			m_is_weak_ref = cref.m_is_weak_ref;
			ref();
			return *this;
		}

		dir_t &operator =(dir_t &&cref) {
			m_dir = cref.m_dir;
			m_is_weak_ref = cref.m_is_weak_ref;
			cref.m_dir = nullptr;
			return *this;
		}

	private:
		idir_t *m_dir;
		bool m_is_weak_ref;

		void ref() {
			if(m_dir) {
				if(m_is_weak_ref)
					m_dir->ref_weak();
				else
					m_dir->ref();
			}
		}

		void unref() {
			if(m_dir) {
				if(m_is_weak_ref)
					m_dir->unref_weak();
				else
					m_dir->unref();
			}
		}
	};

	class file_t {
	public:
		file_t(ifile_t *file, bool weak = false) : m_file(file), m_is_weak_ref(weak) {
			ref();
		}

		file_t(const file_t &cref) {
			m_file = cref.m_file;
			m_is_weak_ref = cref.m_is_weak_ref;
			ref();
		}

		file_t(file_t &&cref) {
			m_file = cref.m_file;
			m_is_weak_ref = cref.m_is_weak_ref;
			cref.m_file = nullptr;
		}

		~file_t() {
			unref();
		}

		file_t &operator =(const file_t &cref) {
			m_file = cref.m_file;
			m_is_weak_ref = cref.m_is_weak_ref;
			ref();
			return *this;
		}

		file_t &operator =(file_t &&cref) {
			m_file = cref.m_file;
			m_is_weak_ref = cref.m_is_weak_ref;
			cref.m_file = nullptr;
			return *this;
		}

	private:
		ifile_t *m_file;
		bool m_is_weak_ref;

		void ref() {
			if(m_file) {
				if(m_is_weak_ref)
					m_file->ref_weak();
				else
					m_file->ref();
			}
		}

		void unref() {
			if(m_file) {
				if(m_is_weak_ref)
					m_file->unref_weak();
				else
					m_file->unref();
			}
		}
	};

	filesystem_t(fsblk_t &blockdev) : m_blockdev(blockdev) {}

	virtual ~filesystem_t() = default;

	virtual void format();
	virtual dir_t root();


protected:
	fsblk_t &m_blockdev;
};

class unformatted_floppy_creator;
	
class filesystem_manager_t {
public:
	struct floppy_enumerator {
		virtual ~floppy_enumerator() = default;

		virtual void add(const filesystem_manager_t *manager, floppy_format_type type, uint32_t image_size, const char *name, const char *description) = 0;
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

	virtual void enumerate(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const;
	virtual void enumerate(hd_enumerator &he) const;
	virtual void enumerate(cdrom_enumerator &ce) const;

	virtual bool can_format() const = 0;
	virtual bool can_read() const = 0;
	virtual bool can_write() const = 0;

	// Create a filesystem object from a block device
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const = 0;

protected:
	filesystem_manager_t() = default;

	static bool has(uint32_t form_factor, const std::vector<uint32_t> &variants, uint32_t ff, uint32_t variant);
	static bool has_variant(const std::vector<uint32_t> &variants, uint32_t variant);
};


typedef filesystem_manager_t *(*filesystem_manager_type)();

// this template function creates a stub which constructs a filesystem manager
template<class _FormatClass>
filesystem_manager_t *filesystem_manager_creator()
{
	return new _FormatClass();
}

#endif
