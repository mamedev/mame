/*****************************************************************************
 *
 * includes/a7800.h
 *
 ****************************************************************************/

#ifndef A7800_H_
#define A7800_H_

#include "machine/6532riot.h"

class a7800_state : public driver_device
{
public:
	a7800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	int m_lines;
	int m_ispal;
	unsigned char *m_cart_bkup;
	unsigned char *m_bios_bkup;
	int m_ctrl_lock;
	int m_ctrl_reg;
	int m_maria_flag;
	unsigned char *m_cartridge_rom;
	UINT16 m_cart_type;
	UINT32 m_cart_size;
	unsigned char m_stick_type;
	UINT8 *m_ROM;
	int m_maria_palette[8][4];
	int m_maria_write_mode;
	int m_maria_scanline;
	unsigned int m_maria_dll;
	unsigned int m_maria_dl;
	int m_maria_holey;
	int m_maria_offset;
	int m_maria_vblank;
	int m_maria_dli;
	int m_maria_dmaon;
	int m_maria_dodma;
	int m_maria_wsync;
	int m_maria_backcolor;
	int m_maria_color_kill;
	int m_maria_cwidth;
	int m_maria_bcntl;
	int m_maria_kangaroo;
	int m_maria_rm;
	unsigned int m_maria_charbase;
	bitmap_ind16 m_bitmap;
	int m_p1_one_button;
	int m_p2_one_button;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(a7800_RAM0_w);
	DECLARE_WRITE8_MEMBER(a7800_cart_w);
	DECLARE_READ8_MEMBER(a7800_TIA_r);
	DECLARE_WRITE8_MEMBER(a7800_TIA_w);
	DECLARE_READ8_MEMBER(a7800_MARIA_r);
	DECLARE_WRITE8_MEMBER(a7800_MARIA_w);
	DECLARE_DRIVER_INIT(a7800_pal);
	DECLARE_DRIVER_INIT(a7800_ntsc);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_PALETTE_INIT(a7800p);
	UINT32 screen_update_a7800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(a7800_interrupt);
	DECLARE_READ8_MEMBER(riot_joystick_r);
	DECLARE_READ8_MEMBER(riot_console_button_r);
	DECLARE_WRITE8_MEMBER(riot_button_pullup_w);
};

/*----------- defined in machine/a7800.c -----------*/

extern const riot6532_interface a7800_r6532_interface;

void a7800_partialhash(hash_collection &dest, const unsigned char *data, unsigned long length, const char *functions);

DEVICE_START( a7800_cart );
DEVICE_IMAGE_LOAD( a7800_cart );

#endif /* A7800_H_ */
