// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/discrete.h"
#include "sound/namco.h"
#include "sound/samples.h"

class galaga_state : public driver_device
{
public:
	galaga_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_galaga_ram1(*this, "galaga_ram1"),
		m_galaga_ram2(*this, "galaga_ram2"),
		m_galaga_ram3(*this, "galaga_ram3"),
		m_galaga_starcontrol(*this, "starcontrol"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_sound(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_galaga_ram1;
	optional_shared_ptr<uint8_t> m_galaga_ram2;
	optional_shared_ptr<uint8_t> m_galaga_ram3;
	optional_shared_ptr<uint8_t> m_galaga_starcontrol;    // 6 addresses
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco_device> m_namco_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	emu_timer *m_cpu3_interrupt_timer;
	uint8_t m_custom_mod;

	/* machine state */
	uint32_t m_stars_scrollx;
	uint32_t m_stars_scrolly;

	uint32_t m_galaga_gfxbank; // used by catsbee

	/* devices */

	/* bank support */

	/* shared */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	uint8_t m_main_irq_mask;
	uint8_t m_sub_irq_mask;
	uint8_t m_sub2_nmi_mask;
	uint8_t bosco_dsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void galaga_flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bosco_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaga_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gatsbee_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out_0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out_1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t namco_52xx_rom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t namco_52xx_si_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t custom_mod_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_galaga();
	void init_gatsbee();
	tilemap_memory_index tilemap_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_galaga();
	void machine_reset_galaga();
	void video_start_galaga();
	void palette_init_galaga(palette_device &palette);
	uint32_t screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_galaga(screen_device &screen, bool state);
	void main_vblank_irq(device_t &device);
	void sub_vblank_irq(device_t &device);
	void cpu3_interrupt_callback(void *ptr, int32_t param);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void bosco_latch_reset();
	struct star
	{
		uint16_t x,y;
		uint8_t col,set;
	};

	static struct star m_star_seed_tab[];
};

DISCRETE_SOUND_EXTERN( galaga );
DISCRETE_SOUND_EXTERN( bosco );
