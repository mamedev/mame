// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    output.h

    General purpose output routines.
***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __OUTPUT_H__
#define __OUTPUT_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*output_notifier_func)(const char *outname, INT32 value, void *param);

// ======================> output_manager

class output_manager
{
	class output_notify
	{
	public:
		output_notify(output_notifier_func callback, void *param)
			: m_notifier(callback),
			m_param(param) { }

		output_notifier_func    m_notifier;       // callback to call
		void *                  m_param;          // parameter to pass the callback
	};


	class output_item
	{
	public:
		std::string         name;           // string name of the item
		UINT32              hash;           // hash for this item name
		UINT32              id;             // unique ID for this item
		INT32               value;          // current value
		std::vector<output_notify> notifylist;     // list of notifier callbacks
	};

public:
	// construction/destruction
	output_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }

	// set the value for a given output
	void set_value(const char *outname, INT32 value);

	// set an indexed value for an output (concatenates basename + index)
	void set_indexed_value(const char *basename, int index, int value);

	// return the current value for a given output
	INT32 get_value(const char *outname);

	// return the current value for a given indexed output
	INT32 get_indexed_value(const char *outname, int index);

	// set a notifier on a particular output, or globally if NULL
	void set_notifier(const char *outname, output_notifier_func callback, void *param);

	// set a notifier on a particular output, or globally if NULL
	void notify_all(output_notifier_func callback, void *param);

	// map a name to a unique ID
	UINT32 name_to_id(const char *outname);

	// map a unique ID back to a name
	const char *id_to_name(UINT32 id);


	// helpers
	void set_led_value(int index, int value) { set_indexed_value("led", index, value ? 1 : 0); }
	void set_lamp_value(int index, int value) { set_indexed_value("lamp", index, value); }
	void set_digit_value(int index, int value) { set_indexed_value("digit", index, value); }
	INT32 get_led_value(int index) { return get_indexed_value("led", index); }
	INT32 get_lamp_value(int index) { return get_indexed_value("lamp", index); }
	INT32 get_digit_value(int index) { return get_indexed_value("digit", index); }

	void pause();
	void resume();
private:
	output_item *find_item(const char *string);
	output_item *create_new_item(const char *outname, INT32 value);

	// internal state
	running_machine &   m_machine;                  // reference to our machine
	std::unordered_map<std::string,output_item> m_itemtable;
	std::vector<output_notify> m_global_notifylist;
	UINT32 m_uniqueid;
};

#endif  // __OUTPUT_H__
