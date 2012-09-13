class qdrmfgp_state : public driver_device
{
public:
	qdrmfgp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram")
		,
		m_workram(*this, "workram"){ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16>	m_nvram;
	UINT8 *m_sndram;
	required_shared_ptr<UINT16> m_workram;
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
	DECLARE_CUSTOM_INPUT_MEMBER(inputs_r);
	DECLARE_CUSTOM_INPUT_MEMBER(battery_sensor_r);
	DECLARE_READ16_MEMBER(ide_std_r);
	DECLARE_WRITE16_MEMBER(ide_std_w);
	DECLARE_READ16_MEMBER(ide_alt_r);
	DECLARE_WRITE16_MEMBER(ide_alt_w);
	DECLARE_WRITE_LINE_MEMBER(qdrmfgp_irq3_ack_w);
	DECLARE_WRITE_LINE_MEMBER(qdrmfgp_irq4_ack_w);
	virtual void machine_reset();
	DECLARE_MACHINE_START(qdrmfgp);
	DECLARE_VIDEO_START(qdrmfgp);
	DECLARE_MACHINE_START(qdrmfgp2);
	DECLARE_VIDEO_START(qdrmfgp2);
};

/*----------- defined in video/qdrmfgp.c -----------*/



SCREEN_UPDATE_IND16( qdrmfgp );

void qdrmfgp_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
void qdrmfgp2_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);
