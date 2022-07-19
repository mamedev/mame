// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of unformatted floppy images

#include "fs_unformatted.h"

namespace fs {

const unformatted_image UNFORMATTED;

const char *unformatted_image::name() const
{
	return "unformatted";
}

const char *unformatted_image::description() const
{
	return "Unformatted floppy image";
}

void unformatted_image::enumerate_f(floppy_enumerator &fe) const
{
	fe.add_raw("u8dsdd", FSI_8_DSDD, "Unformatted 8\" double-sided double-density");
	fe.add_raw("u8dssd", FSI_8_DSSD, "Unformatted 8\" double-sided single-density");
	fe.add_raw("u8sssd", FSI_8_SSSD, "Unformatted 8\" single-sided single-density");

	fe.add_raw("u525dshd", FSI_525_DSHD, "Unformatted 5\"25 double-sided high-density");
	fe.add_raw("u525dsqd", FSI_525_DSQD, "Unformatted 5\"25 double-sided quad-density");
	fe.add_raw("u525dsdd", FSI_525_DSDD, "Unformatted 5\"25 double-sided double-density");
	fe.add_raw("u525dssd", FSI_525_DSSD, "Unformatted 5\"25 double-sided single-density");
	fe.add_raw("u525ssqd", FSI_525_SSQD, "Unformatted 5\"25 single-sided quad-density");
	fe.add_raw("u525ssdd", FSI_525_SSDD, "Unformatted 5\"25 single-sided double-density");
	fe.add_raw("u525sssd", FSI_525_SSSD, "Unformatted 5\"25 single-sided single-density");

	fe.add_raw("u35dsed", FSI_35_DSED, "Unformatted 3\"5 double-sided extra-density");
	fe.add_raw("u35dshd", FSI_35_DSHD, "Unformatted 3\"5 double-sided high-density");
	fe.add_raw("u35dsdd", FSI_35_DSDD, "Unformatted 3\"5 double-sided double-density");
	fe.add_raw("u35ssdd", FSI_35_SSDD, "Unformatted 3\"5 single-sided double-density");

	fe.add_raw("u3dsdd", FSI_35_DSDD, "Unformatted 3\" double-sided double-density");
	fe.add_raw("u3ssdd", FSI_35_SSDD, "Unformatted 3\" single-sided double-density");
}

void unformatted_image::format(u32 key, floppy_image *image)
{
	switch(key) {
	case FSI_8_DSDD: image->set_form_variant(floppy_image::FF_8, floppy_image::DSDD); break;
	case FSI_8_DSSD: image->set_form_variant(floppy_image::FF_8, floppy_image::DSSD); break;
	case FSI_8_SSSD: image->set_form_variant(floppy_image::FF_8, floppy_image::SSSD); break;

	case FSI_525_DSHD: image->set_form_variant(floppy_image::FF_525, floppy_image::DSHD); break;
	case FSI_525_DSQD: image->set_form_variant(floppy_image::FF_525, floppy_image::DSQD); break;
	case FSI_525_DSDD: image->set_form_variant(floppy_image::FF_525, floppy_image::DSDD); break;
	case FSI_525_DSSD: image->set_form_variant(floppy_image::FF_525, floppy_image::DSSD); break;
	case FSI_525_SSQD: image->set_form_variant(floppy_image::FF_525, floppy_image::SSQD); break;
	case FSI_525_SSDD: image->set_form_variant(floppy_image::FF_525, floppy_image::SSDD); break;
	case FSI_525_SSSD: image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD); break;

	case FSI_35_DSED: image->set_form_variant(floppy_image::FF_35, floppy_image::DSED); break;
	case FSI_35_DSHD: image->set_form_variant(floppy_image::FF_35, floppy_image::DSHD); break;
	case FSI_35_DSDD: image->set_form_variant(floppy_image::FF_35, floppy_image::DSDD); break;
	case FSI_35_SSDD: image->set_form_variant(floppy_image::FF_35, floppy_image::SSDD); break;

	case FSI_3_DSDD: image->set_form_variant(floppy_image::FF_3, floppy_image::DSDD); break;
	case FSI_3_SSDD: image->set_form_variant(floppy_image::FF_3, floppy_image::SSDD); break;
	}
}

std::unique_ptr<filesystem_t> unformatted_image::mount(fsblk_t &blockdev) const
{
	return nullptr;
}

bool unformatted_image::can_format() const
{
	return false;
}

bool unformatted_image::can_read() const
{
	return false;
}

bool unformatted_image::can_write() const
{
	return false;
}

bool unformatted_image::has_rsrc() const
{
	return false;
}

} // namespace fs
