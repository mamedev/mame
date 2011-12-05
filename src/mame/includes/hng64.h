#define FIGHT_MCU  1
#define SHOOT_MCU  2
#define RACING_MCU 3
#define SAMSHO_MCU 4

class hng64_state : public driver_device
{
public:
	hng64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
		{ }

	int m_mcu_type;
	UINT32 *m_mainram;
	UINT32 *m_cart;
	UINT32 *m_rombase;

	UINT32 *m_dualport;
	UINT16 *m_soundram;
	UINT32 *m_sysregs;

	/* Communications stuff */
	UINT32 *m_com_ram;
	UINT8  *m_com_op_base;
	UINT8  *m_com_mmu_mem;
	UINT8  *m_com_virtual_mem;
	UINT32 m_com_shared_a;
	UINT32 m_com_shared_b;

	INT32 m_dma_start;
	INT32 m_dma_dst;
	INT32 m_dma_len;

	UINT32 m_mcu_fake_time;
	UINT16 m_mcu_en;

	UINT32 m_activeBuffer;
	UINT32 m_no_machine_error_code;
	int m_interrupt_level_request;

	UINT32 m_unk_vreg_toggle;
	UINT32 m_p1_trig;

	/* 3D stuff */
	UINT32 *m_3d_1;
	UINT32 *m_3d_2;
	UINT32 *m_dl;
	//UINT32 *q2;

	UINT32 *m_spriteram;
	UINT32 *m_videoregs;
	UINT32 *m_spriteregs;
	UINT32 *m_videoram;
	UINT32 *m_tcram;
	UINT32 *m_3dregs;
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

	required_device<cpu_device> m_maincpu;

};


/*----------- defined in video/hng64.c -----------*/

WRITE32_HANDLER( hng64_videoram_w );
void hng64_command3d(running_machine& machine, const UINT16* packet);

VIDEO_START( hng64 );
SCREEN_UPDATE( hng64 );
SCREEN_EOF( hng64 );
