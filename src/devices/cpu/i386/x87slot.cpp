// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    x87 coprocessor sockets for CPUs without an on-chip FPU

***************************************************************************/

#include "emu.h"
#include "x87slot.h"

#include "i386.h"


// the coprocessors, only reachable through their sockets
DECLARE_DEVICE_TYPE(I387,   i387_device)
DECLARE_DEVICE_TYPE(I387SX, i387sx_device)
DECLARE_DEVICE_TYPE(I487SX, i487sx_device)

class x87_coprocessor_device : public device_t, public device_x87_interface
{
protected:
	x87_coprocessor_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_x87_interface(mconfig, *this)
	{
	}

	virtual void device_start() override ATTR_COLD { }
};

class i387_device : public x87_coprocessor_device
{
public:
	i387_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: x87_coprocessor_device(mconfig, I387, tag, owner, clock)
	{
	}
};

class i387sx_device : public x87_coprocessor_device
{
public:
	i387sx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: x87_coprocessor_device(mconfig, I387SX, tag, owner, clock)
	{
	}
};

class i487sx_device : public x87_coprocessor_device
{
public:
	i487sx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: x87_coprocessor_device(mconfig, I487SX, tag, owner, clock)
	{
	}
};

DEFINE_DEVICE_TYPE(I387,   i387_device,   "i387",   "Intel 80387")
DEFINE_DEVICE_TYPE(I387SX, i387sx_device, "i387sx", "Intel 80387SX")
DEFINE_DEVICE_TYPE(I487SX, i487sx_device, "i487sx", "Intel 487SX")

DEFINE_DEVICE_TYPE(I387_SLOT,   i387_slot_device,   "i387_slot",   "80387 coprocessor socket")
DEFINE_DEVICE_TYPE(I387SX_SLOT, i387sx_slot_device, "i387sx_slot", "80387SX coprocessor socket")
DEFINE_DEVICE_TYPE(I487SX_SLOT, i487sx_slot_device, "i487sx_slot", "487SX upgrade socket")


device_x87_interface::device_x87_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "x87")
{
}


x87_slot_device::x87_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const char *option, device_type card)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_x87_interface>(mconfig, *this)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_option(option)
{
	option_reset();
	option_add(option, card);
	set_default_option(nullptr);
	set_fixed(false);
}

void x87_slot_device::device_resolve_objects()
{
	// every device resolves before any starts, so the CPU knows when it starts
	downcast<i386_device &>(*m_cpu).m_x87_present = get_card_device() != nullptr;
}

void x87_slot_device::device_start()
{
}


i387_slot_device::i387_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: x87_slot_device(mconfig, I387_SLOT, tag, owner, clock, "i387", I387)
{
}

i387sx_slot_device::i387sx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: x87_slot_device(mconfig, I387SX_SLOT, tag, owner, clock, "i387sx", I387SX)
{
}

i487sx_slot_device::i487sx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: x87_slot_device(mconfig, I487SX_SLOT, tag, owner, clock, "i487sx", I487SX)
{
}
