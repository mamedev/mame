// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Aaron Giles, Olivier Galibert, R. Belmont
/*
* monitor_common.cpp
*
*/

#include "monitor_common.h"

#include "modules/lib/osdobj_common.h"
#include "modules/osdwindow.h"

#include "osdcore.h"

#include <algorithm>

std::shared_ptr<osd_monitor_info> monitor_module_base::pick_monitor(osd_options& options, int index)
{
	// get the aspect ratio
	float aspect = get_aspect(options.aspect(), options.aspect(index), true);

	auto monitor = pick_monitor_internal(options, index);
	if (aspect != 0)
	{
		monitor->set_aspect(aspect);
	}

	return monitor;
}

std::shared_ptr<osd_monitor_info> monitor_module_base::monitor_from_handle(std::uint64_t handle)
{
	if (!m_initialized)
		return nullptr;

	auto monitor = m_monitor_index[handle];

	// If we have been initialized, make sure we can find the monitor
	assert(monitor != nullptr);

	return monitor;
}

void monitor_module_base::add_monitor(std::shared_ptr<osd_monitor_info> monitor)
{
	list().push_back(monitor);
	m_monitor_index[monitor->oshandle()] = monitor;
}

std::shared_ptr<osd_monitor_info> monitor_module_base::pick_monitor_internal(osd_options& options, int index)
{
	std::string scrname, scrname2;

	// get the screen option
	scrname = options.screen();
	scrname2 = options.screen(index);

	// decide which one we want to use
	if (scrname2 != "auto")
		scrname = scrname2;

	// look for a match in the name first
	if (!scrname.empty())
	{
		auto mon = std::find_if(std::begin(list()), std::end(list()), [&scrname](auto m)
		{
			return m->devicename() == scrname;
		});

		if (mon != std::end(list()))
		{
			return *mon;
		}
	}

	// didn't find it; alternate monitors until we hit the jackpot
	// this allows for more screens than monitors but will put one on each monitor first
	index %= list().size();
	auto next_monitor = list()[index];
	return next_monitor;
}

float monitor_module_base::get_aspect(const char* defdata, const char* data, int report_error)
{
	int num = 0, den = 1;

	if (strcmp(data, OSDOPTVAL_AUTO) == 0)
	{
		if (strcmp(defdata, OSDOPTVAL_AUTO) == 0)
			return 0;
		data = defdata;
	}
	if (sscanf(data, "%d:%d", &num, &den) != 2 && report_error)
		osd_printf_error("Illegal aspect ratio value = %s\n", data);

	return float(num) / float(den);
}

int monitor_module_base::init(const osd_options& options)
{
	if (!m_initialized)
	{
		int result = init_internal(options);

		if (result == 0)
			m_initialized = true;

		return result;
	}

	return 0;
}

void monitor_module_base::exit()
{
	// free all of our monitor information
	list().clear();
	m_monitor_index.clear();
	m_initialized = false;
}
