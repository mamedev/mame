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
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_START, driver_callback_delegate(&_class::MACHINE_START_NAME(_func), #_class "::machine_start_" #_func, downcast<_class *>(owner)));

#define MCFG_MACHINE_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_RESET, driver_callback_delegate(&_class::MACHINE_RESET_NAME(_func), #_class "::machine_reset_" #_func, downcast<_class *>(owner)));

#define MCFG_MACHINE_RESET_REMOVE() \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_MACHINE_RESET, driver_callback_delegate());

// core sound callbacks
#define MCFG_SOUND_START_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_SOUND_START, driver_callback_delegate(&_class::SOUND_START_NAME(_func), #_class "::sound_start_" #_func, downcast<_class *>(owner)));

#define MCFG_SOUND_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_SOUND_RESET, driver_callback_delegate(&_class::SOUND_RESET_NAME(_func), #_class "::sound_reset_" #_func, downcast<_class *>(owner)));


// core video callbacks
#define MCFG_VIDEO_START_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_VIDEO_START, driver_callback_delegate(&_class::VIDEO_START_NAME(_func), #_class "::video_start_" #_func, downcast<_class *>(owner)));

#define MCFG_VIDEO_RESET_OVERRIDE(_class, _func) \
	driver_device::static_set_callback(config.root_device(), driver_device::CB_VIDEO_RESET, driver_callback_delegate(&_class::VIDEO_RESET_NAME(_func), #_class "::video_reset_" #_func, downcast<_class *>(owner)));



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
	driver_device(const machine_config &mconfig, device_type type, std::string tag);
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

	INTERRUPT_GEN_MEMBER( nmi_line_pulse );
	INTERRUPT_GEN_MEMBER( nmi_line_assert );

	INTERRUPT_GEN_MEMBER( irq0_line_hold );
	INTERRUPT_GEN_MEMBER( irq0_line_pulse );
	INTERRUPT_GEN_MEMBER( irq0_line_assert );

	INTERRUPT_GEN_MEMBER( irq1_line_hold );
	INTERRUPT_GEN_MEMBER( irq1_line_pulse );
	INTERRUPT_GEN_MEMBER( irq1_line_assert );

	INTERRUPT_GEN_MEMBER( irq2_line_hold );
	INTERRUPT_GEN_MEMBER( irq2_line_pulse );
	INTERRUPT_GEN_MEMBER( irq2_line_assert );

	INTERRUPT_GEN_MEMBER( irq3_line_hold );
	INTERRUPT_GEN_MEMBER( irq3_line_pulse );
	INTERRUPT_GEN_MEMBER( irq3_line_assert );

	INTERRUPT_GEN_MEMBER( irq4_line_hold );
	INTERRUPT_GEN_MEMBER( irq4_line_pulse );
	INTERRUPT_GEN_MEMBER( irq4_line_assert );

	INTERRUPT_GEN_MEMBER( irq5_line_hold );
	INTERRUPT_GEN_MEMBER( irq5_line_pulse );
	INTERRUPT_GEN_MEMBER( irq5_line_assert );

	INTERRUPT_GEN_MEMBER( irq6_line_hold );
	INTERRUPT_GEN_MEMBER( irq6_line_pulse );
	INTERRUPT_GEN_MEMBER( irq6_line_assert );

	INTERRUPT_GEN_MEMBER( irq7_line_hold );
	INTERRUPT_GEN_MEMBER( irq7_line_pulse );
	INTERRUPT_GEN_MEMBER( irq7_line_assert );

	// watchdog read/write handlers
	DECLARE_WRITE8_MEMBER( watchdog_reset_w );
	DECLARE_READ8_MEMBER( watchdog_reset_r );
	DECLARE_WRITE16_MEMBER( watchdog_reset16_w );
	DECLARE_READ16_MEMBER( watchdog_reset16_r );
	DECLARE_WRITE32_MEMBER( watchdog_reset32_w );
	DECLARE_READ32_MEMBER( watchdog_reset32_r );

	// generic audio
	void soundlatch_setclearedvalue(UINT16 value) { m_latch_clear_value = value; }

	// sound latch readers
	UINT32 soundlatch_read(UINT8 index = 0);
	DECLARE_READ8_MEMBER( soundlatch_byte_r );
	DECLARE_READ8_MEMBER( soundlatch2_byte_r );
	DECLARE_READ8_MEMBER( soundlatch3_byte_r );
	DECLARE_READ8_MEMBER( soundlatch4_byte_r );
	DECLARE_READ16_MEMBER( soundlatch_word_r );
	DECLARE_READ16_MEMBER( soundlatch2_word_r );
	DECLARE_READ16_MEMBER( soundlatch3_word_r );
	DECLARE_READ16_MEMBER( soundlatch4_word_r );

	// sound latch writers
	void soundlatch_write(UINT8 index, UINT32 data);
	void soundlatch_write(UINT32 data) { soundlatch_write(0, data); }
	DECLARE_WRITE8_MEMBER( soundlatch_byte_w );
	DECLARE_WRITE8_MEMBER( soundlatch2_byte_w );
	DECLARE_WRITE8_MEMBER( soundlatch3_byte_w );
	DECLARE_WRITE8_MEMBER( soundlatch4_byte_w );
	DECLARE_WRITE16_MEMBER( soundlatch_word_w );
	DECLARE_WRITE16_MEMBER( soundlatch2_word_w );
	DECLARE_WRITE16_MEMBER( soundlatch3_word_w );
	DECLARE_WRITE16_MEMBER( soundlatch4_word_w );

	// sound latch clearers
	void soundlatch_clear(UINT8 index = 0);
	DECLARE_WRITE8_MEMBER( soundlatch_clear_byte_w );
	DECLARE_WRITE8_MEMBER( soundlatch2_clear_byte_w );
	DECLARE_WRITE8_MEMBER( soundlatch3_clear_byte_w );
	DECLARE_WRITE8_MEMBER( soundlatch4_clear_byte_w );

	// generic video
	void flip_screen_set(UINT32 on);
	void flip_screen_set_no_update(UINT32 on);
	void flip_screen_x_set(UINT32 on);
	void flip_screen_y_set(UINT32 on);
	UINT32 flip_screen() const { return m_flip_screen_x; }
	UINT32 flip_screen_x() const { return m_flip_screen_x; }
	UINT32 flip_screen_y() const { return m_flip_screen_y; }

	// generic input port helpers
	DECLARE_CUSTOM_INPUT_MEMBER( custom_port_read );

	// general fatal error handlers
	DECLARE_READ8_MEMBER( fatal_generic_read );
	DECLARE_WRITE8_MEMBER( fatal_generic_write );

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
	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset_after_children() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
private:
	// helpers
	void irq_pulse_clear(void *ptr, INT32 param);
	void soundlatch_sync_callback(void *ptr, INT32 param);
	void updateflip();

	// configuration state
	const address_space_config  m_space_config;

	// internal state
	const game_driver *     m_system;                   // pointer to the game driver
	driver_callback_delegate m_callbacks[CB_COUNT];     // start/reset callbacks

	// generic audio
	UINT16                  m_latch_clear_value;
	UINT16                  m_latched_value[4];
	UINT8                   m_latch_read[4];

	// generic video
	UINT8                   m_flip_screen_x;
	UINT8                   m_flip_screen_y;
};


// this template function creates a stub which constructs a device
template<class _DriverClass>
device_t *driver_device_creator(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
{
	assert(owner == nullptr);
	assert(clock == 0);
	return global_alloc_clear<_DriverClass>(mconfig, &driver_device_creator<_DriverClass>, tag);
}


#endif  /* __DRIVER_H__ */
