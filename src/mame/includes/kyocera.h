// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __KYOCERA__
#define __KYOCERA__


#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "machine/im6402.h"
#include "machine/ram.h"
#include "machine/rp5c01.h"
#include "machine/upd1990a.h"
#include "video/hd44102.h"
#include "video/hd61830.h"
#include "sound/speaker.h"
#include "rendlay.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define SCREEN_TAG      "screen"
#define I8085_TAG       "m19"
#define I8155_TAG       "m25"
#define UPD1990A_TAG    "m18"
#define IM6402_TAG      "m22"
#define MC14412_TAG     "m31"
#define HD44102_0_TAG   "m1"
#define HD44102_1_TAG   "m2"
#define HD44102_2_TAG   "m3"
#define HD44102_3_TAG   "m4"
#define HD44102_4_TAG   "m5"
#define HD44102_5_TAG   "m6"
#define HD44102_6_TAG   "m7"
#define HD44102_7_TAG   "m8"
#define HD44102_8_TAG   "m9"
#define HD44102_9_TAG   "m10"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"

//#define I8085_TAG     "m19"
//#define I8155_TAG     "m12"
//#define MC14412_TAG   "m8"
#define RP5C01A_TAG     "m301"
#define TCM5089_TAG     "m11"
#define I8251_TAG       "m20"
#define HD61830_TAG     "m18"

class kc85_state : public driver_device
{
public:
	kc85_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8085_TAG),
		m_rtc(*this, UPD1990A_TAG),
		m_uart(*this, IM6402_TAG),
		m_lcdc0(*this, HD44102_0_TAG),
		m_lcdc1(*this, HD44102_1_TAG),
		m_lcdc2(*this, HD44102_2_TAG),
		m_lcdc3(*this, HD44102_3_TAG),
		m_lcdc4(*this, HD44102_4_TAG),
		m_lcdc5(*this, HD44102_5_TAG),
		m_lcdc6(*this, HD44102_6_TAG),
		m_lcdc7(*this, HD44102_7_TAG),
		m_lcdc8(*this, HD44102_8_TAG),
		m_lcdc9(*this, HD44102_9_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_opt_cart(*this, "opt_cartslot"),
		m_ram(*this, RAM_TAG),
		m_rs232(*this, RS232_TAG),
		m_rom(*this, I8085_TAG),
		m_y(*this, "Y%u", 0),
		m_battery(*this, "BATTERY")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;
	optional_device<im6402_device> m_uart;
	required_device<hd44102_device> m_lcdc0;
	required_device<hd44102_device> m_lcdc1;
	required_device<hd44102_device> m_lcdc2;
	required_device<hd44102_device> m_lcdc3;
	required_device<hd44102_device> m_lcdc4;
	required_device<hd44102_device> m_lcdc5;
	required_device<hd44102_device> m_lcdc6;
	required_device<hd44102_device> m_lcdc7;
	required_device<hd44102_device> m_lcdc8;
	required_device<hd44102_device> m_lcdc9;
	required_device<centronics_device> m_centronics;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_opt_cart;
	required_device<ram_device> m_ram;
	required_device<rs232_port_device> m_rs232;
	required_memory_region m_rom;
	required_ioport_array<9> m_y;
	required_ioport m_battery;

	virtual void machine_start() override;
	memory_region *m_opt_region;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t uart_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void uart_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void modem_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lcd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lcd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8155_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8155_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8155_pc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8155_to_w(int state);
	void write_centronics_busy(int state);
	void write_centronics_select(int state);

	/* memory state */
	uint8_t m_bank;           /* memory bank selection */

	/* keyboard state */
	uint16_t m_keylatch;      /* keyboard latch */

	/* sound state */
	int m_buzzer;               /* buzzer select */
	int m_bell;             /* bell output */

	int m_centronics_busy;
	int m_centronics_select;

	void palette_init_kc85(palette_device &palette);
	void kc85_sod_w(int state);
	int kc85_sid_r();
};

class trsm100_state : public kc85_state
{
public:
	trsm100_state(const machine_config &mconfig, device_type type, const char *tag)
		: kc85_state(mconfig, type, tag) { }

	virtual void machine_start() override;
};

class pc8201_state : public kc85_state
{
public:
	pc8201_state(const machine_config &mconfig, device_type type, const char *tag)
		: kc85_state(mconfig, type, tag),
			m_cas_cart(*this, "cas_cartslot")
	{ }

	virtual void machine_start() override;
	required_device<generic_slot_device> m_cas_cart;

	uint8_t bank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t uart_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void romah_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void romal_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void romam_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t romrd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void bankswitch(uint8_t data);

	// ROM cassette
	int m_rom_sel;
	uint32_t m_rom_addr;

	/* peripheral state */
	int m_iosel;                /* serial interface select */
};

class tandy200_state : public driver_device
{
public:
	tandy200_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8085_TAG),
		m_rtc(*this, RP5C01A_TAG),
		m_lcdc(*this, HD61830_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_cent_data_out(*this, "cent_data_out"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_opt_cart(*this, "opt_cartslot"),
		m_ram(*this, RAM_TAG),
		m_rs232(*this, RS232_TAG),
		m_rom(*this, I8085_TAG),
		m_y(*this, "Y%u", 0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<rp5c01_device> m_rtc;
	required_device<hd61830_device> m_lcdc;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_opt_cart;
	required_device<ram_device> m_ram;
	required_device<rs232_port_device> m_rs232;
	required_memory_region m_rom;
	required_ioport_array<9> m_y;

	virtual void machine_start() override;
	memory_region *m_opt_region;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t bank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t stbk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void stbk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8155_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8155_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8155_pc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8155_to_w(int state);
	void kc85_sod_w(int state);
	int kc85_sid_r();
	void write_centronics_busy(int state);
	void write_centronics_select(int state);

	void palette_init_tandy200(palette_device &palette);

	void tandy200_tp_tick(timer_device &timer, void *ptr, int32_t param);

	void bankswitch(uint8_t data);

	/* memory state */
	uint8_t m_bank;           /* memory bank selection */

	/* keyboard state */
	uint16_t m_keylatch;      /* keyboard latch */
	int m_tp;               /* timing pulse */

	/* sound state */
	int m_buzzer;           /* buzzer select */
	int m_bell;             /* bell output */

	int m_centronics_busy;
	int m_centronics_select;
};

/* ---------- defined in video/kyocera.c ---------- */

MACHINE_CONFIG_EXTERN( kc85_video );
MACHINE_CONFIG_EXTERN( tandy200_video );

#endif
