/*************************************************************************

    Taito Triple Screen Games

*************************************************************************/

#include <sound/flt_vol.h>
#include <audio/taitosnd.h>

class ninjaw_state : public driver_device
{
public:
	ninjaw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;

	/* misc */
	UINT16     m_cpua_ctrl;
	INT32      m_banknum;
	int        m_pandata[4];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	tc0140syt_device *m_tc0140syt;
	device_t *m_tc0100scn_1;
	device_t *m_tc0100scn_2;
	device_t *m_tc0100scn_3;
	device_t *m_lscreen;
	device_t *m_mscreen;
	device_t *m_rscreen;
	filter_volume_device *m_2610_1l;
	filter_volume_device *m_2610_1r;
	filter_volume_device *m_2610_2l;
	filter_volume_device *m_2610_2r;
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(ninjaw_sound_w);
	DECLARE_READ16_MEMBER(ninjaw_sound_r);
	DECLARE_WRITE8_MEMBER(ninjaw_pancontrol);
	DECLARE_WRITE16_MEMBER(tc0100scn_triple_screen_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_ninjaw_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ninjaw_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ninjaw_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ninjaw_postload();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int x_offs, int y_offs );
	void parse_control(  )   /* assumes Z80 sandwiched between 68Ks */;
	void reset_sound_region(  );
	UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, device_t *tc0100scn);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
};
