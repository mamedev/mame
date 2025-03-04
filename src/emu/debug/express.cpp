// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    express.cpp

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
#include "emuopts.h"
#include "express.h"
#include "debugger.h"
#include "srcdbg_provider.h"

#include "corestr.h"
#include <cctype>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_OUTPUT_FUNC         osd_printf_info
#define VERBOSE                 0
#include "logmacro.h"


namespace {

/***************************************************************************
    CONSTANTS
***************************************************************************/

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
//  REGISTER SYMBOL ENTRY
//**************************************************************************

// a symbol entry representing a register, with read/write callbacks
class integer_symbol_entry : public symbol_entry
{
public:
	// construction/destruction
	integer_symbol_entry(symbol_table &table, const char *name, symbol_table::read_write rw, u64 *ptr = nullptr);
	integer_symbol_entry(symbol_table &table, const char *name, u64 constval);
	integer_symbol_entry(symbol_table &table, const char *name, symbol_table::getter_func getter, symbol_table::setter_func setter, const std::string &format);

	// symbol access
	virtual bool is_lval() const override { return m_setter != nullptr; }
	virtual u64 value() const override { return m_getter(); }
	virtual void set_value(u64 newvalue) override;

private:
	// internal state
	symbol_table::getter_func   m_getter;
	symbol_table::setter_func   m_setter;
	u64                         m_value;
};


//-------------------------------------------------
//  integer_symbol_entry - constructor
//-------------------------------------------------

integer_symbol_entry::integer_symbol_entry(symbol_table &table, const char *name, symbol_table::read_write rw, u64 *ptr)
	: symbol_entry(table, SMT_INTEGER, name, ""),
		m_getter(ptr
				? symbol_table::getter_func([ptr] () { return *ptr; })
				: symbol_table::getter_func([this] () { return m_value; })),
		m_setter((rw == symbol_table::READ_ONLY)
				? symbol_table::setter_func(nullptr)
				: ptr
				? symbol_table::setter_func([ptr] (u64 value) { *ptr = value; })
				: symbol_table::setter_func([this] (u64 value) { m_value = value; })),
		m_value(0)
{
}


integer_symbol_entry::integer_symbol_entry(symbol_table &table, const char *name, u64 constval)
	: symbol_entry(table, SMT_INTEGER, name, ""),
		m_getter([this] () { return m_value; }),
		m_setter(nullptr),
		m_value(constval)
{
}


integer_symbol_entry::integer_symbol_entry(symbol_table &table, const char *name, symbol_table::getter_func getter, symbol_table::setter_func setter, const std::string &format)
	: symbol_entry(table, SMT_INTEGER, name, format),
		m_getter(std::move(getter)),
		m_setter(std::move(setter)),
		m_value(0)
{
}


//-------------------------------------------------
//  set_value - set the value of this symbol
//-------------------------------------------------

void integer_symbol_entry::set_value(u64 newvalue)
{
	if (m_setter != nullptr)
		m_setter(newvalue);
	else
		throw emu_fatalerror("Symbol '%s' is read-only", m_name);
}



//**************************************************************************
//  FUNCTION SYMBOL ENTRY
//**************************************************************************

// a symbol entry representing a function
class function_symbol_entry : public symbol_entry
{
public:
	// construction/destruction
	function_symbol_entry(symbol_table &table, const char *name, int minparams, int maxparams, symbol_table::execute_func execute);

	// getters
	u16 minparams() const { return m_minparams; }
	u16 maxparams() const { return m_maxparams; }

	// symbol access
	virtual bool is_lval() const override { return false; }
	virtual u64 value() const override;
	virtual void set_value(u64 newvalue) override;

	// execution helper
	virtual u64 execute(int numparams, const u64 *paramlist);

private:
	// internal state
	u16                         m_minparams;
	u16                         m_maxparams;
	symbol_table::execute_func  m_execute;
};


//-------------------------------------------------
//  function_symbol_entry - constructor
//-------------------------------------------------

function_symbol_entry::function_symbol_entry(symbol_table &table, const char *name, int minparams, int maxparams, symbol_table::execute_func execute)
	: symbol_entry(table, SMT_FUNCTION, name, ""),
		m_minparams(minparams),
		m_maxparams(maxparams),
		m_execute(std::move(execute))
{
}


//-------------------------------------------------
//  value - return the value of this symbol
//-------------------------------------------------

u64 function_symbol_entry::value() const
{
	throw emu_fatalerror("Symbol '%s' is a function and cannot be used in this context", m_name);
}


//-------------------------------------------------
//  set_value - set the value of this symbol
//-------------------------------------------------

void function_symbol_entry::set_value(u64 newvalue)
{
	throw emu_fatalerror("Symbol '%s' is a function and cannot be written", m_name);
}


//-------------------------------------------------
//  execute - execute the function
//-------------------------------------------------

u64 function_symbol_entry::execute(int numparams, const u64 *paramlist)
{
	if (numparams < m_minparams)
		throw emu_fatalerror("Function '%s' requires at least %d parameters", m_name, m_minparams);
	if (numparams > m_maxparams)
		throw emu_fatalerror("Function '%s' accepts no more than %d parameters", m_name, m_maxparams);
	return m_execute(numparams, paramlist);
}



//**************************************************************************
//  LOCAL FIXED SYMBOL ENTRY
//**************************************************************************

// a symbol entry representing a lexically-scoped local symbol obtained from
// source-debugging info.  Value is fixed (independent of PC)
class local_fixed_symbol_entry : public symbol_entry
{
public:
	// construction/destruction
	local_fixed_symbol_entry(symbol_table &table, const char *name, symbol_table::getter_func get_pc, const std::vector<std::pair<offs_t,offs_t>> & scope_ranges, u64 value);

	// symbol access
	virtual bool is_lval() const override { return false; }
	virtual u64 value() const override { return m_value_integer; };
	virtual void set_value(u64 newvalue) override;
	virtual bool is_in_scope() const override;

private:
	// internal state
	symbol_table::getter_func m_get_pc;
	const std::vector<std::pair<offs_t,offs_t>> & m_scope_ranges;
	u64 m_value_integer;
	std::string m_value_expression;
};


//-------------------------------------------------
//  local_fixed_symbol_entry - constructor
//-------------------------------------------------

local_fixed_symbol_entry::local_fixed_symbol_entry(symbol_table &table, const char *name, symbol_table::getter_func get_pc, const std::vector<std::pair<offs_t,offs_t>> & scope_ranges, u64 value)
	: symbol_entry(table, SMT_INTEGER, name, "")
	, m_get_pc(get_pc)
	, m_scope_ranges(scope_ranges)
	, m_value_integer(value)
{
}


//-------------------------------------------------
//  set_value - Disallowed, read-only
//-------------------------------------------------

void local_fixed_symbol_entry::set_value(u64 newvalue)
{
	throw emu_fatalerror("Symbol '%s' is the location of a local variable, and is read-only", m_name);
}


//-------------------------------------------------
//  is_in_scope - Determines if symbol is
//  currently in scope
//-------------------------------------------------

bool local_fixed_symbol_entry::is_in_scope() const
{
	u64 pc = m_get_pc();
	for (std::pair<offs_t, offs_t> range : m_scope_ranges)
	{
		if (range.first <= pc && pc <= range.second)
		{
			return true;
		}
	}
	return false;
}


//**************************************************************************
//  LOCAL RELATIVE SYMBOL ENTRY
//**************************************************************************

// a symbol entry representing a lexically-scoped local symbol obtained from
// source-debugging info.  Value determined by an offset to a register
class local_relative_symbol_entry : public symbol_entry
{
public:
	// construction/destruction
	local_relative_symbol_entry(symbol_table &table, const char *name, symbol_table::getter_func get_pc, const std::vector<symbol_table::local_range_expression> & scoped_values);

	// symbol access
	virtual bool is_lval() const override { return false; }
	virtual u64 value() const override;
	virtual void set_value(u64 newvalue) override;
	virtual bool is_in_scope() const override;

private:
	// internal state
	symbol_table::getter_func m_get_pc;
	const std::vector<symbol_table::local_range_expression> & m_local_range_expressions;
};


//-------------------------------------------------
//  local_relative_symbol_entry - constructor
//-------------------------------------------------

local_relative_symbol_entry::local_relative_symbol_entry(symbol_table &table, const char *name, symbol_table::getter_func get_pc, const std::vector<symbol_table::local_range_expression> & scoped_values)
	: symbol_entry(table, SMT_INTEGER, name, "")
	, m_get_pc(get_pc)
	, m_local_range_expressions(scoped_values)
{
}


//-------------------------------------------------
//  value - Calculates value by evaluating
//  expression in the form of a register offset
//-------------------------------------------------

u64 local_relative_symbol_entry::value() const
{
	u64 pc = m_get_pc();
	for (symbol_table::local_range_expression val : m_local_range_expressions)
	{
		if (val.address_range().first <= pc && pc <= val.address_range().second)
		{
			return parsed_expression(m_table, val.expression()).execute();
		}
	}

	assert(false && "local_relative_symbol_entry::value() called when not in scope");
	return 0;
}


//-------------------------------------------------
//  set_value - Disallowed, read-only
//-------------------------------------------------

void local_relative_symbol_entry::set_value(u64 newvalue)
{
	throw emu_fatalerror("Symbol '%s' is the location of a local variable, and is read-only", m_name);
}


//-------------------------------------------------
//  is_in_scope - Determines if symbol is
//  currently in scope
//-------------------------------------------------

bool local_relative_symbol_entry::is_in_scope() const
{
	u64 pc = m_get_pc();
	for (symbol_table::local_range_expression val : m_local_range_expressions)
	{
		if (val.address_range().first <= pc && pc <= val.address_range().second)
		{
			return true;
		}
	}
	return false;
}



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline std::pair<device_t &, char const *> get_device_search(running_machine &machine, device_memory_interface *memintf, char const *tag)
{
	if (tag)
	{
		if (('.' == tag[0]) && (!tag[1] || (':' == tag[1]) || ('^' == tag[1])))
			return std::pair<device_t &, char const *>(memintf ? memintf->device() : machine.root_device(), tag + ((':' == tag[1]) ? 2 : 1));
		else if (('^' == tag[0]) && memintf)
			return std::pair<device_t &, char const *>(memintf->device(), tag);
		else
			return std::pair<device_t &, char const *>(machine.root_device(), tag);
	}
	else if (memintf)
	{
		return std::pair<device_t &, char const *>(memintf->device(), "");
	}
	else
	{
		return std::pair<device_t &, char const *>(machine.root_device(), "");
	}
}

} // anonymous namespace



//**************************************************************************
//  EXPRESSION ERROR
//**************************************************************************

//-------------------------------------------------
//  code_string - return a friendly string for a
//  given expression error
//-------------------------------------------------

std::string expression_error::code_string() const
{
	switch (m_code)
	{
		case NOT_LVAL:                      return "not an lvalue";
		case NOT_RVAL:                      return "not an rvalue";
		case SYNTAX:                        return "syntax error";
		case UNKNOWN_SYMBOL:                return "unknown symbol";
		case INVALID_NUMBER:                return "invalid number";
		case INVALID_TOKEN:                 return "invalid token";
		case STACK_OVERFLOW:                return "stack overflow";
		case STACK_UNDERFLOW:               return "stack underflow";
		case UNBALANCED_PARENS:             return "unbalanced parentheses";
		case DIVIDE_BY_ZERO:                return "divide by zero";
		case OUT_OF_MEMORY:                 return "out of memory";
		case INVALID_PARAM_COUNT:           return "invalid number of parameters";
		case TOO_FEW_PARAMS:                return util::string_format("too few parameters (at least %d required)", m_num);
		case TOO_MANY_PARAMS:               return util::string_format("too many parameters (no more than %d accepted)", m_num);
		case UNBALANCED_QUOTES:             return "unbalanced quotes";
		case TOO_MANY_STRINGS:              return "too many strings";
		case INVALID_MEMORY_SIZE:           return "invalid memory size (b/w/d/q expected)";
		case NO_SUCH_MEMORY_SPACE:          return "non-existent memory space";
		case INVALID_MEMORY_SPACE:          return "invalid memory space (p/d/i/o/r/m expected)";
		case INVALID_MEMORY_NAME:           return "invalid memory name";
		case MISSING_MEMORY_NAME:           return "missing memory name";
		case SRCDBG_UNAVAILABLE:            return "source-level debugging information is unavailable; '" OPTION_SRCDBGINFO "' option required";
		case SRCDBG_FILE_UNAVAILABLE:       return "specified file path does not uniquely identify a source path from the source-level debugging information";
		case SRCDBG_FILE_LINE_UNAVAILABLE:  return "specified file path was found, but no address is attributed to specified line number";
		default:                            return "unknown error";
	}
}



//**************************************************************************
//  SYMBOL ENTRY
//**************************************************************************

//-------------------------------------------------
//  symbol_entry - constructor
//-------------------------------------------------

symbol_entry::symbol_entry(symbol_table &table, symbol_type type, const char *name, const std::string &format)
	: m_table(table),
		m_type(type),
		m_name(name),
		m_format(format)
{
}


//-------------------------------------------------
//  ~symbol_entry - destructor
//-------------------------------------------------

symbol_entry::~symbol_entry()
{
}



//**************************************************************************
//  SYMBOL TABLE
//**************************************************************************

//-------------------------------------------------
//  symbol_table - constructor
//-------------------------------------------------

symbol_table::symbol_table(running_machine &machine, table_type type, symbol_table *parent, device_t *device)
	: m_machine(machine)
	, m_type(type)
	, m_parent(parent)
	, m_memintf(dynamic_cast<device_memory_interface *>(device))
	, m_memory_modified(nullptr)
{
}


//-------------------------------------------------
//  set_memory_modified_func - install notifier
//  for when memory is modified in debugger
//-------------------------------------------------

void symbol_table::set_memory_modified_func(memory_modified_func modified)
{
	m_memory_modified = std::move(modified);
}


//-------------------------------------------------
//  add - add a new u64 pointer symbol
//-------------------------------------------------

symbol_entry &symbol_table::add(const char *name, read_write rw, u64 *ptr)
{
	m_symlist.erase(name);
	return *m_symlist.emplace(name, std::make_unique<integer_symbol_entry>(*this, name, rw, ptr)).first->second;
}


//-------------------------------------------------
//  add - add a new value symbol
//-------------------------------------------------

symbol_entry &symbol_table::add(const char *name, u64 value)
{
	m_symlist.erase(name);
	return *m_symlist.emplace(name, std::make_unique<integer_symbol_entry>(*this, name, value)).first->second;
}


//-------------------------------------------------
//  add - add a new register symbol
//-------------------------------------------------

symbol_entry &symbol_table::add(const char *name, getter_func getter, setter_func setter, const std::string &format_string)
{
	m_symlist.erase(name);
	return *m_symlist.emplace(name, std::make_unique<integer_symbol_entry>(*this, name, getter, setter, format_string)).first->second;
}


//-------------------------------------------------
//  add - add a new function symbol
//-------------------------------------------------

symbol_entry &symbol_table::add(const char *name, int minparams, int maxparams, execute_func execute)
{
	m_symlist.erase(name);
	return *m_symlist.emplace(name, std::make_unique<function_symbol_entry>(*this, name, minparams, maxparams, execute)).first->second;
}


//-------------------------------------------------
//-------------------------------------------------

symbol_entry &symbol_table::add(const char *name, symbol_table::getter_func get_pc, const std::vector<std::pair<offs_t,offs_t>> & scope_ranges, u64 value)
{
	return *m_symlist.emplace(name, std::make_unique<local_fixed_symbol_entry>(*this, name, get_pc, scope_ranges, value)).first->second;
}


//-------------------------------------------------
//-------------------------------------------------

symbol_entry &symbol_table::add(const char *name, symbol_table::getter_func get_pc, const std::vector<local_range_expression> & scoped_values)
{
	return *m_symlist.emplace(name, std::make_unique<local_relative_symbol_entry>(*this, name, get_pc, scoped_values)).first->second;
}


//-------------------------------------------------
//  find_deep - do a deep search for a symbol,
//  looking in the parent if needed
//-------------------------------------------------

symbol_entry *symbol_table::find_deep(const char *symbol, bool skip_srcdbg /* = false */)
{
	// walk up the table hierarchy to find the owner
	for (symbol_table *symtable = this; symtable != nullptr; symtable = symtable->m_parent)
	{
		if (skip_srcdbg && (symtable->type() == SRCDBG_LOCALS || symtable->type() == SRCDBG_GLOBALS))
			continue;
		symbol_entry *entry = symtable->find(symbol);
		if (entry != nullptr && entry->is_in_scope())
			return entry;
	}
	return nullptr;
}


//-------------------------------------------------
//  value - return the value of a symbol
//-------------------------------------------------

u64 symbol_table::value(const char *symbol)
{
	symbol_entry *entry = find_deep(symbol);
	return (entry != nullptr) ? entry->value() : 0;
}


//-------------------------------------------------
//  set_value - set the value of a symbol
//-------------------------------------------------

void symbol_table::set_value(const char *symbol, u64 value)
{
	symbol_entry *entry = find_deep(symbol);
	if (entry != nullptr)
		entry->set_value(value);
}



//**************************************************************************
//  EXPRESSION MEMORY HANDLERS
//**************************************************************************

//-------------------------------------------------
//  read_memory - return 1,2,4 or 8 bytes
//  from the specified memory space
//-------------------------------------------------

u64 symbol_table::read_memory(address_space &space, offs_t address, int size, bool apply_translation)
{
	u64 result = ~u64(0) >> (64 - 8*size);

	address_space *tspace = &space;

	if (apply_translation)
	{
		// mask against the logical byte mask
		address &= space.logaddrmask();

		// translate if necessary; if not mapped, return 0xffffffffffffffff
		if (!space.device().memory().translate(space.spacenum(), device_memory_interface::TR_READ, address, tspace))
			return result;
	}

	// otherwise, call the reading function for the translated address
	switch (size)
	{
	case 1:     result = tspace->read_byte(address);              break;
	case 2:     result = tspace->read_word_unaligned(address);    break;
	case 4:     result = tspace->read_dword_unaligned(address);   break;
	case 8:     result = tspace->read_qword_unaligned(address);   break;
	}
	return result;
}


//-------------------------------------------------
//  write_memory - write 1,2,4 or 8 bytes to the
//  specified memory space
//-------------------------------------------------

void symbol_table::write_memory(address_space &space, offs_t address, u64 data, int size, bool apply_translation)
{
	address_space *tspace = &space;

	if (apply_translation)
	{
		// mask against the logical byte mask
		address &= space.logaddrmask();

		// translate if necessary; if not mapped, we're done
		if (!space.device().memory().translate(space.spacenum(), device_memory_interface::TR_WRITE, address, tspace))
			return;
	}

	// otherwise, call the writing function for the translated address
	switch (size)
	{
	case 1:     tspace->write_byte(address, data);            break;
	case 2:     tspace->write_word_unaligned(address, data);  break;
	case 4:     tspace->write_dword_unaligned(address, data); break;
	case 8:     tspace->write_qword_unaligned(address, data); break;
	}

	notify_memory_modified();
}


//-------------------------------------------------
//  expression_get_space - return a space
//  based on a case insensitive tag search
//-------------------------------------------------

expression_error symbol_table::expression_get_space(const char *tag, int &spacenum, device_memory_interface *&memory)
{
	device_t *device = nullptr;
	std::string spacename;
	if (tag)
	{
		// convert to lowercase then lookup the name (tags are enforced to be all lower case)
		auto base = get_device_search(m_machine, m_memintf, tag);
		device = base.first.subdevice(strmakelower(base.second));

		// if that failed, treat the last component as an address space
		if (!device)
		{
			std::string_view t = base.second;
			auto const delimiter = t.find_last_of(":^");
			bool const found = std::string_view::npos != delimiter;
			if (!found || (':' == t[delimiter]))
			{
				spacename = strmakelower(t.substr(found ? (delimiter + 1) : 0));
				t = t.substr(0, !found ? 0 : !delimiter ? 1 : delimiter);
				if (!t.empty())
					device = base.first.subdevice(strmakelower(t));
				else
					device = m_memintf ? &m_memintf->device() : &m_machine.root_device();
			}
		}
	}
	else if (m_memintf)
	{
		device = &m_memintf->device();
	}

	// if still no device, report error
	if (!device)
	{
		memory = nullptr;
		return expression_error::INVALID_MEMORY_NAME;
	}

	// ensure device has memory interface, and check for space if search not required
	if (!device->interface(memory) || (spacename.empty() && (0 <= spacenum) && !memory->has_space(spacenum)))
	{
		memory = nullptr;
		return expression_error::NO_SUCH_MEMORY_SPACE;
	}

	// search not required
	if (spacename.empty() && (0 <= spacenum))
		return expression_error::NONE;

	// find space by name or take first populated space if required
	for (int i = 0; memory->max_space_count() > i; ++i)
	{
		if (memory->has_space(i) && (spacename.empty() || (memory->space(i).name() == spacename)))
		{
			spacenum = i;
			return expression_error::NONE;
		}
	}

	// space not found
	memory = nullptr;
	return expression_error::NO_SUCH_MEMORY_SPACE;
}


//-------------------------------------------------
//  notify_memory_modified - notify that memory
//  has been changed
//-------------------------------------------------

void symbol_table::notify_memory_modified()
{
	// walk up the table hierarchy to find the owner
	for (symbol_table *symtable = this; symtable != nullptr; symtable = symtable->m_parent)
		if (symtable->m_memory_modified)
			symtable->m_memory_modified();
}


//-------------------------------------------------
//  memory_value - read 1,2,4 or 8 bytes at the
//  given offset in the given address space
//-------------------------------------------------

u64 symbol_table::memory_value(const char *name, expression_space spacenum, u32 address, int size, bool disable_se)
{
	device_memory_interface *memory = m_memintf;

	bool logical = true;
	int space = -1;
	switch (spacenum)
	{
	case EXPSPACE_PROGRAM_PHYSICAL:
	case EXPSPACE_DATA_PHYSICAL:
	case EXPSPACE_IO_PHYSICAL:
	case EXPSPACE_OPCODE_PHYSICAL:
		spacenum = expression_space(spacenum - (EXPSPACE_PROGRAM_PHYSICAL - EXPSPACE_PROGRAM_LOGICAL));
		logical = false;
		[[fallthrough]];
	case EXPSPACE_PROGRAM_LOGICAL:
	case EXPSPACE_DATA_LOGICAL:
	case EXPSPACE_IO_LOGICAL:
	case EXPSPACE_OPCODE_LOGICAL:
		space = AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL);
		expression_get_space(name, space, memory);
		if (memory)
		{
			auto dis = m_machine.disable_side_effects(disable_se);
			return read_memory(memory->space(space), address, size, logical);
		}
		break;

	case EXPSPACE_PRGDIRECT:
	case EXPSPACE_OPDIRECT:
		space = (spacenum == EXPSPACE_OPDIRECT) ? AS_OPCODES : AS_PROGRAM;
		expression_get_space(name, space, memory);
		if (memory)
		{
			auto dis = m_machine.disable_side_effects(disable_se);
			return read_program_direct(memory->space(space), (spacenum == EXPSPACE_OPDIRECT) ? 1 : 0, address, size);
		}
		break;

	case EXPSPACE_REGION:
		if (name)
			return read_memory_region(name, address, size);
		break;

	default:
		break;
	}

	return 0;
}


//-------------------------------------------------
//  read_program_direct - read memory directly
//  from an opcode or RAM pointer
//-------------------------------------------------

u64 symbol_table::read_program_direct(address_space &space, int opcode, offs_t address, int size)
{
	u8 *base;

	// adjust the address into a byte address, but not if being called recursively
	if ((opcode & 2) == 0)
		address = space.address_to_byte(address);

	// call ourself recursively until we are byte-sized
	if (size > 1)
	{
		int halfsize = size / 2;

		// read each half, from lower address to upper address
		u64 r0 = read_program_direct(space, opcode | 2, address + 0, halfsize);
		u64 r1 = read_program_direct(space, opcode | 2, address + halfsize, halfsize);

		// assemble based on the target endianness
		if (space.endianness() == ENDIANNESS_LITTLE)
			return r0 | (r1 << (8 * halfsize));
		else
			return r1 | (r0 << (8 * halfsize));
	}

	// handle the byte-sized final requests
	else
	{
		// lowmask specified which address bits are within the databus width
		offs_t lowmask = space.data_width() / 8 - 1;

		// get the base of memory, aligned to the address minus the lowbits
		base = (u8 *)space.get_read_ptr(address & ~lowmask);

		// if we have a valid base, return the appropriate byte
		if (base != nullptr)
		{
			if (space.endianness() == ENDIANNESS_LITTLE)
				return base[BYTE8_XOR_LE(address) & lowmask];
			else
				return base[BYTE8_XOR_BE(address) & lowmask];
		}
	}

	return 0;
}


//-------------------------------------------------
//  read_memory_region - read memory from a
//  memory region
//-------------------------------------------------

u64 symbol_table::read_memory_region(const char *rgntag, offs_t address, int size)
{
	auto search = get_device_search(m_machine, m_memintf, rgntag);
	memory_region *const region = search.first.memregion(search.second);
	u64 result = ~u64(0) >> (64 - 8*size);

	// make sure we get a valid base before proceeding
	if (region)
	{
		// call ourself recursively until we are byte-sized
		if (size > 1)
		{
			int halfsize = size / 2;
			u64 r0, r1;

			// read each half, from lower address to upper address
			r0 = read_memory_region(rgntag, address + 0, halfsize);
			r1 = read_memory_region(rgntag, address + halfsize, halfsize);

			// assemble based on the target endianness
			if (region->endianness() == ENDIANNESS_LITTLE)
				result = r0 | (r1 << (8 * halfsize));
			else
				result = r1 | (r0 << (8 * halfsize));
		}

		// only process if we're within range
		else if (address < region->bytes())
		{
			// lowmask specified which address bits are within the databus width
			u32 lowmask = region->bytewidth() - 1;
			u8 *base = region->base() + (address & ~lowmask);

			// if we have a valid base, return the appropriate byte
			if (region->endianness() == ENDIANNESS_LITTLE)
				result = base[BYTE8_XOR_LE(address) & lowmask];
			else
				result = base[BYTE8_XOR_BE(address) & lowmask];
		}
	}
	return result;
}


//-------------------------------------------------
//  set_memory_value - write 1,2,4 or 8 bytes at
//  the given offset in the given address space
//-------------------------------------------------

void symbol_table::set_memory_value(const char *name, expression_space spacenum, u32 address, int size, u64 data, bool disable_se)
{
	device_memory_interface *memory = m_memintf;

	bool logical = true;
	int space = -1;
	switch (spacenum)
	{
	case EXPSPACE_PROGRAM_PHYSICAL:
	case EXPSPACE_DATA_PHYSICAL:
	case EXPSPACE_IO_PHYSICAL:
	case EXPSPACE_OPCODE_PHYSICAL:
		spacenum = expression_space(spacenum - (EXPSPACE_PROGRAM_PHYSICAL - EXPSPACE_PROGRAM_LOGICAL));
		logical = false;
		[[fallthrough]];
	case EXPSPACE_PROGRAM_LOGICAL:
	case EXPSPACE_DATA_LOGICAL:
	case EXPSPACE_IO_LOGICAL:
	case EXPSPACE_OPCODE_LOGICAL:
		space = AS_PROGRAM + (spacenum - EXPSPACE_PROGRAM_LOGICAL);
		expression_get_space(name, space, memory);
		if (memory)
		{
			auto dis = m_machine.disable_side_effects(disable_se);
			write_memory(memory->space(space), address, data, size, logical);
		}
		break;

	case EXPSPACE_PRGDIRECT:
	case EXPSPACE_OPDIRECT:
		space = (spacenum == EXPSPACE_OPDIRECT) ? AS_OPCODES : AS_PROGRAM;
		expression_get_space(name, space, memory);
		if (memory)
		{
			auto dis = m_machine.disable_side_effects(disable_se);
			write_program_direct(memory->space(space), (spacenum == EXPSPACE_OPDIRECT) ? 1 : 0, address, size, data);
		}
		break;

	case EXPSPACE_REGION:
		if (name)
			write_memory_region(name, address, size, data);
		break;

	default:
		break;
	}
}


//-------------------------------------------------
//  write_program_direct - write memory directly
//  to an opcode or RAM pointer
//-------------------------------------------------

void symbol_table::write_program_direct(address_space &space, int opcode, offs_t address, int size, u64 data)
{
	// adjust the address into a byte address, but not if being called recursively
	if ((opcode & 2) == 0)
		address = space.address_to_byte(address);

	// call ourself recursively until we are byte-sized
	if (size > 1)
	{
		int halfsize = size / 2;

		// break apart based on the target endianness
		u64 halfmask = ~u64(0) >> (64 - 8 * halfsize);
		u64 r0, r1;
		if (space.endianness() == ENDIANNESS_LITTLE)
		{
			r0 = data & halfmask;
			r1 = (data >> (8 * halfsize)) & halfmask;
		}
		else
		{
			r0 = (data >> (8 * halfsize)) & halfmask;
			r1 = data & halfmask;
		}

		// write each half, from lower address to upper address
		write_program_direct(space, opcode | 2, address + 0, halfsize, r0);
		write_program_direct(space, opcode | 2, address + halfsize, halfsize, r1);
	}

	// handle the byte-sized final case
	else
	{
		// lowmask specified which address bits are within the databus width
		offs_t lowmask = space.data_width() / 8 - 1;

		// get the base of memory, aligned to the address minus the lowbits
		u8 *base = (u8 *)space.get_read_ptr(address & ~lowmask);

		// if we have a valid base, write the appropriate byte
		if (base != nullptr)
		{
			if (space.endianness() == ENDIANNESS_LITTLE)
				base[BYTE8_XOR_LE(address) & lowmask] = data;
			else
				base[BYTE8_XOR_BE(address) & lowmask] = data;
			notify_memory_modified();
		}
	}
}


//-------------------------------------------------
//  write_memory_region - write memory to a
//  memory region
//-------------------------------------------------

void symbol_table::write_memory_region(const char *rgntag, offs_t address, int size, u64 data)
{
	auto search = get_device_search(m_machine, m_memintf, rgntag);
	memory_region *const region = search.first.memregion(search.second);

	// make sure we get a valid base before proceeding
	if (region)
	{
		// call ourself recursively until we are byte-sized
		if (size > 1)
		{
			int halfsize = size / 2;

			// break apart based on the target endianness
			u64 halfmask = ~u64(0) >> (64 - 8 * halfsize);
			u64 r0, r1;
			if (region->endianness() == ENDIANNESS_LITTLE)
			{
				r0 = data & halfmask;
				r1 = (data >> (8 * halfsize)) & halfmask;
			}
			else
			{
				r0 = (data >> (8 * halfsize)) & halfmask;
				r1 = data & halfmask;
			}

			// write each half, from lower address to upper address
			write_memory_region(rgntag, address + 0, halfsize, r0);
			write_memory_region(rgntag, address + halfsize, halfsize, r1);
		}

		// only process if we're within range
		else if (address < region->bytes())
		{
			// lowmask specified which address bits are within the databus width
			u32 lowmask = region->bytewidth() - 1;
			u8 *base = region->base() + (address & ~lowmask);

			// if we have a valid base, set the appropriate byte
			if (region->endianness() == ENDIANNESS_LITTLE)
			{
				base[BYTE8_XOR_LE(address) & lowmask] = data;
			}
			else
			{
				base[BYTE8_XOR_BE(address) & lowmask] = data;
			}
			notify_memory_modified();
		}
	}
}


//-------------------------------------------------
//  memory_valid - return true if the given
//  memory name/space/offset combination is valid
//-------------------------------------------------

expression_error::error_code symbol_table::memory_valid(const char *name, expression_space space)
{
	device_memory_interface *memory = m_memintf;

	int spaceno = -1;
	switch (space)
	{
	case EXPSPACE_PROGRAM_LOGICAL:
	case EXPSPACE_DATA_LOGICAL:
	case EXPSPACE_IO_LOGICAL:
	case EXPSPACE_OPCODE_LOGICAL:
		spaceno = AS_PROGRAM + (space - EXPSPACE_PROGRAM_LOGICAL);
		return expression_get_space(name, spaceno, memory);

	case EXPSPACE_PROGRAM_PHYSICAL:
	case EXPSPACE_DATA_PHYSICAL:
	case EXPSPACE_IO_PHYSICAL:
	case EXPSPACE_OPCODE_PHYSICAL:
		spaceno = AS_PROGRAM + (space - EXPSPACE_PROGRAM_PHYSICAL);
		return expression_get_space(name, spaceno, memory);

	case EXPSPACE_PRGDIRECT:
	case EXPSPACE_OPDIRECT:
		spaceno = (space == EXPSPACE_OPDIRECT) ? AS_OPCODES : AS_PROGRAM;
		return expression_get_space(name, spaceno, memory);

	case EXPSPACE_REGION:
		if (!name)
		{
			return expression_error::MISSING_MEMORY_NAME;
		}
		else
		{
			auto search = get_device_search(m_machine, m_memintf, name);
			memory_region *const region = search.first.memregion(search.second);
			if (!region || !region->base())
				return expression_error::INVALID_MEMORY_NAME;
		}
		break;

	default:
		return expression_error::NO_SUCH_MEMORY_SPACE;
	}

	return expression_error::NONE;
}



//**************************************************************************
//  PARSED EXPRESSION
//**************************************************************************

//-------------------------------------------------
//  parsed_expression - constructor
//-------------------------------------------------

parsed_expression::parsed_expression(symbol_table &symtable)
	: m_symtable(symtable)
	, m_default_base(16)
{
}

parsed_expression::parsed_expression(symbol_table &symtable, std::string_view expression, int default_base)
	: m_symtable(symtable)
	, m_default_base(default_base)
{
	assert(default_base == 8 || default_base == 10 || default_base == 16);

	parse(expression);
}


//-------------------------------------------------
//  parsed_expression - copy constructor
//-------------------------------------------------

parsed_expression::parsed_expression(const parsed_expression &src)
	: m_symtable(src.m_symtable)
	, m_default_base(src.m_default_base)
	, m_original_string(src.m_original_string)
{
	if (!m_original_string.empty())
		parse_string_into_tokens();
}


//-------------------------------------------------
//  parse - parse an expression into tokens
//-------------------------------------------------

void parsed_expression::parse(std::string_view expression)
{
	// copy the string and reset our parsing state
	m_original_string.assign(expression);
	m_tokenlist.clear();
	m_stringlist.clear();

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
	m_default_base = src.m_default_base;
	m_original_string.assign(src.m_original_string);
	if (!m_original_string.empty())
		parse_string_into_tokens();
}


//-------------------------------------------------
//  print_tokens - debugging took to print a
//  human readable token representation
//-------------------------------------------------

void parsed_expression::print_tokens()
{
	LOG("----\n");
	for (parse_token &token : m_tokenlist)
	{
		if (token.is_number())
			LOG("NUMBER: %016X\n", token.value());
		else if (token.is_string())
			LOG("STRING: ""%s""\n", token.string());
		else if (token.is_symbol())
			LOG("SYMBOL: %s%s%s\n", token.symbol().name(), token.symbol().is_function() ? "()" : "", token.symbol().is_lval() ? " &" : "");
		else if (token.is_operator())
		{
			switch (token.optype())
			{
			case TVL_LPAREN:        LOG("(\n");                    break;
			case TVL_RPAREN:        LOG(")\n");                    break;
			case TVL_PLUSPLUS:      LOG("++ (unspecified)\n");     break;
			case TVL_MINUSMINUS:    LOG("-- (unspecified)\n");     break;
			case TVL_PREINCREMENT:  LOG("++ (prefix)\n");          break;
			case TVL_PREDECREMENT:  LOG("-- (prefix)\n");          break;
			case TVL_POSTINCREMENT: LOG("++ (postfix)\n");         break;
			case TVL_POSTDECREMENT: LOG("-- (postfix)\n");         break;
			case TVL_COMPLEMENT:    LOG("!\n");                    break;
			case TVL_NOT:           LOG("~\n");                    break;
			case TVL_UPLUS:         LOG("+ (unary)\n");            break;
			case TVL_UMINUS:        LOG("- (unary)\n");            break;
			case TVL_MULTIPLY:      LOG("*\n");                    break;
			case TVL_DIVIDE:        LOG("/\n");                    break;
			case TVL_MODULO:        LOG("%%\n");                   break;
			case TVL_ADD:           LOG("+\n");                    break;
			case TVL_SUBTRACT:      LOG("-\n");                    break;
			case TVL_LSHIFT:        LOG("<<\n");                   break;
			case TVL_RSHIFT:        LOG(">>\n");                   break;
			case TVL_LESS:          LOG("<\n");                    break;
			case TVL_LESSOREQUAL:   LOG("<=\n");                   break;
			case TVL_GREATER:       LOG(">\n");                    break;
			case TVL_GREATEROREQUAL:LOG(">=\n");                   break;
			case TVL_EQUAL:         LOG("==\n");                   break;
			case TVL_NOTEQUAL:      LOG("!=\n");                   break;
			case TVL_BAND:          LOG("&\n");                    break;
			case TVL_BXOR:          LOG("^\n");                    break;
			case TVL_BOR:           LOG("|\n");                    break;
			case TVL_LAND:          LOG("&&\n");                   break;
			case TVL_LOR:           LOG("||\n");                   break;
			case TVL_ASSIGN:        LOG("=\n");                    break;
			case TVL_ASSIGNMULTIPLY:LOG("*=\n");                   break;
			case TVL_ASSIGNDIVIDE:  LOG("/=\n");                   break;
			case TVL_ASSIGNMODULO:  LOG("%%=\n");                  break;
			case TVL_ASSIGNADD:     LOG("+=\n");                   break;
			case TVL_ASSIGNSUBTRACT:LOG("-=\n");                   break;
			case TVL_ASSIGNLSHIFT:  LOG("<<=\n");                  break;
			case TVL_ASSIGNRSHIFT:  LOG(">>=\n");                  break;
			case TVL_ASSIGNBAND:    LOG("&=\n");                   break;
			case TVL_ASSIGNBXOR:    LOG("^=\n");                   break;
			case TVL_ASSIGNBOR:     LOG("|=\n");                   break;
			case TVL_COMMA:         LOG(",\n");                    break;
			case TVL_MEMORYAT:      LOG(token.memory_side_effects() ? "mem!\n" : "mem@\n");break;
			case TVL_EXECUTEFUNC:   LOG("execute\n");              break;
			default:                LOG("INVALID OPERATOR\n");     break;
			}
		}
		else
			LOG("INVALID\n");
	}
	LOG("----\n");
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
		while (string[0] != 0 && isspace(u8(string[0])))
			string++;
		if (string[0] == 0)
			break;

		// initialize the current token object
		m_tokenlist.emplace_back(string - stringstart);
		parse_token &token = m_tokenlist.back();

		// switch off the first character
		switch (tolower(u8(string[0])))
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

			case '`':
				parse_source_file_position(token, string);
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
	std::string original_symbol_name;
	while (1)
	{
		static const char valid[] = "abcdefghijklmnopqrstuvwxyz0123456789_$#.:\\";
		char val = tolower(u8(string[0]));
		if (val == 0 || strchr(valid, val) == nullptr)
			break;
		buffer.append(&val, 1);
		original_symbol_name.append(string, 1);
		string++;
	}

	// check for memory @ and ! operators
	if (string[0] == '@' || string[0] == '!')
	{
		try
		{
			bool disable_se = string[0] == '@';
			parse_memory_operator(token, buffer.c_str(), disable_se);
			string += 1;
			return;
		}
		catch (const expression_error &)
		{
			// Try some other operator instead
		}
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

	switch (buffer[0])
	{
	// if we have a # prefix, we must be a decimal value
	case '#':
		return parse_number(token, buffer.c_str() + 1, 10, expression_error::INVALID_NUMBER);

	// if we have a $ prefix, we are a hex value
	case '$':
		return parse_number(token, buffer.c_str() + 1, 16, expression_error::INVALID_NUMBER);

	case '0':
		switch (buffer[1])
		{
		// if we have an 0x prefix, we must be a hex value
		case 'x':
		case 'X':
			return parse_number(token, buffer.c_str() + 2, 16, expression_error::INVALID_NUMBER);

		// if we have an 0o prefix, we must be an octal value
		case 'o':
		case 'O':
			return parse_number(token, buffer.c_str() + 2, 8, expression_error::INVALID_NUMBER);

		// if we have an 0b prefix, we must be a binary value
		case 'b':
		case 'B':
			try
			{
				return parse_number(token, buffer.c_str() + 2, 2, expression_error::INVALID_NUMBER);
			}
			catch (expression_error const &err)
			{
				// this is really a hack, but 0B1234 could also hex depending on default base
				if (expression_error::INVALID_NUMBER == err && m_default_base == 16)
					return parse_number(token, buffer.c_str(), m_default_base, expression_error::INVALID_NUMBER);
				else
					throw;
			}

		default:
			; // fall through
		}
		[[fallthrough]];

	default:
		// check for a symbol match
		//
		// If the symbol starts with the disambiguation prefix, user explicitly
		// wants a built-in or device symbol, and not a source-level debugging symbol
		bool skip_srcdbg = false;
		u32 symbol_start_offset = 0;
		constexpr char skip_srcdbg_prefix[] = "ns\\";
		if (buffer.compare(0, sizeof(skip_srcdbg_prefix) - 1, skip_srcdbg_prefix) == 0)
		{
			skip_srcdbg = true;
			symbol_start_offset = sizeof(skip_srcdbg_prefix) - 1;
		}

		// Symbols loaded via source-debugging info are case-sensitive, so first try
		// with the original case specified by user
		symbol_entry *symbol = m_symtable.get().find_deep(original_symbol_name.c_str() + symbol_start_offset, skip_srcdbg);
		if (symbol == nullptr)
		{
			// Not found, try again with lower case
			symbol = m_symtable.get().find_deep(buffer.c_str() + symbol_start_offset, skip_srcdbg);
		}
		if (symbol != nullptr)
		{
			token.configure_symbol(*symbol);

			// if this is a function symbol, synthesize an execute function operator
			if (symbol->is_function())
			{
				m_tokenlist.emplace_back(string - stringstart);
				parse_token &newtoken = m_tokenlist.back();
				newtoken.configure_operator(TVL_EXECUTEFUNC, 0);
			}
			return;
		}

		// attempt to parse as a number in the default base
		parse_number(token, buffer.c_str(), m_default_base, expression_error::UNKNOWN_SYMBOL);
	}
}


//-------------------------------------------------
//  parse_number - parse a number using the
//  given base
//-------------------------------------------------

void parsed_expression::parse_number(parse_token &token, const char *string, int base, expression_error::error_code errcode)
{
	// parse the actual value
	u64 value = 0;
	while (*string != 0)
	{
		// look up the number's value, stopping if not valid
		static const char numbers[] = "0123456789abcdef";
		const char *ptr = strchr(numbers, tolower(u8(*string)));
		if (ptr == nullptr)
			break;

		// if outside of the base, we also stop
		int digit = ptr - numbers;
		if (digit >= base)
			break;

		// shift previous digits up and add in new digit
		value = (value * u64(base)) + digit;
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
	u64 value = 0;
	while (string[0] != 0)
	{
		// allow '' to mean a nested single quote
		if (string[0] == '\'')
		{
			if (string[1] != '\'')
				break;
			string++;
		}
		value = (value << 8) | u8(*string++);
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
	token.configure_string(m_stringlist.emplace(m_stringlist.end(), buffer.c_str())->c_str());
}


//-------------------------------------------------
//  parse_source_file_position - parse source
//  file + line number pair into address
//-------------------------------------------------

void parsed_expression::parse_source_file_position(parse_token &token, const char *&string)
{
	// accumulate a copy of the back-quoted source file path
	string++;
	std::string file_path;
	while (string[0] != 0 && string[0] != '`')
	{
		file_path.append(string++, 1);
	}

	// if we didn't find the ending back-quote, report an error
	if (string[0] != '`')
		throw expression_error(expression_error::UNBALANCED_QUOTES, token.offset());
	string++;

	// The rest of the token should be the base 10 line number
	std::string linenum_buffer;
	while (1)
	{
		static const char valid[] = "0123456789";
		char val = u8(string[0]);
		if (val == 0 || strchr(valid, val) == nullptr)
			break;
		linenum_buffer.append(&val, 1);
		string++;
	}
	parse_token linenum_token;
	parse_number(linenum_token, linenum_buffer.c_str(), 10, expression_error::INVALID_NUMBER);

	// Convert file path and line number to an address
	if (symbols().machine().debugger().srcdbg_provider() == nullptr)
	{
		throw expression_error(expression_error::SRCDBG_UNAVAILABLE, token.offset());
	}
	const srcdbg_provider_base & debug_info = *symbols().machine().debugger().srcdbg_provider();
	std::optional<u32> file_index = debug_info.file_path_to_index(file_path.c_str());
	if (!file_index.has_value())
	{
		throw expression_error(expression_error::SRCDBG_FILE_UNAVAILABLE, token.offset());
	}
	std::vector<srcdbg_provider_base::address_range> ranges;
	debug_info.file_line_to_address_ranges(file_index.value(), linenum_token.value(), ranges);
	if (ranges.size() == 0)
	{
		throw expression_error(expression_error::SRCDBG_FILE_LINE_UNAVAILABLE, token.offset());
	}

	// make the token
	token.configure_number(ranges[0].first);
}


//-------------------------------------------------
//  parse_memory_operator - parse the several
//  forms of memory operators
//-------------------------------------------------

void parsed_expression::parse_memory_operator(parse_token &token, const char *string, bool disable_se)
{
	// if there is a '.', it means we have a name
	const char *startstring = string;
	const char *namestring = nullptr;
	const char *dot = strrchr(string, '.');
	if (dot != nullptr)
	{
		namestring = m_stringlist.emplace(m_stringlist.end(), string, dot)->c_str();
		string = dot + 1;
	}

	int length = (int)strlen(string);
	bool physical = false;
	int space = 'p';
	int size;
	if (length == 3)
	{
		// length 3 means logical/physical, then space, then size
		if (string[0] != 'l' && string[0] != 'p')
			throw expression_error(expression_error::INVALID_MEMORY_SPACE, token.offset() + (string - startstring));
		if (string[1] != 'p' && string[1] != 'd' && string[1] != 'i' && string[1] != '3')
			throw expression_error(expression_error::INVALID_MEMORY_SPACE, token.offset() + (string - startstring));
		physical = (string[0] == 'p');
		space = string[1];
		size = string[2];
	}
	else if (length == 2)
	{
		// length 2 means space then size
		space = string[0];
		size = string[1];
	}
	else if (length == 1)
	{
		// length 1 means size
		size = string[0];
	}
	else
	{
		// anything else is invalid
		throw expression_error(expression_error::INVALID_TOKEN, token.offset());
	}

	// convert the space to flags
	expression_space memspace;
	switch (space)
	{
		case 'p':   memspace = physical ? EXPSPACE_PROGRAM_PHYSICAL : EXPSPACE_PROGRAM_LOGICAL; break;
		case 'd':   memspace = physical ? EXPSPACE_DATA_PHYSICAL    : EXPSPACE_DATA_LOGICAL;    break;
		case 'i':   memspace = physical ? EXPSPACE_IO_PHYSICAL      : EXPSPACE_IO_LOGICAL;      break;
		case '3':   memspace = physical ? EXPSPACE_OPCODE_PHYSICAL  : EXPSPACE_OPCODE_LOGICAL;  break;
		case 'r':   memspace = EXPSPACE_PRGDIRECT;                                              break;
		case 'o':   memspace = EXPSPACE_OPDIRECT;                                               break;
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
	expression_error::error_code err = m_symtable.get().memory_valid(namestring, memspace);
	if (err != expression_error::NONE)
		throw expression_error(err, token.offset() + (string - startstring));

	// configure the token
	token.configure_operator(TVL_MEMORYAT, 2).set_memory_size(memsize).set_memory_space(memspace).set_memory_source(namestring).set_memory_side_effects(disable_se);
}


//-------------------------------------------------
//  normalize_operator - resolve operator
//  ambiguities based on neighboring tokens
//-------------------------------------------------

void parsed_expression::normalize_operator(parse_token &thistoken, parse_token *prevtoken, parse_token *nexttoken, const std::list<parse_token> &stack, bool was_rparen)
{
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
			if (prevtoken == nullptr || (!prevtoken->is_symbol() && !prevtoken->is_number() && !was_rparen))
				thistoken.configure_operator(thistoken.is_operator(TVL_ADD) ? TVL_UPLUS : TVL_UMINUS, 2);
			break;

		// Determine if , refers to a function parameter
		case TVL_COMMA:
			for (auto lookback = stack.begin(); lookback != stack.end(); ++lookback)
			{
				const parse_token &peek = *lookback;

				// if we hit an execute function operator, or else a left parenthesis that is
				// already tagged, then tag us as well
				if (peek.is_operator(TVL_EXECUTEFUNC) || (peek.is_operator(TVL_LPAREN) && peek.is_function_separator()))
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
	std::list<parse_token> stack;
	parse_token *prev = nullptr;

	// this flag is used to avoid looking back at a closing parenthesis that was already destroyed
	bool was_rparen = false;

	// loop over all the original tokens
	std::list<parse_token>::iterator next;
	std::list<parse_token> origlist = std::move(m_tokenlist);
	m_tokenlist.clear();
	for (std::list<parse_token>::iterator token = origlist.begin(); token != origlist.end(); token = next)
	{
		// pre-determine our next token
		next = std::next(token);

		// if the character is an operand, append it to the result string
		if (token->is_number() || token->is_symbol() || token->is_string())
		{
			m_tokenlist.splice(m_tokenlist.end(), origlist, token);

			// remember this as the previous token
			prev = &*token;
			was_rparen = false;
		}

		// if this is an operator, process it
		else if (token->is_operator())
		{
			// normalize the operator based on neighbors
			normalize_operator(*token, prev, next != origlist.end() ? &*next : nullptr, stack, was_rparen);
			was_rparen = false;

			// if the token is an opening parenthesis, push it onto the stack.
			if (token->is_operator(TVL_LPAREN))
				stack.splice(stack.begin(), origlist, token);

			// if the token is a closing parenthesis, pop all operators until we
			// reach an opening parenthesis and append them to the result string,
			// discarding the open parenthesis
			else if (token->is_operator(TVL_RPAREN))
			{
				// find our matching opener
				std::list<parse_token>::iterator lparen = std::find_if(stack.begin(), stack.end(),
					[] (const parse_token &token) { return token.is_operator(TVL_LPAREN); }
				);

				// if we didn't find an open paren, it's an error
				if (lparen == stack.end())
					throw expression_error(expression_error::UNBALANCED_PARENS, token->offset());

				// move the stacked operators to the end of the new list
				m_tokenlist.splice(m_tokenlist.end(), stack, stack.begin(), lparen);

				// free ourself and our matching opening parenthesis
				origlist.erase(token);
				stack.erase(lparen);
				was_rparen = true;
			}

			// if the token is an operator, pop operators until we reach an opening parenthesis,
			// an operator of lower precedence, or a right associative symbol of equal precedence.
			// Push the operator onto the stack.
			else
			{
				int our_precedence = token->precedence();

				// loop until we can't peek at the stack anymore
				std::list<parse_token>::iterator peek;
				for (peek = stack.begin(); peek != stack.end(); ++peek)
				{
					// break if any of the above conditions are true
					if (peek->is_operator(TVL_LPAREN))
						break;
					int stack_precedence = peek->precedence();
					if (stack_precedence > our_precedence || (stack_precedence == our_precedence && peek->right_to_left()))
						break;
				}

				// move the stacked operands to the end of the new list
				m_tokenlist.splice(m_tokenlist.end(), stack, stack.begin(), peek);

				// push the new operator
				stack.splice(stack.begin(), origlist, token);
			}

			if (!was_rparen)
				prev = &*token;
		}
	}

	// it is an error to have a left parenthesis still on the stack
	std::list<parse_token>::iterator lparen = std::find_if(stack.begin(), stack.end(),
		[] (const parse_token &token) { return token.is_operator(TVL_LPAREN); }
	);
	if (lparen != stack.end())
		throw expression_error(expression_error::UNBALANCED_PARENS, lparen->offset());

	// pop all remaining tokens
	m_tokenlist.splice(m_tokenlist.end(), stack, stack.begin(), stack.end());
}


//-------------------------------------------------
//  push_token - push a token onto the stack
//-------------------------------------------------

inline void parsed_expression::push_token(parse_token &token)
{
	// check for overflow
	if (m_token_stack.size() >= m_token_stack.max_size())
		throw expression_error(expression_error::STACK_OVERFLOW, token.offset());

	// push
	m_token_stack.push_back(token);
}


//-------------------------------------------------
//  pop_token - pop a token off the stack
//-------------------------------------------------

inline void parsed_expression::pop_token(parse_token &token)
{
	// check for underflow
	if (m_token_stack.empty())
		throw expression_error(expression_error::STACK_UNDERFLOW, token.offset());

	// pop
	token = std::move(m_token_stack.back());
	m_token_stack.pop_back();
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

u64 parsed_expression::execute_tokens()
{
	// reset the token stack
	m_token_stack.clear();

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
	if (!m_token_stack.empty())
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
	: m_type(INVALID),
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

u64 parsed_expression::parse_token::get_lval_value(symbol_table &table)
{
	// get the value of a symbol
	if (is_symbol())
		return m_symbol->value();

	// or get the value from the memory callbacks
	else if (is_memory())
		return table.memory_value(m_string, memory_space(), address(), 1 << memory_size(), memory_side_effects());

	return 0;
}


//-------------------------------------------------
//  set_lval_value - call the setter function
//  for a SYMBOL token
//-------------------------------------------------

inline void parsed_expression::parse_token::set_lval_value(symbol_table &table, u64 value)
{
	// set the value of a symbol
	if (is_symbol())
		m_symbol->set_value(value);

	// or set the value via the memory callbacks
	else if (is_memory())
		table.set_memory_value(m_string, memory_space(), address(), 1 << memory_size(), value, memory_side_effects());
}


//-------------------------------------------------
//  execute_function - handle an execute function
//  operator
//-------------------------------------------------

void parsed_expression::execute_function(parse_token &token)
{
	// pop off all pushed parameters
	u64 funcparams[MAX_FUNCTION_PARAMS];
	symbol_entry *symbol = nullptr;
	int paramcount = 0;
	while (paramcount < MAX_FUNCTION_PARAMS)
	{
		// peek at the next token on the stack
		if (m_token_stack.empty())
			throw expression_error(expression_error::INVALID_PARAM_COUNT, token.offset());
		parse_token &peek = m_token_stack.back();

		// if it is a function symbol, break out of the loop
		if (peek.is_symbol())
		{
			symbol = &peek.symbol();
			if (symbol->is_function())
			{
				m_token_stack.pop_back();
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

	// validate parameters
	function_symbol_entry *function = downcast<function_symbol_entry *>(symbol);
	if (paramcount < function->minparams())
		throw expression_error(expression_error::TOO_FEW_PARAMS, token.offset(), function->minparams());
	if (paramcount > function->maxparams())
		throw expression_error(expression_error::TOO_MANY_PARAMS, token.offset(), function->maxparams());

	// execute the function and push the result
	parse_token result(token.offset());
	result.configure_number(function->execute(paramcount, &funcparams[MAX_FUNCTION_PARAMS - paramcount]));
	push_token(result);
}
