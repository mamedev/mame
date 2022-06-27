// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_coco_rsdos.cpp

    Management of CoCo "RS-DOS" floppy images

***************************************************************************/

#include "fs_coco_rsdos.h"
#include "coco_rawdsk.h"
#include "util/corestr.h"
#include "util/strformat.h"

#include <bitset>
#include <optional>
#include <string_view>

using namespace fs;

namespace fs { const coco_rsdos_image COCO_RSDOS; }

namespace {

class coco_rsdos_impl : public filesystem_t {
public:
    coco_rsdos_impl(fsblk_t &blockdev);
    virtual ~coco_rsdos_impl() = default;

	static bool validate_filename(std::string_view name);
};
}


//-------------------------------------------------
//  name
//-------------------------------------------------

const char *coco_rsdos_image::name() const
{
	return "coco_rsdos";
}


//-------------------------------------------------
//  description
//-------------------------------------------------

const char *coco_rsdos_image::description() const
{
	return "CoCo RS-DOS";
}


//-------------------------------------------------
//  enumerate_f
//-------------------------------------------------

void coco_rsdos_image::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
	if (has(form_factor, variants, floppy_image::FF_525, floppy_image::SSDD))
	{
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 161280, "coco_rawdsk_rsdos_35", "CoCo Raw Disk RS-DOS single-sided 35 tracks");
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 184320, "coco_rawdsk_rsdos_40", "CoCo Raw Disk RS-DOS single-sided 40 tracks");
	}
}


//-------------------------------------------------
//  can_format
//-------------------------------------------------

bool coco_rsdos_image::can_format() const
{
	return true;
}


//-------------------------------------------------
//  can_read
//-------------------------------------------------

bool coco_rsdos_image::can_read() const
{
	return true;
}


//-------------------------------------------------
//  can_write
//-------------------------------------------------

bool coco_rsdos_image::can_write() const
{
	return false;
}


//-------------------------------------------------
//  has_rsrc
//-------------------------------------------------

bool coco_rsdos_image::has_rsrc() const
{
	return false;
}


//-------------------------------------------------
//  file_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_rsdos_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_description(meta_name::name, "", false, [](const meta_value &m) { return coco_rsdos_impl::validate_filename(m.as_string()); }, "File name, 8.3"));
	results.emplace_back(meta_description(meta_name::file_type, 0, true, nullptr, "Type of the file"));
	results.emplace_back(meta_description(meta_name::ascii_flag, "B", true, nullptr, "Ascii or binary flag"));
	results.emplace_back(meta_description(meta_name::size_in_blocks, 0, true, nullptr, "Number of granules used by the file"));
	results.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "Size of the file in bytes"));
	return results;
}


//-------------------------------------------------
//  mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> coco_rsdos_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<coco_rsdos_impl>(blockdev);
}


//-------------------------------------------------
//  validate_filename
//-------------------------------------------------

bool coco_rsdos_impl::validate_filename(std::string_view name)
{
	auto pos = name.find('.');
	auto stem_length = pos != std::string::npos ? pos : name.size();
	auto ext_length = pos != std::string::npos ? name.size() - pos - 1 : 0;
	return stem_length > 0 && stem_length <= 8 && ext_length <= 3;
}


//-------------------------------------------------
//  impl ctor
//-------------------------------------------------

coco_rsdos_impl::coco_rsdos_impl(fsblk_t &blockdev)
	: filesystem_t(blockdev, 256)
{
}


