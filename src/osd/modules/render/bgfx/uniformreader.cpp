// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  uniformreader.cpp - BGFX shader uniform JSON reader
//
//============================================================

#include "uniformreader.h"

#include "uniform.h"

const uniform_reader::string_to_enum uniform_reader::TYPE_NAMES[uniform_reader::TYPE_COUNT] = {
	{ "int",    bgfx::UniformType::Int1 },
	{ "vec4",   bgfx::UniformType::Vec4 },
	{ "mat3",   bgfx::UniformType::Mat3 },
	{ "mat4",   bgfx::UniformType::Mat4 }
};

bgfx_uniform* uniform_reader::read_from_value(const Value& value, std::string prefix)
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

	const size_t alloc_size = (type_size > array_size) ? type_size : array_size;
	float* data = reinterpret_cast<float*>(new char[alloc_size]);

	unsigned int index = 0;
	for (; index < type_size / 4 && index < value_array.Size(); index++)
	{
		data[index] = (float)value_array[index].GetDouble();
	}

	for (; index < type_size / 4; index++)
	{
		data[index] = 0.0f;
	}

	bgfx_uniform* uniform = new bgfx_uniform(name, type);
	uniform->set((void*)data, type_size);
	delete [] data;

	return uniform;
}

bool uniform_reader::validate_parameters(const Value& value, std::string prefix)
{
	if (!READER_CHECK(value.HasMember("name"), (prefix + "Must have string value 'name' (what is this uniform called in the shader code?)\n").c_str())) return false;
	if (!READER_CHECK(value["name"].IsString(), (prefix + "Value 'name' must be a string\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("type"), (prefix + "Must have string value 'type' [int, vec4, mat3, mat4]\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("values"), (prefix + "Must have array value 'values' (what are the uniform's default values?)\n").c_str())) return false;
	if (!READER_CHECK(value["values"].IsArray(), (prefix + "Value 'values' must be an array\n").c_str())) return false;
	return true;
}
