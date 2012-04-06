

class funybubl_state : public driver_device
{
public:
	funybubl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_paletteram;

	/* devices */
	device_t *m_audiocpu;

	/* memory */
	UINT8      m_banked_vram[0x2000];
	DECLARE_WRITE8_MEMBER(funybubl_vidram_bank_w);
	DECLARE_WRITE8_MEMBER(funybubl_cpurombank_w);
	DECLARE_WRITE8_MEMBER(funybubl_soundcommand_w);
	DECLARE_WRITE8_MEMBER(funybubl_paldatawrite);
};



/*----------- defined in video/funybubl.c -----------*/


VIDEO_START(funybubl);
SCREEN_UPDATE_IND16(funybubl);
