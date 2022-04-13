// license:BSD-3-Clause
// copyright-holders: R. Belmont
//============================================================
//
//  monitor_mac.cpp - Mac monitor info provider
//
//  Mac OSD by R. Belmont
//
//============================================================

#include "emu.h"
#include "modules/osdmodule.h"
#include "monitor_module.h"

#if defined(OSD_MAC)

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

#include <algorithm>

#include "modules/osdwindow.h"
#include "monitor_common.h"
#include "window.h"

 //============================================================
 //  mac_monitor_info
 //============================================================

class mac_monitor_info : public osd_monitor_info
{
public:
	mac_monitor_info(monitor_module &module, CGDirectDisplayID handle, const char *monitor_device, float aspect)
		: osd_monitor_info(module, handle, monitor_device, aspect)
	{
		mac_monitor_info::refresh();
	}

private:
	void refresh() override
	{
		CGRect dimensions;
		dimensions = CGDisplayBounds((CGDirectDisplayID)oshandle());

		m_pos_size = osd_rect(dimensions.origin.x, dimensions.origin.y, dimensions.size.width, dimensions.size.height);
		m_usuable_pos_size = osd_rect(dimensions.origin.x, dimensions.origin.y, dimensions.size.width, dimensions.size.height);
		m_is_primary = CGDisplayIsMain((CGDirectDisplayID)oshandle());
	}
};

//============================================================
//  mac_monitor_module
//============================================================

class mac_monitor_module : public monitor_module_base
{
public:
	mac_monitor_module()
		: monitor_module_base(OSD_MONITOR_PROVIDER, "sdl")
	{
	}

	//
	// Given a proposed rect, returns the monitor whose bounds intersect the most with it
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
//      if (!m_initialized)
			return nullptr;

//      std::uint64_t display = SDL_GetWindowDisplayIndex(static_cast<const mac_window_info &>(window).platform_window());
//      return monitor_from_handle(display);
	}

protected:
	int init_internal(const osd_options& options) override
	{
		// make a list of monitors
		{
			uint32_t uNumDisplays;
			CGDirectDisplayID *displayList;

			osd_printf_verbose("Enter init_monitors\n");

			// pass NULL to get the number of displays
			CGGetActiveDisplayList(1, NULL, &uNumDisplays);

			displayList = (CGDirectDisplayID *)malloc(sizeof(CGDirectDisplayID) * uNumDisplays);
			CGGetActiveDisplayList(uNumDisplays, displayList, &uNumDisplays);

			for (uint32_t disp = 0; disp < uNumDisplays; disp++)
			{
				char temp[64];
				snprintf(temp, sizeof(temp) - 1, "%s%d", OSDOPTION_SCREEN, disp);

				// allocate a new monitor info
				std::shared_ptr<osd_monitor_info> monitor = std::make_shared<mac_monitor_info>(*this, displayList[disp], temp, 1.0f);

				osd_printf_verbose("Adding monitor %s (%d x %d)\n", monitor->devicename(),
				monitor->position_size().width(), monitor->position_size().height());

				// guess the aspect ratio assuming square pixels
				monitor->set_aspect(static_cast<float>(monitor->position_size().width()) / static_cast<float>(monitor->position_size().height()));

				// hook us into the list
				add_monitor(monitor);
			}

			free(displayList);
		}
		osd_printf_verbose("Leave init_monitors\n");

		return 0;
	}

	static int compute_intersection(const osd_rect &rect1, const osd_rect &rect2)
	{
		int min0, min1, max0, max1, result = 0;

		// if either rect is empty, bail early (shouldn't happen, but...)
		if (rect1.width() + rect1.height() == 0)
		{
			return 0;
		}
		if (rect2.width() + rect2.height() == 0)
		{
			return 0;
		}

		// do the X intersection
		min0 = rect1.left();
		max0 = min0 + rect1.width();
		min1 = rect2.left();
		max1 = min1 + rect2.width();
		if (min1 > min0)
		{
			min0 = min1;
		}
		if (max1 > max0)
		{
			max0 = max1;
		}
		result = max0 - min0;

		// and now the Y intersection
		min0 = rect1.top();
		max0 = min0 + rect1.height();
		min1 = rect2.top();
		max1 = min1 + rect2.height();
		if (min1 > min0)
		{
			min0 = min1;
		}
		if (max1 > max0)
		{
			max0 = max1;
		}
		result += max0 - min0;

		return result;
	}

private:
};

#else
MODULE_NOT_SUPPORTED(mac_monitor_module, OSD_MONITOR_PROVIDER, "sdl")
#endif

MODULE_DEFINITION(MONITOR_MAC, mac_monitor_module)
