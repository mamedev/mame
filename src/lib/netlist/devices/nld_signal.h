// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_signal.h
 *
 */

#ifndef NLD_SIGNAL_H_
#define NLD_SIGNAL_H_

#include "nl_base.h"

// ----------------------------------------------------------------------------------------
// MACROS
// ----------------------------------------------------------------------------------------

#define NETLIB_SIGNAL(_name, _num_input, _check, _invert)                           \
	class NETLIB_NAME(_name) : public net_signal_t<_num_input, _check, _invert>     \
	{                                                                               \
	public:                                                                         \
		NETLIB_NAME(_name) ()                                             \
		: net_signal_t<_num_input, _check, _invert>() { }                           \
	}

NETLIB_NAMESPACE_DEVICES_START()

// ----------------------------------------------------------------------------------------
// net_signal_t
// ----------------------------------------------------------------------------------------

template <int _numdev, int _check, int _invert>
class net_signal_t : public device_t
{
public:
	net_signal_t()
	: device_t(), m_active(1)
	{
	}

	void start() override
	{
		const char *sIN[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };

		register_output("Q", m_Q[0]);
		for (int i=0; i < _numdev; i++)
		{
			register_input(sIN[i], m_I[i]);
		}
		save(NLNAME(m_active));
	}

	void reset() override
	{
		m_Q[0].initial(0);
		m_active = 1;
	}

	ATTR_HOT inline netlist_sig_t process()
	{
		for (int i = 0; i< _numdev; i++)
		{
			this->m_I[i].activate();
			if (INPLOGIC(this->m_I[i]) == _check)
			{
				for (int j = 0; j < i; j++)
					this->m_I[j].inactivate();
				for (int j = i + 1; j < _numdev; j++)
					this->m_I[j].inactivate();
				return _check ^ (1 ^ _invert);
			}
		}
		return _check ^ _invert;
	}

	ATTR_HOT virtual void inc_active() override
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22)};
		nl_assert(netlist().use_deactivate());
		if (++m_active == 1)
		{
			// FIXME: need to activate before time is accessed !
			netlist_time mt = netlist_time::zero;
			for (int i = 0; i< _numdev; i++)
			{
				if (this->m_I[i].net().time() > mt)
					mt = this->m_I[i].net().time();
			}
			netlist_sig_t r = process();
			m_Q[0].net().set_Q_time(r, mt + times[r]);
		}
	}

	ATTR_HOT virtual void dec_active() override
	{
		nl_assert(netlist().use_deactivate());
		if (--m_active == 0)
		{
			for (int i = 0; i< _numdev; i++)
				m_I[i].inactivate();
		}
	}

	virtual void update() override
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22)};

		netlist_sig_t r = process();
		OUTLOGIC(this->m_Q[0], r, times[r]);
	}

public:
	logic_input_t m_I[_numdev];
	logic_output_t m_Q[1];
	INT32 m_active;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_SIGNAL_H_ */
