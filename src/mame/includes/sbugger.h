class sbugger_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, sbugger_state(machine)); }

	sbugger_state(running_machine &machine)
		: driver_data_t(machine) { }

	UINT8 *videoram;
	UINT8 *videoram_attr;

	tilemap_t *tilemap;
};


/*----------- defined in video/sbugger.c -----------*/

PALETTE_INIT(sbugger);
VIDEO_UPDATE(sbugger);
VIDEO_START(sbugger);
WRITE8_HANDLER( sbugger_videoram_attr_w );
WRITE8_HANDLER( sbugger_videoram_w );
