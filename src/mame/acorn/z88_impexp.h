// license:BSD-3-Clause
// copyright-holders:Sandro Ronco

#ifndef MAME_ACORN_Z88_IMPEXP_H
#define MAME_ACORN_Z88_IMPEXP_H

#pragma once

#include "bus/rs232/rs232.h"
#include "diserial.h"

#include <queue>


class z88_impexp_device : public device_t,
						public device_serial_interface,
						public device_rs232_port_interface,
						public device_image_interface
{
public:
	z88_impexp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override { device_serial_interface::rx_w(state); }
	virtual void input_rts(int state) override;
	virtual void input_dtr(int state) override { m_dtr = state; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return ""; }
	virtual const char *image_type_name() const noexcept override { return "serial"; }
	virtual const char *image_brief_type_name() const noexcept override { return "serl"; }

	TIMER_CALLBACK_MEMBER(poll_queue);

private:
	void check_filename(std::string &filename);
	void queue();

	enum op_mode_t : uint8_t
	{
		MODE_IDLE,
		MODE_SEND,
		MODE_RECV,
	};

	emu_timer *         m_timer_poll;
	op_mode_t           m_mode;
	int                 m_rts;
	int                 m_dtr;
	std::queue<uint8_t> m_queue;
};

DECLARE_DEVICE_TYPE(Z88_IMPEXP, z88_impexp_device)

#endif // MAME_ACORN_Z88_IMPEXP_H
