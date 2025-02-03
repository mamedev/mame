// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcuml.h

    Universal machine language for dynamic recompiling CPU cores.

***************************************************************************/
#ifndef MAME_CPU_DRCUML_H
#define MAME_CPU_DRCUML_H

#pragma once

#include "drccache.h"
#include "uml.h"

#include <iostream>
#include <list>
#include <memory>
#include <vector>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these options are passed into drcuml_alloc() and control global behaviors



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// opaque structure describing UML generation state
class drcuml_state;


// an integer register, with low/high parts
union drcuml_ireg
{
#ifdef LSB_FIRST
	struct { u32 l, h; }    w;                      // 32-bit low, high parts of the register
#else
	struct { u32 h, l; }    w;                      // 32-bit low, high parts of the register
#endif
	u64                     d;                      // 64-bit full register
};


// a floating-point register, with low/high parts
union drcuml_freg
{
#ifdef LSB_FIRST
	struct { float l, h; }  s;                      // 32-bit low, high parts of the register
#else
	struct { float h, l; }  s;                      // 32-bit low, high parts of the register
#endif
	double                  d;                      // 64-bit full register
};


// the collected machine state of a system
struct drcuml_machine_state
{
	drcuml_ireg             r[uml::REG_I_COUNT];    // integer registers
	drcuml_freg             f[uml::REG_F_COUNT];    // floating-point registers
	u32                     exp;                    // exception parameter register
	u8                      fmod;                   // fmod (floating-point mode) register
	u8                      flags;                  // flags state
};


// hints and information about the back-end
struct drcbe_info
{
	u8                      direct_iregs;           // number of direct-mapped integer registers
	u8                      direct_fregs;           // number of direct-mapped floating point registers
};


// a drcuml_block describes a basic block of instructions
class drcuml_block
{
public:
	// construction/destruction
	drcuml_block(drcuml_state &drcuml, u32 maxinst);
	drcuml_block(drcuml_block const &) = delete;
	drcuml_block &operator=(drcuml_block const &) = delete;
	~drcuml_block();

	// getters
	bool inuse() const { return m_inuse; }
	u32 maxinst() const { return m_maxinst; }

	// code generation
	void begin();
	void end();
	void abort();

	// instruction appending
	uml::instruction &append();
	template <typename Format, typename... Params> void append_comment(Format &&fmt, Params &&... args);

	// this class is thrown if abort() is called
	class abort_compilation : public emu_exception
	{
		friend class drcuml_block;
		abort_compilation() { }
	};

private:
	// internal helpers
	void optimize();
	void disassemble();
	char const *get_comment_text(uml::instruction const &inst, std::string &comment);

	// internal state
	drcuml_state &                  m_drcuml;   // pointer back to the owning UML
	u32                             m_nextinst; // next instruction to fill in the cache
	u32                             m_maxinst;  // maximum number of instructions
	std::vector<uml::instruction>   m_inst;     // pointer to the instruction list
	bool                            m_inuse;    // this block is in use
};


// interface structure for a back-end
class drcbe_interface
{
public:
	// allow deleting through base pointer
	virtual ~drcbe_interface();

	// required overrides
	virtual void reset() = 0;
	virtual int execute(uml::code_handle &entry) = 0;
	virtual void generate(drcuml_block &block, uml::instruction const *instlist, u32 numinst) = 0;
	virtual bool hash_exists(u32 mode, u32 pc) = 0;
	virtual void get_info(drcbe_info &info) = 0;
	virtual bool logging() const { return false; }

protected:
	// base constructor
	drcbe_interface(drcuml_state &drcuml, drc_cache &cache, device_t &device);

	// internal state
	drcuml_state &                  m_drcuml;      // pointer back to our owner
	drc_cache &                     m_cache;       // pointer to the cache
	device_t &                      m_device;      // CPU device we are associated with
	std::vector<address_space *>    m_space;       // pointers to CPU's address space
	drcuml_machine_state &          m_state;       // state of the machine (in near cache)
};


// structure describing UML generation state
class drcuml_state
{
public:
	// construction/destruction
	drcuml_state(device_t &device, drc_cache &cache, u32 flags, int modes, int addrbits, int ignorebits);
	~drcuml_state();

	// getters
	device_t &device() const { return m_device; }
	drc_cache &cache() const { return m_cache; }

	// reset the state
	void reset();
	int execute(uml::code_handle &entry) { return m_beintf->execute(entry); }

	// code generation
	drcuml_block &begin_block(u32 maxinst);

	// back-end interface
	void get_backend_info(drcbe_info &info) { m_beintf->get_info(info); }
	bool hash_exists(u32 mode, u32 pc) { return m_beintf->hash_exists(mode, pc); }
	void generate(drcuml_block &block, uml::instruction *instructions, u32 count) { m_beintf->generate(block, instructions, count); }

	// handle management
	uml::code_handle *handle_alloc(char const *name);

	// symbol management
	void symbol_add(void *base, u32 length, char const *name);
	char const *symbol_find(void *base, u32 *offset = nullptr);

	// logging
	bool logging() const { return bool(m_umllog); }
	template <typename Format, typename... Params>
	void log_printf(Format &&fmt, Params &&...args)
	{
		log_vprintf(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}
	void log_vprintf(util::format_argument_pack<char> const &args);
	void log_flush() { if (logging()) m_umllog->flush(); }
	bool logging_native() const { return m_beintf->logging(); }

private:
	// symbol class
	class symbol
	{
	public:
		// construction/destruction
		symbol(void *base, u32 length, char const *name)
			: m_base(reinterpret_cast<drccodeptr>(base))
			, m_length(length)
			, m_name(name)
		{ }

		// getters
		bool includes(drccodeptr search) const { return (m_base <= search) && ((m_base + m_length) > search); }
		drccodeptr base() const { return m_base; }
		std::string const &name() const { return m_name; }

	private:
		// internal state
		drccodeptr  m_base;     // base of the symbol
		u32         m_length;   // length of the region covered
		std::string m_name;     // name of the symbol
	};

	// internal state
	device_t &                              m_device;           // CPU device we are associated with
	drc_cache &                             m_cache;            // pointer to the codegen cache
	std::unique_ptr<drcbe_interface> const  m_beintf;           // backend interface pointer
	std::unique_ptr<std::ostream> const     m_umllog;           // handle to the UML logfile
	std::list<drcuml_block>                 m_blocklist;        // list of active blocks
	std::list<uml::code_handle>             m_handlelist;       // list of active handles
	std::list<symbol>                       m_symlist;          // list of symbols
};



//**************************************************************************
//  MEMBER TEMPLATES
//**************************************************************************

//-------------------------------------------------
//  comment - attach a comment to the current
//  output location in the specified block
//-------------------------------------------------

template <typename Format, typename... Params>
inline void drcuml_block::append_comment(Format &&fmt, Params &&... args)
{
	// do the printf
	std::string const temp(util::string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));

	// allocate space in the cache to hold the comment
	char *const comment = reinterpret_cast<char *>(m_drcuml.cache().alloc_temporary(temp.length() + 1));
	if (comment)
	{
		strcpy(comment, temp.c_str());

		// add an instruction with a pointer
		append().comment(comment);
	}
}


#endif // MAME_CPU_DRCUML_H
