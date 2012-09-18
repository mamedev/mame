/*************************************************************************

    P&P Marketing Police Trainer hardware

**************************************************************************/

class policetr_state : public driver_device
{
public:
	policetr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_rambase(*this, "rambase"){ }

	UINT32 m_control_data;
	UINT32 m_bsmt_data_bank;
	UINT32 m_bsmt_data_offset;
	UINT32 *m_speedup_data;
	UINT64 m_last_cycles;
	UINT32 m_loop_count;
	offs_t m_speedup_pc;
	required_shared_ptr<UINT32> m_rambase;
	UINT32 m_palette_offset;
	UINT8 m_palette_index;
	UINT8 m_palette_data[3];
	rectangle m_render_clip;
	UINT8 *m_srcbitmap;
	UINT8 *m_dstbitmap;
	UINT16 m_src_xoffs;
	UINT16 m_src_yoffs;
	UINT16 m_dst_xoffs;
	UINT16 m_dst_yoffs;
	UINT8 m_video_latch;
	UINT32 m_srcbitmap_height_mask;
	DECLARE_WRITE32_MEMBER(control_w);
	DECLARE_WRITE32_MEMBER(policetr_bsmt2000_reg_w);
	DECLARE_WRITE32_MEMBER(policetr_bsmt2000_data_w);
	DECLARE_READ32_MEMBER(bsmt2000_data_r);
	DECLARE_WRITE32_MEMBER(speedup_w);
	DECLARE_WRITE32_MEMBER(policetr_video_w);
	DECLARE_READ32_MEMBER(policetr_video_r);
	DECLARE_WRITE32_MEMBER(policetr_palette_offset_w);
	DECLARE_WRITE32_MEMBER(policetr_palette_data_w);
	DECLARE_CUSTOM_INPUT_MEMBER(bsmt_status_r);
	DECLARE_DRIVER_INIT(sshoot12);
	DECLARE_DRIVER_INIT(policetr);
	DECLARE_DRIVER_INIT(sshooter);
	DECLARE_DRIVER_INIT(plctr13b);
	virtual void video_start();
	UINT32 screen_update_policetr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(irq4_gen);
};
