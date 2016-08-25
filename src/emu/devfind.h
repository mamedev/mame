// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/**
 * \file devfind.h
 * Object auto-discovery helpers
 * \defgroup devfind
 * \{
 * Object auto-disovery helpers
 */

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DEVFIND_H
#define MAME_EMU_DEVFIND_H

#pragma once

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
	template <unsigned... V> struct indices { };
	template <unsigned C, unsigned... V> struct range : public range<C - 1, C - 1, V...> { };
	template <unsigned... V> struct range<0U, V...> { typedef indices<V...> type; };
	template <unsigned C> using index_range = typename range<C>::type;

	template <typename F, unsigned... V>
	object_array_finder(device_t &base, F const &fmt, unsigned start, indices<V...>)
		: m_tag{ util::string_format(fmt, start + V)... }
		, m_array{ { base, m_tag[V].c_str() }... }
	{
	}

	template <unsigned... V>
	object_array_finder(device_t &base, std::array<char const *, Count> const &tags, indices<V...>)
		: m_array{ { base, tags[V] }... }
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
	/// \param [in] start Number to add to element index when calculating
	///   values for string format arguments.
	/// \sa util::string_format
	template <typename F> object_array_finder(device_t &base, F const &fmt, unsigned start) : object_array_finder(base, fmt, start, index_range<Count>()) { }

	/// \brief Construct with free-form list of tags
	///
	/// Specify arbitrary tags for objects.  Useful when there is no
	/// particular pattern to the object tags.
	/// \param [in] base Base device to search from.
	/// \param [in] tags Tags to search for, e.g. { "player", "dips" }.
	object_array_finder(device_t &base, std::array<char const *, Count> const &tags) : object_array_finder(base, tags, index_range<Count>()) { }

	/// \brief Element accessor (const)
	///
	/// Returns a const reference to the element at the supplied index.
	/// \param [in] index Index of desired element (zero-based).
	/// \return Reference to element at specified index.
	const T &operator[](unsigned index) const { assert(index < Count); return m_array[index]; }

	/// \brief Element accessor (non-const)
	///
	/// Returns a reference to the element at the supplied index.
	/// \param [in] index Index of desired element (zero-based).
	/// \return Reference to element at specified index.
	T &operator[](unsigned index) { assert(index < Count); return m_array[index]; }
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
	/// Destruction via base class pointer and dynmaic type behaviour
	/// are allowed.
	virtual ~finder_base();

	/// \brief Get next registered object discovery helper
	///
	/// Implementation of basic single-linked list behaviour.
	/// \return Pointer to the next registerd object discovery helper,
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

	/// \brief Get search tag
	///
	/// Returns the search tag.
	/// \return The object tag this helper will search for.
	const char *finder_tag() const { return m_tag; }

	/// \brief Set search tag
	///
	/// Allows search tag to be changed after construction.  Note that
	/// this must be done before resolution time to take effect.  Also
	/// note that the tag is not copied.
	/// \param [in] tag Updated search tag.  This is not copied, it is
	///   the caller's responsibility to ensure this pointer remains
	///   valid until resolution time.
	void set_tag(const char *tag) { m_tag = tag; }

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
	finder_base(device_t &base, const char *tag);

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
	/// \return Base pointer of the memeroy region if a matching region
	///   is found, or nullptr otherwise.
	void *find_memregion(UINT8 width, size_t &length, bool required) const;

	/// \brief Check that memory region exists
	///
	/// Walks ROM regions of all devices starting from the root looking
	/// for one with matching tag and length in bytes.  Prints a warning
	/// message if the region is required, a region iwth the requested
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
	void *find_memshare(UINT8 width, size_t &bytes, bool required) const;

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
	bool report_missing(bool found, const char *objname, bool required) const;

	/// \brief Print a message at warning level
	///
	/// Prints a message if logging is enabled at warning level or more
	/// detailed.  Uses printf semantics of the C runtime library.
	/// \param [in] format Format string as used by printf function in
	///   runtime library
	void printf_warning(const char *format, ...) ATTR_PRINTF(2,3);


	/// \brief Pointer to next registered discovery helper
	///
	/// This is a polymorphic class, so it can't be held in a standardlist
	/// container that requires elements of the same type.  Hence it
	/// implements basic single-linked list behaviour.
	finder_base *const m_next;
	device_t &m_base;
	const char *m_tag;
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
	object_finder_base(device_t &base, const char *tag) : finder_base(base, tag), m_target(nullptr) { }

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
	/// attempted or the seach failed.  Concrete derived classes must
	/// set this in their implementation of the findit member function.
	ObjectClass *m_target;
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
	// construction/destruction
	device_finder(device_t &base, const char *tag = finder_base::DUMMY_TAG) : object_finder_base<DeviceClass, Required>(base, tag) { }

	// make reference use transparent as well
	operator DeviceClass &() { assert(this->m_target); return *this->m_target; }

	// finder
	virtual bool findit(bool isvalidation = false) override
	{
		device_t *const device = this->m_base.subdevice(this->m_tag);
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
	// construction/destruction
	memory_region_finder(device_t &base, const char *tag = finder_base::DUMMY_TAG) : object_finder_base<memory_region, Required>(base, tag) { }

	// make reference use transparent as well
	operator memory_region &() const { assert(this->m_target); return *this->m_target; }

	// finder
	virtual bool findit(bool isvalidation = false) override
	{
		if (isvalidation) return this->validate_memregion(0, Required);
		this->m_target = this->m_base.memregion(this->m_tag);
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

extern template class object_finder_base<memory_region, false>;
extern template class object_finder_base<memory_region, true>;
extern template class memory_region_finder<false>;
extern template class memory_region_finder<true>;


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
	// construction/destruction
	memory_bank_finder(device_t &base, const char *tag = finder_base::DUMMY_TAG) : object_finder_base<memory_bank, Required>(base, tag) { }

	// make reference use transparent as well
	operator memory_bank &() const { assert(this->m_target); return *this->m_target; }

	// finder
	virtual bool findit(bool isvalidation = false) override
	{
		if (isvalidation) return true;
		this->m_target = this->m_base.membank(this->m_tag);
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

extern template class object_finder_base<memory_bank, false>;
extern template class object_finder_base<memory_bank, true>;
extern template class memory_bank_finder<false>;
extern template class memory_bank_finder<true>;


// ======================> ioport_finder

// ioport finder template
template <bool Required>
class ioport_finder : public object_finder_base<ioport_port, Required>
{
public:
	// construction/destruction
	ioport_finder(device_t &base, const char *tag = finder_base::DUMMY_TAG) : object_finder_base<ioport_port, Required>(base, tag) { }

	// read if found, or else return a default value
	ioport_value read_safe(ioport_value defval) { return this->m_target ? this->m_target->read() : defval; }

	// finder
	virtual bool findit(bool isvalidation = false) override
	{
		if (isvalidation) return true;
		this->m_target = this->m_base.ioport(this->m_tag);
		return this->report_missing("I/O port");
	}
};

using optional_ioport = ioport_finder<false>;
using required_ioport = ioport_finder<true>;
template <unsigned Count, bool Required> using ioport_array_finder = object_array_finder<ioport_finder<Required>, Count>;
template <unsigned Count> using optional_ioport_array = ioport_array_finder<Count, false>;
template <unsigned Count> using required_ioport_array = ioport_array_finder<Count, true>;

extern template class object_finder_base<ioport_port, false>;
extern template class object_finder_base<ioport_port, true>;
extern template class ioport_finder<false>;
extern template class ioport_finder<true>;


// ======================> region_ptr_finder

// memory region pointer finder template
template <typename PointerType, bool Required>
class region_ptr_finder : public object_finder_base<PointerType, Required>
{
public:
	// construction/destruction
	region_ptr_finder(device_t &base, const char *tag, size_t length = 0)
		: object_finder_base<PointerType, Required>(base, tag)
		, m_length(length)
	{
	}
	region_ptr_finder(device_t &base, size_t length = 0)
		: object_finder_base<PointerType, Required>(base, finder_base::DUMMY_TAG)
		, m_length(length)
	{
	}

	// operators to make use transparent
	const PointerType &operator[](int index) const { assert(index < m_length); return this->m_target[index]; }
	PointerType &operator[](int index) { assert(index < m_length); return this->m_target[index]; }

	// getter for explicit fetching
	UINT32 length() const { return m_length; }
	UINT32 bytes() const { return m_length * sizeof(PointerType); }
	UINT32 mask() const { return m_length - 1; } // only valid if length is known to be a power of 2

	// finder
	virtual bool findit(bool isvalidation = false) override
	{
		if (isvalidation) return this->validate_memregion(sizeof(PointerType) * m_length, Required);
		this->m_target = reinterpret_cast<PointerType *>(this->find_memregion(sizeof(PointerType), m_length, Required));
		return this->report_missing("memory region");
	}

protected:
	// internal state
	size_t m_length;
};

template <typename PointerType> using optional_region_ptr = region_ptr_finder<PointerType, false>;
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
	shared_ptr_finder(device_t &base, const char *tag = finder_base::DUMMY_TAG, UINT8 width = sizeof(PointerType) * 8)
		: object_finder_base<PointerType, Required>(base, tag)
		, m_bytes(0)
		, m_width(width)
	{
	}

	// operators to make use transparent
	const PointerType &operator[](int index) const { return this->m_target[index]; }
	PointerType &operator[](int index) { return this->m_target[index]; }

	// getter for explicit fetching
	UINT32 bytes() const { return m_bytes; }
	UINT32 mask() const { return m_bytes - 1; } // FIXME: wrong when sizeof(PointerType) != 1

	// setter for setting the object
	void set_target(PointerType *target, size_t bytes) { this->m_target = target; m_bytes = bytes; }

	// dynamic allocation of a shared pointer
	void allocate(UINT32 entries)
	{
		assert(m_allocated.empty());
		m_allocated.resize(entries);
		this->m_target = &m_allocated[0];
		m_bytes = entries * sizeof(PointerType);
		this->m_base.save_item(this->m_allocated, this->m_tag);
	}

	// finder
	virtual bool findit(bool isvalidation = false) override
	{
		if (isvalidation) return true;
		this->m_target = reinterpret_cast<PointerType *>(this->find_memshare(m_width, m_bytes, Required));
		return this->report_missing("shared pointer");
	}

protected:
	// internal state
	size_t m_bytes;
	UINT8 m_width;
	std::vector<PointerType> m_allocated;
};

template <typename PointerType> using optional_shared_ptr = shared_ptr_finder<PointerType, false>;
template <typename PointerType> using required_shared_ptr = shared_ptr_finder<PointerType, true>;
template <typename PointerType, unsigned Count, bool Required> using shared_ptr_array_finder = object_array_finder<shared_ptr_finder<PointerType, Required>, Count>;
template <typename PointerType, unsigned Count> using optional_shared_ptr_array = shared_ptr_array_finder<PointerType, Count, false>;
template <typename PointerType, unsigned Count> using required_shared_ptr_array = shared_ptr_array_finder<PointerType, Count, true>;

#endif // MAME_EMU_DEVFIND_H
/** \} */
