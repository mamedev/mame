// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    DJ Boy

*************************************************************************/

#include "video/kan_pand.h"

#define PROT_OUTPUT_BUFFER_SIZE 8

class djboy_state : public driver_device
{
public:
	djboy_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_cpu1(*this, "cpu1"),
		m_cpu2(*this, "cpu2"),
		m_beast(*this, "beast"),
		m_pandora(*this, "pandora"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
		{ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_paletteram;

	/* ROM banking */
	UINT8       m_bankxor;

	/* video-related */
	tilemap_t   *m_background;
	UINT8       m_videoreg;
	UINT8       m_scrollx;
	UINT8       m_scrolly;

	/* Kaneko BEAST state */
	UINT8       m_data_to_beast;
	UINT8       m_data_to_z80;
	UINT8       m_beast_to_z80_full;
	UINT8       m_z80_to_beast_full;
	UINT8       m_beast_int0_l;
	UINT8       m_beast_p0;
	UINT8       m_beast_p1;
	UINT8       m_beast_p2;
	UINT8       m_beast_p3;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;
	required_device<cpu_device> m_beast;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(beast_data_w);
	DECLARE_READ8_MEMBER(beast_data_r);
	DECLARE_READ8_MEMBER(beast_status_r);
	DECLARE_WRITE8_MEMBER(trigger_nmi_on_cpu0);
	DECLARE_WRITE8_MEMBER(cpu0_bankswitch_w);
	DECLARE_WRITE8_MEMBER(cpu1_bankswitch_w);
	DECLARE_WRITE8_MEMBER(coin_count_w);
	DECLARE_WRITE8_MEMBER(trigger_nmi_on_sound_cpu2);
	DECLARE_WRITE8_MEMBER(cpu2_bankswitch_w);
	DECLARE_READ8_MEMBER(beast_p0_r);
	DECLARE_WRITE8_MEMBER(beast_p0_w);
	DECLARE_READ8_MEMBER(beast_p1_r);
	DECLARE_WRITE8_MEMBER(beast_p1_w);
	DECLARE_READ8_MEMBER(beast_p2_r);
	DECLARE_WRITE8_MEMBER(beast_p2_w);
	DECLARE_READ8_MEMBER(beast_p3_r);
	DECLARE_WRITE8_MEMBER(beast_p3_w);
	DECLARE_WRITE8_MEMBER(djboy_scrollx_w);
	DECLARE_WRITE8_MEMBER(djboy_scrolly_w);
	DECLARE_WRITE8_MEMBER(djboy_videoram_w);
	DECLARE_WRITE8_MEMBER(djboy_paletteram_w);
	DECLARE_DRIVER_INIT(djboy);
	DECLARE_DRIVER_INIT(djboyj);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_djboy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_djboy(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(djboy_scanline);
};
