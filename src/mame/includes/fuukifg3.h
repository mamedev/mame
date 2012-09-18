

/* Define clocks based on actual OSC on the PCB */

#define CPU_CLOCK		(XTAL_40MHz / 2)		/* clock for 68020 */
#define SOUND_CPU_CLOCK		(XTAL_12MHz / 2)		/* clock for Z80 sound CPU */
#define FM_SOUND_CLOCK		(XTAL_33_8688MHz / 2)		/* FM clock */

/* NOTE: YMF278B_STD_CLOCK is defined in /src/emu/sound/ymf278b.h */


class fuuki32_state : public driver_device
{
public:
	fuuki32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_vregs(*this, "vregs"),
		m_priority(*this, "priority"),
		m_tilebank(*this, "tilebank"){ }

	/* memory pointers */
	required_shared_ptr_array<UINT32,4> m_vram;
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_paletteram;
	required_shared_ptr<UINT32> m_vregs;
	required_shared_ptr<UINT32> m_priority;
	required_shared_ptr<UINT32> m_tilebank;
	UINT32 *    m_buf_spriteram;
	UINT32 *    m_buf_spriteram2;

	/* video-related */
	tilemap_t     *m_tilemap[4];
	UINT32      m_spr_buffered_tilebank[2];

	/* misc */
	emu_timer   *m_raster_interrupt_timer;
	UINT8       m_shared_ram[16];

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	DECLARE_WRITE32_MEMBER(paletteram32_xRRRRRGGGGGBBBBB_dword_w);
	DECLARE_READ32_MEMBER(snd_020_r);
	DECLARE_WRITE32_MEMBER(snd_020_w);
	DECLARE_WRITE32_MEMBER(fuuki32_vregs_w);
	DECLARE_WRITE8_MEMBER(fuuki32_sound_bw_w);
	DECLARE_READ8_MEMBER(snd_z80_r);
	DECLARE_WRITE8_MEMBER(snd_z80_w);
	DECLARE_WRITE8_MEMBER(snd_ymf278b_w);
	DECLARE_WRITE32_MEMBER(fuuki32_vram_0_w);
	DECLARE_WRITE32_MEMBER(fuuki32_vram_1_w);
	DECLARE_WRITE32_MEMBER(fuuki32_vram_2_w);
	DECLARE_WRITE32_MEMBER(fuuki32_vram_3_w);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_3);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_fuuki32(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_fuuki32(screen_device &screen, bool state);
};
