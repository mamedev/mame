
#include "cpu/m68000/m68000.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "sound/2413intf.h"
#include "sound/saa1099.h"
#include "sound/upd7759.h"
#include "video/tms34061.h"
#include "machine/nvram.h"
#include "video/awpvid.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"

class jpmsys5_state : public driver_device
{
public:
	jpmsys5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_vfd(*this, "vfd")
		{ }

	UINT8 m_palette[16][3];
	int m_pal_addr;
	int m_pal_idx;
	int m_touch_state;
	emu_timer *m_touch_timer;
	int m_touch_data_count;
	int m_touch_data[3];
	int m_touch_shift_cnt;
	int m_lamp_strobe;
	int m_mpxclk;
	int m_muxram[255];
	int m_alpha_clock;
	optional_device<roc10937_t> m_vfd;
	UINT8 m_a0_acia_dcd;
	UINT8 m_a0_data_out;
	UINT8 m_a0_data_in;
	UINT8 m_a1_acia_dcd;
	UINT8 m_a1_data_out;
	UINT8 m_a1_data_in;
	UINT8 m_a2_acia_dcd;
	UINT8 m_a2_data_out;
	UINT8 m_a2_data_in;
	DECLARE_WRITE16_MEMBER(sys5_tms34061_w);
	DECLARE_READ16_MEMBER(sys5_tms34061_r);
	DECLARE_WRITE16_MEMBER(ramdac_w);
	DECLARE_WRITE16_MEMBER(rombank_w);
	DECLARE_READ16_MEMBER(coins_r);
	DECLARE_WRITE16_MEMBER(coins_w);
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_WRITE16_MEMBER(mux_w);
	DECLARE_READ16_MEMBER(mux_r);
	DECLARE_INPUT_CHANGED_MEMBER(touchscreen_press);
	DECLARE_WRITE16_MEMBER(jpm_upd7759_w);
	DECLARE_READ16_MEMBER(jpm_upd7759_r);
	DECLARE_WRITE_LINE_MEMBER(ptm_irq);
	DECLARE_WRITE8_MEMBER(u26_o1_callback);
	DECLARE_WRITE_LINE_MEMBER(acia_irq);
	DECLARE_READ_LINE_MEMBER(a0_rx_r);
	DECLARE_WRITE_LINE_MEMBER(a0_tx_w);
	DECLARE_READ_LINE_MEMBER(a0_dcd_r);
	DECLARE_READ_LINE_MEMBER(a1_rx_r);
	DECLARE_WRITE_LINE_MEMBER(a1_tx_w);
	DECLARE_READ_LINE_MEMBER(a1_dcd_r);
	DECLARE_READ_LINE_MEMBER(a2_rx_r);
	DECLARE_WRITE_LINE_MEMBER(a2_tx_w);
	DECLARE_READ_LINE_MEMBER(a2_dcd_r);
	DECLARE_READ16_MEMBER(mux_awp_r);
	DECLARE_READ16_MEMBER(coins_awp_r);
	void sys5_draw_lamps();
	DECLARE_MACHINE_START(jpmsys5v);
	DECLARE_MACHINE_RESET(jpmsys5v);
	DECLARE_VIDEO_START(jpmsys5v);
	DECLARE_MACHINE_START(jpmsys5);
	DECLARE_MACHINE_RESET(jpmsys5);
	UINT32 screen_update_jpmsys5v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(touch_cb);
};
