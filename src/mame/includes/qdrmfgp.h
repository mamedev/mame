class qdrmfgp_state : public driver_device
{
public:
	qdrmfgp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
};

/*----------- defined in drivers/qdrmfgp.c -----------*/

int qdrmfgp_get_palette(void);


/*----------- defined in video/qdrmfgp.c -----------*/

VIDEO_START( qdrmfgp );
VIDEO_START( qdrmfgp2 );
VIDEO_UPDATE( qdrmfgp );

void qdrmfgp_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);
void qdrmfgp2_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);
