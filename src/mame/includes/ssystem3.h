// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/ssystem3.h
 *
 ****************************************************************************/

#ifndef SSYSTEM3_H_
#define SSYSTEM3_H_

#include "machine/6522via.h"


struct playfield_t
{
	int signal;
	//  int on;

	int count, bit, started;
	uint8_t data;
	attotime time, high_time, low_time;
	union {
		struct {
			uint8_t header[7];
			uint8_t field[8][8/2];
			uint8_t unknown[5];
		} s;
		uint8_t data[7+8*8/2+5];
	} u;
};

struct lcd_t
{
	uint8_t data[5];
	int clock;
	int count;
};


class ssystem3_state : public driver_device
{
public:
	ssystem3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_via6522_0(*this, "via6522_0")
		, m_configuration(*this, "Configuration")
		, m_matrix(*this, "matrix.%u", 0)
	{ }

	void init_ssystem3();
	virtual void video_start() override;
	void palette_init_ssystem3(palette_device &palette);
	uint32_t screen_update_ssystem3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ssystem3_via_write_a(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ssystem3_via_read_a(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ssystem3_via_read_b(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ssystem3_via_write_b(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ssystem3_lcd_reset();
	void ssystem3_lcd_write(int clock, int data);
	void ssystem3_draw_7segment(bitmap_ind16 &bitmap,int value, int x, int y);
	void ssystem3_draw_led(bitmap_ind16 &bitmap,int16_t color, int x, int y, int ch);
	void ssystem3_playfield_getfigure(int x, int y, int *figure, int *black);
	void ssystem3_playfield_reset();
	void ssystem3_playfield_write(int reset, int signal);
	void ssystem3_playfield_read(int *on, int *ready);

private:
	uint8_t m_porta;
	std::unique_ptr<uint8_t[]> m_videoram;
	playfield_t m_playfield;
	lcd_t m_lcd;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<via6522_device> m_via6522_0;
	required_ioport m_configuration;
	required_ioport_array<4> m_matrix;
};


#endif /* SSYSTEM3_H_ */
