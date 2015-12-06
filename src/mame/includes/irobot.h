// license:BSD-3-Clause
// copyright-holders:Dan Boris
/*************************************************************************

    Atari I, Robot hardware

*************************************************************************/

#define IR_TIMING               1       /* try to emulate MB and VG running time */

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
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8>  m_nvram;
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
	DECLARE_READ8_MEMBER(quad_pokeyn_r);
	DECLARE_WRITE8_MEMBER(quad_pokeyn_w);
	DECLARE_DRIVER_INIT(irobot);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(irobot);
	UINT32 screen_update_irobot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(irobot_irvg_done_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(irobot_irmb_done_callback);
	void _irobot_poly_clear(UINT8 *bitmap_base);
	void irobot_poly_clear();
	void draw_line(UINT8 *polybitmap, int x1, int y1, int x2, int y2, int col);
	void irobot_run_video();
	UINT32 irmb_din(const irmb_ops *curop);
	void irmb_dout(const irmb_ops *curop, UINT32 d);
	void load_oproms();
	void irmb_run();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
