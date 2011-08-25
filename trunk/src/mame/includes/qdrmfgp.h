class qdrmfgp_state : public driver_device
{
public:
	qdrmfgp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
	UINT8 *m_sndram;
	UINT16 *m_workram;
	UINT16 m_control;
	INT32 m_gp2_irq_control;
	INT32 m_pal;
};

/*----------- defined in video/qdrmfgp.c -----------*/

VIDEO_START( qdrmfgp );
VIDEO_START( qdrmfgp2 );
SCREEN_UPDATE( qdrmfgp );

void qdrmfgp_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
void qdrmfgp2_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
