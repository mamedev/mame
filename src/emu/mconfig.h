// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************/
/**
  * @file mconfig.h
  * @brief Core machine configuration macros and functions.
  * @attention Don't include this file directly, use emu.h instead.
  *
  * @defgroup MACHINE_CONFIG Machine configuration macros and functions
  * @{
  */
/***************************************************************************/


#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#include "emuopts.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// by convention, tags should all be lowercase
#define MIN_TAG_LENGTH          1

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
struct gfx_decode_entry;
class driver_device;
class screen_device;

// ======================> machine_config

/** @brief Machine configuration definition. Should not be instantiated directly.
    This class represents sub-device configuration for a machine/device.
    This class is instantiated through @ref MACHINE_CONFIG_STANDALONE "MACHINE_" macros.
    The configuration is built up using @ref MCFG_CORE "MCFG_" macros.
*/
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
	template<class _DeviceClass> inline _DeviceClass *device(const char *tag) const { return downcast<_DeviceClass *>(device(tag)); }

	// public state
	attotime                m_minimum_quantum;          //! minimum scheduling quantum
	std::string             m_perfect_cpu_quantum;      //! tag of CPU to use for "perfect" scheduling
	INT32                   m_watchdog_vblank_count;    //! number of VBLANKs until the watchdog kills us
	attotime                m_watchdog_time;            //! length of time until the watchdog kills us
	bool                    m_force_no_drc;             //! whether or not to force DRC off

	// other parameters
	const char *            m_default_layout;           //! default layout for this machine

	// helpers during configuration; not for general use
	device_t *device_add(device_t *owner, const char *tag, device_type type, UINT32 clock);
	device_t *device_replace(device_t *owner, const char *tag, device_type type, UINT32 clock);
	device_t *device_remove(device_t *owner, const char *tag);
	device_t *device_find(device_t *owner, const char *tag);

private:
	// internal state
	const game_driver &     m_gamedrv;
	emu_options &           m_options;
	std::unique_ptr<device_t>  m_root_device;
};

//*************************************************************************/
/** @anchor MACHINE_CONFIG_STANDALONE
 *  @name Standalone machine config macros*/
//*************************************************************************/

/**
 Returns the internal name for the machine_config.
 @param _name name of desired config
 @hideinitializer
 */
#define MACHINE_CONFIG_NAME(_name) construct_machine_config_##_name

/**
References an external machine_config.
@param _name name of the machine_config to reference
@hideinitializer
*/
#define MACHINE_CONFIG_EXTERN(_name) \
	extern device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner, device_t *device)

//*************************************************************************/
/** @anchor MACHINE_CONFIG_BEGIN_END
 *  @name Machine config start/end macros
 */
//*************************************************************************/

/**
 Begins a new machine_config.
 @param _name name of this config
 @param _class driver_device class for this config
 @hideinitializer
 */
#define MACHINE_CONFIG_START(_name, _class) \
ATTR_COLD device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner, device_t *device) \
{ \
	devcb_base *devcb = NULL; \
	(void)devcb; \
	if (owner == NULL) owner = config.device_add(NULL, "root", &driver_device_creator<_class>, 0);

/**
 Begins a partial machine_config that can only be included in another "root" machine_config. This is also used for machine_configs that are specified as part of a device.
 @param _name name of this config fragment
 @hideinitializer
*/
#define MACHINE_CONFIG_FRAGMENT(_name) \
ATTR_COLD device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner, device_t *device) \
{ \
	devcb_base *devcb = NULL; \
	(void)devcb; \
	assert(owner != NULL);

/**
 Begins a machine_config that is derived from another machine_config.
 @param _name name of this config
 @param _base name of the parent config
 @hideinitializer
*/
#define MACHINE_CONFIG_DERIVED(_name, _base) \
ATTR_COLD device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner, device_t *device) \
{ \
	devcb_base *devcb = NULL; \
	(void)devcb; \
	owner = MACHINE_CONFIG_NAME(_base)(config, owner, device); \
	assert(owner != NULL);

/**
Begins a machine_config that is derived from another machine_config that can specify an alternate driver_device class
@param _name name of this config
@param _base name of the parent config
@param _class name of the alternate driver_device class
@hideinitializer
*/
#define MACHINE_CONFIG_DERIVED_CLASS(_name, _base, _class) \
ATTR_COLD device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner, device_t *device) \
{ \
	devcb_base *devcb = NULL; \
	(void)devcb; \
	if (owner == NULL) owner = config.device_add(NULL, "root", &driver_device_creator<_class>, 0); \
	owner = MACHINE_CONFIG_NAME(_base)(config, owner, device);

/**
Ends a machine_config.
@hideinitializer
*/
#define MACHINE_CONFIG_END \
	return owner; \
}


//*************************************************************************/
/** @anchor MCFG_CORE
 *  @name Core machine config parameters
 */
//*************************************************************************/

/**
Imports a machine_config fragment from another driver
@param _name config fragment to import into this config
@hideinitializer
*/
#define MCFG_FRAGMENT_ADD(_name) \
	MACHINE_CONFIG_NAME(_name)(config, owner, device);


//*************************************************************************/
/** @anchor MCFG_SCHED
 *  @name Scheduling parameters
 */
//*************************************************************************/

/**
Specifies the quantum frequency for this config.
This specifies how frequently the device defined by the config is updated.
@param _time @ref attotime object representing the quantum frequency
@hideinitializer
*/
#define MCFG_QUANTUM_TIME(_time) \
	config.m_minimum_quantum = _time;
/**
Specifies the quantum frequency for this config to be perfect for a specific CPU.
@param _cputag tag string indicating the CPU to use
@hideinitializer
*/
#define MCFG_QUANTUM_PERFECT_CPU(_cputag) \
	config.m_perfect_cpu_quantum = owner->subtag(_cputag);

//*************************************************************************/
/** @anchor MCFG_DRC
 *  @name Recompilation parameters
 */
//*************************************************************************/
/**
Disables DRC for this config.
@hideinitializer
*/
#define MCFG_FORCE_NO_DRC() \
	config.m_force_no_drc = true;

//*************************************************************************/
/** @anchor MCFG_WATCHDOG
*  @name Watchdog configuration parameters
*/
//*************************************************************************/
/**
Configure the watchdog to fire at a specific vblank interval.
@param _count vblank interval
@hideinitializer
*/
#define MCFG_WATCHDOG_VBLANK_INIT(_count) \
	config.m_watchdog_vblank_count = _count;
/**
Configure the watchdog to fire at a specific time interval.
@param _time @ref attotime object representing the interval
@hideinitializer
*/
#define MCFG_WATCHDOG_TIME_INIT(_time) \
	config.m_watchdog_time = _time;

// core video parameters
#define MCFG_DEFAULT_LAYOUT(_layout) \
	config.m_default_layout = &(_layout)[0];

// add/remove devices
#define MCFG_DEVICE_ADD(_tag, _type, _clock) \
	device = config.device_add(owner, _tag, _type, _clock);
#define MCFG_DEVICE_REPLACE(_tag, _type, _clock) \
	device = config.device_replace(owner, _tag, _type, _clock);
#define MCFG_DEVICE_REMOVE(_tag) \
	device = config.device_remove(owner, _tag);
#define MCFG_DEVICE_MODIFY(_tag)    \
	device = config.device_find(owner, _tag);

#endif  /* __MCONFIG_H__ */
/** @} */
