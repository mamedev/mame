// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_NEWBRAIN_H
#define MAME_INCLUDES_NEWBRAIN_H

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
		m_exp(*this, NEWBRAIN_EXPANSION_SLOT_TAG),
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
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( mreq_r );
	DECLARE_WRITE8_MEMBER( mreq_w );
	DECLARE_READ8_MEMBER( iorq_r );
	DECLARE_WRITE8_MEMBER( iorq_w );

	DECLARE_WRITE8_MEMBER( enrg_w );
	DECLARE_WRITE8_MEMBER( tvtl_w );
	DECLARE_READ8_MEMBER( ust_a_r );
	DECLARE_READ8_MEMBER( ust_b_r );

	DECLARE_WRITE8_MEMBER( cop_g_w );
	DECLARE_READ8_MEMBER( cop_g_r );
	DECLARE_WRITE8_MEMBER( cop_d_w );
	DECLARE_READ8_MEMBER( cop_in_r );
	DECLARE_WRITE_LINE_MEMBER( k2_w );
	DECLARE_READ_LINE_MEMBER( tdi_r );
	DECLARE_WRITE_LINE_MEMBER( k1_w );

	void newbrain_iorq(address_map &map);
	void newbrain_mreq(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	enum
	{
		TIMER_ID_RESET,
		TIMER_ID_PWRUP,
		TIMER_ID_CLKINT
	};

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

	int m_405_q;
	uint8_t m_403_q;
	uint8_t m_403_d;
	uint16_t m_402_q;

	int m_rv;
	int m_fs;
	int m_32_40;
	int m_ucr;
	int m_80l;
	uint16_t m_tvl;

	emu_timer *m_clkint_timer;
};

#endif
