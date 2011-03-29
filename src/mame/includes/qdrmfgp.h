class qdrmfgp_state : public driver_device
{
public:
	qdrmfgp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
	UINT8 *sndram;
	UINT16 *workram;
	UINT16 control;
	INT32 gp2_irq_control;
	INT32 pal;
};

/*----------- defined in video/qdrmfgp.c -----------*/

VIDEO_START( qdrmfgp );
VIDEO_START( qdrmfgp2 );
SCREEN_UPDATE( qdrmfgp );

void qdrmfgp_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
void qdrmfgp2_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
