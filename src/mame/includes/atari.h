/******************************************************************************

    Atari 400/800

    ANTIC video controller
    GTIA  graphics television interface adapter

    Juergen Buchmueller, June 1998

******************************************************************************/

#ifndef ATARI_H
#define ATARI_H

#include "machine/6821pia.h"
#include "sound/pokey.h"
#include "video/antic.h"
#include "video/gtia.h"


class atari_common_state : public driver_device
{
public:
	atari_common_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gtia(*this, "gtia"),
		tv_artifacts(0) { }

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual void video_start();
	UINT32 screen_update_atari(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER( a400_interrupt );
	TIMER_DEVICE_CALLBACK_MEMBER( a800_interrupt );
	TIMER_DEVICE_CALLBACK_MEMBER( a800xl_interrupt );
	TIMER_DEVICE_CALLBACK_MEMBER( a5200_interrupt );

	DECLARE_PALETTE_INIT(atari);

	DECLARE_READ8_MEMBER ( atari_antic_r );
	DECLARE_WRITE8_MEMBER ( atari_antic_w );

	POKEY_INTERRUPT_CB_MEMBER(interrupt_cb);
	POKEY_KEYBOARD_CB_MEMBER(a5200_keypads);
	POKEY_KEYBOARD_CB_MEMBER(a800_keyboard);

private:
	static const device_timer_id TIMER_CYCLE_STEAL = 0;
	static const device_timer_id TIMER_ISSUE_DLI = 1;
	static const device_timer_id TIMER_LINE_REND = 2;
	static const device_timer_id TIMER_LINE_DONE = 3;

	required_device<gtia_device> m_gtia;
	UINT32 tv_artifacts;
	void prio_init();
	void cclk_init();
	void artifacts_gfx(UINT8 *src, UINT8 *dst, int width);
	void artifacts_txt(UINT8 * src, UINT8 * dst, int width);
	void antic_linerefresh();
	int cycle();
	void after(int cycles, timer_expired_delegate function);
	TIMER_CALLBACK_MEMBER( antic_issue_dli );
	TIMER_CALLBACK_MEMBER( antic_line_done );
	TIMER_CALLBACK_MEMBER( antic_steal_cycles );
	TIMER_CALLBACK_MEMBER( antic_scanline_render );
	inline void LMS(int new_cmd);
	void antic_scanline_dma(int param);
	void generic_atari_interrupt(int button_count);
	
	int m_antic_render1, m_antic_render2, m_antic_render3;
};

/* video */

#define CYCLES_PER_LINE 114     /* total number of cpu cycles per scanline (incl. hblank) */
#define CYCLES_REFRESH  9       /* number of cycles lost for ANTICs RAM refresh using DMA */
#define CYCLES_HSTART   32      /* where does the ANTIC DMA fetch start */
#define CYCLES_DLI_NMI  7       /* number of cycles until the CPU recognizes a DLI */
#define CYCLES_HSYNC    104     /* where does the HSYNC position of a scanline start */

#define VBL_END         8       /* vblank ends in this scanline */
#define VDATA_START     11      /* video display begins in this scanline */
#define VDATA_END       244     /* video display ends in this scanline */
#define VBL_START       248     /* vblank starts in this scanline */

/* total number of lines per frame (incl. vblank) */
#define TOTAL_LINES_60HZ 262
#define TOTAL_LINES_50HZ 312

/* frame rates */
#define FRAME_RATE_50HZ (double)1789790/114/TOTAL_LINES_50HZ
#define FRAME_RATE_60HZ (double)1789790/114/TOTAL_LINES_60HZ


#endif /* ATARI_H */
