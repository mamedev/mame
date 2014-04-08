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

template <int _numdev>
class net_signal_base_t : public netlist_device_t
{
public:
	net_signal_base_t()
	: netlist_device_t(), m_active(1) { }

	ATTR_COLD void start()
	{
		const char *sIN[8] = { "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8" };

		register_output("Q", m_Q);
		for (int i=0; i < _numdev; i++)
		{
			register_input(sIN[i], m_i[i], netlist_input_t::STATE_INP_ACTIVE);
		}
		save(NAME(m_active));
	}

	ATTR_COLD void reset()
	{
		m_Q.initial(1);
		m_active = 1;
	}

#if (USE_DEACTIVE_DEVICE)
	ATTR_HOT void inc_active()
	{
		if (++m_active == 1)
		{
			update();
		}
	}

	ATTR_HOT void dec_active()
	{
		if (--m_active == 0)
		{
			for (int i = 0; i< _numdev; i++)
				m_i[i].inactivate();
		}
	}
#endif

public:
	netlist_ttl_input_t m_i[_numdev];
	netlist_ttl_output_t m_Q;
	INT32 m_active;
};


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

		register_output("Q", m_Q);
		for (int i=0; i < _numdev; i++)
		{
			register_input(sIN[i], m_i[i]);
		}
		save(NAME(m_active));
	}

	ATTR_COLD void reset()
	{
		m_Q.initial(1);
		m_active = 1;
	}

#if (USE_DEACTIVE_DEVICE)
	ATTR_HOT void inc_active()
	{
		if (++m_active == 1)
		{
			update();
		}
	}

	ATTR_HOT void dec_active()
	{
		if (--m_active == 0)
		{
			for (int i = 0; i< _numdev; i++)
				m_i[i].inactivate();
		}
	}
#endif

	virtual void update()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };

		// FIXME: this check is needed because update is called during startup as well
		if (UNEXPECTED(USE_DEACTIVE_DEVICE && m_active == 0))
			return;

		for (int i = 0; i< _numdev; i++)
		{
			this->m_i[i].activate();
			if (INPLOGIC(this->m_i[i]) == _check)
			{
				for (int j = 0; j < i; j++)
					this->m_i[j].inactivate();
				for (int j = i + 1; j < _numdev; j++)
					this->m_i[j].inactivate();

				OUTLOGIC(this->m_Q, _check ^ (1 ^ _invert), times[_check]);// ? 15000 : 22000);
				return;
			}
		}
		OUTLOGIC(this->m_Q,_check ^ (_invert), times[1-_check]);// ? 22000 : 15000);
	}

public:
	netlist_ttl_input_t m_i[_numdev];
	netlist_ttl_output_t m_Q;
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
		register_output("Q", m_Q);
		register_input("A", m_i[0]);
		register_input("B", m_i[1]);

		save(NAME(m_active));
	}

	ATTR_COLD void reset()
	{
		m_Q.initial(1);
		m_active = 1;
	}

#if (USE_DEACTIVE_DEVICE)
	ATTR_HOT virtual void inc_active()
	{
		if (++m_active == 1)
		{
			update();
		}
	}

	ATTR_HOT virtual void dec_active()
	{
		if (--m_active == 0)
		{
			m_i[0].inactivate();
			m_i[1].inactivate();
		}
	}
#endif

	ATTR_HOT ATTR_ALIGN void update()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22)};

		// FIXME: this check is needed because update is called during startup as well
		if (UNEXPECTED(USE_DEACTIVE_DEVICE && m_active == 0))
			return;

		m_i[0].activate();
		m_i[1].activate();
#if 0
		UINT8 res = _invert ^ 1 ^_check;
		if (INPLOGIC(m_i[0]) ^ _check)
		{
			if (INPLOGIC(m_i[1]) ^ _check)
			{
				res = _invert ^ _check;
			}
			else
				m_i[0].inactivate();
		} else {
			if (INPLOGIC(m_i[1]) ^ _check)
				m_i[1].inactivate();
		}
		OUTLOGIC(m_Q, res, times[res & 1]);// ? 22000 : 15000);
#else
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
		OUTLOGIC(m_Q, res, times[res]);// ? 22000 : 15000);
#endif
	}

public:
	netlist_ttl_input_t m_i[2];
	netlist_ttl_output_t m_Q;
	INT32 m_active;

};

// The following did not improve performance
#if 0

template <UINT8 _check, UINT8 _invert>
class net_signal_t<3, _check, _invert> : public netlist_device_t
{
public:
	net_signal_t() : netlist_device_t(), m_active(1) { }

	ATTR_COLD void start()
	{
		const char *sIN[3] = { "I1", "I2", "I3" };

		register_output("Q", m_Q);
		for (int i=0; i < 3; i++)
		{
			register_input(sIN[i], m_i[i], netlist_input_t::STATE_INP_ACTIVE);
		}
		//m_Q.initial(1);
	}

	ATTR_HOT ATTR_ALIGN void update()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };
		//const UINT8 res_tab[4] = {1, 1, 1, 0 };
		//const UINT8 ai1[4]     = {0, 1, 0, 0 };
		//const UINT8 ai2[4]     = {1, 0, 1, 0 };

		UINT8 res = _invert ^ 1 ^_check;
		m_i[0].activate();
		if (INPLOGIC(m_i[0]) ^ _check)
		{
			m_i[1].activate();
			if (INPLOGIC(m_i[1]) ^ _check)
			{
				m_i[2].activate();
				if (INPLOGIC(m_i[2]) ^ _check)
				{
					res = _invert ^ _check;
				}
				else
					m_i[1].inactivate();
			}
			else
			{
				if (INPLOGIC(m_i[2]) ^ _check)
					m_i[2].inactivate();
				m_i[0].inactivate();
			}
		} else {
			if (INPLOGIC(m_i[1]) ^ _check)
				m_i[1].inactivate();
		}
		OUTLOGIC(m_Q, res, times[1 - res]);// ? 22000 : 15000);
	}
public:
	netlist_ttl_input_t m_i[3];
	netlist_ttl_output_t m_Q;
	INT8 m_active;

};

#endif


template <int _check, int _invert>
class net_signal_t<2, _check, _invert> : public net_signal_2inp_t<_check, _invert>
{
public:
	net_signal_t()
	: net_signal_2inp_t<_check, _invert>() { }
};


#endif /* NLD_SIGNAL_H_ */
