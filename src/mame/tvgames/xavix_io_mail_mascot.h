// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_MAIL_MASCOT_H
#define MAME_TVGAMES_XAVIX_IO_MAIL_MASCOT_H

#pragma once

#include "imagedev/memcard.h"
#include "machine/i2cmem.h"

#include <string>
#include <system_error>
#include <utility>

DECLARE_DEVICE_TYPE(XAVIX_MAILCOT_MASCOT, xavix_mailcot_mascot_device)

class xavix_mailcot_mascot_device : public i2cmem_device, public device_memcard_image_interface
{
public:
	xavix_mailcot_mascot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	int sda_r() { return m_loaded ? read_sda() : 1; }
	u8 id_r() { return m_loaded ? m_mounted_character & 3 : 3; }
	void write(int sda, int scl);
	DECLARE_INPUT_CHANGED_MEMBER(character_changed);

	// device_image_interface implementation
	virtual bool is_reset_on_load() const noexcept override;
	virtual const char *image_type_name() const noexcept override;
	virtual const char *image_brief_type_name() const noexcept override;
	virtual const char *file_extensions() const noexcept override;
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_pre_save() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;

private:
	static int character_capacity(u8 character);
	void set_capacity(int size);
	void insert();

	required_ioport m_id;
	bool m_loaded;
	bool m_pending_create;
	u8 m_mounted_character;
	bool m_saved_present;
	u8 m_saved_character;
	bool m_host_sda;
	bool m_host_scl;
};

#endif // MAME_TVGAMES_XAVIX_IO_MAIL_MASCOT_H
