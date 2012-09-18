/*************************************************************************

    Mouser

*************************************************************************/

class mouser_state : public driver_device
{
public:
	mouser_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	/* misc */
	UINT8      m_sound_byte;
	UINT8      m_nmi_enable;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	DECLARE_WRITE8_MEMBER(mouser_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(mouser_sound_interrupt_w);
	DECLARE_READ8_MEMBER(mouser_sound_byte_r);
	DECLARE_WRITE8_MEMBER(mouser_sound_nmi_clear_w);
	DECLARE_WRITE8_MEMBER(mouser_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(mouser_flip_screen_y_w);
	DECLARE_DRIVER_INIT(mouser);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_mouser(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(mouser_nmi_interrupt);
	INTERRUPT_GEN_MEMBER(mouser_sound_nmi_assert);
};
