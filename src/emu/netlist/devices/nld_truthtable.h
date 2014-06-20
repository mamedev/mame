/*
 * nld_truthtable.h
 *
 *  Created on: 19 Jun 2014
 *      Author: andre
 */

#ifndef NLD_TRUTHTABLE_H_
#define NLD_TRUTHTABLE_H_

#include "../nl_base.h"

template<int m_NI, int m_NO>
class nld_truthtable_t : public netlist_device_t
{
public:

    static const int m_size = (1 << (2 * m_NI + m_NO));

    nld_truthtable_t()
    : netlist_device_t(), m_active(1)
    {
    }

    ATTR_COLD virtual void start()
    {
    }

    ATTR_COLD void help(UINT32 outs[], int cur, nl_util::pstring_list list, UINT64 state, UINT64 ignore, UINT16 val)
    {
        pstring elem = list[cur];
        int start = 0;
        int end = 0;
        int ign = 0;

        if (elem.equals("0"))
        {
            start = 0;
            end = 0;
        }
        else if (elem.equals("1"))
        {
            start = 1;
            end = 1;
        }
        else if (elem.equals("X"))
        {
            start = 0;
            end = 1;
            ign = 1;
        }
        for (int i = start; i <= end; i++)
        {
            const UINT64 nstate = state | (i << cur);
            const UINT64 nignore = ignore | (ign << cur);

            if (cur < list.count() - 1)
            {
                help(outs, cur + 1, list, nstate, nignore, val);
            }
            else
            {
                // cutoff previous inputs and outputs for ignore
                outs[nstate] = val | ((nignore & ((1 << m_NI)-1)) << m_NO);
            }
        }
    }

    ATTR_COLD void setup_tt(const char **truthtable, UINT32 outs[])
    {
        pstring ttline = pstring(truthtable[0]);
        truthtable++;
        {
            nl_util::pstring_list io = nl_util::split(ttline,"|");
            // checks
            assert(io.count() == 2);
            nl_util::pstring_list inout = nl_util::split(io[0], ",");
            assert(inout.count() == 2 * m_NI + m_NO);
            nl_util::pstring_list out = nl_util::split(io[1], ",");
            assert(out.count() == m_NO);

            for (int i=0; i < m_NI; i++)
            {
                register_input(inout[i], m_i[i]);
            }
            for (int i=0; i < m_NO; i++)
            {
                register_output(out[i], m_Q[i]);
            }
            //save(NAME(m_active));
            ttline = pstring(truthtable[0]);
            truthtable++;
        }

        for (int j=0; j < m_size; j++)
            outs[j] = -1;

        while (!ttline.equals(""))
        {
            nl_util::pstring_list io = nl_util::split(ttline,"|");
            // checks
            assert(io.count() == 2);
            nl_util::pstring_list inout = nl_util::split(io[0], ",");
            assert(inout.count() == 2 * m_NI + m_NO);
            nl_util::pstring_list out = nl_util::split(io[1], ",");
            assert(out.count() == m_NO);

            UINT16 val = 0;
            for (int j=0; j<m_NO; j++)
                if (out[j].equals("1"))
                    val = val | (1 << j);

            help(outs, 0, inout, 0 , 0 , val);
            ttline = pstring(truthtable[0]);
            truthtable++;
        }
        for (int j=0; j < m_size; j++)
            printf("%05x %04x %04x\n", j, outs[j] & ((1 << m_NO)-1), outs[j] >> m_NO);
        m_ttp = outs;
    }

    ATTR_COLD void reset()
    {
        //m_Q.initial(1);
        m_active = 1;
        m_last_state = 0;
    }

    ATTR_HOT ATTR_ALIGN void update()
    {
        const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22)};

        // FIXME: this check is needed because update is called during startup as well
        if (UNEXPECTED(USE_DEACTIVE_DEVICE && m_active == 0))
            return;

        UINT32 state = 0;
        for (int i=0; i< m_NI; i++)
        {
            m_i[i].activate();
            state = state | (INPLOGIC(m_i[i]) << i);
        }

        const UINT32 nstate = state | (m_last_state << m_NI);
        const UINT32 out = m_ttp[nstate] & ((1 << m_NO) - 1);
        const UINT32 ign = m_ttp[nstate] >> m_NO;

        for (int i=0; i< m_NI; i++)
            if (ign & (1 << i))
                m_i[i].inactivate();

        for (int i=0; i<m_NO; i++)
            OUTLOGIC(m_Q[i], (out >> i) & 1, times[(out >> i) & 1]);// ? 22000 : 15000);
        m_last_state = (state << m_NO) | out;

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
            for (int i = 0; i< m_NI; i++)
                m_i[i].inactivate();
        }
    }
#endif

public:
    netlist_ttl_input_t m_i[m_NI];
    netlist_ttl_output_t m_Q[m_NO];

    UINT32 *m_ttp;
    INT32 m_active;
    UINT32 m_last_state;
};

#endif /* NLD_TRUTHTABLE_H_ */
