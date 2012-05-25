#define NVRAM_UNLOCK_SEQ_LEN 10

class coolpool_state : public driver_device
{
public:
	coolpool_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram_base(*this, "vram_base"),
		m_nvram(*this, "nvram"){ }

	required_shared_ptr<UINT16> m_vram_base;
	required_shared_ptr<UINT16> m_nvram;

	UINT8 m_cmd_pending;
	UINT16 m_iop_cmd;
	UINT16 m_iop_answer;
	int m_iop_romaddr;

	UINT8 m_newx[3];
	UINT8 m_newy[3];
	UINT8 m_oldx[3];
	UINT8 m_oldy[3];
	int m_dx[3];
	int m_dy[3];

	UINT16 m_result;
	UINT16 m_lastresult;

	device_t *m_maincpu;
	device_t *m_dsp;
	UINT16 m_nvram_write_seq[NVRAM_UNLOCK_SEQ_LEN];
	UINT8 m_nvram_write_enable;
	UINT8 m_old_cmd;
	UINT8 m_same_cmd_count;
	DECLARE_WRITE16_MEMBER(nvram_thrash_w);
	DECLARE_WRITE16_MEMBER(nvram_data_w);
	DECLARE_WRITE16_MEMBER(nvram_thrash_data_w);
	DECLARE_WRITE16_MEMBER(amerdart_misc_w);
	DECLARE_READ16_MEMBER(amerdart_dsp_bio_line_r);
	DECLARE_READ16_MEMBER(amerdart_iop_r);
	DECLARE_WRITE16_MEMBER(amerdart_iop_w);
	DECLARE_READ16_MEMBER(amerdart_dsp_cmd_r);
	DECLARE_WRITE16_MEMBER(amerdart_dsp_answer_w);
	DECLARE_READ16_MEMBER(amerdart_trackball_r);
	DECLARE_WRITE16_MEMBER(coolpool_misc_w);
	DECLARE_WRITE16_MEMBER(coolpool_iop_w);
	DECLARE_READ16_MEMBER(coolpool_iop_r);
	DECLARE_READ16_MEMBER(dsp_cmd_r);
	DECLARE_WRITE16_MEMBER(dsp_answer_w);
	DECLARE_READ16_MEMBER(dsp_bio_line_r);
	DECLARE_READ16_MEMBER(dsp_hold_line_r);
	DECLARE_READ16_MEMBER(dsp_rom_r);
	DECLARE_WRITE16_MEMBER(dsp_romaddr_w);
	DECLARE_READ16_MEMBER(coolpool_input_r);
	DECLARE_WRITE16_MEMBER(dsp_dac_w);
};
