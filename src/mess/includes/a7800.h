/*****************************************************************************
 *
 * includes/a7800.h
 *
 ****************************************************************************/

#ifndef A7800_H_
#define A7800_H_

#include "machine/6532riot.h"
#include "sound/tiasound.h"
#include "sound/tiaintf.h"
#include "bus/a7800/a78_slot.h"


class a7800_state : public driver_device
{
public:
	a7800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tia(*this, "tia"),
		m_io_joysticks(*this, "joysticks"),
		m_io_buttons(*this, "buttons"),
		m_io_vblank(*this, "vblank"),
		m_io_console_buttons(*this, "console_buttons"),
		m_cartslot(*this, "cartslot"),
		m_screen(*this, "screen") { }

	int m_lines;
	int m_ispal;

	int m_ctrl_lock;
	int m_ctrl_reg;
	int m_maria_flag;
	int m_p1_one_button;
	int m_p2_one_button;
	int m_bios_enabled;

	UINT8 *m_bios;

	int m_maria_palette[32];
	int m_line_ram[2][160];
	int m_active_buffer;
	int m_maria_write_mode;
	unsigned int m_maria_dll;
	unsigned int m_maria_dl;
	int m_maria_holey;
	int m_maria_offset;
	int m_maria_vblank;
	int m_maria_dli;
	int m_maria_dmaon;
	int m_maria_dpp;
	int m_maria_wsync;
	int m_maria_backcolor;
	int m_maria_color_kill;
	int m_maria_cwidth;
	int m_maria_bcntl;
	int m_maria_kangaroo;
	int m_maria_rm;
	int m_maria_nmi;
	unsigned int m_maria_charbase;
	bitmap_ind16 m_bitmap;

	DECLARE_READ8_MEMBER(bios_or_cart_r);
	DECLARE_WRITE8_MEMBER(ram0_w);
	DECLARE_READ8_MEMBER(tia_r);
	DECLARE_WRITE8_MEMBER(tia_w);
	DECLARE_READ8_MEMBER(maria_r);
	DECLARE_WRITE8_MEMBER(maria_w);
	DECLARE_DRIVER_INIT(a7800_pal);
	DECLARE_DRIVER_INIT(a7800_ntsc);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(a7800);
	DECLARE_PALETTE_INIT(a7800p);
	UINT32 screen_update_a7800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(a7800_interrupt);
	TIMER_CALLBACK_MEMBER(a7800_maria_startdma);
	DECLARE_READ8_MEMBER(riot_joystick_r);
	DECLARE_READ8_MEMBER(riot_console_button_r);
	DECLARE_WRITE8_MEMBER(riot_button_pullup_w);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<tia_device> m_tia;
	required_ioport m_io_joysticks;
	required_ioport m_io_buttons;
	required_ioport m_io_vblank;
	required_ioport m_io_console_buttons;
	required_device<a78_cart_slot_device> m_cartslot;
	required_device<screen_device> m_screen;

	void maria_draw_scanline();
	int is_holey(unsigned int addr);
	int write_line_ram(int addr, UINT8 offset, int pal);
};

#endif
