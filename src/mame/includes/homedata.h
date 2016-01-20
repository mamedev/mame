// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/2203intf.h"

class homedata_state : public driver_device
{
public:
	homedata_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_vreg(*this, "vreg"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dac(*this, "dac"),
		m_ymsnd(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	/* memory pointers */
	optional_shared_ptr<UINT8> m_vreg;
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	tilemap_t *m_bg_tilemap[2][4];
	int      m_visible_page;
	int      m_priority;
	UINT8    m_reikaids_which;
	int      m_flipscreen;
	UINT8      m_gfx_bank[2];   // pteacher only uses the first one
	UINT8      m_blitter_bank;
	int      m_blitter_param_count;
	UINT8      m_blitter_param[4];      /* buffers last 4 writes to 0x8006 */


	/* misc */
	int      m_vblank;
	int      m_sndbank;
	int      m_keyb;
	int      m_snd_command;
	int      m_upd7807_porta;
	int      m_upd7807_portc;
	int      m_to_cpu;
	int      m_from_cpu;

	/* device */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<dac_device> m_dac;
	optional_device<ym2203_device> m_ymsnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	sn76489a_device *m_sn;
	UINT8 m_prot_data;
	DECLARE_READ8_MEMBER(mrokumei_keyboard_r);
	DECLARE_WRITE8_MEMBER(mrokumei_keyboard_select_w);
	DECLARE_READ8_MEMBER(mrokumei_sound_io_r);
	DECLARE_WRITE8_MEMBER(mrokumei_sound_bank_w);
	DECLARE_WRITE8_MEMBER(mrokumei_sound_io_w);
	DECLARE_WRITE8_MEMBER(mrokumei_sound_cmd_w);
	DECLARE_READ8_MEMBER(reikaids_upd7807_porta_r);
	DECLARE_WRITE8_MEMBER(reikaids_upd7807_porta_w);
	DECLARE_WRITE8_MEMBER(reikaids_upd7807_portc_w);
	DECLARE_READ8_MEMBER(reikaids_io_r);
	DECLARE_READ8_MEMBER(reikaids_snd_command_r);
	DECLARE_WRITE8_MEMBER(reikaids_snd_command_w);
	DECLARE_WRITE8_MEMBER(pteacher_snd_command_w);
	DECLARE_READ8_MEMBER(pteacher_snd_r);
	DECLARE_READ8_MEMBER(pteacher_io_r);
	DECLARE_READ8_MEMBER(pteacher_keyboard_r);
	DECLARE_READ8_MEMBER(pteacher_upd7807_porta_r);
	DECLARE_WRITE8_MEMBER(pteacher_snd_answer_w);
	DECLARE_WRITE8_MEMBER(pteacher_upd7807_porta_w);
	DECLARE_WRITE8_MEMBER(pteacher_upd7807_portc_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(mirderby_prot_r);
	DECLARE_WRITE8_MEMBER(mirderby_prot_w);
	DECLARE_WRITE8_MEMBER(mrokumei_videoram_w);
	DECLARE_WRITE8_MEMBER(reikaids_videoram_w);
	DECLARE_WRITE8_MEMBER(reikaids_gfx_bank_w);
	DECLARE_WRITE8_MEMBER(pteacher_gfx_bank_w);
	DECLARE_WRITE8_MEMBER(homedata_blitter_param_w);
	DECLARE_WRITE8_MEMBER(mrokumei_blitter_bank_w);
	DECLARE_WRITE8_MEMBER(reikaids_blitter_bank_w);
	DECLARE_WRITE8_MEMBER(pteacher_blitter_bank_w);
	DECLARE_WRITE8_MEMBER(mrokumei_blitter_start_w);
	DECLARE_WRITE8_MEMBER(reikaids_blitter_start_w);
	DECLARE_WRITE8_MEMBER(pteacher_blitter_start_w);
	DECLARE_DRIVER_INIT(reikaids);
	DECLARE_DRIVER_INIT(mjikaga);
	DECLARE_DRIVER_INIT(jogakuen);
	DECLARE_DRIVER_INIT(battlcry);
	DECLARE_DRIVER_INIT(mirderby);
	TILE_GET_INFO_MEMBER(mrokumei_get_info0_0);
	TILE_GET_INFO_MEMBER(mrokumei_get_info1_0);
	TILE_GET_INFO_MEMBER(mrokumei_get_info0_1);
	TILE_GET_INFO_MEMBER(mrokumei_get_info1_1);
	TILE_GET_INFO_MEMBER(reikaids_get_info0_0);
	TILE_GET_INFO_MEMBER(reikaids_get_info1_0);
	TILE_GET_INFO_MEMBER(reikaids_get_info0_1);
	TILE_GET_INFO_MEMBER(reikaids_get_info1_1);
	TILE_GET_INFO_MEMBER(reikaids_get_info0_2);
	TILE_GET_INFO_MEMBER(reikaids_get_info1_2);
	TILE_GET_INFO_MEMBER(reikaids_get_info0_3);
	TILE_GET_INFO_MEMBER(reikaids_get_info1_3);
	TILE_GET_INFO_MEMBER(pteacher_get_info0_0);
	TILE_GET_INFO_MEMBER(pteacher_get_info1_0);
	TILE_GET_INFO_MEMBER(pteacher_get_info0_1);
	TILE_GET_INFO_MEMBER(pteacher_get_info1_1);
	TILE_GET_INFO_MEMBER(lemnangl_get_info0_0);
	TILE_GET_INFO_MEMBER(lemnangl_get_info1_0);
	TILE_GET_INFO_MEMBER(lemnangl_get_info0_1);
	TILE_GET_INFO_MEMBER(lemnangl_get_info1_1);
	TILE_GET_INFO_MEMBER(mirderby_get_info0_0);
	TILE_GET_INFO_MEMBER(mirderby_get_info1_0);
	TILE_GET_INFO_MEMBER(mirderby_get_info0_1);
	TILE_GET_INFO_MEMBER(mirderby_get_info1_1);
	DECLARE_MACHINE_START(homedata);
	DECLARE_MACHINE_RESET(homedata);
	DECLARE_VIDEO_START(mrokumei);
	DECLARE_PALETTE_INIT(mrokumei);
	DECLARE_MACHINE_START(reikaids);
	DECLARE_MACHINE_RESET(reikaids);
	DECLARE_VIDEO_START(reikaids);
	DECLARE_PALETTE_INIT(reikaids);
	DECLARE_MACHINE_START(pteacher);
	DECLARE_MACHINE_RESET(pteacher);
	DECLARE_VIDEO_START(pteacher);
	DECLARE_PALETTE_INIT(pteacher);
	DECLARE_VIDEO_START(mirderby);
	DECLARE_PALETTE_INIT(mirderby);
	DECLARE_VIDEO_START(lemnangl);
	UINT32 screen_update_mrokumei(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_reikaids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pteacher(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mirderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_homedata(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(homedata_irq);
	INTERRUPT_GEN_MEMBER(upd7807_irq);
	void mrokumei_handleblit( address_space &space, int rom_base );
	void reikaids_handleblit( address_space &space, int rom_base );
	void pteacher_handleblit( address_space &space, int rom_base );
	inline void mrokumei_info0( tile_data &tileinfo, int tile_index, int page, int gfxbank );
	inline void mrokumei_info1( tile_data &tileinfo, int tile_index, int page, int gfxbank );
	inline void reikaids_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxbank );
	inline void pteacher_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxbank );
	inline void lemnangl_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxset, int gfxbank );
	inline void mirderby_info0( tile_data &tileinfo, int tile_index, int page, int gfxbank );
	inline void mirderby_info1( tile_data &tileinfo, int tile_index, int page, int gfxbank );
};
