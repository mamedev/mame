// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/*************************************************************************

    Mouser

*************************************************************************/
#ifndef MAME_INCLUDES_MOUSER_H
#define MAME_INCLUDES_MOUSER_H

#pragma once

#include "emupal.h"

class mouser_state : public driver_device
{
public:
	mouser_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void mouser(machine_config &config);

	void init_mouser();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* misc */
	bool         m_nmi_enable = false;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	DECLARE_WRITE_LINE_MEMBER(nmi_enable_w);
	void mouser_sound_nmi_clear_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_y_w);
	void mouser_palette(palette_device &palette) const;
	uint32_t screen_update_mouser(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(mouser_nmi_interrupt);
	INTERRUPT_GEN_MEMBER(mouser_sound_nmi_assert);
	void decrypted_opcodes_map(address_map &map);
	void mouser_map(address_map &map);
	void mouser_sound_io_map(address_map &map);
	void mouser_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MOUSER_H
