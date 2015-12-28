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

#ifndef __DISTATE_H__
#define __DISTATE_H__


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
	friend class simple_list<device_state_entry>;

private:
	// construction/destruction
	device_state_entry(int index, const char *symbol, void *dataptr, UINT8 size, device_state_interface *dev);
	device_state_entry(int index, device_state_interface *dev);

public:
	// post-construction modifiers
	device_state_entry &mask(UINT64 _mask) { m_datamask = _mask; format_from_mask(); return *this; }
	device_state_entry &signed_mask(UINT64 _mask) { m_datamask = _mask; m_flags |= DSF_IMPORT_SEXT; format_from_mask(); return *this; }
	device_state_entry &formatstr(const char *_format);
	device_state_entry &callimport() { m_flags |= DSF_IMPORT; return *this; }
	device_state_entry &callexport() { m_flags |= DSF_EXPORT; return *this; }
	device_state_entry &noshow() { m_flags |= DSF_NOSHOW; return *this; }

	// iteration helpers
	const device_state_entry *next() const { return m_next; }

	// query information
	int index() const { return m_index; }
	void *dataptr() const { return m_dataptr.v; }
	const char *symbol() const { return m_symbol.c_str(); }
	bool visible() const { return ((m_flags & DSF_NOSHOW) == 0); }
	bool divider() const { return m_flags & DSF_DIVIDER; }
	device_state_interface *parent_state() const {return m_device_state;}

protected:
	// device state flags
	static const UINT8 DSF_NOSHOW =         0x01;   // don't display this entry in the registers view
	static const UINT8 DSF_IMPORT =         0x02;   // call the import function after writing new data
	static const UINT8 DSF_IMPORT_SEXT =    0x04;   // sign-extend the data when writing new data
	static const UINT8 DSF_EXPORT =         0x08;   // call the export function prior to fetching the data
	static const UINT8 DSF_CUSTOM_STRING =  0x10;   // set if the format has a custom string
	static const UINT8 DSF_DIVIDER       =  0x20;   // set if this is a divider entry

	// helpers
	bool needs_custom_string() const { return ((m_flags & DSF_CUSTOM_STRING) != 0); }
	void format_from_mask();

	// return the current value -- only for our friends who handle export
	bool needs_export() const { return ((m_flags & DSF_EXPORT) != 0); }
	UINT64 value() const;
	std::string &format(std::string &dest, const char *string, bool maxout = false) const;

	// set the current value -- only for our friends who handle import
	bool needs_import() const { return ((m_flags & DSF_IMPORT) != 0); }
	void set_value(UINT64 value) const;
	void set_value(const char *string) const;

	// statics
	static const UINT64 k_decimal_divisor[20];      // divisors for outputting decimal values

	// public state description
	device_state_interface *m_device_state;         // link to parent device state
	device_state_entry *    m_next;                 // link to next item
	UINT32                  m_index;                // index by which this item is referred
	generic_ptr             m_dataptr;              // pointer to where the data lives
	UINT64                  m_datamask;             // mask that applies to the data
	UINT8                   m_datasize;             // size of the data
	UINT8                   m_flags;                // flags for this data
	std::string             m_symbol;               // symbol for display; all lower-case version for expressions
	std::string             m_format;               // supported formats
	bool                    m_default_format;       // true if we are still using default format
	UINT64                  m_sizemask;             // mask derived from the data size
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
	const device_state_entry *state_first() const { return m_state_list.first(); }

	// state getters
	UINT64 state_int(int index);
	std::string &state_string(int index, std::string &dest);
	int state_string_max_length(int index);
	offs_t pc() { return state_int(STATE_GENPC); }
	offs_t pcbase() { return state_int(STATE_GENPCBASE); }
	offs_t sp() { return state_int(STATE_GENSP); }
	UINT64 flags() { return state_int(STATE_GENFLAGS); }

	// state setters
	void set_state_int(int index, UINT64 value);
	void set_state_string(int index, const char *string);
	void set_pc(offs_t pc) { set_state_int(STATE_GENPC, pc); }

	// deliberately ambiguous functions; if you have the state interface
	// just use it or pc() and pcbase() directly
	device_state_interface &state() { return *this; }
	offs_t safe_pc() { return pc(); }
	offs_t safe_pcbase() { return pcbase(); }

public: // protected eventually

	// add a new state item
	template<class _ItemType> device_state_entry &state_add(int index, const char *symbol, _ItemType &data)
	{
		return state_add(index, symbol, &data, sizeof(data));
	}
	device_state_entry &state_add(int index, const char *symbol, void *data, UINT8 size);

	// add a new divider entry
	device_state_entry &state_add_divider(int index);

protected:
	// derived class overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_import(const device_state_entry &entry, std::string &str);
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	// internal operation overrides
	virtual void interface_post_start() override;

	// find the entry for a given index
	const device_state_entry *state_find_entry(int index);

	// constants
	static const int FAST_STATE_MIN = -4;                           // range for fast state
	static const int FAST_STATE_MAX = 256;                          // lookups

	// state
	simple_list<device_state_entry>         m_state_list;           // head of state list
	device_state_entry *                    m_fast_state[FAST_STATE_MAX + 1 - FAST_STATE_MIN];
																	// fast access to common entries
};

// iterator
typedef device_interface_iterator<device_state_interface> state_interface_iterator;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  device_t::safe_pc - return the current PC
//  or 0 if no state object exists
//-------------------------------------------------

inline offs_t device_t::safe_pc() const
{
	return (m_state != nullptr) ? m_state->pc() : 0;
}


//-------------------------------------------------
//  device_t::safe_pcbase - return the current PC
//  base or 0 if no state object exists
//-------------------------------------------------

inline offs_t device_t::safe_pcbase() const
{
	return (m_state != nullptr) ? m_state->pcbase() : 0;
}


#endif  /* __DISTATE_H__ */
