/*************************************************************************

    Skyfox

*************************************************************************/

class skyfox_state : public driver_device
{
public:
	skyfox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	UINT8      m_vreg[8];
	int        m_bg_pos;
	int        m_bg_ctrl;

	/* misc */
	int        m_palette_selected;

	/* devices */
	cpu_device *m_maincpu;
	DECLARE_READ8_MEMBER(skyfox_vregs_r);
	DECLARE_WRITE8_MEMBER(skyfox_vregs_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(skyfox);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_skyfox(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(skyfox_interrupt);
};
