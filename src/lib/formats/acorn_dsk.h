// license:BSD-3-Clause
// copyright-holders:Dirk Best, Nigel Barnes
/***************************************************************************

    Acorn - BBC Micro, Electron, Archimedes

    Disk image formats

***************************************************************************/
#ifndef MAME_FORMATS_ACORN_DSK_H
#define MAME_FORMATS_ACORN_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"

class acorn_ssd_format : public wd177x_format
{
public:
	acorn_ssd_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class acorn_dsd_format : public wd177x_format
{
public:
	acorn_dsd_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class opus_ddos_format : public wd177x_format
{
public:
	opus_ddos_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class acorn_adfs_old_format : public wd177x_format
{
public:
	acorn_adfs_old_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class acorn_adfs_new_format : public wd177x_format
{
public:
	acorn_adfs_new_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class acorn_dos_format : public wd177x_format
{
public:
	acorn_dos_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class opus_ddcpm_format : public floppy_image_format_t
{
public:
	opus_ddcpm_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};


extern const floppy_format_type FLOPPY_ACORN_SSD_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_DSD_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_DOS_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_ADFS_OLD_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_ADFS_NEW_FORMAT;
extern const floppy_format_type FLOPPY_OPUS_DDOS_FORMAT;
extern const floppy_format_type FLOPPY_OPUS_DDCPM_FORMAT;

#endif // MAME_FORMATS_ACORN_DSK_H
