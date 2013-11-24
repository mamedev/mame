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

#include "netlist.h"
#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"
#include "netlist/devices/net_lib.h"

// ----------------------------------------------------------------------------------------
// netlist_mame_device
// ----------------------------------------------------------------------------------------

const device_type NETLIST = &device_creator<netlist_mame_device>;

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NETLIST, "netlist", tag, owner, clock, "netlist_mame", __FILE__),
		device_execute_interface(mconfig, *this),
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
}

void netlist_mame_device::device_config_complete()
{
}

void netlist_mame_device::device_start()
{
	//double dt = clocks_to_attotime(1).as_double();
	m_netlist = global_alloc_clear(netlist_t(*this));
	m_netlist->set_clock_freq(this->clock());

	m_setup = global_alloc_clear(netlist_setup_t(*m_netlist));

	m_setup_func(*m_setup);

	m_setup->start_devices();

	bool allok = true;
	for (device_start_list_t::entry_t *ods = m_device_start_list.first(); ods != NULL; ods = m_device_start_list.next(ods))
		allok &= ods->object()->OnDeviceStart();

	if (!allok)
		fatalerror("required elements not found\n");

	m_setup->resolve_inputs();

	save_state();
	/* TODO: we have to save the round robin queue as well */

	// set our instruction counter
	m_icountptr = &m_icount;
}

void netlist_mame_device::device_reset()
{
	m_netlist->reset();
	m_setup->step_devices_once();
}

void netlist_mame_device::device_stop()
{
	m_setup->print_stats();

	global_free(m_setup);
	m_setup = NULL;
	global_free(m_netlist);
	m_netlist = NULL;
}

void netlist_mame_device::device_post_load()
{
}

void netlist_mame_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

void netlist_mame_device::save_state()
{
#if 0
	for (netlist_setup_t::tagmap_output_t::entry_t *entry = m_setup->m_outputs.first(); entry != NULL; entry = m_setup->m_outputs.next(entry))
	{
		save_item(*entry->object()->Q_ptr(), entry->tag().cstr(), 0);
		save_item(*entry->object()->new_Q_ptr(), entry->tag().cstr(), 1);
	}
#endif
}

UINT64 netlist_mame_device::execute_clocks_to_cycles(UINT64 clocks) const
{
	return clocks;
}

UINT64 netlist_mame_device::execute_cycles_to_clocks(UINT64 cycles) const
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
