/*************************************************************************

    Psikyo Games

*************************************************************************/

class psikyo_state : public driver_device
{
public:
	psikyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT32 *       m_vram_0;
	UINT32 *       m_vram_1;
	UINT32 *       m_vregs;
	UINT32 *       m_spritebuf1;
	UINT32 *       m_spritebuf2;
	UINT32 *       m_bootleg_spritebuffer;
//      UINT32 *       m_paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t        *m_tilemap_0_size0;
	tilemap_t        *m_tilemap_0_size1;
	tilemap_t        *m_tilemap_0_size2;
	tilemap_t        *m_tilemap_0_size3;
	tilemap_t        *m_tilemap_1_size0;
	tilemap_t        *m_tilemap_1_size1;
	tilemap_t        *m_tilemap_1_size2;
	tilemap_t        *m_tilemap_1_size3;
	int            m_tilemap_0_bank;
	int            m_tilemap_1_bank;
	int            m_ka302c_banking;
	UINT32 *m_spriteram;
	size_t m_spriteram_size;

	/* misc */
	UINT8          m_soundlatch;
	int            m_z80_nmi;
	int            m_mcu_status;

	/* devices */
	device_t *m_audiocpu;

	/* game-specific */
	// 1945 MCU
	UINT8          m_s1945_mcu_direction;
	UINT8          m_s1945_mcu_latch1;
	UINT8          m_s1945_mcu_latch2;
	UINT8          m_s1945_mcu_inlatch;
	UINT8          m_s1945_mcu_index;
	UINT8          m_s1945_mcu_latching;
	UINT8          m_s1945_mcu_mode;
	UINT8          m_s1945_mcu_control;
	UINT8          m_s1945_mcu_bctrl;
	const UINT8    *m_s1945_mcu_table;
	DECLARE_READ32_MEMBER(sngkace_input_r);
	DECLARE_READ32_MEMBER(gunbird_input_r);
	DECLARE_WRITE32_MEMBER(psikyo_soundlatch_w);
	DECLARE_WRITE32_MEMBER(s1945_soundlatch_w);
	DECLARE_WRITE32_MEMBER(s1945_mcu_w);
	DECLARE_READ32_MEMBER(s1945_mcu_r);
	DECLARE_READ32_MEMBER(s1945_input_r);
	DECLARE_WRITE32_MEMBER(paletteram32_xRRRRRGGGGGBBBBB_dword_w);
	DECLARE_READ32_MEMBER(s1945bl_oki_r);
	DECLARE_WRITE32_MEMBER(s1945bl_oki_w);
	DECLARE_READ8_MEMBER(psikyo_soundlatch_r);
	DECLARE_WRITE8_MEMBER(psikyo_clear_nmi_w);
	DECLARE_WRITE8_MEMBER(sngkace_sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(gunbird_sound_bankswitch_w);
	DECLARE_WRITE32_MEMBER(psikyo_vram_0_w);
	DECLARE_WRITE32_MEMBER(psikyo_vram_1_w);
};


/*----------- defined in video/psikyo.c -----------*/

void psikyo_switch_banks(running_machine &machine, int tmap, int bank);


VIDEO_START( sngkace );
VIDEO_START( psikyo );
SCREEN_UPDATE_IND16( psikyo );
SCREEN_UPDATE_IND16( psikyo_bootleg );
SCREEN_VBLANK( psikyo );
