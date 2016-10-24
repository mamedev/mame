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

#ifndef __DRIVER_H__
#define __DRIVER_H__


//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

// core machine callbacks
#define MCFG_MACHINE_START_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_START, driver_callback_delegate(&_class::machine_start_##_func, #_class "::machine_start_" #_func, downcast<_class *>(owner)));

#define MCFG_MACHINE_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_RESET, driver_callback_delegate(&_class::machine_reset_##_func, #_class "::machine_reset_" #_func, downcast<_class *>(owner)));

#define MCFG_MACHINE_RESET_REMOVE() \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_RESET, driver_callback_delegate());

// core sound callbacks
#define MCFG_SOUND_START_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_SOUND_START, driver_callback_delegate(&_class::sound_start_##_func, #_class "::sound_start_" #_func, downcast<_class *>(owner)));

#define MCFG_SOUND_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_SOUND_RESET, driver_callback_delegate(&_class::sound_reset_##_func, #_class "::sound_reset_" #_func, downcast<_class *>(owner)));


// core video callbacks
#define MCFG_VIDEO_START_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_VIDEO_START, driver_callback_delegate(&_class::video_start_##_func, #_class "::video_start_" #_func, downcast<_class *>(owner)));

#define MCFG_VIDEO_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_VIDEO_RESET, driver_callback_delegate(&_class::video_reset_##_func, #_class "::video_reset_" #_func, downcast<_class *>(owner)));



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class gfxdecode_device;
class palette_device;
typedef delegate<void ()> driver_callback_delegate;


// ======================> driver_device

// base class for machine driver-specific devices
class driver_device :   public device_t,
						public device_memory_interface
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
	static void static_set_game(device_t &device, const game_driver &game);
	static void static_set_callback(device_t &device, callback_type type, driver_callback_delegate callback);

	// generic helpers
	template<class _DriverClass, void (_DriverClass::*_Function)()>
	static void driver_init_wrapper(running_machine &machine)
	{
		(machine.driver_data<_DriverClass>()->*_Function)();
	}

	// dummy driver_init callbacks
	void init_0() { }

	// memory helpers
	address_space &generic_space() const { return space(AS_PROGRAM); }

	// output heler
	output_manager &output() const { return machine().output(); }

	// generic interrupt generators
	void generic_pulse_irq_line(device_execute_interface &exec, int irqline, int cycles);
	void generic_pulse_irq_line_and_vector(device_execute_interface &exec, int irqline, int vector, int cycles);

	void nmi_line_pulse(device_t &device);
	void nmi_line_assert(device_t &device);

	void irq0_line_hold(device_t &device);
	void irq0_line_pulse(device_t &device);
	void irq0_line_assert(device_t &device);

	void irq1_line_hold(device_t &device);
	void irq1_line_pulse(device_t &device);
	void irq1_line_assert(device_t &device);

	void irq2_line_hold(device_t &device);
	void irq2_line_pulse(device_t &device);
	void irq2_line_assert(device_t &device);

	void irq3_line_hold(device_t &device);
	void irq3_line_pulse(device_t &device);
	void irq3_line_assert(device_t &device);

	void irq4_line_hold(device_t &device);
	void irq4_line_pulse(device_t &device);
	void irq4_line_assert(device_t &device);

	void irq5_line_hold(device_t &device);
	void irq5_line_pulse(device_t &device);
	void irq5_line_assert(device_t &device);

	void irq6_line_hold(device_t &device);
	void irq6_line_pulse(device_t &device);
	void irq6_line_assert(device_t &device);

	void irq7_line_hold(device_t &device);
	void irq7_line_pulse(device_t &device);
	void irq7_line_assert(device_t &device);


	// generic video
	void flip_screen_set(uint32_t on);
	void flip_screen_set_no_update(uint32_t on);
	void flip_screen_x_set(uint32_t on);
	void flip_screen_y_set(uint32_t on);
	uint32_t flip_screen() const { return m_flip_screen_x; }
	uint32_t flip_screen_x() const { return m_flip_screen_x; }
	uint32_t flip_screen_y() const { return m_flip_screen_y; }

	// generic input port helpers
	ioport_value custom_port_read(ioport_field &field, void *param);

	// general fatal error handlers
	uint8_t fatal_generic_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fatal_generic_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset_after_children() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
private:
	// helpers
	void irq_pulse_clear(void *ptr, int32_t param);
	void updateflip();

	// configuration state
	const address_space_config  m_space_config;

	// internal state
	const game_driver *     m_system;                   // pointer to the game driver
	driver_callback_delegate m_callbacks[CB_COUNT];     // start/reset callbacks

	// generic video
	uint8_t                   m_flip_screen_x;
	uint8_t                   m_flip_screen_y;
};


// this template function creates a stub which constructs a device
template<class _DriverClass>
device_t *driver_device_creator(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
{
	assert(owner == nullptr);
	assert(clock == 0);
	return global_alloc_clear<_DriverClass>(mconfig, &driver_device_creator<_DriverClass>, tag);
}


#endif  /* __DRIVER_H__ */
