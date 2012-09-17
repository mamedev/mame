class djmain_state : public driver_device
{
public:
	djmain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_obj_ram(*this, "obj_ram"){ }

	int m_sndram_bank;
	UINT8 *m_sndram;
	int m_turntable_select;
	UINT8 m_turntable_last_pos[2];
	UINT16 m_turntable_pos[2];
	UINT8 m_pending_vb_int;
	UINT16 m_v_ctrl;
	UINT32 m_obj_regs[0xa0/4];
	const UINT8 *m_ide_user_password;
	const UINT8 *m_ide_master_password;
	required_shared_ptr<UINT32> m_obj_ram;
	DECLARE_WRITE32_MEMBER(paletteram32_w);
	DECLARE_WRITE32_MEMBER(sndram_bank_w);
	DECLARE_READ32_MEMBER(sndram_r);
	DECLARE_WRITE32_MEMBER(sndram_w);
	DECLARE_READ32_MEMBER(obj_ctrl_r);
	DECLARE_WRITE32_MEMBER(obj_ctrl_w);
	DECLARE_READ32_MEMBER(obj_rom_r);
	DECLARE_WRITE32_MEMBER(v_ctrl_w);
	DECLARE_READ32_MEMBER(v_rom_r);
	DECLARE_READ8_MEMBER(inp1_r);
	DECLARE_READ8_MEMBER(inp2_r);
	DECLARE_READ32_MEMBER(turntable_r);
	DECLARE_WRITE32_MEMBER(turntable_select_w);
	DECLARE_WRITE32_MEMBER(light_ctrl_1_w);
	DECLARE_WRITE32_MEMBER(light_ctrl_2_w);
	DECLARE_WRITE32_MEMBER(unknown590000_w);
	DECLARE_WRITE32_MEMBER(unknown802000_w);
	DECLARE_WRITE32_MEMBER(unknownc02000_w);
	DECLARE_READ32_MEMBER(ide_std_r);
	DECLARE_WRITE32_MEMBER(ide_std_w);
	DECLARE_READ32_MEMBER(ide_alt_r);
	DECLARE_WRITE32_MEMBER(ide_alt_w);
	DECLARE_DRIVER_INIT(bm7thmix);
	DECLARE_DRIVER_INIT(bm6thmix);
	DECLARE_DRIVER_INIT(hmcompmx);
	DECLARE_DRIVER_INIT(bmfinal);
	DECLARE_DRIVER_INIT(hmcompm2);
	DECLARE_DRIVER_INIT(bm5thmix);
	DECLARE_DRIVER_INIT(bm4thmix);
	DECLARE_DRIVER_INIT(beatmania);
	DECLARE_DRIVER_INIT(bmdct);
	DECLARE_DRIVER_INIT(bmcompm2);
	DECLARE_DRIVER_INIT(bmcorerm);
	DECLARE_DRIVER_INIT(bmclubmx);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_djmain(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/djmain.c -----------*/




void djmain_tile_callback(running_machine& machine, int layer, int *code, int *color, int *flags);
