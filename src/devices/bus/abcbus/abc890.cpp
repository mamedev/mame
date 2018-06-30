// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 890 bus expander emulation

*********************************************************************/

#include "emu.h"
#include "abc890.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC_EXPANSION_UNIT, abc_expansion_unit_device, "abcexp", "ABC Expansion Unit")
DEFINE_DEVICE_TYPE(ABC890,             abc890_device,             "abc890", "ABC 890")
DEFINE_DEVICE_TYPE(ABC894,             abc894_device,             "abc894", "ABC 894")
DEFINE_DEVICE_TYPE(ABC850,             abc850_device,             "abc850", "ABC 850")
DEFINE_DEVICE_TYPE(ABC852,             abc852_device,             "abc852", "ABC 852")
DEFINE_DEVICE_TYPE(ABC856,             abc856_device,             "abc856", "ABC 856")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(abc890_device::device_add_mconfig)
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("mem1", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("mem2", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("mem3", abcbus_cards, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(abc_expansion_unit_device::device_add_mconfig)
	MCFG_ABCBUS_SLOT_ADD("io1", abc80_cards, "abc830")
	MCFG_ABCBUS_SLOT_ADD("io2", abc80_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io3", abc80_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io4", abc80_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("mem1", abc80_cards, "memcard")
	MCFG_ABCBUS_SLOT_ADD("mem2", abc80_cards, "16k")
	MCFG_ABCBUS_SLOT_ADD("mem3", abc80_cards, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(abc894_device::device_add_mconfig)
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(abc850_device::device_add_mconfig)
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, "abc850fdd")
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, "xebec")
	MCFG_SLOT_OPTION_DEFAULT_BIOS("xebec", "ro202")
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io5", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io6", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io7", abcbus_cards, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(abc852_device::device_add_mconfig)
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, "abc850fdd")
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, "xebec")
	MCFG_SLOT_OPTION_DEFAULT_BIOS("xebec", "basf6185")
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io5", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io6", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io7", abcbus_cards, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(abc856_device::device_add_mconfig)
	MCFG_ABCBUS_SLOT_ADD("io1", abcbus_cards, "abc850fdd")
	MCFG_ABCBUS_SLOT_ADD("io2", abcbus_cards, "xebec")
	MCFG_SLOT_OPTION_DEFAULT_BIOS("xebec", "micr1325")
	MCFG_ABCBUS_SLOT_ADD("io3", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io4", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io5", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io6", abcbus_cards, nullptr)
	MCFG_ABCBUS_SLOT_ADD("io7", abcbus_cards, nullptr)
MACHINE_CONFIG_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc890_device - constructor
//-------------------------------------------------

abc890_device::abc890_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this)
{
}

abc890_device::abc890_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	abc890_device(mconfig, ABC890, tag, owner, clock)
{
}

abc_expansion_unit_device::abc_expansion_unit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	abc890_device(mconfig, ABC_EXPANSION_UNIT, tag, owner, clock)
{
}

abc894_device::abc894_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	abc890_device(mconfig, ABC894, tag, owner, clock)
{
}

abc850_device::abc850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	abc890_device(mconfig, ABC850, tag, owner, clock)
{
}

abc852_device::abc852_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	abc890_device(mconfig, ABC852, tag, owner, clock)
{
}

abc856_device::abc856_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	abc890_device(mconfig, ABC856, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc890_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc890_device::device_reset()
{
	for (device_t &device : subdevices())
	{
		device.reset();
	}
}


//-------------------------------------------------
//  abcbus_cs - card select
//-------------------------------------------------

void abc890_device::abcbus_cs(uint8_t data)
{
	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
		slot.write_cs(data);
}


//-------------------------------------------------
//  abcbus_inp - input
//-------------------------------------------------

uint8_t abc890_device::abcbus_inp()
{
	uint8_t data = 0xff;

	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
		data &= slot.read_inp();

	return data;
}


//-------------------------------------------------
//  abcbus_out - output
//-------------------------------------------------

void abc890_device::abcbus_out(uint8_t data)
{
	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
		slot.write_out(data);
}


//-------------------------------------------------
//  abcbus_stat - status
//-------------------------------------------------

uint8_t abc890_device::abcbus_stat()
{
	uint8_t data = 0xff;

	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
		data &= slot.read_stat();

	return data;
}


//-------------------------------------------------
//  abcbus_c1 - command 1
//-------------------------------------------------

void abc890_device::abcbus_c1(uint8_t data)
{
	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
		slot.write_c1(data);
}


//-------------------------------------------------
//  abcbus_c2 - command 2
//-------------------------------------------------

void abc890_device::abcbus_c2(uint8_t data)
{
	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
		slot.write_c2(data);
}


//-------------------------------------------------
//  abcbus_c3 - command 3
//-------------------------------------------------

void abc890_device::abcbus_c3(uint8_t data)
{
	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
		slot.write_c3(data);
}


//-------------------------------------------------
//  abcbus_c4 - command 4
//-------------------------------------------------

void abc890_device::abcbus_c4(uint8_t data)
{
	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
		slot.write_c4(data);
}


//-------------------------------------------------
//  abcbus_xmemfl - extended memory read
//-------------------------------------------------

uint8_t abc890_device::abcbus_xmemfl(offs_t offset)
{
	uint8_t data = 0xff;

	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
	{
		data &= slot.xmemfl_r(offset);
	}

	return data;
}


//-------------------------------------------------
//  abcbus_xmemw - extended memory write
//-------------------------------------------------

void abc890_device::abcbus_xmemw(offs_t offset, uint8_t data)
{
	for (abcbus_slot_device &slot : abcbus_slot_device_iterator(*this))
	{
		slot.xmemw_w(offset, data);
	}
}
