/*************************************************************************

    Psikyo PS6807 (PS4)

*************************************************************************/

#define MASTER_CLOCK 57272700	// main oscillator frequency


class psikyo4_state : public driver_device
{
public:
	psikyo4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_vidregs(*this, "vidregs"),
		m_bgpen_1(*this, "bgpen_1"),
		m_bgpen_2(*this, "bgpen_2"),
		m_paletteram(*this, "paletteram"),
		m_io_select(*this, "io_select"),
		m_ram(*this, "ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_vidregs;
	required_shared_ptr<UINT32> m_bgpen_1;
	required_shared_ptr<UINT32> m_bgpen_2;
	required_shared_ptr<UINT32> m_paletteram;
	required_shared_ptr<UINT32> m_io_select;
	required_shared_ptr<UINT32> m_ram;

	/* video-related */
	double         m_oldbrt1;
	double         m_oldbrt2;

	/* devices */
	cpu_device *m_maincpu;
	DECLARE_WRITE32_MEMBER(ps4_paletteram32_RRRRRRRRGGGGGGGGBBBBBBBBxxxxxxxx_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_bgpen_1_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_bgpen_2_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_screen1_brt_w);
	DECLARE_WRITE32_MEMBER(ps4_screen2_brt_w);
	DECLARE_WRITE32_MEMBER(ps4_vidregs_w);
	DECLARE_WRITE32_MEMBER(hotgmck_pcm_bank_w);
	DECLARE_CUSTOM_INPUT_MEMBER(system_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(mahjong_ctrl_r);
	DECLARE_WRITE32_MEMBER(ps4_eeprom_w);
	DECLARE_READ32_MEMBER(ps4_eeprom_r);
	DECLARE_DRIVER_INIT(hotgmck);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_psikyo4_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_psikyo4_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/psikyo4.c -----------*/




