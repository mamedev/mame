// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/hp_ipc_dsk.cpp

    HP Integral PC format

*********************************************************************/

#include "formats/hp_ipc_dsk.h"

hp_ipc_format::hp_ipc_format() : wd177x_format(formats)
{
}

const char *hp_ipc_format::name() const noexcept
{
	return "hp_ipc";
}

const char *hp_ipc_format::description() const noexcept
{
	return "HP Integral PC disk image";
}

const char *hp_ipc_format::extensions() const noexcept
{
	return "img";
}

const hp_ipc_format::format hp_ipc_format::formats[] = {
	// images from coho.org.  gaps unverified.
	{ floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 77, 2, 512,  {}, 1, {},  60, 22, 43 },
	{}
};

const hp_ipc_format FLOPPY_HP_IPC_FORMAT;

