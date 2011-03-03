class naughtyb_state : public driver_device
{
public:
	naughtyb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 popflame_prot_seed;
	int r_index;
	int prot_count;
	int question_offset;
	UINT8 *videoram2;
	UINT8 *scrollreg;
	int cocktail;
	UINT8 palreg;
	int bankreg;
};


/*----------- defined in video/naughtyb.c -----------*/

WRITE8_HANDLER( naughtyb_videoreg_w );
WRITE8_HANDLER( popflame_videoreg_w );

VIDEO_START( naughtyb );
PALETTE_INIT( naughtyb );
SCREEN_UPDATE( naughtyb );
