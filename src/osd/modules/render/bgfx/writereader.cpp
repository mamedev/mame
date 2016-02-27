#include <bgfx/bgfx.h>

#include "writereader.h"

const write_reader::string_to_enum write_reader::RGB_NAMES[write_reader::RGB_COUNT] = {
	{ "true",   BGFX_STATE_RGB_WRITE },
	{ "false",  0 },
	{ "1",      BGFX_STATE_RGB_WRITE },
	{ "0",      0 }
};

const write_reader::string_to_enum write_reader::ALPHA_NAMES[write_reader::ALPHA_COUNT] = {
	{ "true",   BGFX_STATE_ALPHA_WRITE },
	{ "false",  0 },
	{ "1",      BGFX_STATE_ALPHA_WRITE },
	{ "0",      0 }
};

uint64_t write_reader::read_from_value(const Value& value)
{
	uint64_t rgb = get_enum_from_value(value, "rgb", 0, RGB_NAMES, RGB_COUNT);
	uint64_t alpha = get_enum_from_value(value, "alpha", 0, ALPHA_NAMES, ALPHA_COUNT);
	return rgb | alpha;
}
