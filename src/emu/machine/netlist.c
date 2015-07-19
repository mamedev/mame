// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    netlist.c

    Discrete netlist implementation.

****************************************************************************/

#include "emu.h"
#include "netlist.h"
#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"
#include "netlist/nl_factory.h"
#include "netlist/nl_parser.h"
#include "netlist/devices/net_lib.h"
#include "debugger.h"

//#define LOG_DEV_CALLS(x)   printf x
#define LOG_DEV_CALLS(x)   do { } while (0)

const device_type NETLIST_CORE = &device_creator<netlist_mame_device_t>;
const device_type NETLIST_CPU = &device_creator<netlist_mame_cpu_device_t>;
const device_type NETLIST_SOUND = &device_creator<netlist_mame_sound_device_t>;

/* subdevices */

const device_type NETLIST_ANALOG_INPUT = &device_creator<netlist_mame_analog_input_t>;
const device_type NETLIST_LOGIC_INPUT = &device_creator<netlist_mame_logic_input_t>;
const device_type NETLIST_STREAM_INPUT = &device_creator<netlist_mame_stream_input_t>;

const device_type NETLIST_ANALOG_OUTPUT = &device_creator<netlist_mame_analog_output_t>;
const device_type NETLIST_STREAM_OUTPUT = &device_creator<netlist_mame_stream_output_t>;

// ----------------------------------------------------------------------------------------
// netlist_mame_analog_input_t
// ----------------------------------------------------------------------------------------

void netlist_mame_sub_interface::static_set_mult_offset(device_t &device, const double mult, const double offset)
{
	netlist_mame_sub_interface &netlist = dynamic_cast<netlist_mame_sub_interface &>(device);
	netlist.m_mult = mult;
	netlist.m_offset = offset;
}


netlist_mame_analog_input_t::netlist_mame_analog_input_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, NETLIST_ANALOG_INPUT, "Netlist Analog Input", tag, owner, clock, "netlist_analog_input", __FILE__),
			netlist_mame_sub_interface(*owner),
			m_param(0),
			m_auto_port(true),
			m_param_name("")
{
}

void netlist_mame_analog_input_t::static_set_name(device_t &device, const char *param_name)
{
	netlist_mame_analog_input_t &netlist = downcast<netlist_mame_analog_input_t &>(device);
	netlist.m_param_name = param_name;
}

void netlist_mame_analog_input_t::device_start()
{
	LOG_DEV_CALLS(("start %s\n", tag()));
	netlist::param_t *p = this->nl_owner().setup().find_param(m_param_name);
	m_param = dynamic_cast<netlist::param_double_t *>(p);
	if (m_param == NULL)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name.cstr());
	}
	if (m_mult != 1.0 || m_offset != 0.0)
	{
		// disable automatic scaling for ioports
		m_auto_port = false;
	}

}

// ----------------------------------------------------------------------------------------
// netlist_mame_analog_output_t
// ----------------------------------------------------------------------------------------

netlist_mame_analog_output_t::netlist_mame_analog_output_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, NETLIST_ANALOG_INPUT, "Netlist Analog Output", tag, owner, clock, "netlist_analog_output", __FILE__),
			netlist_mame_sub_interface(*owner),
			m_in("")
{
}

void netlist_mame_analog_output_t::static_set_params(device_t &device, const char *in_name, netlist_analog_output_delegate adelegate)
{
	netlist_mame_analog_output_t &netlist = downcast<netlist_mame_analog_output_t &>(device);
	netlist.m_in = in_name;
	netlist.m_delegate = adelegate;
}

void netlist_mame_analog_output_t::custom_netlist_additions(netlist::setup_t &setup)
{
	pstring dname = "OUT_" + m_in;
	m_delegate.bind_relative_to(owner()->machine().root_device());
	NETLIB_NAME(analog_callback) *dev = downcast<NETLIB_NAME(analog_callback) *>(
			setup.register_dev("NETDEV_CALLBACK", dname));

	dev->register_callback(m_delegate);
	setup.register_link(dname + ".IN", m_in);
}

void netlist_mame_analog_output_t::device_start()
{
	LOG_DEV_CALLS(("start %s\n", tag()));
}


// ----------------------------------------------------------------------------------------
// netlist_mame_logic_input_t
// ----------------------------------------------------------------------------------------

netlist_mame_logic_input_t::netlist_mame_logic_input_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, NETLIST_ANALOG_INPUT, "Netlist Logic Input", tag, owner, clock, "netlist_logic_input", __FILE__),
			netlist_mame_sub_interface(*owner),
			m_param(0),
			m_mask(0xffffffff),
			m_shift(0),
			m_param_name("")
{
}

void netlist_mame_logic_input_t::static_set_params(device_t &device, const char *param_name, const UINT32 mask, const UINT32 shift)
{
	netlist_mame_logic_input_t &netlist = downcast<netlist_mame_logic_input_t &>(device);
	netlist.m_param_name = param_name;
	netlist.m_shift = shift;
	netlist.m_mask = mask;
}

void netlist_mame_logic_input_t::device_start()
{
	LOG_DEV_CALLS(("start %s\n", tag()));
	netlist::param_t *p = downcast<netlist_mame_device_t *>(this->owner())->setup().find_param(m_param_name);
	m_param = dynamic_cast<netlist::param_int_t *>(p);
	if (m_param == NULL)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name.cstr());
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_input_t
// ----------------------------------------------------------------------------------------

netlist_mame_stream_input_t::netlist_mame_stream_input_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, NETLIST_ANALOG_INPUT, "Netlist Stream Input", tag, owner, clock, "netlist_stream_input", __FILE__),
			netlist_mame_sub_interface(*owner),
			m_channel(0),
			m_param_name("")
{
}

void netlist_mame_stream_input_t::static_set_params(device_t &device, int channel, const char *param_name)
{
	netlist_mame_stream_input_t &netlist = downcast<netlist_mame_stream_input_t &>(device);
	netlist.m_param_name = param_name;
	netlist.m_channel = channel;
}

void netlist_mame_stream_input_t::device_start()
{
	LOG_DEV_CALLS(("start %s\n", tag()));
}

void netlist_mame_stream_input_t::custom_netlist_additions(netlist::setup_t &setup)
{
	NETLIB_NAME(sound_in) *snd_in = setup.netlist().get_first_device<NETLIB_NAME(sound_in)>();
	if (snd_in == NULL)
		snd_in = dynamic_cast<NETLIB_NAME(sound_in) *>(setup.register_dev("NETDEV_SOUND_IN", "STREAM_INPUT"));

	pstring sparam = pstring::sprintf("STREAM_INPUT.CHAN%d", m_channel);
	setup.register_param(sparam, m_param_name);
	sparam = pstring::sprintf("STREAM_INPUT.MULT%d", m_channel);
	setup.register_param(sparam, m_mult);
	sparam = pstring::sprintf("STREAM_INPUT.OFFSET%d", m_channel);
	setup.register_param(sparam, m_offset);
}

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_output_t
// ----------------------------------------------------------------------------------------

netlist_mame_stream_output_t::netlist_mame_stream_output_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, NETLIST_ANALOG_INPUT, "Netlist Stream Output", tag, owner, clock, "netlist_stream_output", __FILE__),
			netlist_mame_sub_interface(*owner),
			m_channel(0),
			m_out_name("")
{
}

void netlist_mame_stream_output_t::static_set_params(device_t &device, int channel, const char *out_name)
{
	netlist_mame_stream_output_t &netlist = downcast<netlist_mame_stream_output_t &>(device);
	netlist.m_out_name = out_name;
	netlist.m_channel = channel;
}

void netlist_mame_stream_output_t::device_start()
{
	LOG_DEV_CALLS(("start %s\n", tag()));
}

void netlist_mame_stream_output_t::custom_netlist_additions(netlist::setup_t &setup)
{
	//NETLIB_NAME(sound_out) *snd_out;
	pstring sname = pstring::sprintf("STREAM_OUT_%d", m_channel);

	//snd_out = dynamic_cast<NETLIB_NAME(sound_out) *>(setup.register_dev("nld_sound_out", sname));
	setup.register_dev("NETDEV_SOUND_OUT", sname);

	setup.register_param(sname + ".CHAN" , m_channel);
	setup.register_param(sname + ".MULT",  m_mult);
	setup.register_param(sname + ".OFFSET",  m_offset);
	setup.register_link(sname + ".IN", m_out_name);
}


// ----------------------------------------------------------------------------------------
// netlist_mame_t
// ----------------------------------------------------------------------------------------

void netlist_mame_t::verror(const loglevel_e level, const char *format, va_list ap) const
{
	pstring errstr = pstring(format).vprintf(ap);

	switch (level)
	{
		case NL_WARNING:
			logerror("netlist WARNING: %s\n", errstr.cstr());
			break;
		case NL_LOG:
			logerror("netlist LOG: %s\n", errstr.cstr());
			break;
		case NL_ERROR:
			emu_fatalerror error("netlist ERROR: %s\n", errstr.cstr());
			throw error;
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_device_t
// ----------------------------------------------------------------------------------------

static ADDRESS_MAP_START(program_dummy, AS_PROGRAM, 8, netlist_mame_device_t)
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

netlist_mame_device_t::netlist_mame_device_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NETLIST_CORE, "Netlist core device", tag, owner, clock, "netlist_core", __FILE__),
		m_icount(0),
		m_div(0),
		m_rem(0),
		m_old(netlist::netlist_time::zero),
		m_netlist(NULL),
		m_setup(NULL),
		m_setup_func(NULL)
{
}

netlist_mame_device_t::netlist_mame_device_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *file)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, file),
		m_icount(0),
		m_div(0),
		m_rem(0),
		m_old(netlist::netlist_time::zero),
		m_netlist(NULL),
		m_setup(NULL),
		m_setup_func(NULL)
{
}

void netlist_mame_device_t::static_set_constructor(device_t &device, void (*setup_func)(netlist::setup_t &))
{
	LOG_DEV_CALLS(("static_set_constructor\n"));
	netlist_mame_device_t &netlist = downcast<netlist_mame_device_t &>(device);
	netlist.m_setup_func = setup_func;
}

void netlist_mame_device_t::device_config_complete()
{
	LOG_DEV_CALLS(("device_config_complete\n"));
}

void netlist_mame_device_t::device_start()
{
	LOG_DEV_CALLS(("device_start %s\n", tag()));

	//printf("clock is %d\n", clock());

	m_netlist = global_alloc(netlist_mame_t(*this));
	m_setup = global_alloc(netlist::setup_t(m_netlist));
	netlist().init_object(*m_netlist, "netlist");
	m_setup->init();

	// register additional devices

	nl_register_devices();

	m_setup_func(*m_setup);

	/* let sub-devices tweak the netlist */
	for( device_t *d = this->first_subdevice(); d != NULL; d = d->next() )
	{
		netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(d);
		if( sdev != NULL )
		{
			LOG_DEV_CALLS(("Found subdevice %s/%s\n", d->name(), d->shortname()));
			sdev->custom_netlist_additions(*m_setup);
		}
	}

	m_setup->start_devices();
	m_setup->resolve_inputs();

	netlist().save(NAME(m_rem));
	netlist().save(NAME(m_div));
	netlist().save(NAME(m_old));

	save_state();

	m_old = netlist::netlist_time::zero;
	m_rem = 0;

}

void netlist_mame_device_t::device_clock_changed()
{
	//printf("device_clock_changed\n");
	m_div = netlist::netlist_time::from_hz(clock()).as_raw();
	//m_rem = 0;
	//NL_VERBOSE_OUT(("Setting clock %" I64FMT "d and divisor %d\n", clock(), m_div));
	NL_VERBOSE_OUT(("Setting clock %d and divisor %d\n", clock(), m_div));
	//printf("Setting clock %d and divisor %d\n", clock(), m_div);
}


void netlist_mame_device_t::device_reset()
{
	LOG_DEV_CALLS(("device_reset\n"));
	m_old = netlist::netlist_time::zero;
	m_rem = 0;
	netlist().do_reset();
}

void netlist_mame_device_t::device_stop()
{
	LOG_DEV_CALLS(("device_stop\n"));
	m_setup->print_stats();

	m_netlist->stop();

	global_free(m_setup);
	m_setup = NULL;
	global_free(m_netlist);
	m_netlist = NULL;
}

ATTR_COLD void netlist_mame_device_t::device_post_load()
{
	LOG_DEV_CALLS(("device_post_load\n"));

	netlist().post_load();
	netlist().rebuild_lists();
}

ATTR_COLD void netlist_mame_device_t::device_pre_save()
{
	LOG_DEV_CALLS(("device_pre_save\n"));

	netlist().pre_save();
}

void netlist_mame_device_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

ATTR_HOT ATTR_ALIGN void netlist_mame_device_t::update_time_x()
{
	const netlist::netlist_time delta = netlist().time() - m_old + netlist::netlist_time::from_raw(m_rem);
	m_old = netlist().time();
	m_icount -= divu_64x32_rem(delta.as_raw(), m_div, &m_rem);
}

ATTR_HOT ATTR_ALIGN void netlist_mame_device_t::check_mame_abort_slice()
{
	if (m_icount <= 0)
		netlist().abort_current_queue_slice();
}

ATTR_COLD void netlist_mame_device_t::save_state()
{
	for (int i=0; i< netlist().save_list().size(); i++)
	{
		pstate_entry_t *s = netlist().save_list()[i];
		NL_VERBOSE_OUT(("saving state for %s\n", s->m_name.cstr()));
		switch (s->m_dt)
		{
			case DT_DOUBLE:
				{
					double *td = s->resolved<double>();
					if (td != NULL) save_pointer(td, s->m_name, s->m_count);
				}
				break;
			case DT_FLOAT:
				{
					float *td = s->resolved<float>();
					if (td != NULL) save_pointer(td, s->m_name, s->m_count);
				}
				break;
			case DT_INT64:
				save_pointer((INT64 *) s->m_ptr, s->m_name, s->m_count);
				break;
			case DT_INT16:
				save_pointer((INT16 *) s->m_ptr, s->m_name, s->m_count);
				break;
			case DT_INT8:
				save_pointer((INT8 *) s->m_ptr, s->m_name, s->m_count);
				break;
			case DT_INT:
				save_pointer((int *) s->m_ptr, s->m_name, s->m_count);
				break;
			case DT_BOOLEAN:
				save_pointer((bool *) s->m_ptr, s->m_name, s->m_count);
				break;
			case DT_CUSTOM:
				break;
			case NOT_SUPPORTED:
			default:
				netlist().error("found unsupported save element %s\n", s->m_name.cstr());
				break;
		}
	}

}

// ----------------------------------------------------------------------------------------
// netlist_mame_cpu_device_t
// ----------------------------------------------------------------------------------------

netlist_mame_cpu_device_t::netlist_mame_cpu_device_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: netlist_mame_device_t(mconfig, NETLIST_CPU, "Netlist CPU Device", tag, owner, clock, "netlist_cpu", __FILE__),
		device_execute_interface(mconfig, *this),
		device_state_interface(mconfig, *this),
		device_disasm_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		m_program_config("program", ENDIANNESS_LITTLE, 8, 12, 0, ADDRESS_MAP_NAME(program_dummy))
{
}


void netlist_mame_cpu_device_t::device_start()
{
	netlist_mame_device_t::device_start();

	LOG_DEV_CALLS(("cpu device_start %s\n", tag()));

	// State support

	state_add(STATE_GENPC, "curpc", m_genPC).noshow();

	for (int i=0; i < netlist().m_nets.size(); i++)
	{
		netlist::net_t *n = netlist().m_nets[i];
		if (n->isFamily(netlist::object_t::LOGIC))
		{
			state_add(i*2, n->name(), downcast<netlist::logic_net_t *>(n)->Q_state_ptr());
		}
		else
		{
			state_add(i*2+1, n->name(), downcast<netlist::analog_net_t *>(n)->Q_Analog_state_ptr()).formatstr("%20s");
		}
	}

	// set our instruction counter
	m_icountptr = &m_icount;
}


void netlist_mame_cpu_device_t::nl_register_devices()
{
	setup().factory().register_device<nld_analog_callback>( "NETDEV_CALLBACK", "nld_analog_callback", "-");
}

ATTR_COLD UINT64 netlist_mame_cpu_device_t::execute_clocks_to_cycles(UINT64 clocks) const
{
	return clocks;
}

ATTR_COLD UINT64 netlist_mame_cpu_device_t::execute_cycles_to_clocks(UINT64 cycles) const
{
	return cycles;
}

ATTR_COLD offs_t netlist_mame_cpu_device_t::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	//char tmp[16];
	unsigned startpc = pc;
	int relpc = pc - m_genPC;
	if (relpc >= 0 && relpc < netlist().queue().count())
	{
		int dpc = netlist().queue().count() - relpc - 1;
		// FIXME: 50 below fixes crash in mame-debugger. It's based on try on error.
		snprintf(buffer, 50, "%c %s @%10.7f", (relpc == 0) ? '*' : ' ', netlist().queue()[dpc].object()->name().cstr(),
				netlist().queue()[dpc].exec_time().as_double());
	}
	else
		sprintf(buffer, "%s", "");

	pc+=1;
	return (pc - startpc);
}

ATTR_HOT void netlist_mame_cpu_device_t::execute_run()
{
	bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);
	// debugging
	//m_ppc = m_pc; // copy PC to previous PC
	if (check_debugger)
	{
		while (m_icount > 0)
		{
			m_genPC++;
			m_genPC &= 255;
			debugger_instruction_hook(this, m_genPC);
			netlist().process_queue(netlist::netlist_time::from_raw(m_div));
			update_time_x();
		}
	}
	else
	{
		netlist().process_queue(netlist::netlist_time::from_raw(m_div) * m_icount);
		update_time_x();
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_sound_device_t
// ----------------------------------------------------------------------------------------

netlist_mame_sound_device_t::netlist_mame_sound_device_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: netlist_mame_device_t(mconfig, NETLIST_CPU, "Netlist Sound Device", tag, owner, clock, "netlist_sound", __FILE__),
		device_sound_interface(mconfig, *this)
{
}

void netlist_mame_sound_device_t::device_start()
{
	netlist_mame_device_t::device_start();

	LOG_DEV_CALLS(("sound device_start %s\n", tag()));

	// Configure outputs

	plist_t<nld_sound_out *> outdevs = netlist().get_device_list<nld_sound_out>();
	if (outdevs.size() == 0)
		fatalerror("No output devices");

	m_num_outputs = outdevs.size();

	/* resort channels */
	for (int i=0; i < MAX_OUT; i++) m_out[i] = NULL;
	for (int i=0; i < m_num_outputs; i++)
	{
		int chan = outdevs[i]->m_channel.Value();

		netlist().log("Output %d on channel %d", i, chan);

		if (chan < 0 || chan >= MAX_OUT || chan >= outdevs.size())
			fatalerror("illegal channel number");
		m_out[chan] = outdevs[i];
		m_out[chan]->m_sample = netlist::netlist_time::from_hz(clock());
		m_out[chan]->m_buffer = NULL;
	}

	// Configure inputs

	m_num_inputs = 0;
	m_in = NULL;

	plist_t<nld_sound_in *> indevs = netlist().get_device_list<nld_sound_in>();
	if (indevs.size() > 1)
		fatalerror("A maximum of one input device is allowed!");
	if (indevs.size() == 1)
	{
		m_in = indevs[0];
		m_num_inputs = m_in->resolve();
		m_in->m_inc = netlist::netlist_time::from_hz(clock());
	}

	/* initialize the stream(s) */
	m_stream = machine().sound().stream_alloc(*this, m_num_inputs, m_num_outputs, clock());

}

void netlist_mame_sound_device_t::nl_register_devices()
{
	setup().factory().register_device<nld_sound_out>("NETDEV_SOUND_OUT", "nld_sound_out", "+CHAN");
	setup().factory().register_device<nld_sound_in>("NETDEV_SOUND_IN", "nld_sound_in", "-");
}


void netlist_mame_sound_device_t::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int i=0; i < m_num_outputs; i++)
	{
		m_out[i]->m_buffer = outputs[i];
	}

	if (m_num_inputs)
		m_in->buffer_reset();

	for (int i=0; i < m_num_inputs; i++)
	{
		m_in->m_buffer[i] = inputs[i];
	}

	netlist::netlist_time cur = netlist().time();

	netlist().process_queue(netlist::netlist_time::from_raw(m_div) * samples);

	cur += (netlist::netlist_time::from_raw(m_div) * samples);

	for (int i=0; i < m_num_outputs; i++)
	{
		m_out[i]->sound_update(cur);
		m_out[i]->buffer_reset(cur);
	}
}

// ----------------------------------------------------------------------------------------
// memregion source support
// ----------------------------------------------------------------------------------------

bool netlist_source_memregion_t::parse(netlist::setup_t *setup, const pstring name)
{
	const char *mem = (const char *)downcast<netlist_mame_t &>(setup->netlist()).machine().root_device().memregion(m_name.cstr())->base();
	netlist::parser_t p(*setup);
	return p.parse(mem, name);
}
