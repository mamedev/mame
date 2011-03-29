/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

#include "video/tms34061.h"

#define YBUFFER_COUNT	15

class itech8_state : public driver_device
{
public:
	itech8_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *grom_bank;
	UINT8 blitter_int;
	UINT8 tms34061_int;
	UINT8 periodic_int;
	UINT8 sound_data;
	UINT8 pia_porta_data;
	UINT8 pia_portb_data;
	const rectangle *visarea;
	UINT8 z80_ctrl;
	UINT8 z80_port_val;
	UINT8 z80_clear_to_send;
	UINT16 sensor0;
	UINT16 sensor1;
	UINT16 sensor2;
	UINT16 sensor3;
	UINT8 curvx;
	UINT8 curvy;
	UINT8 curx;
	INT8 xbuffer[YBUFFER_COUNT];
	INT8 ybuffer[YBUFFER_COUNT];
	int ybuffer_next;
	int curxpos;
	int last_ytotal;
	UINT8 crosshair_vis;
	UINT8 blitter_data[16];
	UINT8 blit_in_progress;
	UINT8 page_select;
	offs_t fetch_offset;
	UINT8 fetch_rle_count;
	UINT8 fetch_rle_value;
	UINT8 fetch_rle_literal;
	struct tms34061_display tms_state;
	UINT8 *grom_base;
	UINT32 grom_size;
	UINT8 grmatch_palcontrol;
	UINT8 grmatch_xscroll;
	rgb_t grmatch_palette[2][16];
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
SCREEN_UPDATE( slikshot );


/*----------- defined in video/itech8.c -----------*/

VIDEO_START( itech8 );

WRITE8_HANDLER( itech8_page_w );

WRITE8_HANDLER( itech8_palette_w );

READ8_HANDLER( itech8_blitter_r );
WRITE8_HANDLER( itech8_blitter_w );

WRITE8_HANDLER( itech8_tms34061_w );
READ8_HANDLER( itech8_tms34061_r );

WRITE8_HANDLER( grmatch_palette_w );
WRITE8_HANDLER( grmatch_xscroll_w );
TIMER_DEVICE_CALLBACK( grmatch_palette_update );

SCREEN_UPDATE( itech8_2layer );
SCREEN_UPDATE( itech8_grmatch );
SCREEN_UPDATE( itech8_2page );
SCREEN_UPDATE( itech8_2page_large );
