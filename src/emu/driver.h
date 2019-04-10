// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    driver.h

    Core driver device base class.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DRIVER_H
#define MAME_EMU_DRIVER_H


//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

// core machine callbacks
#define MCFG_MACHINE_START_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_START, driver_callback_delegate(&_class::MACHINE_START_NAME(_func), this));

#define MCFG_MACHINE_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_RESET, driver_callback_delegate(&_class::MACHINE_RESET_NAME(_func), this));

#define MCFG_MACHINE_RESET_REMOVE() \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_RESET, driver_callback_delegate());

// core sound callbacks
#define MCFG_SOUND_START_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_SOUND_START, driver_callback_delegate(&_class::SOUND_START_NAME(_func), this));

#define MCFG_SOUND_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_SOUND_RESET, driver_callback_delegate(&_class::SOUND_RESET_NAME(_func), this));


// core video callbacks
#define MCFG_VIDEO_START_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_VIDEO_START, driver_callback_delegate(&_class::VIDEO_START_NAME(_func), this));

#define MCFG_VIDEO_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_VIDEO_RESET, driver_callback_delegate(&_class::VIDEO_RESET_NAME(_func), this));



//**************************************************************************
//  OTHER MACROS
//**************************************************************************

#define MACHINE_START_NAME(name)    machine_start_##name
#define MACHINE_START_CALL_MEMBER(name) MACHINE_START_NAME(name)()
#define DECLARE_MACHINE_START(name) void MACHINE_START_NAME(name)() ATTR_COLD
#define MACHINE_START_MEMBER(cls,name) void cls::MACHINE_START_NAME(name)()

#define MACHINE_RESET_NAME(name)    machine_reset_##name
#define MACHINE_RESET_CALL_MEMBER(name) MACHINE_RESET_NAME(name)()
#define DECLARE_MACHINE_RESET(name) void MACHINE_RESET_NAME(name)()
#define MACHINE_RESET_MEMBER(cls,name) void cls::MACHINE_RESET_NAME(name)()

#define SOUND_START_NAME(name)      sound_start_##name
#define DECLARE_SOUND_START(name)   void SOUND_START_NAME(name)() ATTR_COLD
#define SOUND_START_MEMBER(cls,name) void cls::SOUND_START_NAME(name)()

#define SOUND_RESET_NAME(name)      sound_reset_##name
#define SOUND_RESET_CALL_MEMBER(name) SOUND_RESET_NAME(name)()
#define DECLARE_SOUND_RESET(name)   void SOUND_RESET_NAME(name)()
#define SOUND_RESET_MEMBER(cls,name) void cls::SOUND_RESET_NAME(name)()

#define VIDEO_START_NAME(name)      video_start_##name
#define VIDEO_START_CALL_MEMBER(name)       VIDEO_START_NAME(name)()
#define DECLARE_VIDEO_START(name)   void VIDEO_START_NAME(name)() ATTR_COLD
#define VIDEO_START_MEMBER(cls,name) void cls::VIDEO_START_NAME(name)()

#define VIDEO_RESET_NAME(name)      video_reset_##name
#define VIDEO_RESET_CALL_MEMBER(name)       VIDEO_RESET_NAME(name)()
#define DECLARE_VIDEO_RESET(name)   void VIDEO_RESET_NAME(name)()
#define VIDEO_RESET_MEMBER(cls,name) void cls::VIDEO_RESET_NAME(name)()



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
typedef delegate<void ()> driver_callback_delegate;


// ======================> driver_device

// base class for machine driver-specific devices
class driver_device : public device_t
{
public:
	// construction/destruction
	driver_device(const machine_config &mconfig, device_type type, const char *tag);
	virtual ~driver_device();

	// getters
	const game_driver &system() const { assert(m_system != nullptr); return *m_system; }

	// indexes into our generic callbacks
	enum callback_type
	{
		CB_MACHINE_START,
		CB_MACHINE_RESET,
		CB_SOUND_START,
		CB_SOUND_RESET,
		CB_VIDEO_START,
		CB_VIDEO_RESET,
		CB_COUNT
	};

	// inline configuration helpers
	void set_game_driver(const game_driver &game);
	static void static_set_callback(device_t &device, callback_type type, driver_callback_delegate callback);

	// dummy driver_init callback
	void empty_init();

	// memory helpers
	address_space &generic_space() const { return machine().dummy_space(); }

	// output heler
	output_manager &output() const { return machine().output(); }

	void nmi_line_pulse(device_t &device);
	void nmi_line_assert(device_t &device);

	void irq0_line_hold(device_t &device);
	void irq0_line_assert(device_t &device);

	void irq1_line_hold(device_t &device);
	void irq1_line_assert(device_t &device);

	void irq2_line_hold(device_t &device);
	void irq2_line_assert(device_t &device);

	void irq3_line_hold(device_t &device);
	void irq3_line_assert(device_t &device);

	void irq4_line_hold(device_t &device);
	void irq4_line_assert(device_t &device);

	void irq5_line_hold(device_t &device);
	void irq5_line_assert(device_t &device);

	void irq6_line_hold(device_t &device);
	void irq6_line_assert(device_t &device);

	void irq7_line_hold(device_t &device);
	void irq7_line_assert(device_t &device);

	virtual void driver_init();

protected:
	// helpers called at startup
	virtual void driver_start();
	virtual void machine_start();
	virtual void sound_start();
	virtual void video_start();

	// helpers called at reset
	virtual void driver_reset();
	virtual void machine_reset();
	virtual void sound_reset();
	virtual void video_reset();

	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset_after_children() override;

	// generic video
	void flip_screen_set(u32 on);
	void flip_screen_x_set(u32 on);
	void flip_screen_y_set(u32 on);
	u32 flip_screen() const { return m_flip_screen_x; }
	u32 flip_screen_x() const { return m_flip_screen_x; }
	u32 flip_screen_y() const { return m_flip_screen_y; }

private:
	// helpers
	void updateflip();

	// internal state
	const game_driver        *m_system;               // pointer to the game driver
	driver_callback_delegate  m_callbacks[CB_COUNT];  // start/reset callbacks

	// generic video
	u8                          m_flip_screen_x;
	u8                          m_flip_screen_y;
};


#endif  /* MAME_EMU_DRIVER_H */
