// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

/**************** Machine stuff ******************/
//#define USE_HD64x180          /* Define if CPU support is available */
//#define TRUXTON2_STEREO       /* Uncomment to hear truxton2 music in stereo */

// We encode priority with colour in the tilemaps, so need a larger palette
#define T2PALETTE_LENGTH 0x10000

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/nmk112.h"
#include "machine/ticket.h"
#include "machine/upd4992.h"
#include "video/gp9001.h"
#include "sound/okim6295.h"

class toaplan2_state : public driver_device
{
public:
	enum
	{
		TIMER_RAISE_IRQ
	};

	toaplan2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram"),
		m_shared_ram16(*this, "shared_ram16"),
		m_paletteram(*this, "palette"),
		m_tx_videoram(*this, "tx_videoram"),
		m_tx_lineselect(*this, "tx_lineselect"),
		m_tx_linescroll(*this, "tx_linescroll"),
		m_tx_gfxram16(*this, "tx_gfxram16"),
		m_mainram16(*this, "mainram16"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_vdp0(*this, "gp9001"),
		m_vdp1(*this, "gp9001_1"),
		m_nmk112(*this, "nmk112"),
		m_oki(*this, "oki"),
		m_oki1(*this, "oki1"),
		m_eeprom(*this, "eeprom"),
		m_rtc(*this, "rtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_hopper(*this, "hopper") { }

	optional_shared_ptr<uint8_t> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	optional_shared_ptr<uint16_t> m_shared_ram16;     // Really 8 bit RAM connected to Z180
	optional_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint16_t> m_tx_videoram;
	optional_shared_ptr<uint16_t> m_tx_lineselect;
	optional_shared_ptr<uint16_t> m_tx_linescroll;
	optional_shared_ptr<uint16_t> m_tx_gfxram16;
	optional_shared_ptr<uint16_t> m_mainram16;

	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp0;
	optional_device<gp9001vdp_device> m_vdp1;
	optional_device<nmk112_device> m_nmk112;
	optional_device<okim6295_device> m_oki;
	optional_device<okim6295_device> m_oki1;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<upd4992_device> m_rtc;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // batrider and bgaregga and batsugun
	optional_device<generic_latch_8_device> m_soundlatch2;
	optional_device<ticket_dispenser_device> m_hopper;

	uint16_t m_mcu_data;
	int8_t m_old_p1_paddle_h; /* For Ghox */
	int8_t m_old_p2_paddle_h;
	uint8_t m_v25_reset_line; /* 0x20 for dogyuun/batsugun, 0x10 for vfive, 0x08 for fixeight */
	uint8_t m_sndirq_line;        /* IRQ4 for batrider, IRQ2 for bbakraid */
	uint8_t m_z80_busreq;

	bitmap_ind8 m_custom_priority_bitmap;
	bitmap_ind16 m_secondary_render_bitmap;

	tilemap_t *m_tx_tilemap;    /* Tilemap for extra-text-layer */
	uint16_t video_count_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void toaplan2_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void toaplan2_coin_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan2_v25_coin_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void shippumd_coin_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t shared_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void shared_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan2_hd647180_cpu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ghox_p1_h_analog_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ghox_p2_h_analog_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ghox_mcu_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ghox_mcu_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ghox_shared_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ghox_shared_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fixeight_subcpu_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void fixeightbl_oki_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t fixeight_region_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void raizing_z80_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void raizing_oki_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bgaregga_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t bgaregga_E01D_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bgaregga_E00C_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t batrider_z80_busack_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void batrider_z80_busreq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t batrider_z80rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void batrider_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void batrider_soundlatch2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void batrider_unknown_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void batrider_clear_sndirq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void batrider_sndirq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void batrider_clear_nmi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t bbakraid_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void bbakraid_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan2_tx_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan2_tx_linescroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void toaplan2_tx_gfxram16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void batrider_textdata_dma_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void batrider_unknown_dma_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void batrider_objectbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value c2map_r(ioport_field &field, void *param);
	void oki_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void oki1_bankswitch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_bbakraid();
	void init_pipibibsbl();
	void init_dogyuun();
	void init_fixeight();
	void init_bgaregga();
	void init_fixeightbl();
	void init_vfive();
	void init_batrider();
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_toaplan2();
	void machine_reset_toaplan2();
	void video_start_toaplan2();
	void machine_reset_ghox();
	void video_start_truxton2();
	void video_start_fixeightbl();
	void video_start_bgaregga();
	void video_start_bgareggabl();
	void video_start_batrider();
	uint32_t screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dogyuun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_toaplan2(screen_device &screen, bool state);
	void toaplan2_vblank_irq1(device_t &device);
	void toaplan2_vblank_irq2(device_t &device);
	void toaplan2_vblank_irq4(device_t &device);
	void bbakraid_snd_interrupt(device_t &device);
	void truxton2_postload();
	void create_tx_tilemap(int dx = 0, int dx_flipped = 0);
	void toaplan2_vblank_irq(int irq_line);

	void pwrkick_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pwrkick_coin_lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void toaplan2_reset(int state);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
