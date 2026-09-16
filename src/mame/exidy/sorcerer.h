// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Robbbert
/*****************************************************************************
 *
 * includes/sorcerer.h
 *
 ****************************************************************************/
#ifndef MAME_EXIDY_SORCERER_H
#define MAME_EXIDY_SORCERER_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "micropolis.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"

#include "formats/sorc_cas.h"
#include "formats/sorc_dsk.h"


class sorcerer_state : public driver_device
{
	static constexpr XTAL ES_VIDEO_CLOCK = 12.638_MHz_XTAL;
	static constexpr XTAL ES_CPU_CLOCK = ES_VIDEO_CLOCK / 6;
	static constexpr XTAL ES_UART_CLOCK = ES_CPU_CLOCK / 440;

	struct cass_data_t {
		struct {
			int length = 0;     // time cassette level is at input.level
			int level = 0;      // cassette level
			int bit = 0;        // bit being read
		} input;
		struct {
			int length = 0;     // time cassette level is at output.level
			int level = 0;      // cassette level
			int bit = 0;        // bit to output
		} output;
	};

public:
	sorcerer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_pcg(*this, "pcg")
		, m_cassette1(*this, "cassette1")
		, m_cassette2(*this, "cassette2")
		, m_uart(*this, "uart")
		, m_uart_clock(*this, "uart_clock")
		, m_rs232(*this, "rs232")
		, m_centronics(*this, "centronics")
		, m_cart(*this, "cartslot")
		, m_ram(*this, RAM_TAG)
		, m_dma(*this, "dma")
		, m_fdc3(*this, "fdc3")
		, m_fdc4(*this, "fdc4")
		, m_floppy20(*this, "fdc2:0")
		, m_floppy21(*this, "fdc2:1")
		, m_floppy30(*this, "fdc3:0")
		, m_floppy31(*this, "fdc3:1")
		, m_floppy32(*this, "fdc3:2")
		, m_floppy33(*this, "fdc3:3")
		, m_floppy40(*this, "fdc4:0")
		, m_floppy41(*this, "fdc4:1")
		, m_floppy42(*this, "fdc4:2")
		, m_floppy43(*this, "fdc4:3")
		, m_iop_config(*this, "CONFIG")
		, m_iop_vs(*this, "VS")
		, m_iop_x(*this, "X.%u", 0U)
	{ }

	void sorcerer(machine_config &config);
	void sorcerera(machine_config &config);
	void sorcererb(machine_config &config);

protected:
	u8 portfd_r();
	u8 portfe_r();
	void portfd_w(u8 data);
	void portfe_w(u8 data);
	void portff_w(u8 data);
	TIMER_CALLBACK_MEMBER(cassette_tc);
	TIMER_CALLBACK_MEMBER(serial_tc);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	void machine_start_common(offs_t endmem);
	void machine_reset_common();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sorcerer_mem(address_map &map) ATTR_COLD;
	void sorcererb_mem(address_map &map) ATTR_COLD;
	void sorcerer_io(address_map &map) ATTR_COLD;
	void sorcerera_io(address_map &map) ATTR_COLD;
	void sorcererb_io(address_map &map) ATTR_COLD;

	u8 m_portfe = 0U;
	u8 m_keyboard_line = 0U;
	emu_timer *m_serial_timer = nullptr;
	emu_timer *m_cassette_timer = nullptr;
	cass_data_t m_cass_data;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_pcg;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<ay31015_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<rs232_port_device> m_rs232;
	required_device<centronics_device> m_centronics;
	optional_device<generic_slot_device> m_cart;
	required_device<ram_device> m_ram;
	optional_device<z80dma_device> m_dma;
	optional_device<fd1793_device> m_fdc3;
	optional_device<wd2793_device> m_fdc4;
	optional_device<floppy_connector> m_floppy20;
	optional_device<floppy_connector> m_floppy21;
	optional_device<floppy_connector> m_floppy30;
	optional_device<floppy_connector> m_floppy31;
	optional_device<floppy_connector> m_floppy32;
	optional_device<floppy_connector> m_floppy33;
	optional_device<floppy_connector> m_floppy40;
	optional_device<floppy_connector> m_floppy41;
	optional_device<floppy_connector> m_floppy42;
	optional_device<floppy_connector> m_floppy43;
	required_ioport m_iop_config;
	required_ioport m_iop_vs;
	required_ioport_array<16> m_iop_x;
	memory_passthrough_handler m_rom_shadow_tap;

private:
	u8 m_port48 = 0;
	u8 m_port34 = 0;
	u8 port34_r();
	u8 port48_r();
	void port34_w(u8 data);
	void port48_w(u8 data);
	void intrq4_w(bool state);
	bool m_halt = 0;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	u8 memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, u8 data);
	u8 io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, u8 data);
};

class sorcererd_state : public sorcerer_state
{
public:
	sorcererd_state(const machine_config &mconfig, device_type type, const char *tag)
		: sorcerer_state(mconfig, type, tag)
		, m_fdc(*this, "fdc")
		, m_fdc2(*this, "fdc2")
		{ }

	void sorcererd(machine_config &config);

private:
	void sorcererd_mem(address_map &map) ATTR_COLD;
	void sorcererd_io(address_map &map) ATTR_COLD;
	void port2c_w(u8 data);
	void intrq2_w(bool state);
	void drq2_w(bool state);
	u8 m_port2c = 0U;
	bool m_wait = false;
	bool m_drq_off = false;
	bool m_intrq_off = false;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	optional_device<micropolis_device> m_fdc;
	optional_device<fd1793_device> m_fdc2;
};


#endif // MAME_EXIDY_SORCERER_H
