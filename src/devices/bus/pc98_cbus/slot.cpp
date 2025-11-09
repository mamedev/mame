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


DEFINE_DEVICE_TYPE(PC98_CBUS_ROOT, pc98_cbus_root_device, "pc98_cbus_root", "PC-98 C-Bus root")
DEFINE_DEVICE_TYPE(PC98_CBUS_SLOT, pc98_cbus_slot_device, "pc98_cbus_slot", "PC-98 C-Bus slot")

//**************************************************************************
//  DEVICE PC9801 ROOT INTERFACE
//**************************************************************************

pc98_cbus_root_device::pc98_cbus_root_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PC98_CBUS_ROOT, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_mem_config("mem_space", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor())
	, m_space_io_config("io_space", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor())
{
}

device_memory_interface::space_config_vector pc98_cbus_root_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_mem_config),
		std::make_pair(AS_IO, &m_space_io_config)
	};
}

void pc98_cbus_root_device::device_start()
{
}

void pc98_cbus_root_device::device_config_complete()
{
	// ...
}


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

pc98_cbus_slot_device::pc98_cbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC98_CBUS_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_memspace(*this, finder_base::DUMMY_TAG, -1)
	, m_iospace(*this, finder_base::DUMMY_TAG, -1)
	, m_int_cb(*this)
//  , m_drq_cb(*this)
//  , m_dma_in_cb(*this, 0)
//  , m_dma_out_cb(*this)
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

