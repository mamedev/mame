// license:GPL-2.0+
// copyright-holders:Dirk Best, Nigel Barnes
/***************************************************************************

    BBC Micro

    Disk image formats

***************************************************************************/

#pragma once

#ifndef __BBC_DSK_H__
#define __BBC_DSK_H__

#include "wd177x_dsk.h"

class bbc_dfs_format : public wd177x_format
{
public:
	bbc_dfs_format();

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class bbc_adfs_format : public wd177x_format
{
public:
	bbc_adfs_format();

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class bbc_dos_format : public wd177x_format
{
public:
	bbc_dos_format();

	virtual int find_size(io_generic *io, UINT32 form_factor) override;
	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class bbc_cpm_format : public wd177x_format
{
public:
	bbc_cpm_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};


extern const floppy_format_type FLOPPY_BBC_DFS_FORMAT;
extern const floppy_format_type FLOPPY_BBC_ADFS_FORMAT;
extern const floppy_format_type FLOPPY_BBC_DOS_FORMAT;
extern const floppy_format_type FLOPPY_BBC_CPM_FORMAT;

#endif // __BBC_DSK_H__
