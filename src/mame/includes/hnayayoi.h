// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Hana Yayoi & other Dynax games (using 1st version of their blitter)

*************************************************************************/
#include "sound/msm5205.h"

class hnayayoi_state : public driver_device
{
public:
	hnayayoi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm") { }

	/* video-related */
	uint8_t      *m_pixmap[8];
	int        m_palbank;
	int        m_total_pixmaps;
	uint8_t      m_blit_layer;
	uint16_t     m_blit_dest;
	uint32_t     m_blit_src;

	/* misc */
	int        m_keyb;
	uint8_t keyboard_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t keyboard_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void keyboard_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blitter_rev1_param_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blitter_rev1_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dynax_blitter_rev1_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hnayayoi_palbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_vclk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_reset_inv_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_hnfubuki();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void video_start_untoucha();
	uint32_t screen_update_hnayayoi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void common_vh_start( int num_pixmaps );
	void copy_pixel( int x, int y, int pen );
	void draw_layer_interleaved( bitmap_ind16 &bitmap, const rectangle &cliprect, int left_pixmap, int right_pixmap, int palbase, int transp );
	void irqhandler(int state);
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
};
