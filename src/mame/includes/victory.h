// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Victory system

****************************************************************************/


#define VICTORY_MAIN_CPU_CLOCK      (XTAL_8MHz / 2)

#define VICTORY_PIXEL_CLOCK             (XTAL_11_289MHz / 2)
#define VICTORY_HTOTAL                  (0x150)
#define VICTORY_HBEND                       (0x000)
#define VICTORY_HBSTART                 (0x100)
#define VICTORY_VTOTAL                  (0x118)
#define VICTORY_VBEND                       (0x000)
#define VICTORY_VBSTART                 (0x100)


/* microcode state */
struct micro_t
{
	UINT16      i;
	UINT16      pc;
	UINT8       r,g,b;
	UINT8       x,xp,y,yp;
	UINT8       cmd,cmdlo;
	emu_timer * timer;
	UINT8       timer_active;
	attotime    endtime;
};

class victory_state : public driver_device
{
public:
	victory_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_videoram(*this, "videoram"),
			m_charram(*this, "charram") ,
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_charram;
	UINT16 m_paletteram[0x40];
	UINT8 *m_bgbitmap;
	UINT8 *m_fgbitmap;
	UINT8 *m_rram;
	UINT8 *m_gram;
	UINT8 *m_bram;
	UINT8 m_vblank_irq;
	UINT8 m_fgcoll;
	UINT8 m_fgcollx;
	UINT8 m_fgcolly;
	UINT8 m_bgcoll;
	UINT8 m_bgcollx;
	UINT8 m_bgcolly;
	UINT8 m_scrollx;
	UINT8 m_scrolly;
	UINT8 m_video_control;
	struct micro_t m_micro;
	DECLARE_WRITE8_MEMBER(lamp_control_w);
	DECLARE_WRITE8_MEMBER(victory_paletteram_w);
	DECLARE_READ8_MEMBER(victory_video_control_r);
	DECLARE_WRITE8_MEMBER(victory_video_control_w);
	virtual void video_start();
	UINT32 screen_update_victory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(victory_vblank_interrupt);
	TIMER_CALLBACK_MEMBER(bgcoll_irq_callback);
	void victory_update_irq();
	void set_palette();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	void update_background();
	void update_foreground();
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
