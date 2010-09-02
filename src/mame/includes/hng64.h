#define FIGHT_MCU  1
#define SHOOT_MCU  2
#define RACING_MCU 3
#define SAMSHO_MCU 4

class hng64_state : public driver_device
{
public:
	hng64_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int mcu_type;
	UINT32 *mainram;
	UINT32 *cart;
	UINT32 *rombase;

	UINT32 *dualport;
	UINT16 *soundram;
	UINT32 *sysregs;

	/* Communications stuff */
	UINT32 *com_ram;
	UINT8  *com_op_base;
	UINT8  *com_mmu_mem;
	UINT8  *com_virtual_mem;
	UINT32 com_shared_a;
	UINT32 com_shared_b;

	INT32 dma_start;
	INT32 dma_dst;
	INT32 dma_len;

	UINT32 mcu_fake_time;
	UINT16 mcu_en;

	UINT32 activeBuffer;
	UINT32 no_machine_error_code;
	int interrupt_level_request;

	UINT32 unk_vreg_toggle;
	UINT32 p1_trig;

	/* 3D stuff */
	UINT32 *_3d_1;
	UINT32 *_3d_2;
	UINT32 *dl;
	//UINT32 *q2;

	UINT32 *spriteram;
	UINT32 *videoregs;
	UINT32 *spriteregs;
	UINT32 *videoram;
	UINT32 *tcram;
	UINT32 *_3dregs;
	UINT8 screen_dis;

	tilemap_t *tilemap0_8x8;
	tilemap_t *tilemap1_8x8;
	tilemap_t *tilemap2_8x8;
	tilemap_t *tilemap3_8x8;

	tilemap_t *tilemap0_16x16;
	tilemap_t *tilemap1_16x16;
	tilemap_t *tilemap2_16x16;
	tilemap_t *tilemap3_16x16;

	tilemap_t *tilemap0_16x16_alt;
	tilemap_t *tilemap1_16x16_alt;
	tilemap_t *tilemap2_16x16_alt;
	tilemap_t *tilemap3_16x16_alt;

	UINT8 additive_tilemap_debug;

	// 3d display buffers
	// (Temporarily global - someday they will live with the proper bit-depth in the memory map)
	float *depthBuffer3d;
	UINT32 *colorBuffer3d;

	UINT32 old_animmask;
	UINT32 old_animbits;
	UINT16 old_tileflags0;
	UINT16 old_tileflags1;
	UINT16 old_tileflags2;
	UINT16 old_tileflags3;

	UINT32 dls[2][0x81];

	// 3d State
	int paletteState3d;
	float projectionMatrix[16];
	float modelViewMatrix[16];
	float cameraMatrix[16];

	float lightStrength;
	float lightVector[3];

};


/*----------- defined in video/hng64.c -----------*/

WRITE32_HANDLER( hng64_videoram_w );
void hng64_command3d(running_machine* machine, const UINT16* packet);

VIDEO_START( hng64 );
VIDEO_UPDATE( hng64 );
