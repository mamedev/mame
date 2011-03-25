class tetrisp2_state : public driver_device
{
public:
	tetrisp2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 systemregs[0x10];
	UINT16 *vram_bg;
	UINT16 *scroll_bg;
	UINT16 *vram_fg;
	UINT16 *scroll_fg;
	UINT16 *vram_rot;
	UINT16 *rotregs;
	UINT8 *priority;
	UINT16 *rocknms_sub_vram_bg;
	UINT16 *rocknms_sub_scroll_bg;
	UINT16 *rocknms_sub_vram_fg;
	UINT16 *rocknms_sub_scroll_fg;
	UINT16 *rocknms_sub_vram_rot;
	UINT16 *rocknms_sub_rotregs;
	UINT16 *rocknms_sub_priority;
	UINT16 rocknms_sub_systemregs[0x10];
	UINT16 rockn_protectdata;
	UINT16 rockn_adpcmbank;
	UINT16 rockn_soundvolume;
	emu_timer *rockn_timer_l4;
	emu_timer *rockn_timer_sub_l4;
	int bank_lo;
	int bank_hi;
	UINT16 *nvram;
	UINT16 rocknms_main2sub;
	UINT16 rocknms_sub2main;
	int flipscreen_old;
	tilemap_t *tilemap_bg;
	tilemap_t *tilemap_fg;
	tilemap_t *tilemap_rot;
	tilemap_t *tilemap_sub_bg;
	tilemap_t *tilemap_sub_fg;
	tilemap_t *tilemap_sub_rot;
};


/*----------- defined in video/tetrisp2.c -----------*/

WRITE16_HANDLER( tetrisp2_palette_w );
WRITE16_HANDLER( rocknms_sub_palette_w );
WRITE8_HANDLER( tetrisp2_priority_w );
WRITE8_HANDLER( rockn_priority_w );
READ8_HANDLER( tetrisp2_priority_r );
WRITE16_HANDLER( rocknms_sub_priority_w );
READ16_HANDLER( nndmseal_priority_r );

WRITE16_HANDLER( tetrisp2_vram_bg_w );
WRITE16_HANDLER( tetrisp2_vram_fg_w );
WRITE16_HANDLER( tetrisp2_vram_rot_w );

WRITE16_HANDLER( rocknms_sub_vram_bg_w );
WRITE16_HANDLER( rocknms_sub_vram_fg_w );
WRITE16_HANDLER( rocknms_sub_vram_rot_w );

VIDEO_START( tetrisp2 );
SCREEN_UPDATE( tetrisp2 );

VIDEO_START( rockntread );
SCREEN_UPDATE( rockntread );

VIDEO_START( rocknms );
SCREEN_UPDATE( rocknms );

VIDEO_START( nndmseal );
