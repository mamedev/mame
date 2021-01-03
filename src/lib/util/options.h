// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    options.h

    Core options code code

***************************************************************************/

#ifndef MAME_LIB_UTIL_OPTIONS_H
#define MAME_LIB_UTIL_OPTIONS_H

#include "corefile.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// option priorities
const int OPTION_PRIORITY_DEFAULT   = 0;            // defaults are at 0 priority
const int OPTION_PRIORITY_LOW       = 50;           // low priority
const int OPTION_PRIORITY_NORMAL    = 100;          // normal priority
const int OPTION_PRIORITY_HIGH      = 150;          // high priority
const int OPTION_PRIORITY_MAXIMUM   = 255;          // maximum priority


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct options_entry;

// exception thrown by core_options when an illegal request is made
class options_exception : public std::exception
{
public:
	const std::string &message() const { return m_message; }
	virtual const char *what() const noexcept override { return message().c_str(); }

protected:
	options_exception(std::string &&message);

private:
	std::string m_message;
};

class options_warning_exception : public options_exception
{
public:
	template <typename... Params> options_warning_exception(const char *fmt, Params &&...args)
		: options_warning_exception(util::string_format(fmt, std::forward<Params>(args)...))
	{
	}

	options_warning_exception(std::string &&message);
	options_warning_exception(const options_warning_exception &) = default;
	options_warning_exception(options_warning_exception &&) = default;
	options_warning_exception& operator=(const options_warning_exception &) = default;
	options_warning_exception& operator=(options_warning_exception &&) = default;
};

class options_error_exception : public options_exception
{
public:
	template <typename... Params> options_error_exception(const char *fmt, Params &&...args)
		: options_error_exception(util::string_format(fmt, std::forward<Params>(args)...))
	{
	}

	options_error_exception(std::string &&message);
	options_error_exception(const options_error_exception &) = default;
	options_error_exception(options_error_exception &&) = default;
	options_error_exception& operator=(const options_error_exception &) = default;
	options_error_exception& operator=(options_error_exception &&) = default;
};


// structure holding information about a collection of options
class core_options
{
	static const int MAX_UNADORNED_OPTIONS = 16;

public:
	enum class option_type
	{
		INVALID,         // invalid
		HEADER,          // a header item
		COMMAND,         // a command
		BOOLEAN,         // boolean option
		INTEGER,         // integer option
		FLOAT,           // floating-point option
		STRING           // string option
	};

	// information about a single entry in the options
	class entry
	{
	public:
		typedef std::shared_ptr<entry> shared_ptr;
		typedef std::shared_ptr<const entry> shared_const_ptr;
		typedef std::weak_ptr<entry> weak_ptr;

		// constructor/destructor
		entry(std::vector<std::string> &&names, option_type type = option_type::STRING, const char *description = nullptr);
		entry(std::string &&name, option_type type = option_type::STRING, const char *description = nullptr);
		entry(const entry &) = delete;
		entry(entry &&) = delete;
		entry& operator=(const entry &) = delete;
		entry& operator=(entry &&) = delete;
		virtual ~entry();

		// accessors
		const std::vector<std::string> &names() const noexcept { return m_names; }
		const std::string &name() const noexcept { return m_names[0]; }
		virtual const char *value() const noexcept;
		int priority() const noexcept { return m_priority; }
		void set_priority(int priority) noexcept { m_priority = priority; }
		option_type type() const noexcept { return m_type; }
		const char *description() const noexcept { return m_description; }
		virtual const std::string &default_value() const noexcept;
		virtual const char *minimum() const noexcept;
		virtual const char *maximum() const noexcept;
		bool has_range() const noexcept;

		// mutators
		void set_value(std::string &&newvalue, int priority, bool always_override = false);
		virtual void set_default_value(std::string &&newvalue);
		void set_description(const char *description) { m_description = description; }
		void set_value_changed_handler(std::function<void(const char *)> &&handler) { m_value_changed_handler = std::move(handler); }
		virtual void revert(int priority_hi, int priority_lo) { }

	protected:
		virtual void internal_set_value(std::string &&newvalue) = 0;

	private:
		void validate(const std::string &value);

		std::vector<std::string>                    m_names;
		int                                         m_priority;
		core_options::option_type                   m_type;
		const char *                                m_description;
		std::function<void(const char *)>           m_value_changed_handler;
	};

	// construction/destruction
	core_options();
	core_options(const core_options &) = delete;
	core_options(core_options &&) = default;
	core_options& operator=(const core_options &) = delete;
	core_options& operator=(core_options &&) = default;
	virtual ~core_options();

	// getters
	const std::string &command() const noexcept { return m_command; }
	const std::vector<std::string> &command_arguments() const noexcept { assert(!m_command.empty()); return m_command_arguments; }
	entry::shared_const_ptr get_entry(const std::string &name) const noexcept;
	entry::shared_ptr get_entry(const std::string &name) noexcept;
	const std::vector<entry::shared_ptr> &entries() const noexcept { return m_entries; }
	bool exists(const std::string &name) const noexcept { return get_entry(name) != nullptr; }
	bool header_exists(const char *description) const noexcept;

	// configuration
	void add_entry(entry::shared_ptr &&entry, const char *after_header = nullptr);
	void add_entry(const options_entry &entry, bool override_existing = false);
	void add_entry(std::vector<std::string> &&names, const char *description, option_type type, std::string &&default_value = "", std::string &&minimum = "", std::string &&maximum = "");
	void add_header(const char *description);
	void add_entries(const options_entry *entrylist, bool override_existing = false);
	void set_default_value(const char *name, const char *defvalue);
	void set_description(const char *name, const char *description);
	void remove_entry(entry &delentry);
	void set_value_changed_handler(const std::string &name, std::function<void(const char *)> &&handler);
	void revert(int priority_hi = OPTION_PRIORITY_MAXIMUM, int priority_lo = OPTION_PRIORITY_DEFAULT);

	// parsing/input
	void parse_command_line(const std::vector<std::string> &args, int priority, bool ignore_unknown_options = false);
	void parse_ini_file(util::core_file &inifile, int priority, bool ignore_unknown_options, bool always_override);
	void copy_from(const core_options &that);

	// output
	std::string output_ini(const core_options *diff = nullptr) const;
	std::string output_help() const;

	// reading
	const char *value(const char *option) const noexcept;
	const char *description(const char *option) const noexcept;
	bool bool_value(const char *option) const { return int_value(option) != 0; }
	int int_value(const char *option) const;
	float float_value(const char *option) const;

	// setting
	void set_value(const std::string &name, const std::string &value, int priority);
	void set_value(const std::string &name, std::string &&value, int priority);
	void set_value(const std::string &name, int value, int priority);
	void set_value(const std::string &name, float value, int priority);

	// misc
	static const char *unadorned(int x = 0) noexcept { return s_option_unadorned[std::min(x, MAX_UNADORNED_OPTIONS - 1)]; }

protected:
	virtual void command_argument_processed() { }

private:
	class simple_entry : public entry
	{
	public:
		// construction/destruction
		simple_entry(std::vector<std::string> &&names, const char *description, core_options::option_type type, std::string &&defdata, std::string &&minimum, std::string &&maximum);
		simple_entry(const simple_entry &) = delete;
		simple_entry(simple_entry &&) = delete;
		simple_entry& operator=(const simple_entry &) = delete;
		simple_entry& operator=(simple_entry &&) = delete;
		~simple_entry();

		// getters
		virtual const char *value() const noexcept override;
		virtual const char *minimum() const noexcept override;
		virtual const char *maximum() const noexcept override;
		virtual const std::string &default_value() const noexcept override;
		virtual void revert(int priority_hi, int priority_lo) override;

		virtual void set_default_value(std::string &&newvalue) override;

	protected:
		virtual void internal_set_value(std::string &&newvalue) override;

	private:
		// internal state
		std::string             m_data;             // data for this item
		std::string             m_defdata;          // default data for this item
		std::string             m_minimum;          // minimum value
		std::string             m_maximum;          // maximum value
	};

	// used internally in core_options
	enum class condition_type
	{
		NONE,
		WARN,
		ERR
	};

	// internal helpers
	void add_to_entry_map(std::string &&name, entry::shared_ptr &entry);
	void do_set_value(entry &curentry, std::string &&data, int priority, std::ostream &error_stream, condition_type &condition);
	void throw_options_exception_if_appropriate(condition_type condition, std::ostringstream &error_stream);

	// internal state
	std::vector<entry::shared_ptr>                      m_entries;              // canonical list of entries
	std::unordered_map<std::string, entry::weak_ptr>    m_entrymap;             // map for fast lookup
	std::string                                         m_command;              // command found
	std::vector<std::string>                            m_command_arguments;    // command arguments
	static const char *const                            s_option_unadorned[];   // array of unadorned option "names"
};


// static structure describing a single option with its description and default value
struct options_entry
{
	const char *                name;               // name on the command line
	const char *                defvalue;           // default value of this argument
	core_options::option_type   type;               // type of option
	const char *                description;        // description for -showusage
};

// legacy option types
const core_options::option_type OPTION_INVALID = core_options::option_type::INVALID;
const core_options::option_type OPTION_HEADER = core_options::option_type::HEADER;
const core_options::option_type OPTION_COMMAND = core_options::option_type::COMMAND;
const core_options::option_type OPTION_BOOLEAN = core_options::option_type::BOOLEAN;
const core_options::option_type OPTION_INTEGER = core_options::option_type::INTEGER;
const core_options::option_type OPTION_FLOAT = core_options::option_type::FLOAT;
const core_options::option_type OPTION_STRING = core_options::option_type::STRING;


#endif // MAME_LIB_UTIL_OPTIONS_H
