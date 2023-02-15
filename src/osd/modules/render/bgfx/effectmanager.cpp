// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effectmanager.cpp - BGFX shader effect manager
//
//  Maintains a string-to-entry lookup of BGFX shader
//  effects, defined by effect.h and read by effectreader.h
//
//============================================================

#include "effectmanager.h"

#include "effect.h"
#include "effectreader.h"
#include "shadermanager.h"

#include "path.h"

#include "osdfile.h"
#include "modules/lib/osdobj_common.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <bx/readerwriter.h>
#include <bx/file.h>

#include <bgfx/bgfx.h>

#include <utility>


static bool prepare_effect_document(const std::string &name, const osd_options &options, rapidjson::Document &document)
{
	std::string full_name = name;
	if (full_name.length() < 5 || (full_name.compare(full_name.length() - 5, 5, ".json") != 0))
	{
		full_name += ".json";
	}

	const std::string path = util::path_concat(options.bgfx_path(), "effects", full_name);

	bx::FileReader reader;
	if (!bx::open(&reader, path.c_str()))
	{
		osd_printf_error("Unable to open effect file %s\n", path);
		return false;
	}

	const int32_t size = bx::getSize(&reader);
	std::unique_ptr<char []> data(new (std::nothrow) char [size + 1]);
	if (!data)
	{
		osd_printf_error("Out of memory reading effect file %s\n", path);
		bx::close(&reader);
		return false;
	}

	bx::ErrorAssert err;
	bx::read(&reader, reinterpret_cast<void*>(data.get()), size, &err);
	bx::close(&reader);
	data[size] = 0;

	document.Parse<rapidjson::kParseCommentsFlag>(data.get());
	data.reset();

	if (document.HasParseError())
	{
		std::string error(rapidjson::GetParseError_En(document.GetParseError()));
		osd_printf_error("Unable to parse effect %s. Errors returned:\n%s\n", path, error);
		return false;
	}

	return true;
}


// keep constructor and destructor out-of-line so the header works with forward declarations

effect_manager::effect_manager(shader_manager& shaders) : m_shaders(shaders)
{
}

effect_manager::~effect_manager()
{
	// the map will automatically dispose of the effects
}

bgfx_effect* effect_manager::get_or_load_effect(const osd_options &options, const std::string &name)
{
	const auto iter = m_effects.find(name);
	if (iter != m_effects.end())
		return iter->second.get();

	return load_effect(options, name);
}

bgfx_effect* effect_manager::load_effect(const osd_options &options, const std::string &name)
{
	rapidjson::Document document;
	if (!prepare_effect_document(name, options, document))
	{
		return nullptr;
	}

	std::unique_ptr<bgfx_effect> effect = effect_reader::read_from_value(name, document, "Effect '" + name + "': ", options, m_shaders);

	if (!effect)
	{
		osd_printf_error("Unable to load effect %s\n", name);
		return nullptr;
	}

	return m_effects.emplace(name, std::move(effect)).first->second.get();
}

bool effect_manager::validate_effect(const osd_options &options, const std::string &name)
{
	rapidjson::Document document;
	if (!prepare_effect_document(name, options, document))
	{
		return false;
	}

	return effect_reader::validate_value(document, "Effect '" + name + "': ", options);
}
