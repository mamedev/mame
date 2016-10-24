// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __VIXEN__
#define __VIXEN__

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "bus/ieee488/ieee488.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/discrete.h"

#define Z8400A_TAG      "5f"
#define FDC1797_TAG     "5n"
#define P8155H_TAG      "2n"
#define P8155H_IO_TAG   "c7"
#define P8251A_TAG      "c3"
#define DISCRETE_TAG    "discrete"
#define SCREEN_TAG      "screen"
#define RS232_TAG       "rs232"

class vixen_state : public driver_device
{
public:
	vixen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z8400A_TAG),
			m_fdc(*this, FDC1797_TAG),
			m_io_i8155(*this, P8155H_IO_TAG),
			m_usart(*this, P8251A_TAG),
			m_discrete(*this, DISCRETE_TAG),
			m_ieee488(*this, IEEE488_TAG),
			m_palette(*this, "palette"),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, FDC1797_TAG":0"),
			m_floppy1(*this, FDC1797_TAG":1"),
			m_rs232(*this, RS232_TAG),
			m_rom(*this, Z8400A_TAG),
			m_sync_rom(*this, "video"),
			m_char_rom(*this, "chargen"),
			m_video_ram(*this, "video_ram"),
			m_key(*this, "KEY.%u", 0),
			m_cmd_d1(0),
			m_fdint(0),
			m_vsync(0),
			m_srq(1),
			m_atn(1),
			m_rxrdy(0),
			m_txrdy(0)
	{ }

	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ieee488_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t i8155_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8155_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8155_pc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void io_i8155_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void io_i8155_pc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void io_i8155_to_w(int state);
	void srq_w(int state);
	void atn_w(int state);
	void rxrdy_w(int state);
	void txrdy_w(int state);
	void fdc_intrq_w(int state);
	void init_vixen();
	void vsync_tick(timer_device &timer, void *ptr, int32_t param);
	int vixen_int_ack(device_t &device, int irqline);
	uint8_t opram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t oprom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	required_device<cpu_device> m_maincpu;
	required_device<fd1797_t> m_fdc;
	required_device<i8155_device> m_io_i8155;
	required_device<i8251_device> m_usart;
	required_device<discrete_sound_device> m_discrete;
	required_device<ieee488_device> m_ieee488;
	required_device<palette_device> m_palette;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<rs232_port_device> m_rs232;
	required_region_ptr<uint8_t> m_rom;
	required_region_ptr<uint8_t> m_sync_rom;
	required_region_ptr<uint8_t> m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	required_ioport_array<8> m_key;

	address_space *m_program;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	void update_interrupt();

	// keyboard state
	uint8_t m_col;

	// interrupt state
	int m_cmd_d0;
	int m_cmd_d1;

	bool m_fdint;
	int m_vsync;

	int m_srq;
	int m_atn;
	int m_enb_srq_int;
	int m_enb_atn_int;

	int m_rxrdy;
	int m_txrdy;
	int m_int_clk;
	int m_enb_xmt_int;
	int m_enb_rcv_int;
	int m_enb_ring_int;

	// video state
	bool m_alt;
	bool m_256;
};

#endif
