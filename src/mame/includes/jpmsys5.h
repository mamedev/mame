// license:BSD-3-Clause
// copyright-holders:Philip Bennett, James Wallace, David Haywood

#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "sound/ym2413.h"
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
	jpmsys5_state(const machine_config &mconfig, device_type type, const char *tag) :
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

	uint8_t m_palette_val[16][3];
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
	uint8_t m_a0_data_out;
	uint8_t m_a1_data_out;
	uint8_t m_a2_data_out;
	void generate_tms34061_interrupt(int state);
	void sys5_tms34061_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sys5_tms34061_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ramdac_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rombank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t coins_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void coins_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t unk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mux_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mux_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void touchscreen_press(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void jpm_upd7759_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t jpm_upd7759_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ptm_irq(int state);
	void u26_o1_callback(int state);
	void pia_irq(int state);
	uint8_t u29_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void u29_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void u29_ca2_w(int state);
	void u29_cb2_w(int state);
	void acia_irq(int state);
	void a0_tx_w(int state);
	void a1_tx_w(int state);
	void a2_tx_w(int state);
	void write_acia_clock(int state);
	uint16_t mux_awp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t coins_awp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sys5_draw_lamps();
	void machine_start_jpmsys5v();
	void machine_reset_jpmsys5v();
	void machine_start_jpmsys5();
	void machine_reset_jpmsys5();
	uint32_t screen_update_jpmsys5v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void touch_cb(void *ptr, int32_t param);
};
