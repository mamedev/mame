// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "machine/c117.h"
#include "sound/dac.h"
#include "sound/namco.h"
#include "video/c116.h"

class namcos1_state : public driver_device
{
public:
	namcos1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_c116(*this, "c116"),
		m_c117(*this, "c117"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_playfield_control(*this, "pfcontrol"),
		m_triram(*this, "triram"),
		m_rom(*this, "user1"),
		m_soundbank(*this, "soundbank"),
		m_mcubank(*this, "mcubank"),
		m_io_dipsw(*this, "DIPSW") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_c116_device> m_c116;
	required_device<namco_c117_device> m_c117;
	required_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_playfield_control;
	required_shared_ptr<UINT8> m_triram;
	required_region_ptr<UINT8> m_rom;

	required_memory_bank m_soundbank;
	required_memory_bank m_mcubank;

	required_ioport m_io_dipsw;

	int m_dac0_value;
	int m_dac1_value;
	int m_dac0_gain;
	int m_dac1_gain;
	int m_key_id;
	int m_key_reg;
	int m_key_rng;
	int m_key_swap4_arg;
	int m_key_swap4;
	int m_key_bottom4;
	int m_key_top4;
	unsigned int m_key_quotient;
	unsigned int m_key_reminder;
	unsigned int m_key_numerator_high_word;
	UINT8 m_key[8];
	int m_mcu_patch_data;
	int m_reset;
	int m_input_count;
	int m_strobe;
	int m_strobe_count;
	int m_stored_input[2];
	tilemap_t *m_bg_tilemap[6];
	UINT8 *m_tilemap_maskdata;
	int m_copy_sprites;
	UINT8 m_drawmode_table[16];

	DECLARE_WRITE_LINE_MEMBER(subres_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(dsw_r);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(dac_gain_w);
	DECLARE_WRITE8_MEMBER(dac0_w);
	DECLARE_WRITE8_MEMBER(dac1_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(mcu_bankswitch_w);
	DECLARE_WRITE8_MEMBER(mcu_patch_w);
	DECLARE_READ8_MEMBER(quester_paddle_r);
	DECLARE_READ8_MEMBER(berabohm_buttons_r);
	DECLARE_READ8_MEMBER(faceoff_inputs_r);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(spriteram_w);
	DECLARE_WRITE8_MEMBER(_3dcs_w);
	DECLARE_READ8_MEMBER(no_key_r);
	DECLARE_WRITE8_MEMBER(no_key_w);
	DECLARE_READ8_MEMBER(key_type1_r);
	DECLARE_WRITE8_MEMBER(key_type1_w);
	DECLARE_READ8_MEMBER(key_type2_r);
	DECLARE_WRITE8_MEMBER(key_type2_w);
	DECLARE_READ8_MEMBER(key_type3_r);
	DECLARE_WRITE8_MEMBER(key_type3_w);

	DECLARE_DRIVER_INIT(pacmania);
	DECLARE_DRIVER_INIT(ws);
	DECLARE_DRIVER_INIT(wldcourt);
	DECLARE_DRIVER_INIT(tankfrc4);
	DECLARE_DRIVER_INIT(blazer);
	DECLARE_DRIVER_INIT(dangseed);
	DECLARE_DRIVER_INIT(splatter);
	DECLARE_DRIVER_INIT(alice);
	DECLARE_DRIVER_INIT(faceoff);
	DECLARE_DRIVER_INIT(puzlclub);
	DECLARE_DRIVER_INIT(bakutotu);
	DECLARE_DRIVER_INIT(rompers);
	DECLARE_DRIVER_INIT(ws90);
	DECLARE_DRIVER_INIT(tankfrce);
	DECLARE_DRIVER_INIT(soukobdx);
	DECLARE_DRIVER_INIT(shadowld);
	DECLARE_DRIVER_INIT(berabohm);
	DECLARE_DRIVER_INIT(galaga88);
	DECLARE_DRIVER_INIT(blastoff);
	DECLARE_DRIVER_INIT(quester);
	DECLARE_DRIVER_INIT(ws89);
	DECLARE_DRIVER_INIT(dspirit);
	DECLARE_DRIVER_INIT(pistoldm);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	void driver_init();

	TILE_GET_INFO_MEMBER(bg_get_info0);
	TILE_GET_INFO_MEMBER(bg_get_info1);
	TILE_GET_INFO_MEMBER(bg_get_info2);
	TILE_GET_INFO_MEMBER(bg_get_info3);
	TILE_GET_INFO_MEMBER(fg_get_info4);
	TILE_GET_INFO_MEMBER(fg_get_info5);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void update_DACs();
	void init_DACs();

private:
	inline void get_tile_info(tile_data &tileinfo,int tile_index,UINT8 *info_vram);
};
