// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __ETI660__
#define __ETI660__

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
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
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, CDP1802_TAG),
			m_cti(*this, CDP1864_TAG),
			m_pia(*this, MC6821_TAG),
			m_cassette(*this, "cassette"),
			m_pa0(*this, "PA0"),
			m_pa1(*this, "PA1"),
			m_pa2(*this, "PA2"),
			m_pa3(*this, "PA3"),
			m_special(*this, "SPECIAL")
	{ }

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<pia6821_device> m_pia;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_pa0;
	required_ioport m_pa1;
	required_ioport m_pa2;
	required_ioport m_pa3;
	required_ioport m_special;

	DECLARE_READ8_MEMBER( pia_r );
	DECLARE_WRITE8_MEMBER( pia_w );
	DECLARE_WRITE8_MEMBER( colorram_w );
	DECLARE_READ_LINE_MEMBER( rdata_r );
	DECLARE_READ_LINE_MEMBER( bdata_r );
	DECLARE_READ_LINE_MEMBER( gdata_r );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_READ8_MEMBER( pia_pa_r );
	DECLARE_WRITE8_MEMBER( pia_pa_w );

	/* keyboard state */
	UINT8 m_keylatch;

	/* video state */
	UINT8 m_color_ram[0x100];
	UINT8 m_color;
};

#endif
