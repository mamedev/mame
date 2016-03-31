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
		m_y0(*this, "Y0"),
		m_y1(*this, "Y1"),
		m_y2(*this, "Y2"),
		m_y3(*this, "Y3"),
		m_y4(*this, "Y4"),
		m_y5(*this, "Y5"),
		m_y6(*this, "Y6"),
		m_y7(*this, "Y7"),
		m_y8(*this, "Y8"),
		m_y9(*this, "Y9"),
		m_y10(*this, "Y10"),
		m_y11(*this, "Y11"),
		m_y12(*this, "Y12"),
		m_y13(*this, "Y13"),
		m_y14(*this, "Y14"),
		m_y15(*this, "Y15"),
		m_pwrup(0),
		m_userint(1),
		m_clkint(1),
		m_copint(1),
		m_keylatch(0),
		m_keydata(0xf)
	{
	}

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( mreq_r );
	DECLARE_WRITE8_MEMBER( mreq_w );
	DECLARE_READ8_MEMBER( iorq_r );
	DECLARE_WRITE8_MEMBER( iorq_w );

	DECLARE_WRITE8_MEMBER( enrg1_w );
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

	INTERRUPT_GEN_MEMBER(newbrain_interrupt);

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
	void tvl(UINT8 data, int a6);

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
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_y10;
	required_ioport m_y11;
	required_ioport m_y12;
	required_ioport m_y13;
	required_ioport m_y14;
	required_ioport m_y15;

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
	UINT16 m_segment_data;

	int m_rv;
	int m_fs;
	int m_32_40;
	int m_ucr;
	int m_80l;
	UINT16 m_tvl;
};


// ---------- defined in video/newbrain.c ----------

MACHINE_CONFIG_EXTERN( newbrain_video );

#endif
