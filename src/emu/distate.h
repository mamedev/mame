// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    distate.h

    Device state interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DISTATE_H
#define MAME_EMU_DISTATE_H


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// standard state indexes
enum
{
	STATE_GENPC = -1,               // generic program counter (live)
	STATE_GENPCBASE = -2,           // generic program counter (base of current instruction)
	STATE_GENSP = -3,               // generic stack pointer
	STATE_GENFLAGS = -4             // generic flags
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> device_state_entry

// class describing a single item of exposed device state
class device_state_entry
{
	friend class device_state_interface;
public:
	// construction/destruction
	device_state_entry(int index, const char *symbol, u8 size, u64 sizemask, u8 flags, device_state_interface *dev);
	device_state_entry(int index, device_state_interface *dev);
	virtual ~device_state_entry();

public:
	// post-construction modifiers
	device_state_entry &mask(u64 _mask) { m_datamask = _mask; format_from_mask(); return *this; }
	device_state_entry &signed_mask(u64 _mask) { m_datamask = _mask; m_flags |= DSF_IMPORT_SEXT; format_from_mask(); return *this; }
	device_state_entry &formatstr(const char *_format);
	device_state_entry &callimport() { m_flags |= DSF_IMPORT; return *this; }
	device_state_entry &callexport() { m_flags |= DSF_EXPORT; return *this; }
	device_state_entry &noshow() { m_flags |= DSF_NOSHOW; return *this; }
	device_state_entry &readonly() { m_flags |= DSF_READONLY; return *this; }

	// query information
	int index() const { return m_index; }
	void *dataptr() const { return entry_baseptr(); }
	u64 datamask() const { return m_datamask; }
	const char *symbol() const { return m_symbol.c_str(); }
	bool visible() const { return ((m_flags & DSF_NOSHOW) == 0); }
	bool writeable() const { return ((m_flags & DSF_READONLY) == 0); }
	bool divider() const { return m_flags & DSF_DIVIDER; }
	bool is_float() const { return m_flags & DSF_FLOATING_POINT; }
	device_state_interface *parent_state() const {return m_device_state;}
	const std::string &format_string() const { return m_format; }

protected:
	// device state flags
	static constexpr u8 DSF_NOSHOW          = 0x01; // don't display this entry in the registers view
	static constexpr u8 DSF_IMPORT          = 0x02; // call the import function after writing new data
	static constexpr u8 DSF_IMPORT_SEXT     = 0x04; // sign-extend the data when writing new data
	static constexpr u8 DSF_EXPORT          = 0x08; // call the export function prior to fetching the data
	static constexpr u8 DSF_CUSTOM_STRING   = 0x10; // set if the format has a custom string
	static constexpr u8 DSF_DIVIDER         = 0x20; // set if this is a divider entry
	static constexpr u8 DSF_READONLY        = 0x40; // set if this entry does not permit writes
	static constexpr u8 DSF_FLOATING_POINT  = 0x80; // set if this entry represents a floating-point value

	// helpers
	bool needs_custom_string() const { return ((m_flags & DSF_CUSTOM_STRING) != 0); }
	void format_from_mask();

	// return the current value -- only for our friends who handle export
	bool needs_export() const { return ((m_flags & DSF_EXPORT) != 0); }
	u64 value() const { return entry_value() & m_datamask; }
	double dvalue() const { return entry_dvalue(); }
	std::string format(const char *string, bool maxout = false) const;

	// set the current value -- only for our friends who handle import
	bool needs_import() const { return ((m_flags & DSF_IMPORT) != 0); }
	void set_value(u64 value) const;
	void set_dvalue(double value) const;
	void set_value(const char *string) const;

	// overrides
	virtual void *entry_baseptr() const;
	virtual u64 entry_value() const;
	virtual void entry_set_value(u64 value) const;
	virtual double entry_dvalue() const;
	virtual void entry_set_dvalue(double value) const;

	// statics
	static const u64 k_decimal_divisor[20];      // divisors for outputting decimal values

	// public state description
	device_state_interface *m_device_state;         // link to parent device state
	u32                     m_index;                // index by which this item is referred
	u64                     m_datamask;             // mask that applies to the data
	u8                      m_datasize;             // size of the data
	u8                      m_flags;                // flags for this data
	std::string             m_symbol;               // symbol for display; all lower-case version for expressions
	std::string             m_format;               // supported formats
	bool                    m_default_format;       // true if we are still using default format
	u64                     m_sizemask;             // mask derived from the data size
};


// ======================> device_state_register

// class template representing a state register of a specific width
template<class ItemType>
class device_state_register : public device_state_entry
{
public:
	// construction/destruction
	device_state_register(int index, const char *symbol, ItemType &data, device_state_interface *dev)
		: device_state_entry(index, symbol, sizeof(ItemType), std::numeric_limits<typename std::make_unsigned<ItemType>::type>::max(), 0, dev),
			m_data(data)
	{
		static_assert(std::is_integral<ItemType>().value, "Registration of non-integer types is not currently supported");
	}

protected:
	// device_state_entry overrides
	virtual void *entry_baseptr() const override { return &m_data; }
	virtual u64 entry_value() const override { return m_data; }
	virtual void entry_set_value(u64 value) const override { m_data = value; }

private:
	ItemType &              m_data;                 // reference to where the data lives
};

// class template representing a boolean state register
template<>
class device_state_register<bool> : public device_state_entry
{
public:
	// construction/destruction
	device_state_register(int index, const char *symbol, bool &data, device_state_interface *dev)
		: device_state_entry(index, symbol, sizeof(bool), 1, 0, dev),
			m_data(data)
	{
	}

protected:
	// device_state_entry overrides
	virtual void *entry_baseptr() const override { return &m_data; }
	virtual u64 entry_value() const override { return m_data; }
	virtual void entry_set_value(u64 value) const override { m_data = bool(value); }

private:
	bool &                  m_data;                 // reference to where the data lives
};

// class template representing a floating-point state register
template<>
class device_state_register<double> : public device_state_entry
{
public:
	// construction/destruction
	device_state_register(int index, const char *symbol, double &data, device_state_interface *dev)
		: device_state_entry(index, symbol, sizeof(double), ~u64(0), DSF_FLOATING_POINT, dev),
			m_data(data)
	{
	}

protected:
	// device_state_entry overrides
	virtual void *entry_baseptr() const override { return &m_data; }
	virtual u64 entry_value() const override { return u64(m_data); }
	virtual void entry_set_value(u64 value) const override { m_data = double(value); }
	virtual double entry_dvalue() const override { return m_data; }
	virtual void entry_set_dvalue(double value) const override { m_data = value; }

private:
	double &                m_data;                 // reference to where the data lives
};


// ======================> device_state_register

// class template representing a state register of a specific width
template<class ItemType>
class device_pseudo_state_register : public device_state_entry
{
public:
	typedef typename std::function<ItemType ()> getter_func;
	typedef typename std::function<void (ItemType)> setter_func;

	// construction/destruction
	device_pseudo_state_register(int index, const char *symbol, getter_func &&getter, setter_func &&setter, device_state_interface *dev)
		: device_state_entry(index, symbol, sizeof(ItemType), std::numeric_limits<ItemType>::max(), 0, dev),
			m_getter(std::move(getter)),
			m_setter(std::move(setter))
	{
	}

protected:
	// device_state_entry overrides
	virtual u64 entry_value() const override { return m_getter(); }
	virtual void entry_set_value(u64 value) const override { m_setter(value); }

private:
	getter_func             m_getter;               // function to retrieve the data
	setter_func             m_setter;               // function to store the data
};


// ======================> device_state_interface

// class representing interface-specific live state
class device_state_interface : public device_interface
{
public:
	// construction/destruction
	device_state_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_state_interface();

	// configuration access
	const std::vector<std::unique_ptr<device_state_entry>> &state_entries() const { return m_state_list; }

	// state getters
	u64 state_int(int index);
	std::string state_string(int index) const;
	int state_string_max_length(int index);
	offs_t pc() { return state_int(STATE_GENPC); }
	offs_t pcbase() { return state_int(STATE_GENPCBASE); }
	offs_t sp() { return state_int(STATE_GENSP); }
	u64 flags() { return state_int(STATE_GENFLAGS); }

	// state setters
	void set_state_int(int index, u64 value);
	void set_state_string(int index, const char *string);
	void set_pc(offs_t pc) { set_state_int(STATE_GENPC, pc); }

	// deliberately ambiguous functions; if you have the state interface
	// just use it directly
	device_state_interface &state() { return *this; }

public: // protected eventually

	// add a new state register item
	template<class ItemType> device_state_entry &state_add(int index, const char *symbol, ItemType &data)
	{
		assert(symbol != nullptr);
		return state_add(std::make_unique<device_state_register<ItemType>>(index, symbol, data, this));
	}

	// add a new state pseudo-register item (template argument must be explicit)
	template<class ItemType> device_state_entry &state_add(int index, const char *symbol,
					typename device_pseudo_state_register<ItemType>::getter_func &&getter,
					typename device_pseudo_state_register<ItemType>::setter_func &&setter)
	{
		assert(symbol != nullptr);
		return state_add(std::make_unique<device_pseudo_state_register<ItemType>>(index, symbol, std::move(getter), std::move(setter), this));
	}
	template<class ItemType> device_state_entry &state_add(int index, const char *symbol,
					typename device_pseudo_state_register<ItemType>::getter_func &&getter)
	{
		assert(symbol != nullptr);
		return state_add(std::make_unique<device_pseudo_state_register<ItemType>>(index, symbol, std::move(getter), [](ItemType){}, this)).readonly();
	}

	device_state_entry &state_add(std::unique_ptr<device_state_entry> &&entry);

	// add a new divider entry
	device_state_entry &state_add_divider(int index) { return state_add(std::make_unique<device_state_entry>(index, this)); }

protected:
	// derived class overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_import(const device_state_entry &entry, std::string &str);
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const;

	// internal operation overrides
	virtual void interface_post_start() override;

	// find the entry for a given index
	const device_state_entry *state_find_entry(int index) const;

	// constants
	static constexpr int FAST_STATE_MIN = -4;                           // range for fast state
	static constexpr int FAST_STATE_MAX = 256;                          // lookups

	// state
	std::vector<std::unique_ptr<device_state_entry>>       m_state_list;           // head of state list
	device_state_entry *                    m_fast_state[FAST_STATE_MAX + 1 - FAST_STATE_MIN];
																	// fast access to common entries
};

// iterator
typedef device_interface_iterator<device_state_interface> state_interface_iterator;

#endif  /* MAME_EMU_DISTATE_H */
