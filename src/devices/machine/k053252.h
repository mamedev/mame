// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**  Konami 053252  **/
/* CRT and interrupt control unit */
#ifndef MAME_DEVICES_MACHINE_K053252_H
#define MAME_DEVICES_MACHINE_K053252_H

#pragma once


#include "screen.h"

#define MCFG_K053252_INT1_CB(_devcb) \
	devcb = &k053252_device::set_int1_cb(*device, DEVCB_##_devcb);

#define MCFG_K053252_INT2_CB(_devcb) \
	devcb = &k053252_device::set_int2_cb(*device, DEVCB_##_devcb);

#define MCFG_K053252_VSYNC_CB(_devcb) \
	devcb = &k053252_device::set_vsync_cb(*device, DEVCB_##_devcb);

#define MCFG_K053252_VBLANK_CB(_devcb) \
	devcb = &k053252_device::set_vblank_cb(*device, DEVCB_##_devcb);

class k053252_device : public device_t,
					   public device_video_interface
{
public:
	k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_int1_cb(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_int1_cb.set_callback(std::forward<_Object>(object)); }
	template<class _Object> static devcb_base &set_int2_cb(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_int2_cb.set_callback(std::forward<_Object>(object)); }
	template<class _Object> static devcb_base &set_vsync_cb(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_vsync_cb.set_callback(std::forward<_Object>(object)); }
	template<class _Object> static devcb_base &set_vblank_cb(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_vblank_cb.set_callback(std::forward<_Object>(object)); }

	DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_WRITE8_MEMBER(hch_w);
	DECLARE_WRITE8_MEMBER(hcl_w);
	DECLARE_WRITE8_MEMBER(hfph_w);
	DECLARE_WRITE8_MEMBER(hfpl_w);
	DECLARE_WRITE8_MEMBER(hbph_w);
	DECLARE_WRITE8_MEMBER(hbpl_w);
	DECLARE_WRITE8_MEMBER(irq1_en_w);
	DECLARE_WRITE8_MEMBER(irq2_en_w);
	DECLARE_WRITE8_MEMBER(vch_w);
	DECLARE_WRITE8_MEMBER(vcl_w);
	DECLARE_WRITE8_MEMBER(vfp_w);
	DECLARE_WRITE8_MEMBER(vbp_w);
	DECLARE_WRITE8_MEMBER(sw_w);
	DECLARE_WRITE8_MEMBER(tm_w);
	DECLARE_WRITE8_MEMBER(irq1_ack_w);
	DECLARE_WRITE8_MEMBER(irq2_ack_w);

	DECLARE_READ8_MEMBER(vcth_r);
	DECLARE_READ8_MEMBER(vctl_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_clock_changed() override;

private:
	enum {
		TIMER_FRAME,
		TIMER_HTIMER,
		TIMER_SOURCE_VBLANK
	};

	enum {
		FT_WAIT_VBLANK_IN,
		FT_WAIT_VSYNC_IN,
		FT_WAIT_VSYNC_OUT,
		FT_WAIT_VBLANK_OUT
	};

	enum {
		HT_WAIT_HBLANK_IN,
		HT_WAIT_HSYNC_IN
	};

	devcb_write_line m_int1_cb;
	devcb_write_line m_int2_cb;
	devcb_write_line m_vblank_cb;
	devcb_write_line m_vsync_cb;

	uint16_t m_vct;
	uint16_t m_hc, m_hfp, m_hbp, m_vc;
	uint8_t m_vfp, m_vbp, m_sw, m_tm;

	bool m_int1_on, m_int1_en, m_int2_on, m_int2_en;

	attotime m_line_duration, m_frame, m_vblank_in_to_vsync_in, m_vsync_in_to_vsync_out;
	attotime m_vsync_out_to_vblank_out, m_vsync_in_to_hblank_in, m_hblank_in_to_hsync_in, m_hsync_in_to_hblank_in;
	int m_timer_frame_state;
	int m_timer_htimer_state;

	emu_timer *m_timer_frame, *m_timer_htimer, *m_timer_source_vblank;

	void frame_transition();
	void htimer_transition();
	void screen_vblank(screen_device &scr, bool state);
	void update_screen();
};

extern const device_type K053252;


#endif  // MAME_DEVICES_MACHINE_K053252_H
