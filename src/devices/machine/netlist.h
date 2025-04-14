// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    netlist.h

    Discrete netlist implementation.

****************************************************************************/

#ifndef MAME_MACHINE_NETLIST_H
#define MAME_MACHINE_NETLIST_H

#include <functional>
#include <deque>

#include "../../lib/netlist/nltypes.h"

#ifndef NETLIST_CREATE_CSV
#define NETLIST_CREATE_CSV (0)
#endif

class netlist_mame_stream_output_device;
class nld_sound_in;

namespace netlist {
	class setup_t;
	class netlist_t;
	class netlist_state_t;
	class nlparse_t;
	template <typename T>
	class param_num_t;
	class param_ptr_t;
}


// MAME specific configuration

#define NETLIST_LOGIC_PORT_CHANGED(_base, _tag)                                     \
	PORT_CHANGED_MEMBER(_base ":" _tag, FUNC(netlist_mame_logic_input_device::input_changed), 0)

#define NETLIST_INT_PORT_CHANGED(_base, _tag)                                     \
	PORT_CHANGED_MEMBER(_base ":" _tag, FUNC(netlist_mame_logic_input_device::input_changed), 0)

#define NETLIST_ANALOG_PORT_CHANGED(_base, _tag)                                    \
	PORT_CHANGED_MEMBER(_base ":" _tag, FUNC(netlist_mame_analog_input_device::input_changed), 0)

/* This macro can only be called from device member */

#define MEMREGION_SOURCE(_name) \
		netlist_mame_device::register_memregion_source(setup, *this,  _name);

#define NETDEV_ANALOG_CALLBACK_MEMBER(_name) \
	void _name(const double data, const attotime &time)

#define NETDEV_LOGIC_CALLBACK_MEMBER(_name) \
	void _name(const int data, const attotime &time)


// ----------------------------------------------------------------------------------------
// netlist_log_csv
// ----------------------------------------------------------------------------------------

template <int USE>
struct netlist_log_csv
{
private:
	static constexpr int MAX_BUFFER_ENTRIES = 1000;
public:
	void open(running_machine &machine, const std::string &name) { }
	void close() { }
	void log_add(char const* param, double value, bool isfloat) { }
	void log_flush(int count = MAX_BUFFER_ENTRIES) { }

private:
};

template <>
struct netlist_log_csv<1>
{
private:
	static constexpr int MAX_BUFFER_ENTRIES = 1000;
public:
	void open(running_machine &machine, const std::string &name);
	void close();

	void log_add(char const* param, double value, bool isfloat);
	void log_flush(int count = MAX_BUFFER_ENTRIES);
private:
	struct buffer_entry
	{
		attotime time;
		bool isfloat;
		double value;
		char const *string;
	};
	std::deque<buffer_entry> m_buffer;
	FILE* m_csv_file = nullptr;
	running_machine * m_machine;
};

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

	void set_setup_func(func_type &&func) noexcept { m_setup_func = std::move(func); }

	netlist::setup_t &setup();
	netlist_mame_t &netlist() noexcept { return *m_netlist; }

	static void register_memregion_source(netlist::nlparse_t &parser, device_t &dev, const char *name);

	auto &log_csv() { return m_log_csv; }
protected:

	netlist_mame_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// Custom to netlist ...
	virtual void nl_register_devices(netlist::nlparse_t &parser) const { }

	// device_t overrides
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_pre_save() override;
	//virtual void device_clock_changed() override;

	void device_start_common();
	void save_state();

	std::unique_ptr<netlist::netlist_state_t> base_validity_check(validity_checker &valid) const;

private:

	void common_dev_start(netlist::netlist_state_t *lnetlist) const;

	std::unique_ptr<netlist_mame_t> m_netlist;

	func_type m_setup_func;
	bool m_device_reset_called;

	netlist_log_csv<NETLIST_CREATE_CSV> m_log_csv;
};

class netlist_mame_cpu_device : public netlist_mame_device,
								public device_execute_interface,
								public device_state_interface,
								public device_disasm_interface,
								public device_memory_interface
{
public:
	static constexpr const unsigned MDIV_SHIFT = 16;

	// construction/destruction
	netlist_mame_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	~netlist_mame_cpu_device()
	{

	}

	offs_t genPC() const { return m_genPC; }

	netlist_mame_cpu_device & set_source(void (*setup_func)(netlist::nlparse_t &))
	{
		set_setup_func(func_type(setup_func));
		return *this;
	}

	template <typename T, typename F>
	netlist_mame_cpu_device & set_source(T *obj, F && f)
	{
		set_setup_func(std::move(std::bind(std::forward<F>(f), obj, std::placeholders::_1)));
		return *this;
	}

	void update_icount(netlist::netlist_time_ext time) noexcept;
	void check_mame_abort_slice() noexcept;

protected:
	// netlist_mame_device
	virtual void nl_register_devices(netlist::nlparse_t &parser) const override;

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override;

	ATTR_HOT virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	//  device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	address_space_config m_program_config;

private:
	netlist::netlist_time_ext nltime_ext_from_clocks(unsigned c) const noexcept;
	netlist::netlist_time nltime_from_clocks(unsigned c) const noexcept;

	int m_icount;
	netlist::netlist_time_ext    m_div;
	netlist::netlist_time_ext    m_rem;
	netlist::netlist_time_ext    m_old;
	offs_t m_genPC;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_cpu_device
// ----------------------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------------------
// netlist_mame_sound_input_buffer
//
// This is a wrapper device to provide operator[] on an input stream.
// ----------------------------------------------------------------------------------------

class netlist_mame_sound_input_buffer
{
public:
	sound_stream *m_stream = nullptr;
	int m_stream_input = 0;

	netlist_mame_sound_input_buffer() {}

	netlist_mame_sound_input_buffer(sound_stream &stream, int input) : m_stream(&stream), m_stream_input(input) { }

	sound_stream::sample_t operator[](std::size_t index) { return m_stream->get(m_stream_input, index); }
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

	netlist_mame_sound_device & set_source(void (*setup_func)(netlist::nlparse_t &))
	{
		set_setup_func(func_type(setup_func));
		return *this;
	}

	template <typename T, typename F>
	netlist_mame_sound_device & set_source(T *obj, F && f)
	{
		set_setup_func(std::move(std::bind(std::forward<F>(f), obj, std::placeholders::_1)));
		return *this;
	}


	inline sound_stream *get_stream() { return m_stream; }
	void update_to_current_time();

	void register_stream_output(int channel, netlist_mame_stream_output_device *so);

protected:

	// netlist_mame_device
	virtual void nl_register_devices(netlist::nlparse_t &parser) const override;

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual void device_validity_check(validity_checker &valid) const override;
	//virtual void device_reset() override ATTR_COLD;

private:
	std::map<int, netlist_mame_stream_output_device *> m_out;
	std::map<std::size_t, nld_sound_in *> m_in;
	std::vector<netlist_mame_sound_input_buffer> m_inbuffer;
	sound_stream *m_stream;
	attotime m_cur_time;
	uint32_t m_sound_clock;
	attotime m_attotime_per_clock;
	attotime m_last_update_to_current_time;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_sub_interface
// ----------------------------------------------------------------------------------------

class netlist_mame_sub_interface
{
public:
	// construction/destruction
	netlist_mame_sub_interface(device_t &owner)
		: m_offset(0.0), m_mult(1.0)
		, m_owner(dynamic_cast<netlist_mame_device *>(&owner))
		, m_sound(dynamic_cast<netlist_mame_sound_device *>(&owner))
		, m_cpu(dynamic_cast<netlist_mame_cpu_device *>(&owner))
	{
	}

	virtual ~netlist_mame_sub_interface()
	{
	}

	virtual void custom_netlist_additions(netlist::nlparse_t &parser) { }
	virtual void pre_parse_action(netlist::nlparse_t &parser) { }
	virtual void validity_helper(validity_checker &valid,
		netlist::netlist_state_t &nlstate) const { }

	inline netlist_mame_device &nl_owner() const { return *m_owner; }

	inline void update_to_current_time()
	{
		if (m_sound != nullptr)
		{
			m_sound->update_to_current_time();
		}
	}

	void set_mult_offset(const double mult, const double offset);

	netlist_mame_sound_device *sound() { return m_sound;}
	netlist_mame_cpu_device   *cpu()   { return m_cpu;}

protected:
	double m_offset;
	double m_mult;

private:
	netlist_mame_device       *const m_owner;
	netlist_mame_sound_device *const m_sound;
	netlist_mame_cpu_device   *const m_cpu;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_analog_input_device
// ----------------------------------------------------------------------------------------

class netlist_mame_analog_input_device : public device_t, public netlist_mame_sub_interface
{
public:

	// construction/destruction
	netlist_mame_analog_input_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *param_name);

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
	inline void write_line(int state)              { write(state);  }
	inline void write8(uint8_t data)               { write(data);   }
	inline void write16(uint16_t data)             { write(data);   }
	inline void write32(uint32_t data)             { write(data);   }
	inline void write64(uint64_t data)             { write(data);   }

	virtual void validity_helper(validity_checker &valid,
		netlist::netlist_state_t &nlstate) const override;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_callback);

private:
	netlist::param_num_t<netlist::nl_fptype> *m_param;
	bool   m_auto_port;
	const char *m_param_name;
	double m_value_to_sync;
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

	template <typename... T>
	void set_params(const char *in_name, T &&... args)
	{
		m_in = in_name;
		m_delegate.set(std::forward<T>(args)...);
	}

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void pre_parse_action(netlist::nlparse_t &parser) override;
	virtual void custom_netlist_additions(netlist::nlparse_t &parser) override;

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

	template <typename... T> void set_params(const char *in_name, T &&... args)
	{
		m_in = in_name;
		m_delegate.set(std::forward<T>(args)...);
	}

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void pre_parse_action(netlist::nlparse_t &parser) override;
	virtual void custom_netlist_additions(netlist::nlparse_t &parser) override;

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
	void write_line(int state)              { write(state);  }
	void write8(uint8_t data)               { write(data);   }
	void write16(uint16_t data)             { write(data);   }
	void write32(uint32_t data)             { write(data);   }
	void write64(uint64_t data)             { write(data);   }

	virtual void validity_helper(validity_checker &valid, netlist::netlist_state_t &nlstate) const override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_callback);

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
	void write_line(int state)              { write(state);  }
	void write8(uint8_t data)               { write(data);   }
	void write16(uint16_t data)             { write(data);   }
	void write32(uint32_t data)             { write(data);   }
	void write64(uint64_t data)             { write(data);   }

	virtual void validity_helper(validity_checker &valid, netlist::netlist_state_t &nlstate) const override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_callback);

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
	netlist_mame_ram_pointer_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *pname);

	uint8_t* ptr() const { return m_data; }

	void set_params(const char *param_name);

	virtual void validity_helper(validity_checker &valid, netlist::netlist_state_t &nlstate) const override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_callback);

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
	virtual void device_start() override ATTR_COLD;
	virtual void custom_netlist_additions(netlist::nlparse_t &parser) override;
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

	void process(netlist::netlist_time_ext tim, netlist::nl_fptype val);

	void buffer_reset(const netlist::netlist_time_ext &upto)
	{
		m_last_buffer_time = upto;
		m_buffer.clear();
	}

	void sound_update_fill(sound_stream &stream, int output);

	void set_sample_time(netlist::netlist_time t) { m_sample_time = t; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void custom_netlist_additions(netlist::nlparse_t &parser) override;
	virtual void pre_parse_action(netlist::nlparse_t &parser) override;

private:
	uint32_t                     m_channel;
	const char *                 m_out_name;

	std::vector<sound_stream::sample_t> m_buffer;
	double                       m_cur;

	netlist::netlist_time        m_sample_time;
	netlist::netlist_time_ext    m_last_buffer_time;
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
