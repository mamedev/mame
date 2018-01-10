// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Generic binary ripple counter emulation

**********************************************************************/

#ifndef MAME_MACHINE_RIPPLE_COUNTER_H
#define MAME_MACHINE_RIPPLE_COUNTER_H

#pragma once

//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RIPPLE_COUNTER_STAGES(_stages) \
	ripple_counter_device::static_set_stages(*device, _stages);

// output callbacks
#define MCFG_RIPPLE_COUNTER_COUNT_OUT_CB(_devcb) \
	devcb = &ripple_counter_device::static_set_count_out_cb(*device, DEVCB_##_devcb);
#define MCFG_RIPPLE_COUNTER_ROM_OUT_CB(_devcb) \
	devcb = &ripple_counter_device::static_set_rom_out_cb(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ripple_counter_device

class ripple_counter_device : public device_t, public device_rom_interface
{
public:
	// construction/destruction
	ripple_counter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// static configuration
	static void static_set_stages(device_t &device, u8 stages);
	template<class Object> static devcb_base &static_set_count_out_cb(device_t &device, Object &&cb)
	{ return downcast<ripple_counter_device &>(device).m_count_out_cb.set_callback(std::forward<Object>(cb)); }
	template<class Object> static devcb_base &static_set_rom_out_cb(device_t &device, Object &&cb)
	{ return downcast<ripple_counter_device &>(device).m_rom_out_cb.set_callback(std::forward<Object>(cb)); }

	// control line handlers
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(reset_w);

	// getters
	u32 count() const { return m_count; }

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_rom_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual void rom_bank_updated() override;

private:
	// internal helpers
	void set_count(u32 count);

	// device callbacks
	devcb_write32 m_count_out_cb;
	devcb_write8 m_rom_out_cb;

	// device timers
	enum
	{
		TIMER_COUNT
	};
	emu_timer *m_count_timer;

	// configuration parameters
	u32 m_count_mask;

	// running state
	u32 m_count;
	bool m_clk;
	bool m_reset;
};

// device type definition
DECLARE_DEVICE_TYPE(RIPPLE_COUNTER, ripple_counter_device)

#endif // MAME_MACHINE_RIPPLE_COUNTER_H
