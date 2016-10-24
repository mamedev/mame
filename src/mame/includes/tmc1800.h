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

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void keylatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dispon_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dispoff_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);

	/* keyboard state */
	int m_keylatch;         /* key latch */
	void init_tmc1800();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

class osc1000b_state : public tmc1800_base_state
{
public:
	osc1000b_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
	{ }


	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void keylatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);

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
			m_key_row(*this, {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"})
	{ }

	required_device<cdp1864_device> m_cti;
	optional_shared_ptr<uint8_t> m_colorram;
	required_ioport_array<8> m_key_row;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void keylatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);
	void dma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int rdata_r();
	int bdata_r();
	int gdata_r();
	void run_pressed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void bankswitch();

	// memory
	int m_rac;
	int m_roc;

	/* video state */
	uint8_t m_color;

	/* keyboard state */
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

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	enum
	{
		TIMER_ID_EF4
	};

	void keylatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);
	void run_pressed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void monitor_pressed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	/* keyboard state */
	int m_keylatch;         /* key latch */
};

/* ---------- defined in video/tmc1800.c ---------- */

MACHINE_CONFIG_EXTERN( tmc1800_video );
MACHINE_CONFIG_EXTERN( osc1000b_video );
MACHINE_CONFIG_EXTERN( tmc2000_video );
MACHINE_CONFIG_EXTERN( nano_video );

#endif
