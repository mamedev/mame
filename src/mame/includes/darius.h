// license:???
// copyright-holders:David Graves, Jarek Burczynski
/*************************************************************************

    Darius

*************************************************************************/

#include "audio/taitosnd.h"
#include "sound/flt_vol.h"
#include "sound/msm5205.h"
#include "video/pc080sn.h"

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
		m_adpcm(*this, "adpcm"),
		m_pc080sn (*this, "pc080sn"),
		m_tc0140syt(*this, "tc0140syt"),
		m_filter0_0l(*this, "filter0.0l"),
		m_filter0_0r(*this, "filter0.0r"),
		m_filter0_1l(*this, "filter0.1l"),
		m_filter0_1r(*this, "filter0.1r"),
		m_filter0_2l(*this, "filter0.2l"),
		m_filter0_2r(*this, "filter0.2r"),
		m_filter0_3l(*this, "filter0.3l"),
		m_filter0_3r(*this, "filter0.3r"),
		m_filter1_0l(*this, "filter1.0l"),
		m_filter1_0r(*this, "filter1.0r"),
		m_filter1_1l(*this, "filter1.1l"),
		m_filter1_1r(*this, "filter1.1r"),
		m_filter1_2l(*this, "filter1.2l"),
		m_filter1_2r(*this, "filter1.2r"),
		m_filter1_3l(*this, "filter1.3l"),
		m_filter1_3r(*this, "filter1.3r"),
		m_msm5205_l(*this, "msm5205.l"),
		m_msm5205_r(*this, "msm5205.r"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_fg_ram;

	/* video-related */
	tilemap_t  *m_fg_tilemap;

	/* misc */
	UINT16     m_cpua_ctrl;
	UINT16     m_coin_word;
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
	required_device<pc080sn_device> m_pc080sn;
	required_device<tc0140syt_device> m_tc0140syt;

	required_device<filter_volume_device> m_filter0_0l;
	required_device<filter_volume_device> m_filter0_0r;
	required_device<filter_volume_device> m_filter0_1l;
	required_device<filter_volume_device> m_filter0_1r;
	required_device<filter_volume_device> m_filter0_2l;
	required_device<filter_volume_device> m_filter0_2r;
	required_device<filter_volume_device> m_filter0_3l;
	required_device<filter_volume_device> m_filter0_3r;
	required_device<filter_volume_device> m_filter1_0l;
	required_device<filter_volume_device> m_filter1_0r;
	required_device<filter_volume_device> m_filter1_1l;
	required_device<filter_volume_device> m_filter1_1r;
	required_device<filter_volume_device> m_filter1_2l;
	required_device<filter_volume_device> m_filter1_2r;
	required_device<filter_volume_device> m_filter1_3l;
	required_device<filter_volume_device> m_filter1_3r;
	required_device<filter_volume_device> m_msm5205_l;
	required_device<filter_volume_device> m_msm5205_r;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

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
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_darius_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_darius_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_darius_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void darius_postload();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int x_offs, int y_offs );
	UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs);
	void parse_control(  )   /* assumes Z80 sandwiched between 68Ks */;
	void update_fm0(  );
	void update_fm1(  );
	void update_psg0( int port );
	void update_psg1( int port );
	void update_da(  );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_WRITE_LINE_MEMBER(darius_adpcm_int);
};
