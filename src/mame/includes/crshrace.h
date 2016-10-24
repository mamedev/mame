// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "video/vsystem_spr.h"
#include "video/k053936.h"

class crshrace_state : public driver_device
{
public:
	crshrace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_z80bank(*this, "bank1"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr(*this, "vsystem_spr"),
		m_k053936(*this, "k053936"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram1;
	required_shared_ptr<uint16_t> m_videoram2;

	required_memory_bank m_z80bank;

	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<vsystem_spr_device> m_spr;
	required_device<k053936_device> m_k053936;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* video-related */
	tilemap_t   *m_tilemap1;
	tilemap_t   *m_tilemap2;
	int       m_roz_bank;
	int       m_gfxctrl;
	int       m_flipscreen;
	uint32_t crshrace_tile_callback( uint32_t code );

	/* misc */
	int m_pending_command;

	/* devices */
	void crshrace_sh_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pending_command_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crshrace_videoram1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void crshrace_videoram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void crshrace_roz_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void crshrace_gfxctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value country_sndpending_r(ioport_field &field, void *param);
	void init_crshrace2();
	void init_crshrace();
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_crshrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_crshrace(screen_device &screen, bool state);
	void draw_bg( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
