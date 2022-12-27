// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetreader.cpp - BGFX target JSON reader
//
//============================================================

#include "targetreader.h"

#include <modules/lib/osdobj_common.h>

#include "bgfxutil.h"
#include "chainmanager.h"
#include "target.h"

#include <cmath>

const target_reader::string_to_enum target_reader::STYLE_NAMES[target_reader::STYLE_COUNT] = {
	{ "guest",  TARGET_STYLE_GUEST },
	{ "native", TARGET_STYLE_NATIVE },
	{ "custom", TARGET_STYLE_CUSTOM }
};

bgfx_target* target_reader::read_from_value(const Value& value, std::string prefix, chain_manager& chains, uint32_t screen_index, uint16_t user_prescale, uint16_t max_prescale_size)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

	std::string target_name = value["name"].GetString();
	uint32_t mode = uint32_t(get_enum_from_value(value, "mode", TARGET_STYLE_NATIVE, STYLE_NAMES, STYLE_COUNT));
	bool bilinear = get_bool(value, "bilinear", true);
	bool double_buffer = get_bool(value, "doublebuffer", true);
	int scale = 1;
	if (value.HasMember("scale"))
	{
		scale = int(floor(value["scale"].GetDouble() + 0.5));
	}
	bool use_user_prescale = get_bool(value, "user_prescale", false);

	uint16_t width = 0;
	uint16_t height = 0;
	uint16_t xprescale = 1;
	uint16_t yprescale = 1;
	switch (mode)
	{
		case TARGET_STYLE_GUEST:
		{
			width = chains.targets().width(TARGET_STYLE_GUEST, screen_index);
			height = chains.targets().height(TARGET_STYLE_GUEST, screen_index);

			if (use_user_prescale)
			{
				xprescale = user_prescale;
				yprescale = user_prescale;
				printf("max prescale size: %d\n", (int)max_prescale_size);
				bgfx_util::find_prescale_factor(width, height, max_prescale_size, xprescale, yprescale);
			}
			break;
		}
		case TARGET_STYLE_NATIVE:
			width = chains.targets().width(TARGET_STYLE_NATIVE, screen_index);
			height = chains.targets().height(TARGET_STYLE_NATIVE, screen_index);

			if (use_user_prescale)
			{
				xprescale = user_prescale;
				yprescale = user_prescale;
				bgfx_util::find_prescale_factor(width, height, max_prescale_size, xprescale, yprescale);
			}
			break;
		case TARGET_STYLE_CUSTOM:
			READER_WARN(!value.HasMember("user_prescale"), (prefix + "Target '" + target_name + "': user_prescale parameter is not used for custom-type render targets.\n").c_str());
			if (!READER_CHECK(value.HasMember("width"), (prefix + "Target '" + target_name + "': Must have numeric value 'width'\n").c_str())) return nullptr;
			if (!READER_CHECK(value["width"].IsNumber(), (prefix + "Target '" + target_name + "': Value 'width' must be a number\n").c_str())) return nullptr;
			if (!READER_CHECK(value.HasMember("height"), (prefix + "Target '" + target_name + "': Must have numeric value 'height'\n").c_str())) return nullptr;
			if (!READER_CHECK(value["height"].IsNumber(), (prefix + "Target '" + target_name + "': Value 'height' must be a number\n").c_str())) return nullptr;
			width = uint16_t(value["width"].GetDouble());
			height = uint16_t(value["height"].GetDouble());
			break;
	}

	return chains.targets().create_target(target_name, bgfx::TextureFormat::BGRA8, width, height, xprescale, yprescale, mode, double_buffer, bilinear, scale, screen_index);
}

bool target_reader::validate_parameters(const Value& value, std::string prefix)
{
	if (!READER_CHECK(value.HasMember("name"), (prefix + "Must have string value 'name'\n").c_str())) return false;
	if (!READER_CHECK(value["name"].IsString(), (prefix + "Value 'name' must be a string\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("mode"), (prefix + "Must have string enum 'mode'\n").c_str())) return false;
	if (!READER_CHECK(value["mode"].IsString(), (prefix + "Value 'mode' must be a string (what screens does this apply to?)\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("bilinear") || value["bilinear"].IsBool(), (prefix + "Value 'bilinear' must be a boolean\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("doublebuffer") || value["doublebuffer"].IsBool(), (prefix + "Value 'doublebuffer' must be a boolean\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("user_prescale") || value["user_prescale"].IsBool(), (prefix + "Value 'user_prescale' must be a boolean\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("scale") || value["scale"].IsNumber(), (prefix + "Value 'scale' must be a numeric value\n").c_str())) return false;
	return true;
}
