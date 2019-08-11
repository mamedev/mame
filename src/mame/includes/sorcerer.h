// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Robbbert
/*****************************************************************************
 *
 * includes/sorcerer.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_SORCERER_H
#define MAME_INCLUDES_SORCERER_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "machine/ram.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "imagedev/floppy.h"
#include "formats/sorc_dsk.h"
#include "formats/sorc_cas.h"
#include "machine/micropolis.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define ES_CPU_CLOCK (12638000 / 6)
#define ES_UART_CLOCK (ES_CPU_CLOCK / 440)

struct cass_data_t {
	struct {
		int length;     // time cassette level is at input.level
		int level;      // cassette level
		int bit;        // bit being read
	} input;
	struct {
		int length;     // time cassette level is at output.level
		int level;      // cassette level
		int bit;        // bit to output
	} output;
};


class sorcerer_state : public driver_device
{
public:
	sorcerer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette1(*this, "cassette1")
		, m_cassette2(*this, "cassette2")
		, m_uart(*this, "uart")
		, m_uart_clock(*this, "uart_clock")
		, m_rs232(*this, "rs232")
		, m_centronics(*this, "centronics")
		, m_cart(*this, "cartslot")
		, m_ram(*this, RAM_TAG)
		, m_dma(*this, "dma")
		, m_fdc(*this, "fdc")
		, m_fdc2(*this, "fdc2")
		, m_fdc3(*this, "fdc3")
		, m_fdc4(*this, "fdc4")
		, m_floppy20(*this, "fdc2:0")
		, m_floppy21(*this, "fdc2:1")
		, m_floppy30(*this, "fdc3:0")
		, m_floppy31(*this, "fdc3:1")
		, m_floppy40(*this, "fdc4:0")
		, m_floppy41(*this, "fdc4:1")
		, m_iop_config(*this, "CONFIG")
		, m_iop_vs(*this, "VS")
		, m_iop_x(*this, "X.%u", 0)
	{ }

	void sorcerer(machine_config &config);
	void sorcerera(machine_config &config);
	void sorcererb(machine_config &config);
	void sorcererd(machine_config &config);

	void init_sorcerer();

private:
	enum
	{
		TIMER_SERIAL,
		TIMER_CASSETTE,
		TIMER_RESET
	};

	DECLARE_READ8_MEMBER(port_fd_r);
	DECLARE_READ8_MEMBER(port_fe_r);
	DECLARE_WRITE8_MEMBER(port2c_w);
	DECLARE_READ8_MEMBER(port34_r);
	DECLARE_WRITE8_MEMBER(port34_w);
	DECLARE_READ8_MEMBER(port48_r);
	DECLARE_WRITE8_MEMBER(port48_w);
	DECLARE_WRITE8_MEMBER(port_fd_w);
	DECLARE_WRITE8_MEMBER(port_fe_w);
	DECLARE_WRITE8_MEMBER(port_ff_w);
	DECLARE_WRITE_LINE_MEMBER(intrq2_w);
	DECLARE_WRITE_LINE_MEMBER(drq2_w);
	DECLARE_WRITE_LINE_MEMBER(intrq4_w);
	DECLARE_MACHINE_START(sorcererd);
	TIMER_CALLBACK_MEMBER(cassette_tc);
	TIMER_CALLBACK_MEMBER(serial_tc);
	TIMER_CALLBACK_MEMBER(sorcerer_reset);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	void machine_start_common(u16 endmem);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sorcerer_mem(address_map &map);
	void sorcererb_mem(address_map &map);
	void sorcererd_mem(address_map &map);
	void sorcerer_io(address_map &map);
	void sorcerera_io(address_map &map);
	void sorcererb_io(address_map &map);
	void sorcererd_io(address_map &map);

	bool m_wait;
	bool m_drq_off;
	bool m_intrq_off;
	bool m_halt;
	uint8_t m_2c;
	uint8_t m_fe;
	uint8_t m_keyboard_line;
	u8 m_port48;
	u8 m_port34;
	const uint8_t *m_p_videoram;
	emu_timer *m_serial_timer;
	emu_timer *m_cassette_timer;
	cass_data_t m_cass_data;
	virtual void video_start() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	required_device<z80_device> m_maincpu;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<ay31015_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<rs232_port_device> m_rs232;
	required_device<centronics_device> m_centronics;
	optional_device<generic_slot_device> m_cart;
	required_device<ram_device> m_ram;
	optional_device<z80dma_device> m_dma;
	optional_device<micropolis_device> m_fdc;
	optional_device<fd1793_device> m_fdc2;
	optional_device<fd1793_device> m_fdc3;
	optional_device<wd2793_device> m_fdc4;
	optional_device<floppy_connector> m_floppy20;
	optional_device<floppy_connector> m_floppy21;
	optional_device<floppy_connector> m_floppy30;
	optional_device<floppy_connector> m_floppy31;
	optional_device<floppy_connector> m_floppy40;
	optional_device<floppy_connector> m_floppy41;
	required_ioport m_iop_config;
	required_ioport m_iop_vs;
	required_ioport_array<16> m_iop_x;
};

#endif // MAME_INCLUDES_SORCERER_H
