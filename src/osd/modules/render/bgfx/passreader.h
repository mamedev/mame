#pragma once

#ifndef __DRAWBGFX_PASS_READER__
#define __DRAWBGFX_PASS_READER__

#include "statereader.h"

class bgfx_pass;
class texture_manager;
class target_manager;
class effect_manager;

class pass_reader : public state_reader
{
public:
    static bgfx_pass* read_from_value(const Value& value, texture_manager& textures, target_manager& targets, effect_manager& effects);

private:
    static void validate_parameters(const Value& value);
};

#endif // __DRAWBGFX_PASS_READER__