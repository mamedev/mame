// license:BSD-3-Clause
// copyright-holders:Mietek Bak

#include "emu.h"

#include "simh_tape_image.h"

// #define VERBOSE         LOG_GENERAL
// #define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SIMH_TAPE_IMAGE, simh_tape_image_device, "simh_tape_image", "SIMH tape image");

//////////////////////////////////////////////////////////////////////////////

// construction

simh_tape_image_device::simh_tape_image_device(const machine_config &config, device_type type, const char *tag, device_t *owner, u32 clock)
	: microtape_image_device(config, type, tag, owner, clock)
	, m_file()
	, m_interface(nullptr)
{
}

simh_tape_image_device::simh_tape_image_device(const machine_config &config, const char *tag, device_t *owner, u32 clock)
	: simh_tape_image_device(config, SIMH_TAPE_IMAGE, tag, owner, clock)
{
}

//////////////////////////////////////////////////////////////////////////////

// device_image_interface implementation

std::pair<std::error_condition, std::string> simh_tape_image_device::call_load()
{
	LOG("simh_tape_image_device::call_load filename=%s\n", filename());
	try {
		m_file.reset(new simh_tape_file(image_core_file(), length(), is_readonly()));
	}
	catch (std::exception &e) { // error: loading tape failed
		return std::make_pair(image_error::INTERNAL, std::string(e.what()));
	}
	return std::make_pair(std::error_condition(), std::string());
}

std::pair<std::error_condition, std::string> simh_tape_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	LOG("simh_tape_image_device::call_create filename=%s format_type=%d\n", filename(), format_type);
	try {
		u64 file_size = 62914560; // we default to 60MB; TODO: allow specifying tape image size, once MAME supports it
		m_file.reset(new simh_tape_file(image_core_file(), file_size, is_readonly(), true));
	}
	catch (std::exception &e) { // error: creating tape failed
		return std::make_pair(image_error::INTERNAL, std::string(e.what()));
	}
	return std::make_pair(std::error_condition(), std::string());
}

void simh_tape_image_device::call_unload()
{
	LOG("simh_tape_image_device::call_unload\n");
	m_file.reset();
}

//////////////////////////////////////////////////////////////////////////////

// device_t implementation

void simh_tape_image_device::device_config_complete()
{
	LOG("simh_tape_image_device::device_config_complete\n");
	add_format(image_brief_type_name(), image_type_name(), file_extensions(), ""); // TODO: not sure these strings are correct
}

void simh_tape_image_device::device_start()
{
	LOG("simh_tape_image_device::device_start\n");
}

void simh_tape_image_device::device_stop()
{
	LOG("simh_tape_image_device::device_stop\n");
}

//////////////////////////////////////////////////////////////////////////////
