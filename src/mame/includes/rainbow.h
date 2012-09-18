/*************************************************************************

    Rainbow Islands

*************************************************************************/

class rbisland_state : public driver_device
{
public:
	rbisland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	UINT16      m_sprite_ctrl;
	UINT16      m_sprites_flipscreen;

	/* misc */
	UINT8       m_jumping_latch;

	/* c-chip */
	UINT8       *m_CRAM[8];
	int         m_extra_version;
	UINT8       m_current_bank;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_pc080sn;
	device_t *m_pc090oj;
	DECLARE_WRITE16_MEMBER(jumping_sound_w);
	DECLARE_READ8_MEMBER(jumping_latch_r);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_ctrl_w);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_bank_w);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_ram_w);
	DECLARE_READ16_MEMBER(rbisland_cchip_ctrl_r);
	DECLARE_READ16_MEMBER(rbisland_cchip_ram_r);
	DECLARE_WRITE16_MEMBER(rbisland_spritectrl_w);
	DECLARE_WRITE16_MEMBER(jumping_spritectrl_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_DRIVER_INIT(jumping);
	DECLARE_DRIVER_INIT(rbislande);
	DECLARE_DRIVER_INIT(rbisland);
	virtual void machine_start();
	DECLARE_VIDEO_START(jumping);
	UINT32 screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_jumping(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in machine/rainbow.c -----------*/
void rbisland_cchip_init(running_machine &machine, int version);
