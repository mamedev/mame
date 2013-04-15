/*************************************************************************

    Darius

*************************************************************************/

#include "sound/flt_vol.h"
#include "audio/taitosnd.h"
#include "sound/msm5205.h"

#define DARIUS_VOL_MAX    (3*2 + 2)
#define DARIUS_PAN_MAX    (2 + 2 + 1)   /* FM 2port + PSG 2port + DA 1port */

class darius_state : public driver_device
{
public:
	darius_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_fg_ram(*this, "fg_ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_cpub(*this, "cpub"),
		m_adpcm(*this, "adpcm") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_fg_ram;

	/* video-related */
	tilemap_t  *m_fg_tilemap;

	/* misc */
	UINT16     m_cpua_ctrl;
	UINT16     m_coin_word;
	INT32      m_banknum;
	UINT8      m_adpcm_command;
	UINT8      m_nmi_enable;
	UINT32     m_def_vol[0x10];
	UINT8      m_vol[DARIUS_VOL_MAX];
	UINT8      m_pan[DARIUS_PAN_MAX];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<cpu_device> m_cpub;
	required_device<cpu_device> m_adpcm;
	tc0140syt_device *m_tc0140syt;
	device_t *m_pc080sn;

	device_t *m_lscreen;
	device_t *m_mscreen;
	device_t *m_rscreen;

	filter_volume_device *m_filter0_0l;
	filter_volume_device *m_filter0_0r;
	filter_volume_device *m_filter0_1l;
	filter_volume_device *m_filter0_1r;
	filter_volume_device *m_filter0_2l;
	filter_volume_device *m_filter0_2r;
	filter_volume_device *m_filter0_3l;
	filter_volume_device *m_filter0_3r;
	filter_volume_device *m_filter1_0l;
	filter_volume_device *m_filter1_0r;
	filter_volume_device *m_filter1_1l;
	filter_volume_device *m_filter1_1r;
	filter_volume_device *m_filter1_2l;
	filter_volume_device *m_filter1_2r;
	filter_volume_device *m_filter1_3l;
	filter_volume_device *m_filter1_3r;
	filter_volume_device *m_msm5205_l;
	filter_volume_device *m_msm5205_r;
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_WRITE16_MEMBER(darius_watchdog_w);
	DECLARE_READ16_MEMBER(darius_ioc_r);
	DECLARE_WRITE16_MEMBER(darius_ioc_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(adpcm_command_w);
	DECLARE_WRITE8_MEMBER(display_value);
	DECLARE_WRITE8_MEMBER(darius_fm0_pan);
	DECLARE_WRITE8_MEMBER(darius_fm1_pan);
	DECLARE_WRITE8_MEMBER(darius_psg0_pan);
	DECLARE_WRITE8_MEMBER(darius_psg1_pan);
	DECLARE_WRITE8_MEMBER(darius_da_pan);
	DECLARE_READ8_MEMBER(adpcm_command_read);
	DECLARE_READ8_MEMBER(readport2);
	DECLARE_READ8_MEMBER(readport3);
	DECLARE_WRITE8_MEMBER(adpcm_nmi_disable);
	DECLARE_WRITE8_MEMBER(adpcm_nmi_enable);
	DECLARE_WRITE16_MEMBER(darius_fg_layer_w);
	DECLARE_WRITE8_MEMBER(darius_write_portA0);
	DECLARE_WRITE8_MEMBER(darius_write_portA1);
	DECLARE_WRITE8_MEMBER(darius_write_portB0);
	DECLARE_WRITE8_MEMBER(darius_write_portB1);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_DRIVER_INIT(darius);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_darius_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_darius_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_darius_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void darius_postload();
	inline void actual_get_fg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int x_offs, int y_offs );
	UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs);
	void parse_control(  )   /* assumes Z80 sandwiched between 68Ks */;
	void reset_sound_region(  );
	void update_fm0(  );
	void update_fm1(  );
	void update_psg0( int port );
	void update_psg1( int port );
	void update_da(  );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_WRITE_LINE_MEMBER(darius_adpcm_int);
};
