// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Pinball Action

*************************************************************************/
#ifndef MAME_INCLUDES_PBACTION_H
#define MAME_INCLUDES_PBACTION_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/z80ctc.h"
#include "emupal.h"
#include "tilemap.h"

class pbaction_state : public driver_device
{
public:
	pbaction_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_work_ram(*this, "work_ram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ctc(*this, "ctc"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_work_ram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;
	tilemap_t  *m_fg_tilemap = nullptr;
	int        m_scroll = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<z80ctc_device> m_ctc;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	emu_timer *m_soundcommand_timer = nullptr;
	uint8_t      m_nmi_mask = 0;
	void pbaction_sh_command_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(sound_trigger);
	void nmi_mask_w(uint8_t data);
	uint8_t sound_data_r();
	void sound_irq_ack_w(uint8_t data);
	uint8_t pbaction2_prot_kludge_r();
	void pbaction_videoram_w(offs_t offset, uint8_t data);
	void pbaction_colorram_w(offs_t offset, uint8_t data);
	void pbaction_videoram2_w(offs_t offset, uint8_t data);
	void pbaction_colorram2_w(offs_t offset, uint8_t data);
	void pbaction_scroll_w(uint8_t data);
	void pbaction_flipscreen_w(uint8_t data);
	void init_pbaction2();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_pbaction(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pbaction_interrupt);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(sound_irq_clear);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void pbaction(machine_config &config);
	void pbactionx(machine_config &config);
	void decrypted_opcodes_map(address_map &map);
	void pbaction_map(address_map &map);
	void pbaction_sound_io_map(address_map &map);
	void pbaction_sound_map(address_map &map);
	void pbaction_alt_sound_map(address_map &map);
};

class pbaction_tecfri_state : public pbaction_state
{
public:
	pbaction_tecfri_state(const machine_config &mconfig, device_type type, const char *tag) :
		pbaction_state(mconfig, type, tag),
		m_subcpu(*this, "subcpu"),
		m_ctc2(*this, "ctc2"),
		m_maintosublatch(*this, "maintosublatch"),
		//m_subtomainlatch(*this, "subtomainlatch"),
		m_digits(*this, "digit%u", 0U)
	{ }

	void pbactiont(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void sub_map(address_map &map);
	void sub_io_map(address_map &map);
	void main_io_map(address_map &map);

	TIMER_CALLBACK_MEMBER(sub_trigger);
	emu_timer *m_subcommand_timer = nullptr;

	uint8_t subcpu_r();
	void subcpu_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(sub8000_w);
	DECLARE_WRITE_LINE_MEMBER(sub8001_w);
	void sub8008_w(uint8_t data);

	void subtomain_w(uint8_t data);
	uint8_t maintosub_r();

	required_device<z80_device> m_subcpu;
	required_device<z80ctc_device> m_ctc2;
	required_device<generic_latch_8_device> m_maintosublatch;
	//required_device<generic_latch_8_device> m_subtomainlatch;
	output_finder<24> m_digits;
	uint8_t m_outlatch = 0;
	uint32_t m_outdata = 0;
};

#endif // MAME_INCLUDES_PBACTION_H
