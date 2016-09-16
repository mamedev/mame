// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Aaron Giles, Olivier Galibert, R. Belmont
/*
 * monitor_module.h
 *
 */

#ifndef MAME_OSD_MODULES_MONITOR_MONITORMODULE_H
#define MAME_OSD_MODULES_MONITOR_MONITORMODULE_H

#include <vector>

#include "modules/osdmodule.h"
#include "modules/osdhelper.h"

//============================================================
//  CONSTANTS
//============================================================

#define OSD_MONITOR_PROVIDER   "monitorprovider"

class monitor_module;
class osd_window;

//============================================================
//  osd_monitor_info
//============================================================

class osd_monitor_info
{
public:
	osd_monitor_info(monitor_module &module, std::uint64_t handle, const char *monitor_device, float aspect)
		: m_is_primary(false), m_name(monitor_device), m_module(module), m_handle(handle), m_aspect(aspect)
	{
	}

	virtual ~osd_monitor_info() { }

	virtual void refresh() = 0;

	std::uint64_t oshandle() const { return m_handle; }
	monitor_module& module() const { return m_module; }

	const osd_rect &position_size() const { return m_pos_size; }
	const osd_rect &usuable_position_size() const { return m_usuable_pos_size; }

	const std::string &devicename() const
	{
		static std::string s_unknown = std::string("UNKNOWN");
		return m_name.length() ? m_name : s_unknown;
	}

	float aspect() const { return m_aspect; }
	float pixel_aspect() const { return m_aspect / (float(m_pos_size.width()) / float(m_pos_size.height())); }

	void update_resolution(const int new_width, const int new_height) const { m_pos_size.resize(new_width, new_height); }
	void set_aspect(const float a) { m_aspect = a; }
	bool is_primary() const { return m_is_primary; }

protected:
	osd_rect            m_pos_size;
	osd_rect            m_usuable_pos_size;
	bool                m_is_primary;
	std::string         m_name;
private:
	monitor_module&     m_module;
	std::uint64_t       m_handle;                 // handle to the monitor
	float               m_aspect;                 // computed/configured aspect ratio of the physical device
};

//============================================================
//  monitor_module
//============================================================

class monitor_module : public osd_module
{
private:
	std::vector<std::shared_ptr<osd_monitor_info>> m_list;

public:
	monitor_module(const char *type, const char *name)
		: osd_module(type, name)
	{
	}

	virtual ~monitor_module() { }

	std::vector<std::shared_ptr<osd_monitor_info>> & list() { return m_list; }

	virtual std::shared_ptr<osd_monitor_info> monitor_from_handle(std::uint64_t handle) = 0;
	virtual std::shared_ptr<osd_monitor_info> monitor_from_rect(const osd_rect &rect) = 0;
	virtual std::shared_ptr<osd_monitor_info> monitor_from_window(const osd_window &window) = 0;

	virtual std::shared_ptr<osd_monitor_info> pick_monitor(osd_options &options, int index) = 0;
};

#endif // MAME_OSD_MODULES_MONITOR_MONITORMODULE_H
