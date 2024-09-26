// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_EFO_CIDELSA_H
#define MAME_EFO_CIDELSA_H

#pragma once


#include "cpu/cosmac/cosmac.h"
#include "cpu/cop400/cop400.h"
#include "sound/cdp1869.h"
#include "sound/ay8910.h"
#include "machine/cdp1852.h"
#include "machine/nvram.h"


#define SCREEN_TAG  "screen"
#define CDP1802_TAG "cdp1802"
#define CDP1869_TAG "cdp1869"
#define COP402N_TAG "cop402n"
#define AY8910_TAG  "ay8910"

#define DESTRYER_CHR1   3579000.0 // unverified
#define DESTRYER_CHR2   XTAL(5'714'300)
#define ALTAIR_CHR1     3579000.0 // unverified
#define ALTAIR_CHR2     cdp1869_device::DOT_CLK_PAL // unverified
#define DRACO_CHR1      XTAL(4'433'610)
#define DRACO_CHR2      cdp1869_device::DOT_CLK_PAL // unverified
#define DRACO_SND_CHR1  XTAL(2'012'160)

#define CIDELSA_PAGERAM_SIZE    0x400
#define DRACO_PAGERAM_SIZE      0x800
#define CIDELSA_CHARRAM_SIZE    0x800

#define CIDELSA_PAGERAM_MASK    0x3ff
#define DRACO_PAGERAM_MASK      0x7ff
#define CIDELSA_CHARRAM_MASK    0x7ff


class cidelsa_state : public driver_device
{
public:
	cidelsa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CDP1802_TAG)
		, m_vis(*this, CDP1869_TAG)
		, m_leds(*this, "led%u", 0U)
	{ }

	void cdp1869_w(offs_t offset, uint8_t data);
	void destryer_out1_w(uint8_t data);
	void altair_out1_w(uint8_t data);

	int clear_r();

	void q_w(int state);
	void prd_w(int state);
	int cdp1869_pcb_r();

	CDP1869_CHAR_RAM_READ_MEMBER(cidelsa_charram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(cidelsa_charram_w);
	CDP1869_PCB_READ_MEMBER(cidelsa_pcb_r);

	void destryera(machine_config &config);
	void altair(machine_config &config);
	void destryer(machine_config &config);
	void destryer_video(machine_config &config);
	void altair_video(machine_config &config);
	void altair_io_map(address_map &map) ATTR_COLD;
	void altair_map(address_map &map) ATTR_COLD;
	void cidelsa_page_ram(address_map &map) ATTR_COLD;
	void destryer_io_map(address_map &map) ATTR_COLD;
	void destryer_map(address_map &map) ATTR_COLD;
	void destryera_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(reset_done);

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1869_device> m_vis;
	output_finder<3> m_leds;

	// cpu state
	int m_reset = 0;
	emu_timer *m_reset_timer = nullptr;

	// video state
	int m_cdp1802_q = 0;
	int m_cdp1869_pcb = 0;

	uint8_t *m_pageram = nullptr;
	std::unique_ptr<uint8_t[]> m_pcbram;
	std::unique_ptr<uint8_t[]> m_charram;
};

class draco_state : public cidelsa_state
{
public:
	draco_state(const machine_config &mconfig, device_type type, const char *tag)
		: cidelsa_state(mconfig, type, tag),
			m_psg(*this, AY8910_TAG)
	{ }

	uint8_t sound_in_r();
	uint8_t psg_r();
	void sound_bankswitch_w(uint8_t data);
	void sound_g_w(uint8_t data);
	void psg_w(uint8_t data);
	void out1_w(uint8_t data);
	void psg_pb_w(uint8_t data);

	CDP1869_CHAR_RAM_READ_MEMBER(draco_charram_r);
	CDP1869_CHAR_RAM_WRITE_MEMBER(draco_charram_w);
	CDP1869_PCB_READ_MEMBER(draco_pcb_r);

	void draco(machine_config &config);
	void draco_video(machine_config &config);
	void draco_io_map(address_map &map) ATTR_COLD;
	void draco_map(address_map &map) ATTR_COLD;
	void draco_page_ram(address_map &map) ATTR_COLD;
	void draco_sound_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<ay8910_device> m_psg;
	// sound state
	int m_sound = 0;
	int m_psg_latch = 0;
};

#endif // MAME_EFO_CIDELSA_H
