// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    netlist.h

    Discrete netlist implementation.

****************************************************************************/

#ifndef MAME_MACHINE_NETLIST_H
#define MAME_MACHINE_NETLIST_H

#include <functional>

#include "../../lib/netlist/nltypes.h"

class nld_sound_out;
class nld_sound_in;

namespace netlist {
	class setup_t;
	class netlist_state_t;
	class nlparse_t;
	template <typename T>
	class param_num_t;
	class param_ptr_t;
}


// MAME specific configuration


#define MCFG_NETLIST_SETUP(_setup)                                                  \
	downcast<netlist_mame_device &>(*device).set_constructor(NETLIST_NAME(_setup));

#define MCFG_NETLIST_SETUP_MEMBER(_obj, _setup)                                \
	downcast<netlist_mame_device &>(*device).set_constructor(_obj, _setup);

#define MCFG_NETLIST_ANALOG_INPUT(_basetag, _tag, _name)                            \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_ANALOG_INPUT, 0)                     \
	downcast<netlist_mame_analog_input_device &>(*device).set_name(_name);

#define MCFG_NETLIST_ANALOG_MULT_OFFSET(_mult, _offset)                             \
	dynamic_cast<netlist_mame_sub_interface &>(*device).set_mult_offset(_mult, _offset);

#define MCFG_NETLIST_ANALOG_OUTPUT(_basetag, _tag, _IN, _class, _member, _class_tag) \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_ANALOG_OUTPUT, 0)                    \
	downcast<netlist_mame_analog_output_device &>(*device).set_params(_IN,              \
				netlist_mame_analog_output_device::output_delegate(& _class :: _member, \
						# _class "::" # _member, _class_tag, (_class *)nullptr)   );

#define MCFG_NETLIST_LOGIC_OUTPUT(_basetag, _tag, _IN, _class, _member, _class_tag) \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_LOGIC_OUTPUT, 0)                    \
	downcast<netlist_mame_logic_output_device &>(*device).set_params(_IN,              \
				netlist_mame_logic_output_device::output_delegate(& _class :: _member, \
						# _class "::" # _member, _class_tag, (_class *)nullptr)   );

#define MCFG_NETLIST_LOGIC_INPUT(_basetag, _tag, _name, _shift)             \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_LOGIC_INPUT, 0)              \
	downcast<netlist_mame_logic_input_device &>(*device).set_params(_name, _shift);

#define MCFG_NETLIST_INT_INPUT(_basetag, _tag, _name, _shift, _mask)        \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_INT_INPUT, 0)                \
	downcast<netlist_mame_int_input_device &>(*device).set_params(_name, _mask, _shift);

#define MCFG_NETLIST_RAM_POINTER(_basetag, _tag, _name) \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_RAM_POINTER, 0) \
	downcast<netlist_mame_ram_pointer_device &>(*device).set_params(_name ".m_RAM");

#define MCFG_NETLIST_STREAM_INPUT(_basetag, _chan, _name)                           \
	MCFG_DEVICE_ADD(_basetag ":cin" # _chan, NETLIST_STREAM_INPUT, 0)               \
	downcast<netlist_mame_stream_input_device &>(*device).set_params(_chan, _name);

#define MCFG_NETLIST_STREAM_OUTPUT(_basetag, _chan, _name)                          \
	MCFG_DEVICE_ADD(_basetag ":cout" # _chan, NETLIST_STREAM_OUTPUT, 0)             \
	downcast<netlist_mame_stream_output_device &>(*device).set_params(_chan, _name);

#define NETLIST_LOGIC_PORT_CHANGED(_base, _tag)                                     \
	PORT_CHANGED_MEMBER(_base ":" _tag, netlist_mame_logic_input_device, input_changed, 0)

#define NETLIST_INT_PORT_CHANGED(_base, _tag)                                     \
	PORT_CHANGED_MEMBER(_base ":" _tag, netlist_mame_logic_input_device, input_changed, 0)

#define NETLIST_ANALOG_PORT_CHANGED(_base, _tag)                                    \
	PORT_CHANGED_MEMBER(_base ":" _tag, netlist_mame_analog_input_device, input_changed, 0)

/* This macro can only be called from device member */

#define MEMREGION_SOURCE(_name) \
		netlist_mame_device::register_memregion_source(setup, *this,  _name);

#define NETDEV_ANALOG_CALLBACK_MEMBER(_name) \
	void _name(const double data, const attotime &time)

#define NETDEV_LOGIC_CALLBACK_MEMBER(_name) \
	void _name(const int data, const attotime &time)



// ----------------------------------------------------------------------------------------
// netlist_mame_device
// ----------------------------------------------------------------------------------------

class netlist_mame_device : public device_t
{
public:
	class netlist_mame_t;
	class netlist_mame_callbacks_t;

	using func_type = std::function<void(netlist::nlparse_t &)>;

	// construction/destruction
	netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~netlist_mame_device();

	void set_constructor(void (*setup_func)(netlist::nlparse_t &))
	{
		m_setup_func = func_type(setup_func);
	}

	template <typename T, typename F>
	void set_constructor(T *obj, F && f)
	{
		m_setup_func = std::move(std::bind(std::forward<F>(f), obj, std::placeholders::_1));
	}

	ATTR_HOT inline netlist::setup_t &setup();
	ATTR_HOT inline netlist_mame_t &netlist() { return *m_netlist; }

	ATTR_HOT void update_icount(netlist::netlist_time time);
	ATTR_HOT void check_mame_abort_slice();

	static void register_memregion_source(netlist::nlparse_t &setup, device_t &dev, const char *name);

	int m_icount;

protected:
	netlist_mame_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// Custom to netlist ...
	virtual void nl_register_devices() { }

	// device_t overrides
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_pre_save() override;
	virtual void device_clock_changed() override;

	netlist::netlist_time m_div;

private:
	void save_state();

	/* timing support here - so sound can hijack it ... */
	netlist::netlist_time        m_rem;
	netlist::netlist_time        m_old;

	netlist::pool_owned_ptr<netlist_mame_t> m_netlist;

	func_type m_setup_func;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_cpu_device
// ----------------------------------------------------------------------------------------

class netlist_mame_cpu_device;

class netlist_disassembler : public util::disasm_interface
{
public:
	netlist_disassembler(netlist_mame_cpu_device *dev);
	virtual ~netlist_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	netlist_mame_cpu_device *m_dev;
};

class netlist_mame_cpu_device : public netlist_mame_device,
								public device_execute_interface,
								public device_state_interface,
								public device_disasm_interface,
								public device_memory_interface
{
public:
	// construction/destruction
	netlist_mame_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	offs_t genPC() const { return m_genPC; }

protected:
	// netlist_mame_device
	virtual void nl_register_devices() override;

	// device_t overrides
	virtual void device_start() override;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override;

	ATTR_HOT virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	//  device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	address_space_config m_program_config;

private:
	offs_t m_genPC;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_sound_device
// ----------------------------------------------------------------------------------------

class netlist_mame_sound_device : public netlist_mame_device,
								  public device_sound_interface
{
public:
	// construction/destruction
	netlist_mame_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	inline sound_stream *get_stream() { return m_stream; }


	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

protected:
	// netlist_mame_device
	virtual void nl_register_devices() override;

	// device_t overrides
	virtual void device_start() override;

private:
	std::map<int, nld_sound_out *> m_out;
	nld_sound_in *m_in;
	sound_stream *m_stream;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_sub_interface
// ----------------------------------------------------------------------------------------

class netlist_mame_sub_interface
{
public:
	// construction/destruction
	netlist_mame_sub_interface(device_t &aowner)
		: m_offset(0.0), m_mult(1.0)
		, m_owner(dynamic_cast<netlist_mame_device *>(&aowner))
		, m_sound(dynamic_cast<netlist_mame_sound_device *>(&aowner))
	{
	}

	virtual void custom_netlist_additions(netlist::netlist_state_t &nlstate) { }
	virtual void pre_parse_action(netlist::netlist_state_t &nlstate) { }

	inline netlist_mame_device &nl_owner() const { return *m_owner; }

	inline void update_to_current_time()
	{
		if (m_sound != nullptr)
			m_sound->get_stream()->update();
	}

	void set_mult_offset(const double mult, const double offset);

protected:
	double m_offset;
	double m_mult;

private:
	netlist_mame_device *const m_owner;
	netlist_mame_sound_device *const m_sound;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_analog_input_device
// ----------------------------------------------------------------------------------------

class netlist_mame_analog_input_device : public device_t, public netlist_mame_sub_interface
{
public:

	// construction/destruction
	netlist_mame_analog_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_name(const char *param_name) { m_param_name = param_name; }

	void write(const double val);

	inline DECLARE_INPUT_CHANGED_MEMBER(input_changed)
	{
		if (m_auto_port)
			write((double(newval) - double(field.minval())) / double(field.maxval() - field.minval()));
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
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	netlist::param_num_t<double> *m_param;
	bool   m_auto_port;
	const char *m_param_name;
	double m_value_for_device_timer;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_analog_output_device
// ----------------------------------------------------------------------------------------

class netlist_mame_analog_output_device : public device_t, public netlist_mame_sub_interface
{
public:
	typedef device_delegate<void (const double, const attotime &)> output_delegate;

	// construction/destruction
	netlist_mame_analog_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_params(const char *in_name, output_delegate &&adelegate);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void custom_netlist_additions(netlist::netlist_state_t &nlstate) override;

private:
	const char *m_in;
	output_delegate m_delegate;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_logic_output_device
// ----------------------------------------------------------------------------------------

class netlist_mame_logic_output_device : public device_t, public netlist_mame_sub_interface
{
public:
	typedef device_delegate<void(const int, const attotime &)> output_delegate;

	// construction/destruction
	netlist_mame_logic_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_params(const char *in_name, output_delegate &&adelegate);
	template <class FunctionClass> void set_params(const char *in_name, void (FunctionClass::*callback)(const int, const attotime &), const char *name)
	{
		set_params(in_name, output_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass> void set_params(const char *in_name, const char *devname, void (FunctionClass::*callback)(const int, const attotime &), const char *name)
	{
		set_params(in_name, output_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void custom_netlist_additions(netlist::netlist_state_t &nlstate) override;

private:
	const char *m_in;
	output_delegate m_delegate;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_int_input_device
// ----------------------------------------------------------------------------------------

class netlist_mame_int_input_device : public device_t, public netlist_mame_sub_interface
{
public:
	// construction/destruction
	netlist_mame_int_input_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *param_name, const uint32_t shift,
		const uint32_t mask)
		: netlist_mame_int_input_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_params(param_name, mask, shift);
	}
	netlist_mame_int_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_params(const char *param_name, const uint32_t mask, const uint32_t shift);

	void write(const uint32_t val);

	inline DECLARE_INPUT_CHANGED_MEMBER(input_changed) { write(newval); }
	DECLARE_WRITE_LINE_MEMBER(write_line)       { write(state);  }
	DECLARE_WRITE8_MEMBER(write8)               { write(data);   }
	DECLARE_WRITE16_MEMBER(write16)             { write(data);   }
	DECLARE_WRITE32_MEMBER(write32)             { write(data);   }
	DECLARE_WRITE64_MEMBER(write64)             { write(data);   }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	netlist::param_num_t<int> *m_param;
	uint32_t m_mask;
	uint32_t m_shift;
	const char *m_param_name;
};


// ----------------------------------------------------------------------------------------
// netlist_mame_logic_input_device
// ----------------------------------------------------------------------------------------

class netlist_mame_logic_input_device : public device_t, public netlist_mame_sub_interface
{
public:
	// construction/destruction
	netlist_mame_logic_input_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *param_name, const uint32_t shift)
		: netlist_mame_logic_input_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_params(param_name, shift);
	}
	netlist_mame_logic_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_params(const char *param_name, const uint32_t shift);

	void write(const uint32_t val);

	inline DECLARE_INPUT_CHANGED_MEMBER(input_changed) { write(newval); }
	DECLARE_WRITE_LINE_MEMBER(write_line)       { write(state);  }
	DECLARE_WRITE8_MEMBER(write8)               { write(data);   }
	DECLARE_WRITE16_MEMBER(write16)             { write(data);   }
	DECLARE_WRITE32_MEMBER(write32)             { write(data);   }
	DECLARE_WRITE64_MEMBER(write64)             { write(data);   }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	netlist::param_num_t<bool> *m_param;
	uint32_t m_shift;
	const char *m_param_name;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_ram_pointer_device
// ----------------------------------------------------------------------------------------

class netlist_mame_ram_pointer_device : public device_t, public netlist_mame_sub_interface
{
public:
	// construction/destruction
	netlist_mame_ram_pointer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t* ptr() const { return m_data; }

	void set_params(const char *param_name);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	netlist::param_ptr_t *m_param;
	const char *m_param_name;
	uint8_t* m_data;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_input_device
// ----------------------------------------------------------------------------------------

class netlist_mame_stream_input_device : public device_t, public netlist_mame_sub_interface
{
public:
	// construction/destruction
	netlist_mame_stream_input_device(const machine_config &mconfig, const char *tag, device_t *owner, int channel, const char *param_name)
		: netlist_mame_stream_input_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_params(channel, param_name);
	}
	netlist_mame_stream_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_params(int channel, const char *param_name);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void custom_netlist_additions(netlist::netlist_state_t &nlstate) override;
private:
	uint32_t m_channel;
	const char *m_param_name;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_output_device
// ----------------------------------------------------------------------------------------

class netlist_mame_stream_output_device : public device_t, public netlist_mame_sub_interface
{
public:
	// construction/destruction
	netlist_mame_stream_output_device(const machine_config &mconfig, const char *tag, device_t *owner, int channel, const char *out_name)
		: netlist_mame_stream_output_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_params(channel, out_name);
	}
	netlist_mame_stream_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_params(int channel, const char *out_name);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void custom_netlist_additions(netlist::netlist_state_t &nlstate) override;

private:
	uint32_t m_channel;
	const char *m_out_name;
};


// device type definition
DECLARE_DEVICE_TYPE(NETLIST_CORE,          netlist_mame_device)
DECLARE_DEVICE_TYPE(NETLIST_CPU,           netlist_mame_cpu_device)
DECLARE_DEVICE_TYPE(NETLIST_SOUND,         netlist_mame_sound_device)
DECLARE_DEVICE_TYPE(NETLIST_ANALOG_INPUT,  netlist_mame_analog_input_device)
DECLARE_DEVICE_TYPE(NETLIST_LOGIC_INPUT,   netlist_mame_logic_input_device)
DECLARE_DEVICE_TYPE(NETLIST_INT_INPUT,     netlist_mame_int_input_device)
DECLARE_DEVICE_TYPE(NETLIST_RAM_POINTER,   netlist_mame_ram_pointer_device)
DECLARE_DEVICE_TYPE(NETLIST_LOGIC_OUTPUT,  netlist_mame_logic_output_device)
DECLARE_DEVICE_TYPE(NETLIST_ANALOG_OUTPUT, netlist_mame_analog_output_device)
DECLARE_DEVICE_TYPE(NETLIST_STREAM_INPUT,  netlist_mame_stream_input_device)
DECLARE_DEVICE_TYPE(NETLIST_STREAM_OUTPUT, netlist_mame_stream_output_device)

#endif // MAME_MACHINE_NETLIST_H
