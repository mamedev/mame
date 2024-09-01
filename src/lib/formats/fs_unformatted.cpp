// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of unformatted floppy images

#include "fs_unformatted.h"
#include "flopimg.h"
#include "fsblk.h"

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
	u32 form_factor = fe.form_factor();
	const std::vector<u32> &variants = fe.variants();

	bool all = form_factor == floppy_image::FF_UNKNOWN;
	u32 best_8 =
		form_factor == floppy_image::FF_8 ?
		has_variant(variants, floppy_image::DSDD) ? FSI_8_DSDD :
		has_variant(variants, floppy_image::DSSD) ? FSI_8_DSSD : FSI_8_SSSD
		: FSI_NONE;

	u32 best_525 =
		form_factor == floppy_image::FF_525 ?
		has_variant(variants, floppy_image::DSHD)   ? FSI_525_DSHD :
		has_variant(variants, floppy_image::DSQD16) ? FSI_525_DSQD16 :
		has_variant(variants, floppy_image::DSQD)   ? FSI_525_DSQD :
		has_variant(variants, floppy_image::DSDD16) ? FSI_525_DSDD16 :
		has_variant(variants, floppy_image::DSDD)   ? FSI_525_DSDD :
		has_variant(variants, floppy_image::SSQD16) ? FSI_525_SSQD16 :
		has_variant(variants, floppy_image::SSQD)   ? FSI_525_SSQD :
		has_variant(variants, floppy_image::SSDD16) ? FSI_525_SSDD16 :
		has_variant(variants, floppy_image::SSDD)   ? FSI_525_SSDD : FSI_525_SSSD
		: FSI_NONE;

	u32 best_35 =
		form_factor == floppy_image::FF_35 ?
		has_variant(variants, floppy_image::DSDD) ? FSI_35_DSDD : FSI_35_SSDD
		: FSI_NONE;

	u32 best_3 =
		form_factor == floppy_image::FF_3 ?
		has_variant(variants, floppy_image::DSDD) ? FSI_3_DSDD : FSI_3_SSDD
		: FSI_NONE;

	if(all || best_8 == FSI_8_DSDD)
		fe.add_raw("u8dsdd", FSI_8_DSDD, "Unformatted 8\" double-sided double-density");
	if(all || best_8 == FSI_8_DSSD)
		fe.add_raw("u8dssd", FSI_8_DSSD, "Unformatted 8\" double-sided single-density");
	if(all || best_8 == FSI_8_SSSD)
		fe.add_raw("u8sssd", FSI_8_SSSD, "Unformatted 8\" single-sided single-density");

	if(all || best_525 == FSI_525_DSHD)
		fe.add_raw("u525dshd", FSI_525_DSHD, "Unformatted 5\"25 double-sided high-density");
	if(all || best_525 == FSI_525_DSQD16)
		fe.add_raw("u525dsqd16", FSI_525_DSQD16, "Unformatted 5\"25 double-sided quad-density 16 hard sectors");
	if(all || best_525 == FSI_525_DSQD)
		fe.add_raw("u525dsqd", FSI_525_DSQD, "Unformatted 5\"25 double-sided quad-density");
	if(all || best_525 == FSI_525_DSDD16)
		fe.add_raw("u525dsdd16", FSI_525_DSDD16, "Unformatted 5\"25 double-sided double-density 16 hard sectors");
	if(all || best_525 == FSI_525_DSDD)
		fe.add_raw("u525dsdd", FSI_525_DSDD, "Unformatted 5\"25 double-sided double-density");
	if(all)
		fe.add_raw("u525dssd", FSI_525_DSSD, "Unformatted 5\"25 double-sided single-density");
	if(all || best_525 == FSI_525_SSQD16)
		fe.add_raw("u525ssqd16", FSI_525_SSQD16, "Unformatted 5\"25 single-sided quad-density 16 hard sectors");
	if(all || best_525 == FSI_525_SSQD)
		fe.add_raw("u525ssqd", FSI_525_SSQD, "Unformatted 5\"25 single-sided quad-density");
	if(all || best_525 == FSI_525_SSDD16)
		fe.add_raw("u525ssdd16", FSI_525_SSDD16, "Unformatted 5\"25 single-sided double-density 16 hard sectors");
	if(all || best_525 == FSI_525_SSDD)
		fe.add_raw("u525ssdd", FSI_525_SSDD, "Unformatted 5\"25 single-sided double-density");
	if(all || best_525 == FSI_525_SSSD)
		fe.add_raw("u525sssd", FSI_525_SSSD, "Unformatted 5\"25 single-sided single-density");

	if(all || has_variant(variants, floppy_image::DSED))
		fe.add_raw("u35dsed", FSI_35_DSED, "Unformatted 3\"5 double-sided extra-density");
	if(all || has_variant(variants, floppy_image::DSHD))
		fe.add_raw("u35dshd", FSI_35_DSHD, "Unformatted 3\"5 double-sided high-density");
	if(all || best_35 == FSI_35_DSDD)
		fe.add_raw("u35dsdd", FSI_35_DSDD, "Unformatted 3\"5 double-sided double-density");
	if(all || best_35 == FSI_35_SSDD)
		fe.add_raw("u35ssdd", FSI_35_SSDD, "Unformatted 3\"5 single-sided double-density");

	if(all || best_3 == FSI_3_DSDD)
		fe.add_raw("u3dsdd", FSI_35_DSDD, "Unformatted 3\" double-sided double-density");
	if(all || best_3 == FSI_3_SSDD)
		fe.add_raw("u3ssdd", FSI_35_SSDD, "Unformatted 3\" single-sided double-density");
}

void unformatted_image::format(u32 key, floppy_image *image)
{
	switch(key) {
	case FSI_8_DSDD: image->set_form_variant(floppy_image::FF_8, floppy_image::DSDD); break;
	case FSI_8_DSSD: image->set_form_variant(floppy_image::FF_8, floppy_image::DSSD); break;
	case FSI_8_SSSD: image->set_form_variant(floppy_image::FF_8, floppy_image::SSSD); break;

	case FSI_525_DSHD:   image->set_form_variant(floppy_image::FF_525, floppy_image::DSHD); break;
	case FSI_525_DSQD16: image->set_form_variant(floppy_image::FF_525, floppy_image::DSQD16); break;
	case FSI_525_DSQD:   image->set_form_variant(floppy_image::FF_525, floppy_image::DSQD); break;
	case FSI_525_DSDD16: image->set_form_variant(floppy_image::FF_525, floppy_image::DSDD16); break;
	case FSI_525_DSDD:   image->set_form_variant(floppy_image::FF_525, floppy_image::DSDD); break;
	case FSI_525_DSSD:   image->set_form_variant(floppy_image::FF_525, floppy_image::DSSD); break;
	case FSI_525_SSQD16: image->set_form_variant(floppy_image::FF_525, floppy_image::SSQD16); break;
	case FSI_525_SSQD:   image->set_form_variant(floppy_image::FF_525, floppy_image::SSQD); break;
	case FSI_525_SSDD16: image->set_form_variant(floppy_image::FF_525, floppy_image::SSDD16); break;
	case FSI_525_SSDD:   image->set_form_variant(floppy_image::FF_525, floppy_image::SSDD); break;
	case FSI_525_SSSD:   image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD); break;

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
