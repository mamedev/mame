#pragma once

#ifndef __DRAWBGFX_WRITE_READER__
#define __DRAWBGFX_WRITE_READER__

#include "statereader.h"

class write_reader : public state_reader {
public:
	static uint64_t read_from_value(const Value& value);

private:
	static const int RGB_COUNT = 4;
	static const int ALPHA_COUNT = 4;
	static const string_to_enum RGB_NAMES[RGB_COUNT];
	static const string_to_enum ALPHA_NAMES[ALPHA_COUNT];
};

#endif // __DRAWBGFX_WRITE_READER__
