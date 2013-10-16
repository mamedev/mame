// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __TMC1800__
#define __TMC1800__


#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "sound/cdp1864.h"
#include "video/cdp1861.h"
#include "sound/beep.h"

#define TMC2000_COLORRAM_SIZE   0x200

#define SCREEN_TAG      "screen"
#define CDP1802_TAG     "cdp1802"
#define CDP1861_TAG     "cdp1861"
#define CDP1864_TAG     "m3"

class tmc1800_base_state : public driver_device
{
public:
	tmc1800_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, CDP1802_TAG),
			m_cassette(*this, "cassette"),
			m_rom(*this, CDP1802_TAG),
			m_run(*this, "RUN"),
			m_ram(*this, RAM_TAG),
			m_beeper(*this, "beeper")
	{ }


	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_memory_region m_rom;
	required_ioport m_run;
	required_device<ram_device> m_ram;
	optional_device<beep_device> m_beeper;

	DECLARE_QUICKLOAD_LOAD_MEMBER( tmc1800 );
};

class tmc1800_state : public tmc1800_base_state
{
public:
	enum
	{
		TIMER_SETUP_BEEP
	};

	tmc1800_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag),
			m_vdc(*this, CDP1861_TAG)
	{ }

	required_device<cdp1861_device> m_vdc;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_READ8_MEMBER( dispon_r );
	DECLARE_WRITE8_MEMBER( dispoff_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );

	/* keyboard state */
	int m_keylatch;         /* key latch */
	DECLARE_DRIVER_INIT(tmc1800);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

class osc1000b_state : public tmc1800_base_state
{
public:
	osc1000b_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
	{ }


	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );

	/* keyboard state */
	int m_keylatch;
};

class tmc2000_state : public tmc1800_base_state
{
public:
	tmc2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag),
			m_cti(*this, CDP1864_TAG),
			m_colorram(*this, "color_ram"),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7")
	{ }

	required_device<cdp1864_device> m_cti;
	optional_shared_ptr<UINT8> m_colorram;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_READ_LINE_MEMBER( rdata_r );
	DECLARE_READ_LINE_MEMBER( bdata_r );
	DECLARE_READ_LINE_MEMBER( gdata_r );
	DECLARE_INPUT_CHANGED_MEMBER( run_pressed );

	void bankswitch();

	// memory
	int m_rac;
	int m_roc;

	/* video state */
	UINT8 m_color;

	/* keyboard state */
	ioport_port* m_key_row[8];
	int m_keylatch;
};

class nano_state : public tmc1800_base_state
{
public:
	nano_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag),
			m_cti(*this, CDP1864_TAG),
			m_ny0(*this, "NY0"),
			m_ny1(*this, "NY1"),
			m_monitor(*this, "MONITOR")
	{ }

	required_device<cdp1864_device> m_cti;
	required_ioport m_ny0;
	required_ioport m_ny1;
	required_ioport m_monitor;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void machine_start();
	virtual void machine_reset();

	enum
	{
		TIMER_ID_EF4
	};

	DECLARE_WRITE8_MEMBER( keylatch_w );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_INPUT_CHANGED_MEMBER( run_pressed );
	DECLARE_INPUT_CHANGED_MEMBER( monitor_pressed );

	/* keyboard state */
	int m_keylatch;         /* key latch */
};

/* ---------- defined in video/tmc1800.c ---------- */

MACHINE_CONFIG_EXTERN( tmc1800_video );
MACHINE_CONFIG_EXTERN( osc1000b_video );
MACHINE_CONFIG_EXTERN( tmc2000_video );
MACHINE_CONFIG_EXTERN( nano_video );

#endif
