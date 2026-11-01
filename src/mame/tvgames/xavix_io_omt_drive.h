// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_OMT_DRIVE_H
#define MAME_TVGAMES_XAVIX_IO_OMT_DRIVE_H

#pragma once

#include "diimage.h"

#include <array>

class xavix_omt_drive_device : public device_t, public device_image_interface
{
public:

	xavix_omt_drive_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	int data_r() const { return m_data; }
	DECLARE_INPUT_CHANGED_MEMBER(send_changed);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_pre_save() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_type_name() const noexcept override { return "drive_packet"; }
	virtual const char *image_brief_type_name() const noexcept override { return "drive"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

private:
	enum : u8 { IDLE, HEADER_HIGH, HEADER_LOW, DATA_HIGH, DATA_LOW, TRAILER };
	static constexpr unsigned PACKET_BYTES = 25;

	void stop();
	TIMER_CALLBACK_MEMBER(pulse);

	emu_timer *m_timer;
	std::array<u8, PACKET_BYTES> m_image;
	std::array<u8, PACKET_BYTES> m_packet;
	std::array<u8, PACKET_BYTES> m_saved_image;
	bool m_loaded;
	bool m_saved_present;
	u8 m_phase;
	u8 m_bit;
	u8 m_data;
};

DECLARE_DEVICE_TYPE(XAVIX_OMT_DRIVE, xavix_omt_drive_device)

#endif // MAME_TVGAMES_XAVIX_IO_OMT_DRIVE_H
