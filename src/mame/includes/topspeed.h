/*************************************************************************

    Top Speed / Full Throttle

*************************************************************************/

class topspeed_state : public driver_device
{
public:
	topspeed_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spritemap(*this, "spritemap"),
		m_raster_ctrl(*this, "raster_ctrl"),
		m_spriteram(*this, "spriteram"),
		m_sharedram(*this, "sharedram")
	{ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spritemap;
	required_shared_ptr<UINT16> m_raster_ctrl;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_sharedram;

	/* adpcm */
	device_t *m_msm_chip[2];
	UINT8 *m_msm_rom[2];
	UINT16 m_msm_start[2];
	UINT16 m_msm_loop[2];
	UINT16 m_msm_pos[2];
	UINT8 m_msm_sel[2];

	/* misc */
	UINT16     m_cpua_ctrl;
	INT32      m_ioc220_port;
	INT32      m_banknum;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	cpu_device *m_subcpu;
	device_t *m_pc080sn_1;
	device_t *m_pc080sn_2;
	device_t *m_tc0220ioc;

	UINT8 m_dislayer[5];
	DECLARE_READ16_MEMBER(sharedram_r);
	DECLARE_WRITE16_MEMBER(sharedram_w);
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_READ8_MEMBER(topspeed_input_bypass_r);
	DECLARE_READ16_MEMBER(topspeed_motor_r);
	DECLARE_WRITE16_MEMBER(topspeed_motor_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(topspeed_msm5205_command_w);
	DECLARE_CUSTOM_INPUT_MEMBER(topspeed_pedal_r);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_topspeed(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/topspeed.c -----------*/


