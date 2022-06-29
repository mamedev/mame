// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_coco_os9.cpp

    Management of CoCo OS-9 floppy images

    OS-9 Level 2 Technical Reference, Chapter 5, Random Block File Manager,
    page 2

    https://colorcomputerarchive.com/repo/Documents/Manuals/Operating%20Systems/OS-9%20Level%202%20Manual%20(Tandy).pdf

***************************************************************************/

#include "fs_coco_os9.h"
#include "coco_rawdsk.h"
#include "strformat.h"


using namespace fs;

namespace fs { const coco_os9_image COCO_OS9; }

namespace {

class coco_os9_impl : public filesystem_t {
public:
	coco_os9_impl(fsblk_t &blockdev);
    virtual ~coco_os9_impl() = default;

	static bool is_ignored_filename(std::string_view name);
	static bool validate_filename(std::string_view name);
};
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  name
//-------------------------------------------------

const char *coco_os9_image::name() const
{
	return "coco_os9";
}


//-------------------------------------------------
//  description
//-------------------------------------------------

const char *coco_os9_image::description() const
{
	return "CoCo OS-9";
}


//-------------------------------------------------
//  enumerate_f
//-------------------------------------------------

void coco_os9_image::enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const
{
	if (has(form_factor, variants, floppy_image::FF_525, floppy_image::SSDD))
	{
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 161280, "coco_rawdsk_os9_35", "CoCo Raw Disk OS-9 single-sided 35 tracks");
		fe.add(FLOPPY_COCO_RAWDSK_FORMAT, 184320, "coco_rawdsk_os9_40", "CoCo Raw Disk OS-9 single-sided 40 tracks");
	}
}


//-------------------------------------------------
//  can_format
//-------------------------------------------------

bool coco_os9_image::can_format() const
{
	return true;
}


//-------------------------------------------------
//  can_read
//-------------------------------------------------

bool coco_os9_image::can_read() const
{
	return true;
}


//-------------------------------------------------
//  can_write
//-------------------------------------------------

bool coco_os9_image::can_write() const
{
	return false;
}


//-------------------------------------------------
//  has_rsrc
//-------------------------------------------------

bool coco_os9_image::has_rsrc() const
{
	return false;
}


//-------------------------------------------------
//  directory_separator
//-------------------------------------------------

char coco_os9_image::directory_separator() const
{
	return '/';
}


//-------------------------------------------------
//  volume_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::volume_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_description(meta_name::name, "UNTITLED", false, [](const meta_value &m) { return m.as_string().size() <= 32; }, "Volume name, up to 32 characters"));
	results.emplace_back(meta_description(meta_name::creation_date, util::arbitrary_datetime::now(), false, nullptr, "Creation time"));
	return results;
}


//-------------------------------------------------
//  file_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::file_meta_description() const
{
	std::vector<meta_description> results;
	results.emplace_back(meta_description(meta_name::name, "", false, [](const meta_value &m) { return coco_os9_impl::validate_filename(m.as_string()); }, "File name"));
	results.emplace_back(meta_description(meta_name::creation_date, util::arbitrary_datetime::now(), false, nullptr, "Creation time"));
	results.emplace_back(meta_description(meta_name::owner_id, 0, true, nullptr, "Owner ID"));
	results.emplace_back(meta_description(meta_name::attributes, "", true, nullptr, "File attributes"));
	results.emplace_back(meta_description(meta_name::length, 0, true, nullptr, "Size of the file in bytes"));
	return results;
}


//-------------------------------------------------
//  directory_meta_description
//-------------------------------------------------

std::vector<meta_description> coco_os9_image::directory_meta_description() const
{
	return file_meta_description();
}

//-------------------------------------------------
//  mount
//-------------------------------------------------

std::unique_ptr<filesystem_t> coco_os9_image::mount(fsblk_t &blockdev) const
{
	return std::make_unique<coco_os9_impl>(blockdev);
}

//-------------------------------------------------
//  impl ctor
//-------------------------------------------------

coco_os9_impl::coco_os9_impl(fsblk_t &blockdev)
	: filesystem_t(blockdev, 256)
{
}

//-------------------------------------------------
//  is_ignored_filename - should this file name be
//  ignored if it is in the file system?
//-------------------------------------------------

bool coco_os9_impl::is_ignored_filename(std::string_view name)
{
	return name.empty()
		|| name[0] == '\0'
		|| name == "."
		|| name == "..";
}

//-------------------------------------------------
//  validate_filename
//-------------------------------------------------

bool coco_os9_impl::validate_filename(std::string_view name)
{
	return !is_ignored_filename(name)
		&& name.size() <= 29
		&& std::find_if(name.begin(), name.end(), [](const char ch) { return ch == '\0' || ch == '/' || ch >= 0x80; }) == name.end();
}
