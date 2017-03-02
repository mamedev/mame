// license:BSD-3-Clause
// copyright-holders:Lee Taylor, Chris Moore
/*************************************************************************

    Cheeky Mouse

*************************************************************************/

#include "sound/dac.h"
#include "screen.h"

class cheekyms_state : public driver_device
{
public:
	cheekyms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac0(*this, "dac0"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_dac3(*this, "dac3"),
		m_dac4(*this, "dac4"),
		m_dac5(*this, "dac5"),
		m_dac6(*this, "dac6"),
		m_dac7(*this, "dac7"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_port_80(*this, "port_80") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<dac_bit_interface> m_dac0;
	required_device<dac_bit_interface> m_dac1;
	required_device<dac_bit_interface> m_dac2;
	required_device<dac_bit_interface> m_dac3;
	required_device<dac_bit_interface> m_dac4;
	required_device<dac_bit_interface> m_dac5;
	required_device<dac_bit_interface> m_dac6;
	required_device<dac_bit_interface> m_dac7;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_port_80;

	/* video-related */
	tilemap_t        *m_cm_tilemap;
	std::unique_ptr<bitmap_ind16>       m_bitmap_buffer;

	uint8_t          m_irq_mask;

	DECLARE_WRITE8_MEMBER(port_40_w);
	DECLARE_WRITE8_MEMBER(port_80_w);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	INTERRUPT_GEN_MEMBER(vblank_irq);

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(cheekyms);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip );
};
