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

#include "imagedev/snapquik.h"
#include "machine/6522via.h"
#include "imagedev/cassette.h"

class microtan_state : public driver_device
{
public:
	enum
	{
		TIMER_READ_CASSETTE,
		TIMER_PULSE_NMI
	};

	microtan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_via6522_0(*this, "via6522_0"),
		m_via6522_1(*this, "via6522_1"),
		m_gfxdecode(*this, "gfxdecode"),
		m_led(*this, "led1")
	{ }

	DECLARE_READ8_MEMBER(microtan_sound_r);
	DECLARE_WRITE8_MEMBER(microtan_sound_w);
	DECLARE_READ8_MEMBER(microtan_bffx_r);
	DECLARE_WRITE8_MEMBER(microtan_bffx_w);
	DECLARE_WRITE8_MEMBER(microtan_videoram_w);
	void init_microtan();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update_microtan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(microtan_interrupt);
	TIMER_CALLBACK_MEMBER(microtan_read_cassette);
	TIMER_CALLBACK_MEMBER(microtan_pulse_nmi);
	DECLARE_READ8_MEMBER(via_0_in_a);
	DECLARE_WRITE8_MEMBER(via_0_out_a);
	DECLARE_WRITE8_MEMBER(via_0_out_b);
	DECLARE_WRITE_LINE_MEMBER(via_0_out_ca2);
	DECLARE_WRITE_LINE_MEMBER(via_0_out_cb2);
	DECLARE_WRITE8_MEMBER(via_1_out_a);
	DECLARE_WRITE8_MEMBER(via_1_out_b);
	DECLARE_WRITE_LINE_MEMBER(via_1_out_ca2);
	DECLARE_WRITE_LINE_MEMBER(via_1_out_cb2);
	DECLARE_WRITE_LINE_MEMBER(via_0_irq);
	DECLARE_WRITE_LINE_MEMBER(via_1_irq);
	uint8_t read_dsw();
	void microtan_set_irq_line();
	void store_key(int key);
	image_verify_result microtan_verify_snapshot(uint8_t *data, int size);
	image_init_result parse_intel_hex(uint8_t *snapshot_buff, char *src);
	image_init_result parse_zillion_hex(uint8_t *snapshot_buff, char *src);
	void microtan_set_cpu_regs(const uint8_t *snapshot_buff, int base);
	void microtan_snapshot_copy(uint8_t *snapshot_buff, int snapshot_size);
	DECLARE_SNAPSHOT_LOAD_MEMBER( microtan );
	DECLARE_QUICKLOAD_LOAD_MEMBER( microtan );

	void microtan(machine_config &config);
	void microtan_map(address_map &map);
protected:
	virtual void machine_start() override { m_led.resolve(); }
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_shared_ptr<uint8_t> m_videoram;
	uint8_t m_chunky_graphics;
	std::unique_ptr<uint8_t[]> m_chunky_buffer;
	uint8_t m_keypad_column;
	uint8_t m_keyboard_ascii;
	emu_timer *m_timer;
	int m_via_0_irq_line;
	int m_via_1_irq_line;
	int m_kbd_irq_line;
	uint8_t m_keyrows[10];
	int m_lastrow;
	int m_mask;
	int m_key;
	int m_repeat;
	int m_repeater;
	tilemap_t *m_bg_tilemap;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<via6522_device> m_via6522_0;
	required_device<via6522_device> m_via6522_1;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<> m_led;
};

#endif // MAME_INCLUDES_MICROTAN_H
