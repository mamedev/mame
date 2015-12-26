// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood, Luca Elia
#include "video/fuukifg.h"

/* Define clocks based on actual OSC on the PCB */

#define CPU_CLOCK       (XTAL_40MHz / 2)        /* clock for 68020 */
#define SOUND_CPU_CLOCK     (XTAL_12MHz / 2)        /* clock for Z80 sound CPU */
#define FM_SOUND_CLOCK      (XTAL_33_8688MHz / 2)       /* FM clock */

/* NOTE: YMF278B_STD_CLOCK is defined in /src/emu/sound/ymf278b.h */


class fuuki32_state : public driver_device
{
public:
	enum
	{
		TIMER_LEVEL_1_INTERRUPT,
		TIMER_VBLANK_INTERRUPT,
		TIMER_RASTER_INTERRUPT
	};

	fuuki32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_fuukivid(*this, "fuukivid"),
		m_vram(*this, "vram"),
		m_vregs(*this, "vregs"),
		m_priority(*this, "priority"),
		m_tilebank(*this, "tilebank")
		{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<fuukivid_device> m_fuukivid;

	/* memory pointers */
	required_shared_ptr_array<UINT32,4> m_vram;
	required_shared_ptr<UINT32> m_vregs;
	required_shared_ptr<UINT32> m_priority;
	required_shared_ptr<UINT32> m_tilebank;
	//UINT32 *    m_buf_spriteram;
	//UINT32 *    m_buf_spriteram2;

	/* video-related */
	tilemap_t     *m_tilemap[4];
	UINT32      m_spr_buffered_tilebank[2];

	/* misc */
	emu_timer   *m_level_1_interrupt_timer;
	emu_timer   *m_vblank_interrupt_timer;
	emu_timer   *m_raster_interrupt_timer;
	UINT8       m_shared_ram[16];

	DECLARE_READ32_MEMBER(snd_020_r);
	DECLARE_WRITE32_MEMBER(snd_020_w);
	DECLARE_WRITE32_MEMBER(vregs_w);
	DECLARE_WRITE8_MEMBER(sound_bw_w);
	DECLARE_READ8_MEMBER(snd_z80_r);
	DECLARE_WRITE8_MEMBER(snd_z80_w);
	DECLARE_WRITE8_MEMBER(snd_ymf278b_w);
	DECLARE_WRITE32_MEMBER(vram_0_w);
	DECLARE_WRITE32_MEMBER(vram_1_w);
	DECLARE_WRITE32_MEMBER(vram_2_w);
	DECLARE_WRITE32_MEMBER(vram_3_w);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_3);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	inline void get_tile_info8bpp(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_);
	inline void get_tile_info4bpp(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_);
	inline void vram_w(offs_t offset, UINT32 data, UINT32 mem_mask, int _N_);
	void draw_layer( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int flag, int pri );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
