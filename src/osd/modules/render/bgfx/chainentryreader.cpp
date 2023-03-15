// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentryreader.cpp - BGFX chain entry JSON reader
//
//============================================================

#include "chainentryreader.h"

#include "emucore.h"
#include "fileio.h"
#include "rendutil.h"
#include <modules/render/copyutil.h>

#include <modules/lib/osdobj_common.h>

#include "texturemanager.h"
#include "targetmanager.h"
#include "effectmanager.h"
#include "chainentry.h"
#include "chainmanager.h"
#include "slider.h"
#include "inputpair.h"
#include "entryuniform.h"
#include "entryuniformreader.h"
#include "suppressor.h"
#include "suppressorreader.h"
#include "clear.h"
#include "clearreader.h"

bgfx_chain_entry* chain_entry_reader::read_from_value(
		const Value &value,
		const std::string &prefix,
		chain_manager &chains,
		std::map<std::string, bgfx_slider*> &sliders,
		std::map<std::string, bgfx_parameter*> &params,
		uint32_t screen_index)
{
	if (!validate_parameters(value, prefix))
	{
		osd_printf_error("Chain entry failed validation.\n");
		return nullptr;
	}

	bgfx_effect* effect = chains.effects().get_or_load_effect(chains.options(), value["effect"].GetString());
	if (effect == nullptr)
	{
		return nullptr;
	}

	std::string name = value["name"].GetString();

	std::vector<bgfx_input_pair*> inputs;
	if (value.HasMember("input"))
	{
		const Value& input_array = value["input"];
		for (uint32_t i = 0; i < input_array.Size(); i++)
		{
			const Value& input = input_array[i];
			if (!READER_CHECK(input.HasMember("sampler"), "%sinput[%u]: Must have string value 'sampler' (what sampler are we binding to?)\n", prefix, i)) return nullptr;
			if (!READER_CHECK(input["sampler"].IsString(), "%sinput[%u]: Value 'sampler' must be a string\n", prefix, i)) return nullptr;
			bool has_texture = input.HasMember("texture");
			bool has_target = input.HasMember("target");
			bool has_option = input.HasMember("option");
			if (!READER_CHECK(has_texture || has_target || has_option, "%sinput[%u]: Must have string value 'target', 'texture' or 'option' (what source are we using?)\n", prefix, i)) return nullptr;
			if (!READER_CHECK(!has_texture || input["texture"].IsString(), "%sinput[%u]: Value 'texture' must be a string\n", prefix, i)) return nullptr;
			if (!READER_CHECK(!has_target || input["target"].IsString(), "%sinput[%u]: Value 'target' must be a string\n", prefix, i)) return nullptr;
			if (!READER_CHECK(!has_option || input["option"].IsString(), "%sinput[%u]: Value 'option' must be a string\n", prefix, i)) return nullptr;
			if (!READER_CHECK(has_target || !input.HasMember("bilinear") || input["bilinear"].IsBool(), "%sinput[%u]: Value 'bilinear' must be a boolean\n", prefix, i)) return nullptr;
			if (!READER_CHECK(has_target || !input.HasMember("clamp") || input["clamp"].IsBool(), "%sinput[%u]: Value 'clamp' must be a boolean\n", prefix, i)) return nullptr;
			if (!READER_CHECK(has_texture || has_option || !input.HasMember("selection") || input["selection"].IsString(), "%sinput[%u]: Value 'selection' must be a string\n", prefix, i)) return nullptr;
			bool bilinear = get_bool(input, "bilinear", true);
			bool clamp = get_bool(input, "clamp", false);
			std::string selection = get_string(input, "selection", "");

			std::vector<std::string> texture_names;
			std::string texture_name = "";
			if (has_texture || has_option)
			{
				if (has_texture)
				{
					texture_name = input["texture"].GetString();
				}
				if (has_option)
				{
					std::string option = input["option"].GetString();

					texture_name = chains.options().value(option.c_str());
				}

				if (texture_name != "" && texture_name != "screen" && texture_name != "palette")
				{
					if (selection == "")
					{
						// create texture for specified file name
						uint32_t flags = bilinear ? 0u : (BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT);
						flags |= clamp ? (BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP) : 0u;
						bgfx_texture* texture = chains.textures().create_png_texture(chains.options().art_path(), texture_name, texture_name, flags, screen_index);
						if (texture == nullptr)
						{
							return nullptr;
						}
					}
					else
					{
						// create texture for specified file name
						uint32_t flags = bilinear ? 0u : (BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT);
						flags |= clamp ? (BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP) : 0u;
						bgfx_texture* texture = chains.textures().create_png_texture(chains.options().art_path(), texture_name, texture_name, flags, screen_index);
						if (texture == nullptr)
						{
							return nullptr;
						}

						// get directory of file
						const size_t last_slash = texture_name.rfind('/');
						const std::string file_directory = last_slash != std::string::npos ? texture_name.substr(0, last_slash) : std::string();
						file_enumerator directory_path(chains.options().art_path());
						while (const osd::directory::entry *entry = directory_path.next(file_directory.empty() ? nullptr : file_directory.c_str()))
						{
							if (entry->type == osd::directory::entry::entry_type::FILE)
							{
								std::string file(entry->name);
								std::string extension(".png");

								// split into file name and extension
								std::string file_name;
								std::string file_extension;
								const size_t last_dot = file.rfind('.');
								if (last_dot != std::string::npos)
								{
									file_name = file.substr(0, last_dot);
									file_extension = file.substr(last_dot, file.length() - last_dot);
								}

								std::string file_path;
								if (file_directory == "")
								{
									file_path = file;
								}
								else
								{
									file_path = file_directory + PATH_SEPARATOR + file;
								}

								// check for .png extension
								if (file_extension == extension && std::find(texture_names.begin(), texture_names.end(), file_path) == texture_names.end())
								{
									// create textures for all files containd in the path of the specified file name
									uint32_t flags = bilinear ? 0u : (BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT);
									flags |= clamp ? (BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP) : 0u;
									bgfx_texture* texture = chains.textures().create_png_texture(chains.options().art_path(), file_path, file_path, flags, screen_index);
									if (texture == nullptr)
									{
										return nullptr;
									}
									texture_names.push_back(file_path);
								}
							}
						}
					}
				}
			}
			else if (has_target)
			{
				texture_name = input["target"].GetString();
			}
			else
			{
				return nullptr;
			}

			std::string sampler = input["sampler"].GetString();
			auto* input_pair = new bgfx_input_pair(i, sampler, texture_name, texture_names, selection, chains, screen_index);
			inputs.push_back(input_pair);
		}
	}

	// Parse whether or not to apply screen tint in this pass
	bool applytint = get_bool(value, "applytint", false);

	// Parse uniforms
	std::vector<bgfx_entry_uniform*> uniforms;
	if (value.HasMember("uniforms"))
	{
		const Value& uniform_array = value["uniforms"];
		for (uint32_t i = 0; i < uniform_array.Size(); i++)
		{
			bgfx_entry_uniform* uniform = entry_uniform_reader::read_from_value(uniform_array[i], prefix + "uniforms[" + std::to_string(i) + "]: ", effect, sliders, params);
			if (uniform == nullptr)
			{
				for (bgfx_entry_uniform* existing_uniform : uniforms) delete existing_uniform;
				return nullptr;
			}
			uniforms.push_back(uniform);
		}
	}

	std::vector<bgfx_suppressor*> suppressors;
	if (value.HasMember("disablewhen"))
	{
		const Value& suppressor_array = value["disablewhen"];
		for (uint32_t i = 0; i < suppressor_array.Size(); i++)
		{
			bgfx_suppressor* suppressor = suppressor_reader::read_from_value(suppressor_array[i], prefix, sliders);
			if (suppressor == nullptr)
			{
				for (bgfx_entry_uniform* uniform : uniforms) delete uniform;
				for (bgfx_suppressor* existing_suppressor : suppressors) delete existing_suppressor;
				return nullptr;
			}
			suppressors.push_back(suppressor);
		}
	}

	// Parse clear state
	clear_state* clear = nullptr;
	if (value.HasMember("clear"))
	{
		clear = clear_reader::read_from_value(value["clear"], prefix + "clear state: ");
	}
	else
	{
		clear = new clear_state(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);
	}

	std::string output = value["output"].GetString();
	return new bgfx_chain_entry(name, effect, clear, suppressors, inputs, uniforms, chains.targets(), output, applytint);
}

bool chain_entry_reader::validate_parameters(const Value& value, const std::string &prefix)
{
	if (!READER_CHECK(value.HasMember("effect"), "%sMust have string value 'effect' (what effect does this entry use?)\n", prefix)) return false;
	if (!READER_CHECK(value["effect"].IsString(), "%sValue 'effect' must be a string\n", prefix)) return false;
	if (!READER_CHECK(value.HasMember("name"), "%sMust have string value 'effect' (what effect does this entry use?)\n", prefix)) return false;
	if (!READER_CHECK(value["name"].IsString(), "%sValue 'name' must be a string\n", prefix)) return false;
	if (!READER_CHECK(value.HasMember("output"), "%sMust have string value 'offset' (what target are we rendering to?)\n", prefix)) return false;
	if (!READER_CHECK(value["output"].IsString(), "%sValue 'output' must be a string\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("input") || value["input"].IsArray(), "%sValue 'input' must be an array\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("uniforms") || value["uniforms"].IsArray(), "%sValue 'uniforms' must be an array\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("disablewhen") || value["disablewhen"].IsArray(), "%sValue 'disablewhen' must be an array\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("applytint") || value["applytint"].IsBool(), "%sValue 'applytint' must be a bool\n", prefix)) return false;
	return true;
}
