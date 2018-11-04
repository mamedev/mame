// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    DJ Boy

*************************************************************************/

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "video/kan_pand.h"

#define PROT_OUTPUT_BUFFER_SIZE 8

class djboy_state : public driver_device
{
public:
	djboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_cpu1(*this, "cpu1"),
		m_cpu2(*this, "cpu2"),
		m_beast(*this, "beast"),
		m_pandora(*this, "pandora"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
		{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_paletteram;

	/* ROM banking */
	uint8_t       m_bankxor;

	/* video-related */
	tilemap_t   *m_background;
	uint8_t       m_videoreg;
	uint8_t       m_scrollx;
	uint8_t       m_scrolly;

	/* Kaneko BEAST state */
	uint8_t       m_data_to_beast;
	uint8_t       m_data_to_z80;
	uint8_t       m_beast_to_z80_full;
	uint8_t       m_z80_to_beast_full;
	uint8_t       m_beast_int0_l;
	uint8_t       m_beast_p0;
	uint8_t       m_beast_p1;
	uint8_t       m_beast_p2;
	uint8_t       m_beast_p3;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;
	required_device<cpu_device> m_beast;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

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
	uint32_t screen_update_djboy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_djboy);
	TIMER_DEVICE_CALLBACK_MEMBER(djboy_scanline);
	void djboy(machine_config &config);
	void cpu0_am(address_map &map);
	void cpu0_port_am(address_map &map);
	void cpu1_am(address_map &map);
	void cpu1_port_am(address_map &map);
	void cpu2_am(address_map &map);
	void cpu2_port_am(address_map &map);
	void djboy_mcu_io_map(address_map &map);
};
