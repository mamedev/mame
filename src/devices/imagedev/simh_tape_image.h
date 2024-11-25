// license:BSD-3-Clause
// copyright-holders:Mietek Bak

#ifndef MAME_DEVICES_IMAGEDEV_SIMH_TAPE_IMAGE_H
#define MAME_DEVICES_IMAGEDEV_SIMH_TAPE_IMAGE_H

#pragma once

#include "magtape.h"

#include "util/simh_tape_file.h"

DECLARE_DEVICE_TYPE(SIMH_TAPE_IMAGE, simh_tape_image_device);

//////////////////////////////////////////////////////////////////////////////

class simh_tape_image_device : public microtape_image_device
{
public:
	// construction
	simh_tape_image_device(const machine_config &config, const char *tag, device_t *owner, u32 clock = 0);

	// device_image_interface implementation
	virtual const char *image_interface() const noexcept override { return m_interface; }
	virtual const char *file_extensions() const noexcept override { return "tap"; }
	virtual const char *image_type_name() const noexcept override { return "tape"; }
	virtual const char *image_brief_type_name() const noexcept override { return "tap"; }
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	// miscellaneous
	void set_interface(const char *interface) { m_interface = interface; }
	simh_tape_file *get_file() const { return m_file.get(); }

protected:
	// construction
	simh_tape_image_device(const machine_config &config, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// state
	std::unique_ptr<simh_tape_file> m_file; // tape image file
	const char *m_interface; // image interface
};

//////////////////////////////////////////////////////////////////////////////

#endif // MAME_DEVICES_IMAGEDEV_SIMH_TAPE_IMAGE_H
