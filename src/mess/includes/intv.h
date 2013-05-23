/*****************************************************************************
 *
 * includes/intv.h
 *
 ****************************************************************************/

#ifndef INTV_H_
#define INTV_H_

#include "sound/ay8910.h"
#include "sound/sp0256.h"

struct intv_sprite_type
{
	int visible;
	int xpos;
	int ypos;
	int coll;
	int collision;
	int doublex;
	int doubley;
	int quady;
	int xflip;
	int yflip;
	int behind_foreground;
	int grom;
	int card;
	int color;
	int doubleyres;
	int dirty;
};

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
		m_intellivoice(*this, "sp0256_speech"),
		m_sound(*this, "ay8914.1"),
		m_ecs_sound(*this, "ay8914.2"),
		m_intvkbd_dualport_ram(*this, "dualport_ram"),
		m_videoram(*this, "videoram"),
		m_keyboard(*this, "keyboard"),
		m_region_maincpu(*this, "maincpu"),
		m_region_ecs_rom(*this, "ecs_rom"),
		m_region_keyboard(*this, "keyboard"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_io_keypad1(*this, "KEYPAD1"),
		m_io_disc1(*this, "DISC1"),
		m_io_discx1(*this, "DISCX1"),
		m_io_discy1(*this, "DISCY1"),
		m_io_keypad2(*this, "KEYPAD2"),
		m_io_disc2(*this, "DISC2"),
		m_io_discx2(*this, "DISCX2"),
		m_io_discy2(*this, "DISCY2"),
		m_io_options(*this, "OPTIONS"),
		m_io_ecs_row0(*this, "ECS_ROW0"),
		m_io_ecs_row1(*this, "ECS_ROW1"),
		m_io_ecs_row2(*this, "ECS_ROW2"),
		m_io_ecs_row3(*this, "ECS_ROW3"),
		m_io_ecs_row4(*this, "ECS_ROW4"),
		m_io_ecs_row5(*this, "ECS_ROW5"),
		m_io_ecs_row6(*this, "ECS_ROW6"),
		m_io_ecs_synth_row0(*this, "ECS_SYNTH_ROW0"),
		m_io_ecs_synth_row1(*this, "ECS_SYNTH_ROW1"),
		m_io_ecs_synth_row2(*this, "ECS_SYNTH_ROW2"),
		m_io_ecs_synth_row3(*this, "ECS_SYNTH_ROW3"),
		m_io_ecs_synth_row4(*this, "ECS_SYNTH_ROW4"),
		m_io_ecs_synth_row5(*this, "ECS_SYNTH_ROW5"),
		m_io_ecs_synth_row6(*this, "ECS_SYNTH_ROW6"),
		m_io_keypad3(*this, "KEYPAD3"),
		m_io_disc3(*this, "DISC3"),
		m_io_discx3(*this, "DISCX3"),
		m_io_discy3(*this, "DISCY3"),
		m_io_keypad4(*this, "KEYPAD4"),
		m_io_disc4(*this, "DISC4"),
		m_io_discx4(*this, "DISCX4"),
		m_io_discy4(*this, "DISCY4"),
		m_io_ecs_cntrlsel(*this, "ECS_CNTRLSEL"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_row2(*this, "ROW2"),
		m_io_row3(*this, "ROW3"),
		m_io_row4(*this, "ROW4"),
		m_io_row5(*this, "ROW5"),
		m_io_row6(*this, "ROW6"),
		m_io_row7(*this, "ROW7"),
		m_io_row8(*this, "ROW8"),
		m_io_row9(*this, "ROW9"),
		m_io_test(*this, "TEST") { }

	required_device<cpu_device> m_maincpu;
	required_device<sp0256_device> m_intellivoice;
	required_device<ay8914_device> m_sound;
	optional_device<ay8914_device> m_ecs_sound;
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

	DECLARE_READ16_MEMBER(intv_cart_ram8_r);
	DECLARE_WRITE16_MEMBER(intv_cart_ram8_w);

	DECLARE_READ8_MEMBER( intv_right_control_r );
	DECLARE_READ8_MEMBER( intv_left_control_r );

	bitmap_ind16 m_bitmap;

	intv_sprite_type m_sprite[STIC_MOBS];
	UINT8 m_sprite_buffers[STIC_MOBS][STIC_CARD_WIDTH*2][STIC_CARD_HEIGHT*4*2*2];
	UINT16 m_backtab_buffer[STIC_BACKTAB_HEIGHT][STIC_BACKTAB_WIDTH];
	UINT8 m_backtab_row;
	UINT8 m_bus_copy_mode;
	int m_color_stack_mode;
	int m_stic_registers[STIC_REGISTERS];
	int m_color_stack_offset;
	int m_stic_handshake;
	int m_border_color;
	int m_col_delay;
	int m_row_delay;
	int m_left_edge_inhibit;
	int m_top_edge_inhibit;
	UINT8 m_gramdirty;
	UINT8 m_gram[512];
	UINT8 m_gramdirtybytes[512];
	UINT16 m_ram16[0x160];
	int m_x_scale;
	int m_y_scale;
	int m_sr1_int_pending;
	UINT8 m_ram8[256];
	UINT8 m_cart_ram8[2048];

	// ecs
	DECLARE_WRITE16_MEMBER(ecs_bank1_page_select);
	DECLARE_WRITE16_MEMBER(ecs_bank2_page_select);
	DECLARE_WRITE16_MEMBER(ecs_bank3_page_select);
	DECLARE_WRITE16_MEMBER(wsmlb_bank_page_select);
	DECLARE_READ16_MEMBER(intv_ecs_ram8_r);
	DECLARE_WRITE16_MEMBER(intv_ecs_ram8_w);

	DECLARE_READ8_MEMBER( intv_ecs_porta_r );
	DECLARE_WRITE8_MEMBER( intv_ecs_porta_w );
	DECLARE_READ8_MEMBER( intv_ecs_portb_r );

	UINT8 m_ecs_ram8[2048];
	UINT8 m_ecs_psg_porta;

	// Keyboard Component
	DECLARE_READ8_MEMBER(intvkbd_tms9927_r);
	DECLARE_WRITE8_MEMBER(intvkbd_tms9927_w);
	DECLARE_WRITE16_MEMBER( intvkbd_dualport16_w );
	DECLARE_READ8_MEMBER( intvkbd_dualport8_lsb_r );
	DECLARE_WRITE8_MEMBER( intvkbd_dualport8_lsb_w );
	DECLARE_READ8_MEMBER( intvkbd_dualport8_msb_r );
	DECLARE_WRITE8_MEMBER( intvkbd_dualport8_msb_w );

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
	DECLARE_DRIVER_INIT(intvkbd);
	DECLARE_DRIVER_INIT(intv);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_RESET(intvecs);
	UINT32 screen_update_intv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_intvkbd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(intv_interrupt2);
	INTERRUPT_GEN_MEMBER(intv_interrupt);
	TIMER_CALLBACK_MEMBER(intv_interrupt2_complete);
	TIMER_CALLBACK_MEMBER(intv_interrupt_complete);
	TIMER_CALLBACK_MEMBER(intv_btb_fill);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( intv_cart );
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( intvkbd_cart );

protected:
	optional_device<cpu_device> m_keyboard;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_ecs_rom;
	optional_memory_region m_region_keyboard;
	optional_memory_bank m_bank1;
	optional_memory_bank m_bank2;
	optional_memory_bank m_bank3;
	optional_memory_bank m_bank4;
	required_ioport m_io_keypad1;
	required_ioport m_io_disc1;
	required_ioport m_io_discx1;
	required_ioport m_io_discy1;
	required_ioport m_io_keypad2;
	required_ioport m_io_disc2;
	required_ioport m_io_discx2;
	required_ioport m_io_discy2;
	required_ioport m_io_options;
	optional_ioport m_io_ecs_row0;
	optional_ioport m_io_ecs_row1;
	optional_ioport m_io_ecs_row2;
	optional_ioport m_io_ecs_row3;
	optional_ioport m_io_ecs_row4;
	optional_ioport m_io_ecs_row5;
	optional_ioport m_io_ecs_row6;
	optional_ioport m_io_ecs_synth_row0;
	optional_ioport m_io_ecs_synth_row1;
	optional_ioport m_io_ecs_synth_row2;
	optional_ioport m_io_ecs_synth_row3;
	optional_ioport m_io_ecs_synth_row4;
	optional_ioport m_io_ecs_synth_row5;
	optional_ioport m_io_ecs_synth_row6;
	optional_ioport m_io_keypad3;
	optional_ioport m_io_disc3;
	optional_ioport m_io_discx3;
	optional_ioport m_io_discy3;
	optional_ioport m_io_keypad4;
	optional_ioport m_io_disc4;
	optional_ioport m_io_discx4;
	optional_ioport m_io_discy4;
	optional_ioport m_io_ecs_cntrlsel;
	optional_ioport m_io_row0;
	optional_ioport m_io_row1;
	optional_ioport m_io_row2;
	optional_ioport m_io_row3;
	optional_ioport m_io_row4;
	optional_ioport m_io_row5;
	optional_ioport m_io_row6;
	optional_ioport m_io_row7;
	optional_ioport m_io_row8;
	optional_ioport m_io_row9;
	optional_ioport m_io_test;

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
