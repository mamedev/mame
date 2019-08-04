// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/******************************************************************************
 *  Microtan 65
 *
 *  variables and function prototypes
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http://www.geoff.org.uk/microtan/index.htm
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://oric.free.fr/microtan.html
 *
 ******************************************************************************/

#ifndef MAME_INCLUDES_MICROTAN_H
#define MAME_INCLUDES_MICROTAN_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "machine/input_merger.h"
#include "machine/timer.h"
#include "bus/tanbus/tanbus.h"
#include "imagedev/snapquik.h"

class microtan_state : public driver_device
{
public:
	microtan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irq_line(*this, "irq_line")
		, m_config(*this, "CONFIG")
		, m_io_keyboard(*this, "KBD%u", 0)
		, m_io_keypad(*this, "KPAD%u", 0)
		, m_keypad(*this, "KEYPAD")
		, m_tanbus(*this, "tanbus")
		, m_videoram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_gfx1(*this, "gfx1")
		, m_led(*this, "led1")
	{ }

	void mt65(machine_config &config);
	void micron(machine_config &config);
	void spinveti(machine_config &config);

	void init_gfx2();
	void init_microtan();

	TIMER_DEVICE_CALLBACK_MEMBER(kbd_scan);
	DECLARE_READ8_MEMBER(bffx_r);
	DECLARE_WRITE8_MEMBER(bffx_w);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);

protected:
	enum { IRQ_KBD, IRQ_TANBUS };

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irq_line;
	required_ioport m_config;
	optional_ioport_array<9> m_io_keyboard;
	optional_ioport_array<4> m_io_keypad;
	optional_ioport m_keypad;
	optional_device<tanbus_device> m_tanbus;

	uint8_t m_keypad_column;
	uint8_t m_keyboard_ascii;
	emu_timer *m_pulse_nmi_timer;
	uint8_t m_keyrows[10];
	int m_lastrow;
	int m_mask;
	int m_key;
	int m_repeat;
	int m_repeater;

	virtual void store_key(int key);

private:
	optional_shared_ptr<uint8_t> m_videoram;
	optional_device<gfxdecode_device> m_gfxdecode;
	optional_memory_region m_gfx1;
	output_finder<> m_led;

	uint8_t m_chunky_graphics;
	std::unique_ptr<uint8_t[]> m_chunky_buffer;
	tilemap_t *m_bg_tilemap;

	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_WRITE8_MEMBER(pgm_chargen_w);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(pulse_nmi);

	image_verify_result verify_snapshot(uint8_t *data, int size);
	image_init_result parse_intel_hex(uint8_t *snapshot_buff, char *src);
	image_init_result parse_zillion_hex(uint8_t *snapshot_buff, char *src);
	void set_cpu_regs(const uint8_t *snapshot_buff, int base);
	void snapshot_copy(uint8_t *snapshot_buff, int snapshot_size);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void mt65_map(address_map &map);
	void spinv_map(address_map &map);
};


class mt6809_state : public microtan_state
{
public:
	using microtan_state::microtan_state;

	void mt6809(machine_config &config);

protected:
	virtual void video_start() override;

	virtual void store_key(int key) override;

private:
	DECLARE_READ8_MEMBER(keyboard_r);

	void mt6809_map(address_map &map);
};

#endif // MAME_INCLUDES_MICROTAN_H
