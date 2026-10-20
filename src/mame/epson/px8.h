// license:GPL-2.0+
// copyright-holders:Curt Coder,Dirk Best
#ifndef MAME_EPSON_PX8_H
#define MAME_EPSON_PX8_H

#pragma once


#include "cpu/z80/z80.h"
#include "cpu/m6800/m6801.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "video/sed1330.h"
#include "screen.h"
#include "bus/epson_sio/pf10.h"
#include "emupal.h"

#include <deque>
#include <map>
#include <utility>
#include <vector>

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define UPD70008_TAG    "4a"
#define UPD7508_TAG     "2e"
#define HD6303_TAG      "13d"
#define SED1320_TAG     "7c"
#define I8251_TAG       "13e"
#define UPD7001_TAG     "1d"
#define SCREEN_TAG      "screen"

#define PX8_VIDEORAM_MASK   0x17ff

/* interrupt sources */
#define INT0_7508       0x01
#define INT1_SERIAL     0x02
#define INT2_RS232      0x04
#define INT3_BARCODE    0x08
#define INT4_FRC        0x10
#define INT5_OPTION     0x20

class px8_state : public driver_device
{
public:
	px8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, UPD70008_TAG)
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_screen(*this, SCREEN_TAG)
		, m_lcdc(*this, SED1320_TAG)
		, m_font(*this, "font")
		, m_capsule(*this, "capsule%u", 1U)
		, m_ksc_io(*this, "KSC%u", 0U)
		, m_sw4(*this, "SW4")
		, m_leds(*this, "led_%u", 0U)
	{ }

	void px8(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<screen_device> m_screen;
	required_device<sed1330_device> m_lcdc;
	required_region_ptr<uint8_t> m_font;
	required_device_array<generic_slot_device, 2> m_capsule;
	required_ioport_array<9> m_ksc_io;
	required_ioport m_sw4;
	output_finder<3> m_leds;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void lcdc_init();
	void lcdc_update(int state);

	uint8_t gah40m_r(offs_t offset);
	void gah40m_w(offs_t offset, uint8_t data);
	uint8_t gah40s_r(offs_t offset);
	void gah40s_w(offs_t offset, uint8_t data);
	void gah40s_ier_w(uint8_t data);
	uint8_t krtn_0_3_r();
	uint8_t krtn_4_7_r();
	void ksc_w(uint8_t data);

	void bankswitch();
	uint8_t krtn_read();

	void update_interrupt();
	int irq_ack(device_t &device, int irqline);
	TIMER_CALLBACK_MEMBER(frc_tick);

	void sub_init();
	void sub_handshake();
	void sub_command(uint8_t data);
	void sub_raise_interrupt();
	void sub_key(uint8_t code);
	void sub_rtc_tick();
	TIMER_CALLBACK_MEMBER(sub_second_tick);
	TIMER_CALLBACK_MEMBER(sub_keyboard_scan);

	uint8_t slave_status_r();
	uint8_t slave_data_r();
	void slave_data_w(uint8_t data);
	void slave_cmd_w(uint8_t data);
	void slave_execute();
	void slave_return(uint8_t code);
	void slave_data(uint8_t data);
	const uint8_t *slave_glyph(uint8_t code);
	bool slave_get_point(int x, int y);
	void slave_set_point(int x, int y, uint8_t op);

	/* GAH40M state */
	uint16_t m_icr = 0;               // input capture register
	uint8_t m_ier = 0;                // interrupt acknowledge register
	uint8_t m_isr = 0;                // interrupt status register
	uint8_t m_sio = 0;                // serial I/O register
	int m_bank0 = 0;

	/* GAH40S state */
	uint16_t m_cnt = 0;               // microcassette tape counter
	int m_swpr = 0;                   // P-ROM power switch
	uint16_t m_pra = 0;               // P-ROM address
	uint8_t m_prd = 0;                // P-ROM data

	/* memory state */
	int m_bk2 = 0;

	/* keyboard state */
	int m_ksc = 0;              // keyboard scan column

	emu_timer *m_frc_timer = nullptr;
	emu_timer *m_sub_second_timer = nullptr;
	emu_timer *m_sub_keyboard_timer = nullptr;
	attotime m_frc_start;
	bool m_rdysio = true;

	/* uPD7508 HLE state */
	bool m_sub_cold = true;
	std::vector<uint8_t> m_sub_cmd;
	std::deque<uint8_t> m_sub_rsp;
	uint8_t m_sub_status = 0;
	std::deque<uint8_t> m_key_buf;
	uint8_t m_key_state[9];
	uint8_t m_repeat_code = 0xff;
	attotime m_repeat_next;
	uint8_t m_repeat_start = 0;
	uint8_t m_repeat_interval = 0;
	bool m_repeat_enb = true;
	bool m_key_intr_enb = true;
	bool m_second_intr_enb = false;
	bool m_alarm_intr_enb = false;
	uint8_t m_alarm[6];
	uint8_t m_rtc[7];
	uint8_t m_power_fail_voltage = 0;
	uint8_t m_full_charge_voltage = 0;

	/* HD6303 HLE state */
	std::unique_ptr<uint8_t[]> m_sram;
	uint8_t m_slave_cmd = 0;
	std::vector<uint8_t> m_slave_buf;
	std::deque<std::pair<uint8_t, bool>> m_slave_rsp;
	std::map<uint8_t, std::vector<uint8_t>> m_gudc;
	uint16_t m_cs_addr = 0;
	uint16_t m_gs_addr = 0;
	uint16_t m_scr_ptr = 0;
	uint8_t m_udc_start = 0;
	bool m_lcd_on = true;
	bool m_char_mode = true;
	bool m_seven_lines = false;
	uint8_t m_curs_mode = 0;
	uint8_t m_curs_x = 0;
	uint8_t m_curs_y = 0;
	uint8_t m_wnd_x = 0;
	uint8_t m_wnd_y = 0;
	uint8_t m_flash = 0;
	bool m_mct_protect = false;
	uint16_t m_mct_counter = 0;
	bool m_prom_power = false;
	bool m_lcd_on_shadow = true;
	uint8_t m_lcd_shadow[60 * 64];

	void px8_palette(palette_device &palette) const;
	void px8_io(address_map &map) ATTR_COLD;
	void px8_mem(address_map &map) ATTR_COLD;
	void px8_slave_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_EPSON_PX8_H
