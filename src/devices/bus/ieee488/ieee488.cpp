// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IEEE-488.1 General Purpose Interface Bus emulation
    (aka HP-IB, GPIB, CBM IEEE)

**********************************************************************/

#include "emu.h"
#include "ieee488.h"

#define VERBOSE 0
#include "logmacro.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


static const char *const SIGNAL_NAME[] = { "EOI", "DAV", "NRFD", "NDAC", "IFC", "SRQ", "ATN", "REN", "DIO" };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(IEEE488,      ieee488_device,      "ieee488",      "IEEE-488 bus")
DEFINE_DEVICE_TYPE(IEEE488_SLOT, ieee488_slot_device, "ieee488_slot", "IEEE-488 slot")



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ieee488_interface - constructor
//-------------------------------------------------

device_ieee488_interface::device_ieee488_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "ieee488"), m_bus(nullptr), m_slot(nullptr), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_ieee488_interface - destructor
//-------------------------------------------------

device_ieee488_interface::~device_ieee488_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ieee488_slot_device - constructor
//-------------------------------------------------

ieee488_slot_device::ieee488_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IEEE488_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_address(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ieee488_slot_device::device_start()
{
	ieee488_device* bus = nullptr;

	for (device_t *device = owner(); device != nullptr; device = device->owner())
	{
		bus = device->subdevice<ieee488_device>(IEEE488_TAG);
		if (bus != nullptr) break;
	}

	assert(bus);

	device_ieee488_interface *dev = dynamic_cast<device_ieee488_interface *>(get_card_device());
	if (dev) bus->add_device(this, get_card_device());
}


void ieee488_slot_device::add_cbm_slot(machine_config &config, const char *_tag, int _address, const char *_def_slot)
{
	ieee488_slot_device &slot(IEEE488_SLOT(config, _tag));
	cbm_ieee488_devices(slot);
	slot.set_default_option(_def_slot);
	slot.set_address(_address);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ieee488_device - constructor
//-------------------------------------------------

ieee488_device::ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IEEE488, tag, owner, clock),
	m_write_eoi(*this),
	m_write_dav(*this),
	m_write_nrfd(*this),
	m_write_ndac(*this),
	m_write_ifc(*this),
	m_write_srq(*this),
	m_write_atn(*this),
	m_write_ren(*this),
	m_write_dio(*this),
	m_update_timer(nullptr)
{
	m_lines.reset();
	std::fill(std::begin(m_line), std::end(m_line), 1);
	m_line[DIO] = 0xff;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ieee488_device::device_start()
{
	m_update_timer = timer_alloc(FUNC(ieee488_device::update_tick), this);

	// a bus hosted by a cartridge has its host CPU outside the bus owner
	for (device_execute_interface &exec : execute_interface_enumerator(machine().root_device()))
		m_execs.push_back(&exec);

	save_item(NAME(m_lines.m_time));
	save_item(NAME(m_lines.m_state));
	save_item(NAME(m_lines.m_count));
	save_item(NAME(m_line));
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ieee488_device::device_stop()
{
	m_device_list.reset();
}


//-------------------------------------------------
//  add_device -
//-------------------------------------------------

void ieee488_device::add_device(ieee488_slot_device *slot, device_t *target)
{
	auto entry = new daisy_entry(target);

	entry->m_interface->m_bus = this;
	entry->m_interface->m_slot = slot;

	m_device_list.append(*entry);
	save_item(NAME(entry->m_lines.m_time), m_device_list.count());
	save_item(NAME(entry->m_lines.m_state), m_device_list.count());
	save_item(NAME(entry->m_lines.m_count), m_device_list.count());
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

ieee488_device::daisy_entry::daisy_entry(device_t *device)
	: m_next(nullptr),
		m_device(device),
		m_interface(nullptr)
{
	m_lines.reset();

	device->interface(m_interface);
}


//-------------------------------------------------
//  source_lines::reset -
//-------------------------------------------------

void ieee488_device::source_lines::reset()
{
	for (int signal = 0; signal < SIGNAL_COUNT; signal++)
	{
		std::fill(std::begin(m_time[signal]), std::end(m_time[signal]), attotime::zero);
		std::fill(std::begin(m_state[signal]), std::end(m_state[signal]), (signal == DIO) ? 0xff : 1);
		m_count[signal] = 1;
	}
}


//-------------------------------------------------
//  set_signal -
//-------------------------------------------------

void ieee488_device::set_signal(device_t *device, int signal, u8 state)
{
	source_lines *lines = &m_lines;

	if (device != this)
	{
		daisy_entry *entry = m_device_list.first();
		while (entry && entry->m_device != device)
			entry = entry->next();
		if (!entry)
			return;
		lines = &entry->m_lines;
	}

	if (signal != DIO)
		state = state ? 1 : 0;
	u8 &count = lines->m_count[signal];
	attotime *const times = lines->m_time[signal];
	u8 *const states = lines->m_state[signal];
	if (states[count - 1] == state)
		return;

	// port outputs change after the write cycle, half a cycle after reads sample their inputs
	attotime const now = machine().time();
	attotime time = now;
	device_execute_interface *const exec = machine().scheduler().currently_executing();
	if (exec)
	{
		attotime const cycle = exec->cycles_to_attotime(1);
		if (!cycle.is_never())
			time += cycle / 2;
	}
	time = std::max(time, times[count - 1]);

	LOG("%s '%s' %s %02x at %s\n", machine().describe_context(), device->tag(), SIGNAL_NAME[signal], state, time.as_string());

	if (times[count - 1] == time)
	{
		if (count > 1 && states[count - 2] == state)
			count--;
		else
			states[count - 1] = state;
	}
	else
	{
		if (count == EDGE_COUNT)
		{
			std::copy(times + 1, times + count, times);
			std::copy(states + 1, states + count, states);
			count--;
		}
		times[count] = time;
		states[count] = state;
		count++;
	}

	if (time <= now)
		update_lines();
	else if (!m_update_timer->enabled() || m_update_timer->expire() > time)
		m_update_timer->adjust(time - now);
}


//-------------------------------------------------
//  update_lines - notify devices of resolved
//  line changes up to the current time
//-------------------------------------------------

void ieee488_device::update_lines()
{
	attotime const now = machine().time();

	for (int signal = 0; signal < SIGNAL_COUNT; signal++)
	{
		u8 const state = get_signal(signal, now);
		if (m_line[signal] == state)
			continue;

		m_line[signal] = state;

		switch (signal)
		{
		case EOI:   m_write_eoi(state);  break;
		case DAV:   m_write_dav(state);  break;
		case NRFD:  m_write_nrfd(state); break;
		case NDAC:  m_write_ndac(state); break;
		case IFC:   m_write_ifc(state);  break;
		case SRQ:   m_write_srq(state);  break;
		case ATN:   m_write_atn(state);  break;
		case REN:   m_write_ren(state);  break;
		case DIO:   m_write_dio(state);  break;
		}

		for (daisy_entry *entry = m_device_list.first(); (signal != DIO) && entry; entry = entry->next())
		{
			switch (signal)
			{
			case EOI:   entry->m_interface->ieee488_eoi(state);  break;
			case DAV:   entry->m_interface->ieee488_dav(state);  break;
			case NRFD:  entry->m_interface->ieee488_nrfd(state); break;
			case NDAC:  entry->m_interface->ieee488_ndac(state); break;
			case IFC:   entry->m_interface->ieee488_ifc(state);  break;
			case SRQ:   entry->m_interface->ieee488_srq(state);  break;
			case ATN:   entry->m_interface->ieee488_atn(state);  break;
			case REN:   entry->m_interface->ieee488_ren(state);  break;
			}
		}

		LOG("EOI %u DAV %u NRFD %u NDAC %u IFC %u SRQ %u ATN %u REN %u DIO %02x\n",
			m_line[EOI], m_line[DAV], m_line[NRFD], m_line[NDAC],
			m_line[IFC], m_line[SRQ], m_line[ATN], m_line[REN], m_line[DIO]);
	}
}


//-------------------------------------------------
//  update_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ieee488_device::update_tick)
{
	update_lines();

	attotime const now = machine().time();
	attotime next = attotime::never;
	auto const find_next = [&now, &next] (source_lines const &lines)
	{
		for (int signal = 0; signal < SIGNAL_COUNT; signal++)
			for (int i = lines.m_count[signal] - 1; (i >= 0) && (lines.m_time[signal][i] > now); i--)
				next = std::min(next, lines.m_time[signal][i]);
	};

	find_next(m_lines);
	for (daisy_entry *entry = m_device_list.first(); entry; entry = entry->next())
		find_next(entry->m_lines);

	if (next.is_never())
		m_update_timer->reset();
	else
		m_update_timer->adjust(next - now);
}


//-------------------------------------------------
//  sample_ready - hold a CPU's bus sample until
//  every other device has passed the point where
//  its outputs could still change the result
//-------------------------------------------------

bool ieee488_device::sample_ready(cpu_device &cpu)
{
	if (!cpu.executing() || machine().side_effects_disabled())
		return true;

	attotime const time = cpu.local_time();
	bool ready = !m_update_timer->enabled() || (m_update_timer->expire() > time);
	attotime stalled = attotime::never;

	for (device_execute_interface *const exec : m_execs)
	{
		if (!ready)
			break;

		if ((&exec->device() == &cpu) || exec->suspended(SUSPEND_ANY_REASON & ~SUSPEND_REASON_TIMESLICE))
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
			cpu.yield();
			machine().scheduler().perfect_quantum(stalled);
		}
		cpu.retry_access();
	}

	return ready;
}


//-------------------------------------------------
//  get_signal -
//-------------------------------------------------

u8 ieee488_device::get_signal(int signal, const attotime &time)
{
	auto const source_state = [signal, &time] (source_lines const &lines)
	{
		int i = lines.m_count[signal] - 1;
		while ((i > 0) && (lines.m_time[signal][i] > time))
			i--;
		return lines.m_state[signal][i];
	};

	u8 state = source_state(m_lines);

	for (daisy_entry *entry = m_device_list.first(); state && entry; entry = entry->next())
		state &= source_state(entry->m_lines);

	return state;
}


//-------------------------------------------------
//  SLOT_INTERFACE( cbm_ieee488_devices )
//-------------------------------------------------

// slot devices
#include "bus/cbmiec/c1526.h"
#include "bus/cbmiec/msdsd.h"
#include "c2031.h"
#include "c2040.h"
#include "c3022.h"
#include "c4022.h"
#include "c6400.h"
#include "c8023p.h"
#include "c8028.h"
#include "c8050.h"
#include "c8280.h"
#include "d9060.h"
#include "hardbox.h"
#include "shark.h"
#include "softbox.h"

void cbm_ieee488_devices(device_slot_interface &device)
{
	device.option_add("c2040", GPIB_C2040);
	device.option_add("c3040", GPIB_C3040);
	device.option_add("c4040", GPIB_C4040);
	device.option_add("c8050", GPIB_C8050);
	device.option_add("c8250", GPIB_C8250);
	device.option_add("sfd1001", GPIB_SFD1001);
	device.option_add("c2031", GPIB_C2031);
	device.option_add("c8280", GPIB_C8280);
	device.option_add("d9060", GPIB_D9060);
	device.option_add("d9090", GPIB_D9090);
	device.option_add("softbox", GPIB_SOFTBOX);
	device.option_add("hardbox", GPIB_HARDBOX);
	device.option_add("msdsd1", GPIB_MSD_SD1);
	device.option_add("msdsd2", GPIB_MSD_SD2);
	device.option_add("shark", GPIB_MSHARK);
	device.option_add("c3022", GPIB_C3022);
	device.option_add("c4022", GPIB_C4022);
	device.option_add("c4023", GPIB_C4023);
	device.option_add("c6400", GPIB_C6400);
	device.option_add("c8023p", GPIB_C8023P);
	device.option_add("c8028", GPIB_C8028);
}

//-------------------------------------------------
//  SLOT_INTERFACE( hp_ieee488_devices )
//-------------------------------------------------

// slot devices
#include "hp9122c.h"
#include "hp9133.h"
#include "hp9895.h"

void hp_ieee488_devices(device_slot_interface &device)
{
	device.option_add("hp9122c", GPIB_HP9122C);
	device.option_add("hp9133", GPIB_HP9133);
	device.option_add("hp9895", GPIB_HP9895);
}

//-------------------------------------------------
//  SLOT_INTERFACE( remote488_devices )
//-------------------------------------------------

// slot devices
#include "remote488.h"

void remote488_devices(device_slot_interface &device)
{
	device.option_add("remote488", GPIB_REMOTE488);
}


//-------------------------------------------------
//  SLOT_INTERFACE( grid_ieee488_devices )
//-------------------------------------------------

// slot devices
#include "grid2102.h"

void grid_ieee488_devices(device_slot_interface &device)
{
	device.option_add("grid2102", GPIB_GRID2102).clock(XTAL(4'000'000));
	device.option_add("grid2101_floppy", GPIB_GRID2101_FLOPPY).clock(XTAL(4'000'000));
	device.option_add("grid2101_hdd", GPIB_GRID2101_HDD).clock(XTAL(4'000'000));
}
