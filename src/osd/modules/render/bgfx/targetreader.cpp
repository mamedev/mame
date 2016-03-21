// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetreader.cpp - BGFX target JSON reader
//
//============================================================

#include <string>

#include "emu.h"
#include <modules/lib/osdobj_common.h>

#include "targetreader.h"
#include "targetmanager.h"
#include "target.h"

const target_reader::string_to_enum target_reader::STYLE_NAMES[target_reader::STYLE_COUNT] = {
	{ "guest",  TARGET_STYLE_GUEST },
	{ "native", TARGET_STYLE_NATIVE },
	{ "custom",	TARGET_STYLE_CUSTOM }
};

bool target_reader::read_from_value(const Value& value, std::string prefix, target_manager& targets, osd_options& options, uint32_t screen_width, uint32_t screen_height)
{
	if (!validate_parameters(value, prefix))
	{
		return false;
	}

	std::string target_name = value["name"].GetString();
	uint32_t mode = uint32_t(get_enum_from_value(value, "mode", TARGET_STYLE_NATIVE, STYLE_NAMES, STYLE_COUNT));
	bool bilinear = get_bool(value, "bilinear", true);
	bool double_buffer = get_bool(value, "doublebuffer", true);
	bool prescale = get_bool(value, "prescale", false);
    bool output = get_bool(value, "output", false);

	uint16_t width = 0;
	uint16_t height = 0;
	switch (mode)
	{
		case TARGET_STYLE_GUEST:
			width = targets.guest_width();
			height = targets.guest_height();
			break;
		case TARGET_STYLE_NATIVE:
			width = screen_width;
			height = screen_height;
			break;
		case TARGET_STYLE_CUSTOM:
			if (!READER_CHECK(value.HasMember("width"), (prefix + "Target '" + target_name + "': Must have numeric value 'width'\n").c_str())) return false;
			if (!READER_CHECK(value["width"].IsNumber(), (prefix + "Target '" + target_name + "': Value 'width' must be a number\n").c_str())) return false;
			if (!READER_CHECK(value.HasMember("height"), (prefix + "Target '" + target_name + "': Must have numeric value 'height'\n").c_str())) return false;
			if (!READER_CHECK(value["height"].IsNumber(), (prefix + "Target '" + target_name + "': Value 'height' must be a number\n").c_str())) return false;
			width = uint32_t(value["width"].GetDouble());
			height = uint32_t(value["height"].GetDouble());
			break;
	}

	uint32_t prescale_x = 1;
	uint32_t prescale_y = 1;
	if (prescale)
	{
		prescale_x = options.bgfx_prescale_x();
		prescale_y = options.bgfx_prescale_y();
	}

    if (!output)
    {
        targets.create_target(target_name, bgfx::TextureFormat::RGBA8, width, height, prescale_x, prescale_y, mode, double_buffer, bilinear, output);
    }

	return true;
}

bool target_reader::validate_parameters(const Value& value, std::string prefix)
{
	if (!READER_CHECK(value.HasMember("name"), (prefix + "Must have string value 'name'\n").c_str())) return false;
	if (!READER_CHECK(value["name"].IsString(), (prefix + "Value 'name' must be a string\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("mode"), (prefix + "Must have string enum 'mode'\n").c_str())) return false;
	if (!READER_CHECK(value["mode"].IsString(), (prefix + "Value 'mode' must be a string (what screens does this apply to?)\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("bilinear") || value["bilinear"].IsBool(), (prefix + "Value 'bilinear' must be a boolean\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("doublebuffer") || value["doublebuffer"].IsBool(), (prefix + "Value 'doublebuffer' must be a boolean\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("prescale") || value["prescale"].IsBool(), (prefix + "Value 'prescale' must be a boolean\n").c_str())) return false;
    if (!READER_CHECK(!value.HasMember("output") || value["output"].IsBool(), (prefix + "Value 'output' must be a boolean\n").c_str())) return false;
    return true;
}
