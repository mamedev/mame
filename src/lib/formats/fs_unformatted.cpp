// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of unformatted floppy images

#include "emu.h"
#include "fs_unformatted.h"

void fs_unformatted::enumerate(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	bool all = form_factor == floppy_image::FF_UNKNOWN;
	u32 best_8 =
		form_factor == floppy_image::FF_8 ?
		has_variant(variants, floppy_image::DSDD) ? FSI_8_DSDD :
		has_variant(variants, floppy_image::DSSD) ? FSI_8_DSSD : FSI_8_SSSD
		: FSI_NONE;

	u32 best_525 =
		form_factor == floppy_image::FF_525 ?
		has_variant(variants, floppy_image::DSHD) ? FSI_525_DSHD :
		has_variant(variants, floppy_image::DSQD) ? FSI_525_DSQD :
		has_variant(variants, floppy_image::DSDD) ? FSI_525_DSDD :
		has_variant(variants, floppy_image::SSQD) ? FSI_525_SSQD :
		has_variant(variants, floppy_image::SSDD) ? FSI_525_SSDD : FSI_525_SSSD
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
		fe.add_raw(this, "u8dsdd", FSI_8_DSDD, "Unformatted 8\" double-sided double-density");
	if(all || best_8 == FSI_8_DSSD)
		fe.add_raw(this, "u8dssd", FSI_8_DSSD, "Unformatted 8\" double-sided single-density");
	if(all || best_8 == FSI_8_SSSD)
		fe.add_raw(this, "u8sssd", FSI_8_SSSD, "Unformatted 8\" single-sided single-density");

	if(all || best_525 == FSI_525_DSHD)
		fe.add_raw(this, "u525dshd", FSI_525_DSHD, "Unformatted 5\"25 double-sided high-density");
	if(all || best_525 == FSI_525_DSQD)
		fe.add_raw(this, "u525dsqd", FSI_525_DSQD, "Unformatted 5\"25 double-sided quad-density");
	if(all || best_525 == FSI_525_DSDD)
		fe.add_raw(this, "u525dsdd", FSI_525_DSDD, "Unformatted 5\"25 double-sided double-density");
	if(all)
		fe.add_raw(this, "u525dssd", FSI_525_DSSD, "Unformatted 5\"25 double-sided single-density");
	if(all || best_525 == FSI_525_SSQD)
		fe.add_raw(this, "u525ssqd", FSI_525_SSQD, "Unformatted 5\"25 single-sided quad-density");
	if(all || best_525 == FSI_525_SSDD)
		fe.add_raw(this, "u525ssdd", FSI_525_SSDD, "Unformatted 5\"25 single-sided double-density");
	if(all || best_525 == FSI_525_SSSD)
		fe.add_raw(this, "u525sssd", FSI_525_SSSD, "Unformatted 5\"25 single-sided single-density");

	if(all || has_variant(variants, floppy_image::DSED))
		fe.add_raw(this, "u35dsed", FSI_35_DSED, "Unformatted 3\"5 double-sided extra-density");
	if(all || has_variant(variants, floppy_image::DSHD))
		fe.add_raw(this, "u35dshd", FSI_35_DSHD, "Unformatted 3\"5 double-sided high-density");
	if(all || best_35 == FSI_35_DSDD)
		fe.add_raw(this, "u35dsdd", FSI_35_DSDD, "Unformatted 3\"5 double-sided double-density");
	if(all || best_35 == FSI_35_SSDD)
		fe.add_raw(this, "u35ssdd", FSI_35_SSDD, "Unformatted 3\"5 single-sided double-density");

	if(all || best_3 == FSI_3_DSDD)
		fe.add_raw(this, "u3dsdd", FSI_35_DSDD, "Unformatted 3\" double-sided double-density");
	if(all || best_3 == FSI_3_SSDD)
		fe.add_raw(this, "u3ssdd", FSI_35_SSDD, "Unformatted 3\" single-sided double-density");
}

void fs_unformatted::floppy_instantiate_raw(u32 key, floppy_image *image) const
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

const filesystem_manager_type FS_UNFORMATTED = &filesystem_manager_creator<fs_unformatted>;
