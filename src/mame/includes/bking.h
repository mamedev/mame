// license:???
// copyright-holders:Ed Mueller, Mike Balfour, Zsolt Vasvari
#include "machine/buggychl.h"

class bking_state : public driver_device
{
public:
	bking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_bmcu(*this, "bmcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_playfield_ram;

	/* video-related */
	bitmap_ind16    m_colmap_bg;
	bitmap_ind16    m_colmap_ball;
	tilemap_t     *m_bg_tilemap;
	int         m_pc3259_output[4];
	int         m_pc3259_mask;
	UINT8       m_xld1;
	UINT8       m_xld2;
	UINT8       m_xld3;
	UINT8       m_yld1;
	UINT8       m_yld2;
	UINT8       m_yld3;
	int         m_ball1_pic;
	int         m_ball2_pic;
	int         m_crow_pic;
	int         m_crow_flip;
	int         m_palette_bank;
	int         m_controller;
	int         m_hit;

	/* sound-related */
	int         m_sound_nmi_enable;
	int         m_pending_nmi;

	/* misc */
	int         m_addr_h;
	int         m_addr_l;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	optional_device<buggychl_mcu_device> m_bmcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

#if 0
	/* 68705 */
	UINT8 m_port_a_in;
	UINT8 m_port_a_out;
	UINT8 m_ddr_a;
	UINT8 m_port_b_in;
	UINT8 m_port_b_out;
	UINT8 m_ddr_b;
#endif
	DECLARE_READ8_MEMBER(bking_sndnmi_disable_r);
	DECLARE_WRITE8_MEMBER(bking_sndnmi_enable_w);
	DECLARE_WRITE8_MEMBER(bking_soundlatch_w);
	DECLARE_WRITE8_MEMBER(bking3_addr_l_w);
	DECLARE_WRITE8_MEMBER(bking3_addr_h_w);
	DECLARE_READ8_MEMBER(bking3_extrarom_r);
	DECLARE_READ8_MEMBER(bking3_ext_check_r);
	DECLARE_READ8_MEMBER(bking3_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(bking3_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(bking3_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(bking3_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(bking3_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(bking3_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(bking3_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(bking_xld1_w);
	DECLARE_WRITE8_MEMBER(bking_yld1_w);
	DECLARE_WRITE8_MEMBER(bking_xld2_w);
	DECLARE_WRITE8_MEMBER(bking_yld2_w);
	DECLARE_WRITE8_MEMBER(bking_xld3_w);
	DECLARE_WRITE8_MEMBER(bking_yld3_w);
	DECLARE_WRITE8_MEMBER(bking_cont1_w);
	DECLARE_WRITE8_MEMBER(bking_cont2_w);
	DECLARE_WRITE8_MEMBER(bking_cont3_w);
	DECLARE_WRITE8_MEMBER(bking_msk_w);
	DECLARE_WRITE8_MEMBER(bking_hitclr_w);
	DECLARE_WRITE8_MEMBER(bking_playfield_w);
	DECLARE_READ8_MEMBER(bking_input_port_5_r);
	DECLARE_READ8_MEMBER(bking_input_port_6_r);
	DECLARE_READ8_MEMBER(bking_pos_r);
	DECLARE_WRITE8_MEMBER(unk_w);
	DECLARE_WRITE8_MEMBER(port_b_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(bking);
	DECLARE_MACHINE_START(bking3);
	DECLARE_MACHINE_RESET(bking3);
	DECLARE_MACHINE_RESET(common);
	UINT32 screen_update_bking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_bking(screen_device &screen, bool state);
};
