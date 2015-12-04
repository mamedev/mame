// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    netlist.h

    Discrete netlist implementation.

****************************************************************************/

#ifndef NETLIST_H
#define NETLIST_H

#include "emu.h"
#include "tagmap.h"

#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"

// MAME specific configuration


#define MCFG_NETLIST_SETUP(_setup)                                                  \
	netlist_mame_device_t::static_set_constructor(*device, NETLIST_NAME(_setup));

#define MCFG_NETLIST_ANALOG_INPUT(_basetag, _tag, _name)                            \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_ANALOG_INPUT, 0)                     \
	netlist_mame_analog_input_t::static_set_name(*device, _name);

#define MCFG_NETLIST_ANALOG_MULT_OFFSET(_mult, _offset)                             \
	netlist_mame_sub_interface::static_set_mult_offset(*device, _mult, _offset);

#define MCFG_NETLIST_ANALOG_OUTPUT(_basetag, _tag, _IN, _class, _member, _class_tag) \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_ANALOG_OUTPUT, 0)                    \
	netlist_mame_analog_output_t::static_set_params(*device, _IN,                   \
				netlist_analog_output_delegate(& _class :: _member,                 \
						# _class "::" # _member, _class_tag, (_class *) 0)   );

#define MCFG_NETLIST_LOGIC_INPUT(_basetag, _tag, _name, _shift, _mask)              \
	MCFG_DEVICE_ADD(_basetag ":" _tag, NETLIST_LOGIC_INPUT, 0)                      \
	netlist_mame_logic_input_t::static_set_params(*device, _name, _mask, _shift);

#define MCFG_NETLIST_STREAM_INPUT(_basetag, _chan, _name)                           \
	MCFG_DEVICE_ADD(_basetag ":cin" # _chan, NETLIST_STREAM_INPUT, 0)               \
	netlist_mame_stream_input_t::static_set_params(*device, _chan, _name);

#define MCFG_NETLIST_STREAM_OUTPUT(_basetag, _chan, _name)                          \
	MCFG_DEVICE_ADD(_basetag ":cout" # _chan, NETLIST_STREAM_OUTPUT, 0)             \
	netlist_mame_stream_output_t::static_set_params(*device, _chan, _name);


#define NETLIST_LOGIC_PORT_CHANGED(_base, _tag)                                     \
	PORT_CHANGED_MEMBER(_base ":" _tag, netlist_mame_logic_input_t, input_changed, 0)

#define NETLIST_ANALOG_PORT_CHANGED(_base, _tag)                                    \
	PORT_CHANGED_MEMBER(_base ":" _tag, netlist_mame_analog_input_t, input_changed, 0)


// ----------------------------------------------------------------------------------------
// Extensions to interface netlist with MAME code ....
// ----------------------------------------------------------------------------------------

class netlist_source_memregion_t : public netlist::setup_t::source_t
{
public:
	netlist_source_memregion_t(pstring name)
	: netlist::setup_t::source_t(), m_name(name)
	{
	}

	bool parse(netlist::setup_t &setup, const pstring &name);
private:
	pstring m_name;
};

#define MEMREGION_SOURCE(_name) \
		setup.register_source(palloc(netlist_source_memregion_t(_name)));

#define NETDEV_ANALOG_CALLBACK_MEMBER(_name) \
	void _name(const double data, const attotime &time)

class netlist_mame_device_t;

class netlist_mame_t : public netlist::netlist_t
{
public:

	netlist_mame_t(netlist_mame_device_t &parent)
	: netlist::netlist_t(),
		m_parent(parent)
	{}
	virtual ~netlist_mame_t() { };

	inline running_machine &machine();

	netlist_mame_device_t &parent() { return m_parent; }

protected:

	void vlog(const plog_level &l, const pstring &ls) const;

private:
	netlist_mame_device_t &m_parent;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_device_t
// ----------------------------------------------------------------------------------------

class netlist_mame_device_t : public device_t
{
public:

	// construction/destruction
	netlist_mame_device_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	netlist_mame_device_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *file);
	virtual ~netlist_mame_device_t() { pstring::resetmem(); }

	static void static_set_constructor(device_t &device, void (*setup_func)(netlist::setup_t &));

	ATTR_HOT inline netlist::setup_t &setup() { return *m_setup; }
	ATTR_HOT inline netlist_mame_t &netlist() { return *m_netlist; }

	ATTR_HOT inline netlist::netlist_time last_time_update() { return m_old; }
	ATTR_HOT void update_time_x();
	ATTR_HOT void check_mame_abort_slice();

	int m_icount;

protected:
	// Custom to netlist ...

	virtual void nl_register_devices() { };

	// device_t overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_pre_save();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	//virtual void device_debug_setup();
	virtual void device_clock_changed();

	netlist::netlist_time m_div;

private:
	void save_state();

	/* timing support here - so sound can hijack it ... */
	netlist::netlist_time        m_rem;
	netlist::netlist_time        m_old;

	netlist_mame_t *    m_netlist;
	netlist::setup_t *   m_setup;

	void (*m_setup_func)(netlist::setup_t &);
};

inline running_machine &netlist_mame_t::machine()
{
	return m_parent.machine();
}

// ----------------------------------------------------------------------------------------
// netlist_mame_cpu_device_t
// ----------------------------------------------------------------------------------------

class netlist_mame_cpu_device_t : public netlist_mame_device_t,
									public device_execute_interface,
									public device_state_interface,
									public device_disasm_interface,
									public device_memory_interface
{
public:

	// construction/destruction
	netlist_mame_cpu_device_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_cpu_device_t() {}

	static void static_set_constructor(device_t &device, void (*setup_func)(netlist::setup_t &));

protected:
	// netlist_mame_device_t
	virtual void nl_register_devices();

	// device_t overrides

	//virtual void device_config_complete();
	virtual void device_start();
	//virtual void device_stop();
	//virtual void device_reset();
	//virtual void device_post_load();
	//virtual void device_pre_save();
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_execute_interface overrides

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
			case AS_IO:      return nullptr;
			default:         return nullptr;
		}
	}

	//  device_state_interface overrides

	virtual void state_string_export(const device_state_entry &entry, std::string &str)
	{
		if (entry.index() >= 0)
		{
			if (entry.index() & 1)
				strprintf(str,"%10.6f", *((double *)entry.dataptr()));
			else
				strprintf(str, "%d", *((netlist_sig_t *)entry.dataptr()));
		}
	}

private:

	int m_genPC;

};

class nld_sound_out;
class nld_sound_in;

// ----------------------------------------------------------------------------------------
// netlist_mame_sound_device_t
// ----------------------------------------------------------------------------------------

class netlist_mame_sound_device_t : public netlist_mame_device_t,
									public device_sound_interface
{
public:

	// construction/destruction
	netlist_mame_sound_device_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_sound_device_t() {}

	static void static_set_constructor(device_t &device, void (*setup_func)(netlist::setup_t &));

	inline sound_stream *get_stream() { return m_stream; }


	// device_sound_interface overrides

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

protected:
	// netlist_mame_device_t
	virtual void nl_register_devices();

	// device_t overrides

	//virtual void device_config_complete();
	virtual void device_start();
	//virtual void device_stop();
	//virtual void device_reset();
	//virtual void device_post_load();
	//virtual void device_pre_save();
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:

	static const int MAX_OUT = 10;
	nld_sound_out *m_out[MAX_OUT];
	nld_sound_in *m_in;
	sound_stream *m_stream;
	int m_num_inputs;
	int m_num_outputs;

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
	{
		m_owner = dynamic_cast<netlist_mame_device_t *>(&aowner);
		m_sound = dynamic_cast<netlist_mame_sound_device_t *>(&aowner);
	}
	virtual ~netlist_mame_sub_interface() { }

	virtual void custom_netlist_additions(netlist::setup_t &setup) { }

	inline netlist_mame_device_t &nl_owner() const { return *m_owner; }

	inline bool is_sound_device() const { return (m_sound != nullptr); }

	inline void update_to_current_time()
	{
		m_sound->get_stream()->update();
	}

	static void static_set_mult_offset(device_t &device, const double mult, const double offset);

protected:
	double m_offset;
	double m_mult;

private:
	netlist_mame_device_t *m_owner;
	netlist_mame_sound_device_t *m_sound;
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

	inline void write(const double val)
	{
		if (is_sound_device())
		{
			update_to_current_time();
			m_param->setTo(val * m_mult + m_offset);
		}
		else
		{
			// FIXME: use device timer ....
			m_param->setTo(val * m_mult + m_offset);
		}
	}

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
	netlist::param_double_t *m_param;
	bool   m_auto_port;
	pstring m_param_name;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_analog_output_t
// ----------------------------------------------------------------------------------------

typedef device_delegate<void (const double, const attotime &)> netlist_analog_output_delegate;

class netlist_mame_analog_output_t : public device_t,
										public netlist_mame_sub_interface
{
public:

	// construction/destruction
	netlist_mame_analog_output_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_analog_output_t() { }

	static void static_set_params(device_t &device, const char *in_name, netlist_analog_output_delegate adelegate);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void custom_netlist_additions(netlist::setup_t &setup);

private:
	pstring m_in;
	netlist_analog_output_delegate m_delegate;
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

	inline void write(const UINT32 val)
	{
		const UINT32 v = (val >> m_shift) & m_mask;
		if (v != m_param->Value())
			synchronize(0, v);
	}

	inline DECLARE_INPUT_CHANGED_MEMBER(input_changed) { write(newval); }
	DECLARE_WRITE_LINE_MEMBER(write_line)       { write(state);  }
	DECLARE_WRITE8_MEMBER(write8)               { write(data);   }
	DECLARE_WRITE16_MEMBER(write16)             { write(data);   }
	DECLARE_WRITE32_MEMBER(write32)             { write(data);   }
	DECLARE_WRITE64_MEMBER(write64)             { write(data);   }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
	{
		if (is_sound_device())
			update_to_current_time();
		m_param->setTo(param);
	}

private:
	netlist::param_int_t *m_param;
	UINT32 m_mask;
	UINT32 m_shift;
	pstring m_param_name;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_input_t
// ----------------------------------------------------------------------------------------

class netlist_mame_stream_input_t :  public device_t,
										public netlist_mame_sub_interface
{
public:

	// construction/destruction
	netlist_mame_stream_input_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_stream_input_t() { }

	static void static_set_params(device_t &device, int channel, const char *param_name);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void custom_netlist_additions(netlist::setup_t &setup);
private:
	UINT32 m_channel;
	pstring m_param_name;
};

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_output_t
// ----------------------------------------------------------------------------------------

class netlist_mame_stream_output_t :  public device_t,
										public netlist_mame_sub_interface
{
public:

	// construction/destruction
	netlist_mame_stream_output_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_stream_output_t() { }

	static void static_set_params(device_t &device, int channel, const char *out_name);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void custom_netlist_additions(netlist::setup_t &setup);
private:
	UINT32 m_channel;
	pstring m_out_name;
};
// ----------------------------------------------------------------------------------------
// netdev_callback
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(analog_callback) : public netlist::device_t
{
public:
	NETLIB_NAME(analog_callback)()
		: device_t(), m_cpu_device(nullptr), m_last(0) { }

	ATTR_COLD void start()
	{
		register_input("IN", m_in);
		m_cpu_device = downcast<netlist_mame_cpu_device_t *>(&downcast<netlist_mame_t &>(netlist()).parent());
		save(NLNAME(m_last));
	}

	ATTR_COLD void reset()
	{
		m_last = 0.0;
	}

	ATTR_COLD void register_callback(netlist_analog_output_delegate callback)
	{
		m_callback = callback;
	}

	ATTR_HOT void update()
	{
		nl_double cur = INPANALOG(m_in);

		// FIXME: make this a parameter
		// avoid calls due to noise
		if (fabs(cur - m_last) > 1e-6)
		{
			m_cpu_device->update_time_x();
			m_callback(cur, m_cpu_device->local_time());
			m_cpu_device->check_mame_abort_slice();
			m_last = cur;
		}
	}

private:
	netlist::analog_input_t m_in;
	netlist_analog_output_delegate m_callback;
	netlist_mame_cpu_device_t *m_cpu_device;
	nl_double m_last;
};

// ----------------------------------------------------------------------------------------
// sound_out
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(sound_out) : public netlist::device_t
{
public:
	NETLIB_NAME(sound_out)()
		: netlist::device_t() { }

	static const int BUFSIZE = 2048;

	ATTR_COLD void start()
	{
		register_input("IN", m_in);
		register_param("CHAN", m_channel, 0);
		register_param("MULT", m_mult, 1000.0);
		register_param("OFFSET", m_offset, 0.0);
		m_sample = netlist::netlist_time::from_hz(1); //sufficiently big enough
		save(NAME(m_last_buffer));
	}

	ATTR_COLD void reset()
	{
		m_cur = 0.0;
		m_last_pos = 0;
		m_last_buffer = netlist::netlist_time::zero;
	}

	ATTR_HOT void sound_update(const netlist::netlist_time upto)
	{
		int pos = (upto - m_last_buffer) / m_sample;
		if (pos >= BUFSIZE)
			netlist().log().fatal("sound {1}: exceeded BUFSIZE\n", name().cstr());
		while (m_last_pos < pos )
		{
			m_buffer[m_last_pos++] = (stream_sample_t) m_cur;
		}
	}

	ATTR_HOT void update()
	{
		nl_double val = INPANALOG(m_in) * m_mult.Value() + m_offset.Value();
		sound_update(netlist().time());
		/* ignore spikes */
		if (std::abs(val) < 32767.0)
			m_cur = val;
		else if (val > 0.0)
			m_cur = 32767.0;
		else
			m_cur = -32767.0;

	}

	ATTR_HOT void buffer_reset(netlist::netlist_time upto)
	{
		m_last_pos = 0;
		m_last_buffer = upto;
		m_cur = 0.0;
	}

	netlist::param_int_t m_channel;
	netlist::param_double_t m_mult;
	netlist::param_double_t m_offset;
	stream_sample_t *m_buffer;
	netlist::netlist_time m_sample;

private:
	netlist::analog_input_t m_in;
	double m_cur;
	int m_last_pos;
	netlist::netlist_time m_last_buffer;
};

// ----------------------------------------------------------------------------------------
// sound_in
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(sound_in) : public netlist::device_t
{
public:
	NETLIB_NAME(sound_in)()
		: netlist::device_t() { }

	static const int MAX_INPUT_CHANNELS = 10;

	ATTR_COLD void start()
	{
		// clock part
		register_output("Q", m_Q);
		register_input("FB", m_feedback);

		connect_late(m_feedback, m_Q);
		m_inc = netlist::netlist_time::from_nsec(1);


		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			register_param(pfmt("CHAN{1}")(i), m_param_name[i], "");
			register_param(pfmt("MULT{1}")(i), m_param_mult[i], 1.0);
			register_param(pfmt("OFFSET{1}")(i), m_param_offset[i], 0.0);
		}
		m_num_channel = 0;
	}

	ATTR_COLD void reset()
	{
		m_pos = 0;
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
			m_buffer[i] = nullptr;
	}

	ATTR_COLD int resolve()
	{
		m_pos = 0;
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			if (m_param_name[i].Value() != "")
			{
				if (i != m_num_channel)
					netlist().log().fatal("sound input numbering has to be sequential!");
				m_num_channel++;
				m_param[i] = dynamic_cast<netlist::param_double_t *>(setup().find_param(m_param_name[i].Value(), true));
			}
		}
		return m_num_channel;
	}

	ATTR_HOT void update()
	{
		for (int i=0; i<m_num_channel; i++)
		{
			if (m_buffer[i] == nullptr)
				break; // stop, called outside of stream_update
			const nl_double v = m_buffer[i][m_pos];
			m_param[i]->setTo(v * m_param_mult[i].Value() + m_param_offset[i].Value());
		}
		m_pos++;
		OUTLOGIC(m_Q, !m_Q.net().as_logic().new_Q(), m_inc  );
	}

	ATTR_HOT void buffer_reset()
	{
		m_pos = 0;
	}

	netlist::param_str_t m_param_name[MAX_INPUT_CHANNELS];
	netlist::param_double_t *m_param[MAX_INPUT_CHANNELS];
	stream_sample_t *m_buffer[MAX_INPUT_CHANNELS];
	netlist::param_double_t m_param_mult[MAX_INPUT_CHANNELS];
	netlist::param_double_t m_param_offset[MAX_INPUT_CHANNELS];
	netlist::netlist_time m_inc;

private:
	netlist::logic_input_t m_feedback;
	netlist::logic_output_t m_Q;

	int m_pos;
	int m_num_channel;
};

// device type definition
extern const device_type NETLIST_CORE;
extern const device_type NETLIST_CPU;
extern const device_type NETLIST_SOUND;
extern const device_type NETLIST_ANALOG_INPUT;
extern const device_type NETLIST_LOGIC_INPUT;

extern const device_type NETLIST_ANALOG_OUTPUT;
extern const device_type NETLIST_STREAM_INPUT;
extern const device_type NETLIST_STREAM_OUTPUT;

#endif
