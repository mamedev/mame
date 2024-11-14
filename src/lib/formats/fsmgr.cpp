// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy and hd images

// Currently limited to floppies and creation of preformatted images

#include "fsmgr.h"
#include "flopimg.h"

namespace fs {

void manager_t::enumerate_f(floppy_enumerator &fe) const
{
}

void manager_t::enumerate_h(hd_enumerator &he) const
{
}

void manager_t::enumerate_c(cdrom_enumerator &ce) const
{
}

bool manager_t::has_variant(const std::vector<u32> &variants, u32 variant)
{
	for(u32 v : variants)
		if(variant == v)
			return true;
	return false;
}

bool manager_t::has(u32 form_factor, const std::vector<u32> &variants, u32 ff, u32 variant)
{
	if(form_factor == floppy_image::FF_UNKNOWN)
		return true;
	if(form_factor != ff)
		return false;
	for(u32 v : variants)
		if(variant == v)
			return true;
	return false;
}

std::vector<meta_description> manager_t::volume_meta_description() const
{
	std::vector<meta_description> res;
	return res;
}

std::vector<meta_description> manager_t::file_meta_description() const
{
	std::vector<meta_description> res;
	return res;
}

std::vector<meta_description> manager_t::directory_meta_description() const
{
	std::vector<meta_description> res;
	return res;
}

char manager_t::directory_separator() const
{
	return 0; // Subdirectories not supported by default
}

manager_t::floppy_enumerator::floppy_enumerator(u32 form_factor, const std::vector<u32> &variants)
	: m_form_factor(form_factor)
	, m_variants(variants)
{
}

void manager_t::floppy_enumerator::add(const floppy_image_format_t &type, u32 form_factor, u32 variant, u32 image_size, const char *name, const char *description)
{
	if (has(m_form_factor, m_variants, form_factor, variant))
		add_format(type, image_size, name, description);
}

} // namespace fs
