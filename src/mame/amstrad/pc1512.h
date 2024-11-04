// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_AMSTRAD_PC1512_H
#define MAME_AMSTRAD_PC1512_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/pc1640_iga.h"
#include "bus/pc1512/mouse.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/buffer.h"
#include "machine/ins8250.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "pc1512kb.h"
#include "machine/upd765.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "ams40041.h"

#define I8086_TAG       "ic120"
#define I8087_TAG       "ic119"
#define I8048_TAG       "i8048"
#define I8237A5_TAG     "ic130"
#define I8259A2_TAG     "ic109"
#define I8253_TAG       "ic114"
#define MC146818_TAG    "ic134"
#define UPD765A_TAG     "ic112"
#define INS8250_TAG     "ic106"
#define AMS40041_TAG    "ic126"
#define CENTRONICS_TAG  "centronics"
#define ISA_BUS_TAG     "isa"
#define RS232_TAG       "rs232"
#define SCREEN_TAG      "screen"

class pc1512_base_state : public driver_device
{
public:
	pc1512_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8086_TAG),
		m_dmac(*this, I8237A5_TAG),
		m_pic(*this, I8259A2_TAG),
		m_pit(*this, I8253_TAG),
		m_rtc(*this, MC146818_TAG),
		m_fdc(*this, UPD765A_TAG),
		m_uart(*this, INS8250_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_cent_data_out(*this, "cent_data_out"),
		m_speaker(*this, "speaker"),
		m_kb(*this, PC1512_KEYBOARD_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy(*this, UPD765A_TAG ":%u", 0U),
		m_bus(*this, ISA_BUS_TAG),
		m_lk(*this, "LK"),
		m_pit1(0),
		m_pit2(0),
		m_status1(0),
		m_status2(0),
		m_port61(0),
		m_nmi_enable(0),
		m_kb_bits(0),
		m_kbclk(1),
		m_kbdata(1),
		m_dreq0(0),
		m_nden(1),
		m_dint(0),
		m_ddrq(0),
		m_fdc_dsr(0),
		m_neop(0),
		m_ack_int_enable(1),
		m_centronics_ack(0),
		m_speaker_drive(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<mc146818_device> m_rtc;
	required_device<upd765a_device> m_fdc;
	required_device<ins8250_device> m_uart;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<speaker_sound_device> m_speaker;
	required_device<pc1512_keyboard_device> m_kb;
	required_device<ram_device> m_ram;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<isa8_device> m_bus;
	required_ioport m_lk;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void update_speaker();
	void update_fdc_int();
	void update_fdc_drq();
	void update_fdc_tc();
	void update_ack();

	uint8_t system_r(offs_t offset);
	void system_w(offs_t offset, uint8_t data);
	uint8_t mouse_r(offs_t offset);
	void mouse_w(offs_t offset, uint8_t data);
	void dma_page_w(offs_t offset, uint8_t data);
	void nmi_mask_w(uint8_t data);
	uint8_t printer_r(offs_t offset);
	void printer_w(offs_t offset, uint8_t data);
	void kbdata_w(int state);
	void kbclk_w(int state);
	void pit1_w(int state);
	void pit2_w(int state);
	void hrq_w(int state);
	void eop_w(int state);
	uint8_t memr_r(offs_t offset);
	void memw_w(offs_t offset, uint8_t data);
	uint8_t ior1_r();
	uint8_t ior2_r();
	uint8_t ior3_r();
	void iow0_w(uint8_t data);
	void iow1_w(uint8_t data);
	void iow2_w(uint8_t data);
	void iow3_w(uint8_t data);
	void dack0_w(int state);
	void dack1_w(int state);
	void dack2_w(int state);
	void dack3_w(int state);
	void fdc_int_w(int state);
	void fdc_drq_w(int state);
	void drive_select_w(uint8_t data);
	void write_centronics_ack(int state);
	void write_centronics_busy(int state);
	void write_centronics_perror(int state);
	void write_centronics_select(int state);
	void write_centronics_fault(int state);
	void mouse_x_w(uint8_t data);
	void mouse_y_w(uint8_t data);

	// system status register
	int m_pit1;
	int m_pit2;
	uint8_t m_status1;
	uint8_t m_status2;
	uint8_t m_port61;

	// interrupt state
	int m_nmi_enable;

	// keyboard state
	uint8_t m_kbd = 0;
	int m_kb_bits;
	int m_kbclk;
	int m_kbdata;

	// mouse state
	uint8_t m_mouse_x_old = 0;
	uint8_t m_mouse_y_old = 0;
	uint8_t m_mouse_x = 0;
	uint8_t m_mouse_y = 0;

	// DMA state
	uint8_t m_dma_page[4]{};
	int m_dma_channel = 0;
	int m_dreq0;

	// floppy state
	int m_nden;
	int m_dint;
	int m_ddrq;
	uint8_t m_fdc_dsr;
	int m_neop;

	// printer state
	int m_ack_int_enable;
	int m_centronics_ack;
	int m_centronics_busy = 0;
	int m_centronics_perror = 0;
	int m_centronics_select = 0;
	int m_centronics_fault = 0;
	uint8_t m_printer_data = 0;
	uint8_t m_printer_control = 0;

	// sound state
	int m_speaker_drive;
};

class pc1512_state : public pc1512_base_state
{
public:
	pc1512_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc1512_base_state(mconfig, type, tag)
		, m_vdu(*this, AMS40041_TAG)
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void pc1512hd(machine_config &config);
	void pc1512(machine_config &config);
	void pc1512dd(machine_config &config);
	void pc1512_io(address_map &map) ATTR_COLD;
	void pc1512_mem(address_map &map) ATTR_COLD;

	required_device<ams40041_device> m_vdu;
};

class pc1640_state : public pc1512_base_state
{
public:
	pc1640_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc1512_base_state(mconfig, type, tag)
		, m_sw(*this, "SW")
		, m_opt(0)
	{ }

	virtual void machine_start() override ATTR_COLD;

	uint8_t io_r(offs_t offset);
	uint8_t printer_r(offs_t offset);

	required_ioport m_sw;

	int m_opt;
	void pc1640hd(machine_config &config);
	void pc1640(machine_config &config);
	void pc1640dd(machine_config &config);
	void pc1640_io(address_map &map) ATTR_COLD;
	void pc1640_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_AMSTRAD_PC1512_H
