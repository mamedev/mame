// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TIPI adapter card for the Peripheral Expansion Box
    See tipi.cpp for documentation

    Michael Zapf
    February 2022

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_TIPI_H
#define MAME_BUS_TI99_PEB_TIPI_H

#pragma once

#include "peribox.h"
#include "client_ws.hpp"

#include <queue>
#include <mutex>

namespace bus::ti99::peb {

class tipi_attached_device;

class tipi_card_device : public device_t, public device_ti99_peribox_card_interface
{

public:
	tipi_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

private:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_stop() override;
	ioport_constructor device_input_ports() const override;
	const tiny_rom_entry *device_rom_region() const override;
	void device_add_mconfig(machine_config &config) override;

	void debug_read(offs_t offset, uint8_t* value);
	void debug_write(offs_t offset, uint8_t data);

	void websocket_opened();
	void websocket_incoming(std::shared_ptr<webpp::ws_client::Message> message);
	void websocket_error(const std::error_code& code);
	void websocket_closed(int i, const std::string& msg);
	void websocket_debug(const char* msg, int i);

	TIMER_CALLBACK_MEMBER(open_websocket);

	void send(const char* message);
	void send(u8* message, int len);
	u8 get_rd();
	void set_tc(u8 data);

	required_device<tipi_attached_device> m_rpi;

	int m_address;
	bool m_dsr;
	bool m_portaccess;
	bool m_waitinit;
	bool m_syncmode;

	// DSR ROM
	uint8_t* m_eprom;

	// Websocket support
	std::unique_ptr<webpp::ws_client> m_wsclient;
	std::shared_ptr<webpp::ws_client::SendStream> m_send_stream;
	std::unique_ptr<u8[]> m_rpimessage;
	int m_msgindex;
	int m_msglength;
	emu_timer* m_restart_timer;
	int m_attempts;
	bool m_connected;
	bool m_rpiconn;
	int m_pausetime;

	// Incoming queue
	std::queue<u8> m_indqueue;
	std::mutex m_mutex;
	bool m_pending_read;

	// Computer interface
	u8 m_tc;
	u8 m_td;
	u8 m_rc;
	u8 m_rd;

	u8 m_lasttc;
};

/*
    Defines the connection to the RPi.
*/
class tipi_attached_device : public device_t, public device_image_interface
{
public:
	tipi_attached_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool is_readable()  const noexcept override           { return true; }
	bool is_writeable() const noexcept override           { return true; }
	bool is_creatable() const noexcept override           { return true; }
	bool is_reset_on_load() const noexcept override       { return false; }
	bool support_command_line_image_creation() const noexcept override { return true; }
	const char *image_type_name() const noexcept override { return "connect"; }
	const char *image_brief_type_name() const noexcept override { return "conn"; }
	const char *image_interface() const noexcept override { return ""; }
	const char *file_extensions() const noexcept override { return ""; }

protected:
	void device_start() override { }
	void call_unload() override;
	std::pair<std::error_condition, std::string> call_load() override;
};


} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_TIPI_RPI, bus::ti99::peb, tipi_attached_device)
DECLARE_DEVICE_TYPE_NS(TI99_TIPI, bus::ti99::peb, tipi_card_device)

#endif // MAME_BUS_TI99_PEB_TIPI_H
