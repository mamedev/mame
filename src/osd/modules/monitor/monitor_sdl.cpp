// license:BSD-3-Clause
// copyright-holders: Brad Hughes, Olivier Galibert, R. Belmont
/*
 * monitor_sdl.cpp
 *
 */

#include "modules/osdmodule.h"
#include "monitor_module.h"

#if defined(OSD_SDL)

#include <algorithm>
#include <SDL2/SDL.h>

#include "modules/osdwindow.h"
#include "monitor_common.h"
#include "osdcore.h"
#include "window.h"

inline osd_rect SDL_Rect_to_osd_rect(const SDL_Rect &r)
{
	return osd_rect(r.x, r.y, r.w, r.h);
}

 //============================================================
 //  sdl_monitor_info
 //============================================================

class sdl_monitor_info : public osd_monitor_info
{
public:
	sdl_monitor_info(monitor_module &module, std::uint64_t handle, const char *monitor_device, float aspect)
		: osd_monitor_info(module, handle, monitor_device, aspect)
	{
		sdl_monitor_info::refresh();
	}

private:
	void refresh() override
	{
		SDL_DisplayMode dmode;

#if defined(SDLMAME_WIN32)
		SDL_GetDesktopDisplayMode(oshandle(), &dmode);
#else
		SDL_GetCurrentDisplayMode(oshandle(), &dmode);
#endif
		SDL_Rect dimensions;
		SDL_GetDisplayBounds(oshandle(), &dimensions);

		m_pos_size = SDL_Rect_to_osd_rect(dimensions);
		m_usuable_pos_size = SDL_Rect_to_osd_rect(dimensions);
		m_is_primary = (oshandle() == 0);
	}
};

//============================================================
//  sdl_monitor_module
//============================================================

class sdl_monitor_module : public monitor_module_base
{
public:
	sdl_monitor_module()
		: monitor_module_base(OSD_MONITOR_PROVIDER, "sdl")
	{
	}

	//
	// Given a proposed rect, returns the monitor whose bounds intersect the most with it
	//
	//   Windows OSD uses this kind of mechanism to constrain resizing but SDL does not
	//   The method below will allow this but isn't used by SDL OSD yet
	//
	std::shared_ptr<osd_monitor_info> monitor_from_rect(const osd_rect& proposed) override
	{
		if (!m_initialized)
			return nullptr;

		// Determines whether the first intersects less with the proposed rect than the second
		auto intersects_less = [&proposed](std::shared_ptr<osd_monitor_info> mon1, std::shared_ptr<osd_monitor_info> mon2)
		{
			int value1 = compute_intersection(mon1->usuable_position_size(), proposed);
			int value2 = compute_intersection(mon2->usuable_position_size(), proposed);

			return value1 < value2;
		};

		// Find the one that intersects with the proposed rect the most
		auto monitor = std::max_element(std::begin(list()), std::end(list()), intersects_less);
		return *monitor;
	}

	std::shared_ptr<osd_monitor_info> monitor_from_window(const osd_window& window) override
	{
		if (!m_initialized)
			return nullptr;

		std::uint64_t display = SDL_GetWindowDisplayIndex(static_cast<const sdl_window_info &>(window).platform_window());
		return monitor_from_handle(display);
	}

protected:
	int init_internal(const osd_options& options) override
	{
		// make a list of monitors
		{
			int i;

			osd_printf_verbose("Enter init_monitors\n");

			for (i = 0; i < SDL_GetNumVideoDisplays(); i++)
			{
				char temp[64];
				snprintf(temp, sizeof(temp) - 1, "%s%d", OSDOPTION_SCREEN, i);

				// allocate a new monitor info
				std::shared_ptr<osd_monitor_info> monitor = std::make_shared<sdl_monitor_info>(*this, i, temp, 1.0f);

				osd_printf_verbose("Adding monitor %s (%d x %d)\n",
						monitor->devicename(),
						monitor->position_size().width(), monitor->position_size().height());

				// guess the aspect ratio assuming square pixels
				monitor->set_aspect(static_cast<float>(monitor->position_size().width()) / static_cast<float>(monitor->position_size().height()));

				// hook us into the list
				add_monitor(monitor);
			}
		}
		osd_printf_verbose("Leave init_monitors\n");

		return 0;
	}

private:
	static void osdrect_to_sdlrect(const osd_rect &osd, SDL_Rect &sdl)
	{
		sdl.h = osd.height();
		sdl.w = osd.width();
		sdl.x = osd.left();
		sdl.y = osd.top();
	}

	static int compute_intersection(const osd_rect &rect1, const osd_rect &rect2)
	{
		SDL_Rect sdl1, sdl2;
		osdrect_to_sdlrect(rect1, sdl1);
		osdrect_to_sdlrect(rect2, sdl2);

		SDL_Rect intersection;
		if (SDL_IntersectRect(&sdl1, &sdl2, &intersection))
			return intersection.w + intersection.h;

		return 0;
	}
};

#else
MODULE_NOT_SUPPORTED(sdl_monitor_module, OSD_MONITOR_PROVIDER, "sdl")
#endif

MODULE_DEFINITION(MONITOR_SDL, sdl_monitor_module)
