// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainmanager.cpp - BGFX shader chain manager
//
//  Maintains a string-to-entry lookup of BGFX shader effect
//  chains, defined by chain.h and read by chainreader.h
//
//============================================================

#include "emu.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <bx/readerwriter.h>
#include <bx/crtimpl.h>

#include "chainmanager.h"
#include "chainreader.h"
#include "chain.h"

using namespace rapidjson;

chain_manager::~chain_manager()
{
	for (std::pair<std::string, bgfx_chain*> chain : m_chains)
	{
		delete chain.second;
	}
	m_chains.clear();
}

bgfx_chain* chain_manager::chain(std::string name, running_machine& machine, uint32_t window_index)
{
	std::map<std::string, bgfx_chain*>::iterator iter = m_chains.find(name + std::to_string(window_index));
	if (iter != m_chains.end())
	{
		return iter->second;
	}

	return load_chain(name, machine, window_index);
}

bgfx_chain* chain_manager::load_chain(std::string name, running_machine& machine, uint32_t window_index)
{
	if (name.length() < 5 || (name.compare(name.length() - 5, 5, ".json")!= 0))
	{
		name = name + ".json";
	}
	std::string path = std::string(m_options.bgfx_path()) + "/chains/" + name;

	bx::CrtFileReader reader;
	if (!bx::open(&reader, path.c_str()))
	{
        printf("Unable to open chain file %s, falling back to no post processing\n", path.c_str());
		return nullptr;
	}

	int32_t size(bx::getSize(&reader));

	char* data = new char[size + 1];
	bx::read(&reader, reinterpret_cast<void*>(data), size);
	bx::close(&reader);
	data[size] = 0;

	Document document;
	document.Parse<0>(data);

	if (document.HasParseError())
	{
		std::string error(GetParseError_En(document.GetParseError()));
        printf("Unable to parse chain %s. Errors returned:\n", path.c_str());
        printf("%s\n", error.c_str());
		return nullptr;
	}

	bgfx_chain* chain = chain_reader::read_from_value(document, name + ": ", m_options, machine, window_index, m_textures, m_targets, m_effects, m_width, m_height);

	if (chain == nullptr)
	{
        printf("Unable to load chain %s, falling back to no post processing\n", path.c_str());
		return nullptr;
	}

	m_chains[name + std::to_string(window_index)] = chain;

	return chain;
}
