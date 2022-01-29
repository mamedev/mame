// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Zsolt Vasvari
// thanks-to: John Butler, Ed Mueller
/***************************************************************************

    Taito Qix hardware

***************************************************************************/
#ifndef MAME_INCLUDES_QIX_H
#define MAME_INCLUDES_QIX_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "cpu/m6805/m68705.h"

#include "machine/6821pia.h"

#include "sound/discrete.h"
#include "sound/sn76496.h"

#include "video/mc6845.h"

#include "screen.h"


#define MAIN_CLOCK_OSC          20000000    /* 20 MHz */
#define SLITHER_CLOCK_OSC       21300000    /* 21.3 MHz */
#define SOUND_CLOCK_OSC         7372800     /* 7.3728 MHz */
#define COIN_CLOCK_OSC          4000000     /* 4 MHz */
#define QIX_CHARACTER_CLOCK     (20000000/2/16)

class qix_state : public driver_device
{
public:
	qix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_videocpu(*this, "videocpu"),
		m_crtc(*this, "vid_u18"),
		m_pia0(*this, "pia0"),
		m_pia1(*this, "pia1"),
		m_pia2(*this, "pia2"),
		m_sndpia0(*this, "sndpia0"),
		m_sndpia1(*this, "sndpia1"),
		m_sndpia2(*this, "sndpia2"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_discrete(*this, "discrete"),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram", 0x10000, ENDIANNESS_BIG),
		m_videoram_address(*this, "videoram_addr"),
		m_videoram_mask(*this, "videoram_mask"),
		m_scanline_latch(*this, "scanline_latch"),
		m_bank0(*this, "bank0"),
		m_bank1(*this, "bank1"),
		m_screen(*this, "screen")
	{ }

	void qix_base(machine_config &config);
	void qix(machine_config &config);
	void qix_video(machine_config &config);
	void qix_audio(machine_config &config);
	void kram3(machine_config &config);
	void kram3_video(machine_config &config);
	void slither(machine_config &config);
	void slither_video(machine_config &config);
	void slither_audio(machine_config &config);

	void init_kram3();

protected:
	virtual void video_start() override;

	/* devices */
	required_device<mc6809e_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<mc6809e_device> m_videocpu;
	required_device<mc6845_device> m_crtc;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_device<pia6821_device> m_sndpia0;
	optional_device<pia6821_device> m_sndpia1;
	optional_device<pia6821_device> m_sndpia2;
	optional_device<sn76489_device> m_sn1;
	optional_device<sn76489_device> m_sn2;
	optional_device<discrete_sound_device> m_discrete;

	/* video state */
	required_shared_ptr<uint8_t> m_paletteram;
	memory_share_creator<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram_address;
	optional_shared_ptr<uint8_t> m_videoram_mask;
	required_shared_ptr<uint8_t> m_scanline_latch;
	uint8_t  m_flip;
	uint8_t  m_palette_bank;
	uint8_t  m_leds;

	optional_memory_bank m_bank0;
	optional_memory_bank m_bank1;
	required_device<screen_device> m_screen;
	std::unique_ptr<uint8_t[]> m_decrypted;
	std::unique_ptr<uint8_t[]> m_decrypted2;

	pen_t m_pens[0x400];
	void qix_data_firq_w(uint8_t data);
	void qix_data_firq_ack_w(uint8_t data);
	uint8_t qix_data_firq_r(address_space &space);
	uint8_t qix_data_firq_ack_r(address_space &space);
	void qix_video_firq_w(uint8_t data);
	void qix_video_firq_ack_w(uint8_t data);
	uint8_t qix_video_firq_r(address_space &space);
	uint8_t qix_video_firq_ack_r(address_space &space);
	uint8_t qix_videoram_r(offs_t offset);
	void qix_videoram_w(offs_t offset, uint8_t data);
	void slither_videoram_w(offs_t offset, uint8_t data);
	uint8_t qix_addresslatch_r(offs_t offset);
	void qix_addresslatch_w(offs_t offset, uint8_t data);
	void slither_addresslatch_w(offs_t offset, uint8_t data);
	void qix_paletteram_w(offs_t offset, uint8_t data);
	void qix_palettebank_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(pia_w_callback);
	TIMER_CALLBACK_MEMBER(deferred_sndpia1_porta_w);
	DECLARE_WRITE_LINE_MEMBER(qix_vsync_changed);
	void qix_pia_w(offs_t offset, uint8_t data);
	void qix_coinctl_w(uint8_t data);
	void slither_76489_0_w(uint8_t data);
	void slither_76489_1_w(uint8_t data);
	uint8_t slither_trak_lr_r();
	uint8_t slither_trak_ud_r();
	DECLARE_WRITE_LINE_MEMBER(display_enable_changed);
	DECLARE_WRITE_LINE_MEMBER(qix_flip_screen_w);
	void qix_dac_w(uint8_t data);
	void qix_vol_w(uint8_t data);
	void sndpia_2_warning_w(uint8_t data);
	void sync_sndpia1_porta_w(uint8_t data);
	void slither_coinctl_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(qix_pia_dint);
	DECLARE_WRITE_LINE_MEMBER(qix_pia_sint);
	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);
	void set_pen(int offs);
	int kram3_permut1(int idx, int value);
	int kram3_permut2(int tbl_index, int idx, const uint8_t *xor_table);
	int kram3_decrypt(int address, int value);
	DECLARE_WRITE_LINE_MEMBER(kram3_lic_maincpu_changed);
	DECLARE_WRITE_LINE_MEMBER(kram3_lic_videocpu_changed);

	void audio_map(address_map &map);
	void kram3_main_map(address_map &map);
	void kram3_video_map(address_map &map);
	void main_map(address_map &map);
	void qix_video_map(address_map &map);
	void slither_video_map(address_map &map);
};

class qixmcu_state : public qix_state
{
public:
	qixmcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		qix_state(mconfig, type, tag),
		m_mcu(*this, "mcu")
	{ }

	void mcu(machine_config &config);

protected:
	virtual void machine_start() override;

	optional_device<m68705p_device> m_mcu;

private:
	uint8_t coin_r();
	void coin_w(uint8_t data);
	void coinctrl_w(uint8_t data);

	uint8_t mcu_portb_r();
	uint8_t mcu_portc_r();
	void mcu_porta_w(uint8_t data);
	void mcu_portb_w(uint8_t data);

	/* machine state */
	uint8_t  m_68705_porta_out;
	uint8_t  m_coinctrl;
};

class zookeep_state : public qixmcu_state
{
public:
	zookeep_state(const machine_config &mconfig, device_type type, const char *tag) :
		qixmcu_state(mconfig, type, tag),
		m_vidbank(*this, "bank1")
	{ }

	void zookeep(machine_config &config);
	void zookeepbl(machine_config &config);
	void video(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void bankswitch_w(uint8_t data);

	void main_map(address_map &map);
	void video_map(address_map &map);

	required_memory_bank m_vidbank;
};

#endif // MAME_INCLUDES_QIX_H
