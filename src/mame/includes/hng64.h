#include "machine/msm6242.h"

enum
{
	FIGHT_MCU = 1,
	SHOOT_MCU,
	RACING_MCU,
	SAMSHO_MCU,
	BURIKI_MCU
};



class hng64_state : public driver_device
{
public:
	hng64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_comm(*this, "comm"),
		m_rtc(*this, "rtc"),
		m_mainram(*this, "mainram"),
		m_cart(*this, "cart"),
		m_sysregs(*this, "sysregs"),
		m_dualport(*this, "dualport"),
		m_rombase(*this, "rombase"),
		m_spriteram(*this, "spriteram"),
		m_spriteregs(*this, "spriteregs"),
		m_videoram(*this, "videoram"),
		m_videoregs(*this, "videoregs"),
		m_tcram(*this, "tcram"),
		m_dl(*this, "dl"),
		m_3dregs(*this, "3dregs"),
		m_3d_1(*this, "3d_1"),
		m_3d_2(*this, "3d_2"),
		m_com_ram(*this, "com_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_comm;
	required_device<msm6242_device> m_rtc;
	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_cart;
	required_shared_ptr<UINT32> m_sysregs;
	required_shared_ptr<UINT32> m_dualport;
	required_shared_ptr<UINT32> m_rombase;
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_spriteregs;
	required_shared_ptr<UINT32> m_videoram;
	required_shared_ptr<UINT32> m_videoregs;
	required_shared_ptr<UINT32> m_tcram;
	/* 3D stuff */
	required_shared_ptr<UINT32> m_dl;
	required_shared_ptr<UINT32> m_3dregs;
	required_shared_ptr<UINT32> m_3d_1;
	required_shared_ptr<UINT32> m_3d_2;

	required_shared_ptr<UINT32> m_com_ram;
	//required_shared_ptr<UINT8> m_com_mmu_mem;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	int m_mcu_type;

	UINT16 *m_soundram;
	UINT16 *m_soundram2;

	/* Communications stuff */
	UINT8  *m_com_op_base;
	UINT8  *m_com_virtual_mem;
	UINT8 m_com_shared[8];

	INT32 m_dma_start;
	INT32 m_dma_dst;
	INT32 m_dma_len;

	UINT32 m_mcu_fake_time;
	UINT16 m_mcu_en;

	UINT32 m_activeBuffer;
	UINT32 m_no_machine_error_code;

	UINT32 m_unk_vreg_toggle;
	UINT32 m_p1_trig;

	//UINT32 *q2;

	UINT8 m_screen_dis;

	tilemap_t *m_tilemap0_8x8;
	tilemap_t *m_tilemap1_8x8;
	tilemap_t *m_tilemap2_8x8;
	tilemap_t *m_tilemap3_8x8;

	tilemap_t *m_tilemap0_16x16;
	tilemap_t *m_tilemap1_16x16;
	tilemap_t *m_tilemap2_16x16;
	tilemap_t *m_tilemap3_16x16;

	tilemap_t *m_tilemap0_16x16_alt;
	tilemap_t *m_tilemap1_16x16_alt;
	tilemap_t *m_tilemap2_16x16_alt;
	tilemap_t *m_tilemap3_16x16_alt;

	UINT8 m_additive_tilemap_debug;

	// 3d display buffers
	// (Temporarily global - someday they will live with the proper bit-depth in the memory map)
	float *m_depthBuffer3d;
	UINT32 *m_colorBuffer3d;

	UINT32 m_old_animmask;
	UINT32 m_old_animbits;
	UINT16 m_old_tileflags0;
	UINT16 m_old_tileflags1;
	UINT16 m_old_tileflags2;
	UINT16 m_old_tileflags3;

	UINT32 m_dls[2][0x81];

	// 3d State
	int m_paletteState3d;
	float m_projectionMatrix[16];
	float m_modelViewMatrix[16];
	float m_cameraMatrix[16];

	float m_lightStrength;
	float m_lightVector[3];

	DECLARE_READ32_MEMBER(hng64_random_read);
	DECLARE_READ32_MEMBER(hng64_com_r);
	DECLARE_WRITE32_MEMBER(hng64_com_w);
	DECLARE_WRITE8_MEMBER(hng64_com_share_w);
	DECLARE_READ8_MEMBER(hng64_com_share_r);
	DECLARE_WRITE8_MEMBER(hng64_com_share_mips_w);
	DECLARE_READ8_MEMBER(hng64_com_share_mips_r);
	DECLARE_WRITE32_MEMBER(hng64_pal_w);
	DECLARE_READ32_MEMBER(hng64_sysregs_r);
	DECLARE_WRITE32_MEMBER(hng64_sysregs_w);
	DECLARE_READ32_MEMBER(fight_io_r);
	DECLARE_READ32_MEMBER(samsho_io_r);
	DECLARE_READ32_MEMBER(shoot_io_r);
	DECLARE_READ32_MEMBER(racing_io_r);
	DECLARE_READ32_MEMBER(hng64_dualport_r);
	DECLARE_WRITE32_MEMBER(hng64_dualport_w);
	DECLARE_READ32_MEMBER(hng64_3d_1_r);
	DECLARE_READ32_MEMBER(hng64_3d_2_r);
	DECLARE_WRITE32_MEMBER(hng64_3d_1_w);
	DECLARE_WRITE32_MEMBER(hng64_3d_2_w);
	DECLARE_WRITE32_MEMBER(dl_w);
	DECLARE_READ32_MEMBER(dl_r);
	DECLARE_WRITE32_MEMBER(dl_control_w);
	DECLARE_WRITE32_MEMBER(dl_upload_w);
	DECLARE_WRITE32_MEMBER(tcram_w);
	DECLARE_READ32_MEMBER(tcram_r);
	DECLARE_READ32_MEMBER(unk_vreg_r);
	DECLARE_WRITE32_MEMBER(hng64_soundram_w);
	DECLARE_READ32_MEMBER(hng64_soundram_r);

	// not actually used, but left in code so you can turn it and see the (possibly undesired?) behavior, see notes in memory map
	DECLARE_WRITE32_MEMBER(hng64_soundram2_w);
	DECLARE_READ32_MEMBER(hng64_soundram2_r);

	DECLARE_WRITE32_MEMBER(hng64_soundcpu_enable_w);

	DECLARE_WRITE32_MEMBER(hng64_sprite_clear_even_w);
	DECLARE_WRITE32_MEMBER(hng64_sprite_clear_odd_w);
	DECLARE_WRITE32_MEMBER(trap_write);
	DECLARE_WRITE32_MEMBER(activate_3d_buffer);
	DECLARE_READ8_MEMBER(hng64_comm_shared_r);
	DECLARE_WRITE8_MEMBER(hng64_comm_shared_w);
	DECLARE_WRITE32_MEMBER(hng64_videoram_w);
	DECLARE_READ8_MEMBER(hng64_comm_space_r);
	DECLARE_WRITE8_MEMBER(hng64_comm_space_w);
	DECLARE_READ8_MEMBER(hng64_comm_mmu_r);
	DECLARE_WRITE8_MEMBER(hng64_comm_mmu_w);
	DECLARE_DRIVER_INIT(hng64_race);
	DECLARE_DRIVER_INIT(fatfurwa);
	DECLARE_DRIVER_INIT(buriki);
	DECLARE_DRIVER_INIT(hng64);
	DECLARE_DRIVER_INIT(hng64_shoot);
	DECLARE_DRIVER_INIT(ss64);
	DECLARE_DRIVER_INIT(hng64_fght);
	DECLARE_DRIVER_INIT(hng64_reorder_gfx);

	void m_set_irq(UINT32 irq_vector);
	UINT32 m_irq_pending;
	UINT8 *m_comm_rom;
	UINT8 *m_comm_ram;
	UINT8 m_mmu_regs[8];
	UINT32 m_mmua[6];
	UINT16 m_mmub[6];
	UINT8 read_comm_data(UINT32 offset);
	void write_comm_data(UINT32 offset,UINT8 data);
	int m_irq_level;
	TILE_GET_INFO_MEMBER(get_hng64_tile0_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile0_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile1_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile1_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile2_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile2_16x16_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile3_8x8_info);
	TILE_GET_INFO_MEMBER(get_hng64_tile3_16x16_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_hng64(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_hng64(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(hng64_irq);


	DECLARE_CUSTOM_INPUT_MEMBER(left_handle_r);
	DECLARE_CUSTOM_INPUT_MEMBER(right_handle_r);
	DECLARE_CUSTOM_INPUT_MEMBER(acc_down_r);
	DECLARE_CUSTOM_INPUT_MEMBER(brake_down_r);
	void clear3d();
	TIMER_CALLBACK_MEMBER(hng64_3dfifo_processed);
};

/*----------- defined in video/hng64.c -----------*/
void hng64_command3d(running_machine& machine, const UINT16* packet);
