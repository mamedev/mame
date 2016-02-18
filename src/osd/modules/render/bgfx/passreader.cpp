#include <string>

#include "emu.h"

#include "passreader.h"

#include "texturemanager.h"
#include "targetmanager.h"
#include "effectmanager.h"
#include "pass.h"

bgfx_pass* pass_reader::read_from_value(const Value& value, texture_manager& textures, target_manager& targets, effect_manager& effects)
{
    validate_parameters(value);

	bgfx_effect* effect = effects.effect(value["effect"].GetString());

	std::vector<bgfx_texture*> inputs;
	if (value.HasMember("input"))
	{
		const Value& input_array = value["input"];
		for (UINT32 i = 0; i < input_array.Size(); i++)
		{
			inputs.push_back(textures.texture(input_array[i].GetString()));
		}
	}

	bgfx_target* output = targets.target(value["output"].GetString());

	return new bgfx_pass(value["name"].GetString(), effect, inputs, output);
}

void pass_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("name"));
	assert(value["name"].IsString());
	assert(value.HasMember("output"));
	assert(value["output"].IsString());
	assert(value.HasMember("effect"));
	assert(value["effect"].IsString());
	if (value.HasMember("input"))
	{
		const Value& input_array = value["input"];
		for (int i = 0; i < input_array.Size(); i++)
		{
			assert(uniform_array[i].IsString());
		}
	}
}