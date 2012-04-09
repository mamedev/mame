class qdrmfgp_state : public driver_device
{
public:
	qdrmfgp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16>	m_nvram;
	UINT8 *m_sndram;
	UINT16 *m_workram;
	UINT16 m_control;
	INT32 m_gp2_irq_control;
	INT32 m_pal;
	DECLARE_WRITE16_MEMBER(gp_control_w);
	DECLARE_WRITE16_MEMBER(gp2_control_w);
	DECLARE_READ16_MEMBER(v_rom_r);
	DECLARE_READ16_MEMBER(gp2_vram_r);
	DECLARE_READ16_MEMBER(gp2_vram_mirror_r);
	DECLARE_WRITE16_MEMBER(gp2_vram_w);
	DECLARE_WRITE16_MEMBER(gp2_vram_mirror_w);
	DECLARE_READ16_MEMBER(sndram_r);
	DECLARE_WRITE16_MEMBER(sndram_w);
	DECLARE_READ16_MEMBER(gp2_ide_std_r);
};

/*----------- defined in video/qdrmfgp.c -----------*/

VIDEO_START( qdrmfgp );
VIDEO_START( qdrmfgp2 );
SCREEN_UPDATE_IND16( qdrmfgp );

void qdrmfgp_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
void qdrmfgp2_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
