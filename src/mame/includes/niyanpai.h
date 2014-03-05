#include "sound/dac.h"
#include "machine/tmp68301.h"
#define VRAM_MAX    3

class niyanpai_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER
	};

	niyanpai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_tmp68301(*this, "tmp68301"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	int m_musobana_inputport;
	int m_musobana_outcoin_flag;
	UINT8 m_pio_dir[5];
	UINT8 m_pio_latch[5];
	int m_scrollx[VRAM_MAX];
	int m_scrolly[VRAM_MAX];
	int m_blitter_destx[VRAM_MAX];
	int m_blitter_desty[VRAM_MAX];
	int m_blitter_sizex[VRAM_MAX];
	int m_blitter_sizey[VRAM_MAX];
	int m_blitter_src_addr[VRAM_MAX];
	int m_blitter_direction_x[VRAM_MAX];
	int m_blitter_direction_y[VRAM_MAX];
	int m_dispflag[VRAM_MAX];
	int m_flipscreen[VRAM_MAX];
	int m_clutmode[VRAM_MAX];
	int m_transparency[VRAM_MAX];
	int m_clutsel[VRAM_MAX];
	int m_screen_refresh;
	int m_nb19010_busyctr;
	int m_nb19010_busyflag;
	bitmap_ind16 m_tmpbitmap[VRAM_MAX];
	UINT16 *m_videoram[VRAM_MAX];
	UINT16 *m_videoworkram[VRAM_MAX];
	UINT16 *m_palette_ptr;
	UINT8 *m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
	DECLARE_READ8_MEMBER(niyanpai_sound_r);
	DECLARE_WRITE16_MEMBER(niyanpai_sound_w);
	DECLARE_WRITE8_MEMBER(niyanpai_soundclr_w);
	DECLARE_READ8_MEMBER(tmpz84c011_pio_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_pio_w);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pa_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pb_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pc_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pd_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pe_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pa_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pb_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pc_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pd_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pe_w);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pa_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pb_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pc_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pd_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pe_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pa_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pb_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pc_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pd_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pe_w);
	DECLARE_READ16_MEMBER(niyanpai_dipsw_r);
	DECLARE_READ16_MEMBER(musobana_inputport_0_r);
	DECLARE_WRITE16_MEMBER(musobana_inputport_w);
	DECLARE_READ16_MEMBER(niyanpai_palette_r);
	DECLARE_WRITE16_MEMBER(niyanpai_palette_w);
	DECLARE_WRITE16_MEMBER(niyanpai_blitter_0_w);
	DECLARE_WRITE16_MEMBER(niyanpai_blitter_1_w);
	DECLARE_WRITE16_MEMBER(niyanpai_blitter_2_w);
	DECLARE_READ16_MEMBER(niyanpai_blitter_0_r);
	DECLARE_READ16_MEMBER(niyanpai_blitter_1_r);
	DECLARE_READ16_MEMBER(niyanpai_blitter_2_r);
	DECLARE_WRITE16_MEMBER(niyanpai_clut_0_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clut_1_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clut_2_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clutsel_0_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clutsel_1_w);
	DECLARE_WRITE16_MEMBER(niyanpai_clutsel_2_w);
	DECLARE_CUSTOM_INPUT_MEMBER(musobana_outcoin_flag_r);
	DECLARE_DRIVER_INIT(niyanpai);
	DECLARE_WRITE16_MEMBER(tmp68301_parallel_port_w);
	UINT8 m_motor_on;
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_niyanpai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(niyanpai_interrupt);
	int niyanpai_blitter_r(int vram, int offset);
	void niyanpai_blitter_w(int vram, int offset, int data);
	void niyanpai_clutsel_w(int vram, int data);
	void niyanpai_clut_w(int vram, int offset, int data);
	void niyanpai_vramflip(int vram);
	void update_pixel(int vram, int x, int y);
	void niyanpai_gfxdraw(int vram);
	void niyanpai_soundbank_w(int data);
	required_device<cpu_device> m_maincpu;
	required_device<tmp68301_device> m_tmp68301;
	required_device<dac_device> m_dac1;
	required_device<dac_device> m_dac2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
