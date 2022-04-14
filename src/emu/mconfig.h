// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_MCONFIG_H
#define MAME_EMU_MCONFIG_H

#include <cassert>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// by convention, tags should all be lowercase
#define MIN_TAG_LENGTH          1

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace emu::detail {

class machine_config_replace
{
public:
	machine_config_replace(machine_config_replace const &) = default;
	machine_config &config;
private:
	machine_config_replace(machine_config &c) : config(c) { }
	friend class ::machine_config;
};

} // namespace emu::detail


/// \brief Internal layout description
///
/// Holds the compressed and decompressed data size, compression method,
/// and a reference to the compressed layout data.  Note that copying
/// the structure will not copy the referenced data.
struct internal_layout
{
	enum class compression { NONE, ZLIB };

	size_t decompressed_size;
	size_t compressed_size;
	compression compression_type;
	u8 const *data;
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
	device_t &root_device() const { assert(m_root_device); return *m_root_device; }
	device_t &current_device() const { assert(m_current_device); return *m_current_device; }
	emu_options &options() const { return m_options; }
	device_t *device(const char *tag) const { return root_device().subdevice(tag); }
	template <class DeviceClass> DeviceClass *device(const char *tag) const { return downcast<DeviceClass *>(device(tag)); }
	attotime maximum_quantum(attotime const &default_quantum) const;
	device_execute_interface *perfect_quantum_device() const;

	/// \brief Apply visitor to internal layouts
	///
	/// Calls the supplied visitor for each device with an internal
	/// layout.  The order of devices is implementation-dependent.
	/// \param [in] op The visitor.  It must provide a function call
	//    operator that can be invoked with two arguments: a reference
	//    to a #device_t and a const reference to an #internal_layout.
	template <typename T> void apply_default_layouts(T &&op) const
	{
		for (std::pair<char const *const, internal_layout const *> const &lay : m_default_layouts)
			op(*device(lay.first), *lay.second);
	}

	/// \brief Get a device replacement helper
	///
	/// Pass the result in place of the machine configuration itself to
	/// replace an existing device.
	/// \return A device replacement helper to pass to a device type
	///   when replacing an existing device.
	emu::detail::machine_config_replace replace() { return emu::detail::machine_config_replace(*this); }

	/// \brief Set internal layout for current device
	///
	/// Set internal layout for current device.  Each device in the
	/// system can have its own internal layout.  Tags in the layout
	/// will be resolved relative to the device.  Replaces previously
	/// set layout if any.
	/// \param [in] layout Reference to the internal layout description
	///   structure.  Neither the description structure nor the
	///   compressed data is copied.  It is the caller's responsibility
	///   to ensure both remain valid until layouts and views are
	///   instantiated.
	void set_default_layout(internal_layout const &layout);

	/// \brief Set maximum scheduling quantum
	///
	/// Set the maximum scheduling quantum required for the current
	/// device.  The smallest maximum quantum requested by a device in
	/// the system will be used.
	/// \param [in] quantum Maximum scheduling quantum in attoseconds.
	void set_maximum_quantum(attotime const &quantum);

	template <typename T>
	void set_perfect_quantum(T &&tag)
	{
		set_perfect_quantum(current_device(), std::forward<T>(tag));
	}
	template <class DeviceClass, bool Required>
	void set_perfect_quantum(device_finder<DeviceClass, Required> const &finder)
	{
		std::pair<device_t &, char const *> const target(finder.finder_target());
		set_perfect_quantum(target.first, target.second);
	}
	template <class DeviceClass, bool Required>
	void set_perfect_quantum(device_finder<DeviceClass, Required> &finder)
	{
		set_perfect_quantum(const_cast<device_finder<DeviceClass, Required> const &>(finder));
	}

	// helpers during configuration; not for general use
	token begin_configuration(device_t &device)
	{
		assert(!m_current_device);
		m_current_device = &device;
		return token(*this, device);
	}
	device_t *device_add(const char *tag, device_type type, u32 clock);
	template <typename Creator>
	device_t *device_add(const char *tag, Creator &&type, u32 clock)
	{
		return device_add(tag, device_type(type), clock);
	}
	template <typename Creator, typename... Params>
	auto device_add(const char *tag, Creator &&type, Params &&... args)
	{
		std::pair<const char *, device_t *> const owner(resolve_owner(tag));
		auto device(type.create(*this, owner.first, owner.second, std::forward<Params>(args)...));
		auto &result(*device);
		assert(type.type() == typeid(result));
		add_device(std::move(device), owner.second);
		return &result;
	}
	template <typename Creator, typename... Params>
	auto device_add(const char *tag, Creator &&type, XTAL clock, Params &&... args)
	{
		clock.validate(std::string("Instantiating device ") + tag);
		return device_add(tag, std::forward<Creator>(type), clock.value(), std::forward<Params>(args)...);
	}
	device_t *device_replace(const char *tag, device_type type, u32 clock);
	template <typename Creator>
	device_t *device_replace(const char *tag, Creator &&type, u32 clock)
	{
		return device_replace(tag, device_type(type), clock);
	}
	template <typename Creator, typename... Params>
	auto device_replace(const char *tag, Creator &&type, Params &&... args)
	{
		std::tuple<const char *, device_t *, device_t *> const existing(prepare_replace(tag));
		auto device(type.create(*this, std::get<0>(existing), std::get<1>(existing), std::forward<Params>(args)...));
		auto &result(*device);
		assert(type.type() == typeid(result));
		replace_device(std::move(device), *std::get<1>(existing), std::get<2>(existing));
		return &result;
	}
	template <typename Creator, typename... Params>
	auto device_replace(const char *tag, Creator &&type, XTAL clock, Params &&... args)
	{
		clock.validate(std::string("Replacing device ") + tag);
		return device_replace(tag, std::forward<Creator>(type), clock.value(), std::forward<Params>(args)...);
	}
	device_t *device_remove(const char *tag);

private:
	class current_device_stack;
	typedef std::map<char const *, internal_layout const *, bool (*)(char const *, char const *)> default_layout_map;
	typedef std::map<char const *, attotime, bool (*)(char const *, char const *)> maximum_quantum_map;

	// internal helpers
	std::pair<const char *, device_t *> resolve_owner(const char *tag) const;
	std::tuple<const char *, device_t *, device_t *> prepare_replace(const char *tag);
	device_t &add_device(std::unique_ptr<device_t> &&device, device_t *owner);
	device_t &replace_device(std::unique_ptr<device_t> &&device, device_t &owner, device_t *existing);
	void remove_references(device_t &device);
	void set_perfect_quantum(device_t &device, std::string tag);

	// internal state
	game_driver const &                 m_gamedrv;
	emu_options &                       m_options;
	std::unique_ptr<device_t>           m_root_device;
	default_layout_map                  m_default_layouts;
	device_t *                          m_current_device;
	maximum_quantum_map                 m_maximum_quantums;
	std::pair<device_t *, std::string>  m_perfect_quantum_device;
};

#endif // MAME_EMU_MCONFIG_H
