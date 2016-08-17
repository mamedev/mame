// license:BSD-3-Clause
// copyright-holders:Carl
#include "atom_dsk.h"

atom_format::atom_format() : wd177x_format(formats)
{
}

const char *atom_format::name() const
{
	return "atom";
}

const char *atom_format::description() const
{
	return "Acorn Atom disk image";
}

const char *atom_format::extensions() const
{
	return "40t,dsk";
}

const atom_format::format atom_format::formats[] =
{
	{ // 100k 40 track single sided single density
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 10, 40, 1, 256, {}, 0, {}, 40, 10, 10
	},
	{}
};

const floppy_format_type FLOPPY_ATOM_FORMAT = &floppy_image_format_creator<atom_format>;
