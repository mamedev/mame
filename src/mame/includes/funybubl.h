

class funybubl_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, funybubl_state(machine)); }

	funybubl_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    banked_vram;
	UINT8 *    paletteram;

	/* devices */
	running_device *audiocpu;
};



/*----------- defined in video/funybubl.c -----------*/

WRITE8_HANDLER ( funybubl_paldatawrite );

VIDEO_START(funybubl);
VIDEO_UPDATE(funybubl);
