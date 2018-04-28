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
	class token
	{
	public:
		token(machine_config &host, device_t &device) : m_host(host), m_device(&device)
		{
			assert(m_device == m_host.m_current_device);
		}
		token(token &&that) : m_host(that.m_host), m_device(that.m_device)
		{
			that.m_device = nullptr;
			assert(!m_device || (m_device == m_host.m_current_device));
		}
		token(token const &) = delete;
		token &operator=(token &&) = delete;
		token &operator=(token const &) = delete;
		~token()
		{
			if (m_device)
			{
				assert(m_device == m_host.m_current_device);
				m_host.m_current_device = nullptr;
			}
		}
	private:
		machine_config &m_host;
		device_t *m_device;
	};

	// construction/destruction
	machine_config(const game_driver &gamedrv, emu_options &options);
	~machine_config();

	// getters
	const game_driver &gamedrv() const { return m_gamedrv; }
	device_t &root_device() const { assert(m_root_device != nullptr); return *m_root_device; }
	emu_options &options() const { return m_options; }
	inline device_t *device(const char *tag) const { return root_device().subdevice(tag); }
	template <class DeviceClass> inline DeviceClass *device(const char *tag) const { return downcast<DeviceClass *>(device(tag)); }

	// public state
	attotime                m_minimum_quantum;          // minimum scheduling quantum
	std::string             m_perfect_cpu_quantum;      // tag of CPU to use for "perfect" scheduling

	// other parameters
	const internal_layout *            m_default_layout;           // default layout for this machine

	// helpers during configuration; not for general use
	token begin_configuration(device_t &device)
	{
		assert(!m_current_device);
		m_current_device = &device;
		return token(*this, device);
	}
	device_t *device_add(const char *tag, device_type type, u32 clock);
	template <typename... Params>
	device_t *device_add(const char *tag, device_type type, const XTAL &clock, Params &&... args)
	{
		clock.validate(std::string("Instantiating device ") + tag);
		return device_add(tag, type, clock.value(), std::forward<Params>(args)...);
	}
	device_t *device_replace(const char *tag, device_type type, u32 clock);
	device_t *device_replace(const char *tag, device_type type, const XTAL &xtal);
	device_t *device_remove(const char *tag);
	device_t *device_find(device_t *owner, const char *tag);

private:
	class current_device_stack;

	// internal helpers
	void remove_references(ATTR_UNUSED device_t &device);

	// internal state
	game_driver const &         m_gamedrv;
	emu_options &               m_options;
	std::unique_ptr<device_t>   m_root_device;
	device_t *                  m_current_device;
};


namespace emu { namespace detail {

template <class DeviceClass> template <typename... Params>
DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config &config, char const *tag, Params &&... args) const
{
	return dynamic_cast<DeviceClass &>(*config.device_add(tag, *this, std::forward<Params>(args)...));
}

} } // namespace emu::detail


//*************************************************************************/
/** @name Machine config start/end macros */
//*************************************************************************/

/**
 @def MACHINE_CONFIG_START(_name)
 Begins a device machine configuration member
 @param _name name of this config
 @hideinitializer
*/
#define MACHINE_CONFIG_START(_name) \
ATTR_COLD void _name(machine_config &config) \
{ \
	device_t *device = nullptr; \
	devcb_base *devcb = nullptr; \
	(void)device; \
	(void)devcb;

/**
@def MACHINE_CONFIG_END
Ends a machine_config.
@hideinitializer
*/
#define MACHINE_CONFIG_END \
}

//*************************************************************************/
/** @name Core machine config options */
//*************************************************************************/

// scheduling parameters
#define MCFG_QUANTUM_TIME(_time) \
	config.m_minimum_quantum = _time;
#define MCFG_QUANTUM_PERFECT_CPU(_cputag) \
	config.m_perfect_cpu_quantum = subtag(_cputag);

// core video parameters
#define MCFG_DEFAULT_LAYOUT(_layout) \
	config.m_default_layout = &(_layout);

// add/remove devices
#define MCFG_DEVICE_ADD(_tag, _type, _clock) \
	device = config.device_add(_tag, _type, _clock);
#define MCFG_DEVICE_REPLACE(_tag, _type, _clock) \
	device = config.device_replace(_tag, _type, _clock);
#define MCFG_DEVICE_REMOVE(_tag) \
	device = config.device_remove(_tag);
#define MCFG_DEVICE_MODIFY(_tag)    \
	device = config.device_find(this, _tag);

#endif  /* MAME_EMU_MCONFIG_H */
/** @} */
