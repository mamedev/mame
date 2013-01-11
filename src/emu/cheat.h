/***************************************************************************

    cheat.h

    Cheat system.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __CHEAT_H__
#define __CHEAT_H__

#include "debug/express.h"
#include "ui.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum script_state
{
	SCRIPT_STATE_OFF = 0,
	SCRIPT_STATE_ON,
	SCRIPT_STATE_RUN,
	SCRIPT_STATE_CHANGE,
	SCRIPT_STATE_COUNT
};
DECLARE_ENUM_OPERATORS(script_state)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cheat_manager;


// ======================> number_and_format

// helper class to remember a format along with a number
class number_and_format
{
public:
	// construction/destruction
	number_and_format(UINT64 value = 0, int format = 0)
		: m_value(value),
			m_format(format) { }

	// pass-through to look like a regular number
	operator UINT64 &() { return m_value; }
	operator const UINT64 &() const { return m_value; }

	// format the number according to its format
	const char *format(astring &string) const;

private:
	// internal state
	UINT64          m_value;
	int             m_format;
};


// ======================> cheat_parameter

// a parameter for a cheat, which can be set in the UI
class cheat_parameter
{
public:
	// construction/destruction
	cheat_parameter(cheat_manager &manager, symbol_table &symbols, const char *filename, xml_data_node &paramnode);

	// queries
	const char *text();
	bool has_itemlist() const { return (m_itemlist.count() != 0); }
	bool is_minimum() const { return (m_value == ((m_itemlist.count() == 0) ? m_minval : m_itemlist.first()->value())); }
	bool is_maximum() const { return (m_value == ((m_itemlist.count() == 0) ? m_maxval : m_itemlist.last()->value())); }

	// state setters
	bool set_minimum_state();
	bool set_prev_state();
	bool set_next_state();

	// actions
	void save(emu_file &cheatfile) const;

private:
	// a single item in a parameter item list
	class item
	{
		friend class simple_list<item>;

	public:
		// construction/destruction
		item(const char *text, UINT64 value, int valformat)
			: m_next(NULL),
				m_text(text),
				m_value(value, valformat) { }

		// getters
		item *next() const { return m_next; }
		const number_and_format &value() const { return m_value; }
		const char *text() const { return m_text; }

	private:
		// internal state
		item *              m_next;                         // next item in list
		astring             m_text;                         // name of the item
		number_and_format   m_value;                        // value of the item
	};

	// internal state
	number_and_format   m_minval;                       // minimum value
	number_and_format   m_maxval;                       // maximum value
	number_and_format   m_stepval;                      // step value
	UINT64              m_value;                        // live value of the parameter
	astring             m_curtext;                      // holding for a value string
	simple_list<item>   m_itemlist;                     // list of items
};


// ======================> cheat_script

// a script entry, specifying which state to execute under
class cheat_script
{
	friend class simple_list<cheat_script>;

public:
	// construction/destruction
	cheat_script(cheat_manager &manager, symbol_table &symbols, const char *filename, xml_data_node &scriptnode);

	// getters
	script_state state() const { return m_state; }

	// actions
	void execute(cheat_manager &manager, UINT64 &argindex);
	void save(emu_file &cheatfile) const;

private:
	// an entry within the script
	class script_entry
	{
		friend class simple_list<script_entry>;

	public:
		// construction/destruction
		script_entry(cheat_manager &manager, symbol_table &symbols, const char *filename, xml_data_node &entrynode, bool isaction);

		// getters
		script_entry *next() const { return m_next; }

		// actions
		void execute(cheat_manager &manager, UINT64 &argindex);
		void save(emu_file &cheatfile) const;

	private:
		// an argument for output
		class output_argument
		{
			friend class simple_list<output_argument>;

		public:
			// construction/destruction
			output_argument(cheat_manager &manager, symbol_table &symbols, const char *filename, xml_data_node &argnode);

			// getters
			output_argument *next() const { return m_next; }
			int count() const { return m_count; }
			int values(UINT64 &argindex, UINT64 *result);

			// actions
			void save(emu_file &cheatfile) const;

		private:
			// internal state
			output_argument *   m_next;                         // link to next argument
			parsed_expression   m_expression;                   // expression for argument
			UINT64              m_count;                        // number of repetitions
		};

		// internal helpers
		void validate_format(const char *filename, int line);

		// internal state
		script_entry *      m_next;                         // link to next entry
		parsed_expression   m_condition;                    // condition under which this is executed
		parsed_expression   m_expression;                   // expression to execute
		astring             m_format;                       // string format to print
		simple_list<output_argument> m_arglist;             // list of arguments
		INT8                m_line;                         // which line to print on
		UINT8               m_justify;                      // justification when printing

		// constants
		static const int MAX_ARGUMENTS = 32;
	};

	// internal state
	simple_list<script_entry> m_entrylist;              // list of actions to perform
	script_state        m_state;                        // which state this script is for
};


// ======================> cheat_entry

// a single cheat
class cheat_entry
{
	friend class simple_list<cheat_entry>;

public:
	// construction/destruction
	cheat_entry(cheat_manager &manager, symbol_table &globaltable, const char *filename, xml_data_node &cheatnode);
	~cheat_entry();

	// getters
	cheat_manager &manager() const { return m_manager; }
	cheat_entry *next() const { return m_next; }
	script_state state() const { return m_state; }
	const char *description() const { return m_description; }
	const char *comment() const { return m_comment; }

	// script detection
	bool has_run_script() const { return (m_run_script != NULL); }
	bool has_on_script() const { return (m_on_script != NULL); }
	bool has_off_script() const { return (m_off_script != NULL); }
	bool has_change_script() const { return (m_change_script != NULL); }

	// script execution
	void execute_off_script() { if (has_off_script()) m_off_script->execute(m_manager, m_argindex); }
	void execute_on_script() { if (has_on_script()) m_on_script->execute(m_manager, m_argindex); }
	void execute_run_script() { if (has_run_script()) m_run_script->execute(m_manager, m_argindex); }
	void execute_change_script() { if (has_change_script()) m_change_script->execute(m_manager, m_argindex); }

	// cheat classification
	bool is_text_only() const { return (m_parameter == NULL && !has_run_script() && !has_off_script() && !has_on_script()); }
	bool is_oneshot() const { return (m_parameter == NULL && !has_run_script() && !has_off_script() && has_on_script()); }
	bool is_onoff() const { return (m_parameter == NULL && (has_run_script() || (has_off_script() && has_on_script()))); }
	bool is_value_parameter() const { return (m_parameter != NULL && !m_parameter->has_itemlist()); }
	bool is_itemlist_parameter() const { return (m_parameter != NULL && m_parameter->has_itemlist()); }
	bool is_oneshot_parameter() const { return (m_parameter != NULL && !has_run_script() && !has_off_script() && has_change_script()); }

	// actions
	bool activate();
	bool select_default_state();
	bool select_previous_state();
	bool select_next_state();
	void save(emu_file &cheatfile) const;

	// UI helpers
	void menu_text(astring &description, astring &state, UINT32 &flags);

	// per-frame update
	void frame_update() { if (m_state == SCRIPT_STATE_RUN) execute_run_script(); }

private:
	// internal helpers
	bool set_state(script_state newstate);
	cheat_script *&script_for_state(script_state state);

	// internal state
	cheat_manager &     m_manager;                      // reference to our manager
	cheat_entry *       m_next;                         // next cheat entry
	astring             m_description;                  // string description/menu title
	astring             m_comment;                      // comment data
	cheat_parameter *   m_parameter;                    // parameter
	cheat_script *      m_on_script;                    // script to run when turning on
	cheat_script *      m_off_script;                   // script to run when turning off
	cheat_script *      m_change_script;                // script to run when value changes
	cheat_script *      m_run_script;                   // script to run each frame when on
	symbol_table        m_symbols;                      // symbol table for this cheat
	script_state        m_state;                        // current cheat state
	UINT32              m_numtemp;                      // number of temporary variables
	UINT64              m_argindex;                     // argument index variable

	// constants
	static const int DEFAULT_TEMP_VARIABLES = 10;
};


// ======================> cheat_manager

// private machine-global data
class cheat_manager
{
public:
	// construction/destruction
	cheat_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	bool enabled() const { return !m_disabled; }
	cheat_entry *first() const { return m_cheatlist.first(); }

	// setters
	void set_enable(bool enable = true);

	// actions
	void reload();
	bool save_all(const char *filename);
	void render_text(render_container &container);

	// output helpers
	astring &get_output_astring(int row, int justify);

	// global helpers
	static const char *quote_expression(astring &string, const parsed_expression &expression);
	static UINT64 execute_frombcd(symbol_table &table, void *ref, int params, const UINT64 *param);
	static UINT64 execute_tobcd(symbol_table &table, void *ref, int params, const UINT64 *param);

private:
	// internal helpers
	void frame_update();
	void load_cheats(const char *filename);

	// internal state
	running_machine &   m_machine;                          // reference to our machine
	simple_list<cheat_entry> m_cheatlist;                   // cheat list
	UINT64              m_framecount;                       // frame count
	astring             m_output[UI_TARGET_FONT_ROWS*2];    // array of output strings
	UINT8               m_justify[UI_TARGET_FONT_ROWS*2];   // justification for each string
	UINT8               m_numlines;                         // number of lines available for output
	INT8                m_lastline;                         // last line used for output
	bool                m_disabled;                         // true if the cheat engine is disabled
	symbol_table        m_symtable;                         // global symbol table

	// constants
	static const int CHEAT_VERSION = 1;
};


#endif  /* __CHEAT_H__ */
