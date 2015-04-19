// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_signal.h
 *
 */

#ifndef NLD_SIGNAL_H_
#define NLD_SIGNAL_H_

#include "../nl_base.h"

// ----------------------------------------------------------------------------------------
// MACROS
// ----------------------------------------------------------------------------------------

#define NETLIB_SIGNAL(_name, _num_input, _check, _invert)                           \
	class NETLIB_NAME(_name) : public net_signal_t<_num_input, _check, _invert>     \
	{                                                                               \
	public:                                                                         \
		ATTR_COLD NETLIB_NAME(_name) ()                                             \
		: net_signal_t<_num_input, _check, _invert>() { }                           \
	}

// ----------------------------------------------------------------------------------------
// net_signal_t
// ----------------------------------------------------------------------------------------

template <int _numdev, int _check, int _invert>
class net_signal_t : public netlist_device_t
{
public:
	net_signal_t()
	: netlist_device_t(), m_active(1)
	{
	}

	ATTR_COLD void start()
	{
		const char *sIN[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };

		register_output("Q", m_Q[0]);
		for (int i=0; i < _numdev; i++)
		{
			register_input(sIN[i], m_i[i]);
		}
		save(NLNAME(m_active));
	}

	ATTR_COLD void reset()
	{
		m_Q[0].initial(1);
		m_active = 1;
	}

	ATTR_HOT void inc_active()
	{
		nl_assert(netlist().use_deactivate());
		if (++m_active == 1)
		{
			update();
		}
	}

	ATTR_HOT void dec_active()
	{
		nl_assert(netlist().use_deactivate());
		if (--m_active == 0)
		{
			for (int i = 0; i< _numdev; i++)
				m_i[i].inactivate();
		}
	}

	virtual void update()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22)};

		// FIXME: this check is needed because update is called during startup as well
		//if (m_active == 0 && netlist().use_deactivate())
		//	return;

		for (int i = 0; i< _numdev; i++)
		{
			this->m_i[i].activate();
			if (INPLOGIC(this->m_i[i]) == _check)
			{
				for (int j = 0; j < i; j++)
					this->m_i[j].inactivate();
				for (int j = i + 1; j < _numdev; j++)
					this->m_i[j].inactivate();

				OUTLOGIC(this->m_Q[0], _check ^ (1 ^ _invert), times[_check ^ (1 ^ _invert)]);// ? 15000 : 22000);
				return;
			}
		}
		OUTLOGIC(this->m_Q[0],_check ^ (_invert), times[_check ^ (_invert)]);// ? 22000 : 15000);
	}

public:
	netlist_ttl_input_t m_i[_numdev];
	netlist_ttl_output_t m_Q[1];
	INT32 m_active;
};

template <int _check, int _invert>
class net_signal_2inp_t: public netlist_device_t
{
public:
	net_signal_2inp_t()
	: netlist_device_t(), m_active(1)
	{
	}

	ATTR_COLD void start()
	{
		register_output("Q", m_Q[0]);
		register_input("A", m_i[0]);
		register_input("B", m_i[1]);

		save(NLNAME(m_active));
	}

	ATTR_COLD void reset()
	{
		m_Q[0].initial(1);
		m_active = 1;
	}

	ATTR_HOT virtual void inc_active()
	{
		nl_assert(netlist().use_deactivate());
		if (++m_active == 1)
		{
			update();
		}
	}

	ATTR_HOT virtual void dec_active()
	{
		nl_assert(netlist().use_deactivate());
		if (--m_active == 0)
		{
			m_i[0].inactivate();
			m_i[1].inactivate();
		}
	}

	ATTR_HOT ATTR_ALIGN void update()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22)};

		// FIXME: this check is needed because update is called during startup as well
		//if (m_active == 0 && netlist().use_deactivate())
		//	return;

		m_i[0].activate();
		m_i[1].activate();

		const UINT8 val = (INPLOGIC(m_i[0]) ^ _check) | ((INPLOGIC(m_i[1]) ^ _check) << 1);
		UINT8 res = _invert ^ 1 ^_check;
		switch (val)
		{
			case 1:
				m_i[0].inactivate();
				break;
			case 2:
				m_i[1].inactivate();
				break;
			case 3:
				res = _invert ^ _check;
				break;
		}
		OUTLOGIC(m_Q[0], res, times[res]);// ? 22000 : 15000);
	}

public:
	netlist_ttl_input_t m_i[2];
	netlist_ttl_output_t m_Q[1];
	INT32 m_active;

};

template <int _check, int _invert>
class net_signal_t<2, _check, _invert> : public net_signal_2inp_t<_check, _invert>
{
public:
	net_signal_t()
	: net_signal_2inp_t<_check, _invert>() { }
};


#endif /* NLD_SIGNAL_H_ */
