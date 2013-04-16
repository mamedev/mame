/*************************************************************************

    Operation Thunderbolt

*************************************************************************/

#include "machine/eeprom.h"
#include "sound/flt_vol.h"
#include "audio/taitosnd.h"
#include "machine/taitoio.h"

struct othunder_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};


class othunder_state : public driver_device
{
public:
	othunder_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this,"spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_eeprom(*this, "eeprom"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0110pcr(*this, "tc0110pcr"),
		m_tc0140syt(*this, "tc0140syt"),
		m_2610_0l(*this, "2610.0l"),
		m_2610_0r(*this, "2610.0r"),
		m_2610_1l(*this, "2610.1l"),
		m_2610_1r(*this, "2610.1r"),
		m_2610_2l(*this, "2610.2l"),
		m_2610_2r(*this, "2610.2r") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	struct othunder_tempsprite *m_spritelist;

	/* misc */
	int        m_vblank_irq;
	int        m_ad_irq;
	INT32      m_banknum;
	int        m_pan[4];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<eeprom_device> m_eeprom;
	required_device<tc0220ioc_device> m_tc0220ioc;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0110pcr_device> m_tc0110pcr;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device<filter_volume_device> m_2610_0l;
	required_device<filter_volume_device> m_2610_0r;
	required_device<filter_volume_device> m_2610_1l;
	required_device<filter_volume_device> m_2610_1r;
	required_device<filter_volume_device> m_2610_2l;
	required_device<filter_volume_device> m_2610_2r;
	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_WRITE16_MEMBER(othunder_tc0220ioc_w);
	DECLARE_READ16_MEMBER(othunder_tc0220ioc_r);
	DECLARE_READ16_MEMBER(othunder_lightgun_r);
	DECLARE_WRITE16_MEMBER(othunder_lightgun_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(othunder_sound_w);
	DECLARE_READ16_MEMBER(othunder_sound_r);
	DECLARE_WRITE8_MEMBER(othunder_TC0310FAM_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_othunder(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	TIMER_CALLBACK_MEMBER(ad_interrupt);
	void reset_sound_region();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const int *primasks, int y_offs );
	void update_irq(  );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
};
