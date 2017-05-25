// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************/
/**
  * @file mconfig.h
  * @defgroup MACHINE_CONFIG Machine configuration macros and functions
  * @{
  */
/***************************************************************************/


#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_MCONFIG_H
#define MAME_EMU_MCONFIG_H

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// by convention, tags should all be lowercase
#define MIN_TAG_LENGTH          1

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct internal_layout
{
	size_t decompressed_size;
	size_t compressed_size;
	u8 compression_type;
	const u8* data;
};


// ======================> machine_config

// machine configuration definition
class machine_config
{
	DISABLE_COPYING(machine_config);

	friend class running_machine;

public:
	// construction/destruction
	machine_config(const game_driver &gamedrv, emu_options &options);
	~machine_config();

	// getters
	const game_driver &gamedrv() const { return m_gamedrv; }
	device_t &root_device() const { assert(m_root_device != nullptr); return *m_root_device; }
	screen_device *first_screen() const;
	emu_options &options() const { return m_options; }
	inline device_t *device(const char *tag) const { return root_device().subdevice(tag); }
	template <class DeviceClass> inline DeviceClass *device(const char *tag) const { return downcast<DeviceClass *>(device(tag)); }

	// public state
	attotime                m_minimum_quantum;          // minimum scheduling quantum
	std::string             m_perfect_cpu_quantum;      // tag of CPU to use for "perfect" scheduling

	// other parameters
	const internal_layout *            m_default_layout;           // default layout for this machine

	// helpers during configuration; not for general use
	device_t *device_add(device_t *owner, const char *tag, device_type type, u32 clock);
	device_t *device_replace(device_t *owner, const char *tag, device_type type, u32 clock);
	device_t *device_remove(device_t *owner, const char *tag);
	device_t *device_find(device_t *owner, const char *tag);

private:
	// internal helpers
	void remove_references(ATTR_UNUSED device_t &device);

	// internal state
	const game_driver &     m_gamedrv;
	emu_options &           m_options;
	std::unique_ptr<device_t>  m_root_device;
};


//*************************************************************************/
/** @name Machine config start/end macros */
//*************************************************************************/

/**
 @def MACHINE_CONFIG_NAME(_name)
 Returns the internal name for the machine config.
 @param _name name of desired config
 @hideinitializer
 */
#define MACHINE_CONFIG_NAME(_name) construct_machine_config_##_name

/**
 @def MACHINE_CONFIG_START(_name)
 Begins a new machine/device config.
 @param _name name of this config
 @hideinitializer
 */
#define MACHINE_CONFIG_START(_name) \
ATTR_COLD void MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner, device_t *device) \
{ \
	devcb_base *devcb = nullptr; \
	(void)devcb; \
	assert(owner != nullptr);

/**
 @def MACHINE_CONFIG_DERIVED(_name, _base)
 Begins a machine_config that is derived from another machine_config.
 @param _name name of this config
 @param _base name of the parent config
 @hideinitializer
*/
#define MACHINE_CONFIG_DERIVED(_name, _base) \
ATTR_COLD void MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner, device_t *device) \
{ \
	devcb_base *devcb = nullptr; \
	(void)devcb; \
	assert(owner != nullptr); \
	MACHINE_CONFIG_NAME(_base)(config, owner, device);

/**
 @def MACHINE_CONFIG_MEMBER(_name)
 Begins a device machine configuration member (usually overriding device_t::device_add_mconfig).
 @param _name name of this config
 @param _base name of the parent config
 @hideinitializer
*/
#define MACHINE_CONFIG_MEMBER(_name) \
ATTR_COLD void _name(machine_config &config) \
{ \
	device_t *const owner = this; \
	device_t *device = nullptr; \
	devcb_base *devcb = nullptr; \
	(void)owner; \
	(void)device; \
	(void)devcb; \

/**
@def MACHINE_CONFIG_END
Ends a machine_config.
@hideinitializer
*/
#define MACHINE_CONFIG_END \
}

//*************************************************************************/
/** @name Standalone machine config macros */
//*************************************************************************/

/**
@def MACHINE_CONFIG_EXTERN(_name)
References an external machine config.
@param _name Name of the machine config to reference
@hideinitializer
*/
#define MACHINE_CONFIG_EXTERN(_name) \
	extern void MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner, device_t *device)

//*************************************************************************/
/** @name Core machine config options */
//*************************************************************************/

// importing data from other machine drivers
#define MCFG_FRAGMENT_ADD(_name) \
	MACHINE_CONFIG_NAME(_name)(config, owner, device);


// scheduling parameters
#define MCFG_QUANTUM_TIME(_time) \
	config.m_minimum_quantum = _time;
#define MCFG_QUANTUM_PERFECT_CPU(_cputag) \
	config.m_perfect_cpu_quantum = owner->subtag(_cputag);

// core video parameters
#define MCFG_DEFAULT_LAYOUT(_layout) \
	config.m_default_layout = &(_layout);

// add/remove devices
#define MCFG_DEVICE_ADD(_tag, _type, _clock) \
	device = config.device_add(owner, _tag, _type, _clock);
#define MCFG_DEVICE_REPLACE(_tag, _type, _clock) \
	device = config.device_replace(owner, _tag, _type, _clock);
#define MCFG_DEVICE_REMOVE(_tag) \
	device = config.device_remove(owner, _tag);
#define MCFG_DEVICE_MODIFY(_tag)    \
	device = config.device_find(owner, _tag);

#endif  /* MAME_EMU_MCONFIG_H */
/** @} */
