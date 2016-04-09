// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainmanager.cpp - BGFX shader chain manager
//
//  Provides loading for BGFX shader effect chains, defined
//  by chain.h and read by chainreader.h
//
//============================================================

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include "emu.h"
#include "window.h"

#include <bx/readerwriter.h>
#include <bx/crtimpl.h>

#include "bgfxutil.h"

#include "chainmanager.h"
#include "chainreader.h"
#include "chain.h"

#include "texture.h"
#include "target.h"
#include "slider.h"

using namespace rapidjson;

chain_manager::chain_manager(running_machine& machine, osd_options& options, texture_manager& textures, target_manager& targets, effect_manager& effects, uint32_t window_index)
    : m_machine(machine)
    , m_options(options)
    , m_textures(textures)
    , m_targets(targets)
    , m_effects(effects)
    , m_window_index(window_index)
{
    load_screen_chains(options.bgfx_screen_chains());
}

chain_manager::~chain_manager()
{
    for (std::vector<bgfx_chain*> screen_chains : m_screen_chains)
    {
        for (bgfx_chain* chain : screen_chains)
        {
            delete chain;
        }
    }
}

bgfx_chain* chain_manager::load_chain(std::string name, running_machine& machine, uint32_t window_index, uint32_t screen_index)
{
	if (name.length() < 5 || (name.compare(name.length() - 5, 5, ".json")!= 0))
	{
		name = name + ".json";
	}
	std::string path = std::string(m_options.bgfx_path()) + "/chains/" + name;

	bx::CrtFileReader reader;
	if (!bx::open(&reader, path.c_str()))
	{
		printf("Unable to open chain file %s, falling back to no post processing\n", path.c_str());
		return nullptr;
	}

	int32_t size(bx::getSize(&reader));

	char* data = new char[size + 1];
	bx::read(&reader, reinterpret_cast<void*>(data), size);
	bx::close(&reader);
	data[size] = 0;

	Document document;
	document.Parse<kParseCommentsFlag>(data);

	delete [] data;

	if (document.HasParseError())
	{
		std::string error(GetParseError_En(document.GetParseError()));
		printf("Unable to parse chain %s. Errors returned:\n", path.c_str());
		printf("%s\n", error.c_str());
		return nullptr;
	}

	bgfx_chain* chain = chain_reader::read_from_value(document, name + ": ", m_options, machine, window_index, screen_index, m_textures, m_targets, m_effects);

	if (chain == nullptr)
	{
		printf("Unable to load chain %s, falling back to no post processing\n", path.c_str());
		return nullptr;
	}

	return chain;
}

void chain_manager::load_screen_chains(std::string chain_str)
{
    std::vector<std::vector<std::string>> chain_names = split_option_string(chain_str);
    load_chains(chain_names);
}

std::vector<std::vector<std::string>> chain_manager::split_option_string(std::string chain_str) const
{
    std::vector<std::vector<std::string>> chain_names;
    chain_names.push_back(std::vector<std::string>());

    uint32_t length = chain_str.length();
    uint32_t win = 0;
    uint32_t last_start = 0;
    for (uint32_t i = 0; i < length + 1; i++) {
        if (i == length || chain_str[i] == ',' || chain_str[i] == ':') {
            chain_names[win].push_back(chain_str.substr(last_start, i - last_start));
            last_start = i + 1;
            if (chain_str[i] == ':') {
                win++;
                chain_names.push_back(std::vector<std::string>());
            }
        }
    }

    return chain_names;
}

void chain_manager::load_chains(std::vector<std::vector<std::string>>& chain_names)
{
    for (uint32_t win = 0; win < chain_names.size(); win++) {
        m_screen_chains.push_back(std::vector<bgfx_chain*>());
        if (win != m_window_index) {
            continue;
        }
        for (uint32_t screen = 0; screen < chain_names[win].size(); screen++) {
            bgfx_chain* chain = load_chain(chain_names[win][screen], m_machine, win, screen);
            if (chain == nullptr) {
                chain_names.clear();
                return;
            }
            m_screen_chains[win].push_back(chain);
        }
    }
}

bgfx_chain* chain_manager::screen_chain(uint32_t screen)
{
    if (screen >= m_screen_chains[m_window_index].size())
    {
        return m_screen_chains[m_window_index][m_screen_chains[m_window_index].size() - 1];
    }
    else
    {
        return m_screen_chains[m_window_index][screen];
    }
}

void chain_manager::process_screen_quad(uint32_t view, uint32_t screen, render_primitive* prim, osd_window &window)
{
    uint16_t tex_width(prim->texture.width);
    uint16_t tex_height(prim->texture.height);

    const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(prim->flags & PRIMFLAG_TEXFORMAT_MASK,
        tex_width, tex_height, prim->texture.rowpixels, prim->texture.palette, prim->texture.base);

    std::string full_name = "screen" + std::to_string(screen);
    bgfx_texture *texture = new bgfx_texture(full_name, bgfx::TextureFormat::RGBA8, tex_width, tex_height, mem);
    m_textures.add_provider(full_name, texture);

    m_targets.update_target_sizes(screen, tex_width, tex_height, TARGET_STYLE_GUEST);

    bgfx_chain* chain = screen_chain(screen);
    chain->process(prim, view, screen, m_textures, window, bgfx_util::get_blend_state(PRIMFLAG_GET_BLENDMODE(prim->flags)));
    view += chain->applicable_passes();

    m_textures.add_provider(full_name, nullptr);
    delete texture;
}

std::vector<render_primitive*> chain_manager::count_screens(render_primitive* prim)
{
    std::vector<render_primitive*> screens;

    int screen_count = 0;
    std::vector<void*> bases;
    while (prim != nullptr) {
        if (PRIMFLAG_GET_SCREENTEX(prim->flags)) {
            bool found = false;
            for (void* base : bases) {
                if (base == prim->texture.base) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                screen_count++;
                screens.push_back(prim);
                bases.push_back(prim->texture.base);
            }
        }
        prim = prim->next();
    }

    const uint32_t available_chains = m_screen_chains[m_window_index].size();
    if (screen_count >= available_chains)
    {
        screen_count = available_chains;
    }

    if (screen_count > 0) {
        m_targets.update_screen_count(screen_count);
    }

    return screens;
}

uint32_t chain_manager::handle_screen_chains(uint32_t view, render_primitive *starting_prim, osd_window& window) {
    if (m_screen_chains.size() <= m_window_index || m_screen_chains[m_window_index].size() == 0) {
        return 0;
    }

    std::vector<render_primitive*> screens = count_screens(starting_prim);

    if (screens.size() == 0) {
        return 0;
    }

    const uint32_t available_chains = m_screen_chains[m_window_index].size();

    // Process each screen as necessary
    uint32_t used_views = 0;
    int screen_index = 0;
    for (render_primitive* prim : screens) {
        if (screen_index >= available_chains) {
            break;
        }

        uint16_t screen_width(floor((prim->bounds.x1 - prim->bounds.x0) + 0.5f));
        uint16_t screen_height(floor((prim->bounds.y1 - prim->bounds.y0) + 0.5f));
        if (window.swap_xy()) {
            std::swap(screen_width, screen_height);
        }

        m_targets.update_target_sizes(screen_index, screen_width, screen_height, TARGET_STYLE_NATIVE);
        process_screen_quad(view + used_views, screen_index, prim, window);
        used_views += screen_chain(screen_index)->applicable_passes();

        screen_index++;
    }

    bgfx::setViewFrameBuffer(view + used_views, BGFX_INVALID_HANDLE);

    return used_views;
}

bool chain_manager::has_applicable_pass(uint32_t screen)
{
    return m_screen_chains.size() > m_window_index && screen < m_screen_chains[m_window_index].size();
}

slider_state* chain_manager::get_slider_list()
{
    if (m_screen_chains.size() <= m_window_index || m_screen_chains[m_window_index].size() == 0) {
        return nullptr;
    }

    slider_state *listhead = nullptr;
    slider_state **tailptr = &listhead;
    for (std::vector<bgfx_chain*> screen : m_screen_chains) {
        for (bgfx_chain* chain : screen) {
            std::vector<bgfx_slider*> sliders = chain->sliders();
            for (bgfx_slider* slider : sliders) {
                if (*tailptr == nullptr) {
                    *tailptr = slider->core_slider();
                } else {
                    (*tailptr)->next = slider->core_slider();
                    tailptr = &(*tailptr)->next;
                }
            }
        }
    }
    if (*tailptr != nullptr) {
        (*tailptr)->next = nullptr;
    }
    return listhead;
}