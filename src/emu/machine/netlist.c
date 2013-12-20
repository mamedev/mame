/***************************************************************************

    netlist.c

    Discrete netlist implementation.

****************************************************************************

    Couriersud reserves the right to license the code under a less restrictive
    license going forward.

    Copyright Nicola Salmoria and the MAME team
    All rights reserved.

    Redistribution and use of this code or any derivative works are permitted
    provided that the following conditions are met:

    * Redistributions may not be sold, nor may they be used in a commercial
    product or activity.

    * Redistributions that are modified from the original source must include the
    complete source code, including the source code for all components used by a
    binary built from the modified sources. However, as a special exception, the
    source code distributed need not include anything that is normally distributed
    (in either source or binary form) with the major components (compiler, kernel,
    and so on) of the operating system on which the executable runs, unless that
    component itself accompanies the executable.

    * Redistributions must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "emu.h"
#include "netlist.h"
#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"
#include "netlist/devices/net_lib.h"

//#define LOG_DEV_CALLS(x)   printf x
#define LOG_DEV_CALLS(x)   do { } while (0)

// ----------------------------------------------------------------------------------------
// netlist_mame_device
// ----------------------------------------------------------------------------------------

const device_type NETLIST = &device_creator<netlist_mame_device>;

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NETLIST, "netlist", tag, owner, clock, "netlist_mame", __FILE__),
		device_execute_interface(mconfig, *this),
        //device_state_interface(mconfig, *this),
		m_device_start_list(100),
		m_netlist(NULL),
		m_setup(NULL),
		m_setup_func(NULL),
		m_icount(0)
{
}

void netlist_mame_device::static_set_constructor(device_t &device, void (*setup_func)(netlist_setup_t &))
{
	netlist_mame_device &netlist = downcast<netlist_mame_device &>(device);
	netlist.m_setup_func = setup_func;
	LOG_DEV_CALLS(("static_set_constructor\n"));
}

void netlist_mame_device::device_config_complete()
{
    LOG_DEV_CALLS(("device_config_complete\n"));
}

void netlist_mame_device::device_start()
{
    LOG_DEV_CALLS(("device_start\n"));

	m_netlist = global_alloc_clear(netlist_mame_t(*this));
	m_setup = global_alloc_clear(netlist_setup_t(*m_netlist));
    m_netlist->init_object(*m_netlist, "netlist");
    m_setup->init();

    m_netlist->set_clock_freq(this->clock());

    // register additional devices

	m_setup->factory().register_device<nld_analog_callback>( "NETDEV_CALLBACK", "nld_analog_callback");

	m_setup_func(*m_setup);

	m_setup->start_devices();
	m_setup->resolve_inputs();

    bool allok = true;
    for (device_start_list_t::entry_t *ods = m_device_start_list.first(); ods != NULL; ods = m_device_start_list.next(ods))
        allok &= ods->object()->OnDeviceStart();

    if (!allok)
        m_netlist->xfatalerror("required elements not found\n");

	save_state();

	// set our instruction counter
	m_icountptr = &m_icount;
}

void netlist_mame_device::device_reset()
{
    LOG_DEV_CALLS(("device_reset\n"));
	m_netlist->reset();
}

void netlist_mame_device::device_stop()
{
    LOG_DEV_CALLS(("device_stop\n"));
	m_setup->print_stats();

	global_free(m_setup);
	m_setup = NULL;
	global_free(m_netlist);
	m_netlist = NULL;
}

ATTR_COLD void netlist_mame_device::device_post_load()
{
    LOG_DEV_CALLS(("device_post_load\n"));
    m_netlist->queue().clear();
    NL_VERBOSE_OUT(("current time %f qsize %d\n", m_netlist->time().as_double(), qsize));
    for (int i = 0; i < qsize; i++ )
    {
        netlist_net_t *n = m_netlist->find_net(qtemp[i].m_name);
        NL_VERBOSE_OUT(("Got %s ==> %p\n", qtemp[i].m_name, n));
        NL_VERBOSE_OUT(("schedule time %f (%f)\n", n->time().as_double(), qtemp[i].m_time.as_double()));
        m_netlist->queue().push(netlist_base_t::queue_t::entry_t(qtemp[i].m_time, *n));
    }
}

ATTR_COLD void netlist_mame_device::device_pre_save()
{
    LOG_DEV_CALLS(("device_pre_save\n"));

    qsize = m_netlist->queue().count();
    NL_VERBOSE_OUT(("current time %f qsize %d\n", m_netlist->time().as_double(), qsize));
    for (int i = 0; i < qsize; i++ )
    {
        qtemp[i].m_time =  m_netlist->queue().listptr()[i].time();
        const char *p = m_netlist->queue().listptr()[i].object().name().cstr();
        int n = MIN(63, strlen(p));
        strncpy(qtemp[i].m_name, p, n);
        qtemp[i].m_name[n] = 0;
    }
#if 0

    netlist_time *nlt = (netlist_time *) ;
    netlist_base_t::queue_t::entry_t *p = m_netlist->queue().listptr()[i];
    netlist_time *nlt = (netlist_time *) p->time_ptr();
    save_pointer(nlt->get_internaltype_ptr(), "queue", 1, i);
#endif
}

void netlist_mame_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}



ATTR_COLD void netlist_mame_device::save_state()
{
    for (netlist_setup_t::save_entry_list_t::entry_t *p = setup().m_save.first(); p != NULL; p = setup().m_save.next(p))
    {
        netlist_setup_t::save_entry_t *s = p->object();
        NL_VERBOSE_OUT(("saving state for %s\n", s->m_name.cstr()));
        switch (s->m_dt)
        {
            case DT_DOUBLE:
                save_pointer((double *) s->m_ptr, s->m_name, 1);
                break;
            case DT_INT64:
                save_pointer((INT64 *) s->m_ptr, s->m_name, 1);
                break;
            case DT_INT8:
                save_pointer((INT8 *) s->m_ptr, s->m_name, 1);
                break;
            case DT_INT:
                save_pointer((int *) s->m_ptr, s->m_name, 1);
                break;
            case DT_BOOLEAN:
                save_pointer((bool *) s->m_ptr, s->m_name, 1);
                break;
#if 0
            case DT_NLTIME:
                {
                    netlist_time *nlt = (netlist_time *) s->m_ptr;
                    //save_pointer((netlist_time::INTERNALTYPE *) s->m_ptr, s->m_name, 1);
                    //save_pointer(nlt->get_internaltype_ptr(), s->m_name, 1);
                    save_item(*nlt->get_internaltype_ptr(), s->m_name.cstr());
                }
                break;
#endif
            case NOT_SUPPORTED:
            default:
                m_netlist->xfatalerror("found unsupported save element %s\n", s->m_name.cstr());
                break;
        }
    }

    // handle the queue

    save_item(NAME(qsize));
    for (int i = 0; i < m_netlist->queue().capacity(); i++ )
    {
        save_pointer(qtemp[i].m_time.get_internaltype_ptr(), "queue_time", 1, i);
        save_pointer(qtemp[i].m_name, "queue_name", sizeof(qtemp[i].m_name), i);

    }
#if 0

    netlist_time *nlt = (netlist_time *) ;
    netlist_base_t::queue_t::entry_t *p = m_netlist->queue().listptr()[i];
    netlist_time *nlt = (netlist_time *) p->time_ptr();
    save_pointer(nlt->get_internaltype_ptr(), "queue", 1, i);
#endif
}

ATTR_COLD UINT64 netlist_mame_device::execute_clocks_to_cycles(UINT64 clocks) const
{
	return clocks;
}

ATTR_COLD UINT64 netlist_mame_device::execute_cycles_to_clocks(UINT64 cycles) const
{
	return cycles;
}

ATTR_HOT void netlist_mame_device::execute_run()
{
	//bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	// debugging
	//m_ppc = m_pc; // copy PC to previous PC
	//if (check_debugger)
	//  debugger_instruction_hook(this, 0); //m_pc);

	m_netlist->process_queue(m_icount);
}
