/*
 * monitor_retro.cpp
 *
 */
#include "emu.h"
#include "modules/osdmodule.h"
#include "monitor_module.h"

#include "modules/osdwindow.h"
#include "monitor_common.h"

#include "libretro/libretro-internal/libretro_shared.h"

 //============================================================
 //  sdl_monitor_info
 //============================================================

class retro_monitor_info : public osd_monitor_info
{
public:
	retro_monitor_info(monitor_module &module, std::uint64_t handle, const char *monitor_device, float aspect)
		: osd_monitor_info(module, handle, monitor_device, aspect)
	{
		retro_monitor_info::refresh();
	}

private:
	void refresh() override
	{

		m_pos_size = osd_rect(0,0, fb_width, fb_height);
		m_usuable_pos_size = osd_rect(0,0, fb_width, fb_height);
		m_is_primary = (oshandle() == 0);

	if(alternate_renderer==false)
		set_aspect(retro_aspect);
		//printf("refreshmonitor  (%d x %d) a:%f\n", fb_width, fb_height,retro_aspect);
	}

};

//============================================================
//  sdl_monitor_module
//============================================================

class retro_monitor_module : public monitor_module_base
{
public:
	retro_monitor_module()
		: monitor_module_base(OSD_MONITOR_PROVIDER, "retro")
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

		std::uint64_t display = 0;
		return monitor_from_handle(display);
	}

protected:
	int init_internal(const osd_options& options) override
	{
		// make a list of monitors
		{
			int i;

			osd_printf_verbose("Enter init_monitors\n");

			for (i = 0; i < 1; i++)
			{
				char temp[64];
				snprintf(temp, sizeof(temp) - 1, "%s%d", OSDOPTION_SCREEN, i);

				// allocate a new monitor info
				std::shared_ptr<osd_monitor_info> monitor = std::make_shared<retro_monitor_info>(*this, i, temp, 1.0f);

				osd_printf_verbose("Adding monitor %s (%d x %d)\n", monitor->devicename().c_str(),
				monitor->position_size().width(), monitor->position_size().height());

				// guess the aspect ratio assuming square pixels
				monitor->set_aspect(static_cast<float>(monitor->position_size().width()) / static_cast<float>(monitor->position_size().height()));
printf("Adding monitor %s (%d x %d) a:%f\n", monitor->devicename().c_str(),
				monitor->position_size().width(), monitor->position_size().height(),(float)monitor->position_size().width()/(float) monitor->position_size().height());

				// hook us into the list
				add_monitor(monitor);
			}
		}
		osd_printf_verbose("Leave init_monitors\n");

		return 0;
	}

private:
/*
		sdl.h = osd.height();
		sdl.w = osd.width();
		sdl.x = osd.left();
		sdl.y = osd.top();

*/
	static int compute_intersection(const osd_rect &rect1, const osd_rect &rect2)
	{
/*
		SDL_Rect intersection;
		if (SDL_IntersectRect(&sdl1, &sdl2, &intersection))
			return intersection.w + intersection.h;
*/
		return 0;
	}
};


MODULE_DEFINITION(MONITOR_RETRO, retro_monitor_module)
