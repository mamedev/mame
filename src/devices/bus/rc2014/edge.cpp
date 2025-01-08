// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Modular Backplane Extensions

****************************************************************************/

#include "emu.h"
#include "edge.h"
#include "modules.h"

namespace {

//**************************************************************************
//  SC106 - Modular Backplane (RC2014)
//  Module author: Stephen C Cousins
//**************************************************************************

class sc106_device : public device_t, public device_rc2014_rc80_card_interface
{
public:
	// construction/destruction
	sc106_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	sc106_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<rc2014_rc80_bus_device> m_rc80_bus;
};

sc106_device::sc106_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rc2014_rc80_card_interface(mconfig, *this)
	, m_rc80_bus(*this, ":bus")
{
}

sc106_device::sc106_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sc106_device(mconfig, RC2014_SC106, tag, owner, clock)
{
}

void sc106_device::device_start()
{
}

void sc106_device::device_add_mconfig(machine_config &config)
{
	RC2014_RC80_SLOT(config, "1", m_rc80_bus, rc2014_rc80_bus_modules, nullptr);
	RC2014_RC80_SLOT(config, "2", m_rc80_bus, rc2014_rc80_bus_modules, nullptr);
	RC2014_RC80_SLOT(config, "3", m_rc80_bus, rc2014_rc80_bus_modules, nullptr);
	RC2014_RC80_SLOT(config, "4", m_rc80_bus, rc2014_rc80_bus_modules, nullptr);
	RC2014_RC80_SLOT(config, "5", m_rc80_bus, rc2014_rc80_bus_modules, nullptr);
	RC2014_RC80_SLOT(config, "6", m_rc80_bus, rc2014_rc80_bus_modules, nullptr);
	RC2014_RC80_SLOT(config, "e", m_rc80_bus, rc2014_rc80_bus_edge_modules, nullptr);

}

//**************************************************************************
//  SC107 - Modular Backplane (RC2014)
//  Module author: Stephen C Cousins
//
//  TODO: Pins 37 and 38 form an interrupt daisy chain
//**************************************************************************

class sc107_device : public sc106_device
{
	public:
	// construction/destruction
	sc107_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

sc107_device::sc107_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sc106_device(mconfig, RC2014_SC107, tag, owner, clock)
{
}

//**************************************************************************
//  SC113 - Modular Backplane (RC2014)
//  Module author: Stephen C Cousins
//
//  TODO: Jumpers on board
//**************************************************************************

class sc113_device : public sc106_device
{
	public:
	// construction/destruction
	sc113_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

sc113_device::sc113_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sc106_device(mconfig, RC2014_SC113, tag, owner, clock)
{
}

//**************************************************************************
//  SC141 - Modular Backplane (RC2014)
//  Module author: Stephen C Cousins
//**************************************************************************

class sc141_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	sc141_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<rc2014_bus_device> m_rc40_bus;
};

sc141_device::sc141_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_SC141, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_rc40_bus(*this, ":bus")
{
}

void sc141_device::device_start()
{
}

void sc141_device::device_add_mconfig(machine_config &config)
{
	RC2014_SLOT(config, "1", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "2", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "3", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "4", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "5", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "6", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "7", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "8", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "9", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "10", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "11", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "12", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "e",  m_rc40_bus, rc2014_bus_edge_modules, nullptr);
}

//**************************************************************************
//  SC147 - Modular Backplane (RC2014)
//  Module author: Stephen C Cousins
//**************************************************************************

class sc147_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	sc147_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<rc2014_bus_device> m_rc40_bus;
};

sc147_device::sc147_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_SC147, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_rc40_bus(*this, ":bus")
{
}

void sc147_device::device_start()
{
}

void sc147_device::device_add_mconfig(machine_config &config)
{
	RC2014_SLOT(config, "1", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "2", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "3", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "4", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "5", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "6", m_rc40_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "e", m_rc40_bus, rc2014_bus_edge_modules, nullptr);
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SC106, device_rc2014_rc80_card_interface, sc106_device, "rc2014_sc106", "SC106 - Modular Backplane (RC2014)")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SC107, device_rc2014_rc80_card_interface, sc107_device, "rc2014_sc107", "SC107 - Modular Backplane (RC2014)")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SC113, device_rc2014_rc80_card_interface, sc113_device, "rc2014_sc113", "SC113 - Modular Backplane (RC2014)")

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SC141, device_rc2014_card_interface, sc141_device, "rc2014_sc141", "SC141 - Modular Backplane (RC2014)")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SC147, device_rc2014_card_interface, sc147_device, "rc2014_sc147", "SC147 - Modular Backplane (RC2014)")
