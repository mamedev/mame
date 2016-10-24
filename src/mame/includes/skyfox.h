// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Skyfox

*************************************************************************/

#include "machine/gen_latch.h"

class skyfox_state : public driver_device
{
public:
	skyfox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram")
	{ }

	/* devices/memory pointers */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_bg_pos;
	int m_bg_ctrl;

	void skyfox_vregs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_skyfox();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette_init_skyfox(palette_device &palette);
	uint32_t screen_update_skyfox(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void skyfox_interrupt(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
