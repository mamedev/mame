// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD parallel cable (RAMLink <-> CMD HD) emulation

**********************************************************************/

#include "emu.h"
#include "cmdpar.h"



//**************************************************************************
//  DEVICE CMD PARALLEL INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cmd_parallel_interface - constructor
//-------------------------------------------------

device_cmd_parallel_interface::device_cmd_parallel_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "cmdpar"),
	m_data_out(0xff),
	m_lines_out(0xff)
{
}


//-------------------------------------------------
//  ~device_cmd_parallel_interface - destructor
//-------------------------------------------------

device_cmd_parallel_interface::~device_cmd_parallel_interface()
{
}


//-------------------------------------------------
//  interface_pre_start - find the other end
//-------------------------------------------------

void device_cmd_parallel_interface::interface_pre_start()
{
	m_peers.clear();
	m_execs.clear();

	for (device_cmd_parallel_interface &peer : device_interface_enumerator<device_cmd_parallel_interface>(device().machine().root_device()))
		if (&peer != this)
			m_peers.push_back(&peer);

	for (device_execute_interface &exec : execute_interface_enumerator(device().machine().root_device()))
		m_execs.push_back(&exec);
}


//-------------------------------------------------
//  interface_post_start -
//-------------------------------------------------

void device_cmd_parallel_interface::interface_post_start()
{
	device().save_item(NAME(m_data_out));
	device().save_item(NAME(m_lines_out));
}


//-------------------------------------------------
//  cmdpar_data_w -
//-------------------------------------------------

void device_cmd_parallel_interface::cmdpar_data_w(u8 data)
{
	m_data_out = data;
}


//-------------------------------------------------
//  cmdpar_lines_w -
//-------------------------------------------------

void device_cmd_parallel_interface::cmdpar_lines_w(u8 lines)
{
	u8 const old = cmdpar_lines_r();

	m_lines_out = lines | 0xf0;

	u8 const state = cmdpar_lines_r();

	if (state != old)
		for (device_cmd_parallel_interface *peer : m_peers)
			peer->cmdpar_lines_changed(state, old);
}


//-------------------------------------------------
//  cmdpar_lines_changed - another end changed
//  the cable's control lines
//-------------------------------------------------

void device_cmd_parallel_interface::cmdpar_lines_changed(u8 lines, u8 old)
{
}


//-------------------------------------------------
//  cmdpar_data_r -
//-------------------------------------------------

u8 device_cmd_parallel_interface::cmdpar_data_r() const
{
	u8 data = m_data_out;

	for (device_cmd_parallel_interface const *peer : m_peers)
		data &= peer->m_data_out;

	return data;
}


//-------------------------------------------------
//  cmdpar_lines_r -
//-------------------------------------------------

u8 device_cmd_parallel_interface::cmdpar_lines_r() const
{
	u8 lines = m_lines_out;

	for (device_cmd_parallel_interface const *peer : m_peers)
		lines &= peer->m_lines_out;

	return lines;
}


//-------------------------------------------------
//  cmdpar_sample_ready - hold the accessing CPU
//  until every other CPU has caught up with it,
//  so cable accesses happen in emulated time order
//-------------------------------------------------

bool device_cmd_parallel_interface::cmdpar_sample_ready()
{
	if (m_peers.empty() || device().machine().side_effects_disabled())
		return true;

	device_execute_interface *const executing = device().machine().scheduler().currently_executing();
	if (!executing)
		return true;

	cpu_device *const cpu = dynamic_cast<cpu_device *>(&executing->device());
	if (!cpu || !cpu->executing())
		return true;

	attotime const time = cpu->local_time();
	bool ready = true;
	attotime stalled = attotime::never;

	for (device_execute_interface *const exec : m_execs)
	{
		if (!ready)
			break;

		if ((exec == executing) || exec->suspended(SUSPEND_ANY_REASON & ~SUSPEND_REASON_TIMESLICE))
			continue;

		attotime const cycle = exec->cycles_to_attotime(1);
		if (!cycle.is_never())
		{
			ready = (exec->local_time() + (cycle / 2)) > time;
			if (!ready && ((exec->local_time() + cycle) > time))
				stalled = cycle;
		}
	}

	if (!ready)
	{
		// a device less than a cycle behind can only run once this slice ends
		if (!stalled.is_never())
		{
			cpu->yield();
			device().machine().scheduler().perfect_quantum(stalled);
		}
		cpu->retry_access();
	}

	return ready;
}
