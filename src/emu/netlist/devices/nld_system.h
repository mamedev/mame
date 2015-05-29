// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_system.h
 *
 * netlist devices defined in the core
 */

#ifndef NLD_SYSTEM_H_
#define NLD_SYSTEM_H_

#include "../nl_setup.h"
#include "../nl_base.h"
#include "../analog/nld_twoterm.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

#define TTL_INPUT(_name, _v)                                                   \
		NET_REGISTER_DEV(ttl_input, _name)                                     \
		PARAM(_name.IN, _v)

#define ANALOG_INPUT(_name, _v)                                                \
		NET_REGISTER_DEV(analog_input, _name)                                  \
		PARAM(_name.IN, _v)

#define MAINCLOCK(_name, _freq)                                                \
		NET_REGISTER_DEV(mainclock, _name)                                     \
		PARAM(_name.FREQ, _freq)

#define CLOCK(_name, _freq)                                                    \
		NET_REGISTER_DEV(clock, _name)                                         \
		PARAM(_name.FREQ, _freq)

#define EXTCLOCK(_name, _freq, _pattern)                                       \
		NET_REGISTER_DEV(extclock, _name)                                      \
		PARAM(_name.FREQ, _freq)                                               \
		PARAM(_name.PATTERN, _pattern)

#define GNDA()                                                                 \
		NET_REGISTER_DEV(gnd, GND)

#define DUMMY_INPUT(_name)                                                     \
		NET_REGISTER_DEV(dummy_input, _name)

#define FRONTIER(_name, _IN, _OUT)                                             \
		NET_REGISTER_DEV(frontier, _name)                                      \
		NET_C(_IN, _name.I)                                                    \
		NET_C(_OUT, _name.Q)

#define RES_SWITCH(_name, _IN, _P1, _P2)                                       \
		NET_REGISTER_DEV(res_sw, _name)                                        \
		NET_C(_IN, _name.I)                                                    \
		NET_C(_P1, _name.1)                                                    \
		NET_C(_P2, _name.2)
/* Default device to hold netlist parameters */
#define PARAMETERS(_name)                                                      \
		NET_REGISTER_DEV(netlistparams, _name)
// -----------------------------------------------------------------------------
// mainclock
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(netlistparams,
public:
		netlist_param_logic_t m_use_deactivate;
);

// -----------------------------------------------------------------------------
// mainclock
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(mainclock,
public:
	netlist_logic_output_t m_Q;

	netlist_param_double_t m_freq;
	netlist_time m_inc;

	ATTR_HOT inline static void mc_update(netlist_logic_net_t &net);
);

// -----------------------------------------------------------------------------
// clock
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(clock,
	netlist_logic_input_t m_feedback;
	netlist_logic_output_t m_Q;

	netlist_param_double_t m_freq;
	netlist_time m_inc;
);

// -----------------------------------------------------------------------------
// extclock
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(extclock,
	netlist_logic_input_t m_feedback;
	netlist_logic_output_t m_Q;

	netlist_param_double_t m_freq;
	netlist_param_str_t m_pattern;
	netlist_param_double_t m_offset;

	UINT8 m_cnt;
	UINT8 m_size;
	netlist_time m_off;
	netlist_time m_inc[32];
);

// -----------------------------------------------------------------------------
// Special support devices ...
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(ttl_input,
	netlist_logic_output_t m_Q;

	netlist_param_logic_t m_IN;
);

NETLIB_DEVICE_WITH_PARAMS(analog_input,
	netlist_analog_output_t m_Q;

	netlist_param_double_t m_IN;
);

// -----------------------------------------------------------------------------
// nld_gnd
// -----------------------------------------------------------------------------

class NETLIB_NAME(gnd) : public netlist_device_t
{
public:
	ATTR_COLD NETLIB_NAME(gnd)()
			: netlist_device_t(GND) { }

	/* ATTR_COLD */ virtual ~NETLIB_NAME(gnd)() {}

protected:

	ATTR_COLD void start()
	{
		register_output("Q", m_Q);
	}

	ATTR_COLD void reset()
	{
	}

	ATTR_HOT void update()
	{
		OUTANALOG(m_Q, 0.0);
	}

private:
	netlist_analog_output_t m_Q;

};

// -----------------------------------------------------------------------------
// nld_dummy_input
// -----------------------------------------------------------------------------

class NETLIB_NAME(dummy_input) : public netlist_device_t
{
public:
	ATTR_COLD NETLIB_NAME(dummy_input)()
			: netlist_device_t(DUMMY) { }

	/* ATTR_COLD */ virtual ~NETLIB_NAME(dummy_input)() {}

protected:

	ATTR_COLD void start()
	{
		register_input("I", m_I);
	}

	ATTR_COLD void reset()
	{
	}

	ATTR_HOT void update()
	{
	}

private:
	netlist_analog_input_t m_I;

};

// -----------------------------------------------------------------------------
// nld_frontier
// -----------------------------------------------------------------------------

class NETLIB_NAME(frontier) : public netlist_device_t
{
public:
	ATTR_COLD NETLIB_NAME(frontier)()
			: netlist_device_t(DUMMY) { }

	/* ATTR_COLD */ virtual ~NETLIB_NAME(frontier)() {}

protected:

	ATTR_COLD void start()
	{
		register_input("I", m_I);
		register_output("Q", m_Q);
	}

	ATTR_COLD void reset()
	{
	}

	ATTR_HOT void update()
	{
		OUTANALOG(m_Q, INPANALOG(m_I));
	}

private:
	netlist_analog_input_t m_I;
	netlist_analog_output_t m_Q;

};

// -----------------------------------------------------------------------------
// nld_res_sw
// -----------------------------------------------------------------------------

class NETLIB_NAME(res_sw) : public netlist_device_t
{
public:
	ATTR_COLD NETLIB_NAME(res_sw)()
			: netlist_device_t() { }

	/* ATTR_COLD */ virtual ~NETLIB_NAME(res_sw)() {}

	netlist_param_double_t m_RON;
	netlist_param_double_t m_ROFF;
	netlist_logic_input_t m_I;
	NETLIB_NAME(R) m_R;

protected:

	ATTR_COLD void start();
	ATTR_COLD void reset();
	ATTR_HOT void update();
	ATTR_HOT void update_param();

private:
	UINT8 m_last_state;
};

// -----------------------------------------------------------------------------
// nld_base_proxy
// -----------------------------------------------------------------------------

class nld_base_proxy : public netlist_device_t
{
public:
	ATTR_COLD nld_base_proxy(netlist_logic_t &inout_proxied, netlist_core_terminal_t &proxy_inout)
			: netlist_device_t()
	{
		m_logic_family = inout_proxied.logic_family();
		m_term_proxied = &inout_proxied;
		m_proxy_term = &proxy_inout;
	}

	/* ATTR_COLD */ virtual ~nld_base_proxy() {}

	ATTR_COLD netlist_logic_t &term_proxied() const { return *m_term_proxied; }
	ATTR_COLD netlist_core_terminal_t &proxy_term() const { return *m_proxy_term; }

protected:

	/* ATTR_COLD */ virtual const netlist_logic_family_desc_t &logic_family() const
	{
		return *m_logic_family;
	}

private:
	const netlist_logic_family_desc_t *m_logic_family;
	netlist_logic_t *m_term_proxied;
	netlist_core_terminal_t *m_proxy_term;
};

// -----------------------------------------------------------------------------
// nld_a_to_d_proxy
// -----------------------------------------------------------------------------

class nld_a_to_d_proxy : public nld_base_proxy
{
public:
	ATTR_COLD nld_a_to_d_proxy(netlist_logic_input_t &in_proxied)
			: nld_base_proxy(in_proxied, m_I)
	{
	}

	/* ATTR_COLD */ virtual ~nld_a_to_d_proxy() {}

	netlist_analog_input_t m_I;
	netlist_logic_output_t m_Q;

protected:
	ATTR_COLD void start()
	{
		register_input("I", m_I);
		register_output("Q", m_Q);
	}

	ATTR_COLD void reset()
	{
	}

	ATTR_HOT void update()
	{
		if (m_I.Q_Analog() > logic_family().m_high_thresh_V)
			OUTLOGIC(m_Q, 1, NLTIME_FROM_NS(1));
		else if (m_I.Q_Analog() < logic_family().m_low_thresh_V)
			OUTLOGIC(m_Q, 0, NLTIME_FROM_NS(1));
		else
		{
			// do nothing
		}
	}
private:
};

// -----------------------------------------------------------------------------
// nld_base_d_to_a_proxy
// -----------------------------------------------------------------------------

class nld_base_d_to_a_proxy : public nld_base_proxy
{
public:
	ATTR_COLD nld_base_d_to_a_proxy(netlist_logic_output_t &out_proxied, netlist_core_terminal_t &proxy_out)
			: nld_base_proxy(out_proxied, proxy_out)
	{
	}

	/* ATTR_COLD */ virtual ~nld_base_d_to_a_proxy() {}

	/* ATTR_COLD */ virtual netlist_logic_input_t &in() { return m_I; }

protected:
	/* ATTR_COLD */ virtual void start()
	{
		register_input("I", m_I);
	}

	netlist_logic_input_t m_I;

private:
};

class nld_d_to_a_proxy : public nld_base_d_to_a_proxy
{
public:
	ATTR_COLD nld_d_to_a_proxy(netlist_logic_output_t &out_proxied)
	: nld_base_d_to_a_proxy(out_proxied, m_RV.m_P)
	, m_RV(TWOTERM)
	, m_last_state(-1)
	, m_is_timestep(false)
	{
	}

	/* ATTR_COLD */ virtual ~nld_d_to_a_proxy() {}

protected:
	/* ATTR_COLD */ virtual void start();

	/* ATTR_COLD */ virtual void reset();

	ATTR_HOT void update();

private:
	netlist_analog_output_t m_Q;
	nld_twoterm m_RV;
	int m_last_state;
	bool m_is_timestep;
};

#endif /* NLD_SYSTEM_H_ */
