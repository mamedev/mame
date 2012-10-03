class shangha3_state : public driver_device
{
public:
	shangha3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ram(*this, "ram"){ }

	int m_prot_count;
	required_shared_ptr<UINT16> m_ram;

	int m_do_shadows;

	UINT16 m_gfxlist_addr;
	bitmap_ind16 m_rawbitmap;

	UINT8 m_drawmode_table[16];
	DECLARE_READ16_MEMBER(shangha3_prot_r);
	DECLARE_WRITE16_MEMBER(shangha3_prot_w);
	DECLARE_WRITE16_MEMBER(shangha3_coinctrl_w);
	DECLARE_WRITE16_MEMBER(heberpop_coinctrl_w);
	DECLARE_WRITE16_MEMBER(blocken_coinctrl_w);
	DECLARE_WRITE16_MEMBER(heberpop_sound_command_w);
	DECLARE_WRITE16_MEMBER(shangha3_flipscreen_w);
	DECLARE_WRITE16_MEMBER(shangha3_gfxlist_addr_w);
	DECLARE_WRITE16_MEMBER(shangha3_blitter_go_w);
	DECLARE_WRITE16_MEMBER(shangha3_irq_ack_w);
	DECLARE_DRIVER_INIT(shangha3);
	DECLARE_DRIVER_INIT(heberpop);
	virtual void video_start();
	UINT32 screen_update_shangha3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
