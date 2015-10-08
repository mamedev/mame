// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Nathan Woods
/*****************************************************************************
 *
 * includes/dai.h
 *
 ****************************************************************************/

#ifndef DAI_H_
#define DAI_H_

#include "cpu/i8085/i8085.h"
#include "audio/dai_snd.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/tms5501.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"


class dai_state : public driver_device
{
public:
	enum
	{
		TIMER_BOOTSTRAP,
		TIMER_TMS5501
	};

	dai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pit(*this, "pit8253"),
		m_tms5501(*this, "tms5501"),
		m_sound(*this, "custom"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<tms5501_device> m_tms5501;
	required_device<dai_sound_device> m_sound;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;

	UINT8 m_paddle_select;
	UINT8 m_paddle_enable;
	UINT8 m_cassette_motor[2];
	UINT8 m_keyboard_scan_mask;
	unsigned short m_4_colours_palette[4];
	DECLARE_WRITE8_MEMBER(dai_stack_interrupt_circuit_w);
	DECLARE_READ8_MEMBER(dai_io_discrete_devices_r);
	DECLARE_WRITE8_MEMBER(dai_io_discrete_devices_w);
	DECLARE_READ8_MEMBER(dai_amd9511_r);
	DECLARE_WRITE8_MEMBER(dai_amd9511_w);
	DECLARE_READ8_MEMBER(dai_pit_r);
	DECLARE_WRITE8_MEMBER(dai_pit_w);
	DECLARE_READ8_MEMBER(dai_keyboard_r);
	DECLARE_WRITE8_MEMBER(dai_keyboard_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(dai);
	UINT32 screen_update_dai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dai_update_memory(int dai_rom_bank);
	IRQ_CALLBACK_MEMBER(int_ack);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};


/*----------- defined in video/dai.c -----------*/

extern const unsigned char dai_palette[16*3];


#endif /* DAI_H_ */
