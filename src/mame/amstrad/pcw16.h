// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/pcw16.h
 *
 ****************************************************************************/
#ifndef MAME_AMSTRAD_PCW16_H
#define MAME_AMSTRAD_PCW16_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/upd765.h"     /* FDC superio */
#include "machine/pc_lpt.h"     /* PC-Parallel Port */
#include "machine/pckeybrd.h"   /* PC-AT keyboard */
#include "machine/upd765.h"     /* FDC superio */
#include "machine/ins8250.h"    /* pc com port */
#include "sound/beep.h"         /* pcw/pcw16 beeper */
#include "machine/intelfsh.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "emupal.h"

#define PCW16_BORDER_HEIGHT 8
#define PCW16_BORDER_WIDTH 8
#define PCW16_NUM_COLOURS 32
#define PCW16_DISPLAY_WIDTH 640
#define PCW16_DISPLAY_HEIGHT 480

#define PCW16_SCREEN_WIDTH  (PCW16_DISPLAY_WIDTH + (PCW16_BORDER_WIDTH<<1))
#define PCW16_SCREEN_HEIGHT (PCW16_DISPLAY_HEIGHT  + (PCW16_BORDER_HEIGHT<<1))


class pcw16_state : public driver_device
{
public:
	pcw16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_flash0(*this, "flash0"),
		m_flash1(*this, "flash1"),
		m_fdc(*this, "fdc"),
		m_uart2(*this, "ns16550_2"),
		m_beeper(*this, "beeper"),
		m_ram(*this, RAM_TAG),
		m_keyboard(*this, "at_keyboard"),
		m_region_rom(*this, "maincpu"),
		m_io_extra(*this, "EXTRA")
	{ }

	void pcw16(machine_config &config);

protected:
	void pcw16_palette_w(offs_t offset, uint8_t data);
	uint8_t pcw16_bankhw_r(offs_t offset);
	void pcw16_bankhw_w(offs_t offset, uint8_t data);
	void pcw16_video_control_w(uint8_t data);
	uint8_t pcw16_keyboard_data_shift_r();
	void pcw16_keyboard_data_shift_w(uint8_t data);
	uint8_t pcw16_keyboard_status_r();
	void pcw16_keyboard_control_w(uint8_t data);
	uint8_t rtc_year_invalid_r();
	uint8_t rtc_month_r();
	uint8_t rtc_days_r();
	uint8_t rtc_hours_r();
	uint8_t rtc_minutes_r();
	uint8_t rtc_seconds_r();
	uint8_t rtc_256ths_seconds_r();
	void rtc_control_w(uint8_t data);
	void rtc_seconds_w(uint8_t data);
	void rtc_minutes_w(uint8_t data);
	void rtc_hours_w(uint8_t data);
	void rtc_days_w(uint8_t data);
	void rtc_month_w(uint8_t data);
	void rtc_year_w(uint8_t data);
	uint8_t pcw16_system_status_r();
	uint8_t pcw16_timer_interrupt_counter_r();
	void pcw16_system_control_w(uint8_t data);
	uint8_t pcw16_mem_r(offs_t offset);
	void pcw16_mem_w(offs_t offset, uint8_t data);
	void pcw16_keyboard_init();
	void pcw16_keyboard_refresh_outputs();
	void pcw16_keyboard_set_clock_state(int state);
	void pcw16_keyboard_int(int state);
	void pcw16_keyboard_reset();
	int pcw16_keyboard_can_transmit();
	void pcw16_keyboard_signal_byte_received(int data);
	void pcw16_refresh_ints();
	void rtc_setup_max_days();
	uint8_t pcw16_read_mem(uint8_t bank, uint16_t offset);
	void pcw16_write_mem(uint8_t bank, uint16_t offset, uint8_t data);
	uint8_t read_bank_data(uint8_t type, uint16_t offset);
	void write_bank_data(uint8_t type, uint16_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void pcw16_colours(palette_device &palette) const;
	uint32_t screen_update_pcw16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(pcw16_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(rtc_timer_callback);
	void pcw16_com_interrupt_1(int state);
	void pcw16_com_interrupt_2(int state);
	void pcw16_keyboard_callback(int state);

	void trigger_fdc_int();
	void fdc_interrupt(int state);
	inline void pcw16_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);
	void pcw16_vh_decode_mode0(bitmap_ind16 &bitmap, int x, int y, uint8_t byte);
	void pcw16_vh_decode_mode1(bitmap_ind16 &bitmap, int x, int y, uint8_t byte);
	void pcw16_vh_decode_mode2(bitmap_ind16 &bitmap, int x, int y, uint8_t byte);

	void pcw16_io(address_map &map) ATTR_COLD;
	void pcw16_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<intel_e28f008sa_device> m_flash0;
	required_device<intel_e28f008sa_device> m_flash1;
	required_device<pc_fdc_superio_device> m_fdc;
	required_device<ns16550_device> m_uart2;
	required_device<beep_device> m_beeper;
	required_device<ram_device> m_ram;
	required_device<at_keyboard_device> m_keyboard;
	required_memory_region m_region_rom;
	required_ioport m_io_extra;

	unsigned long m_interrupt_counter = 0;
	int m_banks[4]{};
	int m_4_bit_port = 0;
	int m_fdc_int_code = 0;
	int m_system_status = 0;
	char *m_mem_ptr[4]{};
	unsigned char m_keyboard_data_shift = 0;
	int m_keyboard_parity_table[256]{};
	int m_keyboard_bits = 0;
	int m_keyboard_bits_output = 0;
	int m_keyboard_state = 0;
	int m_keyboard_previous_state = 0;
	unsigned char m_rtc_seconds = 0;
	unsigned char m_rtc_minutes = 0;
	unsigned char m_rtc_hours = 0;
	unsigned char m_rtc_days_max = 0;
	unsigned char m_rtc_days = 0;
	unsigned char m_rtc_months = 0;
	unsigned char m_rtc_years = 0;
	unsigned char m_rtc_control = 0;
	unsigned char m_rtc_256ths_seconds = 0;
	int m_previous_fdc_int_state = 0;
	int m_colour_palette[16]{};
	int m_video_control = 0;
};

#endif // MAME_AMSTRAD_PCW16_H
