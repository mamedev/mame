// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Alex Marshall
/******************************************************************************

Jaleco MS32 System Control Unit

******************************************************************************/

#ifndef MAME_JALECO_JALECO_MS32_SYSCTRL_H
#define MAME_JALECO_JALECO_MS32_SYSCTRL_H

#pragma once

#include "screen.h"
#include "machine/timer.h"

//*****************************************************************************
//  TYPE DEFINITIONS
//*****************************************************************************

// ======================> jaleco_ms32_sysctrl_device

class jaleco_ms32_sysctrl_device : public device_t
{
public:
	// construction/destruction
	template <typename T>
	jaleco_ms32_sysctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag)
		: jaleco_ms32_sysctrl_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	jaleco_ms32_sysctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// I/O operations
	void amap(address_map &map) ATTR_COLD;
	auto flip_screen_cb() { return m_flip_screen_cb.bind(); }
	auto vblank_cb() { return m_vblank_cb.bind(); }
	auto field_cb() { return m_field_cb.bind(); }
	auto prg_timer_cb() { return m_prg_timer_cb.bind(); }
	auto sound_ack_cb() { return m_sound_ack_cb.bind(); }
	auto sound_reset_cb() { return m_sound_reset_cb.bind(); }
	void set_invert_vblank_lines(bool enable) { m_invert_vblank_lines = enable; }
//  template <typename T> void set_screen(T &&screen_tag) { m_screen.set_tag(std::forward<T>(screen_tag)); printf("xxx"); }

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(prg_timer_tick);

private:
	required_device<screen_device> m_screen;

	void control_w(u16 data);
	void hblank_w(u16 data);
	void hdisplay_w(u16 data);
	void hbp_w(u16 data);
	void hfp_w(u16 data);
	void vblank_w(u16 data);
	void vdisplay_w(u16 data);
	void vbp_w(u16 data);
	void vfp_w(u16 data);
	void timer_interval_w(u16 data);
	void sound_reset_w(u16 data);
	void vblank_ack_w(u16 data);
	void field_ack_w(u16 data);
	void timer_ack_w(u16 data);
	void sound_ack_w(u16 data);
	void irq_ack_w(u16 data);

	devcb_write_line m_flip_screen_cb;
	devcb_write_line m_vblank_cb;
	devcb_write_line m_field_cb;
	devcb_write_line m_prg_timer_cb;
	devcb_write_line m_sound_ack_cb;
	devcb_write_line m_sound_reset_cb;
	u8 m_dotclock_setting;
	inline u32 get_dotclock_frequency();
	bool m_flip_screen_state;
	struct {
		u16 horz_blank, horz_display, vert_blank, vert_display;
	}m_crtc;
	inline u16 crtc_vtotal();
	inline void crtc_refresh_screen_params();
	struct {
		bool irq_enable;
		u16 interval;
		emu_timer *prg_irq;
	}m_timer;

	emu_timer *m_timer_scanline;

	void flush_prg_timer();
	TIMER_CALLBACK_MEMBER(flush_scanline_timer);

	bool m_invert_vblank_lines;
};

// device type definition
DECLARE_DEVICE_TYPE(JALECO_MS32_SYSCTRL, jaleco_ms32_sysctrl_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_JALECO_JALECO_MS32_SYSCTRL_H
