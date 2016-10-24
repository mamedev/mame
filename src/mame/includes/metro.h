// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/*************************************************************************

    Metro Games

*************************************************************************/

#include "sound/okim6295.h"
#include "sound/ym2151.h"
#include "sound/es8712.h"
#include "video/k053936.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"

class metro_state : public driver_device
{
public:
	enum
	{
		TIMER_KARATOUR_IRQ,
		TIMER_MOUJA_IRQ,
		TIMER_METRO_BLIT_DONE
	};

	metro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_ymsnd(*this, "ymsnd"),
		m_essnd(*this, "essnd"),
		m_k053936(*this, "k053936") ,
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_spriteram(*this, "spriteram"),
		m_tiletable(*this, "tiletable"),
		m_blitter_regs(*this, "blitter_regs"),
		m_scroll(*this, "scroll"),
		m_window(*this, "window"),
		m_irq_enable(*this, "irq_enable"),
		m_irq_levels(*this, "irq_levels"),
		m_irq_vectors(*this, "irq_vectors"),
		m_rombank(*this, "rombank"),
		m_videoregs(*this, "videoregs"),
		m_screenctrl(*this, "screenctrl"),
		m_input_sel(*this, "input_sel"),
		m_k053936_ram(*this, "k053936_ram")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<device_t> m_ymsnd; // TODO set correct type
	optional_device<es8712_device> m_essnd;
	optional_device<k053936_device> m_k053936;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_vram_0;
	optional_shared_ptr<uint16_t> m_vram_1;
	optional_shared_ptr<uint16_t> m_vram_2;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_tiletable;
	optional_shared_ptr<uint16_t> m_blitter_regs;
	optional_shared_ptr<uint16_t> m_scroll;
	optional_shared_ptr<uint16_t> m_window;
	optional_shared_ptr<uint16_t> m_irq_enable;
	optional_shared_ptr<uint16_t> m_irq_levels;
	optional_shared_ptr<uint16_t> m_irq_vectors;
	optional_shared_ptr<uint16_t> m_rombank;
	required_shared_ptr<uint16_t> m_videoregs;
	optional_shared_ptr<uint16_t> m_screenctrl;
	optional_shared_ptr<uint16_t> m_input_sel;
	optional_shared_ptr<uint16_t> m_k053936_ram;

	int         m_flip_screen;

	/* video-related */
	tilemap_t   *m_k053936_tilemap;
	int         m_tilemap_scrolldx[3];

	int         m_support_8bpp;
	int         m_support_16x16;
	int         m_has_zoom;
	int         m_sprite_xoffs;
	int         m_sprite_yoffs;
	int         m_sprite_xoffs_dx;

	std::unique_ptr<uint8_t[]>      m_expanded_gfx1;

	/* irq_related */
	int         m_vblank_bit;
	int         m_blitter_bit;
	int         m_irq_line;
	uint8_t       m_requested_int[8];
	emu_timer   *m_mouja_irq_timer;

	/* sound related */
	uint16_t      m_soundstatus;
	int         m_porta;
	int         m_portb;
	int         m_busy_sndcpu;

	/* misc */
	int         m_gakusai_oki_bank_lo;
	int         m_gakusai_oki_bank_hi;


	uint16_t metro_irq_cause_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void metro_irq_cause_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mouja_irq_timer_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void metro_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t metro_soundstatus_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void metro_soundstatus_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void metro_sound_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void daitorid_sound_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t metro_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void metro_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metro_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void daitorid_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void metro_coin_lockout_1word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void metro_coin_lockout_4words_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t metro_bankedrom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void metro_blitter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t balcube_dsw_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t karatour_vram_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t karatour_vram_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t karatour_vram_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void karatour_vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void karatour_vram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void karatour_vram_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gakusai_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void blzntrnd_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blzntrnd_sh_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void puzzlet_irq_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_0_clr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_1_clr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_2_clr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void puzzlet_portb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void metro_k053936_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void metro_vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void metro_vram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void metro_vram_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void metro_window_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blt_write( address_space &space, const int tmap, const offs_t offs, const uint16_t data, const uint16_t mask );
	ioport_value custom_soundstatus_r(ioport_field &field, void *param);
	void gakusai_oki_bank_hi_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gakusai_oki_bank_lo_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gakusai_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gakusai_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dokyusp_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dokyusp_eeprom_bit_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dokyusp_eeprom_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mouja_sound_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gakusai_oki_bank_set();

	// vmetal
	void vmetal_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vmetal_es8712_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_karatour();
	void init_daitorid();
	void init_blzntrnd();
	void init_mouja();
	void init_balcube();
	void init_gakusai();
	void init_dharmak();
	void init_puzzlet();
	void init_metro();
	void metro_k053936_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void metro_k053936_gstrik2_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index tilemap_scan_gstrik2(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void expand_gfx1();
	void video_start_metro_i4100();
	void video_start_metro_i4220();
	void video_start_metro_i4220_dx_tmap();
	void video_start_metro_i4220_dx_sprite();
	void video_start_metro_i4300();
	void video_start_blzntrnd();
	void video_start_gstrik2();
	uint32_t screen_update_metro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void metro_vblank_interrupt(device_t &device);
	void metro_periodic_interrupt(device_t &device);
	void karatour_interrupt(device_t &device);
	void puzzlet_interrupt(device_t &device);
	void metro_blit_done(void *ptr, int32_t param);
	void update_irq_state();
	int metro_irq_callback(device_t &device, int irqline);
	inline uint8_t get_tile_pix( uint16_t code, uint8_t x, uint8_t y, int big, uint16_t *pix );
	inline void metro_vram_w( offs_t offset, uint16_t data, uint16_t mem_mask, int layer, uint16_t *vram );
	void metro_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_layers( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int layers_ctrl );
	inline int blt_read( const uint8_t *ROM, const int offs );
	void metro_common(  );
	void draw_tilemap( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t flags, uint32_t pcode,
					int sx, int sy, int wx, int wy, int big, uint16_t *tilemapram, int layer );
	int metro_rxd_r();

protected:
	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
