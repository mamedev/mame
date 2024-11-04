// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef MAME_TELERCAS_TMC2000E_H
#define MAME_TELERCAS_TMC2000E_H


#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "sound/cdp1864.h"

#define SCREEN_TAG      "screen"
#define CDP1802_TAG     "cdp1802"
#define CDP1864_TAG     "cdp1864"

#define TMC2000E_COLORRAM_SIZE 0x100 // ???

class tmc2000e_state : public driver_device
{
public:
	tmc2000e_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CDP1802_TAG)
		, m_cti(*this, CDP1864_TAG)
		, m_cassette(*this, "cassette")
		, m_colorram(*this, "colorram")
		, m_key_row(*this, {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"})
		, m_run(*this, "RUN")
		, m_led(*this, "led1")
	{ }

	void tmc2000e(machine_config &config);

private:
	uint8_t vismac_r();
	void vismac_w(uint8_t data);
	uint8_t floppy_r();
	void floppy_w(uint8_t data);
	uint8_t ascii_keyboard_r();
	uint8_t io_r();
	void io_w(uint8_t data);
	void io_select_w(uint8_t data);
	void keyboard_latch_w(uint8_t data);
	int rdata_r();
	int bdata_r();
	int gdata_r();
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);
	void dma_w(offs_t offset, uint8_t data);

	/* video state */
	int m_cdp1864_efx = 0;      /* EFx */
	uint8_t m_color = 0;

	/* keyboard state */
	int m_keylatch = 0;         /* key latch */
	void tmc2000e_io_map(address_map &map) ATTR_COLD;
	void tmc2000e_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<cassette_image_device> m_cassette;
	required_shared_ptr<uint8_t> m_colorram;
	required_ioport_array<8> m_key_row;
	required_ioport m_run;
	output_finder<> m_led;
};

#endif // MAME_TELERCAS_TMC2000E_H
