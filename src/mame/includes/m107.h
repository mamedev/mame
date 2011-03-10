/*************************************************************************

    Irem M107 hardware

*************************************************************************/


typedef struct _pf_layer_info pf_layer_info;
struct _pf_layer_info
{
	tilemap_t *		tmap;
	UINT16			vram_base;
	UINT16			control[4];
};

class m107_state : public driver_device
{
public:
	m107_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	emu_timer *scanline_timer;
	UINT8 irq_vectorbase;
	int irqvector;
	int sound_status;
	UINT16 *vram_data;
	UINT8 spritesystem;
	UINT8 sprite_display;
	UINT16 raster_irq_position;
	pf_layer_info pf_layer[4];
	UINT16 control[0x10];
	UINT16 *spriteram;
	UINT16 *buffered_spriteram;
};


/*----------- defined in video/m107.c -----------*/

WRITE16_HANDLER( m107_spritebuffer_w );
SCREEN_UPDATE( m107 );
VIDEO_START( m107 );
WRITE16_HANDLER( m107_control_w );
WRITE16_HANDLER( m107_vram_w );
