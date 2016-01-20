// license:BSD-3-Clause
// copyright-holders:Philip Bennett, James Wallace, David Haywood

#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "sound/2413intf.h"
#include "sound/upd7759.h"
#include "video/tms34061.h"
#include "machine/nvram.h"
#include "video/awpvid.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"

class jpmsys5_state : public driver_device
{
public:
	jpmsys5_state(const machine_config &mconfig, device_type type, std::string tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_acia6850_0(*this, "acia6850_0"),
		m_acia6850_1(*this, "acia6850_1"),
		m_acia6850_2(*this, "acia6850_2"),
		m_upd7759(*this, "upd7759"),
		m_tms34061(*this, "tms34061"),
		m_vfd(*this, "vfd"),
		m_direct_port(*this, "DIRECT"),
		m_palette(*this, "palette"),
		m_meters(*this, "meters") { }

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia6850_0;
	required_device<acia6850_device> m_acia6850_1;
	required_device<acia6850_device> m_acia6850_2;
	required_device<upd7759_device> m_upd7759;
	optional_device<tms34061_device> m_tms34061;
	optional_device<s16lf01_t> m_vfd;
	required_ioport m_direct_port;
	optional_device<palette_device> m_palette;
	optional_device<meters_device> m_meters; //jpmsys5v doesn't use this

	UINT8 m_palette_val[16][3];
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
	int m_chop;
	UINT8 m_a0_data_out;
	UINT8 m_a1_data_out;
	UINT8 m_a2_data_out;
	DECLARE_WRITE_LINE_MEMBER(generate_tms34061_interrupt);
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
	DECLARE_WRITE_LINE_MEMBER(pia_irq);
	DECLARE_READ8_MEMBER(u29_porta_r);
	DECLARE_WRITE8_MEMBER(u29_portb_w);
	DECLARE_WRITE_LINE_MEMBER(u29_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u29_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(acia_irq);
	DECLARE_WRITE_LINE_MEMBER(a0_tx_w);
	DECLARE_WRITE_LINE_MEMBER(a1_tx_w);
	DECLARE_WRITE_LINE_MEMBER(a2_tx_w);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_READ16_MEMBER(mux_awp_r);
	DECLARE_READ16_MEMBER(coins_awp_r);
	void sys5_draw_lamps();
	DECLARE_MACHINE_START(jpmsys5v);
	DECLARE_MACHINE_RESET(jpmsys5v);
	DECLARE_MACHINE_START(jpmsys5);
	DECLARE_MACHINE_RESET(jpmsys5);
	UINT32 screen_update_jpmsys5v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(touch_cb);
};
