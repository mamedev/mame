// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef MAME_RCA_VIP_H
#define MAME_RCA_VIP_H

#include "bus/vip/byteio.h"
#include "bus/vip/exp.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "sound/discrete.h"
#include "video/cdp1861.h"

#define SCREEN_TAG      "screen"
#define CDP1802_TAG     "u1"
#define CDP1861_TAG     "u2"
#define DISCRETE_TAG    "discrete"


class vp111_state : public driver_device
{
public:
	vp111_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CDP1802_TAG)
		, m_vdc(*this, CDP1861_TAG)
		, m_ram(*this, RAM_TAG)
		, m_cassette(*this, "cassette")
		, m_beeper(*this, DISCRETE_TAG)
		, m_rom(*this, CDP1802_TAG)
		, m_chip8(*this, "chip8")
		, m_chip8x(*this, "chip8x")
		, m_run(*this, "RUN")
		, m_keypad(*this, "KEYPAD")
		, m_io_beeper(*this, "BEEPER")
		, m_leds(*this, "led%u", 0U)
		, m_8000(1)
	{ }

	void vp111(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(reset_w);
	DECLARE_INPUT_CHANGED_MEMBER(beeper_w);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);
	virtual uint8_t io_r(offs_t offset);
	virtual void io_w(offs_t offset, uint8_t data);

	virtual int int_r();
	virtual int dma_out_r();
	virtual int ef1_r();
	virtual int ef3_r();
	virtual int ef4_r();
	virtual void q_w(int state);
	virtual void run_w(int state);

	int cs(offs_t offset) const { return BIT(offset, 15) || m_8000; }
	uint8_t board_r(offs_t offset, uint8_t data, int minh);
	void board_w(offs_t offset, uint8_t data, int minh);

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1861_device> m_vdc;
	required_device<ram_device> m_ram;

private:
	int clear_r();
	int ef2_r();

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void vip_io(address_map &map) ATTR_COLD;
	void vip_mem(address_map &map) ATTR_COLD;

	required_device<cassette_image_device> m_cassette;
	required_device<discrete_sound_device> m_beeper;
	required_memory_region m_rom;
	required_memory_region m_chip8;
	required_memory_region m_chip8x;
	required_ioport m_run;
	required_ioport m_keypad;
	required_ioport m_io_beeper;
	output_finder<3> m_leds;

	// memory state
	int m_8000;

	// keyboard state
	int m_keylatch = 0;
};


class vip_state : public vp111_state
{
public:
	vip_state(const machine_config &mconfig, device_type type, const char *tag)
		: vp111_state(mconfig, type, tag)
		, m_byteio(*this, VIP_BYTEIO_PORT_TAG)
		, m_exp(*this, "exp")
	{ }

	void vip(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual uint8_t io_r(offs_t offset) override;
	virtual void io_w(offs_t offset, uint8_t data) override;

	virtual int int_r() override;
	virtual int dma_out_r() override;
	virtual int ef1_r() override;
	virtual int ef3_r() override;
	virtual int ef4_r() override;
	virtual void q_w(int state) override;
	virtual void run_w(int state) override;

private:
	static int cdef(offs_t offset) { return !((offset >= 0xc00) && (offset < 0x1000)); }

	void byteio_inst_w(int state);

	void exp_int_w(int state);
	void exp_dma_out_w(int state);
	void exp_dma_in_w(int state);

	required_device<vip_byteio_port_device> m_byteio;
	required_device<vip_expansion_slot_device> m_exp;

	// interrupt state
	int m_exp_int = CLEAR_LINE;
	int m_exp_dma_out = CLEAR_LINE;
	int m_exp_dma_in = CLEAR_LINE;

	// expansion state
	uint8_t m_byteio_data = 0;
	bool m_byteio_inst = false;
};

#endif // MAME_RCA_VIP_H
