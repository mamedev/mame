// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/**
 * \file devfind.h
 * Object auto-discovery helpers
 * \defgroup devfind
 * \{
 * Object auto-discovery helpers
 */

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DEVFIND_H
#define MAME_EMU_DEVFIND_H

#pragma once

#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/// \brief Helper class to find arrays of devices, etc.
///
/// Useful when a machine/device has a number of similar subdevices, I/O
/// ports, memory regions, etc.  Template arguments are the element type
/// and number of elements in the array.  It's assumed that the element
/// can be constructed with a device_t reference and a C string tag.
template <typename T, unsigned Count>
class object_array_finder
{
private:
	template <typename F, typename... Param, unsigned... V>
	object_array_finder(device_t &base, F const &fmt, unsigned start, std::integer_sequence<unsigned, V...>, Param const &... arg)
		: m_tag{ util::string_format(fmt, start + V)... }
		, m_array{ { base, m_tag[V].c_str(), arg... }... }
	{
	}

	template <typename... Param, unsigned... V>
	object_array_finder(device_t &base, std::array<char const *, Count> const &tags, std::integer_sequence<unsigned, V...>, Param const &... arg)
		: m_array{ { base, tags[V], arg... }... }
	{
	}

	/// \brief Generated tag names
	///
	/// Finder objects do not copy the search tag supplied at
	/// construction.  Tags that are programmatically generated at
	/// construction are stored here so they persist until resolution
	/// time (and beyond).
	std::string const m_tag[Count];

	/// \brief The object discovery elements
	///
	/// These are the actual object discovery helpers.  Note that this
	/// member must be initialised after m_tag, as it may depend on
	/// programmatically generated tags.
	T m_array[Count];

public:
	/// \brief Element type for Container concept
	typedef T value_type;

	/// \brief Reference to element type for Container concept
	typedef T &reference;

	/// \brief Reference to constant element type for Container concept
	typedef T const &const_reference;

	/// \brief Iterator for Container concept
	typedef T *iterator;

	/// \brief Constant iterator for Container concept
	typedef T const *const_iterator;

	/// \brief Iterator difference type for Container concept
	typedef typename std::iterator_traits<iterator>::difference_type difference_type;

	/// \brief Size type for Container concept
	typedef std::make_unsigned_t<difference_type> size_type;

	/// \brief Construct with programmatically generated tags
	///
	/// Specify a format string and starting number.  A single unsigned
	/// int format argument is supplied containing the (zero-based)
	/// element index added to the starting number.  For example if
	/// Count = 2, ("p%u_joy", 1) expands to ("p1_joy", "p2_joy").  The
	/// relaxed format rules used by util::string_format apply.
	/// \param [in] base Base device to search from.
	/// \param [in] fmt Search tag format, should expect single unsigned
	///   int argument.
	/// \param [in] start Number to add to element index when
	///   calculating values for string format argument.
	/// \arg [in] Optional additional constructor argument(s) passed to
	///   all elements.
	/// \sa util::string_format
	template <typename F, typename... Param>
	object_array_finder(device_t &base, F const &fmt, unsigned start, Param const &... arg)
		: object_array_finder(base, fmt, start, std::make_integer_sequence<unsigned, Count>(), arg...)
	{
	}

	/// \brief Construct with free-form list of tags
	///
	/// Specify arbitrary tags for objects.  Useful when there is no
	/// particular pattern to the object tags.
	/// \param [in] base Base device to search from.
	/// \param [in] tags Tags to search for, e.g. { "player", "dips" }.
	///   The tags are not copied, it is the caller's responsibility to
	///   ensure the pointers remain valid until resolution time.
	/// \arg [in] Optional additional constructor argument(s) passed to
	///   all elements.
	template <typename... Param>
	object_array_finder(device_t &base, std::array<char const *, Count> const &tags, Param const &... arg)
		: object_array_finder(base, tags, std::make_integer_sequence<unsigned, Count>(), arg...)
	{
	}

	/// \brief Get iterator to first element
	///
	/// Returns an iterator to the first element in the array.
	/// \return Iterator to first element.
	const_iterator begin() const { return m_array; }
	iterator begin() { return m_array; }

	/// \brief Get iterator beyond last element
	///
	/// Returns an iterator one past the last element in the array.
	/// \return Iterator one past last element.
	const_iterator end() const { return m_array + Count; }
	iterator end() { return m_array + Count; }

	/// \brief Get constant iterator to first element
	///
	/// Returns a constant iterator to the first element in the array.
	/// \return Constant iterator to first element.
	const_iterator cbegin() const { return m_array; }

	/// \brief Get constant iterator beyond last element
	///
	/// Returns aconstant iterator one past the last element in the
	/// array.
	/// \return Constant iterator one past last element.
	const_iterator cend() const { return m_array + Count; }

	/// \brief Get array size
	///
	/// Returns number of elements in the array (compile-time constant).
	/// \return The size of the array.
	constexpr size_type size() const { return Count; }

	/// \brief Get maximum array size
	///
	/// Returns maximum number of elements in the array (compile-time
	/// constant, always equal to the size of the array).
	/// \return The size of the array.
	constexpr size_type max_size() const { return Count; }

	/// \brief Does array have no elements
	///
	/// Returns whether the arary has no elements (compile-time
	/// constant).
	/// \return True if the array has no elements, false otherwise.
	constexpr bool empty() const { return !Count; }

	/// \brief Get first element
	///
	/// Returns a reference to the first element in the array.
	/// \return Reference to first element.
	T const &front() const { return m_array[0]; }
	T &front() { return m_array[0]; }

	/// \brief Get last element
	///
	/// Returns a reference to the last element in the array.
	/// \return Reference to last element.
	T const &back() const { return m_array[Count - 1]; }
	T &back() { return m_array[Count - 1]; }

	/// \brief Element accessor (const)
	///
	/// Returns a const reference to the element at the supplied index.
	/// \param [in] index Index of desired element (zero-based).
	/// \return Constant reference to element at specified index.
	T const &operator[](unsigned index) const { assert(index < Count); return m_array[index]; }

	/// \brief Element accessor (non-const)
	///
	/// Returns a reference to the element at the supplied index.
	/// \param [in] index Index of desired element (zero-based).
	/// \return Reference to element at specified index.
	T &operator[](unsigned index) { assert(index < Count); return m_array[index]; }

	/// \brief Checked element accesor
	///
	/// Returns a reference to the element at the supplied index if less
	/// than the size of the array, or throws std::out_of_range
	/// otherwise.
	/// \param [in] index Index of desired element (zero-based).
	/// \return Reference to element at specified index.
	/// \throw std::out_of_range
	T const &at(unsigned index) const { if (Count > index) return m_array[index]; else throw std::out_of_range("Index out of range"); }
	T &at(unsigned index) { if (Count > index) return m_array[index]; else throw std::out_of_range("Index out of range"); }
};


/// \brief Base class for object discovery helpers
///
/// Abstract non-template base class for object auto-discovery helpers.
/// Provides the interface that the device_t uses to manage discovery at
/// resolution time.
class finder_base
{
public:
	/// \brief Destructor
	///
	/// Destruction via base class pointer and dynamic type behaviour
	/// are allowed.
	virtual ~finder_base();

	/// \brief Get next registered object discovery helper
	///
	/// Implementation of basic single-linked list behaviour.
	/// \return Pointer to the next registered object discovery helper,
	///   or nullptr if this is the last.
	finder_base *next() const { return m_next; }

	/// \brief Attempt discovery
	///
	/// Concrete derived classes must implement this member function.
	/// Should return false if the the object is required but not found,
	/// or true otherwise (the report_missing member function can assist
	/// in implementing this behaviour).
	/// \param [in] isvalidation Pass true if this is a dry run (i.e. no
	///   intention to actually start the device), or false otherwise.
	/// \return False if the object is required but not found, or true
	///   otherwise.
	virtual bool findit(bool isvalidation = false) = 0;

	/// \brief Clear temporary binding from configuration
	///
	/// Concrete derived classes must implement this member function.
	/// Object finders may allow temporary binding to the anticipated
	/// target during configuration.  This needs to be cleared to ensure
	/// the correct target is found if a device further up the hierarchy
	/// subsequently removes or replaces devices.
	virtual void end_configuration() = 0;

	/// \brief Get search tag
	///
	/// Returns the search tag.
	/// \return The object tag this helper will search for.
	char const *finder_tag() const { return m_tag; }

	/// \brief Get search target
	///
	/// Returns the search base device and tag.
	/// \return a pair consisting of a reference to the device to search
	///   relative to and the relative tag.
	std::pair<device_t &, char const *> finder_target() const { return std::make_pair(m_base, m_tag); }

	/// \brief Set search tag
	///
	/// Allows search tag to be changed after construction.  Note that
	/// this must be done before resolution time to take effect.  Also
	/// note that the tag is not copied.
	/// \param [in] base Updated search base.  The tag must be specified
	///   relative to this device.
	/// \param [in] tag Updated search tag.  This is not copied, it is
	///   the caller's responsibility to ensure this pointer remains
	///   valid until resolution time.
	void set_tag(device_t &base, char const *tag)
	{
		assert(!m_resolved);
		m_base = base;
		m_tag = tag;
	}

	/// \brief Set search tag
	///
	/// Allows search tag to be changed after construction.  Note that
	/// this must be done before resolution time to take effect.  Also
	/// note that the tag is not copied.
	/// \param [in] tag Updated search tag relative to the current
	///   device being configured.  This is not copied, it is the
	///   caller's responsibility to ensure this pointer remains valid
	///   until resolution time.
	void set_tag(char const *tag);

	/// \brief Set search tag
	///
	/// Allows search tag to be changed after construction.  Note that
	/// this must be done before resolution time to take effect.
	/// \param [in] finder Object finder to take the search base and tag
	///   from.
	void set_tag(finder_base const &finder)
	{
		assert(!m_resolved);
		std::tie(m_base, m_tag) = finder.finder_target();
	}

	/// \brief Dummy tag always treated as not found
	constexpr static char DUMMY_TAG[17] = "finder_dummy_tag";

protected:
	/// \brief Designated constructor
	///
	/// Construct base object discovery helper and register with device
	/// to be invoked at resolution time.
	/// \param [in] base Base device to search from.
	/// \param [in] tag Object tag to search for.  This is not copied,
	///   it is the caller's responsibility to ensure this pointer
	///   remains valid until resolution time.
	finder_base(device_t &base, char const *tag);

	/// \brief Find a memory region
	///
	/// Look up memory region and check that its length and width match
	/// desired values.  Returns pointer to the base of the region if a
	/// matching region is found, or nullptr otherwise.  Prints a
	/// message at warning level if the region is required, a region
	/// with the requested tag is found, but it doesn't match the
	/// desired width and length.
	/// \param [in] width Desired region width in bytes.
	/// \param [in,out] length On entry, the desired region length in
	///   width units, or 0U to match any region length.  Set to the
	///   length of the region in width units if a matching region is
	///   found, or 0U otherwise.
	/// \param [in] required Whether warning message should be printed
	///   if a region with matching tag of incorrect width/length is
	///   found.
	/// \return Base pointer of the memory region if a matching region
	///   is found, or nullptr otherwise.
	void *find_memregion(u8 width, size_t &length, bool required) const;

	/// \brief Check that memory region exists
	///
	/// Walks ROM regions of all devices starting from the root looking
	/// for one with matching tag and length in bytes.  Prints a warning
	/// message if the region is required, a region with the requested
	/// tag is found, but its length does not match.  Calls
	/// report_missing to print an error message if the region is
	/// not found.  Returns true if the region is required but no
	/// matching region is found, or false otherwise.
	/// \param [in] bytes Desired region length in bytes, or 0U to match
	///   any length.
	/// \param [in] required True if the region is required, or false if
	///   it is optional.
	/// \return True if the region is optional, or if the region is
	///   required and a matching region is found, or false otherwise.
	bool validate_memregion(size_t bytes, bool required) const;

	/// \brief Find a memory share
	///
	/// Look up memory share and check that its width matches desired
	/// value.  Returns pointer to base of memory share if a matching
	/// share is found, or nullptr otherwise.  Prints a message at
	/// warning level if the memory share is required, a memory share
	/// with the requested tag is found, but it doesn't match the
	/// desired width.
	/// \param [in] width Desired memory share width in bits.
	/// \param [out] bytes Set to memoyr share length in bytes if a
	///   matching memory share is found, otherwise left unchanged.
	/// \param [in] required. Whether warning message should be printed
	///   if a memory share with matching tag of incorrect width is
	///   found.
	/// \return Pointer to base of memory share if a matching memory
	///   share is found, or nullptr otherwise.
	void *find_memshare(u8 width, size_t &bytes, bool required) const;

	/// \brief Find an address space
	///
	/// Look up address space and check that its width matches desired
	/// value.  Returns pointer to address space if a matching space
	/// is found, or nullptr otherwise.  Prints a message at warning
	/// level if the address space is required, a device with the
	/// requested tag is found, but it doesn't have a memory interface
	/// or a space with the designated number.
	/// \param [in] spacenum Address space number.
	/// \param [in] width Specific data width, or 0.
	/// \param [in] required. Whether warning message should be printed
	///   if a device with no memory interface or space of that number
	///   is found.
	/// \return Pointer to address space if a matching address space
	///   is found, or nullptr otherwise.
	address_space *find_addrspace(int spacenum, u8 width, bool required) const;

	/// \brief Check that address space exists
	///
	/// Returns true if the space is required but no matching space is
	/// found, or false otherwise.
	/// \param [in] spacenum Address space number.
	/// \param [in] width Specific data width, or 0.
	/// \param [in] required. Whether warning message should be printed
	///   if a device with no memory interface or space of that number
	///   is found.
	/// \return True if the space is optional, or if the space is
	///   space and a matching space is found, or false otherwise.
	bool validate_addrspace(int spacenum, u8 width, bool required) const;

	/// \brief Log if object was not found
	///
	/// Logs a message at error level if the target object is required
	/// and the search tag is the dummy tag, or the target object is
	/// required and not found.  Logs a message at verbose level if the
	/// object is optional and not found.  Returns true if the object is
	/// found or not required, and false otherwise.
	/// \param [in] found Whether the target object has been found.
	/// \param [in] objname Display name for target object type.
	/// \param [in] required True if the object is required (validation
	///   error if not found), or false if optional.
	/// \return True if found or not required, false otherwise.
	bool report_missing(bool found, char const *objname, bool required) const;

	/// \brief Print a message at warning level
	///
	/// Prints a message if logging is enabled at warning level or more
	/// detailed.  Uses printf semantics of the C runtime library.
	/// \param [in] format Format string as used by printf function in
	///   runtime library
	void printf_warning(char const *format, ...) ATTR_PRINTF(2,3);


	/// \brief Pointer to next registered discovery helper
	///
	/// This is a polymorphic class, so it can't be held in a standardlist
	/// container that requires elements of the same type.  Hence it
	/// implements basic single-linked list behaviour.
	finder_base *const m_next;

	/// \brief Base device to search from
	std::reference_wrapper<device_t> m_base;

	/// \brief Object tag to search for
	char const *m_tag;

	/// \brief Set when object resolution completes
	bool m_resolved;
};


/// \brief Base class for object discovery helpers
///
/// Abstract template base for auto-discovery of objects of a particular
/// type.  Provides implicit cast-to-pointer and pointer member access
/// operators.  Template arguments are the type of object to discover,
/// and whether failure to find the object is considered an error.
/// Assumes that non-null pointer is found, and null pointer is not
/// found.
template <class ObjectClass, bool Required>
class object_finder_base : public finder_base
{
public:
	/// \brief Clear temporary binding from configuration
	///
	/// Object finders may allow temporary binding to the anticipated
	/// target during configuration.  This needs to be cleared to ensure
	/// the correct target is found if a device further up the hierarchy
	/// subsequently removes or replaces devices.
	virtual void end_configuration() override { assert(!m_resolved); m_target = nullptr; }

	/// \brief Get pointer to target object
	/// \return Pointer to target object if found, or nullptr otherwise.
	ObjectClass *target() const { return m_target; }

	/// \brief Return whether target has been found
	///
	/// Works on the assumption that the target object pointer will be
	/// non-null if the target has been found, and null otherwise.
	/// \return True if object has been found, or false otherwise.
	bool found() const { return m_target != nullptr; }

	/// \brief Cast-to-pointer operator
	///
	/// Allows implicit casting to a pointer to the target object.
	/// Returns a null pointer if resolution has not been attempted or
	// object was not found.
	/// \return Pointer to target object if found, or nullptr otherwise.
	operator ObjectClass *() const { return m_target; }

	/// \brief Pointer member access operator
	///
	/// Allows pointer-member-style access to members of the target
	/// object.  Asserts that the target object has been found.
	/// \return Pointer to target object if found, or nullptr otherwise.
	virtual ObjectClass *operator->() const { assert(m_target); return m_target; }

protected:
	/// \brief Designated constructor
	///
	/// Construct base, register with base device to be invoked at
	/// resolution time, and initialise target object pointer to
	/// nullptr.
	/// \param [in] base Base device to search from.
	/// \param [in] tag Object tag to search for.  This is not copied,
	///   it is the caller's responsibility to ensure this pointer
	///   remains valid until resolution time.
	object_finder_base(device_t &base, const char *tag) : finder_base(base, tag) { }

	/// \brief Log if object was not found
	///
	/// Logs a message if the target object was not found, and returns
	/// false if this is problematic.  Calls base implementation,
	/// supplying values for found and required parameters.  See base
	/// implementation for more detail.
	/// \param [in] objname Display name for target object type.
	/// \return True if found or not required, false otherwise.
	bool report_missing(char const *objname) const { return finder_base::report_missing(found(), objname, Required); }

	/// \brief Pointer to target object
	///
	/// Pointer to target object, or nullptr if resolution has not been
	/// attempted or the search failed.  Concrete derived classes must
	/// set this in their implementation of the findit member function.
	ObjectClass *m_target = nullptr;
};


/// \brief Device finder template
///
/// Template arguments are the device class to find, and whether the
/// device is required.  It is a validation error if a required device
/// is not found.  If a device with matching tag is found but the class
/// does not match, a message is printed at warning level.  This class
/// is generally not used directly, instead the optional_device and
/// required_device helpers are used.
/// \sa optional_device required_device
template <class DeviceClass, bool Required>
class device_finder : public object_finder_base<DeviceClass, Required>
{
public:
	using object_finder_base<DeviceClass, Required>::set_tag;

	/// \brief Device finder constructor
	/// \param [in] base Base device to search from.
	/// \param [in] tag Device tag to search for.  This is not copied,
	///   it is the caller's responsibility to ensure this pointer
	///   remains valid until resolution time.
	device_finder(device_t &base, char const *tag) : object_finder_base<DeviceClass, Required>(base, tag) { }

	/// \brief Set search tag
	///
	/// Allows search tag to be changed after construction.  Note that
	/// this must be done before resolution time to take effect.  Note
	/// that this binds to a particular instance, so the device must not
	/// be removed or replaced, as it will cause a use-after-free when
	/// resolving objects.
	/// \param [in] object Object to refer to.
	void set_tag(DeviceClass &object) { set_tag(object, DEVICE_SELF); }

	/// \brief Set target during configuration
	///
	/// During configuration, device_finder instances may be assigned
	/// a reference to the anticipated target device to avoid the need
	/// for tempories during configuration.  Normal resolution will
	/// still happen after machine configuration is completed to ensure
	/// device removal/replacement is handled properly.
	/// \param [in] device Reference to anticipated target device.
	/// \return The same reference supplied by the caller.
	template <typename T>
	std::enable_if_t<std::is_convertible<T *, DeviceClass *>::value, T &> operator=(T &device)
	{
		assert(!this->m_resolved);
		assert(is_expected_tag(device));
		this->m_target = &device;
		return device;
	}

private:
	/// \brief Check that device implementation has expected tag
	/// \param [in] device Reference to device.
	/// \return True if supplied device matches the configured target
	///   tag, or false otherwise.
	template <typename T>
	std::enable_if_t<emu::detail::is_device_implementation<T>::value, bool> is_expected_tag(T const &device) const
	{
		return this->m_base.get().subtag(this->m_tag) == device.tag();
	}

	/// \brief Check that device mixin has expected tag
	/// \param [in] device Reference to interface/mixin.
	/// \return True if supplied mixin matches the configured target
	///   tag, or false otherwise.
	template <typename T>
	std::enable_if_t<emu::detail::is_device_interface<T>::value, bool> is_expected_tag(T const &interface) const
	{
		return this->m_base.get().subtag(this->m_tag) == interface.device().tag();
	}

	/// \brief Find device
	///
	/// Find device of desired type with requested tag.  If a device
	/// with the requested tag is found but the type is incorrect, a
	/// warning message will be printed.  This method is called by the
	/// base device at resolution time.
	/// \param [in] isvalidation True if this is a dry run (not
	///   intending to run the machine, just checking for errors).
	/// \return True if the device is optional or if a matching device
	///   is found, false otherwise.
	virtual bool findit(bool isvalidation) override
	{
		if (!isvalidation)
		{
			assert(!this->m_resolved);
			this->m_resolved = true;
		}

		device_t *const device = this->m_base.get().subdevice(this->m_tag);
		this->m_target = dynamic_cast<DeviceClass *>(device);
		if (device && !this->m_target)
			this->printf_warning("Device '%s' found but is of incorrect type (actual type is %s)\n", this->m_tag, device->name());

		return this->report_missing("device");
	}
};

/// \brief Optional device finder
///
/// Finds device with maching type and tag.  If a device with matching
/// tag is found but the type does not match, a message is printed at
/// warning level.  No error is generated if a matching device is not
/// found (the target object pointer will be null).  If you have a
/// number of similar optional devices, consider using
/// optional_device_array.
/// \sa required_device optional_device_array device_finder
template <class DeviceClass> using optional_device = device_finder<DeviceClass, false>;

/// \brief Required device finder
///
/// Finds device with maching type and tag.  If a device with matching
/// tag is found but the type does not match, a message is printed at
/// warning level.  A validation error is generated if a matching device
/// is not found.  If you have a number of similar required devices,
/// consider using required_device_array.
/// \sa optional_device required_device_array device_finder
template <class DeviceClass> using required_device = device_finder<DeviceClass, true>;

template <class DeviceClass, unsigned Count, bool Required> using device_array_finder = object_array_finder<device_finder<DeviceClass, Required>, Count>;
template <class DeviceClass, unsigned Count> using optional_device_array = device_array_finder<DeviceClass, Count, false>;
template <class DeviceClass, unsigned Count> using required_device_array = device_array_finder<DeviceClass, Count, true>;


/// \brief Memory region finder template
///
/// Template argument is whether the memory region is required.  It is a
/// validation error if a required memory region is not found.  This
/// class is generally not used directly, instead the
/// optional_memory_region and required_memory_region helpers are used.
/// \sa optional_memory_region required_memory_region
template <bool Required>
class memory_region_finder : public object_finder_base<memory_region, Required>
{
public:
	/// \brief Memory region finder constructor
	/// \param [in] base Base device to search from.
	/// \param [in] tag Memory region tag to search for.  This is not
	///   copied, it is the caller's responsibility to ensure this
	///   pointer remains valid until resolution time.
	memory_region_finder(device_t &base, char const *tag) : object_finder_base<memory_region, Required>(base, tag) { }

private:
	/// \brief Find memory region
	///
	/// Find memory region with requested tag.  For a dry run, the
	/// target object pointer will not be set.  This method is called by
	/// the base device at resolution time.
	/// \param [in] isvalidation True if this is a dry run (not
	///   intending to run the machine, just checking for errors).
	/// \return True if the memory region is optional or if a matching
	///   memory region is found, false otherwise.
	virtual bool findit(bool isvalidation) override
	{
		if (isvalidation)
			return this->validate_memregion(0, Required);

		assert(!this->m_resolved);
		this->m_resolved = true;
		this->m_target = this->m_base.get().memregion(this->m_tag);
		return this->report_missing("memory region");
	}
};

/// \brief Optional memory region finder
///
/// Finds memory region with maching tag.  No error is generated if a
/// matching memory region is not found (the target object pointer will
/// be null).  If you have a number of similar optional memory regions,
/// consider using optional_memory_region_array.
/// \sa required_memory_region optional_memory_region_array
///   memory_region_finder
using optional_memory_region = memory_region_finder<false>;

/// \brief Required memory region finder
///
/// Finds memory region with maching tag.  A validation error is
/// generated if a matching memory region is not found.  If you have a
/// number of similar required memory regions, consider using
/// required_memory_region_array.
/// \sa optional_memory_region required_memory_region_array
///   memory_region_finder
using required_memory_region = memory_region_finder<true>;

template <unsigned Count, bool Required> using memory_region_array_finder = object_array_finder<memory_region_finder<Required>, Count>;
template <unsigned Count> using optional_memory_region_array = memory_region_array_finder<Count, false>;
template <unsigned Count> using required_memory_region_array = memory_region_array_finder<Count, true>;


/// \brief Memory bank finder template
///
/// Template argument is whether the memory bank is required.  It is a
/// validation error if a required memory bank is not found.  This class
/// is generally not used directly, instead the optional_memory_bank and
/// required_memory_bank helpers are used.
/// \sa optional_memory_bank required_memory_bank
template <bool Required>
class memory_bank_finder : public object_finder_base<memory_bank, Required>
{
public:
	/// \brief Memory bank finder constructor
	/// \param [in] base Base device to search from.
	/// \param [in] tag Memory bank tag to search for.  This is not
	///   copied, it is the caller's responsibility to ensure this
	///   pointer remains valid until resolution time.
	memory_bank_finder(device_t &base, char const *tag) : object_finder_base<memory_bank, Required>(base, tag) { }

	/// \brief Find memory bank
	///
	/// Find memory bank with requested tag.  Just returns true for a
	/// dry run.  This method is called by the base device at resolution
	/// time.
	/// \param [in] isvalidation True if this is a dry run (not
	///   intending to run the machine, just checking for errors).
	/// \return True if the memory bank is optional, a matching memory
	///   bank is found or this is a dry run, false otherwise.
	virtual bool findit(bool isvalidation) override
	{
		if (isvalidation)
			return true;

		assert(!this->m_resolved);
		this->m_resolved = true;
		this->m_target = this->m_base.get().membank(this->m_tag);
		return this->report_missing("memory bank");
	}
};

/// \brief Optional memory bank finder
///
/// Finds memory bank with maching tag.  No error is generated if a
/// matching memory bank is not found (the target object pointer will
/// be null).  If you have a number of similar optional memory banks,
/// consider using optional_memory_bank_array.
/// \sa required_memory_bank optional_memory_bank_array
///   memory_bank_finder
using optional_memory_bank = memory_bank_finder<false>;

/// \brief Required memory bank finder
///
/// Finds memory bank with maching tag.  A validation error is
/// generated if a matching memory bank is not found.  If you have a
/// number of similar required memory banks, consider using
/// required_memory_bank_array.
/// \sa optional_memory_bank required_memory_bank_array
///   memory_bank_finder
using required_memory_bank = memory_bank_finder<true>;

template <unsigned Count, bool Required> using memory_bank_array_finder = object_array_finder<memory_bank_finder<Required>, Count>;
template <unsigned Count> using optional_memory_bank_array = memory_bank_array_finder<Count, false>;
template <unsigned Count> using required_memory_bank_array = memory_bank_array_finder<Count, true>;


/// \brief I/O port finder template
///
/// Template argument is whether the I/O port is required.  It is a
/// validation error if a required I/O port is not found.  This class is
/// generally not used directly, instead the optional_ioport and
/// required_ioport helpers are used.
/// \sa optional_ioport required_ioport
template <bool Required>
class ioport_finder : public object_finder_base<ioport_port, Required>
{
public:
	/// \brief I/O port finder constructor
	/// \param [in] base Base device to search from.
	/// \param [in] tag I/O port tag to search for.  This is not copied,
	///   it is the caller's responsibility to ensure this pointer
	///   remains valid until resolution time.
	ioport_finder(device_t &base, char const *tag) : object_finder_base<ioport_port, Required>(base, tag) { }

	/// \brief Read I/O port if found or return default value
	///
	/// If the I/O port was found, this reads a value from the I/O port
	/// and returns it.  If the I/O port was not found, the default
	/// value (supplied as a parameter) is returned.
	/// \param [in] defval Value to return if I/O port was not found.
	/// \return Value read from I/O port if found, or supplied default
	///   value otherwise.
	ioport_value read_safe(ioport_value defval) { return this->m_target ? this->m_target->read() : defval; }

private:
	/// \brief Find I/O port
	///
	/// Find I/O port with requested tag.  Just returns true for a dry
	/// run.  This method is called by the base device at resolution
	/// time.
	/// \param [in] isvalidation True if this is a dry run (not
	///   intending to run the machine, just checking for errors).
	/// \return True if the I/O port is optional, a matching I/O port is
	///   is found or this is a dry run, false otherwise.
	virtual bool findit(bool isvalidation) override
	{
		if (isvalidation)
			return true;

		assert(!this->m_resolved);
		this->m_resolved = true;
		this->m_target = this->m_base.get().ioport(this->m_tag);
		return this->report_missing("I/O port");
	}
};

/// \brief Optional I/O port finder
///
/// Finds I/O port with maching tag.  No error is generated if a
/// matching I/O port is not found (the target object pointer will be
/// null).  If you have a number of similar optional I/O ports, consider
/// using optional_ioport_array.
/// \sa required_ioport optional_ioport_array ioport_finder
using optional_ioport = ioport_finder<false>;

/// \brief Required I/O port finder
///
/// Finds I/O port with maching tag.  A validation error is generated if
/// a matching I/O port is not found.  If you have a number of similar
/// required I/O ports, consider using required_ioport_array.
/// \sa optional_ioport required_ioport_array ioport_finder
using required_ioport = ioport_finder<true>;

template <unsigned Count, bool Required> using ioport_array_finder = object_array_finder<ioport_finder<Required>, Count>;
template <unsigned Count> using optional_ioport_array = ioport_array_finder<Count, false>;
template <unsigned Count> using required_ioport_array = ioport_array_finder<Count, true>;


/// \brief Address space finder template
///
/// Template argument is whether the address space is required.  It is a
/// validation error if a required address space is not found.  This class is
/// generally not used directly, instead the optional_address_space and
/// required_address_space helpers are used.
/// \sa optional_address_space required_address_space
template <bool Required>
class address_space_finder : public object_finder_base<address_space, Required>
{
public:
	/// \brief Address space finder constructor
	/// \param [in] base Base device to search from.
	/// \param [in] tag Address space tag to search for.  This is not copied,
	///   it is the caller's responsibility to ensure this pointer
	///   remains valid until resolution time.
	/// \param [in] spacenum Address space number.
	/// \param [in] width Specific data width (optional).
	address_space_finder(device_t &base, char const *tag, int spacenum, u8 width = 0) : object_finder_base<address_space, Required>(base, tag), m_spacenum(spacenum), m_data_width(width) { }

	/// \brief Set search tag and space number
	///
	/// Allows search tag to be changed after construction.  Note that
	/// this must be done before resolution time to take effect.  Also
	/// note that the tag is not copied.
	/// \param [in] base Updated search base.  The tag must be specified
	///   relative to this device.
	/// \param [in] tag Updated search tag.  This is not copied, it is
	///   the caller's responsibility to ensure this pointer remains
	///   valid until resolution time.
	/// \param [in] spacenum Address space number.
	void set_tag(device_t &base, char const *tag, int spacenum) { finder_base::set_tag(base, tag); m_spacenum = spacenum; }

	/// \brief Set search tag and space number
	///
	/// Allows search tag to be changed after construction.  Note that
	/// this must be done before resolution time to take effect.  Also
	/// note that the tag is not copied.
	/// \param [in] tag Updated search tag relative to the current
	///   device being configured.  This is not copied, it is the
	///   caller's responsibility to ensure this pointer remains valid
	///   until resolution time.
	/// \param [in] spacenum Address space number.
	void set_tag(char const *tag, int spacenum) { finder_base::set_tag(tag); m_spacenum = spacenum; }

	/// \brief Set search tag and space number
	///
	/// Allows search tag to be changed after construction.  Note that
	/// this must be done before resolution time to take effect.
	/// \param [in] finder Object finder to take the search base and tag
	///   from.
	/// \param [in] spacenum Address space number.
	void set_tag(finder_base const &finder, int spacenum) { finder_base::set_tag(finder); this->m_spacenum = spacenum; }

	/// \brief Set data width of space
	///
	/// Allows data width to be specified after construction.  Note that
	/// this must be done before resolution time to take effect.
	/// \param [in] width Data width in bits (0 = don't care).
	void set_data_width(u8 width) { this->m_data_width = width; }

	/// \brief Get space number
	///
	/// Returns the configured address space number.
	/// \return The space number to be found.
	int spacenum() const { return m_spacenum; }

private:
	/// \brief Find address space
	///
	/// Find address space with requested tag.  For a dry run, the
	/// target object pointer will not be set.  This method is called by
	/// the base device at resolution time.
	/// \param [in] isvalidation True if this is a dry run (not
	///   intending to run the machine, just checking for errors).
	/// \return True if the address space is optional, a matching address space is
	///   is found or this is a dry run, false otherwise.
	virtual bool findit(bool isvalidation) override
	{
		if (isvalidation)
			return this->validate_addrspace(this->m_spacenum, this->m_data_width, Required);

		assert(!this->m_resolved);
		this->m_resolved = true;
		this->m_target = this->find_addrspace(this->m_spacenum, this->m_data_width, Required);
		return this->report_missing("address space");
	}

	int m_spacenum;
	u8 m_data_width;
};

/// \brief Optional address space finder
///
/// Finds address space with maching tag and number.  No error is generated if a
/// matching address space is not found (the target object pointer will be
/// null).
/// \sa required_address_space address_space_finder
using optional_address_space = address_space_finder<false>;

/// \brief Required address space finder
///
/// Finds address space with maching tag and number.  A validation error is generated if
/// a matching address space is not found.
/// \sa optional_address_space address_space_finder
using required_address_space = address_space_finder<true>;


/// \brief Memory region base pointer finder
///
/// Template arguments are the element type of the memory region and
/// whether the memory region is required.  It is a validation error if
/// a required memory region is not found.  This class is generally not
/// used directly, instead the optional_region_ptr and
/// required_region_ptr helpers are used.
/// \sa optional_region_ptr required_region_ptr
template <typename PointerType, bool Required>
class region_ptr_finder : public object_finder_base<PointerType, Required>
{
public:
	/// \brief Memory region base pointer finder constructor
	///
	/// Desired width is implied by sizeof(PointerType).
	/// \param [in] base Base device to search from.
	/// \param [in] tag Memory region tag to search for.  This is not
	///   copied, it is the caller's responsibility to ensure this
	///   pointer remains valid until resolution time.
	/// \param [in] length Desired memory region length in units of the
	///   size of the element type, or zero to match any region length.
	region_ptr_finder(device_t &base, char const *tag, size_t length = 0)
		: object_finder_base<PointerType, Required>(base, tag)
		, m_desired_length(length)
		, m_length(0)
	{
	}

	/// \brief Array access operator
	///
	/// Returns a non-const reference to the element of the memory
	/// region at the supplied zero-based index.
	/// Behaviour is undefined for negative element indices.
	/// \param [in] index Non-negative element index.
	/// \return Non-const reference to element at requested index.
	PointerType &operator[](int index) const { assert(index < m_length); return this->m_target[index]; }

	/// \brief Get length in units of elements
	/// \return Length in units of elements or zero if no matching
	///   memory region has been found.
	u32 length() const { return m_length; }

	/// \brief Get length in units of bytes
	/// \return Length in units of bytes or zero if no matching memory
	///   region has been found.
	u32 bytes() const { return m_length * sizeof(PointerType); }

	/// \brief Get index mask
	///
	/// Returns the length in units of elements minus one, which can be
	/// used as a mask for index values if the length is a power of two.
	/// Result is undefined if no matching memory region has been found.
	/// \return Length in units of elements minus one.
	u32 mask() const { return m_length - 1; }

private:
	/// \brief Find memory region base pointer
	///
	/// Find base pointer of memory region with with requested tag,
	/// width and length.  Width of memory region is checked against
	/// sizeof(PointerType).  For a dry run, only the tag and length are
	/// checked - the width is not checked and the target pointer is not
	/// set.  This method is called by the base device at resolution
	/// time.
	/// \param [in] isvalidation True if this is a dry run (not
	///   intending to run the machine, just checking for errors).
	/// \return True if the memory region is optional or a matching
	///   memory region is found, or false otherwise.
	virtual bool findit(bool isvalidation) override
	{
		if (isvalidation)
			return this->validate_memregion(sizeof(PointerType) * m_desired_length, Required);

		assert(!this->m_resolved);
		this->m_resolved = true;
		m_length = m_desired_length;
		this->m_target = reinterpret_cast<PointerType *>(this->find_memregion(sizeof(PointerType), m_length, Required));
		return this->report_missing("memory region");
	}

	/// \brief Desired region length
	///
	/// Desired region length in units of elements.
	size_t const m_desired_length;

	/// \brief Matched region length
	///
	/// Actual length of the region that was found in units of
	/// elements, or zero if no matching region has been found.
	size_t m_length;
};

/// \brief Optional memory region base pointer finder
///
/// Finds base pointer of memory region with maching tag, width and
/// length.  No error is generated if a matching memory region is not
/// found (the target pointer will be null).  If you have a number of
/// similar optional memory regions, consider using
/// optional_region_ptr_array.
/// \sa required_region_ptr optional_region_ptr_array region_ptr_finder
template <typename PointerType> using optional_region_ptr = region_ptr_finder<PointerType, false>;

/// \brief Required memory region base pointer finder
///
/// Finds base pointer of memory region with maching tag, width and
/// length.  A validation error is generated if a matching memory region
/// is not found.  If you have a number of similar required memory
/// regions, consider using required_region_ptr_array.
/// \sa optional_region_ptr required_region_ptr_array region_ptr_finder
template <typename PointerType> using required_region_ptr = region_ptr_finder<PointerType, true>;

template <typename PointerType, unsigned Count, bool Required> using region_ptr_array_finder = object_array_finder<region_ptr_finder<PointerType, Required>, Count>;
template <typename PointerType, unsigned Count> using optional_region_ptr_array = region_ptr_array_finder<PointerType, Count, false>;
template <typename PointerType, unsigned Count> using required_region_ptr_array = region_ptr_array_finder<PointerType, Count, true>;


// ======================> shared_ptr_finder

// shared pointer finder template
template <typename PointerType, bool Required>
class shared_ptr_finder : public object_finder_base<PointerType, Required>
{
public:
	// construction/destruction
	shared_ptr_finder(device_t &base, char const *tag, u8 width = sizeof(PointerType) * 8)
		: object_finder_base<PointerType, Required>(base, tag)
		, m_width(width)
		, m_bytes(0)
		, m_allocated(0)
	{
	}

	// operators to make use transparent
	PointerType &operator[](int index) const { return this->m_target[index]; }

	// getter for explicit fetching
	u32 bytes() const { return m_bytes; }
	u32 mask() const { return m_bytes - 1; } // FIXME: wrong when sizeof(PointerType) != 1

	// setter for setting the object
	void set_target(PointerType *target, size_t bytes) { this->m_target = target; m_bytes = bytes; }

	// dynamic allocation of a shared pointer
	void allocate(u32 entries)
	{
		assert(m_allocated.empty());
		m_allocated.resize(entries);
		this->m_target = &m_allocated[0];
		m_bytes = entries * sizeof(PointerType);
		this->m_base.get().save_item(m_allocated, this->m_tag);
	}

private:
	// finder
	virtual bool findit(bool isvalidation) override
	{
		if (isvalidation)
			return true;

		assert(!this->m_resolved);
		this->m_resolved = true;
		this->m_target = reinterpret_cast<PointerType *>(this->find_memshare(m_width, m_bytes, Required));
		return this->report_missing("shared pointer");
	}

	// internal state
	u8 const m_width;
	size_t m_bytes;
	std::vector<PointerType> m_allocated;
};

template <typename PointerType> using optional_shared_ptr = shared_ptr_finder<PointerType, false>;
template <typename PointerType> using required_shared_ptr = shared_ptr_finder<PointerType, true>;
template <typename PointerType, unsigned Count, bool Required> using shared_ptr_array_finder = object_array_finder<shared_ptr_finder<PointerType, Required>, Count>;
template <typename PointerType, unsigned Count> using optional_shared_ptr_array = shared_ptr_array_finder<PointerType, Count, false>;
template <typename PointerType, unsigned Count> using required_shared_ptr_array = shared_ptr_array_finder<PointerType, Count, true>;



//**************************************************************************
//  EXTERNAL TEMPLATE INSTANTIATIONS
//**************************************************************************

extern template class object_finder_base<memory_region, false>;
extern template class object_finder_base<memory_region, true>;
extern template class object_finder_base<memory_bank, false>;
extern template class object_finder_base<memory_bank, true>;
extern template class object_finder_base<ioport_port, false>;
extern template class object_finder_base<ioport_port, true>;

extern template class object_finder_base<u8, false>;
extern template class object_finder_base<u8, true>;
extern template class object_finder_base<u16, false>;
extern template class object_finder_base<u16, true>;
extern template class object_finder_base<u32, false>;
extern template class object_finder_base<u32, true>;
extern template class object_finder_base<u64, false>;
extern template class object_finder_base<u64, true>;

extern template class object_finder_base<s8, false>;
extern template class object_finder_base<s8, true>;
extern template class object_finder_base<s16, false>;
extern template class object_finder_base<s16, true>;
extern template class object_finder_base<s32, false>;
extern template class object_finder_base<s32, true>;
extern template class object_finder_base<s64, false>;
extern template class object_finder_base<s64, true>;

extern template class memory_region_finder<false>;
extern template class memory_region_finder<true>;

extern template class memory_bank_finder<false>;
extern template class memory_bank_finder<true>;

extern template class ioport_finder<false>;
extern template class ioport_finder<true>;

extern template class region_ptr_finder<u8, false>;
extern template class region_ptr_finder<u8, true>;
extern template class region_ptr_finder<u16, false>;
extern template class region_ptr_finder<u16, true>;
extern template class region_ptr_finder<u32, false>;
extern template class region_ptr_finder<u32, true>;
extern template class region_ptr_finder<u64, false>;
extern template class region_ptr_finder<u64, true>;

extern template class region_ptr_finder<s8, false>;
extern template class region_ptr_finder<s8, true>;
extern template class region_ptr_finder<s16, false>;
extern template class region_ptr_finder<s16, true>;
extern template class region_ptr_finder<s32, false>;
extern template class region_ptr_finder<s32, true>;
extern template class region_ptr_finder<s64, false>;
extern template class region_ptr_finder<s64, true>;

extern template class shared_ptr_finder<u8, false>;
extern template class shared_ptr_finder<u8, true>;
extern template class shared_ptr_finder<u16, false>;
extern template class shared_ptr_finder<u16, true>;
extern template class shared_ptr_finder<u32, false>;
extern template class shared_ptr_finder<u32, true>;
extern template class shared_ptr_finder<u64, false>;
extern template class shared_ptr_finder<u64, true>;

extern template class shared_ptr_finder<s8, false>;
extern template class shared_ptr_finder<s8, true>;
extern template class shared_ptr_finder<s16, false>;
extern template class shared_ptr_finder<s16, true>;
extern template class shared_ptr_finder<s32, false>;
extern template class shared_ptr_finder<s32, true>;
extern template class shared_ptr_finder<s64, false>;
extern template class shared_ptr_finder<s64, true>;

#endif // MAME_EMU_DEVFIND_H
/** \} */
