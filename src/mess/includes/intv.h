/*****************************************************************************
 *
 * includes/intv.h
 *
 ****************************************************************************/

#ifndef INTV_H_
#define INTV_H_

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
	intv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_intellivoice(*this, "sp0256_speech"),
	m_sound(*this, "ay8914.1"),
	m_ecs_sound(*this, "ay8914.2"),
	m_intvkbd_dualport_ram(*this, "dualport_ram"),
	m_videoram(*this, "videoram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_intellivoice;
	required_device<device_t> m_sound;
	optional_device<device_t> m_ecs_sound;
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
};

/*----------- defined in video/intv.c -----------*/

extern VIDEO_START( intv );
extern SCREEN_UPDATE_IND16( intv );
extern SCREEN_UPDATE_IND16( intvkbd );

void intv_stic_screenrefresh(running_machine &machine);

/*----------- defined in machine/intv.c -----------*/

/*  for the console alone... */

DEVICE_START( intv_cart );
DEVICE_IMAGE_LOAD( intv_cart );

extern MACHINE_RESET( intv );
extern INTERRUPT_GEN( intv_interrupt );

// ECS
extern MACHINE_RESET( intvecs );

/* for the console + keyboard component... */

DEVICE_IMAGE_LOAD( intvkbd_cart );

#endif /* INTV_H_ */
