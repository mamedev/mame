/*************************************************************************

    Art & Magic hardware

**************************************************************************/

class artmagic_state : public driver_device
{
public:
	artmagic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_control(*this, "control"),
		m_vram0(*this, "vram0"),
		m_vram1(*this, "vram1"){ }

	required_shared_ptr<UINT16> m_control;
	UINT8 m_tms_irq;
	UINT8 m_hack_irq;
	UINT8 m_prot_input[16];
	UINT8 m_prot_input_index;
	UINT8 m_prot_output[16];
	UINT8 m_prot_output_index;
	UINT8 m_prot_output_bit;
	UINT8 m_prot_bit_index;
	UINT16 m_prot_save;
	void (*m_protection_handler)(running_machine &);
	required_shared_ptr<UINT16> m_vram0;
	required_shared_ptr<UINT16> m_vram1;
	int m_xor[16];
	int m_is_stoneball;
	UINT16 *m_blitter_base;
	UINT32 m_blitter_mask;
	UINT16 m_blitter_data[8];
	UINT8 m_blitter_page;
	attotime m_blitter_busy_until;
	DECLARE_READ16_MEMBER(tms_host_r);
	DECLARE_WRITE16_MEMBER(tms_host_w);
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_READ16_MEMBER(ultennis_hack_r);
	DECLARE_WRITE16_MEMBER(protection_bit_w);
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_READ16_MEMBER(artmagic_blitter_r);
	DECLARE_WRITE16_MEMBER(artmagic_blitter_w);
	DECLARE_CUSTOM_INPUT_MEMBER(prot_r);
	DECLARE_DRIVER_INIT(shtstar);
	DECLARE_DRIVER_INIT(cheesech);
	DECLARE_DRIVER_INIT(ultennis);
	DECLARE_DRIVER_INIT(stonebal);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	TIMER_CALLBACK_MEMBER(irq_off);
};


/*----------- defined in video/artmagic.c -----------*/
void artmagic_to_shiftreg(address_space &space, offs_t address, UINT16 *data);
void artmagic_from_shiftreg(address_space &space, offs_t address, UINT16 *data);
void artmagic_scanline(screen_device &screen, bitmap_rgb32 &bitmap, int scanline, const tms34010_display_params *params);
