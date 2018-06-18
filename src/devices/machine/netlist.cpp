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

#include "netlist/plib/palloc.h"

#include "debugger.h"

#include <cmath>
#include <memory>
#include <string>
#include <utility>



DEFINE_DEVICE_TYPE(NETLIST_CORE,  netlist_mame_device,       "netlist_core",  "Netlist Core Device")
DEFINE_DEVICE_TYPE(NETLIST_CPU,   netlist_mame_cpu_device,   "netlist_cpu",   "Netlist CPU Device")
DEFINE_DEVICE_TYPE(NETLIST_SOUND, netlist_mame_sound_device, "netlist_sound", "Netlist Sound Device")

/* subdevices */

DEFINE_DEVICE_TYPE(NETLIST_ANALOG_INPUT,  netlist_mame_analog_input_device,  "nl_analog_in",  "Netlist Analog Input")
DEFINE_DEVICE_TYPE(NETLIST_INT_INPUT,     netlist_mame_int_input_device,     "nl_int_out",    "Netlist Integer Output")
DEFINE_DEVICE_TYPE(NETLIST_RAM_POINTER,   netlist_mame_ram_pointer_device,   "nl_ram_ptr",    "Netlist RAM Pointer")
DEFINE_DEVICE_TYPE(NETLIST_LOGIC_INPUT,   netlist_mame_logic_input_device,   "nl_logic_in",   "Netlist Logic Input")
DEFINE_DEVICE_TYPE(NETLIST_STREAM_INPUT,  netlist_mame_stream_input_device,  "nl_stream_in",  "Netlist Stream Input")

DEFINE_DEVICE_TYPE(NETLIST_LOGIC_OUTPUT,  netlist_mame_logic_output_device,  "nl_logic_out",  "Netlist Logic Output")
DEFINE_DEVICE_TYPE(NETLIST_ANALOG_OUTPUT, netlist_mame_analog_output_device, "nl_analog_out", "Netlist Analog Output")
DEFINE_DEVICE_TYPE(NETLIST_STREAM_OUTPUT, netlist_mame_stream_output_device, "nl_stream_out", "Netlist Stream Output")


// ----------------------------------------------------------------------------------------
// Special netlist extension devices  ....
// ----------------------------------------------------------------------------------------

class netlist_mame_device::netlist_mame_t : public netlist::netlist_t
{
public:

	netlist_mame_t(netlist_mame_device &parent, const pstring &aname)
		: netlist::netlist_t(aname)
		, m_parent(parent)
	{
	}

	running_machine &machine() { return m_parent.machine(); }


	netlist_mame_device &parent() { return m_parent; }

protected:
	void vlog(const plib::plog_level &l, const pstring &ls) const override
	{
		switch (l)
		{
		case plib::plog_level::DEBUG:
			m_parent.logerror("netlist DEBUG: %s\n", ls.c_str());
			break;
		case plib::plog_level::INFO:
			m_parent.logerror("netlist INFO: %s\n", ls.c_str());
			break;
		case plib::plog_level::VERBOSE:
			m_parent.logerror("netlist VERBOSE: %s\n", ls.c_str());
			break;
		case plib::plog_level::WARNING:
			m_parent.logerror("netlist WARNING: %s\n", ls.c_str());
			break;
		case plib::plog_level::ERROR:
			m_parent.logerror("netlist ERROR: %s\n", ls.c_str());
			break;
		case plib::plog_level::FATAL:
			emu_fatalerror error("netlist ERROR: %s\n", ls.c_str());
			throw error;
		}
	}

private:
	netlist_mame_device &m_parent;
};


namespace {

// ----------------------------------------------------------------------------------------
// analog_callback
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(analog_callback) : public netlist::device_t
{
public:
	NETLIB_NAME(analog_callback)(netlist::netlist_t &anetlist, const pstring &name)
		: device_t(anetlist, name)
		, m_in(*this, "IN")
		, m_cpu_device(nullptr)
		, m_last(*this, "m_last", 0)
	{
		m_cpu_device = downcast<netlist_mame_cpu_device *>(&downcast<netlist_mame_device::netlist_mame_t &>(netlist()).parent());
	}

	ATTR_COLD void reset() override
	{
		m_last = 0.0;
	}

	ATTR_COLD void register_callback(netlist_mame_analog_output_device::output_delegate &&callback)
	{
		m_callback = std::move(callback);
	}

	NETLIB_UPDATEI()
	{
		nl_double cur = m_in();

		// FIXME: make this a parameter
		// avoid calls due to noise
		if (std::fabs(cur - m_last) > 1e-6)
		{
			m_cpu_device->update_time_x();
			m_callback(cur, m_cpu_device->local_time());
			m_cpu_device->check_mame_abort_slice();
			m_last = cur;
		}
	}

private:
	netlist::analog_input_t m_in;
	netlist_mame_analog_output_device::output_delegate m_callback;
	netlist_mame_cpu_device *m_cpu_device;
	netlist::state_var<nl_double> m_last;
};

// ----------------------------------------------------------------------------------------
// logic_callback
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(logic_callback) : public netlist::device_t
{
public:
	NETLIB_NAME(logic_callback)(netlist::netlist_t &anetlist, const pstring &name)
		: device_t(anetlist, name)
		, m_in(*this, "IN")
		, m_cpu_device(nullptr)
		, m_last(*this, "m_last", 0)
	{
		m_cpu_device = downcast<netlist_mame_cpu_device *>(&downcast<netlist_mame_device::netlist_mame_t &>(netlist()).parent());
	}

	ATTR_COLD void reset() override
	{
		m_last = 0;
	}

	ATTR_COLD void register_callback(netlist_mame_logic_output_device::output_delegate &&callback)
	{
		m_callback = std::move(callback);
	}

	NETLIB_UPDATEI()
	{
		netlist_sig_t cur = m_in();

		// FIXME: make this a parameter
		// avoid calls due to noise
		if (cur != m_last)
		{
			m_cpu_device->update_time_x();
			m_callback(cur, m_cpu_device->local_time());
			m_cpu_device->check_mame_abort_slice();
			m_last = cur;
		}
	}

private:
	netlist::logic_input_t m_in;
	netlist_mame_logic_output_device::output_delegate m_callback;
	netlist_mame_cpu_device *m_cpu_device;
	netlist::state_var<netlist_sig_t> m_last;
};

// ----------------------------------------------------------------------------------------
// Extensions to interface netlist with MAME code ....
// ----------------------------------------------------------------------------------------

class netlist_source_memregion_t : public netlist::source_t
{
public:
	netlist_source_memregion_t(netlist::setup_t &setup, pstring name)
	: netlist::source_t(setup), m_name(name)
	{
	}

	virtual std::unique_ptr<plib::pistream> stream(const pstring &name) override;
private:
	pstring m_name;
};

class netlist_data_memregions_t : public netlist::source_t
{
public:
	netlist_data_memregions_t(netlist::setup_t &setup);

	virtual std::unique_ptr<plib::pistream> stream(const pstring &name) override;
};


// ----------------------------------------------------------------------------------------
// memregion source support
// ----------------------------------------------------------------------------------------

std::unique_ptr<plib::pistream> netlist_source_memregion_t::stream(const pstring &name)
{
	memory_region *mem = downcast<netlist_mame_device::netlist_mame_t &>(setup().netlist()).machine().root_device().memregion(m_name.c_str());
	return plib::make_unique_base<plib::pistream, plib::pimemstream>(mem->base(), mem->bytes());
}

netlist_data_memregions_t::netlist_data_memregions_t(netlist::setup_t &setup)
	: netlist::source_t(setup, netlist::source_t::DATA)
{
}

std::unique_ptr<plib::pistream> netlist_data_memregions_t::stream(const pstring &name)
{
	memory_region *mem = downcast<netlist_mame_device::netlist_mame_t &>(setup().netlist()).parent().memregion(name.c_str());
	//memory_region *mem = downcast<netlist_mame_t &>(setup().netlist()).machine().root_device().memregion(name.c_str());
	if (mem != nullptr)
	{
		return plib::make_unique_base<plib::pistream, plib::pimemstream>(mem->base(), mem->bytes());
	}
	else
	{
		// This should be the last data provider being called - last resort
		fatalerror("data named %s not found in device rom regions\n", name.c_str());
		return std::unique_ptr<plib::pistream>(nullptr);
	}
}

} // anonymous namespace



// ----------------------------------------------------------------------------------------
// sound_out
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(sound_out) : public netlist::device_t
{
public:
	NETLIB_NAME(sound_out)(netlist::netlist_t &anetlist, const pstring &name)
		: netlist::device_t(anetlist, name)
		, m_channel(*this, "CHAN", 0)
		, m_mult(*this, "MULT", 1000.0)
		, m_offset(*this, "OFFSET", 0.0)
		, m_buffer(nullptr)
		, m_sample(netlist::netlist_time::from_hz(1)) //sufficiently big enough
		, m_in(*this, "IN")
		, m_cur(0.0)
		, m_last_pos(0)
		, m_last_buffer(*this, "m_last_buffer", netlist::netlist_time::zero())
	{
	}

	static const int BUFSIZE = 2048;

	ATTR_COLD void reset() override
	{
		m_cur = 0.0;
		m_last_pos = 0;
		m_last_buffer = netlist::netlist_time::zero();
	}

	ATTR_HOT void sound_update(const netlist::netlist_time &upto)
	{
		int pos = (upto - m_last_buffer) / m_sample;
		if (pos >= BUFSIZE)
			netlist().log().fatal("sound {1}: exceeded BUFSIZE\n", name().c_str());
		while (m_last_pos < pos )
		{
			m_buffer[m_last_pos++] = (stream_sample_t) m_cur;
		}
	}

	NETLIB_UPDATEI()
	{
		nl_double val = m_in() * m_mult() + m_offset();
		sound_update(netlist().time());
		/* ignore spikes */
		if (std::abs(val) < 32767.0)
			m_cur = val;
		else if (val > 0.0)
			m_cur = 32767.0;
		else
			m_cur = -32767.0;

	}

public:
	ATTR_HOT void buffer_reset(const netlist::netlist_time &upto)
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
	netlist::state_var<netlist::netlist_time> m_last_buffer;
};

// ----------------------------------------------------------------------------------------
// sound_in
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(sound_in) : public netlist::device_t
{
public:
	NETLIB_NAME(sound_in)(netlist::netlist_t &anetlist, const pstring &name)
	: netlist::device_t(anetlist, name)
	, m_feedback(*this, "FB") // clock part
	, m_Q(*this, "Q")
	, m_pos(0)
	, m_num_channel(0)
	{
		connect(m_feedback, m_Q);
		m_inc = netlist::netlist_time::from_nsec(1);


		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			m_param_name[i] = std::make_unique<netlist::param_str_t>(*this, plib::pfmt("CHAN{1}")(i), "");
			m_param_mult[i] = std::make_unique<netlist::param_double_t>(*this, plib::pfmt("MULT{1}")(i), 1.0);
			m_param_offset[i] = std::make_unique<netlist::param_double_t>(*this, plib::pfmt("OFFSET{1}")(i), 0.0);
		}
	}

	static const int MAX_INPUT_CHANNELS = 10;

	ATTR_COLD void reset() override
	{
		m_pos = 0;
		for (auto & elem : m_buffer)
			elem = nullptr;
	}

	ATTR_COLD int resolve()
	{
		m_pos = 0;
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			if ((*m_param_name[i])() != pstring(""))
			{
				if (i != m_num_channel)
					netlist().log().fatal("sound input numbering has to be sequential!");
				m_num_channel++;
				m_param[i] = dynamic_cast<netlist::param_double_t *>(setup().find_param((*m_param_name[i])(), true));
			}
		}
		return m_num_channel;
	}

	NETLIB_UPDATEI()
	{
		for (int i=0; i<m_num_channel; i++)
		{
			if (m_buffer[i] == nullptr)
				break; // stop, called outside of stream_update
			const nl_double v = m_buffer[i][m_pos];
			m_param[i]->setTo(v * (*m_param_mult[i])() + (*m_param_offset[i])());
		}
		m_pos++;
		m_Q.net().toggle_and_push_to_queue(m_inc);
	}

public:
	ATTR_HOT void buffer_reset()
	{
		m_pos = 0;
	}

	std::unique_ptr<netlist::param_str_t> m_param_name[MAX_INPUT_CHANNELS];
	netlist::param_double_t *m_param[MAX_INPUT_CHANNELS];
	stream_sample_t *m_buffer[MAX_INPUT_CHANNELS];
	std::unique_ptr<netlist::param_double_t> m_param_mult[MAX_INPUT_CHANNELS];
	std::unique_ptr<netlist::param_double_t> m_param_offset[MAX_INPUT_CHANNELS];
	netlist::netlist_time m_inc;

private:
	netlist::logic_input_t m_feedback;
	netlist::logic_output_t m_Q;

	int m_pos;
	int m_num_channel;
};


// netlib #defines this and it fights with logmacro.h
#undef LOG

#define LOG_GENERAL     (1U << 0)
#define LOG_DEV_CALLS   (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_DEV_CALLS)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGDEVCALLS(...) LOGMASKED(LOG_DEV_CALLS, __VA_ARGS__)


netlist::setup_t &netlist_mame_device::setup()
{
	return m_netlist->setup();
}

void netlist_mame_device::register_memregion_source(netlist::setup_t &setup, const char *name)
{
	setup.register_source(plib::make_unique_base<netlist::source_t, netlist_source_memregion_t>(setup, pstring(name, pstring::UTF8)));
}

void netlist_mame_analog_input_device::write(const double val)
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

void netlist_mame_int_input_device::write(const uint32_t val)
{
	const uint32_t v = (val >> m_shift) & m_mask;
	if (v != (*m_param)())
		synchronize(0, v);
}

void netlist_mame_logic_input_device::write(const uint32_t val)
{
	const uint32_t v = (val >> m_shift) & 1;
	if (v != (*m_param)())
		synchronize(0, v);
}

void netlist_mame_int_input_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (is_sound_device())
		update_to_current_time();
	m_param->setTo(param);
}

void netlist_mame_logic_input_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (is_sound_device())
		update_to_current_time();
	m_param->setTo(param);
}

void netlist_mame_ram_pointer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_data = (*m_param)();
}

device_memory_interface::space_config_vector netlist_mame_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void netlist_mame_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() >= 0)
	{
		if (entry.index() & 1)
			str = string_format("%10.6f", *((double *)entry.dataptr()));
		else
			str = string_format("%d", *((netlist_sig_t *)entry.dataptr()));
	}
}


// ----------------------------------------------------------------------------------------
// netlist_mame_analog_input_device
// ----------------------------------------------------------------------------------------

void netlist_mame_sub_interface::set_mult_offset(const double mult, const double offset)
{
	m_mult = mult;
	m_offset = offset;
}


netlist_mame_analog_input_device::netlist_mame_analog_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_ANALOG_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_auto_port(true)
	, m_param_name("")
{
}

void netlist_mame_analog_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_t *p = this->nl_owner().setup().find_param(pstring(m_param_name, pstring::UTF8));
	m_param = dynamic_cast<netlist::param_double_t *>(p);
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
	if (m_mult != 1.0 || m_offset != 0.0)
	{
		// disable automatic scaling for ioports
		m_auto_port = false;
	}

}


// ----------------------------------------------------------------------------------------
// netlist_mame_analog_output_device
// ----------------------------------------------------------------------------------------

netlist_mame_analog_output_device::netlist_mame_analog_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_ANALOG_OUTPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_in("")
{
}

void netlist_mame_analog_output_device::set_params(const char *in_name, output_delegate &&adelegate)
{
	m_in = in_name;
	m_delegate = std::move(adelegate);
}

void netlist_mame_analog_output_device::custom_netlist_additions(netlist::setup_t &setup)
{
	const pstring pin(m_in, pstring::UTF8);
	pstring dname = pstring("OUT_") + pin;
	m_delegate.bind_relative_to(owner()->machine().root_device());

	plib::owned_ptr<netlist::device_t> dev = plib::owned_ptr<netlist::device_t>::Create<NETLIB_NAME(analog_callback)>(setup.netlist(), setup.build_fqn(dname));
	static_cast<NETLIB_NAME(analog_callback) *>(dev.get())->register_callback(std::move(m_delegate));
	setup.netlist().register_dev(std::move(dev));
	setup.register_link(dname + ".IN", pin);
}

void netlist_mame_analog_output_device::device_start()
{
	LOGDEVCALLS("start\n");
}


// ----------------------------------------------------------------------------------------
// netlist_mame_logic_output_device
// ----------------------------------------------------------------------------------------

netlist_mame_logic_output_device::netlist_mame_logic_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_LOGIC_OUTPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_in("")
{
}

void netlist_mame_logic_output_device::set_params(const char *in_name, output_delegate &&adelegate)
{
	m_in = in_name;
	m_delegate = std::move(adelegate);
}

void netlist_mame_logic_output_device::custom_netlist_additions(netlist::setup_t &setup)
{
	pstring pin(m_in, pstring::UTF8);
	pstring dname = "OUT_" + pin;
	m_delegate.bind_relative_to(owner()->machine().root_device());

	plib::owned_ptr<netlist::device_t> dev = plib::owned_ptr<netlist::device_t>::Create<NETLIB_NAME(logic_callback)>(setup.netlist(), setup.build_fqn(dname));
	static_cast<NETLIB_NAME(logic_callback) *>(dev.get())->register_callback(std::move(m_delegate));
	setup.netlist().register_dev(std::move(dev));
	setup.register_link(dname + ".IN", pin);
}

void netlist_mame_logic_output_device::device_start()
{
	LOGDEVCALLS("start\n");
}


// ----------------------------------------------------------------------------------------
// netlist_mame_int_input_device
// ----------------------------------------------------------------------------------------

netlist_mame_int_input_device::netlist_mame_int_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_INT_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_mask(0xffffffff)
	, m_shift(0)
	, m_param_name("")
{
}

void netlist_mame_int_input_device::set_params(const char *param_name, const uint32_t mask, const uint32_t shift)
{
	if (LOG_DEV_CALLS) logerror("set_params\n");
	m_param_name = param_name;
	m_shift = shift;
	m_mask = mask;
}

void netlist_mame_int_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_t *p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name, pstring::UTF8));
	m_param = dynamic_cast<netlist::param_int_t *>(p);
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_logic_input_device
// ----------------------------------------------------------------------------------------

netlist_mame_logic_input_device::netlist_mame_logic_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_LOGIC_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_shift(0)
	, m_param_name("")
{
}

void netlist_mame_logic_input_device::set_params(const char *param_name, const uint32_t shift)
{
	if (LOG_DEV_CALLS) logerror("set_params\n");
	m_param_name = param_name;
	m_shift = shift;
}

void netlist_mame_logic_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_t *p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name, pstring::UTF8));
	m_param = dynamic_cast<netlist::param_logic_t *>(p);
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}


// ----------------------------------------------------------------------------------------
// netlist_mame_ram_pointer_device
// ----------------------------------------------------------------------------------------

netlist_mame_ram_pointer_device::netlist_mame_ram_pointer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_RAM_POINTER, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_param_name("")
	, m_data(nullptr)
{
}

void netlist_mame_ram_pointer_device::set_params(const char *param_name)
{
	if (LOG_DEV_CALLS) logerror("set_params\n");
	m_param_name = param_name;
}

void netlist_mame_ram_pointer_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_t *p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name, pstring::UTF8));
	m_param = dynamic_cast<netlist::param_ptr_t *>(p);
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}

	m_data = (*m_param)();
}

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_input_device
// ----------------------------------------------------------------------------------------

netlist_mame_stream_input_device::netlist_mame_stream_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_STREAM_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_channel(0)
	, m_param_name("")
{
}

void netlist_mame_stream_input_device::set_params(int channel, const char *param_name)
{
	m_param_name = param_name;
	m_channel = channel;
}

void netlist_mame_stream_input_device::device_start()
{
	LOGDEVCALLS("start\n");
}

void netlist_mame_stream_input_device::custom_netlist_additions(netlist::setup_t &setup)
{
	if (!setup.device_exists("STREAM_INPUT"))
		setup.register_dev("NETDEV_SOUND_IN", "STREAM_INPUT");

	pstring sparam = plib::pfmt("STREAM_INPUT.CHAN{1}")(m_channel);
	setup.register_param(sparam, pstring(m_param_name, pstring::UTF8));
	sparam = plib::pfmt("STREAM_INPUT.MULT{1}")(m_channel);
	setup.register_param(sparam, m_mult);
	sparam = plib::pfmt("STREAM_INPUT.OFFSET{1}")(m_channel);
	setup.register_param(sparam, m_offset);
}

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_output_device
// ----------------------------------------------------------------------------------------

netlist_mame_stream_output_device::netlist_mame_stream_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_STREAM_OUTPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_channel(0)
	, m_out_name("")
{
}

void netlist_mame_stream_output_device::set_params(int channel, const char *out_name)
{
	m_out_name = out_name;
	m_channel = channel;
}

void netlist_mame_stream_output_device::device_start()
{
	LOGDEVCALLS("start\n");
}

void netlist_mame_stream_output_device::custom_netlist_additions(netlist::setup_t &setup)
{
	//NETLIB_NAME(sound_out) *snd_out;
	pstring sname = plib::pfmt("STREAM_OUT_{1}")(m_channel);

	//snd_out = dynamic_cast<NETLIB_NAME(sound_out) *>(setup.register_dev("nld_sound_out", sname));
	setup.register_dev("NETDEV_SOUND_OUT", sname);

	setup.register_param(sname + ".CHAN" , m_channel);
	setup.register_param(sname + ".MULT",  m_mult);
	setup.register_param(sname + ".OFFSET",  m_offset);
	setup.register_link(sname + ".IN", pstring(m_out_name, pstring::UTF8));
}


// ----------------------------------------------------------------------------------------
// netlist_mame_device
// ----------------------------------------------------------------------------------------

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: netlist_mame_device(mconfig, NETLIST_CORE, tag, owner, clock)
{
}

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_icount(0)
	, m_old(netlist::netlist_time::zero())
	, m_netlist(nullptr)
	, m_setup_func(nullptr)
{
}

netlist_mame_device::~netlist_mame_device()
{
	LOGDEVCALLS("~netlist_mame_device\n");
}

void netlist_mame_device::set_constructor(void (*setup_func)(netlist::setup_t &))
{
	if (LOG_DEV_CALLS) logerror("set_constructor\n");
	m_setup_func = setup_func;
}

void netlist_mame_device::device_config_complete()
{
	LOGDEVCALLS("device_config_complete %s\n", this->mconfig().gamedrv().name);
}

void netlist_mame_device::device_validity_check(validity_checker &valid) const
{
	LOGDEVCALLS("device_validity_check %s\n", this->mconfig().gamedrv().name);
}


void netlist_mame_device::device_start()
{
	LOGDEVCALLS("device_start entry\n");

	//printf("clock is %d\n", clock());

	m_netlist = global_alloc(netlist_mame_t(*this, "netlist"));

	// register additional devices

	nl_register_devices();

	/* let sub-devices add sources and do stuff prior to parsing */
	for (device_t &d : subdevices())
	{
		netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
		if( sdev != nullptr )
		{
			LOGDEVCALLS("Preparse subdevice %s/%s\n", d.name(), d.shortname());
			sdev->pre_parse_action(setup());
		}
	}

	/* add default data provider for roms */
	setup().register_source(plib::make_unique_base<netlist::source_t, netlist_data_memregions_t>(setup()));

	m_setup_func(setup());

	/* let sub-devices tweak the netlist */
	for (device_t &d : subdevices())
	{
		netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
		if( sdev != nullptr )
		{
			LOGDEVCALLS("Found subdevice %s/%s\n", d.name(), d.shortname());
			sdev->custom_netlist_additions(setup());
		}
	}

	netlist().start();

	netlist().save(*this, m_rem, "m_rem");
	netlist().save(*this, m_div, "m_div");
	netlist().save(*this, m_old, "m_old");

	save_state();

	m_old = netlist::netlist_time::zero();
	m_rem = netlist::netlist_time::zero();

	LOGDEVCALLS("device_start exit\n");
}

void netlist_mame_device::device_clock_changed()
{
	m_div = netlist::netlist_time::from_hz(clock());
	netlist().log().debug("Setting clock {1} and divisor {2}\n", clock(), m_div.as_double());
}


void netlist_mame_device::device_reset()
{
	LOGDEVCALLS("device_reset\n");
	m_old = netlist::netlist_time::zero();
	m_rem = netlist::netlist_time::zero();
	netlist().reset();
}

void netlist_mame_device::device_stop()
{
	LOGDEVCALLS("device_stop\n");
	netlist().stop();

	global_free(m_netlist);
	m_netlist = nullptr;
}

ATTR_COLD void netlist_mame_device::device_post_load()
{
	LOGDEVCALLS("device_post_load\n");

	netlist().state().post_load();
	netlist().rebuild_lists();
}

ATTR_COLD void netlist_mame_device::device_pre_save()
{
	LOGDEVCALLS("device_pre_save\n");

	netlist().state().pre_save();
}

void netlist_mame_device::update_time_x()
{
	const netlist::netlist_time newt(netlist().time());
	const netlist::netlist_time delta(newt - m_old + m_rem);
	const uint64_t d = delta / m_div;
	m_old = newt;
	m_rem = delta - (m_div * d);
	m_icount -= d;
}

void netlist_mame_device::check_mame_abort_slice()
{
	if (m_icount <= 0)
		netlist().abort_current_queue_slice();
}

ATTR_COLD void netlist_mame_device::save_state()
{
	for (auto const & s : netlist().state().save_list())
	{
		netlist().log().debug("saving state for {1}\n", s->m_name.c_str());
		if (s->m_dt.is_float)
		{
			if (s->m_dt.size == sizeof(double))
				save_pointer((double *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else if (s->m_dt.size == sizeof(float))
				save_pointer((float *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else
				netlist().log().fatal("Unknown floating type for {1}\n", s->m_name.c_str());
		}
		else if (s->m_dt.is_integral)
		{
			if (s->m_dt.size == sizeof(int64_t))
				save_pointer((int64_t *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else if (s->m_dt.size == sizeof(int32_t))
				save_pointer((int32_t *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else if (s->m_dt.size == sizeof(int16_t))
				save_pointer((int16_t *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else if (s->m_dt.size == sizeof(int8_t))
				save_pointer((int8_t *) s->m_ptr, s->m_name.c_str(), s->m_count);
#if (PHAS_INT128)
			else if (s->m_dt.size == sizeof(INT128))
				save_pointer((int64_t *) s->m_ptr, s->m_name.c_str(), s->m_count * 2);
#endif
			else
				netlist().log().fatal("Unknown integral type size {1} for {2}\n", s->m_dt.size, s->m_name.c_str());
		}
		else if (s->m_dt.is_custom)
		{
			/* do nothing */
		}
		else
			netlist().log().fatal("found unsupported save element {1}\n", s->m_name);
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_cpu_device
// ----------------------------------------------------------------------------------------

netlist_mame_cpu_device::netlist_mame_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: netlist_mame_device(mconfig, NETLIST_CPU, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, device_state_interface(mconfig, *this)
	, device_disasm_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 12) // Interface is needed to keep debugger happy
	, m_genPC(0)
{
}


void netlist_mame_cpu_device::device_start()
{
	netlist_mame_device::device_start();

	// State support

	state_add(STATE_GENPC, "GENPC", m_genPC).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_genPC).noshow();

	for (int i=0; i < netlist().m_nets.size(); i++)
	{
		netlist::detail::net_t *n = netlist().m_nets[i].get();
		if (n->is_logic())
		{
			state_add(i*2, n->name().c_str(), *downcast<netlist::logic_net_t *>(n)->Q_state_ptr());
		}
		else
		{
			state_add(i*2+1, n->name().c_str(), *downcast<netlist::analog_net_t *>(n)->Q_Analog_state_ptr());
		}
	}

	// set our instruction counter
	set_icountptr(m_icount);
}


void netlist_mame_cpu_device::nl_register_devices()
{
	setup().factory().register_device<nld_analog_callback>( "NETDEV_CALLBACK", "nld_analog_callback", "-");
}

ATTR_COLD uint64_t netlist_mame_cpu_device::execute_clocks_to_cycles(uint64_t clocks) const
{
	return clocks;
}

ATTR_COLD uint64_t netlist_mame_cpu_device::execute_cycles_to_clocks(uint64_t cycles) const
{
	return cycles;
}

ATTR_HOT void netlist_mame_cpu_device::execute_run()
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
			debugger_instruction_hook(m_genPC);
			netlist().process_queue(m_div);
			update_time_x();
		}
	}
	else
	{
		netlist().process_queue(m_div * m_icount);
		update_time_x();
	}
}

std::unique_ptr<util::disasm_interface> netlist_mame_cpu_device::create_disassembler()
{
	return std::make_unique<netlist_disassembler>(this);
}

netlist_disassembler::netlist_disassembler(netlist_mame_cpu_device *dev) : m_dev(dev)
{
}

u32 netlist_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t netlist_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	unsigned startpc = pc;
	int relpc = pc - m_dev->genPC();
	if (relpc >= 0 && relpc < m_dev->netlist().queue().size())
	{
		int dpc = m_dev->netlist().queue().size() - relpc - 1;
		util::stream_format(stream, "%c %s @%10.7f", (relpc == 0) ? '*' : ' ', m_dev->netlist().queue()[dpc].m_object->name().c_str(),
				m_dev->netlist().queue()[dpc].m_exec_time.as_double());
	}

	pc+=1;
	return (pc - startpc);
}

// ----------------------------------------------------------------------------------------
// netlist_mame_sound_device
// ----------------------------------------------------------------------------------------

netlist_mame_sound_device::netlist_mame_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: netlist_mame_device(mconfig, NETLIST_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_out{nullptr}
	, m_in(nullptr)
	, m_stream(nullptr)
	, m_num_inputs(0)
	, m_num_outputs(0)
{
}

void netlist_mame_sound_device::device_start()
{
	netlist_mame_device::device_start();

	LOGDEVCALLS("sound device_start\n");

	// Configure outputs

	std::vector<nld_sound_out *> outdevs = netlist().get_device_list<nld_sound_out>();
	if (outdevs.size() == 0)
		fatalerror("No output devices");

	m_num_outputs = outdevs.size();

	/* resort channels */
	for (int i=0; i < MAX_OUT; i++) m_out[i] = nullptr;
	for (int i=0; i < m_num_outputs; i++)
	{
		int chan = outdevs[i]->m_channel();

		netlist().log().verbose("Output %d on channel %d", i, chan);

		if (chan < 0 || chan >= MAX_OUT || chan >= outdevs.size())
			fatalerror("illegal channel number");
		m_out[chan] = outdevs[i];
		m_out[chan]->m_sample = netlist::netlist_time::from_hz(clock());
		m_out[chan]->m_buffer = nullptr;
	}

	// Configure inputs

	m_num_inputs = 0;
	m_in = nullptr;

	std::vector<nld_sound_in *> indevs = netlist().get_device_list<nld_sound_in>();
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

void netlist_mame_sound_device::nl_register_devices()
{
	setup().factory().register_device<nld_sound_out>("NETDEV_SOUND_OUT", "nld_sound_out", "+CHAN");
	setup().factory().register_device<nld_sound_in>("NETDEV_SOUND_IN", "nld_sound_in", "-");
}


void netlist_mame_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
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

	netlist::netlist_time cur(netlist().time());

	netlist().process_queue(m_div * samples);

	cur += (m_div * samples);

	for (int i=0; i < m_num_outputs; i++)
	{
		m_out[i]->sound_update(cur);
		m_out[i]->buffer_reset(cur);
	}
}
