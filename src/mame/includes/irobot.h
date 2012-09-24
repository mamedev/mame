/*************************************************************************

    Atari I, Robot hardware

*************************************************************************/

#define IR_TIMING				1		/* try to emulate MB and VG running time */

struct irmb_ops
{
	const struct irmb_ops *nxtop;
	UINT32 func;
	UINT32 diradd;
	UINT32 latchmask;
	UINT32 *areg;
	UINT32 *breg;
	UINT8 cycles;
	UINT8 diren;
	UINT8 flags;
	UINT8 ramsel;
};


class irobot_state : public driver_device
{
public:
	irobot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8>	m_nvram;
	required_shared_ptr<UINT8> m_videoram;
	UINT8 m_vg_clear;
	UINT8 m_bufsel;
	UINT8 m_alphamap;
	UINT8 *m_combase;
	UINT8 m_irvg_vblank;
	UINT8 m_irvg_running;
	UINT8 m_irmb_running;
#if IR_TIMING
	timer_device *m_irvg_timer;
	timer_device *m_irmb_timer;
#endif
	UINT8 *m_comRAM[2];
	UINT8 *m_mbRAM;
	UINT8 *m_mbROM;
	UINT8 m_control_num;
	UINT8 m_statwr;
	UINT8 m_out0;
	UINT8 m_outx;
	UINT8 m_mpage;
	UINT8 *m_combase_mb;
	irmb_ops *m_mbops;
	const irmb_ops *m_irmb_stack[16];
	UINT32 m_irmb_regs[16];
	UINT32 m_irmb_latch;
	UINT8 *m_polybitmap1;
	UINT8 *m_polybitmap2;
	int m_ir_xmin;
	int m_ir_ymin;
	int m_ir_xmax;
	int m_ir_ymax;
	DECLARE_WRITE8_MEMBER(irobot_nvram_w);
	DECLARE_WRITE8_MEMBER(irobot_clearirq_w);
	DECLARE_WRITE8_MEMBER(irobot_clearfirq_w);
	DECLARE_READ8_MEMBER(irobot_sharedmem_r);
	DECLARE_WRITE8_MEMBER(irobot_sharedmem_w);
	DECLARE_WRITE8_MEMBER(irobot_statwr_w);
	DECLARE_WRITE8_MEMBER(irobot_out0_w);
	DECLARE_WRITE8_MEMBER(irobot_rom_banksel_w);
	DECLARE_WRITE8_MEMBER(irobot_control_w);
	DECLARE_READ8_MEMBER(irobot_control_r);
	DECLARE_READ8_MEMBER(irobot_status_r);
	DECLARE_WRITE8_MEMBER(irobot_paletteram_w);
	DECLARE_DRIVER_INIT(irobot);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_irobot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
};

/*----------- defined in machine/irobot.c -----------*/
TIMER_DEVICE_CALLBACK( irobot_irvg_done_callback );
TIMER_DEVICE_CALLBACK( irobot_irmb_done_callback );

/*----------- defined in video/irobot.c -----------*/
void irobot_poly_clear(running_machine &machine);
void irobot_run_video(running_machine &machine);
