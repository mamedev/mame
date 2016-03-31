// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    express.h

    Generic expressions engine.

***************************************************************************/

#pragma once

#ifndef __EXPRESS_H__
#define __EXPRESS_H__

#include "emu.h"


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
	EXPSPACE_SPACE3_LOGICAL,
	EXPSPACE_PROGRAM_PHYSICAL,
	EXPSPACE_DATA_PHYSICAL,
	EXPSPACE_IO_PHYSICAL,
	EXPSPACE_SPACE3_PHYSICAL,
	EXPSPACE_OPCODE,
	EXPSPACE_RAMWRITE,
	EXPSPACE_REGION
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class symbol_table;


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
		UNBALANCED_QUOTES,
		TOO_MANY_STRINGS,
		INVALID_MEMORY_SIZE,
		INVALID_MEMORY_SPACE,
		NO_SUCH_MEMORY_SPACE,
		INVALID_MEMORY_NAME,
		MISSING_MEMORY_NAME
	};

	// construction/destruction
	expression_error(error_code code, int offset = 0)
		: m_code(code),
			m_offset(offset) { }

	// operators
	operator error_code() const { return m_code; }

	// getters
	error_code code() const { return m_code; }
	int offset() const { return m_offset; }
	const char *code_string() const;

private:
	// internal state
	error_code          m_code;
	int                 m_offset;
};


// ======================> symbol_entry

// symbol_entry describes a symbol in a symbol table
class symbol_entry
{
	friend class simple_list<symbol_entry>;

protected:
	// symbol types
	enum symbol_type
	{
		SMT_INTEGER,
		SMT_FUNCTION
	};

	// construction/destruction
	symbol_entry(symbol_table &table, symbol_type type, const char *name, void *ref);
	virtual ~symbol_entry();

public:
	// getters
	symbol_entry *next() const { return m_next; }
	const char *name() const { return m_name.c_str(); }

	// type checking
	bool is_function() const { return (m_type == SMT_FUNCTION); }

	// symbol access
	virtual bool is_lval() const = 0;
	virtual UINT64 value() const = 0;
	virtual void set_value(UINT64 newvalue) = 0;

protected:
	// internal state
	symbol_entry *  m_next;                     // link to next entry
	symbol_table &  m_table;                    // pointer back to the owning table
	symbol_type     m_type;                     // type of symbol
	std::string     m_name;                     // name of the symbol
	void *          m_ref;                      // internal reference
};


// ======================> symbol_table

// a symbol_table holds symbols of various types which the expression engine
// queries to look up symbols
class symbol_table
{
public:
	// callback functions for getting/setting a symbol value
	typedef UINT64 (*getter_func)(symbol_table &table, void *symref);
	typedef void (*setter_func)(symbol_table &table, void *symref, UINT64 value);

	// callback functions for function execution
	typedef UINT64 (*execute_func)(symbol_table &table, void *symref, int numparams, const UINT64 *paramlist);

	// callback functions for memory reads/writes
	typedef expression_error::error_code (*valid_func)(void *cbparam, const char *name, expression_space space);
	typedef UINT64 (*read_func)(void *cbparam, const char *name, expression_space space, UINT32 offset, int size);
	typedef void (*write_func)(void *cbparam, const char *name, expression_space space, UINT32 offset, int size, UINT64 value);

	enum read_write
	{
		READ_ONLY = 0,
		READ_WRITE
	};

	// construction/destruction
	symbol_table(void *globalref, symbol_table *parent = nullptr);

	// getters
	const tagged_list<symbol_entry> &entries() const { return m_symlist; }
	symbol_table *parent() const { return m_parent; }
	void *globalref() const { return m_globalref; }

	// setters
	void configure_memory(void *param, valid_func valid, read_func read, write_func write);

	// symbol access
	void add(const char *name, read_write rw, UINT64 *ptr = nullptr);
	void add(const char *name, UINT64 constvalue);
	void add(const char *name, void *ref, getter_func getter, setter_func setter = nullptr);
	void add(const char *name, void *ref, int minparams, int maxparams, execute_func execute);
	symbol_entry *find(const char *name) { return m_symlist.find(name); }
	symbol_entry *find_deep(const char *name);

	// value getter/setter
	UINT64 value(const char *symbol);
	void set_value(const char *symbol, UINT64 value);

	// memory accessors
	expression_error::error_code memory_valid(const char *name, expression_space space);
	UINT64 memory_value(const char *name, expression_space space, UINT32 offset, int size);
	void set_memory_value(const char *name, expression_space space, UINT32 offset, int size, UINT64 value);

private:
	// internal state
	symbol_table *          m_parent;           // pointer to the parent symbol table
	void *                  m_globalref;        // global reference parameter
	tagged_list<symbol_entry> m_symlist;        // list of symbols
	void *                  m_memory_param;     // callback parameter for memory
	valid_func              m_memory_valid;     // validation callback
	read_func               m_memory_read;      // read callback
	write_func              m_memory_write;     // write callback
};


// ======================> parsed_expression

// a parsed_expression holds a pre-parsed expression that can be
// efficiently executed at a later time
class parsed_expression
{
public:
	// construction/destruction
	parsed_expression(const parsed_expression &src) { copy(src); }
	parsed_expression(symbol_table *symtable = nullptr, const char *expression = nullptr, UINT64 *result = nullptr);

	// operators
	parsed_expression &operator=(const parsed_expression &src) { copy(src); return *this; }

	// getters
	bool is_empty() const { return (m_tokenlist.count() == 0); }
	const char *original_string() const { return m_original_string.c_str(); }
	symbol_table *symbols() const { return m_symtable; }

	// setters
	void set_symbols(symbol_table *symtable) { m_symtable = symtable; }

	// execution
	void parse(const char *string);
	UINT64 execute() { return execute_tokens(); }

private:
	// a single token
	class parse_token
	{
		friend class simple_list<parse_token>;

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
			TIN_PRECEDENCE_SHIFT    = 24,       // 8 bits (24-31)
			TIN_PRECEDENCE_MASK     = 0xff << TIN_PRECEDENCE_SHIFT
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
		parse_token *next() const { return m_next; }
		int offset() const { return m_offset; }
		bool is_number() const { return (m_type == NUMBER); }
		bool is_string() const { return (m_type == STRING); }
		bool is_memory() const { return (m_type == MEMORY); }
		bool is_symbol() const { return (m_type == SYMBOL); }
		bool is_operator() const { return (m_type == OPERATOR); }
		bool is_operator(UINT8 type) const { return (m_type == OPERATOR && optype() == type); }
		bool is_lval() const { return ((m_type == SYMBOL && m_symbol->is_lval()) || m_type == MEMORY); }

		UINT64 value() const { assert(m_type == NUMBER); return m_value; }
		UINT32 address() const { assert(m_type == MEMORY); return m_value; }
		symbol_entry *symbol() const { assert(m_type == SYMBOL); return m_symbol; }

		UINT8 optype() const { assert(m_type == OPERATOR); return (m_flags & TIN_OPTYPE_MASK) >> TIN_OPTYPE_SHIFT; }
		UINT8 precedence() const { assert(m_type == OPERATOR); return (m_flags & TIN_PRECEDENCE_MASK) >> TIN_PRECEDENCE_SHIFT; }
		bool is_function_separator() const { assert(m_type == OPERATOR); return ((m_flags & TIN_FUNCTION_MASK) != 0); }
		bool right_to_left() const { assert(m_type == OPERATOR); return ((m_flags & TIN_RIGHT_TO_LEFT_MASK) != 0); }
		expression_space memory_space() const { assert(m_type == OPERATOR || m_type == MEMORY); return expression_space((m_flags & TIN_MEMORY_SPACE_MASK) >> TIN_MEMORY_SPACE_SHIFT); }
		int memory_size() const { assert(m_type == OPERATOR || m_type == MEMORY); return (m_flags & TIN_MEMORY_SIZE_MASK) >> TIN_MEMORY_SIZE_SHIFT; }

		// setters
		parse_token &set_offset(int offset) { m_offset = offset; return *this; }
		parse_token &set_offset(const parse_token &src) { m_offset = src.m_offset; return *this; }
		parse_token &set_offset(const parse_token &src1, const parse_token &src2) { m_offset = MIN(src1.m_offset, src2.m_offset); return *this; }
		parse_token &configure_number(UINT64 value) { m_type = NUMBER; m_value = value; return *this; }
		parse_token &configure_string(const char *string) { m_type = STRING; m_string = string; return *this; }
		parse_token &configure_memory(UINT32 address, parse_token &memoryat) { m_type = MEMORY; m_value = address; m_flags = memoryat.m_flags; m_string = memoryat.m_string; return *this; }
		parse_token &configure_symbol(symbol_entry &symbol) { m_type = SYMBOL; m_symbol = &symbol; return *this; }
		parse_token &configure_operator(UINT8 optype, UINT8 precedence)
			{ m_type = OPERATOR; m_flags = ((optype << TIN_OPTYPE_SHIFT) & TIN_OPTYPE_MASK) | ((precedence << TIN_PRECEDENCE_SHIFT) & TIN_PRECEDENCE_MASK); return *this; }

		parse_token &set_function_separator() { assert(m_type == OPERATOR); m_flags |= TIN_FUNCTION_MASK; return *this; }
		parse_token &set_right_to_left() { assert(m_type == OPERATOR); m_flags |= TIN_RIGHT_TO_LEFT_MASK; return *this; }
		parse_token &set_memory_space(expression_space space) { assert(m_type == OPERATOR || m_type == MEMORY); m_flags = (m_flags & ~TIN_MEMORY_SPACE_MASK) | ((space << TIN_MEMORY_SPACE_SHIFT) & TIN_MEMORY_SPACE_MASK); return *this; }
		parse_token &set_memory_size(int log2ofbits) { assert(m_type == OPERATOR || m_type == MEMORY); m_flags = (m_flags & ~TIN_MEMORY_SIZE_MASK) | ((log2ofbits << TIN_MEMORY_SIZE_SHIFT) & TIN_MEMORY_SIZE_MASK); return *this; }
		parse_token &set_memory_source(const char *string) { assert(m_type == OPERATOR || m_type == MEMORY); m_string = string; return *this; }

		// access
		UINT64 get_lval_value(symbol_table *symtable);
		void set_lval_value(symbol_table *symtable, UINT64 value);

	private:
		// internal state
		parse_token *           m_next;             // next token in list
		token_type              m_type;             // type of token
		int                     m_offset;           // offset within the string
		UINT64                  m_value;            // integral value
		UINT32                  m_flags;            // additional flags/info
		const char *            m_string;           // associated string
		symbol_entry *          m_symbol;           // symbol pointer
	};

	// an expression_string holds an indexed string parsed from the expression
	class expression_string
	{
		friend class simple_list<expression_string>;

	public:
		// construction/destruction
		expression_string(const char *string, int length = 0)
			: m_next(nullptr),
				m_string(string, (length == 0) ? strlen(string) : length) { }

		// operators
		operator const char *() { return m_string.c_str(); }
		operator const char *() const { return m_string.c_str(); }

	private:
		// internal state
		expression_string * m_next;                     // next string in list
		std::string         m_string;                   // copy of the string
	};

	// internal helpers
	void copy(const parsed_expression &src);
	void print_tokens(FILE *out);

	// parsing helpers
	void parse_string_into_tokens();
	void parse_symbol_or_number(parse_token &token, const char *&string);
	void parse_number(parse_token &token, const char *string, int base, expression_error::error_code errcode);
	void parse_quoted_char(parse_token &token, const char *&string);
	void parse_quoted_string(parse_token &token, const char *&string);
	void parse_memory_operator(parse_token &token, const char *string);
	void normalize_operator(parse_token *prevtoken, parse_token &thistoken);
	void infix_to_postfix();

	// execution helpers
	void push_token(parse_token &token);
	void pop_token(parse_token &token);
	parse_token *peek_token(int count);
	void pop_token_lval(parse_token &token);
	void pop_token_rval(parse_token &token);
	UINT64 execute_tokens();
	void execute_function(parse_token &token);

	// constants
	static const int MAX_FUNCTION_PARAMS = 16;
	static const int MAX_STACK_DEPTH = 16;

	// internal state
	symbol_table *      m_symtable;                     // symbol table
	std::string         m_original_string;              // original string (prior to parsing)
	simple_list<parse_token> m_tokenlist;               // token list
	simple_list<expression_string> m_stringlist;        // string list
	int                 m_token_stack_ptr;              // stack pointer (used during execution)
	parse_token         m_token_stack[MAX_STACK_DEPTH]; // token stack (used during execution)
};


#endif
