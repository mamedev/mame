// license:BSD-3-Clause
// copyright-holders:Brian Johnson

#ifndef MAME_INCLUDES_NABUPC_H
#define MAME_INCLUDES_NABUPC_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "bus/nabupc/option.h"
#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "sound/ay8910.h"
#include "speaker.h"
#include "video/tms9928a.h"


class nabupc_state : public driver_device
{
public:
	nabupc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_tms9928a(*this, "tms9928a")
		, m_screen(*this, "screen")
		, m_ay8910(*this, "ay8910")
		, m_speaker(*this, "speaker")
		, m_kbduart(*this, "kbduart")
		, m_hccauart(*this, "hccauart")
		, m_ram(*this, RAM_TAG)
		, m_centronics(*this, "centronics")
		, m_bus(*this, "bus")
		, m_rom_base(*this, "bios")
		, m_bios_sel(*this, "CONFIG")
		, m_leds(*this, "led%u", 0U)
		, m_irq_in_prio(0xFF)
		, m_int_lines(0)
		, m_porta(0)
		, m_portb(0)
		, m_control(0)
	{
	}

	void nabupc(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void memory_map(address_map &map);
	void io_map(address_map &map);
	uint8_t read_mem(offs_t offset);

	uint8_t psg_portb_r();
	void psg_porta_w(uint8_t data);
	void control_w(uint8_t data);
	void centronics_busy_handler(uint8_t state);
	void update_irq();

	DECLARE_WRITE_LINE_MEMBER(hcca_fe_w);
	DECLARE_WRITE_LINE_MEMBER(hcca_oe_w);
	DECLARE_WRITE_LINE_MEMBER(rxrdy_w);
	DECLARE_WRITE_LINE_MEMBER(vdp_int_w);
	DECLARE_WRITE_LINE_MEMBER(hcca_dr_w);
	DECLARE_WRITE_LINE_MEMBER(hcca_tbre_w);
	DECLARE_WRITE_LINE_MEMBER(j9_int_w);
	DECLARE_WRITE_LINE_MEMBER(j10_int_w);
	DECLARE_WRITE_LINE_MEMBER(j11_int_w);
	DECLARE_WRITE_LINE_MEMBER(j12_int_w);

	IRQ_CALLBACK_MEMBER(int_ack_cb);

	required_device<z80_device> m_maincpu;
	required_device<tms9928a_device> m_tms9928a;
	required_device<screen_device> m_screen;
	required_device<ay8910_device> m_ay8910;
	required_device<speaker_device> m_speaker;
	required_device<i8251_device> m_kbduart;
	required_device<ay31015_device> m_hccauart;
	required_device<ram_device> m_ram;
	required_device<centronics_device> m_centronics;
	required_device<bus::nabupc::option_bus_device> m_bus;
	required_region_ptr<uint8_t> m_rom_base;
	required_ioport m_bios_sel;

	output_finder<4> m_leds;

	uint16_t m_irq_in_prio;
	uint8_t m_int_lines;
	uint8_t m_porta;
	uint8_t m_portb;
	uint8_t m_control;
	uint16_t m_bios_size;
};

#endif // MAME_INCLUDES_NABUPC_H
