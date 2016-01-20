// license:BSD-3-Clause
// copyright-holders:Mark McDougall
#include "sound/msm5205.h"

class stfight_state : public driver_device
{
public:
	enum
	{
		TIMER_STFIGHT_INTERRUPT_1
	};

	stfight_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_text_char_ram(*this, "text_char_ram"),
		m_text_attr_ram(*this, "text_attr_ram"),
		m_tx_vram(*this, "tx_vram"),
		m_vh_latch_ram(*this, "vh_latch_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_shared_ptr<UINT8> m_text_char_ram;
	optional_shared_ptr<UINT8> m_text_attr_ram;
	optional_shared_ptr<UINT8> m_tx_vram;
	required_shared_ptr<UINT8> m_vh_latch_ram;
	required_shared_ptr<UINT8> m_sprite_ram;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	UINT8 *m_decrypt;
	UINT8 m_fm_data;
	UINT8 m_cpu_to_mcu_data;
	UINT8 m_cpu_to_mcu_empty;

	UINT16 m_adpcm_data_offs;
	UINT8 m_adpcm_nibble;
	UINT8 m_adpcm_reset;

	UINT8 m_coin_state;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_sprite_base;

	DECLARE_WRITE_LINE_MEMBER(stfight_adpcm_int);

	DECLARE_DRIVER_INIT(stfight);
	DECLARE_DRIVER_INIT(empcity);
	DECLARE_DRIVER_INIT(cshooter);

	DECLARE_WRITE8_MEMBER(stfight_io_w);
	DECLARE_READ8_MEMBER(stfight_coin_r);
	DECLARE_WRITE8_MEMBER(stfight_coin_w);
	DECLARE_WRITE8_MEMBER(stfight_fm_w);
	DECLARE_WRITE8_MEMBER(stfight_mcu_w);

	DECLARE_WRITE8_MEMBER(stfight_bank_w);
	DECLARE_WRITE8_MEMBER(stfight_text_char_w);
	DECLARE_WRITE8_MEMBER(stfight_text_attr_w);
	DECLARE_WRITE8_MEMBER(stfight_sprite_bank_w);
	DECLARE_WRITE8_MEMBER(stfight_vh_latch_w);
	DECLARE_WRITE8_MEMBER(cshooter_text_w);

	DECLARE_READ8_MEMBER(stfight_fm_r);

	TILEMAP_MAPPER_MEMBER(fg_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(bg_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_cshooter_tx_tile_info);

	DECLARE_VIDEO_START(stfight);
	DECLARE_VIDEO_START(cshooter);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(stfight);
	UINT32 screen_update_stfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cshooter(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(stfight_vb_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cshooter_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	/*
	    MCU specifics
	*/

	DECLARE_READ8_MEMBER(stfight_68705_port_a_r);
	DECLARE_READ8_MEMBER(stfight_68705_port_b_r);
	DECLARE_READ8_MEMBER(stfight_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(stfight_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(stfight_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(stfight_68705_port_c_w);

	DECLARE_WRITE8_MEMBER(stfight_68705_ddr_a_w);
	DECLARE_WRITE8_MEMBER(stfight_68705_ddr_b_w);
	DECLARE_WRITE8_MEMBER(stfight_68705_ddr_c_w);

	UINT8 m_portA_out, m_portA_in;
	UINT8 m_portB_out, m_portB_in;
	UINT8 m_portC_out, m_portC_in;
	UINT8 m_ddrA, m_ddrB, m_ddrC;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
