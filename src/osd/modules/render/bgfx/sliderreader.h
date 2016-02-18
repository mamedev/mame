#pragma once

#ifndef __DRAWBGFX_SLIDER_READER__
#define __DRAWBGFX_SLIDER_READER__

#include "statereader.h"

class bgfx_slider;
class shader_manager;

class slider_reader : public state_reader
{
public:
    static bgfx_slider* read_from_value(const Value& value);

private:
    static void validate_parameters(const Value& value);
};

#endif // __DRAWBGFX_SLIDER_READER__