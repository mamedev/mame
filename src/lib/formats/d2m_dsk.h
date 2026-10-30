// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d2m_dsk.h

    CMD FD-2000/FD-4000 disk image formats (D1M/D2M/D4M)

*********************************************************************/

#ifndef MAME_FORMATS_D2M_DSK_H
#define MAME_FORMATS_D2M_DSK_H

#pragma once

#include "upd765_dsk.h"


class cmd_fd_format : public upd765_format
{
public:
	virtual bool load(util::random_read &io, uint32_t form_factor,
		const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io,
		const std::vector<uint32_t> &variants, const floppy_image &image) const override;

protected:
	cmd_fd_format(const format *formats);

private:
	floppy_image_format_t::desc_e *get_desc_mfm_cmd(
		const format &f, int &current_size, int &end_gap_index) const;

	const format *const m_formats;
};


class d1m_format : public cmd_fd_format
{
public:
	d1m_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


class d2m_format : public cmd_fd_format
{
public:
	d2m_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


class d4m_format : public cmd_fd_format
{
public:
	d4m_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};


extern const d1m_format FLOPPY_D1M_FORMAT;
extern const d2m_format FLOPPY_D2M_FORMAT;
extern const d4m_format FLOPPY_D4M_FORMAT;

#endif // MAME_FORMATS_D2M_DSK_H
