// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,David Haywood

#include "machine/74259.h"
#include "emupal.h"
#include "tilemap.h"

class freekick_state : public driver_device
{
public:
	freekick_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_bank1(*this, "bank1"),
		m_bank1d(*this, "bank1d") { }

	void base(machine_config &config);
	void oigas(machine_config &config);
	void pbillrd(machine_config &config);
	void pbillrdbl(machine_config &config);
	void gigas(machine_config &config);
	void gigasm(machine_config &config);
	void pbillrdm(machine_config &config);
	void omega(machine_config &config);
	void freekick(machine_config &config);

	void init_gigas();
	void init_gigasb();
	void init_pbillrdbl();
	void init_pbillrds();

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_freek_tilemap;

	/* misc */
	int        m_inval;
	int        m_outval;
	int        m_cnt;   // used by oigas
	int        m_romaddr;
	int        m_spinner;
	int        m_nmi_en;
	int        m_ff_data;
	std::unique_ptr<uint8_t[]> m_decrypted_opcodes;
	DECLARE_WRITE_LINE_MEMBER(flipscreen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_y_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_w);
	DECLARE_WRITE_LINE_MEMBER(spinner_select_w);
	void gigas_spinner_select_w(uint8_t data);
	uint8_t spinner_r();
	void pbillrd_bankswitch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(nmi_enable_w);
	void oigas_5_w(uint8_t data);
	uint8_t oigas_3_r();
	uint8_t oigas_2_r();
	uint8_t freekick_ff_r();
	void freekick_ff_w(uint8_t data);
	void freek_videoram_w(offs_t offset, uint8_t data);
	void snd_rom_addr_l_w(uint8_t data);
	void snd_rom_addr_h_w(uint8_t data);
	uint8_t snd_rom_r();
	TILE_GET_INFO_MEMBER(get_freek_tile_info);
	virtual void video_start() override;
	DECLARE_MACHINE_START(pbillrd);
	DECLARE_MACHINE_RESET(freekick);
	DECLARE_MACHINE_START(freekick);
	DECLARE_MACHINE_START(oigas);
	DECLARE_MACHINE_RESET(oigas);
	uint32_t screen_update_pbillrd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_freekick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gigas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void gigas_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void pbillrd_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void freekick_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;
	optional_memory_bank m_bank1, m_bank1d;
	void decrypted_opcodes_map(address_map &map);
	void freekick_io_map(address_map &map);
	void freekick_map(address_map &map);
	void gigas_io_map(address_map &map);
	void gigas_map(address_map &map);
	void oigas_io_map(address_map &map);
	void omega_io_map(address_map &map);
	void omega_map(address_map &map);
	void pbillrd_map(address_map &map);
	void pbillrdbl_map(address_map &map);
};
