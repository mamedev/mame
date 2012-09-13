

class funybubl_state : public driver_device
{
public:
	funybubl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_paletteram(*this, "paletteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;

	/* devices */
	cpu_device *m_audiocpu;

	/* memory */
	UINT8      m_banked_vram[0x2000];
	DECLARE_WRITE8_MEMBER(funybubl_vidram_bank_w);
	DECLARE_WRITE8_MEMBER(funybubl_cpurombank_w);
	DECLARE_WRITE8_MEMBER(funybubl_soundcommand_w);
	DECLARE_WRITE8_MEMBER(funybubl_paldatawrite);
	DECLARE_WRITE8_MEMBER(funybubl_oki_bank_sw);
	virtual void machine_start();
	virtual void video_start();
};



/*----------- defined in video/funybubl.c -----------*/



SCREEN_UPDATE_IND16(funybubl);
