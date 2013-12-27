/***************************************************************************

    netlist.h

    Discrete netlist implementation.

****************************************************************************

    Couriersud reserves the right to license the code under a less restrictive
    license going forward.

    Copyright Nicola Salmoria and the MAME team
    All rights reserved.

    Redistribution and use of this code or any derivative works are permitted
    provided that the following conditions are met:

    * Redistributions may not be sold, nor may they be used in a commercial
    product or activity.

    * Redistributions that are modified from the original source must include the
    complete source code, including the source code for all components used by a
    binary built from the modified sources. However, as a special exception, the
    source code distributed need not include anything that is normally distributed
    (in either source or binary form) with the major components (compiler, kernel,
    and so on) of the operating system on which the executable runs, unless that
    component itself accompanies the executable.

    * Redistributions must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.


****************************************************************************/

#ifndef NETLIST_H
#define NETLIST_H

#include "emu.h"
#include "tagmap.h"

#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"

// MAME specific configuration

#define MCFG_NETLIST_ADD(_tag, _setup )                                             \
	MCFG_DEVICE_ADD(_tag, NETLIST, NETLIST_CLOCK)                                   \
	MCFG_NETLIST_SETUP(_setup)

#define MCFG_NETLIST_REPLACE(_tag, _setup)                                          \
	MCFG_DEVICE_REPLACE(_tag, NETLIST, NETLIST_CLOCK)                               \
	MCFG_NETLIST_SETUP(_setup)

#define MCFG_NETLIST_SETUP(_setup)                                                  \
	netlist_mame_device_t::static_set_constructor(*device, NETLIST_NAME(_setup));

#define MCFG_NETLIST_ANALOG_INPUT(_basetag, _tag, _name)                            \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_ANALOG_INPUT, 0)                     \
	netlist_mame_analog_input_t::static_set_name(*device, _name);

#define MCFG_NETLIST_ANALOG_INPUT_MULT_OFFSET(_mult, _offset)                       \
	netlist_mame_analog_input_t::static_set_mult_offset(*device, _mult, _offset);

#define NETLIST_ANALOG_PORT_CHANGED(_base, _tag)                                    \
	PORT_CHANGED_MEMBER(_base ":" _tag, netlist_mame_analog_input_t, input_changed, 0)

#define MCFG_NETLIST_LOGIC_INPUT(_basetag, _tag, _name, _shift, _mask)              \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_LOGIC_INPUT, 0)                      \
	netlist_mame_logic_input_t::static_set_params(*device, _name, _mask, _shift);

#define NETLIST_LOGIC_PORT_CHANGED(_base, _tag)                                     \
	PORT_CHANGED_MEMBER(_base ":" _tag, netlist_mame_logic_input_t, input_changed, 0)

// ----------------------------------------------------------------------------------------
// Extensions to interface netlist with MAME code ....
// ----------------------------------------------------------------------------------------

#define NETLIST_MEMREGION(_name)                                                    \
		netlist.parse((char *)downcast<netlist_mame_t &>(netlist.netlist()).machine().root_device().memregion(_name)->base());

#define NETDEV_ANALOG_CALLBACK(_name, _IN, _class, _member, _tag) \
		{ \
			NETLIB_NAME(analog_callback) *dev = downcast<NETLIB_NAME(analog_callback) *>(netlist.register_dev(NET_NEW(analog_callback), # _name)); \
			netlist_analog_output_delegate d = netlist_analog_output_delegate(& _class :: _member, # _class "::" # _member, _tag, (_class *) 0); \
			dev->register_callback(d); \
		} \
		NET_CONNECT(_name, IN, _IN)

#define NETDEV_ANALOG_CALLBACK_MEMBER(_name) \
	void _name(const double data, const attotime &time)

class netlist_mame_device_t;

class netlist_mame_t : public netlist_base_t
{
public:

	netlist_mame_t(netlist_mame_device_t &parent)
	: netlist_base_t(),
		m_parent(parent)
	{}
	virtual ~netlist_mame_t() { };

	inline running_machine &machine();

	netlist_mame_device_t &parent() { return m_parent; }

protected:

	void vfatalerror(const char *format, va_list ap) const
	{
		emu_fatalerror error(format, ap);
		throw error;
	}

private:
	netlist_mame_device_t &m_parent;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_device_t
// ----------------------------------------------------------------------------------------

class netlist_mame_device_t : public device_t,
								public device_execute_interface,
								public device_state_interface,
								public device_disasm_interface,
								public device_memory_interface
{
public:

	// construction/destruction
	netlist_mame_device_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_device_t() {}

	static void static_set_constructor(device_t &device, void (*setup_func)(netlist_setup_t &));

	netlist_setup_t &setup() { return *m_setup; }
	netlist_mame_t &netlist() { return *m_netlist; }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_pre_save();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const;
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const;

	ATTR_HOT virtual void execute_run();

	// device_disasm_interface overrides
	ATTR_COLD virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	ATTR_COLD virtual UINT32 disasm_max_opcode_bytes() const { return 1; }
	ATTR_COLD virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// device_memory_interface overrides

	address_space_config m_program_config;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		switch (spacenum)
		{
			case AS_PROGRAM: return &m_program_config;
			case AS_IO:      return NULL;
			default:         return NULL;
		}
	}

	virtual void state_string_export(const device_state_entry &entry, astring &string)
	{
		if (entry.index() >= 0)
		{
			if (entry.index() & 1)
				string.format("%10.6f", *((double *) entry.dataptr()));
			else
				string.format("%d", *((netlist_sig_t *) entry.dataptr()));
		}
	}


	netlist_mame_t *m_netlist;
	netlist_setup_t *m_setup;

private:

	void save_state();

	void (*m_setup_func)(netlist_setup_t &);

	int m_icount;
	int m_genPC;

};

inline running_machine &netlist_mame_t::machine()
{
	return m_parent.machine();
}

// ----------------------------------------------------------------------------------------
// netlist_mame_sub_interface
// ----------------------------------------------------------------------------------------

class netlist_mame_sub_interface
{
public:
	// construction/destruction
	netlist_mame_sub_interface(netlist_mame_device_t &obj) : m_object(obj) {}
	virtual ~netlist_mame_sub_interface() { }

	virtual void custom_netlist_additions(netlist_base_t &netlist) { }

	inline netlist_mame_device_t &object() { return m_object; }
private:
	netlist_mame_device_t &m_object;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_analog_input_t
// ----------------------------------------------------------------------------------------

class netlist_mame_analog_input_t : public device_t,
									public netlist_mame_sub_interface
{
public:

	// construction/destruction
	netlist_mame_analog_input_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_analog_input_t() { }

	static void static_set_name(device_t &device, const char *param_name);
	static void static_set_mult_offset(device_t &device, const double mult, const double offset);

	inline void write(const double val) { m_param->setTo(val * m_mult + m_offset); }

	inline DECLARE_INPUT_CHANGED_MEMBER(input_changed)
	{
		if (m_auto_port)
			write(((double) newval - (double) field.minval())/((double) (field.maxval()-field.minval()) ) );
		else
			write(newval);
	}
	inline DECLARE_WRITE_LINE_MEMBER(write_line)       { write(state);  }
	inline DECLARE_WRITE8_MEMBER(write8)               { write(data);   }
	inline DECLARE_WRITE16_MEMBER(write16)             { write(data);   }
	inline DECLARE_WRITE32_MEMBER(write32)             { write(data);   }
	inline DECLARE_WRITE64_MEMBER(write64)             { write(data);   }

protected:
	// device-level overrides
	virtual void device_start();

private:
	netlist_param_double_t *m_param;
	double m_offset;
	double m_mult;
	bool   m_auto_port;
	pstring m_param_name;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_logic_input_t
// ----------------------------------------------------------------------------------------

class netlist_mame_logic_input_t :  public device_t,
									public netlist_mame_sub_interface
{
public:

	// construction/destruction
	netlist_mame_logic_input_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_logic_input_t() { }

	static void static_set_params(device_t &device, const char *param_name, const UINT32 mask, const UINT32 shift);

	inline void write(const UINT32 val) { m_param->setTo((val >> m_shift) & m_mask); }

	inline DECLARE_INPUT_CHANGED_MEMBER(input_changed) { write(newval); }
	DECLARE_WRITE_LINE_MEMBER(write_line)       { write(state);  }
	DECLARE_WRITE8_MEMBER(write8)               { write(data);   }
	DECLARE_WRITE16_MEMBER(write16)             { write(data);   }
	DECLARE_WRITE32_MEMBER(write32)             { write(data);   }
	DECLARE_WRITE64_MEMBER(write64)             { write(data);   }

protected:
	// device-level overrides
	virtual void device_start();

private:
	netlist_param_int_t *m_param;
	UINT32 m_mask;
	UINT32 m_shift;
	pstring m_param_name;
};

// ----------------------------------------------------------------------------------------
// netdev_callback
// ----------------------------------------------------------------------------------------

typedef device_delegate<void (const double, const attotime &)> netlist_analog_output_delegate;

class NETLIB_NAME(analog_callback) : public netlist_device_t
{
public:
	NETLIB_NAME(analog_callback)()
		: netlist_device_t() { }

	ATTR_COLD void start()
	{
		register_input("IN", m_in);
		m_callback.bind_relative_to(downcast<netlist_mame_t &>(netlist()).machine().root_device());
	}

	ATTR_COLD void register_callback(netlist_analog_output_delegate callback)
	{
		m_callback = callback;
	}

	ATTR_HOT void update()
	{
		// FIXME: Remove after device cleanup
		if (!m_callback.isnull())
			m_callback(INPANALOG(m_in), downcast<netlist_mame_t &>(netlist()).parent().local_time());
	}

private:
	netlist_analog_input_t m_in;
	netlist_analog_output_delegate m_callback;
};

class NETLIB_NAME(sound) : public netlist_device_t
{
public:
	NETLIB_NAME(sound)()
		: netlist_device_t() { }

	static const int BUFSIZE = 2048;

	ATTR_COLD void start()
	{
		register_input("IN", m_in);
		m_cur = 0;
		m_last_pos = 0;
		m_last_buffer = netlist_time::zero;
		m_sample = netlist_time::zero;  // FIXME: divide by zero
		}

	ATTR_HOT void sound_update()
	{
		netlist_time current = netlist().time();
		int pos = (current - m_last_buffer) / m_sample;
		if (pos >= BUFSIZE)
			netlist().xfatalerror("sound %s: exceeded BUFSIZE\n", name().cstr());
		while (m_last_pos < pos )
		{
			m_buffer[m_last_pos++] = m_cur;
		}
	}

	ATTR_HOT void update()
	{
		double val = INPANALOG(m_in);
		sound_update();
		m_cur = val;
	}

private:
	netlist_analog_input_t m_in;
	netlist_time m_sample;
	double m_cur;
	int m_last_pos;
	netlist_time m_last_buffer;
	stream_sample_t m_buffer[BUFSIZE];
};


// device type definition
extern const device_type NETLIST;
extern const device_type NETLIST_ANALOG_INPUT;
extern const device_type NETLIST_LOGIC_INPUT;

#endif
