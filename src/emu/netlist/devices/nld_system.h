// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_system.h
 *
 * netlist devices defined in the core
 */

#ifndef NLD_SYSTEM_H_
#define NLD_SYSTEM_H_

#include "nl_setup.h"
#include "nl_base.h"
#include "nl_factory.h"
#include "analog/nld_twoterm.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

#define TTL_INPUT(_name, _v)                                                   \
		NET_REGISTER_DEV(TTL_INPUT, _name)                                   \
		PARAM(_name.IN, _v)

#define LOGIC_INPUT(_name, _v, _family)                                        \
		NET_REGISTER_DEV(LOGIC_INPUT, _name)                                   \
		PARAM(_name.IN, _v)                                                    \
		PARAM(_name.FAMILY, _family)

#define ANALOG_INPUT(_name, _v)                                                \
		NET_REGISTER_DEV(ANALOG_INPUT, _name)                                  \
		PARAM(_name.IN, _v)

#define MAINCLOCK(_name, _freq)                                                \
		NET_REGISTER_DEV(MAINCLOCK, _name)                                     \
		PARAM(_name.FREQ, _freq)

#define CLOCK(_name, _freq)                                                    \
		NET_REGISTER_DEV(CLOCK, _name)                                         \
		PARAM(_name.FREQ, _freq)

#define EXTCLOCK(_name, _freq, _pattern)                                       \
		NET_REGISTER_DEV(EXTCLOCK, _name)                                      \
		PARAM(_name.FREQ, _freq)                                               \
		PARAM(_name.PATTERN, _pattern)

#define GNDA()                                                                 \
		NET_REGISTER_DEV(GNDA, GND)

#define DUMMY_INPUT(_name)                                                     \
		NET_REGISTER_DEV(DUMMY_INPUT, _name)

//FIXME: Usage discouraged, use OPTIMIZE_FRONTIER instead
#define FRONTIER_DEV(_name, _IN, _G, _OUT)                                     \
		NET_REGISTER_DEV(FRONTIER_DEV, _name)                                      \
		NET_C(_IN, _name.I)                                                    \
		NET_C(_G,  _name.G)                                                    \
		NET_C(_OUT, _name.Q)

#define OPTIMIZE_FRONTIER(_attach, _r_in, _r_out)                              \
		setup.register_frontier(# _attach, _r_in, _r_out);

#define RES_SWITCH(_name, _IN, _P1, _P2)                                       \
		NET_REGISTER_DEV(RES_SWITCH, _name)                                        \
		NET_C(_IN, _name.I)                                                    \
		NET_C(_P1, _name.1)                                                    \
		NET_C(_P2, _name.2)

/* Default device to hold netlist parameters */
#define PARAMETERS(_name)                                                      \
		NET_REGISTER_DEV(PARAMETERS, _name)

#define AFUNC(_name, _N, _F)                                                   \
		NET_REGISTER_DEV(AFUNC, _name)                                      \
		PARAM(_name.N, _N)                                                     \
		PARAM(_name.FUNC, _F)

NETLIB_NAMESPACE_DEVICES_START()

// -----------------------------------------------------------------------------
// netlistparams
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(netlistparams,
public:
		param_logic_t m_use_deactivate;
);

// -----------------------------------------------------------------------------
// mainclock
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(mainclock,
public:
	logic_output_t m_Q;

	param_double_t m_freq;
	netlist_time m_inc;

	ATTR_HOT inline static void mc_update(logic_net_t &net);
);

// -----------------------------------------------------------------------------
// clock
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(clock,
	logic_input_t m_feedback;
	logic_output_t m_Q;

	param_double_t m_freq;
	netlist_time m_inc;
);

// -----------------------------------------------------------------------------
// extclock
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(extclock,
	logic_input_t m_feedback;
	logic_output_t m_Q;

	param_double_t m_freq;
	param_str_t m_pattern;
	param_double_t m_offset;

	UINT8 m_cnt;
	UINT8 m_size;
	netlist_time m_off;
	netlist_time m_inc[32];
);

// -----------------------------------------------------------------------------
// Special support devices ...
// -----------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(logic_input,

	virtual void stop();
	logic_output_t m_Q;

	param_logic_t m_IN;
	param_model_t m_FAMILY;
);

NETLIB_DEVICE_WITH_PARAMS(analog_input,
	analog_output_t m_Q;

	param_double_t m_IN;
);

// -----------------------------------------------------------------------------
// nld_gnd
// -----------------------------------------------------------------------------

class NETLIB_NAME(gnd) : public device_t
{
public:
	NETLIB_NAME(gnd)()
			: device_t(GND) { }

	virtual ~NETLIB_NAME(gnd)() {}

protected:

	void start()
	{
		register_output("Q", m_Q);
	}

	void reset()
	{
	}

	void update()
	{
		OUTANALOG(m_Q, 0.0);
	}

private:
	analog_output_t m_Q;

};

// -----------------------------------------------------------------------------
// nld_dummy_input
// -----------------------------------------------------------------------------

class NETLIB_NAME(dummy_input) : public device_t
{
public:
	NETLIB_NAME(dummy_input)()
			: device_t(DUMMY) { }

	virtual ~NETLIB_NAME(dummy_input)() {}

protected:

	void start()
	{
		register_input("I", m_I);
	}

	void reset()
	{
	}

	void update()
	{
	}

private:
	analog_input_t m_I;

};

// -----------------------------------------------------------------------------
// nld_frontier
// -----------------------------------------------------------------------------

class NETLIB_NAME(frontier) : public device_t
{
public:
	NETLIB_NAME(frontier)()
			: device_t(DUMMY) { }

	virtual ~NETLIB_NAME(frontier)() {}

protected:

	void start()
	{
		register_param("RIN", m_p_RIN, 1.0e6);
		register_param("ROUT", m_p_ROUT, 50.0);

		register_input("_I", m_I);
		register_terminal("I",m_RIN.m_P);
		register_terminal("G",m_RIN.m_N);
		connect_late(m_I, m_RIN.m_P);

		register_output("_Q", m_Q);
		register_terminal("_OP",m_ROUT.m_P);
		register_terminal("Q",m_ROUT.m_N);
		connect_late(m_Q, m_ROUT.m_P);
	}

	void reset()
	{
		m_RIN.set(1.0 / m_p_RIN.Value(),0,0);
		m_ROUT.set(1.0 / m_p_ROUT.Value(),0,0);
	}

	void update()
	{
		OUTANALOG(m_Q, INPANALOG(m_I));
	}

private:
	NETLIB_NAME(twoterm) m_RIN;
	NETLIB_NAME(twoterm) m_ROUT;
	analog_input_t m_I;
	analog_output_t m_Q;

	param_double_t m_p_RIN;
	param_double_t m_p_ROUT;
};

/* -----------------------------------------------------------------------------
 * nld_function
 *
 * FIXME: Currently a proof of concept to get congo bongo working
 * ----------------------------------------------------------------------------- */

class NETLIB_NAME(function) : public device_t
{
public:
	NETLIB_NAME(function)()
			: device_t() { }

	virtual ~NETLIB_NAME(function)() {}

protected:

	void start();
	void reset();
	void update();

private:

	enum rpn_cmd
	{
		ADD,
		MULT,
		SUB,
		DIV,
		PUSH_CONST,
		PUSH_INPUT
	};

	struct rpn_inst
	{
		rpn_inst() : m_cmd(ADD), m_param(0.0) { }
		rpn_cmd m_cmd;
		nl_double m_param;
	};

	param_int_t m_N;
	param_str_t m_func;
	analog_output_t m_Q;
	analog_input_t m_I[10];

	plist_t<rpn_inst> m_precompiled;
};

// -----------------------------------------------------------------------------
// nld_res_sw
// -----------------------------------------------------------------------------

class NETLIB_NAME(res_sw) : public device_t
{
public:
	NETLIB_NAME(res_sw)()
			: device_t() { }

	virtual ~NETLIB_NAME(res_sw)() {}

	param_double_t m_RON;
	param_double_t m_ROFF;
	logic_input_t m_I;
	NETLIB_NAME(R) m_R;

protected:

	void start();
	void reset();
	ATTR_HOT void update();
	ATTR_HOT void update_param();

private:
	UINT8 m_last_state;
};

// -----------------------------------------------------------------------------
// nld_base_proxy
// -----------------------------------------------------------------------------

class nld_base_proxy : public device_t
{
public:
	nld_base_proxy(logic_t *inout_proxied, core_terminal_t *proxy_inout)
			: device_t()
	{
		m_logic_family = inout_proxied->logic_family();
		m_term_proxied = inout_proxied;
		m_proxy_term = proxy_inout;
	}

	virtual ~nld_base_proxy() {}

	logic_t &term_proxied() const { return *m_term_proxied; }
	core_terminal_t &proxy_term() const { return *m_proxy_term; }

protected:

	virtual const logic_family_desc_t &logic_family() const
	{
		return *m_logic_family;
	}

private:
	const logic_family_desc_t *m_logic_family;
	logic_t *m_term_proxied;
	core_terminal_t *m_proxy_term;
};

// -----------------------------------------------------------------------------
// nld_a_to_d_proxy
// -----------------------------------------------------------------------------

class nld_a_to_d_proxy : public nld_base_proxy
{
public:
	nld_a_to_d_proxy(logic_input_t *in_proxied)
			: nld_base_proxy(in_proxied, &m_I)
	{
	}

	virtual ~nld_a_to_d_proxy() {}

	analog_input_t m_I;
	logic_output_t m_Q;

protected:
	void start()
	{
		register_input("I", m_I);
		register_output("Q", m_Q);
	}

	void reset()
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
	virtual ~nld_base_d_to_a_proxy() {}

	virtual logic_input_t &in() { return m_I; }

protected:
	nld_base_d_to_a_proxy(logic_output_t *out_proxied, core_terminal_t &proxy_out)
			: nld_base_proxy(out_proxied, &proxy_out)
	{
	}

	virtual void start()
	{
		register_input("I", m_I);
	}

	logic_input_t m_I;

private:
};

class nld_d_to_a_proxy : public nld_base_d_to_a_proxy
{
public:
	nld_d_to_a_proxy(logic_output_t *out_proxied)
	: nld_base_d_to_a_proxy(out_proxied, m_RV.m_P)
	, m_RV(TWOTERM)
	, m_last_state(-1)
	, m_is_timestep(false)
	{
	}

	virtual ~nld_d_to_a_proxy() {}

protected:
	virtual void start();

	virtual void reset();

	ATTR_HOT void update();

private:
	analog_output_t m_Q;
	nld_twoterm m_RV;
	int m_last_state;
	bool m_is_timestep;
};


class factory_lib_entry_t : public base_factory_t
{
	P_PREVENT_COPYING(factory_lib_entry_t)
public:

	ATTR_COLD factory_lib_entry_t(setup_t &setup, const pstring &name, const pstring &classname,
			const pstring &def_param)
	: base_factory_t(name, classname, def_param), m_setup(setup) {  }

	class dummy : public device_t
	{
	public:
		dummy(const pstring &dev_name) : device_t(), m_dev_name(dev_name) { }
	protected:
		virtual void init(netlist_t &anetlist, const pstring &aname)
		{
			anetlist.setup().namespace_push(aname);
			anetlist.setup().include(m_dev_name);
			anetlist.setup().namespace_pop();
		}
		void start() { }
		void reset() { }
		void update() { }

		pstring m_dev_name;
	};

	ATTR_COLD device_t *Create()
	{
		device_t *r = palloc(dummy(this->name()));
		return r;
	}

private:
	setup_t &m_setup;
};


NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_SYSTEM_H_ */
