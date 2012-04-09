/*************************************************************************

    Psikyo PS6807 (PS4)

*************************************************************************/

#define MASTER_CLOCK 57272700	// main oscillator frequency


class psikyo4_state : public driver_device
{
public:
	psikyo4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT32 *       m_vidregs;
	UINT32 *       m_paletteram;
	UINT32 *       m_ram;
	UINT32 *       m_io_select;
	UINT32 *       m_bgpen_1;
	UINT32 *       m_bgpen_2;
	UINT32 *       m_spriteram;
	size_t         m_spriteram_size;

	/* video-related */
	double         m_oldbrt1;
	double         m_oldbrt2;

	/* devices */
	device_t *m_maincpu;
	DECLARE_WRITE32_MEMBER(ps4_paletteram32_RRRRRRRRGGGGGGGGBBBBBBBBxxxxxxxx_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_bgpen_1_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_bgpen_2_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_screen1_brt_w);
	DECLARE_WRITE32_MEMBER(ps4_screen2_brt_w);
	DECLARE_WRITE32_MEMBER(ps4_vidregs_w);
	DECLARE_WRITE32_MEMBER(hotgmck_pcm_bank_w);
	DECLARE_CUSTOM_INPUT_MEMBER(system_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(mahjong_ctrl_r);
};


/*----------- defined in video/psikyo4.c -----------*/

VIDEO_START( psikyo4 );
SCREEN_UPDATE_IND16( psikyo4_left );
SCREEN_UPDATE_IND16( psikyo4_right );
