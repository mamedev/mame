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
#include "debugger.h"

//#define LOG_DEV_CALLS(x)   printf x
#define LOG_DEV_CALLS(x)   do { } while (0)

// ----------------------------------------------------------------------------------------
// netlist_mame_device
// ----------------------------------------------------------------------------------------

const device_type NETLIST = &device_creator<netlist_mame_device>;

static ADDRESS_MAP_START(program_dummy, AS_PROGRAM, 8, netlist_mame_device)
    AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NETLIST, "netlist", tag, owner, clock, "netlist_mame", __FILE__),
		device_execute_interface(mconfig, *this),
        device_state_interface(mconfig, *this),
        device_disasm_interface(mconfig, *this),
        device_memory_interface(mconfig, *this),
		m_device_start_list(100),
        m_program_config("program", ENDIANNESS_LITTLE, 8, 12, 0, ADDRESS_MAP_NAME(program_dummy)),
		m_netlist(NULL),
		m_setup(NULL),
		m_setup_func(NULL),
		m_icount(0),
		m_genPC(0)
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

	// State support

    state_add(STATE_GENPC, "curpc", m_genPC).noshow();

    for (int i=0; i < m_netlist->m_nets.count(); i++)
    {
        netlist_net_t *n = m_netlist->m_nets[i];
        if (n->isRailNet())
        {
            state_add(i*2, n->name(), n->m_cur.Q);
        }
        else
        {
            state_add(i*2+1, n->name(), n->m_cur.Analog).formatstr("%20s");
        }
    }

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

    m_netlist->post_load();
}

ATTR_COLD void netlist_mame_device::device_pre_save()
{
	LOG_DEV_CALLS(("device_pre_save\n"));

	m_netlist->pre_save();
}

void netlist_mame_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}



ATTR_COLD void netlist_mame_device::save_state()
{
    for (pstate_entry_t::list_t::entry_t *p = m_netlist->save_list().first(); p != NULL; p = m_netlist->save_list().next(p))
    {
        pstate_entry_t *s = p->object();
        NL_VERBOSE_OUT(("saving state for %s\n", s->m_name.cstr()));
        switch (s->m_dt)
        {
            case DT_DOUBLE:
                save_pointer((double *) s->m_ptr, s->m_name, s->m_count);
                break;
            case DT_INT64:
                save_pointer((INT64 *) s->m_ptr, s->m_name, s->m_count);
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
            case NOT_SUPPORTED:
            default:
                m_netlist->xfatalerror("found unsupported save element %s\n", s->m_name.cstr());
                break;
        }
    }

}

ATTR_COLD UINT64 netlist_mame_device::execute_clocks_to_cycles(UINT64 clocks) const
{
	return clocks;
}

ATTR_COLD UINT64 netlist_mame_device::execute_cycles_to_clocks(UINT64 cycles) const
{
	return cycles;
}

ATTR_COLD offs_t netlist_mame_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
    //char tmp[16];
    unsigned startpc = pc;
    int relpc = pc - m_genPC;
    //UINT16 opcode = (oprom[pc - startpc] << 8) | oprom[pc+1 - startpc];
    //UINT8 inst = opcode >> 13;

    if (relpc >= 0 && relpc < m_netlist->queue().count())
    {
        //            sprintf(buffer, "%04x %02d %s", pc, relpc, m_netlist->queue()[m_netlist->queue().count() - relpc - 1].object().name().cstr());
        int dpc = m_netlist->queue().count() - relpc - 1;
        sprintf(buffer, "%c %s @%10.7f", (relpc == 0) ? '*' : ' ', m_netlist->queue()[dpc].object().name().cstr(),
                m_netlist->queue()[dpc].time().as_double());
    }
    else
        sprintf(buffer, "%s", "");
    pc+=1;
    return (pc - startpc);
}

ATTR_HOT void netlist_mame_device::execute_run()
{
	bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);
	// debugging
	//m_ppc = m_pc; // copy PC to previous PC
	if (check_debugger)
	{
	    while (m_icount > 0)
	    {
	        int m_temp = 1;
            m_genPC++;
            m_genPC &= 255;
	        debugger_instruction_hook(this, m_genPC);
	        m_netlist->process_queue(m_temp);
	        m_icount -= (1 - m_temp);
	    }
	}
	else
	    m_netlist->process_queue(m_icount);
}

