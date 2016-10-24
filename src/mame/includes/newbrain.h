// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __NEWBRAIN__
#define __NEWBRAIN__


#include "emu.h"
#include "bus/newbrain/exp.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "cpu/cop400/cop400.h"
#include "imagedev/cassette.h"
#include "machine/rescap.h"
#include "machine/ram.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "409"
#define COP420_TAG      "419"
#define CASSETTE_TAG    "cassette"
#define CASSETTE2_TAG   "cassette2"
#define RS232_V24_TAG   "to"
#define RS232_PRN_TAG   "po"

class newbrain_state : public driver_device
{
public:
	newbrain_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_cop(*this, COP420_TAG),
		m_palette(*this, "palette"),
		m_exp(*this, NEWBRAIN_EXPANSION_SLOT_TAG),
		m_cassette1(*this, CASSETTE_TAG),
		m_cassette2(*this, CASSETTE2_TAG),
		m_rs232_v24(*this, RS232_V24_TAG),
		m_rs232_prn(*this, RS232_PRN_TAG),
		m_ram(*this, RAM_TAG),
		m_rom(*this, Z80_TAG),
		m_char_rom(*this, "chargen"),
		m_y(*this, "Y%u", 0),
		m_pwrup(0),
		m_userint(1),
		m_clkint(1),
		m_copint(1),
		m_keylatch(0),
		m_keydata(0xf)
	{
	}

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t mreq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mreq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iorq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iorq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void enrg1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tvtl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ust_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ust_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void cop_g_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cop_g_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cop_d_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cop_in_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void k2_w(int state);
	int tdi_r();
	void k1_w(int state);

	void newbrain_interrupt(device_t &device);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	enum
	{
		TIMER_ID_RESET,
		TIMER_ID_PWRUP
	};

	void check_interrupt();
	void clclk();
	int tpin();
	void tm();

	int get_reset_t();
	int get_pwrup_t();

	void screen_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tvl(uint8_t data, int a6);

	required_device<z80_device> m_maincpu;
	required_device<cop400_cpu_device> m_cop;
	required_device<palette_device> m_palette;
	required_device<newbrain_expansion_slot_t> m_exp;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<rs232_port_device> m_rs232_v24;
	required_device<rs232_port_device> m_rs232_prn;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	required_ioport_array<16> m_y;

	int m_clk;
	int m_tvp;
	int m_pwrup;
	int m_userint;
	int m_clkint;
	int m_copint;

	int m_cop_so;
	int m_cop_tdo;
	int m_cop_g1;
	int m_cop_g3;
	int m_cop_k6;

	int m_keylatch;
	int m_keydata;
	uint16_t m_segment_data;

	int m_rv;
	int m_fs;
	int m_32_40;
	int m_ucr;
	int m_80l;
	uint16_t m_tvl;
};


// ---------- defined in video/newbrain.c ----------

MACHINE_CONFIG_EXTERN( newbrain_video );

#endif
