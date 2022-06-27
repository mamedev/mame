// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/poly88.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_POLY88_H
#define MAME_INCLUDES_POLY88_H

#pragma once

#include "bus/s100/s100.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/mm5307.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/timer.h"

class poly88_state : public driver_device
{
public:
	poly88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_onboard_io(*this, "onboard_io")
		, m_s100(*this, "s100")
		, m_s100_slot(*this, "s100:%u", 1U)
		, m_usart(*this, "usart")
		, m_brg(*this, "brg")
		, m_cassette(*this, "cassette")
		, m_onboard_rom(*this, "maincpu")
		, m_linec(*this, "CONFIG")
		, m_onboard_config(*this, "ONBOARD")
	{ }

	void poly88(machine_config &config);
	void poly8813(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	bool is_onboard(offs_t offset);
	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	uint8_t in_r(offs_t offset);
	void out_w(offs_t offset, uint8_t data);
	void baud_rate_w(uint8_t data);
	void intr_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	DECLARE_WRITE_LINE_MEMBER(cassette_clock_w);
	TIMER_DEVICE_CALLBACK_MEMBER(rtc_tick);
	DECLARE_WRITE_LINE_MEMBER(vi2_w);
	DECLARE_WRITE_LINE_MEMBER(vi5_w);
	DECLARE_WRITE_LINE_MEMBER(usart_ready_w);
	IRQ_CALLBACK_MEMBER(poly88_irq_callback);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	void s100_mem(address_map &map);
	void s100_io(address_map &map);
	void poly88_mem(address_map &map);
	void poly88_io(address_map &map);
	void poly8813_mem(address_map &map);
	void poly8813_io(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_onboard_io;
	required_device<s100_bus_device> m_s100;
	optional_device_array<s100_slot_device, 9> m_s100_slot;
	required_device<i8251_device> m_usart;
	required_device<mm5307_device> m_brg;
	required_device<cassette_image_device> m_cassette;
	required_region_ptr<u8> m_onboard_rom;
	required_ioport m_linec;
	required_ioport m_onboard_config;

	uint8_t m_int_vector = 0;
	bool m_dtr = false, m_rts = false, m_txd = false, m_rxd = false, m_cassold = false, m_casspol = false;
	u8 m_cass_data[5] = {};
	std::unique_ptr<u8[]> m_onboard_ram;
	bool m_onboard_disable = false;
};

#endif // MAME_INCLUDES_POLY88_H
