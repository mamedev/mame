// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Pinball Action

*************************************************************************/

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/z80ctc.h"
#include "emupal.h"

class pbaction_state : public driver_device
{
public:
	pbaction_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_work_ram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	int        m_scroll;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<z80ctc_device> m_ctc;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	uint8_t      m_nmi_mask;
	DECLARE_WRITE8_MEMBER(pbaction_sh_command_w);
	TIMER_CALLBACK_MEMBER(sound_trigger);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(sound_irq_ack_w);
	DECLARE_READ8_MEMBER(pbaction2_prot_kludge_r);
	DECLARE_WRITE8_MEMBER(pbaction_videoram_w);
	DECLARE_WRITE8_MEMBER(pbaction_colorram_w);
	DECLARE_WRITE8_MEMBER(pbaction_videoram2_w);
	DECLARE_WRITE8_MEMBER(pbaction_colorram2_w);
	DECLARE_WRITE8_MEMBER(pbaction_scroll_w);
	DECLARE_WRITE8_MEMBER(pbaction_flipscreen_w);
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
	void pbactiont(machine_config &config);
	void pbactionx(machine_config &config);
	void decrypted_opcodes_map(address_map &map);
	void pbaction_map(address_map &map);
	void pbaction_sound_io_map(address_map &map);
	void pbaction_sound_map(address_map &map);
	void pbactiont_sound_map(address_map &map);
};
