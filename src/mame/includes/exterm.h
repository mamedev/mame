/*************************************************************************

    Gottlieb Exterminator hardware

*************************************************************************/

class exterm_state : public driver_device
{
public:
	exterm_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 aimpos[2];
	UINT8 trackball_old[2];
	UINT8 master_sound_latch;
	UINT8 slave_sound_latch;
	UINT8 sound_control;
	UINT8 dac_value[2];
	UINT16 last;
	UINT16 *master_videoram;
	UINT16 *slave_videoram;
};


/*----------- defined in video/exterm.c -----------*/

PALETTE_INIT( exterm );
void exterm_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

void exterm_to_shiftreg_master(address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_from_shiftreg_master(address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_to_shiftreg_slave(address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_from_shiftreg_slave(address_space *space, UINT32 address, UINT16* shiftreg);
