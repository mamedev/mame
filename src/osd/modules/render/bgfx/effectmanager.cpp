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

#include "effectreader.h"
#include "effect.h"

#include "osdfile.h"
#include "modules/lib/osdobj_common.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <bx/readerwriter.h>
#include <bx/file.h>

#include <bgfx/bgfx.h>

static bool prepare_effect_document(std::string &name, osd_options &options, rapidjson::Document &document)
{
	std::string full_name = name;
	if (full_name.length() < 5 || (full_name.compare(full_name.length() - 5, 5, ".json") != 0))
	{
		full_name = full_name + ".json";
	}

	std::string path = osd_subst_env(util::string_format("%s" PATH_SEPARATOR "effects" PATH_SEPARATOR, options.bgfx_path()));
	path += full_name;

	bx::FileReader reader;
	if (!bx::open(&reader, path.c_str()))
	{
		osd_printf_error("Unable to open effect file %s\n", path.c_str());
		return false;
	}

	int32_t size (bx::getSize(&reader));
	char* data = new char[size + 1];
	bx::read(&reader, reinterpret_cast<void*>(data), size);
	bx::close(&reader);
	data[size] = 0;

	document.Parse<rapidjson::kParseCommentsFlag>(data);

	delete [] data;

	if (document.HasParseError())
	{
		std::string error(rapidjson::GetParseError_En(document.GetParseError()));
		osd_printf_error("Unable to parse effect %s. Errors returned:\n", path.c_str());
		osd_printf_error("%s\n", error.c_str());
		return false;
	}

	return true;
}

effect_manager::~effect_manager()
{
	for (std::pair<std::string, bgfx_effect*> effect : m_effects)
	{
		delete effect.second;
	}
	m_effects.clear();
}

bgfx_effect* effect_manager::get_or_load_effect(osd_options &options, std::string name)
{
	std::map<std::string, bgfx_effect*>::iterator iter = m_effects.find(name);
	if (iter != m_effects.end())
	{
		return iter->second;
	}

	return load_effect(options, name);
}

bgfx_effect* effect_manager::load_effect(osd_options &options, std::string name)
{
	rapidjson::Document document;
	if (!prepare_effect_document(name, options, document))
	{
		return nullptr;
	}

	bgfx_effect* effect = effect_reader::read_from_value(document, "Effect '" + name + "': ", options, m_shaders);

	if (effect == nullptr)
	{
		osd_printf_error("Unable to load effect %s\n", name.c_str());
		return nullptr;
	}

	m_effects[name] = effect;

	return effect;
}

bool effect_manager::validate_effect(osd_options &options, std::string name)
{
	rapidjson::Document document;
	if (!prepare_effect_document(name, options, document))
	{
		return false;
	}

	return effect_reader::validate_value(document, "Effect '" + name + "': ", options);
}
