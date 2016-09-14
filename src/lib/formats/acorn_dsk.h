// license:GPL-2.0+
// copyright-holders:Dirk Best, Nigel Barnes
/***************************************************************************

    Acorn - BBC Micro, Electron, Archimedes

    Disk image formats

***************************************************************************/

#pragma once

#ifndef __ACORN_DSK_H__
#define __ACORN_DSK_H__

#include "wd177x_dsk.h"

class acorn_ssd_format : public wd177x_format
{
public:
	acorn_ssd_format();

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
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

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
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

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
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

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
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

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
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

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class acorn_cpm_format : public wd177x_format
{
public:
	acorn_cpm_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class torch_cpn_format : public wd177x_format
{
public:
	torch_cpn_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};


extern const floppy_format_type FLOPPY_ACORN_SSD_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_DSD_FORMAT;
extern const floppy_format_type FLOPPY_OPUS_DDOS_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_ADFS_OLD_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_ADFS_NEW_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_DOS_FORMAT;
extern const floppy_format_type FLOPPY_ACORN_CPM_FORMAT;
extern const floppy_format_type FLOPPY_TORCH_CPN_FORMAT;

#endif // __ACORN_DSK_H__
