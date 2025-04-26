// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Zsolt Vasvari
// thanks-to: John Butler, Ed Mueller
/***************************************************************************

    Taito Qix hardware

***************************************************************************/
#ifndef MAME_TAITO_QIX_H
#define MAME_TAITO_QIX_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "cpu/m6805/m68705.h"

#include "machine/6821pia.h"

#include "sound/discrete.h"
#include "sound/sn76496.h"

#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"


class qix_state : public driver_device
{
public:
	qix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_videocpu(*this, "videocpu"),
		m_crtc(*this, "vid_u18"),
		m_pia(*this, "pia%u", 0U),
		m_sndpia(*this, "sndpia%u", 0U),
		m_discrete(*this, "discrete"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram", 0x10000, ENDIANNESS_BIG),
		m_videoram_address(*this, "videoram_addr"),
		m_scanline_latch(*this, "scanline_latch"),
		m_videobank(*this, "videobank")
	{ }

	void qix(machine_config &config) ATTR_COLD;

protected:
	static constexpr XTAL MAIN_CLOCK_OSC  = XTAL(20'000'000);    /* 20 MHz */

	virtual void video_start() override ATTR_COLD;

	/* devices */
	required_device<mc6809e_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<mc6809e_device> m_videocpu;
	required_device<mc6845_device> m_crtc;
	required_device_array<pia6821_device, 3> m_pia;
	optional_device_array<pia6821_device, 3> m_sndpia;
	optional_device<discrete_sound_device> m_discrete;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video state */
	memory_share_creator<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram_address;
	required_shared_ptr<uint8_t> m_scanline_latch;

	optional_memory_bank m_videobank;

	bool     m_flip = false;
	uint8_t  m_palette_bank = 0U;
	uint8_t  m_leds = 0U;

	static rgb_t qix_R2G2B2I2(uint32_t raw);
	void data_firq_w(uint8_t data);
	void data_firq_ack_w(uint8_t data);
	uint8_t data_firq_r(address_space &space);
	uint8_t data_firq_ack_r(address_space &space);
	void video_firq_w(uint8_t data);
	void video_firq_ack_w(uint8_t data);
	uint8_t video_firq_r(address_space &space);
	uint8_t video_firq_ack_r(address_space &space);
	uint8_t videoram_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t addresslatch_r();
	void addresslatch_w(uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	void palettebank_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(pia_w_callback);
	TIMER_CALLBACK_MEMBER(deferred_sndpia1_porta_w);
	void vsync_changed(int state);
	void pia_w(offs_t offset, uint8_t data);
	void coinctr_w(uint8_t data);
	void display_enable_changed(int state);
	void flip_screen_w(int state);
	void dac_w(uint8_t data);
	void vol_w(uint8_t data);
	void sndpia_2_warning_w(uint8_t data);
	void sync_sndpia1_porta_w(uint8_t data);
	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);

	void qix_base(machine_config &config) ATTR_COLD;
	void qix_video(machine_config &config) ATTR_COLD;
	void qix_audio(machine_config &config) ATTR_COLD;

	void audio_map(address_map &map) ATTR_COLD;
	void qix_main_map(address_map &map) ATTR_COLD;
	void qix_video_map(address_map &map) ATTR_COLD;
};

// with encrypted CPU opcode
class kram3_state : public qix_state
{
public:
	kram3_state(const machine_config &mconfig, device_type type, const char *tag) :
		qix_state(mconfig, type, tag),
		m_mainbank(*this, "mainbank")
	{ }

	void kram3(machine_config &config) ATTR_COLD;

	void init_kram3();

protected:
	void video(machine_config &config) ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;
	void video_map(address_map &map) ATTR_COLD;

private:
	void lic_maincpu_changed(int state);
	void lic_videocpu_changed(int state);

	std::unique_ptr<uint8_t[]> m_main_decrypted;
	std::unique_ptr<uint8_t[]> m_video_decrypted;

	required_memory_bank m_mainbank;
};

// with different sound system, video RAM write masking feature
class slither_state : public qix_state
{
public:
	slither_state(const machine_config &mconfig, device_type type, const char *tag) :
		qix_state(mconfig, type, tag),
		m_sn(*this, "sn%u", 1U),
		m_videoram_mask(*this, "videoram_mask"),
		m_trak(*this, "AN%u", 0U)
	{ }

	void slither(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	void video(machine_config &config) ATTR_COLD;
	void audio(machine_config &config) ATTR_COLD;

	void video_map(address_map &map) ATTR_COLD;

private:
	static constexpr XTAL SLITHER_CLOCK_OSC = XTAL(21'300'000);    /* 21.3 MHz */

	template <unsigned Which> uint8_t trak_r();
	template <unsigned Which> void sn76489_ctrl_w(int state);
	void slither_coinctr_w(uint8_t data);
	void slither_videoram_w(offs_t offset, uint8_t data);
	void slither_addresslatch_w(uint8_t data);

	required_device_array<sn76489_device, 2> m_sn;
	required_shared_ptr<uint8_t> m_videoram_mask;
	required_ioport_array<4> m_trak;

	bool m_sn76489_ctrl[2] = { false, false };
};

// with MCU
class qixmcu_state : public qix_state
{
public:
	qixmcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		qix_state(mconfig, type, tag),
		m_mcu(*this, "mcu"),
		m_coin(*this, "COIN")
	{ }

	void mcu(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	optional_device<m68705p_device> m_mcu;
	required_ioport m_coin;

private:
	uint8_t coin_r();
	void coin_w(uint8_t data);
	void coinctrl_w(uint8_t data);

	uint8_t mcu_portb_r();
	uint8_t mcu_portc_r();
	void mcu_porta_w(uint8_t data);
	void mcu_portb_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	/* machine state */
	uint8_t  m_68705_porta_out = 0;
	uint8_t  m_coinctrl = 0;
};

// with video CPU bankswitching
class zookeep_state : public qixmcu_state
{
public:
	zookeep_state(const machine_config &mconfig, device_type type, const char *tag) :
		qixmcu_state(mconfig, type, tag)
	{ }

	void zookeep(machine_config &config) ATTR_COLD;
	void zookeepbl(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	void video(machine_config &config) ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;
	void video_map(address_map &map) ATTR_COLD;

private:
	void bankswitch_w(uint8_t data);
};

#endif // MAME_TAITO_QIX_H
