/***************************************************************************

    devcb2.h

    Device callback interface helpers.

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

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVCB2_H__
#define __DEVCB2_H__


//**************************************************************************
//  MACROS
//**************************************************************************

// wrappers for ioports, constants, and loggers
#define DEVCB2_NULL devcb2_base::null_desc()
#define DEVCB2_NOOP devcb2_base::null_desc()
#define DEVCB2_IOPORT(_tag) devcb2_base::ioport_desc(_tag)
#define DEVCB2_CONSTANT(_value) devcb2_base::constant_desc(_value)
#define DEVCB2_LOGGER(_string, _value) devcb2_base::logger_desc(_string, _value)
#define DEVCB2_INPUTLINE(_tag, _line) devcb2_base::inputline_desc(_tag, _line)

// wrappers for read callbacks into the owner device 
#define DEVCB2_READLINE(_class, _func) read_line_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
#define DEVCB2_READ8(_class, _func) read8_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
#define DEVCB2_READ16(_class, _func) read16_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
#define DEVCB2_READ32(_class, _func) read32_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
#define DEVCB2_READ64(_class, _func) read64_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
 
// wrappers for read callbacks into any tagged device 
#define DEVCB2_DEVREADLINE(tag, _class, _func) read_line_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
#define DEVCB2_DEVREAD8(tag, _class, _func) read8_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
#define DEVCB2_DEVREAD16(tag, _class, _func) read16_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
#define DEVCB2_DEVREAD32(tag, _class, _func) read32_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
#define DEVCB2_DEVREAD64(tag, _class, _func) read64_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
 
// wrappers for write callbacks into the owner device 
#define DEVCB2_WRITELINE(_class, _func) write_line_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
#define DEVCB2_WRITE8(_class, _func) write8_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
#define DEVCB2_WRITE16(_class, _func) write16_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
#define DEVCB2_WRITE32(_class, _func) write32_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
#define DEVCB2_WRITE64(_class, _func) write64_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0)
 
// wrappers for write callbacks into any tagged device 
#define DEVCB2_DEVWRITELINE(tag, _class, _func) write_line_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
#define DEVCB2_DEVWRITE8(tag, _class, _func) write8_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
#define DEVCB2_DEVWRITE16(tag, _class, _func) write16_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
#define DEVCB2_DEVWRITE32(tag, _class, _func) write32_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)
#define DEVCB2_DEVWRITE64(tag, _class, _func) write64_delegate(&_class::_func, #_class "::" #_func, tag, (_class *)0)

// machine config helpers to add shift, mask, or address space configuration
#define MCFG_DEVCB_SHIFT(_shift) devcb->set_shift(_shift);
#define MCFG_DEVCB_MASK(_mask) devcb->set_mask(_mask);
#define MCFG_DEVCB_XOR(_xor) devcb->set_xor(_xor);
#define MCFG_DEVCB_INVERT devcb->set_xor(~U64(0));
#define MCFG_DEVCB_ADDRESS_SPACE(_device, _spacenum) devcb->set_space(_device, _spacenum);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// base delegate type for a read_line/write_line
typedef device_delegate<int ()> read_line_delegate;
typedef device_delegate<void (int)> write_line_delegate;


// ======================> devcb2_base

class devcb2_base
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
		CALLBACK_LOG,
		CALLBACK_CONSTANT,
		CALLBACK_INPUTLINE
	};

	// construction/destruction
	devcb2_base(device_t &device, UINT64 defmask);

public:
	// getters
	bool isnull() const { return (m_type == CALLBACK_NONE); }

	// additional configuration
	devcb2_base &set_space(const char *device, address_spacenum space = AS_0) { m_space_tag = device; m_space_num = space; return *this; }
	devcb2_base &set_rshift(int rshift) { m_rshift = rshift; return *this; }
	devcb2_base &set_mask(UINT64 mask) { m_mask = mask; return *this; }
	devcb2_base &set_xor(UINT64 xorval) { m_xor = xorval; return *this; }
	
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
	
	class constant_desc
	{
	public:
		constant_desc(UINT64 value) { m_value = value; }
		UINT64 m_value;
	};
	
	class logger_desc
	{
	public:
		logger_desc(const char *string, UINT64 value = 0) { m_string = string; m_value = value; }
		const char *m_string;
		UINT64 m_value;
	};

	class inputline_desc
	{
	public:
		inputline_desc(const char *tag, int inputnum) { m_tag = tag; m_inputnum = inputnum; }
		const char *m_tag;
		int m_inputnum;
	};

	// shared callback setters
	devcb2_base &set_callback(null_desc null) { reset(CALLBACK_NONE); return *this; }
	devcb2_base &set_callback(ioport_desc ioport) { reset(CALLBACK_IOPORT); m_target_tag = ioport.m_tag; return *this; }
	devcb2_base &set_callback(constant_desc constant) { reset(CALLBACK_CONSTANT); m_target_int = constant.m_value; return *this; }
	devcb2_base &set_callback(logger_desc logger) { reset(CALLBACK_LOG); m_target_int = logger.m_value; m_target_tag = logger.m_string; return *this; }
	devcb2_base &set_callback(inputline_desc inputline) { reset(CALLBACK_INPUTLINE); m_target_tag = inputline.m_tag; m_target_int = inputline.m_inputnum; return *this; }

protected:
	// internal helpers
	inline UINT64 shift_mask_xor(UINT64 value) const { return (((m_rshift < 0) ? (value << -m_rshift) : (value >> m_rshift)) ^ m_xor) & m_mask; }
	inline UINT64 unshift_mask(UINT64 value) const { return (m_rshift < 0) ? ((value & m_mask) >> -m_rshift) : ((value & m_mask) << m_rshift); }
	inline UINT64 unshift_mask_xor(UINT64 value) const { return (m_rshift < 0) ? (((value ^ m_xor) & m_mask) >> -m_rshift) : (((value ^ m_xor) & m_mask) << m_rshift); }
	void reset(callback_type type = CALLBACK_NONE);
	void resolve_ioport();
	void resolve_inputline();
	void resolve_space();
	
	// the callback target is going to be one of these
	union callback_target
	{
		void *				ptr;
		device_t *			device;
		ioport_port *		ioport;
	};
	
	// configuration
	device_t &			m_device;				// reference to our owning device
	callback_type		m_type;					// type of callback registered
	const char *		m_target_tag;			// tag of target object
	UINT64				m_target_int;			// integer value of target object
	const char *		m_space_tag;			// tag of address space device
	address_spacenum	m_space_num;			// address space number of space device

	// derived state
	address_space *		m_space;				// target address space
	callback_target		m_target;				// resolved pointer to target object
	int 				m_rshift;				// right shift to apply to data read
	UINT64 				m_mask;					// mask to apply to data read
	UINT64 				m_xor;					// XOR to apply to data read
};


// ======================> devcb2_read_base

class devcb2_read_base : public devcb2_base
{
protected:
	// construction/destruction
	devcb2_read_base(device_t &device, UINT64 defmask);
	
public:
	// callback configuration
	using devcb2_base::set_callback;
	devcb2_base &set_callback(read_line_delegate func) { reset(CALLBACK_LINE); m_readline = func; return *this; }
	devcb2_base &set_callback(read8_delegate func) { reset(CALLBACK_8); m_read8 = func; return *this; }
	devcb2_base &set_callback(read16_delegate func) { reset(CALLBACK_16); m_read16 = func; return *this; }
	devcb2_base &set_callback(read32_delegate func) { reset(CALLBACK_32); m_read32 = func; return *this; }
	devcb2_base &set_callback(read64_delegate func) { reset(CALLBACK_64); m_read64 = func; return *this; }
	
	// resolution
	void resolve();
	void resolve_safe(UINT64 none_constant_value);

protected:
	// internal helpers
	void reset(callback_type type = CALLBACK_NONE);

	// adapters
	UINT64 read_unresolved_adapter(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read_line_adapter(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read8_adapter(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read16_adapter(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read32_adapter(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read64_adapter(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read_ioport_adapter(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read_logged_adapter(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read_constant_adapter(address_space &space, offs_t offset, UINT64 mask);

	// configuration
	read_line_delegate	m_readline;				// copy of registered line reader
	read8_delegate		m_read8;				// copy of registered 8-bit reader
	read16_delegate		m_read16;				// copy of registered 16-bit reader
	read32_delegate		m_read32;				// copy of registered 32-bit reader
	read64_delegate		m_read64;				// copy of registered 64-bit reader

	// derived state
	typedef UINT64 (devcb2_read_base::*adapter_func)(address_space &, offs_t, UINT64);
	adapter_func		m_adapter;				// actual callback to invoke
};


// ======================> devcb2_write_base

class devcb2_write_base : public devcb2_base
{
protected:
	// construction/destruction
	devcb2_write_base(device_t &device, UINT64 defmask);
	
public:
	// callback configuration
	using devcb2_base::set_callback;
	devcb2_base &set_callback(write_line_delegate func) { reset(CALLBACK_LINE); m_writeline = func; return *this; }
	devcb2_base &set_callback(write8_delegate func) { reset(CALLBACK_8); m_write8 = func; return *this; }
	devcb2_base &set_callback(write16_delegate func) { reset(CALLBACK_16); m_write16 = func; return *this; }
	devcb2_base &set_callback(write32_delegate func) { reset(CALLBACK_32); m_write32 = func; return *this; }
	devcb2_base &set_callback(write64_delegate func) { reset(CALLBACK_64); m_write64 = func; return *this; }

	// resolution
	void resolve();
	void resolve_safe();

protected:
	// internal helpers
	void reset(callback_type type = CALLBACK_NONE);

	// adapters
	void write_unresolved_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write_line_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write8_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write16_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write32_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write64_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write_ioport_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write_logged_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write_noop_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write_inputline_adapter(address_space &space, offs_t offset, UINT64 data, UINT64 mask);

	// configuration
	write_line_delegate	m_writeline;			// copy of registered line writer
	write8_delegate		m_write8;				// copy of registered 8-bit writer
	write16_delegate	m_write16;				// copy of registered 16-bit writer
	write32_delegate	m_write32;				// copy of registered 32-bit writer
	write64_delegate	m_write64;				// copy of registered 64-bit writer

	// derived state
	typedef void (devcb2_write_base::*adapter_func)(address_space &, offs_t, UINT64, UINT64);
	adapter_func		m_adapter;				// actual callback to invoke
};


// ======================> devcb2_read_line

class devcb2_read_line : public devcb2_read_base
{
public:
	devcb2_read_line(device_t &device) : devcb2_read_base(device, 0xff) { }
	int operator()() { return (this->*m_adapter)(*m_space, 0, U64(0xff)) & 1; }
	int operator()(address_space &space) { return (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, 0, U64(0xff)) & 1; }
};


// ======================> devcb2_read8

class devcb2_read8 : public devcb2_read_base
{
public:
	devcb2_read8(device_t &device) : devcb2_read_base(device, 0xff) { }
	UINT8 operator()(offs_t offset = 0, UINT8 mask = 0xff) { return (this->*m_adapter)(*m_space, offset, mask) & mask; }
	UINT8 operator()(address_space &space, offs_t offset = 0, UINT8 mask = 0xff) { return (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb2_read16

class devcb2_read16 : public devcb2_read_base
{
public:
	devcb2_read16(device_t &device) : devcb2_read_base(device, 0xffff) { }
	UINT16 operator()(offs_t offset = 0, UINT16 mask = 0xffff) { return (this->*m_adapter)(*m_space, offset, mask) & mask; }
	UINT16 operator()(address_space &space, offs_t offset = 0, UINT16 mask = 0xffff) { return (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb2_read32

class devcb2_read32 : public devcb2_read_base
{
public:
	devcb2_read32(device_t &device) : devcb2_read_base(device, 0xffffffff) { }
	UINT32 operator()(offs_t offset = 0, UINT32 mask = 0xffffffff) { return (this->*m_adapter)(*m_space, offset, mask) & mask; }
	UINT32 operator()(address_space &space, offs_t offset = 0, UINT32 mask = 0xffffffff) { return (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb2_read64

class devcb2_read64 : public devcb2_read_base
{
public:
	devcb2_read64(device_t &device) : devcb2_read_base(device, U64(0xffffffffffffffff)) { }
	UINT64 operator()(offs_t offset = 0, UINT64 mask = U64(0xffffffffffffffff)) { return (this->*m_adapter)(*m_space, offset, mask) & mask; }
	UINT64 operator()(address_space &space, offs_t offset = 0, UINT64 mask = U64(0xffffffffffffffff)) { return (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb2_write_line

class devcb2_write_line : public devcb2_write_base
{
public:
	devcb2_write_line(device_t &device) : devcb2_write_base(device, 0xff) { }
	void operator()(int state) { (this->*m_adapter)(*m_space, 0, state & 1, U64(0xff)); }
	void operator()(address_space &space, int state) { (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, 0, state & 1, U64(0xff)); }
};


// ======================> devcb2_write8

class devcb2_write8 : public devcb2_write_base
{
public:
	devcb2_write8(device_t &device) : devcb2_write_base(device, 0xff) { }
	void operator()(UINT8 data, UINT8 mask = 0xff) { (this->*m_adapter)(*m_space, 0, data, mask); }
	void operator()(offs_t offset, UINT8 data, UINT8 mask = 0xff) { (this->*m_adapter)(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, UINT8 data, UINT8 mask = 0xff) { (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb2_write16

class devcb2_write16 : public devcb2_write_base
{
public:
	devcb2_write16(device_t &device) : devcb2_write_base(device, 0xffff) { }
	void operator()(UINT16 data, UINT16 mask = 0xffff) { (this->*m_adapter)(*m_space, 0, data, mask); }
	void operator()(offs_t offset, UINT16 data, UINT16 mask = 0xffff) { (this->*m_adapter)(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, UINT16 data, UINT16 mask = 0xffff) { (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb2_write32

class devcb2_write32 : public devcb2_write_base
{
public:
	devcb2_write32(device_t &device) : devcb2_write_base(device, 0xffffffff) { }
	void operator()(UINT32 data, UINT32 mask = 0xffffffff) { (this->*m_adapter)(*m_space, 0, data, mask); }
	void operator()(offs_t offset, UINT32 data, UINT32 mask = 0xffffffff) { (this->*m_adapter)(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, UINT32 data, UINT32 mask = 0xffffffff) { (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb2_write64

class devcb2_write64 : public devcb2_write_base
{
public:
	devcb2_write64(device_t &device) : devcb2_write_base(device, U64(0xffffffffffffffff)) { }
	void operator()(UINT64 data, UINT64 mask = U64(0xffffffffffffffff)) { (this->*m_adapter)(*m_space, 0, data, mask); }
	void operator()(offs_t offset, UINT64 data, UINT64 mask = U64(0xffffffffffffffff)) { (this->*m_adapter)(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, UINT64 data, UINT64 mask = U64(0xffffffffffffffff)) { (this->*m_adapter)((m_space_tag != NULL) ? *m_space : space, offset, data, mask); }
};


#endif	/* __DEVCB2_H__ */
