/*****************************************************************************
 *
 * includes/cbmb.h
 *
 * Commodore B Series Computer
 *
 * peter.trauner@jk.uni-linz.ac.at
 *
 ****************************************************************************/

#ifndef CBMB_H_
#define CBMB_H_

#include "video/mc6845.h"
#include "machine/6526cia.h"
#include "machine/ieee488.h"
#include "imagedev/cartslot.h"

class cbmb_state : public driver_device
{
public:
	cbmb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_ieee(*this, IEEE488_TAG),
		m_basic(*this, "basic"),
		m_videoram(*this, "videoram"),
		m_kernal(*this, "kernal"),
		m_colorram(*this, "colorram"){ }

	required_device<ieee488_device> m_ieee;
	required_shared_ptr<UINT8> m_basic;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_kernal;
	optional_shared_ptr<UINT8> m_colorram;

	DECLARE_READ8_MEMBER( vic_lightpen_x_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_y_cb );
	DECLARE_READ8_MEMBER( vic_lightpen_button_cb );
	DECLARE_READ8_MEMBER( vic_dma_read );
	DECLARE_READ8_MEMBER( vic_dma_read_color );
	DECLARE_READ8_MEMBER( vic_rdy_cb );

	/* keyboard lines */
	int m_cbmb_keyline_a;
	int m_cbmb_keyline_b;
	int m_cbmb_keyline_c;

	int m_p500;
	int m_cbm700;
	int m_cbm_ntsc;
	int m_keyline_a;
	int m_keyline_b;
	int m_keyline_c;
	UINT8 *m_chargen;
	int m_old_level;
	int m_irq_level;
	int m_font;
	DECLARE_WRITE8_MEMBER(cbmb_colorram_w);
	DECLARE_DRIVER_INIT(cbm600);
	DECLARE_DRIVER_INIT(p500);
	DECLARE_DRIVER_INIT(cbm600hu);
	DECLARE_DRIVER_INIT(cbm600pal);
	DECLARE_DRIVER_INIT(cbm700);
	DECLARE_MACHINE_RESET(cbmb);
	DECLARE_VIDEO_START(cbmb_crtc);
	DECLARE_PALETTE_INIT(cbm700);
	DECLARE_VIDEO_START(cbm700);
};

/*----------- defined in machine/cbmb.c -----------*/

extern const mos6526_interface cbmb_cia;


READ8_DEVICE_HANDLER( cbmb_tpi0_port_a_r );
WRITE8_DEVICE_HANDLER( cbmb_tpi0_port_a_w );
READ8_DEVICE_HANDLER( cbmb_tpi0_port_b_r );
WRITE8_DEVICE_HANDLER( cbmb_tpi0_port_b_w );

WRITE8_DEVICE_HANDLER( cbmb_keyboard_line_select_a );
WRITE8_DEVICE_HANDLER( cbmb_keyboard_line_select_b );
WRITE8_DEVICE_HANDLER( cbmb_keyboard_line_select_c );
READ8_DEVICE_HANDLER( cbmb_keyboard_line_a );
READ8_DEVICE_HANDLER( cbmb_keyboard_line_b );
READ8_DEVICE_HANDLER( cbmb_keyboard_line_c );
WRITE_LINE_DEVICE_HANDLER( cbmb_irq );

int cbmb_dma_read(running_machine &machine, int offset);
int cbmb_dma_read_color(running_machine &machine, int offset);

WRITE_LINE_DEVICE_HANDLER( cbmb_change_font );



MACHINE_CONFIG_EXTERN( cbmb_cartslot );


/*----------- defined in video/cbmb.c -----------*/


MC6845_UPDATE_ROW( cbm600_update_row );
MC6845_UPDATE_ROW( cbm700_update_row );
WRITE_LINE_DEVICE_HANDLER( cbmb_display_enable_changed );

void cbm600_vh_init(running_machine &machine);
void cbm700_vh_init(running_machine &machine);


void cbmb_vh_set_font(running_machine &machine, int font);


#endif /* CBMB_H_ */
