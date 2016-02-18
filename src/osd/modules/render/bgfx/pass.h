#pragma once

#ifndef __DRAWBGFX_PASS__
#define __DRAWBGFX_PASS__

#include <string>
#include <vector>

class render_primitive;
class bgfx_effect;
class bgfx_texture;
class bgfx_target;

class bgfx_pass
{
public:
    bgfx_pass(std::string name, bgfx_effect* effect, std::vector<bgfx_texture*>& inputs, bgfx_target* output);
    ~bgfx_pass();

    void submit(render_primitive* prim, int view);

    // Getters
    std::string name() const { return m_name; }

private:
	std::string					m_name;
    bgfx_effect*				m_effect;
    std::vector<bgfx_texture*>	m_inputs;
    bgfx_target*				m_output;
};

#endif // __DRAWBGFX_PASS__