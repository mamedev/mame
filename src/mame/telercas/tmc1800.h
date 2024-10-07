// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_TELERCAS_TMC1800_H
#define MAME_TELERCAS_TMC1800_H

#pragma once


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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CDP1802_TAG)
		, m_cassette(*this, "cassette")
		, m_rom(*this, CDP1802_TAG)
		, m_run(*this, "RUN")
		, m_ram(*this, RAM_TAG)
		, m_beeper(*this, "beeper")
	{ }

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

protected:
	required_device<cosmac_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_memory_region m_rom;
	required_ioport m_run;
	required_device<ram_device> m_ram;
	optional_device<beep_device> m_beeper;
};

class tmc1800_state : public tmc1800_base_state
{
public:
	tmc1800_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
		, m_vdc(*this, CDP1861_TAG)
	{ }

	void keylatch_w(uint8_t data);
	uint8_t dispon_r();
	void dispoff_w(uint8_t data);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);

	void tmc1800(machine_config &config);
	void tmc1800_video(machine_config &config);
	void tmc1800_io_map(address_map &map) ATTR_COLD;
	void tmc1800_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cdp1861_device> m_vdc;
	/* keyboard state */
	int m_keylatch = 0;
};

class osc1000b_state : public tmc1800_base_state
{
public:
	osc1000b_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
	{ }


	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void keylatch_w(uint8_t data);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);

	void osc1000b(machine_config &config);
	void osc1000b_video(machine_config &config);
	void osc1000b_io_map(address_map &map) ATTR_COLD;
	void osc1000b_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	/* keyboard state */
	int m_keylatch = 0;
};

class tmc2000_state : public tmc1800_base_state
{
public:
	tmc2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
		, m_cti(*this, CDP1864_TAG)
		, m_colorram(*this, "color_ram", TMC2000_COLORRAM_SIZE, ENDIANNESS_LITTLE)
		, m_key_row(*this, {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"})
		, m_led(*this, "led1")
	{ }

	void keylatch_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);
	void dma_w(offs_t offset, uint8_t data);
	int rdata_r();
	int bdata_r();
	int gdata_r();
	DECLARE_INPUT_CHANGED_MEMBER( run_pressed );

	void bankswitch();

	void tmc2000(machine_config &config);
	void tmc2000_video(machine_config &config);
	void tmc2000_io_map(address_map &map) ATTR_COLD;
	void tmc2000_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cdp1864_device> m_cti;
	memory_share_creator<uint8_t> m_colorram;
	required_ioport_array<8> m_key_row;
	output_finder<> m_led;

	// memory
	int m_rac = 0;
	int m_roc = 0;

	/* video state */
	uint8_t m_color = 0;

	/* keyboard state */
	int m_keylatch = 0;
};

class nano_state : public tmc1800_base_state
{
public:
	nano_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
		, m_cti(*this, CDP1864_TAG)
		, m_ny0(*this, "NY0")
		, m_ny1(*this, "NY1")
		, m_monitor(*this, "MONITOR")
		, m_led(*this, "led1")
	{ }

	void keylatch_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);
	DECLARE_INPUT_CHANGED_MEMBER( run_pressed );
	DECLARE_INPUT_CHANGED_MEMBER( monitor_pressed );

	void nano(machine_config &config);
	void nano_video(machine_config &config);
	void nano_io_map(address_map &map) ATTR_COLD;
	void nano_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(assert_ef4);

	required_device<cdp1864_device> m_cti;
	required_ioport m_ny0;
	required_ioport m_ny1;
	required_ioport m_monitor;
	output_finder<> m_led;

	emu_timer *m_ef4_timer = nullptr;

	/* keyboard state */
	int m_keylatch = 0;
};

#endif // MAME_TELERCAS_TMC1800_H
