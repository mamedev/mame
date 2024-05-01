// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  uniformreader.cpp - BGFX shader uniform JSON reader
//
//============================================================

#include "uniformreader.h"

#include "uniform.h"

#include <cstdlib>


const uniform_reader::string_to_enum uniform_reader::TYPE_NAMES[uniform_reader::TYPE_COUNT] = {
	{ "int",    bgfx::UniformType::Sampler },
	{ "vec4",   bgfx::UniformType::Vec4 },
	{ "mat3",   bgfx::UniformType::Mat3 },
	{ "mat4",   bgfx::UniformType::Mat4 }
};

std::unique_ptr<bgfx_uniform> uniform_reader::read_from_value(const Value& value, const std::string &prefix)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}
	const char* name = value["name"].GetString();

	bgfx::UniformType::Enum type = bgfx::UniformType::Enum(get_enum_from_value(value, "type", bgfx::UniformType::Vec4, TYPE_NAMES, TYPE_COUNT));
	const size_t type_size = bgfx_uniform::get_size_for_type(type);

	const Value& value_array = value["values"];
	const size_t array_size = value_array.Size() * sizeof(float);

	auto* data = reinterpret_cast<float*>(std::malloc(std::max(type_size, array_size)));

	unsigned int index = 0;
	for (; index < type_size / 4 && index < value_array.Size(); index++)
		data[index] = float(value_array[index].GetDouble());

	for (; index < type_size / 4; index++)
		data[index] = 0.0f;

	auto uniform = std::make_unique<bgfx_uniform>(name, type);
	uniform->set(data, type_size);
	std::free(data);

	return uniform;
}

bool uniform_reader::validate_parameters(const Value& value, const std::string &prefix)
{
	if (!READER_CHECK(value.HasMember("name"), "%sMust have string value 'name' (what is this uniform called in the shader code?)\n", prefix)) return false;
	if (!READER_CHECK(value["name"].IsString(), "%sValue 'name' must be a string\n", prefix)) return false;
	if (!READER_CHECK(value.HasMember("type"), "%sMust have string value 'type' [int, vec4, mat3, mat4]\n", prefix)) return false;
	if (!READER_CHECK(value.HasMember("values"), "%sMust have array value 'values' (what are the uniform's default values?)\n", prefix)) return false;
	if (!READER_CHECK(value["values"].IsArray(), "%sValue 'values' must be an array\n", prefix)) return false;
	return true;
}
