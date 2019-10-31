// license:BSD-3-Clause
// copyright-holders:AJR,68bit

#ifndef MAME_MACHINE_SWTPC8212_H
#define MAME_MACHINE_SWTPC8212_H

#pragma once

#include "cpu/m6800/m6800.h"
#include "imagedev/printer.h"
#include "machine/6821pia.h"
#include "machine/ins8250.h"
#include "machine/terminal.h"
#include "video/mc6845.h"

INPUT_PORTS_EXTERN(swtpc8212);


class swtpc8212_device : public device_t
{
public:
	swtpc8212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(keypad_changed);

	// Interface to a RS232 connection.
	auto rs232_conn_txd_handler() { return m_rs232_conn_txd_handler.bind(); }
	auto rs232_conn_dtr_handler() { return m_rs232_conn_dtr_handler.bind(); }
	auto rs232_conn_rts_handler() { return m_rs232_conn_rts_handler.bind(); }
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_dcd_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_dsr_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_ri_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_cts_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_conn_rxd_w);

protected:
	swtpc8212_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	enum
	{
		BELL_TIMER_ID = 1
	};

	void mem_map(address_map &map);

	required_device<m6802_cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<ins8250_device> m_uart;
	required_device<mc6845_device> m_crtc;
	required_region_ptr<u8> m_chargen1;
	required_region_ptr<u8> m_chargen2;
	required_shared_ptr<u8> m_video_ram;
	required_ioport m_config;
	required_ioport m_keypad;
	required_ioport m_one_stop_bit;
	emu_timer *m_bell_timer;
	required_device<beep_device> m_beeper;
	required_device<printer_image_device> m_printer;

	devcb_write_line m_rs232_conn_txd_handler;
	devcb_write_line m_rs232_conn_dtr_handler;
	devcb_write_line m_rs232_conn_rts_handler;

	MC6845_UPDATE_ROW(update_row);

	DECLARE_WRITE8_MEMBER(latch_w);
	uint8_t m_latch_data;

	DECLARE_READ8_MEMBER(pia0_pa_r);
	DECLARE_READ8_MEMBER(pia0_pb_r);
	DECLARE_WRITE_LINE_MEMBER(pia0_ca2_w);

	DECLARE_WRITE8_MEMBER(pia1_pa_w);
	DECLARE_READ_LINE_MEMBER(pia1_ca1_r);
	DECLARE_WRITE_LINE_MEMBER(pia1_ca2_w);

	uint8_t m_keyboard_data;
	uint8_t m_keypad_data;
	uint8_t m_printer_data;
	uint8_t m_printer_data_ready;

	void keyboard_put(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(write_txd);
	DECLARE_WRITE_LINE_MEMBER(write_dtr);
	DECLARE_WRITE_LINE_MEMBER(write_rts);
};

DECLARE_DEVICE_TYPE(SWTPC8212, swtpc8212_device)

#endif // MAME_MACHINE_SWTPC8212_H
