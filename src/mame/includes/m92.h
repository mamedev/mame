/*************************************************************************

    Irem M92 hardware

*************************************************************************/

typedef struct _pf_layer_info pf_layer_info;
struct _pf_layer_info
{
	tilemap_t *		tmap;
	tilemap_t *		wide_tmap;
	UINT16			vram_base;
	UINT16			control[4];
};

class m92_state : public driver_device
{
public:
	m92_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 irqvector;
	UINT16 sound_status;
	UINT32 bankaddress;
	emu_timer *scanline_timer;
	UINT8 irq_vectorbase;
	UINT32 raster_irq_position;
	UINT16 *vram_data;
	UINT16 *spritecontrol;
	UINT8 sprite_buffer_busy;
	UINT8 game_kludge;
	pf_layer_info pf_layer[3];
	UINT16 pf_master_control[4];
	INT32 sprite_list;
	int palette_bank;
};


/*----------- defined in drivers/m92.c -----------*/

extern void m92_sprite_interrupt(running_machine *machine);


/*----------- defined in video/m92.c -----------*/

WRITE16_HANDLER( m92_spritecontrol_w );
WRITE16_HANDLER( m92_videocontrol_w );
READ16_HANDLER( m92_paletteram_r );
WRITE16_HANDLER( m92_paletteram_w );
WRITE16_HANDLER( m92_vram_w );
WRITE16_HANDLER( m92_pf1_control_w );
WRITE16_HANDLER( m92_pf2_control_w );
WRITE16_HANDLER( m92_pf3_control_w );
WRITE16_HANDLER( m92_master_control_w );

VIDEO_START( m92 );
VIDEO_UPDATE( m92 );
