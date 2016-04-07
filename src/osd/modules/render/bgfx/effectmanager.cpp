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

#include "emu.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <bgfx/bgfxplatform.h>
#include <bgfx/bgfx.h>
#include <bx/readerwriter.h>
#include <bx/crtimpl.h>

#include "effectmanager.h"
#include "effectreader.h"
#include "effect.h"

using namespace rapidjson;

effect_manager::~effect_manager()
{
	for (std::pair<std::string, bgfx_effect*> effect : m_effects)
	{
		delete effect.second;
	}
	m_effects.clear();
}

bgfx_effect* effect_manager::effect(std::string name)
{
	std::map<std::string, bgfx_effect*>::iterator iter = m_effects.find(name);
	if (iter != m_effects.end())
	{
		return iter->second;
	}

	return load_effect(name);
}

bgfx_effect* effect_manager::load_effect(std::string name)
{
	std::string full_name = name;
	if (full_name.length() < 5 || (full_name.compare(full_name.length() - 5, 5, ".json") != 0)) {
		full_name = full_name + ".json";
	}
	std::string path = std::string(m_options.bgfx_path()) + "/effects/" + full_name;

	bx::CrtFileReader reader;
	if (!bx::open(&reader, path.c_str()))
	{
		printf("Unable to open effect file %s\n", path.c_str());
		return nullptr;
	}

	int32_t size (bx::getSize(&reader));

	char* data = new char[size + 1];
	bx::read(&reader, reinterpret_cast<void*>(data), size);
	bx::close(&reader);
	data[size] = 0;

	Document document;
	document.Parse<kParseCommentsFlag>(data);

	delete [] data;

	if (document.HasParseError()) {
		std::string error(GetParseError_En(document.GetParseError()));
		printf("Unable to parse effect %s. Errors returned:\n", path.c_str());
		printf("%s\n", error.c_str());
		return nullptr;
	}

	bgfx_effect* effect = effect_reader::read_from_value(document, "Effect '" + name + "': ", m_shaders);

	if (effect == nullptr) {
		printf("Unable to load effect %s\n", path.c_str());
		return nullptr;
	}

	m_effects[name] = effect;

	return effect;
}
