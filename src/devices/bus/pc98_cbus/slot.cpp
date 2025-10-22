// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

C-bus slot interface for PC-98xx family

a.k.a. NEC version of the ISA bus.
C-bus -> Compatible Bus

References:
- https://98epjunk.shakunage.net/sw/ext_card/ext_card.html
- https://ja.wikipedia.org/wiki/C%E3%83%90%E3%82%B9
- https://www.pc-9800.net/db2/db2_ga_index.htm
- http://ookumaneko.s1005.xrea.com/pcibios.htm (PCI era mapping)

TODO:
- stub interface, checkout what actually belongs here
- move pc9801_cbus_devices declaration from pc9801 driver in here;
- 8-bit I/O smearing should be handled here;
- INT# should be handled here too;
- Best way to inform user when it tries to install incompatible boards?
- Support for PCI bridging on later machines (cfr. pc9821cx3);

**************************************************************************************************/

#include "emu.h"

#include "slot.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PC98_CBUS_SLOT, pc98_cbus_slot_device, "pc98_cbus_slot", "PC-98 C-Bus slot")



//**************************************************************************
//  DEVICE PC9801 CARD INTERFACE
//**************************************************************************

#if 0
//-------------------------------------------------
//  device_pc9801cbus_card_interface - constructor
//-------------------------------------------------

device_pc9801cbus_card_interface::device_pc9801cbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_pc9801cbus_card_interface - destructor
//-------------------------------------------------

device_pc9801cbus_card_interface::~device_pc9801cbus_card_interface()
{
}
#endif


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc98_cbus_slot_device - constructor
//-------------------------------------------------

pc98_cbus_slot_device::pc98_cbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC98_CBUS_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_memspace(*this, finder_base::DUMMY_TAG, -1),
	m_iospace(*this, finder_base::DUMMY_TAG, -1),
	m_int_callback(*this)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pc98_cbus_slot_device::device_config_complete()
{
	// ...
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc98_cbus_slot_device::device_start()
{
//  m_card = dynamic_cast<device_pc9801_slot_card_interface *>(get_card_device());
}

template<typename R, typename W> void pc98_cbus_slot_device::install_io(offs_t start, offs_t end, R rhandler, W whandler)
{
	int buswidth = m_iospace->data_width();
	switch(buswidth)
	{
		case 8:
			m_iospace->install_readwrite_handler(start, end, rhandler, whandler, 0);
			break;
		case 16:
			m_iospace->install_readwrite_handler(start, end, rhandler, whandler, 0xffff);
			break;
		case 32:
			m_iospace->install_readwrite_handler(start, end, rhandler, whandler, 0xffffffff);
			break;
		default:
			fatalerror("PC-9801 C-bus: Bus width %d not supported\n", buswidth);
	}
}

template void pc98_cbus_slot_device::install_io<read8_delegate,    write8_delegate   >(offs_t start, offs_t end, read8_delegate rhandler,    write8_delegate whandler);
template void pc98_cbus_slot_device::install_io<read8s_delegate,   write8s_delegate  >(offs_t start, offs_t end, read8s_delegate rhandler,   write8s_delegate whandler);
template void pc98_cbus_slot_device::install_io<read8sm_delegate,  write8sm_delegate >(offs_t start, offs_t end, read8sm_delegate rhandler,  write8sm_delegate whandler);
template void pc98_cbus_slot_device::install_io<read8smo_delegate, write8smo_delegate>(offs_t start, offs_t end, read8smo_delegate rhandler, write8smo_delegate whandler);

// boilerplate code for boards that has configurable I/O with either Jumpers or Dip-Switches
// NB: client must have a mechanism to remember what port has been used before and after calling this,
// in order to avoid "last instantiated wins" issues with overlapping board full configs.
// TODO: refactor to actually be useful for PCI archs
void pc98_cbus_slot_device::flush_install_io(const char *client_tag, u16 old_io, u16 new_io, u16 size, read8sm_delegate rhandler, write8sm_delegate whandler)
{
	// initialize if client have this unmapped (such as first boot)
	// device_start fns cannot read input ports ...
	if (old_io == 0)
		old_io = new_io;

	logerror("%s: %s uninstall I/O at %04x-%04x\n",
		this->tag(),
		client_tag,
		old_io,
		old_io + size
	);
	this->io_space().unmap_readwrite(old_io, old_io + size);

	logerror("%s: %s install I/O at %04x-%04x\n",
		this->tag(),
		client_tag,
		new_io,
		new_io + size
	);
	this->install_io(
		new_io,
		new_io + size,
		rhandler,
		whandler
	);
}
