class djmain_state : public driver_device
{
public:
	djmain_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int sndram_bank;
	UINT8 *sndram;
	int turntable_select;
	UINT8 turntable_last_pos[2];
	UINT16 turntable_pos[2];
	UINT8 pending_vb_int;
	UINT16 v_ctrl;
	UINT32 obj_regs[0xa0/4];
	const UINT8 *ide_user_password;
	const UINT8 *ide_master_password;
	UINT32 *obj_ram;
};


/*----------- defined in video/djmain.c -----------*/

SCREEN_UPDATE( djmain );
VIDEO_START( djmain );

void djmain_tile_callback(running_machine* machine, int layer, int *code, int *color, int *flags);
