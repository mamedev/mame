/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

#include "video/tms34061.h"

#define YBUFFER_COUNT	15

class itech8_state : public driver_device
{
public:
	itech8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_visarea(0, 0, 0, 0) { }

	UINT8 *m_grom_bank;
	UINT8 m_blitter_int;
	UINT8 m_tms34061_int;
	UINT8 m_periodic_int;
	UINT8 m_sound_data;
	UINT8 m_pia_porta_data;
	UINT8 m_pia_portb_data;
	rectangle m_visarea;
	UINT8 m_z80_ctrl;
	UINT8 m_z80_port_val;
	UINT8 m_z80_clear_to_send;
	UINT16 m_sensor0;
	UINT16 m_sensor1;
	UINT16 m_sensor2;
	UINT16 m_sensor3;
	UINT8 m_curvx;
	UINT8 m_curvy;
	UINT8 m_curx;
	INT8 m_xbuffer[YBUFFER_COUNT];
	INT8 m_ybuffer[YBUFFER_COUNT];
	int m_ybuffer_next;
	int m_curxpos;
	int m_last_ytotal;
	UINT8 m_crosshair_vis;
	UINT8 m_blitter_data[16];
	UINT8 m_blit_in_progress;
	UINT8 m_page_select;
	offs_t m_fetch_offset;
	UINT8 m_fetch_rle_count;
	UINT8 m_fetch_rle_value;
	UINT8 m_fetch_rle_literal;
	struct tms34061_display m_tms_state;
	UINT8 *m_grom_base;
	UINT32 m_grom_size;
	UINT8 m_grmatch_palcontrol;
	UINT8 m_grmatch_xscroll;
	rgb_t m_grmatch_palette[2][16];
	DECLARE_WRITE8_MEMBER(itech8_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(blitter_w);
	DECLARE_WRITE8_MEMBER(rimrockn_bank_w);
	DECLARE_WRITE8_MEMBER(pia_portb_out);
	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_WRITE8_MEMBER(gtg2_sound_data_w);
	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE16_MEMBER(grom_bank16_w);
	DECLARE_WRITE16_MEMBER(display_page16_w);
	DECLARE_WRITE16_MEMBER(palette16_w);
	DECLARE_WRITE8_MEMBER(itech8_palette_w);
	DECLARE_WRITE8_MEMBER(itech8_page_w);
	DECLARE_READ8_MEMBER(itech8_blitter_r);
	DECLARE_WRITE8_MEMBER(itech8_blitter_w);
	DECLARE_WRITE8_MEMBER(itech8_tms34061_w);
	DECLARE_READ8_MEMBER(itech8_tms34061_r);
	DECLARE_WRITE8_MEMBER(grmatch_palette_w);
	DECLARE_WRITE8_MEMBER(grmatch_xscroll_w);
};


/*----------- defined in drivers/itech8.c -----------*/

void itech8_update_interrupts(running_machine &machine, int periodic, int tms34061, int blitter);


/*----------- defined in machine/slikshot.c -----------*/

READ8_HANDLER( slikz80_port_r );
WRITE8_HANDLER( slikz80_port_w );

READ8_HANDLER( slikshot_z80_r );
READ8_HANDLER( slikshot_z80_control_r );
WRITE8_HANDLER( slikshot_z80_control_w );

VIDEO_START( slikshot );
SCREEN_UPDATE_RGB32( slikshot );


/*----------- defined in video/itech8.c -----------*/

VIDEO_START( itech8 );





TIMER_DEVICE_CALLBACK( grmatch_palette_update );

SCREEN_UPDATE_RGB32( itech8_2layer );
SCREEN_UPDATE_RGB32( itech8_grmatch );
SCREEN_UPDATE_RGB32( itech8_2page );
SCREEN_UPDATE_RGB32( itech8_2page_large );
