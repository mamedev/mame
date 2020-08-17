// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_tristate.cpp
 *
 */

#include "nld_tristate.h"
#include "netlist/nl_base.h"

// FIXME: netlist now supports proper tristate outputs. All of this is legacy
//        now and needs to be removed at some time.

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(tristate)
	{
		NETLIB_CONSTRUCTOR(tristate)
		, m_CEQ(*this, { "CEQ1", "CEQ2" }, NETLIB_DELEGATE(inputs))
		, m_D(*this, { "D1", "D2" }, NETLIB_DELEGATE(inputs))
		, m_Q(*this, "Q")
		{
		}

	protected:
		object_array_t<logic_input_t, 2> m_CEQ;
		object_array_t<logic_input_t, 2> m_D;
		logic_output_t m_Q;
	private:
		NETLIB_HANDLERI(inputs)
		{
			unsigned q = 0;
			if (!m_CEQ[0]())
				q |= m_D[0]();
			if (!m_CEQ[1]())
				q |= m_D[1]();

			m_Q.push(q, NLTIME_FROM_NS(1));
		}
	};

	NETLIB_OBJECT(tristate3)
	{
		NETLIB_CONSTRUCTOR(tristate3)
		, m_CEQ(*this, { "CEQ1", "CEQ2", "CEQ3" }, NETLIB_DELEGATE(inputs) )
		, m_D(*this, { "D1", "D2", "D3" }, NETLIB_DELEGATE(inputs) )
		, m_Q(*this, "Q")
		{
		}

	protected:
		object_array_t<logic_input_t, 3> m_CEQ;
		object_array_t<logic_input_t, 3> m_D;
		logic_output_t m_Q;
	private:
		NETLIB_HANDLERI(inputs)
		{
			unsigned q = 0;
			if (!m_CEQ[0]())
				q |= m_D[0]();
			if (!m_CEQ[1]())
				q |= m_D[1]();
			if (!m_CEQ[2]())
				q |= m_D[2]();

			m_Q.push(q, NLTIME_FROM_NS(1));
		}
	};

	NETLIB_DEVICE_IMPL(tristate,  "TTL_TRISTATE",  "+CEQ1,+D1,+CEQ2,+D2")
	NETLIB_DEVICE_IMPL(tristate3, "TTL_TRISTATE3", "")

	} //namespace devices
} // namespace netlist
