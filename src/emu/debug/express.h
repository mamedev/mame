// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    express.h

    Generic expressions engine.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_EXPRESS_H
#define MAME_EMU_DEBUG_EXPRESS_H

#pragma once

#include "emucore.h"

#include <deque>
#include <functional>
#include <list>
#include <string_view>
#include <unordered_map>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// values for the address space passed to external_read/write_memory
enum expression_space
{
	EXPSPACE_INVALID,
	EXPSPACE_PROGRAM_LOGICAL,
	EXPSPACE_DATA_LOGICAL,
	EXPSPACE_IO_LOGICAL,
	EXPSPACE_OPCODE_LOGICAL,
	EXPSPACE_PROGRAM_PHYSICAL,
	EXPSPACE_DATA_PHYSICAL,
	EXPSPACE_IO_PHYSICAL,
	EXPSPACE_OPCODE_PHYSICAL,
	EXPSPACE_PRGDIRECT,
	EXPSPACE_OPDIRECT,
	EXPSPACE_REGION
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

using offs_t = u32;

// ======================> expression_error

// an expression_error holds an error code and a string offset
class expression_error
{
public:
	// codes
	enum error_code
	{
		NONE,
		NOT_LVAL,
		NOT_RVAL,
		SYNTAX,
		UNKNOWN_SYMBOL,
		INVALID_NUMBER,
		INVALID_TOKEN,
		STACK_OVERFLOW,
		STACK_UNDERFLOW,
		UNBALANCED_PARENS,
		DIVIDE_BY_ZERO,
		OUT_OF_MEMORY,
		INVALID_PARAM_COUNT,
		TOO_FEW_PARAMS,
		TOO_MANY_PARAMS,
		UNBALANCED_QUOTES,
		TOO_MANY_STRINGS,
		INVALID_MEMORY_SIZE,
		INVALID_MEMORY_SPACE,
		NO_SUCH_MEMORY_SPACE,
		INVALID_MEMORY_NAME,
		MISSING_MEMORY_NAME
	};

	// construction/destruction
	expression_error(error_code code, int offset = 0, int num = 0)
		: m_code(code),
			m_offset(offset),
			m_num(num) { }

	// operators
	operator error_code() const { return m_code; }

	// getters
	error_code code() const { return m_code; }
	int offset() const { return m_offset; }
	std::string code_string() const;

private:
	// internal state
	error_code          m_code;
	int                 m_offset;
	int                 m_num;
};


// ======================> symbol_entry

// symbol_entry describes a symbol in a symbol table
class symbol_entry
{
protected:
	// symbol types
	enum symbol_type
	{
		SMT_INTEGER,
		SMT_FUNCTION
	};

	// construction/destruction
	symbol_entry(symbol_table &table, symbol_type type, const char *name, const std::string &format);
public:
	virtual ~symbol_entry();

	// getters
	const char *name() const { return m_name.c_str(); }
	const std::string &format() const { return m_format; }

	// type checking
	bool is_function() const { return (m_type == SMT_FUNCTION); }

	// symbol access
	virtual bool is_lval() const = 0;
	virtual u64 value() const = 0;
	virtual void set_value(u64 newvalue) = 0;

protected:
	// internal state
	symbol_table &  m_table;                    // pointer back to the owning table
	symbol_type     m_type;                     // type of symbol
	std::string     m_name;                     // name of the symbol
	std::string     m_format;                   // format of symbol (or empty if unspecified)
};


// ======================> symbol_table

// a symbol_table holds symbols of various types which the expression engine
// queries to look up symbols
class symbol_table
{
public:
	// callback functions for getting/setting a symbol value
	typedef std::function<u64()> getter_func;
	typedef std::function<void(u64 value)> setter_func;

	// callback functions for function execution
	typedef std::function<u64(int numparams, const u64 *paramlist)> execute_func;

	// callback functions for memory reads/writes
	typedef std::function<void()> memory_modified_func;

	enum read_write
	{
		READ_ONLY = 0,
		READ_WRITE
	};

	// construction/destruction
	symbol_table(running_machine &machine, symbol_table *parent = nullptr, device_t *device = nullptr);

	// getters
	const std::unordered_map<std::string, std::unique_ptr<symbol_entry>> &entries() const { return m_symlist; }
	symbol_table *parent() const { return m_parent; }
	running_machine &machine() { return m_machine; }

	// setters
	void set_memory_modified_func(memory_modified_func modified);

	// symbol access
	symbol_entry &add(const char *name, read_write rw, u64 *ptr = nullptr);
	symbol_entry &add(const char *name, u64 constvalue);
	symbol_entry &add(const char *name, getter_func getter, setter_func setter = nullptr, const std::string &format_string = "");
	symbol_entry &add(const char *name, int minparams, int maxparams, execute_func execute);
	symbol_entry *find(const char *name) const { if (name) { auto search = m_symlist.find(name); if (search != m_symlist.end()) return search->second.get(); else return nullptr; } else return nullptr; }
	symbol_entry *find_deep(const char *name);

	// value getter/setter
	u64 value(const char *symbol);
	void set_value(const char *symbol, u64 value);

	// memory accessors
	expression_error::error_code memory_valid(const char *name, expression_space space);
	u64 memory_value(const char *name, expression_space space, u32 offset, int size, bool disable_se);
	void set_memory_value(const char *name, expression_space space, u32 offset, int size, u64 value, bool disable_se);
	u64 read_memory(address_space &space, offs_t address, int size, bool apply_translation);
	void write_memory(address_space &space, offs_t address, u64 data, int size, bool apply_translation);

private:
	// memory helpers
	u64 read_program_direct(address_space &space, int opcode, offs_t address, int size);
	u64 read_memory_region(const char *rgntag, offs_t address, int size);
	void write_program_direct(address_space &space, int opcode, offs_t address, int size, u64 data);
	void write_memory_region(const char *rgntag, offs_t address, int size, u64 data);
	expression_error expression_get_space(const char *tag, int &spacenum, device_memory_interface *&memory);
	void notify_memory_modified();

	// internal state
	running_machine &       m_machine;          // reference to the machine
	symbol_table *          m_parent;           // pointer to the parent symbol table
	std::unordered_map<std::string,std::unique_ptr<symbol_entry>> m_symlist;        // list of symbols
	device_memory_interface *const m_memintf;   // pointer to the local memory interface (if any)
	memory_modified_func    m_memory_modified;  // memory modified callback
};


// ======================> parsed_expression

// a parsed_expression holds a pre-parsed expression that can be
// efficiently executed at a later time
class parsed_expression
{
public:
	// construction/destruction
	parsed_expression(symbol_table &symtable);
	parsed_expression(symbol_table &symtable, std::string_view expression, int default_base = 16);
	parsed_expression(const parsed_expression &src);
	parsed_expression(parsed_expression &&src) = default;

	// operators
	parsed_expression &operator=(const parsed_expression &src) { copy(src); return *this; }
	parsed_expression &operator=(parsed_expression &&src) = default;

	// getters
	bool is_empty() const { return m_tokenlist.empty(); }
	const char *original_string() const { return m_original_string.c_str(); }
	symbol_table &symbols() const { return m_symtable.get(); }

	// setters
	void set_symbols(symbol_table &symtable) { m_symtable = std::reference_wrapper<symbol_table>(symtable); }
	void set_default_base(int base) { assert(base == 8 || base == 10 || base == 16); m_default_base = base; }

	// execution
	void parse(std::string_view string);
	u64 execute() { return execute_tokens(); }

private:
	// a single token
	class parse_token
	{
		// operator flags
		enum
		{
			TIN_OPTYPE_SHIFT        = 0,        // 8 bits (0-7)
			TIN_OPTYPE_MASK         = 0xff << TIN_OPTYPE_SHIFT,
			TIN_RIGHT_TO_LEFT_SHIFT = 16,       // 1 bit  (16)
			TIN_RIGHT_TO_LEFT_MASK  = 1 << TIN_RIGHT_TO_LEFT_SHIFT,
			TIN_FUNCTION_SHIFT      = 17,       // 1 bit  (17)
			TIN_FUNCTION_MASK       = 1 << TIN_FUNCTION_SHIFT,
			TIN_MEMORY_SIZE_SHIFT   = 18,       // 2 bits (18-19)
			TIN_MEMORY_SIZE_MASK    = 3 << TIN_MEMORY_SIZE_SHIFT,
			TIN_MEMORY_SPACE_SHIFT  = 20,       // 4 bits (20-23)
			TIN_MEMORY_SPACE_MASK   = 0xf << TIN_MEMORY_SPACE_SHIFT,
			TIN_PRECEDENCE_SHIFT    = 24,       // 5 bits (24-28)
			TIN_PRECEDENCE_MASK     = 0x1f << TIN_PRECEDENCE_SHIFT,
			TIN_SIDE_EFFECT_SHIFT   = 29,       // 1 bit  (29)
			TIN_SIDE_EFFECT_MASK    = 1 << TIN_SIDE_EFFECT_SHIFT
		};

		// types of tokens
		enum token_type
		{
			INVALID = 0,
			NUMBER,
			STRING,
			MEMORY,
			SYMBOL,
			OPERATOR
		};

	public:
		// construction/destruction
		parse_token(int offset = 0);

		// getters
		int offset() const { return m_offset; }
		bool is_number() const { return (m_type == NUMBER); }
		bool is_string() const { return (m_type == STRING); }
		bool is_memory() const { return (m_type == MEMORY); }
		bool is_symbol() const { return (m_type == SYMBOL); }
		bool is_operator() const { return (m_type == OPERATOR); }
		bool is_operator(u8 type) const { return (m_type == OPERATOR && optype() == type); }
		bool is_lval() const { return ((m_type == SYMBOL && m_symbol->is_lval()) || m_type == MEMORY); }

		u64 value() const { assert(m_type == NUMBER); return m_value; }
		const char *string() const { assert(m_type == STRING); return m_string; }
		u32 address() const { assert(m_type == MEMORY); return m_value; }
		symbol_entry &symbol() const { assert(m_type == SYMBOL); return *m_symbol; }

		u8 optype() const { assert(m_type == OPERATOR); return (m_flags & TIN_OPTYPE_MASK) >> TIN_OPTYPE_SHIFT; }
		u8 precedence() const { assert(m_type == OPERATOR); return (m_flags & TIN_PRECEDENCE_MASK) >> TIN_PRECEDENCE_SHIFT; }
		bool is_function_separator() const { assert(m_type == OPERATOR); return ((m_flags & TIN_FUNCTION_MASK) != 0); }
		bool right_to_left() const { assert(m_type == OPERATOR); return ((m_flags & TIN_RIGHT_TO_LEFT_MASK) != 0); }
		expression_space memory_space() const { assert(m_type == OPERATOR || m_type == MEMORY); return expression_space((m_flags & TIN_MEMORY_SPACE_MASK) >> TIN_MEMORY_SPACE_SHIFT); }
		int memory_size() const { assert(m_type == OPERATOR || m_type == MEMORY); return (m_flags & TIN_MEMORY_SIZE_MASK) >> TIN_MEMORY_SIZE_SHIFT; }
		bool memory_side_effects() const { assert(m_type == OPERATOR || m_type == MEMORY); return (m_flags & TIN_SIDE_EFFECT_MASK) >> TIN_SIDE_EFFECT_SHIFT; }

		// setters
		parse_token &set_offset(int offset) { m_offset = offset; return *this; }
		parse_token &set_offset(const parse_token &src) { m_offset = src.m_offset; return *this; }
		parse_token &set_offset(const parse_token &src1, const parse_token &src2) { m_offset = std::min(src1.m_offset, src2.m_offset); return *this; }
		parse_token &configure_number(u64 value) { m_type = NUMBER; m_value = value; return *this; }
		parse_token &configure_string(const char *string) { m_type = STRING; m_string = string; return *this; }
		parse_token &configure_memory(u32 address, parse_token &memoryat) { m_type = MEMORY; m_value = address; m_flags = memoryat.m_flags; m_string = memoryat.m_string; return *this; }
		parse_token &configure_symbol(symbol_entry &symbol) { m_type = SYMBOL; m_symbol = &symbol; return *this; }
		parse_token &configure_operator(u8 optype, u8 precedence)
			{ m_type = OPERATOR; m_flags = ((optype << TIN_OPTYPE_SHIFT) & TIN_OPTYPE_MASK) | ((precedence << TIN_PRECEDENCE_SHIFT) & TIN_PRECEDENCE_MASK); return *this; }

		parse_token &set_function_separator() { assert(m_type == OPERATOR); m_flags |= TIN_FUNCTION_MASK; return *this; }
		parse_token &set_right_to_left() { assert(m_type == OPERATOR); m_flags |= TIN_RIGHT_TO_LEFT_MASK; return *this; }
		parse_token &set_memory_space(expression_space space) { assert(m_type == OPERATOR || m_type == MEMORY); m_flags = (m_flags & ~TIN_MEMORY_SPACE_MASK) | ((space << TIN_MEMORY_SPACE_SHIFT) & TIN_MEMORY_SPACE_MASK); return *this; }
		parse_token &set_memory_size(int log2ofbits) { assert(m_type == OPERATOR || m_type == MEMORY); m_flags = (m_flags & ~TIN_MEMORY_SIZE_MASK) | ((log2ofbits << TIN_MEMORY_SIZE_SHIFT) & TIN_MEMORY_SIZE_MASK); return *this; }
		parse_token &set_memory_side_effects(bool disable_se) { assert(m_type == OPERATOR || m_type == MEMORY); m_flags = disable_se ? m_flags | TIN_SIDE_EFFECT_MASK : m_flags & ~TIN_SIDE_EFFECT_MASK; return *this; }
		parse_token &set_memory_source(const char *string) { assert(m_type == OPERATOR || m_type == MEMORY); m_string = string; return *this; }

		// access
		u64 get_lval_value(symbol_table &symtable);
		void set_lval_value(symbol_table &symtable, u64 value);

	private:
		// internal state
		token_type              m_type;             // type of token
		int                     m_offset;           // offset within the string
		u64                     m_value;            // integral value
		u32                     m_flags;            // additional flags/info
		const char *            m_string;           // associated string
		symbol_entry *          m_symbol;           // symbol pointer
	};

	// internal helpers
	void copy(const parsed_expression &src);
	void print_tokens();

	// parsing helpers
	void parse_string_into_tokens();
	void parse_symbol_or_number(parse_token &token, const char *&string);
	void parse_number(parse_token &token, const char *string, int base, expression_error::error_code errcode);
	void parse_quoted_char(parse_token &token, const char *&string);
	void parse_quoted_string(parse_token &token, const char *&string);
	void parse_memory_operator(parse_token &token, const char *string, bool disable_se);
	void normalize_operator(parse_token &thistoken, parse_token *prevtoken, parse_token *nexttoken, const std::list<parse_token> &stack, bool was_rparen);
	void infix_to_postfix();

	// execution helpers
	void push_token(parse_token &token);
	void pop_token(parse_token &token);
	void pop_token_lval(parse_token &token);
	void pop_token_rval(parse_token &token);
	u64 execute_tokens();
	void execute_function(parse_token &token);

	// constants
	static const int MAX_FUNCTION_PARAMS = 16;

	// internal state
	std::reference_wrapper<symbol_table> m_symtable;    // symbol table
	int                 m_default_base;                 // default base
	std::string         m_original_string;              // original string (prior to parsing)
	std::list<parse_token> m_tokenlist;                 // token list
	std::list<std::string> m_stringlist;                // string list
	std::deque<parse_token> m_token_stack;              // token stack (used during execution)
};

#endif // MAME_EMU_DEBUG_EXPRESS_H
