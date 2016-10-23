// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, Couriersud
/*************************************************************************

    IronHorse

*************************************************************************/

#include "machine/gen_latch.h"

class ironhors_state : public driver_device
{
public:
	ironhors_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_interrupt_enable(*this, "int_enable"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram(*this, "spriteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_interrupt_enable;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_palettebank;
	int        m_charbank;
	int        m_spriterambank;

	void sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void charbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palettebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void filter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t farwest_soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(farwest_get_bg_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(ironhors);
	void video_start_farwest();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_farwest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void farwest_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	TIMER_DEVICE_CALLBACK_MEMBER(farwest_irq);
};
