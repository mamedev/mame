// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    device.h

    Device interface functions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DEVICE_H
#define MAME_EMU_DEVICE_H

#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <vector>



//**************************************************************************
//  MACROS
//**************************************************************************

// macro for specifying a clock derived from an owning device
#define DERIVED_CLOCK(num, den)     (0xff000000 | ((num) << 12) | ((den) << 0))



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// configure devices

#define DECLARE_READ_LINE_MEMBER(name)      int  name()
#define READ_LINE_MEMBER(name)              int  name()
#define DECLARE_WRITE_LINE_MEMBER(name)     void name([[maybe_unused]] int state)
#define WRITE_LINE_MEMBER(name)             void name([[maybe_unused]] int state)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// use this to refer to the owning device when providing a device tag
static const char DEVICE_SELF[] = "";

// use this to refer to the owning device's owner when providing a device tag
static const char DEVICE_SELF_OWNER[] = "^";


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace emu::detail {

class device_type_impl_base;


template <typename T>
using is_device_implementation = std::bool_constant<std::is_base_of_v<device_t, T> >;

template <typename T>
using is_device_interface = std::bool_constant<std::is_base_of_v<device_interface, T> && !is_device_implementation<T>::value>;


struct device_feature
{
	enum type : u32
	{
		// Functionality-related
		PROTECTION  = u32(1) <<  0,
		TIMING      = u32(1) <<  1,

		// Graphics
		GRAPHICS    = u32(1) <<  2,
		PALETTE     = u32(1) <<  3,

		// Sound
		SOUND       = u32(1) <<  4,

		// Capture/Media Output
		CAPTURE     = u32(1) <<  5,
		CAMERA      = u32(1) <<  6,
		MICROPHONE  = u32(1) <<  7,

		// Controls/HID
		CONTROLS    = u32(1) <<  8,
		KEYBOARD    = u32(1) <<  9,
		MOUSE       = u32(1) << 10,

		// Media Output
		MEDIA       = u32(1) << 11,
		DISK        = u32(1) << 12,
		PRINTER     = u32(1) << 13,
		TAPE        = u32(1) << 14,
		PUNCH       = u32(1) << 15,
		DRUM        = u32(1) << 16,
		ROM         = u32(1) << 17,

		// Comms/Network
		COMMS       = u32(1) << 18,
		LAN         = u32(1) << 19,
		WAN         = u32(1) << 20,

		NONE        = u32(0),
		ALL         = (u32(1) << 21) - 1U
	};
};

DECLARE_ENUM_BITWISE_OPERATORS(device_feature::type);


class device_registrar
{
private:
	class const_iterator_helper;

public:
	class const_iterator
	{
	public:
		typedef std::ptrdiff_t difference_type;
		typedef device_type_impl_base value_type;
		typedef device_type_impl_base *pointer;
		typedef device_type_impl_base &reference;
		typedef std::forward_iterator_tag iterator_category;

		const_iterator() = default;
		const_iterator(const_iterator const &) = default;
		const_iterator &operator=(const_iterator const &) = default;

		bool operator==(const_iterator const &that) const { return m_type == that.m_type; }
		bool operator!=(const_iterator const &that) const { return m_type != that.m_type; }
		reference operator*() const { assert(m_type); return *m_type; }
		pointer operator->() const { return m_type; }
		const_iterator &operator++();
		const_iterator operator++(int) { const_iterator const result(*this); ++*this; return result; }

	private:
		friend class const_iterator_helper;

		pointer m_type = nullptr;
	};

	// explicit constructor is required for const variable initialization
	constexpr device_registrar() { }

	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }
	const_iterator cbegin() const;
	const_iterator cend() const;

private:
	friend class device_type_impl_base;

	class const_iterator_helper : public const_iterator
	{
	public:
		const_iterator_helper(device_type_impl_base *type) { m_type = type; }
	};

	static device_type_impl_base *register_device(device_type_impl_base &type);
};


template <class DeviceClass, char const *ShortName, char const *FullName, char const *Source>
struct device_tag_struct { typedef DeviceClass type; };
template <class DriverClass, char const *ShortName, char const *FullName, char const *Source, device_feature::type Unemulated, device_feature::type Imperfect>
struct driver_tag_struct { typedef DriverClass type; };

template <class DeviceClass, char const *ShortName, char const *FullName, char const *Source>
auto device_tag_func() { return device_tag_struct<DeviceClass, ShortName, FullName, Source>{ }; };
template <class DriverClass, char const *ShortName, char const *FullName, char const *Source, device_feature::type Unemulated, device_feature::type Imperfect>
auto driver_tag_func() { return driver_tag_struct<DriverClass, ShortName, FullName, Source, Unemulated, Imperfect>{ }; };

class device_type_impl_base
{
private:
	friend class device_registrar;

	typedef std::unique_ptr<device_t> (*create_func)(device_type_impl_base const &type, machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	device_type_impl_base(device_type_impl_base const &) = delete;
	device_type_impl_base(device_type_impl_base &&) = delete;
	device_type_impl_base &operator=(device_type_impl_base const &) = delete;
	device_type_impl_base &operator=(device_type_impl_base &&) = delete;

	template <typename DeviceClass>
	static std::unique_ptr<device_t> create_device(device_type_impl_base const &type, machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	{
		return std::make_unique<DeviceClass>(mconfig, tag, owner, clock);
	}

	template <typename DriverClass>
	static std::unique_ptr<device_t> create_driver(device_type_impl_base const &type, machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	{
		assert(!owner);
		assert(!clock);

		return std::make_unique<DriverClass>(mconfig, type, tag);
	}

	create_func const m_creator;
	std::type_info const &m_type;
	char const *const m_shortname;
	char const *const m_fullname;
	char const *const m_source;
	device_feature::type const m_unemulated_features;
	device_feature::type const m_imperfect_features;
	device_type_impl_base const *const m_parent_rom;

	device_type_impl_base *m_next;

public:
	using exposed_type = device_t;

	device_type_impl_base(std::nullptr_t)
		: m_creator(nullptr)
		, m_type(typeid(std::nullptr_t))
		, m_shortname(nullptr)
		, m_fullname(nullptr)
		, m_source(nullptr)
		, m_unemulated_features(device_feature::NONE)
		, m_imperfect_features(device_feature::NONE)
		, m_parent_rom(nullptr)
		, m_next(nullptr)
	{
	}

	template <class DeviceClass, char const *ShortName, char const *FullName, char const *Source>
	device_type_impl_base(device_tag_struct<DeviceClass, ShortName, FullName, Source> (*)())
		: m_creator(&create_device<DeviceClass>)
		, m_type(typeid(DeviceClass))
		, m_shortname(ShortName)
		, m_fullname(FullName)
		, m_source(Source)
		, m_unemulated_features(DeviceClass::unemulated_features())
		, m_imperfect_features(DeviceClass::imperfect_features())
		, m_parent_rom(DeviceClass::parent_rom_device_type())
		, m_next(device_registrar::register_device(*this))
	{
	}

	template <class DriverClass, char const *ShortName, char const *FullName, char const *Source, device_feature::type Unemulated, device_feature::type Imperfect>
	device_type_impl_base(driver_tag_struct<DriverClass, ShortName, FullName, Source, Unemulated, Imperfect> (*)())
		: m_creator(&create_driver<DriverClass>)
		, m_type(typeid(DriverClass))
		, m_shortname(ShortName)
		, m_fullname(FullName)
		, m_source(Source)
		, m_unemulated_features(DriverClass::unemulated_features() | Unemulated)
		, m_imperfect_features((DriverClass::imperfect_features() & ~Unemulated) | Imperfect)
		, m_parent_rom(DriverClass::parent_rom_device_type())
		, m_next(nullptr)
	{
	}

	std::type_info const &type() const { return m_type; }
	char const *shortname() const { return m_shortname; }
	char const *fullname() const { return m_fullname; }
	char const *source() const { return m_source; }
	device_feature::type unemulated_features() const { return m_unemulated_features; }
	device_feature::type imperfect_features() const { return m_imperfect_features; }
	device_type_impl_base const *parent_rom_device_type() const { return m_parent_rom; }

	std::unique_ptr<device_t> create(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) const
	{
		return m_creator(*this, mconfig, tag, owner, clock);
	}

	explicit operator bool() const { return bool(m_creator); }
	bool operator==(device_type_impl_base const &that) const { return &that == this; }
	bool operator!=(device_type_impl_base const &that) const { return &that != this; }
};


template <class DeviceClass>
class device_type_impl : public device_type_impl_base
{
public:
	using exposed_type = DeviceClass;

	using device_type_impl_base::device_type_impl_base;
	using device_type_impl_base::create;

	template <typename... Params>
	std::unique_ptr<DeviceClass> create(machine_config &mconfig, char const *tag, device_t *owner, Params &&... args) const
	{
		return std::make_unique<DeviceClass>(mconfig, tag, owner, std::forward<Params>(args)...);
	}

	template <typename... Params> DeviceClass &operator()(machine_config &mconfig, char const *tag, Params &&... args) const;
	template <typename Exposed, bool Required, typename... Params> DeviceClass &operator()(machine_config &mconfig, device_finder<Exposed, Required> &finder, Params &&... args) const;
	template <typename... Params> DeviceClass &operator()(machine_config_replace replace, char const *tag, Params &&... args) const;
	template <typename Exposed, bool Required, typename... Params> DeviceClass &operator()(machine_config_replace replace, device_finder<Exposed, Required> &finder, Params &&... args) const;
};


inline device_registrar::const_iterator &device_registrar::const_iterator::operator++() { m_type = m_type->m_next; return *this; }

} // namespace emu::detail


// device types
typedef emu::detail::device_type_impl_base const &device_type;
typedef std::add_pointer_t<device_type> device_type_ptr;
extern emu::detail::device_registrar const registered_device_types;

template <
		typename DeviceClass,
		char const *ShortName,
		char const *FullName,
		char const *Source>
constexpr auto device_creator = &emu::detail::device_tag_func<DeviceClass, ShortName, FullName, Source>;

template <
		typename DriverClass,
		char const *ShortName,
		char const *FullName,
		char const *Source,
		emu::detail::device_feature::type Unemulated,
		emu::detail::device_feature::type Imperfect>
constexpr auto driver_device_creator = &emu::detail::driver_tag_func<DriverClass, ShortName, FullName, Source, Unemulated, Imperfect>;


/// \addtogroup machinedef
/// \{

/// \brief Declare a device type
///
/// Declares a device type where the exposed device class is in the
/// global namespace.  Must be used in the global namespace.
///
/// In addition to declaring the device type itself, a forward
/// declaration for the exposed device class is generated, and automatic
/// instantiation of device finder templates for the exposed device
/// class is suppressed.
/// \param Type The device type name (an identifier).  By convention,
///   these start with an uppercase letter and consist only of uppercase
///   letters and underscores.
/// \param Class The exposed device class name.  Must be the device
///   implementation class, or a public base of it, and must be derived
///   from #device_t or #device_interface.
/// \sa DECLARE_DEVICE_TYPE_NS DEFINE_DEVICE_TYPE
///   DEFINE_DEVICE_TYPE_PRIVATE
#define DECLARE_DEVICE_TYPE(Type, Class) \
		class Class; \
		extern emu::detail::device_type_impl<Class> const Type; \
		extern template class device_finder<Class, false>; \
		extern template class device_finder<Class, true>;


/// \brief Declare a device type for a class in a namespace
///
/// Declares a device type where the exposed device class is not in the
/// global namespace.  Must be used in the global namespace.
///
/// In addition to declaring the device type itself, a forward
/// declaration for the exposed device class is generated, and automatic
/// instantiation of device finder templates for the exposed device
/// class is suppressed.
/// \param Type The device type name (an identifier).  By convention,
///   these start with an uppercase letter and consist only of uppercase
///   letters and underscores.
/// \param Namespace The fully qualified name of the namespace
///   containing the exposed device class.
/// \param Class The exposed device class name, without namespace
///   qualifiers.  Must be the device implementation class, or a public
///   base of it, and must be derived from #device_t or
///   #device_interface.
/// \sa DECLARE_DEVICE_TYPE DEFINE_DEVICE_TYPE
///   DEFINE_DEVICE_TYPE_PRIVATE
#define DECLARE_DEVICE_TYPE_NS(Type, Namespace, Class) \
		namespace Namespace { class Class; } \
		extern emu::detail::device_type_impl<Namespace::Class> const Type; \
		extern template class device_finder<Namespace::Class, false>; \
		extern template class device_finder<Namespace::Class, true>;


/// \brief Define a device type
///
/// Defines a device type where the exposed device class is the same as
/// the device implementation class.  Must be used in the global
/// namespace.
///
/// As well as defining the device type, device finder templates are
/// instantiated for the device class.
/// \param Type The device type name (an identifier).  By convention,
///   these start with an uppercase letter and consist only of uppercase
///   letters and underscores.
/// \param Class The device implementation class name.  Must be the same
///   as the exposed device class, and must be derived from #device_t.
/// \param ShortName The short name of the device, used for
///   identification, and in filesystem paths for assets and data.  Must
///   be a string no longer than thirty-two characters, containing only
///   ASCII lowercase letters, digits and underscores.  Must be globally
///   unique across systems and devices.
/// \param FullName Display name for the device.  Must be a string, and
///   must be globally unique across systems and devices.
/// \sa DECLARE_DEVICE_TYPE DECLARE_DEVICE_TYPE_NS
///   DEFINE_DEVICE_TYPE_PRIVATE
#define DEFINE_DEVICE_TYPE(Type, Class, ShortName, FullName) \
		namespace { \
			struct Type##_device_traits { static constexpr char const shortname[] = ShortName, fullname[] = FullName, source[] = __FILE__; }; \
			constexpr char const Type##_device_traits::shortname[], Type##_device_traits::fullname[], Type##_device_traits::source[]; \
		} \
		emu::detail::device_type_impl<Class> const Type = device_creator<Class, (Type##_device_traits::shortname), (Type##_device_traits::fullname), (Type##_device_traits::source)>; \
		template class device_finder<Class, false>; \
		template class device_finder<Class, true>;


/// \brief Define a device type with a private implementation class
///
/// Defines a device type where the exposed device class is a base of
/// the device implementation class.  Must be used in the global
/// namespace.
///
/// Device finder templates are not instantiated for the exposed device
/// class.  This must be done explicitly in a single location for the
/// project.
/// \param Type The device type name (an identifier).  By convention,
///   these start with an uppercase letter and consist only of uppercase
///   letters and underscores.
/// \param Base The fully-qualified exposed device class name.  Must be
///   a public base of the device implementation class, and must be
///   derived from #device_t or #device_interface.
/// \param Class The fully-qualified device implementation class name.
///   Must be derived from the exposed device class, and indirectly from
///   #device_t.
/// \param ShortName The short name of the device, used for
///   identification, and in filesystem paths for assets and data.  Must
///   be a string no longer than thirty-two characters, containing only
///   ASCII lowercase letters, digits and underscores.  Must be globally
///   unique across systems and devices.
/// \param FullName Display name for the device.  Must be a string, and
///   must be globally unique across systems and devices.
/// \sa DECLARE_DEVICE_TYPE DECLARE_DEVICE_TYPE_NS DEFINE_DEVICE_TYPE
#define DEFINE_DEVICE_TYPE_PRIVATE(Type, Base, Class, ShortName, FullName) \
		namespace { \
			struct Type##_device_traits { static constexpr char const shortname[] = ShortName, fullname[] = FullName, source[] = __FILE__; }; \
			constexpr char const Type##_device_traits::shortname[], Type##_device_traits::fullname[], Type##_device_traits::source[]; \
		} \
		emu::detail::device_type_impl<Base> const Type = device_creator<Class, (Type##_device_traits::shortname), (Type##_device_traits::fullname), (Type##_device_traits::source)>;

/// \}


/// \brief Start order dependencies not satisfied exception
///
/// May be thrown from the start member functions of #device_t and
/// #device_interface implementations if start order dependencies have
/// not been satisfied.  MAME will start additional devices before
/// reattempting to start the device that threw the exception.
/// \sa device_t::device_start device_interface::interface_pre_start
class device_missing_dependencies : public emu_exception { };


/// \brief Base class for devices
///
/// The base class for all device implementations in MAME's modular
/// architecture.
class device_t : public delegate_late_bind
{
	DISABLE_COPYING(device_t);

	friend class simple_list<device_t>;
	friend class running_machine;
	friend class finder_base;
	friend class devcb_base;

	class subdevice_list
	{
		friend class device_t;
		friend class machine_config;

	public:
		// construction/destruction
		subdevice_list() { }

		// getters
		device_t *first() const { return m_list.first(); }
		int count() const { return m_list.count(); }
		bool empty() const { return m_list.empty(); }

		// range iterators
		using auto_iterator = simple_list<device_t>::auto_iterator;
		auto_iterator begin() const { return m_list.begin(); }
		auto_iterator end() const { return m_list.end(); }

	private:
		// private helpers
		device_t &append(std::unique_ptr<device_t> &&device);
		device_t &replace_and_remove(std::unique_ptr<device_t> &&device, device_t &existing);
		void remove(device_t &device);
		device_t *find(std::string_view name) const
		{
			auto result = m_tagmap.find(name);
			if (result != m_tagmap.end())
				return &result->second.get();
			else
				return nullptr;
		}

		// private state
		simple_list<device_t>   m_list;         // list of sub-devices we own
		std::unordered_map<std::string_view, std::reference_wrapper<device_t>> m_tagmap;      // map of devices looked up and found by subtag
	};

	class interface_list
	{
		friend class device_t;
		friend class device_interface;
		friend class device_memory_interface;
		friend class device_execute_interface;

	public:
		class auto_iterator
		{
		public:
			typedef std::ptrdiff_t difference_type;
			typedef device_interface value_type;
			typedef device_interface *pointer;
			typedef device_interface &reference;
			typedef std::forward_iterator_tag iterator_category;

			// construction/destruction
			auto_iterator(device_interface *intf) : m_current(intf) { }

			// required operator overloads
			bool operator==(const auto_iterator &iter) const { return m_current == iter.m_current; }
			bool operator!=(const auto_iterator &iter) const { return m_current != iter.m_current; }
			device_interface &operator*() const { return *m_current; }
			device_interface *operator->() const { return m_current; }
			auto_iterator &operator++();
			auto_iterator operator++(int);

		private:
			// private state
			device_interface *m_current;
		};

		// construction/destruction
		interface_list() : m_head(nullptr), m_execute(nullptr), m_memory(nullptr) { }

		// getters
		device_interface *first() const { return m_head; }

		// range iterators
		auto_iterator begin() const { return auto_iterator(m_head); }
		auto_iterator end() const { return auto_iterator(nullptr); }

	private:
		device_interface *m_head;               // head of interface list
		device_execute_interface *m_execute;    // pre-cached pointer to execute interface
		device_memory_interface *m_memory;      // pre-cached pointer to memory interface
	};

protected:
	// construction/destruction
	device_t(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			u32 clock);

public:
	// device flags
	using feature = emu::detail::device_feature;
	using feature_type = emu::detail::device_feature::type;

	/// \brief Report unemulated features
	///
	/// Implement this member in a derived class to declare features
	/// that are not emulated.  This will propagate to all other devices
	/// and systems that use the device.  Unemulated features are shown
	/// in the system selection UI, and cause a red warning to be
	/// displayed on starting a system.
	/// \return Bitwise or of the feature constants for unemulated
	///   features of the device.
	/// \sa imperfect_features
	static constexpr feature_type unemulated_features() { return feature::NONE; }

	/// \brief Report imperfectly emulated features
	///
	/// Implement this member in a derived class to declare features
	/// that are imperfectly emulated.  This will propagate to all other
	/// devices and systems that use the device.  Imperfectly emulated
	/// features are shown in the system selection UI, and cause a
	/// yellow warning to be displayed on starting a system (provided
	/// there are no unemulated features, which take precedence and
	/// cause the warning to be red).
	///
	/// An exception is imperfectly emulated protection, which results
	/// in a red warning being displayed when starting a system.
	/// \return Bitwise or of the feature constants for imperfectly
	///   emulated features of the device.
	/// \sa unemulated_features
	static constexpr feature_type imperfect_features() { return feature::NONE; }

	/// \brief Get parent device type for ROM search
	///
	/// Implement this member in a derived class to declare the parent
	/// device type for the purpose of searching for ROMs.  Only one
	/// level is allowed.  It is an error if the parent device type
	/// itself declares a parent device type.
	/// \return Pointer to parent device type, or nullptr.
	static auto parent_rom_device_type() { return nullptr; }

	virtual ~device_t();

	// getters
	bool has_running_machine() const { return m_machine != nullptr; }
	running_machine &machine() const { /*assert(m_machine != nullptr);*/ return *m_machine; }
	const char *tag() const { return m_tag.c_str(); }
	const char *basetag() const { return m_basetag.c_str(); }
	device_type type() const { return m_type; }
	const char *name() const { return m_type.fullname(); }
	const char *shortname() const { return m_type.shortname(); }
	virtual std::vector<std::string> searchpath() const;
	const char *source() const { return m_type.source(); }
	device_t *owner() const { return m_owner; }
	device_t *next() const { return m_next; }
	u32 configured_clock() const { return m_configured_clock; }
	const machine_config &mconfig() const { return m_machine_config; }
	const input_device_default *input_ports_defaults() const { return m_input_defaults; }
	const std::vector<rom_entry> &rom_region_vector() const;
	const tiny_rom_entry *rom_region() const { return device_rom_region(); }
	ioport_constructor input_ports() const { return device_input_ports(); }
	std::string const &get_default_bios_tag() const { return m_default_bios_tag; }
	u8 default_bios() const { assert(configured()); return m_default_bios; }
	u8 system_bios() const { return m_system_bios; }

	// interface helpers
	interface_list &interfaces() { return m_interfaces; }
	const interface_list &interfaces() const { return m_interfaces; }
	template<class DeviceClass> bool interface(DeviceClass *&intf) { intf = dynamic_cast<DeviceClass *>(this); return (intf != nullptr); }
	template<class DeviceClass> bool interface(DeviceClass *&intf) const { intf = dynamic_cast<const DeviceClass *>(this); return (intf != nullptr); }

	// specialized helpers for common core interfaces
	bool interface(device_execute_interface *&intf) { intf = m_interfaces.m_execute; return (intf != nullptr); }
	bool interface(device_execute_interface *&intf) const { intf = m_interfaces.m_execute; return (intf != nullptr); }
	bool interface(device_memory_interface *&intf) { intf = m_interfaces.m_memory; return (intf != nullptr); }
	bool interface(device_memory_interface *&intf) const { intf = m_interfaces.m_memory; return (intf != nullptr); }
	device_execute_interface &execute() const { assert(m_interfaces.m_execute != nullptr); return *m_interfaces.m_execute; }
	device_memory_interface &memory() const { assert(m_interfaces.m_memory != nullptr); return *m_interfaces.m_memory; }

	// owned object helpers
	subdevice_list &subdevices() { return m_subdevices; }
	const subdevice_list &subdevices() const { return m_subdevices; }

	// device-relative tag lookups
	std::string subtag(std::string_view tag) const;
	std::string siblingtag(std::string_view tag) const { return (m_owner != nullptr) ? m_owner->subtag(tag) : std::string(tag); }
	memory_region *memregion(std::string_view tag) const;
	memory_share *memshare(std::string_view tag) const;
	memory_bank *membank(std::string_view tag) const;
	ioport_port *ioport(std::string_view tag) const;
	device_t *subdevice(std::string_view tag) const;
	device_t *siblingdevice(std::string_view tag) const;
	template<class DeviceClass> DeviceClass *subdevice(std::string_view tag) const { return downcast<DeviceClass *>(subdevice(tag)); }
	template<class DeviceClass> DeviceClass *siblingdevice(std::string_view tag) const { return downcast<DeviceClass *>(siblingdevice(tag)); }
	std::string parameter(std::string_view tag) const;

	// configuration helpers
	void add_machine_configuration(machine_config &config);
	void set_clock(u32 clock);
	void set_clock(const XTAL &xtal) { set_clock(xtal.value()); }
	void set_input_default(const input_device_default *config) { m_input_defaults = config; }
	template <typename... Params> void set_default_bios_tag(Params &&... args) { assert(!configured()); m_default_bios_tag.assign(std::forward<Params>(args)...); }

	// state helpers
	void config_complete();
	bool configured() const { return m_config_complete; }
	void validity_check(validity_checker &valid) const;
	bool started() const { return m_started; }
	void reset();

	// clock/timing accessors
	u32 clock() const { return m_clock; }
	u32 unscaled_clock() const { return m_unscaled_clock; }
	void set_unscaled_clock(u32 clock, bool sync_on_new_clock_domain = false);
	void set_unscaled_clock(const XTAL &xtal, bool sync_on_new_clock_domain = false) { set_unscaled_clock(xtal.value(), sync_on_new_clock_domain); }
	void set_unscaled_clock_int(u32 clock) { set_unscaled_clock(clock, false); } // non-overloaded name because binding to overloads is ugly
	void set_unscaled_clock_int_sync(u32 clock) { set_unscaled_clock(clock, true); } // non-overloaded name because binding to overloads is ugly
	double clock_scale() const { return m_clock_scale; }
	void set_clock_scale(double clockscale);
	attotime clocks_to_attotime(u64 clocks) const noexcept;
	u64 attotime_to_clocks(const attotime &duration) const noexcept;

	// timer interfaces
	template <typename... T> emu_timer *timer_alloc(T &&... args);

	/// \brief Register data for save states
	///
	/// Registers data to be automatically saved/restored.  Can be used
	/// with fixed-sized integer types, enumerated types marked safe for
	/// saving (see #ALLOW_SAVE_TYPE and #ALLOW_SAVE_TYPE_AND_VECTOR).
	/// Supports C arrays, \c std::array and \c std::vector.  Note that
	/// \c std::vector instances must not be resized after being
	/// registered to be saved/restored.
	/// \param value Reference to the data to be saved/restored.  The
	///   \c NAME macro can be used to simplify specifying the
	///   \p valname argument at the same time.
	/// \param [in] valname The name of the saved item.  The combination
	///   of the \p valname and \p index arguments must be unique across
	///   saved items for a device.
	/// \param [in] index A numeric value to distinguish between saved
	///   items with the same name.
	template <typename ItemType>
	void ATTR_COLD save_item(ItemType &value, const char *valname, int index = 0)
	{
		assert(m_save);
		m_save->save_item(this, name(), tag(), index, value, valname);
	}

	/// \brief Register a member of a structure in an array for save
	///   states
	///
	/// Registers data to be automatically saved/restored.  Can be used
	/// with fixed-sized integer types, enumerated types marked safe for
	/// saving (see #ALLOW_SAVE_TYPE and #ALLOW_SAVE_TYPE_AND_VECTOR).
	/// Used to allow saving/restoring members of structures in C arrays
	/// or \c std::array instances.
	/// \param value Reference to the array of structures containing the
	///   member to be saved/restored.  The #STRUCT_MEMBER macro can be
	///   used to simplify specifying the \p element and \p valname
	///   arguments at the same time.
	/// \param [in] element Pointer to the member of the structure to
	///   save/restore.
	/// \param [in] valname The name of the saved item.  The combination
	///   of the \p valname and \p index arguments must be unique across
	///   saved items for a device.
	/// \param [in] index A numeric value to distinguish between saved
	///   items with the same name.
	template <typename ItemType, typename StructType, typename ElementType>
	void ATTR_COLD save_item(ItemType &value, ElementType StructType::*element, const char *valname, int index = 0)
	{
		assert(m_save);
		m_save->save_item(this, name(), tag(), index, value, element, valname);
	}

	/// \brief Register an array of indeterminate for save states
	///
	/// Registers data to be automatically saved/restored when the
	/// length of the outermost array cannot be automatically determined
	/// at compile time.  Can be used with C arrays of indeterminate
	/// length, pointers, and \c std::unique_ptr instances pointing to
	/// arrays.  Use #save_item if length of the array can be determined
	/// at compile time.
	/// \param value Pointer to the array containing the data to be
	///   saved/restored.  The \c NAME macro can be used to simplify
	///   specifying the \p valname argument at the same time.
	/// \param [in] valname The name of the saved item.  The combination
	///   of the \p valname and \p index arguments must be unique across
	///   saved items for a device.
	/// \param [in] count The number of elements in the outermost array.
	/// \param [in] index A numeric value to distinguish between saved
	///   items with the same name.
	/// \sa save_item
	template<typename ItemType>
	void ATTR_COLD save_pointer(ItemType &&value, const char *valname, u32 count, int index = 0)
	{
		assert(m_save);
		m_save->save_pointer(this, name(), tag(), index, std::forward<ItemType>(value), valname, count);
	}

	/// \brief Register a member of a structure in an array of
	///   indeterminate for save states
	///
	/// Registers data to be automatically saved/restored when the
	/// length of the outermost array cannot be automatically determined
	/// at compile time.  Can be used with C arrays of indeterminate
	/// length, pointers, and \c std::unique_ptr instances pointing to
	/// arrays.  Use #save_item if length of the array can be determined
	/// at compile time.
	/// \param value Pointer to the array of structures containing the
	///   member to be saved/restored.  The #STRUCT_MEMBER macro can be
	///   used to simplify specifying the \p element and \p valname
	///   arguments at the same time.
	/// \param [in] element Pointer to the member of the structure to
	///   save/restore.
	/// \param [in] valname The name of the saved item.  The combination
	///   of the \p valname and \p index arguments must be unique across
	///   saved items for a device.
	/// \param [in] count The number of elements in the outermost array.
	/// \param [in] index A numeric value to distinguish between saved
	///   items with the same name.
	/// \sa save_item
	template<typename ItemType, typename StructType, typename ElementType>
	void ATTR_COLD save_pointer(ItemType &&value, ElementType StructType::*element, const char *valname, u32 count, int index = 0)
	{
		assert(m_save);
		m_save->save_pointer(this, name(), tag(), index, std::forward<ItemType>(value), element, valname, count);
	}

	// debugging
	device_debug *debug() const { return m_debug.get(); }

	void set_system_bios(u8 bios) { m_system_bios = bios; }
	bool findit(validity_checker *valid) const;

	// misc
	template <typename Format, typename... Params> void popmessage(Format &&fmt, Params &&... args) const;
	template <typename Format, typename... Params> void logerror(Format &&fmt, Params &&... args) const;
	void view_register(memory_view *view);

protected:
	// miscellaneous helpers
	void set_machine(running_machine &machine);
	void resolve_pre_map();
	void resolve_post_map();
	void start();
	void stop();
	void debug_setup();
	void pre_save();
	void post_load();
	void notify_clock_changed(bool sync_on_new_clock_domain = false);
	finder_base *register_auto_finder(finder_base &autodev);
	void register_callback(devcb_base &callback);

	//------------------- begin derived class overrides

	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const;
	virtual void device_add_mconfig(machine_config &config);
	virtual ioport_constructor device_input_ports() const;

	/// \brief Finalise device configuration
	///
	/// Perform any final configuration tasks after all devices in the
	/// system have added machine configuration.  This is called after
	/// any #device_interface mix-in \c interface_config_complete
	/// members have completed.
	///
	/// Note that automatic object finders will not have been resolved
	/// at the time this member is called.
	virtual void device_config_complete();

	/// \brief Additional device validity checks
	///
	/// Implement this member to provide additional validity checks.
	/// Report errors using #osd_printf_error and report warnings using
	/// #osd_printf_warning.  The system being validated, device type
	/// and device tag are collected automatically.  Do not throw
	/// exceptions to report errors.
	///
	/// This provides an opportunity to check that the device has been
	/// configured correctly.  Systems are validated on start, and also
	/// when the user manually runs a validity check.  Validity checks
	/// are only run for devices configured in runnable systems, not
	/// when checking that a device can be instantiated in isolation.
	/// \param [in] valid Reference to the validity checker object
	///   performing validation (provides some helper member functions).
	/// \sa device_interface::interface_validity_check
	virtual void device_validity_check(validity_checker &valid) const ATTR_COLD;

	/// \brief Resolve objects that may be needed while starting
	///
	/// Implement this member to complete object resolution before any
	/// devices are started.  For example it may be necessary to resolve
	/// callbacks before any devices start so initial input conditions
	/// can be set.  This is called after all registered automatic
	/// object finders are resolved.
	virtual void device_resolve_objects() ATTR_COLD;

	/// \brief Device start handler
	///
	/// Implement this member to set up the initial state of the device
	/// on start.  This will be called after all #device_interface
	/// mix-in \c interface_pre_start members have completed
	/// successfully.  If the device can't start until another device
	/// has completed starting, throw a #device_missing_dependencies
	/// exception.  Starting will be postponed until additional devices
	/// have started.
	///
	/// If a device's direct base class is not #device_t, it's good
	/// practice to check start order dependencies (and throw
	/// #device_missing_dependencies if necessary) before calling the
	/// base implementation.  This will ensure that the base
	/// implementation won't be called twice if starting needs to be
	/// postponed.
	///
	/// This is the correct place to register for save states.
	/// \sa device_reset device_stop save_item save_pointer
	///   device_interface::interface_pre_start
	///   device_interface::interface_post_start
	virtual void device_start() ATTR_COLD = 0;

	/// \brief Device stop handler
	///
	/// Implement this member to perform additional tasks on ending an
	/// emulation session.  You may deallocate memory here.  This is
	/// called after \c interface_pre_stop is called for all
	/// #device_interface mix-ins, and before \c interface_post_stop is
	/// called for any #device_interface mix-ins.
	/// \sa device_interface::interface_pre_stop
	///   device_interface::interface_post_stop
	virtual void device_stop() ATTR_COLD;

	/// \brief Device reset handler
	///
	/// Implement this member to provide reset behaviour.  This is
	/// called after all #device_interface mix-in \c interface_pre_reset
	/// members have completed, and before any child devices are reset.
	/// All devices are reset at the beginning of an emulation session
	/// (after all devices have been started), and also when the user
	/// requests a soft reset (by pressing F3 by default, and also
	/// available from the debugger).
	///
	/// Note that child devices are reset automatically when a device is
	/// reset.  You should not reset child devices manually from this
	/// member.  If you need to provide additional behaviour after child
	/// devices are reset, implement #device_reset_after_children.
	///
	/// Only implement warm reset behaviour in this member.  Initial
	/// cold reset conditions should be set up in #device_start.
	/// \sa device_reset_after_children device_start
	///   device_interface::interface_pre_reset
	///   device_interface::interface_post_reset
	virtual void device_reset() ATTR_COLD;

	/// \brief Additional reset behaviour after child device reset
	///
	/// Implement this member to provide additional reset behaviour
	/// after child devices are reset.  This is called when resetting a
	/// device after #device_reset has been called and all child devices
	/// have been reset, and before any #device_interface mix-in
	/// \c interface_post_reset members are called.
	/// \sa device_reset device_interface::interface_pre_reset
	///   device_interface::interface_post_reset
	virtual void device_reset_after_children() ATTR_COLD;

	/// \brief Prepare for a save state to be written
	///
	/// Implement this member to perform any tasks necessary before any
	/// registered save state items are recorded.  For example it may be
	/// necessary to flush caches, serialise self-referencing members or
	/// pointers into data structures.  This is called after all
	/// #device_interface mix-in \c interface_pre_save members are
	/// called.
	/// \sa device_post_load save_item save_pointer
	///   device_interface::interface_pre_save
	virtual void device_pre_save() ATTR_COLD;

	/// \brief Complete save state loading
	///
	/// Implement this member to perform any tasks necessary after
	/// registered save state items are loaded.  For example it may be
	/// necessary to update or invalidate caches, or de-serialise
	/// pointers into data structures.  This is called after all
	/// #device_interface mix-in \c interface_post_load members are
	/// called.
	/// \sa device_pre_save save_item save_pointer
	///   device_interface::interface_post_load
	virtual void device_post_load() ATTR_COLD;

	virtual void device_clock_changed();
	virtual void device_debug_setup();

	//------------------- end derived class overrides

	// core device properties
	device_type             m_type;                 // device type

	// device relationships & interfaces
	device_t *              m_owner;                // device that owns us
	device_t *              m_next;                 // next device by the same owner (of any type/class)
	subdevice_list          m_subdevices;           // container for list of subdevices
	interface_list          m_interfaces;           // container for list of interfaces

	// device clocks
	u32                     m_configured_clock;     // originally configured device clock
	u32                     m_unscaled_clock;       // current unscaled device clock
	u32                     m_clock;                // current device clock, after scaling
	double                  m_clock_scale;          // clock scale factor
	attoseconds_t           m_attoseconds_per_clock;// period in attoseconds

	std::unique_ptr<device_debug> m_debug;
	const machine_config &  m_machine_config;       // reference to the machine's configuration
	const input_device_default *m_input_defaults;   // devices input ports default overrides

	u8                      m_system_bios;          // the system BIOS we wish to load
	u8                      m_default_bios;         // the default system BIOS
	std::string             m_default_bios_tag;     // tag of the default system BIOS

private:
	// internal helpers
	device_t *subdevice_slow(std::string_view tag) const;
	void calculate_derived_clock();

	// private state; accessor use required
	running_machine *       m_machine;
	save_manager *          m_save;
	std::string             m_tag;                  // full tag for this instance
	std::string             m_basetag;              // base part of the tag
	bool                    m_config_complete;      // have we completed our configuration?
	bool                    m_started;              // true if the start function has succeeded
	finder_base *           m_auto_finder_list;     // list of objects to auto-find
	mutable std::vector<rom_entry>  m_rom_entries;
	std::list<devcb_base *> m_callbacks;
	std::vector<memory_view *> m_viewlist;          // list of views

	// string formatting buffer for logerror
	mutable util::ovectorstream m_string_buffer;
};


/// \brief Device mix-in base
///
/// Provides a base for #device_t mix-ins that integrate with the device
/// lifecycle.  Derived classes are used to implement a number of
/// standard concepts and interfaces, and integrate with the scheduler,
/// debugger and user interface.
class device_interface
{
	DISABLE_COPYING(device_interface);

protected:
	// construction/destruction
	device_interface(device_t &device, const char *type);
	virtual ~device_interface();

public:
	const char *interface_type() const { return m_type; }

	// casting helpers
	device_t &device() { return m_device; }
	const device_t &device() const { return m_device; }
	operator device_t &() { return m_device; }

	// iteration helpers
	device_interface *interface_next() const { return m_interface_next; }

	// optional operation overrides

	/// \brief Finalise mix-in configuration
	///
	/// Perform any final configuration tasks after all devices in the
	/// system have added machine configuration.  This is called before
	/// \c device_config_complete is called for the device.
	///
	/// Note that automatic object finders will not have been resolved
	/// at this time.
	/// \sa device_t::device_config_complete
	virtual void interface_config_complete();

	/// \brief Additional mix-in validity checks
	///
	/// Implement this member to provide additional validity checks.
	/// Report errors using #osd_printf_error and report warnings using
	/// #osd_printf_warning.  The system being validated, device type
	/// and device tag are collected automatically.  Do not throw
	/// exceptions to report errors.
	///
	/// This provides an opportunity to check that the mix-in has been
	/// configured correctly.  Systems are validated on start, and also
	/// when the user manually runs a validity check.  Validity checks
	/// are only run for devices configured in runnable systems, not
	/// when checking that a device can be instantiated in isolation.
	/// \param [in] valid Reference to the validity checker object
	///   performing validation (provides some helper member functions).
	/// \sa device_t::device_validity_check
	virtual void interface_validity_check(validity_checker &valid) const ATTR_COLD;

	/// \brief Mix-in start handler
	///
	/// Implement this member to set up the initial state of the mix-in
	/// on start.  This is called before the device_start member is
	/// called for the device.  If the mix-in can't be started until
	/// another device has started, throw a #device_missing_dependencies
	/// exception.  Starting will be postponed until additional devices
	/// have been started.
	///
	/// Note that this member may be called multiple times if another
	/// \c device_interface mix-in throws a #device_missing_dependencies
	/// exception from its \c interface_pre_start member, or if the
	/// device throws a #device_missing_dependencies exception from its
	/// \c device_start member.  You must check to ensure that
	/// operations like resource allocation are not performed multiple
	/// times, or postpone them until #interface_post_start is called.
	///
	/// It's simpler to register for save states when
	/// #interface_post_start is called.
	/// \sa interface_post_start device_t::device_start
	virtual void interface_pre_start() ATTR_COLD;

	/// \brief Mix-in start completion handler
	///
	/// Implement this member to complete mix-in start-up.  This is
	/// called after #interface_pre_start is called for all
	/// device_interface mix-ins, and after \c device_start is called
	/// for the device.  This member will only be called once, it will
	/// not be called multiple times if device starting is postponed.
	///
	/// This member must not throw #device_missing_dependencies (start
	/// order dependencies should be checked in #interface_pre_start).
	/// This is the appropriate place to allocate resources like
	/// timers and register for save states.
	/// \sa interface_pre_start device_t::device_start
	///   device_t::save_item device_t::save_pointer
	virtual void interface_post_start() ATTR_COLD;

	/// \brief Mix-in reset handler
	///
	/// Implement this member to provide reset behaviour.  This is
	/// called before \c device_reset is called for the device, and
	/// before any child devices are reset.  Only implement warm reset
	/// behaviour in this member.  Initial cold reset conditions should
	/// be set up in #interface_pre_start and/or #interface_post_start.
	/// If you need to provide additional behaviour after child devices
	/// are reset, implement #interface_post_reset.
	/// \sa interface_post_reset device_t::device_reset
	virtual void interface_pre_reset() ATTR_COLD;

	/// \brief Mix-in reset completion handler
	///
	/// Implement this member to provide additional reset behaviour
	/// after child devices are reset.  This is called after
	/// \c device_reset_after_children has been called for the device.
	/// \sa interface_pre_reset device_t::device_reset
	///   device_t::device_reset_after_children
	virtual void interface_post_reset() ATTR_COLD;

	/// \brief Mix-in stop handler
	///
	/// Implement this member to perform additional tasks on ending an
	/// emulation session.  Do not deallocate anything that may need to
	/// be referenced from another device_interface mix-in's
	/// \c interface_pre_stop member or from the device's \c device_stop
	/// member.  This is called before device_stop is called for the
	/// device.
	/// \sa interface_post_stop device_t::device_stop
	virtual void interface_pre_stop() ATTR_COLD;

	/// \brief Mix-in stop completion handler
	///
	/// Implement this member to perform additional tasks on ending an
	/// emulation session after the device is stopped.  You can
	/// deallocate memory here.  This is called after \c device_stop is
	/// called for the device.
	/// \sa interface_pre_stop device_t::device_stop
	virtual void interface_post_stop() ATTR_COLD;

	/// \brief Prepare for a save state to be written
	///
	/// Implement this member to perform any tasks necessary before any
	/// registered save state items are recorded.  For example it may be
	/// necessary to flush caches, serialise self-referencing members or
	/// pointers into data structures.  This is called before
	/// \c device_pre_save is called for the device.
	/// \sa interface_post_load device_t::device_pre_save
	virtual void interface_pre_save() ATTR_COLD;

	/// \brief Complete save state loading
	///
	/// Implement this member to perform any tasks necessary after
	/// registered save state items are loaded.  For example it may be
	/// necessary to update or invalidate caches, or de-serialise
	/// pointers into data structures.  This is called before
	/// \c device_post_load is called for the device.
	/// \sa interface_pre_save device_t::device_post_load
	virtual void interface_post_load() ATTR_COLD;

	virtual void interface_clock_changed(bool sync_on_new_clock_domain);
	virtual void interface_debug_setup();

private:
	// internal state
	device_interface *      m_interface_next;
	device_t &              m_device;
	const char *            m_type;
};


// ======================> device_enumerator

// helper class to iterate over the hierarchy of devices depth-first
class device_enumerator
{
public:
	class iterator
	{
	public:
		typedef std::ptrdiff_t difference_type;
		typedef device_t value_type;
		typedef device_t *pointer;
		typedef device_t &reference;
		typedef std::forward_iterator_tag iterator_category;

		// construction
		iterator(device_t *devptr, int curdepth, int maxdepth)
			: m_curdevice(devptr)
			, m_curdepth(curdepth)
			, m_maxdepth(maxdepth)
		{
		}

		// getters
		device_t *current() const { return m_curdevice; }
		int depth() const { return m_curdepth; }

		// required operator overrides
		bool operator==(iterator const &iter) const { return m_curdevice == iter.m_curdevice; }
		bool operator!=(iterator const &iter) const { return m_curdevice != iter.m_curdevice; }
		device_t &operator*() const { assert(m_curdevice); return *m_curdevice; }
		device_t *operator->() const { return m_curdevice; }
		iterator &operator++() { advance(); return *this; }
		iterator operator++(int) { iterator const result(*this); ++*this; return result; }

	protected:
		// search depth-first for the next device
		void advance()
		{
			// remember our starting position, and end immediately if we're nullptr
			if (m_curdevice)
			{
				device_t *start = m_curdevice;

				// search down first
				if (m_curdepth < m_maxdepth)
				{
					m_curdevice = start->subdevices().first();
					if (m_curdevice)
					{
						m_curdepth++;
						return;
					}
				}

				// search next for neighbors up the ownership chain
				while (m_curdepth > 0 && start)
				{
					// found a neighbor? great!
					m_curdevice = start->next();
					if (m_curdevice)
						return;

					// no? try our parent
					start = start->owner();
					m_curdepth--;
				}

				// returned to the top; we're done
				m_curdevice = nullptr;
			}
		}

		// protected state
		device_t *  m_curdevice;
		int         m_curdepth;
		int         m_maxdepth;
	};

	// construction
	device_enumerator(device_t &root, int maxdepth = 255) : m_root(root), m_maxdepth(maxdepth) { }

	// standard iterators
	iterator begin() const { return iterator(&m_root, 0, m_maxdepth); }
	iterator end() const { return iterator(nullptr, 0, m_maxdepth); }

	// return first item
	device_t *first() const { return begin().current(); }

	// return the number of items available
	int count() const
	{
		int result = 0;
		for (device_t &item : *this)
		{
			(void)&item;
			result++;
		}
		return result;
	}

	// return the index of a given item in the virtual list
	int indexof(device_t &device) const
	{
		int index = 0;
		for (device_t &item : *this)
		{
			if (&item == &device)
				return index;
			else
				index++;
		}
		return -1;
	}

	// return the indexed item in the list
	device_t *byindex(int index) const
	{
		for (device_t &item : *this)
			if (index-- == 0)
				return &item;
		return nullptr;
	}

private:
	// internal state
	device_t &  m_root;
	int         m_maxdepth;
};


// ======================> device_type_enumerator

// helper class to find devices of a given type in the device hierarchy
template <class DeviceType, class DeviceClass = DeviceType>
class device_type_enumerator
{
public:
	class iterator : protected device_enumerator::iterator
	{
	public:
		using device_enumerator::iterator::difference_type;
		using device_enumerator::iterator::iterator_category;
		using device_enumerator::iterator::depth;

		typedef DeviceClass value_type;
		typedef DeviceClass *pointer;
		typedef DeviceClass &reference;

		// construction
		iterator(device_t *devptr, int curdepth, int maxdepth) : device_enumerator::iterator(devptr, curdepth, maxdepth)
		{
			// make sure the first device is of the specified type
			while (m_curdevice && (m_curdevice->type().type() != typeid(DeviceType)))
				advance();
		}

		// required operator overrides
		bool operator==(iterator const &iter) const { return m_curdevice == iter.m_curdevice; }
		bool operator!=(iterator const &iter) const { return m_curdevice != iter.m_curdevice; }

		// getters returning specified device type
		DeviceClass *current() const { return downcast<DeviceClass *>(m_curdevice); }
		DeviceClass &operator*() const { assert(m_curdevice); return downcast<DeviceClass &>(*m_curdevice); }
		DeviceClass *operator->() const { return downcast<DeviceClass *>(m_curdevice); }

		// search for devices of the specified type
		iterator &operator++()
		{
			do { advance(); } while (m_curdevice && (m_curdevice->type().type() != typeid(DeviceType)));
			return *this;
		}

		iterator operator++(int) { iterator const result(*this); ++*this; return result; }
	};

	// construction
	device_type_enumerator(device_t &root, int maxdepth = 255) : m_root(root), m_maxdepth(maxdepth) { }

	// standard iterators
	iterator begin() const { return iterator(&m_root, 0, m_maxdepth); }
	iterator end() const { return iterator(nullptr, 0, m_maxdepth); }
	iterator cbegin() const { return iterator(&m_root, 0, m_maxdepth); }
	iterator cend() const { return iterator(nullptr, 0, m_maxdepth); }

	// return first item
	DeviceClass *first() const { return begin().current(); }

	// return the number of items available
	int count() const { return std::distance(cbegin(), cend()); }

	// return the index of a given item in the virtual list
	int indexof(DeviceClass &device) const
	{
		int index = 0;
		for (DeviceClass &item : *this)
		{
			if (&item == &device)
				return index;
			else
				index++;
		}
		return -1;
	}

	// return the indexed item in the list
	DeviceClass *byindex(int index) const
	{
		for (DeviceClass &item : *this)
			if (index-- == 0)
				return &item;
		return nullptr;
	}

private:
	// internal state
	device_t &      m_root;
	int             m_maxdepth;
};


// ======================> device_interface_enumerator

// helper class to find devices with a given interface in the device hierarchy
// also works for finding devices derived from a given subclass
template <class InterfaceClass>
class device_interface_enumerator
{
public:
	class iterator : public device_enumerator::iterator
	{
	public:
		// construction
		iterator(device_t *devptr, int curdepth, int maxdepth) : device_enumerator::iterator(devptr, curdepth, maxdepth)
		{
			// set the iterator for the first device with the interface
			find_interface();
		}

		// getters returning specified interface type
		InterfaceClass *current() const { return m_interface; }
		InterfaceClass &operator*() const { assert(m_interface != nullptr); return *m_interface; }

		// search for devices with the specified interface
		const iterator &operator++() { advance(); find_interface(); return *this; }
		iterator operator++(int) { iterator const result(*this); ++*this; return result; }

	private:
		// private helper
		void find_interface()
		{
			// advance until finding a device with the interface
			for ( ; m_curdevice; advance())
				if (m_curdevice->interface(m_interface))
					return;

			// if we run out of devices, make sure the interface pointer is null
			m_interface = nullptr;
		}

		// private state
		InterfaceClass *m_interface;
	};

	// construction
	device_interface_enumerator(device_t &root, int maxdepth = 255) : m_root(root), m_maxdepth(maxdepth) { }

	// standard iterators
	iterator begin() const { return iterator(&m_root, 0, m_maxdepth); }
	iterator end() const { return iterator(nullptr, 0, m_maxdepth); }

	// return first item
	InterfaceClass *first() const { return begin().current(); }

	// return the number of items available
	int count() const
	{
		int result = 0;
		for (InterfaceClass &item : *this)
		{
			(void)&item;
			result++;
		}
		return result;
	}

	// return the index of a given item in the virtual list
	int indexof(InterfaceClass &intrf) const
	{
		int index = 0;
		for (InterfaceClass &item : *this)
		{
			if (&item == &intrf)
				return index;
			else
				index++;
		}
		return -1;
	}

	// return the indexed item in the list
	InterfaceClass *byindex(int index) const
	{
		for (InterfaceClass &item : *this)
			if (index-- == 0)
				return &item;
		return nullptr;
	}

private:
	// internal state
	device_t &      m_root;
	int             m_maxdepth;
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  subdevice - given a tag, find the device by
//  name relative to this device
//-------------------------------------------------

inline device_t *device_t::subdevice(std::string_view tag) const
{
	// empty string means this device (DEVICE_SELF)
	if (tag.empty())
		return const_cast<device_t *>(this);

	// do a quick lookup and return that if possible
	auto quick = m_subdevices.m_tagmap.find(tag);
	return (quick != m_subdevices.m_tagmap.end()) ? &quick->second.get() : subdevice_slow(tag);
}


//-------------------------------------------------
//  siblingdevice - given a tag, find the device
//  by name relative to this device's parent
//-------------------------------------------------

inline device_t *device_t::siblingdevice(std::string_view tag) const
{
	// empty string means this device (DEVICE_SELF)
	if (tag.empty())
		return const_cast<device_t *>(this);

	// leading caret implies the owner, just skip it
	if (tag[0] == '^') tag.remove_prefix(1);

	// query relative to the parent, if we have one
	if (m_owner != nullptr)
		return m_owner->subdevice(tag);

	// otherwise, it's nullptr unless the tag is absolute
	return (!tag.empty() && tag[0] == ':') ? subdevice(tag) : nullptr;
}


// these operators requires device_interface to be a complete type
inline device_t::interface_list::auto_iterator &device_t::interface_list::auto_iterator::operator++()
{
	m_current = m_current->interface_next();
	return *this;
}

inline device_t::interface_list::auto_iterator device_t::interface_list::auto_iterator::operator++(int)
{
	auto_iterator result(*this);
	m_current = m_current->interface_next();
	return result;
}


#endif // MAME_EMU_DEVICE_H
