/*************************************************************************

    Gottlieb Exterminator hardware

*************************************************************************/

class exterm_state : public driver_device
{
public:
	exterm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_aimpos[2];
	UINT8 m_trackball_old[2];
	UINT8 m_master_sound_latch;
	UINT8 m_slave_sound_latch;
	UINT8 m_sound_control;
	UINT8 m_dac_value[2];
	UINT16 m_last;
	UINT16 *m_master_videoram;
	UINT16 *m_slave_videoram;
};


/*----------- defined in video/exterm.c -----------*/

PALETTE_INIT( exterm );
void exterm_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

void exterm_to_shiftreg_master(address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_from_shiftreg_master(address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_to_shiftreg_slave(address_space *space, UINT32 address, UINT16* shiftreg);
void exterm_from_shiftreg_slave(address_space *space, UINT32 address, UINT16* shiftreg);
