class taitosj_state : public driver_device
{
public:
	taitosj_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 sndnmi_disable;
	UINT8 input_port_4_f0;
	UINT8 kikstart_gears[2];
	INT8 dac_out;
	UINT8 dac_vol;
	UINT8 *videoram_1;
	UINT8 *videoram_2;
	UINT8 *videoram_3;
	UINT8 *spriteram;
	UINT8 *paletteram;
	UINT8 *characterram;
	UINT8 *scroll;
	UINT8 *colscrolly;
	UINT8 *gfxpointer;
	UINT8 *colorbank;
	UINT8 *video_mode;
	UINT8 *video_priority;
	UINT8 *collision_reg;
	UINT8 *kikstart_scrollram;
	UINT8 fromz80;
	UINT8 toz80;
	UINT8 zaccept;
	UINT8 zready;
	UINT8 busreq;
	UINT8 portA_in;
	UINT8 portA_out;
	UINT8 spacecr_prot_value;
	UINT8 protection_value;
	UINT32 address;
	bitmap_t *layer_bitmap[3];
	bitmap_t *sprite_sprite_collbitmap1;
	bitmap_t *sprite_sprite_collbitmap2;
	bitmap_t *sprite_layer_collbitmap1;
	bitmap_t *sprite_layer_collbitmap2[3];
	int draw_order[32][4];
};


/*----------- defined in machine/taitosj.c -----------*/

MACHINE_START( taitosj );
MACHINE_RESET( taitosj );
WRITE8_HANDLER( taitosj_bankswitch_w );
READ8_HANDLER( taitosj_fake_data_r );
READ8_HANDLER( taitosj_fake_status_r );
WRITE8_HANDLER( taitosj_fake_data_w );
READ8_HANDLER( taitosj_mcu_data_r );
READ8_HANDLER( taitosj_mcu_status_r );
WRITE8_HANDLER( taitosj_mcu_data_w );
READ8_HANDLER( taitosj_68705_portA_r );
READ8_HANDLER( taitosj_68705_portB_r );
READ8_HANDLER( taitosj_68705_portC_r );
WRITE8_HANDLER( taitosj_68705_portA_w );
WRITE8_HANDLER( taitosj_68705_portB_w );

READ8_HANDLER( spacecr_prot_r );
WRITE8_HANDLER( alpine_protection_w );
WRITE8_HANDLER( alpinea_bankswitch_w );
READ8_HANDLER( alpine_port_2_r );


/*----------- defined in video/taitosj.c -----------*/

READ8_HANDLER( taitosj_gfxrom_r );
WRITE8_HANDLER( taitosj_characterram_w );
WRITE8_HANDLER( junglhbr_characterram_w );
WRITE8_HANDLER( taitosj_collision_reg_clear_w );
VIDEO_START( taitosj );
VIDEO_UPDATE( taitosj );
VIDEO_UPDATE( kikstart );
