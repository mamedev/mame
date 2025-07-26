#ifndef MAME_MACHINE_PL6_EXP_H
#define MAME_MACHINE_PL6_EXP_H

#pragma once

#include "emu.h"

#include "machine/ram.h"
#include "video/mb86292.h"


DECLARE_DEVICE_TYPE(PLUTO6_EXPANSION_SLOT, pluto6_expansion_slot_device)

class pluto6_expansion_card_interface;

// Bus
class pluto6_expansion_slot_device : public device_t, public device_single_card_slot_interface<pluto6_expansion_card_interface>
{
	friend class pluto6_expansion_card_interface;

public:
	pluto6_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;


private:
	pluto6_expansion_card_interface *m_dev;
};

// Base card interface
class pluto6_expansion_card_interface : public device_interface
{
	friend class pluto6_expansion_slot_device;
public:
	virtual ~pluto6_expansion_card_interface(){}

protected:
	pluto6_expansion_card_interface(const machine_config &mconfig, device_t &device);

	pluto6_expansion_slot_device *m_slot;
};

// Calypso32 GPU (8MB VRAM)
DECLARE_DEVICE_TYPE(HEBER_CALYPSO_GPU, pluto6_calypso32_device)

class pluto6_calypso32_device : public device_t, public pluto6_expansion_card_interface
{
	public:
		pluto6_calypso32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	protected:
		pluto6_calypso32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

		virtual void device_start() override ATTR_COLD {};
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	private:
		required_device<mb86292_device> m_gpu;
		required_device<ram_device> m_vram;
};


#endif // MAME_MACHINE_PL6_EXP_H
