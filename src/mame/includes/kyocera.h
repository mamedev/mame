// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_KYOCERA_H
#define MAME_INCLUDES_KYOCERA_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/buffer.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "machine/im6402.h"
#include "machine/ram.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"
#include "machine/upd1990a.h"
#include "sound/spkrdev.h"
#include "video/hd44102.h"
#include "video/hd61830.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

#include "emupal.h"
#include "rendlay.h"


#define SCREEN_TAG      "screen"
#define I8085_TAG       "m19"
#define I8155_TAG       "m25"
#define UPD1990A_TAG    "m18"
#define IM6402_TAG      "m22"
#define MC14412_TAG     "m31"
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
		m_lcdc(*this, "m%u", 0U),
		m_centronics(*this, CENTRONICS_TAG),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_opt_cart(*this, "opt_cartslot"),
		m_ram(*this, RAM_TAG),
		m_rs232(*this, RS232_TAG),
		m_rom(*this, I8085_TAG),
		m_y(*this, "Y%u", 0U),
		m_battery(*this, "BATTERY"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2")
	{ }

	void kc85(machine_config &config);
	void kc85_video(machine_config &config);

protected:
	DECLARE_WRITE_LINE_MEMBER(kc85_sod_w);
	DECLARE_READ_LINE_MEMBER(kc85_sid_r);

	void i8155_pa_w(uint8_t data);
	void i8155_pb_w(uint8_t data);
	uint8_t i8155_pc_r();

	DECLARE_WRITE_LINE_MEMBER( i8155_to_w );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_select );

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;
	optional_device<im6402_device> m_uart;
	required_device_array<hd44102_device, 10> m_lcdc;
	required_device<centronics_device> m_centronics;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_opt_cart;
	required_device<ram_device> m_ram;
	required_device<rs232_port_device> m_rs232;
	required_memory_region m_rom;
	required_ioport_array<9> m_y;
	required_ioport m_battery;
	memory_bank_creator m_bank1;
	memory_bank_creator m_bank2;

	virtual void machine_start() override;
	memory_region *m_opt_region;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t uart_r();
	uint8_t uart_status_r();
	void uart_ctrl_w(uint8_t data);
	void modem_w(uint8_t data);
	void ctrl_w(uint8_t data);
	uint8_t keyboard_r();
	uint8_t lcd_r(offs_t offset);
	void lcd_w(offs_t offset, uint8_t data);

	/* memory state */
	uint8_t m_bank;           /* memory bank selection */

	/* keyboard state */
	uint16_t m_keylatch;      /* keyboard latch */

	/* sound state */
	int m_buzzer;               /* buzzer select */
	int m_bell;             /* bell output */

	int m_centronics_busy;
	int m_centronics_select;

	void kc85_palette(palette_device &palette) const;
	void kc85_io(address_map &map);
	void kc85_mem(address_map &map);
	void trsm100_io(address_map &map);
};

class trsm100_state : public kc85_state
{
public:
	trsm100_state(const machine_config &mconfig, device_type type, const char *tag) :
		kc85_state(mconfig, type, tag)
	{ }

	void trsm100(machine_config &config);
	void tandy102(machine_config &config);

private:
	virtual void machine_start() override;
};

class pc8201_state : public kc85_state
{
public:
	pc8201_state(const machine_config &mconfig, device_type type, const char *tag) :
		kc85_state(mconfig, type, tag),
		m_cas_cart(*this, "cas_cartslot")
	{ }

	void pc8300(machine_config &config);
	void pc8201(machine_config &config);

private:
	virtual void machine_start() override;
	required_device<generic_slot_device> m_cas_cart;

	uint8_t bank_r();
	void bank_w(uint8_t data);
	void scp_w(uint8_t data);
	uint8_t uart_status_r();
	void romah_w(uint8_t data);
	void romal_w(uint8_t data);
	void romam_w(uint8_t data);
	uint8_t romrd_r();

	void bankswitch(uint8_t data);

	// ROM cassette
	int m_rom_sel;
	uint32_t m_rom_addr;

	/* peripheral state */
	int m_iosel;                /* serial interface select */
	void pc8201_io(address_map &map);
	void pc8201_mem(address_map &map);
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
		m_y(*this, "Y%u", 0U),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2")
	{ }

	void tandy200(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<i8085a_cpu_device> m_maincpu;
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
	memory_bank_creator m_bank1;
	memory_bank_creator m_bank2;

	memory_region *m_opt_region;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t bank_r();
	void bank_w(uint8_t data);
	uint8_t stbk_r();
	void stbk_w(uint8_t data);
	void i8155_pa_w(uint8_t data);
	void i8155_pb_w(uint8_t data);
	uint8_t i8155_pc_r();
	DECLARE_WRITE_LINE_MEMBER( i8155_to_w );
	DECLARE_WRITE_LINE_MEMBER(kc85_sod_w);
	DECLARE_READ_LINE_MEMBER(kc85_sid_r);
	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_select );

	void tandy200_palette(palette_device &palette) const;

	TIMER_DEVICE_CALLBACK_MEMBER(tandy200_tp_tick);

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
	void tandy200_video(machine_config &config);
	void tandy200_io(address_map &map);
	void tandy200_lcdc(address_map &map);
	void tandy200_mem(address_map &map);
};

#endif // MAME_INCLUDES_KYOCERA_H
