// license:BSD-3-Clause
// copyright-holders:Philip Bennett,Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino, Ernesto Corvi, David Haywood, R. Belmont
/*************************************************************************

    Double Dragon & Double Dragon II (but also China Gate)

*************************************************************************/
#include "sound/msm5205.h"

class ddragon_state : public driver_device
{
public:
	ddragon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rambase(*this, "rambase"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_comram(*this, "comram"),
		m_spriteram(*this, "spriteram"),
		m_scrollx_lo(*this, "scrollx_lo"),
		m_scrolly_lo(*this, "scrolly_lo"),
		m_darktowr_mcu_ports(*this, "darktowr_mcu"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_subcpu(*this, "sub"),
		m_adpcm1(*this, "adpcm1"),
		m_adpcm2(*this, "adpcm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<UINT8> m_rambase;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	optional_shared_ptr<UINT8> m_comram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scrollx_lo;
	required_shared_ptr<UINT8> m_scrolly_lo;
	optional_shared_ptr<UINT8> m_darktowr_mcu_ports;

	/* video-related */
	tilemap_t      *m_fg_tilemap;
	tilemap_t      *m_bg_tilemap;
	UINT8          m_technos_video_hw;
	UINT8          m_scrollx_hi;
	UINT8          m_scrolly_hi;

	/* misc */
	UINT8          m_ddragon_sub_port;
	UINT8          m_sprite_irq;
	UINT8          m_sound_irq;
	UINT8          m_ym_irq;
	UINT8          m_adpcm_sound_irq;
	UINT32         m_adpcm_pos[2];
	UINT32         m_adpcm_end[2];
	UINT8          m_adpcm_idle[2];
	int            m_adpcm_data[2];

	/* for Sai Yu Gou Ma Roku */
	int            m_adpcm_addr;
	int            m_i8748_P1;
	int            m_i8748_P2;
	int            m_pcm_shift;
	int            m_pcm_nibble;
	int            m_mcu_command;
#if 0
	int            m_m5205_clk;
#endif

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<msm5205_device> m_adpcm1;
	optional_device<msm5205_device> m_adpcm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;


	int scanline_to_vcount(int scanline);
	void ddragon_interrupt_ack(address_space &space, offs_t offset, UINT8 data);
	void dd_adpcm_int(msm5205_device *device, int chip);

	/* video/ddragon.c */
	TILEMAP_MAPPER_MEMBER(background_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_16color_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	UINT32 screen_update_ddragon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_MACHINE_START(ddragon);
	DECLARE_MACHINE_RESET(ddragon);
	DECLARE_VIDEO_START(ddragon);

	TIMER_DEVICE_CALLBACK_MEMBER(ddragon_scanline);

	DECLARE_WRITE_LINE_MEMBER(irq_handler);
	DECLARE_WRITE8_MEMBER(ddragon_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(ddragon_fgvideoram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(subcpu_bus_free);
	DECLARE_WRITE8_MEMBER(ddragon_bankswitch_w);
	DECLARE_WRITE8_MEMBER(toffy_bankswitch_w);
	DECLARE_READ8_MEMBER(darktowr_mcu_bank_r);
	DECLARE_READ8_MEMBER(soundlatch_ack_r);
	DECLARE_WRITE8_MEMBER(darktowr_mcu_bank_w);
	DECLARE_WRITE8_MEMBER(darktowr_bankswitch_w);
	DECLARE_READ8_MEMBER(ddragon_interrupt_r);
	DECLARE_WRITE8_MEMBER(ddragon_interrupt_w);
	DECLARE_WRITE8_MEMBER(ddragon2_sub_irq_ack_w);
	DECLARE_WRITE8_MEMBER(ddragon2_sub_irq_w);
	DECLARE_WRITE8_MEMBER(darktowr_mcu_w);
	DECLARE_READ8_MEMBER(ddragon_hd63701_internal_registers_r);
	DECLARE_WRITE8_MEMBER(ddragon_hd63701_internal_registers_w);
	DECLARE_READ8_MEMBER(ddragon_comram_r);
	DECLARE_WRITE8_MEMBER(ddragon_comram_w);
	DECLARE_WRITE8_MEMBER(dd_adpcm_w);
	DECLARE_READ8_MEMBER(dd_adpcm_status_r);
	DECLARE_WRITE8_MEMBER(ddragonba_port_w);
	DECLARE_WRITE_LINE_MEMBER(dd_adpcm_int_1);
	DECLARE_WRITE_LINE_MEMBER(dd_adpcm_int_2);

	DECLARE_DRIVER_INIT(toffy);
	DECLARE_DRIVER_INIT(darktowr);
	DECLARE_DRIVER_INIT(ddragon2);
	DECLARE_DRIVER_INIT(ddragon);
	DECLARE_DRIVER_INIT(ddragon6809);
};
