// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Jaleco MS32 System Control Unit

***************************************************************************/

#ifndef MAME_MACHINE_JALECO_MS32_SYSCTRL_H
#define MAME_MACHINE_JALECO_MS32_SYSCTRL_H

#pragma once

#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jaleco_ms32_sysctrl_device

class jaleco_ms32_sysctrl_device : public device_t,
								   //public device_memory_interface,
								   public device_video_interface
{
public:
	// construction/destruction
	jaleco_ms32_sysctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
//	void write(offs_t offset, u16 data, u16 mem_mask = ~0);
//	u16 read(offs_t offset, u16 mem_mask = ~0);
	void amap(address_map &map);
	auto flip_screen_cb() { return m_flip_screen_cb.bind(); }
	auto vblank_cb() { return m_vblank_cb.bind(); }
	auto field_cb() { return m_field_cb.bind(); }
	auto prg_timer_cb() { return m_prg_timer_cb.bind(); }
	auto sound_reset_cb() { return m_sound_reset_cb.bind(); }

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
//	virtual space_config_vector memory_space_config() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

//	const address_space_config m_space_config;
//	void io_map(address_map &map);
	
private:
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

	devcb_write_line m_flip_screen_cb;
	devcb_write_line m_vblank_cb;
	devcb_write_line m_field_cb;
	devcb_write_line m_prg_timer_cb;
	devcb_write_line m_sound_reset_cb;
	u8 m_dotclock_setting;
	inline u32 get_dotclock_frequency();
	bool m_flip_screen_state;
	inline u16 clamp_to_12bits_neg(u16 raw_data);
	struct {
		u16 horz_blank, horz_display, vert_blank, vert_display;
	}m_crtc;
	inline void crtc_refresh_screen_params();
	struct {
		bool irq_enable;
		u16 interval;
		emu_timer *prg_irq;
	}m_timer;
	
	emu_timer *m_vblank_timer;
	emu_timer *m_field_timer;
	enum timer_id
	{
		VBLANK_TIMER,
		FIELD_TIMER,
		PRG_TIMER
	};

	inline void flush_prg_timer();
	inline void flush_vblank_timer();
	inline void flush_field_timer();
};


// device type definition
DECLARE_DEVICE_TYPE(JALECO_MS32_SYSCTRL, jaleco_ms32_sysctrl_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_JALECO_MS32_SYSCTRL_H
