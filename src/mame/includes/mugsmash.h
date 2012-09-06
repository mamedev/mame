
class mugsmash_state : public driver_device
{
public:
	mugsmash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_regs1(*this, "regs1"),
		m_regs2(*this, "regs2"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT16> m_videoram1;
	required_shared_ptr<UINT16> m_videoram2;
	required_shared_ptr<UINT16> m_regs1;
	required_shared_ptr<UINT16> m_regs2;
	required_shared_ptr<UINT16> m_spriteram;

	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;

	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(mugsmash_reg2_w);
	DECLARE_READ16_MEMBER(mugsmash_input_ports_r);
	DECLARE_WRITE16_MEMBER(mugsmash_videoram1_w);
	DECLARE_WRITE16_MEMBER(mugsmash_videoram2_w);
	DECLARE_WRITE16_MEMBER(mugsmash_reg_w);
	TILE_GET_INFO_MEMBER(get_mugsmash_tile_info1);
	TILE_GET_INFO_MEMBER(get_mugsmash_tile_info2);
};


/*----------- defined in video/mugsmash.c -----------*/

VIDEO_START( mugsmash );
SCREEN_UPDATE_IND16( mugsmash );

