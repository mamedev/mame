// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*****************************************************************************
 *
 * includes/llc.h
 *
 ****************************************************************************/

#ifndef LLC_H_
#define LLC_H_

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/ram.h"
#include "machine/k7659kb.h"
#include "sound/speaker.h"

class llc_state : public driver_device
{
public:
	llc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_speaker(*this, "speaker"),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	DECLARE_WRITE8_MEMBER(llc2_rom_disable_w);
	DECLARE_WRITE8_MEMBER(llc2_basic_enable_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(llc1_port1_a_r);
	DECLARE_READ8_MEMBER(llc1_port2_a_r);
	DECLARE_READ8_MEMBER(llc1_port2_b_r);
	DECLARE_WRITE8_MEMBER(llc1_port1_a_w);
	DECLARE_WRITE8_MEMBER(llc1_port1_b_w);
	DECLARE_READ8_MEMBER(llc2_port1_b_r);
	DECLARE_READ8_MEMBER(llc2_port2_a_r);
	DECLARE_WRITE8_MEMBER(llc2_port1_b_w);
	const uint8_t *m_p_chargen;
	optional_device<speaker_sound_device> m_speaker;
	optional_shared_ptr<uint8_t> m_p_videoram;
	bool m_rv;
	uint8_t m_term_status;
	uint8_t m_llc1_key;
private:
	uint8_t m_porta;
	uint8_t m_term_data;
public:
	DECLARE_DRIVER_INIT(llc2);
	DECLARE_DRIVER_INIT(llc1);
	virtual void video_start() override;
	DECLARE_MACHINE_START(llc1);
	DECLARE_MACHINE_RESET(llc1);
	DECLARE_MACHINE_RESET(llc2);
	uint32_t screen_update_llc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_llc2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	optional_device<ram_device> m_ram;
};

#endif
