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
			m_palette(*this, "palette")
		{
			m_irq4_disabled = 0;
		}

	/* memory pointers */
	required_shared_ptr<UINT16> m_videoregs;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT8> m_z80_mainram;
	required_shared_ptr<UINT16> m_mainram;
	UINT16 *      m_bg_videoram;
	UINT16 *      m_tx_videoram;
	UINT16 *      m_rowscrollram;
	std::unique_ptr<UINT8[]>      m_sprite_a_region;
	size_t        m_sprite_a_region_size;
	std::unique_ptr<UINT16[]>     m_spritebufferram; // buffered spriteram

	/* video-related */
	tilemap_t       *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;
	UINT16        *m_sprite_temp_render;
	bitmap_rgb32      m_tmppgmbitmap;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	device_t *m_ics;

	/* used by rendering */
	UINT8 *m_bdata;
	size_t  m_bdatasize;
	int m_aoffset;
	int m_boffset;

	/* hack */
	int m_irq4_disabled;

	/* calendar */
	UINT8        m_cal_val;
	UINT8        m_cal_mask;
	UINT8        m_cal_com;
	UINT8        m_cal_cnt;
	system_time  m_systime;

	DECLARE_READ16_MEMBER(pgm_videoram_r);
	DECLARE_WRITE16_MEMBER(pgm_videoram_w);
	DECLARE_WRITE16_MEMBER(pgm_coin_counter_w);
	DECLARE_READ16_MEMBER(z80_ram_r);
	DECLARE_WRITE16_MEMBER(z80_ram_w);
	DECLARE_WRITE16_MEMBER(z80_reset_w);
	DECLARE_WRITE16_MEMBER(z80_ctrl_w);
	DECLARE_WRITE16_MEMBER(m68k_l1_w);
	DECLARE_WRITE8_MEMBER(z80_l3_w);
	DECLARE_WRITE16_MEMBER(pgm_tx_videoram_w);
	DECLARE_WRITE16_MEMBER(pgm_bg_videoram_w);
	DECLARE_WRITE_LINE_MEMBER(pgm_sound_irq);

	DECLARE_DRIVER_INIT(pgm);

	TILE_GET_INFO_MEMBER(get_pgm_tx_tilemap_tile_info);
	TILE_GET_INFO_MEMBER(get_pgm_bg_tilemap_tile_info);
	DECLARE_VIDEO_START(pgm);
	DECLARE_MACHINE_START(pgm);
	DECLARE_MACHINE_RESET(pgm);
	UINT32 screen_update_pgm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_pgm(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(pgm_interrupt);

	inline void pgm_draw_pix( int xdrawpos, int pri, UINT16* dest, UINT8* destpri, UINT16 srcdat);
	inline void pgm_draw_pix_nopri( int xdrawpos, UINT16* dest, UINT8* destpri, UINT16 srcdat);
	inline void pgm_draw_pix_pri( int xdrawpos, UINT16* dest, UINT8* destpri, UINT16 srcdat);
	void draw_sprite_line( int wide, UINT16* dest, UINT8* destpri, int xzoom, int xgrow, int flip, int xpos, int pri, int realxsize, int palt, int draw );
	void draw_sprite_new_zoomed( int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, UINT32 xzoom, int xgrow, UINT32 yzoom, int ygrow, int pri );
	void draw_sprite_line_basic( int wide, UINT16* dest, UINT8* destpri, int flip, int xpos, int pri, int realxsize, int palt, int draw );
	void draw_sprite_new_basic( int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, int pri );
	void draw_sprites( bitmap_ind16& spritebitmap, UINT16 *sprite_source, bitmap_ind8& priority_bitmap );
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
void pgm_sound_irq( device_t *device, int level );

ADDRESS_MAP_EXTERN( pgm_mem, 16 );
ADDRESS_MAP_EXTERN( pgm_basic_mem, 16 );
ADDRESS_MAP_EXTERN( pgm_base_mem, 16 );
