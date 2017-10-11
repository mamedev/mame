// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devcb.h

    Device callback interface helpers.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DEVCB_H
#define MAME_EMU_DEVCB_H


//**************************************************************************
//  MACROS
//**************************************************************************

// wrappers for ioports, constants, and loggers
#define DEVCB_NOOP devcb_base::null_desc()
#define DEVCB_IOPORT(_tag) devcb_base::ioport_desc(_tag)
#define DEVCB_MEMBANK(_tag) devcb_base::membank_desc(_tag)
#define DEVCB_CONSTANT(_value) devcb_base::constant_desc(_value)
#define DEVCB_LOGGER(_string) devcb_base::logger_desc(_string)
#define DEVCB_INPUTLINE(_tag, _line) devcb_base::inputline_desc(_tag, _line)
#define DEVCB_ASSERTLINE(_tag, _line) devcb_base::assertline_desc(_tag, _line)
#define DEVCB_CLEARLINE(_tag, _line) devcb_base::clearline_desc(_tag, _line)
#define DEVCB_HOLDLINE(_tag, _line) devcb_base::holdline_desc(_tag, _line)
#define DEVCB_VCC DEVCB_CONSTANT(1)
#define DEVCB_GND DEVCB_CONSTANT(0)

// wrappers for read callbacks into the owner device
#define DEVCB_READLINE(_class, _func) read_line_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)
#define DEVCB_READ8(_class, _func) read8_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)
#define DEVCB_READ16(_class, _func) read16_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)
#define DEVCB_READ32(_class, _func) read32_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)
#define DEVCB_READ64(_class, _func) read64_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)

// wrappers for read callbacks into any tagged device
#define DEVCB_DEVREADLINE(tag, _class, _func) read_line_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)
#define DEVCB_DEVREAD8(tag, _class, _func) read8_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)
#define DEVCB_DEVREAD16(tag, _class, _func) read16_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)
#define DEVCB_DEVREAD32(tag, _class, _func) read32_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)
#define DEVCB_DEVREAD64(tag, _class, _func) read64_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)

// wrappers for write callbacks into the owner device
#define DEVCB_WRITELINE(_class, _func) write_line_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)
#define DEVCB_WRITE8(_class, _func) write8_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)
#define DEVCB_WRITE16(_class, _func) write16_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)
#define DEVCB_WRITE32(_class, _func) write32_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)
#define DEVCB_WRITE64(_class, _func) write64_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)nullptr)

// wrappers for write callbacks into any tagged device
#define DEVCB_DEVWRITELINE(tag, _class, _func) write_line_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)
#define DEVCB_DEVWRITE8(tag, _class, _func) write8_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)
#define DEVCB_DEVWRITE16(tag, _class, _func) write16_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)
#define DEVCB_DEVWRITE32(tag, _class, _func) write32_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)
#define DEVCB_DEVWRITE64(tag, _class, _func) write64_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)nullptr)

// machine config helpers to add shift, mask, or address space configuration
#define MCFG_DEVCB_RSHIFT(_shift) devcb->set_rshift(_shift);
#define MCFG_DEVCB_MASK(_mask) devcb->set_mask(_mask);
#define MCFG_DEVCB_BIT(_bit) devcb->set_rshift(-(_bit)).set_mask(1ULL << (_bit));
#define MCFG_DEVCB_XOR(_xor) devcb->set_xor(_xor);
#define MCFG_DEVCB_INVERT devcb->set_xor(~u64(0));
#define MCFG_DEVCB_ADDRESS_SPACE(_device, _spacenum) devcb->set_space(_device, _spacenum);

// machine config helpers for chaining callbacks
#define MCFG_DEVCB_CHAIN_INPUT(_desc) devcb = &downcast<devcb_read_base &>(*devcb).chain_alloc().set_callback(DEVCB_##_desc);
#define MCFG_DEVCB_CHAIN_OUTPUT(_desc) devcb = &downcast<devcb_write_base &>(*devcb).chain_alloc().set_callback(DEVCB_##_desc);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// base delegate type for a read_line/write_line
typedef device_delegate<int ()> read_line_delegate;
typedef device_delegate<void (int)> write_line_delegate;


// ======================> devcb_base

class devcb_base
{
protected:
	// enumerate the types of callbacks
	enum callback_type
	{
		CALLBACK_NONE,
		CALLBACK_LINE,
		CALLBACK_8,
		CALLBACK_16,
		CALLBACK_32,
		CALLBACK_64,
		CALLBACK_IOPORT,
		CALLBACK_MEMBANK,
		CALLBACK_LOG,
		CALLBACK_CONSTANT,
		CALLBACK_INPUTLINE,
		CALLBACK_ASSERTLINE,
		CALLBACK_CLEARLINE,
		CALLBACK_HOLDLINE
	};

	// construction/destruction
	devcb_base(device_t &device, u64 defmask);
	virtual ~devcb_base();

public:
	// getters
	bool isnull() const { return (m_type == CALLBACK_NONE); }

	// additional configuration
	devcb_base &set_space(const char *device, int space = 0) { m_space_tag = device; m_space_num = space; return *this; }
	devcb_base &set_rshift(int rshift) { m_rshift = rshift; return *this; }
	devcb_base &set_mask(u64 mask) { m_mask = mask; return *this; }
	devcb_base &set_xor(u64 xorval) { m_xor = xorval; return *this; }

	// construction helper classes
	class null_desc
	{
	public:
		null_desc() { }
	};

	class ioport_desc
	{
	public:
		ioport_desc(const char *tag) { m_tag = tag; }
		const char *m_tag;
	};

	class membank_desc
	{
	public:
		membank_desc(const char *tag) { m_tag = tag; }
		const char *m_tag;
	};

	class constant_desc
	{
	public:
		constant_desc(u64 value) { m_value = value; }
		u64 m_value;
	};

	class logger_desc
	{
	public:
		logger_desc(const char *string) { m_string = string; }
		const char *m_string;
	};

	class inputline_desc
	{
	public:
		inputline_desc(const char *tag, int inputnum) { m_tag = tag; m_inputnum = inputnum; }
		const char *m_tag;
		int m_inputnum;
	};

	class assertline_desc
	{
	public:
		assertline_desc(const char *tag, int inputnum) { m_tag = tag; m_inputnum = inputnum; }
		const char *m_tag;
		int m_inputnum;
	};

	class clearline_desc
	{
	public:
		clearline_desc(const char *tag, int inputnum) { m_tag = tag; m_inputnum = inputnum; }
		const char *m_tag;
		int m_inputnum;
	};

	class holdline_desc
	{
	public:
		holdline_desc(const char *tag, int inputnum) { m_tag = tag; m_inputnum = inputnum; }
		const char *m_tag;
		int m_inputnum;
	};

	// shared callback setters
	devcb_base &set_callback(null_desc null) { reset(CALLBACK_NONE); return *this; }
	devcb_base &set_callback(ioport_desc ioport) { reset(CALLBACK_IOPORT); m_target_tag = ioport.m_tag; return *this; }
	devcb_base &set_callback(membank_desc membank) { reset(CALLBACK_MEMBANK); m_target_tag = membank.m_tag; return *this; }
	devcb_base &set_callback(constant_desc constant) { reset(CALLBACK_CONSTANT); m_target_int = constant.m_value; return *this; }
	devcb_base &set_callback(logger_desc logger) { reset(CALLBACK_LOG); m_target_tag = logger.m_string; return *this; }
	void reset() { reset(CALLBACK_NONE); }

protected:
	// internal helpers
	inline u64 shift_mask(u64 value) const { return ((m_rshift < 0) ? (value << -m_rshift) : (value >> m_rshift)) & m_mask; }
	inline u64 shift_mask_xor(u64 value) const { return (((m_rshift < 0) ? (value << -m_rshift) : (value >> m_rshift)) ^ m_xor) & m_mask; }
	inline u64 unshift_mask(u64 value) const { return (m_rshift < 0) ? ((value & m_mask) >> -m_rshift) : ((value & m_mask) << m_rshift); }
	inline u64 unshift_mask_xor(u64 value) const { return (m_rshift < 0) ? (((value ^ m_xor) & m_mask) >> -m_rshift) : (((value ^ m_xor) & m_mask) << m_rshift); }
	void reset(callback_type type);
	virtual void devcb_reset() = 0;
	void resolve_ioport();
	void resolve_membank();
	void resolve_inputline();
	void resolve_space();

	// the callback target is going to be one of these
	union callback_target
	{
		void *              ptr;
		device_t *          device;
		ioport_port *       ioport;
		memory_bank *       membank;
	};

	// configuration
	device_t &          m_device;               // reference to our owning device
	callback_type       m_type;                 // type of callback registered
	const char *        m_target_tag;           // tag of target object
	u64                 m_target_int;           // integer value of target object
	const char *        m_space_tag;            // tag of address space device
	int    m_space_num;            // address space number of space device

	// derived state
	address_space *     m_space;                // target address space
	callback_target     m_target;               // resolved pointer to target object
	int                 m_rshift;               // right shift to apply to data read
	u64                 m_mask;                 // mask to apply to data read
	const u64           m_defmask;              // default mask
	u64                 m_xor;                  // XOR to apply to data read
};


// ======================> devcb_read_base

class devcb_read_base : public devcb_base
{
protected:
	// construction/destruction
	devcb_read_base(device_t &device, u64 defmask);

public:
	// callback configuration
	using devcb_base::set_callback;
	devcb_base &set_callback(read_line_delegate func) { reset(CALLBACK_LINE); m_readline = func; return *this; }
	devcb_base &set_callback(read8_delegate func) { reset(CALLBACK_8); m_read8 = func; return *this; }
	devcb_base &set_callback(read16_delegate func) { reset(CALLBACK_16); m_read16 = func; return *this; }
	devcb_base &set_callback(read32_delegate func) { reset(CALLBACK_32); m_read32 = func; return *this; }
	devcb_base &set_callback(read64_delegate func) { reset(CALLBACK_64); m_read64 = func; return *this; }
	devcb_read_base &chain_alloc();

	// resolution
	void resolve();
	void resolve_safe(u64 none_constant_value);

protected:
	// internal helpers
	virtual void devcb_reset() override;
	inline u64 read(address_space &space, offs_t offset, u64 mask);

private:
	// adapters
	u64 read_unresolved_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read_line_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read8_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read16_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read32_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read64_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read_ioport_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read_logged_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read_constant_adapter(address_space &space, offs_t offset, u64 mask);

	// configuration
	read_line_delegate  m_readline;             // copy of registered line reader
	read8_delegate      m_read8;                // copy of registered 8-bit reader
	read16_delegate     m_read16;               // copy of registered 16-bit reader
	read32_delegate     m_read32;               // copy of registered 32-bit reader
	read64_delegate     m_read64;               // copy of registered 64-bit reader

	// derived state
	typedef u64 (devcb_read_base::*adapter_func)(address_space &, offs_t, u64);
	adapter_func        m_adapter;              // actual callback to invoke
	std::unique_ptr<devcb_read_base> m_chain;   // next callback for chained input
};


// ======================> devcb_write_base

class devcb_write_base : public devcb_base
{
protected:
	// construction/destruction
	devcb_write_base(device_t &device, u64 defmask);

public:
	// callback configuration
	using devcb_base::set_callback;
	devcb_base &set_callback(write_line_delegate func) { reset(CALLBACK_LINE); m_writeline = func; return *this; }
	devcb_base &set_callback(write8_delegate func) { reset(CALLBACK_8); m_write8 = func; return *this; }
	devcb_base &set_callback(write16_delegate func) { reset(CALLBACK_16); m_write16 = func; return *this; }
	devcb_base &set_callback(write32_delegate func) { reset(CALLBACK_32); m_write32 = func; return *this; }
	devcb_base &set_callback(write64_delegate func) { reset(CALLBACK_64); m_write64 = func; return *this; }
	devcb_base &set_callback(inputline_desc inputline) { reset(CALLBACK_INPUTLINE); m_target_tag = inputline.m_tag; m_target_int = inputline.m_inputnum; return *this; }
	devcb_base &set_callback(assertline_desc inputline) { reset(CALLBACK_ASSERTLINE); m_target_tag = inputline.m_tag; m_target_int = inputline.m_inputnum; return *this; }
	devcb_base &set_callback(clearline_desc inputline) { reset(CALLBACK_CLEARLINE); m_target_tag = inputline.m_tag; m_target_int = inputline.m_inputnum; return *this; }
	devcb_base &set_callback(holdline_desc inputline) { reset(CALLBACK_HOLDLINE); m_target_tag = inputline.m_tag; m_target_int = inputline.m_inputnum; return *this; }
	devcb_write_base &chain_alloc();

	// resolution
	void resolve();
	void resolve_safe();

protected:
	// internal helpers
	virtual void devcb_reset() override;
	inline void write(address_space &space, offs_t offset, u64 data, u64 mask);

private:
	// adapters
	void write_unresolved_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_line_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write8_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write16_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write32_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write64_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_ioport_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_membank_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_logged_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_noop_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_inputline_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_assertline_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_clearline_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_holdline_adapter(address_space &space, offs_t offset, u64 data, u64 mask);

	// configuration
	write_line_delegate m_writeline;            // copy of registered line writer
	write8_delegate     m_write8;               // copy of registered 8-bit writer
	write16_delegate    m_write16;              // copy of registered 16-bit writer
	write32_delegate    m_write32;              // copy of registered 32-bit writer
	write64_delegate    m_write64;              // copy of registered 64-bit writer

	// derived state
	typedef void (devcb_write_base::*adapter_func)(address_space &, offs_t, u64, u64);
	adapter_func        m_adapter;              // actual callback to invoke
	std::unique_ptr<devcb_write_base> m_chain;  // next callback for chained output
};


// ======================> devcb_read_line

class devcb_read_line : public devcb_read_base
{
public:
	devcb_read_line(device_t &device) : devcb_read_base(device, 0xff) { }
	int operator()() { return read(*m_space, 0, 0xffU) & 1; }
	int operator()(address_space &space) { return read((m_space_tag != nullptr) ? *m_space : space, 0, 0xffU) & 1; }
};


// ======================> devcb_read8

class devcb_read8 : public devcb_read_base
{
public:
	devcb_read8(device_t &device) : devcb_read_base(device, 0xff) { }
	u8 operator()(offs_t offset = 0, u8 mask = 0xff) { return read(*m_space, offset, mask) & mask; }
	u8 operator()(address_space &space, offs_t offset = 0, u8 mask = 0xff) { return read((m_space_tag != nullptr) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb_read16

class devcb_read16 : public devcb_read_base
{
public:
	devcb_read16(device_t &device) : devcb_read_base(device, 0xffff) { }
	u16 operator()(offs_t offset = 0, u16 mask = 0xffff) { return read(*m_space, offset, mask) & mask; }
	u16 operator()(address_space &space, offs_t offset = 0, u16 mask = 0xffff) { return read((m_space_tag != nullptr) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb_read32

class devcb_read32 : public devcb_read_base
{
public:
	devcb_read32(device_t &device) : devcb_read_base(device, 0xffffffff) { }
	u32 operator()(offs_t offset = 0, u32 mask = 0xffffffff) { return read(*m_space, offset, mask) & mask; }
	u32 operator()(address_space &space, offs_t offset = 0, u32 mask = 0xffffffff) { return read((m_space_tag != nullptr) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb_read64

class devcb_read64 : public devcb_read_base
{
public:
	devcb_read64(device_t &device) : devcb_read_base(device, 0xffffffffffffffffU) { }
	u64 operator()(offs_t offset = 0, u64 mask = 0xffffffffffffffffU) { return read(*m_space, offset, mask) & mask; }
	u64 operator()(address_space &space, offs_t offset = 0, u64 mask = 0xffffffffffffffffU) { return read((m_space_tag != nullptr) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb_write_line

class devcb_write_line : public devcb_write_base
{
public:
	devcb_write_line(device_t &device) : devcb_write_base(device, 0xff) { }
	void operator()(int state) { write(*m_space, 0, state & 1, 0xffU); }
	void operator()(address_space &space, int state) { write((m_space_tag != nullptr) ? *m_space : space, 0, state & 1, 0xffU); }
};


// ======================> devcb_write8

class devcb_write8 : public devcb_write_base
{
public:
	devcb_write8(device_t &device) : devcb_write_base(device, 0xff) { }
	void operator()(u8 data, u8 mask = 0xff) { write(*m_space, 0, data, mask); }
	void operator()(offs_t offset, u8 data, u8 mask = 0xff) { write(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, u8 data, u8 mask = 0xff) { write((m_space_tag != nullptr) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb_write16

class devcb_write16 : public devcb_write_base
{
public:
	devcb_write16(device_t &device) : devcb_write_base(device, 0xffff) { }
	void operator()(u16 data, u16 mask = 0xffff) { write(*m_space, 0, data, mask); }
	void operator()(offs_t offset, u16 data, u16 mask = 0xffff) { write(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, u16 data, u16 mask = 0xffff) { write((m_space_tag != nullptr) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb_write32

class devcb_write32 : public devcb_write_base
{
public:
	devcb_write32(device_t &device) : devcb_write_base(device, 0xffffffff) { }
	void operator()(u32 data, u32 mask = 0xffffffff) { write(*m_space, 0, data, mask); }
	void operator()(offs_t offset, u32 data, u32 mask = 0xffffffff) { write(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, u32 data, u32 mask = 0xffffffff) { write((m_space_tag != nullptr) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb_write64

class devcb_write64 : public devcb_write_base
{
public:
	devcb_write64(device_t &device) : devcb_write_base(device, 0xffffffffffffffffU) { }
	void operator()(u64 data, u64 mask = 0xffffffffffffffffU) { write(*m_space, 0, data, mask); }
	void operator()(offs_t offset, u64 data, u64 mask = 0xffffffffffffffffU) { write(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, u64 data, u64 mask = 0xffffffffffffffffU) { write((m_space_tag != nullptr) ? *m_space : space, offset, data, mask); }
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  read - generic read callback dispatch
//-------------------------------------------------

inline u64 devcb_read_base::read(address_space &space, offs_t offset, u64 mask)
{
	u64 result = (this->*m_adapter)(space, offset, mask);
	if (m_chain != nullptr)
		result |= m_chain->read(space, offset, mask);
	return result;
}


//-------------------------------------------------
//  write - generic write callback dispatch
//-------------------------------------------------

inline void devcb_write_base::write(address_space &space, offs_t offset, u64 data, u64 mask)
{
	(this->*m_adapter)(space, offset, data, mask);
	if (m_chain != nullptr)
		m_chain->write(space, offset, data, mask);
}


#endif  /* MAME_EMU_DEVCB_H */
