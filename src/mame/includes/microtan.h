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
 *  for his site http://www.geo255.redhotant.com
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://www.ifrance.com/oric/microtan.html
 *
 ******************************************************************************/

#ifndef MAME_INCLUDES_MICROTAN_H
#define MAME_INCLUDES_MICROTAN_H

#pragma once

#include "imagedev/snapquik.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "sound/ay8910.h"
#include "imagedev/cassette.h"

class microtan_state : public driver_device
{
public:
	microtan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_irq_line(*this, "irq_line"),
		m_cassette(*this, "cassette"),
		m_via6522(*this, "via6522%u", 0),
		m_ay8910(*this, "ay8910%u", 0),
		m_gfxdecode(*this, "gfxdecode"),
		m_led(*this, "led1")
	{ }

	void microtan(machine_config &config);

	void init_microtan();

protected:
	enum
	{
		TIMER_READ_CASSETTE,
		TIMER_PULSE_NMI
	};

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum { IRQ_VIA_0, IRQ_VIA_1, IRQ_KBD };

	required_shared_ptr<uint8_t> m_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irq_line;
	required_device<cassette_image_device> m_cassette;
	required_device_array<via6522_device, 2> m_via6522;
	required_device_array<ay8910_device, 2> m_ay8910;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<> m_led;

	uint8_t m_chunky_graphics;
	std::unique_ptr<uint8_t[]> m_chunky_buffer;
	uint8_t m_keypad_column;
	uint8_t m_keyboard_ascii;
	emu_timer *m_read_cassette_timer;
	emu_timer *m_pulse_nmi_timer;
	uint8_t m_keyrows[10];
	int m_lastrow;
	int m_mask;
	int m_key;
	int m_repeat;
	int m_repeater;
	tilemap_t *m_bg_tilemap;

	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(bffx_r);
	DECLARE_WRITE8_MEMBER(bffx_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(read_cassette);
	TIMER_CALLBACK_MEMBER(pulse_nmi);
	DECLARE_READ8_MEMBER(via_0_in_a);
	DECLARE_WRITE8_MEMBER(via_0_out_a);
	DECLARE_WRITE8_MEMBER(via_0_out_b);
	DECLARE_WRITE_LINE_MEMBER(via_0_out_ca2);
	DECLARE_WRITE_LINE_MEMBER(via_0_out_cb2);
	DECLARE_WRITE8_MEMBER(via_1_out_a);
	DECLARE_WRITE8_MEMBER(via_1_out_b);
	DECLARE_WRITE_LINE_MEMBER(via_1_out_ca2);
	DECLARE_WRITE_LINE_MEMBER(via_1_out_cb2);
	uint8_t read_dsw();
	void store_key(int key);
	image_verify_result verify_snapshot(uint8_t *data, int size);
	image_init_result parse_intel_hex(uint8_t *snapshot_buff, char *src);
	image_init_result parse_zillion_hex(uint8_t *snapshot_buff, char *src);
	void set_cpu_regs(const uint8_t *snapshot_buff, int base);
	void snapshot_copy(uint8_t *snapshot_buff, int snapshot_size);
	DECLARE_SNAPSHOT_LOAD_MEMBER( microtan );
	DECLARE_QUICKLOAD_LOAD_MEMBER( microtan );

	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_MICROTAN_H
