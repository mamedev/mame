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

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


class acorn_dsd_format : public wd177x_format
{
public:
	acorn_dsd_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


class opus_ddos_format : public wd177x_format
{
public:
	opus_ddos_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


class acorn_adfs_old_format : public wd177x_format
{
public:
	acorn_adfs_old_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


class acorn_adfs_new_format : public wd177x_format
{
public:
	acorn_adfs_new_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


class acorn_dos_format : public wd177x_format
{
public:
	acorn_dos_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


class opus_ddcpm_format : public floppy_image_format_t
{
public:
	opus_ddcpm_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
};


class cumana_dfs_format : public wd177x_format
{
public:
	cumana_dfs_format();

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


extern const acorn_ssd_format FLOPPY_ACORN_SSD_FORMAT;
extern const acorn_dsd_format FLOPPY_ACORN_DSD_FORMAT;
extern const acorn_dos_format FLOPPY_ACORN_DOS_FORMAT;
extern const acorn_adfs_old_format FLOPPY_ACORN_ADFS_OLD_FORMAT;
extern const acorn_adfs_new_format FLOPPY_ACORN_ADFS_NEW_FORMAT;
extern const opus_ddos_format FLOPPY_OPUS_DDOS_FORMAT;
extern const opus_ddcpm_format FLOPPY_OPUS_DDCPM_FORMAT;
extern const cumana_dfs_format FLOPPY_CUMANA_DFS_FORMAT;

#endif // MAME_FORMATS_ACORN_DSK_H
