// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
#include "sound/msm5205.h"

class lucky74_state : public driver_device
{
public:
	lucky74_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_colorram(*this, "bg_colorram"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode") { }

	UINT8 m_ym2149_portb;
	UINT8 m_usart_8251;
	UINT8 m_copro_sm7831;
	int m_adpcm_pos;
	int m_adpcm_end;
	int m_adpcm_data;
	UINT8 m_adpcm_reg[6];
	UINT8 m_adpcm_busy_line;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_fg_colorram;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_bg_colorram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(custom_09R81P_port_r);
	DECLARE_WRITE8_MEMBER(custom_09R81P_port_w);
	DECLARE_READ8_MEMBER(usart_8251_r);
	DECLARE_WRITE8_MEMBER(usart_8251_w);
	DECLARE_READ8_MEMBER(copro_sm7831_r);
	DECLARE_WRITE8_MEMBER(copro_sm7831_w);
	DECLARE_WRITE8_MEMBER(lucky74_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(lucky74_fg_colorram_w);
	DECLARE_WRITE8_MEMBER(lucky74_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(lucky74_bg_colorram_w);
	DECLARE_WRITE8_MEMBER(ym2149_portb_w);
	DECLARE_WRITE8_MEMBER(lamps_a_w);
	DECLARE_WRITE8_MEMBER(lamps_b_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	DECLARE_PALETTE_INIT(lucky74);
	UINT32 screen_update_lucky74(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(nmi_interrupt);
	DECLARE_WRITE_LINE_MEMBER(lucky74_adpcm_int);
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
};
