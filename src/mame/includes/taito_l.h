// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "sound/msm5205.h"
#include "sound/2203intf.h"

#define TAITOL_SPRITERAM_SIZE 0x400

class taitol_state : public driver_device
{
public:
	taitol_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	/* memory pointers */
	uint8_t *       m_shared_ram;

	/* video-related */
	tilemap_t *m_bg18_tilemap;
	tilemap_t *m_bg19_tilemap;
	tilemap_t *m_ch1a_tilemap;
	uint8_t m_buff_spriteram[TAITOL_SPRITERAM_SIZE];
	int m_cur_ctrl;
	int m_horshoes_gfxbank;
	int m_bankc[4];
	int m_flipscreen;

	/* misc */
	void (taitol_state::*m_current_notifier[4])(int);
	uint8_t *m_current_base[4];

	int m_cur_rombank;
	int m_cur_rombank2;
	int m_cur_rambank[4];
	int m_irq_adr_table[3];
	int m_irq_enable;
	int m_adpcm_pos;
	int m_adpcm_data;
	int m_trackx;
	int m_tracky;
	int m_mux_ctrl;
	int m_extport;
	int m_last_irq_level;
	int m_high;
	int m_high2;
	int m_last_data_adr;
	int m_last_data;
	int m_cur_bank;

	const uint8_t *m_mcu_reply;
	int m_mcu_pos;
	int m_mcu_reply_len;

	const char *m_porte0_tag;
	const char *m_porte1_tag;
	const char *m_portf0_tag;
	const char *m_portf1_tag;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory buffers */
	uint8_t         m_rambanks[0x1000 * 12];
	uint8_t         m_palette_ram[0x1000];
	uint8_t         m_empty_ram[0x1000];
	void irq_adr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t irq_adr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t irq_enable_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rombankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rombank2switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rombankswitch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t rombank2switch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rambankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rambankswitch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bank0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mux_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mux_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mux_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void champwr_msm5205_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void champwr_msm5205_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t horshoes_tracky_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t horshoes_trackx_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t horshoes_tracky_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t horshoes_tracky_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t horshoes_trackx_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t horshoes_trackx_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void horshoes_bankg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void taitol_bankc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t taitol_bankc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void taitol_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t taitol_control_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t portB_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t extport_select_and_ym2203_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void champwr_msm5205_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void champwr_msm5205_stop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void champwr_msm5205_volume_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_plottinga();
	void get_bg18_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg19_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_ch1a_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_taito_l();
	void machine_reset_fhawk();
	void video_start_taitol();
	void machine_reset_kurikint();
	void machine_reset_plotting();
	void machine_reset_evilston();
	void machine_reset_champwr();
	void machine_reset_raimais();
	void machine_reset_puzznic();
	void machine_reset_horshoes();
	void machine_reset_palamed();
	void machine_reset_cachat();
	uint32_t screen_update_taitol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_taitol(screen_device &screen, bool state);
	void vbl_interrupt(timer_device &timer, void *ptr, int32_t param);
	int irq_callback(device_t &device, int irqline);
	void taitol_chardef14_m( int offset );
	void taitol_chardef15_m( int offset );
	void taitol_chardef16_m( int offset );
	void taitol_chardef17_m( int offset );
	void taitol_chardef1c_m( int offset );
	void taitol_chardef1d_m( int offset );
	void taitol_chardef1e_m( int offset );
	void taitol_chardef1f_m( int offset );
	void taitol_bg18_m( int offset );
	void taitol_bg19_m( int offset );
	void taitol_char1a_m( int offset );
	void taitol_obj1b_m( int offset );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void palette_notifier(int addr);
	void state_register(  );
	void taito_machine_reset();
	void bank_w(address_space &space, offs_t offset, uint8_t data, int banknum );
	void champwr_msm5205_vck(int state);
};
