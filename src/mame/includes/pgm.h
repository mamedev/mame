// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

#include "machine/v3021.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/arm7/arm7.h"
#include "sound/ics2115.h"
#include "cpu/arm7/arm7core.h"
#include "machine/nvram.h"
#include "machine/pgmcrypt.h"
#include "machine/gen_latch.h"
#include "machine/igs025.h"
#include "machine/igs022.h"
#include "machine/igs028.h"

#define PGMARM7LOGERROR 0

class pgm_state : public driver_device
{
public:
	pgm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_videoregs(*this, "videoregs"),
			m_videoram(*this, "videoram"),
			m_z80_mainram(*this, "z80_mainram"),
			m_mainram(*this, "sram"),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_soundlatch(*this, "soundlatch"),
			m_soundlatch3(*this, "soundlatch3")
		{
			m_irq4_disabled = 0;
		}

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoregs;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint8_t> m_z80_mainram;
	required_shared_ptr<uint16_t> m_mainram;
	uint16_t *      m_bg_videoram;
	uint16_t *      m_tx_videoram;
	uint16_t *      m_rowscrollram;
	std::unique_ptr<uint8_t[]>      m_sprite_a_region;
	size_t        m_sprite_a_region_size;
	std::unique_ptr<uint16_t[]>     m_spritebufferram; // buffered spriteram

	/* video-related */
	tilemap_t       *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;
	uint16_t        *m_sprite_temp_render;
	bitmap_rgb32      m_tmppgmbitmap;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch3;
	device_t *m_ics;

	/* used by rendering */
	uint8_t *m_bdata;
	size_t  m_bdatasize;
	int m_aoffset;
	int m_boffset;

	/* hack */
	int m_irq4_disabled;

	/* calendar */
	uint8_t        m_cal_val;
	uint8_t        m_cal_mask;
	uint8_t        m_cal_com;
	uint8_t        m_cal_cnt;
	system_time  m_systime;

	uint16_t pgm_videoram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pgm_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pgm_coin_counter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t z80_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void z80_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void z80_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void z80_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void m68k_l1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void z80_l3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pgm_tx_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pgm_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_pgm();

	void get_pgm_tx_tilemap_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pgm_bg_tilemap_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_pgm();
	void machine_start_pgm();
	void machine_reset_pgm();
	uint32_t screen_update_pgm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_pgm(screen_device &screen, bool state);
	void pgm_interrupt(timer_device &timer, void *ptr, int32_t param);

	inline void pgm_draw_pix( int xdrawpos, int pri, uint16_t* dest, uint8_t* destpri, uint16_t srcdat);
	inline void pgm_draw_pix_nopri( int xdrawpos, uint16_t* dest, uint8_t* destpri, uint16_t srcdat);
	inline void pgm_draw_pix_pri( int xdrawpos, uint16_t* dest, uint8_t* destpri, uint16_t srcdat);
	void draw_sprite_line( int wide, uint16_t* dest, uint8_t* destpri, int xzoom, int xgrow, int flip, int xpos, int pri, int realxsize, int palt, int draw );
	void draw_sprite_new_zoomed( int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, uint32_t xzoom, int xgrow, uint32_t yzoom, int ygrow, int pri );
	void draw_sprite_line_basic( int wide, uint16_t* dest, uint8_t* destpri, int flip, int xpos, int pri, int realxsize, int palt, int draw );
	void draw_sprite_new_basic( int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, int pri );
	void draw_sprites( bitmap_ind16& spritebitmap, uint16_t *sprite_source, bitmap_ind8& priority_bitmap );
	void expand_colourdata();
	void pgm_basic_init( bool set_bank = true);
};



/*----------- defined in drivers/pgm.c -----------*/

INPUT_PORTS_EXTERN( pgm );

GFXDECODE_EXTERN( pgm );

MACHINE_CONFIG_EXTERN( pgm );
MACHINE_CONFIG_EXTERN( pgmbase );

ADDRESS_MAP_EXTERN( pgm_z80_mem, 8 );
ADDRESS_MAP_EXTERN( pgm_z80_io, 8 );

ADDRESS_MAP_EXTERN( pgm_mem, 16 );
ADDRESS_MAP_EXTERN( pgm_basic_mem, 16 );
ADDRESS_MAP_EXTERN( pgm_base_mem, 16 );
