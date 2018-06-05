// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Glass

*************************************************************************/

#include "machine/74259.h"

class glass_state : public driver_device
{
public:
	glass_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"),
		m_pant{ nullptr, nullptr }
	{ }


	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;

	/* video-related */
	tilemap_t     *m_pant[2];
	std::unique_ptr<bitmap_ind16> m_screen_bitmap;

	/* misc */
	int         m_current_bit;
	int         m_current_command;
	int         m_cause_interrupt;
	int         m_blitter_serial_buffer[5];

	DECLARE_WRITE8_MEMBER(shareram_w);
	DECLARE_READ8_MEMBER(shareram_r);
	DECLARE_WRITE16_MEMBER(clr_int_w);
	DECLARE_WRITE16_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(coin_w);
	DECLARE_WRITE16_MEMBER(blitter_w);
	DECLARE_WRITE16_MEMBER(vram_w);

	DECLARE_WRITE_LINE_MEMBER(coin1_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);

	void init_glass();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_tile_info_screen0);
	TILE_GET_INFO_MEMBER(get_tile_info_screen1);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void ROM16_split_gfx( const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2 );
	void glass(machine_config &config);
	void glass_ds5002fp(machine_config &config);
	void glass_map(address_map &map);
	void mcu_hostmem_map(address_map &map);
	void oki_map(address_map &map);
};
