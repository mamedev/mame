class sshangha_state : public driver_device
{
public:
	sshangha_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *prot_data;
	UINT16 *sound_shared_ram;
	UINT16 *pf1_data;
	UINT16 *pf2_data;
	UINT16 *pf1_rowscroll;
	UINT16 *pf2_rowscroll;
	UINT16 control_0[8];
	tilemap_t *pf1_8x8_tilemap;
	tilemap_t *pf1_16x16_tilemap;
	tilemap_t *pf2_tilemap;
	int pf1_bank;
	int pf2_bank;
	int video_control;
	int last_pf1_bank;
	int last_pf2_bank;
};


/*----------- defined in video/sshangha.c -----------*/

VIDEO_START( sshangha );
VIDEO_UPDATE( sshangha );

WRITE16_HANDLER( sshangha_pf2_data_w );
WRITE16_HANDLER( sshangha_pf1_data_w );
WRITE16_HANDLER( sshangha_control_0_w );
WRITE16_HANDLER( sshangha_video_w );
