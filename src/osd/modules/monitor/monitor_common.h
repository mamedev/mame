// license:BSD-3-Clause
// copyright-holders:Brad Hughes
/*
* monitor_common.h
*
*/
#ifndef MAME_OSD_MONITOR_MONITOR_COMMON_H
#define MAME_OSD_MONITOR_MONITOR_COMMON_H

#pragma once

#include "monitor_module.h"

#include <cstdint>
#include <map>
#include <memory>


//============================================================
//  monitor_module_base
//============================================================

class monitor_module_base : public monitor_module
{
public:
	monitor_module_base(const char* type, const char* name) :
		monitor_module(type, name),
		m_initialized(false)
	{
	}

	std::shared_ptr<osd_monitor_info> pick_monitor(osd_options& options, int index) override;
	std::shared_ptr<osd_monitor_info> monitor_from_handle(std::uint64_t handle) override;

	int init(osd_interface &osd, const osd_options& options) override;
	void exit() override;

protected:
	virtual int init_internal(const osd_options &options) = 0;
	void add_monitor(std::shared_ptr<osd_monitor_info> monitor);

	bool m_initialized;

private:
	static float get_aspect(const char *defdata, const char *data, int report_error);

	std::map<std::uint64_t, std::shared_ptr<osd_monitor_info> > m_monitor_index;
	std::shared_ptr<osd_monitor_info> pick_monitor_internal(osd_options& options, int index);
};

#endif // MAME_OSD_MONITOR_MONITOR_COMMON_H
