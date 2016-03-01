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

bgfx_chain* chain_manager::chain(std::string name, running_machine& machine)
{
	std::map<std::string, bgfx_chain*>::iterator iter = m_chains.find(name);
	if (iter != m_chains.end())
	{
		return iter->second;
	}

	return load_chain(name, machine);
}

bgfx_chain* chain_manager::load_chain(std::string name, running_machine& machine) {
	std::string path = "bgfx/chains/" + name + ".json";

	bx::CrtFileReader reader;
	bx::open(&reader, path.c_str());

	int32_t size(bx::getSize(&reader));

	char* data = new char[size + 1];
	bx::read(&reader, reinterpret_cast<void*>(data), size);
	bx::close(&reader);
	data[size] = 0;

	Document document;
	document.Parse<0>(data);
	bgfx_chain* chain = chain_reader::read_from_value(document, machine, m_textures, m_targets, m_effects, m_width, m_height);

	m_chains[name] = chain;

	return chain;
}
