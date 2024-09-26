// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_GRUNDY_NEWBRAIN_H
#define MAME_GRUNDY_NEWBRAIN_H

#pragma once


#include "bus/newbrain/exp.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "cpu/cop400/cop400.h"
#include "imagedev/cassette.h"
#include "machine/rescap.h"
#include "machine/ram.h"
#include "emupal.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "409"
#define COP420_TAG      "419"
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
		m_exp(*this, "exp"),
		m_cassette1(*this, "cassette1"),
		m_cassette2(*this, "cassette2"),
		m_rs232_v24(*this, RS232_V24_TAG),
		m_rs232_prn(*this, RS232_PRN_TAG),
		m_ram(*this, RAM_TAG),
		m_rom(*this, Z80_TAG),
		m_char_rom(*this, "chargen"),
		m_y(*this, "Y%u", 0),
		m_digits(*this, "digit%u", 0U),
		m_pwrup(0),
		m_userint(1),
		m_clkint(1),
		m_copint(1),
		m_405_q(0),
		m_403_q(0xf)
	{
	}

	void newbrain(machine_config &config);
	void newbrain_a(machine_config &config);
	void newbrain_ad(machine_config &config);
	void newbrain_md(machine_config &config);
	void newbrain_video(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void newbrain_iorq(address_map &map) ATTR_COLD;
	void newbrain_mreq(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_reset);
	TIMER_CALLBACK_MEMBER(power_on);
	TIMER_CALLBACK_MEMBER(clear_clkint);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);
	uint8_t iorq_r(offs_t offset);
	void iorq_w(offs_t offset, uint8_t data);

	void enrg_w(uint8_t data);
	void tvtl_w(uint8_t data);
	uint8_t ust_a_r();
	uint8_t ust_b_r();

	void cop_g_w(uint8_t data);
	uint8_t cop_g_r();
	void cop_d_w(uint8_t data);
	uint8_t cop_in_r();
	void k2_w(int state);
	int tdi_r();
	void k1_w(int state);

	void check_interrupt();
	void clclk();
	int tpin();
	void tm();

	int get_reset_t();
	int get_pwrup_t();

	void do_screen_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tvl(uint8_t data, int a6);

	required_device<z80_device> m_maincpu;
	required_device<cop400_cpu_device> m_cop;
	required_device<palette_device> m_palette;
	required_device<newbrain_expansion_slot_device> m_exp;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<rs232_port_device> m_rs232_v24;
	required_device<rs232_port_device> m_rs232_prn;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	required_ioport_array<16> m_y;
	output_finder<16> m_digits;

	int m_clk = 0;
	int m_tvp = 0;
	int m_pwrup = 0;
	int m_userint = 0;
	int m_clkint = 0;
	int m_copint = 0;

	int m_cop_so = 0;
	int m_cop_tdo = 0;
	int m_cop_g1 = 0;
	int m_cop_g3 = 0;
	int m_cop_k6 = 0;

	int m_405_q = 0;
	uint8_t m_403_q = 0;
	uint8_t m_403_d = 0;
	uint16_t m_402_q = 0;

	int m_rv = 0;
	int m_fs = 0;
	int m_32_40 = 0;
	int m_ucr = 0;
	int m_80l = 0;
	uint16_t m_tvl = 0;

	emu_timer *m_reset_timer = nullptr;
	emu_timer *m_power_timer = nullptr;
	emu_timer *m_clkint_timer = nullptr;
};

#endif // MAME_GRUNDY_NEWBRAIN_H
