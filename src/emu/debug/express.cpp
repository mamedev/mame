// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    express.c

    Generic expressions engine.

****************************************************************************

    Operator precedence
    ===================
    0x0000       ( )
    0x0001       ++ (postfix), -- (postfix)
    0x0002       ++ (prefix), -- (prefix), ~, !, - (unary), + (unary), b@, w@, d@, q@
    0x0003       *, /, %
    0x0004       + -
    0x0005       << >>
    0x0006       < <= > >=
    0x0007       == !=
    0x0008       &
    0x0009       ^
    0x000a       |
    0x000b       &&
    0x000c       ||
    0x000d       = *= /= %= += -= <<= >>= &= |= ^=
    0x000e       ,
    0x000f       func()

***************************************************************************/

#include "emu.h"
#include "express.h"
#include <ctype.h>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define DEBUG_TOKENS            0



/***************************************************************************
    CONSTANTS
***************************************************************************/

#ifndef DEFAULT_BASE
#define DEFAULT_BASE            16          // hex unless otherwise specified
#endif


// token.value values if token.is_operator()
enum
{
	TVL_LPAREN,
	TVL_RPAREN,
	TVL_PLUSPLUS,
	TVL_MINUSMINUS,
	TVL_PREINCREMENT,
	TVL_PREDECREMENT,
	TVL_POSTINCREMENT,
	TVL_POSTDECREMENT,
	TVL_COMPLEMENT,
	TVL_NOT,
	TVL_UPLUS,
	TVL_UMINUS,
	TVL_MULTIPLY,
	TVL_DIVIDE,
	TVL_MODULO,
	TVL_ADD,
	TVL_SUBTRACT,
	TVL_LSHIFT,
	TVL_RSHIFT,
	TVL_LESS,
	TVL_LESSOREQUAL,
	TVL_GREATER,
	TVL_GREATEROREQUAL,
	TVL_EQUAL,
	TVL_NOTEQUAL,
	TVL_BAND,
	TVL_BXOR,
	TVL_BOR,
	TVL_LAND,
	TVL_LOR,
	TVL_ASSIGN,
	TVL_ASSIGNMULTIPLY,
	TVL_ASSIGNDIVIDE,
	TVL_ASSIGNMODULO,
	TVL_ASSIGNADD,
	TVL_ASSIGNSUBTRACT,
	TVL_ASSIGNLSHIFT,
	TVL_ASSIGNRSHIFT,
	TVL_ASSIGNBAND,
	TVL_ASSIGNBXOR,
	TVL_ASSIGNBOR,
	TVL_COMMA,
	TVL_MEMORYAT,
	TVL_EXECUTEFUNC
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// a symbol entry representing a register, with read/write callbacks
class integer_symbol_entry : public symbol_entry
{
public:
	// construction/destruction
	integer_symbol_entry(symbol_table &table, const char *name, symbol_table::read_write rw, UINT64 *ptr = nullptr);
	integer_symbol_entry(symbol_table &table, const char *name, UINT64 constval);
	integer_symbol_entry(symbol_table &table, const char *name, void *ref, symbol_table::getter_func getter, symbol_table::setter_func setter);

	// symbol access
	virtual bool is_lval() const override;
	virtual UINT64 value() const override;
	virtual void set_value(UINT64 newvalue) override;

private:
	// internal helpers
	static UINT64 internal_getter(symbol_table &table, void *symref);
	static void internal_setter(symbol_table &table, void *symref, UINT64 value);

	// internal state
	symbol_table::getter_func   m_getter;
	symbol_table::setter_func   m_setter;
	UINT64                      m_value;
};


// a symbol entry representing a function
class function_symbol_entry : public symbol_entry
{
public:
	// construction/destruction
	function_symbol_entry(symbol_table &table, const char *name, void *ref, int minparams, int maxparams, symbol_table::execute_func execute);

	// symbol access
	virtual bool is_lval() const override;
	virtual UINT64 value() const override;
	virtual void set_value(UINT64 newvalue) override;

	// execution helper
	virtual UINT64 execute(int numparams, const UINT64 *paramlist);

private:
	// internal state
	UINT16                      m_minparams;
	UINT16                      m_maxparams;
	symbol_table::execute_func  m_execute;
};



//**************************************************************************
//  EXPRESSION ERROR
//**************************************************************************

//-------------------------------------------------
//  code_string - return a friendly string for a
//  given expression error
//-------------------------------------------------

const char *expression_error::code_string() const
{
	switch (m_code)
	{
		case NOT_LVAL:              return "not an lvalue";
		case NOT_RVAL:              return "not an rvalue";
		case SYNTAX:                return "syntax error";
		case UNKNOWN_SYMBOL:        return "unknown symbol";
		case INVALID_NUMBER:        return "invalid number";
		case INVALID_TOKEN:         return "invalid token";
		case STACK_OVERFLOW:        return "stack overflow";
		case STACK_UNDERFLOW:       return "stack underflow";
		case UNBALANCED_PARENS:     return "unbalanced parentheses";
		case DIVIDE_BY_ZERO:        return "divide by zero";
		case OUT_OF_MEMORY:         return "out of memory";
		case INVALID_PARAM_COUNT:   return "invalid number of parameters";
		case UNBALANCED_QUOTES:     return "unbalanced quotes";
		case TOO_MANY_STRINGS:      return "too many strings";
		case INVALID_MEMORY_SIZE:   return "invalid memory size (b/w/d/q expected)";
		case NO_SUCH_MEMORY_SPACE:  return "non-existent memory space";
		case INVALID_MEMORY_SPACE:  return "invalid memory space (p/d/i/o/r/m expected)";
		case INVALID_MEMORY_NAME:   return "invalid memory name";
		case MISSING_MEMORY_NAME:   return "missing memory name";
		default:                    return "unknown error";
	}
}



//**************************************************************************
//  SYMBOL ENTRY
//**************************************************************************

//-------------------------------------------------
//  symbol_entry - constructor
//-------------------------------------------------

symbol_entry::symbol_entry(symbol_table &table, symbol_type type, const char *name, void *ref)
	: m_next(nullptr),
		m_table(table),
		m_type(type),
		m_name(name),
		m_ref(ref)
{
}


//-------------------------------------------------
//  ~symbol_entry - destructor
//-------------------------------------------------

symbol_entry::~symbol_entry()
{
}



//**************************************************************************
//  REGISTER SYMBOL ENTRY
//**************************************************************************

//-------------------------------------------------
//  integer_symbol_entry - constructor
//-------------------------------------------------

integer_symbol_entry::integer_symbol_entry(symbol_table &table, const char *name, symbol_table::read_write rw, UINT64 *ptr)
	: symbol_entry(table, SMT_INTEGER, name, (ptr == nullptr) ? &m_value : ptr),
		m_getter(internal_getter),
		m_setter((rw == symbol_table::READ_ONLY) ? nullptr : internal_setter),
		m_value(0)
{
}


integer_symbol_entry::integer_symbol_entry(symbol_table &table, const char *name, UINT64 constval)
	: symbol_entry(table, SMT_INTEGER, name, &m_value),
		m_getter(internal_getter),
		m_setter(nullptr),
		m_value(constval)
{
}


integer_symbol_entry::integer_symbol_entry(symbol_table &table, const char *name, void *ref, symbol_table::getter_func getter, symbol_table::setter_func setter)
	: symbol_entry(table, SMT_INTEGER, name, ref),
		m_getter(getter),
		m_setter(setter),
		m_value(0)
{
}


//-------------------------------------------------
//  is_lval - is this symbol allowable as an lval?
//-------------------------------------------------

bool integer_symbol_entry::is_lval() const
{
	return (m_setter != nullptr);
}


//-------------------------------------------------
//  value - return the value of this symbol
//-------------------------------------------------

UINT64 integer_symbol_entry::value() const
{
	return (*m_getter)(m_table, m_ref);
}


//-------------------------------------------------
//  set_value - set the value of this symbol
//-------------------------------------------------

void integer_symbol_entry::set_value(UINT64 newvalue)
{
	if (m_setter != nullptr)
		(*m_setter)(m_table, m_ref, newvalue);
	else
		throw emu_fatalerror("Symbol '%s' is read-only", m_name.c_str());
}


//-------------------------------------------------
//  internal_getter - internal helper for
//  returning the value of a variable
//-------------------------------------------------

UINT64 integer_symbol_entry::internal_getter(symbol_table &table, void *symref)
{
	return *(UINT64 *)symref;
}


//-------------------------------------------------
//  internal_setter - internal helper for setting
//  the value of a variable
//-------------------------------------------------

void integer_symbol_entry::internal_setter(symbol_table &table, void *symref, UINT64 value)
{
	*(UINT64 *)symref = value;
}



//**************************************************************************
//  FUNCTION SYMBOL ENTRY
//**************************************************************************

//-------------------------------------------------
//  function_symbol_entry - constructor
//-------------------------------------------------

function_symbol_entry::function_symbol_entry(symbol_table &table, const char *name, void *ref, int minparams, int maxparams, symbol_table::execute_func execute)
	: symbol_entry(table, SMT_FUNCTION, name, ref),
		m_minparams(minparams),
		m_maxparams(maxparams),
		m_execute(execute)
{
}


//-------------------------------------------------
//  is_lval - is this symbol allowable as an lval?
//-------------------------------------------------

bool function_symbol_entry::is_lval() const
{
	return false;
}


//-------------------------------------------------
//  value - return the value of this symbol
//-------------------------------------------------

UINT64 function_symbol_entry::value() const
{
	throw emu_fatalerror("Symbol '%s' is a function and cannot be used in this context", m_name.c_str());
}


//-------------------------------------------------
//  set_value - set the value of this symbol
//-------------------------------------------------

void function_symbol_entry::set_value(UINT64 newvalue)
{
	throw emu_fatalerror("Symbol '%s' is a function and cannot be written", m_name.c_str());
}


//-------------------------------------------------
//  execute - execute the function
//-------------------------------------------------

UINT64 function_symbol_entry::execute(int numparams, const UINT64 *paramlist)
{
	if (numparams < m_minparams)
		throw emu_fatalerror("Function '%s' requires at least %d parameters", m_name.c_str(), m_minparams);
	if (numparams > m_maxparams)
		throw emu_fatalerror("Function '%s' accepts no more than %d parameters", m_name.c_str(), m_maxparams);
	return (*m_execute)(m_table, m_ref, numparams, paramlist);
}



//**************************************************************************
//  SYMBOL TABLE
//**************************************************************************

//-------------------------------------------------
//  symbol_table - constructor
//-------------------------------------------------

symbol_table::symbol_table(void *globalref, symbol_table *parent)
	: m_parent(parent),
		m_globalref(globalref),
		m_memory_param(nullptr),
		m_memory_valid(nullptr),
		m_memory_read(nullptr),
		m_memory_write(nullptr)
{
}


//-------------------------------------------------
//  add - add a new UINT64 pointer symbol
//-------------------------------------------------

void symbol_table::configure_memory(void *param, valid_func valid, read_func read, write_func write)
{
	m_memory_param = param;
	m_memory_valid = valid;
	m_memory_read = read;
	m_memory_write = write;
}


//-------------------------------------------------
//  add - add a new UINT64 pointer symbol
//-------------------------------------------------

void symbol_table::add(const char *name, read_write rw, UINT64 *ptr)
{
	m_symlist.remove(name);
	m_symlist.append(name, *global_alloc(integer_symbol_entry(*this, name, rw, ptr)));
}


//-------------------------------------------------
//  add - add a new value symbol
//-------------------------------------------------

void symbol_table::add(const char *name, UINT64 value)
{
	m_symlist.remove(name);
	m_symlist.append(name, *global_alloc(integer_symbol_entry(*this, name, value)));
}


//-------------------------------------------------
//  add - add a new register symbol
//-------------------------------------------------

void symbol_table::add(const char *name, void *ref, getter_func getter, setter_func setter)
{
	m_symlist.remove(name);
	m_symlist.append(name, *global_alloc(integer_symbol_entry(*this, name, ref, getter, setter)));
}


//-------------------------------------------------
//  add - add a new function symbol
//-------------------------------------------------

void symbol_table::add(const char *name, void *ref, int minparams, int maxparams, execute_func execute)
{
	m_symlist.remove(name);
	m_symlist.append(name, *global_alloc(function_symbol_entry(*this, name, ref, minparams, maxparams, execute)));
}


//-------------------------------------------------
//  find_deep - do a deep search for a symbol,
//  looking in the parent if needed
//-------------------------------------------------

symbol_entry *symbol_table::find_deep(const char *symbol)
{
	// walk up the table hierarchy to find the owner
	for (symbol_table *symtable = this; symtable != nullptr; symtable = symtable->m_parent)
	{
		symbol_entry *entry = symtable->find(symbol);
		if (entry != nullptr)
			return entry;
	}
	return nullptr;
}


//-------------------------------------------------
//  value - return the value of a symbol
//-------------------------------------------------

UINT64 symbol_table::value(const char *symbol)
{
	symbol_entry *entry = find_deep(symbol);
	return (entry != nullptr) ? entry->value() : 0;
}


//-------------------------------------------------
//  set_value - set the value of a symbol
//-------------------------------------------------

void symbol_table::set_value(const char *symbol, UINT64 value)
{
	symbol_entry *entry = find_deep(symbol);
	if (entry != nullptr)
		entry->set_value(value);
}


//-------------------------------------------------
//  memory_valid - return true if the given
//  memory name/space/offset combination is valid
//-------------------------------------------------

expression_error::error_code symbol_table::memory_valid(const char *name, expression_space space)
{
	// walk up the table hierarchy to find the owner
	for (symbol_table *symtable = this; symtable != nullptr; symtable = symtable->m_parent)
		if (symtable->m_memory_valid != nullptr)
		{
			expression_error::error_code err = (*symtable->m_memory_valid)(symtable->m_memory_param, name, space);
			if (err != expression_error::NO_SUCH_MEMORY_SPACE)
				return err;
		}
	return expression_error::NO_SUCH_MEMORY_SPACE;
}


//-------------------------------------------------
//  memory_value - return a value read from memory
//-------------------------------------------------

UINT64 symbol_table::memory_value(const char *name, expression_space space, UINT32 offset, int size)
{
	// walk up the table hierarchy to find the owner
	for (symbol_table *symtable = this; symtable != nullptr; symtable = symtable->m_parent)
		if (symtable->m_memory_valid != nullptr)
		{
			expression_error::error_code err = (*symtable->m_memory_valid)(symtable->m_memory_param, name, space);
			if (err != expression_error::NO_SUCH_MEMORY_SPACE && symtable->m_memory_read != nullptr)
				return (*symtable->m_memory_read)(symtable->m_memory_param, name, space, offset, size);
			return 0;
		}
	return 0;
}


//-------------------------------------------------
//  set_memory_value - write a value to memory
//-------------------------------------------------

void symbol_table::set_memory_value(const char *name, expression_space space, UINT32 offset, int size, UINT64 value)
{
	// walk up the table hierarchy to find the owner
	for (symbol_table *symtable = this; symtable != nullptr; symtable = symtable->m_parent)
		if (symtable->m_memory_valid != nullptr)
		{
			expression_error::error_code err = (*symtable->m_memory_valid)(symtable->m_memory_param, name, space);
			if (err != expression_error::NO_SUCH_MEMORY_SPACE && symtable->m_memory_write != nullptr)
				(*symtable->m_memory_write)(symtable->m_memory_param, name, space, offset, size, value);
			return;
		}
}



//**************************************************************************
//  PARSED EXPRESSION
//**************************************************************************

//-------------------------------------------------
//  parsed_expression - constructor
//-------------------------------------------------

parsed_expression::parsed_expression(symbol_table *symtable, const char *expression, UINT64 *result)
	: m_symtable(symtable),
	m_token_stack_ptr(0)
{
	// if we got an expression parse it
	if (expression != nullptr)
		parse(expression);

	// if we get a result pointer, execute it
	if (result != nullptr)
		*result = execute();
}


//-------------------------------------------------
//  parse - parse an expression into tokens
//-------------------------------------------------

void parsed_expression::parse(const char *expression)
{
	// copy the string and reset our parsing state
	m_original_string.assign(expression);
	m_tokenlist.reset();
	m_stringlist.reset();

	// first parse the tokens into the token array in order
	parse_string_into_tokens();

	// convert the infix order to postfix order
	infix_to_postfix();
}


//-------------------------------------------------
//  copy - copy an expression from another source
//-------------------------------------------------

void parsed_expression::copy(const parsed_expression &src)
{
	m_symtable = src.m_symtable;
	m_original_string.assign(src.m_original_string);
	if (!m_original_string.empty())
		parse_string_into_tokens();
}


//-------------------------------------------------
//  print_tokens - debugging took to print a
//  human readable token representation
//-------------------------------------------------

void parsed_expression::print_tokens(FILE *out)
{
#if DEBUG_TOKENS
	osd_printf_debug("----\n");
	for (parse_token *token = m_tokens.first(); token != NULL; token = token->next())
	{
		switch (token->type)
		{
			default:
			case parse_token::INVALID:
				fprintf(out, "INVALID\n");
				break;

			case parse_token::END:
				fprintf(out, "END\n");
				break;

			case parse_token::NUMBER:
				fprintf(out, "NUMBER: %08X%08X\n", (UINT32)(token->value.i >> 32), (UINT32)token->value.i);
				break;

			case parse_token::STRING:
				fprintf(out, "STRING: ""%s""\n", token->string);
				break;

			case parse_token::SYMBOL:
				fprintf(out, "SYMBOL: %08X%08X\n", (UINT32)(token->value.i >> 32), (UINT32)token->value.i);
				break;

			case parse_token::OPERATOR:
				switch (token->value.i)
				{
					case TVL_LPAREN:        fprintf(out, "(\n");                    break;
					case TVL_RPAREN:        fprintf(out, ")\n");                    break;
					case TVL_PLUSPLUS:      fprintf(out, "++ (unspecified)\n");     break;
					case TVL_MINUSMINUS:    fprintf(out, "-- (unspecified)\n");     break;
					case TVL_PREINCREMENT:  fprintf(out, "++ (prefix)\n");          break;
					case TVL_PREDECREMENT:  fprintf(out, "-- (prefix)\n");          break;
					case TVL_POSTINCREMENT: fprintf(out, "++ (postfix)\n");         break;
					case TVL_POSTDECREMENT: fprintf(out, "-- (postfix)\n");         break;
					case TVL_COMPLEMENT:    fprintf(out, "!\n");                    break;
					case TVL_NOT:           fprintf(out, "~\n");                    break;
					case TVL_UPLUS:         fprintf(out, "+ (unary)\n");            break;
					case TVL_UMINUS:        fprintf(out, "- (unary)\n");            break;
					case TVL_MULTIPLY:      fprintf(out, "*\n");                    break;
					case TVL_DIVIDE:        fprintf(out, "/\n");                    break;
					case TVL_MODULO:        fprintf(out, "%%\n");                   break;
					case TVL_ADD:           fprintf(out, "+\n");                    break;
					case TVL_SUBTRACT:      fprintf(out, "-\n");                    break;
					case TVL_LSHIFT:        fprintf(out, "<<\n");                   break;
					case TVL_RSHIFT:        fprintf(out, ">>\n");                   break;
					case TVL_LESS:          fprintf(out, "<\n");                    break;
					case TVL_LESSOREQUAL:   fprintf(out, "<=\n");                   break;
					case TVL_GREATER:       fprintf(out, ">\n");                    break;
					case TVL_GREATEROREQUAL:fprintf(out, ">=\n");                   break;
					case TVL_EQUAL:         fprintf(out, "==\n");                   break;
					case TVL_NOTEQUAL:      fprintf(out, "!=\n");                   break;
					case TVL_BAND:          fprintf(out, "&\n");                    break;
					case TVL_BXOR:          fprintf(out, "^\n");                    break;
					case TVL_BOR:           fprintf(out, "|\n");                    break;
					case TVL_LAND:          fprintf(out, "&&\n");                   break;
					case TVL_LOR:           fprintf(out, "||\n");                   break;
					case TVL_ASSIGN:        fprintf(out, "=\n");                    break;
					case TVL_ASSIGNMULTIPLY:fprintf(out, "*=\n");                   break;
					case TVL_ASSIGNDIVIDE:  fprintf(out, "/=\n");                   break;
					case TVL_ASSIGNMODULO:  fprintf(out, "%%=\n");                  break;
					case TVL_ASSIGNADD:     fprintf(out, "+=\n");                   break;
					case TVL_ASSIGNSUBTRACT:fprintf(out, "-=\n");                   break;
					case TVL_ASSIGNLSHIFT:  fprintf(out, "<<=\n");                  break;
					case TVL_ASSIGNRSHIFT:  fprintf(out, ">>=\n");                  break;
					case TVL_ASSIGNBAND:    fprintf(out, "&=\n");                   break;
					case TVL_ASSIGNBXOR:    fprintf(out, "^=\n");                   break;
					case TVL_ASSIGNBOR:     fprintf(out, "|=\n");                   break;
					case TVL_COMMA:         fprintf(out, ",\n");                    break;
					case TVL_MEMORYAT:      fprintf(out, "mem@\n");                 break;
					case TVL_EXECUTEFUNC:   fprintf(out, "execute\n");              break;
					default:                fprintf(out, "INVALID OPERATOR\n");     break;
				}
				break;
		}
	}
	osd_printf_debug("----\n");
#endif
}


//-------------------------------------------------
//  parse_string_into_tokens - take an expression
//  string and break it into a sequence of tokens
//-------------------------------------------------

void parsed_expression::parse_string_into_tokens()
{
	// loop until done
	const char *stringstart = m_original_string.c_str();
	const char *string = stringstart;
	while (string[0] != 0)
	{
		// ignore any whitespace
		while (string[0] != 0 && isspace((UINT8)string[0]))
			string++;
		if (string[0] == 0)
			break;

		// initialize the current token object
		parse_token &token = m_tokenlist.append(*global_alloc(parse_token(string - stringstart)));

		// switch off the first character
		switch (tolower((UINT8)string[0]))
		{
			case '(':
				string += 1, token.configure_operator(TVL_LPAREN, 0);
				break;

			case ')':
				string += 1, token.configure_operator(TVL_RPAREN, 0);
				break;

			case '~':
				string += 1, token.configure_operator(TVL_NOT, 2);
				break;

			case ',':
				string += 1, token.configure_operator(TVL_COMMA, 14);
				break;

			case '+':
				if (string[1] == '+')
					string += 2, token.configure_operator(TVL_PLUSPLUS, 1);
				else if (string[1] == '=')
					string += 2, token.configure_operator(TVL_ASSIGNADD, 13).set_right_to_left();
				else
					string += 1, token.configure_operator(TVL_ADD, 4);
				break;

			case '-':
				if (string[1] == '-')
					string += 2, token.configure_operator(TVL_MINUSMINUS, 1);
				else if (string[1] == '=')
					string += 2, token.configure_operator(TVL_ASSIGNSUBTRACT, 13).set_right_to_left();
				else
					string += 1, token.configure_operator(TVL_SUBTRACT, 4);
				break;

			case '*':
				if (string[1] == '=')
					string += 2, token.configure_operator(TVL_ASSIGNMULTIPLY, 13).set_right_to_left();
				else
					string += 1, token.configure_operator(TVL_MULTIPLY, 3);
				break;

			case '/':
				if (string[1] == '=')
					string += 2, token.configure_operator(TVL_ASSIGNDIVIDE, 13).set_right_to_left();
				else
					string += 1, token.configure_operator(TVL_DIVIDE, 3);
				break;

			case '%':
				if (string[1] == '=')
					string += 2, token.configure_operator(TVL_ASSIGNMODULO, 13).set_right_to_left();
				else
					string += 1, token.configure_operator(TVL_MODULO, 3);
				break;

			case '<':
				if (string[1] == '<' && string[2] == '=')
					string += 3, token.configure_operator(TVL_ASSIGNLSHIFT, 13).set_right_to_left();
				else if (string[1] == '<')
					string += 2, token.configure_operator(TVL_LSHIFT, 5);
				else if (string[1] == '=')
					string += 2, token.configure_operator(TVL_LESSOREQUAL, 6);
				else
					string += 1, token.configure_operator(TVL_LESS, 6);
				break;

			case '>':
				if (string[1] == '>' && string[2] == '=')
					string += 3, token.configure_operator(TVL_ASSIGNRSHIFT, 13).set_right_to_left();
				else if (string[1] == '>')
					string += 2, token.configure_operator(TVL_RSHIFT, 5);
				else if (string[1] == '=')
					string += 2, token.configure_operator(TVL_GREATEROREQUAL, 6);
				else
					string += 1, token.configure_operator(TVL_GREATER, 6);
				break;

			case '=':
				if (string[1] == '=')
					string += 2, token.configure_operator(TVL_EQUAL, 7);
				else
					string += 1, token.configure_operator(TVL_ASSIGN, 13).set_right_to_left();
				break;

			case '!':
				if (string[1] == '=')
					string += 2, token.configure_operator(TVL_NOTEQUAL, 7);
				else
					string += 2, token.configure_operator(TVL_COMPLEMENT, 2);
				break;

			case '&':
				if (string[1] == '&')
					string += 2, token.configure_operator(TVL_LAND, 11);
				else if (string[1] == '=')
					string += 2, token.configure_operator(TVL_ASSIGNBAND, 13).set_right_to_left();
				else
					string += 1, token.configure_operator(TVL_BAND, 8);
				break;

			case '|':
				if (string[1] == '|')
					string += 2, token.configure_operator(TVL_LOR, 12);
				else if (string[1] == '=')
					string += 2, token.configure_operator(TVL_ASSIGNBOR, 13).set_right_to_left();
				else
					string += 1, token.configure_operator(TVL_BOR, 10);
				break;

			case '^':
				if (string[1] == '=')
					string += 2, token.configure_operator(TVL_ASSIGNBXOR, 13).set_right_to_left();
				else
					string += 1, token.configure_operator(TVL_BXOR, 9);
				break;

			case '"':
				parse_quoted_string(token, string);
				break;

			case '\'':
				parse_quoted_char(token, string);
				break;

			default:
				parse_symbol_or_number(token, string);
				break;
		}
	}
}


//-------------------------------------------------
//  parse_symbol_or_number - parse a substring
//  into either a symbol or a number or an
//  expanded operator
//-------------------------------------------------

void parsed_expression::parse_symbol_or_number(parse_token &token, const char *&string)
{
	// accumulate a lower-case version of the symbol
	const char *stringstart = string;
	std::string buffer;
	while (1)
	{
		static const char valid[] = "abcdefghijklmnopqrstuvwxyz0123456789_$#.:";
		char val = tolower((UINT8)string[0]);
		if (val == 0 || strchr(valid, val) == nullptr)
			break;
		buffer.append(&val, 1);
		string++;
	}

	// check for memory @ operators
	if (string[0] == '@')
	{
		string += 1;
		return parse_memory_operator(token, buffer.c_str());
	}

	// empty string is automatically invalid
	if (buffer.empty())
		throw expression_error(expression_error::INVALID_TOKEN, token.offset());

	// check for wordy variants on standard operators
	if (buffer.compare("bnot")==0)
		{ token.configure_operator(TVL_NOT, 2); return; }
	if (buffer.compare("plus") == 0)
		{ token.configure_operator(TVL_ADD, 4); return; }
	if (buffer.compare("minus") == 0)
		{ token.configure_operator(TVL_SUBTRACT, 4); return; }
	if (buffer.compare("times") == 0 || buffer.compare("mul") == 0)
		{ token.configure_operator(TVL_MULTIPLY, 3); return; }
	if (buffer.compare("div") == 0)
		{ token.configure_operator(TVL_DIVIDE, 3); return; }
	if (buffer.compare("mod") == 0)
		{ token.configure_operator(TVL_MODULO, 3); return; }
	if (buffer.compare("lt") == 0)
		{ token.configure_operator(TVL_LESS, 6); return; }
	if (buffer.compare("le") == 0)
		{ token.configure_operator(TVL_LESSOREQUAL, 6); return; }
	if (buffer.compare("gt") == 0)
		{ token.configure_operator(TVL_GREATER, 6); return; }
	if (buffer.compare("ge") == 0)
		{ token.configure_operator(TVL_GREATEROREQUAL, 6); return; }
	if (buffer.compare("eq") == 0)
		{ token.configure_operator(TVL_EQUAL, 7); return; }
	if (buffer.compare("ne") == 0)
		{ token.configure_operator(TVL_NOTEQUAL, 7); return; }
	if (buffer.compare("not") == 0)
		{ token.configure_operator(TVL_COMPLEMENT, 2); return; }
	if (buffer.compare("and") == 0)
		{ token.configure_operator(TVL_LAND, 8); return; }
	if (buffer.compare("band") == 0)
		{ token.configure_operator(TVL_BAND, 8); return; }
	if (buffer.compare("or") == 0)
		{ token.configure_operator(TVL_LOR, 12); return; }
	if (buffer.compare("bor") == 0)
		{ token.configure_operator(TVL_BOR, 10); return; }
	if (buffer.compare("bxor") == 0)
		{ token.configure_operator(TVL_BXOR, 9); return; }
	if (buffer.compare("lshift") == 0)
		{ token.configure_operator(TVL_LSHIFT, 5); return; }
	if (buffer.compare("rshift") == 0)
		{ token.configure_operator(TVL_RSHIFT, 5); return; }

	// if we have an 0x prefix, we must be a hex value
	if (buffer[0] == '0' && buffer[1] == 'x')
		return parse_number(token, buffer.c_str() + 2, 16, expression_error::INVALID_NUMBER);

	// if we have a # prefix, we must be a decimal value
	if (buffer[0] == '#')
		return parse_number(token, buffer.c_str() + 1, 10, expression_error::INVALID_NUMBER);

	// if we have a $ prefix, we are a hex value
	if (buffer[0] == '$')
		return parse_number(token, buffer.c_str() + 1, 16, expression_error::INVALID_NUMBER);

	// check for a symbol match
	symbol_entry *symbol = m_symtable->find_deep(buffer.c_str());
	if (symbol != nullptr)
	{
		token.configure_symbol(*symbol);

		// if this is a function symbol, synthesize an execute function operator
		if (symbol->is_function())
		{
			parse_token &newtoken = m_tokenlist.append(*global_alloc(parse_token(string - stringstart)));
			newtoken.configure_operator(TVL_EXECUTEFUNC, 0);
		}
		return;
	}

	// attempt to parse as a number in the default base
	parse_number(token, buffer.c_str(), DEFAULT_BASE, expression_error::UNKNOWN_SYMBOL);
}


//-------------------------------------------------
//  parse_number - parse a number using the
//  given base
//-------------------------------------------------

void parsed_expression::parse_number(parse_token &token, const char *string, int base, expression_error::error_code errcode)
{
	// parse the actual value
	UINT64 value = 0;
	while (*string != 0)
	{
		// look up the number's value, stopping if not valid
		static const char numbers[] = "0123456789abcdef";
		const char *ptr = strchr(numbers, tolower((UINT8)*string));
		if (ptr == nullptr)
			break;

		// if outside of the base, we also stop
		int digit = ptr - numbers;
		if (digit >= base)
			break;

		// shift previous digits up and add in new digit
		value = (value * (UINT64)base) + digit;
		string++;
	}

	// if we succeeded as a number, make it so
	if (*string == 0)
		token.configure_number(value);
	else
		throw expression_error(errcode, token.offset());
}


//-------------------------------------------------
//  parse_quoted_char - parse a single-quoted
//  character constant
//-------------------------------------------------

void parsed_expression::parse_quoted_char(parse_token &token, const char *&string)
{
	// accumulate the value of the character token
	string++;
	UINT64 value = 0;
	while (string[0] != 0)
	{
		// allow '' to mean a nested single quote
		if (string[0] == '\'')
		{
			if (string[1] != '\'')
				break;
			string++;
		}
		value = (value << 8) | (UINT8)*string++;
	}

	// if we didn't find the ending quote, report an error
	if (string[0] != '\'')
		throw expression_error(expression_error::UNBALANCED_QUOTES, token.offset());
	string++;

	// make it a number token
	token.configure_number(value);
}


//-------------------------------------------------
//  parse_quoted_string - parse a double-quoted
//  string constant
//-------------------------------------------------

void parsed_expression::parse_quoted_string(parse_token &token, const char *&string)
{
	// accumulate a copy of the quoted string
	string++;
	std::string buffer;
	while (string[0] != 0)
	{
		// allow "" to mean a nested double-quote
		if (string[0] == '"')
		{
			if (string[1] != '"')
				break;
			string++;
		}
		buffer.append(string++, 1);
	}

	// if we didn't find the ending quote, report an error
	if (string[0] != '"')
		throw expression_error(expression_error::UNBALANCED_QUOTES, token.offset());
	string++;

	// make the token
	token.configure_string(m_stringlist.append(*global_alloc(expression_string(buffer.c_str()))));
}


//-------------------------------------------------
//  parse_memory_operator - parse the several
//  forms of memory operators
//-------------------------------------------------

void parsed_expression::parse_memory_operator(parse_token &token, const char *string)
{
	// if there is a '.', it means we have a name
	const char *startstring = string;
	const char *namestring = nullptr;
	const char *dot = strrchr(string, '.');
	if (dot != nullptr)
	{
		namestring = m_stringlist.append(*global_alloc(expression_string(string, dot - string)));
		string = dot + 1;
	}

	// length 3 means logical/physical, then space, then size
	int length = (int)strlen(string);
	bool physical = false;
	int space = 'p';
	int size;
	if (length == 3)
	{
		if (string[0] != 'l' && string[0] != 'p')
			throw expression_error(expression_error::INVALID_MEMORY_SPACE, token.offset() + (string - startstring));
		if (string[1] != 'p' && string[1] != 'd' && string[1] != 'i' && string[1] != '3')
			throw expression_error(expression_error::INVALID_MEMORY_SPACE, token.offset() + (string - startstring));
		physical = (string[0] == 'p');
		space = string[1];
		size = string[2];
	}

	// length 2 means space then size
	else if (length == 2)
	{
		space = string[0];
		size = string[1];
	}

	// length 1 means size
	else if (length == 1)
		size = string[0];

	// anything else is invalid
	else
		throw expression_error(expression_error::INVALID_TOKEN, token.offset());

	// convert the space to flags
	expression_space memspace;
	switch (space)
	{
		case 'p':   memspace = physical ? EXPSPACE_PROGRAM_PHYSICAL : EXPSPACE_PROGRAM_LOGICAL; break;
		case 'd':   memspace = physical ? EXPSPACE_DATA_PHYSICAL    : EXPSPACE_DATA_LOGICAL;    break;
		case 'i':   memspace = physical ? EXPSPACE_IO_PHYSICAL      : EXPSPACE_IO_LOGICAL;      break;
		case '3':   memspace = physical ? EXPSPACE_SPACE3_PHYSICAL  : EXPSPACE_SPACE3_LOGICAL;  break;
		case 'o':   memspace = EXPSPACE_OPCODE;                                                 break;
		case 'r':   memspace = EXPSPACE_RAMWRITE;                                               break;
		case 'm':   memspace = EXPSPACE_REGION;                                                 break;
		default:    throw expression_error(expression_error::INVALID_MEMORY_SPACE, token.offset() + (string - startstring));
	}

	// convert the size to flags
	int memsize;
	switch (size)
	{
		case 'b':   memsize = 0;    break;
		case 'w':   memsize = 1;    break;
		case 'd':   memsize = 2;    break;
		case 'q':   memsize = 3;    break;
		default:    throw expression_error(expression_error::INVALID_MEMORY_SIZE, token.offset() + (string - startstring) + length - 1);
	}

	// validate the name
	if (m_symtable != nullptr)
	{
		expression_error::error_code err = m_symtable->memory_valid(namestring, memspace);
		if (err != expression_error::NONE)
			throw expression_error(err, token.offset() + (string - startstring));
	}

	// configure the token
	token.configure_operator(TVL_MEMORYAT, 2).set_memory_size(memsize).set_memory_space(memspace).set_memory_source(namestring);
}


//-------------------------------------------------
//  normalize_operator - resolve operator
//  ambiguities based on neighboring tokens
//-------------------------------------------------

void parsed_expression::normalize_operator(parse_token *prevtoken, parse_token &thistoken)
{
	parse_token *nexttoken = thistoken.next();
	switch (thistoken.optype())
	{
		// Determine if an open paren is part of a function or not
		case TVL_LPAREN:
			if (prevtoken != nullptr && prevtoken->is_operator(TVL_EXECUTEFUNC))
				thistoken.set_function_separator();
			break;

		// Determine if ++ is a pre or post increment
		case TVL_PLUSPLUS:
			if (nexttoken != nullptr && (nexttoken->is_symbol() || (nexttoken->is_operator(TVL_MEMORYAT))))
				thistoken.configure_operator(TVL_PREINCREMENT, 2);
			else if (prevtoken != nullptr && (prevtoken->is_symbol() || (prevtoken->is_operator(TVL_MEMORYAT))))
				thistoken.configure_operator(TVL_POSTINCREMENT, 1);
			else
				throw expression_error(expression_error::SYNTAX, thistoken.offset());
			break;

		// Determine if -- is a pre or post decrement
		case TVL_MINUSMINUS:
			if (nexttoken != nullptr && (nexttoken->is_symbol() || (nexttoken->is_operator(TVL_MEMORYAT))))
				thistoken.configure_operator(TVL_PREDECREMENT, 2);
			else if (prevtoken != nullptr && (prevtoken->is_symbol() || (prevtoken->is_operator(TVL_MEMORYAT))))
				thistoken.configure_operator(TVL_POSTDECREMENT, 1);
			else
				throw expression_error(expression_error::SYNTAX, thistoken.offset());
			break;

		// Determine if +/- is a unary or binary
		case TVL_ADD:
		case TVL_SUBTRACT:
			// Assume we're unary if we are the first token, or if the previous token is not
			// a symbol, a number, or a right parenthesis
			if (prevtoken == nullptr || (!prevtoken->is_symbol() && !prevtoken->is_number() && !prevtoken->is_operator(TVL_RPAREN)))
				thistoken.configure_operator(thistoken.is_operator(TVL_ADD) ? TVL_UPLUS : TVL_UMINUS, 2);
			break;

		// Determine if , refers to a function parameter
		case TVL_COMMA:
			for (int lookback = 0; lookback < MAX_STACK_DEPTH; lookback++)
			{
				parse_token *peek = peek_token(lookback);
				if (peek == nullptr)
					break;

				// if we hit an execute function operator, or else a left parenthesis that is
				// already tagged, then tag us as well
				if (peek->is_operator(TVL_EXECUTEFUNC) || (peek->is_operator(TVL_LPAREN) && peek->is_function_separator()))
				{
					thistoken.set_function_separator();
					break;
				}
			}
			break;
	}
}


//-------------------------------------------------
//  infix_to_postfix - convert an infix sequence
//  of tokens to a postfix sequence for processing
//-------------------------------------------------

void parsed_expression::infix_to_postfix()
{
	simple_list<parse_token> stack;

	// loop over all the original tokens
	parse_token *prev = nullptr;
	parse_token *next;
	for (parse_token *token = m_tokenlist.detach_all(); token != nullptr; prev = token, token = next)
	{
		// pre-determine our next token
		next = token->next();

		// if the character is an operand, append it to the result string
		if (token->is_number() || token->is_symbol() || token->is_string())
			m_tokenlist.append(*token);

		// if this is an operator, process it
		else if (token->is_operator())
		{
			// normalize the operator based on neighbors
			normalize_operator(prev, *token);

			// if the token is an opening parenthesis, push it onto the stack.
			if (token->is_operator(TVL_LPAREN))
				stack.prepend(*token);

			// if the token is a closing parenthesis, pop all operators until we
			// reach an opening parenthesis and append them to the result string,
			// discaring the open parenthesis
			else if (token->is_operator(TVL_RPAREN))
			{
				// loop until we find our matching opener
				parse_token *popped;
				while ((popped = stack.detach_head()) != nullptr)
				{
					if (popped->is_operator(TVL_LPAREN))
						break;
					m_tokenlist.append(*popped);
				}

				// if we didn't find an open paren, it's an error
				if (popped == nullptr)
					throw expression_error(expression_error::UNBALANCED_PARENS, token->offset());

				// free ourself and our matching opening parenthesis
				global_free(token);
				global_free(popped);
			}

			// if the token is an operator, pop operators until we reach an opening parenthesis,
			// an operator of lower precedence, or a right associative symbol of equal precedence.
			// Push the operator onto the stack.
			else
			{
				int our_precedence = token->precedence();

				// loop until we can't peek at the stack anymore
				parse_token *peek;
				while ((peek = stack.first()) != nullptr)
				{
					// break if any of the above conditions are true
					if (peek->is_operator(TVL_LPAREN))
						break;
					int stack_precedence = peek->precedence();
					if (stack_precedence > our_precedence || (stack_precedence == our_precedence && peek->right_to_left()))
						break;

					// pop this token
					m_tokenlist.append(*stack.detach_head());
				}

				// push the new operator
				stack.prepend(*token);
			}
		}
	}

	// finish popping the stack
	parse_token *popped;
	while ((popped = stack.detach_head()) != nullptr)
	{
		// it is an error to have a left parenthesis still on the stack
		if (popped->is_operator(TVL_LPAREN))
			throw expression_error(expression_error::UNBALANCED_PARENS, popped->offset());

		// pop this token
		m_tokenlist.append(*popped);
	}
}


//-------------------------------------------------
//  push_token - push a token onto the stack
//-------------------------------------------------

inline void parsed_expression::push_token(parse_token &token)
{
	// check for overflow
	if (m_token_stack_ptr >= MAX_STACK_DEPTH)
		throw expression_error(expression_error::STACK_OVERFLOW, token.offset());

	// push
	m_token_stack[m_token_stack_ptr++] = token;
}


//-------------------------------------------------
//  pop_token - pop a token off the stack
//-------------------------------------------------

inline void parsed_expression::pop_token(parse_token &token)
{
	// check for underflow
	if (m_token_stack_ptr == 0)
		throw expression_error(expression_error::STACK_UNDERFLOW, token.offset());

	// pop
	token = m_token_stack[--m_token_stack_ptr];
}


//-------------------------------------------------
//  peek_token - look at a token some number of
//  entries up the stack
//-------------------------------------------------

inline parsed_expression::parse_token *parsed_expression::peek_token(int count)
{
	if (m_token_stack_ptr <= count)
		return nullptr;
	return &m_token_stack[m_token_stack_ptr - count - 1];
}


//-------------------------------------------------
//  pop_token_lval - pop a token off the stack
//  and ensure that it is a proper lval
//-------------------------------------------------

inline void parsed_expression::pop_token_lval(parse_token &token)
{
	// start with normal pop
	pop_token(token);

	// if we're not an lval, throw an error
	if (!token.is_lval())
		throw expression_error(expression_error::NOT_LVAL, token.offset());
}


//-------------------------------------------------
//  pop_token_rval - pop a token off the stack
//  and ensure that it is a proper rval
//-------------------------------------------------

inline void parsed_expression::pop_token_rval(parse_token &token)
{
	// start with normal pop
	pop_token(token);

	// symbol and memory tokens get resolved down to number tokens
	if (token.is_symbol() || token.is_memory())
		token.configure_number(token.get_lval_value(m_symtable));

	// to be an rval, the final token must be a number
	if (!token.is_number())
		throw expression_error(expression_error::NOT_RVAL, token.offset());
}


//-------------------------------------------------
//  execute_tokens - execute a postfix sequence
//  of tokens
//-------------------------------------------------

UINT64 parsed_expression::execute_tokens()
{
	// reset the token stack
	m_token_stack_ptr = 0;

	// loop over the entire sequence
	parse_token t1, t2, result;
	for (parse_token &token : m_tokenlist)
	{
		// symbols/numbers/strings just get pushed
		if (!token.is_operator())
		{
			push_token(token);
			continue;
		}

		// otherwise, switch off the operator
		switch (token.optype())
		{
			case TVL_PREINCREMENT:
				pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) + 1).set_offset(t1));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_PREDECREMENT:
				pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) - 1).set_offset(t1));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_POSTINCREMENT:
				pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable)).set_offset(t1));
				t1.set_lval_value(m_symtable, result.value() + 1);
				break;

			case TVL_POSTDECREMENT:
				pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable)).set_offset(t1));
				t1.set_lval_value(m_symtable, result.value() - 1);
				break;

			case TVL_COMPLEMENT:
				pop_token_rval(t1);
				push_token(result.configure_number(!t1.value()).set_offset(t1));
				break;

			case TVL_NOT:
				pop_token_rval(t1);
				push_token(result.configure_number(~t1.value()).set_offset(t1));
				break;

			case TVL_UPLUS:
				pop_token_rval(t1);
				push_token(result.configure_number(t1.value()).set_offset(t1));
				break;

			case TVL_UMINUS:
				pop_token_rval(t1);
				push_token(result.configure_number(-t1.value()).set_offset(t1));
				break;

			case TVL_MULTIPLY:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() * t2.value()).set_offset(t1, t2));
				break;

			case TVL_DIVIDE:
				pop_token_rval(t2); pop_token_rval(t1);
				if (t2.value() == 0)
					throw expression_error(expression_error::DIVIDE_BY_ZERO, t2.offset());
				push_token(result.configure_number(t1.value() / t2.value()).set_offset(t1, t2));
				break;

			case TVL_MODULO:
				pop_token_rval(t2); pop_token_rval(t1);
				if (t2.value() == 0)
					throw expression_error(expression_error::DIVIDE_BY_ZERO, t2.offset());
				push_token(result.configure_number(t1.value() % t2.value()).set_offset(t1, t2));
				break;

			case TVL_ADD:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() + t2.value()).set_offset(t1, t2));
				break;

			case TVL_SUBTRACT:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() - t2.value()).set_offset(t1, t2));
				break;

			case TVL_LSHIFT:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() << t2.value()).set_offset(t1, t2));
				break;

			case TVL_RSHIFT:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() >> t2.value()).set_offset(t1, t2));
				break;

			case TVL_LESS:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() < t2.value()).set_offset(t1, t2));
				break;

			case TVL_LESSOREQUAL:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() <= t2.value()).set_offset(t1, t2));
				break;

			case TVL_GREATER:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() > t2.value()).set_offset(t1, t2));
				break;

			case TVL_GREATEROREQUAL:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() >= t2.value()).set_offset(t1, t2));
				break;

			case TVL_EQUAL:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() == t2.value()).set_offset(t1, t2));
				break;

			case TVL_NOTEQUAL:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() != t2.value()).set_offset(t1, t2));
				break;

			case TVL_BAND:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() & t2.value()).set_offset(t1, t2));
				break;

			case TVL_BXOR:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() ^ t2.value()).set_offset(t1, t2));
				break;

			case TVL_BOR:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() | t2.value()).set_offset(t1, t2));
				break;

			case TVL_LAND:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() && t2.value()).set_offset(t1, t2));
				break;

			case TVL_LOR:
				pop_token_rval(t2); pop_token_rval(t1);
				push_token(result.configure_number(t1.value() || t2.value()).set_offset(t1, t2));
				break;

			case TVL_ASSIGN:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t2.value()).set_offset(t2));
				t1.set_lval_value(m_symtable, t2.value());
				break;

			case TVL_ASSIGNMULTIPLY:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) * t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNDIVIDE:
				pop_token_rval(t2); pop_token_lval(t1);
				if (t2.value() == 0)
					throw expression_error(expression_error::DIVIDE_BY_ZERO, t2.offset());
				push_token(result.configure_number(t1.get_lval_value(m_symtable) / t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNMODULO:
				pop_token_rval(t2); pop_token_lval(t1);
				if (t2.value() == 0)
					throw expression_error(expression_error::DIVIDE_BY_ZERO, t2.offset());
				push_token(result.configure_number(t1.get_lval_value(m_symtable) % t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNADD:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) + t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNSUBTRACT:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) - t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNLSHIFT:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) << t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNRSHIFT:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) >> t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNBAND:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) & t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNBXOR:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) ^ t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_ASSIGNBOR:
				pop_token_rval(t2); pop_token_lval(t1);
				push_token(result.configure_number(t1.get_lval_value(m_symtable) | t2.value()).set_offset(t1, t2));
				t1.set_lval_value(m_symtable, result.value());
				break;

			case TVL_COMMA:
				if (!token.is_function_separator())
				{
					pop_token_rval(t2); pop_token_rval(t1);
					push_token(t2);
				}
				break;

			case TVL_MEMORYAT:
				pop_token_rval(t1);
				push_token(result.configure_memory(t1.value(), token));
				break;

			case TVL_EXECUTEFUNC:
				execute_function(token);
				break;

			default:
				throw expression_error(expression_error::SYNTAX, token.offset());
		}
	}

	// pop the final result
	pop_token_rval(result);

	// error if our stack isn't empty
	if (peek_token(0) != nullptr)
		throw expression_error(expression_error::SYNTAX, 0);

	return result.value();
}



//**************************************************************************
//  PARSE TOKEN
//**************************************************************************

//-------------------------------------------------
//  parse_token - constructor
//-------------------------------------------------

parsed_expression::parse_token::parse_token(int offset)
	: m_next(nullptr),
		m_type(INVALID),
		m_offset(offset),
		m_value(0),
		m_flags(0),
		m_string(nullptr),
		m_symbol(nullptr)
{
}


//-------------------------------------------------
//  get_lval_value - call the getter function
//  for a SYMBOL token
//-------------------------------------------------

UINT64 parsed_expression::parse_token::get_lval_value(symbol_table *table)
{
	// get the value of a symbol
	if (is_symbol())
		return m_symbol->value();

	// or get the value from the memory callbacks
	else if (is_memory() && table != nullptr)
		return table->memory_value(m_string, memory_space(), address(), 1 << memory_size());

	return 0;
}


//-------------------------------------------------
//  set_lval_value - call the setter function
//  for a SYMBOL token
//-------------------------------------------------

inline void parsed_expression::parse_token::set_lval_value(symbol_table *table, UINT64 value)
{
	// set the value of a symbol
	if (is_symbol())
		m_symbol->set_value(value);

	// or set the value via the memory callbacks
	else if (is_memory() && table != nullptr)
		table->set_memory_value(m_string, memory_space(), address(), 1 << memory_size(), value);
}


//-------------------------------------------------
//  execute_function - handle an execute function
//  operator
//-------------------------------------------------

void parsed_expression::execute_function(parse_token &token)
{
	// pop off all pushed parameters
	UINT64 funcparams[MAX_FUNCTION_PARAMS];
	symbol_entry *symbol = nullptr;
	int paramcount = 0;
	while (paramcount < MAX_FUNCTION_PARAMS)
	{
		// peek at the next token on the stack
		parse_token *peek = peek_token(0);
		if (peek == nullptr)
			throw expression_error(expression_error::INVALID_PARAM_COUNT, token.offset());

		// if it is a function symbol, break out of the loop
		if (peek->is_symbol())
		{
			symbol = peek->symbol();
			if (symbol->is_function())
			{
				parse_token t1;
				pop_token(t1);
				break;
			}
		}

		// otherwise, pop as a standard rval
		parse_token t1;
		pop_token_rval(t1);
		funcparams[MAX_FUNCTION_PARAMS - (++paramcount)] = t1.value();
	}

	// if we didn't find the symbol, fail
	if (paramcount == MAX_FUNCTION_PARAMS)
		throw expression_error(expression_error::INVALID_PARAM_COUNT, token.offset());

	// execute the function and push the result
	function_symbol_entry *function = downcast<function_symbol_entry *>(symbol);
	parse_token result(token.offset());
	result.configure_number(function->execute(paramcount, &funcparams[MAX_FUNCTION_PARAMS - paramcount]));
	push_token(result);
}
