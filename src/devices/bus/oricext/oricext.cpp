// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "oricext.h"
#include "jasmin.h"
#include "microdisc.h"

const device_type ORICEXT_CONNECTOR = &device_creator<oricext_connector>;

oricext_connector::oricext_connector(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ORICEXT_CONNECTOR, "ORIC extension connector", tag, owner, clock, "oricext_connector", __FILE__),
	device_slot_interface(mconfig, *this),
	irq_handler(*this)
{
}

oricext_connector::~oricext_connector()
{
}

void oricext_connector::set_cputag(std::string tag)
{
	cputag = tag;
}

void oricext_connector::device_start()
{
	irq_handler.resolve_safe();
}

void oricext_connector::irq_w(int state)
{
	irq_handler(state);
}

void oricext_connector::device_config_complete()
{
	oricext_device *dev = dynamic_cast<oricext_device *>(get_card_device());
	if(dev)
		dev->set_cputag(cputag);
}

oricext_device::oricext_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_slot_card_interface(mconfig, *this),	
	cpu(nullptr),
	connector(nullptr),
	bank_c000_r(nullptr),
	bank_e000_r(nullptr),
	bank_f800_r(nullptr),
	bank_c000_w(nullptr),
	bank_e000_w(nullptr),
	bank_f800_w(nullptr),
	rom(nullptr),
	ram(nullptr)
{
}

void oricext_device::set_cputag(std::string tag)
{
	cputag = tag;
}

void oricext_device::device_start()
{
	cpu = machine().device<m6502_device>(cputag);
	connector = downcast<oricext_connector *>(owner());
	bank_c000_r = membank(":bank_c000_r");
	bank_e000_r = membank(":bank_e000_r");
	bank_f800_r = membank(":bank_f800_r");
	bank_c000_w = membank(":bank_c000_w");
	bank_e000_w = membank(":bank_e000_w");
	bank_f800_w = membank(":bank_f800_w");
	rom = (UINT8 *)machine().root_device().memregion(cputag)->base();
	ram = (UINT8 *)memshare(":ram")->ptr();

	memset(junk_read, 0xff, sizeof(junk_read));
	memset(junk_write, 0x00, sizeof(junk_write));
}

WRITE_LINE_MEMBER(oricext_device::irq_w)
{
	connector->irq_w(state);
}

SLOT_INTERFACE_START(oricext_intf)
	SLOT_INTERFACE("jasmin", JASMIN)
	SLOT_INTERFACE("microdisc", MICRODISC)
SLOT_INTERFACE_END
