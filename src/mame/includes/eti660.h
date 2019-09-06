// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_ETI660_H
#define MAME_INCLUDES_ETI660_H

#pragma once

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "machine/6821pia.h"
#include "machine/rescap.h"
#include "sound/cdp1864.h"

#define SCREEN_TAG      "screen"
#define CDP1802_TAG     "ic3"
#define CDP1864_TAG     "ic4"
#define MC6821_TAG      "ic5"

enum
{
	LED_POWER = 0,
	LED_PULSE
};

class eti660_state : public driver_device
{
public:
	eti660_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, CDP1802_TAG)
		, m_cti(*this, CDP1864_TAG)
		, m_pia(*this, MC6821_TAG)
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "KEY.%u", 0)
		, m_special(*this, "SPECIAL")
		, m_leds(*this, "led%u", 0U)
	{ }

	void eti660(machine_config &config);

private:
	DECLARE_READ8_MEMBER( pia_r );
	DECLARE_WRITE8_MEMBER( pia_w );
	DECLARE_WRITE8_MEMBER( colorram_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_WRITE_LINE_MEMBER( ca2_w );
	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_READ8_MEMBER( pia_pa_r );
	DECLARE_WRITE8_MEMBER( pia_pa_w );
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	required_shared_ptr<uint8_t> m_p_videoram;

	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<pia6821_device> m_pia;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<4> m_io_keyboard;
	required_ioport m_special;
	output_finder<2> m_leds;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint16_t m_resetcnt;

	/* keyboard state */
	uint8_t m_keylatch;

	/* video state */
	uint8_t m_color_ram[0xc0];
	uint8_t m_color;
	bool m_color_on;
};

#endif
