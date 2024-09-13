// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    cheat.h

    Cheat system.

***************************************************************************/

#ifndef MAME_FRONTEND_CHEAT_H
#define MAME_FRONTEND_CHEAT_H

#pragma once

#include "debug/express.h"
#include "ui/text.h"
#include "xmlfile.h"


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
DECLARE_ENUM_INCDEC_OPERATORS(script_state)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cheat_manager;
class mame_ui_manager;


// ======================> number_and_format

// helper class to remember a format along with a number
class number_and_format
{
public:
	// construction/destruction
	constexpr number_and_format(
			uint64_t value = 0,
			util::xml::data_node::int_format format = util::xml::data_node::int_format::DECIMAL)
		: m_value(value)
		, m_format(format)
	{
	}

	// copyable/movable
	constexpr number_and_format(number_and_format const &) = default;
	number_and_format(number_and_format &&) = default;
	number_and_format &operator=(number_and_format const &) = default;
	number_and_format &operator=(number_and_format &&) = default;

	// pass-through to look like a regular number
	operator uint64_t &() { return m_value; }
	operator const uint64_t &() const { return m_value; }

	// format the number according to its format
	std::string format() const;

private:
	// internal state
	uint64_t                            m_value;
	util::xml::data_node::int_format    m_format;
};


// ======================> cheat_parameter

// a parameter for a cheat, which can be set in the UI
class cheat_parameter
{
public:
	// construction/destruction
	cheat_parameter(
			cheat_manager &manager,
			symbol_table &symbols,
			std::string const &filename,
			util::xml::data_node const &paramnode);

	// queries
	char const *text();
	bool has_itemlist() const { return !m_itemlist.empty(); }
	bool is_minimum() const { return (m_itemlist.empty() ? m_minval : m_itemlist.front().value()) == m_value; }
	bool is_maximum() const { return (m_itemlist.empty() ? m_maxval : m_itemlist.back().value()) == m_value; }

	// state setters
	bool set_minimum_state();
	bool set_prev_state();
	bool set_next_state();

	// actions
	void save(util::core_file &cheatfile) const;

private:
	// a single item in a parameter item list
	class item
	{
	public:
		// construction/destruction
		item(const char *text, uint64_t value, util::xml::data_node::int_format valformat)
			: m_text(text)
			, m_value(value, valformat)
		{
		}

		// copyable/movable
		item(item const &) = default;
		item(item &&) = default;
		item &operator=(item const &) = default;
		item &operator=(item &&) = default;

		// getters
		number_and_format const &value() const { return m_value; }
		char const *text() const { return m_text.c_str(); }

	private:
		// internal state
		std::string         m_text;     // name of the item
		number_and_format   m_value;    // value of the item
	};

	// internal state
	number_and_format   m_minval;       // minimum value
	number_and_format   m_maxval;       // maximum value
	number_and_format   m_stepval;      // step value
	uint64_t            m_value;        // live value of the parameter
	std::string         m_curtext;      // holding for a value string
	std::vector<item>   m_itemlist;     // list of items
};


// ======================> cheat_script

// a script entry, specifying which state to execute under
class cheat_script
{
public:
	// construction/destruction
	cheat_script(
			cheat_manager &manager,
			symbol_table &symbols,
			std::string const &filename,
			util::xml::data_node const &scriptnode);

	// getters
	script_state state() const { return m_state; }

	// actions
	void execute(cheat_manager &manager, uint64_t &argindex);
	void save(util::core_file &cheatfile) const;

private:
	// an entry within the script
	class script_entry
	{
	public:
		// construction/destruction
		script_entry(
				cheat_manager &manager,
				symbol_table &symbols,
				std::string const &filename,
				util::xml::data_node const &entrynode,
				bool isaction);

		// actions
		void execute(cheat_manager &manager, uint64_t &argindex);
		void save(util::core_file &cheatfile) const;

	private:
		// an argument for output
		class output_argument
		{
		public:
			// construction/destruction
			output_argument(
					cheat_manager &manager,
					symbol_table &symbols,
					std::string const &filename,
					util::xml::data_node const &argnode);

			// getters
			int count() const { return m_count; }
			int values(uint64_t &argindex, uint64_t *result);

			// actions
			void save(util::core_file &cheatfile) const;

		private:
			// internal state
			parsed_expression   m_expression;   // expression for argument
			uint64_t            m_count;        // number of repetitions
		};

		// internal helpers
		void validate_format(std::string const &filename, int line);

		// internal state
		parsed_expression                               m_condition;    // condition under which this is executed
		parsed_expression                               m_expression;   // expression to execute
		std::string                                     m_format;       // string format to print
		std::vector<std::unique_ptr<output_argument>>   m_arglist;      // list of arguments
		int8_t                                          m_line;         // which line to print on
		ui::text_layout::text_justify                   m_justify;      // justification when printing

		// constants
		static constexpr int MAX_ARGUMENTS = 32;
	};

	// internal state
	std::vector<std::unique_ptr<script_entry>>  m_entrylist;    // list of actions to perform
	script_state                                m_state;        // which state this script is for
};


// ======================> cheat_entry

// a single cheat
class cheat_entry
{
public:
	// construction/destruction
	cheat_entry(cheat_manager &manager, symbol_table &globaltable, std::string const &filename, util::xml::data_node const &cheatnode);
	~cheat_entry();

	// getters
	cheat_manager &manager() const { return m_manager; }
	script_state state() const { return m_state; }
	const char *description() const { return m_description.c_str(); }
	const char *comment() const { return m_comment.c_str(); }

	// script detection
	bool has_run_script() const { return (m_run_script != nullptr); }
	bool has_on_script() const { return (m_on_script != nullptr); }
	bool has_off_script() const { return (m_off_script != nullptr); }
	bool has_change_script() const { return (m_change_script != nullptr); }

	// script execution
	void execute_off_script() { if (has_off_script()) m_off_script->execute(m_manager, m_argindex); }
	void execute_on_script() { if (has_on_script()) m_on_script->execute(m_manager, m_argindex); }
	void execute_run_script() { if (has_run_script()) m_run_script->execute(m_manager, m_argindex); }
	void execute_change_script() { if (has_change_script()) m_change_script->execute(m_manager, m_argindex); }

	// cheat classification
	bool is_text_only() const { return (m_parameter == nullptr && !has_run_script() && !has_off_script() && !has_on_script()); }
	bool is_oneshot() const { return (m_parameter == nullptr && !has_run_script() && !has_off_script() && has_on_script()); }
	bool is_onoff() const { return (m_parameter == nullptr && (has_run_script() || (has_off_script() && has_on_script()))); }
	bool is_value_parameter() const { return (m_parameter != nullptr && !m_parameter->has_itemlist()); }
	bool is_itemlist_parameter() const { return (m_parameter != nullptr && m_parameter->has_itemlist()); }
	bool is_oneshot_parameter() const { return (m_parameter != nullptr && !has_run_script() && !has_off_script() && has_change_script()); }
	bool is_duplicate() const;

	// actions
	bool activate();
	bool select_default_state();
	bool select_previous_state();
	bool select_next_state();
	void save(util::core_file &cheatfile) const;

	// UI helpers
	void menu_text(std::string &description, std::string &state, uint32_t &flags);

	// per-frame update
	void frame_update() { if (m_state == SCRIPT_STATE_RUN) execute_run_script(); }

private:
	// internal helpers
	bool set_state(script_state newstate);
	std::unique_ptr<cheat_script> &script_for_state(script_state state);

	// internal state
	cheat_manager &                     m_manager;          // reference to our manager
	std::string                         m_description;      // string description/menu title
	std::string                         m_comment;          // comment data
	std::unique_ptr<cheat_parameter>    m_parameter;        // parameter
	std::unique_ptr<cheat_script>       m_on_script;        // script to run when turning on
	std::unique_ptr<cheat_script>       m_off_script;       // script to run when turning off
	std::unique_ptr<cheat_script>       m_change_script;    // script to run when value changes
	std::unique_ptr<cheat_script>       m_run_script;       // script to run each frame when on
	symbol_table                        m_symbols;          // symbol table for this cheat
	script_state                        m_state;            // current cheat state
	uint32_t                            m_numtemp;          // number of temporary variables
	uint64_t                            m_argindex;         // argument index variable

	// constants
	static constexpr int DEFAULT_TEMP_VARIABLES = 10;
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
	std::vector<std::unique_ptr<cheat_entry>> const &entries() const { return m_cheatlist; }

	// setters
	void set_enable(bool enable, bool show);

	// actions
	void reload();
	bool save_all(std::string const &filename);
	void render_text(mame_ui_manager &mui, render_container &container);

	// output helpers
	std::string &get_output_string(int row, ui::text_layout::text_justify justify);

	// global helpers
	static std::string quote_expression(parsed_expression const &expression);
	static uint64_t execute_frombcd(int params, uint64_t const *param);
	static uint64_t execute_tobcd(int params, uint64_t const *param);

private:
	// internal helpers
	void frame_update();
	void load_cheats(std::string const &filename);

	// internal state
	running_machine &                           m_machine;      // reference to our machine
	std::vector<std::unique_ptr<cheat_entry>>   m_cheatlist;    // cheat list
	uint64_t                                    m_framecount;   // frame count
	std::vector<std::string>                    m_output;       // array of output strings
	std::vector<ui::text_layout::text_justify>  m_justify;      // justification for each string
	uint8_t                                     m_numlines;     // number of lines available for output
	int8_t                                      m_lastline;     // last line used for output
	bool                                        m_disabled;     // true if the cheat engine is disabled
	symbol_table                                m_symtable;     // global symbol table

	// constants
	static constexpr int CHEAT_VERSION = 1;
};


#endif  /* MAME_FRONTEND_CHEAT_H */
