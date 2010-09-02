class shangha3_state : public driver_device
{
public:
	shangha3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int prot_count;
	UINT16 *ram;
	size_t ram_size;

	int do_shadows;

	UINT16 gfxlist_addr;
	bitmap_t *rawbitmap;

	UINT8 drawmode_table[16];
};


/*----------- defined in video/shangha3.c -----------*/

WRITE16_HANDLER( shangha3_flipscreen_w );
WRITE16_HANDLER( shangha3_gfxlist_addr_w );
WRITE16_HANDLER( shangha3_blitter_go_w );
VIDEO_START( shangha3 );
VIDEO_UPDATE( shangha3 );
