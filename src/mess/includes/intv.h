// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
/*****************************************************************************
 *
 * includes/intv.h
 *
 ****************************************************************************/

#ifndef INTV_H_
#define INTV_H_

#include "sound/ay8910.h"
#include "video/stic.h"
#include "bus/intv/slot.h"
#include "bus/intv/voice.h"
#include "bus/intv/ecs.h"
//#include "bus/intv/keycomp.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class intv_state : public driver_device
{
public:
	enum
	{
		TIMER_INTV_INTERRUPT2_COMPLETE,
		TIMER_INTV_INTERRUPT_COMPLETE,
		TIMER_INTV_BTB_FILL
	};

	intv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "ay8914"),
		m_stic(*this, "stic"),
		m_cart(*this, "cartslot"),
		m_intvkbd_dualport_ram(*this, "dualport_ram"),
		m_videoram(*this, "videoram"),
		m_keyboard(*this, "keyboard"),
		m_iocart1(*this, "ioslot1"),
		m_iocart2(*this, "ioslot2"),
		m_region_maincpu(*this, "maincpu"),
		m_region_keyboard(*this, "keyboard"),
		m_io_options(*this, "OPTIONS"),
		m_io_test(*this, "TEST"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ay8914_device> m_sound;
	required_device<stic_device> m_stic;
	optional_device<intv_cart_slot_device> m_cart;
	optional_shared_ptr<UINT16> m_intvkbd_dualport_ram;
	optional_shared_ptr<UINT8> m_videoram;

	DECLARE_READ16_MEMBER(intv_stic_r);
	DECLARE_WRITE16_MEMBER(intv_stic_w);
	DECLARE_READ16_MEMBER(intv_gram_r);
	DECLARE_WRITE16_MEMBER(intv_gram_w);
	DECLARE_READ16_MEMBER(intv_ram8_r);
	DECLARE_WRITE16_MEMBER(intv_ram8_w);
	DECLARE_READ16_MEMBER(intv_ram16_r);
	DECLARE_WRITE16_MEMBER(intv_ram16_w);
	DECLARE_READ8_MEMBER(intvkb_iocart_r);

	DECLARE_READ8_MEMBER(intv_right_control_r);
	DECLARE_READ8_MEMBER(intv_left_control_r);

	UINT8 m_bus_copy_mode;
	UINT8 m_backtab_row;
	UINT16 m_ram16[0x160];
	int m_sr1_int_pending;
	UINT8 m_ram8[256];

	// Keyboard Component
	DECLARE_READ8_MEMBER(intvkbd_tms9927_r);
	DECLARE_WRITE8_MEMBER(intvkbd_tms9927_w);
	DECLARE_WRITE16_MEMBER(intvkbd_dualport16_w);
	DECLARE_READ8_MEMBER(intvkbd_dualport8_lsb_r);
	DECLARE_WRITE8_MEMBER(intvkbd_dualport8_lsb_w);
	DECLARE_READ8_MEMBER(intvkbd_dualport8_msb_r);
	DECLARE_WRITE8_MEMBER(intvkbd_dualport8_msb_w);

	UINT8 m_tms9927_num_rows;
	UINT8 m_tms9927_cursor_col;
	UINT8 m_tms9927_cursor_row;
	UINT8 m_tms9927_last_row;

	int m_intvkbd_text_blanked;
	int m_intvkbd_keyboard_col;
	int m_tape_int_pending;
	int m_tape_interrupts_enabled;
	int m_tape_unknown_write[6];
	int m_tape_motor_mode;
	DECLARE_DRIVER_INIT(intvecs);
	DECLARE_DRIVER_INIT(intvkbd);
	DECLARE_DRIVER_INIT(intv);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(intv);
	void ecs_banks_restore();
	UINT32 screen_update_intv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_intvkbd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(intv_interrupt2);
	INTERRUPT_GEN_MEMBER(intv_interrupt);
	TIMER_CALLBACK_MEMBER(intv_interrupt2_complete);
	TIMER_CALLBACK_MEMBER(intv_interrupt_complete);
	TIMER_CALLBACK_MEMBER(intv_btb_fill);

protected:
	int m_is_keybd;

	optional_device<cpu_device> m_keyboard;
	optional_device<generic_slot_device> m_iocart1;
	optional_device<generic_slot_device> m_iocart2;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_keyboard;
	required_ioport m_io_options;
	optional_ioport m_io_test;

	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	ioport_port *m_keypad[4];
	ioport_port *m_disc[4];
	ioport_port *m_discx[4];
	ioport_port *m_discy[4];
	ioport_port *m_intv_keyboard[10];

	int intv_load_rom_file(device_image_interface &image);
	UINT8 intv_control_r(int hand);
	void intv_set_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);
	UINT32 intv_get_pixel(bitmap_ind16 &bitmap, int x, int y);
	void intv_plot_box(bitmap_ind16 &bm, int x, int y, int w, int h, int color);
	int sprites_collide(int spriteNum1, int spriteNum2);
	void determine_sprite_collisions();
	void render_sprites();
	void render_line(bitmap_ind16 &bitmap, UINT8 nextByte, UINT16 x, UINT16 y, UINT8 fgcolor, UINT8 bgcolor);
	void render_colored_squares(bitmap_ind16 &bitmap, UINT16 x, UINT16 y, UINT8 color0, UINT8 color1, UINT8 color2, UINT8 color3);
	void render_color_stack_mode(bitmap_ind16 &bitmap);
	void render_fg_bg_mode(bitmap_ind16 &bitmap);
	void copy_sprites_to_background(bitmap_ind16 &bitmap);
	void render_background(bitmap_ind16 &bitmap);
	void draw_borders(bitmap_ind16 &bm);
	void intv_stic_screenrefresh();
	void draw_background(bitmap_ind16 &bitmap, int transparency);
	void draw_sprites(bitmap_ind16 &bitmap, int behind_foreground);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

#endif /* INTV_H_ */
