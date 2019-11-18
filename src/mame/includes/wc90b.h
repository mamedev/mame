// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_INCLUDES_WC90B_H
#define MAME_INCLUDES_WC90B_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class wc90b_state : public driver_device
{
public:
	wc90b_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_soundlatch(*this, "soundlatch"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll1x(*this, "scroll1x"),
		m_scroll2x(*this, "scroll2x"),
		m_scroll1y(*this, "scroll1y"),
		m_scroll2y(*this, "scroll2y"),
		m_scroll_x_lo(*this, "scroll_x_lo")
	{ }

	void wc90b(machine_config &config);
	void eurogael(machine_config &config);

	void init_wc90b();

	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(txvideoram_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	void sound_cpu(address_map &map);
	void wc90b_map2(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_spriteram;


	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );

private:

	optional_shared_ptr<uint8_t> m_scroll1x;
	optional_shared_ptr<uint8_t> m_scroll2x;
	optional_shared_ptr<uint8_t> m_scroll1y;
	optional_shared_ptr<uint8_t> m_scroll2y;
	optional_shared_ptr<uint8_t> m_scroll_x_lo;

	void wc90b_map1(address_map &map);

	int m_msm5205next;
	int m_toggle;

	DECLARE_WRITE8_MEMBER(bankswitch1_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(adpcm_control_w);
	DECLARE_READ8_MEMBER(master_irq_ack_r);
	DECLARE_WRITE8_MEMBER(slave_irq_ack_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
};


class eurogael_state : public wc90b_state
{
public:
	eurogael_state(const machine_config &mconfig, device_type type, const char *tag) :
		wc90b_state(mconfig, type, tag),
		m_bgscroll(*this, "bgscroll")
	{ }

	void eurogael(machine_config &config);

protected:
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority ) override;

private:
	DECLARE_WRITE8_MEMBER(master_irq_ack_w);
	required_shared_ptr<uint8_t> m_bgscroll;

	void map1(address_map &map);
};


#endif // MAME_INCLUDES_WC90B_H
